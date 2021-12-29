// CriticalSectionSyncTest.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

HANDLE hMutex; // auto-reset
unsigned  __stdcall TestCriticalSection(void* test) 
{
    ::WaitForSingleObject(hMutex, INFINITE);
    for (int i = 0; i < 10'0000'0000; ++i)
    {
        
    }
    ::ReleaseMutex(hMutex);
    return 0;
}

int main()
{
    WSAData wsaData;
    ::WSAStartup(MAKEWORD(2, 2), &wsaData);

    hMutex = ::CreateMutex(NULL, FALSE, NULL);
    __int64 startTick = ::GetTickCount64();
    
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, TestCriticalSection, 0, 0, 0);

    ::WaitForSingleObject(hThread, INFINITE);
    
    __int64 endTick = ::GetTickCount64();
    std::cout << "걸린 ms : " << endTick - startTick << std::endl;
    ::CloseHandle(hMutex);
}
