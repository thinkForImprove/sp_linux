/***************************************************************
* 文件名称：XFS_FIDC_DEC.cpp
* 文件描述：非接读卡器模块子命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_FIDC.h"
#include "file_access.h"
#include "data_convertor.h"


//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_FIDC::InnerOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // Open前下传初始参数(非断线重连)
    if (bReConn == FALSE)
    {
        // 设置SDK路径
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pDev->SetData(SET_LIB_PATH, m_stConfig.szSDKPath);
        }

        // 设置设备打开模式
        m_pDev->SetData(SET_DEV_OPENMODE, &(m_stConfig.stDevOpenMode));

        // 设置图像参数
        m_pDev->SetData(SET_IMAGE_PAR, &(m_stConfig.stImageParam));
    }

    // 打开设备
    INT nRet = m_pDev->Open(DEVTYPE2STR(m_stConfig.wDeviceType));
    if (nRet != IDC_SUCCESS)
    {
        if (bReConn == FALSE)
        {
            Log(ThisModule, __LINE__, "Open[%s] fail, ErrCode = %d, Return: %d",
                DEVTYPE2STR(m_stConfig.wDeviceType), nRet, ConvertDevErrCode2WFS(nRet));
        }
        return ConvertDevErrCode2WFS(nRet);
    }

    ControlLight(GLIGHTS_ACT_CLOSE);  // 非接灯复原
    m_bChipPowerOff = FALSE;

    // 设备初始化
    nRet = m_pDev->Reset(MEDIA_NOTACTION);       // 初始化介质无动作
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备Open: 初始化: ->Init(%d) Fail, ErrCode: %d, Return: %d",
            MEDIA_NOTACTION, nRet, ConvertDevErrCode2WFS(nRet));
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        Log(ThisModule, __LINE__, "设备Open: 初始化: ->Init(%d) Succ",
            MEDIA_NOTACTION);
    }

    if (bReConn != TRUE)
    {
        //SetInit();      // 非断线重连时初始功能设置
    }

    // 组织扩展数据
    UpdateExtra();

    // 更新一次状态
    OnStatus();

    if (bReConn == TRUE)
    {
        Log(ThisModule, __LINE__, "断线重连成功, Extra=%s.", m_cStaExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s.", m_cStaExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}

// 加载DevXXX动态库
bool CXFS_FIDC::LoadDevDll(LPCSTR ThisModule)
{
    if (m_pDev == nullptr)
    {
        if (m_pDev.Load(m_stConfig.szDevDllName, "CreateIDevIDC", DEVTYPE2STR(m_stConfig.wDeviceType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DriverType=%d|%s, ERR:%s",
                m_stConfig.szDevDllName, m_stConfig.wDeviceType,
                DEVTYPE2STR(m_stConfig.wDeviceType), m_pDev.LastError().toUtf8().constData());
            return false;
        } else
        {
            Log(ThisModule, __LINE__, "加载库: DriverDllName=%s, DriverType=%d|%s, Succ.",
                m_stConfig.szDevDllName, m_stConfig.wDeviceType, DEVTYPE2STR(m_stConfig.wDeviceType));
        }
    }
    return (m_pDev != nullptr);
}

// 加载INI设置
void CXFS_FIDC::InitConfig()
{
    THISMODULE(__FUNCTION__);

    CHAR    szIniAppName[MAX_PATH];
    CHAR    szBuffer[MAX_PATH];
    INT     nTmp;

    // DevFIDC动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型
    m_stConfig.wDeviceType = m_cXfsReg.GetValue("DriverType", (DWORD)0);

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.wOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.wOpenFailRet != 0 && m_stConfig.wOpenFailRet != 1)
    {
        m_stConfig.wOpenFailRet = 0;
    }

    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.wDeviceType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    //-----------------------------------明泰设备参数获取-----------------------------------
    if (m_stConfig.wDeviceType == XFS_MT50)
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
        /*if (stDevOpenModeTmp.wProtocol != 0 && stDevOpenModeTmp.wProtocol != 1)
        {
            stDevOpenModeTmp.wProtocol = 0;
        }*/
        // 上电寻卡模式(0:全部卡片都响应该命令,除了在HALT状态的那些卡片, 1:在任何状态的全部卡片都将响应该命令, 缺省0)
        stDevOpenModeTmp.nOtherParam[0] = (WORD)m_cXfsReg.GetValue(szIniAppName, "DetectCardMode", (DWORD)0);
        if (stDevOpenModeTmp.nOtherParam[0] != 0 && stDevOpenModeTmp.nOtherParam[0] != 1)
        {
            stDevOpenModeTmp.nOtherParam[0] = 0;
        }

        memcpy(&(m_stConfig.stDevOpenMode), &stDevOpenModeTmp, sizeof(STDEVICEOPENMODE));
    }

    //-----------------------------------CONFIG下属性获取-----------------------------------
    // 退卡时无卡是否报MediaRemoved事件(0不上报, 1上报, 缺省1)
    m_stConfig.bPostRemovedAftEjectFixed = m_cXfsReg.GetValue("CONFIG", "PostRemovedAftEjectFixed", (DWORD)1) == 1;
    m_stConfig.wBeepControl = m_cXfsReg.GetValue("CONFIG", "BeepControl", (DWORD)0);
    // 排卡后未取走卡,灭灯超时时间(单位:秒, 缺省30秒)
    m_stConfig.nTakeCardTimeout = m_cXfsReg.GetValue("CONFIG", "TakeCardTimeout", (DWORD)30);


    //--------------------------READCARD_COFNIG下属性获取--------------------------------
    //-----------------------------------读卡器相关配置-----------------------------------
    // 是否支持写磁(0:不支持, 1:支持, 缺省0)
    m_stConfig.wCanWriteTrack = m_cXfsReg.GetValue("READCARD_COFNIG", "CanWriteTrack", (DWORD)0);
    if (m_stConfig.wCanWriteTrack != 0 && m_stConfig.wCanWriteTrack != 1)
    {
        m_stConfig.wCanWriteTrack = 0;
    }


    //-----------------设备鸣响设置定义----------------
    // 是否支持鸣响(0:不支持, 1:支持, 缺省1)
    m_stConfig.stConfig_Beep.wSupp = m_cXfsReg.GetValue("BEEP_CFG", "BeepSup", (DWORD)1);
    if (m_stConfig.stConfig_Beep.wSupp != 0 && m_stConfig.stConfig_Beep.wSupp != 1)
    {
        m_stConfig.stConfig_Beep.wSupp = 1;
    }

    // 鸣响方式(0:硬件控制, 1:SP发送鸣响控制, 缺省1)
    m_stConfig.stConfig_Beep.wCont = m_cXfsReg.GetValue("BEEP_CFG", "BeepCont", (DWORD)1);
    if (m_stConfig.stConfig_Beep.wCont != 0 && m_stConfig.stConfig_Beep.wCont != 1)
    {
        m_stConfig.stConfig_Beep.wCont = 1;
    }

    // 一次鸣响时间(缺省100,单位:毫秒)
    m_stConfig.stConfig_Beep.wMesc = m_cXfsReg.GetValue("BEEP_CFG", "BeepMesc", (DWORD)100);
    if (m_stConfig.stConfig_Beep.wMesc < 0 || m_stConfig.stConfig_Beep.wMesc > 65536)
    {
        m_stConfig.stConfig_Beep.wMesc = 1000;
    }

    // 鸣响频率/鸣响间隔(缺省1000,单位:毫秒)
    m_stConfig.stConfig_Beep.wInterval = m_cXfsReg.GetValue("BEEP_CFG", "BeepInterval", (DWORD)1000);
    if (m_stConfig.stConfig_Beep.wInterval < 0 || m_stConfig.stConfig_Beep.wInterval > 65536)
    {
        m_stConfig.stConfig_Beep.wInterval = 1000;
    }

    // 每次鸣响的次数(缺省1)
    m_stConfig.stConfig_Beep.wCount = m_cXfsReg.GetValue("BEEP_CFG", "BeepCount", (DWORD)1);
    if (m_stConfig.stConfig_Beep.wCount < 0 || m_stConfig.stConfig_Beep.wCount > 65536)
    {
        m_stConfig.stConfig_Beep.wCount = 1;
    }


    //-----------------指示灯设置定义----------------
    // 是否支持指示灯(0:不支持, 1:支持, 缺省0)
    m_stConfig.stConfig_Light.wSupp = m_cXfsReg.GetValue("LIGHT_CFG", "LightSup", (DWORD)0);
    if (m_stConfig.stConfig_Light.wSupp != 0 && m_stConfig.stConfig_Light.wSupp != 1)
    {
        m_stConfig.stConfig_Light.wSupp = 0;
    }

    // 指示灯控制方式(0:硬件控制, 1:SP发送控制, 缺省1)
    m_stConfig.stConfig_Light.wCont = m_cXfsReg.GetValue("LIGHT_CFG", "LightCont", (DWORD)1);
    if (m_stConfig.stConfig_Light.wCont != 0 && m_stConfig.stConfig_Light.wCont != 1)
    {
        m_stConfig.stConfig_Light.wCont = 0;
    }

    // 指示灯动作间隔时间,包含快中慢三种(缺省50, 单位:毫秒)
    m_stConfig.stConfig_Light.wFaseDelayTime = m_cXfsReg.GetValue("LIGHT_CFG", "FastDelayTime", (DWORD)6);
    m_stConfig.stConfig_Light.wMiddleDelayTime = m_cXfsReg.GetValue("LIGHT_CFG", "MiddleDelayTime", (DWORD)12);
    m_stConfig.stConfig_Light.wSlowDelayTime = m_cXfsReg.GetValue("LIGHT_CFG", "SlowDelayTime", (DWORD)20);


    //-----------------IMAGE_CONFIG设置定义----------------
    // 证件头像保存路径(缺省路径为当前用户主目录下的image目录)
    strcpy(m_stConfig.stImageParam.szIDCardImgSavePath,
           m_cXfsReg.GetValue("IMAGE_CONFIG", "IDCardImageSavePath", ""));
    if (strlen(m_stConfig.stImageParam.szIDCardImgSavePath) < 1)
    {
        MSET_0(m_stConfig.stImageParam.szIDCardImgSavePath);
        sprintf(m_stConfig.stImageParam.szIDCardImgSavePath, "%s/image", getenv("HOME"));
    }
    if (FileAccess::create_directory_by_path(m_stConfig.stImageParam.szIDCardImgSavePath, FALSE) != TRUE)
    {
        Log(ThisModule, __LINE__,
            "FileAccess::create_directory_by_path(FALSE) 创建目录[%s]失败. Not Return Error. ",
            m_stConfig.stImageParam.szIDCardImgSavePath);
    }

    // 证件保存头像名(包含扩展名,不包含路径,空为不指定,用缺省值)
    strcpy(m_stConfig.stImageParam.szIDCardImgName,
           m_cXfsReg.GetValue("IMAGE_CONFIG", "IDCardImageSaveName", "IDCardHeadImg.bmp"));

}

