/***************************************************************
* 文件名称：XFS_IDC_DEC.cpp
* 文件描述：读卡器模块子命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_IDC.h"

//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_IDC::InnerOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    INT nRet = IDC_SUCCESS;

    // Open前下传初始参数(非断线重连)
    if (bReConn == FALSE)
    {
        // 设置SDK路径
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pIDCDev->SetData(SET_LIB_PATH, m_stConfig.szSDKPath);
        }

        // 设置设备打开模式
        m_pIDCDev->SetData(SET_DEV_OPENMODE, &(m_stConfig.stDevOpenMode));

        // 设置进卡检查模式
        m_pIDCDev->SetData(SET_DEV_PREIC, &(m_stConfig.wSupportPredictIC));

        // 设置设备辅助参数
        m_pIDCDev->SetData(SET_DEV_AUXPARAM, m_stConfig.nDevAuxParam);
    }

    // 打开IDC设备
    nRet = m_pIDCDev->Open(DEVTYPE2STR(m_stConfig.wDeviceType));
    if (nRet != IDC_SUCCESS)
    {
        if (bReConn == FALSE)
        {
            Log(ThisModule, __LINE__, "IDC: Open[%s] fail, ErrCode = %d, Return: %d",
                DEVTYPE2STR(m_stConfig.wDeviceType), nRet, ConvertDevErrCode2WFS(nRet));
        }
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    // 读卡器初始化
    /*hRet = DeviceIDCInit(bReConn);
    if (hRet != WFS_SUCCESS)
    {
        if (bReConn == FALSE)
        {
            Log(ThisModule, __LINE__, "IDC: 读卡器初始化 fail, ErrCode = %d, Return: %d",
                hRet, hRet);
        }
        return hRet;
    }*/

    // 清除硬件回收计数
    m_pIDCDev->SetData(SET_DEV_RETAINCNT);

    // 吞卡计数处理
    //UpdateRetainCards(MCM_NOACTION);

    // 组织扩展数据
    UpdateExtra();

    // 更新一次状态
    OnStatus();

    // 初始关闭抖动进卡
    m_pIDCDev->SetData(SET_DEV_WOBBLE_CLOSE);

    if (bReConn == TRUE)
    {
        Log(ThisModule, __LINE__, "IDC: 断线重连成功, Extra=%s.", m_cStatExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, __LINE__, "IDC: 打开设备连接成功, Extra=%s.", m_cStatExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}


// 加载DevIDC动态库
BOOL CXFS_IDC::LoadDevIDCDll(LPCSTR ThisModule)
{
    if (m_pIDCDev == nullptr)
    {
        if (m_pIDCDev.Load(m_stConfig.szDevDllNameIDC, "CreateIDevIDC", DEVTYPE2STR(m_stConfig.wDeviceType)) != 0)
        {
            Log(ThisModule, __LINE__, "IDC: 加载库失败: DriverDllName=%s, DriverType=%d|%s, ERR:%s",
                m_stConfig.szDevDllNameIDC, m_stConfig.wDeviceType,
                DEVTYPE2STR(m_stConfig.wDeviceType), m_pIDCDev.LastError().toUtf8().constData());
            SetErrorDetail(0, (LPSTR)EC_XFS_DevXXXLoadFail);
            return false;
        } else
        {
            Log(ThisModule, __LINE__, "IDC: 加载库: DriverDllName=%s, DriverType=%d|%s, Succ.",
                m_stConfig.szDevDllNameIDC, m_stConfig.wDeviceType, DEVTYPE2STR(m_stConfig.wDeviceType));
        }
    }
    return (m_pIDCDev != nullptr);
}

// 加载INI设置
INT CXFS_IDC::InitConfig()
{
    CHAR    szIniAppName[MAX_PATH];
    CHAR    szBuffer[MAX_PATH];
    INT     nTmp;

    // DevIDC动态库名
    strcpy(m_stConfig.szDevDllNameIDC, m_cXfsReg.GetValue("IDCDriverDllName", ""));

    // DevSIU动态库名
    strcpy(m_stConfig.szDevDllNameSIU, m_cXfsReg.GetValue("SIUDriverDllName", ""));

    // 设备类型
    m_stConfig.wDeviceType = m_cXfsReg.GetValue("DriverType", (DWORD)0);    

    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.wDeviceType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    //-----------------------------------创自设备参数获取-----------------------------------
    if (m_stConfig.wDeviceType == XFS_CRT350N)
    {
        STDEVICEOPENMODE    stDevOpenModeTmp;

        // 打开方式(0串口/1USBHID,缺省0)
        stDevOpenModeTmp.wOpenMode = (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenMode", (DWORD)0);
        if (stDevOpenModeTmp.wOpenMode < 0 || stDevOpenModeTmp.wOpenMode > 1)
        {
            stDevOpenModeTmp.wOpenMode = 0;
        }
        // 设备路径(适用于串口和USBHID,缺省空)
        strcpy(stDevOpenModeTmp.szDevPath, m_cXfsReg.GetValue(szIniAppName, "DevPath", ""));
        // 波特率(适用于串口,缺省9600)
        stDevOpenModeTmp.wBaudRate = (WORD)m_cXfsReg.GetValue(szIniAppName, "BaudRate", (DWORD)9600);
        // 设备VID(适用于USBHID,4位16进制字符,缺省空)
        strcpy(stDevOpenModeTmp.szHidVid, m_cXfsReg.GetValue(szIniAppName, "VendorId", ""));
        // 设备PID(适用于USBHID,4位16进制字符,缺省空)
        strcpy(stDevOpenModeTmp.szHidPid, m_cXfsReg.GetValue(szIniAppName, "ProductId", ""));
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
    // Open时是否执行Reset动作(0不执行/1执行,缺省0)
    /*m_stConfig.wOpenResetSupp = (WORD)m_cXfsReg.GetValue("OPEN_CONFIG", "OpenResetSupp", (DWORD)0);
    if (m_stConfig.wOpenResetSupp != 0 &&
        m_stConfig.wOpenResetSupp != 1)
    {
        m_stConfig.wOpenResetSupp = 0;
    }*/

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.wOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.wOpenFailRet != 0 && m_stConfig.wOpenFailRet != 1)
    {
        m_stConfig.wOpenFailRet = 0;
    }

    // Open后复位卡动作(0:不支持, 1:无动作, 2:持卡, 3:退卡, 4:吞卡, 缺省0)
    m_stConfig.wOpenResetCardAction = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenResetCardAction", (DWORD)0);
    if (m_stConfig.wOpenResetCardAction < 0 && m_stConfig.wOpenResetCardAction > 4)
    {
        m_stConfig.wOpenResetCardAction = 0;
    }


    //------------------------[RESET_CONFIG]下参数------------------------
    // Reset时卡动作(0无动作/1退卡/2吞卡,缺省0)
    m_stConfig.wResetCardAction = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetCardAction", (DWORD)0);
    if (m_stConfig.wResetCardAction != 0 &&
        m_stConfig.wResetCardAction != 1 &&
        m_stConfig.wResetCardAction != 2)
    {
        m_stConfig.wResetCardAction = 0;
    }

    // Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功,缺省0)
    m_stConfig.wResetFailReturn = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetFailReturn", (DWORD)0);
    if (m_stConfig.wResetFailReturn != 0 &&
        m_stConfig.wResetFailReturn != 1)
    {
        m_stConfig.wResetFailReturn = 0;
    }


    //------------------------[RETAIN_CONFIG]下参数------------------------
    //------------------------回收相关配置----------------------------------
    // 是否支持回收功能(0不支持/1支持,缺省0)
    m_stConfig.wRetainSupp = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG", "RetainSupp", (DWORD)0);
    if (m_stConfig.wRetainSupp != 0 &&
        m_stConfig.wRetainSupp != 1)
    {
        m_stConfig.wRetainSupp = 0;
    }

    // 吞卡计数
    m_stConfig.wRetainCardCount = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG", "RetainCardCount", (DWORD)0);

    // 回收将满报警阀值(缺省15)
    m_stConfig.wRetainThreshold = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG", "RetainThreshold", (DWORD)15);

    // 回收满阀值(缺省20)
    m_stConfig.wRetainFull = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG", "RetainFull", (DWORD)20);


    //------------------------[READCARD_COFNIG]下参数------------------------
    //------------------------读卡器相关配置----------------------------------
    // 是否支持写磁(1:支持, 0:不支持, 缺省0)
    m_stConfig.wCanWriteTrack = (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "CanWriteTrack", (DWORD)0);

    // 磁通感应器是否可用(0:不可用, 1:可用, 缺省0)
    m_stConfig.wFluxSensorSupp = (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "FluxSensorSupp", (DWORD)0);

    // 退卡到出口后是否支持重新吸入读写(0:不支持, 1:支持, 缺省0)
    m_stConfig.wRWAccFollowingEject = (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "RWAccFollowingEject", (DWORD)0);

    // 是否需要支持抖动功能(0:不支持, 1:支持, 缺省1)
    m_stConfig.wNeedWobble = (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "NeedWobble", (DWORD)1);

    // 掉电时卡的控制方式,缺省2
    // 0:退卡到前出口; 1:写磁时不退卡,其他情况退卡; 2:无动作(不退卡); 3:退卡到前出口,30秒未取则吞卡; 4: 吞卡
    m_stConfig.stDevOpenMode.nOtherParam[10] =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "PowerOffCardAction", (DWORD)2);

    // 掉电时写卡命令处理(0:停止写; 1:不停止写, 缺省0)
    m_stConfig.stDevOpenMode.nOtherParam[11] =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "PowerOffStopWrite", (DWORD)0);

    // 初始化时是否测试闸门开关(0:不测试, 1:测试, 缺省0)
    m_stConfig.stDevOpenMode.nOtherParam[12] =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "InitTestFloorPush", (DWORD)0);

    // 是否启用硬件回收计数(0:不启用, 1:启用, 缺省:1)
    m_stConfig.stDevOpenMode.nOtherParam[13] =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "DevRetainCountEnable", (DWORD)1);

    // 进卡超时时间(0:无超时, >0:指定超时, 缺省0,单位:秒)
    // SP命令下发不包含超时时间时,以该项配置为准
    m_stConfig.dwInCardTimeOut =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "InCardTimeOut", (DWORD)0);

    // 退卡时无卡是否报MediaRemoved事件(0不上报, 1上报, 缺省0)
    m_stConfig.wPostRemovedAftEjectFixed =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "PostRemovedAftEjectFixed", (DWORD)0);

    // 回收盒满是否支持进卡(0:不支持, 1:支持, 缺省0)
    m_stConfig.wAcceptWhenCardFull =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "AcceptWhenCardFull", (DWORD)0);

    // 卡时是否有磁道(0:无, 1:有, 缺省1)
    m_stConfig.wFluxInActive =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "FluxInActive", (DWORD)1);

    // 进卡检查模式,缺省1
    // 0:所有卡可进
    // 1:判磁或IC(磁条卡或IC卡可进)(命令入参可指定是否有磁)
    // 2:判磁且IC(只支持芯片且有磁道)(命令入参可指定是否有磁)
    m_stConfig.wSupportPredictIC =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "SupportPredictIC", (DWORD)1);

    // 防盜钩功能支持(0:不支持, 1:支持, 缺省:0)
    m_stConfig.wTamperSupp =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "TamperSensorSupport", (DWORD)0);

    // 吸卡时后出口进卡处理(0:不处理, 1:正常吸卡, 2:吞卡, 缺省1)
    m_stConfig.wAfterInCardOpen =
            (WORD)m_cXfsReg.GetValue("READCARDER_COFNIG", "AfterInCardOpen", (DWORD)0);
    if (m_stConfig.wAfterInCardOpen < 0 || m_stConfig.wAfterInCardOpen > 2)
    {
        m_stConfig.wAfterInCardOpen = 1;
    }


    //------------------------[SKIMMING_CONFIG]下参数------------------------
    //------------------------异物检知相关设置----------------------------------
    // 是否支持卡口异物检知功能(0:不支持, 1:支持, 缺省0)
    m_stConfig.wSkimmingSupp = (WORD)m_cXfsReg.GetValue("SKIMMING_CONFIG", "SkimmingSupp", (DWORD)0);

    // 检测间隔时间(缺省:3, 单位:秒)
    m_stConfig.wSkimmingMonitorInterval =
            (WORD)m_cXfsReg.GetValue("SKIMMING_CONFIG", "SkimmingMonitorInterval", (DWORD)3);

    // 判故障持续故障时间(缺省:60, 单位:秒)
    m_stConfig.wSkimmingErrorDuration =
            (WORD)m_cXfsReg.GetValue("SKIMMING_CONFIG", "SkimmingErrorDuration", (DWORD)60);

    // 判正常持续正常时间(缺省:3, 单位:秒)
    m_stConfig.wSkimmingNormalDuration =
            (WORD)m_cXfsReg.GetValue("SKIMMING_CONFIG", "SkimmingNormalDuration", (DWORD)60);


    //------------------------[FRAUDDETE_CONFIG]下参数------------------------
    //------------------------欺诈检测相关设置----------------------------------
    // 防逗卡保护功能是否支持(0:不支持, 1:支持, 缺省0)
    m_stConfig.wTeaseCardProtectSupp =
            (WORD)m_cXfsReg.GetValue("FRAUDDETE_CONFIG", "TeaseCardProtectSupp", (DWORD)0);
    if (m_stConfig.wTeaseCardProtectSupp < 0 || m_stConfig.wTeaseCardProtectSupp > 1)
    {
        m_stConfig.wTeaseCardProtectSupp = 0;
    }

    // 防逗卡保护生效的进卡次数上限(缺省5), 达到该上限后, 触发防逗卡保护机制
    m_stConfig.wTeaseInCardCount =
            (WORD)m_cXfsReg.GetValue("FRAUDDETE_CONFIG", "TeaseInCardCount", (DWORD)5);

    // 防逗卡保护生效后持续时间(0:持续保护, >0:按指定时间保护, 缺省0, 单位:秒)
    m_stConfig.wTeaseCardProtectDate =
            (WORD)m_cXfsReg.GetValue("FRAUDDETE_CONFIG", "TeaseCardProtectDate", (DWORD)0);


    //------------------------[TESTER_CONFIG]下参数------------------------
    //------------------------测试模式相关配置----------------------------------
    // 是否启用测试模式(0:不启用, 1:启用, 缺省0)
    m_stConfig.wTestModeIsSup = (WORD)m_cXfsReg.GetValue("TESTER_CONFIG", "TestModeIsSup", (DWORD)0);

    // 测试模式处理1: 读卡数据完成后返回先返Insert事件,再返Complete
    // 0: 该配置无效, >0:Insert下发后到Complete返回时间(单位:毫秒), 缺省0
    m_stConfig.nTestInsertComplete =
            (WORD)m_cXfsReg.GetValue("TESTER_CONFIG", "TestInsertComplete", (DWORD)0);


    // 硬件辅助参数填充
    m_stConfig.nDevAuxParam[0] = m_stConfig.wTamperSupp;            // 防盜钩功能支持
    m_stConfig.nDevAuxParam[1] = m_stConfig.wTeaseCardProtectSupp;  // 防逗卡功能是否支持
    m_stConfig.nDevAuxParam[2] = m_stConfig.wTeaseInCardCount;      // 防逗卡进卡次数
    m_stConfig.nDevAuxParam[3] = m_stConfig.wTeaseCardProtectDate;  // 防逗卡保护时间

    PrintIniIDC();  // INI配置输出

    // 退卡模块INI配置参数获取
    InitConfigCRM();

    return 0;
}

