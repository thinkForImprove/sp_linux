/***************************************************************
* 文件名称：XFS_UKEY.cpp
* 文件描述：UKEY发放模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月24日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_UKEY.h"

//-----------------------------------------------------------------------------------
//-----------------------------------Open及初始化处理----------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_UKEY::StartOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    INT nRet = 0;

    if (bReConn == FALSE)   // 非重连状态,避免重复调用
    {
        // Open前下传初始参数
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pDev->SetData(m_stConfig.szSDKPath, DTYPE_LIB_PATH);
        }

        // 指定设备设备下传初始参数
        if (m_stConfig.nDriverType == IXFSUKEY_TYPE_ACTU6SS39)
        {
            ;
        }
    }

    // 打开连接
    nRet = m_pDev->Open(m_stConfig.szDeviceConList);
    if (nRet != CRD_SUCCESS)
    {
        if (bReConn == FALSE)   // 非重连状态
        {
            Log(ThisModule, __LINE__, "打开设备连接: ->Open(%s) Fail, ErrCode: %d,ReturnCode:%d.",
                m_stConfig.szDeviceConList, nRet, ConvertErrCode(nRet));
        } else
        {
            if (m_nReConRet != nRet)
            {
                Log(ThisModule, __LINE__, "断线重连:打开设备连接失败．ReturnCode:%d.", ConvertErrCode(nRet));
                m_nReConRet = nRet;
            };
        }

        return ConvertErrCode(nRet);
    }

    if (bReConn == FALSE)   // 非重连状态(Open时动作)
    {
        hRet = InitOpen();
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "设备打开: 初始化相关处理: ->InitOpen() Fail, ReturnErr: %d．ReturnCode:%d.",
                hRet, hRet);
            return nRet;
        }
    }

    // 更新扩展状态
    CHAR szDevVer[128] = { 0x00 };
    m_pDev->GetVersion(szDevVer, sizeof(szDevVer) - 1, 1);

    CHAR szFWVer[128] = { 0x00 };
    m_pDev->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

    m_cExtra.AddExtra("VRTCount", "1");
    //m_cExtra.AddExtra("VRT[00]_XFSCPR", (char*)byVRTU);
    //m_cExtra.AddExtra("VRT[01]_DevCPR", szDevVer);
    m_cExtra.AddExtra("FirmwareVersion", szFWVer);
    //m_cExtra.AddExtra("LastErrorCode", "");
    //m_cExtra.AddExtra("LastErrorDetail", "");

    // 更新一次状态
    OnStatus();

    if (bReConn == FALSE)   // 非重连状态
    {
        Log(ThisModule, 1, "打开设备连接成功, IDCExtra=%s.", m_cExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, 1, "断线重连:打开设备连接成功, IDCExtra=%s.", m_cExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}

// Open设备初始化相关子处理
// 如果出现错误需要终止执行,return;不需要,不加return
HRESULT CXFS_UKEY::InitOpen()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;
    INT nRet = CRD_SUCCESS;

    if (m_stConfig.usOpenResetSup > 0)  // Open执行Reset
    {
        USHORT usAction = 0;
        if (m_stConfig.usOpenResetSup == 1)     // 无动作
        {
            usAction = WFS_CRD_NOACTION;
        } else
        if (m_stConfig.usOpenResetSup == 2)     // 回收
        {
            usAction = WFS_CRD_RETAIN;
        } else
        if (m_stConfig.usOpenResetSup == 3)     // 弹卡
        {
            usAction = WFS_CRD_EJECT;
        }

        hRet = InnerReset(usAction, 0);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开设备后Reset[%d]: ->InnerReset(%d) Fail, ErrCode: %d,ReturnCode:%d.",
                m_stConfig.usOpenResetSup, usAction, hRet);
        }
    }

    // 设置报文 命令下发超时时间
    if (m_stConfig.uiSndTimeOut > 0)
    {
        m_pDev->SetData(&(m_stConfig.uiSndTimeOut), 1, DTYPE_SET_SNDTIMEOUT);
    }

    // 设置报文 命令接收超时时间
    if (m_stConfig.uiRcvTimeOut > 0)
    {
        m_pDev->SetData(&(m_stConfig.uiRcvTimeOut), 1, DTYPE_SET_RCVTIMEOUT);
    }

    // 设置掉电后处理
    if (m_stConfig.usPowerOffMode > 0)
    {
        nRet = m_pDev->SetData(nullptr, m_stConfig.usPowerOffMode, DTYPE_SET_POWEROFFMODE);
        if (nRet != CRD_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开设备后: 设置掉电后处理: ->SetData(null, %d, %d) Fail, ErrCode: %d, Not ReturnCode:%d.",
                m_stConfig.usPowerOffMode, DTYPE_SET_POWEROFFMODE, nRet, ConvertErrCode(nRet));
            // return ConvertErrCode(nRet);
        }
    }

    // 取硬件参数
    /*if (m_stConfig.nDriverType == IXFSUKEY_TYPE_ACTU6SS39)
    {
        INT nSize = sizeof(m_stConfig.stCfg_ACTU6SS39.szDevParam);
        nRet = m_pDev->GetData(m_stConfig.stCfg_ACTU6SS39.szDevParam, &nSize, DTYPE_GET_DEVPARAM);
        if (nRet != CRD_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开设备后: 取硬件参数: ->GetData(%d) Fail, ErrCode: %d, Not ReturnCode:%d.",
                DTYPE_GET_DEVPARAM, nRet, ConvertErrCode(nRet));
            // return ConvertErrCode(nRet);
        } else
        {
            Log(ThisModule, __LINE__, "打开设备后: 取硬件参数: ->GetData(%d) Succ, ErrCode: %d, "
                                      "Param: %02X %02X %02X %02X %02X.",
                DTYPE_GET_DEVPARAM, nRet, m_stConfig.stCfg_ACTU6SS39.szDevParam[0],
                m_stConfig.stCfg_ACTU6SS39.szDevParam[1],m_stConfig.stCfg_ACTU6SS39.szDevParam[2],
                m_stConfig.stCfg_ACTU6SS39.szDevParam[3],m_stConfig.stCfg_ACTU6SS39.szDevParam[4]);
        }
    }*/

    return hRet;
}

