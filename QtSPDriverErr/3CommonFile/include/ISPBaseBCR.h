#pragma once
#include <QtCore/qglobal.h>
#include "ISPBaseClass.h"
#include "XFSBCR.H"

//////////////////////////////////////////////////////////////////////////
#if defined(SPBASEBCR_LIBRARY)
#define SPBASEBCR_EXPORT Q_DECL_EXPORT
#else
#define SPBASEBCR_EXPORT Q_DECL_IMPORT
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

    // BCR类型接口
    virtual HRESULT GetStatus(LPWFSBCRSTATUS &lpstStatus){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT GetCapabilities(LPWFSBCRCAPS &lpstCaps){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT ReadBCR(const WFSBCRREADINPUT &stReadInput, LPWFSBCRREADOUTPUT *&lppReadOutput,
                            DWORD dwTimeOut){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT Reset(){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT SetGuidLight(const WFSBCRSETGUIDLIGHT &stLight){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT PowerSaveControl(const WFSBCRPOWERSAVECONTROL &stPowerCtrl){return WFS_ERR_UNSUPP_COMMAND;}
};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseBCR
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
    // 发送二维码位置事件
    virtual bool FireBarCodePosition(WORD wStatus) = 0;
};

//////////////////////////////////////////////////////////////////////////
extern "C" SPBASEBCR_EXPORT long CreateISPBaseBCR(LPCSTR lpDevType, ISPBaseBCR *&p);
//////////////////////////////////////////////////////////////////////////