// INI配置输出
INT CXFS_IDC::PrintIniIDC()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    QString qsIniPrt = "";
    CHAR szBuff[256] = { 0x00 };

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t DevIDC动态库名: default->SIUDriverDllName = %s",
            m_stConfig.szDevDllNameIDC);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t DevSIU动态库名: default->SIUDriverDllName = %s",
            m_stConfig.szDevDllNameSIU);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 设备类型: default->DriverType = %d",
            m_stConfig.wDeviceType);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 设备SDK库路径: DEVICE_SET_%d->SDK_Path = %s",
            m_stConfig.wDeviceType, m_stConfig.szSDKPath);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 打开方式(0串口/1USBHID): DEVICE_SET_%d->OpenMode = %d",
            m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.wOpenMode);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 设备路径: DEVICE_SET_%d->DevPath = %s",
            m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szDevPath);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 波特率: DEVICE_SET_%d->DevPath = %d",
            m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.wBaudRate);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 设备VID: DEVICE_SET_%d->VendorId = %s",
            m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szHidVid);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 设备PID: DEVICE_SET_%d->ProductId = %s",
            m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szHidPid);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 通讯协议: DEVICE_SET_%d->Protocol = %d",
            m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.wProtocol);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 命令下发超时时间(单位:毫秒): DEVICE_SET_%d->SndTimeOut = %d",
            m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.nOtherParam[0]);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 命令接收超时时间(单位:毫秒): DEVICE_SET_%d->RcvTimeOut = %d",
            m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.nOtherParam[1]);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t Open失败时返回值(0原样返回/1返回SUCCESS): OPEN_CONFIG->OpenFailRet = %d",
            m_stConfig.wOpenFailRet);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t Open后复位卡动作(0:不支持,1:无动作,2:持卡,3:退卡,4:吞卡): OPEN_CONFIG->OpenResetCardAction = %d",
            m_stConfig.wOpenResetCardAction);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t Reset时卡动作(0无动作/1退卡/2吞卡): RESET_CONFIG->ResetCardAction = %d",
            m_stConfig.wResetCardAction);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功): RESET_CONFIG->ResetFailReturn = %d",
            m_stConfig.wResetFailReturn);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 是否支持回收功能(0不支持/1支持): RETAIN_CONFIG->RetainSupp = %d",
            m_stConfig.wRetainSupp);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 吞卡计数: RETAIN_CONFIG->RetainCardCount = %d",
            m_stConfig.wRetainCardCount);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 回收将满报警阀值: RETAIN_CONFIG->RetainThreshold = %d",
            m_stConfig.wRetainThreshold);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 是否支持写磁(1:支持,0:不支持): READCARDER_COFNIG->CanWriteTrack = %d",
            m_stConfig.wCanWriteTrack);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 磁通感应器是否可用(0:不可用,1:可用): READCARDER_COFNIG->FluxSensorSupp = %d",
            m_stConfig.wFluxSensorSupp);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 退卡到出口后是否支持重新吸入读写(0:不支持,1:支持): READCARDER_COFNIG->RWAccFollowingEject = %d",
            m_stConfig.wRWAccFollowingEject);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 是否需要支持抖动功能(0:不支持,1:支持): READCARDER_COFNIG->NeedWobble = %d",
            m_stConfig.wNeedWobble);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 掉电时卡的控制方式(0:退卡到前出口;1:写磁时不退卡,其他情况退卡;2:无动作,不退);"
                    "3:退卡到前出口,30秒未取则吞卡;4: 吞卡): READCARDER_COFNIG->PowerOffCardAction = %d",
            m_stConfig.stDevOpenMode.nOtherParam[0]);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 掉电时写卡命令处理(0:停止写;1:不停止写): READCARDER_COFNIG->PowerOffStopWrite = %d",
            m_stConfig.stDevOpenMode.nOtherParam[1]);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 初始化时是否测试闸门开关(0:不测试,1:测试): READCARDER_COFNIG->InitTestFloorPush = %d",
            m_stConfig.stDevOpenMode.nOtherParam[2]);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 是否启用硬件回收计数(0:不启用,1:启用): READCARDER_COFNIG->DevRetainCountEnable = %d",
            m_stConfig.stDevOpenMode.nOtherParam[3]);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 进卡超时时间(0:无超时,>0:指定超时,单位:秒): READCARDER_COFNIG->InCardTimeOut = %d",
            m_stConfig.dwInCardTimeOut);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 退卡时无卡是否报MediaRemoved事件(0不上报,1上报): READCARDER_COFNIG->PostRemovedAftEjectFixed = %d",
            m_stConfig.wPostRemovedAftEjectFixed);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 回收盒满是否支持进卡(0:不支持,1:支持): READCARDER_COFNIG->AcceptWhenCardFull = %d",
            m_stConfig.wAcceptWhenCardFull);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 卡是否有磁(0:无,1:有): READCARDER_COFNIG->FluxInActive = %d",
            m_stConfig.wFluxInActive);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 进卡检查模式(0:所有卡可进,1:判磁或IC,2:判磁且IC): READCARDER_COFNIG->SupportPredictIC = %d",
            m_stConfig.wSupportPredictIC);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 防盜钩功能支持(0:不支持,1:支持): READCARDER_COFNIG->TamperSensorSupport = %d",
            m_stConfig.wTamperSupp);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 吸卡时后出口进卡处理(0:不处理,1:正常吸卡,2:吞卡): READCARDER_COFNIG->AfterInCardOpen = %d",
            m_stConfig.wAfterInCardOpen);
    qsIniPrt.append(szBuff);
    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 是否支持卡口异物检知功能(0:不支持,1:支持): SKIMMING_CONFIG->SkimmingSupp = %d",
            m_stConfig.wSkimmingSupp);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 检测间隔时间(单位:秒): SKIMMING_CONFIG->SkimmingMonitorInterval = %d",
            m_stConfig.wSkimmingMonitorInterval);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 判故障持续故障时间(单位:秒): SKIMMING_CONFIG->SkimmingErrorDuration = %d",
            m_stConfig.wSkimmingErrorDuration);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 判正常持续正常时间(缺省:3, 单位:秒): SKIMMING_CONFIG->SkimmingNormalDuration = %d",
            m_stConfig.wSkimmingNormalDuration);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 防逗卡保护功能是否支持(0:不支持,1:支持): FRAUDDETE_CONFIG->TeaseCardProtectSupp = %d",
            m_stConfig.wTeaseCardProtectSupp);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 防逗卡保护生效的进卡次数上限: FRAUDDETE_CONFIG->FraudInCardCount = %d",
            m_stConfig.wTeaseInCardCount);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 防逗卡保护生效后持续时间: FRAUDDETE_CONFIG->TeaseCardProtectDate = %d",
            m_stConfig.wTeaseCardProtectDate);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 是否启用测试模式(0:不启用,1:启用): TESTER_CONFIG->TestModeIsSup = %d",
            m_stConfig.wTestModeIsSup);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 测试模式处理1: 读卡数据完成后返回先返Insert事件,再返Complete: "
                    "TESTER_CONFIG->TestInsertComplete = %d",
            m_stConfig.nTestInsertComplete);
    qsIniPrt.append(szBuff);

    Log(ThisModule, __LINE__, "IDC INI配置取得如下: %s", qsIniPrt.toStdString().c_str());

    return WFS_SUCCESS;
}

