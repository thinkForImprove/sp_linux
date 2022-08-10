/***************************************************************
* 文件名称：XFS_HCAM_DEC.cpp
* 文件描述：摄像头模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_HCAM.h"

#include "file_access.h"
#include "data_convertor.h"


//-------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_HCAM::InnerOpen(BOOL bReConn/* = FALSE*/)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    INT nRet = CAM_SUCCESS;

    // Open前下传初始参数(非断线重连)
    if (bReConn == FALSE)
    {
        // 设置SDK路径
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pCAMDev->SetData(SET_LIB_PATH, m_stConfig.szSDKPath);
        }

        // 设置初始化参数
        m_pCAMDev->SetData(SET_DEV_INIT, &(m_stConfig.stInitPar));

        // 设置设备打开模式
        m_pCAMDev->SetData(SET_DEV_OPENMODE, &(m_stConfig.stDevOpenMode));

        // 设置设备摄像模式
        m_pCAMDev->SetData(SET_DEV_VIDEOMODE, &(m_stConfig.stVideoPar));
    }

    // 打开CAM设备
    nRet = m_pCAMDev->Open(DEVTYPE2STR(m_stConfig.wDeviceType));
    if (nRet != CAM_SUCCESS)
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

// 加载DevHCAM动态库
BOOL CXFS_HCAM::LoadDevHCAMDll(LPCSTR ThisModule)
{
    if (m_pCAMDev == nullptr)
    {
        if (m_pCAMDev.Load(m_stConfig.szDevDllName, "CreateIDevCAM",
                           DEVTYPE2STR(m_stConfig.wDeviceType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DriverType=%d|%s, ERR:%s",
                m_stConfig.szDevDllName, m_stConfig.wDeviceType,
                DEVTYPE2STR(m_stConfig.wDeviceType), m_pCAMDev.LastError().toUtf8().constData());
            SetErrorDetail((LPSTR)EC_CAM_XFS_libCAMLoadFail);
            return false;
        } else
        {
            Log(ThisModule, __LINE__, "IDC: 加载库: DriverDllName=%s, DriverType=%d|%s, Succ.",
                m_stConfig.szDevDllName, m_stConfig.wDeviceType, DEVTYPE2STR(m_stConfig.wDeviceType));
        }
    }
    return (m_pCAMDev != nullptr);
}

// 读INI设置
INT CXFS_HCAM::InitConfig()
{
    THISMODULE(__FUNCTION__);

    CHAR    szIniAppName[MAX_PATH];
    CHAR    szBuffer[MAX_PATH];
    INT     nTmp;

    // DevHCAM动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("HCAMDriverDllName", ""));

    // ------------------------[CONFIG]下参数读取------------------------
    // 设备未连接时状态显示(0:NODEVICE, 1:OFFLINE, 缺省0)
    m_stConfig.wDevNotFoundStat = m_cXfsReg.GetValue("CONFIG", "DevNotFoundStat", (DWORD)0);

    // 截取画面帧的分辨率(Width),缺省0
    m_stConfig.wFrameResoWidth = m_cXfsReg.GetValue("CONFIG", "FrameResoWidth", (DWORD)0);

    // 截取画面帧的分辨率(Height),缺省0
    m_stConfig.wFrameResoHeight = m_cXfsReg.GetValue("CONFIG", "FrameResoHeight", (DWORD)0);

    // 镜像转换, 0:不转换, 1:左右转换, 2:上下转换, 3:上下左右转换, 缺省0
    m_stConfig.wDisplayFlip = m_cXfsReg.GetValue("CONFIG", "DisplayFlip", (DWORD)0);
    if (m_stConfig.wDisplayFlip < 0 || m_stConfig.wDisplayFlip > 3)
    {
        m_stConfig.wDisplayFlip = 0;
    }
    m_stConfig.stVideoPar.nOtherParam[0] = m_stConfig.wDisplayFlip;


    // ------------------------[DEVICE_CONFIG]下参数读取------------------------
    // 设备类型
    m_stConfig.wDeviceType = m_cXfsReg.GetValue("DEVICE_CONFIG", "DriverType", (DWORD)XFS_JDY5001A0809);

    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.wDeviceType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));


    //-------------------------------健德源单目设备参数获取-----------------------------------
    if (m_stConfig.wDeviceType == XFS_JDY5001A0809)      // JDY-5001A-0809(健德源单目)
    {
        STDEVICEOPENMODE stDevOpen;

        stDevOpen.nOtherParam[0] = XFS_JDY5001A0809;  // 设备类型

        // 打开方式(0:序列方式打开, 1:VidPid打开, 缺省0)
        stDevOpen.wOpenMode = (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenMode", (DWORD)0);
        // 设备序列,缺省
        strcpy(stDevOpen.szDevPath[0], m_cXfsReg.GetValue(szIniAppName, "Idx", "/dev/video0"));
        // VID/PID
        strcpy(stDevOpen.szHidVid[0], m_cXfsReg.GetValue(szIniAppName, "Vid", ""));
        strcpy(stDevOpen.szHidPid[0], m_cXfsReg.GetValue(szIniAppName, "Pid", ""));

        memcpy(&m_stConfig.stDevOpenMode, &stDevOpen, sizeof(STDEVICEOPENMODE));

        // 摄像刷新时间(毫秒,缺省30)
        m_stConfig.unWinRefreshTime = m_cXfsReg.GetValue(szIniAppName, "WinRefreshTime", (DWORD)30);
        if (m_stConfig.unWinRefreshTime < 0)
        {
            m_stConfig.unWinRefreshTime = 30;
        }

        m_stConfig.stInitPar.nParInt[0] = m_stConfig.unWinRefreshTime;
    }

    // ------------------------摄像显示参数设置参数读取------------------------
    // 亮度(-255 ~ 255)(RGB相关)
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, "Video_Bright", "99999999"));
    m_stConfig.stVideoPar.duBright = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                             DataConvertor::str2number(szBuffer);
    // 对比度
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, "Video_Contrast", "99999999"));
    m_stConfig.stVideoPar.duContrast = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                               DataConvertor::str2number(szBuffer);
    // 饱和度    
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, "Video_Saturation", "99999999"));
    m_stConfig.stVideoPar.duSaturation = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                               DataConvertor::str2number(szBuffer);
    // 色调
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, "Video_Hue", "99999999"));
    m_stConfig.stVideoPar.duHue = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                          DataConvertor::str2number(szBuffer);
    // 曝光
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, "Video_Exposure", "99999999"));
    m_stConfig.stVideoPar.duExposure = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                               DataConvertor::str2number(szBuffer);


    // ------------------------[CAMERA_SHAREMEMORY]下参数读取------------------------
    // 摄像数据共享内存名(32位,缺省CamShowSharedMemData)
    strcpy(m_stConfig.szCamDataSMemName,
           m_cXfsReg.GetValue("CAMERA_SHAREMEMORY", "ShowSharedMemName", SHAREDMEMNAME_CAMDATA));
    //摄像数据共享内存Size(缺省10M)
    //m_stConfig.ulCamDataSMemSize =
    //        (ULONG)m_cXfsReg.GetValue("CAMERA_SHAREMEMORY", "ShowSharedMemSize", (ULONG)SHAREDMEMSIZE_CAMDATA);
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("CAMERA_SHAREMEMORY", "ShowSharedMemSize", "10485760"));
    m_stConfig.ulCamDataSMemSize = strlen(szBuffer) == 0 ? (ULONG)SHAREDMEMSIZE_CAMDATA :
                                                               DataConvertor::str2number(szBuffer);

    MCPY_NOLEN(m_stConfig.stInitPar.szParStr[0], m_stConfig.szCamDataSMemName);
    m_stConfig.stInitPar.lParLong[0] = m_stConfig.ulCamDataSMemSize;

    //--------------------------[TAKEPIC_CONFIG]命令相关设置参数获取--------------------------
    // TakePictrue/TakePictrueEx超时(单位:毫秒,缺省0)
    m_stConfig.dwTakePicTimeOut = m_cXfsReg.GetValue("TAKEPIC_CONFIG", "TakePicTimeOut", (DWORD)0);
    if (m_stConfig.dwTakePicTimeOut < 0)
    {
        m_stConfig.dwTakePicTimeOut = 0;
    }

    // TakePicture命令缺省保存目录
    strcpy(m_stConfig.szTakePicDefSavePath,
           m_cXfsReg.GetValue("TAKEPIC_CONFIG", "TakePicDefSavePath", ""));
    if (strlen((char*)m_stConfig.szTakePicDefSavePath) < 5 || m_stConfig.szTakePicDefSavePath[0] != '/')
    {
        MSET_0(m_stConfig.szTakePicDefSavePath);
    }


    //--------------------------Open命令相关设置参数获取--------------------------
    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.wOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.wOpenFailRet != 0 && m_stConfig.wOpenFailRet != 1)
    {
        m_stConfig.wOpenFailRet = 0;
    }


    //--------------------------Reset命令相关设置参数获取--------------------------
    // Reset执行时是否支持关闭摄像窗口(0:不支持,1支持,缺省0)
    m_stConfig.wResetCloseDiaplay = m_cXfsReg.GetValue("RESET_CONFIG", "ResetCloseDiaplay", (DWORD)0);
    if (m_stConfig.wResetCloseDiaplay < 0 || m_stConfig.wResetCloseDiaplay > 1)
    {
        m_stConfig.wResetCloseDiaplay = 0;
    }

    //--------------------------指定银行相关设置参数获取--------------------------
    // 指定银行，缺省0
    m_stConfig.wBank = (WORD)m_cXfsReg.GetValue("BANK", "BankNo", (DWORD)BANK_NOALL);
    m_stConfig.stInitPar.nParInt[1] = m_stConfig.wBank;

    //--------------------------指定错误码相关设置参数获取--------------------------
    // 是否支持Status显示错误码(0:不支持, 1:支持显示当前错误码, 2:支持显示当前和上一次错误码, 缺省2)
    m_stConfig.wErrDetailShowSup = m_cXfsReg.GetValue("ERRDETAIL_CONFIG", "ErrDetailShowSup", (DWORD)2);
    if (m_stConfig.wErrDetailShowSup < 0 || m_stConfig.wErrDetailShowSup > 2)
    {
        m_stConfig.wErrDetailShowSup = 2;
    }

    // 当前错误码Key名, 最多31个字符, 缺省ErrorDetail
    MSET_0(szBuffer);
    MSET_0(m_stConfig.szErrDetailKeyName);
    strcpy(szBuffer, m_cXfsReg.GetValue("ERRDETAIL_CONFIG", "ErrDetailKeyName", "ErrorDetail"));
    if (strlen(szBuffer) == 0)
    {
        sprintf(m_stConfig.szErrDetailKeyName, "ErrorDetail");
    } else
    {
        MCPY_NOLEN(m_stConfig.szErrDetailKeyName, szBuffer);
    }

    // 上一次错误码Key名, 最多31个字符, 缺省LastErrorDetail
    MSET_0(szBuffer);
    MSET_0(m_stConfig.szLastErrDetailKeyName);
    strcpy(szBuffer, m_cXfsReg.GetValue("ERRDETAIL_CONFIG", "LastErrDetailKeyName", "LastErrorDetail"));
    if (strlen(szBuffer) == 0)
    {
        sprintf(m_stConfig.szLastErrDetailKeyName, "ErrorDetail");
    } else
    {
        MCPY_NOLEN(m_stConfig.szLastErrDetailKeyName, szBuffer);
    }

    PrintIniData();  // INI配置输出

    return WFS_SUCCESS;
}

