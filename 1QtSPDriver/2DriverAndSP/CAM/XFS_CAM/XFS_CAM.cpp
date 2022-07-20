#include "XFS_CAM.h"
#include "file_access.h"
#include <algorithm>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <QSettings>
#include <QMetaType>
#include <QWindow>

// CAM SP 版本号
BYTE    byVRTU[17] = {"HWCAMSTE00000100"};

static const char *DEVTYPE = "CAM";
static const char *ThisFile = "XFS_CAM.cpp";
//////////////////////////////////////////////////////////////////////////
CXFS_CAM::CXFS_CAM(): m_pMutexGetStatus(nullptr)
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    memset(&m_sCamIniConfig, 0x00, sizeof(ST_CAM_INI_CONFIG));
    memset(&m_stDevInitParam, 0x00, sizeof(ST_CAM_DEV_INIT_PARAM));
    memset(&m_stStatus, 0x00, sizeof(WFSCAMSTATUS));
    memset(&m_stStatusOld, 0x00, sizeof(WFSCAMSTATUS));
    memset(&m_stCaps, 0x00, sizeof(WFSCAMCAPS));
    bDisplyOK = FALSE;
    m_bIsOpenOk = FALSE;						// 30-00-00-00(FT#0031)

    m_qSharedMemData = nullptr;
    showWin = nullptr;
    qRegisterMetaType<CAM_SHOWWIN_INFO>("CAM_SHOWWIN_INFO");
    connect(this, SIGNAL(vSignShowWin(CAM_SHOWWIN_INFO)), this, SLOT(vSlotShowWin(CAM_SHOWWIN_INFO)), Qt::AutoConnection);
    connect(this, SIGNAL(vSignHideWin()), this, SLOT(vSlotHideWin()), Qt::AutoConnection);

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

    // 创建共享内存(摄像)
    if (bCreateSharedMemory(m_sCamIniConfig.szCamDataSMemName,
                            m_sCamIniConfig.ulCamDataSMemSize) != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像共享内存创建失败");
        return WFS_ERR_INTERNAL_ERROR;
    }
/*
    QFile file(m_sCamIniConfig.szCamCheckOpenFile);
    if (!file.exists())
    {
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        file.close();
    }
*/
    // 初始化设备状态结构
    InitStatus();

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
                           "CreateIDevCAM", DEVTYPE_CHG(m_sCamIniConfig.wCamDevType));
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__,
                "加载库失败: DriverDllName=%s, DEVTYPE=%d|%s. ReturnCode:%s",
                m_sCamIniConfig.szDevDllName, m_sCamIniConfig.wCamDevType,
                DEVTYPE_CHG(m_sCamIniConfig.wCamDevType),
                m_pDev.LastError().toUtf8().constData());
            //return WFS_ERR_HARDWARE_ERROR;
            return hErrCodeChg(hRet);
        }
    }


    m_pDev->SetData(&(m_stDevInitParam), DATATYPE_INIT);
    /*
    if(m_cINI.IsNeedUpdateINI() == false)
    {
        m_cINI.SetNewSection("default");
        cPR = m_cINI.GetWriterSection("default");
    }
    */
    // 打开连接
    hRet = m_pDev->Open(nullptr);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败．ReturnCode:%d", hRet);
        //return WFS_ERR_HARDWARE_ERROR;
        //hRet = OnStatus();
        int i;
        char vid_1[5] = {0};
        for(i=0; i < 4; i++)
        {
            if(m_sCamIniConfig.stCamOpenType.szNisVid[i] > 'A' && m_sCamIniConfig.stCamOpenType.szNisVid[i] < 'Z')
            {
                vid_1[i] = m_sCamIniConfig.stCamOpenType.szNisVid[i] + 32;
            }
            else
            {
                vid_1[i] = m_sCamIniConfig.stCamOpenType.szNisVid[i];
            }
        }
        Log(ThisModule, __LINE__, "sec : %s", vid_1);

        char pid_1[5] = {0};
        for(i=0; i < 4; i++)
        {
            if(m_sCamIniConfig.stCamOpenType.szNisPid[i] > 'A' && m_sCamIniConfig.stCamOpenType.szNisPid[i] < 'Z')
            {
                pid_1[i] = m_sCamIniConfig.stCamOpenType.szNisPid[i] + 32;
            }
            else
            {
                pid_1[i] = m_sCamIniConfig.stCamOpenType.szNisPid[i];
            }
        }
        Log(ThisModule, __LINE__, "sec : %s", pid_1);

        char vid_2[5] = {0};
        for(i=0; i < 4; i++)
        {
            if(m_sCamIniConfig.stCamOpenType.szVisVid[i] > 'A' && m_sCamIniConfig.stCamOpenType.szVisVid[i] < 'Z')
            {
                vid_2[i] = m_sCamIniConfig.stCamOpenType.szVisVid[i] + 32;
            }
            else
            {
                vid_2[i] = m_sCamIniConfig.stCamOpenType.szVisVid[i];
            }
        }
        Log(ThisModule, __LINE__, "sec : %s", vid_2);

        char pid_2[5] = {0};
        for(i=0; i < 4; i++)
        {
            if(m_sCamIniConfig.stCamOpenType.szVisPid[i] > 'A' && m_sCamIniConfig.stCamOpenType.szVisPid[i] < 'Z')
            {
                pid_2[i] = m_sCamIniConfig.stCamOpenType.szVisPid[i] + 32;
            }
            else
            {
                pid_2[i] = m_sCamIniConfig.stCamOpenType.szVisPid[i];
            }
        }
        Log(ThisModule, __LINE__, "sec : %s", pid_2);

        int nRet = SearchVideoIdxFromVidPid(vid_1, pid_1);
        Log(ThisModule, __LINE__, "di yi gen:%d", nRet);

        int rRet = SearchVideoIdxFromVidPid(vid_2, pid_2);
        Log(ThisModule, __LINE__, "di er gen:%d", rRet);

        int count = nRet + rRet;
        if(count < 2 && count != 1)
        {
            if(m_sCamIniConfig.szCamCheckOpenFile != nullptr)
            {
                Log(ThisModule, __LINE__, "打开设备连接失败2222．ReturnCode:%s", m_sCamIniConfig.szCamCheckOpenFile);

                QByteArray strFile(m_sCamIniConfig.szCamCheckOpenFile);
                CINIFileReader m_cINI;
                m_cINI.LoadINIFile(strFile.constData());
                CINIWriter cPR = m_cINI.GetWriterSection("default");
                cPR.SetValue("provider","NODEVICE");
            }
        }
        return hErrCodeChg(hRet);
    }
    // 更新扩展状态
    CHAR szDevCAMVer[MAX_PATH] = { 0x00 };
    m_pDev->GetVersion(szDevCAMVer, sizeof(szDevCAMVer) - 1, 1);

    CHAR szFWVer[MAX_PATH] = { 0x00 };
    m_pDev->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

    CHAR szDevOtherVer[MAX_PATH] = { 0x00 };
    m_pDev->GetVersion(szDevOtherVer, sizeof(szDevOtherVer) - 1, 3);

    m_cExtra.AddExtra("VRTCount", "4");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", szDevCAMVer);
    m_cExtra.AddExtra("VRTDetail[02]", szFWVer);
    m_cExtra.AddExtra("VRTDetail[03]", szDevOtherVer);

    // 更新一次状态
    OnStatus();

    Log(ThisModule, 1, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    return WFS_SUCCESS;
}