void CXFS_IDC::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 新增IC卡操作
    m_stStatus.fwChipPower  = WFS_IDC_CHIPNODEVICE;
    m_stStatus.fwMedia = WFS_IDC_MEDIANOTPRESENT;
    m_stStatus.fwSecurity = WFS_IDC_SECNOTSUPP;
    m_stStatus.lpszExtra = nullptr;

    // 吞卡计数初始化
    if (m_stConfig.wRetainSupp == 0)    // 不支持吞卡
    {
        m_stStatus.usCards = 0;
        m_stStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    } else
    {
        m_stStatus.usCards = m_stConfig.wRetainCardCount;
        if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainFull)  // 回收满
        {
            m_stStatus.fwRetainBin = WFS_IDC_RETAINBINFULL;
        } else
        if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainThreshold) // 回收将满
        {
            m_stStatus.fwRetainBin = WFS_IDC_RETAINBINHIGH;
        } else
        {
            m_stStatus.fwRetainBin = WFS_IDC_RETAINBINOK;
        }
    }
}

void CXFS_IDC::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stCaps.wClass = WFS_SERVICE_CLASS_IDC;
    m_stCaps.fwType = WFS_IDC_TYPEMOTOR;
    m_stCaps.bCompound = FALSE;
    m_stCaps.fwReadTracks = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
    m_stCaps.fwWriteTracks = (m_stConfig.wCanWriteTrack == 1 ?
                            (WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3) : WFS_IDC_NOTSUPP);
    // 新增IC卡操作
    m_stCaps.fwChipProtocols = WFS_IDC_CHIPT0 | WFS_IDC_CHIPT1;
    m_stCaps.fwChipPower = WFS_IDC_CHIPPOWERCOLD | WFS_IDC_CHIPPOWERWARM | WFS_IDC_CHIPPOWEROFF;
    m_stCaps.fwSecType = WFS_IDC_SECNOTSUPP;
    m_stCaps.fwPowerOnOption = WFS_IDC_RETAIN;
    m_stCaps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_stCaps.fwWriteMode = (m_stConfig.wCanWriteTrack == 1 ?
                              (WFS_IDC_LOCO | WFS_IDC_HICO | WFS_IDC_AUTO) : WFS_IDC_NOTSUPP);
    m_stCaps.usCards = (m_stConfig.wRetainSupp == 1 ? m_stConfig.wRetainFull : 0);
    m_stCaps.bFluxSensorProgrammable = (m_stConfig.wFluxSensorSupp == 0 ? FALSE : TRUE);
    m_stCaps.bReadWriteAccessFollowingEject = (m_stConfig.wRWAccFollowingEject == 0 ? FALSE : TRUE);
    m_stCaps.lpszExtra = nullptr;
}

