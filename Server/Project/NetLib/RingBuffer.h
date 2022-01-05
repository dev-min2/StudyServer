#pragma once
#include "std.h"
#include "Monitor.h"
class RingBuffer : public Monitor
{
public:
	RingBuffer();
	~RingBuffer();

	// 링 버퍼 메모리 할당
	bool			Create(int bufferSIze = MAX_RINGBUFSIZE);
	// 초기화
	bool			Initialize();
	//할당된 버퍼 크기를 반환
	inline  int		GetBufferSize() { return m_bufferSize; }

	//해당하는 내부 버퍼의 포인터를 반환
	inline char*	GetBeginMark() { return m_pBeginMark; }
	inline char*	GetCurrentMark() { return m_pCurrentMark; }
	inline char*	GetEndMark() { return m_pEndMark; }
	//내부 버퍼의 현재 포인터를 전진
	char*			ForwardMark(int forwardLen);
	char*			ForwardMark(int forwardLen, int nextLen, DWORD remainLen);

	// 사용된 내부 버퍼 해제
	void			ReleaseBuffer(int releaseSize);
	// 사용된 내부 버퍼 크기 반환
	inline int		GetUsedBufferSize() { return m_usedBufferSize; }
	// 누적 버퍼 사용양 반환
	inline int		GetAllUsedBufferSize() { return m_allUserBufSize; }
	// 내부 버퍼 데이터 읽어서 반환
	char*			GetBuffer(int readSize, int* pReadSize);
private:
	char*			m_pRingBuffer;			// 실제 데이터를 저장하는 버퍼 포인터

	// LastMoveMark까지 버퍼의 위치를 가리키는 포인터 변수들.
	char*			m_pBeginMark;				// 버퍼의 처음부분을 가리키고 있는 포인터
	char*			m_pEndMark;				// 버퍼의 마지막 부분을 가리키고 있는 포인터
	char*			m_pCurrentMark;			// 버퍼의 현재까지 사용된 부분을 가리키고 있는 포인터.
	char*			m_pGettedBufferMark;		// 현재까지 데이터를 읽은 버퍼 포인터
	char*			m_pLastMoveMark;			// recycle되기 전에 마지막 포인터

	int				m_bufferSize; // 내부 버퍼의 총 크기
	int				m_usedBufferSize; // 현재 사용된 내부 버퍼의 크기
	UINT			m_allUserBufSize; // 총 처리된 데이터 양
	Monitor			m_csRingBuffer; // 동기화 객체

private:
	// 복사 금지.
	RingBuffer(const RingBuffer& rhs)			 = delete;
	RingBuffer& operator=(const RingBuffer& rhs) = delete;
	


};