INT CXFS_CAM::SearchVideoIdxFromVidPid(LPSTR lpVid, LPSTR lpPid)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
#define MCMP_IS0(a, b) \
    (memcmp(a, b, strlen(b)) == 0 && memcmp(a, b, strlen(a)) == 0)
    struct udev *stUDev = nullptr;
    struct udev_enumerate *stUDevEnumErate = nullptr;
    struct udev_list_entry *stUDevList = nullptr;
    INT nVideoIdx = 0;
    int count = 0;
    // 实例化
    stUDev = udev_new();
    if (stUDev == nullptr)
    {
        return -1;
    }

    //
    stUDevEnumErate = udev_enumerate_new(stUDev);
    if (stUDevEnumErate == nullptr)
    {
        return -2;
    }

    //
    udev_enumerate_add_match_subsystem(stUDevEnumErate, "video4linux");
    udev_enumerate_scan_devices(stUDevEnumErate);
    udev_list_entry_foreach(stUDevList, udev_enumerate_get_list_entry(stUDevEnumErate))
    {
        struct udev_device *stDevice = nullptr;
        stDevice = udev_device_new_from_syspath(udev_enumerate_get_udev(stUDevEnumErate),
                                                udev_list_entry_get_name(stUDevList));
        const char *szBuffer = udev_device_get_property_value(stDevice, "ID_MODEL_ID");
        if (stDevice != nullptr)
        {
            CHAR *szGetVid = nullptr, *szGetPid = nullptr;
            szGetVid = (CHAR*)udev_device_get_property_value(stDevice, "ID_VENDOR_ID");
            szGetPid = (CHAR*)udev_device_get_property_value(stDevice, "ID_MODEL_ID");
            if (szGetVid != nullptr && szGetVid != nullptr)
            {
                if (MCMP_IS0(szGetVid, lpVid) && MCMP_IS0(szGetPid, lpPid))
                {
                    count++;
                    nVideoIdx = atoi(udev_device_get_sysnum(stDevice));
                    udev_device_unref(stDevice);
                    udev_enumerate_unref(stUDevEnumErate);
                    udev_unref(stUDev);
                    return count;
                }
            }
            udev_device_unref(stDevice);
        } else
        {
            udev_enumerate_unref(stUDevEnumErate);
            udev_unref(stUDev);
            return 0;
        }
    }

    return 0;
}

