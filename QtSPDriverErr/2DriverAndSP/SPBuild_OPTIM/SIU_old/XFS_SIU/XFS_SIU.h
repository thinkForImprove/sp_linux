#pragma once
#include "IDevSIU.h"
#include "ISPBaseSIU.h"
#include "QtTypeInclude.h"
//////////////////////////////////////////////////////////////////////////
typedef list<WFSSIUENABLE>          listWFSSIUENABLE;
//////////////////////////////////////////////////////////////////////////
class CXFS_SIU : public ICmdFunc, public CLogManage
{
public:
    CXFS_SIU();
    virtual ~CXFS_SIU();
public:
    // 开始运行SP
    long StartRun();

protected:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // SIU类型接口
    virtual HRESULT GetStatus(LPWFSSIUSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSSIUCAPS &lpCaps);
    virtual HRESULT SetEnableEvent(const LPWFSSIUENABLE lpEvent);
    virtual HRESULT SetDoor(const LPWFSSIUSETDOOR lpDoor);
    virtual HRESULT SetIndicator(const LPWFSSIUSETINDICATOR lpIndicator);
    virtual HRESULT SetGuidLight(const LPWFSSIUSETGUIDLIGHT lpGuidLight);
    virtual HRESULT Reset();

protected:
    // 启动事件
    void EnableEvent();
    // 转换状态
    void ConvertStatus(const DEVSIUSTATUS &stDevStatus, WFSSIUSTATUS &stSiuStatus);
    // 更新状态
    bool UpdateStatus(const WFSSIUSTATUS &stStatus);
    // 更新能力
    bool UpdateCap();
    // 设备状态是否正常
    bool IsDevStatusOK();
    // 检测是否支持门
    bool IsSupportDoor(const LPWFSSIUSETDOOR lpDoor);
    // 检测是否支持指示符
    bool IsSupportIndicator(const LPWFSSIUSETINDICATOR lpIndicator);
    // 检测是否支持灯
    bool IsSupportGuidLight(const LPWFSSIUSETGUIDLIGHT lpLight);
    // 发状态改变事件
    bool FirePortEvent(const LPWFSSIUPORTEVENT lpEvent);
    // 是否允许发事件
    bool IsEnableEvent(const LPWFSSIUPORTEVENT lpEvent);
    // 更新扩展状态，strErrCode错误码的后三位，因前六位是固化的
    void UpdateExtra(string strErrCode, string strDevVer = "");
    // 读取配置文件
    void ReadConfig();                          //40-00-00-00(FT#0002)
private:
    CQtDLLLoader<ISPBaseSIU>                m_pBase;
    CQtDLLLoader<IDevSIU>                   m_pDev;
    WFSSIUSTATUS                            m_stStatus;
    WFSSIUCAPS                              m_stCaps;
    CXfsRegValue                            m_cXfsReg;
    CSimpleMutex                           *m_pMutexGetStatus;
    string                                  m_strLogicalName;
    string                                  m_strSPName;
    CExtraInforHelper                       m_cExtra;
    bool                                    m_bSaftDoorToOperatorSwitch;
    listWFSSIUENABLE                        m_ltEnable;
    std::recursive_mutex                    m_cEnableMutex;

    int                                     m_iMaintenanceStatusValue;      //40-00-00-00(FT#0002)
};

