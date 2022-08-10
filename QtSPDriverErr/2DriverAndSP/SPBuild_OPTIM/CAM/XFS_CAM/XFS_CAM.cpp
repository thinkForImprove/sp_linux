/***************************************************************************
* 文件名称: XFS_CAM.cpp
* 文件描述: 摄像头模块命令处理接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年4月4日
* 文件版本: 1.0.0.1
***************************************************************************/

#include "XFS_CAM.h"


static const char *DEVTYPE = "CAM";
static const char *ThisFile = "XFS_CAM.cpp";
#define LOG_NAME    "XFS_CAM.log"

/***************************************************************************
// 主处理                                                                   *
***************************************************************************/
CXFS_CAM::CXFS_CAM(): m_pMutexGetStatus(nullptr)
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);

    MSET_S(&m_stConfig, sizeof(STINICONFIG));           // INI结构体
    m_stCamModeInfo.Clear();                            // 摄像模式结构体变量
    MSET_S(&m_stStatus, sizeof(CWfsCAMStatus));         // 状态结构体
    MSET_S(&m_stStatusOLD, sizeof(CWfsCAMStatus));      // 备份上一次状态结构体
    MSET_S(&m_stCaps, sizeof(CWfsCAMCap));              // 能力值结构体
    m_cStatExtra.Clear();                               // 状态扩展数据
    m_cCapsExtra.Clear();                               // 能力值扩展数据
    MSET_XS(m_nRetErrOLD, 0, sizeof(INT) * 12);         // 处理错误值保存(0:HCAM断线重连)
    MSET_0(m_szFileName);                               // 拍照文件名
    showWin = nullptr;                                  // 摄像窗口
    m_qSharedMemData = nullptr;                         // 摄像数据共享内存
    m_bCmdRunning = FALSE;                              // 是否命令执行中
    m_wDisplayOK = 0;                                   // Display是否已执行
    m_wWindowsRunMode = 0;                              // 窗口运行方式
    MSET_XS(m_wDeviceShowWinMode, WIN_RUN_SDK, sizeof(WORD) * DEV_CNT);// 设备窗口显示方式
    MSET_S(m_szWinProcessPath, sizeof(CHAR) * DEV_CNT * 256);// 摄像窗口外接程序
    m_wIsDevOpenFlag = 0;                               // 设备Open标记(用于记录Open是否成功/断线)
    MSET_0(m_szBuffer);                                 // 公用Buffer
    m_bLiveEventWaitExit = FALSE;                       // 指示线程结束标记(活检状态监听事件)    

    // 设置错误码: 指定SPName, XFSName
    m_clErrorDet.SetSPName((LPSTR)DEVTYPE);
    m_clErrorDet.SetXFSName();
}

CXFS_CAM::~CXFS_CAM()
{
    OnClose();
}