// 设备状态更新
WORD CXFS_FIDC::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题

    INT     nRet = IDC_SUCCESS;

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFireTaken          = FALSE;
    BOOL    bNeedFireMediaInserted  = FALSE;

    STDEVIDCSTATUS stDevStatus;
    nRet = m_pDev->GetStatus(stDevStatus);

    //----------------------设备状态处理----------------------
    // 返回值处理
    switch (nRet)
    {

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
    m_stStatus.fwRetainBin = ConvertRetainBinStatus2WFS(stDevStatus.wRetainBin);

    //----------------------Security状态处理----------------------
    m_stStatus.fwSecurity = ConvertSecurityStatus2WFS(stDevStatus.wSecurity);

    //----------------------ChipPower状态处理----------------------
    m_stStatus.fwChipPower = ConvertChipPowerStatus2WFS(stDevStatus.wChipPower);

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
    if (m_WaitTaken == WTF_TAKEN)   // Taken事件检查
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
       m_WaitTaken = WTF_NONE;     // Taken标记复位

       ControlLight(GLIGHTS_ACT_CLOSE); // 关闭非接灯
       if(m_stConfig.wBeepControl == 1)
       {
           ControlBeep();
       }
        m_ulTakeMonitorStartTime = 0;
        Log(ThisModule, __LINE__, "介质取走");
        sprintf(szFireBuffer + strlen(szFireBuffer), "MediaRemoved|");
    }

    // 非接灯超时处理
    if (m_ulTakeMonitorStartTime > 0 && m_stConfig.nTakeCardTimeout > 0)
    {
        if(CQtTime::GetSysTick() - m_ulTakeMonitorStartTime > (ULONG)(m_stConfig.nTakeCardTimeout * 1000))
        {
            ControlLight(GLIGHTS_ACT_CLOSE);//关闭非接灯
            m_ulTakeMonitorStartTime = 0;
        }
    }

    // 比较两次状态记录LOG
    //if (memcmp(&m_stStatusOLD, &m_stStatus, sizeof(WFSPTRSTATUS)) != 0)
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