HRESULT CXFS_CAM::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pDev != nullptr)
    {
        m_pDev->Reset();// 退出前，取消一次
        m_pDev->Close();
    }

    SharedMemRelease();

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
// 新增Open处理,初始Open和断线重连Open	// 30-00-00-00(FT#0031)
HRESULT CXFS_CAM::StartOpen()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = CAM_SUCCESS;

    // 打开连接
    hRet = m_pDev->Open(nullptr);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败．ReturnCode:%d", hRet);
        //return WFS_ERR_HARDWARE_ERROR;
        return WFS_SUCCESS; //hErrCodeChg(hRet);
    }

    // 更新扩展状态
    CHAR szDevCAMVer[MAX_PATH] = { 0x00 };
    m_pDev->GetVersion(szDevCAMVer, sizeof(szDevCAMVer) - 1, 1);

    CHAR szFWVer[MAX_PATH] = { 0x00 };
    m_pDev->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

    CHAR szDevOtherVer[MAX_PATH] = { 0x00 };
    m_pDev->GetVersion(szDevOtherVer, sizeof(szDevOtherVer) - 1, 3);


    m_cExtra.AddExtra("VRTCount", "4");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", szDevCAMVer);
    m_cExtra.AddExtra("VRTDetail[02]", szFWVer);
    m_cExtra.AddExtra("VRTDetail[03]", szDevOtherVer);

    m_bIsOpenOk = TRUE;

    // 更新一次状态
    OnStatus();

    Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    return WFS_SUCCESS;
}
HRESULT CXFS_CAM::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
       m_pDev->Reset();

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
    WORD    wImageType;

    if (bDisplyOK != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像(TakePicture)失败: 命令序列错误,Display未执行．");
        return WFS_ERR_DEV_NOT_READY;
    }

    LPWFSCAMTAKEPICT	lpCmdData = NULL;
    lpCmdData = (LPWFSCAMTAKEPICT)&stTakePict;

    memset(szFileNameFromAp, 0x00, sizeof(szFileNameFromAp));

    if (strlen(m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath) > 0)
    {
        sprintf(szFileNameFromAp, "%s", m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath);
    } else
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:INI->TakePicDefSavePath[%s]无效．",
            m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath);
        return WFS_ERR_INVALID_DATA;
    }

    if(*szFileNameFromAp == 0 || szFileNameFromAp[0] != '/')
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:INI->TakePicDefSavePath[%s]无效．",
                m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath);
        return WFS_ERR_INVALID_DATA;
    }

    std::string szFileName;
    std::string szFilePath = szFileNameFromAp;		// FilePath and FileName
    int iIndex = szFilePath.rfind('.');
    if (iIndex == -1)
    {
        Log(ThisModule, __LINE__, "摄像(TakePicture)失败:INI->TakePicDefSavePath[%s]无效．",
            m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath);
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
        wImageType = (wImageType | m_sCamIniConfig.stCamSaveImageCfg.wTakePicMakeFormatFlag);
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

HRESULT CXFS_CAM::TakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;
    WORD    wImageType;

    if (bDisplyOK != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败: 命令序列错误,Display未执行．");
        return WFS_ERR_DEV_NOT_READY;
    }

    LPWFSCAMTAKEPICTEX	lpCmdData = NULL;
    lpCmdData = (LPWFSCAMTAKEPICTEX)&stTakePict;

    memset(szFileNameFromAp, 0x00, sizeof(szFileNameFromAp));

    if (lpCmdData->lpszPictureFile == NULL)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[NULL]无效．");
        return WFS_ERR_INVALID_DATA;
    }

    if (strlen(lpCmdData->lpszPictureFile) > 0)
    {
        sprintf(szFileNameFromAp, "%s", lpCmdData->lpszPictureFile);
    } else
    if (strlen(m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath) > 0)
    {
        sprintf(szFileNameFromAp, "%s", m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath);
    } else
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]无效．", szFileNameFromAp);
        return WFS_ERR_INVALID_DATA;
    }

    if(*szFileNameFromAp == 0 || szFileNameFromAp[0] != '/')
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]无效．", szFileNameFromAp);
        return WFS_ERR_INVALID_DATA;
    }

    std::string szFileName;
    std::string szFilePath = szFileNameFromAp;		// FilePath and FileName
    int iIndex = szFilePath.rfind('.');
    if (iIndex == -1)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]无效．", szFileNameFromAp);
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
        wImageType = (wImageType | m_sCamIniConfig.stCamSaveImageCfg.wTakePicMakeFormatFlag);
    }

    // 目录验证
    if (bPathCheckAndCreate(szFileNameFromAp, FALSE) != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]创建目录结构失败．", szFileNameFromAp);
        return WFS_ERR_INTERNAL_ERROR;
    }

    hRet = m_pDev->TakePictureEx((LPSTR)(szFileName.c_str()),//lpCmdData->lpszPictureFile,
                                 lpCmdData->lpszCamData,
                                 wImageType, TRUE, m_sCamIniConfig.wTakePicTimeOut);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "摄像命令(TakePictureEx(%s, %s, %d, %d, %d))下发失败．ReturnCode:%d",
                                    (LPSTR)(szFileName.c_str()), lpCmdData->lpszCamData,
                                    wImageType, TRUE, m_sCamIniConfig.wTakePicTimeOut, hRet);
        //return WFS_ERR_HARDWARE_ERROR;
        return hErrCodeChg(hRet);
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::Display(const WFSCAMDISP &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    LPWFSCAMDISP lpDisplay = NULL;
    lpDisplay = (LPWFSCAMDISP)&stTakePict;

    if(lpDisplay->wAction != WFS_CAM_CREATE && lpDisplay->wAction != WFS_CAM_DESTROY)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败:入参wAction[%d]无效．ReturnCode:%d",
            lpDisplay->wAction, hRet);
        return WFS_ERR_INVALID_DATA;
    }

    if(lpDisplay->wHeight == 0 || lpDisplay->wWidth == 0)// || lpDisplay->hWnd == NULL)
    {
        /*Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败:入参wHeight[%d]/wWidth[%d]/hWnd[%d]无效．ReturnCode:%d",
            lpDisplay->wHeight, lpDisplay->wWidth, lpDisplay->hWnd, hRet);*/
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败:入参wHeight[%d]/wWidth[%d]无效．ReturnCode:%d",
                    lpDisplay->wHeight, lpDisplay->wWidth, hRet);
        return WFS_ERR_INVALID_DATA;
    }

    //紫金AP传入参数为322*322，显示画面异常，更改为320*320
    if(322 == lpDisplay->wWidth && 322 == lpDisplay->wHeight){
        lpDisplay->wWidth = 320;
        lpDisplay->wHeight = 320;
    }

    hRet = m_pDev->Display(lpDisplay->hWnd, lpDisplay->wAction, lpDisplay->wX, lpDisplay->wY, lpDisplay->wWidth, lpDisplay->wHeight);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败．ReturnCode:%d", hRet);
        //return WFS_ERR_HARDWARE_ERROR;
        return hErrCodeChg(hRet);
    }

    if (lpDisplay->wAction == WFS_CAM_CREATE)
    {
        CAM_SHOWWIN_INFO stCamInfo;
        stCamInfo.hWnd = lpDisplay->hWnd;
        stCamInfo.wX = lpDisplay->wX;
        stCamInfo.wY = lpDisplay->wY;
        stCamInfo.wWidth = lpDisplay->wWidth;
        stCamInfo.wHeight = lpDisplay->wHeight;
        emit vSignShowWin(stCamInfo);

        /*if (showWin->isVisible() != TRUE)
        {
            Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败．SP无法创建摄像界面", hRet);
            return WFS_ERR_INVALID_DATA;
        }*/

        bDisplyOK = TRUE;
    } else
    if (lpDisplay->wAction == WFS_CAM_DESTROY)
    {
        emit vSignHideWin();
        bDisplyOK = FALSE;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    bDisplyOK = FALSE;

    hRet = m_pDev->Reset();
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "设备复位失败．ReturnCode:%d", hRet);
        //return WFS_ERR_HARDWARE_ERROR;
        return hErrCodeChg(hRet);
    }

    return WFS_SUCCESS;
}

