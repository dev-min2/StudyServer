#include "Monitor.h"

Monitor::Monitor()
{
	::InitializeCriticalSection(&m_csObject);
}

Monitor::~Monitor()
{
	// ������ csObject ���ҽ� ��ȯ.
	::DeleteCriticalSection(&m_csObject);
}

BOOL Monitor::TryEntry()
{
	// TryEnterCriticalSection�� ������ �����ϸ� True�� ��ȯ�ϸ鼭 ����.
	// ������ �Ұ����ϸ� ��ٷ� false��ȯ �� ������� �ʰ� �Ѿ��.
	// EnterCriticalSection�� �Ӱ迵���� ���� ȹ���Ҷ����� �۾��� block.
	// TryEnterCriticalSection�� ���� ȹ�� �����ϴ� �ƴϴ� ����� ��� ��ȯ.
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
