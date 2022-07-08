#pragma once
#include "ISPBaseClass.h"
#include "XFSIDC.H"

//////////////////////////////////////////////////////////////////////////
#if defined(SPBASEMSR_LIBRARY)
    #define SPBASEMSR_EXPORT Q_DECL_EXPORT
#else
    #define SPBASEMSR_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
// 命令执行和结果数据返回
struct ICmdFunc
{
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName) = 0;
    virtual HRESULT OnClose() = 0;
    virtual HRESULT OnStatus() = 0;
    virtual HRESULT OnWaitTaken() = 0;
    virtual HRESULT OnCancelAsyncRequest() = 0;
    virtual HRESULT OnUpdateDevPDL() = 0;

    // IDC类型接口
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus) = 0;
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps) = 0;
    // 执行命令
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut) = 0;
    virtual HRESULT Reset(LPWORD lpResetIn) = 0;

};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseMSR
{
    // 释放接口
    virtual void Release() = 0;
    // 注册回调接口
    virtual void RegisterICmdFunc(ICmdFunc *pCmdFunc) = 0;
    // 开始运行
    virtual bool StartRun() = 0;
    // 获取SPBase数据
    virtual void GetSPBaseData(SPBASEDATA &stData) = 0;
    // 发送事件
    virtual bool FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData) = 0;
    // 发送状态改变事件
    virtual bool FireStatusChanged(DWORD dwStatus) = 0;
    // 发送故障状态事件
    virtual bool FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription = nullptr) = 0;
};

extern "C" SPBASEMSR_EXPORT long CreateISPBaseMSR(LPCSTR lpDevType, ISPBaseMSR *&p);
//////////////////////////////////////////////////////////////////////////