// 更新扩展数据
void CXFS_IDC::UpdateExtra()
{
    CHAR szFWVersion[64] = { 0x00 };
    LONG lFWVerSize = 0;

    // 组织状态扩展数据
    //m_cStatExtra.Clear();
    m_cStatExtra.AddExtra("VRTCount", "2");
    m_cStatExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cStatExtra.AddExtra("VRTDetail[01]", (char*)byDevIDCVRTU);

    // 取固件版本写入扩展数据
    m_pIDCDev->GetVersion(GET_VER_FW, szFWVersion, 64);
    if (strlen(szFWVersion) > 0)
    {
        m_cStatExtra.AddExtra("VRTCount", "3");
        m_cStatExtra.AddExtra("VRTDetail[02]", szFWVersion);
    }

    m_cCapsExtra.Clear();
    m_cCapsExtra.CopyFrom(m_cStatExtra);
}

// 设备状态实时更新
WORD CXFS_IDC::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题

    INT     nRet = IDC_SUCCESS;

    WORD fwDevice = WFS_IDC_DEVHWERROR;
    DWORD dwHWAct = WFS_ERR_ACT_NOACTION;
    CHAR  szHWDesc[1024] = { 0x00 };

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFireTaken          = FALSE;
    BOOL    bNeedFireMediaInserted  = FALSE;

    // 取设备状态
    STDEVIDCSTATUS stDevStatus;
    nRet = m_pIDCDev->GetStatus(stDevStatus);

    //----------------------设备状态处理----------------------
    // 返回值处理
    if (nRet != IDC_SUCCESS)
    {
        SetErrorDetail();
    }

    //----------------------Device状态处理----------------------
    m_stStatus.fwDevice = ConvertDeviceStatus2WFS(stDevStatus.wDevice);
    if (m_stStatus.fwDevice == WFS_IDC_DEVOFFLINE)
    {
        m_stStatus.fwMedia = WFS_IDC_MEDIAUNKNOWN;
    }

    // Device == ONLINE && 有命令在执行中,设置Device = BUSY
    /*if (m_stStatus.fwDevice == WFS_PTR_DEVONLINE && m_bCmdRuning == TRUE)    // 命令执行中
    {
        m_stStatus.fwDevice == WFS_PTR_DEVBUSY;
    }*/

    //----------------------Media状态处理----------------------
    m_stStatus.fwMedia = ConvertMediaStatus2WFS(stDevStatus.wMedia);
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAJAMMED)
    {
        m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
    }

    //----------------------RetainBin状态处理----------------------
    //m_stStatus.fwRetainBin = ConvertRetainBinStatus2WFS(stDevStatus.wRetainBin);
    // 回收状态参考INI配置
    if (m_stConfig.wRetainSupp == 1)
    {
        if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainFull)
        {
            if (m_stStatus.fwRetainBin != WFS_IDC_RETAINBINFULL)
            {
                FireRetainBinThreshold(WFS_IDC_RETAINBINFULL);
                m_stStatus.fwRetainBin = WFS_IDC_RETAINBINFULL;
            }
        } else
        if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainThreshold)
        {
            if (m_stStatus.fwRetainBin != WFS_IDC_RETAINBINHIGH)
            {
                FireRetainBinThreshold(WFS_IDC_RETAINBINHIGH);
                m_stStatus.fwRetainBin = WFS_IDC_RETAINBINHIGH;
            }
        } else
        {
            if (m_stStatus.fwRetainBin != WFS_IDC_RETAINBINOK)
            {
                FireRetainBinThreshold(WFS_IDC_RETAINBINOK);
                m_stStatus.fwRetainBin = WFS_IDC_RETAINBINOK;
            }
        }
    } else
    {
        m_stStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    }

    //----------------------Security状态处理----------------------
    m_stStatus.fwSecurity = ConvertSecurityStatus2WFS(stDevStatus.wSecurity);

    //----------------------ChipPower状态处理----------------------
    m_stStatus.fwChipPower = ConvertChipPowerStatus2WFS(stDevStatus.wChipPower);

    //----------------------检查异物检知异常----------------------
    if (m_stStatus.fwDevice == WFS_IDC_DEVONLINE &&
        m_bIsHaveSkimming == TRUE)
    {
        m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
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

    // MediaInsert事件检查
    /*if (m_stStatusOLD.fwMedia == WFS_PTR_MEDIANOTPRESENT &&
        (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT || m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING))
    {
        bNeedFireMediaInserted = TRUE;
    }*/

    // 后出口是否有卡
    if (m_bAfteIsHaveCard != (stDevStatus.wOtherCode[0] == 1 ? TRUE : FALSE))
    {
        Log(ThisModule, __LINE__, "读卡器后出口:%s -> %s",
            m_bAfteIsHaveCard == TRUE ? "有卡" : "无卡", m_bAfteIsHaveCard == TRUE ? "无卡" : "有卡");
        m_bAfteIsHaveCard = (stDevStatus.wOtherCode[0] == 1 ? TRUE : FALSE);
    }



    //----------------------事件上报----------------------
    // 硬件故障事件
    if (bNeedFireHWError)
    {
        if (stDevStatus.wOtherCode[1] == 1)         // 防逗卡保护生效
        {
            if (m_stConfig.wTeaseCardProtectDate > 0)
            {
                dwHWAct = WFS_ERR_ACT_RESET;        // 需要复位
                sprintf(szHWDesc, "Tease Card Protect StartUp, Need Reset");
            } else
            {
                dwHWAct = WFS_ERR_ACT_NOACTION;     // 自动复位
                sprintf(szHWDesc, "Tease Card Protect StartUp, Auto Reset");
            }
        } else
        {
            if (m_bIsHaveSkimming == TRUE)          // 检知异物
            {
                dwHWAct = WFS_ERR_ACT_HWCLEAR;      // 硬件错误
                sprintf(szHWDesc, "Detection Have Abnormal Items");
            }
        }

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
        Log(ThisModule, __LINE__, "IDC: 介质插入");
        sprintf(szFireBuffer + strlen(szFireBuffer), "MediaInsert|");
    }

    // Taken事件
    if (bNeedFireTaken)
    {
        FireMediaRemoved();
        m_enWaitTaken = WTF_NONE;     // Taken标记复位
        Log(ThisModule, __LINE__, "IDC: 介质取走");
        sprintf(szFireBuffer + strlen(szFireBuffer), "MediaRemoved|");
    }

    // 比较两次状态记录LOG
    if (m_stStatus.Diff(m_stStatusOLD) == true)
    {
        Log(ThisModule, __LINE__, "IDC: 状态结果比较: Device:%d->%d%s|Media:%d->%d%s|RetainBin:%d->%d%s|"
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

// 读卡器设备初始化
HRESULT CXFS_IDC::DeviceIDCInit(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;

    // 执行初始化(复位)
    nRet = m_pIDCDev->Reset(MEDIA_NOTACTION, m_stConfig.wNeedWobble/*抖动功能*/);
    if (nRet != IDC_SUCCESS)    // 复位失败
    {
        if (bReConn == FALSE)
        {
            Log(ThisModule, __LINE__, "IDC: 复位: ->Reset(%d, %d) Fail, ErrCode: %s, Return: %d",
                MEDIA_NOTACTION, m_stConfig.wNeedWobble, ConvertDevErrCodeToStr(nRet),
                ConvertDevErrCode2WFS(nRet));
        }
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    return WFS_SUCCESS;
}

// Open后复位
HRESULT CXFS_IDC::InnerOpenReset()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;

    UpdateDeviceStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 卡口/内部有卡
    if (CHK_MEDIA_ISHAVE(m_stStatus.fwMedia))
    {
        // Open后复位卡动作(0:不支持, 1:无动作, 2:持卡, 3:退卡, 4:吞卡, 缺省0)
        if (m_stConfig.wOpenResetCardAction == 3)   // 退卡
        {
            if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)    // 卡在出口
            {
                m_stStatus.fwMedia = WFS_IDC_MEDIAENTERING;
                m_enWaitTaken = WTF_TAKEN;
                Log(ThisModule, __LINE__, "IDC: Open后复位: 退卡: 卡已在出口, 不执行退卡操作.");
            } else
            {
                hRet = InnerReset(WFS_IDC_EJECT);
                if (hRet != WFS_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "IDC: Open后复位: 内部有卡: 退卡: ->InnerReset(%d) Fail, ErrCode: %d, Return: %d",
                        WFS_IDC_EJECT, hRet, hRet);
                    return hRet;
                }
            }
        } else
        if (m_stConfig.wOpenResetCardAction == 4)   // 吞卡
        {
            hRet = InnerReset(WFS_IDC_RETAIN);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "IDC: Open后复位: 内部有卡: 吞卡: ->InnerReset(%d) Fail, ErrCode: %d, Return: %d",
                    WFS_IDC_RETAIN, hRet, hRet);
                return hRet;
            }
        }
    } else
    {
        // 设备初始化(无动作)
        hRet = InnerReset(WFS_IDC_NOACTION);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "IDC: Open后复位: 内部无卡: 无动作初始化: ->InnerReset(%d) Fail, ErrCode: %d, Return: %d",
                WFS_IDC_NOACTION, hRet, hRet);
            return hRet;
        }
    }

    return WFS_SUCCESS;
}

