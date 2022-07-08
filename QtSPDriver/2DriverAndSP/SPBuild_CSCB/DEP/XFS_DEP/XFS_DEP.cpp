#include "XFS_DEP.h"

static const char *DEVTYPE = "DEP";
static const char *ThisFile = "XFS_DEP.cpp";
//////////////////////////////////////////////////////////////////////////
CXFS_DEP::CXFS_DEP(): m_pMutexGetStatus(nullptr)
{
    m_bShutterOpen = FALSE;
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    memset(&m_stStatus, 0x00, sizeof(m_stStatus));
    memset(&m_stCaps, 0x00, sizeof(m_stCaps));
}
CXFS_DEP::~CXFS_DEP()
{
}

long CXFS_DEP::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseDEP
    if (0 != m_pBase.Load("SPBaseDEP.dll", "CreateISPBaseDEP", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseDEP失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_DEP::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    std::string strPort = m_cXfsReg.GetValue("CONFIG", "Port", "");
    std::string strDevDll = m_cXfsReg.GetValue("DriverDllName", "");
    if (strDevDll.empty() || strPort.empty())
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName或Port配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 加载DEV
    HRESULT hRet = m_pDev.Load(strDevDll.c_str(), "CreateIDevDEP", DEVTYPE);
    if (0 != hRet)
    {
        Log(ThisModule, __LINE__, "加载%s失败, hRet=%d", strDevDll.c_str(), hRet);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 打开连接
    hRet = m_pDev->Open(strPort.c_str());
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败！, Port=%s", strPort.c_str());
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 复位设备
    hRet = m_pDev->Reset();
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "设备复位失败！");
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 更新扩展状态
    char szDevVer[256] = { 0 };
    m_pDev->GetDevInfo(szDevVer);
    UpdateExtra("000", szDevVer);

    InitStatus();
    InitCaps();

    // 更新一次状态
    OnStatus();

    Log(ThisModule, 1, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pDev != nullptr)
    {
        m_pDev->Close();
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::OnStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 空闲更新状态
    DEVDEPSTATUS stStatus;
    if (m_pDev != nullptr)
        m_pDev->GetStatus(stStatus);
    UpdateStatus(stStatus);
    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}


HRESULT CXFS_DEP::GetStatus(LPWFSDEPSTATUS &lpstStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //状态
    lpstStatus = &m_stStatus;
    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::GetCapabilities(LPWFSDEPCAPS &lpstCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 能力
    InitCaps();
    lpstCaps = &m_stCaps;
    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::Entry(LPWFSDEPENVELOPE lpWfsDepEnvelope)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DEP::Dispense()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DEP::Retract(LPWFSDEPENVELOPE lpWfsDepEnvelope)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DEP::ResetCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DEP::OpenShutter()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_bShutterOpen)
        return WFS_SUCCESS;

    long lRet = m_pDev->OpenShutter();
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "开门失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_bShutterOpen = TRUE;
    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::CloseShutter()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (!m_bShutterOpen)
        return WFS_SUCCESS;

    long lRet = m_pDev->CloseShutter();
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "关门失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_bShutterOpen = FALSE;
    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::Reset(LPDWORD lpDword)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = m_pDev->Reset();
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "复位失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (m_bShutterOpen)
    {
        long lRet = m_pDev->CloseShutter();
        if (0 != lRet)
        {
            Log(ThisModule, __LINE__, "复位失败：%d", lRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    m_bShutterOpen = FALSE;
    return WFS_SUCCESS;
}

HRESULT CXFS_DEP::SetGuidAnceLight(LPWFSDEPSETGUIDLIGHT lpWfsDEPSetGuidLight)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DEP::SupplyReplenish(LPWFSDEPSUPPLYREPLEN lpWfsDepSupplyReplen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DEP::PowerSaveControl(LPWFSDEPPOWERSAVECONTROL lpWfsDEPPowerSaveControl)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

// 状态初始化
void CXFS_DEP::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    memset(&m_stStatus, 0x00, sizeof(WFSDEPSTATUS));

    m_stStatus.fwDevice                  = WFS_DEP_DEVNODEVICE;
    m_stStatus.fwDepContainer            = WFS_DEP_DEPUNKNOWN;
    m_stStatus.fwDepTransport            = WFS_DEP_DEPNOTSUPP;
    m_stStatus.fwEnvSupply               = WFS_DEP_ENVNOTSUPP;
    m_stStatus.fwEnvDispenser            = WFS_DEP_ENVNOTSUPP;
    m_stStatus.fwPrinter                 = WFS_DEP_PTRNOTSUPP;
    m_stStatus.fwToner                   = WFS_DEP_TONERNOTSUPP;
    m_stStatus.fwShutter                 = WFS_DEP_SHTUNKNOWN;
    m_stStatus.wNumOfDeposits            = 0;
    m_stStatus.lpszExtra                 = (LPSTR)m_cExtra.GetExtra();
    ZeroMemory(m_stStatus.dwGuidLights, sizeof(DWORD) * sizeof(m_stStatus.dwGuidLights));
    m_stStatus.fwDepositLocation         = 0;
    m_stStatus.wDevicePosition           = 0;
    m_stStatus.usPowerSaveRecoveryTime   = 0;
}

// 能力值初始化
void CXFS_DEP::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    memset(&m_stCaps, 0x00, sizeof(WFSDEPCAPS));

    m_stCaps.wClass                      = WFS_SERVICE_CLASS_DEP;
    m_stCaps.fwType                      = WFS_DEP_TYPEENVELOPE;
    m_stCaps.fwEnvSupply                 = WFS_DEP_ENVNONE;
    m_stCaps.bDepTransport               = TRUE;
    m_stCaps.bPrinter                    = FALSE;
    m_stCaps.bToner                      = FALSE;
    m_stCaps.bShutter                    = TRUE;
    m_stCaps.bPrintOnRetracts            = FALSE;
    m_stCaps.fwRetractEnvelope           = WFS_DEP_NORETRACT;
    m_stCaps.wMaxNumChars                = MAX_EXT;
    m_stCaps.fwCharSupport               = WFS_DEP_ASCII;
    m_stCaps.lpszExtra                   = (LPSTR)m_cExtra.GetExtra();
    ZeroMemory(m_stCaps.dwGuidLights, sizeof(DWORD) * sizeof(m_stStatus.dwGuidLights));
    m_stCaps.bPowerSaveControl           = FALSE;
}

bool CXFS_DEP::UpdateStatus(const DEVDEPSTATUS &stStatus)
{
    AutoMutex(*m_pMutexGetStatus);
    WORD fwDevice = WFS_DEP_DEVHWERROR;

    // 设备状态
    UpdateExtra(stStatus.szErrCode);
    switch (stStatus.wDevice)
    {
    case DEVICE_OFFLINE:
        fwDevice = WFS_DEP_DEVOFFLINE;
        break;
    case DEVICE_ONLINE:
            fwDevice = WFS_DEP_DEVONLINE;
        break;
    case DEVICE_BUSY:
            fwDevice = WFS_DEP_DEVBUSY;
        break;
    default:
        fwDevice = WFS_DEP_DEVHWERROR;
        break;
    }

    // 判断状态是否有变化
    if (m_stStatus.fwDevice != fwDevice)
    {
        m_pBase->FireStatusChanged(fwDevice);
        // 故障时，也要上报故障事件
        if (fwDevice == WFS_DEP_DEVHWERROR)
            m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_cExtra.GetErrDetail("ErrorDetail"));
    }

    m_stStatus.fwDevice = fwDevice;
    m_stStatus.fwShutter = stStatus.wShutterOpen;
    m_stStatus.fwDepContainer = stStatus.wDepContainer;

    // 扩展状态
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();

    return true;
}

void CXFS_DEP::UpdateExtra(string strErrCode, string strDevVer/* = ""*/)
{
    if (strErrCode == "000")
        strErrCode.insert(0, "000000");// 没故障时显示
    else
        strErrCode.insert(0, "001707");// 固化的前六位

    m_cExtra.AddExtra("LastErrorCode", strErrCode.c_str());
    if (!strDevVer.empty())
    {
        char szSPVer[64] = { 0 };
        char szVer[64] = { 0 };
        //GetFileVersion(szVer);
        sprintf(szSPVer, "%08sV%07s", "DEP_V310", szVer);
        m_cExtra.AddExtra("VRTCount", "2");
        m_cExtra.AddExtra("VRTDetail", szSPVer);                      // SP版本程序名称8位+版本8位
        m_cExtra.AddExtra("FirmwareVersion", strDevVer.c_str());      // Firmware(1)版本程序名称8位+版本8位
    }
}

bool CXFS_DEP::IsDevStatusOK()
{
    return (m_stStatus.fwDevice == WFS_DEP_DEVONLINE) ? true : false;
}

//////////////////////////////////////////////////////////////////////////

