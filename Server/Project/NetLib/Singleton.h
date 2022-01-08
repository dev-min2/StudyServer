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
	// 이 클래스를 상속하는 모든 싱글톤 클래스들을 관리한다.
	using SINGLETON_LIST = std::list<Singleton*>;
	Singleton();
	virtual ~Singleton();
public:
	virtual void ReleaseInstance() abstract; // = 0으로 구현하는 순수 가상함수와 동일. (해당 클래스는 추상 클래스가 된다.)
	static void ReleaseAll();

private:
	static SINGLETON_LIST m_listSingleton;


};

