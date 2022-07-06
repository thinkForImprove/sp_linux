#pragma once
#include "ISPBaseSIU.h"
//////////////////////////////////////////////////////////////////////////
class  CSPBaseSIU : public ISPBaseSIU, public CLogManage, public ICmdRun
{
public:
    CSPBaseSIU(LPCSTR lpLogType);
    virtual ~CSPBaseSIU();
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
    // 事件数据
    HRESULT Fmt_WFSSIUPORTEVENT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSSIUPORTERROR(LPVOID lpData, LPWFSRESULT &lpResult);

    // 结果数据
    HRESULT Fmt_WFSSIUSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSSIUCAPS(LPVOID lpData, LPWFSRESULT &lpResult);

    // 没有参数的
    HRESULT Fmt_NODATA(LPWFSRESULT &lpResult);

private:
    char                                m_szLogType[MAX_PATH];
    ICmdFunc                            *m_pCmdFunc;
    CQtDLLLoader<ISPBaseClass>          m_pBase;
    CQtDLLLoader<IWFMShareMenory>       m_pIWFM;
};
