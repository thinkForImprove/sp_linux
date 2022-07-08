#include "XFS_IDX.h"

#include <unistd.h>
#include <sys/stat.h>

// IDX SP 版本号
BYTE    byVRTU[17] = {"HWIDXSTE00000100"};

#define LOG_NAME       "XFS_IDX.log"

static const char *DEVTYPE = "IDX";
static const char *ThisFile = "XFS_IDX.cpp";


//////////////////////////////////////////////////////////////////////////

CXFS_IDX::CXFS_IDX()
{
    //SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);
    m_stIdxIniConfig.clear();
    m_bJamm = FALSE;
    m_WaitTaken = WTF_NONE;
    m_enCardPosition = CARDPOS_UNKNOWN;
    memset(&m_Caps, 0x00, sizeof(WFSIDCCAPS));
    m_CardDatas.Clear();
    m_cExtra.Clear();
}

CXFS_IDX::~CXFS_IDX()
{
    //
}


long CXFS_IDX::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseIDC
    if (0 != m_pBase.Load("SPBaseIDX.dll", "CreateISPBaseIDX", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseIDX失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();

    return 0;
}

// ------基本接口----------------------------
// 打开设备
HRESULT CXFS_IDX::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // INI读取
    InitConfig();

    // 设备驱动动态库验证
    if (strlen(m_stIdxIniConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 加载设备驱动动态库
    if (m_pDev == NULL)
    {
        hRet = m_pDev.Load(m_stIdxIniConfig.szDevDllName,
                           "CreateIDevIDX", DEVTYPE_CHG(m_stIdxIniConfig.wDeviceType));
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__,
                "加载库失败: DriverDllName=%s, DEVTYPE=%d|%s. ReturnCode:%s",
                m_stIdxIniConfig.szDevDllName, m_stIdxIniConfig.wDeviceType,
                DEVTYPE_CHG(m_stIdxIniConfig.wDeviceType),
                m_pDev.LastError().toUtf8().constData());
            return hErrCodeChg(hRet);
        }
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = StartOpen();
    if (m_stIdxIniConfig.nOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

// 关闭设备
HRESULT CXFS_IDX::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
        m_pDev->Close();

    return WFS_SUCCESS;
}

// 状态更新
HRESULT CXFS_IDX::OnStatus()
{
    UpdateStatus();

    // 断线后进行重连
    if (m_Status.fwDevice == WFS_IDC_DEVOFFLINE)
    {
        StartOpen();
        UpdateStatus();
    }

    return WFS_SUCCESS;
}

// Taken上报
HRESULT CXFS_IDX::OnWaitTaken()
{
    if (m_WaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    WaitItemTaken();
    return WFS_SUCCESS;
}

HRESULT CXFS_IDX::OnCancelAsyncRequest()
{
    if (m_pDev != nullptr)
        m_pDev->Cancel();
    return WFS_SUCCESS;
}

HRESULT CXFS_IDX::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// ------查询命令----------------------------

// 状态查询
HRESULT CXFS_IDX::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpStatus = &m_Status;

    return WFS_SUCCESS;
}

// 能力值查询
HRESULT CXFS_IDX::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCaps = &m_Caps;

    return WFS_SUCCESS;
}

// ------执行命令----------------------------

// 退卡
HRESULT CXFS_IDX::EjectCard(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    Q_UNUSED(dwTimeOut)

    //CardPosition enCardPos = CARDPOS_UNKNOWN;

    return EjectExecute();
}

// 吞卡
long CXFS_IDX::RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_stIdxIniConfig.wRetainSupp == 1)
    {
        return WFS_ERR_INVALID_COMMAND;
    } else
    {
        return WFS_ERR_INVALID_COMMAND;
    }
}

// 读卡
HRESULT CXFS_IDX::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wOption = *(WORD *)lpReadData;

    // 读卡时不选任何参数值返回DATAINVALID
    //if ((wOption & 0x803F) == 0)
    WORD nTrack = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
    if (wOption & nTrack == 0)
    {
        Log(ThisModule, -1, "接收ReadRawData参数无效");
        return WFS_ERR_INVALID_DATA;
    }

    m_CardDatas.Clear();
    long hRes = ReadRawDataExecute(wOption, dwTimeOut);
    if (hRes != WFS_SUCCESS)
    {
        Log(ThisModule, -1, "AcceptAndReadTrack failed");
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_CardDatas;

    if (m_stIdxIniConfig.wReadEndRunEject == 1 && hRes == WFS_SUCCESS)
    {
        return EjectExecute();
    }

    return hRes;
}

