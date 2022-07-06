#pragma once
#include "ISPBasePTR.h"

//////////////////////////////////////////////////////////////////////////
class  CSPBasePTR : public ISPBasePTR, public CLogManage, public ICmdRun
{
public:
    CSPBasePTR(LPCSTR lpLogType);
    virtual ~CSPBasePTR();
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
    HRESULT Fmt_WFSPTRSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPTRCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPTRFORMLIST(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPTRMEDIALIST(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSFRMHEADER(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSFRMMEIDA(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSFRMFIELD(LPVOID lpData, LPWFSRESULT &lpResult);
    //EXE
    HRESULT Fmt_WFSPTRREADFORMOUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPTRRAWDATAIN(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPTRMEDIAEXT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPTRIMAGE(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSRETRACTMEDIAOUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSMEDIAPRESENTED(LPVOID lpData, LPWFSRESULT &lpResult);

    //EVENT
    HRESULT Fmt_WFS_PTR_NOMEDIA(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_EXEE_PTR_FIELDERROR(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_USRE_PTR_RETRACTBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_USRE_PTR_PAPERTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_DATA_WORD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_SRVE_PTR_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_NODATA(LPVOID lpData, LPWFSRESULT &lpResult);

private:
    char                                m_szLogType[MAX_PATH];
    ICmdFunc                            *m_pCmdFunc;
    CQtDLLLoader<ISPBaseClass>          m_pBase;
    CQtDLLLoader<IWFMShareMenory>       m_pIWFM;
};
