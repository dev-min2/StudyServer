#pragma once
#include "std.h"
#define MAX_SOCKBUF 1024 // ��Ŷ(����� ����)ũ��
#define MAX_CLIENT 100 // �ִ� ���Ӱ����� Ŭ���̾�Ʈ ��
#define MAX_WORKERTHREAD 4 // ������ Ǯ(CP��ü)�� ���� ������ ��

enum class IO_TYPE
{
	IO_RECV,
	IO_SEND
};

// Overlapped ����ü�� Ȯ���Ͽ� ���.
struct OverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;
	int			  m_index;
	WSABUF		  m_wsaBuf;
	char		  m_dataBuffer[MAX_SOCKBUF];
	IO_TYPE		  m_ioType;
};

// Ŭ���̾�Ʈ(Session) ������ ������� ����ü
struct ClientInfo
{
	ClientInfo() : m_socketClient(INVALID_SOCKET)
	{
		::ZeroMemory(&m_recvOverlapped, sizeof(OverlappedEx));
		::ZeroMemory(&m_sendOverlapped, sizeof(OverlappedEx));
	}

	SOCKET			m_socketClient; 
	OverlappedEx	m_recvOverlapped; // Recv Overlapped(�񵿱�) I/O �۾��� ���� ����
	OverlappedEx	m_sendOverlapped; // Send Overlapped(�񵿱�) I/O �۾��� ���� ����
};

class IOCompletionPort
{
	friend unsigned int WINAPI CallWorkerThread(LPVOID p);
	friend unsigned int WINAPI CallAccepterThread(LPVOID p);
public:
	IOCompletionPort();
	virtual ~IOCompletionPort();

public:
	//������ �ʱ�ȭ�ϴ� �Լ�
	bool InitSocket();
	//������ ������ ���� ��Ų��.
	void CloseSocket(ClientInfo* pClientInfo, bool bIsForce = false);

	//������ �ּ������� ���ϰ� �����Ű�� ���� ��û�� �ޱ� ���� �� ������ ����ϴ� �Լ�
	bool BindandListen(int nBindPort);
	//���� ��û�� �����ϰ� �޼����� �޾Ƽ� ó���ϴ� �Լ�
	bool StartServer();

	//�����Ǿ��ִ� �����带 �ı��Ѵ�.
	void DestroyThread();
private:
	//Overlapped I/O�۾��� ���� �����带 ����
	bool CreateWokerThread();
	//accept��û�� ó���ϴ� ������ ����
	bool CreateAccepterThread();

	//������ ���� ClientInfo�� ��ȯ
	ClientInfo* GetEmptyClientInfo();

	// CP��ü�� Completion Key(�Ϸ�Ű)�� ����ϴ� �Լ�.
	bool BindIOCompletionPort(ClientInfo* pClientInfo);

	//WSARecv Overlapped I/O �۾��� ��Ų��.
	bool BindRecv(ClientInfo* pClientInfo);

	//WSASend Overlapped I/O�۾��� ��Ų��.
	bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen);

	bool IOProcess(ClientInfo* pClient,OverlappedEx* pOverlapped, DWORD& transferred);

	//Overlapped I/O�۾��� ���� �Ϸ� �뺸�� �޾� 
	//�׿� �ش��ϴ� ó���� �ϴ� �Լ�
	void WokerThread();
	//������� ������ �޴� ������
	void AccepterThread();



private:
	//Ŭ���̾�Ʈ ���� ����ü
	ClientInfo* m_pClientInfo;
	//��������
	SOCKET		m_hSocketListen;
	// ����� Ŭ���̾�Ʈ ��
	int			m_clientCount;
	// ��Ŀ������ �ڵ� �迭
	HANDLE		m_hWorkerThread[MAX_WORKERTHREAD];
	// ���� ������ �ڵ�
	HANDLE		m_hAccepterThread;
	// CP��ü
	HANDLE		m_hCompletionPort;
	// �۾� ������ BOOL
	bool		m_bWorkerRun;
	// ���� ������ BOOL
	bool		m_bAccepterRun;
	// ���� ����
	char		m_socketBuff[MAX_SOCKBUF];

};

