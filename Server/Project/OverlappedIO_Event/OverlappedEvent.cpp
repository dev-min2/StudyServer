#include "OverlappedEvent.h"

// 비동기 IO처리 (OVERLAPPED IO). 비동기 IO와 OVERLAPPED IO는 거의 같은말. 결과 확인을 이벤트,콜백방식을 채택한게 OVERLAPPED IO
unsigned int WINAPI CallWorkerThread(LPVOID p)
{
    OverlappedEvent* pOverlappedEvent = (OverlappedEvent*)p;
    pOverlappedEvent->WokerThread();
    return 0;
}

// 연결처리. 이거도 보통 쓰레드 하나를 두어 처리한다.
unsigned int WINAPI CallAccepterThread(LPVOID p)
{
    OverlappedEvent* pOverlappedEvent = (OverlappedEvent*)p;
    pOverlappedEvent->AccepterThread();
    return 0;
}









OverlappedEvent::OverlappedEvent()
    : m_bWorkerRun(true), m_bAccepterRun(true), m_clientCnt(0), m_hWorkerThread(nullptr), m_hAccepterThread(nullptr)
{
    ::ZeroMemory(m_socketBuf,MAX_SOCKBUF);

    int i;
    for (i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
    {
        m_clientInfo.m_socketClient[i] = INVALID_SOCKET;
        m_clientInfo.m_eventHandle[i] = WSACreateEvent(); // Manual-reset, Non-signaled상태로 생성.
        ::ZeroMemory(&m_clientInfo.m_overlappedEx[i], sizeof(OverlappedEx));
    }
}

OverlappedEvent::~OverlappedEvent()
{
    ::WSACleanup();

    // 리슨소켓 닫기.
    ::shutdown(m_clientInfo.m_socketClient[0], SD_BOTH);
    ::closesocket(m_clientInfo.m_socketClient[0]);
    ::SetEvent(m_clientInfo.m_eventHandle[0]); // signaled상태로 만든다.
    m_bWorkerRun = false;
    m_bAccepterRun = false;

    // 쓰레드 종료까지 대기.
    ::WaitForSingleObject(m_hWorkerThread, INFINITE);
    ::WaitForSingleObject(m_hAccepterThread, INFINITE);
}

bool OverlappedEvent::InitSocket()
{
    WSADATA wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return false;
    }

    // 연결지향형 TCP, Overlapped I/O 소켓을 생성(리슨 소켓)
    m_clientInfo.m_socketClient[0] = ::WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
    if (m_clientInfo.m_socketClient[0] == INVALID_SOCKET)
    {
        return false;
    }

    std::cout << "소켓 초기화 성공" << std::endl;

    return true;
}

