#include "IOCompletionPort.h"


// WSARecv와 WSASend의 Overlapped I/O 작업 처리를 위한 쓰레드함수
unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	IOCompletionPort* pEvent = (IOCompletionPort*)p;
	pEvent->WokerThread();
	return 0;
}

// 연결처리 쓰레드 함수.
unsigned int WINAPI CallAccepterThread(LPVOID p)
{
	IOCompletionPort* pEvent = (IOCompletionPort*)p;
	pEvent->AccepterThread();
	return 0;
}



IOCompletionPort::IOCompletionPort() :m_hSocketListen(INVALID_SOCKET),
	m_clientCount(0),m_hAccepterThread(INVALID_HANDLE_VALUE),m_hCompletionPort(INVALID_HANDLE_VALUE),
	m_bWorkerRun(true),m_bAccepterRun(true)
{
	// 클라이언트 관리를 위한 ClientInfo 배열 동적(★) 생성.
	m_pClientInfo = new ClientInfo[MAX_CLIENT]();

	for (int i = 0; i < MAX_WORKERTHREAD; ++i)
		m_hWorkerThread[i] = INVALID_HANDLE_VALUE;
	::ZeroMemory(m_socketBuff, sizeof(MAX_SOCKBUF));
}

IOCompletionPort::~IOCompletionPort()
{
	::WSACleanup();
	if (m_pClientInfo)
	{
		delete[] m_pClientInfo;
		m_pClientInfo = nullptr;
	}
}

bool IOCompletionPort::InitSocket()
{
	WSADATA wsaData;

	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "윈속 초기화 에러" << std::endl;
		return false;
	}

	m_hSocketListen = ::socket(AF_INET, SOCK_STREAM, 0);

	if (m_hSocketListen == INVALID_SOCKET)
	{
		std::cout << "ListenSocket 생성 에러" << std::endl;
		return false;
	}

	u_long argp = 1;
	::ioctlsocket(m_hSocketListen, FIONBIO, &argp);

	
	std::cout << "소켓 초기화 성공!" << std::endl;
	return true;
}

void IOCompletionPort::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	struct linger linger = { 0,0 }; // SO_DONTLINGER로 설정

	//bIsForce가 true이면 SO_LINGER, timeout = 0으로 설정하여 강제 종료 (송수신버퍼내용 날림)
	if (bIsForce)
		linger.l_onoff = 1;

	::shutdown(pClientInfo->m_socketClient, SD_BOTH);

	::setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

	::closesocket(pClientInfo->m_socketClient);

	::ZeroMemory(pClientInfo, sizeof(ClientInfo));

}

bool IOCompletionPort::BindandListen(int nBindPort)
{
	SOCKADDR_IN serverAddr;
	::ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(nBindPort);

	int ret = ::bind(m_hSocketListen, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR_IN));
	if (ret == SOCKET_ERROR)
		return false;

	ret = ::listen(m_hSocketListen, MAX_CLIENT);
	if (ret == SOCKET_ERROR)
		return false;

	std::cout << "bind And listen 성공!" << std::endl;

	return true;
}

bool IOCompletionPort::StartServer()
{
	// ★ CP객체 생성
	m_hCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (m_hCompletionPort == INVALID_HANDLE_VALUE)
	{
		std::cout << "CP객체 생성 실패" << std::endl;
		return false;
	}

	bool bRet = CreateWokerThread();
	if (bRet == false)
		return false;
	bRet = CreateAccepterThread();
	if (bRet == false)
		return false;

	std::cout << "서버 시작..." << std::endl;

	return true;
}

bool IOCompletionPort::CreateWokerThread()
{
	UINT threadId = 0;

	//WaitingThread Queue에 대기 상태로 넣을 쓰레드들 생성(쓰레드 풀)
	// 권장되는 개수 (cpu 개수 * 2 ) + 1
	for (int i = 0; i < MAX_WORKERTHREAD; ++i)
	{
		m_hWorkerThread[i] = (HANDLE)::_beginthreadex(NULL, 0, CallWorkerThread, this, CREATE_SUSPENDED, &threadId);
		if (m_hWorkerThread[i] == INVALID_HANDLE_VALUE)
		{
			std::cout << "쓰레드 생성 실패" << std::endl;
			return false;
		}
		::ResumeThread(m_hWorkerThread[i]);
	}

	std::cout << "WorkerThread 시작.." << std::endl;

	return true;
}

