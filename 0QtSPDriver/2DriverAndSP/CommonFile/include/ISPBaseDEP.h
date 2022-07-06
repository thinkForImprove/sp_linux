#pragma once
#include "ISPBaseClass.h"
#include "XFSDEP.H"
//////////////////////////////////////////////////////////////////////////
#if defined(SPBASEDEP_LIBRARY)
#define SPBASEDEP_EXPORT Q_DECL_EXPORT
#else
#define SPBASEDEP_EXPORT Q_DECL_IMPORT
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
    virtual HRESULT GetStatus(LPWFSDEPSTATUS &lpStatus) = 0;
    virtual HRESULT GetCapabilities(LPWFSDEPCAPS &lpCaps) = 0;
    // 执行命令
    virtual HRESULT Entry(LPWFSDEPENVELOPE lpWfsDepEnvelope) = 0;
    virtual HRESULT Dispense() = 0;
    virtual HRESULT Retract(LPWFSDEPENVELOPE lpWfsDepEnvelope) = 0;
    virtual HRESULT ResetCount() = 0;
    virtual HRESULT OpenShutter() = 0;
    virtual HRESULT CloseShutter() = 0;
    virtual HRESULT Reset(LPDWORD lpDword) = 0;
    virtual HRESULT SetGuidAnceLight(LPWFSDEPSETGUIDLIGHT lpWfsDEPSetGuidLight) = 0;
    virtual HRESULT SupplyReplenish(LPWFSDEPSUPPLYREPLEN lpWfsDepSupplyReplen) = 0;
    virtual HRESULT PowerSaveControl(LPWFSDEPPOWERSAVECONTROL lpWfsDEPPowerSaveControl) = 0;
};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseDEP
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
    //拿走事件
    virtual bool FireEnvTaken() = 0;
    //发出存入事件
    virtual bool FireDeposited() = 0;
    //发出位置错误事件
    virtual bool FirePositError(LPLONG lplError) = 0;
    //发出存入阈值事件
    virtual bool FireDepThreshold(DWORD status) = 0;
    //发出Toner阈值事件
    virtual bool FireTonerThreshold(DWORD status) = 0;
    //发出Env阈值事件
    virtual bool FireEnvThreshold(DWORD status) = 0;
    //发出箱子重入事件
    virtual bool FireContInserted() = 0;
    //发出箱子移除事件
    virtual bool FireContRemove() = 0;
    //发出Env插入事件
    virtual bool FireEvnInserted() = 0;
    //发出检测到插入事件
    virtual bool FireMediaDetected(WORD wDispenseMedia, WORD wDepositMedia) = 0;
    //发出插入事件
    virtual bool FireInsertDeposit() = 0;
};
extern "C" SPBASEDEP_EXPORT long CreateISPBaseDEP(LPCSTR lpDevType, ISPBaseDEP *&p);
//////////////////////////////////////////////////////////////////////////