bool CXFS_CAM::UpdateStatus(const DEVCAMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);

    AutoMutex(*m_pMutexGetStatus);

    WORD fwDevice = WFS_CAM_DEVONLINE;
    /*WORD fwDevice;
    WORD fwMedia[WFS_CAM_CAMERAS_SIZE];
    WORD fwCameras[WFS_CAM_CAMERAS_SIZE];
    USHORT usPictures[WFS_CAM_CAMERAS_SIZE];
    LPSTR lpszExtra;
    WORD wAntiFraudModule;*/

    memcpy(&m_stStatusOld, &m_stStatus, sizeof(WFSCAMSTATUS));

    // 设备状态
    //UpdateExtra(stStatus.szErrCode);
    switch (stStatus.fwDevice)
    {
    case WFS_CAM_DEVNODEVICE:
    case DEVICE_OFFLINE:
        //fwDevice = WFS_CAM_DEVOFFLINE;
        fwDevice = WFS_CAM_DEVHWERROR;
        break;
    case DEVICE_ONLINE:
        fwDevice = WFS_CAM_DEVONLINE;
        break;
    default:
        fwDevice = WFS_CAM_DEVHWERROR;
        break;
    }

    // 判断状态是否有变化
    if (m_stStatus.fwDevice != fwDevice)
    {
        m_pBase->FireStatusChanged(fwDevice);
        // 故障时，也要上报故障事件
        if (fwDevice == WFS_CAM_DEVHWERROR)
            m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_cExtra.GetErrDetail("ErrorDetail"));
    }
    m_stStatus.fwDevice = fwDevice;

    m_stStatus.fwMedia[WFS_CAM_PERSON] =
            (stStatus.fwMedia[WFS_CAM_PERSON] == MEDIA_OK ? WFS_CAM_MEDIAOK : WFS_CAM_MEDIAUNKNOWN);
    m_stStatus.fwCameras[WFS_CAM_PERSON] =
            (stStatus.fwCameras[WFS_CAM_PERSON] == STATUS_OK ? WFS_CAM_CAMOK : WFS_CAM_CAMNOTSUPP);

    // 扩展状态
//    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();


    // 测试
    //m_pBase->FireEvent(WFS_SYSTEM_EVENT, WFS_SYSE_DEVICE_STATUS, &m_stStatus);*/

    return true;
}

void CXFS_CAM::UpdateExtra(string strErrCode, string strDevVer/* = ""*/)
{
    /*if (strErrCode == "000")
        strErrCode.insert(0, "000000");// 没故障时显示
    else
        strErrCode.insert(0, "001707");// 固化的前六位

    m_cExtra.AddExtra("ErrorDetail", strErrCode.c_str());
    if (!strDevVer.empty())
    {
        char szSPVer[64] = { 0 };
        char szVer[64] = { 0 };
        //GetFileVersion(szVer);
        sprintf(szSPVer, "%08sV%07s", "BCR_V310", szVer);
        m_cExtra.AddExtra("VRTCount", "2");
        m_cExtra.AddExtra("VRTDetail[00]", szSPVer);                // SP版本程序名称8位+版本8位
        m_cExtra.AddExtra("VRTDetail[01]", strDevVer.c_str());      // Firmware(1)版本程序名称8位+版本8位
    }*/
}

bool CXFS_CAM::IsDevStatusOK()
{
    return (m_stStatus.fwDevice == WFS_CAM_DEVONLINE) ? true : false;
}

