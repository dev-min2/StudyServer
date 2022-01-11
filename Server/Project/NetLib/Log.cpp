#include "Log.h"
IMPLEMENT_SINGLETON(Log);


//�����尡 ó���ϴ� �Լ�.
void Log::OnProcess()
{
	UINT logCount = m_queueLogMsg.GetQueueSize();
	for (UINT i = 0; i < logCount; ++i)
	{
		LogMsg* pLogMsg = m_queueLogMsg.GetFrontQueue();
		//�α� ���
		LogOutput(pLogMsg->m_eLogInfoType, pLogMsg->m_outputString);
		m_queueLogMsg.PopQueue();
	}

}

void Log::OutputFile(char* outputString)
{
	if (m_logFile == NULL)
		return;

	DWORD writtenBytes = 0;
	DWORD size = 0;
	size = ::GetFileSize(m_logFile, NULL);
	//ȭ�� �뷮�� ���ѿ� �ɷȴٸ�
	if (size > m_fileMaxSize || size > MAX_LOGFILE_SIZE)
	{
		char strTime[100];
		time_t curTime;
		tm* locTime;

		curTime = ::time(NULL);
		locTime = ::localtime(&curTime);
		::strftime(strTime, 100, "%m��%d��%H��%M��", locTime);
		m_logFileName[::strlen(m_logFileName) - 21] = NULL;
		::sprintf(m_logFileName, "%s_%s.log", m_logFileName, strTime);
		::CloseHandle(m_logFile);
		m_logFile = NULL;
		InitFile();
	}

	//ȭ���� ������ ���� ������ �ű��
	::SetFilePointer(m_logFile, 0, 0, FILE_END);
	bool ret = ::WriteFile(m_logFile, outputString, ::strlen(outputString), &writtenBytes, NULL);

}

void Log::OutputDB(char* outputString)
{
	//-- DB�� �α׸� ����� �ڵ�
}


void Log::OutputWindow(enumLogInfoType eLogInfo, char* outputString)
{
	if (m_hWnd == NULL)
		return;
	::SendMessageA(m_hWnd, WM_DEBUGMSG, (WPARAM)outputString, (LPARAM)eLogInfo);
}

void Log::OutputDebugWnd(char* outputString)
{
	//�����â�� ��� (����� ��Ʃ��� ���â)
	::OutputDebugStringA(outputString);
}

void Log::OutputUDP(enumLogInfoType eLogInfo, char* outputString)
{
	//udp packet�� ������ ����,.
	SOCKADDR_IN addr;
	::ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::inet_addr(m_IP);
	addr.sin_port = ::htons(m_udpPort);
	
	//���� ��Ŷ����
	int bufLen = ::strlen(outputString);
	if (m_sockUdp == INVALID_SOCKET)
	{
		m_sockUdp = ::socket(AF_INET, SOCK_DGRAM, 0);
	}
	int res = ::sendto(m_sockUdp, outputString, bufLen, 0, (SOCKADDR*)&addr, sizeof(addr));

}

void Log::OutputTCP(enumLogInfoType eLogInfo, char* outputString)
{
	int len = ::strlen(outputString);
	int res = ::send(m_sockTcp, outputString, len, 0);
}

bool Log::InitDB()
{
	/*--
	��� ����
	--*/
	return true;
}

