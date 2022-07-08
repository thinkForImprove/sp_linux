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

    m_wDeviceModeSup = CAM_MODE_ROOM;           // 摄像模式支持(INI指定),缺省人脸
    m_wDeviceType = 0;                          // 人脸摄像设备类型(INI指定),缺省0
    m_wDeviceRoomType = 0;                      // 全景摄像设备类型(INI指定),缺省0
    m_wDeviceHighType = 0;                      // 高拍摄像设备类型(INI指定),缺省0

    //bDisplyOK = FALSE;
    //m_bIsOpenOk = FALSE;						// 30-00-00-00(FT#0031)
    m_wDisplayOK = 0;                           // 当前摄像窗口模式
    m_wIsOpenOk = 0;                            // 设备打开

    m_qSharedMemData = nullptr;
    showWin = nullptr;

    m_bLiveEventWaitExit = FALSE;               // 指示线程结束标记(活检状态监听事件)
    m_wVRTCount = 0;
    m_bCmdRunning = FALSE;                      // 当前是否有命令执行中
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

    // 创建共享内存(云从摄像使用)
    if (m_wDeviceType == CAM_DEV_CLOUDWALK ||
        m_wDeviceRoomType == CAM_DEV_CLOUDWALK ||
        m_wDeviceHighType == CAM_DEV_CLOUDWALK ||
        m_wDeviceType == CAM_DEV_ZLF1000A3 ||
        m_wDeviceRoomType == CAM_DEV_ZLF1000A3 ||
        m_wDeviceHighType == CAM_DEV_ZLF1000A3)
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

    // 加载设备驱动动态库(人脸+高拍)
    if ((m_wDeviceModeSup & CAM_MODE_PERSON) == CAM_MODE_PERSON)    // 支持人脸摄像
    {
        if (m_pDev == nullptr)
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
    }

    if ((m_wDeviceModeSup & CAM_MODE_ROOM) == CAM_MODE_ROOM)    // 支持全景摄像
    {
        // 同一设备只申请一个DevCAM接口实例,如果已申请则不再新建
        if (m_wDeviceRoomType != m_wDeviceType ||
            (m_wDeviceRoomType == m_wDeviceType && m_pDev == nullptr))
        {
            if (m_pDevRoom == NULL)
            {
                hRet = m_pDevRoom.Load(m_sCamIniConfig.szDevDllName,
                                       "CreateIDevCAM", DEVTYPE2STR(m_wDeviceRoomType));
                if (hRet != 0)
                {
                    Log(ThisModule, __LINE__,
                        "加载库失败: DriverDllName=%s, DEVTYPE=%d. ReturnCode:%s",
                        m_sCamIniConfig.szDevDllName, m_wDeviceRoomType,
                        m_pDevRoom.LastError().toUtf8().constData());
                    //return WFS_ERR_HARDWARE_ERROR;
                    return hErrCodeChg(hRet);
                }
            }

            m_pDevRoom->SetData(&(m_stDevInitParam), DATATYPE_INIT);
            m_pDevRoom->SetData(&(m_sCamIniConfig.stCamImageMothod), DATATYPE_SET_IMAGEPAR);    // 设置图像参数
        } else
        {
            if (m_pDev != nullptr)
            {
                m_pDevRoom = m_pDev.GetType();
            }
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_HIGH) == CAM_MODE_HIGH)    // 支持高拍摄像
    {
        if ((m_wDeviceHighType != m_wDeviceType && m_wDeviceHighType != m_wDeviceRoomType) ||
            (m_wDeviceHighType == m_wDeviceType && m_pDev == nullptr) ||
            (m_wDeviceHighType == m_wDeviceRoomType && m_pDevRoom == nullptr))
        {
            if (m_pDevHigh == NULL)
            {
                hRet = m_pDevHigh.Load(m_sCamIniConfig.szDevDllName,
                                       "CreateIDevCAM", DEVTYPE2STR(m_wDeviceHighType));
                if (hRet != 0)
                {
                    Log(ThisModule, __LINE__,
                        "加载库失败: DriverDllName=%s, DEVTYPE=%d. ReturnCode:%s",
                        m_sCamIniConfig.szDevDllName, m_wDeviceHighType,
                        m_pDevHigh.LastError().toUtf8().constData());
                    //return WFS_ERR_HARDWARE_ERROR;
                    return hErrCodeChg(hRet);
                }
            }

            m_pDevHigh->SetData(&(m_stDevInitParam), DATATYPE_INIT);
            m_pDevHigh->SetData(&(m_sCamIniConfig.stCamImageMothod), DATATYPE_SET_IMAGEPAR);    // 设置图像参数
        } else
        {
            if (m_wDeviceHighType == m_wDeviceType && m_pDev != nullptr)
            {
                m_pDevHigh = m_pDev.GetType();
            } else
            if (m_wDeviceHighType == m_wDeviceRoomType && m_pDevRoom != nullptr)
            {
                m_pDevHigh = m_pDevRoom.GetType();
            }
        }
    }

    return StartOpen();
}

