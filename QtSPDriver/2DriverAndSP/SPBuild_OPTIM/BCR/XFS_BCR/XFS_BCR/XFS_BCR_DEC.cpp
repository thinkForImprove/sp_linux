/***************************************************************
* 文件名称: XFS_BCR_DEC.cpp
* 文件描述: 二维码模块子命令处理接口(BCR命令系)
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2019年7月6日
* 文件版本: 1.0.0.1
****************************************************************/

#include "XFS_BCR.h"
#include <QTextCodec>

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
    CHAR    szBuffer[1024] = { 0x00 };
    INT     nTmp;

    /*********************************************************************
     * STDEVICEOPENMODE.nOtherParam[32]: 设备Open模式结构体.其他参数 类别说明
     *   [0]: 命令下发超时时间
     *   [1]: 命令接收超时时间
     *   [2]: 扫码识读模式
     *   [3]: 扫码读码时是否设置条码类型限制
    *********************************************************************/

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
        stDevOpenModeTmp.wProtocol[0] = (WORD)m_cXfsReg.GetValue(szIniAppName, "Protocol", (DWORD)0);
        // 命令下发超时时间,缺省0,单位:毫秒
        stDevOpenModeTmp.nOtherParam[0] =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "SndTimeOut", (DWORD)0);
        // 命令接收超时时间,缺省0,单位:毫秒
        stDevOpenModeTmp.nOtherParam[1] =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "RcvTimeOut", (DWORD)0);

        // 扫码识读模式(0:硬件默认, 1:手动识读, 2:命令连续识读, 3:上电连续识读, 4:感应识读, 缺省0)
        stDevOpenModeTmp.nOtherParam[2] =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "ScanSymMode", (DWORD)0);
        if (stDevOpenModeTmp.nOtherParam[2] < 0 || stDevOpenModeTmp.nOtherParam[2] > 4)
        {
            stDevOpenModeTmp.nOtherParam[2] = 0;
        }

        // 扫码读码时是否设置条码类型限制(0:不设置,按硬件所有类型识别, 1:根据入参设置类型识别限制, 缺省0)
        stDevOpenModeTmp.nOtherParam[3] =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "ReadBcrSymModeSet", (DWORD)0);
        if (stDevOpenModeTmp.nOtherParam[3] != 0 &&
            stDevOpenModeTmp.nOtherParam[3] != 1)
        {
            stDevOpenModeTmp.nOtherParam[3] = 0;
        }

        // 设备是否支持识别条码类型(0:不支持, 1:支持, 缺省0)
        m_stConfig.wDistSymModeSup = m_cXfsReg.GetValue(szIniAppName, "DistSymModeSup", (DWORD)0);
        if (m_stConfig.wOpenFailRet != 0 && m_stConfig.wOpenFailRet != 1)
        {
            m_stConfig.wOpenFailRet = 0;
        }

        // 设备支持的条码类型, 空值或不设置为使用硬件默认支持, 缺省空
        strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, "SymSuppList", ""));
        if (strlen(szBuffer) > 0)
        {
            // 解析数据
            ULONG ulArraySize = DataConvertor::split_string(szBuffer, ',', nullptr, 0);
            if (ulArraySize > 0)
            {
                CHAR szArray[ulArraySize][CONST_VALUE_260];
                DataConvertor::split_string(szBuffer, ',', szArray, ulArraySize);

                INT nIdx = 0;
                for (INT i = 0; i < ulArraySize; i ++)
                {
                    if ((nTmp = DataConvertor::str2number(szArray[i])) > 0)
                    {
                        nIdx ++;
                        m_stConfig.wSymSuppList[nIdx] = nTmp;
                    }
                }
                m_stConfig.wSymSuppList[0] = nIdx;
            }
        }

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
    // 扫码读码返回数据格式(0:16进制, 1:ASCII, 缺省0)
    m_stConfig.wReadBcrRetDataMode = (WORD)m_cXfsReg.GetValue("BCR_COFNIG", "ReadBcrRetDataMode", (DWORD)0);
    if (m_stConfig.wReadBcrRetDataMode != 0 &&
        m_stConfig.wReadBcrRetDataMode != 1)
    {
        m_stConfig.wReadBcrRetDataMode = 0;
    }

    // 扫码读码返回数据编码格式(0:UTF8, 1:GBK, 缺省0)
    m_stConfig.wReadBcrRetDataCode = (WORD)m_cXfsReg.GetValue("BCR_COFNIG", "ReadBcrRetDataCode", (DWORD)0);
    if (m_stConfig.wReadBcrRetDataCode != 0 &&
        m_stConfig.wReadBcrRetDataCode != 1)
    {
        m_stConfig.wReadBcrRetDataCode = 0;
    }


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
    PRINT_INI_BUF2("\n\t\t\t\t 通讯协议: DEVICE_SET_%d->Protocol = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.wProtocol[0]);
    PRINT_INI_BUF2("\n\t\t\t\t 命令下发超时时间(单位:毫秒): DEVICE_SET_%d->SndTimeOut = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.nOtherParam[0]);
    PRINT_INI_BUF2("\n\t\t\t\t 命令接收超时时间(单位:毫秒): DEVICE_SET_%d->RcvTimeOut = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.nOtherParam[1]);
    PRINT_INI_BUF2("\n\t\t\t\t 扫码识读模式(0:硬件默认,1:手动识读,2:命令连续识读,3:上电连续识读,4:感应识读): DEVICE_SET_%d->ScanSymMode = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.nOtherParam[2]);
    PRINT_INI_BUF2("\n\t\t\t\t 扫码读码时是否设置条码类型限制(0:不设置,按硬件所有类型识别,1:根据入参设置类型识别限制): DEVICE_SET_%d->ReadBcrSymModeSet = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.nOtherParam[3]);
    PRINT_INI_BUF2("\n\t\t\t\t 设备是否支持识别条码类型(0:不支持,1:支持): DEVICE_SET_%d->DistSymModeSup = %d", m_stConfig.wDeviceType, m_stConfig.wDistSymModeSup);
    PRINT_INI_BUF("\n\t\t\t\t Open失败时返回值(0原样返回/1返回SUCCESS): OPEN_CONFIG->OpenFailRet = %d", m_stConfig.wOpenFailRet);
    PRINT_INI_BUF("\n\t\t\t\t Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功): RESET_CONFIG->ResetFailReturn = %d", m_stConfig.wResetFailReturn);
    PRINT_INI_BUF("\n\t\t\t\t 扫码读码返回数据格式(0:16进制,1:ASCII): BCR_COFNIG->ReadBcrRetDataMode = %d", m_stConfig.wReadBcrRetDataMode);
    PRINT_INI_BUF("\n\t\t\t\t 扫码读码返回数据编码格式(0:UTF8,1:GBK): BCR_COFNIG->ReadBcrRetDataCode = %d", m_stConfig.wReadBcrRetDataCode);
    PRINT_INI_BUF("\n\t\t\t\t 是否启用测试模式(0:不启用,1:启用): TESTER_CONFIG->TestModeIsSup = %d",m_stConfig.wTestModeIsSup);

    Log(ThisModule, __LINE__, "BCRBCR INI配置取得如下: %s", qsIniPrt.toStdString().c_str());

    return WFS_SUCCESS;
}

void CXFS_BCR::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stStatus.Clear();
}

void CXFS_BCR::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备是否支持识别条码类型(0:不支持, 1:支持)
    if (m_stConfig.wDistSymModeSup == 1)
    {
        m_stCaps.bCanFilterSymbologies = TRUE;

        if (m_stCaps.lpwSymbologies != nullptr)
        {
            delete [] m_stCaps.lpwSymbologies;
            m_stCaps.lpwSymbologies = nullptr;
        }

        if (m_stConfig.wSymSuppList[0] > 1)
        {
            INT nSize = m_stConfig.wSymSuppList[0] + 2;
            m_stCaps.lpwSymbologies = new WORD[nSize];
            memset(m_stCaps.lpwSymbologies, 0x00, sizeof(WORD) * nSize);
            for (INT i = 0; i < m_stConfig.wSymSuppList[0]; i++)
            {
                m_stCaps.lpwSymbologies[i] = m_stConfig.wSymSuppList[i + 1];
            }
        }
    } else
    {
        m_stCaps.bCanFilterSymbologies = FALSE;
        m_stCaps.lpwSymbologies = nullptr;
    }
}

// 更新扩展数据
void CXFS_BCR::UpdateExtra()
{
    CHAR szFWVersion[64] = { 0x00 };

    // 组织状态扩展数据
    //m_cStatExtra.Clear();
    m_cStatExtra.AddExtra("VRTCount", "2");
    m_cStatExtra.AddExtra("VRTDetail[00]", (LPCSTR)byXFSVRTU);
    m_cStatExtra.AddExtra("VRTDetail[01]", (LPCSTR)byDevBCRVRTU);

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
    BOOL    bNeedFireDevPosition    = FALSE;

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

    //----------------------wBcrScanner状态处理----------------------
    m_stStatus.fwBCRScanner = ConvertScannerStatus2WFS(stDevStatus.wBcrScanner);

    //----------------------wGuidLights状态处理----------------------
    
    //----------------------wPosition状态处理----------------------
    m_stStatus.wDevicePosition = ConvertDevPosStatus2WFS(stDevStatus.wPosition);

    //----------------------usPowerSaveRecoveryTime状态处理----------------------
    m_stStatus.usPowerSaveRecoveryTime = stDevStatus.usPowerTime;

    //----------------------状态检查----------------------
    // Device状态变化检查
    if (m_stStatus.fwDevice != m_stStatusOLD.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        if (m_stStatus.fwDevice == WFS_BCR_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }

    //----------------------设备未知检查----------------------
    // 设备位置状态变化检查
    if (m_stStatus.wDevicePosition != m_stStatusOLD.wDevicePosition)
    {
        bNeedFireDevPosition = TRUE;
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

    // 设备位置变化事件
    if (bNeedFireDevPosition)
    {
        FireSetDevicePosition(m_stStatus.wDevicePosition);
        sprintf(szFireBuffer + strlen(szFireBuffer), "DevicePosition:%d|",  m_stStatus.wDevicePosition);
    }

    // 比较两次状态记录LOG
    if (m_stStatus.Diff(m_stStatusOLD) == true)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|Scanner:%d->%d%s|Position:%d->%d%s|"
                            "PowerRecTime:%d->%d%s|; 事件上报记录: %s.",
            m_stStatusOLD.fwDevice, m_stStatus.fwDevice,
            (m_stStatusOLD.fwDevice != m_stStatus.fwDevice ? " *" : ""),
            m_stStatusOLD.fwBCRScanner, m_stStatus.fwBCRScanner,
            (m_stStatusOLD.fwBCRScanner != m_stStatus.fwBCRScanner ? " *" : ""),
            m_stStatusOLD.wDevicePosition, m_stStatus.wDevicePosition,
            (m_stStatusOLD.wDevicePosition != m_stStatus.wDevicePosition ? " *" : ""),
            m_stStatusOLD.usPowerSaveRecoveryTime, m_stStatus.usPowerSaveRecoveryTime,
            (m_stStatusOLD.usPowerSaveRecoveryTime != m_stStatus.usPowerSaveRecoveryTime ? " *" : ""),
            szFireBuffer);
        m_stStatusOLD.Copy(m_stStatus);
    }

    return 0;
}

// 扫码读码子处理
HRESULT CXFS_BCR::InnerReadBcr(LPWORD lpwSymList, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = BCR_SUCCESS;
    INT nCodeType = 0;          // 数据编码格式
    CHAR szBcrDataAscii[65536];
    DWORD dwBcrDataAsciiSize;
    CHAR szBcrDataResult[65536];
    DWORD dwBcrDataResultSize;

    // 扫码
    STREADBCRIN stReadBcrIn;
    STREADBCROUT stReadBcrOut;

    // 组织入参
    stReadBcrIn.dwTimeOut = dwTimeOut;
    if (m_stConfig.wReadBcrRetDataMode == 0)    // INI配置返回条码数据模式(Hex)
    {
        stReadBcrIn.wSymDataMode = SYMD_HEX;
    } else
    {
        stReadBcrIn.wSymDataMode = SYMD_ASCII;
    }

    // 扫码列表处理
    if (m_stCaps.bCanFilterSymbologies == TRUE)
    {
        if (lpwSymList != nullptr)
        {
            for (INT i = 0; ; i++)
            {
                if (lpwSymList[i] == 0)
                {
                    break;
                }
                stReadBcrIn.wSymType[i + 1] = ConvertWFSSymDevToDev(lpwSymList[i]);
                stReadBcrIn.wSymType[0] ++;
            }
        }
    }

    // 执行扫码读码
    nRet = m_pBCRDev->ReadBCR(stReadBcrIn, stReadBcrOut);
    if (nRet != BCR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "扫码读码: ->ReadBCR() Fail, ErrCode: %d, Return: %d",
            nRet, ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    // 组织应答数据
    UINT uCount = 1;
    if (m_clReadBcrOut.GetSize() == 0)
    {
        if (m_clReadBcrOut.NewBuff(uCount) != TRUE)
        {
            Log(ThisModule, __LINE__, "扫码读码: 命令执行成功: 申请应答Buffer失败, Return: %d",
                WFS_ERR_OUT_OF_MEMORY);
            SetErrorDetail((LPSTR)EX_XFS_MemoryApply);
            return WFS_ERR_OUT_OF_MEMORY;
        }
    }

    LPWFSBCRREADOUTPUT pData = nullptr;
    for (UINT i = 0; i < uCount; i++)
    {
        pData = m_clReadBcrOut.GetBuff(i);
        if (pData != nullptr)
        {
            pData->wSymbology = stReadBcrOut.wSymType;
            pData->lpszSymbologyName = nullptr;

            if (stReadBcrOut.dwSymDataSize > 0)
            {
                // DevBCR返回数据转换为ASCII格式
                MSET_0(szBcrDataAscii);
                if (stReadBcrOut.wSymDataMode == SYMD_HEX)  // DevXXX返回条码数据为HEX,先转换为ASCII
                {
                    dwBcrDataAsciiSize = DataConvertor::Hex2Ascii(stReadBcrOut.szSymData, stReadBcrOut.dwSymDataSize,
                                                                  szBcrDataAscii, sizeof(szBcrDataAscii));
                } else                                      // DevXXX返回条码数据为ASCII,不转换
                {
                    MCPY_LEN(szBcrDataAscii, stReadBcrOut.szSymData, stReadBcrOut.dwSymDataSize);
                    dwBcrDataAsciiSize = stReadBcrOut.dwSymDataSize;
                }

                // 检查条码数据编码格式
                nCodeType = DataConvertor::ChkDataIsUTF8(szBcrDataAscii, dwBcrDataAsciiSize);

                // 条码数据编码格式处理(UTF8/GBK)
                MSET_0(szBcrDataResult);
                if (m_stConfig.wReadBcrRetDataCode == 0)    // INI配置返回条码数据编码格式为UTF8
                {
                    if (nCodeType == CODE_UTF8) // 数据为UTF8编码,不转换
                    {
                        MCPY_LEN(szBcrDataResult, szBcrDataAscii, dwBcrDataAsciiSize);
                    } else                      // 数据为GBK编码,转换为UTF8
                    {
                        dwBcrDataResultSize = DataConvertor::string_ascii_to_utf8(szBcrDataAscii, szBcrDataResult,
                                                                                  sizeof(szBcrDataResult));
                        if (dwBcrDataResultSize < 1)
                        {
                            Log(ThisModule, __LINE__, "扫码读码: 命令执行成功: 数据为GBK编码,转换为UTF8编码失败, Return: %d",
                                WFS_ERR_SOFTWARE_ERROR);
                            SetErrorDetail((LPSTR)EX_XFS_CodeChange);
                            return WFS_ERR_OUT_OF_MEMORY;
                        }
                    }
                } else                                      // INI配置返回条码数据编码格式为GBK
                {
                    if (nCodeType == CODE_UTF8) // 数据为UTF8编码,转换为GBK
                    {
                        dwBcrDataResultSize = DataConvertor::string_utf8_to_ascii(szBcrDataAscii, szBcrDataResult,
                                                                                  sizeof(szBcrDataResult));
                        if (dwBcrDataResultSize < 1)
                        {
                            Log(ThisModule, __LINE__, "扫码读码: 命令执行成功: 数据为UTF8编码,转换为GBK编码失败, Return: %d",
                                WFS_ERR_SOFTWARE_ERROR);
                            SetErrorDetail((LPSTR)EX_XFS_CodeChange);
                            return WFS_ERR_OUT_OF_MEMORY;
                        }
                    } else                      // 数据为GBK编码,不转换
                    {
                        MCPY_LEN(szBcrDataResult, szBcrDataAscii, dwBcrDataAsciiSize);
                        dwBcrDataResultSize = dwBcrDataAsciiSize;
                    }
                }

                // 条码返回数据模式处理(16进制/ASCII)
                if (m_stConfig.wReadBcrRetDataMode == 0)    // INI配置返回条码数据模式为Hex, 转换HEX
                {
                    pData->lpxBarcodeData->usLength =
                            DataConvertor::Ascii2Hex(szBcrDataResult, dwBcrDataResultSize,
                                                     (LPSTR)pData->lpxBarcodeData->lpbData, m_clReadBcrOut.GetDataMemSize());
                } else                                      // INI配置返回条码数据模式为Ascii
                {
                    pData->lpxBarcodeData->usLength = (WORD)dwBcrDataResultSize;
                        memcpy(pData->lpxBarcodeData->lpbData, szBcrDataResult, dwBcrDataResultSize);
                }

                // 条码返回数据模式处理(16进制/ASCII)
                /*
                if (m_stConfig.wReadBcrRetDataMode == 0)    // INI配置返回条码数据模式为Hex
                {
                    if (stReadBcrOut.wSymDataMode == SYMD_HEX)  // 返回条码数据为HEX(不转换)
                    {
                        pData->lpxBarcodeData->usLength = (WORD)stReadBcrOut.dwSymDataSize;
                        memcpy(pData->lpxBarcodeData->lpbData, stReadBcrOut.szSymData, stReadBcrOut.dwSymDataSize);
                    } else                                      // 返回条码数据为ASCII, 转换为HEX
                    {
                        pData->lpxBarcodeData->usLength =
                                DataConvertor::Ascii2Hex(stReadBcrOut.szSymData, stReadBcrOut.dwSymDataSize,
                                                         (LPSTR)pData->lpxBarcodeData->lpbData, m_clReadBcrOut.GetDataMemSize());
                    }
                } else                                      // INI配置返回条码数据模式为Ascii
                {
                    if (stReadBcrOut.wSymDataMode == SYMD_HEX)  // 返回条码数据为HEX, 转换为ASCII
                    {
                        pData->lpxBarcodeData->usLength =
                                DataConvertor::Hex2Ascii(stReadBcrOut.szSymData, stReadBcrOut.dwSymDataSize,
                                                         (LPSTR)pData->lpxBarcodeData->lpbData, m_clReadBcrOut.GetDataMemSize());
                    } else  // 返回条码数据为ASCII, 不转换为
                    {
                        pData->lpxBarcodeData->usLength = (WORD)stReadBcrOut.dwSymDataSize;
                        memcpy(pData->lpxBarcodeData->lpbData, stReadBcrOut.szSymData, stReadBcrOut.dwSymDataSize);
                    }

                }*/
            } else
            {
                pData->lpxBarcodeData->usLength = (WORD)stReadBcrOut.dwSymDataSize;
                memcpy(pData->lpxBarcodeData->lpbData, stReadBcrOut.szSymData, stReadBcrOut.dwSymDataSize);
            }
        }
    }

    return WFS_SUCCESS;
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

// -------------------------------------- END --------------------------------------
