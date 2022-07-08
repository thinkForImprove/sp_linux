#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "XFSAPI.H"

//////////////////////////////////////////////////////////////////////////
#if defined(AGENTXFS_LIBRARY)
#define IAGENTBASE_EXPORT Q_DECL_IMPORT
#else
#define IAGENTBASE_EXPORT Q_DECL_EXPORT
#endif
//////////////////////////////////////////////////////////////////////////

inline UINT GetLenOfSZZ(const char *lpszz)
{
    const char *p = lpszz;
    while (TRUE)
    {
        if (p == nullptr || (p + 1) == nullptr)
        {
            return 0;
        }
        if ((*p == 0x00) && (*(p + 1) == 0x00))
        {
            break;
        }
        p++;
    }
    return (UINT)((p - lpszz) + 2);
}

// 实现命令数据拷贝
struct IAgentBase
{
    virtual void Release() = 0;
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData) = 0;
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData) = 0;
    virtual HRESULT GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData) = 0;
    virtual HRESULT ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData) = 0;
    virtual HRESULT CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult) = 0;
};

extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p);
//////////////////////////////////////////////////////////////////////////