// 读INI配置
int CXFS_UKEY::InitConfig()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR    szIniAppName[256];
    CHAR    szKeyName[256];
    CHAR    szBuffer[65536];
    INT     nCount = 0;

    // 底层设备控制动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型(0/)
    m_stConfig.nDriverType = m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (INT)0);

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.usOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (INT)0);
    if (m_stConfig.usOpenFailRet != 0 && m_stConfig.usOpenFailRet != 1)
    {
        m_stConfig.usOpenFailRet = 0;
    }

    // Open时Reset设置(0不执行Reset/1Reset无动作/2Reset回收/3Reset弹卡,缺省0)
    m_stConfig.usOpenResetSup = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenResetSup", (INT)0);
    if (m_stConfig.usOpenResetSup > 3)
    {
        m_stConfig.usOpenResetSup = 0;
    }

    // 介质在出口时是否退回重读(0不支持/1支持,缺省0）
    m_stConfig.usEjectAgainRead = m_cXfsReg.GetValue("READRAWDATA_CONFIG", "EjectAgainRead", (INT)0);
    if (m_stConfig.usEjectAgainRead > 1)
    {
        m_stConfig.usEjectAgainRead = 0;
    }


    // ----------------------------------------设备分类相关信息获取----------------------------------------
    // 根据设备类型获取相关参数
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDriverType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    // 1. 设备连接串,缺省(COM:/dev/ttyS0:E,8,1)
    strcpy(m_stConfig.szDeviceConList, m_cXfsReg.GetValue(szIniAppName, "DeviceConList", ""));

    // 命令下发超时时间(单位:毫秒,缺省:3000)
    m_stConfig.uiSndTimeOut = m_cXfsReg.GetValue(szIniAppName, "SndCmdTimeOut", (INT)0);

    // 命令接收超时时间(单位:毫秒,缺省:5000)
    m_stConfig.uiRcvTimeOut = m_cXfsReg.GetValue(szIniAppName, "RcvCmdTimeOut", (INT)0);

    // 断电后功能模式(0不支持/1无动作/2回收/3弹KEY,缺省0)
    m_stConfig.usPowerOffMode = m_cXfsReg.GetValue(szIniAppName, "PowerOffMode", (INT)0);
    if (m_stConfig.usPowerOffMode > 3)
    {
        m_stConfig.usPowerOffMode = 0;
    }

    // 获取类型=0设备INI设置
    if (m_stConfig.nDriverType == IXFSUKEY_TYPE_ACTU6SS39)
    {
        //memset(szIniAppName, 0x00, sizeof(szIniAppName));
        //sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDriverType);
    }

    // ----------------------------------------UKEY箱相关信息获取----------------------------------------
    // --------读UKEY箱相关参数-------
    // 卡箱数目
    m_stUKEYBoxList.usBoxCount = m_cXfsReg.GetValue("DISPBOX_CONFIG", "DispBoxCount", (INT)0);

    // 读指定卡箱箱相关参数
    for (INT i = 0; i < DISPBOX_COUNT; i ++)
    {
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DISPBOX_%d", i + 1);

        // 卡箱索引号
        m_stUKEYBoxList.stDispBox[i].usNumber = i + 1;
        // 卡箱是否存在/使用
        m_stUKEYBoxList.stDispBox[i].bIsHave = (m_cXfsReg.GetValue(szIniAppName, "Useing", (INT)0) == 0 ? FALSE : TRUE);
        // 索引号[卡箱在UNITINFO中的唯一标识]
        //m_stUKEYBoxList.stDispBox[i].usNumber = m_cXfsReg.GetValue(szIniAppName, "BoxNumber", (INT)0);
        // 卡箱类型(0发卡箱/1回收箱)
        m_stUKEYBoxList.stDispBox[i].usBoxType = m_cXfsReg.GetValue(szIniAppName, "BoxType", (INT)0);
        // 卡箱内卡类型
        strcpy(m_stUKEYBoxList.stDispBox[i].szThingName, m_cXfsReg.GetValue(szIniAppName, "ThingType", ""));
        // 初始卡数目
        m_stUKEYBoxList.stDispBox[i].usInitCnt = m_cXfsReg.GetValue(szIniAppName, "InitCnt", (INT)0);
        // 当前卡数目
        m_stUKEYBoxList.stDispBox[i].usCount = m_cXfsReg.GetValue(szIniAppName, "Count", (INT)0);
        // 进入回收箱数目
        m_stUKEYBoxList.stDispBox[i].usRetainCnt = m_cXfsReg.GetValue(szIniAppName, "RetainCnt", (INT)0);
        // 报警阀值(LOW/HIGH)
        m_stUKEYBoxList.stDispBox[i].usThreshold = m_cXfsReg.GetValue(szIniAppName, "Threshold", (INT)0);
        // 是否基于硬件传感器生成阀值事件(0否/1是)
        m_stUKEYBoxList.stDispBox[i].bHardSensor = m_cXfsReg.GetValue(szIniAppName, "HardSensorEvent", (INT)0);
        m_stUKEYBoxList.stDispBox[i].usStatus = UNITINFO_STAT_UNKNOWN; // 卡单元初始状态设置为无法确定
    }
    memcpy(&m_stUKEYBoxListOld, &m_stUKEYBoxList, sizeof(STDISPBOXLIST));


    // ----------------------------------------银行分类相关信息获取----------------------------------------
    // 根据银行获取特殊设置
    m_stConfig.usBank = m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (INT)0);

}

// 初始化 IDC Status应答类
void CXFS_UKEY::InitIDCStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stIDCStatus.fwDevice    = WFS_IDC_DEVNODEVICE;
    m_stIDCStatus.fwMedia     = WFS_IDC_MEDIAUNKNOWN;
    m_stIDCStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    m_stIDCStatus.fwSecurity  = WFS_IDC_SECNOTSUPP;
    m_stIDCStatus.usCards     = 0;
    m_stIDCStatus.fwChipPower = WFS_IDC_CHIPNOTSUPP;
    m_stIDCStatus.lpszExtra   = (LPSTR)m_cExtra.GetExtra();;

    m_stIDCStatusOLD.Copy(m_stIDCStatus);   // 当前状态备份到OLD
}

// 初始化 IDC Caps应答类
void CXFS_UKEY::InitIDCCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    USHORT usRetainMaxCount = 0;
    if (m_stUKEYBoxList.GetRetainNum() == DISPBOX_RETNO)
    {
        usRetainMaxCount = m_stUKEYBoxList.GetDispBoxData(m_stUKEYBoxList.GetRetainNum(), DISPRWT_MAXCNT);
    }

    m_stIDCCaps.wClass          = WFS_SERVICE_CLASS_IDC;
    m_stIDCCaps.fwType          = WFS_IDC_TYPEMOTOR;
    m_stIDCCaps.bCompound       = false;
    m_stIDCCaps.fwReadTracks    = WFS_IDC_TRACK3;
    m_stIDCCaps.fwWriteTracks   = WFS_IDC_NOTSUPP;
    m_stIDCCaps.fwChipProtocols = WFS_IDC_NOTSUPP;
    m_stIDCCaps.usCards         = usRetainMaxCount;
    m_stIDCCaps.fwSecType       = WFS_IDC_SECNOTSUPP;
    m_stIDCCaps.fwPowerOnOption = WFS_IDC_NOACTION;
    m_stIDCCaps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_stIDCCaps.bFluxSensorProgrammable = false;
    m_stIDCCaps.bReadWriteAccessFollowingEject = false;
    m_stIDCCaps.fwWriteMode     = WFS_IDC_NOTSUPP;
    m_stIDCCaps.fwChipPower     = WFS_IDC_NOTSUPP;
    m_stIDCCaps.lpszExtra       = (LPSTR)m_cExtra.GetExtra();
}

// 初始化 CRD Status应答类
void CXFS_UKEY::InitCRDStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stCRDStatusOLD.Copy(m_stCRDStatus);
    m_stCRDStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
}

// 初始化 CRD Caps应答类
void CXFS_UKEY::InitCRDCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stCRDCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();

}