HRESULT CXFS_CAM::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    /*if (bDisplyOK == TRUE)  // Display执行OK,有摄像窗口,执行关闭
    {
        m_pDev->Display(0, WFS_CAM_DESTROY, 0, 0, 0, 0);
    }*/

    if ((m_wDeviceModeSup & CAM_MODE_PERSON) == CAM_MODE_PERSON)    // 支持人脸摄像
    {
        if (m_pDev != nullptr)
        {
            //m_pDev->Reset();// 退出前，取消一次
            m_pDev->Close();
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_ROOM) == CAM_MODE_ROOM)    // 支持全景摄像
    {
        if (m_wDeviceRoomType != m_wDeviceType)
        {
            if (m_pDevRoom != nullptr)
            {
                m_pDevRoom->Close();
            }
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_HIGH) == CAM_MODE_HIGH)    // 支持高拍摄像
    {
        if (m_wDeviceHighType != m_wDeviceType &&
            m_wDeviceHighType != m_wDeviceRoomType)
        {
            if (m_pDevHigh != nullptr)
            {
                m_pDevHigh->Close();
            }
        }
    }

    if (m_wDeviceType == CAM_DEV_CLOUDWALK ||
        m_wDeviceRoomType == CAM_DEV_CLOUDWALK ||
        m_wDeviceHighType == CAM_DEV_CLOUDWALK)
    {
        SharedMemRelease();
        emit vSignHideWin();
    }

    if (showWin != nullptr)
    {
        delete showWin;
        showWin = nullptr;
    }

    //m_bIsOpenOk = FALSE;			// 30-00-00-00(FT#0031)
    m_wIsOpenOk = 0;

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::OnStatus()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 空闲更新状态
    DEVCAMSTATUS stStatus, stStatusRoom, stStatusHigh;
    if ((m_wDeviceModeSup & CAM_MODE_PERSON) == CAM_MODE_PERSON)    // 人脸摄像
    {
        if (m_pDev != nullptr)
        {
            INT nRet = m_pDev->GetStatus(stStatus);		// 30-00-00-00(FT#0031)
            if ((m_wIsOpenOk & CAM_MODE_PERSON) != CAM_MODE_PERSON ||					// 30-00-00-00(FT#0031)
                stStatus.fwDevice == DEVICE_OFFLINE)		// 30-00-00-00(FT#0031)
            {											// 30-00-00-00(FT#0031)
                StartOpen();    // 重连					// 30-00-00-00(FT#0031)
            }											// 30-00-00-00(FT#0031)
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_ROOM) == CAM_MODE_ROOM)    // 全景摄像
    {
        if (m_pDevRoom != nullptr)
        {
            INT nRet = m_pDevRoom->GetStatus(stStatusRoom);
            if ((m_wIsOpenOk & CAM_MODE_ROOM) != CAM_MODE_ROOM ||
                stStatusRoom.fwDevice == DEVICE_OFFLINE)
            {
                StartOpen();    // 重连
            }
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_HIGH) == CAM_MODE_HIGH)    // 高拍摄像
    {
        if (m_pDevHigh != nullptr)
        {
            INT nRet = m_pDevHigh->GetStatus(stStatusHigh);
            if ((m_wIsOpenOk & CAM_MODE_HIGH) != CAM_MODE_HIGH ||
                stStatusHigh.fwDevice == DEVICE_OFFLINE)
            {
                StartOpen();    // 重连
            }
        }
    }

    UpdateStatus(stStatus, stStatusRoom, stStatusHigh);

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if ((m_wDeviceModeSup & CAM_MODE_ROOM) == CAM_MODE_ROOM)    // 支持全景摄像
    {
        if (m_pDevRoom != nullptr)
        {
            m_pDevRoom->Cancel();
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_PERSON) == CAM_MODE_PERSON)    // 支持人脸摄像
    {
        if (m_pDev != nullptr)
        {												// 30-00-00-00(FT#0031)
            m_pDev->Cancel();							// 30-00-00-00(FT#0031)
        }		    									// 30-00-00-00(FT#0031)
    }

    if ((m_wDeviceModeSup & CAM_MODE_HIGH) == CAM_MODE_HIGH)    // 支持全景摄像
    {
        if (m_pDevHigh != nullptr)
        {
            m_pDevHigh->Cancel();
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

HRESULT CXFS_CAM::GetStatus(LPWFSCAMSTATUS &lpstStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //状态
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpstStatus = &m_stStatus;

    // Device状态ONLINE 同时 有当前命令执行中, 设置Device状态为BUSY
    if (m_stStatus.fwDevice == WFS_CAM_DEVONLINE && m_bCmdRunning == TRUE)
    {
        m_stStatus.fwDevice = WFS_CAM_DEVBUSY;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::GetCapabilities(LPWFSCAMCAPS &lpstCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 能力值
    m_stCaps.wClass = WFS_SERVICE_CLASS_CAM;
    m_stCaps.fwType = WFS_CAM_TYPE_CAM;
    m_stCaps.fwCameras[WFS_CAM_ROOM]        =
            ((m_wDeviceModeSup & CAM_MODE_ROOM) ==
               CAM_MODE_ROOM ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE);      // 全景摄像
    m_stCaps.fwCameras[WFS_CAM_PERSON]      =
            ((m_wDeviceModeSup & CAM_MODE_PERSON) ==
               CAM_MODE_PERSON ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE);    // 人脸摄像
    m_stCaps.fwCameras[WFS_CAM_EXITSLOT]    = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_EXTRA]       = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_HIGHTCAMERA] =
            ((m_wDeviceModeSup & CAM_MODE_HIGH) ==
               CAM_MODE_HIGH ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE);      // 高拍摄像
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
    WORD    wImageType;

    if (m_wDisplayOK != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像(TakePicture)失败: 命令序列错误,Display未执行．");
        return WFS_ERR_DEV_NOT_READY;
    }

    LPWFSCAMTAKEPICT	lpCmdData = NULL;
    lpCmdData = (LPWFSCAMTAKEPICT)&stTakePict;

    memset(szFileNameFromAp, 0x00, sizeof(szFileNameFromAp));

    if (strlen(m_sCamIniConfig.szTakePicDefSavePath) > 0)
    {
        sprintf(szFileNameFromAp, "%s", m_sCamIniConfig.szTakePicDefSavePath);
    } else
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:INI->TakePicDefSavePath[%s]无效．",
            m_sCamIniConfig.szTakePicDefSavePath);
        return WFS_ERR_INVALID_DATA;
    }

    if(*szFileNameFromAp == 0 || szFileNameFromAp[0] != '/')
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:INI->TakePicDefSavePath[%s]无效．",
                m_sCamIniConfig.szTakePicDefSavePath);
        return WFS_ERR_INVALID_DATA;
    }

    std::string szFileName;
    std::string szFilePath = szFileNameFromAp;		// FilePath and FileName
    int iIndex = szFilePath.rfind('.');
    if (iIndex == -1)
    {
        Log(ThisModule, __LINE__, "摄像(TakePicture)失败:INI->TakePicDefSavePath[%s]无效．",
            m_sCamIniConfig.szTakePicDefSavePath);
        return WFS_ERR_INVALID_DATA;
    } else
    {
        szFileName.clear();
        szFileName = szFilePath.substr(0, iIndex);

        std::string szEx = szFilePath.substr(iIndex, szFilePath.length() - iIndex);
        std::transform(szEx.begin(), szEx.end(), szEx.begin(), ::toupper);
        if (szEx.compare(".BASE64") == 0)
        {
            wImageType = PIC_BASE64;
        } else
        if (szEx.compare(".JPG") == 0)
        {
            wImageType = PIC_JPG;
        } else {
            wImageType = PIC_BMP;
        }
        wImageType = (wImageType | m_sCamIniConfig.wTakePicMakeFormatFlag);
    }

    // 目录验证
    if (bPathCheckAndCreate(szFileNameFromAp, FALSE) != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像(TakePicture)失败:INI->TakePicDefSavePath[%s]创建目录结构失败．", szFileNameFromAp);
        return WFS_ERR_INTERNAL_ERROR;
    }

    hRet = m_pDev->TakePictureEx((LPSTR)(szFileName.c_str()),//lpCmdData->lpszPictureFile,
                                 lpCmdData->lpszCamData,
                                 wImageType, TRUE, m_sCamIniConfig.wTakePicTimeOut);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "摄像命令(TakePicture(%s, %s, %d, %d, %d))下发失败．ReturnCode:%d",
                                    (LPSTR)(szFileName.c_str()), lpCmdData->lpszCamData,
                                    wImageType, TRUE, m_sCamIniConfig.wTakePicTimeOut, hRet);
        return hErrCodeChg(hRet);
    }

    return WFS_SUCCESS;
}