void CXFS_CAM::InitConifig()
{
    LPCSTR ThisModule = "InitConifig";

    WORD wTmp = 0;

    memset(&m_sCamIniConfig, 0x00, sizeof(ST_CAM_INI_CONFIG));

    strcpy(m_sCamIniConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型(0云从双目摄像/1其他,缺省0)
    m_sCamIniConfig.wCamDevType = (WORD)m_cXfsReg.GetValue("Camera_DevType",
                                                           "CameraDevType", (int)CAM_DEV_CLOUDWALK);

    // 打开方式,0表示序号打开/1表示vid pid打开,缺省0
    m_sCamIniConfig.stCamOpenType.wOpenType = (WORD)m_cXfsReg.GetValue("Camera_OpenType", "opentype", (int)CAM_OPEN_DEVIDX);
    // 可见光模组枚举序号,缺省1
    m_sCamIniConfig.stCamOpenType.wVisCamIdx = (WORD)m_cXfsReg.GetValue("Camera_OpenType", "visCamIdx", 1);
    // 红外光模组枚举序号，缺省0
    m_sCamIniConfig.stCamOpenType.wNisCamIdx = (WORD)m_cXfsReg.GetValue("Camera_OpenType", "nisCamIdx", int(0));
    // 可见光模组vid/pid
    strcpy(m_sCamIniConfig.stCamOpenType.szVisVid, m_cXfsReg.GetValue("Camera_OpenType", "vis_vid", "0C45"));
    strcpy(m_sCamIniConfig.stCamOpenType.szVisPid, m_cXfsReg.GetValue("Camera_OpenType", "vis_pid", "C812"));
    // 红外光模组vid/pid
    strcpy(m_sCamIniConfig.stCamOpenType.szNisVid, m_cXfsReg.GetValue("Camera_OpenType", "nis_vid", "0C45"));
    strcpy(m_sCamIniConfig.stCamOpenType.szNisPid, m_cXfsReg.GetValue("Camera_OpenType", "nis_pid", "C811"));


    // 摄像数据共享内存名(32位,缺省CamShowSharedMemData)
    strcpy(m_sCamIniConfig.szCamDataSMemName, m_cXfsReg.GetValue("Camera_SharedMem",
                                                                 "ShowSharedMemName", SHAREDMEMNAME_CAMDATA));
    //摄像数据共享内存Size(缺省10M)
    m_sCamIniConfig.ulCamDataSMemSize = (ULONG)m_cXfsReg.GetValue("Camera_OpenType",
                                                                  "ShowSharedMemSize", (ULONG)SHAREDMEMSIZE_CAMDATA);

    // 模型加载方式, 0:文件加载 1: 内存加载
    m_sCamIniConfig.stCamCwInitParam.nModelMode = m_cXfsReg.GetValue("CWDetector_CFG", "modelMode", 1);
    // 活体类型, 0:不活检 1: 红外活体 2:结构光活体
    m_sCamIniConfig.stCamCwInitParam.nLivenessMode = m_cXfsReg.GetValue("CWDetector_CFG", "livenessMode", 1);
    // 授权类型, 1:芯片授权 2：hasp授权 3:临时授权 4:云从相机绑定授权
    m_sCamIniConfig.stCamCwInitParam.nLicenseType = m_cXfsReg.GetValue("CWDetector_CFG", "licenseType", 1);
    // 算法矩阵文件,可以不写，使用默认
    strcpy(m_sCamIniConfig.stCamCwInitParam.szConfigFile, m_cXfsReg.GetValue("CWDetector_CFG", "configFile", ""));
    // 人脸检测模型,可以不写，使用默认
    strcpy(m_sCamIniConfig.stCamCwInitParam.szFaceDetectFile, m_cXfsReg.GetValue("CWDetector_CFG", "faceDetectFile", ""));
    // 关键点检测模型,可以不写，使用默认
    strcpy(m_sCamIniConfig.stCamCwInitParam.szKeyPointDetectFile, m_cXfsReg.GetValue("CWDetector_CFG", "keyPointDetectFile", ""));
    // 关键点跟踪模型,可以不写，使用默认
    strcpy(m_sCamIniConfig.stCamCwInitParam.szKeyPointTrackFile, m_cXfsReg.GetValue("CWDetector_CFG", "keyPointTrackFile", ""));
    // 人脸质量评估模型,可以不写，使用默认
    strcpy(m_sCamIniConfig.stCamCwInitParam.szFaceQualityFile, m_cXfsReg.GetValue("CWDetector_CFG", "faceQualityFile", ""));
    // 活体模型,可以不写，使用默认
    strcpy(m_sCamIniConfig.stCamCwInitParam.szFaceLivenessFile, m_cXfsReg.GetValue("CWDetector_CFG", "faceLivenessFile", ""));
    // 人脸检测模型,可以不写，使用默认
    strcpy(m_sCamIniConfig.stCamCwInitParam.szFaceKeyPointTrackFile, m_cXfsReg.GetValue("CWDetector_CFG", "faceKeyPointTrackFile", ""));
    // 是否使用GPU(1/使用GPU,-1使用CPU)
    m_sCamIniConfig.stCamCwInitParam.nGpu = m_cXfsReg.GetValue("CWDetector_CFG", "GPU", int(0));
    if (m_sCamIniConfig.stCamCwInitParam.nGpu == 0)
    {
        m_sCamIniConfig.stCamCwInitParam.nGpu = -1;
    }
    // 是否多线程检测
    m_sCamIniConfig.stCamCwInitParam.nMultiThread = m_cXfsReg.GetValue("CWDetector_CFG", "multiThread", 1);

    // 摄像刷新时间(秒,缺省30秒)
    m_sCamIniConfig.unWinRefreshTime = m_cXfsReg.GetValue("Camera_ShowCfg", "WinRefreshTime", int(30));
    if (m_sCamIniConfig.unWinRefreshTime < 0)
    {
        m_sCamIniConfig.unWinRefreshTime = 30;
    }

    // TakePictrue/TakePictrueEx超时(单位:秒,缺省30)
    m_sCamIniConfig.wTakePicTimeOut = m_cXfsReg.GetValue("TakePic", "TakePicTimeOut", int(30));
    if (m_sCamIniConfig.wTakePicTimeOut < 0)
    {
        m_sCamIniConfig.wTakePicTimeOut = 30;
    }

    //双目摄像支持注册文件(缺省/home/projects/wsap/cfg/Camera.ini)
    strcpy(m_sCamIniConfig.szCameraSupFile, m_cXfsReg.GetValue("Camera_Info", "CameraSupFile", SPCAMERASUPFILE));
    if (strlen((char*)m_sCamIniConfig.szCameraSupFile) < 2 ||
        m_sCamIniConfig.szCameraSupFile[0]  != '/') {
        memset(m_sCamIniConfig.szCameraSupFile, 0x00, sizeof(m_sCamIniConfig.szCameraSupFile));
        memcpy(m_sCamIniConfig.szCameraSupFile, SPCAMERASUPFILE, strlen(SPCAMERASUPFILE));
    }

    // 双目安装
    QSettings qReadCameraSupFile(m_sCamIniConfig.szCameraSupFile, QSettings::IniFormat);
    //qReadCfgFile.setIniCodec(QTextCodec::codecForName("utf-8"));

    if (qReadCameraSupFile.contains("CFG/IsInstallCA") != TRUE)
    {
        m_sCamIniConfig.wIsInstallCAM = 3;
    } else
    {
        m_sCamIniConfig.wIsInstallCAM = qReadCameraSupFile.value("CFG/IsInstallCAM", (int)0).toInt();
    }

    // TakePic命令生成格式(1base64/2JPG/4BMP，缺省4)
    m_sCamIniConfig.stCamSaveImageCfg.wTakePicMakeFormatFlag = (WORD)m_cXfsReg.GetValue("Camera_Info",
                                "TakePicMakeFormat", (int)PIC_JPG);

    // TakePic 缺省保存目录
    strcpy(m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath, m_cXfsReg.GetValue("Camera_Info", "TakePicDefSavePath", ""));
    if (strlen((char*)m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath) < 2 ||
        m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath[0]  != '/') {
        memset(m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath, 0x00,
               sizeof(m_sCamIniConfig.stCamSaveImageCfg.byTakePicDefSavePath));
    }

    // 摄像图片信息文件 CameraInfo.ini 全路径
    strcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageInfoFile, m_cXfsReg.GetValue("Camera_Info", "ImageInfoFile", IMAGEINFOFILE));
    if (strlen((char*)m_sCamIniConfig.stCamSaveImageCfg.byImageInfoFile) < 2 ||
        m_sCamIniConfig.stCamSaveImageCfg.byImageInfoFile[0] != '/') {
        memset(m_sCamIniConfig.stCamSaveImageCfg.byImageInfoFile, 0x00,
               sizeof(m_sCamIniConfig.stCamSaveImageCfg.byImageInfoFile));
        memcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageInfoFile, IMAGEINFOFILE, strlen(IMAGEINFOFILE));
    }

    strcpy(m_sCamIniConfig.szCamCheckOpenFile, m_cXfsReg.GetValue("Camera_Info", "CheckOpen", ""));
    if (strlen((char*)m_sCamIniConfig.szCamCheckOpenFile) < 2 ||
        m_sCamIniConfig.szCamCheckOpenFile[0] != '/') {
        memset(m_sCamIniConfig.szCamCheckOpenFile, 0x00,
               sizeof(m_sCamIniConfig.szCamCheckOpenFile));
    }
    // CameraPic备份保存全路径	BASE64
    strcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBASE64, m_cXfsReg.GetValue("Camera_Info", "ImageSavePathBASE64", IMAGESAVEPATH_BASE64));
    if (strlen((char*)m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBASE64) < 2 ||
        m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBASE64[0]  != '/') {
        memset(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBASE64, 0x00,
               sizeof(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBASE64));
        memcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBASE64, IMAGESAVEPATH_BASE64, strlen(IMAGESAVEPATH_BASE64));
    }

    // CameraPic备份保存全路径 JPG
    strcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathJPG, m_cXfsReg.GetValue("Camera_Info", "ImageSavePathJPG", IMAGESAVEPATH_JPG));
    if (strlen((char*)m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathJPG) < 2 ||
        m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathJPG[0] != '/') {
        memset(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathJPG, 0x00,
               sizeof(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathJPG));
        memcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathJPG, IMAGESAVEPATH_JPG, strlen(IMAGESAVEPATH_JPG));
    }

    // CameraPic备份保存全路径	BMP
    strcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBMP, m_cXfsReg.GetValue("Camera_Info", "ImageSavePathBMP", IMAGESAVEPATH_BMP));
    if (strlen((char*)m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBMP) < 2 ||
        m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBMP[0] != '/') {
        memset(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBMP, 0x00,
               sizeof(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBMP));
        memcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBMP, IMAGESAVEPATH_BMP, strlen(IMAGESAVEPATH_BMP));
    }

    // 摄像备份信息文件绝对路径
    strcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageSaveInfoPath, m_cXfsReg.GetValue("Camera_Info", "ImageSaveInfoPath", IMAGESAVEINFOFILE));
    if (strlen((char*)m_sCamIniConfig.stCamSaveImageCfg.byImageSaveInfoPath) < 2 ||
        m_sCamIniConfig.stCamSaveImageCfg.byImageSaveInfoPath[0] != '/') {
        memset(m_sCamIniConfig.stCamSaveImageCfg.byImageSaveInfoPath, 0x00,
               sizeof(m_sCamIniConfig.stCamSaveImageCfg.byImageSaveInfoPath));
        memcpy(m_sCamIniConfig.stCamSaveImageCfg.byImageSaveInfoPath, IMAGESAVEINFOFILE, strlen(IMAGESAVEINFOFILE));
    }

    // 摄像图片备份配置文件全路径
    strcpy(m_sCamIniConfig.stCamSaveImageCfg.bySPCamerasNoCfgFile, m_cXfsReg.GetValue("Camera_Info", "SPCamerasNoCfgFile", SPCAMERASNOCFG));
    if (strlen((char*)m_sCamIniConfig.stCamSaveImageCfg.bySPCamerasNoCfgFile) < 2 ||
        m_sCamIniConfig.stCamSaveImageCfg.bySPCamerasNoCfgFile[0] != '/') {
        memset(m_sCamIniConfig.stCamSaveImageCfg.bySPCamerasNoCfgFile, 0x00,
               sizeof(m_sCamIniConfig.stCamSaveImageCfg.bySPCamerasNoCfgFile));
        memcpy(m_sCamIniConfig.stCamSaveImageCfg.bySPCamerasNoCfgFile, SPCAMERASNOCFG, strlen(SPCAMERASNOCFG));
    }

    QSettings qReadCfgFile(m_sCamIniConfig.stCamSaveImageCfg.bySPCamerasNoCfgFile, QSettings::IniFormat);
    //qReadCfgFile.setIniCodec(QTextCodec::codecForName("utf-8"));

    // 是否启用单天累加文件
    wTmp = qReadCfgFile.value("/CFG/IsNeedDailyRecord", (int)1).toInt();
    if (wTmp == 1) {
        m_sCamIniConfig.stCamSaveImageCfg.bIsNeedDailyRecord = TRUE;
    } else {
        m_sCamIniConfig.stCamSaveImageCfg.bIsNeedDailyRecord = FALSE;
    }

    // CameraPic备份保存天数
    m_sCamIniConfig.stCamSaveImageCfg.wSaveTime =  qReadCfgFile.value("/CFG/SavaTime", int(30)).toInt();

    // CameraPic备份保存过期清理
    vDelImageInfoDir();


    memset(&m_stDevInitParam, 0x00, sizeof(ST_CAM_DEV_INIT_PARAM));
    memcpy(&(m_stDevInitParam.stCamOpenType), &(m_sCamIniConfig.stCamOpenType),
           sizeof(ST_CAM_OPENTYPE));
    memcpy(&(m_stDevInitParam.stCamCwInitParam), &(m_sCamIniConfig.stCamCwInitParam),
           sizeof(ST_CAM_CW_INIT_PARAM));
    memcpy(&(m_stDevInitParam.stCamSaveImageCfg), &(m_sCamIniConfig.stCamSaveImageCfg),
           sizeof(ST_CAM_SAVEIMAGE_CFG));
    m_stDevInitParam.ulCamDataSMemSize = m_sCamIniConfig.ulCamDataSMemSize;
    memcpy(m_stDevInitParam.szCamDataSMemName, m_sCamIniConfig.szCamDataSMemName,
           sizeof(m_sCamIniConfig.szCamDataSMemName));
}

