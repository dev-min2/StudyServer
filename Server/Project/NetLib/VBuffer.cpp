#include "VBuffer.h"

IMPLEMENT_SINGLETON(VBuffer)


VBuffer::VBuffer(int maxBuffSize) : m_pVBuffer(new char[maxBuffSize]),m_maxBufSize(maxBuffSize),
m_pCurMark(nullptr),m_curBufSize(0)
{
	//Init();
	::ZeroMemory(m_pVBuffer, maxBuffSize);
}

VBuffer::~VBuffer()
{
	if (m_pVBuffer)
		delete[] m_pVBuffer;
	m_pVBuffer = nullptr;
}

void VBuffer::GetChar(char& ch)
{
	// 인자로 넘겨준 변수에 데이터를 넣어준다.
	ch = *m_pCurMark;
	m_pCurMark   += sizeof(char);
	m_curBufSize += sizeof(char);
}

void VBuffer::GetShort(short& num)
{
	int len = sizeof(short);
	::CopyMemory(&num, m_pCurMark, len);
	m_pCurMark += len;
	m_curBufSize += len;
}

void VBuffer::GetInteger(int& num)
{
	int len = sizeof(INT32);
	::CopyMemory(&num, m_pCurMark, len);
	m_pCurMark += len;
	m_curBufSize += len;
}

void VBuffer::GetStrirng(char* pBuffer)
{
	// 문자열 길이를 알려주기 위한 2바이트가 할당되어있다.
	// 그것을 먼저 읽어와야 한다.
	short len = 0;
	GetShort(len); // 여기서 m_pCurMark가 2바이트 이동했음.(문자열 길이 읽어옴)
	if (len < 0 || len > MAX_PBUFSIZE) // PacketPool에서 버퍼 한개당 size
		return;
	::strncpy(pBuffer, m_pCurMark, len);
	*(pBuffer + len) = '\0'; // 널문자.
	m_pCurMark	 += len;
	m_curBufSize += len;
}

// 문자열 말고 다른 byte stream을 읽을 때 쓰인다.
void VBuffer::GetStream(char* pBuffer, short len)
{
	if (len < 0 || len > MAX_PBUFSIZE)
		return;
	::CopyMemory(pBuffer, m_pCurMark, len);
	m_pCurMark += len;
	m_curBufSize += len;
}

void VBuffer::SetInteger(int num)
{
	int len = sizeof(INT32);
	::CopyMemory(m_pCurMark, &num, len);
	m_pCurMark += len;
	m_curBufSize += len;
}

void VBuffer::SetShort(short num)
{
	int len = sizeof(short);
	::CopyMemory(m_pCurMark, &num, len);
	m_pCurMark += len;
	m_curBufSize += len;
}

void VBuffer::SetChar(char ch)
{
	int len = sizeof(char);
	::CopyMemory(m_pCurMark, &ch, len);
	m_pCurMark += len;
	m_curBufSize += len;
}

void VBuffer::SetString(char* pBuffer)
{
	// 먼저 문자열 길이 2바이트 할당.
	short len = ::strlen(pBuffer); // 널문자 할당은 Get에서 따로해준다.
	if (len < 0 || len > MAX_PBUFSIZE)
		return;
	SetShort(len); // 문자열 길이 2바이트 할당.

	::CopyMemory(m_pCurMark, pBuffer, len);
	m_pCurMark += len;
	m_curBufSize += len;
}

void VBuffer::SetStream(char* pBuffer, short len)
{
	::CopyMemory(m_pCurMark, pBuffer, len);
	m_pCurMark += len;
	m_curBufSize += len;
}

void VBuffer::SetBuffer(char* pVBuffer)
{
	// 패킷을 불러와 읽는다.
	m_pCurMark = pVBuffer + PACKET_SIZE_LENGTH;
	m_curBufSize = 0;
}


bool VBuffer::CopyBuffer(char* pDestBuffer)
{
	// 패킷길이 복사.
	::CopyMemory(m_pVBuffer, &m_curBufSize, PACKET_SIZE_LENGTH);
	// 설정한 데이터 값들 패킷버퍼에 복사.
	::CopyMemory(pDestBuffer, m_pVBuffer, m_curBufSize);
	return true;
}

void VBuffer::Init()
{
	// PACKET_SIZE_LENGTH만큼 포인터를 증가시킨다.(4)
	// 위치 세팅하는 것.
	m_pCurMark	 = m_pVBuffer + PACKET_SIZE_LENGTH;
	m_curBufSize = PACKET_SIZE_LENGTH;
}
