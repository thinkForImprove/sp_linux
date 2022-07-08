#pragma once
#include "ISPBaseClass.h"
#include "XFSCAM.H"

//////////////////////////////////////////////////////////////////////////
#if defined(SPBASESIG_LIBRARY)
    #define SPBASESIG_EXPORT Q_DECL_EXPORT
#else
    #define SPBASESIG_EXPORT Q_DECL_IMPORT
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

    // cam类型接口
    virtual HRESULT GetStatus(LPWFSCAMSTATUS &lpStatus) = 0;
    virtual HRESULT GetCapabilities(LPWFSCAMCAPS &lpCaps) = 0;
    virtual HRESULT Display(const WFSCAMDISP &stDispEx, DWORD dwTimeout) = 0;
    virtual HRESULT GetSignature(const WFSCAMGETSIGNATURE &stGetSig, WFSCAMSIGNDATA &stSignData, DWORD dwTimeout) = 0;
    virtual HRESULT Reset() = 0;
};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseSIG
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
    virtual bool FireStatusChanged(DWORD wStatus) = 0;
    // 发送故障状态事件
    virtual bool FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription = nullptr) = 0;
};

extern "C" SPBASESIG_EXPORT long CreateISPBaseSIG(LPCSTR lpDevType, ISPBaseSIG *&p);
//////////////////////////////////////////////////////////////////////////