// INI配置输出
INT CXFS_HCAM::PrintIniData()
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

    PRINT_INI_BUF("\n\t\t\t\t DevHCAM动态库名: default->HCAMDriverDllName = %s", m_stConfig.szDevDllName);
    PRINT_INI_BUF("\n\t\t\t\t 设备未连接时状态显示(0:NODEVICE,1:OFFLINE): CONFIG->DevNotFoundStat = %d", m_stConfig.wDevNotFoundStat);
    PRINT_INI_BUF("\n\t\t\t\t 截取画面帧的分辨率(Width): CONFIG->FrameResoWidth = %d", m_stConfig.wFrameResoWidth);
    PRINT_INI_BUF("\n\t\t\t\t 截取画面帧的分辨率(Height): CONFIG->FrameResoHeight = %d", m_stConfig.wFrameResoHeight);
    PRINT_INI_BUF("\n\t\t\t\t 镜像转换(0:不转换,1:左右转换,2:上下转换,3:上下左右转换): CONFIG->DisplayFlip = %d", m_stConfig.wDisplayFlip);
    PRINT_INI_BUF("\n\t\t\t\t 设备类型: CONFIG->DriverType = %d", m_stConfig.wDeviceType);
    PRINT_INI_BUF2("\n\t\t\t\t 设备SDK库路径: DEVICE_SET_%d->SDK_Path = %s", m_stConfig.wDeviceType, m_stConfig.szSDKPath);
    PRINT_INI_BUF2("\n\t\t\t\t 打开方式(0:序列方式打开,1:VidPid打开): DEVICE_SET_%d->OpenMode = %d",
                   m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.wOpenMode);
    PRINT_INI_BUF2("\n\t\t\t\t 设备序列: DEVICE_SET_%d->Idx = %d", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szDevPath[0]);
    PRINT_INI_BUF2("\n\t\t\t\t VID: DEVICE_SET_%d->Vid = %s", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szHidVid[0]);
    PRINT_INI_BUF2("\n\t\t\t\t PID: DEVICE_SET_%d->Pid = %s", m_stConfig.wDeviceType, m_stConfig.stDevOpenMode.szHidPid[0]);
    PRINT_INI_BUF2("\n\t\t\t\t 摄像刷新时间(毫秒): DEVICE_SET_%d-> = %.6lf", m_stConfig.wDeviceType, m_stConfig.unWinRefreshTime);
    PRINT_INI_BUF2("\n\t\t\t\t 亮度: DEVICE_SET_%d->Video_Bright = %.6lf", m_stConfig.wDeviceType, m_stConfig.stVideoPar.duBright);
    PRINT_INI_BUF2("\n\t\t\t\t 对比度: DEVICE_SET_%d->Video_Contrast = %.6lf", m_stConfig.wDeviceType, m_stConfig.stVideoPar.duContrast);
    PRINT_INI_BUF2("\n\t\t\t\t 饱和度: DEVICE_SET_%d->Video_Saturation = %.6lf", m_stConfig.wDeviceType, m_stConfig.stVideoPar.duSaturation);
    PRINT_INI_BUF2("\n\t\t\t\t 色调: DEVICE_SET_%d->Video_Hue = %.6lf", m_stConfig.wDeviceType, m_stConfig.stVideoPar.duHue);
    PRINT_INI_BUF2("\n\t\t\t\t 曝光: DEVICE_SET_%d->Video_Exposure = %.6lf", m_stConfig.wDeviceType, m_stConfig.stVideoPar.duExposure);
    PRINT_INI_BUF("\n\t\t\t\t 摄像数据共享内存名(<32位): CAMERA_SHAREMEMORY->ShowSharedMemName = %s", m_stConfig.szCamDataSMemName);
    PRINT_INI_BUF("\n\t\t\t\t 摄像数据共享内存Size: CAMERA_SHAREMEMORY->ShowSharedMemSize = %ld", m_stConfig.ulCamDataSMemSize);
    PRINT_INI_BUF("\n\t\t\t\t TakePictrue/TakePictrueEx超时(毫秒): TakePic->TakePicTimeOut = %d", m_stConfig.dwTakePicTimeOut);
    PRINT_INI_BUF("\n\t\t\t\t TakePicture命令缺省保存目录: TAKEPIC_CONFIG->TakePicDefSavePath = %s", m_stConfig.szTakePicDefSavePath);
    PRINT_INI_BUF("\n\t\t\t\t Open失败时返回值(0原样返回/1返回SUCCESS): OPEN_CONFIG->OpenFailRet = %d", m_stConfig.wOpenFailRet);
    PRINT_INI_BUF("\n\t\t\t\t Reset执行时是否支持关闭摄像窗口(0:不支持,1支持): RESET_CONFIG->ResetCloseDiaplay = %d", m_stConfig.wResetCloseDiaplay);
    PRINT_INI_BUF("\n\t\t\t\t 指定银行: BANK->BankNo = %d", m_stConfig.wBank);
    PRINT_INI_BUF("\n\t\t\t\t 是否支持Status显示错误码: ERRDETAIL_CONFIG->ErrDetailShowSup = %d", m_stConfig.wErrDetailShowSup);
    PRINT_INI_BUF("\n\t\t\t\t 当前错误码Key名: ERRDETAIL_CONFIG->ErrDetailKeyName = %s", m_stConfig.szErrDetailKeyName);
    PRINT_INI_BUF("\n\t\t\t\t 上一次错误码Key名: ERRDETAIL_CONFIG->LastErrDetailKeyName = %s", m_stConfig.szLastErrDetailKeyName);

    Log(ThisModule, __LINE__, "INI配置取得如下: %s", qsIniPrt.toStdString().c_str());

    return WFS_SUCCESS;
}