// 复位
HRESULT CXFS_IDX::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wAction = *lpResetIn;
    // 没有动作参数时由SP->INI决定
    if (wAction == 0)
    {
        if (m_stIdxIniConfig.wResetCardAction == CARDACTION_NOACTION)
        {
            wAction = WFS_IDC_NOACTION;
        } else
        if (m_stIdxIniConfig.wResetCardAction == CARDACTION_EJECT)
        {
            wAction = WFS_IDC_EJECT;
        } else
        if (m_stIdxIniConfig.wResetCardAction == CARDACTION_RETAIN)
        {
            wAction = WFS_IDC_RETAIN;
        }
    }

    // 无效参数
    if (wAction < WFS_IDC_NOACTION || wAction > WFS_IDC_RETAIN)
    {
        return WFS_ERR_INVALID_DATA;
    }

    CardAction ActFlag;
    if (wAction == WFS_IDC_RETAIN)
    {
        ActFlag = CARDACTION_RETAIN;
    }
    else if (wAction == WFS_IDC_NOACTION)
    {
        ActFlag = CARDACTION_NOACTION;
    }
    else if (wAction == WFS_IDC_EJECT)
    {
        ActFlag = CARDACTION_EJECT;
    }
    else
    {
        return WFS_ERR_INVALID_DATA;
    }

    HRESULT hRet = ResetExecute(ActFlag);
    if (m_stIdxIniConfig.wResetFailReturn == 0)
    {
        return hRet;
    } else
    {
        return WFS_SUCCESS;
    }
}

//  ------处理流程----------------------------

// 状态初始化
void CXFS_IDX::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Status.fwDevice    = WFS_IDC_DEVNODEVICE;
    m_Status.fwMedia     = WFS_IDC_MEDIAUNKNOWN;
    m_Status.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    m_Status.fwSecurity  = WFS_IDC_SECNOTSUPP;
    m_Status.usCards     = 0;
    m_Status.fwChipPower = WFS_IDC_CHIPNOTSUPP;
    m_Status.lpszExtra   = nullptr;

    m_OldStatus.fwDevice    = m_Status.fwDevice;
    m_OldStatus.fwMedia     = m_Status.fwMedia;
    m_OldStatus.fwRetainBin = m_Status.fwRetainBin;
    m_OldStatus.fwSecurity  = m_Status.fwSecurity;
    m_OldStatus.usCards     = m_Status.usCards;
    m_OldStatus.fwChipPower = m_Status.fwChipPower;
    m_OldStatus.lpszExtra   = m_Status.lpszExtra;
}

// 能力值初始化
void CXFS_IDX::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Caps.wClass = WFS_SERVICE_CLASS_IDC;
    m_Caps.fwType = WFS_IDC_TYPESWIPE;
    m_Caps.bCompound = FALSE;
    m_Caps.fwReadTracks = WFS_IDC_NOTSUPP;
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
    m_Caps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
}

// 读卡子处理1
HRESULT CXFS_IDX::ReadRawDataExecute(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();



//    if (m_Status.fwDevice == WFS_IDC_DEVHWERROR ||
//        m_Status.fwDevice == WFS_IDC_DEVOFFLINE ||
//        m_Status.fwDevice == WFS_IDC_DEVPOWEROFF)
//    {
//        Log(ThisModule, -1, "m_Status.fwDevice = %d",m_Status.fwDevice);
//        return WFS_ERR_HARDWARE_ERROR;
//    }

    //CardPosition enCardPos = CARDPOS_UNKNOWN;
    int nRet = 0;

    // 取卡位置
    nRet = m_pDev->GetCardPosition(m_enCardPosition);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "ReadRawData前检测是否有卡失败: %s", GetErrorStr(nRet));
        return hErrCodeChg(nRet);
    }

    if (m_enCardPosition != CARDPOS_INTERNAL)  // 内部无卡
    {
        // 进卡
        nRet = m_pDev->AcceptCard(dwTimeOut);
        if (nRet < 0)
        {
            Log(ThisModule, 1, "进卡失败，返回码nRet = %d|%s", nRet, GetErrorStr(nRet));
            return hErrCodeChg(nRet);
        }
        else if ((nRet == ERR_IDX_INSERT_TIMEOUT) ||
                 (nRet == ERR_IDX_USER_CANCEL))
        {
            // 取卡位置
            int nRet1 = m_pDev->GetCardPosition(m_enCardPosition);
            if (nRet1 < 0)
            {
                Log(ThisModule, -1, "AcceptCard进卡超时后检测是否有卡失败: %s", GetErrorStr(nRet1));
                return hErrCodeChg(nRet1);
            }
            if (m_enCardPosition == CARDPOS_INTERNAL || m_enCardPosition == CARDPOS_SCANNING)  // 内部有卡，执行退卡
            {
                m_pDev->EjectCard();
            }

            if (nRet == ERR_IDX_INSERT_TIMEOUT)
            {
                return WFS_ERR_TIMEOUT;
            }
            else
            {
                return WFS_ERR_CANCELED;
            }
        }
        else
        {
            Log(ThisModule, 1, "进卡成功");
            m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
            FireCardInserted();
        }
    }

    m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;

    return ReadTrackData(dwReadOption);
}

