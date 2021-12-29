// EchoServer1227.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "ServerSocket.h"

int main()
{
	std::cout << "===========Server==========" << std::endl;
	ServerSocket s;
	s.ServerInit();
	s.ServerBind();
	s.ServerListen();
	while (true)
	{
		bool ret = s.ServerAccept();
		if (ret)
			break;
	}
	std::cout << "Client 연결 성공" << std::endl;

	while (true) // 서버에서 읽고 Echo.
	{
		int recvLen = ::recv(s.GetClientSocket(), s.GetBuffer(), BUF_SIZE,0);
		if (recvLen == 0) // 연결종료.
			break;
		else if (recvLen == -1)
		{
			std::cout << "에러" << std::endl;
			break;
		}
		
		s.SetBuffer(recvLen, '\0');

		std::cout << "[Client]client로 부터 받은 데어터 크기 : "  << recvLen << std::endl;
		std::cout << "[Client]client로 부터 받은 메세지 : "      <<  s.GetBuffer() << std::endl;

		// 다시 send.

		int sendLen = ::send(s.GetClientSocket(), s.GetBuffer(), recvLen, 0);
		if(sendLen == -1)
		{
			std::cout << "에러" << std::endl;
			break;
		}

		std::cout << "[Server]보낸 크기 : " << sendLen << std::endl;
		std::cout << "[Server]보낸 메세지 : " << s.GetBuffer() << std::endl;

		::ZeroMemory(s.GetBuffer(), sizeof(BUF_SIZE));
	}
	std::cout << "연결 종료.." << std::endl;
	return 0;
}