void CXFS_FIDC::InitStatus()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 新增IC卡操作
    m_stStatus.fwChipPower  = WFS_IDC_CHIPNODEVICE;
    m_stStatus.fwMedia = WFS_IDC_MEDIANOTPRESENT;
    m_stStatus.fwSecurity = WFS_IDC_SECNOTSUPP;
    m_stStatus.usCards = m_cXfsReg.GetValue("CONFIG", "card_num",  m_stStatus.usCards);
    m_stStatus.lpszExtra = NULL;

    m_stStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
}

void CXFS_FIDC::InitCaps()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_Caps.wClass = WFS_SERVICE_CLASS_IDC;
    m_Caps.fwType = WFS_IDC_TYPECONTACTLESS;
    m_Caps.bCompound = FALSE;
    m_Caps.fwReadTracks = WFS_IDC_NOTSUPP;
    m_Caps.fwWriteTracks = WFS_IDC_NOTSUPP;
    // 新增IC卡操作
    m_Caps.fwChipProtocols = WFS_IDC_CHIPT0 | WFS_IDC_CHIPT1;
    m_Caps.fwChipPower = WFS_IDC_CHIPPOWERCOLD | WFS_IDC_CHIPPOWERWARM | WFS_IDC_CHIPPOWEROFF;
    m_Caps.fwSecType = WFS_IDC_SECNOTSUPP;
    m_Caps.fwPowerOnOption = WFS_IDC_NOACTION;
    m_Caps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_Caps.fwWriteMode = (m_stConfig.wCanWriteTrack == 1 ?
                          (WFS_IDC_LOCO | WFS_IDC_HICO | WFS_IDC_AUTO) : WFS_IDC_NOTSUPP);
    m_Caps.usCards                        = 0;
    m_Caps.bFluxSensorProgrammable        = FALSE;
    m_Caps.bReadWriteAccessFollowingEject = FALSE;
    m_Caps.lpszExtra = NULL;
}

