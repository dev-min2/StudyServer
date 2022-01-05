#include "Monitor.h"

Monitor::Monitor()
{
	::InitializeCriticalSection(&m_csObject);
}

Monitor::~Monitor()
{
	// 생성한 csObject 리소스 반환.
	::DeleteCriticalSection(&m_csObject);
}

BOOL Monitor::TryEntry()
{
	// TryEnterCriticalSection는 진입이 가능하면 True를 반환하면서 진입.
	// 진입이 불가능하면 곧바로 false반환 후 대기하지 않고 넘어간다.
	// EnterCriticalSection은 임계영역의 락을 획득할때까지 작업을 block.
	// TryEnterCriticalSection는 락이 획득 가능하던 아니던 결과를 즉시 반환.
	return ::TryEnterCriticalSection(&m_csObject);
}

void Monitor::Enter()
{
	::EnterCriticalSection(&m_csObject);
}

void Monitor::Leave()
{
	::LeaveCriticalSection(&m_csObject);
}

Monitor::Owner::Owner(Monitor& crit) :m_csSyncObject(crit)
{
	m_csSyncObject.Enter();
}

Monitor::Owner::~Owner()
{
	m_csSyncObject.Leave();
}
