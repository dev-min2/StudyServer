#pragma once

#include "std.h"
#include "Singleton.h"

class VBuffer : public Singleton
{
	DECLARE_SINGLETON(VBuffer);
public:
	// ������. �Է¹��� ũ�⸦ ���ڷ� �޾� ���ι��� ����.
	VBuffer(int maxBuffSize = MAX_VPBUFSIZE);
	~VBuffer();
public:
	void GetChar(char& ch);
	void GetShort(short& num);
	void GetInteger(int& num);
	void GetStrirng(char* pBuffer);
	void GetStream(char* pBuffer, short len);
	// ������� ���� ���ۿ��� ������ ���� �ش��ϴ� ���� �о�� ���ڷ� �־��� ������ �־��ش�.
	// ���� �а��� �����Ǿ� �ִ� ������ �����Ͱ� �ڵ����� ���� ������ ���� ũ�⸸ŭ ����.
public:
	void SetInteger(int num);
	void SetShort(short num);
	void SetChar(char ch);
	void SetString(char* pBuffer);
	void SetStream(char* pBuffer, short len);
	// ������� ���ڷ� �Ѱ��� ���� ���� ������ ���ο� �����ϴ� ������ �ϴ� �Լ�.
	// ���� �����ϰ� ���� ���� ������ �����Ͱ� ������ ���� ������ �� ũ�⸸ŭ �����Ѵ�.
	void SetBuffer(char* pVBuffer);


public:
	inline int		GetMaxBufSize() { return m_maxBufSize; }
	inline int		GetCurBufSize() { return m_curBufSize; }
	inline char*	GetCurMark()	{ return m_pCurMark; }
	inline char*	GetBeginMark() { return m_pVBuffer; }
public:
	// CopyBuffer�� VBufferŬ������ ���� ���۸� ���ڷ� ���� ���� ���ۿ� �����Ѵ�.
	// �̰��� ���� ���ۿ� ���� ������ ���� ��� �� ������ �Ŀ� ���� ���۸� ����� ���۷� �ű� �� ����ϰ�
	// �����ϱ� ���� �ʱ�ȭ �� ����ξ��� ��Ŷ ���̸� �������ش�.
	bool CopyBuffer(char* pDestBuffer);
	void Init();

private:
	char* m_pVBuffer; // ��������
	char* m_pCurMark; // ���� ���� ��ġ.
	int   m_maxBufSize; // �ִ� ���� ������
	int   m_curBufSize; // ���� ���� ���� ������

private: // ���� ����.
	VBuffer(const VBuffer& rhs) = delete;
	VBuffer& operator=(const VBuffer& rhs) = delete;
};

CREATE_FUNCTION(VBuffer, GetVBuffer);