// 读卡子处理2
long CXFS_IDX::ReadTrackData(DWORD dwReadOption)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STTRACK_INFO    trackInfo;
    // ReadRawData前先将磁道初始化避免KAL出错崩溃
    if (dwReadOption & WFS_IDC_TRACK1)
    {
        SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAINVALID, 0, NULL);
    }
    if (dwReadOption & WFS_IDC_TRACK2)
    {
        SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAINVALID, 0, NULL);
    }
    if (dwReadOption & WFS_IDC_TRACK3)
    {
        SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAINVALID, 0, NULL);
    }

    int nRet = m_pDev->ReadTracks(trackInfo);
    if (nRet == 0)
    {
        Log("ReadTrack", __LINE__, "ReadTracks()调用成功");
        m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
        if (dwReadOption & WFS_IDC_TRACK1)
        {
            if (trackInfo.TrackData[0].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[0].szTrack), (BYTE *)trackInfo.TrackData[0].szTrack);
            }
        }

        if (dwReadOption & WFS_IDC_TRACK2)
        {
            if (trackInfo.TrackData[1].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[1].szTrack), (BYTE *)trackInfo.TrackData[1].szTrack);
            }
        }

        if (dwReadOption & WFS_IDC_TRACK3)
        {
            if (trackInfo.TrackData[2].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[2].szTrack), (BYTE *)trackInfo.TrackData[2].szTrack);
            }
        }
    }
    else if (nRet == ERR_IDX_NOTRACK)
    {
        return WFS_ERR_IDC_INVALIDMEDIA;
    }
    else if (nRet == ERR_IDX_CARDTOOSHORT)
    {
        return WFS_ERR_IDC_CARDTOOSHORT;
    }
    else if (nRet == ERR_IDX_CARDTOOLONG)
    {
        return WFS_ERR_IDC_CARDTOOLONG;
    }
    else if (nRet == ERR_IDX_READERROR)
    {
        return WFS_ERR_IDC_INVALIDDATA;
    }
    else if (nRet == ERR_IDX_INVALIDCARD)
    {
        return WFS_SUCCESS;
    }
    else
    {
        Log("ReadTrack", -1, "ReadTrack时出错:%s", GetErrorStr(nRet));
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

// 读卡子处理３
void CXFS_IDX::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
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
HRESULT CXFS_IDX::ResetExecute(CardAction enCardAct)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //CardPosition enCardPos = CARDPOS_UNKNOWN;
    int nRet = 0;

    // 取卡位置
    nRet = m_pDev->GetCardPosition(m_enCardPosition);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "Reset前检测卡失败: %s", GetErrorStr(nRet));
        //return hErrCodeChg(nRet);
    }
    if (m_enCardPosition == CARDPOS_NOCARD) // 没有检测到卡
    {
        Log(ThisModule, -1, "Reset前没有检测到卡");
        //return WFS_ERR_IDX_NOMEDIA;
    }

    //if (m_stIdxIniConfig.wRetainSupp == 0 && enCardAct == CARDACTION_RETAIN)
    if (enCardAct == CARDACTION_RETAIN)
    {
        enCardAct = CARDACTION_NOACTION;
    }

    nRet = m_pDev->Reset(enCardAct);
    if (nRet >= 0)
    {
        //if (enCardAct == CARDACTION_EJECT && m_enCardPosition != CARDPOS_NOCARD)
        //{
        //    m_WaitTaken = WTF_TAKEN;
        //}

        int nRet1 = m_pDev->GetCardPosition(m_enCardPosition);
        if (nRet1 < 0)
        {
            Log(ThisModule, -1, "Reset后检测卡失败:%s", GetErrorStr(nRet1));
            return hErrCodeChg(nRet1);;
        }
        else if (nRet1 >= 0)
        {
            if (enCardAct == CARDACTION_EJECT)
            {
                m_Status.fwMedia = WFS_IDC_MEDIAENTERING;
                if (m_enCardPosition != CARDPOS_NOCARD)
                    m_WaitTaken = WTF_TAKEN;
            }

            if (enCardAct == CARDACTION_NOACTION && m_enCardPosition != CARDPOS_NOCARD)
            {
                FireMediaDetected(WFS_IDC_CARDREADPOSITION);
            }

            m_bJamm = FALSE;
            Log(ThisModule, -1, "Reset时检测到卡");
            return WFS_SUCCESS;
        }
        else
        {
            m_Status.fwMedia = WFS_IDC_MEDIANOTPRESENT;
            return WFS_SUCCESS;
        }
    }
    else // 复位动作失败
    {
        if (nRet == ERR_IDX_JAMMED)
        {
            FireMediaDetected(WFS_IDC_CARDJAMMED);
        }
    }

    Log(ThisModule, -1, "Reset失败: %s", GetErrorStr(nRet));

    if (nRet == ERR_IDX_JAMMED)
    {
        return WFS_ERR_IDC_MEDIAJAM;
    }
    else
    if (nRet == ERR_IDX_INVALIDCARD)
    {
        return WFS_ERR_IDC_INVALIDMEDIA;
    }
    else
    {
        return WFS_ERR_HARDWARE_ERROR;
    }
}

