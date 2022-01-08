#pragma once

#include "std.h"

class Thread
{
public:
	Thread();
	virtual ~Thread();

public:
	// 틱 쓰레드는 waitTick / 1000 만큼 대기하면서 실행.
	// 즉 waitTick / 1000 시간마다 자식의 OnProcess가 처리 된다.
	bool CreateThread(DWORD waitTick);
	void DestroyThread();
	void Run();
	void Stop();
	void TickThread();
	virtual void OnProcess() abstract; // 순수 가상함수.
	// 몇번의 틱이 돌았는지 반환.
	inline DWORD GetTickCount() { return m_tickCount; }
	bool IsRun() { return m_bRunState; }

protected:
	HANDLE		m_hThread; // 쓰레드 핸들
	WSAEVENT	m_hQuitEvent; // 이벤트 핸들 (쓰레드 종료를 위해 존재한다)
	bool		m_bRunState;
	DWORD		m_waitTick;
	DWORD		m_tickCount;




};

