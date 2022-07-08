#include "XFS_BCR.h"

static const char *DEVTYPE = "BCR";
static const char *ThisFile = "XFS_BCR.cpp";
//////////////////////////////////////////////////////////////////////////
CXFS_BCR::CXFS_BCR(): m_pMutexGetStatus(nullptr)
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    memset(&m_stStatus, 0x00, sizeof(m_stStatus));
    memset(&m_stCaps, 0x00, sizeof(m_stCaps));
    m_bScanerOn = false;
}
CXFS_BCR::~CXFS_BCR()
{
}

long CXFS_BCR::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseBCR
    if (0 != m_pBase.Load("SPBaseBCR.dll", "CreateISPBaseBCR", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseBCR失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_BCR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    m_strPort = m_cXfsReg.GetValue("CONFIG", "Port", "");
    std::string strDevDll = m_cXfsReg.GetValue("DriverDllName", "");
    if (strDevDll.empty() || m_strPort.empty())
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName或Port配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 加载DEV
    HRESULT hRet = m_pDev.Load(strDevDll.c_str(), "CreateIDevBCR", DEVTYPE);
    if (0 != hRet)
    {
        Log(ThisModule, __LINE__, "加载%s失败, hRet=%d", strDevDll.c_str(), hRet);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 打开连接
    hRet = m_pDev->Open(m_strPort.c_str());
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败！, Port=%s", m_strPort.c_str());
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 复位设备
    hRet = m_pDev->Reset();
    if (hRet != 0)
    {
        // 复位失败，不用返回故障，只要更新状态为故障就行了
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

    // 更新一次状态
    OnStatus();

    Log(ThisModule, 1, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pDev != nullptr)
    {
        m_pDev->CancelRead();// 退出前，取消一次
        m_pDev->Close();
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR::OnStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 空闲更新状态
    DEVBCRSTATUS stStatus;
    if (m_pDev != nullptr)
        m_pDev->GetStatus(stStatus);
    UpdateStatus(stStatus);
    if(stStatus.wDevice == DEVICE_OFFLINE){
        m_pDev->Close();
        m_pDev->Open(m_strPort.c_str());
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
        m_pDev->CancelRead();
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}


HRESULT CXFS_BCR::GetStatus(LPWFSBCRSTATUS &lpstStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //状态
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpstStatus = &m_stStatus;
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR::GetCapabilities(LPWFSBCRCAPS &lpstCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 能力
    m_stCaps.wClass = WFS_SERVICE_CLASS_BCR;
    m_stCaps.bCompound = FALSE;
    m_stCaps.bCanFilterSymbologies = FALSE;
    if (m_stCaps.lpwSymbologies == nullptr)
    {
        int nSize = WFS_BCR_SYM_KOREANPOST + 2;
        m_stCaps.lpwSymbologies = new WORD[nSize];
        memset(m_stCaps.lpwSymbologies, 0x00, sizeof(WORD) * nSize);
        for (int i = 0; i < nSize && i < WFS_BCR_SYM_KOREANPOST; i++)
        {
            m_stCaps.lpwSymbologies[i] = i + 1;
        }
    }

    m_stCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpstCaps = &m_stCaps;
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR::ReadBCR(const WFSBCRREADINPUT &stReadInput, LPWFSBCRREADOUTPUT *&lppReadOutput, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!IsDevStatusOK())
    {
        Log(ThisModule, __LINE__, "设备故障，扫描码失败");
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 自动更新Scancer状态
    CAutoSetScannerStatus _auto(&m_bScanerOn);
    OnStatus();// 更新一次状态

    // 参数分析
    char szData[8192] = { 0 };
    DWORD dwLen = sizeof(szData);
    DWORD dwType = 0;
    if (stReadInput.lpwSymbologies != nullptr)
    {
        for (int i = 0; ; i++)
        {
            if (stReadInput.lpwSymbologies[i] == 0)
            {
                break;
            }
            dwType += stReadInput.lpwSymbologies[i];
        }
    }

    // 开始等待扫描二维码
    long lRet = m_pDev->ReadBCR(dwType, szData, dwLen, dwTimeOut);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "扫描码失败：%d", lRet);
        if (lRet == -4)
            return WFS_ERR_CANCELED;
        else if (lRet = -9)
            return WFS_ERR_TIMEOUT;
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 暂时只支持一个码
    UINT uCount = 1;
    if (m_cReadOutData.GetSize() == 0)
    {
        if (!m_cReadOutData.NewBuff(uCount))
        {
            Log(ThisModule, __LINE__, "申请内存失败");
            return WFS_ERR_OUT_OF_MEMORY;
        }
    }

    LPWFSBCRREADOUTPUT pData = nullptr;
    for (UINT i = 0; i < uCount; i++)
    {
        pData = m_cReadOutData.GetBuff(i);
        if (pData != nullptr)
        {
            pData->wSymbology = (WORD)dwType;
            pData->lpszSymbologyName = nullptr;
            pData->lpxBarcodeData->usLength = (WORD)dwLen;
            memcpy(pData->lpxBarcodeData->lpbData, szData, dwLen);
        }
    }

    lppReadOutput = m_cReadOutData.GetData();
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = m_pDev->Reset();
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "复位失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_BCR::SetGuidLight(const WFSBCRSETGUIDLIGHT &stLight)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_BCR::PowerSaveControl(const WFSBCRPOWERSAVECONTROL &stPowerCtrl)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

bool CXFS_BCR::UpdateStatus(const DEVBCRSTATUS &stStatus)
{
    AutoMutex(*m_pMutexGetStatus);
    WORD fwScanner = WFS_BCR_SCANNERUNKNOWN;
    WORD fwDevice = WFS_BCR_DEVHWERROR;

    // 设备状态
    UpdateExtra(stStatus.szErrCode);
    switch (stStatus.wDevice)
    {
    case DEVICE_OFFLINE:
        fwDevice = WFS_BCR_DEVOFFLINE;
        break;
    case DEVICE_ONLINE:
        {
            fwDevice = WFS_BCR_DEVONLINE;
            fwScanner = m_bScanerOn ? WFS_BCR_SCANNERON : WFS_BCR_SCANNEROFF;
        }
        break;
    default:
        fwDevice = WFS_BCR_DEVHWERROR;
        break;
    }

    // 判断状态是否有变化
    if (m_stStatus.fwDevice != fwDevice)
    {
        m_pBase->FireStatusChanged(fwDevice);
        // 故障时，也要上报故障事件
        if (fwDevice == WFS_BCR_DEVHWERROR)
            m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_cExtra.GetErrDetail("ErrorDetail"));
    }
    m_stStatus.fwDevice = fwDevice;
    m_stStatus.fwBCRScanner = fwScanner;

    // 扩展状态
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();

    // 测试
    //m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_DEVICE_STATUS, &m_stStatus);
    return true;
}

void CXFS_BCR::UpdateExtra(string strErrCode, string strDevVer/* = ""*/)
{
    if (strErrCode == "000")
        strErrCode.insert(0, "000000");// 没故障时显示
    else
        strErrCode.insert(0, "001707");// 固化的前六位

    m_cExtra.AddExtra("ErrorDetail", strErrCode.c_str());
    if (!strDevVer.empty())
    {
        char szSPVer[64] = { 0 };
        char szVer[64] = { 0 };
        //GetFileVersion(szVer);
        sprintf(szSPVer, "%08sV%07s", "BCR_V310", szVer);
        m_cExtra.AddExtra("VRTCount", "2");
        m_cExtra.AddExtra("VRTDetail[00]", szSPVer);                // SP版本程序名称8位+版本8位
        m_cExtra.AddExtra("VRTDetail[01]", strDevVer.c_str());      // Firmware(1)版本程序名称8位+版本8位
    }
}

bool CXFS_BCR::IsDevStatusOK()
{
    return (m_stStatus.fwDevice == WFS_BCR_DEVONLINE) ? true : false;
}

//////////////////////////////////////////////////////////////////////////

