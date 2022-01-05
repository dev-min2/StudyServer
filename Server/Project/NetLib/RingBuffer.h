#pragma once
#include "std.h"
#include "Monitor.h"
class RingBuffer : public Monitor
{
public:
	RingBuffer();
	~RingBuffer();

	// �� ���� �޸� �Ҵ�
	bool			Create(int bufferSIze = MAX_RINGBUFSIZE);
	// �ʱ�ȭ
	bool			Initialize();
	//�Ҵ�� ���� ũ�⸦ ��ȯ
	inline  int		GetBufferSize() { return m_bufferSize; }

	//�ش��ϴ� ���� ������ �����͸� ��ȯ
	inline char*	GetBeginMark() { return m_pBeginMark; }
	inline char*	GetCurrentMark() { return m_pCurrentMark; }
	inline char*	GetEndMark() { return m_pEndMark; }
	//���� ������ ���� �����͸� ����
	char*			ForwardMark(int forwardLen);
	char*			ForwardMark(int forwardLen, int nextLen, DWORD remainLen);

	// ���� ���� ���� ����
	void			ReleaseBuffer(int releaseSize);
	// ���� ���� ���� ũ�� ��ȯ
	inline int		GetUsedBufferSize() { return m_usedBufferSize; }
	// ���� ���� ���� ��ȯ
	inline int		GetAllUsedBufferSize() { return m_allUserBufSize; }
	// ���� ���� ������ �о ��ȯ
	char*			GetBuffer(int readSize, int* pReadSize);
private:
	char*			m_pRingBuffer;			// ���� �����͸� �����ϴ� ���� ������

	// LastMoveMark���� ������ ��ġ�� ����Ű�� ������ ������.
	char*			m_pBeginMark;				// ������ ó���κ��� ����Ű�� �ִ� ������
	char*			m_pEndMark;				// ������ ������ �κ��� ����Ű�� �ִ� ������
	char*			m_pCurrentMark;			// ������ ������� ���� �κ��� ����Ű�� �ִ� ������.
	char*			m_pGettedBufferMark;		// ������� �����͸� ���� ���� ������
	char*			m_pLastMoveMark;			// recycle�Ǳ� ���� ������ ������

	int				m_bufferSize; // ���� ������ �� ũ��
	int				m_usedBufferSize; // ���� ���� ���� ������ ũ��
	UINT			m_allUserBufSize; // �� ó���� ������ ��
	Monitor			m_csRingBuffer; // ����ȭ ��ü

private:
	// ���� ����.
	RingBuffer(const RingBuffer& rhs)			 = delete;
	RingBuffer& operator=(const RingBuffer& rhs) = delete;
	


};

