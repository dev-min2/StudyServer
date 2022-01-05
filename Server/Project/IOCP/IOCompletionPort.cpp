#include "IOCompletionPort.h"


// WSARecv�� WSASend�� Overlapped I/O �۾� ó���� ���� �������Լ�
unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	IOCompletionPort* pEvent = (IOCompletionPort*)p;
	pEvent->WokerThread();
	return 0;
}

// ����ó�� ������ �Լ�.
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
	// Ŭ���̾�Ʈ ������ ���� ClientInfo �迭 ����(��) ����.
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
		std::cout << "���� �ʱ�ȭ ����" << std::endl;
		return false;
	}

	m_hSocketListen = ::socket(AF_INET, SOCK_STREAM, 0);

	if (m_hSocketListen == INVALID_SOCKET)
	{
		std::cout << "ListenSocket ���� ����" << std::endl;
		return false;
	}

	u_long argp = 1;
	::ioctlsocket(m_hSocketListen, FIONBIO, &argp);

	
	std::cout << "���� �ʱ�ȭ ����!" << std::endl;
	return true;
}

void IOCompletionPort::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	struct linger linger = { 0,0 }; // SO_DONTLINGER�� ����

	//bIsForce�� true�̸� SO_LINGER, timeout = 0���� �����Ͽ� ���� ���� (�ۼ��Ź��۳��� ����)
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

	std::cout << "bind And listen ����!" << std::endl;

	return true;
}

bool IOCompletionPort::StartServer()
{
	// �� CP��ü ����
	m_hCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (m_hCompletionPort == INVALID_HANDLE_VALUE)
	{
		std::cout << "CP��ü ���� ����" << std::endl;
		return false;
	}

	bool bRet = CreateWokerThread();
	if (bRet == false)
		return false;
	bRet = CreateAccepterThread();
	if (bRet == false)
		return false;

	std::cout << "���� ����..." << std::endl;

	return true;
}

bool IOCompletionPort::CreateWokerThread()
{
	UINT threadId = 0;

	//WaitingThread Queue�� ��� ���·� ���� ������� ����(������ Ǯ)
	// ����Ǵ� ���� (cpu ���� * 2 ) + 1
	for (int i = 0; i < MAX_WORKERTHREAD; ++i)
	{
		m_hWorkerThread[i] = (HANDLE)::_beginthreadex(NULL, 0, CallWorkerThread, this, CREATE_SUSPENDED, &threadId);
		if (m_hWorkerThread[i] == INVALID_HANDLE_VALUE)
		{
			std::cout << "������ ���� ����" << std::endl;
			return false;
		}
		::ResumeThread(m_hWorkerThread[i]);
	}

	std::cout << "WorkerThread ����.." << std::endl;

	return true;
}

bool IOCompletionPort::CreateAccepterThread()
{
	UINT threadId = 0;

	m_hAccepterThread = (HANDLE)::_beginthreadex(NULL, 0, CallAccepterThread, this, CREATE_SUSPENDED, &threadId);

	if (m_hAccepterThread == INVALID_HANDLE_VALUE)
	{
		std::cout << "������ ���� ����" << std::endl;
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
	// ���ϰ� �Ϸ�Ű CP��ü�� ���.
	// ���⼭ �Ϸ�Ű�� GQCS�Լ����� �ٽ� �޾ƺ� �� ����.
	// �Ϸ�Ű�� ClientInfo�� �״�� �ǳ��ش�.
	// �� �Լ��� ��ȯ���� �츮�� ������ CP��ü �ڵ��� ��ȯ.
	HANDLE compareCP;
	compareCP = ::CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient,
		m_hCompletionPort, reinterpret_cast<ULONG_PTR>(pClientInfo),
		0);
	if (compareCP == NULL || compareCP != m_hCompletionPort)
		return false;

	bool check = BindRecv(pClientInfo); // �� �ѹ� Recv��û�� �ɾ�����Ѵ�.
	while(!check)
		check =BindRecv(pClientInfo);

	return true;
}

