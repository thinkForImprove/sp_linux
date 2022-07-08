/***************************************************************
* 文件名称：XFS_CAM.cpp
* 文件描述：摄像头模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_CAM.h"


static const char *DEVTYPE = "CAM";
static const char *ThisFile = "XFS_CAM.cpp";
#define LOG_NAME    "XFS_CAM.log"				// 30-00-00-00(FT#0031)

//////////////////////////////////////////////////////////////////////////
CXFS_CAM::CXFS_CAM(): m_pMutexGetStatus(nullptr)
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);	// 30-00-00-00(FT#0031)
    memset(&m_sCamIniConfig, 0x00, sizeof(ST_CAM_INI_CONFIG));
    memset(&m_stDevInitParam, 0x00, sizeof(ST_CAM_DEV_INIT_PARAM));
    memset(&m_stStatus, 0x00, sizeof(WFSCAMSTATUS));
    memset(&m_stStatusOld, 0x00, sizeof(WFSCAMSTATUS));
    memset(&m_stCaps, 0x00, sizeof(WFSCAMCAPS));

    m_wDeviceType = 0;                          // 设备类型(INI指定),缺省0

    m_bDisplayOK = FALSE;
    m_bIsOpenOk = FALSE;						// 30-00-00-00(FT#0031)

    m_qSharedMemData = nullptr;
    showWin = nullptr;

    m_bLiveEventWaitExit = FALSE;               // 指示线程结束标记(活检状态监听事件)
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

    HRESULT hRet = 0;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // 读INI
    InitConifig();

    // 初始化设备状态结构		// 30-00-00-00(FT#0031)
    InitStatus();				// 30-00-00-00(FT#0031)

    // 创建共享内存(摄像)
    if (m_wDeviceType == CAM_DEV_CLOUDWALK)
    {
        if (bCreateSharedMemory(m_sCamIniConfig.szCamDataSMemName,
                                m_sCamIniConfig.ulCamDataSMemSize) != TRUE)
        {
            Log(ThisModule, __LINE__, "摄像共享内存创建失败");
            return WFS_ERR_INTERNAL_ERROR;
        }

        qRegisterMetaType<CAM_SHOWWIN_INFO>("CAM_SHOWWIN_INFO");
        connect(this, SIGNAL(vSignShowWin(CAM_SHOWWIN_INFO)), this, SLOT(vSlotShowWin(CAM_SHOWWIN_INFO)), Qt::AutoConnection);
        connect(this, SIGNAL(vSignHideWin()), this, SLOT(vSlotHideWin()), Qt::AutoConnection);
    }    

    // 设备驱动动态库验证
    if (strlen(m_sCamIniConfig.szDevDllName) < 1)
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
        hRet = m_pDev.Load(m_sCamIniConfig.szDevDllName,
                           "CreateIDevCAM", DEVTYPE2STR(m_wDeviceType));
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__,
                "加载库失败: DriverDllName=%s, DEVTYPE=%d. ReturnCode:%s",
                m_sCamIniConfig.szDevDllName, m_wDeviceType,
                m_pDev.LastError().toUtf8().constData());
            //return WFS_ERR_HARDWARE_ERROR;
            return hErrCodeChg(hRet);
        }
    }

    m_pDev->SetData(&(m_stDevInitParam), DATATYPE_INIT);
    m_pDev->SetData(&(m_sCamIniConfig.stCamImageMothod), DATATYPE_SET_IMAGEPAR);    // 设置图像参数

    return StartOpen();
}

HRESULT CXFS_CAM::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    /*if (m_bDisplayOK == TRUE)  // Display执行OK,有摄像窗口,执行关闭
    {
        m_pDev->Display(0, WFS_CAM_DESTROY, 0, 0, 0, 0);
    }*/

    if (m_pDev != nullptr)
    {
        //m_pDev->Reset();// 退出前，取消一次
        m_pDev->Close();
    }

    if (m_wDeviceType == CAM_DEV_CLOUDWALK)
    {
        SharedMemRelease();
        emit vSignHideWin();
    }

    if (showWin != nullptr)
    {
        delete showWin;
        showWin = nullptr;
    }

    m_bIsOpenOk = FALSE;			// 30-00-00-00(FT#0031)

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::OnStatus()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 空闲更新状态
    DEVCAMSTATUS stStatus;
    if (m_pDev != nullptr)
    {
        INT nRet = m_pDev->GetStatus(stStatus);		// 30-00-00-00(FT#0031)
        if (m_bIsOpenOk == FALSE &&					// 30-00-00-00(FT#0031)
            stStatus.fwDevice == DEVICE_ONLINE)		// 30-00-00-00(FT#0031)
        {											// 30-00-00-00(FT#0031)
            StartOpen();    // 重连					// 30-00-00-00(FT#0031)
        }											// 30-00-00-00(FT#0031)
    }
    UpdateStatus(stStatus);

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
    {												// 30-00-00-00(FT#0031)
        m_pDev->Cancel();							// 30-00-00-00(FT#0031)
    }												// 30-00-00-00(FT#0031)


    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::GetStatus(LPWFSCAMSTATUS &lpstStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //状态
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpstStatus = &m_stStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::GetCapabilities(LPWFSCAMCAPS &lpstCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 能力值
    m_stCaps.wClass = WFS_SERVICE_CLASS_CAM;
    m_stCaps.fwType = WFS_CAM_TYPE_CAM;
    m_stCaps.fwCameras[WFS_CAM_ROOM]        = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_PERSON]      = WFS_CAM_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_EXITSLOT]    = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_EXTRA]       = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_HIGHTCAMERA] = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[5]                   = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_PANORAMIC]   = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[7]                   = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.usMaxPictures = 1000;
    m_stCaps.fwCamData = WFS_CAM_MANADD;
    m_stCaps.usMaxDataLength = 67;
    m_stCaps.fwCharSupport = WFS_CAM_ASCII;
    m_stCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    m_stCaps.bPictureFile = TRUE;
    m_stCaps.bAntiFraudModule = FALSE;

    lpstCaps = &m_stCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::TakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;
    m_bCmdRunning = TRUE;   // 命令执行中
    hRet = InnerTakePicture(stTakePict, dwTimeout);
    m_bCmdRunning = FALSE;  // 命令结束
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "摄像命令(TakePicture)失败: ->InnerTakePicture() Fail. ReturnErr: %d, ReturnCode:%d",
            hRet, hRet);
        return hRet;
    }

    return hRet;
}

