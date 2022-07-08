#ifndef XFS_VDM_H
#define XFS_VDM_H
#include "IDevVDM.h"
#include "ISPBaseVDM.h"
#include "QtTypeInclude.h"

class CXFS_VDM : public ICmdFunc, public CLogManage
{
public:
    CXFS_VDM();
    virtual ~CXFS_VDM();
public:
    //开始运行SP
    long StartRun();

protected:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // VDM类型接口
    virtual HRESULT GetStatus(LPWFSVDMSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSVDMCAPS &lpCaps);
    virtual HRESULT EnterModeREQ(DWORD dwTimeout);
    virtual HRESULT EnterModeACK();
    virtual HRESULT ExitModeREQ(DWORD dwTimeout);
    virtual HRESULT ExitModeACK();
protected:
    // 更新状态
    void UpdateStatus(DEVVDMSTATUS &stDevVdmStatus);
    // 更新能力
    void UpdateCap();
    // 更新扩展状态，strErrCode错误码的后三位，因前六位是固化的
    void UpdateExtra(string strErrCode);
    void UpdateCapsExtra(string strDevVer);
private:
    CQtDLLLoader<ISPBaseVDM>                    m_pBase;
    CQtDLLLoader<IDevVDM>                       m_pDev;
    WFSVDMSTATUS                                m_stStatus;
    WFSVDMCAPS                                  m_stCaps;
    CXfsRegValue                                m_cxfsReg;
    CSimpleMutex                                *m_pMutexGetstatus;
    string                                      m_strLogicName;
    string                                      m_strSPName;
    CExtraInforHelper                           m_cStatusExtra;
    CExtraInforHelper                           m_cCapsExtra;

    DWORD                                       m_dwServiceStatus;
};

#endif // XFS_VDM_H
