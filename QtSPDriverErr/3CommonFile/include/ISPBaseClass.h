#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "SimpleMutex.h"
#include "XfsRegValue.h"
#include "XfsSPIHelper.h"
#include "ExtraInforHelper.h"
//////////////////////////////////////////////////////////////////////////
#if defined(ISPBASECLASS_LIBRARY)
#define SPBASE_EXPORT     Q_DECL_EXPORT
#else
#define SPBASE_EXPORT     Q_DECL_IMPORT
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
//////////////////////////////////////////////////////////////////////////
typedef struct tag_sp_base_data
{
    CSimpleMutex    *pMutex;                // 状态更新互斥类
    char            szLogicalName[256];     // 打开的逻辑名
    HSERVICE        hService;               //当前服务句柄
    char            szAppID[256];           //当前应用ID
} SPBASEDATA;
//////////////////////////////////////////////////////////////////////////
// 命令执行和结果数据返回
struct ICmdRun
{
    virtual HRESULT OnOpen(LPCSTR lpLogicalName) = 0;
    virtual HRESULT OnClose() = 0;
    virtual HRESULT OnStatus() = 0;
    virtual HRESULT OnWaitTaken() = 0;
    virtual HRESULT OnCancelAsyncRequest() = 0;
    virtual HRESULT OnUpdateDevPDL() = 0;

    virtual HRESULT FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult) = 0;
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult) = 0;
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult) = 0;

    //其他接口
    virtual HRESULT OnClearCancelSemphoreCount(){           //30-00-00-00(FT#0070)
        return WFS_SUCCESS;                                 //30-00-00-00(FT#0070)
    }                                                       //30-00-00-00(FT#0070)
};
//////////////////////////////////////////////////////////////////////////
struct  ISPBaseClass
{
    // 释放接口
    virtual void Release() = 0;
    // 注册回调接口
    virtual void RegisterICmdRun(ICmdRun *pResult) = 0;
    // 开始运行
    virtual bool StartRun() = 0;
    // 获取SPBase数据
    virtual void GetSPBaseData(SPBASEDATA &stData) = 0;
    // 发送事件
    virtual bool FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData) = 0;
    // 打包状态改变事件数据，此是通用的
    virtual HRESULT Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData) = 0;
    // 打包故障状态事件数据，此是通用的
    virtual HRESULT Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData) = 0;
    // 打包扩展状态数据，此是通用的
    virtual HRESULT Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra) = 0;
};

//////////////////////////////////////////////////////////////////////////
extern "C" SPBASE_EXPORT long CreateISPBaseClass(LPCSTR lpDevType, ISPBaseClass *&p);
//////////////////////////////////////////////////////////////////////////
