#include "OverlappedEvent.h"

// �񵿱� IOó�� (OVERLAPPED IO). �񵿱� IO�� OVERLAPPED IO�� ���� ������. ��� Ȯ���� �̺�Ʈ,�ݹ����� ä���Ѱ� OVERLAPPED IO
unsigned int WINAPI CallWorkerThread(LPVOID p)
{
    OverlappedEvent* pOverlappedEvent = (OverlappedEvent*)p;
    pOverlappedEvent->WokerThread();
    return 0;
}

// ����ó��. �̰ŵ� ���� ������ �ϳ��� �ξ� ó���Ѵ�.
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
        m_clientInfo.m_eventHandle[i] = WSACreateEvent(); // Manual-reset, Non-signaled���·� ����.
        ::ZeroMemory(&m_clientInfo.m_overlappedEx[i], sizeof(OverlappedEx));
    }
}

OverlappedEvent::~OverlappedEvent()
{
    ::WSACleanup();

    // �������� �ݱ�.
    ::shutdown(m_clientInfo.m_socketClient[0], SD_BOTH);
    ::closesocket(m_clientInfo.m_socketClient[0]);
    ::SetEvent(m_clientInfo.m_eventHandle[0]); // signaled���·� �����.
    m_bWorkerRun = false;
    m_bAccepterRun = false;

    // ������ ������� ���.
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

    // ���������� TCP, Overlapped I/O ������ ����(���� ����)
    m_clientInfo.m_socketClient[0] = ::WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
    if (m_clientInfo.m_socketClient[0] == INVALID_SOCKET)
    {
        return false;
    }

    std::cout << "���� �ʱ�ȭ ����" << std::endl;

    return true;
}

void OverlappedEvent::CloseSocket(SOCKET socketClose, bool bIsForce)
{
    // ���ڷ� �Ѿ�� ���� ����.

    struct linger linger = { 0,0 }; // ���Ͽɼ� SO_DONTLINGER�� ���� (�������ϱ�)

    // bIsForce�� true�̸� SO_LINGER, timeout = 0���� �����Ͽ� ��������.
    if (bIsForce)
        linger.l_onoff = 1;

    // ���� ������ �ۼ����� ��� �ݴ´�.
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

    std::cout << "bind And Listen ����! " << std::endl;
    return true;
}

bool OverlappedEvent::StartServer()
{
    // ���ӵ� Ŭ���̾�Ʈ �ּ� ������ ������ ����ü.
    bool bRet = CreateWokerThread(); // ��Ŀ ������ ����.
    if (bRet == false)
        return false;
    bRet = CreateAccepterThread();
    if (bRet == false)
        return false;

    // ���� ���ſ� �̺�Ʈ ����.
    m_clientInfo.m_eventHandle[0] = ::WSACreateEvent();

    return true;
}