void CXFS_CAM::InitStatus()
{
    memset(&m_stStatus, 0x00, sizeof(WFSCAMSTATUS));
    m_stStatus.fwDevice      = WFS_CAM_DEVNODEVICE;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.fwMedia[i] = WFS_CAM_MEDIAUNKNOWN;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.fwCameras[i] = WFS_CAM_CAMNOTSUPP;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.usPictures[i] = WFS_CAM_ROOM;
    m_stStatus.lpszExtra = NULL;
    m_stStatus.wAntiFraudModule = 0;
    if(m_stStatus.fwDevice == WFS_CAM_DEVNODEVICE)
        m_stStatus.fwDevice = WFS_CAM_DEVHWERROR;
}

// lpspath: 绝对路径； bFolder:是否不包含文件名的目录路径
BOOL CXFS_CAM::bPathCheckAndCreate(LPSTR lpsPath, BOOL bFolder)
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

HRESULT CXFS_CAM::hErrCodeChg(LONG lDevCode)
{
    switch(lDevCode)
    {
        case CAM_SUCCESS:       // (0)
            return WFS_SUCCESS;                 // 0
        case ERR_CONFIG:        // (-1)
        case ERR_OPEN_CAMER:    // (-2)
            return WFS_ERR_HARDWARE_ERROR;      // -14
        case ERR_INVALID_PARAM: // (-3)
            return WFS_ERR_INVALID_DATA;        // -52
        case ERR_LOAD_IMAGE:    // (-4)
            return WFS_ERR_HARDWARE_ERROR;      // -14
        case ERR_FRAME:         // (-5)
            return WFS_ERR_HARDWARE_ERROR;      // -14
        case ERR_IMWRITE:       // (-6)
            return WFS_ERR_HARDWARE_ERROR;      // -14
        case ERR_SAVE_IMAGE:    // (-7)
            return WFS_ERR_HARDWARE_ERROR;      // -14
        case ERR_OTHER:         // (-8)
            return WFS_ERR_HARDWARE_ERROR;      // -14
        case ERR_TIMEOUT:       // (-9)
            return WFS_ERR_TIMEOUT;             // -48
        case ERR_SHAREDMEM:     // (-10)
            return WFS_ERR_INTERNAL_ERROR;      // -15
        case ERR_PROCESS:       // (-11)
            return WFS_ERR_INTERNAL_ERROR;      // -15
        default:
            return WFS_ERR_HARDWARE_ERROR;      // -14
    }
}


