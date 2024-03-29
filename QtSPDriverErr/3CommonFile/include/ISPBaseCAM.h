﻿#pragma once
#include "ISPBaseClass.h"
#include "XFSCAM.H"

//////////////////////////////////////////////////////////////////////////
#if defined(SPBASECAM_LIBRARY)
    #define SPBASECAM_EXPORT Q_DECL_EXPORT
#else
    #define SPBASECAM_EXPORT Q_DECL_IMPORT
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

    // CAM接口
    virtual HRESULT GetStatus(LPWFSCAMSTATUS &lpStatus){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT GetCapabilities(LPWFSCAMCAPS &lpCaps){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT TakePicture(const WFSCAMTAKEPICT &stTakePict,
                                DWORD dwTimeout){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT TakePictureEx(const WFSCAMTAKEPICTEX &stTakePict,
                                  DWORD dwTimeout){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT Display(const WFSCAMDISP &stTakePict,
                            DWORD dwTimeout){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT DisplayEx(const WFSCAMDISPEX &stDisplayEx,
                              DWORD dwTimeout){return WFS_ERR_UNSUPP_COMMAND;}
#ifdef ICBC_VERSION
    virtual HRESULT Record(const WFSCAMRECORD &stRecord){return WFS_ERR_UNSUPP_COMMAND;}
#endif
    virtual HRESULT GetSignature(const WFSCAMGETSIGNATURE &stGetSig, WFSCAMSIGNDATA &stSignData,
                                 DWORD dwTimeout){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT Reset(){return WFS_ERR_UNSUPP_COMMAND;}
};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseCAM
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

extern "C" SPBASECAM_EXPORT long CreateISPBaseCAM(LPCSTR lpDevType, ISPBaseCAM *&p);
//////////////////////////////////////////////////////////////////////////
