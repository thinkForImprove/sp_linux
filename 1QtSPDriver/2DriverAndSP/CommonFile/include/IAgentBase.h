#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"

//////////////////////////////////////////////////////////////////////////
#if defined(AGENTXFS_LIBRARY)
#define IAGENTBASE_EXPORT Q_DECL_IMPORT
#else
#define IAGENTBASE_EXPORT Q_DECL_EXPORT
#endif
//////////////////////////////////////////////////////////////////////////
// 实现命令数据拷贝
struct IAgentBase
{
    virtual void Release() = 0;
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData) = 0;
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData) = 0;
};

extern "C" IAGENTBASE_EXPORT long CreateIAgentBase(IAgentBase *&p);
//////////////////////////////////////////////////////////////////////////


