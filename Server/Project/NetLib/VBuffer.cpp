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
	// ���ڷ� �Ѱ��� ������ �����͸� �־��ش�.
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
	// ���ڿ� ���̸� �˷��ֱ� ���� 2����Ʈ�� �Ҵ�Ǿ��ִ�.
	// �װ��� ���� �о�;� �Ѵ�.
	short len = 0;
	GetShort(len); // ���⼭ m_pCurMark�� 2����Ʈ �̵�����.(���ڿ� ���� �о��)
	if (len < 0 || len > MAX_PBUFSIZE) // PacketPool���� ���� �Ѱ��� size
		return;
	::strncpy(pBuffer, m_pCurMark, len);
	*(pBuffer + len) = '\0'; // �ι���.
	m_pCurMark	 += len;
	m_curBufSize += len;
}

// ���ڿ� ���� �ٸ� byte stream�� ���� �� ���δ�.
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
	// ���� ���ڿ� ���� 2����Ʈ �Ҵ�.
	short len = ::strlen(pBuffer); // �ι��� �Ҵ��� Get���� �������ش�.
	if (len < 0 || len > MAX_PBUFSIZE)
		return;
	SetShort(len); // ���ڿ� ���� 2����Ʈ �Ҵ�.

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
	// ��Ŷ�� �ҷ��� �д´�.
	m_pCurMark = pVBuffer + PACKET_SIZE_LENGTH;
	m_curBufSize = 0;
}


bool VBuffer::CopyBuffer(char* pDestBuffer)
{
	// ��Ŷ���� ����.
	::CopyMemory(m_pVBuffer, &m_curBufSize, PACKET_SIZE_LENGTH);
	// ������ ������ ���� ��Ŷ���ۿ� ����.
	::CopyMemory(pDestBuffer, m_pVBuffer, m_curBufSize);
	return true;
}

void VBuffer::Init()
{
	// PACKET_SIZE_LENGTH��ŭ �����͸� ������Ų��.(4)
	// ��ġ �����ϴ� ��.
	m_pCurMark	 = m_pVBuffer + PACKET_SIZE_LENGTH;
	m_curBufSize = PACKET_SIZE_LENGTH;
}
