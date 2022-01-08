#pragma once

#define DECLARE_SINGLETON(className)\
public:\
	static className* GetInstance();\
	virtual void ReleaseInstance();\
private:\
	static className* m_pInstance;

#define CREATE_FUNCTION(className, funcName)\
	static className* ##funcName()\
	{\
		return className::GetInstance();\
	}

#define IMPLEMENT_SINGLETON(className)\
className*	className::m_pInstance = nullptr;\
className*	className::GetInstance()\
{\
	if(m_pInstance == nullptr)\
		m_pInstance = new className();\
	return m_pInstance;\
}\
void className::ReleaseInstance()\
{\
	if(m_pInstance != nullptr)\
	{\
		delete m_pInstance;\
		m_pInstance = nullptr;\
	}\
}


#include <list>

class Singleton
{
public:
	// �� Ŭ������ ����ϴ� ��� �̱��� Ŭ�������� �����Ѵ�.
	using SINGLETON_LIST = std::list<Singleton*>;
	Singleton();
	virtual ~Singleton();
public:
	virtual void ReleaseInstance() abstract; // = 0���� �����ϴ� ���� �����Լ��� ����. (�ش� Ŭ������ �߻� Ŭ������ �ȴ�.)
	static void ReleaseAll();

private:
	static SINGLETON_LIST m_listSingleton;


};