/*****************************************************************************
** 清除指定日期以前的Camera Image 备份保存目录
*/
void CXFS_CAM::vDelImageInfoDir()
{
    time_t		time_tmp = time(NULL);
    struct tm*	tm_now = localtime(&time_tmp);
    tm_now->tm_mday = tm_now->tm_mday - 2;
    time_tmp = mktime(tm_now);
    tm_now = localtime(&time_tmp);

    //QString		cDelDate, cTemp, cFileName;
    CHAR    cDelDate[MAX_PATH] = { 0x00 };
    CHAR    cTemp[MAX_PATH] = { 0x00 };
    CHAR    cFileName[MAX_PATH] = { 0x00 };
    CHAR    cTmp[128];

    sprintf(cDelDate, "%04d%02d%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
    for(int i = 0; i < 3; i ++) {
        if (i == 0)
            sprintf(cTemp, "%s", m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBASE64);
        if (i == 1)
            sprintf(cTemp, "%s", m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathJPG);
        if (i == 2)
            sprintf(cTemp, "%s", m_sCamIniConfig.stCamSaveImageCfg.byImageSavePathBMP);

        QFileInfo qf(cTemp);
        if (qf.isFile())
        {
            continue;
        }

        if (qf.isDir())
        {
            QDir dir(cTemp);
            if (dir.exists() != TRUE)
            {
                return;
            }

            dir.setFilter(QDir::Dirs | QDir::Files);// 只遍历文件和目录

            QFileInfoList list = dir.entryInfoList();
            QStringList string_list;

            if (list.size() <= 0)
            {
                continue;
            } else
            {
                for (int i = 0; i < list.size(); ++i)
                {
                    QFileInfo fileInfo = list.at(i);

                    QString filename = fileInfo.fileName();

                    if ((QString::compare(filename, QString("."), Qt::CaseInsensitive) == 0) ||
                        (QString::compare(filename, QString(".."), Qt::CaseInsensitive) == 0) ||
                        (QString::compare(filename, QString(cDelDate),Qt::CaseInsensitive) > 0))//通过QFileInfo来实现对文件类型的过滤
                    {
                        continue;
                    }

                    filename.prepend("/");
                    filename.prepend(cTemp);

                    if (fileInfo.isFile())
                    {
                        QFile::remove(filename);
                    }

                    if (fileInfo.isDir())
                    {
                        vDeleteDirectory((LPSTR)filename.toStdString().c_str());
                    }
                }
            }
        }
    }
}


/*****************************************************************************
** 清除指定目录下文件并删除指定目录
*/
void CXFS_CAM::vDeleteDirectory(LPSTR lpDirName)
{
    QDir dir(lpDirName);
    if (dir.exists() != TRUE)
    {
        return;
    }

    dir.setFilter(QDir::Dirs | QDir::Files);// 只遍历文件和目录

    QFileInfoList list = dir.entryInfoList();
    QStringList string_list;

    if (list.size() <= 0)
    {
        dir.rmdir(lpDirName);
    } else
    {
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);

            QString filename = fileInfo.fileName();

            if ((QString::compare(filename, QString("."), Qt::CaseInsensitive) == 0) ||
                (QString::compare(filename, QString(".."), Qt::CaseInsensitive) == 0))
            {
                continue;
            }

            filename.prepend("/");
            filename.prepend(lpDirName);

            if (fileInfo.isFile())
            {
                QFile::remove(filename);
            }

            if (fileInfo.isDir())
            {
                vDeleteDirectory((LPSTR)filename.toStdString().c_str());
            }
        }
        dir.rmdir(lpDirName);
    }
}

