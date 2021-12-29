#pragma once
#include "std.h"
class ServerSocket
{
public:
	ServerSocket();
	~ServerSocket();
public:
	BOOL	ServerInit();
	BOOL	ServerBind();
	BOOL	ServerListen();
	BOOL	ServerAccept();
	SOCKET	GetClientSocket() { return m_hConnClientSock; }
	char*	GetBuffer()		  { return m_socketBuf; }
	void    SetBuffer(int idx, char t) { m_socketBuf[idx] = t; }
private:
	void ErrorHandler(const char* error);
private:
	SOCKET		m_hListenSocket;
	SOCKET		m_hConnClientSock;
	SOCKADDR_IN m_listenSockAddr;
	SOCKADDR_IN m_clientSockAddr;
	char		m_socketBuf[BUF_SIZE];
};