// ReadRawData子处理
HRESULT CXFS_FIDC::InnerReadRawData(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    m_bChipPowerOff = FALSE;
    //BOOL bMagneticCard = FALSE;

    ControlLight(GLIGHTS_ACT_FLASH);  // 打开非接灯,闪烁

    MEDIA_ACTION enMediaAction = MEDIA_ACCEPT_IC; // IC卡
    if ((dwReadOption & WFS_IDC_FRONTIMAGE) == WFS_IDC_FRONTIMAGE ||
        (dwReadOption & WFS_IDC_BACKIMAGE) == WFS_IDC_BACKIMAGE)
    {
        enMediaAction = MEDIA_ACCEPT_IDCARD;    // 身份证
    }

    nRet = m_pDev->MediaControl(enMediaAction, dwTimeOut);    // 进卡
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__, "进卡: ->MeidaControl(%d, %d) Fail, ErrCode: %d, Return: %d",
            enMediaAction, dwTimeOut, nRet, ConvertDevErrCode2WFS(nRet));
        ControlLight(GLIGHTS_ACT_CLOSE);  // 关闭非接灯
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        ControlBeep();                  // 设备鸣响
        ControlLight(GLIGHTS_ACT_OPEN); // 非接灯变为常亮
        Log(ThisModule, __LINE__, "进卡: ->MeidaControl(%d, %d) Succ",
            enMediaAction, dwTimeOut);
        m_stStatus.fwMedia = WFS_IDC_MEDIAPRESENT;
        FireCardInserted();
        m_WaitTaken = WTF_TAKEN;
    }

    // 读芯片数据(激活IC卡)
    STMEDIARW stMediaRead;
    stMediaRead.dwRWType = ConvertWFS2MediaRW(dwReadOption);
    stMediaRead.dwTimeOut = dwTimeOut;
    stMediaRead.stData[3].wSize = sizeof(stMediaRead.stData[3].szData);
    nRet = m_pDev->MediaReadWrite(MEDIA_READ, stMediaRead);
    if (nRet != IDC_SUCCESS)
    {
        m_stStatus.fwChipPower  = WFS_IDC_CHIPNODEVICE;
        if (nRet == ERR_IDC_MED_MULTICARD)      // 检测到多张卡
        {
           m_bMultiCard = TRUE;
           m_cStaExtra.AddExtra("ICErrorDetail", "0000");
           Log(ThisModule, __LINE__, "读卡: ->MediaReadWrite(%d) Fail, ErrCode: %d, Return: %d",
               MEDIA_READ, nRet, WFS_ERR_IDC_INVALIDMEDIA);
           FireCardInvalidMedia();
           return WFS_ERR_IDC_INVALIDMEDIA;
        } else
        {
            Log(ThisModule, __LINE__, "读卡: ->MediaReadWrite(%d) Fail, ErrCode: %d, Return: %d",
                MEDIA_READ, nRet, ConvertDevErrCode2WFS(nRet));
            return ConvertDevErrCode2WFS(nRet);
        }
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
    if ((dwReadOption & WFS_IDC_FRONTIMAGE) == WFS_IDC_FRONTIMAGE)
    {
        if (stMediaRead.stData[4].wResult == RW_RESULT_SUCC)
        {
            SetTrackInfo(WFS_IDC_FRONTIMAGE, WFS_IDC_DATAOK,
                         stMediaRead.stData[4].wSize, (BYTE *)stMediaRead.stData[4].szData);
        } else
        {
            SetTrackInfo(WFS_IDC_FRONTIMAGE, ConvertMediaRWResult2WFS(stMediaRead.stData[4].wResult), 0, nullptr);
        }
    }
    if ((dwReadOption & WFS_IDC_BACKIMAGE) == WFS_IDC_BACKIMAGE)
    {
        if (stMediaRead.stData[5].wResult == RW_RESULT_SUCC)
        {
            SetTrackInfo(WFS_IDC_BACKIMAGE, WFS_IDC_DATAOK,
                         stMediaRead.stData[5].wSize, (BYTE *)stMediaRead.stData[5].szData);
        } else
        {
            SetTrackInfo(WFS_IDC_BACKIMAGE, ConvertMediaRWResult2WFS(stMediaRead.stData[5].wResult), 0, nullptr);
        }
    }

    return WFS_SUCCESS;
}

