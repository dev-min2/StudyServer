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
		if ((*singletonIter) == this) // �ڱ��ڽ� �ı�
			break;

		++singletonIter;
	}
	m_listSingleton.erase(singletonIter);
}

void Singleton::ReleaseAll()
{
	// ó�� ���Ҵ� �̱��� Ŭ���� �ڱ��ڽ��̹Ƿ�. �Ųٷ� �����.
	SINGLETON_LIST::reverse_iterator singleIter = m_listSingleton.rbegin();
	while (singleIter != m_listSingleton.rend())
	{
		(*singleIter)->ReleaseInstance();
		++singleIter;
	}
	m_listSingleton.clear();
}