bool IOCompletionPort::BindRecv(ClientInfo* pClientInfo)
{
	DWORD flag = 0;
	DWORD recvNumBytes = 0;

	// Overlapped I/O�� ���� ���� ����
	pClientInfo->m_recvOverlapped.m_wsaBuf.len = MAX_SOCKBUF;
	pClientInfo->m_recvOverlapped.m_wsaBuf.buf = pClientInfo->m_recvOverlapped.m_dataBuffer;
	pClientInfo->m_recvOverlapped.m_ioType = IO_TYPE::IO_RECV;

	// WSARecv �۾� ��û (�񵿱� I/O = Overlapped I/O)
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

	// ���ڼ����� ���Ź��� �޽��� ����.
	::CopyMemory(pClientInfo->m_sendOverlapped.m_dataBuffer, pMsg, nLen);

	// Overlapped I/O�� ���� ���� ����
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


// Ŭ���̾�Ʈ ���� ó���ϴ� �Լ�(AccepterThread�� ó��)
void IOCompletionPort::AccepterThread()
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	while (m_bAccepterRun)
	{
		// ���� ���� ����ü�� �ε����� ���´�.
		ClientInfo* pClient = GetEmptyClientInfo();
		if (pClient == nullptr)
			return;
		// Ŭ���̾�Ʈ ���� ��û�� �ö����� ���
		pClient->m_socketClient = ::accept(m_hSocketListen, (SOCKADDR*)&clientAddr, &addrLen);
		if (pClient->m_socketClient == INVALID_SOCKET)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			break;
		}

		// Ŭ���̾�Ʈ�� ����� ������ CP��ü�� ����Ѵ�.
		bool ret = BindIOCompletionPort(pClient);
		if (!ret)
			return;

		++m_clientCount;
	}
}


void IOCompletionPort::WokerThread()
{
	//CompletionKey(�Ϸ� Ű)�� ���� ������ ����
	ClientInfo* cpKey = nullptr;
	// �Լ� ȣ�� ��������
	bool bSuccess = true;
	//Overlapped I/O�۾����� �ۼ��ŵ� ������ ũ��
	DWORD transferred = 0;
	// Overlapped I/O�۾����� �Ѱ��� Overlapped����ü
	LPOVERLAPPED pOverlapped = nullptr;
	OverlappedEx* originOverlapped = nullptr;
	while (m_bWorkerRun)
	{
		// GQCS�Լ��� ���� ��������� WatingThread �� �� �����°� �ȴ�. (������ Ǯ)
		// �Ϸ�� Overlapped I/O�۾��� ����� IOCP queue(�Ϸ�ť)�� ���� ������� �� �ϳ��� ���
		// �� �۾��� ������ ��ó���� �ñ��.
		
		bSuccess = ::GetQueuedCompletionStatus(m_hCompletionPort, &transferred, (ULONG_PTR*)&cpKey,
			&pOverlapped, INFINITE);

		// ����ڰ� ������ ������ ��.
		if (transferred == 0 && bSuccess == FALSE)
		{
			::closesocket(cpKey->m_socketClient);
			continue;
		}

		// ����� ������ ���� �޽��� ó��
		if (bSuccess == TRUE && transferred == 0 && pOverlapped == nullptr)
		{
			m_bWorkerRun = false;
			continue;
		}
		if (pOverlapped == nullptr)
			continue;

		//ĳ����
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
	// RECVó��.(���ڴ� �ٽ� ����)
	switch (pOverlapped->m_ioType)
	{
	case IO_TYPE::IO_RECV:
		pOverlapped->m_dataBuffer[transferred] = '\0';
		std::cout << "[����] bytes " << transferred << " msg : " << pOverlapped->m_dataBuffer << std::endl;
		SendMsg(pClient, pOverlapped->m_dataBuffer, transferred);
		break;

	case IO_TYPE::IO_SEND:
		pOverlapped->m_dataBuffer[transferred] = '\0';
		std::cout << "[�۽�] bytes : " << transferred << " msg : " << pOverlapped->m_dataBuffer << std::endl;
		BindRecv(pClient); // �ٽ� Recv�ɾ��ش�. 
		break;
	default: // ����
		return false;
	}
	
	return true;

}

void IOCompletionPort::DestroyThread()
{
	for (int i = 0; i < MAX_WORKERTHREAD; ++i)
	{
		//WaitingThread Queue���� ������� (�����忡 ����� ���� �޽���)�� ������.
		// NULL�� �Ǿ��. �� WorkerThread���� �Ϸ�ť�� �����͸� �̾ƿ��µ� overlapped�� null�̸�
		// �����ȣ.
		::PostQueuedCompletionStatus(m_hCompletionPort, 0, 0, NULL);
	}

	for (int i = 0; i < MAX_WORKERTHREAD; ++i)
	{
		//������ �ڵ��� �ݰ� �����尡 ����ɶ����� ��ٸ���.
		::CloseHandle(m_hWorkerThread[i]);
		::WaitForSingleObject(m_hWorkerThread[i], INFINITE);
	}

	m_bAccepterRun = false;
	m_bWorkerRun = false;
	::closesocket(m_hSocketListen);

	::CloseHandle(m_hAccepterThread);
	::WaitForSingleObject(m_hAccepterThread, INFINITE);
}
