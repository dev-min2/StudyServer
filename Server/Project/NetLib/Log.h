#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "LogDef.h"
#include "Queue.h"
#include "Thread.h"
#include "Singleton.h"
//메시지 구조체
struct LogMsg
{
	enumLogInfoType m_eLogInfoType;
	char			m_outputString[MAX_OUTPUT_LENGTH];
};

// 초기화 구조체 정의..
struct LogConfig
{
	////////////////////////////////////////////////////////////////////////////
	//배열순서(파일[0],디비[1],윈도우[2],디버그창[3],udp[4])
	//각배열에 출력하고싶은 LogInfo레벨을 or연산하여 넣는다.
	//예)파일에 LOG_INFO_NORMAL, 윈도우에 LOG_ALL
	//s_eLogInfoType[ STORAGE_FILE ] = LOG_INFO_NORMAL
	//s_eLogInfoType[ STORAGE_WINDOW ] = LOG_ALL
	int				m_logInfoTypes[MAX_STORAGE_TYPE];
	char			m_logFileName[MAX_FILENAME_LENGTH];
	// 로그 파일의 형식을 지정한다. xml OR TEXT 둘다 가능.
	enumLogFileType m_eLogFileType;
	//TCP/UDP로 로그를 남길 IP,PORT.
	char			m_IP[MAX_IP_LENGTH];
	int				m_udpPort;
	int				m_tcpPort;
	//서버 타입, 로그서버에 등록될 서버타입을 결정
	int				m_serverType;


	//DB로 로그를 남길 DSN정보
	char			m_dsnName[MAX_DSN_NAME];
	char			m_dsnID[MAX_DSN_ID];
	char			m_dsnPW[MAX_DSN_PW];
	//윈도우로 로그를 남길 윈도우 핸들.
	HWND			m_hWnd;
	//Log 처리 시간 기본으로 1초마다 처리
	DWORD			m_processTick;
	//Log파일 사이즈가 m_fileMaxSize보다 크면 새로운 파일을 만든다.
	DWORD			m_fileMaxSize;

	LogConfig()
	{
		::ZeroMemory(this, sizeof(LogConfig));
		m_processTick = DEFAULT_TICK; // 1000 / 1000;
		m_udpPort	  = DEFAULT_UDPPORT;
		m_tcpPort	  = DEFAULT_TCPPORT;
		m_fileMaxSize = 1024 * 50000; // 50MB 기본으로 설정. 최대 100MB
	}
};

class Log : public Thread, public Singleton
{
	DECLARE_SINGLETON(Log);
public:
	Log();
	~Log();

	//인터페이스 함수
	bool		Init(LogConfig& config);
	void		LogOutput(enumLogInfoType eLogInfo, char* outputString);
	void		LogOutputLastErrorToMsgBox(char* outputString);
	// 모든 로그를 끝낸다.
	void		CloseAllLog();
public:
	// 쓰레드 처리 함수
	void		OnProcess() override;
	void		SetHWND(HWND hWnd = NULL) { m_hWnd = hWnd; }
public:
	// 큐에 관련된 함수 현재 큐 크기.
	UINT		GetQueueSize() { return m_queueLogMsg.GetQueueSize(); }
	void		InsertMsgToQueue(LogMsg* pLogMsg) { m_queueLogMsg.PushQueue(pLogMsg); }
private:
	int				m_logInfoTypes[MAX_STORAGE_TYPE];
	char			m_logFileName[MAX_FILENAME_LENGTH];
	// 로그 파일의 형식을 지정한다. xml OR TEXT 둘다 가능.
	enumLogFileType m_eLogFileType;
	//TCP/UDP로 로그를 남길 IP,PORT.
	char			m_IP[MAX_IP_LENGTH];
	int				m_udpPort;
	int				m_tcpPort;
	//서버 타입, 로그서버에 등록될 서버타입을 결정
	int				m_serverType;


	//DB로 로그를 남길 DSN정보
	char			m_dsnName[MAX_DSN_NAME];
	char			m_dsnID[MAX_DSN_ID];
	char			m_dsnPW[MAX_DSN_PW];
	// 로그 저장 변수
	char			m_outStr[MAX_OUTPUT_LENGTH];

	// 윈도우로 로그를 남기기 위한 윈도우 핸들.
	HWND			m_hWnd; 
	//File Handle변수
	HANDLE			m_logFile;
	//TCP/UDP소켓
	SOCKET			m_sockTcp;
	SOCKET			m_sockUdp;
	//메시지 큐
	Queue<LogMsg*>	m_queueLogMsg;
	//현재 메세지 버퍼 위치
	int				m_msgBufferIdx;
	DWORD			m_fileMaxSize;

	/////////////////////////////////////////////////////////////////////////////
	//내부 호출 함수
	//출력관련..함수	
	void OutputFile(char* outputString);
	void OutputDB(char* outputString);
	void OutputWindow(enumLogInfoType eLogInfo, char* outputString);
	void OutputDebugWnd(char* outputString);
	void OutputUDP(enumLogInfoType eLogInfo, char* outputString);
	void OutputTCP(enumLogInfoType eLogInfo, char* outputString);


	//초기화 함수들
	bool InitDB();
	bool InitFile();
	bool InitUDP();
	bool InitTCP();
};


//글로벌 변수
static char		g_outStr[MAX_OUTPUT_LENGTH];
static LogMsg	g_logMsg[MAX_QUEUE_CNT];
static Monitor	g_log;


//실제 사용자가 쓰는 함수
//초기화 함수
bool INIT_LOG(LogConfig& config);
//로그를 남기는 함수
//로그 메시지 쓰는 방법 : (에러나 정보가 발생한 함수 | 에러나 정보에 관한 내용)
//ex) IocpServer::BindIocp() | bind() Failed..
void EnQueueLog(enumLogInfoType eLogInfoType, const char* outputString, ...);
// 로그를 끝낸다.
void CLOSE_LOG();


CREATE_FUNCTION(Log, GetLog);