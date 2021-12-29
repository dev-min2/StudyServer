// EchoClient.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "std.h"
#include <ws2tcpip.h>

void ErrorHandler(const char* error)
{
    std::cout << "에러 이유 : " << error << " " << ::WSAGetLastError() << std::endl;
}
int main()
{
    
    Sleep(1000);
    std::cout << "===========Client==========" << std::endl;
    WSAData wsaData;
    int ret = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    char buffer[BUF_SIZE];
    if (ret != 0)
    {
        return 0;
    }

    SOCKET socketClient = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socketClient == INVALID_SOCKET)
    {
        ErrorHandler("소켓 생성에러");
        return 0;
    }

    SOCKADDR_IN serverAddr;
    ::ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    //::inet_pton(AF_INET, SERVER_IP, &serverAddr);
    serverAddr.sin_addr.s_addr = ::inet_addr(SERVER_IP);
    serverAddr.sin_port = ::htons(PORT_NUM);
    int ret2 = ::connect(socketClient, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (ret2 == SOCKET_ERROR)
    {
        ErrorHandler("소켓 연결 에러");
        return 0;
    }

    std::cout << "서버 연결 성공!" << std::endl;

    while (true)
    {
        std::cin >> buffer;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(BUF_SIZE, '\n');
            continue;
        }
        if (::strcmp(buffer, "quit") == 0)
        {
            std::cout << "종료 입력" << std::endl;
            break;
        }
        int sendLen = ::send(socketClient, buffer, strlen(buffer), 0);

        if (sendLen < 0)
        {
            ErrorHandler("Send 에러");
            return 0;
        }
        std::cout << "[Client] 보낸 크기 : " << sendLen << std::endl;
        std::cout << "[Client] 보낸 메시지 : " << buffer << std::endl;

        ::ZeroMemory(buffer, sizeof(BUF_SIZE));
        int recvLen = ::recv(socketClient, buffer, sendLen, 0);
        if (recvLen == 0)
        {
            std::cout << "[Server] 연결 종료" << std::endl;
            break;
        }
        else if (recvLen < 0)
        {
            ErrorHandler("전송 에러");
            return 0;
        }

        std::cout << "[Server] 받은 크기 : " << recvLen << std::endl;
        std::cout << "[Server] 받은 메시지 : " << buffer << std::endl;

        ::ZeroMemory(buffer, sizeof(BUF_SIZE));


    }



    ::shutdown(socketClient, SD_BOTH);
    ::closesocket(socketClient);
    ::WSACleanup();


    return 0;

}
