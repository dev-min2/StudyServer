#pragma once
#include "std.h"

// ����Ǵ°��� ����.
class Monitor
{
public:
	class Owner
	{
	// OwnerŬ������ �ڵ����� CS��ü�� �������� �����ϱ� ���� �������

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

	// Enter�� Leave�� ���� �����ɰ� �����ϴ� ����.
	// ���� m_csObject�� ���.
	void Enter();
	void Leave();
private:
	CRITICAL_SECTION m_csObject;
	Monitor(const Monitor& rhs);
	Monitor& operator=(const Monitor& rhs);
};