void OverlappedEvent::CloseSocket(SOCKET socketClose, bool bIsForce)
{
    // 인자로 넘어온 소켓 종료.

    struct linger linger = { 0,0 }; // 소켓옵션 SO_DONTLINGER로 설정 (지연안하기)

    // bIsForce가 true이면 SO_LINGER, timeout = 0으로 설정하여 강제종료.
    if (bIsForce)
        linger.l_onoff = 1;

    // 닫을 소켓의 송수신을 모두 닫는다.
    ::shutdown(socketClose, SD_BOTH);
    ::setsockopt(socketClose, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
    ::closesocket(socketClose);

    socketClose = INVALID_SOCKET;
}

bool OverlappedEvent::BindandListen(int nBindPort)
{
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(PORT_NUM);

    if (::bind(m_clientInfo.m_socketClient[0], (SOCKADDR*)&serverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
        return false;

    if (::listen(m_clientInfo.m_socketClient[0], 5) == SOCKET_ERROR)
        return false;

    std::cout << "bind And Listen 성공! " << std::endl;
    return true;
}

bool OverlappedEvent::StartServer()
{
    // 접속된 클라이언트 주소 정보를 저장할 구조체.
    bool bRet = CreateWokerThread(); // 워커 쓰레드 생성.
    if (bRet == false)
        return false;
    bRet = CreateAccepterThread();
    if (bRet == false)
        return false;

    // 정보 갱신용 이벤트 생성.
    m_clientInfo.m_eventHandle[0] = ::WSACreateEvent();

    return true;
}

bool OverlappedEvent::CreateWokerThread()
{
    UINT threadId = 0;

    // 보통 인자로 자기자신을 넘겨준다. 그렇기에 이 클래스는 Singleton으로 관리되는게 좋음.
    //CREATE_SUSPENDED는 ResumeThread함수를 호출하기전까지 쓰레드는 생성만되고 함수 진입안함.
    m_hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, CallWorkerThread, this, CREATE_SUSPENDED, &threadId);
    if (m_hWorkerThread == NULL)
        return false;


    ::ResumeThread(m_hWorkerThread);
    return true;
}

bool OverlappedEvent::CreateAccepterThread()
{
    UINT threadId = 0;

    m_hAccepterThread = (HANDLE)_beginthreadex(NULL, 0, CallAccepterThread, this, CREATE_SUSPENDED, &threadId);
    if (m_hAccepterThread == NULL)
        return false;

    ::ResumeThread(m_hAccepterThread);
    return true;
}

// 사용되지 않는 인덱스 반환.
int OverlappedEvent::GetEmptyIndex()
{
    // 0번째 배열은 리슨소켓에서 사용되는 이벤트 객체
    for (int i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
    {
        if (m_clientInfo.m_socketClient[i] == INVALID_SOCKET)
            return i;
    }
    return -1;
}

bool OverlappedEvent::BindRecv(int nIdx)
{
    DWORD flag = 0;
    DWORD recvNumBytes = 0;

    m_clientInfo.m_eventHandle[nIdx] = ::WSACreateEvent();

    //Overlapped I/O 
    m_clientInfo.m_overlappedEx[nIdx].m_wsaOverlapped.hEvent = m_clientInfo.m_eventHandle[nIdx];
    m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf.len = MAX_SOCKBUF;
    m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf.buf = m_clientInfo.m_overlappedEx[nIdx].m_dataBuffer;
    m_clientInfo.m_overlappedEx[nIdx].m_index = nIdx;
    m_clientInfo.m_overlappedEx[nIdx].m_ioType = IO_TYPE::IO_RECV;

    // 바로 반환이 될수도 있고(recvNumBytes에 수신 데이터 담긴다) 아닐수도있다(PENDING)
    int ret = ::WSARecv(m_clientInfo.m_socketClient[nIdx], &m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf,
        1, &recvNumBytes, &flag, (LPWSAOVERLAPPED)&m_clientInfo.m_overlappedEx[nIdx], nullptr);
    
    
    
    // 에러인데 PENDING까지 아니라면 탈출. (연결이 끊긴걸로 판단)
    if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
    {
        return false;
    }
    // 바로 반환되는것과 바로 반환이 안되면 PENDNG처리가 따로되어야하지만, 위에서는 그냥 바로 false때림.
    

    return true;
}

bool OverlappedEvent::SendMsg(int nIdx, char* pMsg, int nLen)
{
    DWORD recvNumBytes = 0;

    // 에코서버니 수신받은 메시지를 복사한다.
    ::CopyMemory(m_clientInfo.m_overlappedEx[nIdx].m_dataBuffer, pMsg, nLen);

    // Overlapped I/O를 위한 셋팅
    m_clientInfo.m_overlappedEx[nIdx].m_wsaOverlapped.hEvent = m_clientInfo.m_eventHandle[nIdx];
    m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf.len = nLen;
    m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf.buf = m_clientInfo.m_overlappedEx[nIdx].m_dataBuffer;
    m_clientInfo.m_overlappedEx[nIdx].m_index = nIdx;
    m_clientInfo.m_overlappedEx[nIdx].m_ioType = IO_TYPE::IO_SEND;

    int ret = ::WSASend(m_clientInfo.m_socketClient[nIdx], &m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf,
        1, &recvNumBytes, 0, (LPWSAOVERLAPPED)&m_clientInfo.m_overlappedEx[nIdx], nullptr);

    // 에러인데 PENDING까지 아니라면 탈출. (연결이 끊긴걸로 판단)
    if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
    {
        return false;
    }

    return true;
}
//일하는 쓰레드. 즉 Overlapped I/O처리 
void OverlappedEvent::WokerThread()
{
    while (m_bWorkerRun)
    {
        // 요청한  Overlapped I/O 작업이 완료되었는지 이벤트를 기다린다.

        DWORD objIdx = ::WSAWaitForMultipleEvents(WSA_MAXIMUM_WAIT_EVENTS,
            m_clientInfo.m_eventHandle, FALSE, INFINITE, FALSE);

        // 에러 발생
        if (objIdx == WSA_WAIT_FAILED)
            break;

        // 이벤트를 리셋.
        ::WSAResetEvent(m_clientInfo.m_eventHandle[objIdx]); // Non-signaled상태로 만든다. (Set은 signaled상태로 만듬)

        // 접속이 들어왔다.
        if (objIdx == WSA_WAIT_EVENT_0)
            continue;

        //Overlapped IO에 대한 결과처리
        OverlappedResult(objIdx);
    }
}

// 사용자의 접속을 처리하는 쓰레드 함수
void OverlappedEvent::AccepterThread()
{
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);

    while (m_bAccepterRun)
    {
        // 접속을 받을 구조체의 인덱스를 얻어온다.
        int idx = GetEmptyIndex(); // 빈 인덱스. 즉 배열에서 빈 부분.

        if (idx == -1) // 가득찬 상황(64개 오버)
        {
            return;
        }

        // 클라이언트 접속 요청이 들어올때까지 대기.(동기방식)
        m_clientInfo.m_socketClient[idx] = ::accept(m_clientInfo.m_socketClient[0],
            (SOCKADDR*)&clientAddr, &addrLen);

        if (m_clientInfo.m_socketClient[idx] == INVALID_SOCKET)
            return;

        // 새로 생성된 소켓 이벤트 생성해준 후,(이벤트 셀렉트랑 비슷함 결국 소켓 == 이벤트 1대1연결구조)
        // 그리고 WSARecv를 미리 걸어둠. (해당 소켓의 입력버퍼에 내용 있으면 읽어들임)
        bool ret = BindRecv(idx); 
        if (ret == false)
            return;

        // 클라이언트 개수 증가
        ++m_clientCnt;

        // 클라이어트가 접속되었으므로 WorkerThread에게 이벤트로 알린다.
        ::WSASetEvent(m_clientInfo.m_eventHandle[0]); // 리슨소켓의 이벤트이지만, 워커쓰레드용 이벤트이기도함.

    }
}

void OverlappedEvent::OverlappedResult(int nIdx)
{
    DWORD transfer = 0; // 송수신받은 바이트
    DWORD flags = 0;

    // PENDING일때 체크하는 함수이긴함.
    bool ret = ::WSAGetOverlappedResult(m_clientInfo.m_socketClient[nIdx],
        (LPWSAOVERLAPPED)&m_clientInfo.m_overlappedEx[nIdx], &transfer, FALSE, &flags);
    if (ret && transfer == 0) 
        return;

    // 연결해제 요청
    if (transfer == 0)
    {
        ::closesocket(m_clientInfo.m_socketClient[nIdx]);
        --m_clientCnt;
        return;
    }

    // OverlappedEx 추출.
    OverlappedEx* pOverlappedEx = &m_clientInfo.m_overlappedEx[nIdx];
    switch (pOverlappedEx->m_ioType)
    {
        // WSARECV로 Overlapped I/O가 완료된 경우
    case IO_TYPE::IO_RECV:
        pOverlappedEx->m_dataBuffer[transfer] = '\0';
        std::cout << "[수신] bytes : " << transfer << " msg : " << pOverlappedEx->m_dataBuffer << std::endl;

        //클라이언트에게 에코
        SendMsg(nIdx, pOverlappedEx->m_dataBuffer, transfer);
        break;
    case IO_TYPE::IO_SEND:
        pOverlappedEx->m_dataBuffer[transfer] = '\0';
        std::cout << "[송신] bytes : " << transfer << " msg : " << pOverlappedEx->m_dataBuffer << std::endl;
        BindRecv(nIdx); // 다시 Recv걸어준다. 
        break;

    default:
        break;
    }
    

}

void OverlappedEvent::DestroyThread()
{
    ::shutdown(m_clientInfo.m_socketClient[0], SD_BOTH);
    ::closesocket(m_clientInfo.m_socketClient[0]); // 리슨소켓 파괴
    ::SetEvent(m_clientInfo.m_eventHandle[0]);
    m_bWorkerRun = false;
    m_bAccepterRun = false;

    ::WaitForSingleObject(m_hAccepterThread, INFINITE);
    ::WaitForSingleObject(m_hWorkerThread, INFINITE);
}