// 状态结构体实例初始化
void CXFS_HCAM::InitStatus()
{
    m_stStatus.Clear();
    m_stStatus.fwDevice      = WFS_CAM_DEVNODEVICE;
}

// 能力值结构体实例初始化
void CXFS_HCAM::InitCaps()
{
    m_stCaps.wClass = WFS_SERVICE_CLASS_CAM;
    m_stCaps.lpszExtra = nullptr;
}

// 更新扩展数据
void CXFS_HCAM::UpdateExtra()
{
    CHAR szFWVersion[64] = { 0x00 };

    // 组织状态扩展数据
    m_cStatExtra.Clear();
    m_cStatExtra.AddExtra("VRTCount", "2");
    m_cStatExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cStatExtra.AddExtra("VRTDetail[01]", (char*)byDevVRTU);

    // 取固件版本写入扩展数据
    m_pCAMDev->GetVersion(GET_VER_FW, szFWVersion, 64);
    if (strlen(szFWVersion) > 0)
    {
        m_cStatExtra.AddExtra("VRTCount", "3");
        m_cStatExtra.AddExtra("VRTDetail[02]", szFWVersion);
    }

    m_cCapsExtra.Clear();
    m_cCapsExtra.CopyFrom(m_cStatExtra);
}

// 设备状态实时更新
WORD CXFS_HCAM::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);      // 必须加此互斥，防止同时读写数据问题

    INT     nRet = CAM_SUCCESS;
    WORD    fwDevice = WFS_CAM_DEVHWERROR;
    DWORD   dwHWAct = WFS_ERR_ACT_NOACTION;
    CHAR    szHWDesc[1024] = { 0x00 };

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFireHWError        = FALSE;

    // 取设备状态
    STDEVCAMSTATUS stDevStatus;
    nRet = m_pCAMDev->GetStatus(stDevStatus);

    //----------------------设备状态处理----------------------
    // 返回值处理
    switch (nRet)
    {
        ;
    }

    //----------------------Device状态处理----------------------
    m_stStatus.fwDevice = ConvertDeviceStatus2WFS(stDevStatus.wDevice);
    if (m_stConfig.wDevNotFoundStat != 0 &&
        m_stStatus.fwDevice == WFS_CAM_DEVNODEVICE)
    {
        m_stStatus.fwDevice = WFS_CAM_DEVOFFLINE;
    }
    if (m_stStatus.fwDevice != WFS_CAM_DEVONLINE &&
        m_stStatus.fwDevice != WFS_CAM_DEVBUSY)
    {
        if (m_bDisplayOK == TRUE)   // 断线时窗口有显示:执行关闭
        {
            if (m_wWindowsRunMode == WIN_RUN_SHARED)
            {
                emit vSignHideWin();
            }
            m_bDisplayOK = FALSE;
        }

        for (INT i = 0; i < CAM_CAMERAS_SIZE; i ++)
        {
            m_stStatus.fwMedia[i] = WFS_CAM_MEDIAUNKNOWN;
        }
    }

    // Device == ONLINE && 有命令在执行中,设置Device = BUSY
    if (m_stStatus.fwDevice == WFS_CAM_DEVONLINE && m_bCmdRunning == TRUE)    // 命令执行中
    {
        m_stStatus.fwDevice = WFS_CAM_DEVBUSY;
    }

    //----------------------Media状态处理----------------------
    for (INT i = 0; i < CAM_CAMERAS_SIZE; i ++)
    {
        m_stStatus.fwMedia[i] = ConvertMediaStatus2WFS(stDevStatus.wMedia[i]);
        /*(if (m_stStatus.fwMedia[i] == WFS_CAM_MEDIAUNKNOWN)
        {
            m_stStatus.fwDevice = WFS_CAM_DEVHWERROR;
        }*/
    }


    //----------------------Cameras状态处理----------------------
    for (INT i = 0; i < CAM_CAMERAS_SIZE; i ++)
    {
        m_stStatus.fwCameras[i] = ConvertCamerasStatus2WFS(stDevStatus.fwCameras[i]);
        /*if (m_stStatus.fwCameras[i] == WFS_CAM_CAMUNKNOWN)
        {
            m_stStatus.fwDevice = WFS_CAM_DEVHWERROR;
        }*/
    }

    //----------------------状态检查----------------------
    // Device状态变化检查
    if (m_stStatus.fwDevice != m_stStatusOLD.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        if (m_stStatus.fwDevice == WFS_CAM_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
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

    // 其他检查上报
    if (stDevStatus.nOtherCode[0] == 1) // 上报Display命令及关联错误
    {
        dwHWAct = ConvertEventAct2WFS(stDevStatus.nOtherCode[2]);
        FireHWEvent(dwHWAct, szHWDesc);
        sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%s|", dwHWAct, szHWDesc);

        // 关闭窗口
        if (m_wWindowsRunMode == WIN_RUN_SHARED)
        {
            emit vSignHideWin();
        }
        m_bDisplayOK = FALSE;
    }

    // 比较两次状态记录LOG
    if (m_stStatus.Diff(m_stStatusOLD) == true)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|"
                                  "Media[ROOM]:%d->%d%s|Media[PERSON]:%d->%d%s|Media[EXITSLO]:%d->%d%s|"
                                  "Cameras[ROOM]:%d->%d%s|Cameras[PERSON]:%d->%d%s|Cameras[EXITSLO]:%d->%d%s|; "
                                  "事件上报记录: %s.",
            m_stStatusOLD.fwDevice, m_stStatus.fwDevice, (m_stStatusOLD.fwDevice != m_stStatus.fwDevice ? " *" : ""),
            m_stStatusOLD.fwMedia[WFS_CAM_ROOM], m_stStatus.fwMedia[WFS_CAM_ROOM],
             (m_stStatusOLD.fwMedia[WFS_CAM_ROOM] != m_stStatus.fwMedia[WFS_CAM_ROOM] ? " *" : ""),
            m_stStatusOLD.fwMedia[WFS_CAM_PERSON], m_stStatus.fwMedia[WFS_CAM_PERSON],
             (m_stStatusOLD.fwMedia[WFS_CAM_PERSON] != m_stStatus.fwMedia[WFS_CAM_PERSON] ? " *" : ""),
            m_stStatusOLD.fwMedia[WFS_CAM_EXITSLOT], m_stStatus.fwMedia[WFS_CAM_EXITSLOT],
             (m_stStatusOLD.fwMedia[WFS_CAM_EXITSLOT] != m_stStatus.fwMedia[WFS_CAM_EXITSLOT] ? " *" : ""),
            m_stStatusOLD.fwCameras[WFS_CAM_ROOM], m_stStatus.fwCameras[WFS_CAM_ROOM],
             (m_stStatusOLD.fwCameras[WFS_CAM_ROOM] != m_stStatus.fwCameras[WFS_CAM_ROOM] ? " *" : ""),
            m_stStatusOLD.fwCameras[WFS_CAM_PERSON], m_stStatus.fwCameras[WFS_CAM_PERSON],
             (m_stStatusOLD.fwCameras[WFS_CAM_PERSON] != m_stStatus.fwCameras[WFS_CAM_PERSON] ? " *" : ""),
            m_stStatusOLD.fwCameras[WFS_CAM_EXITSLOT], m_stStatus.fwCameras[WFS_CAM_EXITSLOT],
             (m_stStatusOLD.fwCameras[WFS_CAM_EXITSLOT] != m_stStatus.fwCameras[WFS_CAM_EXITSLOT] ? " *" : ""),
            szFireBuffer);
        m_stStatusOLD.Copy(m_stStatus);
    }

    return 0;
}

