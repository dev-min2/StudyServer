#pragma once
#include "std.h"

#define 	MAX_STORAGE_TYPE		6
#define		MAX_FILENAME_LENGTH		100
#define		MAX_DSN_NAME			100
#define		MAX_DSN_ID				20
#define		MAX_DSN_PW				20
#define		MAX_STRING_LENGTH		1024	
#define		MAX_OUTPUT_LENGTH		1024 * 4
#define		MAX_QUEUE_CNT			10000
#define		WM_DEBUGMSG				WM_USER + 1
#define		DEFAULT_UDPPORT			1555
#define		DEFAULT_TCPPORT			1556
#define		DEFAULT_TICK			1000
#define		MAX_LOGFILE_SIZE		1024 * 200000   //200MB	

//각 등급마다 16진수설정.
// OR(|)연산을 위해 2^n승씩 증가.
// 0000/0000 에서 비트는 2^n의 자리를 차지.
enum class enumLogInfoType
{
	  LOG_NONE				= 0x00000000
	, LOG_INFO_LOW			= 0x00000001
	, LOG_INFO_NORMAL		= 0x00000002
	, LOG_INFO_HIGH			= 0x00000004
	, LOG_INFO_CRITICAL		= 0x00000008
	, LOG_INFO_ALL			= 0x0000000F
	, LOG_ERROR_LOW			= 0x00000010
	, LOG_ERROR_NORMAL		= 0x00000020
	, LOG_ERROR_HIGH		= 0x00000040
	, LOG_ERROR_CRITICAL	= 0x00000080
	, LOG_ERROR_ALL			= 0x00000100
	, LOG_ALL				= 0x000001FF
};

// 정보의 분류별로 여러가지 방법으로 로그를 저장한다.
// 즉 로그를 저장할 매체에 대한 선언
enum class enumLogStorageType
{
	  STORAGE_FILE		= 0x000000000
	, STORAGE_DB		= 0x000000001
	, STORAGE_WINDOW	= 0x000000002
	, STORAGE_OUTPUTWND = 0x000000003
	, STORAGE_UDP		= 0x000000004
	, STORAGE_TCP		= 0x000000005
};

enum class enumLogFileType
{
	  FILETYPE_NONE		= 0x00
	, FILETYPE_XML		= 0x01
	, FILETYPE_TEXT		= 0x02
	, FILETYPE_ALL		= 0x03
};
static char	logInfoType_StringTable[][100] =
{
		"LOG_NONE",
		"LOG_INFO_LOW",													//0x00000001
		"LOG_INFO_NORMAL",												//0x00000002
		"LOG_INFO_LOW , LOG_INFO_NORMAL",								//0x00000003	
		"LOG_INFO_HIGH",												//0x00000004	
		"LOG_INFO_LOW , LOG_INFO_HIGH",									//0x00000005
		"LOG_INFO_NORMAL , LOG_INFO_HIGH",								//0x00000006
		"LOG_INFO_LOW , LOG_INFO_NORMAL , LOG_INFO_HIGH",				//0x00000007
		"LOG_INFO_CRITICAL",											//0x00000008
		"LOG_INFO_LOW , LOG_INFO_CRITICAL",								//0x00000009
		"LOG_INFO_NORMAL , LOG_INFO_CRITICAL",							//0x0000000A
		"LOG_INFO_LOW , LOG_INFO_NORMAL , LOG_INFO_CRITICAL",			//0x0000000B
		"LOG_INFO_HIGH , LOG_INFO_CRITICAL",							//0x0000000C
		"LOG_INFO_LOW , LOG_INFO_HIGH , LOG_INFO_CRITICAL",				//0x0000000D
		"LOG_INFO_NORMAL , LOG_INFO_HIGH , LOG_INFO_CRITICAL",			//0x0000000E
		"LOG_INFO_ALL",													//0x0000000F

		"LOG_ERROR_LOW",												//0x00000010
		"LOG_ERROR_NORMAL",												//0x00000020
		"LOG_ERROR_LOW , LOG_ERROR_NORMAL",								//0x00000030	
		"LOG_ERROR_HIGH",												//0x00000040	
		"LOG_ERROR_LOW , LOG_ERROR_HIGH",								//0x00000050
		"LOG_ERROR_NORMAL , LOG_ERROR_HIGH",							//0x00000060
		"LOG_ERROR_LOW , LOG_ERROR_NORMAL , LOG_ERROR_HIGH",			//0x00000070
		"LOG_ERROR_CRITICAL",											//0x00000080
		"LOG_ERROR_LOW , LOG_ERROR_CRITICAL",							//0x00000090
		"LOG_ERROR_NORMAL , LOG_ERROR_CRITICAL",						//0x000000A0
		"LOG_ERROR_LOW , LOG_ERROR_NORMAL , LOG_ERROR_CRITICAL",		//0x000000B0
		"LOG_ERROR_HIGH , LOG_ERROR_CRITICAL",							//0x000000C0
		"LOG_ERROR_LOW , LOG_ERROR_HIGH , LOG_ERROR_CRITICAL",			//0x000000D0
		"LOG_ERROR_NORMAL , LOG_ERROR_HIGH , LOG_ERROR_CRITICAL",		//0x000000F0
		"LOG_ERROR_ALL",												//0x00000100
		"LOG_ALL"														//0x00000200	
};
