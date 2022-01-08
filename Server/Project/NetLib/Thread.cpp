#include "Thread.h"

Thread::Thread() : m_hThread(INVALID_HANDLE_VALUE),m_bRunState(false),m_tickCount(0),m_waitTick(0)
{
	// manual-reset, non-signaled���·� ����.
	m_hQuitEvent = ::WSACreateEvent();
}

Thread::~Thread()
{
	::CloseHandle(m_hQuitEvent);
	if (m_hThread)
		::CloseHandle(m_hThread);
}

unsigned int WINAPI CallTickThread(LPVOID p) // this�� �Ѿ�´�.
{
	Thread* pTickThread = (Thread*)p;
	pTickThread->TickThread(); // ������� ƽ ������ �Լ��� ȣ���Ѵ�. 

	return 0;
}


bool Thread::CreateThread(DWORD waitTick) // ����� ƽ�� ���ڷ� �Ѿ�´�.
{
	UINT uiThreadId = 0;
	// �����·� ������ ����
	m_hThread = (HANDLE)::_beginthreadex(nullptr, 0, CallTickThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_hThread)
	{
		// �α� Ŭ���� �߰� ���� (����ó��)
		return false;
	}

	m_waitTick = waitTick;
	return true;
}

void Thread::DestroyThread()
{
	Run(); // �����尡 ��������� �� �����Ƿ�. ���� Run�� ȣ��(�����ִ��ص� ����üũ�� ���� ���õȴ�)
	::SetEvent(m_hQuitEvent); // �̺�Ʈ�� signaled���·� �ٲ۴�. -> �� �̺�Ʈ�� ������ ������� TickThread�Լ����� �������´�.
	::WaitForSingleObject(m_hThread, INFINITE); // �����尡 ����ɶ����� ����Ѵ�.
}

void Thread::Run()
{
	// �����带 ���۽�Ų��. 
	if (m_bRunState == false)
	{
		m_bRunState = true;
		::ResumeThread(m_hThread);
	}
}

void Thread::Stop()
{
	// �����带 �����.
	if (m_bRunState == true)
	{
		m_bRunState = false;
		::SuspendThread(m_hThread);
	}
}

void Thread::TickThread()
{
	// �����尡 ó���ϴ� ���� �Լ�.
	// �ڽ� Ŭ������ OnProcess�� ȣ���Ѵ�.
	while (true)
	{
		DWORD ret = ::WaitForSingleObject(m_hQuitEvent, m_waitTick); // waitTick��ŭ ���.
		if (ret == WAIT_OBJECT_0) // ���� m_hQuitEvent�� signaled��� (DestroyThread�Լ��� ȣ��Ǿ��ٴ� �Ҹ�)
		{
			break; // �������´�.
		}
		else if(ret == WAIT_TIMEOUT) // �װԾƴ϶� Ÿ�Ӿƿ��̶��. -> ƽ��ŭ ����ϴ� OnProcessȣ��. (ƽ��ŭ ��� -> OnProcessȣ���� �ݺ�)
		{
			++m_tickCount;
			OnProcess();
		}
	}
}