//-----------------------------------------------------------------------------------
//--------------------------------------功能处理---------------------------------------
// 取状态
INT CXFS_UKEY::UpdateDevStatus()
{
    THISMODULE(__FUNCTION__);

    INT nRet = CRD_SUCCESS;
    STCRDDEVSTATUS stStatus;

    // 取状态
    if((nRet = m_pDev->GetDevStat(stStatus)) != CRD_SUCCESS)
    {
        /*Log(ThisModule, __LINE__, "取状态: ->GetDevStat() Fail, ErrCode=%d, Return %d.",
            nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);*/

        if (nRet == ERR_CRD_NOT_OPEN)   // 未Open,重连
        {
            StartOpen(TRUE);    // 重连
            nRet = m_pDev->GetDevStat(stStatus);
        }
    }

    // CRD: 状态
    m_stCRDStatus.fwDevice = ConvertDeviceStatus(stStatus.wDevice);                     // 设备状态
    m_stCRDStatus.fwDispenser = ConvertCRDTransportStatus(stStatus.wDispensr);          // 单元状态
    m_stCRDStatus.fwTransport = ConvertCRDDispensrStatus(stStatus.wTransport);          // 传输状态
    m_stCRDStatus.fwMedia = ConvertMediaStatus(stStatus.wMedia);                        // 介质状态
    m_stCRDStatus.fwShutter = ConvertCRDShutterStatus(stStatus.wShutter);               // 门状态
    m_stCRDStatus.wDevicePosition = ConvertCRDDevicePosStatus(stStatus.wDevicePos);     // 设备位置状态
    m_stCRDStatus.wAntiFraudModule = ConvertCRDAntiFraudStatus(stStatus.wAntiFraudMod); // 反欺诈模块状态

    // IDC状态
    m_stIDCStatus.fwDevice = ConvertDeviceStatus(stStatus.wDevice, CONVERT_IDC);        // 设备状态
    m_stIDCStatus.fwMedia = ConvertMediaStatus(stStatus.wMedia, CONVERT_IDC);           // 介质状态
    m_stIDCStatus.usCards = m_stUKEYBoxList.GetDispBoxData(m_stUKEYBoxList.GetRetainNum(), DISPRWT_COUNT);   // 回收计数
    m_stIDCStatus.fwRetainBin = ConvertDBoxStat2WFS(
                m_stUKEYBoxList.GetDispBoxData(m_stUKEYBoxList.GetRetainNum(), DISPRWT_STATUS), CONVERT_IDC);// 回收状态

    // 检查DevCRD返回卡箱类型是否与INI设置一致
    // ????????

    // 更新卡箱状态
    for (INT i = 1; i <= CARDBOX_COUNT; i ++)   // 箱索引号(1~16)
    {
        if (stStatus.GetUnitType(i) != UNIT_NOTSUPP)    // 非不支持
        {
            m_stUKEYBoxList.SetDispBoxData(i, ConvertCRDUnitStat2DBoxStat(stStatus.GetUnitStatus(i)), DISPRWT_STATUS);
            if (stStatus.GetUnitStatus(i) == UNITINFO_STAT_EMPTY && // DevCRD检测到卡单元空
                stStatus.GetUnitType(i) == UNIT_STORAGE)            // 存储箱
            {
                //m_stUKEYBoxList.SetDispBoxData(i, 0, DISPRWT_COUNT);    // 设置对应卡单元计数=0
            }
        }
    }

    // ---------------------------Event 处理---------------------------
    if (m_enWaitTaken == WTF_TAKEN)    // 准备Taken事件
    {
        if (/*m_stCRDStatusOLD.fwMedia == WFS_CRD_MEDIAEXITING &&*/     // 上一次状态为卡口有卡
            m_stCRDStatus.fwMedia == WFS_CRD_MEDIANOTPRESENT)       // 当前状态为卡口无卡
        {
            CRD_FireMediaRemoved();
            IDC_FireMediaRemoved();
            m_enWaitTaken = WTF_NONE;
        }
    }
    // 设备位置变化
    if (m_stCRDStatus.wDevicePosition != WFS_CRD_DEVICEPOSNOTSUPP &&
        m_stCRDStatusOLD.wDevicePosition != WFS_CRD_DEVICEPOSNOTSUPP)    // 非不支持状态
    {
        if (m_stCRDStatus.wDevicePosition != m_stCRDStatusOLD.wDevicePosition)
        {
            LPWFSCRDDEVICEPOSITION lpWfsDevicePos = new WFSCRDDEVICEPOSITION();
            lpWfsDevicePos->wPosition = m_stCRDStatus.wDevicePosition;
            CRD_FireDEvicePosition(lpWfsDevicePos);
        }
    }
    // 卡单元信息变化检查并上报Event:WFS_SRVE_CRD_CARDUNITINFOCHANGED
    CRD_FireUnitInfoChanged_Pack();
    // 卡单元阀值条件复合,上报Event:WFS_USRE_CRD_CARDUNITTHRESHOLD
    CRD_FireUnitThresHold_Pack();

    // IDC事件上报
    if (m_stIDCStatusOLD.fwDevice != m_stIDCStatus.fwDevice)
    {
        IDC_FireStatusChanged(m_stIDCStatus.fwDevice);      // 上报状态变化事件
        if (m_stIDCStatus.fwDevice != WFS_IDC_DEVONLINE &&
                m_stIDCStatus.fwDevice != WFS_IDC_DEVBUSY)
        {
            IDC_FireHWEvent(WFS_ERR_ACT_NOACTION, nullptr); // 上报HWERR事件
        }
    }

    // 状态变化记录到Log
    if (m_stCRDStatusOLD.Diff(m_stCRDStatus) == true)
    {
        Log(ThisModule, -1, "CRD状态结果比较: Device:%d->%d%s|Dispenser:%d->%d%s|Transport:%d->%d%s|"
                            "Media:%d->%d%s|Shutter:%d->%d%s|DevicePosition:%d->%d%s|; 事件上报记录:.",
            m_stCRDStatusOLD.fwDevice, m_stCRDStatus.fwDevice,
                (m_stCRDStatusOLD.fwDevice != m_stCRDStatus.fwDevice ? " *" : " "),
            m_stCRDStatusOLD.fwDispenser, m_stCRDStatus.fwDispenser,
                (m_stCRDStatusOLD.fwDispenser != m_stCRDStatus.fwDispenser ? " *" : " "),
            m_stCRDStatusOLD.fwTransport, m_stCRDStatus.fwTransport,
                (m_stCRDStatusOLD.fwTransport != m_stCRDStatus.fwTransport ? " *" : " "),
            m_stCRDStatusOLD.fwMedia, m_stCRDStatus.fwMedia,
                (m_stCRDStatusOLD.fwMedia != m_stCRDStatus.fwMedia ? " *" : " "),
            m_stCRDStatusOLD.fwShutter, m_stCRDStatus.fwShutter,
                (m_stCRDStatusOLD.fwShutter != m_stCRDStatus.fwShutter ? " *" : " "),
            m_stCRDStatusOLD.wDevicePosition, m_stCRDStatus.wDevicePosition,
                (m_stCRDStatusOLD.wDevicePosition != m_stCRDStatus.wDevicePosition ? " *" : " "));
    }
    m_stCRDStatusOLD.Copy(m_stCRDStatus);   // 当前状态备份到OLD

    if (m_stIDCStatusOLD.Diff(m_stIDCStatus) == true)
    {
        Log(ThisModule, -1, "IDC状态结果比较: Device:%d->%d%s|Media:%d->%d%s|RetainBin:%d->%d%s|"
                            "Security:%d->%d%s|usCards:%d->%d%s|; 事件上报记录:.",
            m_stIDCStatusOLD.fwDevice, m_stIDCStatus.fwDevice,
                (m_stIDCStatusOLD.fwDevice != m_stIDCStatus.fwDevice ? " *" : " "),
            m_stIDCStatusOLD.fwMedia, m_stIDCStatus.fwMedia,
                (m_stIDCStatusOLD.fwMedia != m_stIDCStatus.fwMedia ? " *" : " "),
            m_stIDCStatusOLD.fwRetainBin, m_stIDCStatus.fwRetainBin,
                (m_stIDCStatusOLD.fwRetainBin != m_stIDCStatus.fwRetainBin ? " *" : " "),
            m_stIDCStatusOLD.fwSecurity, m_stIDCStatus.fwSecurity,
                (m_stIDCStatusOLD.fwSecurity != m_stIDCStatus.fwSecurity ? " *" : " "),
            m_stIDCStatusOLD.usCards, m_stIDCStatus.usCards,
                (m_stIDCStatusOLD.usCards != m_stIDCStatus.usCards ? " *" : " "));
    }
    m_stIDCStatusOLD.Copy(m_stIDCStatus);   // 当前状态备份到OLD

    // 陕西信合特殊处理: 状态OFFLINE 显示为 HWERROR
    if (m_stConfig.usBank == BANK_SXXH)
    {
        if (m_stCRDStatus.fwDevice == WFS_CRD_DEVOFFLINE)
        {
            m_stCRDStatus.fwDevice = WFS_CRD_DEVHWERROR;
        }
        if (m_stIDCStatus.fwDevice == WFS_IDC_DEVOFFLINE)
        {
            m_stIDCStatus.fwDevice = WFS_IDC_DEVHWERROR;
        }
    }

}

