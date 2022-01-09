#pragma once
#include "Monitor.h"

template <typename T>
class Queue : public Monitor
{
public:
	Queue(int MaxSize = MAX_QUEUESIZE);
	~Queue();
public:
	// ť�� ������ ����
	bool PushQueue(T queueItem);
	// ť�� ũ�� ����
	void PopQueue();
	// ť�� ������� Ȯ��
	bool IsEmpty();
	//������ ��������
	T GetFrontQueue();
	//ť�� ���� ũ�� ��ȯ
	int GetQueueSize();

	// ť�� �ִ� ũ�⸦ ��ȯ
	int GetQueueMaxSize() { return m_queueMaxSize; }
	void SetQueueMaxSiz(int MaxSize) { m_queueMaxSize = MaxSize; }
	void ClearQueue();

private:
	// ���� �����͸� �����ϴ� �迭
	T* m_arrQueue;
	int m_queueMaxSize;
	// ����ȭ
	Monitor m_csQueue;

	int m_curSize;
	// ť�� �迭�� ���� ������ �����͸� ����Ŵ.
	int m_endMark;
	// ť�� �迭�� ���� ù��° �����͸� ����Ŵ.
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
		// endMark�� ������ �Դٸ� �ٽ� 0���� ���� ������ �߰�.
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
		// ���⼭ Pop�� ó�������� �ʴ´�!
		return m_arrQueue[m_beginMark];
	}
}

template<typename T>
bool Queue<T>::IsEmpty()
{
	bool flag = false;
	// ���Ͷ� �����.
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

