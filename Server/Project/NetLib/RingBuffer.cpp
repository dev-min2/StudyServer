#include "RingBuffer.h"

RingBuffer::RingBuffer() : m_pRingBuffer(nullptr),m_pBeginMark(nullptr),m_pEndMark(nullptr),m_pCurrentMark(nullptr),
m_pGettedBufferMark(nullptr),m_pLastMoveMark(nullptr),m_usedBufferSize(0),m_allUsedBufSize(0)
{
    
}

RingBuffer::~RingBuffer()
{
    if (m_pBeginMark != nullptr)
        delete[] m_pBeginMark;;
}


bool RingBuffer::Initialize()
{
    Monitor::Owner lock(m_csRingBuffer); // Lock
    {
        m_usedBufferSize = 0;
        m_pCurrentMark = m_pBeginMark;
        m_pGettedBufferMark = m_pBeginMark;
        m_pLastMoveMark = m_pEndMark;
        m_allUsedBufSize = 0;
    }
    return true;
}

bool RingBuffer::Create(int bufferSIze)
{
    if (m_pBeginMark != nullptr)
        delete[] m_pBeginMark;

    m_pBeginMark = new char[bufferSIze];

    if (m_pBeginMark == nullptr) // 메모리 할당이 안되었으면
    {
        std::cout << "링버퍼 메모리 할당 실패." << std::endl;
        return false;
    }

    m_pEndMark = m_pBeginMark + bufferSIze - 1;
    return true;
}


char* RingBuffer::ForwardMark(int forwardLen)
{
    char* pPreCurrentMark = nullptr;
    Monitor::Owner lock(m_csRingBuffer);
    {
        //링 버퍼 오버플로우 체크
        if (m_usedBufferSize + forwardLen > m_bufferSize)
            return nullptr;


        if ((m_pEndMark - m_pCurrentMark) >= forwardLen)
        {
            pPreCurrentMark = m_pCurrentMark;
            m_pCurrentMark += forwardLen;
        }
        else
        {
            //순환되기 전 마지막 좌표를 저장
            m_pLastMoveMark = m_pCurrentMark;
            m_pCurrentMark = m_pBeginMark + forwardLen;
            pPreCurrentMark = m_pBeginMark;
        }
        m_usedBufferSize += forwardLen;
        m_allUsedBufSize += forwardLen;
    }
    return pPreCurrentMark;
}

char* RingBuffer::ForwardMark(int forwardLen, int nextLen, DWORD remainLen)
{
    Monitor::Owner lock(m_csRingBuffer);
    {
        // 링 버퍼 오버플로 체크
        if (m_usedBufferSize + forwardLen + nextLen > m_bufferSize)
            return nullptr;

        // 그게 아니라면 전진.
        if ((m_pEndMark - m_pCurrentMark) > (nextLen + forwardLen))
            m_pCurrentMark += forwardLen;
        else // 다시 좌표를 처음으로.
        {
            // 순환되기전 마지막 좌표를 저장.
            m_pLastMoveMark = m_pCurrentMark;
            ::CopyMemory(m_pBeginMark, m_pCurrentMark - (remainLen - forwardLen), remainLen);
            m_pCurrentMark = m_pBeginMark + remainLen;
        }
        m_usedBufferSize += forwardLen;
        m_allUsedBufSize += forwardLen;
    }
    return m_pCurrentMark;
}

void RingBuffer::ReleaseBuffer(int releaseSize)
{
    Monitor::Owner lock(m_csRingBuffer);
    {
        m_usedBufferSize -= releaseSize;
    }
}

char* RingBuffer::GetBuffer(int readSize, int* pReadSize)
{
    char* pRet = nullptr;
    Monitor::Owner lock(m_csRingBuffer);
    {
        // 마지막까지 다 읽었다면 그 읽어드릴 버퍼의 포인터는 맨 앞으로 옮긴다.
        if (m_pLastMoveMark == m_pGettedBufferMark)
        {
            m_pGettedBufferMark = m_pBeginMark;
            m_pLastMoveMark = m_pEndMark;
        }

        // 현재 버퍼에 있는 size가 읽어드릴 size보다 크다면
        if (m_usedBufferSize > readSize)
        {
            // 링 버퍼의 끝인지 판단한다.
            if ((m_pLastMoveMark - m_pGettedBufferMark) >= readSize)
            {
                *pReadSize = readSize;
                pRet = m_pGettedBufferMark;
                m_pGettedBufferMark += readSize;
            }
            else
            {
                *pReadSize = (int)(m_pLastMoveMark - m_pGettedBufferMark);
                pRet = m_pGettedBufferMark;
                m_pGettedBufferMark += *pReadSize;
            }
        }
        else if (m_usedBufferSize > 0)
        {
            // 링 버퍼의 끝인지 판단.
            if ((m_pLastMoveMark - m_pGettedBufferMark) >= m_usedBufferSize)
            {
                *pReadSize = m_usedBufferSize;
                pRet = m_pGettedBufferMark;
                m_pGettedBufferMark += m_usedBufferSize;
            }
            else
            {
                *pReadSize = (int)(m_pLastMoveMark - m_pGettedBufferMark);
                pRet = m_pGettedBufferMark;
                m_pGettedBufferMark += *pReadSize;
            }
        }
    }
    return pRet;
}
