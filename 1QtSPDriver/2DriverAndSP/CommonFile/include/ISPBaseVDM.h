#pragma once
#include "XFSVDM.H"
#include "ISPBaseClass.h"

//////////////////////////////////////////////////////////////////////////
#if defined(SPBASEVDM_LIBRARY)
#define SPBASEVDM_EXPORT Q_DECL_EXPORT
#else
#define SPBASEVDM_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
// 命令执行和结果数据返回
struct ICmdFunc
{
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName) = 0;
    virtual HRESULT OnClose() = 0;
    virtual HRESULT OnStatus() = 0;
    virtual HRESULT OnCancelAsyncRequest() = 0;
    virtual HRESULT OnUpdateDevPDL() = 0;

    // VDM类型接口
    virtual HRESULT GetStatus(LPWFSVDMSTATUS &lpStatus) = 0;
    virtual HRESULT GetCapabilities(LPWFSVDMCAPS &lpCaps) = 0;
    virtual HRESULT EnterModeREQ(DWORD dwTimeout) = 0;
    virtual HRESULT EnterModeACK() = 0;
    virtual HRESULT ExitModeREQ(DWORD dwTimeout) = 0;
    virtual HRESULT ExitModeACK() = 0;
};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseVDM
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

extern "C" SPBASEVDM_EXPORT long CreateISPBaseVDM(LPCSTR lpDevType, ISPBaseVDM *&p);
//////////////////////////////////////////////////////////////////////////