bool OverlappedEvent::CreateWokerThread()
{
    UINT threadId = 0;

    // ���� ���ڷ� �ڱ��ڽ��� �Ѱ��ش�. �׷��⿡ �� Ŭ������ Singleton���� �����Ǵ°� ����.
    //CREATE_SUSPENDED�� ResumeThread�Լ��� ȣ���ϱ������� ������� �������ǰ� �Լ� ���Ծ���.
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

// ������ �ʴ� �ε��� ��ȯ.
int OverlappedEvent::GetEmptyIndex()
{
    // 0��° �迭�� �������Ͽ��� ���Ǵ� �̺�Ʈ ��ü
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

    // �ٷ� ��ȯ�� �ɼ��� �ְ�(recvNumBytes�� ���� ������ ����) �ƴҼ����ִ�(PENDING)
    int ret = ::WSARecv(m_clientInfo.m_socketClient[nIdx], &m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf,
        1, &recvNumBytes, &flag, (LPWSAOVERLAPPED)&m_clientInfo.m_overlappedEx[nIdx], nullptr);
    
    
    
    // �����ε� PENDING���� �ƴ϶�� Ż��. (������ ����ɷ� �Ǵ�)
    if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
    {
        return false;
    }
    // �ٷ� ��ȯ�Ǵ°Ͱ� �ٷ� ��ȯ�� �ȵǸ� PENDNGó���� ���εǾ��������, �������� �׳� �ٷ� false����.
    

    return true;
}

bool OverlappedEvent::SendMsg(int nIdx, char* pMsg, int nLen)
{
    DWORD recvNumBytes = 0;

    // ���ڼ����� ���Ź��� �޽����� �����Ѵ�.
    ::CopyMemory(m_clientInfo.m_overlappedEx[nIdx].m_dataBuffer, pMsg, nLen);

    // Overlapped I/O�� ���� ����
    m_clientInfo.m_overlappedEx[nIdx].m_wsaOverlapped.hEvent = m_clientInfo.m_eventHandle[nIdx];
    m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf.len = nLen;
    m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf.buf = m_clientInfo.m_overlappedEx[nIdx].m_dataBuffer;
    m_clientInfo.m_overlappedEx[nIdx].m_index = nIdx;
    m_clientInfo.m_overlappedEx[nIdx].m_ioType = IO_TYPE::IO_SEND;

    int ret = ::WSASend(m_clientInfo.m_socketClient[nIdx], &m_clientInfo.m_overlappedEx[nIdx].m_wsaBuf,
        1, &recvNumBytes, 0, (LPWSAOVERLAPPED)&m_clientInfo.m_overlappedEx[nIdx], nullptr);

    // �����ε� PENDING���� �ƴ϶�� Ż��. (������ ����ɷ� �Ǵ�)
    if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
    {
        return false;
    }

    return true;
}
//���ϴ� ������. �� Overlapped I/Oó�� 
void OverlappedEvent::WokerThread()
{
    while (m_bWorkerRun)
    {
        // ��û��  Overlapped I/O �۾��� �Ϸ�Ǿ����� �̺�Ʈ�� ��ٸ���.

        DWORD objIdx = ::WSAWaitForMultipleEvents(WSA_MAXIMUM_WAIT_EVENTS,
            m_clientInfo.m_eventHandle, FALSE, INFINITE, FALSE);

        // ���� �߻�
        if (objIdx == WSA_WAIT_FAILED)
            break;

        // �̺�Ʈ�� ����.
        ::WSAResetEvent(m_clientInfo.m_eventHandle[objIdx]); // Non-signaled���·� �����. (Set�� signaled���·� ����)

        // ������ ���Դ�.
        if (objIdx == WSA_WAIT_EVENT_0)
            continue;

        //Overlapped IO�� ���� ���ó��
        OverlappedResult(objIdx);
    }
}

// ������� ������ ó���ϴ� ������ �Լ�
void OverlappedEvent::AccepterThread()
{
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);

    while (m_bAccepterRun)
    {
        // ������ ���� ����ü�� �ε����� ���´�.
        int idx = GetEmptyIndex(); // �� �ε���. �� �迭���� �� �κ�.

        if (idx == -1) // ������ ��Ȳ(64�� ����)
        {
            return;
        }

        // Ŭ���̾�Ʈ ���� ��û�� ���ö����� ���.(������)
        m_clientInfo.m_socketClient[idx] = ::accept(m_clientInfo.m_socketClient[0],
            (SOCKADDR*)&clientAddr, &addrLen);

        if (m_clientInfo.m_socketClient[idx] == INVALID_SOCKET)
            return;

        // ���� ������ ���� �̺�Ʈ �������� ��,(�̺�Ʈ ����Ʈ�� ����� �ᱹ ���� == �̺�Ʈ 1��1���ᱸ��)
        // �׸��� WSARecv�� �̸� �ɾ��. (�ش� ������ �Է¹��ۿ� ���� ������ �о����)
        bool ret = BindRecv(idx); 
        if (ret == false)
            return;

        // Ŭ���̾�Ʈ ���� ����
        ++m_clientCnt;

        // Ŭ���̾�Ʈ�� ���ӵǾ����Ƿ� WorkerThread���� �̺�Ʈ�� �˸���.
        ::WSASetEvent(m_clientInfo.m_eventHandle[0]); // ���������� �̺�Ʈ������, ��Ŀ������� �̺�Ʈ�̱⵵��.

    }
}

void OverlappedEvent::OverlappedResult(int nIdx)
{
    DWORD transfer = 0; // �ۼ��Ź��� ����Ʈ
    DWORD flags = 0;

    // PENDING�϶� üũ�ϴ� �Լ��̱���.
    bool ret = ::WSAGetOverlappedResult(m_clientInfo.m_socketClient[nIdx],
        (LPWSAOVERLAPPED)&m_clientInfo.m_overlappedEx[nIdx], &transfer, FALSE, &flags);
    if (ret && transfer == 0) 
        return;

    // �������� ��û
    if (transfer == 0)
    {
        ::closesocket(m_clientInfo.m_socketClient[nIdx]);
        --m_clientCnt;
        return;
    }

    // OverlappedEx ����.
    OverlappedEx* pOverlappedEx = &m_clientInfo.m_overlappedEx[nIdx];
    switch (pOverlappedEx->m_ioType)
    {
        // WSARECV�� Overlapped I/O�� �Ϸ�� ���
    case IO_TYPE::IO_RECV:
        pOverlappedEx->m_dataBuffer[transfer] = '\0';
        std::cout << "[����] bytes : " << transfer << " msg : " << pOverlappedEx->m_dataBuffer << std::endl;

        //Ŭ���̾�Ʈ���� ����
        SendMsg(nIdx, pOverlappedEx->m_dataBuffer, transfer);
        break;
    case IO_TYPE::IO_SEND:
        pOverlappedEx->m_dataBuffer[transfer] = '\0';
        std::cout << "[�۽�] bytes : " << transfer << " msg : " << pOverlappedEx->m_dataBuffer << std::endl;
        BindRecv(nIdx); // �ٽ� Recv�ɾ��ش�. 
        break;

    default:
        break;
    }
    

}

void OverlappedEvent::DestroyThread()
{
    ::shutdown(m_clientInfo.m_socketClient[0], SD_BOTH);
    ::closesocket(m_clientInfo.m_socketClient[0]); // �������� �ı�
    ::SetEvent(m_clientInfo.m_eventHandle[0]);
    m_bWorkerRun = false;
    m_bAccepterRun = false;

    ::WaitForSingleObject(m_hAccepterThread, INFINITE);
    ::WaitForSingleObject(m_hWorkerThread, INFINITE);
}