// 摄像处理
HRESULT CXFS_HCAM::InnerTakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WFSCAMTAKEPICTEX stCamTakePictEx;

    stCamTakePictEx.wCamera = stTakePict.wCamera;
    //MCPY_NOLEN(stCamTakePictEx.lpszCamData, stTakePict.lpszCamData);
    //MCPY_NOLEN(stCamTakePictEx.lpszUNICODECamData, stTakePict.lpszUNICODECamData);
    stCamTakePictEx.lpszCamData = stTakePict.lpszCamData;
    stCamTakePictEx.lpszUNICODECamData = stTakePict.lpszUNICODECamData;

    if (strlen(m_stConfig.szTakePicDefSavePath) > 5)
    {
        //MCPY_NOLEN(stCamTakePictEx.lpszPictureFile, m_stConfig.szTakePicDefSavePath);
        stCamTakePictEx.lpszPictureFile = strdup(m_stConfig.szTakePicDefSavePath);
    } else
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败 :INI->TakePicDefSavePath[%s]设置图像保存文件无效, Return: %d.",
            m_stConfig.szTakePicDefSavePath, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    }

    HRESULT hRet = InnerTakePictureEx(stCamTakePictEx, dwTimeout);

    stCamTakePictEx.lpszCamData = nullptr;
    stCamTakePictEx.lpszUNICODECamData = nullptr;
    free(stCamTakePictEx.lpszPictureFile);

    return hRet;
}