bool Log::InitFile()
{
	m_logFile = ::CreateFileA(m_logFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(m_logFile == NULL)
		return false;
	return true;
}

bool Log::InitUDP()
{
	WSADATA	wsa;
	int ret = ::WSAStartup(MAKEWORD(2, 2), &wsa);
	if (ret) // 0�̾�� �Ѵ�.
		return false;
	return true;
}

bool Log::InitTCP()
{
	WSADATA	wsa;
	int ret = ::WSAStartup(MAKEWORD(2, 2), &wsa);
	if (ret) // 0�̾�� �Ѵ�.
		return false;


	m_sockTcp = ::socket(AF_INET, SOCK_STREAM, 0);

	if (m_sockTcp != INVALID_SOCKET)
		return false;


	SOCKADDR_IN addr;
	::ZeroMemory(&addr, sizeof(SOCKADDR_IN));
	addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = ::inet_addr(m_IP);
	addr.sin_port		 = ::htons(m_tcpPort);

	// ���� ��Ŷ ����
	
	ret = ::connect(m_sockTcp, (SOCKADDR*)&addr, sizeof(SOCKADDR));
	if (ret == SOCKET_ERROR)
		return false;
	return true;
}

bool INIT_LOG(LogConfig& config)
{
	return GetLog()->Init(config);
}

void EnQueueLog(enumLogInfoType eLogInfoType, const char* outputString, ...)
{
	Monitor::Owner lock(g_log);
	UINT queueCnt = GetLog()->GetQueueSize();
	//���� ť ����� �ʰ��ߴٸ�
	if (queueCnt >= MAX_QUEUE_CNT)
		return;

	va_list argptr;
	// argptr�� �� ó�� �����μ��� ����Ű���� �ʱ�ȭ.
	va_start(argptr, outputString);
	vsprintf(g_logMsg[queueCnt].m_outputString, outputString, argptr);
	va_end(argptr);

	g_logMsg[queueCnt].m_eLogInfoType = eLogInfoType;
	GetLog()->InsertMsgToQueue(&g_logMsg[queueCnt]);
}

void CLOSE_LOG()
{
	GetLog()->CloseAllLog();
}

Log::Log()
{
	::ZeroMemory(m_logInfoTypes, MAX_STORAGE_TYPE * sizeof(int));
	::ZeroMemory(m_logFileName, MAX_FILENAME_LENGTH);
	::ZeroMemory(m_IP, MAX_IP_LENGTH);
	::ZeroMemory(m_dsnName, MAX_DSN_NAME);
	::ZeroMemory(m_dsnID, MAX_DSN_ID);
	::ZeroMemory(m_dsnPW, MAX_DSN_PW);
	m_eLogFileType = enumLogFileType::FILETYPE_NONE;
	m_hWnd = NULL;
	m_logFile = NULL;
	m_sockUdp = INVALID_SOCKET;
	m_msgBufferIdx = 0;
	m_udpPort = DEFAULT_UDPPORT;
	m_tcpPort = DEFAULT_TCPPORT;
	m_serverType = 0;
	m_fileMaxSize = 0;
}

Log::~Log()
{
}

bool Log::Init(LogConfig& config)
{
	char	strTime[100];
	time_t	curTime;
	tm* locTime; // Timer
	curTime = ::time(NULL);
	locTime = ::localtime(&curTime);

	//�� ������ ����
	::CopyMemory(m_logInfoTypes, config.m_logInfoTypes, MAX_STORAGE_TYPE * sizeof(INT32));

	::strftime(strTime, 100, "%m��%d��%H��%M��", locTime);

	//LOG ���丮 ����
	::CreateDirectoryA("..\\..\\LOG", NULL);
	::sprintf(m_logFileName, "..\\..\\Log\\%s_%s.log", config.m_logFileName, strTime);

	::strncpy(m_IP, config.m_IP, MAX_IP_LENGTH);
	::strncpy(m_dsnName, config.m_dsnName, MAX_DSN_NAME);
	::strncpy(m_dsnID, config.m_dsnID, MAX_DSN_ID);
	::strncpy(m_dsnPW, config.m_dsnPW, MAX_DSN_PW);

	m_eLogFileType	= config.m_eLogFileType;
	m_tcpPort		= config.m_tcpPort;
	m_udpPort		= config.m_udpPort;
	m_serverType	= config.m_serverType;
	m_fileMaxSize	= config.m_fileMaxSize;

	m_hWnd			= config.m_hWnd;
	bool bRet		= false;

	//���Ϸα׸� �����ߴٸ�
	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_FILE] != (int)enumLogInfoType::LOG_NONE)
	{
		bRet = InitFile();
	}
	if (!bRet)
	{
		CloseAllLog();
		return false;
	}
	//db�α׸� �����ߴٸ�
	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_DB] != (int)enumLogInfoType::LOG_NONE)
	{
		bRet = InitDB();
	}
	if (!bRet)
	{
		CloseAllLog();
		return false;
	}
	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_UDP] != (int)enumLogInfoType::LOG_NONE)
	{
		bRet = InitUDP();
	}
	if (!bRet)
	{
		CloseAllLog();
		return false;
	}

	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_TCP] != (int)enumLogInfoType::LOG_NONE)
	{
		bRet = InitTCP();
	}
	if (!bRet)
	{
		CloseAllLog();
		return false;
	}


	Thread::CreateThread(config.m_processTick); // ������ ����
	Thread::Run(); // ����!



	return true;
}