// 读卡子处理
HRESULT CXFS_IDC::InnerAcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;
    BOOL bMagneticCard = FALSE;     // 是否磁条卡

    m_bChipPowerOff = FALSE;

    // 回收盒检查(INI)
    if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainFull)     // 回收满
    {
        if (m_stConfig.wAcceptWhenCardFull == 1)
        {
            Log(ThisModule, __LINE__,
                "读卡子处理: 当前回收计数[%d] >= INI设置回收满阀值[%d], INI设置回收满允许读卡, Not Return.",
                m_stConfig.wRetainCardCount, m_stConfig.wRetainFull);
        } else
        {
            Log(ThisModule, __LINE__,
                "读卡子处理: 当前回收计数[%d] >= INI设置回收满阀值[%d], Return: %d.",
                m_stConfig.wRetainCardCount, m_stConfig.wRetainFull, WFS_ERR_IDC_RETRACTBINFULL);
            SetErrorDetail(0, (LPSTR)EC_XFS_BoxFull);
            return WFS_ERR_IDC_RETRACTBINFULL;
        }
    }

    // 需要进卡操作
    MEDIA_ACTION enMediaAction = MEDIA_ACCEPT;   // 进卡入参:磁条卡(有磁)
    if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT ||    // 内部无卡
        m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)        // 卡在出口
    {
        // 增加是否接受磁卡的判断
        if ((dwReadOption & WFS_IDC_FLUXINACTIVE) == WFS_IDC_FLUXINACTIVE ||    // 磁功能失效
            m_stConfig.wFluxInActive == 0)                                      // INI设置无磁
        {
            enMediaAction = MEDIA_ACCEPT_IC;    // 进卡入参:IC卡(无磁)
        }

        // 进卡前: 支持抖动进卡则设置开启,否者关闭
        m_stConfig.wNeedWobble == 1 ? m_pIDCDev->SetData(SET_DEV_WOBBLE_OPEN) :
                                      m_pIDCDev->SetData(SET_DEV_WOBBLE_CLOSE);
        nRet = m_pIDCDev->MediaControl(enMediaAction, dwTimeOut);    // 进卡
        m_pIDCDev->SetData(SET_DEV_WOBBLE_CLOSE);   // 进卡后: 关闭抖动进卡
        if (nRet != IDC_SUCCESS && nRet != ERR_IDC_INCARD_AFTER)
        {
            Log(ThisModule, __LINE__, "IDC: 进卡: ->MeidaControl(%d, %d) Fail, ErrCode: %s, Return: %d",
                enMediaAction, dwTimeOut, ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
            SetErrorDetail();
            return ConvertDevErrCode2WFS(nRet);
        } else
        {
            if (nRet == ERR_IDC_INCARD_AFTER)   // 后出口进卡,回收
            {
                // 吸卡时后出口进卡处理(0:不处理, 1:正常吸卡, 2:吞卡, 缺省1)
                if (m_stConfig.wAfterInCardOpen ==1)    // 正常吸卡
                {
                    Log(ThisModule, __LINE__, "IDC: 后出口: 进卡: ->MeidaControl(%d, %d) Succ",
                        enMediaAction, dwTimeOut);
                } else
                if (m_stConfig.wAfterInCardOpen == 2)
                {
                    Log(ThisModule, __LINE__, "IDC: 后出口: 进卡: ->MeidaControl(%d, %d) Succ, INI设定: 吞卡...",
                        enMediaAction, dwTimeOut);

                    hRet = InnerRetainCard();
                    if (hRet != WFS_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "IDC: 后出口: 进卡: INI设定吞卡: ->InnerRetainCard() fail, "
                            "RetCode: %d, Return: %d.", hRet, hRet);
                        return hRet;
                    }
                }
            } else
            {
                Log(ThisModule, __LINE__, "IDC: 进卡: ->MeidaControl(%d, %d) Succ",
                    enMediaAction, dwTimeOut);
            }


            m_stStatus.fwMedia = WFS_IDC_MEDIAPRESENT;            
            if (m_stConfig.nTestInsertComplete < 1) // 非测试模式上报Inserted
            {
                FireCardInserted();
            }
            m_enWaitTaken = WTF_TAKEN;
        }
    } else
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT ||   // 内部有卡
        m_stStatus.fwMedia == WFS_IDC_MEDIALATCHED)     // 内部有卡且锁定
    {
        //创自读卡器在异常状态时，卡位置有可能不正确，移动到读卡位置
        /*if(m_strDevName == "CRT"){
            nRet = m_pIDCDev->Init(CARDACTION_NOACTION, (WobbleAction)m_stConfig.usNeedWobble);
            UpdateDevStatus(nRet);
            if(nRet < 0){
                return WFS_ERR_HARDWARE_ERROR;
            }
        }*/
    }

    // 读磁道/芯片数据
    STMEDIARW stMediaRead;
    stMediaRead.dwRWType = ConvertWFS2MediaRW(dwReadOption);
    stMediaRead.dwTimeOut = dwTimeOut;
    nRet = m_pIDCDev->MediaReadWrite(MEDIA_READ, stMediaRead);
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__, "IDC: 读卡: ->MediaReadWrite(%d) Fail, ErrCode: %d, Return: %d",
                MEDIA_READ, nRet, ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        m_stStatus.fwChipPower  = WFS_IDC_CHIPONLINE;
        m_stStatus.fwMedia = WFS_IDC_MEDIAPRESENT;
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
    if ((dwReadOption & WFS_IDC_CHIP) == WFS_IDC_CHIP)
    {
        if (stMediaRead.stData[3].wResult == RW_RESULT_SUCC)
        {
            SetTrackInfo(WFS_IDC_CHIP, WFS_IDC_DATAOK,
                         stMediaRead.stData[3].wSize, (BYTE *)stMediaRead.stData[3].szData);
        } else
        {
            SetTrackInfo(WFS_IDC_CHIP, ConvertMediaRWResult2WFS(stMediaRead.stData[3].wResult), 0, nullptr);
        }
    }

    if (m_stConfig.nTestInsertComplete > 0) // 测试模式1处理
    {
        FireCardInserted(); // 上报Inserted
        QTime qtTimeCurr1 = QTime::currentTime();     // 取当前时间
        // 循环检测是否放入卡
        while(1)
        {
            QCoreApplication::processEvents();
            if (qtTimeCurr1.msecsTo(QTime::currentTime()) >=
                m_stConfig.nTestInsertComplete) // 时间差(毫秒)
            {
                break;
            }
            usleep(1000);     // 休眠1毫秒
        }
    }

    return WFS_SUCCESS;
}

