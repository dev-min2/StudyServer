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

    if (m_pBeginMark == nullptr) // �޸� �Ҵ��� �ȵǾ�����
    {
        std::cout << "������ �޸� �Ҵ� ����." << std::endl;
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
        //�� ���� �����÷ο� üũ
        if (m_usedBufferSize + forwardLen > m_bufferSize)
            return nullptr;


        if ((m_pEndMark - m_pCurrentMark) >= forwardLen)
        {
            pPreCurrentMark = m_pCurrentMark;
            m_pCurrentMark += forwardLen;
        }
        else
        {
            //��ȯ�Ǳ� �� ������ ��ǥ�� ����
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
        // �� ���� �����÷� üũ
        if (m_usedBufferSize + forwardLen + nextLen > m_bufferSize)
            return nullptr;

        // �װ� �ƴ϶�� ����.
        if ((m_pEndMark - m_pCurrentMark) > (nextLen + forwardLen))
            m_pCurrentMark += forwardLen;
        else // �ٽ� ��ǥ�� ó������.
        {
            // ��ȯ�Ǳ��� ������ ��ǥ�� ����.
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
        // ���������� �� �о��ٸ� �� �о�帱 ������ �����ʹ� �� ������ �ű��.
        if (m_pLastMoveMark == m_pGettedBufferMark)
        {
            m_pGettedBufferMark = m_pBeginMark;
            m_pLastMoveMark = m_pEndMark;
        }

        // ���� ���ۿ� �ִ� size�� �о�帱 size���� ũ�ٸ�
        if (m_usedBufferSize > readSize)
        {
            // �� ������ ������ �Ǵ��Ѵ�.
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
            // �� ������ ������ �Ǵ�.
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