// 拍照处理扩展
HRESULT CXFS_HCAM::InnerTakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CAM_SUCCESS;

    if (m_bDisplayOK != TRUE)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 命令序列错误, Display未执行, Return: %d", WFS_ERR_DEV_NOT_READY);
        SetErrorDetail((LPSTR)EC_CAM_XFS_NotDisplay);
        return WFS_ERR_DEV_NOT_READY;
    }

    LPWFSCAMTAKEPICTEX	lpCmdData = NULL;
    lpCmdData = (LPWFSCAMTAKEPICTEX)&stTakePict;
    STTAKEPICTUREPAR stTakePicPar;

    // 图片文件名及路径检查
    if (lpCmdData->lpszPictureFile == NULL)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[NULL]无效, Return: %d", WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    }

    MSET_0(m_szFileName);
    if (strlen(lpCmdData->lpszPictureFile) > 5)
    {
        sprintf(m_szFileName, "%s", lpCmdData->lpszPictureFile);
    } else
    if (strlen(m_stConfig.szTakePicDefSavePath) > 0)
    {
        sprintf(m_szFileName, "%s", m_stConfig.szTakePicDefSavePath);
    } else
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[%s]无效, Return: %d.",
            lpCmdData->lpszPictureFile, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    }

    if (m_szFileName[0] != '/')
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[%s]无效, Return: %d.",
            lpCmdData->lpszPictureFile, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    }

    // 检查扩展名
    std::string stdFilePath = m_szFileName;
    int iIndex = stdFilePath.rfind('.');
    if (iIndex == -1)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[%s]无效, 未指定图片文件扩展名, Return: %d.",
            m_szFileName, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    } else
    {
        std::string stdTmp;
        stdTmp = stdFilePath.substr(0, iIndex);

        std::string szEx = stdFilePath.substr(iIndex, stdFilePath.length() - iIndex);
        std::transform(szEx.begin(), szEx.end(), szEx.begin(), ::toupper);
        if (szEx.compare(".BASE64") == 0)
        {
            stTakePicPar.wPicType = PIC_BASE64;
        } else
        if (szEx.compare(".JPG") == 0)
        {
            stTakePicPar.wPicType = PIC_JPG;
        } else
        {
            stTakePicPar.wPicType = PIC_BMP;
        }
    }

    // 目录验证
    if (FileAccess::create_directory_by_path(m_szFileName, true) != true)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[%s]创建目录结构失败, Return: %d.",
            m_szFileName, WFS_ERR_INTERNAL_ERROR);
        SetErrorDetail((LPSTR)EC_ERR_FilePathCrtErr);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 根据INI配置超时设置
    stTakePicPar.dwTimeOut =
            (m_stConfig.dwTakePicTimeOut == 0 ? dwTimeout : m_stConfig.dwTakePicTimeOut);

    // 组织命令下发入参
    stTakePicPar.wCameraAction = lpCmdData->wCamera;                // 摄像模式
    MCPY_NOLEN(stTakePicPar.szCamData, lpCmdData->lpszCamData);     // 水印
    MCPY_NOLEN(stTakePicPar.szFileName, m_szFileName);              // 保存图片文件名

    // 命令下发
    nRet = m_pCAMDev->TakePicture(stTakePicPar);
    if (nRet != CAM_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "拍照: ->TakePicture() fail, ErrCode: %s, Return: %d.",
            ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    // TakePicture命令完成后处理
    InnerTakePicAfterMothod();

    return WFS_SUCCESS;
}