// 取UnitInfo子处理(Info)
HRESULT CXFS_UKEY::InnerGetCardUnitInfo(LPWFSCRDCUINFO &lpCardUnit)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    UpdateDevStatus();   // 更新一次状态,保证取到的是最新的信息

    // 通过INI卡箱列表+设备卡箱状态组织应答
    LPWFSCRDCUINFO lpWfsCrdCUUnit = nullptr;
    LPWFSCRDCARDUNIT lpWfsCrdUnit = nullptr;
    INT nCnt = 0;
    USHORT usBoxNum = 0;

    // 卡箱数目
    lpWfsCrdCUUnit = new WFSCRDCUINFO();
    lpCardUnit = lpWfsCrdCUUnit;
    lpWfsCrdCUUnit->usCount = (USHORT)m_stUKEYBoxList.GetDispBoxCount();
    if (lpWfsCrdCUUnit->usCount < 1)
    {
        lpWfsCrdCUUnit->lppList = nullptr;
    } else
    {
        lpWfsCrdCUUnit->lppList = new LPWFSCRDCARDUNIT[lpWfsCrdCUUnit->usCount];

        // 顺序查询所有箱信息,取标记为HAVA的信息返回
        for(INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (m_stUKEYBoxList.GetDispBoxOrderIsHave(i) == DISPBOX_RETNO)
            {
                continue;   // 指定顺序号的箱不存在
            }

            usBoxNum = m_stUKEYBoxList.GetDispBoxOrderIsNumber(i);// 指定箱顺序号返回箱索引号

            lpWfsCrdCUUnit->lppList[i] = new WFSCRDCARDUNIT();
            lpWfsCrdUnit = lpWfsCrdCUUnit->lppList[i];

            memset(lpWfsCrdUnit, 0x00, sizeof(WFSCRDCARDUNIT));
            // 卡单元结构索引号(1~N)
            lpWfsCrdUnit->usNumber = usBoxNum;
            // 卡单元类型
            nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_TYPE);
            lpWfsCrdUnit->usType = (nCnt == DISPBOX_STORAGE ? WFS_CRD_SUPPLYBIN : WFS_CRD_RETAINBIN);
            // 卡单元中卡初始数目
            nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_INITCNT);
            lpWfsCrdUnit->ulInitialCount = (nCnt < 0 ? 0 : nCnt);
            // 卡单元中卡当前数目
            nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_COUNT);
            lpWfsCrdUnit->ulCount = (nCnt < 0 ? 0 : nCnt);
            // 卡单元进入回收箱中卡数目
            nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_RETCNT);
            lpWfsCrdUnit->ulRetainCount = (ULONG)(nCnt < 0 ? 0 : nCnt);
            // 阀值
            nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_THREH);
            lpWfsCrdUnit->ulThreshold = (ULONG)(nCnt < 0 ? 0 : nCnt);
            // 卡箱状态
            lpWfsCrdUnit->usStatus = ConvertDBoxStat2WFS(m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_STATUS));
            // 是否基于硬件传感器生成阀值事件
            nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_HARDSR);
            lpWfsCrdUnit->bHardwareSensor = (ULONG)(nCnt == 0 ? false : true);
            // 卡箱中卡类型
            nCnt = strlen(m_stUKEYBoxList.GetDispBoxCardType(usBoxNum));
            if (nCnt < 1)
            {
                lpWfsCrdUnit->lpszCardName = nullptr;
            } else
            {
                lpWfsCrdUnit->lpszCardName = new CHAR[nCnt + 1];
                memset(lpWfsCrdUnit->lpszCardName, 0x00, nCnt + 1);
                memcpy(lpWfsCrdUnit->lpszCardName, m_stUKEYBoxList.GetDispBoxCardType(usBoxNum), nCnt);
            }

            // 回收箱空时状态为OK
            if (lpWfsCrdUnit->usType == WFS_CRD_RETAINBIN && lpWfsCrdUnit->usStatus == WFS_CRD_STATCUEMPTY)
            {
                lpWfsCrdUnit->usStatus = WFS_CRD_STATCUOK;
            }
        }
    }

    return WFS_SUCCESS;
}

