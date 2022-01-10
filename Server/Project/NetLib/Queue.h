#pragma once
#include "Monitor.h"

template <typename T>
class Queue : public Monitor
{
public:
	Queue(int MaxSize = MAX_QUEUESIZE);
	~Queue();
public:
	// 큐에 데이터 삽입
	bool PushQueue(T queueItem);
	// 큐의 크기 감소
	void PopQueue();
	// 큐가 비었는지 확인
	bool IsEmpty();
	//데이터 가져오기
	T GetFrontQueue();
	//큐의 현재 크기 반환
	int GetQueueSize();

	// 큐의 최대 크기를 반환
	int GetQueueMaxSize() { return m_queueMaxSize; }
	void SetQueueMaxSiz(int MaxSize) { m_queueMaxSize = MaxSize; }
	void ClearQueue();

private:
	// 실제 데이터를 저장하는 배열
	T* m_arrQueue;
	int m_queueMaxSize;
	// 동기화
	Monitor m_csQueue;

	int m_curSize;
	// 큐의 배열에 가장 마지막 데이터를 가르킴.
	int m_endMark;
	// 큐의 배열에 가장 첫번째 데이터를 가르킴.
	int m_beginMark;

};

template<typename T>
Queue<T>::Queue(int maxSize)
{
	m_arrQueue = new T[maxSize];
	m_queueMaxSize = maxSize;
	ClearQueue();
}

template<typename T>
Queue<T>::~Queue()
{
	delete[] m_arrQueue;
}

template<typename T>
bool Queue<T>::PushQueue(T queueItem)
{
	Monitor::Owner lock(m_csQueue);
	{
		if (m_curSize >= m_queueMaxSize)
			return false;

		++m_curSize;
		// endMark가 끝까지 왔다면 다시 0으로 돌려 데이터 추가.
		if (m_endMark == m_queueMaxSize)
			m_endMark = 0;
		m_arrQueue[m_endMark++] = queueItem;
	}
}

template<typename T>
void Queue<T>::PopQueue()
{
	Monitor::Owner lock(m_csQueue);
	{
		--m_curSize;
		++m_beginMark;
	}
}

template<typename T>
T Queue<T>::GetFrontQueue()
{
	Monitor::Owner lock(m_csQueue);
	{
		if (m_curSize <= 0)
			return NULL;
		if (m_beginMark == m_queueMaxSize)
			m_beginMark = 0;
		// 여기서 Pop을 처리하지는 않는다!
		return m_arrQueue[m_beginMark];
	}
}

template<typename T>
bool Queue<T>::IsEmpty()
{
	bool flag = false;
	// 인터락 아토믹.
	Monitor::Owner lock(m_csQueue);
	{
		flag = (m_curSize > 0) ? false : true;
	}
	return flag;
}

template<typename T>
int Queue<T>::GetQueueSize()
{
	int size;
	Monitor::Owner lock(m_csQueue);
	{
		size = m_curSize;
	}
	return size;
}

template<typename T>
void Queue<T>::ClearQueue()
{
	Monitor::Owner lock(m_csQueue);
	{
		m_curSize = 0;
		m_endMark = 0;
		m_beginMark = 0;
	}
}

