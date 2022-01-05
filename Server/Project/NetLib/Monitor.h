#pragma once
#include "std.h"

// 복사되는것을 방지.
class Monitor
{
public:
	class Owner
	{
	// Owner클래스는 자동으로 CS객체의 소유권을 해제하기 위해 만들어짐

	public:
		Owner(Monitor& crit);
		~Owner();
	private:
		Monitor& m_csSyncObject;
		Owner(const Owner& rhs);
		Owner& operator=(const Owner& rhs);
	};
	Monitor();
	~Monitor();
	BOOL TryEntry();

	// Enter와 Leave는 각각 락을걸고 해제하는 역할.
	// 밑의 m_csObject가 대상.
	void Enter();
	void Leave();
private:
	CRITICAL_SECTION m_csObject;
	Monitor(const Monitor& rhs);
	Monitor& operator=(const Monitor& rhs);
};

