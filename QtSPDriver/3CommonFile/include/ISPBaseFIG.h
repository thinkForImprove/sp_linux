#pragma once
#include "ISPBaseClass.h"
#include "XFSPTR.H"

//////////////////////////////////////////////////////////////////////////
#if defined(SPBASEFIG_LIBRARY)
    #define SPBASEFIG_EXPORT Q_DECL_EXPORT
#else
    #define SPBASEFIG_EXPORT Q_DECL_IMPORT
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

    // FIG类型接口
    // 查询命令
    virtual HRESULT GetStatus(LPWFSPTRSTATUS &lpStatus) = 0;
    virtual HRESULT GetCapabilities(LPWFSPTRCAPS &lpCaps) = 0;
    // 执行命令
    virtual HRESULT ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut) = 0;
    virtual HRESULT Reset(const LPWFSPTRRESET lpReset) = 0;

};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseFIG
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

extern "C" SPBASEFIG_EXPORT long CreateISPBaseFIG(LPCSTR lpDevType, ISPBaseFIG *&p);
//////////////////////////////////////////////////////////////////////////