// 退卡子处理
HRESULT CXFS_IDX::EjectExecute()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = m_pDev->GetCardPosition(m_enCardPosition);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "退卡前检测卡失败:%s", GetErrorStr(nRet));
        return hErrCodeChg(nRet);
    }
    if (m_enCardPosition == CARDPOS_NOCARD) // 没有检测到卡
    {
        Log(ThisModule, -1, "退卡时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }
    else
    if (//m_enCardPosition == CARDPOS_ENTERING ||                       // 40-00-00-00(FT#0010)
        m_enCardPosition == CARDPOS_SCANNING ||
        m_enCardPosition == CARDPOS_INTERNAL)   // 卡在扫描位置/内部       // 40-00-00-00(FT#0010)
    {
        int nRet = m_pDev->EjectCard();
        if (nRet >= 0)
        {
            usleep(500 * 1000); // Eject命令下发后等待500毫秒,确保取到有效的卡状态
            int nRet1 = m_pDev->GetCardPosition(m_enCardPosition);
            if (nRet1 < 0)
            {
                Log(ThisModule, -1, "退卡后检测卡失败: %s", GetErrorStr(nRet1));
                return hErrCodeChg(nRet1);
            }

            if (m_enCardPosition != CARDPOS_NOCARD)
            {
                m_WaitTaken = WTF_TAKEN;
            }

            m_Status.fwMedia = WFS_IDC_MEDIAENTERING;
            return WFS_SUCCESS;
        }
        else
        if (nRet == ERR_IDX_JAMMED)
        {
            Log(ThisModule, -1, "退卡时:%s", GetErrorStr(nRet));
            return WFS_ERR_IDC_MEDIAJAM;
        }
        else
        {
            Log(ThisModule, -1, "退卡时:%s", GetErrorStr(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        }
    }
    else
    {
        Log(ThisModule, -1, "退卡时卡状态异常: %d|状态未知", m_enCardPosition);
    }

    return WFS_ERR_HARDWARE_ERROR;
}

// 状态更新子处理
long CXFS_IDX::UpdateStatus()
{
    THISMODULE(__FUNCTION__);

    int nRet = 0;

    DWORD dwHWAct = WFS_ERR_ACT_NOACTION;

    DEVIDXSTATUS stDevIdxStat;
    nRet = m_pDev->GetStatus(stDevIdxStat);
    if (nRet != ERR_IDX_SUCCESS)
    {
        switch (nRet)
        {
            case ERR_IDX_PARAM_ERR:
            case ERR_IDX_READERROR:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_NOACTION;
                break;
            case ERR_IDX_COMM_ERR:
            case ERR_IDX_OFFLINE:
            case ERR_IDX_NO_DEVICE:
            case ERR_IDX_READTIMEOUT:
                m_Status.fwDevice = WFS_IDC_DEVOFFLINE;
                dwHWAct = WFS_ERR_ACT_NOACTION;
                break;
            case ERR_IDX_STATUS_ERR:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
            case ERR_IDX_JAMMED:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                m_bJamm = TRUE;
                break;
            case ERR_IDX_NOT_OPEN:
                m_Status.fwDevice = WFS_IDC_DEVOFFLINE;
                break;
            case ERR_IDX_HWERR:
            case ERR_IDX_OTHER:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_HWCLEAR;
                break;
            case ERR_IDX_RETAINBINFULL:
                m_Status.fwDevice = WFS_IDC_DEVONLINE;
                break;
            case ERR_IDX_USERERR:
                m_Status.fwDevice = WFS_IDC_DEVUSERERROR;
                break;
            default:
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
        }
    } else
    {
        m_Status.fwDevice = wConvertDeviceStatus(stDevIdxStat.wDevice);
        m_Status.fwMedia = wConvertMediaStatus(stDevIdxStat.wMedia);
    }

    if (nRet < 0)
    {
        //m_bNeedRepair = TRUE;
    }


    if (m_Status.fwDevice != m_OldStatus.fwDevice)
    {
        FireStatusChanged(m_Status.fwDevice);
        if (m_Status.fwDevice != WFS_IDC_DEVONLINE && m_Status.fwDevice != WFS_IDC_DEVBUSY)
        {
            FireHWEvent(dwHWAct, nullptr);
        }
    }

    if (m_Status.fwMedia == WFS_IDC_MEDIAPRESENT)
    {
        m_WaitTaken = WTF_NONE;
    } else
    if (m_Status.fwMedia == WFS_IDC_MEDIANOTPRESENT)
    {
        if (m_WaitTaken == WTF_TAKEN)
        {
            FireMediaRemoved();
            Log(ThisModule, 1, "用户取走卡");
            m_WaitTaken = WTF_NONE;
        } else
        {
            m_WaitTaken = WTF_NONE;
        }
    }

    m_Status.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    //memcpy(m_Status, m_OldStatus, sizeof(CWfsIDCStatus));
    m_OldStatus.fwDevice = m_Status.fwDevice;
    m_OldStatus.fwMedia = m_Status.fwMedia;

    return WFS_SUCCESS;
}

// Device状态转换为WFS标准
WORD CXFS_IDX::wConvertDeviceStatus(WORD wDevIdxStat)
{
    switch(wDevIdxStat)
    {
        case DEV_ONLINE:
            return WFS_IDC_DEVONLINE;
        case DEV_OFFLINE:
            return WFS_IDC_DEVOFFLINE;
        case DEV_POWEROFF:
            return WFS_IDC_DEVPOWEROFF;
        case DEV_NODEVICE:
            return WFS_IDC_DEVNODEVICE;
        case DEV_HWERROR:
            return WFS_IDC_DEVHWERROR;
        case DEV_USERERROR:
            return WFS_IDC_DEVUSERERROR;
        case DEV_BUSY:
            return WFS_IDC_DEVBUSY;
    }

    return (0);
}

// Media状态转换为WFS标准
WORD CXFS_IDX::wConvertMediaStatus(WORD wDevIdxStat)
{
    switch(wDevIdxStat)
    {
        case MEDIA_PRESENT:
            return WFS_IDC_MEDIAPRESENT;
        case MEDIA_NOTPRESENT:
            return WFS_IDC_MEDIANOTPRESENT;
        case MEDIA_JAMMED:
            return WFS_IDC_MEDIAJAMMED;
        case MEDIA_NOTSUPP:
            return WFS_IDC_MEDIANOTSUPP;
        case MEDIA_UNKNOWN:
            return WFS_IDC_MEDIAUNKNOWN;
        case MEDIA_ENTERING:
            return WFS_IDC_MEDIAENTERING;
    }

    return (0);
}

// DevIDX返回值转换为WFS标准
HRESULT CXFS_IDX::hErrCodeChg(LONG lDevCode)
{
    switch (lDevCode)
    {
    case ERR_IDX_SUCCESS:                 return WFS_SUCCESS;
    case ERR_IDX_INSERT_TIMEOUT:          return WFS_ERR_TIMEOUT;
    case ERR_IDX_USER_CANCEL:             return WFS_ERR_CANCELED;
    case ERR_IDX_COMM_ERR:                return WFS_ERR_CONNECTION_LOST;
    case ERR_IDX_JAMMED:                  return WFS_ERR_IDC_MEDIAJAM;
    case ERR_IDX_OFFLINE:                 return WFS_ERR_CONNECTION_LOST;
    case ERR_IDX_NOT_OPEN:                return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_RETAINBINFULL:           return WFS_ERR_IDC_RETAINBINFULL;
    case ERR_IDX_HWERR:                   return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_STATUS_ERR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_UNSUP_CMD:               return WFS_ERR_UNSUPP_COMMAND;
    case ERR_IDX_PARAM_ERR:               return WFS_ERR_INVALID_DATA;
    case ERR_IDX_READTIMEOUT:             return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_WRITETIMEOUT:            return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_READERROR:               return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_WRITEERROR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_INVALIDCARD:             return WFS_ERR_IDC_INVALIDMEDIA;
    case ERR_IDX_NOTRACK:                 return WFS_ERR_IDC_INVALIDMEDIA;
    case ERR_IDX_CARDPULLOUT:             return WFS_ERR_USER_ERROR;
    case ERR_IDX_CARDTOOSHORT:            return WFS_ERR_IDC_CARDTOOSHORT;
    case ERR_IDX_CARDTOOLONG:             return WFS_ERR_IDC_CARDTOOLONG;
    case ERR_IDX_WRITETRACKERROR:         return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_PROTOCOLNOTSUPP:         return WFS_ERR_UNSUPP_DATA;
    case ERR_IDX_ICRW_ERROR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_NO_DEVICE:               return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_OTHER:                   return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDX_USERERR:                 return WFS_ERR_USER_ERROR;
    case ERR_IDX_TAMPER:                  return WFS_ERR_USER_ERROR;
    default:                              return WFS_ERR_HARDWARE_ERROR;
    }
}

// DevIDX返回值转换为明文解释
LPCSTR CXFS_IDX::GetErrorStr(int nCode)
{
    switch (nCode)
    {
    case ERR_IDX_SUCCESS:       return "操作成功";
    case ERR_IDX_INSERT_TIMEOUT: return "进卡超时";
    case ERR_IDX_USER_CANCEL:   return "用户取消";
    case ERR_IDX_PARAM_ERR:     return "参数错误";
    case ERR_IDX_COMM_ERR:      return "通讯错误";
    case ERR_IDX_STATUS_ERR:    return "读卡器状态出错";
    case ERR_IDX_JAMMED:        return "读卡器堵卡";
    case ERR_IDX_OFFLINE:       return "读卡器脱机";
    case ERR_IDX_NOT_OPEN:      return "没有调用Open";
    case ERR_IDX_RETAINBINFULL: return "回收箱满";
    case ERR_IDX_HWERR:         return "硬件故障";
    case ERR_IDX_READTIMEOUT:   return "读数据超时";
    case ERR_IDX_READERROR:     return "读数据错误";
    case ERR_IDX_INVALIDCARD:   return "无效芯片，或磁道数据无效";
    case ERR_IDX_NOTRACK:       return "非磁卡，未检测到磁道";
    case ERR_IDX_CARDPULLOUT:   return "接收卡时，卡被拖出";
    case ERR_IDX_CARDTOOSHORT:  return "卡太短";
    case ERR_IDX_CARDTOOLONG:   return "卡太长";
    case ERR_IDX_PROTOCOLNOTSUPP: return "不支持的IC通讯协议";
    case ERR_IDX_NO_DEVICE:     return "指定名的设备不存在";
    case ERR_IDX_OTHER:         return "其它错误，如调用API错误等";
    default:                    return "未定义错误码";
    }
}

// Taken子处理
long CXFS_IDX::WaitItemTaken()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CardPosition enCardPos = CARDPOS_UNKNOWN;

    int nRet = m_pDev->GetCardPosition(enCardPos);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "WaitItemTaken检测卡失败: %s,Taken标记归零.", GetErrorStr(nRet));
        m_WaitTaken = WTF_NONE;
        return hErrCodeChg(nRet);
    }

    if ((m_enCardPosition == CARDPOS_ENTERING) &&   // 卡在门口
        (enCardPos == CARDPOS_NOCARD))  // 无卡
    {
        FireMediaRemoved();
        m_WaitTaken = WTF_NONE;
    }

    return 0;
}