void CXFS_CAM::vSlotShowWin(CAM_SHOWWIN_INFO stCamShowInfo)
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

    char cSharedMemName_Data[25] = { 0x00 };

    if ((nRet = showWin->nConnSharedMem(m_sCamIniConfig.szCamDataSMemName)) > 0)
    {
        showWin->Release();
        return;
    }

    showWin->ShowWin(stCamShowInfo.wX, stCamShowInfo.wY,
                     stCamShowInfo.wWidth, stCamShowInfo.wHeight);

    showWin->vTimerStart(m_sCamIniConfig.unWinRefreshTime);

    //showWin->show();
}

void CXFS_CAM::vSlotHideWin()
{
    if (showWin != nullptr)
    {
        showWin->hide();
        delete showWin;
        showWin = nullptr;
    }
}

BOOL CXFS_CAM::bCreateSharedMemory(LPSTR lpMemName, ULONG ulSize)
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

    if (m_qSharedMemData->create(1024 * 1024 * 14, QSharedMemory::ReadWrite) != true)
    {
        if (m_qSharedMemData->error() == QSharedMemory::AlreadyExists)
        {
            Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s)共享内存已存在,不继续创建. ", lpMemName);
            m_qSharedMemData->attach();
            if (m_qSharedMemData->isAttached() != true)
            {
                Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存挂载失败, ReturnCode:%d|%s. ",
                    lpMemName, ulSize, m_qSharedMemData->error(), m_qSharedMemData->errorString().toUtf8().data());
                return FALSE;
            }
        } else
        {
            Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存创建失败, ReturnCode:%d|%s. ",
                lpMemName, ulSize, m_qSharedMemData->error(), m_qSharedMemData->errorString().toUtf8().data());
            return FALSE;
        }
    }

    Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存创建完成. ",  lpMemName, ulSize);


    return TRUE;
}

void CXFS_CAM::SharedMemRelease()
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

//////////////////////////////////////////////////////////////////////////