// 写卡子处理
HRESULT CXFS_IDC::InnerAcceptAndWriteTrack(DWORD dwReadOption, DWORD dwTimeOut)
{
    // 初始化

    // 清除硬件回收计数(CRT-350N)

    return WFS_SUCCESS;
}

// 退卡子处理
HRESULT CXFS_IDC::InnerEject()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    if (m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT)    // 卡在内部
    {
        nRet = m_pIDCDev->MediaControl(MEDIA_EJECT);
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__, "IDC: 退卡: ->MediaControl(%d) Fail, ErrCode: %s, Return: %d",
                MEDIA_EJECT, ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
            SetErrorDetail();
            return ConvertDevErrCode2WFS(nRet);
        } else
        {
            usleep(500 * 1000);     // Eject命令下发后等待500毫秒,确保取到有效的卡状态
            UpdateDeviceStatus();   // 更新当前状态
            if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING ||  // 卡在出口
                m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)  // 内部无卡&&完全弹出
            {
                m_enWaitTaken = WTF_TAKEN;
            }
        }
    }

    return WFS_SUCCESS;
}

// 吞卡子处理
HRESULT CXFS_IDC::InnerRetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    nRet = m_pIDCDev->MediaControl(MEDIA_RETRACT);
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__, "IDC: 吞卡: ->MediaControl(%d) Fail, ErrCode: %s, Return: %d",
            MEDIA_EJECT, ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        usleep(500 * 1000);     // 吞卡命令下发后等待500毫秒,确保取到有效的卡状态
        UpdateDeviceStatus();   // 更新当前状态
        if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)      // 内部无卡
        {
            FireMediaRetained();            // 上报吞卡事件
            UpdateRetainCards(RETCNT_ADD_ONE); // 吞卡计数+1
        }
    }

    return WFS_SUCCESS;
}

