#include "ServerSocket.h"

ServerSocket::ServerSocket() : m_hConnClientSock(INVALID_SOCKET),m_hListenSocket(INVALID_SOCKET), m_listenSockAddr(), m_clientSockAddr()
{
    ::ZeroMemory(&m_socketBuf, sizeof(BUF_SIZE));
}

ServerSocket::~ServerSocket()
{
    ::shutdown(m_hListenSocket, SD_BOTH);
    ::shutdown(m_hConnClientSock, SD_BOTH);

    ::closesocket(m_hConnClientSock);
    ::closesocket(m_hListenSocket);
    ::WSACleanup();
}

BOOL ServerSocket::ServerInit()
{
    WSAData wsaData;

    UINT ret = ::WSAStartup(MAKEWORD(2, 2), &wsaData); // ���� DLL ���� ���̺귯�� ����
    if (ret != 0) // 0�� ��ȯ�Ǿ�� ����� �Ȱ�
    {
        ErrorHandler("WSAStartUp");
        return FALSE;
    }

    m_hListenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_hListenSocket == INVALID_SOCKET)
    {
        ErrorHandler("Socket Create");
        return FALSE;
    }

    std::cout << "���� �ʱ�ȭ ����" << std::endl;

    return TRUE;
}

BOOL ServerSocket::ServerBind()
{
    ::ZeroMemory(&m_listenSockAddr, sizeof(SOCKADDR_IN));
    m_listenSockAddr.sin_family      = AF_INET;
    m_listenSockAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    m_listenSockAddr.sin_port        = ::htons(PORT_NUM);

    UINT ret = ::bind(m_hListenSocket, (SOCKADDR*)&m_listenSockAddr, sizeof(SOCKADDR));
    if (ret != 0)
    {
        ErrorHandler("Bind");
        return FALSE;
    }

    std::cout << "���� ���ε� ����" << std::endl;

    return TRUE;
}

BOOL ServerSocket::ServerListen()
{
    // �� ���ڴ� ��� ���� ť.
    UINT ret = ::listen(m_hListenSocket, 5);
    if (ret != 0)
    {
        ErrorHandler("listen");
        return FALSE;
    }
    std::cout << "���� ���� ��ȯ " << std::endl;
    return TRUE;
}

BOOL ServerSocket::ServerAccept()
{
    int nameLen = sizeof(SOCKADDR_IN);
    m_hConnClientSock = ::accept(m_hListenSocket, (SOCKADDR*)&m_clientSockAddr, &nameLen);
    if (m_hConnClientSock == INVALID_SOCKET)
        return FALSE;

    return TRUE;
}

void ServerSocket::ErrorHandler(const char* error)
{
    std::cout << "���� ���� : " << error << " " << ::WSAGetLastError() << std::endl;
}