HRESULT CXFS_CAM::TakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;
    m_bCmdRunning = TRUE;   // 命令执行中
    hRet = InnerTakePictureEx(stTakePict, dwTimeout);
    m_bCmdRunning = FALSE;  // 命令结束
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "摄像命令(TakePictureEx)失败: ->InnerTakePictureEx() Fail. ReturnErr: %d, ReturnCode:%d",
            hRet, hRet);
        return hRet;
    }

    return hRet;
}

HRESULT CXFS_CAM::Display(const WFSCAMDISP &stDisplay, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    hRet = InnerDisplay(stDisplay, dwTimeout);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: ->InnerDisplay() Fail. ReturnErr: %d, ReturnCode:%d",
            hRet, hRet);
        return hRet;
    }

    return hRet;
}

// 启动窗口+拍照处理
HRESULT CXFS_CAM::DisplayEx(const WFSCAMDISPEX &stDisplayEx, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;
    m_bCmdRunning = TRUE;   // 命令执行中
    hRet = InnerDisplayEx(stDisplayEx, dwTimeout);
    m_bCmdRunning = FALSE;  // 命令结束
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口拍照(DisplayEx)失败: ->InnerDisplayEx() Fail. ReturnErr: %d, ReturnCode:%d",
            hRet, hRet);
        return hRet;
    }

    return hRet;
}

HRESULT CXFS_CAM::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    if (m_bDisplayOK == TRUE)  // Display执行OK,有摄像窗口
    {
        if (m_sCamIniConfig.wResetCloseDiaplay == 1)    // Reset执行时是否支持关闭摄像窗口
        {
            m_bLiveEventWaitExit = TRUE;    // 命令返回则立即结束活体图像结果检查线程
            //m_pDev->Cancel();               // 执行一次取消,避免TakePicture命令正在执行中导致无法关闭窗口

            hRet = m_pDev->Display(0, WFS_CAM_DESTROY, 0, 0, 0, 0);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__, "复位: 关闭摄像窗口(Display)失败, ErrCode:%d, 不处理.", hRet);
            } else
            {
                if (m_wDeviceType == CAM_DEV_CLOUDWALK)
                {
                    emit vSignHideWin();
                }
                Log(ThisModule, __LINE__, "复位: 关闭摄像窗口(Display)完成, ErrCode:%d, 不处理.", hRet);
                m_bDisplayOK = FALSE;
            }
        }
    }

    hRet = m_pDev->Reset();
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "设备复位失败．ReturnCode:%d", hRet);
        //return WFS_ERR_HARDWARE_ERROR;
        return hErrCodeChg(hRet);
    }

    return WFS_SUCCESS;
}