// 复位子处理
// bIsCheck: 复位前是否进行检查
HRESULT CXFS_IDC::InnerReset(WORD wAction, BOOL bIsCheck)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;
    BOOL bIsHaveCard = FALSE;

    if (bIsCheck)
    {
        UpdateDeviceStatus(); // 更新当前设备介质状态

        // 复位前卡状态检查
        if (CHK_MEDIA_ISHAVE(m_stStatus.fwMedia))      // 卡在内部/出口
        {
            FireMediaDetected(WFS_IDC_CARDREADPOSITION);
            bIsHaveCard = TRUE;
        }

        // 指定回收同时有卡存在时: 回收盒检查
        if (wAction == WFS_IDC_RETAIN && bIsHaveCard == TRUE)
        {
            if (m_stConfig.wRetainSupp == 0)    // INI设置不支持吞卡
            {
                Log(ThisModule, __LINE__,
                    "回收: 参数吞卡: INI设置不支持吞卡, Return: %d.", WFS_ERR_UNSUPP_DATA);
                SetErrorDetail(0, (LPSTR)EC_XFS_RetainNoSupp);
                return WFS_ERR_UNSUPP_DATA;
            }

            if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainFull)     // 回收满
            {
                Log(ThisModule, __LINE__,
                    "复位: 参数吞卡: 当前回收计数[%d] > INI设置回收警阀值[%d], Return: %d.",
                    WFS_ERR_IDC_RETRACTBINFULL);
                SetErrorDetail(0, (LPSTR)EC_XFS_BoxFull);
                return WFS_ERR_IDC_RETRACTBINFULL;
            }

            // Taken标记归零
            m_enWaitTaken = WTF_NONE;
        }
    }

    MEDIA_ACTION enMediaAct;
    if (wAction == WFS_IDC_RETAIN)
    {
        enMediaAct = MEDIA_RETRACT;     // 吞卡
    } else
    if (wAction == WFS_IDC_NOACTION)
    {
        enMediaAct = MEDIA_NOTACTION;   // 无动作
    } else
    if (wAction == WFS_IDC_EJECT)
    {
        enMediaAct = MEDIA_EJECT;       // 退卡
    }

    // 执行复位
    nRet = m_pIDCDev->Reset(enMediaAct, m_stConfig.wNeedWobble/*抖动功能*/);
    if (nRet != IDC_SUCCESS)    // 复位失败
    {
        if (nRet == ERR_IDC_MED_JAMMED) // JAM
        {
            FireMediaDetected(WFS_IDC_CARDJAMMED);
        }

        Log(ThisModule, __LINE__, "IDC: 复位: ->Reset(%d, %d) Fail, ErrCode: %s, Return: %d",
            enMediaAct, m_stConfig.wNeedWobble, ConvertDevErrCodeToStr(nRet),
            ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        if (wAction == WFS_IDC_EJECT && bIsHaveCard == TRUE)   // 入参指定退卡
        {
            UpdateDeviceStatus();     // 更新当前状态
            if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING ||  // 卡在出口
                (bIsHaveCard == TRUE && m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT) ||
                m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)  // 内部无卡&&完全弹出
            {
                m_enWaitTaken = WTF_TAKEN;    // 设置Taken标记
            }
        } else
        if (wAction == WFS_IDC_RETAIN && bIsHaveCard == TRUE)   // 入参指定吞卡
        {
            UpdateDeviceStatus();     // 更新当前状态
            if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)      // 内部无卡
            {
                FireMediaRetained();            // 上报吞卡事件
                UpdateRetainCards(RETCNT_ADD_ONE); // 吞卡计数+1
            }
        }
    }

    return WFS_SUCCESS;
}

