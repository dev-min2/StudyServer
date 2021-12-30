#pragma once
#include "std.h"
#define MAX_SOCKBUF 1024 // 패킷(현재는 버퍼)크기
#define MAX_CLIENT 100 // 최대 접속가능한 클라이언트 수
#define MAX_WORKERTHREAD 4 // 쓰레드 풀(CP객체)에 넣을 쓰레드 수

enum class IO_TYPE
{
	IO_RECV,
	IO_SEND
};

// Overlapped 구조체를 확장하여 사용.
struct OverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;
	int			  m_index;
	WSABUF		  m_wsaBuf;
	char		  m_dataBuffer[MAX_SOCKBUF];
	IO_TYPE		  m_ioType;
};

// 클라이언트(Session) 정보를 담기위한 구조체
struct ClientInfo
{
	ClientInfo() : m_socketClient(INVALID_SOCKET)
	{
		::ZeroMemory(&m_recvOverlapped, sizeof(OverlappedEx));
		::ZeroMemory(&m_sendOverlapped, sizeof(OverlappedEx));
	}

	SOCKET			m_socketClient; 
	OverlappedEx	m_recvOverlapped; // Recv Overlapped(비동기) I/O 작업을 위한 변수
	OverlappedEx	m_sendOverlapped; // Send Overlapped(비동기) I/O 작업을 위한 변수
};

class IOCompletionPort
{
	friend unsigned int WINAPI CallWorkerThread(LPVOID p);
	friend unsigned int WINAPI CallAccepterThread(LPVOID p);
public:
	IOCompletionPort();
	virtual ~IOCompletionPort();

public:
	//소켓을 초기화하는 함수
	bool InitSocket();
	//소켓의 연결을 종료 시킨다.
	void CloseSocket(ClientInfo* pClientInfo, bool bIsForce = false);

	//서버의 주소정보를 소켓과 연결시키고 접속 요청을 받기 위해 그 소켓을 등록하는 함수
	bool BindandListen(int nBindPort);
	//접속 요청을 수락하고 메세지를 받아서 처리하는 함수
	bool StartServer();

	//생성되어있는 쓰레드를 파괴한다.
	void DestroyThread();
private:
	//Overlapped I/O작업을 위한 쓰레드를 생성
	bool CreateWokerThread();
	//accept요청을 처리하는 쓰레드 생성
	bool CreateAccepterThread();

	//사용되지 않은 ClientInfo를 반환
	ClientInfo* GetEmptyClientInfo();

	// CP객체와 Completion Key(완료키)를 등록하는 함수.
	bool BindIOCompletionPort(ClientInfo* pClientInfo);

	//WSARecv Overlapped I/O 작업을 시킨다.
	bool BindRecv(ClientInfo* pClientInfo);

	//WSASend Overlapped I/O작업을 시킨다.
	bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen);

	bool IOProcess(ClientInfo* pClient,OverlappedEx* pOverlapped, DWORD& transferred);

	//Overlapped I/O작업에 대한 완료 통보를 받아 
	//그에 해당하는 처리를 하는 함수
	void WokerThread();
	//사용자의 접속을 받는 쓰레드
	void AccepterThread();



private:
	//클라이언트 정보 구조체
	ClientInfo* m_pClientInfo;
	//리슨소켓
	SOCKET		m_hSocketListen;
	// 연결된 클라이언트 수
	int			m_clientCount;
	// 워커쓰레드 핸들 배열
	HANDLE		m_hWorkerThread[MAX_WORKERTHREAD];
	// 접속 쓰레드 핸들
	HANDLE		m_hAccepterThread;
	// CP객체
	HANDLE		m_hCompletionPort;
	// 작업 쓰레드 BOOL
	bool		m_bWorkerRun;
	// 접속 쓰레드 BOOL
	bool		m_bAccepterRun;
	// 소켓 버퍼
	char		m_socketBuff[MAX_SOCKBUF];

};

