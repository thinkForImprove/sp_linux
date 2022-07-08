/***************************************************************
* 文件名称：XFS_CAM.cpp
* 文件描述：摄像头模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_HCAM.h"


static const char *DEVTYPE = "HCAM";
static const char *ThisFile = "XFS_HCAM.cpp";
#define LOG_NAME    "XFS_HCAM.log"

//////////////////////////////////////////////////////////////////////////
CXFS_HCAM::CXFS_HCAM(): m_pMutexGetStatus(nullptr), m_clErrorDet((LPSTR)DEVTYPE)
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);

    MSET_S(&m_stConfig, sizeof(STINICONFIG));           // INI结构体
    MSET_S(&m_stStatus, sizeof(CWfsCAMStatus));         // 状态结构体
    MSET_S(&m_stStatusOLD, sizeof(CWfsCAMStatus));      // 备份上一次状态结构体
    MSET_S(&m_stCaps, sizeof(CWfsCAMCap));              // 能力值结构体
    MSET_XS(m_nRetErrOLD, 0, sizeof(INT) * 12);         // 处理错误值保存(0:HCAM断线重连)
    MSET_0(m_szFileName);                               // 拍照文件名
    showWin = nullptr;                                  // 摄像窗口
    m_qSharedMemData = nullptr;                         // 摄像数据共享内存
    m_bCmdRunning = FALSE;                              // 是否命令执行中
    m_bDisplayOK = FALSE;                               // Display是否已执行
    m_wWindowsRunMode = WIN_RUN_SHARED;                 // 窗口运行方式
}

CXFS_HCAM::~CXFS_HCAM()
{
    OnClose();
}

long CXFS_HCAM::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();    

    // 加载BaseCAM
    if (0 != m_pBase.Load("SPBaseCAM.dll", "CreateISPBaseCAM", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseCAM失败");        
        SetErrorDetail((LPSTR)EC_XFS_SPBaseLoadFail);
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_HCAM::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    InitConfig();       // 读INI
    InitCaps();         // 能力值初始化
    InitStatus();       // 状态值初始化

    // 检查INI设置设备类型
    if (m_stConfig.wDeviceType != XFS_JDY5001A0809)
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定了不支持的设备类型[%d], Return :%d.",
            m_stConfig.wDeviceType, WFS_ERR_DEV_NOT_READY);
        SetErrorDetail((LPSTR)EC_XFS_DevNotSupp);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 指定窗口运行方式(1:共享内存)
    if (m_stConfig.wDeviceType == XFS_JDY5001A0809)
    {
        m_wWindowsRunMode = WIN_RUN_SHARED;
    }

    // 创建共享内存(摄像)
    if (m_wWindowsRunMode == WIN_RUN_SHARED)
    {
        if (bCreateSharedMemory(m_stConfig.szCamDataSMemName,
                                m_stConfig.ulCamDataSMemSize) != TRUE)
        {
            Log(ThisModule, __LINE__, "摄像共享内存[%s|%ld]创建失败, Return: %d",
                m_stConfig.szCamDataSMemName, m_stConfig.ulCamDataSMemSize,
                WFS_ERR_INTERNAL_ERROR);
            SetErrorDetail((LPSTR)EC_XFS_ShareMemCrt);
            return WFS_ERR_INTERNAL_ERROR;
        }

        qRegisterMetaType<STSHOWWININFO>("STSHOWWININFO");
        connect(this, SIGNAL(vSignShowWin(STSHOWWININFO)), this, SLOT(vSlotShowWin(STSHOWWININFO)), Qt::AutoConnection);
        connect(this, SIGNAL(vSignHideWin()), this, SLOT(vSlotHideWin()), Qt::AutoConnection);
    }

    // 设备驱动动态库验证: 检查DevXXX库是否存在
    if (strlen(m_stConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败", m_strSPName.c_str());
        SetErrorDetail((LPSTR)EC_CAM_XFS_libCAMNotFound);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 加载DevXXX动态库
    if (LoadDevHCAMDll(ThisModule) != TRUE)
    {
        return WFS_ERR_INTERNAL_ERROR;
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = InnerOpen();
    if (m_stConfig.wOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

HRESULT CXFS_HCAM::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pCAMDev != nullptr)
    {
        m_pCAMDev->Close();
    }

    // 共享内存方式窗口,要注销释放
    if (m_wWindowsRunMode == WIN_RUN_SHARED)
    {
        SharedMemRelease();
        emit vSignHideWin();
    }

    if (showWin != nullptr)
    {
        delete showWin;
        showWin = nullptr;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_HCAM::OnStatus()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;
    UpdateDeviceStatus();
    if(m_stStatus.fwDevice == WFS_CAM_DEVOFFLINE ||
       m_stStatus.fwDevice == WFS_CAM_DEVPOWEROFF ||
       m_stStatus.fwDevice == WFS_CAM_DEVNODEVICE)
    {
        // 断线自动重连
        m_pCAMDev->SetData(SET_DEV_RECON, nullptr); // 设置为断线重连执行状态
        hRet = InnerOpen(TRUE);                     // 执行断线重连
        if (hRet != WFS_SUCCESS)
        {
            if (m_nRetErrOLD[0] != hRet)
            {
                Log(ThisModule, __LINE__,
                    "断线重连中: OnStatus->InnerOpen(%d) fail, ErrCode: %d.", TRUE, hRet);
                m_nRetErrOLD[0] = hRet;
            }
        } else
        {
            if (m_nRetErrOLD[0] != hRet)
            {
                Log(ThisModule, __LINE__,
                    "断线重连完成: OnStatus->InnerOpen(%d) succ, RetCode:%d.", TRUE, hRet);
                m_nRetErrOLD[0] = hRet;
            }
        }
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_HCAM::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pCAMDev != nullptr)
    {
        m_pCAMDev->Cancel();
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_HCAM::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

//----------------------------INFO接口----------------------------
// 查询状态
HRESULT CXFS_HCAM::GetStatus(LPWFSCAMSTATUS &lpstStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 错误码
    if (m_stConfig.wErrDetailShowSup == 1 || m_stConfig.wErrDetailShowSup == 2)
    {
        m_cStatExtra.AddExtra(m_stConfig.szErrDetailKeyName, m_clErrorDet.GetSPErrCode());
    }
    if (m_stConfig.wErrDetailShowSup == 2)
    {
        m_cStatExtra.AddExtra(m_stConfig.szLastErrDetailKeyName, m_clErrorDet.GetSPErrCodeLast());
    }

    // 状态
    m_stStatus.lpszExtra = (LPSTR)m_cStatExtra.GetExtra();
    lpstStatus = &m_stStatus;


    return WFS_SUCCESS;
}

// 查询能力值
HRESULT CXFS_HCAM::GetCapabilities(LPWFSCAMCAPS &lpstCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stCaps.lpszExtra = (LPSTR)m_cCapsExtra.GetExtra();
    lpstCaps = &m_stCaps;

    return WFS_SUCCESS;
}

//----------------------------CMD接口----------------------------
// 拍照
HRESULT CXFS_HCAM::TakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    UpdateDeviceStatus(); // 更新当前设备状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    m_bCmdRunning = TRUE;   // 命令执行中
    hRet = InnerTakePicture(stTakePict, dwTimeout);
    m_bCmdRunning = FALSE;  // 命令结束
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "拍照: ->InnerTakePicture() Fail, ErrCode: %d, Return: %d.",
            hRet, hRet);
        return hRet;
    }

    return hRet;
}

// 拍照扩展
HRESULT CXFS_HCAM::TakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    UpdateDeviceStatus(); // 更新当前设备状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    m_bCmdRunning = TRUE;   // 命令执行中
    hRet = InnerTakePictureEx(stTakePict, dwTimeout);
    m_bCmdRunning = FALSE;  // 命令结束
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "拍照扩展: ->InnerTakePictureEx() Fail, ErrCode: %d, Return: %d.",
            hRet, hRet);
        return hRet;
    }

    return hRet;
}

// 打开摄像窗口
HRESULT CXFS_HCAM::Display(const WFSCAMDISP &stDisplay, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    UpdateDeviceStatus(); // 更新当前设备状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    hRet = InnerDisplay(stDisplay, dwTimeout);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: ->InnerDisplay() Fail, ErrCode: %d, Return: %d.",
            hRet, hRet);
        return hRet;
    }

    return hRet;
}

HRESULT CXFS_HCAM::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CAM_SUCCESS;

    UpdateDeviceStatus(); // 更新当前设备状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    if (m_bDisplayOK == TRUE)  // Display执行OK,有摄像窗口
    {
        if (m_stConfig.wResetCloseDiaplay == 1)    // Reset执行时是否支持关闭摄像窗口
        {
            STDISPLAYPAR stDispPar;
            stDispPar.wAction = WFS_CAM_DESTROY;

            nRet = m_pCAMDev->Display(stDispPar);
            if (nRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__, "复位: 关闭摄像窗口: ->Display() Fail, ErrCode: %d, 不处理.", nRet);
            } else
            {
                if (m_wWindowsRunMode == WIN_RUN_SHARED)
                {
                    emit vSignHideWin();
                }
                Log(ThisModule, __LINE__, "复位: ->Display() Succ, ErrCode: %d, 不处理.", nRet);
                m_bDisplayOK = FALSE;
            }
        }
    }

    nRet = m_pCAMDev->Reset();
    if (nRet != CAM_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设备: ->Reset() Fail, ErrCode: %s, Return: %d.",
            ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        return ConvertDevErrCode2WFS(nRet);
    }

    m_clErrorDet.SetErrCodeInit();

    return WFS_SUCCESS;
}