// 更新吞卡计数相关
INT CXFS_IDC::UpdateRetainCards(EN_RETAIN_CNT enMode, INT nCnt)
{
    THISMODULE(__FUNCTION__);

    BOOL bRet = FALSE;

    if (enMode == RETCNT_ADD_ONE)   // 吞卡数增1
    {
        m_stConfig.wRetainCardCount ++; // INI吞卡计数++
    } else
    if (enMode == RETCNT_RED_ONE)   // 吞卡数减1
    {
        if (m_stConfig.wRetainCardCount > 0)
        {
            m_stConfig.wRetainCardCount --;
        }
    } else
    if (enMode == RETCNT_CLEAR)     // 吞卡数清零
    {
        m_stConfig.wRetainCardCount = 0;
    } else
    if (enMode == RETCNT_SETSUM)    // RETCNT_SETSUM
    {
        bRet = m_cXfsReg.SetValue("RETAIN_CONFIG", "RetainFull",
                                  std::to_string(nCnt).c_str());
        if (bRet != TRUE)
        {
            Log(ThisModule, __LINE__, "IDC: 设置INI中回收满总数为[%d]失败", nCnt);
            return 1;
        }
        Log(ThisModule, __LINE__, "IDC: 设置INI中回收满总数为[%d]完成", nCnt);
        return 0;
    } else
    {
        return 0;
    }

    // 数据持久化(写入INI)
    bRet = m_cXfsReg.SetValue("RETAIN_CONFIG", "RetainCardCount",
                                   std::to_string(m_stConfig.wRetainCardCount).c_str());
    if (bRet != TRUE)
    {
        Log(ThisModule, __LINE__, "IDC: 设置INI中回收计数为[%d]失败", m_stConfig.wRetainCardCount);
        return 1;
    }
    Log(ThisModule, __LINE__, "IDC: 设置INI中回收计数为[%d]完成", m_stConfig.wRetainCardCount);
    return 0;
}

// 读卡应答数据处理
void CXFS_IDC::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
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
bool CXFS_IDC::GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho)
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

// 功能: 用指定FORM分析磁道数据
// 输入: pForm   : 指定FORM
// 返回: WFS_SUCCESS:成功, 否则失败。
HRESULT CXFS_IDC::SPParseData(SP_IDC_FORM *pForm)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    int iTrack;
    int Value[3];
    ZeroMemory(Value, sizeof(Value));
    char head[30] = {0};
    for (iTrack = 0; iTrack < 3; iTrack++)
    {
        BOOL bNeedTrack = (TracksToDataSourceOption(pForm->szTracks) & TrackIndexToDataSourceOption(iTrack)) &&
                          NeedTrack(pForm->szTracks, iTrack);
        sprintf(head, "TRACK%d", iTrack + 1);
        LPWFSIDCCARDDATA pCardData = m_CardDatas.Find(TrackIndexToDataSourceOption(iTrack));
        if (!pCardData)
        {
            if (bNeedTrack)
            {
                LogWrite(ThisModule, -1, IDS_NO_TRACK_DATA, pForm->FormName.c_str(), iTrack + 1);
                FireInvalidTrackData(WFS_IDC_DATAMISSING, head, NULL);
            }
            continue;
        }
        if (pCardData->wStatus == WFS_IDC_DATAOK)
            Value[iTrack] = true;
        else
        {
            if (bNeedTrack)
            {
                LogWrite(ThisModule, -1, IDS_TRACK_DATA_ERROR, iTrack + 1, pCardData->wStatus);
                FireInvalidTrackData(pCardData->wStatus, head, (char *)pCardData->lpbData);
            }
            continue;
        }

        vector<int> SepList;
        GenSepList((char *)pCardData->lpbData, pCardData->ulDataLength, pForm->cFieldSeparatorTracks[iTrack], SepList);

        CMultiString ms;
        char *pHead = head;
        FFLIT itField;
        for (itField = pForm->TracksFields[iTrack].begin(); itField != pForm->TracksFields[iTrack].end(); itField++)
        {
            // 分析字段数据写入ms中
            LPBYTE lpByte = pCardData->lpbData;
            int nLen = pCardData->ulDataLength;
            long nFieldLen = 0;
            long nFieldOffset = 0;
            if (!ComputeFieldInfo(lpByte, nLen, (*itField)->FieldSepIndices, (*itField)->nOffsets, SepList, nFieldOffset, nFieldLen))
            {
                LogWrite(ThisModule, -1, IDS_COMPUTE_FIELD_FAILED, (*itField)->FieldName.c_str());
                break;
            }
            char buf[1024];
            strncpy(buf, (const char *)lpByte + nFieldOffset, nFieldLen);
            buf[nFieldLen] = 0;
            char temp[1024];
            if (pHead)
            {
                sprintf(temp, "%s:%s=%s", pHead, (*itField)->FieldName.c_str(), buf);
            } else
            {
                sprintf(temp, "%s=%s", (*itField)->FieldName.c_str(), buf);
            }
            ms.Add(temp);
            pHead = NULL;
        }
        if (itField != pForm->TracksFields[iTrack].end())
        {
            if (bNeedTrack)
                FireInvalidTrackData(pCardData->wStatus, head, (char *)pCardData->lpbData);
            continue;
        }
        m_TrackData.Add(ms);
    }
    const char *p = pForm->szTracks;
    BOOL bSucc;
    if (!ComputeTracks(&p, Value, bSucc) || !bSucc)
    {
        LogWrite(ThisModule, -1, IDS_NO_ALL_TRACK, pForm->FormName.c_str(), pForm->sTracks.c_str(), Value[0], Value[1], Value[2]);
        hRet = WFS_ERR_IDC_INVALIDDATA;
    }
    return hRet;
}

// 设置ErrorDetail错误码
INT CXFS_IDC::SetErrorDetail(WORD wDevType, LPSTR lpCode)
{
    if (lpCode == nullptr)
    {
        CHAR szErrCode[1024] = { 0x00 };
        if (wDevType == 0)  // IDC
        {
            m_pIDCDev->GetData(GET_DEV_ERRCODE, szErrCode);
        } else
        if (wDevType == 1)  // CRM
        {
            m_pCRMDev->GetData(GET_DEV_ERRCODE, szErrCode);
        } else
        if (wDevType == 2)  // CRD
        {
            //m_pIDCDev->GetData(GET_DEV_ERRCODE, szErrCode);
        }

        m_clErrorDet.SetXFSErrCode(szErrCode + 1);
    } else
    {
        m_clErrorDet.SetXFSErrCode(lpCode);
    }

    return WFS_SUCCESS;
}