// 设置UnitInfo子处理(Info)
HRESULT CXFS_UKEY::InnerSetCardUnitInfo(LPWFSCRDCUINFO lpCuInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 入参数据检查
    for (INT i = 0; i < lpCuInfo->usCount; i ++)
    {
        if (lpCuInfo->lppList[i] == nullptr)
        {
            Log(ThisModule, __LINE__, "Input Param: 箱信息集合数目: usCount[%d] > 0, 箱信息集合BUFF: lppList[%d] == NULL, Return: %d.",
                lpCuInfo->usCount, i, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    if (lpCuInfo->usCount > m_stUKEYBoxList.GetDispBoxCount()) //
    {
        Log(ThisModule, __LINE__, "Input Param: 箱信息集合数目: usCount = %d, SP支持箱数目: BoxCount = %d, 超出上限, Return: %d.",
            lpCuInfo->usCount, m_stUKEYBoxList.GetDispBoxCount(), WFS_ERR_SOFTWARE_ERROR);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    // Check 入参箱类型和INI设定是否一致
    for(INT i = 0; i < lpCuInfo->usCount; i ++)
    {
        // 卡单元结构索引号(1~N): UKEY箱
        if (lpCuInfo->lppList[i]->usType == WFS_CRD_SUPPLYBIN &&
            m_stUKEYBoxList.GetDispBoxIsHave(lpCuInfo->lppList[i]->usNumber, DISPBOX_STORAGE) == DISPBOX_RETNO)
        {
            Log(ThisModule, __LINE__, "Input Param: lppList[%d].usNumber[%d] -> 指定索引的发UKEY箱不存在, Return: %d.",
                i, lpCuInfo->lppList[i]->usNumber, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
        // 卡单元结构索引号(1~N): 回收箱
        if (lpCuInfo->lppList[i]->usType == WFS_CRD_RETAIN &&
            m_stUKEYBoxList.GetDispBoxIsHave(lpCuInfo->lppList[i]->usNumber, DISPBOX_RETRACT) == DISPBOX_RETNO)
        {
            Log(ThisModule, __LINE__, "Input Param: lppList[%d].usNumber[%d] -> 指定索引的回收箱不存在, Return: %d.",
                i, lpCuInfo->lppList[i]->usNumber, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    // 设置卡箱信息并更新INI
    INT nCnt = 0;
    INT nNumber = 0;
    LPWFSCRDCARDUNIT lpCardUnit = nullptr;
    for(INT i = 0; i < lpCuInfo->usCount; i ++)
    {
        lpCardUnit = (LPWFSCRDCARDUNIT)lpCuInfo->lppList[i];
        nNumber = lpCardUnit->usNumber;

        // 卡单元类型
        nCnt = (lpCardUnit->usType == WFS_CRD_SUPPLYBIN ? DISPBOX_STORAGE : DISPBOX_RETRACT);
        m_stUKEYBoxList.SetDispBoxData(nNumber, nCnt, DISPRWT_TYPE);
        SetDispBoxCfg(nNumber, nCnt, DISPRWT_TYPE);
        // 卡单元中卡初始数目
        m_stUKEYBoxList.SetDispBoxData(nNumber, lpCardUnit->ulInitialCount, DISPRWT_INITCNT);
        SetDispBoxCfg(nNumber, lpCardUnit->ulInitialCount, DISPRWT_INITCNT);
        // 卡单元中卡当前数目
        m_stUKEYBoxList.SetDispBoxData(nNumber, lpCardUnit->ulCount, DISPRWT_COUNT);
        SetDispBoxCfg(nNumber, lpCardUnit->ulCount, DISPRWT_COUNT);
        // 卡单元进入回收箱中卡数目
        m_stUKEYBoxList.SetDispBoxData(nNumber, lpCardUnit->ulRetainCount, DISPRWT_RETCNT);
        SetDispBoxCfg(nNumber, lpCardUnit->ulRetainCount, DISPRWT_RETCNT);
        // 阀值
        m_stUKEYBoxList.SetDispBoxData(nNumber, lpCardUnit->ulThreshold, DISPRWT_THREH);
        SetDispBoxCfg(nNumber, lpCardUnit->ulThreshold, DISPRWT_THREH);
        // 是否基于硬件传感器生成阀值事件
        nCnt = (lpCardUnit->bHardwareSensor == false ? 0 : 1);
        m_stUKEYBoxList.SetDispBoxData(nNumber, nCnt, DISPRWT_HARDSR);
        SetDispBoxCfg(nNumber, nCnt, DISPRWT_HARDSR);
        // 卡箱中卡类型
        if (lpCardUnit->lpszCardName != nullptr && strlen(lpCardUnit->lpszCardName) > 0)
        {
            m_stUKEYBoxList.SetDispBoxData(nNumber, lpCardUnit->lpszCardName);
            SetDispBoxCfg(nNumber, lpCardUnit->lpszCardName, DISPRWT_THINGTP);
        } else
        {
            m_stUKEYBoxList.SetDispBoxData(nNumber, "");
            SetDispBoxCfg(nNumber, "", DISPRWT_THINGTP);
        }

        CRD_FireUnitInfoChanged_Pack();// 卡单元信息变化检查并上报Event:WFS_SRVE_CRD_CARDUNITINFOCHANGED
    }

    return WFS_SUCCESS;
}

// 发卡子处理(CMD)
HRESULT CXFS_UKEY::InnerDispenseCard(USHORT usBoxNum, BOOL bPresent)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    /* Return WFS Event:
     * WFS_EXEE_CRD_CARDUNITERROR: 在分发操作中引起卡单元错误
     * WFS_USRE_CRD_CARDUNITTHRESHOLD: 卡单元已达到阀值
     * WFS_SRVE_CRD_MEDIAREMOVED: 卡已被用户拿走
     */

    INT nRet = CRD_SUCCESS;
    INT nCardCount = 0;  // 保存卡单元数目

    LPWFSCRDCUERROR lpCUError = nullptr;

    UpdateDevStatus();   // 更新一次状态

    // 回收箱满检查
    if (m_stIDCStatus.fwRetainBin == WFS_IDC_RETAINBINFULL)
    {
        Log(ThisModule, __LINE__, "发卡: 回收箱检查: FULL, Return: %d.",
            WFS_ERR_CRD_RETAINBINFULL);

        return WFS_ERR_CRD_RETAINBINFULL;
    }

    // 卡箱索引Check
    if (m_stUKEYBoxList.GetDispBoxIsHave(usBoxNum, DISPBOX_STORAGE) == DISPBOX_RETNO)
    {
        Log(ThisModule, __LINE__, "发卡: Check BoxNo[%d] Is Invalid: 箱号不存在, Return: %d.",
            usBoxNum, WFS_ERR_CRD_INVALIDCARDUNIT);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(usBoxNum, WFS_CRD_CARDUNITINVALID);

        return WFS_ERR_CRD_INVALIDCARDUNIT;
    }

    // 卡箱类型Check
    if (m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_TYPE) == DISPBOX_RETRACT)
    {
        Log(ThisModule, __LINE__, "发卡: Check BoxNo[%d] Is Invalid: 指定箱号是回收箱, Return: %d.",
            usBoxNum, WFS_ERR_CRD_INVALIDCARDUNIT);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(usBoxNum, WFS_CRD_CARDUNITINVALID);

        return WFS_ERR_CRD_INVALIDCARDUNIT;
    }

    // 检查卡箱状态
    nRet = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_STATUS);
    if (nRet == DISPBOX_NOHAVE || nRet == DISPBOX_ERR) // 故障+不存在
    {
        Log(ThisModule, __LINE__, "发卡: Check BoxNo[%d] Is Error[%d], Return: %d.",
            usBoxNum, nRet, WFS_ERR_CRD_CARDUNITERROR);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(usBoxNum, WFS_CRD_CARDUNITERROR);

        return WFS_ERR_CRD_CARDUNITERROR;
    } else
    if (nRet == DISPBOX_EMPTY) // 空
    {
        Log(ThisModule, __LINE__, "发卡: Check BoxNo[%d] Is Empty[%d], Return: %d.",
            usBoxNum, nRet, WFS_ERR_CRD_CARDUNITERROR);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(usBoxNum, WFS_CRD_CARDUNITEMPTY);

        return WFS_ERR_CRD_CARDUNITERROR;
    }

    // 检查卡箱逻辑数: 空,报错
    if ((nRet = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_COUNT)) < 1)
    {
        Log(ThisModule, __LINE__, "发卡: Check BoxNo[%d] CardCount[%d] < 1, Return: %d.",
            usBoxNum, nRet, WFS_ERR_CRD_CARDUNITERROR);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(usBoxNum, WFS_CRD_CARDUNITEMPTY);

        return WFS_ERR_CRD_CARDUNITERROR;
    }

    // 发UKey前内部是否有介质
    if (m_stCRDStatus.fwMedia == WFS_CRD_MEDIAPRESENT ||    // 通道内有介质
        m_stCRDStatus.fwMedia == WFS_CRD_MEDIAEXITING)      // 出口有介质
    {
        Log(ThisModule, __LINE__, "发卡: Media=%d : 通道内或出口有UKey, Return: %d.",
            m_stCRDStatus.fwMedia, WFS_ERR_CRD_CARDUNITERROR);
        return WFS_ERR_CRD_DEVICE_OCCUPIED;
    }

    // 下发发卡命令
    nRet = m_pDev->DispenseCard(usBoxNum, bPresent == TRUE ? DC_BOX2DOOR : DC_BOX2FSCAN_NOS/*DC_BOX2FSCAN*/);
    if (nRet != CRD_SUCCESS)
    {
        Log(ThisModule, __LINE__, "发卡: BoxNo[%d] : ->DispenseCard(%d, %d) Fail. "
                            "RetCode: %d, Return: %d.",
            usBoxNum, usBoxNum, bPresent == TRUE ? DC_BOX2DOOR : DC_BOX2FSCAN,
            nRet, ConvertErrCode(nRet));

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(usBoxNum, WFS_CRD_CARDUNITERROR);

        // UKey已出箱,执行计数
        if (m_stCRDStatus.fwMedia == WFS_CRD_MEDIAPRESENT ||    // 通道内有介质
            m_stCRDStatus.fwMedia == WFS_CRD_MEDIAEXITING)      // 出口有介质
        {
            goto _CARD_COUNT;
        }

        return ConvertErrCode(nRet);
    }

    if (bPresent == TRUE)      // 出口有卡
    {
        m_enWaitTaken = WTF_TAKEN;
    }

    _CARD_COUNT:
    // 发卡计数-1
    nCardCount = m_stUKEYBoxList.GetBoxCardCount(usBoxNum);
    nCardCount = (nCardCount - 1 < 0 ? 0 : nCardCount -1);              // 计数-1 < 0时,设置为0(计数最小值为0)
    m_stUKEYBoxList.SetDispBoxData(usBoxNum, nCardCount, DISPRWT_COUNT); // 箱单元结构体更新
    SetDispBoxCfg(usBoxNum, nCardCount, DISPRWT_COUNT);                  // INI更新

    return ConvertErrCode(nRet);
}

// 弹卡子处理(CMD)
HRESULT CXFS_UKEY::InnerEjecdCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CRD_SUCCESS;

    UpdateDevStatus();   // 更新一次状态

    // 发UKey前检查内部是否有介质
    if (m_stCRDStatus.fwMedia == WFS_CRD_MEDIAPRESENT)    // 通道内有UKEY
    {
        // 下发弹卡命令
        nRet = m_pDev->EjectCard();
        if (nRet != CRD_SUCCESS)
        {
            Log(ThisModule, __LINE__, "EjectCard: ->EjectCard() Fail. RetCode: %d, Return: %d.",
                nRet, ConvertErrCode(nRet));
            return ConvertErrCode(nRet);
        }
    } else
    if (m_stCRDStatus.fwMedia == WFS_CRD_MEDIANOTPRESENT)   // 通道内无UKEY
    {
        Log(ThisModule, __LINE__, "EjectCard: 通道内无UKey. Return: %d.",
            WFS_ERR_CRD_NOMEDIA);
        return WFS_ERR_CRD_NOMEDIA;
    } else
    if (m_stCRDStatus.fwMedia == WFS_CRD_MEDIAEXITING)      // UKEY在出口
    {
        Log(ThisModule, __LINE__, "EjectCard: 通道内无UKey,UKey在出口. Return: %d.",
            WFS_ERR_CRD_NOMEDIA);
        return WFS_ERR_CRD_NOMEDIA;
    }

    if (nRet == CRD_SUCCESS || m_stCRDStatus.fwMedia == WFS_CRD_MEDIAEXITING)      // 出口有卡
    {
        m_enWaitTaken = WTF_TAKEN;
    }

    return WFS_SUCCESS;
}

// 吞卡子处理(CMD)
HRESULT CXFS_UKEY::InnerRetainCard(USHORT usBoxNum)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CRD_SUCCESS;
    INT nCardCount = 0;

    UpdateDevStatus();   // 更新一次状态

    // 发UKey前检查内部和出口是否有介质
    if (m_stCRDStatus.fwMedia == WFS_CRD_MEDIAPRESENT ||    // 通道内有介质
        m_stCRDStatus.fwMedia == WFS_CRD_MEDIAEXITING)      // 出口有介质
    {
        m_enWaitTaken = WTF_NONE;  // TAKEN事件标记归零
        // 下发吞卡命令
        nRet = m_pDev->RetainCard();
        if (nRet != CRD_SUCCESS)
        {
            Log(ThisModule, __LINE__, "RetainCard: ->RetainCard() Fail. RetCode: %d, Return: %d.",
                nRet, ConvertErrCode(nRet));
            return ConvertErrCode(nRet);
        }
    } else
    if (m_stCRDStatus.fwMedia == WFS_CRD_MEDIANOTPRESENT)    // 通道内无UKEY
    {
        Log(ThisModule, __LINE__, "RetainCard: 通道内无UKey. Return: %d.",
            WFS_ERR_CRD_NOMEDIA);
        return WFS_ERR_CRD_NOMEDIA;
    }

    if (m_stIDCStatus.fwRetainBin == WFS_IDC_RETAINBINFULL) // 回收箱状态满
    {
        Log(ThisModule, __LINE__, "RetainCard: 回收箱状态满. Return: %d.",
            WFS_ERR_CRD_RETAINBINFULL);
        return WFS_ERR_CRD_RETAINBINFULL;
    }

    // 吞卡计数
    INT nRetainNum = m_stUKEYBoxList.GetRetainNum();
    nCardCount = m_stUKEYBoxList.GetBoxCardCount(nRetainNum) + 1;
    m_stUKEYBoxList.SetDispBoxData(nRetainNum, nCardCount, DISPRWT_COUNT);
    SetDispBoxCfg(nRetainNum, nCardCount, DISPRWT_COUNT);

    return WFS_SUCCESS;
}

// 复位子处理(CMD)
HRESULT CXFS_UKEY::InnerReset(USHORT usAction, USHORT usUnitNum)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CRD_SUCCESS;
    INT nCardCount = 0;
    USHORT usMode = RESET_NOACTION;
    WORD wMediaOLD = m_stCRDStatus.fwMedia;

    //UpdateDevStatus();   // 更新一次状态

    if (usAction == WFS_IDC_EJECT || usAction == WFS_CRD_EJECT)     // 退出
    {
        usMode = RESET_EJECT;
    } else
    if (usAction == WFS_IDC_RETAIN || usAction == WFS_CRD_RETAIN)   // 回收
    {
        usMode = RESET_RETAIN;
        m_enWaitTaken =  WTF_NONE;  // TAKEN事件标记归零
    }

    // 下发复位命令
    nRet = m_pDev->Reset(usMode);
    if (nRet != CRD_SUCCESS)
    {
        Log(ThisModule, __LINE__, "Reset: ->Reset(%d) Fail. RetCode: %d, Return: %d.",
            nRet, usMode, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    // 吞卡计数: 原通道内/出口有介质
    if (usMode == RESET_RETAIN &&
        (wMediaOLD == WFS_CRD_MEDIAPRESENT || wMediaOLD == WFS_CRD_MEDIAEXITING))
    {
        nCardCount = m_stUKEYBoxList.GetBoxCardCount(usUnitNum) + 1;
        m_stUKEYBoxList.SetDispBoxData(usUnitNum, nCardCount, DISPRWT_COUNT);
        SetDispBoxCfg(usUnitNum, nCardCount, DISPRWT_COUNT);
    }

    return WFS_SUCCESS;
}

// 读UKEY编号(CMD)
HRESULT CXFS_UKEY::InnerReadRawData(DWORD dwOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CRD_SUCCESS;
    CHAR szUKeyNo[1024] = { 0x00 };
    INT usUKeyNoLen = 1024;

    if (m_stIDCStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)   // 通道内无介质
    {
        Log(ThisModule, __LINE__, "读UKEY编号: 通道内无UKEY: Return: %d.", WFS_ERR_IDC_NOMEDIA);
        return WFS_ERR_IDC_NOMEDIA;
    } else
    if (m_stIDCStatus.fwMedia == WFS_CRD_MEDIAEXITING)      // 出口有介质
    {
        if (m_stConfig.usEjectAgainRead == 1)   // 允许退回重读
        {
            // 下发弹卡命令: UKey从出口发到前端扫描位置
            nRet = m_pDev->EjectCard(DC_DOOR2FSCAN);
            if (nRet != CRD_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "读UKEY编号: UKey从出口发到前端扫描位置: ->EjectCard(%d) Fail. RetCode: %d, Return: %d.",
                    DC_DOOR2FSCAN, nRet, ConvertErrCode(nRet));
                return ConvertErrCode(nRet);
            }
        } else
        {
            Log(ThisModule, __LINE__, "读UKEY编号: 扫描位置无UKEY: Return: %d.", WFS_ERR_IDC_NOMEDIA);
            return WFS_ERR_IDC_NOMEDIA;
        }
    }

    // 读UKEY编号
    nRet = m_pDev->GetData(szUKeyNo, &usUKeyNoLen, DTYPE_GET_UKEYNO);
    if (nRet != CRD_SUCCESS)
    {
        if (nRet == ERR_CRD_SCAN)
        {
            Log(ThisModule, __LINE__, "读UKEY编号: ->GetData(%s, %d, %d) Fail. RetCode: %d, Return: %d.",
                szUKeyNo, usUKeyNoLen, DTYPE_GET_UKEYNO, WFS_ERR_IDC_INVALIDMEDIA);
            return WFS_ERR_IDC_INVALIDMEDIA;
        }

        Log(ThisModule, __LINE__, "读UKEY编号: ->GetData(%s, %d, %d) Fail. RetCode: %d, Return: %d.",
            szUKeyNo, usUKeyNoLen, DTYPE_GET_UKEYNO, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }
    SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAOK, usUKeyNoLen, (LPBYTE)szUKeyNo);
    return WFS_SUCCESS;
}

// 设置INI中卡箱单元信息
int CXFS_UKEY::SetDispBoxCfg(USHORT usBoxNum, ULONG ulData, USHORT usType)
{
    return SetDispBoxCfg(usBoxNum, (LPSTR)std::to_string(ulData).c_str(), usType);
}

int CXFS_UKEY::SetDispBoxCfg(USHORT usBoxNum, LPSTR lpData, USHORT usType)
{
    CHAR    szIniAppName[256] = { 0x00 };

    sprintf(szIniAppName, "DISPBOX_%d", usBoxNum);

    switch(usType)
    {
        case DISPRWT_TYPE       :       // 箱类型
            m_cXfsReg.SetValue(szIniAppName, "BoxType", lpData);
            break;
        case DISPRWT_THINGTP       :     // 箱内卡类型
            m_cXfsReg.SetValue(szIniAppName, "ThingType", lpData);
            break;
        case DISPRWT_INITCNT    :       // 初始数目
            m_cXfsReg.SetValue(szIniAppName, "InitCnt", lpData);
            break;
        case DISPRWT_COUNT      :       // 当前数目
            m_cXfsReg.SetValue(szIniAppName, "Count", lpData);
            break;
        case DISPRWT_RETCNT     :       // 进入回收数目
            m_cXfsReg.SetValue(szIniAppName, "RetainCnt", lpData);
            break;
        case DISPRWT_THREH      :       // 报警阀值(HIGH回收箱/LOW发卡箱)
            m_cXfsReg.SetValue(szIniAppName, "Threshold", lpData);
            break;
        case DISPRWT_HARDSR     :       // 是否基于硬件传感器生成阀值事件
            m_cXfsReg.SetValue(szIniAppName, "HardSensorEvent", lpData);
            break;
    }

    return 0;
}

// 根据卡单元索引号赋值卡信息结构体
void CXFS_UKEY::CardUnitInfoPack(LPWFSCRDCARDUNIT lpUnit, USHORT usBoxNum)
{
    INT nCnt = 0;

    if (lpUnit == nullptr)
    {
        lpUnit = new WFSCRDCARDUNIT();
        if (lpUnit == nullptr)
        {
            return;
        }
    }

    // 卡单元结构索引号(1~N)
    lpUnit->usNumber = usBoxNum;
    // 卡单元类型
    nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_TYPE);
    lpUnit->usType = (nCnt == DISPBOX_STORAGE ? WFS_CRD_SUPPLYBIN : WFS_CRD_RETAINBIN);
    // 卡单元中卡初始数目
    nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_INITCNT);
    lpUnit->ulInitialCount = (nCnt < 0 ? 0 : nCnt);
    // 卡单元中卡当前数目
    nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_COUNT);
    lpUnit->ulCount = (nCnt < 0 ? 0 : nCnt);
    // 卡单元进入回收箱中卡数目
    nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_RETCNT);
    lpUnit->ulRetainCount = (ULONG)(nCnt < 0 ? 0 : nCnt);
    // 阀值
    nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_THREH);
    lpUnit->ulThreshold = (ULONG)(nCnt < 0 ? 0 : nCnt);
    // 卡箱状态
    lpUnit->usStatus = ConvertDBoxStat2WFS(m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_STATUS));
    // 是否基于硬件传感器生成阀值事件
    nCnt = m_stUKEYBoxList.GetDispBoxData(usBoxNum, DISPRWT_HARDSR);
    lpUnit->bHardwareSensor = (ULONG)(nCnt == 0 ? false : true);
    // 卡箱中卡类型
    nCnt = strlen(m_stUKEYBoxList.GetDispBoxCardType(usBoxNum));
    if (nCnt < 1)
    {
        lpUnit->lpszCardName = nullptr;
    } else
    {
        lpUnit->lpszCardName = new CHAR[nCnt + 1];
        memset(lpUnit->lpszCardName, 0x00, nCnt + 1);
        memcpy(lpUnit->lpszCardName, m_stUKEYBoxList.GetDispBoxCardType(usBoxNum), nCnt);
    }
}

