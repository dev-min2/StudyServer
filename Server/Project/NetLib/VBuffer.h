#pragma once

#include "std.h"
#include "Singleton.h"

class VBuffer : public Singleton
{
	DECLARE_SINGLETON(VBuffer);
public:
	// 생성자. 입력받은 크기를 인자로 받아 내부버퍼 생성.
	VBuffer(int maxBuffSize = MAX_VPBUFSIZE);
	~VBuffer();
public:
	void GetChar(char& ch);
	void GetShort(short& num);
	void GetInteger(int& num);
	void GetStrirng(char* pBuffer);
	void GetStream(char* pBuffer, short len);
	// 여기까지 가변 버퍼에서 데이터 형에 해당하는 값을 읽어와 인자로 넣어준 변수에 넣어준다.
	// 값을 읽고나면 설정되어 있는 버퍼의 포인터가 자동으로 읽은 데이터 형의 크기만큼 증가.
public:
	void SetInteger(int num);
	void SetShort(short num);
	void SetChar(char ch);
	void SetString(char* pBuffer);
	void SetStream(char* pBuffer, short len);
	// 여기까지 인자로 넘겨준 값을 가변 버퍼의 내부에 설정하는 역할을 하는 함수.
	// 값을 설정하고 나면 내부 버퍼의 포인터가 설정한 값의 데이터 형 크기만큼 증가한다.
	void SetBuffer(char* pVBuffer);


public:
	inline int		GetMaxBufSize() { return m_maxBufSize; }
	inline int		GetCurBufSize() { return m_curBufSize; }
	inline char*	GetCurMark()	{ return m_pCurMark; }
	inline char*	GetBeginMark() { return m_pVBuffer; }
public:
	// CopyBuffer는 VBuffer클래스의 내부 버퍼를 인자로 받은 변수 버퍼에 복사한다.
	// 이것은 내부 버퍼에 가변 데이터 값을 모두 다 설정한 후에 내부 버퍼를 사용자 버퍼로 옮길 때 사용하고
	// 복사하기 전에 초기화 때 비워두었던 패킷 길이를 설정해준다.
	bool CopyBuffer(char* pDestBuffer);
	void Init();

private:
	char* m_pVBuffer; // 실제버퍼
	char* m_pCurMark; // 현재 버퍼 위치.
	int   m_maxBufSize; // 최대 버퍼 사이즈
	int   m_curBufSize; // 현재 사용된 버퍼 사이즈

private: // 복생 금지.
	VBuffer(const VBuffer& rhs) = delete;
	VBuffer& operator=(const VBuffer& rhs) = delete;
};

CREATE_FUNCTION(VBuffer, GetVBuffer);