// 窗口处理
HRESULT CXFS_HCAM::InnerDisplay(const WFSCAMDISP &stDisplay, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CAM_SUCCESS;

    LPWFSCAMDISP lpDisplay = NULL;
    lpDisplay = (LPWFSCAMDISP)&stDisplay;

    // 根据INI配置设置水平分辨率参数
    if (lpDisplay->wHpixel == 0 && m_stConfig.wFrameResoWidth > 0)
    {
        lpDisplay->wHpixel = m_stConfig.wFrameResoWidth;
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 入参 水平分辨率Hpixel=%d, INI设置截取画面分辨率>0, 设置水平分辨率Hpixel为%d.",
            lpDisplay->wHpixel, m_stConfig.wFrameResoWidth);
    }

    // 根据INI配置设置垂直分辨率参数
    if (lpDisplay->wVpixel == 0 && m_stConfig.wFrameResoHeight > 0)
    {
        lpDisplay->wVpixel = m_stConfig.wFrameResoHeight;
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 入参 垂直分辨率Vpixel=%d, INI设置截取画面分辨率>0, 设置垂直分辨率Vpixel为%d.",
            lpDisplay->wVpixel, m_stConfig.wFrameResoHeight);
    }

    if(lpDisplay->wAction != WFS_CAM_CREATE &&      // 创建摄像窗口
       lpDisplay->wAction != WFS_CAM_DESTROY &&     // 销毁摄像窗口
       lpDisplay->wAction != WFS_CAM_PAUSE &&       // 暂停摄像窗口
       lpDisplay->wAction != WFS_CAM_RESUME)
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 失败: 入参wAction[%d]无效．Return: %d",
            lpDisplay->wAction, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_ERR_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    // 创建窗口时宽高检查
    if(lpDisplay->wAction == WFS_CAM_CREATE &&
       (lpDisplay->wHeight == 0 || lpDisplay->wWidth == 0))
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 失败: 入参wHeight[%d]/wWidth[%d]无效．Return: %d",
            lpDisplay->wHeight, lpDisplay->wWidth, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_ERR_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    STDISPLAYPAR stDisplayPar;
    stDisplayPar.dwTimeOut = dwTimeout;
    stDisplayPar.hWnd = lpDisplay->hWnd;
    stDisplayPar.wCamera = lpDisplay->wCamera;
    stDisplayPar.wAction = lpDisplay->wAction;
    stDisplayPar.wX = lpDisplay->wX;
    stDisplayPar.wY = lpDisplay->wY;
    stDisplayPar.wWidth = lpDisplay->wWidth;
    stDisplayPar.wHeight = lpDisplay->wHeight;
    stDisplayPar.wHpixel = lpDisplay->wHpixel;
    stDisplayPar.wVpixel = lpDisplay->wVpixel;

    nRet = m_pCAMDev->Display(stDisplayPar);
    if (nRet != CAM_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: ->Display() fail, ErrCode: %s, Return: %d",
            ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    if (lpDisplay->wAction == WFS_CAM_CREATE)
    {
        if (m_wWindowsRunMode == WIN_RUN_SHARED)
        {
            STSHOWWININFO stCamInfo;
            stCamInfo.hWnd = lpDisplay->hWnd;
            stCamInfo.wX = lpDisplay->wX;
            stCamInfo.wY = lpDisplay->wY;
            stCamInfo.wWidth = lpDisplay->wWidth;
            stCamInfo.wHeight = lpDisplay->wHeight;
            emit vSignShowWin(stCamInfo);
        }
        m_bDisplayOK = TRUE;
    } else
    if (lpDisplay->wAction == WFS_CAM_DESTROY)
    {
        if (m_wWindowsRunMode == WIN_RUN_SHARED)
        {
            emit vSignHideWin();
        }

        m_bDisplayOK = FALSE;
    }

    return WFS_SUCCESS;
}