long CXFS_CAM::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();    

    // 加载BaseCAM
    if (0 != m_pBase.Load("SPBaseCAM.dll", "CreateISPBaseCAM", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseCAM失败");        
        SetErrorDetail((LPSTR)EC_ERR_SPBaseLoadFail);
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_CAM::OnOpen(LPCSTR lpLogicalName)
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

    // 检查INI设置摄像支持和设备类型
    hRet = ChkOpenIsCamDevMode();
    if (hRet != WFS_SUCCESS)
    {
        return hRet;
    }

    // 检查设备摄像窗口启动模式
    hRet = ChkOpenShowWinMode();
    if (hRet != WFS_SUCCESS)
    {
        return hRet;
    }

    // 设备驱动动态库验证: 检查DevXXX库是否存在
    if (strlen(m_stConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__,
            "SP[%s]的INI.DriverDllName配置项为空或读取失败", m_strSPName.c_str());
        SetErrorDetail((LPSTR)EC_CAM_XFS_libCAMNotFound);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 加载DevXXX动态库
    if (LoadDevCAMDll(ThisModule) != TRUE)
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

HRESULT CXFS_CAM::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 关闭所有设备连接
    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        if (MI_Enable(i) == TRUE && MI_DevDll(i) != nullptr)
        {
            MI_DevDll(i)->Close();
        }
    }

    // 释放所有设备DevXXX库连接
    m_stCamModeInfo.ReleaseDevCAMDllAll();
    m_stCamModeInfo.Clear();

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

HRESULT CXFS_CAM::OnStatus()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;

    // 取设备状态
    UpdateDeviceStatus();

    // 检查设备状态
    if(m_stStatus.fwDevice == WFS_CAM_DEVOFFLINE ||
       m_stStatus.fwDevice == WFS_CAM_DEVPOWEROFF ||
       m_stStatus.fwDevice == WFS_CAM_DEVNODEVICE)
    {
        // 循环检查: 摄像模式支持且对应设备处于NotOpen状态, 需要重连
        for (INT i = 0; i < LCMODE_CNT; i ++)
        {
            if (MI_Enable(i) == TRUE && AND_IS0(m_wIsDevOpenFlag, LCMODE_TO_CMODE(i)))
            {
                if (MI_DevDll(i) != nullptr)
                {
                    MI_DevDll(i)->SetData(SET_DEV_RECON, nullptr); // 设置为断线重连执行状态
                }
            }
        }

        // 重新Open
        hRet = InnerOpen(TRUE);
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

HRESULT CXFS_CAM::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 根据当前窗口运行模式选择对应的设备连接
    if (m_wWindowsRunMode > 0)
    {
        //m_stCamModeInfo.stModeList[CMODE_TO_LCMODE(m_wWindowsRunMode)].m_pCAMDev->Cancel();
        if (MI_DevDll(CMODE_TO_LCMODE(m_wWindowsRunMode)) != nullptr)
        {
            MI_DevDll(CMODE_TO_LCMODE(m_wWindowsRunMode))->Cancel();
        }
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

//----------------------------INFO接口----------------------------
// 查询状态
HRESULT CXFS_CAM::GetStatus(LPWFSCAMSTATUS &lpstStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 取当前设备状态
    UpdateDeviceStatus();

    // 设置错误码
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
HRESULT CXFS_CAM::GetCapabilities(LPWFSCAMCAPS &lpstCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stCaps.lpszExtra = (LPSTR)m_cCapsExtra.GetExtra();
    lpstCaps = &m_stCaps;

    return WFS_SUCCESS;
}

//----------------------------CMD接口----------------------------
// 拍照
HRESULT CXFS_CAM::TakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout)
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
HRESULT CXFS_CAM::TakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout)
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
HRESULT CXFS_CAM::Display(const WFSCAMDISP &stDisplay, DWORD dwTimeout)
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

HRESULT CXFS_CAM::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CAM_SUCCESS;
    BOOL bResetResult = TRUE;
    INT nRetErr = CAM_SUCCESS;

    UpdateDeviceStatus(); // 更新当前设备状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    if (m_wDisplayOK > 0)  // Display执行OK,有摄像窗口打开
    {
        if (m_stConfig.wResetCloseDiaplay == 1)    // Reset执行时支持关闭摄像窗口
        {
            m_bLiveEventWaitExit = TRUE;    // 命令返回则立即结束活体图像结果检查线程

            if (MI_DevDll(CMODE_TO_LCMODE(m_wDisplayOK)) != nullptr)
            {
                STDISPLAYPAR stDispPar;
                stDispPar.wAction = WFS_CAM_DESTROY;

                nRet = MI_DevDll(CMODE_TO_LCMODE(m_wDisplayOK))->Display(stDispPar);
                if (nRet != WFS_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "复位: 关闭摄像窗口: ->Display() Fail, ErrCode: %d, 不处理.", nRet);
                } else
                {
                    if (m_wWindowsRunMode == WIN_RUN_SHARED)
                    {
                        emit vSignHideWin();
                    }
                    Log(ThisModule, __LINE__,
                        "复位: ->Display() Succ, ErrCode: %d, 不处理.", nRet);
                    m_wDisplayOK = 0;
                }
            }
        }
    }

    // 对所有支持的设备进行复位
    for (INT i = 0; i < LCMODE_CNT; i ++)
    {
        if (MI_Enable(i) == TRUE)
        {
            if (MI_DevDll(i) != nullptr)
            {
                nRet = MI_DevDll(i)->Reset();
                if (nRet != CAM_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "复位: %s|%s: ->Reset() Fail, ErrCode: %s, Return: %d.",
                        MI_ModeName(i), DEVTYPE2STR(MI_GetDevType(i)),
                        ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
                    nRetErr = nRet;
                    bResetResult = FALSE;
                }
            } else
            {
                Log(ThisModule, __LINE__,
                    "复位: %s|%s: 设备连接为NULL, 不执行, 不处理.",
                    MI_ModeName(i), DEVTYPE2STR(MI_GetDevType(i)));
            }
        }
    }

    // 复位失败, 返回最后一个设备的错误码
    if (bResetResult != TRUE)
    {
        Log(ThisModule, __LINE__,
            "复位: ->Reset() Fail, 返回最后一次ErrCode: %s, Return: %d.",
            ConvertDevErrCodeToStr(nRetErr), ConvertDevErrCode2WFS(nRetErr));
        return ConvertDevErrCode2WFS(nRetErr);
    }

    m_clErrorDet.SetErrCodeInit();

    return WFS_SUCCESS;
}

// -------------------------------------- END --------------------------------------