// ------事件处理----------------------------

// 硬件故障事件
void CXFS_IDX::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

// 状态变化事件
void CXFS_IDX::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

// 进卡事件
void CXFS_IDX::FireCardInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIAINSERTED, nullptr);
}

// 退卡/移走卡事件
void CXFS_IDX::FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIAREMOVED, nullptr);
}

//　吞卡事件
void CXFS_IDX::FireMediaRetained()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIARETAINED, nullptr);
}

// 回收相关事件
void CXFS_IDX::FireRetainBinThreshold(WORD wReBin)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_IDC_RETAINBINTHRESHOLD, (LPVOID)&wReBin);
}

// 复位时检测到卡事件
void CXFS_IDX::FireMediaDetected(WORD ResetOut)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIADETECTED, (LPVOID)&ResetOut);
}

// 出现无效磁道事件
void CXFS_IDX::FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData)
{
    WFSIDCTRACKEVENT data;
    data.fwStatus   = wStatus;
    data.lpstrTrack = pTrackName;
    data.lpstrData  = pTrackData;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDTRACKDATA, &data);
}


// ------INI处理----------------------------
// 读INI
int CXFS_IDX::InitConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD    wTmp = 0;
    CHAR    szTmp[BUF_SIZE256];

    // -----[default]下参数------------
    // 底层设备控制动态库名
    memset(&m_stIdxIniConfig, 0x00, sizeof(ST_IDX_INI_CONFIG));
    strcpy(m_stIdxIniConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));


    // -----[DEVICE_CONFIG]下参数------------
    // 设备类型(0/新北洋BS-ID81)
    m_stIdxIniConfig.wDeviceType = (WORD)m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (INT)0);


    // -----[OPEN_CONFIG]下参数------------
    // Open时是否执行Reset动作(0不执行/1执行,缺省0)
    m_stIdxIniConfig.wOpenResetSupp = (WORD)m_cXfsReg.GetValue("OPEN_CONFIG", "OpenResetSupp", (INT)0);
    if (m_stIdxIniConfig.wOpenResetSupp != 0 &&
        m_stIdxIniConfig.wOpenResetSupp != 1)
    {
        m_stIdxIniConfig.wOpenResetSupp = 0;
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stIdxIniConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (INT)0);
    if (m_stIdxIniConfig.nOpenFailRet != 0 && m_stIdxIniConfig.nOpenFailRet != 1)
    {
        m_stIdxIniConfig.nOpenFailRet = 0;
    }


    // -----[RESET_CONFIG]下参数------------
    // Reset时卡动作(0无动作/1退卡/2吞卡,缺省0)
    m_stIdxIniConfig.wResetCardAction = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetCardAction", CARDACTION_NOACTION);
    if (m_stIdxIniConfig.wResetCardAction != CARDACTION_NOACTION &&
        m_stIdxIniConfig.wResetCardAction != CARDACTION_EJECT &&
        m_stIdxIniConfig.wResetCardAction != CARDACTION_RETAIN)
    {
        m_stIdxIniConfig.wResetCardAction = CARDACTION_NOACTION;
    }

    // Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功,缺省0)
    m_stIdxIniConfig.wResetFailReturn = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetFailReturn", (INT)0);
    if (m_stIdxIniConfig.wResetFailReturn != 0 &&
        m_stIdxIniConfig.wResetFailReturn != 1)
    {
        m_stIdxIniConfig.wResetFailReturn = 0;
    }


    // -----[REJECT_CONFIG]下参数------------
    // 退卡时完全弹出/保持在门口(0保持在门口/1完全弹出,缺省0)
    m_stIdxIniConfig.wEjectMode = (WORD)m_cXfsReg.GetValue("EJECT_CONFIG", "EjectMode", (INT)0);
    if (m_stIdxIniConfig.wEjectMode != 0 &&
        m_stIdxIniConfig.wEjectMode != 1)
    {
        m_stIdxIniConfig.wEjectMode = 0;
    }
    m_stIdxIniConfig.stIdxInitParamInfo.wEjectMode = m_stIdxIniConfig.wEjectMode;

    // -----[RETAIN_CONFIG]下参数------------
    // 是否支持回收功能(0不支持/1支持,缺省0)
    m_stIdxIniConfig.wRetainSupp = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG(", "RetainSupp", (INT)0);
    if (m_stIdxIniConfig.wRetainSupp != 0 &&
        m_stIdxIniConfig.wRetainSupp != 1)
    {
        m_stIdxIniConfig.wRetainSupp = 0;
    }


    // -----[READRAWDATA_CONFIG]下参数------------
    // Readrawdata命令执行完成后是否自动退卡(0不支持/1支持,缺省0)
    m_stIdxIniConfig.wReadEndRunEject = (WORD)m_cXfsReg.GetValue("READRAWDATA_CONFIG", "ReadEndRunEject", (INT)0);
    if (m_stIdxIniConfig.wReadEndRunEject != 0 &&
        m_stIdxIniConfig.wReadEndRunEject != 1)
    {
        m_stIdxIniConfig.wReadEndRunEject = 0;
    }


    // -----[IMAGE_CONFIG]下参数------------
    // 证件头像保存路径
    strcpy(m_stIdxIniConfig.stIdxInitParamInfo.szHeadImgSavePath, m_cXfsReg.GetValue("IMAGE_CONFIG", "HeadImageSavePath", HEADIMAGE_SAVE_DEF));
    if (strlen(m_stIdxIniConfig.stIdxInitParamInfo.szHeadImgSavePath) < 1)
    {
        memset(m_stIdxIniConfig.stIdxInitParamInfo.szHeadImgSavePath, 0x00, 256);
        sprintf(m_stIdxIniConfig.stIdxInitParamInfo.szHeadImgSavePath,
                "%s/image", getenv("HOME"));
    }
    if (bPathCheckAndCreate(m_stIdxIniConfig.stIdxInitParamInfo.szHeadImgSavePath) != TRUE)
    {
        Log(ThisModule, -1, "bPathCheckAndCreate() 创建目录[%s]失败. Not Return Error. ",
            m_stIdxIniConfig.stIdxInitParamInfo.szHeadImgSavePath);
    }

    // 证件扫描保存路
    strcpy(m_stIdxIniConfig.stIdxInitParamInfo.szScanImgSavePath, m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageSavePath", SCANIMAGE_SAVE_DEF));
    if (strlen(m_stIdxIniConfig.stIdxInitParamInfo.szScanImgSavePath) < 1)
    {
        memset(m_stIdxIniConfig.stIdxInitParamInfo.szScanImgSavePath, 0x00, 256);
        sprintf(m_stIdxIniConfig.stIdxInitParamInfo.szScanImgSavePath,
                "%s/image", getenv("HOME"));
    }
    if (bPathCheckAndCreate(m_stIdxIniConfig.stIdxInitParamInfo.szScanImgSavePath) != TRUE)
    {
        Log(ThisModule, -1, "bPathCheckAndCreate() 创建目录[%s]失败. Not Return Error. ",
            m_stIdxIniConfig.stIdxInitParamInfo.szScanImgSavePath);
    }

    // 保存头像名(包含扩展名,不包含路径,空为不指定,用缺省值)
    strcpy(m_stIdxIniConfig.stIdxInitParamInfo.szHeadImgName, m_cXfsReg.GetValue("IMAGE_CONFIG", "HeadImageSaveName", ""));

    // 证件扫描正面图保存名(包含扩展名,不包含路径,空为不指定,用缺省值)
    strcpy(m_stIdxIniConfig.stIdxInitParamInfo.szScanImgFrontName, m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageFrontSaveName", ""));

    // 证件扫描背面图保存名(包含扩展名,不包含路径,空为不指定,用缺省值)
    strcpy(m_stIdxIniConfig.stIdxInitParamInfo.szScanImgBackName, m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageBackSaveName", ""));

    // 证件扫描图片保存格式(0BMP/1JPG,缺省1)
    m_stIdxIniConfig.stIdxInitParamInfo.wScanImgSaveType = (WORD)m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageSaveType", 1);

    // 证件扫描图片保存缩放放比例(0.1~3.0,缺省2.0)
    memset(szTmp, 0x00, sizeof(szTmp));
    strcpy(szTmp, m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageSavaZoomScale", "2.0"));
    m_stIdxIniConfig.stIdxInitParamInfo.m_fScanImgSaveZoomSc = QString(szTmp).toFloat();

    // 银行编号
    /* // 40-00-00-00(FT#0010) DELETE END
    m_stIdxIniConfig.stIdxInitParamInfo.wBankNo = (WORD)m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (INT)0);
    if (m_stIdxIniConfig.stIdxInitParamInfo.wBankNo < 0 && m_stIdxIniConfig.stIdxInitParamInfo.wBankNo > 1)
    {
        m_stIdxIniConfig.stIdxInitParamInfo.wBankNo = 0;
    }*/ // 40-00-00-00(FT#0010) DELETE END
    m_stIdxIniConfig.wBankNo = (WORD)m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (INT)0);   // 40-00-00-00(FT#0010)
    if (m_stIdxIniConfig.wBankNo < 0 && m_stIdxIniConfig.wBankNo > 1)                       // 40-00-00-00(FT#0010)
    {                                                                                       // 40-00-00-00(FT#0010)
        m_stIdxIniConfig.wBankNo = 0;                                                       // 40-00-00-00(FT#0010)
    }                                                                                       // 40-00-00-00(FT#0010)
    m_stIdxIniConfig.stIdxInitParamInfo.wBankNo = m_stIdxIniConfig.wBankNo;                 // 40-00-00-00(FT#0010)

    return WFS_SUCCESS;
}

// lpspath: 绝对路径； bFolder:是否不包含文件名的目录路径
BOOL CXFS_IDX::bPathCheckAndCreate(LPSTR lpsPath, BOOL bFolder)
{
    char    szNewDir[MAX_PATH] = "\0";
    char    szBufDir[MAX_PATH] = "\0";
    char    *pBuf;

    int len = strlen(lpsPath);
    int i = 0;
    strcpy(szNewDir, lpsPath);
    if (bFolder == FALSE)   // 最后为文件名，去掉文件名，转换为目录路径
    {
        for (i = len; i > 0; i--)
        {
            if(lpsPath[i] == '/')
            {
                break;
            }
        }
        if (i > 0)
        {
            szNewDir[i] = '\0';
        }
    }

    pBuf = strtok(szNewDir + 1, "/");
    while (pBuf)
    {
        strcat(szBufDir,"/");
        strcat(szBufDir,pBuf);

        if (access(szBufDir, F_OK) != 0)
        {
            if (mkdir(szBufDir, S_IRWXU | S_IRWXG | S_IROTH) != 0)
            {
                return FALSE;
            }
        }

        pBuf = strtok(NULL, "/");
    }

    return TRUE;
}


// Open设备及初始化相关子处理
HRESULT CXFS_IDX::StartOpen()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    m_pDev->SetData(&(m_stIdxIniConfig.stIdxInitParamInfo), DATATYPE_INIT);

    // 打开连接
    hRet = m_pDev->Open(nullptr);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败．ReturnCode:%d", hRet);
        return hErrCodeChg(hRet);
    }

    // Reset
    if (m_stIdxIniConfig.wOpenResetSupp == 1)
    {
        HRESULT hRet = ResetExecute((CardAction)m_stIdxIniConfig.wResetCardAction);
        if (hRet != WFS_SUCCESS)
        {

            if (m_stIdxIniConfig.wResetFailReturn == 0)
            {
                Log(ThisModule, __LINE__, "打开设备连接后Reset失败．Return Error. ");
                return hRet;
            } else
            {
                Log(ThisModule, __LINE__, "打开设备连接后Reset失败．Not Return Error. ");
            }
        }
    }

    // 更新扩展状态
    CHAR szDevIDXVer[BUF_SIZE32] = { 0x00 };
    m_pDev->GetVersion(szDevIDXVer, sizeof(szDevIDXVer) - 1, 1);

    CHAR szFWVer[BUF_SIZE32] = { 0x00 };
    m_pDev->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

    CHAR szSWVer[BUF_SIZE32] = { 0x00 };
    m_pDev->GetVersion(szSWVer, sizeof(szSWVer) - 1, 3);

    m_cExtra.AddExtra("VRTCount", "4");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", szDevIDXVer);
    m_cExtra.AddExtra("VRTDetail[02]", szFWVer);
    m_cExtra.AddExtra("VRTDetail[03]", szSWVer);

    // Capabilities Status　初始化
    InitCaps();
    InitStatus();

    // 更新一次状态
    OnStatus();

    Log(ThisModule, 1, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());

    return WFS_SUCCESS;
}
