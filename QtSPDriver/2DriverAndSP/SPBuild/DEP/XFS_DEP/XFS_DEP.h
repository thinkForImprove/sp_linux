#pragma once
#include "IDevDEP.h"
#include "ISPBaseDEP.h"
#include "QtTypeInclude.h"

//////////////////////////////////////////////////////////////////////////
class CXFS_DEP : public ICmdFunc, public CLogManage
{
public:
    CXFS_DEP();
    virtual ~CXFS_DEP();
public:
    // 开始运行SP
    long StartRun();

protected:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // 查询命令
    virtual HRESULT GetStatus(LPWFSDEPSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSDEPCAPS &lpCaps);
    // 执行命令
    virtual HRESULT Entry(LPWFSDEPENVELOPE lpWfsDepEnvelope);
    virtual HRESULT Dispense();
    virtual HRESULT Retract(LPWFSDEPENVELOPE lpWfsDepEnvelope);
    virtual HRESULT ResetCount();
    virtual HRESULT OpenShutter();
    virtual HRESULT CloseShutter();
    virtual HRESULT Reset(LPDWORD lpDword);
    virtual HRESULT SetGuidAnceLight(LPWFSDEPSETGUIDLIGHT lpWfsDEPSetGuidLight);
    virtual HRESULT SupplyReplenish(LPWFSDEPSUPPLYREPLEN lpWfsDepSupplyReplen);
    virtual HRESULT PowerSaveControl(LPWFSDEPPOWERSAVECONTROL lpWfsDEPPowerSaveControl);

protected:
    // 状态初始化
    void InitStatus();
    // 能力值初始化
    void InitCaps();
    // 更新状态
    bool UpdateStatus(const DEVDEPSTATUS &stStatus);
    // 更新扩展状态，strErrCode错误码的后三位，因前六位是固化的
    void UpdateExtra(string strErrCode, string strDevVer = "");
    // 设备状态是否正常
    bool IsDevStatusOK();
private:
    BOOL                                    m_bShutterOpen;
    char                                    m_szLogType[MAX_PATH];
    CQtDLLLoader<ISPBaseDEP>                m_pBase;
    CQtDLLLoader<IDevDEP>                   m_pDev;
    WFSDEPSTATUS                            m_stStatus;
    WFSDEPCAPS                              m_stCaps;
    CAutoEvent                              m_cCancelEvent;
    CXfsRegValue                            m_cXfsReg;
    std::string                             m_strLogicalName;
    std::string                             m_strSPName;
    CExtraInforHelper                       m_cExtra;
    CSimpleMutex                           *m_pMutexGetStatus;
};
