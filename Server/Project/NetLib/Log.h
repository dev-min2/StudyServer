#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "LogDef.h"
#include "Queue.h"
#include "Thread.h"
#include "Singleton.h"
//�޽��� ����ü
struct LogMsg
{
	enumLogInfoType m_eLogInfoType;
	char			m_outputString[MAX_OUTPUT_LENGTH];
};

// �ʱ�ȭ ����ü ����..
struct LogConfig
{
	////////////////////////////////////////////////////////////////////////////
	//�迭����(����[0],���[1],������[2],�����â[3],udp[4])
	//���迭�� ����ϰ���� LogInfo������ or�����Ͽ� �ִ´�.
	//��)���Ͽ� LOG_INFO_NORMAL, �����쿡 LOG_ALL
	//s_eLogInfoType[ STORAGE_FILE ] = LOG_INFO_NORMAL
	//s_eLogInfoType[ STORAGE_WINDOW ] = LOG_ALL
	int				m_logInfoTypes[MAX_STORAGE_TYPE];
	char			m_logFileName[MAX_FILENAME_LENGTH];
	// �α� ������ ������ �����Ѵ�. xml OR TEXT �Ѵ� ����.
	enumLogFileType m_eLogFileType;
	//TCP/UDP�� �α׸� ���� IP,PORT.
	char			m_IP[MAX_IP_LENGTH];
	int				m_udpPort;
	int				m_tcpPort;
	//���� Ÿ��, �α׼����� ��ϵ� ����Ÿ���� ����
	int				m_serverType;


	//DB�� �α׸� ���� DSN����
	char			m_dsnName[MAX_DSN_NAME];
	char			m_dsnID[MAX_DSN_ID];
	char			m_dsnPW[MAX_DSN_PW];
	//������� �α׸� ���� ������ �ڵ�.
	HWND			m_hWnd;
	//Log ó�� �ð� �⺻���� 1�ʸ��� ó��
	DWORD			m_processTick;
	//Log���� ����� m_fileMaxSize���� ũ�� ���ο� ������ �����.
	DWORD			m_fileMaxSize;

	LogConfig()
	{
		::ZeroMemory(this, sizeof(LogConfig));
		m_processTick = DEFAULT_TICK; // 1000 / 1000;
		m_udpPort	  = DEFAULT_UDPPORT;
		m_tcpPort	  = DEFAULT_TCPPORT;
		m_fileMaxSize = 1024 * 50000; // 50MB �⺻���� ����. �ִ� 100MB
	}
};

class Log : public Thread, public Singleton
{
	DECLARE_SINGLETON(Log);
public:
	Log();
	~Log();

	//�������̽� �Լ�
	bool		Init(LogConfig& config);
	void		LogOutput(enumLogInfoType eLogInfo, char* outputString);
	void		LogOutputLastErrorToMsgBox(char* outputString);
	// ��� �α׸� ������.
	void		CloseAllLog();
public:
	// ������ ó�� �Լ�
	void		OnProcess() override;
	void		SetHWND(HWND hWnd = NULL) { m_hWnd = hWnd; }
public:
	// ť�� ���õ� �Լ� ���� ť ũ��.
	UINT		GetQueueSize() { return m_queueLogMsg.GetQueueSize(); }
	void		InsertMsgToQueue(LogMsg* pLogMsg) { m_queueLogMsg.PushQueue(pLogMsg); }
private:
	int				m_logInfoTypes[MAX_STORAGE_TYPE];
	char			m_logFileName[MAX_FILENAME_LENGTH];
	// �α� ������ ������ �����Ѵ�. xml OR TEXT �Ѵ� ����.
	enumLogFileType m_eLogFileType;
	//TCP/UDP�� �α׸� ���� IP,PORT.
	char			m_IP[MAX_IP_LENGTH];
	int				m_udpPort;
	int				m_tcpPort;
	//���� Ÿ��, �α׼����� ��ϵ� ����Ÿ���� ����
	int				m_serverType;


	//DB�� �α׸� ���� DSN����
	char			m_dsnName[MAX_DSN_NAME];
	char			m_dsnID[MAX_DSN_ID];
	char			m_dsnPW[MAX_DSN_PW];
	// �α� ���� ����
	char			m_outStr[MAX_OUTPUT_LENGTH];

	// ������� �α׸� ����� ���� ������ �ڵ�.
	HWND			m_hWnd; 
	//File Handle����
	HANDLE			m_logFile;
	//TCP/UDP����
	SOCKET			m_sockTcp;
	SOCKET			m_sockUdp;
	//�޽��� ť
	Queue<LogMsg*>	m_queueLogMsg;
	//���� �޼��� ���� ��ġ
	int				m_msgBufferIdx;
	DWORD			m_fileMaxSize;

	/////////////////////////////////////////////////////////////////////////////
	//���� ȣ�� �Լ�
	//��°���..�Լ�	
	void OutputFile(char* outputString);
	void OutputDB(char* outputString);
	void OutputWindow(enumLogInfoType eLogInfo, char* outputString);
	void OutputDebugWnd(char* outputString);
	void OutputUDP(enumLogInfoType eLogInfo, char* outputString);
	void OutputTCP(enumLogInfoType eLogInfo, char* outputString);


	//�ʱ�ȭ �Լ���
	bool InitDB();
	bool InitFile();
	bool InitUDP();
	bool InitTCP();
};


//�۷ι� ����
static char		g_outStr[MAX_OUTPUT_LENGTH];
static LogMsg	g_logMsg[MAX_QUEUE_CNT];
static Monitor	g_log;


//���� ����ڰ� ���� �Լ�
//�ʱ�ȭ �Լ�
bool INIT_LOG(LogConfig& config);
//�α׸� ����� �Լ�
//�α� �޽��� ���� ��� : (������ ������ �߻��� �Լ� | ������ ������ ���� ����)
//ex) IocpServer::BindIocp() | bind() Failed..
void EnQueueLog(enumLogInfoType eLogInfoType, const char* outputString, ...);
// �α׸� ������.
void CLOSE_LOG();


CREATE_FUNCTION(Log, GetLog);