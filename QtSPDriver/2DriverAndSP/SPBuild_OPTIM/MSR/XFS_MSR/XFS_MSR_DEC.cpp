/***************************************************************
* 文件名称：XFS_MSR_DEC.cpp
* 文件描述：刷折器模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_MSR.h"

//------------------------------------处理流程------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_MSR::InnerOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    // Open前下传初始参数
    if (bReConn == FALSE)
    {
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pDev->SetData(SET_LIB_PATH, m_stConfig.szSDKPath);
        }
        m_pDev->SetData(SET_DEV_INIT, &(m_stConfig.stMsrInitParamInfo));
    }

    // 打开连接
    nRet = m_pDev->Open(nullptr);
    if (nRet != IDC_SUCCESS)
    {
        if (bReConn == FALSE)
        {
            Log(ThisModule, __LINE__, "打开设备连接失败, ErrCode:%d, Return: %d",
                nRet, ConvertDevErrCode2WFS(nRet));
        }
        return ConvertDevErrCode2WFS(nRet);
    }

    // Reset
    if (m_stConfig.wOpenResetSupp == 1)
    {
        HRESULT hRet = InnerReset();
        if (hRet != WFS_SUCCESS)
        {

            if (m_stConfig.wResetFailReturn == 0)
            {
                Log(ThisModule, __LINE__, "打开设备连接后Reset失败．Return: %d", hRet);
                return hRet;
            } else
            {
                Log(ThisModule, __LINE__, "打开设备连接后Reset失败．Not Return Error. ");
            }
        }
    }

    // 更新扩展状态
    UpdateExtra();

    // 更新一次状态
    OnStatus();

    if (bReConn == FALSE)
    {
        Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, __LINE__, "断线重连,打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}

// 读INI
int CXFS_MSR::InitConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD    wTmp = 0;
    CHAR    szTmp[1024];
    CHAR    szIniAppName[256];

    // 底层设备控制动态库名
    memset(&m_stConfig, 0x00, sizeof(STINICONFIG));
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));


    // 设备类型(0:WBT-2172-ZD/1:WB-CS)
    m_stConfig.wDeviceType = (WORD)m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (DWORD)0);

    // Open时是否执行Reset动作(0不执行/1执行,缺省0)
    m_stConfig.wOpenResetSupp = (WORD)m_cXfsReg.GetValue("OPEN_CONFIG", "OpenResetSupp", (DWORD)0);
    if (m_stConfig.wOpenResetSupp != 0 &&
        m_stConfig.wOpenResetSupp != 1)
    {
        m_stConfig.wOpenResetSupp = 0;
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.nOpenFailRet != 0 && m_stConfig.nOpenFailRet != 1)
    {
        m_stConfig.nOpenFailRet = 0;
    }

    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_OPENMODE_%d", m_stConfig.wDeviceType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    // 读设备Open相关参数
    if (m_stConfig.wDeviceType == XFS_TYPE_WBCS10 ||
        m_stConfig.wDeviceType == XFS_TYPE_WBT2172)
    {
        STDEVICEOPENMODE    stDevOpenModeTmp;
        STDEVICELOG         stDeviceLogTmp;

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
        // 是否启用动态库底层处理日志记录(0不启用/1启用,缺省0)
        stDeviceLogTmp.wEnableLog = (WORD)m_cXfsReg.GetValue(szIniAppName, "IsEnableDevLog", (DWORD)1);
        if (stDeviceLogTmp.wEnableLog < 0 && stDeviceLogTmp.wEnableLog > 1)
        {
            stDeviceLogTmp.wEnableLog = 0;
        }

        // 动态库底层处理日志记录级别(缺省4)
        // 0禁止显示写日志, 1:允许记录信息级日志, 2:允许记录调试级日志,
        // 3允许记录警告级日志, 4允许记录错误等级日志，记录发送接收数据
        stDeviceLogTmp.wLogLevel = (WORD)m_cXfsReg.GetValue(szIniAppName, "DeviceLogLevel", (DWORD)1);

        // 动态库底层处理日志路径(绝对路径,不设置则与动态库处于同一路径下)
        strcpy(stDeviceLogTmp.szLogPath, m_cXfsReg.GetValue(szIniAppName, "DeviceLog", ""));

        memcpy(&(m_stConfig.stMsrInitParamInfo.stDeviceLog), &stDeviceLogTmp, sizeof(STDEVICELOG));
        memcpy(&(m_stConfig.stMsrInitParamInfo.stDeviceOpenMode), &stDevOpenModeTmp, sizeof(STDEVICEOPENMODE));
    }

    // Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功,缺省0)
    m_stConfig.wResetFailReturn = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetFailReturn", (DWORD)0);
    if (m_stConfig.wResetFailReturn != 0 &&
        m_stConfig.wResetFailReturn != 1)
    {
        m_stConfig.wResetFailReturn = 0;
    }

    return WFS_SUCCESS;
}

// 状态初始化
void CXFS_MSR::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stStatus.fwDevice    = WFS_IDC_DEVNODEVICE;
    m_stStatus.fwMedia     = WFS_IDC_MEDIAUNKNOWN;
    m_stStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    m_stStatus.fwSecurity  = WFS_IDC_SECNOTSUPP;
    m_stStatus.usCards     = 0;
    m_stStatus.fwChipPower = WFS_IDC_CHIPNOTSUPP;
    m_stStatus.lpszExtra   = nullptr;

    m_stStatusOLD.fwDevice    = m_stStatus.fwDevice;
    m_stStatusOLD.fwMedia     = m_stStatus.fwMedia;
    m_stStatusOLD.fwRetainBin = m_stStatus.fwRetainBin;
    m_stStatusOLD.fwSecurity  = m_stStatus.fwSecurity;
    m_stStatusOLD.usCards     = m_stStatus.usCards;
    m_stStatusOLD.fwChipPower = m_stStatus.fwChipPower;
    m_stStatusOLD.lpszExtra   = m_stStatus.lpszExtra;
}

// 能力值初始化
void CXFS_MSR::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Caps.wClass = WFS_SERVICE_CLASS_IDC;
    m_Caps.fwType = WFS_IDC_TYPESWIPE;
    m_Caps.bCompound = FALSE;
    m_Caps.fwReadTracks = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
    m_Caps.fwWriteTracks = WFS_IDC_NOTSUPP;
    m_Caps.usCards                        = 0;  // 指定回收盒可以容纳的最大卡张数(如果不可用则为0)
    m_Caps.fwChipProtocols = WFS_IDC_NOTSUPP;
    m_Caps.fwSecType = WFS_IDC_SECNOTSUPP;
    m_Caps.fwPowerOnOption = WFS_IDC_NOACTION;
    m_Caps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_Caps.bFluxSensorProgrammable        = FALSE;
    m_Caps.bReadWriteAccessFollowingEject = FALSE;
    m_Caps.fwWriteMode = WFS_IDC_NOTSUPP;
    m_Caps.fwChipPower = WFS_IDC_NOTSUPP;
    m_Caps.lpszExtra = nullptr;//(LPSTR)m_cExtra.GetExtra();
}

// 读卡子处理1
HRESULT CXFS_MSR::InnerReadRawData(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    if (m_stStatus.fwDevice == WFS_IDC_DEVHWERROR ||
        m_stStatus.fwDevice == WFS_IDC_DEVOFFLINE ||
        m_stStatus.fwDevice == WFS_IDC_DEVPOWEROFF)
    {
        Log(ThisModule, __LINE__,
            "读卡子处理: 取设备状态: m_stStatus.fwDevice = %d, Return: %d.",
            m_stStatus.fwDevice, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 读数据
    STMEDIARW stMediaRead;
    stMediaRead.dwRWType = ConvertWFS2MediaRW(dwReadOption);
    stMediaRead.dwTimeOut = dwTimeOut;
    nRet = m_pDev->MediaReadWrite(MEDIA_READ, stMediaRead);
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡子处理: ->MediaReadWrite(%d) Fail, ErrCode: %s, Return: %d",
            MEDIA_READ, ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));

        // 返回错误非以下值时,认定已刷卡,上报InsertCard事件
        if (nRet != ERR_IDC_INSERT_TIMEOUT &&   // 进卡超时
            nRet != ERR_IDC_USER_CANCEL &&      // 用户取消
            nRet != ERR_IDC_PARAM_ERR &&        // 参数错误
            nRet != ERR_IDC_RETAIN_FULL &&      // 回收箱满
            nRet != ERR_IDC_READ_TIMEOUT &&     // 读数据超时
            nRet != ERR_IDC_WRITE_TIMEOUT &&    // 写数据超时
            nRet != ERR_IDC_DEV_HWERR &&        // 硬件故障
            nRet != ERR_IDC_DEV_NOTFOUND &&     // 指定名的设备不存在
            nRet != ERR_IDC_OTHER)              // 其它错误，如调用API错误等
        {
            FireCardInserted(); // 入卡事件
        }

        return ConvertDevErrCode2WFS(nRet);
    }

    // 组织应答数据
    if ((dwReadOption & WFS_IDC_TRACK1) == WFS_IDC_TRACK1)
    {
        if (stMediaRead.stData[0].wResult == RW_RESULT_SUCC)
        {
            SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAOK,
                         stMediaRead.stData[0].wSize, (BYTE *)stMediaRead.stData[0].szData);
        } else
        {
            SetTrackInfo(WFS_IDC_TRACK1, ConvertMediaRWResult2WFS(stMediaRead.stData[0].wResult), 0, nullptr);
        }
    }
    if ((dwReadOption & WFS_IDC_TRACK2) == WFS_IDC_TRACK2)
    {
        if (stMediaRead.stData[1].wResult == RW_RESULT_SUCC)
        {
            SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAOK,
                         stMediaRead.stData[1].wSize, (BYTE *)stMediaRead.stData[1].szData);
        } else
        {
            SetTrackInfo(WFS_IDC_TRACK2, ConvertMediaRWResult2WFS(stMediaRead.stData[1].wResult), 0, nullptr);
        }
    }
    if ((dwReadOption & WFS_IDC_TRACK3) == WFS_IDC_TRACK3)
    {
        if (stMediaRead.stData[2].wResult == RW_RESULT_SUCC)
        {
            SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAOK,
                         stMediaRead.stData[2].wSize, (BYTE *)stMediaRead.stData[2].szData);
        } else
        {
            SetTrackInfo(WFS_IDC_TRACK3, ConvertMediaRWResult2WFS(stMediaRead.stData[2].wResult), 0, nullptr);
        }
    }

    return WFS_SUCCESS;
}

// 读卡数据结构体变量初始化
void CXFS_MSR::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
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

// 复位子处理
HRESULT CXFS_MSR::InnerReset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    nRet = m_pDev->Reset(MEDIA_NOTACTION);
    if (nRet != IDC_SUCCESS) // 复位动作失败
    {
        Log(ThisModule, __LINE__, "复位子处理: ->Reset(%d)失败, ErrCode: %s, Return:%d.",
            MEDIA_NOTACTION, ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        return WFS_SUCCESS;
    }
}

// 状态更新子处理
long CXFS_MSR::UpdateStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题

    INT     nRet = IDC_SUCCESS;
    DWORD   dwHWAct = WFS_ERR_ACT_NOACTION;
    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log
    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFireTaken          = FALSE;
    BOOL    bNeedFireMediaInserted  = FALSE;

    //----------------------设备状态处理----------------------
    STDEVIDCSTATUS stDevStatus;
    nRet = m_pDev->GetStatus(stDevStatus);
    if (nRet != IDC_SUCCESS)    // 返回值处理
    {
        switch (nRet)
        {
            case ERR_IDC_PARAM_ERR:
            case ERR_IDC_READ_ERR:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_NOACTION;
                break;
            case ERR_IDC_COMM_ERR:
            case ERR_IDC_DEV_OFFLINE:
            case ERR_IDC_DEV_NOTFOUND:
            case ERR_IDC_READ_TIMEOUT:
            case ERR_IDC_DEV_NOTOPEN:
                m_stStatus.fwDevice = WFS_IDC_DEVOFFLINE;
                dwHWAct = WFS_ERR_ACT_NOACTION;
                break;
            case ERR_IDC_DEV_STAT_ERR:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
            case ERR_IDC_MED_JAMMED:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                //m_bJamm = TRUE;
                break;
//            case ERR_IDC_NOT_OPEN:
            case ERR_IDC_DEV_HWERR:
            case ERR_IDC_OTHER:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_HWCLEAR;
                break;
            case ERR_IDC_RETAIN_FULL:
                m_stStatus.fwDevice = WFS_IDC_DEVONLINE;
                break;
            case ERR_IDC_USER_ERR:
                m_stStatus.fwDevice = WFS_IDC_DEVUSERERROR;
                break;
            default:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
        }
    } else
    {
        m_stStatus.fwDevice = ConvertDeviceStatus2WFS(stDevStatus.wDevice);
        m_stStatus.fwMedia = ConvertMediaStatus2WFS(stDevStatus.wMedia);
    }

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

    //----------------------事件上报----------------------
    // 硬件故障事件
    if (bNeedFireHWError)
    {
        FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%d|",
                    WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
    }

    // 设备状态变化事件
    if (bNeedFireStatusChanged)
    {
        FireStatusChanged(m_stStatus.fwDevice);
        sprintf(szFireBuffer + strlen(szFireBuffer), "StatusChange:%d|",  m_stStatus.fwDevice);
    }

    // 比较两次状态记录LOG
    if (m_stStatus.Diff(m_stStatusOLD) == true)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|Media:%d->%d%s|Paper[0]:%d->%d%s|"
                            "Ink:%d->%d%s|Toner:%d->%d%s|Lamp:%d->%d%s|; 事件上报记录: %s.",
            m_stStatusOLD.fwDevice, m_stStatus.fwDevice, (m_stStatusOLD.fwDevice != m_stStatus.fwDevice ? " *" : ""),
            m_stStatusOLD.fwMedia, m_stStatus.fwMedia, (m_stStatusOLD.fwMedia != m_stStatus.fwMedia ? " *" : ""),
            m_stStatusOLD.fwRetainBin, m_stStatus.fwRetainBin, (m_stStatusOLD.fwRetainBin != m_stStatus.fwRetainBin ? " *" : ""),
            m_stStatusOLD.fwSecurity, m_stStatus.fwSecurity, (m_stStatusOLD.fwSecurity != m_stStatus.fwSecurity ? " *" : ""),
            m_stStatusOLD.usCards, m_stStatus.usCards, (m_stStatusOLD.usCards != m_stStatus.usCards ? " *" : ""),
            m_stStatusOLD.fwChipPower, m_stStatus.fwChipPower, (m_stStatusOLD.fwChipPower != m_stStatus.fwChipPower ? " *" : ""),
            szFireBuffer);
        m_stStatusOLD.Copy(m_stStatus);
    }

    return WFS_SUCCESS;
}

// 更新扩展数据
void CXFS_MSR::UpdateExtra()
{
    m_cExtra.AddExtra("VRTCount", "2");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", (char*)byDevVRTU);

    // 获取固件版本
    CHAR szFWVer[128] = { 0x00 };
    m_pDev->GetVersion(GET_VER_FW, szFWVer, sizeof(szFWVer) - 1);
    if (strlen(szFWVer) == 0)   // 获取失败等待1秒再次获取
    {
        sleep(1);
        m_pDev->GetVersion(GET_VER_FW, szFWVer, sizeof(szFWVer) - 1);
    }
    if (strlen(szFWVer) > 0)
    {
        m_cExtra.AddExtra("VRTCount", "3");
        m_cExtra.AddExtra("VRTDetail[02]_FW", szFWVer);
    }

    CHAR szSWVer[128] = { 0x00 };
    m_pDev->GetVersion(GET_VER_LIB, szSWVer, sizeof(szSWVer) - 1);
    if (strlen(szSWVer) > 0)
    {
        if (strlen(szFWVer) > 0)
        {
            m_cExtra.AddExtra("VRTCount", "4");
            m_cExtra.AddExtra("VRTDetail[03]_SOFT", szSWVer);
        } else
        {
            m_cExtra.AddExtra("VRTCount", "3");
            m_cExtra.AddExtra("VRTDetail[02]_SOFT", szSWVer);
        }
    }
}

