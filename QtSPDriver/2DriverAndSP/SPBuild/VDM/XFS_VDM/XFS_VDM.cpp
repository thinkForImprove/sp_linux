#include "XFS_VDM.h"

static const char *ThisFile = "XFS_VDM.cpp";
static const char *DEVTYPE = "VDM";

#define VDM_SP_VERSION "HWVDM30-00-00-00"

CXFS_VDM::CXFS_VDM()
{
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    ZeroMemory(&m_stStatus, sizeof(m_stStatus));
    ZeroMemory(&m_stCaps, sizeof(m_stCaps));

    m_dwServiceStatus = WFS_VDM_INACTIVE;
}

CXFS_VDM::~CXFS_VDM()
{

}

long CXFS_VDM::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //加载SPBaseVDM
    if(m_pBase.Load("SPBaseVDM.dll", "CreateISPBaseVDM", DEVTYPE) != 0)
    {
        Log(ThisModule, __LINE__, "加载库失败：SPBaseVDM.dll，失败原因：%s", m_pBase.LastError().toStdString().c_str());
        return -1;
    }

    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_VDM::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = 0;

    m_cxfsReg.SetLogicalName(lpLogicalName);
    m_strLogicName = lpLogicalName;
    m_strSPName = m_cxfsReg.GetSPName();
    string strDevDll = m_cxfsReg.GetValue("DriverDllName", "");

    // 获取更新状态互斥量
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetstatus = stData.pMutex;

    //更新设备能力
    UpdateCap();
    DEVVDMSTATUS stDevVdmStatus;
    stDevVdmStatus.wDevice = WFS_STAT_DEVOFFLINE;
    strcpy(stDevVdmStatus.szErrCode, "000");
    UpdateStatus(stDevVdmStatus);

    //加载DEV
    if(m_pDev.Load(strDevDll.c_str(), "CreateIDevVDM", DEVTYPE) != 0){
        Log(ThisModule, __LINE__, "库加载失败：%s，失败原因：%s", strDevDll.c_str(),
            m_pBase.LastError().toStdString().c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    //打开连接
    lRet = m_pDev->Open();
    if(lRet != 0){
        Log(ThisModule, __LINE__, "打开设备连接失败");
        return WFS_ERR_HARDWARE_ERROR;
    }

    //更新状态
    OnStatus();

    //获取硬件版本信息
    char szDevVer[MAX_PATH] = {0};
    m_pDev->GetDevInfo(szDevVer);
    UpdateCapsExtra(szDevVer);
    UpdateCap();

    //复位设备
    lRet = m_pDev->Reset();
    if(lRet != 0){
        Log(ThisModule, __LINE__, "设备复位失败！");
        return WFS_ERR_HARDWARE_ERROR;
    }

    //读取配置
    //Todo:后续需要时添加

    Log(ThisModule, 1, "打开设备连接成功，StatusExtra=%s", m_cStatusExtra.GetExtraInfo().c_str());

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::OnStatus()
{
    DEVVDMSTATUS stDevStatus;
    memset(&stDevStatus, 0, sizeof(stDevStatus));
    if(m_pDev != nullptr){
        m_pDev->GetStatus(stDevStatus);
        UpdateStatus(stDevStatus);
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::GetStatus(LPWFSVDMSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stStatus.lpszExtra = (LPSTR)m_cStatusExtra.GetExtra();
    lpStatus = &m_stStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::GetCapabilities(LPWFSVDMCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stCaps.lpszExtra = (LPSTR)m_cCapsExtra.GetExtra();
    lpCaps = &m_stCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::EnterModeREQ(DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pDev){
        return WFS_ERR_HARDWARE_ERROR;
    }

    if(m_pDev->EnterVDMReq() != 0){
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_dwServiceStatus = WFS_VDM_ENTERPENDING;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_VDM_ENTER_MODE_REQ, nullptr);

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::EnterModeACK()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pDev){
        return WFS_ERR_HARDWARE_ERROR;
    }

    if(m_dwServiceStatus != WFS_VDM_ENTERPENDING){
        return WFS_ERR_SEQUENCE_ERROR;
    }

    if(m_pDev->EnterVDMAck() != 0){
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_dwServiceStatus = WFS_VDM_ACTIVE;
    m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_VDM_MODEENTERED, nullptr);

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::ExitModeREQ(DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pDev){
        return WFS_ERR_HARDWARE_ERROR;
    }

    if(m_pDev->ExitVDMReq() != 0){
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_dwServiceStatus = WFS_VDM_EXITPENDING;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_VDM_EXIT_MODE_REQ, nullptr);

    return WFS_SUCCESS;
}

HRESULT CXFS_VDM::ExitModeACK()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(nullptr == m_pDev){
        return WFS_ERR_HARDWARE_ERROR;
    }

    if(m_dwServiceStatus != WFS_VDM_EXITPENDING){
        return WFS_ERR_SEQUENCE_ERROR;
    }

    if(m_pDev->ExitVDMAck() != 0){
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_dwServiceStatus = WFS_VDM_INACTIVE;
    m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_VDM_MODEEXITED, nullptr);

    return WFS_SUCCESS;
}

void CXFS_VDM::UpdateStatus(DEVVDMSTATUS &stDevVdmStatus)
{
    AutoMutex(*m_pMutexGetstatus);

    WFSVDMSTATUS stVdmStatus;
    memset(&stVdmStatus, 0, sizeof(stVdmStatus));
    WORD wDevice = WFS_STAT_DEVOFFLINE;
    switch(stDevVdmStatus.wDevice){
    case DEVICE_OFFLINE:
        wDevice = WFS_STAT_DEVOFFLINE;
        break;
    case DEVICE_ONLINE:
        wDevice = WFS_STAT_DEVONLINE;
        break;
    default:
        break;
    }

    if(m_stStatus.wDevice != wDevice){
        //发送状态改变事件
        m_pBase->FireStatusChanged(wDevice);
    }

    //更新扩展域错误码
    UpdateExtra(stDevVdmStatus.szErrCode);

    stVdmStatus.wDevice = wDevice;
    stVdmStatus.wService = m_dwServiceStatus;
    stVdmStatus.lppAppStatus = nullptr;    

    memcpy(&m_stStatus, &stVdmStatus, sizeof(stVdmStatus));

    return;
}

void CXFS_VDM::UpdateCap()
{
    m_stCaps.wClass = WFS_SERVICE_CLASS_VDM;    

    return;
}

void CXFS_VDM::UpdateExtra(string strErrCode)
{
    if(strErrCode == "000"){
        strErrCode.insert(0, "000000");
    } else {
        strErrCode.insert(0, "001101");
    }

    m_cStatusExtra.AddExtra("ErrorDetail", strErrCode.c_str());
    return;
}

void CXFS_VDM::UpdateCapsExtra(string strDevVer)
{
    m_cCapsExtra.AddExtra("SP VRT", VDM_SP_VERSION);
    if(!strDevVer.empty()){
        m_cCapsExtra.AddExtra("VRTCount", "1");
        m_cCapsExtra.AddExtra("VRTDetail[00]", strDevVer.c_str());
    }

    return;
}