// CWFSIDCCardDataPtrArray类处理
void CXFS_UKEY::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
{
    WFSIDCCARDDATA data;
    data.wDataSource    = wSource;
    data.wStatus        = wStatus;
    data.ulDataLength   = uLen;
    data.lpbData        = pData;
    data.fwWriteMethod  = 0;

    m_clCardData.SetAt(wSource, data);
    return;
}


//-----------------------------------------------------------------------------------
//------------------------------------格式转换WFS-------------------------------------
// 错误码转换为WFS格式
INT CXFS_UKEY::ConvertErrCode(INT nRet)
{
    switch (nRet)
    {
        case CRD_SUCCESS:                   return WFS_SUCCESS;
        case ERR_CRD_INSERT_TIMEOUT:        return WFS_ERR_TIMEOUT;             // CRM:进卡超时
        case ERR_CRD_USER_CANCEL:           return WFS_ERR_CANCELED;            // CRM:用户取消
        case ERR_CRD_COMM_ERR:              return WFS_ERR_CONNECTION_LOST;     // CRM:通讯错误
        case ERR_CRD_JAMMED:                return WFS_ERR_CRD_MEDIAJAM;        // CRM:堵卡
        case ERR_CRD_OFFLINE:               return WFS_ERR_CONNECTION_LOST;     // CRM:脱机
        case ERR_CRD_NOT_OPEN:              return WFS_ERR_HARDWARE_ERROR;      // CRM:没有打开
        case ERR_CRD_UNIT_FULL:             return WFS_ERR_CRD_RETAINBINFULL;   // CRM:卡箱满
        case ERR_CRD_UNIT_EMPTY:            return WFS_ERR_CRD_CARDUNITERROR;   // CRM:卡箱空
        case ERR_CRD_UNIT_NOTFOUND:         return WFS_ERR_CRD_INVALIDCARDUNIT; // CRM:卡箱不存在
        case ERR_CRD_HWERR:                 return WFS_ERR_HARDWARE_ERROR;      // CRM:硬件故障
        case ERR_CRD_STATUS_ERR:            return WFS_ERR_HARDWARE_ERROR;      // CRM:状态出错
        case ERR_CRD_UNSUP_CMD:             return WFS_ERR_UNSUPP_COMMAND;      // CRM:不支持的指令
        case ERR_CRD_PARAM_ERR:             return WFS_ERR_INVALID_DATA;        // CRM:参数错误
        case ERR_CRD_READTIMEOUT:           return WFS_ERR_HARDWARE_ERROR;      // CRM:读数据超时
        case ERR_CRD_WRITETIMEOUT:          return WFS_ERR_HARDWARE_ERROR;      // CRM:写数据超时
        case ERR_CRD_READERROR:             return WFS_ERR_HARDWARE_ERROR;      // CRM:读数据错
        case ERR_CRD_WRITEERROR:            return WFS_ERR_HARDWARE_ERROR;      // CRM:写数据错
        case ERR_CRD_DATA_INVALID:          return WFS_ERR_HARDWARE_ERROR;      // CRM:无效数据
        case ERR_CRD_LOAD_LIB:              return WFS_ERR_INTERNAL_ERROR;      // CRM:动态库错误
        case ERR_CRD_OTHER:                 return WFS_ERR_HARDWARE_ERROR;      // CRM:其他错误/未知错误
        default:                            return WFS_ERR_HARDWARE_ERROR;
    }
}