void Log::CloseAllLog()
{
	::ZeroMemory(m_logInfoTypes, MAX_STORAGE_TYPE * sizeof(int));
	::ZeroMemory(m_logFileName, MAX_FILENAME_LENGTH);
	::ZeroMemory(m_IP, MAX_IP_LENGTH);
	::ZeroMemory(m_dsnName, MAX_DSN_NAME);
	::ZeroMemory(m_dsnID, MAX_DSN_ID);
	::ZeroMemory(m_dsnPW, MAX_DSN_PW);
	m_udpPort = DEFAULT_UDPPORT;
	m_tcpPort = DEFAULT_TCPPORT;
	m_eLogFileType = enumLogFileType::FILETYPE_NONE;
	m_hWnd = NULL;
	m_msgBufferIdx = 0;

	//ȭ�Ϸα� ������
	if (m_logFile)
	{
		::CloseHandle(m_logFile);
		m_logFile = NULL;
	}
	//UDP���� �ʱ�ȭ
	if (m_sockUdp != INVALID_SOCKET)
	{
		::closesocket(m_sockUdp);
		m_sockUdp = INVALID_SOCKET;
	}
	//TCP���� �ʱ�ȭ
	if (m_sockTcp != INVALID_SOCKET)
	{
		::shutdown(m_sockTcp, SD_BOTH);
		::closesocket(m_sockTcp);
		m_sockTcp = INVALID_SOCKET;
	}

	// ������ ���߱�
	Thread::Stop();


}


void Log::LogOutput(enumLogInfoType eLogInfo, char* outputString)
{
	// ��� �������ݷ� �ٸ� ���α׷����� �α׸� �������� ���� Ÿ�԰� �޼����� �������Ѵ�.
	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_UDP] & (int)eLogInfo)
	{
		OutputUDP(eLogInfo, outputString);
	}
	if(m_logInfoTypes[(int)enumLogStorageType::STORAGE_TCP] & (int)eLogInfo)
	{
		OutputTCP(eLogInfo, outputString);
	}

	//�α�,�ð� : ���� ���� : �������� : ��� ���ڿ�
	//���� �ð� ������
	char time[25];
	time_t curTime;
	tm* locTime;

	//LOG ENUM�� StringTable���� ������ ��ġ ��Ų��.
	int idx = (int)eLogInfo;
	if (((int)eLogInfo >> 8) != 0)
		idx = ((int)eLogInfo >> 8) + 0x20 - 3;
	else if (((int)eLogInfo >> 4) != 0)
		idx = ((int)eLogInfo >> 4) + 0x10 - 1;
	if (idx < 0 || idx > 31)
		return;
	curTime = ::time(NULL);
	locTime = ::localtime(&curTime);

	::strftime(time, 25, "%Y/%m/%d(%H:%M:%S)", locTime);
	::sprintf(m_outStr, "%s | %s | %s | %s%c%c", time, ((int)eLogInfo >> 4) ? "����" : "����",
		logInfoType_StringTable[idx],
		outputString, 0x0d, 0x0a);

	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_FILE] & (int)eLogInfo)
	{
		OutputFile(m_outStr);
	}
	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_DB] & (int)eLogInfo)
	{
		OutputDB(m_outStr);
	}

	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_WINDOW] & (int)eLogInfo)
	{
		OutputWindow(eLogInfo,m_outStr);
	}

	if (m_logInfoTypes[(int)enumLogStorageType::STORAGE_OUTPUTWND] & (int)eLogInfo)
	{
		OutputDebugWnd(m_outStr);
	}
}

void Log::LogOutputLastErrorToMsgBox(char* outputString)
{
	int lastError = ::GetLastError();
	if (lastError == 0)
		return;
	LPVOID pDump;
	DWORD res;
	res = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, lastError,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&pDump, 0, NULL);

	::sprintf(g_outStr, "������ġ : %s \n ������ȣ : %d\n���� : %s", outputString, lastError, pDump);
	::MessageBoxA(NULL, g_outStr, "GetLastError Ȯ��", MB_OK);

	if (res)
		::LocalFree(pDump);

}

