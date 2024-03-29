﻿#pragma once
#include "ISPBaseIDC.h"

//////////////////////////////////////////////////////////////////////////
class CSPBaseIDC : public ISPBaseIDC, public CLogManage, public ICmdRun
{
public:
    CSPBaseIDC(LPCSTR lpLogType);
    virtual ~CSPBaseIDC();

public:
    // 释放接口
    virtual void Release();
    // 注册回调接口
    virtual void RegisterICmdFunc(ICmdFunc *pCmdFunc);
    // 开始运行
    virtual bool StartRun();
    // 获取SPBase数据
    virtual void GetSPBaseData(SPBASEDATA &stData);
    // 发送事件
    virtual bool FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData);
    // 发送状态改变事件
    virtual bool FireStatusChanged(DWORD wStatus);
    // 发送故障状态事件
    virtual bool FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription = nullptr);

protected:
    // ISPBaseClass实现
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();
    virtual HRESULT FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult);

    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, LPWFSRESULT &lpResult);

protected:
    // 格式并保存结果数据
    // 结果数据
#ifdef CARD_REJECT_GD_MODE
    HRESULT Fmt_WFSIDCCMSTRING(LPBYTE lpData, DWORD dwLen, LPWFSRESULT &lpResult);
#endif
    HRESULT Fmt_WFSIDCSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCFORM(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCRETAINCARD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCARDDATAARY(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCHIPIO(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCHIPPOWEROUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCFORMLIST(LPVOID lpData, LPWFSRESULT &lpResult);

    // 事件数据
    HRESULT Fmt_WFSIDCTRACKEVENT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_RETAINBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCARDACT(LPVOID lpData, LPWFSRESULT &lpResult);

    // CRD系列命令(Card Dispenser) 结果数据
    HRESULT Fmt_WFSCRDSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCRDCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCRDUNITINFO(LPVOID lpData, LPWFSRESULT &lpResult);
    // CRD系列命令(Card Dispenser) 事件数据
    HRESULT Fmt_SRVE_CRD_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_SRVE_CRD_CARDUNITINFOCHANGED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_SRVE_CRD_DEVICEPOSITION(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_SRVE_CRD_POWER_SAVE_CHANGE(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_USRE_CRD_CARDUNITTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_EXEE_CRD_CARDUNITERROR(LPVOID lpData, LPWFSRESULT &lpResult);

private:
    char m_szLogType[MAX_PATH];
    ICmdFunc *m_pCmdFunc;
    CQtDLLLoader<ISPBaseClass> m_pBase;
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};