bool IOCompletionPort::CreateAccepterThread()
{
	UINT threadId = 0;

	m_hAccepterThread = (HANDLE)::_beginthreadex(NULL, 0, CallAccepterThread, this, CREATE_SUSPENDED, &threadId);

	if (m_hAccepterThread == INVALID_HANDLE_VALUE)
	{
		std::cout << "쓰레드 생성 실패" << std::endl;
		return false;
	}

	::ResumeThread(m_hAccepterThread);

	return true;
}

ClientInfo* IOCompletionPort::GetEmptyClientInfo()
{
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (m_pClientInfo[i].m_socketClient == INVALID_SOCKET)
			return &m_pClientInfo[i];
	}
	return nullptr;
}

bool IOCompletionPort::BindIOCompletionPort(ClientInfo* pClientInfo)
{
	// 소켓과 완료키 CP객체에 등록.
	// 여기서 완료키는 GQCS함수에서 다시 받아볼 수 있음.
	// 완료키는 ClientInfo를 그대로 건네준다.
	// 이 함수의 반환값은 우리가 생성한 CP객체 핸들을 반환.
	HANDLE compareCP;
	compareCP = ::CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient,
		m_hCompletionPort, reinterpret_cast<ULONG_PTR>(pClientInfo),
		0);
	if (compareCP == NULL || compareCP != m_hCompletionPort)
		return false;

	bool check = BindRecv(pClientInfo); // 꼭 한번 Recv요청을 걸어놔야한다.
	while(!check)
		check =BindRecv(pClientInfo);

	return true;
}

bool IOCompletionPort::BindRecv(ClientInfo* pClientInfo)
{
	DWORD flag = 0;
	DWORD recvNumBytes = 0;

	// Overlapped I/O를 위한 정보 세팅
	pClientInfo->m_recvOverlapped.m_wsaBuf.len = MAX_SOCKBUF;
	pClientInfo->m_recvOverlapped.m_wsaBuf.buf = pClientInfo->m_recvOverlapped.m_dataBuffer;
	pClientInfo->m_recvOverlapped.m_ioType = IO_TYPE::IO_RECV;

	// WSARecv 작업 요청 (비동기 I/O = Overlapped I/O)
	int ret = ::WSARecv(pClientInfo->m_socketClient,
		&pClientInfo->m_recvOverlapped.m_wsaBuf, 1, &recvNumBytes,
		&flag, &pClientInfo->m_recvOverlapped.m_wsaOverlapped,
		nullptr);

	if (ret == SOCKET_ERROR && ::WSAGetLastError() != ERROR_IO_PENDING)
		return false;

	return true;
}

bool IOCompletionPort::SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen)
{
	DWORD sendNumBytes = 0;

	// 에코서버니 수신받은 메시지 복사.
	::CopyMemory(pClientInfo->m_sendOverlapped.m_dataBuffer, pMsg, nLen);

	// Overlapped I/O를 위한 정보 세팅
	pClientInfo->m_sendOverlapped.m_wsaBuf.buf = pClientInfo->m_sendOverlapped.m_dataBuffer;
	pClientInfo->m_sendOverlapped.m_wsaBuf.len = nLen;
	pClientInfo->m_sendOverlapped.m_ioType = IO_TYPE::IO_SEND;

	int ret = ::WSASend(pClientInfo->m_socketClient, &pClientInfo->m_sendOverlapped.m_wsaBuf,
		1, &sendNumBytes, 0, &pClientInfo->m_sendOverlapped.m_wsaOverlapped,
		nullptr);

	if (ret == SOCKET_ERROR && ::WSAGetLastError() != ERROR_IO_PENDING)
		return false;

	return true;
}


