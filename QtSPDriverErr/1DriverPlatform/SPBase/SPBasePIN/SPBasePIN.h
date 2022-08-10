#pragma once
#include "ISPBasePIN.h"
//////////////////////////////////////////////////////////////////////////
class  CSPBasePIN : public ISPBasePIN, public CLogManage, public ICmdRun
{
public:
    CSPBasePIN(LPCSTR lpLogType);
    virtual ~CSPBasePIN();
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

    virtual HRESULT OnClearCancelSemphoreCount() override;          //30-00-00-00(FT#0070)
protected:
    // 格式并保存结果数据
    // 公用格式
    HRESULT Fmt_WFSXDATA(CAutoWFMFreeBuffer &_auto, const LPWFSXDATA lpXData, LPWFSXDATA &lpNewXData, LPWFSRESULT &lpResult);

    HRESULT Fmt_WFSPINSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINKEYDETAIL(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINFUNCKEYDETAIL(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSXDATA(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINKEYDETAILEX(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINSECUREKEYDETAIL(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINLOCALDESRESULT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINCREATEOFFSETRESULT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINLOCALEUROCHEQUERESULT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINDATA(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINLOCALBANKSYSRESULT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINBANKSYSIO(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINSECMSG(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINENCIO(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINKCV(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINSTARTKEYEXCHANGE(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINIMPORTRSASIGNEDDESKEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINGENERATERSAKEYPAIR(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINGETCERTIFICATEOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINREPLACECERTIFICATEOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINIMPORTRSAENCIPHEREDPKCS7KEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINEMVIMPORTPUBLICKEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINDIGESTOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINENTRY(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINPRESENTRESULT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINSECUREKEYENTRYOUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINIMPORTRSAPUBLICKEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINLOADCERTIFICATEOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINKEY(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPININIT(LPVOID lpData, LPWFSRESULT &lpResult);
private:
    char                                m_szLogType[MAX_PATH];
    ICmdFunc                            *m_pCmdFunc;
    CQtDLLLoader<ISPBaseClass>          m_pBase;
    CQtDLLLoader<IWFMShareMenory>       m_pIWFM;
};