// 指示灯控制
void CXFS_FIDC::ControlLight(WORD wAction, WORD wType)
{
    if (m_stConfig.stConfig_Light.wSupp == 1)
    {
        STGLIGHTSCONT stLights;
        stLights.wDelay = m_stConfig.stConfig_Light.wSlowDelayTime;
        stLights.enLigType = (GUID_LIGHTS_TYPE)wType;
        stLights.enLigAct = (GUID_LIGHTS_ACTION)wAction;
        stLights.wContMode = m_stConfig.stConfig_Light.wCont;
        m_pDev->SetData(SET_GLIGHT_CONTROL, &stLights);
    }
}

// 鸣响控制
void CXFS_FIDC::ControlBeep()
{
    if (m_stConfig.stConfig_Beep.wSupp == 1)
    {
        STBEEPCONT stBeep;
        stBeep.wBeepMsec = m_stConfig.stConfig_Beep.wMesc;
        stBeep.wBeepCount = m_stConfig.stConfig_Beep.wCount;
        stBeep.wBeepInterval = m_stConfig.stConfig_Beep.wInterval;
        stBeep.wContMode = m_stConfig.stConfig_Beep.wCont;
        m_pDev->SetData(SET_BEEP_CONTROL, &stBeep);
    }
}

// 更新扩展数据
void CXFS_FIDC::UpdateExtra()
{
    CHAR szFWVersion[64] = { 0x00 };
    LONG lFWVerSize = 0;

    // 组织状态扩展数据
    m_cStaExtra.Clear();
    m_cStaExtra.AddExtra("VRTCount", "2");
    m_cStaExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cStaExtra.AddExtra("VRTDetail[01]", (char*)byDevVRTU);

    // 取固件版本写入扩展数据
    m_pDev->GetVersion(GET_VER_FW, szFWVersion, 64);
    if (strlen(szFWVersion) > 0)
    {
        m_cStaExtra.AddExtra("VRTCount", "3");
        m_cStaExtra.AddExtra("VRTDetail[02]_FW", szFWVersion);
    }

    m_cCapExtra.Clear();
    m_cCapExtra.CopyFrom(m_cStaExtra);
}

void CXFS_FIDC::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
{
    WFSIDCCARDDATA data;
    data.wDataSource    = wSource;
    data.wStatus        = wStatus;
    data.ulDataLength   = uLen;
    data.lpbData        = pData;
    data.fwWriteMethod  = 0;

    m_cCardData.SetAt(wSource, data);
    return;
}

bool CXFS_FIDC::GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho)
{
    LPWFSIDCCARDDATA pCardData = m_cCardData.GetAt(wSource);
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
