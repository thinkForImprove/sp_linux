#include "MultiString.h"
#include <assert.h>
#include <string.h>
#include <vector>
using namespace std;
//每个需要跟踪new分配是否正确delete的CPP文件加入以下几行
//#include "DumpAllocMem.h"

/*#if defined(DEBUG) || defined(_DEBUG)
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif*/

//宏定义
#define DELETEMULTISTRING()     DeleteMultiString(&m_pData);

//类型定义
struct MULTISTRING  //CMultiString的m_pData的实际数据类型
{
    long nRefCount;
    long nLen;
    vector<char *> ps;
    BOOL bReadOnly;
    char *pData;
    long nBufLen;

    MULTISTRING()
    {
        nRefCount = 1;
        nLen = 0;
        bReadOnly = 0;
        pData = NULL;
        nBufLen = 0;
    }
};

typedef MULTISTRING MULTIMULTISTRING;
//------------------------全局变量---------------------
const char conNullMMS[3] = {0, 0, 0};

//------------------------函数模型---------------------
static inline DWORD MinAllocSize(DWORD l);
static void DeleteMultiString(void **ppData);
static void ReAssignData(MULTISTRING *pData, int nNewLen);

//------------------------局部函数---------------------
//功能：计算最小分配尺寸，2的N次方
//输入：l, 要求的尺寸
//返回：应该分配的尺寸
static inline DWORD MinAllocSize(DWORD l)
{
    DWORD val = 1;
    while (val < l)
        val <<= 1;
    return val;
}

//功能：删除CMultiString的m_pData。
//输入：ppData, 指向CMultiString的m_pData的指针，返回时*ppData被置为NULL
//返回：无。
static void DeleteMultiString(void **ppData)
{
    if (*ppData)
    {
        MULTISTRING *pData = (MULTISTRING *)*ppData;
        pData->nRefCount--;
        if (pData->nRefCount == 0)
        {
            if (!pData->bReadOnly && pData->pData)
            {
                delete [] pData->pData;
            }
            pData->ps.clear();
            delete pData;
        }
        *ppData = nullptr;
    }
}

//功能：根据nNewLen重新分配CMultiString的m_pData的pData
//输入：pData, 即CMultiString的m_pData
//      nNewLen, 要求的新长度
//返回：无。
static void ReAssignData(MULTISTRING *pData, int nNewLen)
{
    assert(!pData->bReadOnly);
    if (pData->nBufLen >= nNewLen)
        return;

    //分配并拷贝老的数据
    DWORD dwAllocSize = MinAllocSize(nNewLen);
    char *pBuf = new char[dwAllocSize];
    if (pData->pData && pData->nLen) //如果存在老的数据，拷贝之
        memcpy(pBuf, pData->pData, pData->nLen);

    //修改列表中指针指向新的缓冲区
    vector<char *>::iterator it;
    for (it = pData->ps.begin(); it != pData->ps.end(); it++)
    {
        *it = *it - pData->pData + pBuf;
    }

    //删除老的缓冲区，pData指向新的缓冲区
    if (pData->pData)
        delete [] pData->pData;
    pData->pData = pBuf;
    pData->nBufLen = dwAllocSize;
}

//------------------------类CMultString的实现---------------------
CMultiString::CMultiString()
{
    m_pData = nullptr;
}

CMultiString::CMultiString(LPCSTR szzStr, BOOL bReadOnly)
{
    m_pData = nullptr;
    if (!szzStr)
        return;
    MULTISTRING *pData = new MULTISTRING;
    m_pData = pData;
    pData->bReadOnly = bReadOnly;
    LPCSTR p = szzStr;
    if (bReadOnly)
        pData->pData = (char *)szzStr;
    else
    {
        //计算长度
        while (p[0] || p[1])
            p++;
        int DataLen = p + 2 - szzStr;
        ReAssignData(pData, DataLen);
        memcpy(pData->pData, szzStr, DataLen);
        p = pData->pData;
    }

    while (1)
    {
        int nLen = strlen(p);
        pData->nLen += nLen + 1;
        pData->ps.push_back((char *)p);
        p += nLen + 1;
        if (!p[0])
        {
            pData->nLen++;
            break;
        }
    }
}

CMultiString::CMultiString(const CMultiString &szz)
{
    m_pData = szz.m_pData;
    if (m_pData)
        ((MULTISTRING *)m_pData)->nRefCount++;
}

CMultiString::~CMultiString()
{
    DeleteMultiString(&m_pData);
}

const CMultiString &CMultiString::operator=(LPCSTR szzStr)
{
    DeleteMultiString(&m_pData);
    *this = CMultiString(szzStr, FALSE);
    return *this;
}

const CMultiString &CMultiString::operator=(const CMultiString &szz)
{
    DeleteMultiString(&m_pData);
    m_pData = szz.m_pData;
    if (m_pData)
        ((MULTISTRING *)m_pData)->nRefCount++;
    return *this;
}

long CMultiString::GetCount() const
{
    return m_pData ? ((MULTISTRING *)m_pData)->ps.size() : 0;
}

long CMultiString::GetTotalLen() const
{
    return m_pData ? ((MULTISTRING *)m_pData)->nLen : 0;
}

