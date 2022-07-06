#pragma once

#include "QtTypeDef.h"
//////////////////////////////////////////////////////////////////////////
class CMultiString
{
public:
    CMultiString();
    CMultiString(LPCSTR szzStr, BOOL bReadOnly = TRUE);
    CMultiString(const CMultiString &szz);
    const CMultiString &operator=(LPCSTR szzStr);
    const CMultiString &operator=(const CMultiString &szz);
    virtual ~CMultiString();
    long GetCount() const;
    LPCSTR GetAt(long index) const;
    long GetTotalLen() const;
    operator LPCSTR() const;
    BOOL Add(LPCSTR pStr);
    void Clear();
protected:
    void *m_pData;
};

class CMultiMultiString
{
public:
    CMultiMultiString();
    CMultiMultiString(LPCSTR szzzStr, BOOL bReadOnly = TRUE);
    CMultiMultiString(const CMultiMultiString &szz);
    const CMultiMultiString &operator=(LPCSTR szzStr);
    const CMultiMultiString &operator=(const CMultiMultiString &szz);
    virtual ~CMultiMultiString();
    long GetCount() const;
    LPCSTR GetAt(long index) const;
    long GetTotalLen() const;
    operator LPCSTR() const;
    BOOL Add(const CMultiString &ms);
protected:
    void *m_pData;
};
