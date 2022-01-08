#include "Thread.h"

Thread::Thread() : m_hThread(INVALID_HANDLE_VALUE),m_bRunState(false),m_tickCount(0),m_waitTick(0)
{
	// manual-reset, non-signaled상태로 생성.
	m_hQuitEvent = ::WSACreateEvent();
}

Thread::~Thread()
{
	::CloseHandle(m_hQuitEvent);
	if (m_hThread)
		::CloseHandle(m_hThread);
}

unsigned int WINAPI CallTickThread(LPVOID p) // this가 넘어온다.
{
	Thread* pTickThread = (Thread*)p;
	pTickThread->TickThread(); // 쓰레드는 틱 쓰레드 함수를 호출한다. 

	return 0;
}


bool Thread::CreateThread(DWORD waitTick) // 대기할 틱이 인자로 넘어온다.
{
	UINT uiThreadId = 0;
	// 대기상태로 쓰레드 생성
	m_hThread = (HANDLE)::_beginthreadex(nullptr, 0, CallTickThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_hThread)
	{
		// 로그 클래스 추가 예정 (에러처리)
		return false;
	}

	m_waitTick = waitTick;
	return true;
}

void Thread::DestroyThread()
{
	Run(); // 쓰레드가 멈춘상태일 수 있으므로. 먼저 Run을 호출(돌고있다해도 조건체크를 통해 무시된다)
	::SetEvent(m_hQuitEvent); // 이벤트를 signaled상태로 바꾼다. -> 이 이벤트가 켜지면 쓰레드는 TickThread함수에서 빠져나온다.
	::WaitForSingleObject(m_hThread, INFINITE); // 쓰레드가 종료될때까지 대기한다.
}

void Thread::Run()
{
	// 쓰레드를 동작시킨다. 
	if (m_bRunState == false)
	{
		m_bRunState = true;
		::ResumeThread(m_hThread);
	}
}

void Thread::Stop()
{
	// 쓰레드를 멈춘다.
	if (m_bRunState == true)
	{
		m_bRunState = false;
		::SuspendThread(m_hThread);
	}
}

void Thread::TickThread()
{
	// 쓰레드가 처리하는 메인 함수.
	// 자식 클래스의 OnProcess를 호출한다.
	while (true)
	{
		DWORD ret = ::WaitForSingleObject(m_hQuitEvent, m_waitTick); // waitTick만큼 대기.
		if (ret == WAIT_OBJECT_0) // 만약 m_hQuitEvent가 signaled라면 (DestroyThread함수가 호출되었다는 소리)
		{
			break; // 빠져나온다.
		}
		else if(ret == WAIT_TIMEOUT) // 그게아니라 타임아웃이라면. -> 틱만큼 대기하다 OnProcess호출. (틱만큼 대기 -> OnProcess호출이 반복)
		{
			++m_tickCount;
			OnProcess();
		}
	}
}