// 摄像处理
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

// 窗口处理
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

    if (m_sCamIniConfig.wResetCloseDiaplay == 1)    // Reset执行时是否支持关闭摄像窗口
    {
        m_bLiveEventWaitExit = TRUE;    // 命令返回则立即结束活体图像结果检查线程
        if (m_wDisplayOK == CAM_MODE_ROOM)  // Display执行OK,有摄像窗口(全景)
        {
            //m_pDevRoom->Cancel();               // 执行一次取消,避免TakePicture命令正在执行中导致无法关闭窗口
            hRet = m_pDevRoom->Display(0, WFS_CAM_DESTROY, 0, 0, 0, 0);
        } else
        if (m_wDisplayOK == CAM_MODE_PERSON)  // Display执行OK,有摄像窗口(人脸)
        {
            //m_pDev->Cancel();               // 执行一次取消,避免TakePicture命令正在执行中导致无法关闭窗口
            hRet = m_pDev->Display(0, WFS_CAM_DESTROY, 0, 0, 0, 0);
        } else
        if (m_wDisplayOK == CAM_MODE_HIGH)  // Display执行OK,有摄像窗口(高拍)
        {
            //m_pDev->Cancel();               // 执行一次取消,避免TakePicture命令正在执行中导致无法关闭窗口
            hRet = m_pDevHigh->Display(0, WFS_CAM_DESTROY, 0, 0, 0, 0);
        }

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
            m_wDisplayOK = 0;
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_PERSON) == CAM_MODE_PERSON)    // 支持人脸摄像
    {
        hRet = m_pDev->Reset();
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__, "人脸摄像设备复位失败．ReturnCode:%d", hRet);
            return hErrCodeChg(hRet);
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_ROOM) == CAM_MODE_ROOM)        // 支持全景摄像
    {
        if (m_wDeviceRoomType != m_wDeviceType)
        {
            hRet = m_pDevRoom->Reset();
            if (hRet != 0)
            {
                Log(ThisModule, __LINE__, "全景摄像设备复位失败．ReturnCode:%d", hRet);
                return hErrCodeChg(hRet);
            }
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_HIGH) == CAM_MODE_HIGH)        // 支持高拍摄像
    {
        if (m_wDeviceHighType != m_wDeviceType &&
            m_wDeviceHighType != m_wDeviceRoomType)
        {
            hRet = m_pDevHigh->Reset();
            if (hRet != 0)
            {
                Log(ThisModule, __LINE__, "高拍摄像设备复位失败．ReturnCode:%d", hRet);
                return hErrCodeChg(hRet);
            }
        }
    }

    return WFS_SUCCESS;
}

// LiveEvent循环执行(支持人脸摄像时有效)
void CXFS_CAM::ThreadLiveEvent_Wait()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wError = 0;
    WORD wGetErr = 0;

    while(true)
    {
        if (m_bLiveEventWaitExit == TRUE)
        {
            break;
        }

        m_pDev->GetData(&wGetErr, DATATYPE_LIVESTAT);

        if (wGetErr > 0 && wGetErr != wError)
        {
            wError = wGetErr;
            FireEXEE_LIVEERROR_Event(ConvertLiveErr2WFS(wError));
        }

        usleep(1000 * 20);
    }

    m_bLiveEventWaitExit = TRUE;
}

void CXFS_CAM::FireEXEE_LIVEERROR_Event(WORD wError)
{
    WFSCAMLIVEERROR stLiveErr;
    stLiveErr.wLiveError = wError;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CAM_LIVEERROR, &stLiveErr);
}

//////////////////////////////////////////////////////////////////////////