//--------------------------------窗口处理---------------------------------------
// 显示窗口(启动SP内建窗口及共享内存计时器)
void CXFS_HCAM::vSlotShowWin(STSHOWWININFO stCamShowInfo)
{
    int nRet = 0;

    if (showWin != nullptr)
    {
        showWin->hide();
        delete showWin;
        showWin = nullptr;
    }

    QWidget *myWidget = nullptr;
    myWidget = QWidget::createWindowContainer(QWindow::fromWinId((WId)stCamShowInfo.hWnd));

    showWin = new MainWindow(myWidget);

    if ((nRet = showWin->nConnSharedMem(m_stConfig.szCamDataSMemName)) > 0)
    {
        showWin->Release();
        return;
    }

    showWin->ShowWin(stCamShowInfo.wX, stCamShowInfo.wY,
                     stCamShowInfo.wWidth, stCamShowInfo.wHeight);

    showWin->vTimerStart(m_stConfig.unWinRefreshTime);

    //showWin->show();
}

// 结束显示窗口
void CXFS_HCAM::vSlotHideWin()
{
    if (showWin != nullptr)
    {
        showWin->hide();
        delete showWin;
        showWin = nullptr;
    }
}

//-----------------------------共享内存处理----------------------------------------
// 创建共享内存
BOOL CXFS_HCAM::bCreateSharedMemory(LPSTR lpMemName, ULONG ulSize)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 创建共享内存Data
    if (m_qSharedMemData == nullptr)
    {
        m_qSharedMemData = new QSharedMemory(lpMemName);   // 构造实例对象 + 为实例对象指定关键字(给共享内存命名)
        if (m_qSharedMemData == nullptr)
        {
            Log(ThisModule, __LINE__, "wCreateSharedMemory->创建SharedMemeory(%s)实例失败. ", lpMemName);
            return FALSE;
        }
    }

    if (m_qSharedMemData->create(ulSize, QSharedMemory::ReadWrite) != true)
    {
        if (m_qSharedMemData->error() == QSharedMemory::AlreadyExists)
        {
            Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s)共享内存已存在,不继续创建. ", lpMemName);
            m_qSharedMemData->attach();
            if (m_qSharedMemData->isAttached() != true)
            {
                Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存挂载失败, ReturnCode:%d|%s. ",
                    lpMemName, ulSize, m_qSharedMemData->error(), m_qSharedMemData->errorString().toUtf8().data());
                SetErrorDetail((LPSTR)EC_ERR_ShareMemCrt);
                return FALSE;
            }
        } else
        {
            Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存创建失败, ReturnCode:%d|%s. ",
                lpMemName, ulSize, m_qSharedMemData->error(), m_qSharedMemData->errorString().toUtf8().data());
            SetErrorDetail((LPSTR)EC_ERR_ShareMemCrt);
            return FALSE;
        }
    }

    Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存创建完成. ",  lpMemName, ulSize);

    return TRUE;
}

// 销毁共享内存
void CXFS_HCAM::SharedMemRelease()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_qSharedMemData != nullptr)
    {
        if (m_qSharedMemData->isAttached())
        {
            m_qSharedMemData->detach();
            delete m_qSharedMemData;
            m_qSharedMemData = nullptr;
        }
    }

    return;
}

//-----------------------------其他处理----------------------------------------
// TakePicture命令完成后处理
INT CXFS_HCAM::InnerTakePicAfterMothod()
{
    THISMODULE(__FUNCTION__);

    return WFS_SUCCESS;
}

// 设置ErrorDetail错误码
INT CXFS_HCAM::SetErrorDetail(LPSTR lpCode)
{
    if (lpCode == nullptr)
    {
        CHAR szErrCode[1024] = { 0x00 };
        m_pCAMDev->GetData(GET_DEV_ERRCODE, szErrCode);
        m_clErrorDet.SetErrCode(szErrCode);
    } else
    {
        m_clErrorDet.SetXFSErrCode(lpCode);
    }

    return WFS_SUCCESS;
}

