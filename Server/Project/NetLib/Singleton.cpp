#include "Singleton.h"


Singleton::SINGLETON_LIST Singleton::m_listSingleton;

Singleton::Singleton()
{
	m_listSingleton.push_back(this);
}

Singleton::~Singleton()
{
	SINGLETON_LIST::iterator singletonIter = m_listSingleton.begin();
	for (; singletonIter != m_listSingleton.end(); )
	{
		if ((*singletonIter) == this) // 자기자신 파괴
			break;

		++singletonIter;
	}
	m_listSingleton.erase(singletonIter);
}

void Singleton::ReleaseAll()
{
	// 처음 원소는 싱글턴 클래스 자기자신이므로. 거꾸로 지운다.
	SINGLETON_LIST::reverse_iterator singleIter = m_listSingleton.rbegin();
	while (singleIter != m_listSingleton.rend())
	{
		(*singleIter)->ReleaseInstance();
		++singleIter;
	}
	m_listSingleton.clear();
}
