#include "StringBuffer.h"
#include <QDebug>


//////////////////////////////////////////////////////////////////////
//              CStringBuffer
//////////////////////////////////////////////////////////////////////
CStringBuffer::CStringBuffer(DWORD dwPreallocLen,
                             const char *pTabStr, const char *pLFStr)
{
    assert(dwPreallocLen > 0);
    assert(pTabStr != NULL);
    assert(pLFStr != NULL);
    assert(strlen(pTabStr) < sizeof(m_szTabStr));
    assert(strlen(pLFStr) < sizeof(m_szLFStr));

    m_dwDataLen = 0;
    m_dwBufLen = dwPreallocLen;
    m_pData = new char[dwPreallocLen];
    m_pData[0] = 0;
    strcpy(m_szTabStr, pTabStr);
    strcpy(m_szLFStr, pLFStr);
}

CStringBuffer::~CStringBuffer()
{
    Clear();
}

void CStringBuffer::Clear()
{
    if (m_pData != NULL)
    {
        delete [] m_pData;
        m_pData = NULL;
    }
    m_dwBufLen = 0;
    m_dwDataLen = 0;
}

CStringBuffer &CStringBuffer::ClearContent()
{
    m_dwDataLen = 0;
    return *this;
}

CStringBuffer &CStringBuffer::AddTab(int iTabNum)
{
    for (int i = 0; i < iTabNum; i++)
    {
        Add(m_szTabStr);
    }
    return *this;
}

CStringBuffer &CStringBuffer::EndLine()
{
    Add(m_szLFStr);
    return *this;
}

void CStringBuffer::EnsureLeftLen(DWORD dwLen)
{
    if (m_dwDataLen + dwLen >= m_dwBufLen)
    {
        while (m_dwBufLen <= m_dwDataLen + dwLen)
        {
            m_dwBufLen *= 2;
        }
        char *pNewBuf = new char[m_dwBufLen];
        if (m_dwDataLen > 0)
        {
            memcpy(pNewBuf, m_pData, m_dwDataLen);
        }
        pNewBuf[m_dwDataLen] = 0;
        delete m_pData;
        m_pData = pNewBuf;
    }
}

CStringBuffer &CStringBuffer::Add(const char *pStr)
{
    DWORD dwLen = strlen(pStr);
    EnsureLeftLen(dwLen);
    memcpy(m_pData + m_dwDataLen, pStr, dwLen);
    m_dwDataLen += dwLen;
    m_pData[m_dwDataLen] = 0;
    return *this;
}

CStringBuffer &CStringBuffer::AddF(const char *pFormat, ...)
{
    va_list vl;
    va_start(vl, pFormat);
    QString strTemp;
    strTemp.vsprintf(pFormat, vl);
    va_end(vl);
    int nLen = strTemp.length();
    EnsureLeftLen(nLen);
    va_list vlist;
    va_start(vlist, pFormat);
    int nRet = vsprintf(m_pData + m_dwDataLen, pFormat, vlist);
    va_end(vlist);
    if (nRet > 0)
    {
        m_dwDataLen += (DWORD)nRet;
    }
    return *this;
}