CMultiString::operator LPCSTR() const
{
    return m_pData ? ((MULTISTRING *)m_pData)->pData : conNullMMS;
}

LPCSTR CMultiString::GetAt(long index) const
{
    if (!m_pData || index < 0 || index >= GetCount())
        return nullptr;
    return ((MULTISTRING *)m_pData)->ps[index];
}

//功能：加新的字串到现在的多字串后面
//输入：pStr, 要加的字串
//返回：TRUE，成功；否则失败。
BOOL CMultiString::Add(LPCSTR pStr)
{
    //如果m_pData不存在，分配之
    MULTISTRING *pData = (MULTISTRING *)m_pData;
    if (!pData)
    {
        m_pData = pData = new MULTISTRING;
        pData->bReadOnly = false;
    }

    if (pData->bReadOnly)   //只读不可写
        return false;

    //计算拷贝的偏移和新的长度
    int nOffset = pData->nLen;
    int nNewLen = 0;
    int nLen = strlen(pStr) + 1;
    if (pData->ps.size() > 0)
    {
        nOffset--;
        nNewLen = pData->nLen + nLen;
    }
    else
    {
        nNewLen = nLen + 1;
    }

    //重新分配内存并拷贝数据
    ReAssignData(pData, nNewLen);
    pData->ps.push_back(pData->pData + nOffset);
    pData->nLen = nNewLen;
    memcpy(pData->pData + nOffset, pStr, nLen);
    pData->pData[nNewLen - 1] = 0;
    return TRUE;
}

//------------------------类CMultiMultiString的实现---------------------
CMultiMultiString::CMultiMultiString()
{
    m_pData = nullptr;
}

CMultiMultiString::CMultiMultiString(LPCSTR szzzStr, BOOL bReadOnly)
{
    m_pData = nullptr;
    if (!szzzStr)
        return;
    MULTIMULTISTRING *pData = new MULTIMULTISTRING;
    m_pData = pData;
    pData->bReadOnly = bReadOnly;
    LPCSTR p = szzzStr;
    if (bReadOnly)
        pData->pData = (char *)szzzStr;
    else
    {
        //计算长度
        while (p[0] || p[1] || p[2])
            p++;
        int DataLen = p + 3 - szzzStr;
        ReAssignData(pData, DataLen);
        memcpy(pData->pData, szzzStr, DataLen);
        p = pData->pData;
    }

    while (1)
    {
        CMultiString ms(p, true);
        int nLen = ms.GetTotalLen();
        pData->nLen += nLen;
        pData->ps.push_back((char *)p);
        p += nLen;
        if (!p[0])
        {
            pData->nLen++;
            break;
        }
    }
}

CMultiMultiString::CMultiMultiString(const CMultiMultiString &szzz)
{
    m_pData = szzz.m_pData;
    if (m_pData)
        ((MULTIMULTISTRING *)m_pData)->nRefCount++;
}

const CMultiMultiString &CMultiMultiString::operator=(LPCSTR szzzStr)
{
    DELETEMULTISTRING()
    *this = CMultiMultiString(szzzStr);
    return *this;
}

const CMultiMultiString &CMultiMultiString::operator=(const CMultiMultiString &szzz)
{
    DELETEMULTISTRING()
    m_pData = szzz.m_pData;
    if (m_pData)
        ((MULTIMULTISTRING *)m_pData)->nRefCount++;

    return *this;
}

CMultiMultiString::~CMultiMultiString()
{
    DELETEMULTISTRING()
}

long CMultiMultiString::GetCount() const
{
    return m_pData ? ((MULTIMULTISTRING *)m_pData)->ps.size() : 0;
}

LPCSTR CMultiMultiString::GetAt(long index) const
{
    if (!m_pData || index < 0 || index >= GetCount())
        return nullptr;
    return ((MULTIMULTISTRING *)m_pData)->ps[index];
}

long CMultiMultiString::GetTotalLen() const
{
    return m_pData ? ((MULTIMULTISTRING *)m_pData)->nLen : 0;
}

CMultiMultiString::operator LPCSTR() const
{
    return m_pData ? ((MULTIMULTISTRING *)m_pData)->pData : conNullMMS;
}

BOOL CMultiMultiString::Add(const CMultiString &ms)
{
    //如果m_pData不存在，分配之
    MULTIMULTISTRING *pData = (MULTIMULTISTRING *)m_pData;
    if (!pData)
    {
        m_pData = pData = new MULTIMULTISTRING;
        pData->bReadOnly = false;
    }

    if (pData->bReadOnly)   //只读不可写
        return false;

    //计算拷贝的偏移和新的长度
    int nOffset = pData->nLen;
    int nNewLen = 0;
    int nLen = ms.GetTotalLen();
    if (pData->ps.size() > 0)
    {
        nOffset--;
        nNewLen = pData->nLen + nLen;
    }
    else
    {
        nNewLen = nLen + 1;
    }

    //重新分配内存并拷贝数据
    ReAssignData(pData, nNewLen);
    pData->ps.push_back(pData->pData + nOffset);
    pData->nLen = nNewLen;
    memcpy(pData->pData + nOffset, (LPCSTR)ms, nLen);
    pData->pData[nNewLen - 1] = 0;
    return TRUE;
}