// 设备状态转换为WFS格式
LONG CXFS_UKEY::ConvertDeviceStatus(USHORT usStat, CONVERTYPE enTYPE)
{
    switch (usStat)
    {
        case DEVICE_STAT_ONLINE     :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVONLINE : WFS_IDC_DEVONLINE);         // 设备正常
        case DEVICE_STAT_OFFLINE    :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVOFFLINE: WFS_IDC_DEVOFFLINE);        // 设备脱机
        case DEVICE_STAT_POWEROFF   :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVPOWEROFF: WFS_IDC_DEVPOWEROFF);      // 设备断电
        case DEVICE_STAT_NODEVICE   :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVNODEVICE: WFS_IDC_DEVNODEVICE);      // 设备不存在
        case DEVICE_STAT_HWERROR    :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVHWERROR: WFS_IDC_DEVHWERROR);        // 设备故障
        case DEVICE_STAT_USERERROR  :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVUSERERROR: WFS_IDC_DEVUSERERROR);    // 设备存在,但有人阻止设备操作
        case DEVICE_STAT_BUSY       :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVBUSY: WFS_IDC_DEVBUSY);              // 设备读写中
        case DEVICE_STAT_FRAUDAT    :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVFRAUDATTEMPT : WFS_IDC_DEVONLINE);   // 设备存在,但有检测到欺诈企图
        case DEVICE_STAT_POTENTIAL  :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVPOTENTIALFRAUD : WFS_IDC_DEVONLINE); // 设备检测到欺诈企图但可继续使用,应用决定是否脱机
        default:
            return (enTYPE == CONVERT_CRD ? WFS_CRD_DEVOFFLINE : WFS_IDC_DEVOFFLINE);
    }
}

// 总单元状态转换为WFS格式
LONG CXFS_UKEY::ConvertCRDDispensrStatus(USHORT usStat)
{
    switch (usStat)
    {
        case DISP_STAT_OK           : return WFS_CRD_DISPCUOK;          // 所有单元正常
        case DISP_STAT_STATE        : return WFS_CRD_DISPCUSTATE;       // 1个/多个单元处于低/空/不工作状态,但仍有一个单元在工作
        case DISP_STAT_STOP         : return WFS_CRD_DISPCUSTOP;        // 所有单元故障/空/停止工作
        case DISP_STAT_UNKNOWN      : return WFS_CRD_DISPCUUNKNOWN;     // 无法确定单元状态
        default: return WFS_CRD_DISPCUUNKNOWN;
    }
}

// 传送模块状态转换为WFS格式
LONG CXFS_UKEY::ConvertCRDTransportStatus(USHORT usStat)
{
    switch (usStat)
    {
        case TRANS_STAT_OK          : return WFS_CRD_TPOK;              // 正常
        case TRANS_STAT_INOP        : return WFS_CRD_TPINOP;            // 传输不工作(故障/堵塞)
        case TRANS_STAT_UNKNOWN     : return WFS_CRD_TPUNKNOWN;         // 无法确定状态
        case TRANS_STAT_NOTSUPP     : return WFS_CRD_TPNOTSUPPORTED;    // 不支持状态报告
        default: return WFS_CRD_TPNOTSUPPORTED;
    }
}