// 클라이언트 연결 처리하는 함수(AccepterThread가 처리)
void IOCompletionPort::AccepterThread()
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	while (m_bAccepterRun)
	{
		// 접속 받을 구조체의 인덱스를 얻어온다.
		ClientInfo* pClient = GetEmptyClientInfo();
		if (pClient == nullptr)
			return;
		// 클라이언트 접속 요청이 올때까지 대기
		pClient->m_socketClient = ::accept(m_hSocketListen, (SOCKADDR*)&clientAddr, &addrLen);
		if (pClient->m_socketClient == INVALID_SOCKET)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			break;
		}

		// 클라이언트와 연결된 소켓은 CP객체에 등록한다.
		bool ret = BindIOCompletionPort(pClient);
		if (!ret)
			return;

		++m_clientCount;
	}
}


void IOCompletionPort::WokerThread()
{
	//CompletionKey(완료 키)를 받을 포인터 변수
	ClientInfo* cpKey = nullptr;
	// 함수 호출 성공여부
	bool bSuccess = true;
	//Overlapped I/O작업에서 송수신된 데이터 크기
	DWORD transferred = 0;
	// Overlapped I/O작업에서 넘겨준 Overlapped구조체
	LPOVERLAPPED pOverlapped = nullptr;
	OverlappedEx* originOverlapped = nullptr;
	while (m_bWorkerRun)
	{
		// GQCS함수로 인해 쓰레드들은 WatingThread 에 들어가 대기상태가 된다. (쓰레드 풀)
		// 완료된 Overlapped I/O작업이 생기면 IOCP queue(완료큐)에 들어가고 쓰레드들 중 하나가 깨어나
		// 그 작업을 가져와 뒤처리를 맡긴다.
		
		bSuccess = ::GetQueuedCompletionStatus(m_hCompletionPort, &transferred, (ULONG_PTR*)&cpKey,
			&pOverlapped, INFINITE);

		// 사용자가 접속을 끊었을 때.
		if (transferred == 0 && bSuccess == FALSE)
		{
			::closesocket(cpKey->m_socketClient);
			continue;
		}

		// 사용자 쓰레드 종료 메시지 처리
		if (bSuccess == TRUE && transferred == 0 && pOverlapped == nullptr)
		{
			m_bWorkerRun = false;
			continue;
		}
		if (pOverlapped == nullptr)
			continue;

		//캐스팅
		originOverlapped = (OverlappedEx*)pOverlapped;


		bool bRet = IOProcess(cpKey,originOverlapped,transferred);
		if (bRet == false)
		{
			m_bWorkerRun = false;
			continue;
		}
	}

}

bool IOCompletionPort::IOProcess(ClientInfo* pClient,OverlappedEx* pOverlapped,DWORD& transferred)
{
	// RECV처리.(에코니 다시 전송)
	switch (pOverlapped->m_ioType)
	{
	case IO_TYPE::IO_RECV:
		pOverlapped->m_dataBuffer[transferred] = '\0';
		std::cout << "[수신] bytes " << transferred << " msg : " << pOverlapped->m_dataBuffer << std::endl;
		SendMsg(pClient, pOverlapped->m_dataBuffer, transferred);
		break;

	case IO_TYPE::IO_SEND:
		pOverlapped->m_dataBuffer[transferred] = '\0';
		std::cout << "[송신] bytes : " << transferred << " msg : " << pOverlapped->m_dataBuffer << std::endl;
		BindRecv(pClient); // 다시 Recv걸어준다. 
		break;
	default: // 예외
		return false;
	}
	
	return true;

}

void IOCompletionPort::DestroyThread()
{
	for (int i = 0; i < MAX_WORKERTHREAD; ++i)
	{
		//WaitingThread Queue에서 대기중인 (쓰레드에 사용자 종료 메시지)를 보낸다.
		// NULL을 실어보냄. 즉 WorkerThread에서 완료큐에 데이터를 뽑아오는데 overlapped가 null이면
		// 종료신호.
		::PostQueuedCompletionStatus(m_hCompletionPort, 0, 0, NULL);
	}

	for (int i = 0; i < MAX_WORKERTHREAD; ++i)
	{
		//쓰레드 핸들을 닫고 쓰레드가 종료될때까지 기다린다.
		::CloseHandle(m_hWorkerThread[i]);
		::WaitForSingleObject(m_hWorkerThread[i], INFINITE);
	}

	m_bAccepterRun = false;
	m_bWorkerRun = false;
	::closesocket(m_hSocketListen);

	::CloseHandle(m_hAccepterThread);
	::WaitForSingleObject(m_hAccepterThread, INFINITE);
}
