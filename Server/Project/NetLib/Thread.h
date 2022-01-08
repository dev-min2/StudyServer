#pragma once

#include "std.h"

class Thread
{
public:
	Thread();
	virtual ~Thread();

public:
	// ƽ ������� waitTick / 1000 ��ŭ ����ϸ鼭 ����.
	// �� waitTick / 1000 �ð����� �ڽ��� OnProcess�� ó�� �ȴ�.
	bool CreateThread(DWORD waitTick);
	void DestroyThread();
	void Run();
	void Stop();
	void TickThread();
	virtual void OnProcess() abstract; // ���� �����Լ�.
	// ����� ƽ�� ���Ҵ��� ��ȯ.
	inline DWORD GetTickCount() { return m_tickCount; }
	bool IsRun() { return m_bRunState; }

protected:
	HANDLE		m_hThread; // ������ �ڵ�
	WSAEVENT	m_hQuitEvent; // �̺�Ʈ �ڵ� (������ ���Ḧ ���� �����Ѵ�)
	bool		m_bRunState;
	DWORD		m_waitTick;
	DWORD		m_tickCount;




};

