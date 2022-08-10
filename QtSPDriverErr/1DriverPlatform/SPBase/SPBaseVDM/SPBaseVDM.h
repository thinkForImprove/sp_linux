#ifndef SPBASEVDM_H
#define SPBASEVDM_H

#include "ISPBaseVDM.h"

class CSPBaseVDM : public ISPBaseVDM, public CLogManage, public ICmdRun
{

public:
    CSPBaseVDM(LPCSTR lpLogType);
    virtual ~CSPBaseVDM();
public:
    //释放接口
    virtual void Release();
    //注册回掉接口
    virtual void RegisterICmdFunc(ICmdFunc *pCmdFunc);
    //开始运行
    virtual bool StartRun();
    // 获取SPBase数据
    virtual void GetSPBaseData(SPBASEDATA &stData);
    // 发送事件
    virtual bool FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData);
    // 发送设备状态改变事件
    virtual bool FireStatusChanged(DWORD dwStatus);
    // 发送设备故障事件
    virtual bool FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription = nullptr);
protected:
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
    HRESULT Fmt_WFSVDMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSVDMCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
private:
    char                            m_szLogType[MAX_PATH];
    ICmdFunc                        *m_pCmdFunc;
    CQtDLLLoader<ISPBaseClass>      m_pBase;
    CQtDLLLoader<IWFMShareMenory>   m_pIWFM;
};

#endif // SPBASEVDM_H
