/***************************************************************************
* 文件名称: XFS_BCR_IDC_DEC.cpp
* 文件描述: 条码阅读模块子命令处理接口(IDC命令系)
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年7月13日
* 文件版本: 1.0.0.1
******************************************************************************************************************************************************/

#include "XFS_BCR_IDC.h"

//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_BCR::InnerOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    INT nRet = BCR_SUCCESS;

    // Open前下传初始参数(非断线重连)
    if (bReConn == FALSE)
    {
        // 设置SDK路径
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pBCRDev->SetData(SET_LIB_PATH, m_stConfig.szSDKPath);
        }

        // 设置设备打开模式
        m_pBCRDev->SetData(SET_DEV_OPENMODE, &(m_stConfig.stDevOpenMode));
    }

    // 打开设备
    nRet = m_pBCRDev->Open(DEVTYPE2STR(m_stConfig.wDeviceType));
    if (nRet != BCR_SUCCESS)
    {
        if (bReConn == FALSE)
        {
            Log(ThisModule, __LINE__, "Open[%s] fail, ErrCode = %d, Return: %d",
                DEVTYPE2STR(m_stConfig.wDeviceType), nRet, ConvertDevErrCode2WFS(nRet));
        }
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    // 组织扩展数据
    UpdateExtra();

    // 更新一次状态
    OnStatus();

    if (bReConn == TRUE)
    {
        Log(ThisModule, __LINE__, "断线重连成功, Extra=%s.", m_cStatExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s.", m_cStatExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}


// 加载DevBCR动态库
BOOL CXFS_BCR::LoadDevBCRDll(LPCSTR ThisModule)
{
    if (m_pBCRDev == nullptr)
    {
        if (m_pBCRDev.Load(m_stConfig.szDevDllNameBCR, "CreateIDevBCR", DEVTYPE2STR(m_stConfig.wDeviceType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DriverType=%d|%s, ERR:%s",
                m_stConfig.szDevDllNameBCR, m_stConfig.wDeviceType,
                DEVTYPE2STR(m_stConfig.wDeviceType), m_pBCRDev.LastError().toUtf8().constData());
            SetErrorDetail((LPSTR)EC_XFS_DevXXXLoadFail);
            return false;
        } else
        {
            Log(ThisModule, __LINE__, "加载库: DriverDllName=%s, DriverType=%d|%s, Succ.",
                m_stConfig.szDevDllNameBCR, m_stConfig.wDeviceType, DEVTYPE2STR(m_stConfig.wDeviceType));
        }
    }
    return (m_pBCRDev != nullptr);
}

// 加载INI设置
INT CXFS_BCR::InitConfig()
{
    CHAR    szIniAppName[MAX_PATH];
    CHAR    szBuffer[MAX_PATH];
    INT     nTmp;

    // DevBCR动态库名
    strcpy(m_stConfig.szDevDllNameBCR, m_cXfsReg.GetValue("BCRDriverDllName", ""));

    // 设备类型
    m_stConfig.wDeviceType = m_cXfsReg.GetValue("DriverType", (DWORD)0);    

    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.wDeviceType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    //-----------------------------------创自设备参数获取-----------------------------------
    if (m_stConfig.wDeviceType == XFS_NT0861)
    {
        STDEVICEOPENMODE    stDevOpenModeTmp;

        // 打开方式(0串口/1USBHID,缺省0)
        stDevOpenModeTmp.wOpenMode = (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenMode", (DWORD)0);
        if (stDevOpenModeTmp.wOpenMode < 0 || stDevOpenModeTmp.wOpenMode > 1)
        {
            stDevOpenModeTmp.wOpenMode = 0;
        }
        // 设备路径(适用于串口和USBHID,缺省空)
        strcpy(stDevOpenModeTmp.szDevPath[0], m_cXfsReg.GetValue(szIniAppName, "DevPath", ""));
        // 波特率(适用于串口,缺省9600)
        stDevOpenModeTmp.wBaudRate[0] = (WORD)m_cXfsReg.GetValue(szIniAppName, "BaudRate", (DWORD)9600);
        // 设备VID(适用于USBHID,4位16进制字符,缺省空)
        strcpy(stDevOpenModeTmp.szHidVid[0], m_cXfsReg.GetValue(szIniAppName, "VendorId", ""));
        // 设备PID(适用于USBHID,4位16进制字符,缺省空)
        strcpy(stDevOpenModeTmp.szHidPid[0], m_cXfsReg.GetValue(szIniAppName, "ProductId", ""));
        // 通讯协议(0:拆分协议, 1:合并协议, 缺省0)
        stDevOpenModeTmp.wProtocol = (WORD)m_cXfsReg.GetValue(szIniAppName, "Protocol", (DWORD)0);
        // 命令下发超时时间,缺省0,单位:毫秒
        stDevOpenModeTmp.nOtherParam[0] =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "SndTimeOut", (DWORD)0);
        // 命令接收超时时间,缺省0,单位:毫秒
        stDevOpenModeTmp.nOtherParam[1] =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "RcvTimeOut", (DWORD)0);

        memcpy(&(m_stConfig.stDevOpenMode), &stDevOpenModeTmp, sizeof(STDEVICEOPENMODE));
    }

    // ------------------------[OPEN_CONFIG]下参数------------------------
    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.wOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.wOpenFailRet != 0 && m_stConfig.wOpenFailRet != 1)
    {
        m_stConfig.wOpenFailRet = 0;
    }


    //------------------------[RESET_CONFIG]下参数------------------------
    // Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功,缺省0)
    m_stConfig.wResetFailReturn = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetFailReturn", (DWORD)0);
    if (m_stConfig.wResetFailReturn != 0 &&
        m_stConfig.wResetFailReturn != 1)
    {
        m_stConfig.wResetFailReturn = 0;
    }

    //------------------------[BCR_COFNIG]下参数------------------------


    //------------------------[TESTER_CONFIG]下参数------------------------
    //------------------------测试模式相关配置----------------------------------
    // 是否启用测试模式(0:不启用, 1:启用, 缺省0)
    m_stConfig.wTestModeIsSup = (WORD)m_cXfsReg.GetValue("TESTER_CONFIG", "TestModeIsSup", (DWORD)0);


    PrintIniBCR();  // INI配置输出

    return 0;
}

// INI配置输出
INT CXFS_BCR::PrintIniBCR()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
#define PRINT_INI_BUF(STR1, STR2) \
    MSET_0(szBuff); \
    sprintf(szBuff, STR1, STR2); \
    qsIniPrt.append(szBuff);

#define PRINT_INI_BUF2(STR1, STR2, STR3) \
    MSET_0(szBuff); \
    sprintf(szBuff, STR1, STR2, STR3); \
    qsIniPrt.append(szBuff);

    QString qsIniPrt = "";
    CHAR szBuff[256] = { 0x00 };

    PRINT_INI_BUF("\n\t\t\t\t DevBCR动态库名: default->BCRDriverDllName = %s", m_stConfig.szDevDllNameBCR);
    PRINT_INI_BUF("\n\t\t\t\t 设备类型: default->DriverType = %d", m_stConfig.wDeviceType);
    PRINT_INI_BUF2("\n\t\t\t\t 设备SDK库路径: DEVICE_SET_%d->SDK_Path = %s", m_stConfig.wDeviceType, m_stConfig.szSDKPath);
    PRINT_INI_BUF2("\n\t\t\t\t 打开方式(0串口/1USBHID): DEVICE_SET_%d->OpenMode = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.wOpenMode);
    PRINT_INI_BUF2("\n\t\t\t\t 设备路径: DEVICE_SET_%d->DevPath = %s", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szDevPath[0]);
    PRINT_INI_BUF2("\n\t\t\t\t 波特率: DEVICE_SET_%d->DevPath = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.wBaudRate[0]);
    PRINT_INI_BUF2("\n\t\t\t\t 设备VID: DEVICE_SET_%d->VendorId = %s", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szHidVid[0]);
    PRINT_INI_BUF2("\n\t\t\t\t 设备PID: DEVICE_SET_%d->ProductId = %s", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szHidPid[0]);
    PRINT_INI_BUF2("\n\t\t\t\t 通讯协议: DEVICE_SET_%d->Protocol = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.wProtocol);
    PRINT_INI_BUF2("\n\t\t\t\t 命令下发超时时间(单位:毫秒): DEVICE_SET_%d->SndTimeOut = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.nOtherParam[0]);
    PRINT_INI_BUF2("\n\t\t\t\t 命令接收超时时间(单位:毫秒): DEVICE_SET_%d->RcvTimeOut = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.nOtherParam[1]);
    PRINT_INI_BUF("\n\t\t\t\t Open失败时返回值(0原样返回/1返回SUCCESS): OPEN_CONFIG->OpenFailRet = %d", m_stConfig.wOpenFailRet);
    PRINT_INI_BUF("\n\t\t\t\t Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功): RESET_CONFIG->ResetFailReturn = %d", m_stConfig.wResetFailReturn);
    PRINT_INI_BUF("\n\t\t\t\t 是否启用测试模式(0:不启用,1:启用): TESTER_CONFIG->TestModeIsSup = %d",m_stConfig.wTestModeIsSup);

    Log(ThisModule, __LINE__, "BCRIDC INI配置取得如下: %s", qsIniPrt.toStdString().c_str());

    return WFS_SUCCESS;
}

void CXFS_BCR::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stStatus.fwChipPower  = WFS_IDC_CHIPNODEVICE;
    m_stStatus.fwMedia = WFS_IDC_MEDIANOTPRESENT;
    m_stStatus.fwSecurity = WFS_IDC_SECNOTSUPP;
    m_stStatus.lpszExtra = nullptr;
    m_stStatus.usCards = 0;
    m_stStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    m_stStatus.fwChipPower = WFS_IDC_CHIPNOTSUPP;
}

void CXFS_BCR::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stCaps.wClass = WFS_SERVICE_CLASS_IDC;
    m_stCaps.fwType = WFS_IDC_TYPEMOTOR;
    m_stCaps.bCompound = FALSE;
    m_stCaps.fwReadTracks = WFS_IDC_TRACK_WM;
    m_stCaps.fwWriteTracks = WFS_IDC_NOTSUPP;
    m_stCaps.fwChipProtocols = WFS_IDC_NOTSUPP;
    m_stCaps.usCards = 0;
    m_stCaps.fwSecType = WFS_IDC_SECNOTSUPP;
    m_stCaps.fwPowerOnOption = WFS_IDC_NOACTION;
    m_stCaps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_stCaps.bFluxSensorProgrammable = FALSE;
    m_stCaps.bReadWriteAccessFollowingEject = FALSE;
    m_stCaps.fwWriteMode = WFS_IDC_NOTSUPP;
    m_stCaps.fwChipPower = WFS_IDC_NOTSUPP;
    m_stCaps.lpszExtra = nullptr;
}

// 更新扩展数据
void CXFS_BCR::UpdateExtra()
{
    CHAR szFWVersion[64] = { 0x00 };

    // 组织状态扩展数据
    //m_cStatExtra.Clear();
    m_cStatExtra.AddExtra("VRTCount", "2");
    m_cStatExtra.AddExtra("VRTDetail[00]", (LPCSTR)byXFSVRTU);
    m_cStatExtra.AddExtra("VRTDetail[01]", (LPCSTR)byDevIDCVRTU);

    // 取固件版本写入扩展数据
    MCPY_NOLEN(szFWVersion, "FW:");
    m_pBCRDev->GetVersion(GET_VER_FW, szFWVersion + 3, 64 - 3);
    if (strlen(szFWVersion) - 3 > 0)
    {
        m_cStatExtra.AddExtra("VRTCount", "3");
        m_cStatExtra.AddExtra("VRTDetail[02]", szFWVersion);
    }

    m_cCapsExtra.Clear();
    m_cCapsExtra.CopyFrom(m_cStatExtra);
}

// 设备状态实时更新
WORD CXFS_BCR::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题

    INT     nRet = BCR_SUCCESS;

    DWORD dwHWAct = WFS_ERR_ACT_NOACTION;
    CHAR  szHWDesc[1024] = { 0x00 };

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFireTaken          = FALSE;
    BOOL    bNeedFireMediaInserted  = FALSE;

    // 取设备状态
    STDEVBCRSTATUS stDevStatus;
    nRet = m_pBCRDev->GetStatus(stDevStatus);

    //----------------------设备状态处理----------------------
    // 返回值处理
    if (nRet != BCR_SUCCESS)
    {
        SetErrorDetail();
    }

    //----------------------Device状态处理----------------------
    m_stStatus.fwDevice = ConvertDeviceStatus2WFS(stDevStatus.wDevice);

    // Device == ONLINE && 有命令在执行中,设置Device = BUSY
    /*if (m_stStatus.fwDevice == WFS_PTR_DEVONLINE && m_bCmdRuning == TRUE)    // 命令执行中
    {
        m_stStatus.fwDevice == WFS_PTR_DEVBUSY;
    }*/

    //----------------------Media状态处理----------------------
    m_stStatus.fwMedia = WFS_IDC_MEDIANOTSUPP;

    //----------------------RetainBin状态处理----------------------
    m_stStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;

    //----------------------Security状态处理----------------------
    m_stStatus.fwSecurity = WFS_IDC_SECNOTSUPP;

    //----------------------ChipPower状态处理----------------------
    m_stStatus.fwChipPower = WFS_IDC_CHIPNOTSUPP;

    //----------------------状态检查----------------------
    // Device状态变化检查
    if (m_stStatus.fwDevice != m_stStatusOLD.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        if (m_stStatus.fwDevice == WFS_IDC_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }

    // Media状态变化检查
    if (m_enWaitTaken == WTF_TAKEN)   // Taken事件检查
    {
        // 当前状态无卡 && 上一次内部或出口有卡,则卡被取走
        if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT &&
            (m_stStatusOLD.fwMedia == WFS_IDC_MEDIAPRESENT || m_stStatusOLD.fwMedia == WFS_IDC_MEDIAENTERING))
        {
            bNeedFireTaken = TRUE;  // 设置上报Taken事件标记
        }
    }

    //----------------------事件上报----------------------
    // 硬件故障事件
    if (bNeedFireHWError)
    {
        FireHWEvent(dwHWAct, szHWDesc);
        sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%s|", dwHWAct, szHWDesc);
    }

    // 设备状态变化事件
    if (bNeedFireStatusChanged)
    {
        FireStatusChanged(m_stStatus.fwDevice);
        sprintf(szFireBuffer + strlen(szFireBuffer), "StatusChange:%d|",  m_stStatus.fwDevice);
    }

    // 介质插入事件
    if (bNeedFireMediaInserted == TRUE /*&& m_bCmdRuning == TRUE*/)
    {
        FireCardInserted();
        Log(ThisModule, __LINE__, "介质插入");
        sprintf(szFireBuffer + strlen(szFireBuffer), "MediaInsert|");
    }

    // Taken事件
    if (bNeedFireTaken)
    {
        FireMediaRemoved();
        m_enWaitTaken = WTF_NONE;     // Taken标记复位
        Log(ThisModule, __LINE__, "介质取走");
        sprintf(szFireBuffer + strlen(szFireBuffer), "MediaRemoved|");
    }

    // 比较两次状态记录LOG
    if (m_stStatus.Diff(m_stStatusOLD) == true)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|Media:%d->%d%s|RetainBin:%d->%d%s|"
                            "Security:%d->%d%s|usCards:%d->%d%s|ChipPower:%d->%d%s|; 事件上报记录: %s.",
            m_stStatusOLD.fwDevice, m_stStatus.fwDevice, (m_stStatusOLD.fwDevice != m_stStatus.fwDevice ? " *" : ""),
            m_stStatusOLD.fwMedia, m_stStatus.fwMedia, (m_stStatusOLD.fwMedia != m_stStatus.fwMedia ? " *" : ""),
            m_stStatusOLD.fwRetainBin, m_stStatus.fwRetainBin, (m_stStatusOLD.fwRetainBin != m_stStatus.fwRetainBin ? " *" : ""),
            m_stStatusOLD.fwSecurity, m_stStatus.fwSecurity, (m_stStatusOLD.fwSecurity != m_stStatus.fwSecurity ? " *" : ""),
            m_stStatusOLD.usCards, m_stStatus.usCards, (m_stStatusOLD.usCards != m_stStatus.usCards ? " *" : ""),
            m_stStatusOLD.fwChipPower, m_stStatus.fwChipPower, (m_stStatusOLD.fwChipPower != m_stStatus.fwChipPower ? " *" : ""),
            szFireBuffer);
        m_stStatusOLD.Copy(m_stStatus);
    }

    return 0;
}

// 读卡子处理
HRESULT CXFS_BCR::InnerAcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = BCR_SUCCESS;

    // 扫码
    STREADBCRIN stReadBcrIn;
    STREADBCROUT stReadBcrOut;
    stReadBcrIn.dwTimeOut = dwTimeOut;
    stReadBcrIn.wSymDataMode = 1;       // 返回条码数据模式(1Hex)
    nRet = m_pBCRDev->ReadBCR(stReadBcrIn, stReadBcrOut);
    if (nRet != BCR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡(扫码): ->MediaReadWrite() Fail, ErrCode: %d, Return: %d",
            nRet, ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    // 组织应答数据
    if (stReadBcrOut.nSymDataSize == 0)
    {
        SetTrackInfo(WFS_IDC_TRACK_WM, WFS_IDC_DATAMISSING, 0, nullptr);
    } else
    {
        SetTrackInfo(WFS_IDC_TRACK_WM, WFS_IDC_DATAOK,
                     (ULONG)stReadBcrOut.nSymDataSize, (LPBYTE)stReadBcrOut.szSymData);
        stReadBcrOut.Clear();
    }

    return WFS_SUCCESS;
}

// 复位子处理
HRESULT CXFS_BCR::InnerReset(WORD wAction)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = BCR_SUCCESS;

    // 执行复位
    nRet = m_pBCRDev->Reset(wAction);
    if (nRet != BCR_SUCCESS)    // 复位失败
    {
        Log(ThisModule, __LINE__, "复位: ->Reset(%d) Fail, ErrCode: %s, Return: %d",
            wAction, ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    return WFS_SUCCESS;
}

// 读卡应答数据处理
void CXFS_BCR::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
{
    WFSIDCCARDDATA data;
    data.wDataSource    = wSource;
    data.wStatus        = wStatus;
    data.ulDataLength   = uLen;
    data.lpbData        = pData;
    data.fwWriteMethod  = 0;

    m_CardDatas.SetAt(wSource, data);
    return;
}

// 读卡应答数据处理
bool CXFS_BCR::GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho)
{
    LPWFSIDCCARDDATA pCardData = m_CardDatas.GetAt(wSource);
    if (!pCardData)
    {
        *pLen = 0;
        return FALSE;
    }

    if (*pLen < pCardData->ulDataLength)
    {
        *pLen = pCardData->ulDataLength;
        return false;
    }

    *pLen           = pCardData->ulDataLength;
    *pWriteMetho    = pCardData->fwWriteMethod;
    memcpy(pData, pCardData->lpbData, *pLen);
    return true;
}

// 设置ErrorDetail错误码
INT CXFS_BCR::SetErrorDetail(LPSTR lpCode)
{
    if (lpCode == nullptr)
    {
        CHAR szErrCode[1024] = { 0x00 };
        m_pBCRDev->GetData(GET_DEV_ERRCODE, szErrCode);
        m_clErrorDet.SetXFSErrCode(szErrCode + 1);
    } else
    {
        m_clErrorDet.SetXFSErrCode(lpCode);
    }

    return WFS_SUCCESS;
}

// -------------------------------- END -----------------------------------