// 介质状态转换为WFS格式
LONG CXFS_UKEY::ConvertMediaStatus(USHORT usStat, CONVERTYPE enTYPE)
{
    switch (usStat)
    {
        case MEDIA_STAT_PRESENT     :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_MEDIAPRESENT : WFS_IDC_MEDIAPRESENT);           // 通道内有介质
        case MEDIA_STAT_NOTPRESENT  :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_MEDIANOTPRESENT : WFS_IDC_MEDIANOTPRESENT);     // 通道内无介质
        case MEDIA_STAT_JAMMED      :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_MEDIAJAMMED : WFS_IDC_MEDIAJAMMED);             // 通道内有介质且被夹住
        case MEDIA_STAT_NOTSUPP     :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_MEDIANOTSUPP : WFS_IDC_MEDIANOTSUPP);           // 不支持检测介质状态
        case MEDIA_STAT_UNKNOWN     :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_MEDIAUNKNOWN : WFS_IDC_MEDIAUNKNOWN);           // 介质状态未知
        case MEDIA_STAT_EXITING     :
            return (enTYPE == CONVERT_CRD ? WFS_CRD_MEDIAEXITING : WFS_IDC_MEDIAENTERING);          // 介质在出口插槽(CRD专用)
        default:
            return (enTYPE == CONVERT_CRD ? WFS_CRD_MEDIAUNKNOWN : WFS_IDC_MEDIAUNKNOWN);
    }
}

// 门状态转换为WFS格式
LONG CXFS_UKEY::ConvertCRDShutterStatus(USHORT usStat)
{
    switch (usStat)
    {
        case SHUTTER_STAT_CLOSED    : return WFS_CRD_SHTCLOSED;         // 关闭
        case SHUTTER_STAT_OPEN      : return WFS_CRD_SHTOPEN;           // 打开
        case SHUTTER_STAT_JAMMED    : return WFS_CRD_SHTJAMMED;         // 卡住
        case SHUTTER_STAT_UNKNOWN   : return WFS_CRD_SHTUNKNOWN;        // 未知
        case SHUTTER_STAT_NOTSUPP   : return WFS_CRD_SHTNOTSUPPORTED;   // 不支持状态查询
        default: return WFS_CRD_SHTUNKNOWN;
    }
}

// 指定设备位置转换为WFS格式
LONG CXFS_UKEY::ConvertCRDDevicePosStatus(USHORT usStat)
{
    switch (usStat)
    {
        case DEVPOS_STAT_INPOS      : return WFS_CRD_DEVICEINPOSITION;  // 设备处于正常工作位置
        case DEVPOS_STAT_NOTINPOS   : return WFS_CRD_DEVICENOTINPOSITION;// 设备不在正常工作位置
        case DEVPOS_STAT_UNKNOWN    : return WFS_CRD_DEVICEPOSUNKNOWN;  // 未知
        case DEVPOS_STAT_NOTSUPP    : return WFS_CRD_DEVICEPOSNOTSUPP;  // 不支持状态查询
        default: return WFS_CRD_DEVICEPOSUNKNOWN;
    }
}

// 反欺诈模块状态转换为WFS格式
LONG CXFS_UKEY::ConvertCRDAntiFraudStatus(USHORT usStat)
{
    switch (usStat)
    {
        case ANFRAUD_STAT_OK        : return WFS_CRD_AFMOK;             // 正常
        case ANFRAUD_STAT_INOP      : return WFS_CRD_AFMINOP;           // 不可用
        case ANFRAUD_STAT_DETECTED  : return WFS_CRD_AFMDEVICEDETECTED; // 检测到外部设备
        case ANFRAUD_STAT_UNKNOWN   : return WFS_CRD_AFMUNKNOWN;        // 未知
        case ANFRAUD_STAT_NOTSUPP   : return WFS_CRD_AFMNOTSUPP;        // 不支持状态查询
        default: return WFS_CRD_AFMUNKNOWN;
    }
}

// DispBox单元状态转换为WFS格式
LONG CXFS_UKEY::ConvertDBoxStat2WFS(INT nStat, CONVERTYPE enTYPE)
{
    if(enTYPE == CONVERT_CRD)
    {
        switch (nStat)
        {
            case DISPBOX_NOHAVE :   return WFS_CRD_STATCUMISSING;   // 不存在
            case DISPBOX_ISHAVE :   return WFS_CRD_STATCUOK;        // 存在
            case DISPBOX_LOW    :   return WFS_CRD_STATCULOW;       // 箱将空
            case DISPBOX_EMPTY  :   return WFS_CRD_STATCUEMPTY;     // 箱空
            case DISPBOX_HIGH   :   return WFS_CRD_STATCUHIGH;      // 箱将满/箱将空
            case DISPBOX_FULL   :   return WFS_CRD_STATCUFULL;      // 箱满
            case DISPBOX_ERR    :   return WFS_CRD_STATCUINOP;      // 故障
            default: return WFS_CRD_STATCUUNKNOWN;
        }
    } else
    {
        switch (nStat)
        {
            case DISPBOX_NOHAVE :   return WFS_IDC_RETAINNOTSUPP;   // 不存在
            case DISPBOX_ISHAVE :   return WFS_CRD_STATCUOK;        // 存在
            case DISPBOX_LOW    :   return WFS_IDC_RETAINBINOK;     // 箱将空
            case DISPBOX_EMPTY  :   return WFS_IDC_RETAINBINOK;     // 箱空
            case DISPBOX_HIGH   :   return WFS_CRD_STATCUHIGH;      // 箱将满/箱将空
            case DISPBOX_FULL   :   return WFS_IDC_RETAINBINFULL;   // 箱满
            case DISPBOX_ERR    :   return WFS_IDC_RETAINNOTSUPP;   // 故障
            default: return WFS_IDC_RETAINNOTSUPP;
        }
    }
}

// CRD定义Unit单元状态转换为DISPBOX结构体状态值转换为WFS格式
LONG CXFS_UKEY::ConvertCRDUnitStat2DBoxStat(USHORT usStat)
{
    switch (usStat)
    {
        case UNITINFO_STAT_MISSING  :   return DISPBOX_NOHAVE;  // 不存在
        case UNITINFO_STAT_LOW      :   return DISPBOX_LOW;     // 将空
        case UNITINFO_STAT_EMPTY    :   return DISPBOX_EMPTY;   // 空
        case UNITINFO_STAT_HIGH     :   return DISPBOX_HIGH;    // 将满
        case UNITINFO_STAT_FULL     :   return DISPBOX_FULL;    // 满
        case UNITINFO_STAT_INOP     :   return DISPBOX_ERR;     // 故障
        case UNITINFO_STAT_UNKNOWN  :   return DISPBOX_ERR;     // 无法确定状态
        default: return DISPBOX_ERR;
    }
}

// WFS CRD 错误码 转换为 WFS IDC 错误码
LONG CXFS_UKEY::ConvertWfsCRD2IDC(INT nErrCode)
{
    switch (nErrCode)
    {
        case WFS_ERR_CRD_MEDIAJAM               :   return WFS_ERR_IDC_MEDIAJAM;
        case WFS_ERR_CRD_NOMEDIA                :   return WFS_ERR_IDC_NOMEDIA;
        case WFS_ERR_CRD_MEDIARETAINED          :   return WFS_ERR_IDC_MEDIARETAINED;
        case WFS_ERR_CRD_RETAINBINFULL          :   return WFS_ERR_IDC_RETAINBINFULL;
        case WFS_ERR_CRD_SHUTTERFAIL            :   return WFS_ERR_IDC_SHUTTERFAIL;
        default: return nErrCode;
    //case WFS_ERR_CRD_DEVICE_OCCUPIED        :   return ;
    //case WFS_ERR_CRD_CARDUNITERROR          :   return ;
    //case WFS_ERR_CRD_INVALIDCARDUNIT        :   return ;
    //case WFS_ERR_CRD_INVALID_PORT           :   return ;
    //case WFS_ERR_CRD_INVALIDRETAINBIN       :   return ;
    //case WFS_ERR_CRD_POWERSAVETOOSHORT      :   return ;
    //case WFS_ERR_CRD_POWERSAVEMEDIAPRESENT  :   return ;
    }
}


//--------------------------方法处理------------------------------

