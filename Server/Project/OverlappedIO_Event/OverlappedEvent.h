#pragma once
#include "std.h"
#define MAX_SOCKBUF 1024

enum class IO_TYPE
{
	IO_RECV,
	IO_SEND
};

// 워커쓰레드 모델을 채택.
// 각각 접속을 처리하는 쓰레드, 일하는 쓰레드(IO처리)

// Overlapped 구조체를 확장하여 사용.
struct OverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;
	int			  m_index;
	WSABUF		  m_wsaBuf;
	char		  m_dataBuffer[MAX_SOCKBUF];
	IO_TYPE		  m_ioType;
};

// 클라이언트 정보를 담기위한 구조체
struct ClientInfo
{
	SOCKET			m_socketClient[WSA_MAXIMUM_WAIT_EVENTS]; // 64
	WSAEVENT		m_eventHandle[WSA_MAXIMUM_WAIT_EVENTS];
	OverlappedEx	m_overlappedEx[WSA_MAXIMUM_WAIT_EVENTS];
};

class OverlappedEvent
{
public:
	OverlappedEvent();
	virtual ~OverlappedEvent();
public:
	//------서버 클라이언트 공통함수-------//
	//소켓을 초기화하는 함수
	bool InitSocket();
	//소켓의 연결을 종료 시킨다.
	void CloseSocket(SOCKET socketClose, bool bIsForce = false);

	//------서버용 함수-------//
	//서버의 주소정보를 소켓과 연결시키고 접속 요청을 받기 위해 그 소켓을 등록하는 함수
	bool BindandListen(int nBindPort);
	//접속 요청을 수락하고 메세지를 받아서 처리하는 함수
	bool StartServer();

	//Overlapped I/O작업을 위한 쓰레드를 생성
	bool CreateWokerThread();
	//accept요청을 처리하는 쓰레드 생성
	bool CreateAccepterThread();

	//사용되지 않은 index반환
	int GetEmptyIndex();

	//WSARecv Overlapped I/O 작업을 시킨다.
	bool BindRecv(int nIdx);

	//WSASend Overlapped I/O작업을 시킨다.
	bool SendMsg(int nIdx, char* pMsg, int nLen);

	//Overlapped I/O작업에 대한 완료 통보를 받아 
	//그에 해당하는 처리를 하는 함수
	void WokerThread();
	//사용자의 접속을 받는 쓰레드
	void AccepterThread();

	//Overlapped I/O 완료에 대한 결과 처리
	void OverlappedResult(int nIdx);

	//생성되어있는 쓰레드를 파괴한다.
	void DestroyThread();

private:
	//클라이언트 정보 저장 구조체
	ClientInfo	m_clientInfo;

	//접속 되어있는 클라이언트 수
	int			m_clientCnt;
	//메인 윈도우 포인터

	//작업 쓰레드 핸들
	HANDLE		m_hWorkerThread;
	//접속 쓰레드 핸들
	HANDLE		m_hAccepterThread;
	//작업 쓰레드 동작 플래그
	bool		m_bWorkerRun;
	//접속 쓰레드 동작 플래그
	bool		m_bAccepterRun;

	//소켓 버퍼
	char		m_socketBuf[MAX_SOCKBUF];
};

