/***************************************************************
* 文件名称：XFS_CAM_DEC.cpp
* 文件描述：摄像头模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_CAM.h"


//-------------------------------------------------------------------------
// 新增Open处理,初始Open和断线重连Open	// 30-00-00-00(FT#0031)
HRESULT CXFS_CAM::StartOpen()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    m_wVRTCount = 2;

    m_cExtra.AddExtra("VRTCount", "2");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", (char*)byDevVRTU);

    // 打开连接
    if ((m_wDeviceModeSup & CAM_MODE_PERSON) == CAM_MODE_PERSON &&  // 支持人脸摄像
        (m_wIsOpenOk & CAM_MODE_PERSON) != CAM_MODE_PERSON)         // 人脸摄像未OpenOk
    {
        hRet = m_pDev->Open(nullptr);
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__, "打开[人脸摄像]设备连接失败．ReturnCode:%d", hRet);
            //return WFS_ERR_HARDWARE_ERROR;
            if (m_wDeviceModeSup == CAM_MODE_PERSON)    // 只支持人脸摄像时返回
            {
                return WFS_SUCCESS; //hErrCodeChg(hRet);
            }
        } else
        {
            // 更新扩展状态
            CHAR szDevCAMVer[MAX_PATH] = { 0x00 };
            m_pDev->GetVersion(szDevCAMVer, sizeof(szDevCAMVer) - 1, 1);

            CHAR szFWVer[MAX_PATH] = { 0x00 };
            m_pDev->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

            CHAR szDevOtherVer[MAX_PATH] = { 0x00 };
            m_pDev->GetVersion(szDevOtherVer, sizeof(szDevOtherVer) - 1, 3);

            //m_cExtra.AddExtra("VRTDetail[02]", szFWVer);
            //m_cExtra.AddExtra("VRTDetail[03]", szDevOtherVer);

            m_wIsOpenOk += CAM_MODE_PERSON;
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_ROOM) == CAM_MODE_ROOM &&  // 支持全景摄像
        (m_wIsOpenOk & CAM_MODE_ROOM) != CAM_MODE_ROOM)         // 全景摄像未OpenOk
    {
        if (m_wDeviceRoomType != m_wDeviceType)
        {
            hRet = m_pDevRoom->Open(nullptr);
            if (hRet != 0)
            {
                Log(ThisModule, __LINE__, "打开[全景摄像]设备连接失败．ReturnCode:%d", hRet);
                //return WFS_ERR_HARDWARE_ERROR;
                return WFS_SUCCESS; //hErrCodeChg(hRet);
            }

            // 更新扩展状态
            CHAR szDevCAMVer[MAX_PATH] = { 0x00 };
            m_pDevRoom->GetVersion(szDevCAMVer, sizeof(szDevCAMVer) - 1, 1);

            CHAR szFWVer[MAX_PATH] = { 0x00 };
            m_pDevRoom->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

            CHAR szDevOtherVer[MAX_PATH] = { 0x00 };
            m_pDevRoom->GetVersion(szDevOtherVer, sizeof(szDevOtherVer) - 1, 3);

            //m_cExtra.AddExtra("VRTDetail[02]", szFWVer);
            //m_cExtra.AddExtra("VRTDetail[03]", szDevOtherVer);

            m_wIsOpenOk += CAM_MODE_ROOM;
        } else
        {
            if ((m_wIsOpenOk & CAM_MODE_PERSON) == CAM_MODE_PERSON)
            {
                m_wIsOpenOk += CAM_MODE_ROOM;
            }
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_HIGH) == CAM_MODE_HIGH &&  // 支持高拍摄像
        (m_wIsOpenOk & CAM_MODE_HIGH) != CAM_MODE_HIGH)         // 高拍摄像未OpenOk
    {
        // 同一设备只Open一个DevCAM实例
        if (m_wDeviceHighType != m_wDeviceType &&
            m_wDeviceHighType != m_wDeviceRoomType)
        {
            hRet = m_pDevHigh->Open(nullptr);
            if (hRet != 0)
            {
                Log(ThisModule, __LINE__, "打开[高拍摄像]设备连接失败．ReturnCode:%d", hRet);
                //return WFS_ERR_HARDWARE_ERROR;
                return WFS_SUCCESS; //hErrCodeChg(hRet);
            }

            // 更新扩展状态
            CHAR szDevCAMVer[MAX_PATH] = { 0x00 };
            m_pDevHigh->GetVersion(szDevCAMVer, sizeof(szDevCAMVer) - 1, 1);

            CHAR szFWVer[MAX_PATH] = { 0x00 };
            m_pDevHigh->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

            CHAR szDevOtherVer[MAX_PATH] = { 0x00 };
            m_pDevHigh->GetVersion(szDevOtherVer, sizeof(szDevOtherVer) - 1, 3);

            //m_cExtra.AddExtra("VRTDetail[02]", szFWVer);
            //m_cExtra.AddExtra("VRTDetail[03]", szDevOtherVer);

            m_wIsOpenOk += CAM_MODE_HIGH;
        } else
        {
            if (m_wDeviceHighType == m_wDeviceType && (m_wIsOpenOk & CAM_MODE_PERSON) == CAM_MODE_PERSON)
            {
                m_wIsOpenOk += CAM_MODE_HIGH;
            } else
            if (m_wDeviceHighType == m_wDeviceRoomType && (m_wIsOpenOk & CAM_MODE_ROOM) == CAM_MODE_ROOM)
            {
                m_wIsOpenOk += CAM_MODE_HIGH;
            }
        }
    }

    //m_bIsOpenOk = TRUE;

    // 更新一次状态
    OnStatus();

    Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    return WFS_SUCCESS;
}

bool CXFS_CAM::UpdateStatus(const DEVCAMSTATUS &stStatus, const DEVCAMSTATUS &stStatusRoom,
                            const DEVCAMSTATUS &stStatusHigh)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();		// 30-00-00-00(FT#0031)
    AutoMutex(*m_pMutexGetStatus);

    WORD fwDevice = WFS_CAM_DEVONLINE;

    memcpy(&m_stStatusOld, &m_stStatus, sizeof(WFSCAMSTATUS));

    // 设备状态
    //UpdateExtra(stStatus.szErrCode);
    if ((m_wDeviceModeSup & CAM_MODE_PERSON) == CAM_MODE_PERSON)    // 人脸摄像
    {
        switch (stStatus.fwDevice)
        {
            case DEVICE_OFFLINE:
                fwDevice = WFS_CAM_DEVOFFLINE;
                break;
            case DEVICE_ONLINE:
            case DEVICE_BUSY:					// 30-00-00-00(FT#0031)
                fwDevice = WFS_CAM_DEVONLINE;
                break;
            case DEVICE_NODEVICE:				// 30-00-00-00(FT#0031)
                fwDevice = WFS_CAM_DEVNODEVICE;	// 30-00-00-00(FT#0031)
                break;							// 30-00-00-00(FT#0031)
            default:
                fwDevice = WFS_CAM_DEVHWERROR;
                break;
        }

        switch(stStatus.fwMedia[WFS_CAM_PERSON])
        {
            case MEDIA_OK     : m_stStatus.fwMedia[WFS_CAM_PERSON] = WFS_CAM_MEDIAOK; break;
            case MEDIA_HIGH   : m_stStatus.fwMedia[WFS_CAM_PERSON] = WFS_CAM_MEDIAHIGH; break;
            case MEDIA_FULL   : m_stStatus.fwMedia[WFS_CAM_PERSON] = WFS_CAM_MEDIAFULL; break;
            case MEDIA_UNKNOWN: m_stStatus.fwMedia[WFS_CAM_PERSON] = WFS_CAM_MEDIAUNKNOWN; break;
            case MEDIA_NOTSUPP: m_stStatus.fwMedia[WFS_CAM_PERSON] = WFS_CAM_MEDIANOTSUPP; break;
        }
        switch(stStatus.fwCameras[WFS_CAM_PERSON])
        {
            case STATUS_NOTSUPP: m_stStatus.fwCameras[WFS_CAM_PERSON] = WFS_CAM_CAMNOTSUPP; break;
            case STATUS_OK     : m_stStatus.fwCameras[WFS_CAM_PERSON] = WFS_CAM_CAMOK; break;
            case STATUS_INOP   : m_stStatus.fwCameras[WFS_CAM_PERSON] = WFS_CAM_CAMINOP; break;
            case STATUS_UNKNOWN: m_stStatus.fwCameras[WFS_CAM_PERSON] = WFS_CAM_CAMNOTSUPP; break;
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_ROOM) == CAM_MODE_ROOM)    // 全景摄像
    {
        if (fwDevice == WFS_CAM_DEVONLINE)
        {
            switch (stStatusRoom.fwDevice)
            {
                case DEVICE_OFFLINE:
                    fwDevice = WFS_CAM_DEVOFFLINE;
                    break;
                case DEVICE_ONLINE:
                case DEVICE_BUSY:
                    break;
                case DEVICE_NODEVICE:
                    fwDevice = WFS_CAM_DEVNODEVICE;
                    break;
                default:
                    fwDevice = WFS_CAM_DEVHWERROR;
                    break;
            }
        }

        switch(stStatusRoom.fwMedia[WFS_CAM_PERSON])
        {
            case MEDIA_OK     : m_stStatus.fwMedia[WFS_CAM_ROOM] = WFS_CAM_MEDIAOK; break;
            case MEDIA_HIGH   : m_stStatus.fwMedia[WFS_CAM_ROOM] = WFS_CAM_MEDIAHIGH; break;
            case MEDIA_FULL   : m_stStatus.fwMedia[WFS_CAM_ROOM] = WFS_CAM_MEDIAFULL; break;
            case MEDIA_UNKNOWN: m_stStatus.fwMedia[WFS_CAM_ROOM] = WFS_CAM_MEDIAUNKNOWN; break;
            case MEDIA_NOTSUPP: m_stStatus.fwMedia[WFS_CAM_ROOM] = WFS_CAM_MEDIANOTSUPP; break;
        }
        switch(stStatusRoom.fwCameras[WFS_CAM_PERSON])
        {
            case STATUS_NOTSUPP: m_stStatus.fwCameras[WFS_CAM_ROOM] = WFS_CAM_CAMNOTSUPP; break;
            case STATUS_OK     : m_stStatus.fwCameras[WFS_CAM_ROOM] = WFS_CAM_CAMOK; break;
            case STATUS_INOP   : m_stStatus.fwCameras[WFS_CAM_ROOM] = WFS_CAM_CAMINOP; break;
            case STATUS_UNKNOWN: m_stStatus.fwCameras[WFS_CAM_ROOM] = WFS_CAM_CAMNOTSUPP; break;
        }
    }

    if ((m_wDeviceModeSup & CAM_MODE_HIGH) == CAM_MODE_HIGH)    // 高拍摄像
    {
        if (fwDevice == WFS_CAM_DEVONLINE)
        {
            switch (stStatusHigh.fwDevice)
            {
                case DEVICE_OFFLINE:
                    fwDevice = WFS_CAM_DEVOFFLINE;
                    break;
                case DEVICE_ONLINE:
                case DEVICE_BUSY:
                    break;
                case DEVICE_NODEVICE:
                    fwDevice = WFS_CAM_DEVNODEVICE;
                    break;
                default:
                    fwDevice = WFS_CAM_DEVHWERROR;
                    break;
            }

            switch(stStatusHigh.fwMedia[WFS_CAM_PERSON])
            {
                case MEDIA_OK     : m_stStatus.fwMedia[WFS_CAM_HIGHTCAMERA] = WFS_CAM_MEDIAOK; break;
                case MEDIA_HIGH   : m_stStatus.fwMedia[WFS_CAM_HIGHTCAMERA] = WFS_CAM_MEDIAHIGH; break;
                case MEDIA_FULL   : m_stStatus.fwMedia[WFS_CAM_HIGHTCAMERA] = WFS_CAM_MEDIAFULL; break;
                case MEDIA_UNKNOWN: m_stStatus.fwMedia[WFS_CAM_HIGHTCAMERA] = WFS_CAM_MEDIAUNKNOWN; break;
                case MEDIA_NOTSUPP: m_stStatus.fwMedia[WFS_CAM_HIGHTCAMERA] = WFS_CAM_MEDIANOTSUPP; break;
            }
            switch(stStatusHigh.fwCameras[WFS_CAM_PERSON])
            {
                case STATUS_NOTSUPP: m_stStatus.fwCameras[WFS_CAM_HIGHTCAMERA] = WFS_CAM_CAMNOTSUPP; break;
                case STATUS_OK     : m_stStatus.fwCameras[WFS_CAM_HIGHTCAMERA] = WFS_CAM_CAMOK; break;
                case STATUS_INOP   : m_stStatus.fwCameras[WFS_CAM_HIGHTCAMERA] = WFS_CAM_CAMINOP; break;
                case STATUS_UNKNOWN: m_stStatus.fwCameras[WFS_CAM_HIGHTCAMERA] = WFS_CAM_CAMNOTSUPP; break;
            }
        }
    }

    m_stStatus.fwDevice = fwDevice;
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();// 扩展状态

    // 判断状态是否有变化
    if (m_stStatusOld.fwDevice != m_stStatus.fwDevice)		// 30-00-00-00(FT#0031)
    {
        m_pBase->FireStatusChanged(fwDevice);
        // 故障时，也要上报故障事件
        if (fwDevice == WFS_CAM_DEVHWERROR)
        {										// 30-00-00-00(FT#0031)
            m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_cExtra.GetErrDetail("ErrorDetail"));
        }										// 30-00-00-00(FT#0031)
    }

    return true;
}

// 摄像处理
HRESULT CXFS_CAM::InnerTakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = CAM_SUCCESS;
    WORD    wImageType;     // 生成图像格式

    // 检查Display是否执行
    if (m_wDisplayOK == 0)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败: 命令序列错误, Display未执行．");
        return WFS_ERR_DEV_NOT_READY;
    }

    LPWFSCAMTAKEPICTEX	lpCmdData = NULL;
    lpCmdData = (LPWFSCAMTAKEPICTEX)&stTakePict;

    // 检查入参模式wCamera
    if (lpCmdData->wCamera == WFS_CAM_ROOM)             // 全景模式
    {
        if (m_wDisplayOK != CAM_MODE_ROOM)
        {
            Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败: 全景摄像设备未Display．");
            return WFS_ERR_DEV_NOT_READY;
        }
    } else
    if (lpCmdData->wCamera == WFS_CAM_PERSON)           // 人脸模式
    {
        if (m_wDisplayOK != CAM_MODE_PERSON)
        {
            Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败: 人脸摄像设备未Display．");
            return WFS_ERR_DEV_NOT_READY;
        }
    } else
    if (lpCmdData->wCamera == WFS_CAM_HIGHTCAMERA)     // 高拍模式
    {
        if (m_wDisplayOK != CAM_MODE_HIGH)
        {
            Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败: 高拍摄像设备未Display．");
            return WFS_ERR_DEV_NOT_READY;
        }
    } else
    if (lpCmdData->wCamera == WFS_CAM_FILE)             // 资料补拍
    {
        ;   // 无检查
    } else
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败: 入参wCamera[%d]!=%d/%d无效．ReturnCode:%d",
            lpCmdData->wCamera, WFS_CAM_ROOM, WFS_CAM_PERSON, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }


    memset(szFileNameFromAp, 0x00, sizeof(szFileNameFromAp));

    // 检查参数: 摄像文件名: NULL
    if (lpCmdData->lpszPictureFile == NULL)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[NULL]无效．");
        return WFS_ERR_INVALID_DATA;
    }

    // 检查参数: 摄像文件名: Len > 0,取值; Len <=0 && INI设置缺省, 使用INI设置; 其他报错
    if (strlen(lpCmdData->lpszPictureFile) > 0)
    {
        sprintf(szFileNameFromAp, "%s", lpCmdData->lpszPictureFile);
    } else
    if (strlen(m_sCamIniConfig.szTakePicDefSavePath) > 0)
    {
        sprintf(szFileNameFromAp, "%s", m_sCamIniConfig.szTakePicDefSavePath);
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

    // 检查摄像文件名+文件格式
    std::string szFileName;
    std::string szFilePath = szFileNameFromAp;		// FilePath and FileName
    int iIndex = szFilePath.rfind('.');
    if (iIndex == -1)   // 摄像文件名无效
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
        wImageType = (wImageType | m_sCamIniConfig.wTakePicMakeFormatFlag);
    }

    // 目录验证
    if (bPathCheckAndCreate(szFileNameFromAp, FALSE) != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]创建目录结构失败．", szFileNameFromAp);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 超时设置(INI设置0时取入参,否则用INI设置超时值)
    DWORD dwTimeOutSize = (m_sCamIniConfig.wTakePicTimeOut == 0 ? dwTimeout : m_sCamIniConfig.wTakePicTimeOut);	// 30-00-00-00(FT#0031)

    // 摄像模式Check
    WORD wCamera = TAKEPIC_PERSON;  // 缺省人脸

    // 指定银行特殊处理
    BOOL bBankSet = FALSE;  // 采用银行特殊完成标记(用于银行处理配置不完整则忽略的情况)
    if (m_sCamIniConfig.wBank == BANK_BCS)  // 长沙银行+云从摄像
    {
        if (m_wDeviceType == CAM_DEV_CLOUDWALK)
        {
            if (strlen(m_sCamIniConfig.stCamMethodBCS.szMT1_PerSonName) > 0 && lpCmdData->wCamera != WFS_CAM_FILE)
            {
                std::string szFileNew = szFilePath.substr(0, szFilePath.rfind('/') + 1);
                szFileNew.append(m_sCamIniConfig.stCamMethodBCS.szMT1_PerSonName);
                m_pDev->SetData((char*)szFileNew.c_str(), DATATYPE_PERSON);
                wCamera = TAKEPIC_ROOM; // 全景
                bBankSet = TRUE;
            }
        }
    }

    if (bBankSet != TRUE)   // 银行处理配置不完整则忽略
    {
        switch(lpCmdData->wCamera)
        {
            case WFS_CAM_ROOM:      // XFS全景
                wCamera = TAKEPIC_ROOM;
                break;
            case WFS_CAM_PERSON:    // XFS人脸
                wCamera = TAKEPIC_PERSON;
                break;
            case WFS_CAM_HIGHTCAMERA:// XFS高拍
                wCamera = TAKEPIC_HIGH;
            break;
            case WFS_CAM_FILE:      // 资料补拍
                wCamera = TAKEPIC_FILE;
                break;

        }
    }

    // 支持上报 活体图像检测错误事件(支持人脸摄像时有效)
    if (m_sCamIniConfig.wEventLiveErrorSup == 1)
    {
        if ((m_wDeviceModeSup & CAM_MODE_PERSON) == CAM_MODE_PERSON)  // 支持人脸摄像
        {
            // 建立线程，循环检查活体图像结果
            m_bLiveEventWaitExit = FALSE; // 指示线程结束标记:F
            if (!m_thRunLiveEventWait.joinable())
            {
                m_thRunLiveEventWait = std::thread(&CXFS_CAM::ThreadLiveEvent_Wait, this);
                if (m_thRunLiveEventWait.joinable())
                {
                    m_thRunLiveEventWait.detach();
                }
            }
        }
    }

    // 命令下发
    hRet = CameraMode_TakePictureEx(lpCmdData->wCamera, szFileNameFromAp, lpCmdData->lpszCamData,
                                    wImageType, TRUE, dwTimeOutSize);				// 30-00-00-00(FT#0031)
    m_bLiveEventWaitExit = TRUE;    // 命令返回则立即结束活体图像结果检查线程
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "摄像命令(TakePictureEx(%s, %s, %d, %d, %d))下发失败．ReturnCode:%d",
                                    szFileNameFromAp, lpCmdData->lpszCamData,
                                    wImageType, TRUE, dwTimeOutSize, hRet);		// 30-00-00-00(FT#0031)
        //return WFS_ERR_HARDWARE_ERROR;
        return hErrCodeChg(hRet);
    }

    // TakePicture命令完成后处理
    InnerTakePicAfterMothod();

    return WFS_SUCCESS;
}

// 窗口处理
HRESULT CXFS_CAM::InnerDisplay(const WFSCAMDISP &stDisplay, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = CAM_SUCCESS;

    LPWFSCAMDISP lpDisplay = NULL;
    lpDisplay = (LPWFSCAMDISP)&stDisplay;

    BOOL bIsRunWindows = FALSE;     // 是否启用SP数据流窗口方式(云从+哲林适用)


    // 摄像模式检查
    if (lpDisplay->wCamera == WFS_CAM_ROOM)     // 全景模式
    {
        if ((m_wDeviceModeSup & CAM_MODE_ROOM) != CAM_MODE_ROOM)    // 全景模式不支持
        {
            Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 入参wCamera[%d]无效,全景/环境模式不支持．ReturnCode:%d",
                lpDisplay->wCamera, WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }
        if (m_wDeviceRoomType == CAM_DEV_CLOUDWALK || m_wDeviceRoomType == CAM_DEV_ZLF1000A3)
        {
            bIsRunWindows = TRUE;
        }
    } else
    if (lpDisplay->wCamera == WFS_CAM_PERSON)   // 人脸模式
    {
        if ((m_wDeviceModeSup & CAM_MODE_PERSON) != CAM_MODE_PERSON) // 人脸模式不支持
        {
            Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 入参wCamera[%d]无效,人脸模式不支持．ReturnCode:%d",
                lpDisplay->wCamera, WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }
        if (m_wDeviceType == CAM_DEV_CLOUDWALK || m_wDeviceType == CAM_DEV_ZLF1000A3)
        {
            bIsRunWindows = TRUE;
        }
    } else
    if (lpDisplay->wCamera == WFS_CAM_HIGHTCAMERA)   // 高拍模式
    {
        if ((m_wDeviceModeSup & CAM_MODE_HIGH) != CAM_MODE_HIGH)    // 高拍模式不支持
        {
            Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 入参wCamera[%d]无效,高拍模式不支持．ReturnCode:%d",
                lpDisplay->wCamera, WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }
        if (m_wDeviceHighType == CAM_DEV_CLOUDWALK || m_wDeviceHighType == CAM_DEV_ZLF1000A3)
        {
            bIsRunWindows = TRUE;
        }
    } else
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 入参wCamera[%d]!=%d/%d无效．ReturnCode:%d",
            lpDisplay->wCamera, WFS_CAM_ROOM, WFS_CAM_PERSON, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    // 检查Action参数(只支持开启/销毁/暂停/恢复)
    if(lpDisplay->wAction != WFS_CAM_CREATE &&      // 开启窗口
       lpDisplay->wAction != WFS_CAM_DESTROY &&     // 销毁窗口
       lpDisplay->wAction != WFS_CAM_PAUSE &&       // 暂停窗口摄像
       lpDisplay->wAction != WFS_CAM_RESUME)        // 恢复窗口摄像
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 入参wAction[%d]无效．ReturnCode:%d",
            lpDisplay->wAction, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    // 摄像是否Open检查(同时间只允许打开一类设备)
    if(lpDisplay->wAction == WFS_CAM_CREATE)        // 开启窗口
    {
        if (lpDisplay->wCamera == WFS_CAM_ROOM)     // 全景窗口
        {
            if ((m_wDisplayOK & CAM_MODE_PERSON) == CAM_MODE_PERSON ||
                (m_wDisplayOK & CAM_MODE_HIGH) == CAM_MODE_HIGH)        // 人脸|高拍窗口已开启
            {
                Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 全景: 人脸|高拍窗口已开启．ReturnCode:%d",
                    WFS_ERR_DEV_NOT_READY);
                return WFS_ERR_DEV_NOT_READY;
            } else
            if ((m_wDisplayOK & CAM_MODE_ROOM) == CAM_MODE_ROOM)        // 全景窗口已开启
            {
                Log(ThisModule, __LINE__, "创建摄像窗口(Display): 全景窗口已开启,不重复开启．ReturnCode:%d",
                    WFS_SUCCESS);
                return WFS_SUCCESS;
            }
        } else
        if (lpDisplay->wCamera == WFS_CAM_PERSON)   // 人脸窗口
        {
            if ((m_wDisplayOK & CAM_MODE_ROOM) == CAM_MODE_ROOM ||
                (m_wDisplayOK & CAM_MODE_HIGH) == CAM_MODE_HIGH)        // 全景|高拍窗口已开启
            {
                Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 人脸: 全景|高拍窗口已开启．ReturnCode:%d",
                    WFS_ERR_DEV_NOT_READY);
                return WFS_ERR_DEV_NOT_READY;
            } else
            if ((m_wDisplayOK & CAM_MODE_PERSON) == CAM_MODE_PERSON)    // 人脸窗口已开启
            {
                Log(ThisModule, __LINE__, "创建摄像窗口(Display): 人脸窗口已开启,不重复开启．ReturnCode:%d",
                    WFS_SUCCESS);
                return WFS_SUCCESS;
            }
        } else
        if (lpDisplay->wCamera == WFS_CAM_HIGHTCAMERA)   // 高拍窗口
        {
            if ((m_wDisplayOK & CAM_MODE_ROOM) == CAM_MODE_ROOM ||
                (m_wDisplayOK & CAM_MODE_PERSON) == CAM_MODE_PERSON)    // 全景|人脸窗口已开启
            {
                Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 高拍: 全景|人脸窗口已开启．ReturnCode:%d",
                    WFS_ERR_DEV_NOT_READY);
                return WFS_ERR_DEV_NOT_READY;
            } else
            if ((m_wDisplayOK & CAM_MODE_HIGH) == CAM_MODE_HIGH)        // 高拍窗口已开启
            {
                Log(ThisModule, __LINE__, "创建摄像窗口(Display): 高拍窗口已开启,不重复开启．ReturnCode:%d",
                    WFS_SUCCESS);
                return WFS_SUCCESS;
            }
        }
    }

    // 销毁/暂停/恢复时检查窗口是否Open
    if (lpDisplay->wAction == WFS_CAM_DESTROY ||     // 销毁窗口
        lpDisplay->wAction == WFS_CAM_PAUSE ||       // 暂停窗口摄像
        lpDisplay->wAction == WFS_CAM_RESUME)        // 恢复窗口摄像
    {
        if (lpDisplay->wCamera == WFS_CAM_ROOM)     // 全景窗口
        {
            if ((m_wDisplayOK & CAM_MODE_ROOM) != CAM_MODE_ROOM)    // 未开启
            {
                Log(ThisModule, __LINE__, "暂停/恢复摄像窗口(Display): 全景窗口未开启．ReturnCode:%d",
                    WFS_SUCCESS);
                return WFS_SUCCESS;
            }
        } else
        if (lpDisplay->wCamera == WFS_CAM_PERSON)   // 人脸窗口
        {
            if ((m_wDisplayOK & CAM_MODE_PERSON) != CAM_MODE_PERSON)    // 未开启
            {
                Log(ThisModule, __LINE__, "暂停/恢复摄像窗口(Display): 人脸窗口未开启．ReturnCode:%d",
                    WFS_SUCCESS);
                return WFS_SUCCESS;
            }
        } else
        if (lpDisplay->wCamera == WFS_CAM_HIGHTCAMERA)   // 高拍窗口
        {
            if ((m_wDisplayOK & CAM_MODE_HIGH) != CAM_MODE_HIGH)    // 未开启
            {
                Log(ThisModule, __LINE__, "暂停/恢复摄像窗口(Display): 高拍窗口未开启．ReturnCode:%d",
                    WFS_SUCCESS);
                return WFS_SUCCESS;
            }
        }
    }

    // 创建窗口检查宽高,不检查窗口ID
    if(lpDisplay->wAction == WFS_CAM_CREATE && (lpDisplay->wHeight == 0 || lpDisplay->wWidth == 0))
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败: 入参wHeight[%d]/wWidth[%d]无效．ReturnCode:%d",
            lpDisplay->wHeight, lpDisplay->wWidth, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    // 长沙银行窗口处理(特殊处理)
    if (m_sCamIniConfig.wBank == BANK_BCS)
    {
        if (lpDisplay->wCamera == WFS_CAM_FILE)  // 云从摄像+资料补拍
        {
            hRet = CameraMode_Display(lpDisplay->wCamera,lpDisplay->hWnd, lpDisplay->wAction,
                                      lpDisplay->wX, lpDisplay->wY,
                                      m_sCamIniConfig.stCamCwInitParam.shResoWidth,
                                      m_sCamIniConfig.stCamCwInitParam.shResoHeight, 0,0);

        }
    } else  // 通用窗口处理
    {
        hRet = CameraMode_Display(lpDisplay->wCamera,
                                  lpDisplay->hWnd,                         // 传入窗口ID
                                  lpDisplay->wAction,                      // 窗口控制项
                                  lpDisplay->wX, lpDisplay->wY,            // 窗口左上角坐标
                                  lpDisplay->wWidth, lpDisplay->wHeight,0,0);  // 窗口宽高
    }
    if (hRet != CAM_SUCCESS)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败．ReturnCode:%d", hRet);
        return hErrCodeChg(hRet);
    }

    // 开启/销毁窗口处理(特殊处理)
    if (lpDisplay->wAction == WFS_CAM_CREATE)
    {
        if (bIsRunWindows == TRUE)     // 启用SP数据流窗口方式(云从+哲林适用)
        {
            CAM_SHOWWIN_INFO stCamInfo;
            stCamInfo.hWnd = lpDisplay->hWnd;
            stCamInfo.wX = lpDisplay->wX;
            stCamInfo.wY = lpDisplay->wY;
            stCamInfo.wWidth = lpDisplay->wWidth;
            stCamInfo.wHeight = lpDisplay->wHeight;
            emit vSignShowWin(stCamInfo);
        }

        //bDisplyOK = TRUE;   // 窗口开启标记:T
        m_wDisplayOK = (lpDisplay->wCamera == WFS_CAM_ROOM ? CAM_MODE_ROOM :
                        (lpDisplay->wCamera == WFS_CAM_PERSON ? CAM_MODE_PERSON: CAM_MODE_HIGH));
    } else
    if (lpDisplay->wAction == WFS_CAM_DESTROY)
    {
        if (bIsRunWindows == TRUE)     // 启用SP数据流窗口方式(云从+哲林适用)
        {
            emit vSignHideWin();
        }

        //bDisplyOK = FALSE;   // 窗口开启标记:F
        m_wDisplayOK = 0;
    }

    return WFS_SUCCESS;
}

// 窗口处理
HRESULT CXFS_CAM::InnerDisplayEx(const WFSCAMDISPEX &stDisplayEx, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    LPWFSCAMDISPEX lpDisplayEx = NULL;
    lpDisplayEx = (LPWFSCAMDISPEX)&stDisplayEx;

    // 启动窗口
    WFSCAMDISP stDisplay;
    stDisplay.wCamera = lpDisplayEx->wCamera;
    stDisplay.wAction = WFS_CAM_CREATE;
    stDisplay.wWidth = lpDisplayEx->wWidth;
    stDisplay.wHeight = lpDisplayEx->wHeight;
    stDisplay.wX = lpDisplayEx->wX;
    stDisplay.wY = lpDisplayEx->wY;
    stDisplay.hWnd = lpDisplayEx->hWnd;
    stDisplay.wHpixel = lpDisplayEx->wHpixel;
    stDisplay.wVpixel = lpDisplayEx->wVpixel;
    hRet = InnerDisplay(stDisplay, dwTimeout);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口失败: ->InnerDisplay() Fail. ReturnCode:%d",
            hRet, hRet);
        return hRet;
    }

    // 摄像拍照
    WFSCAMTAKEPICTEX stWfsTakePicEx;
    stWfsTakePicEx.wCamera = lpDisplayEx->wCamera;
    stWfsTakePicEx.lpszCamData = lpDisplayEx->pszTexData;
    stWfsTakePicEx.lpszPictureFile = lpDisplayEx->pszPictureFile;
    if ((hRet = InnerTakePictureEx(stWfsTakePicEx, dwTimeout)) != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "摄像拍照: ->InnerTakePictureEx()失败, ReturnCode: %d, 进入关闭窗口处理.", hRet);
    }


    // 关闭窗口
    stDisplay.wAction = WFS_CAM_DESTROY;
    hRet = InnerDisplay(stDisplay, dwTimeout);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "销毁摄像窗口失败: ->InnerDisplay() Fail. ReturnCode:%d",
            hRet, hRet);
        return hRet;
    }

    return WFS_SUCCESS;
}

// 按指定模式调用DevCAM::Display接口
HRESULT CXFS_CAM::CameraMode_Display(WORD wCamMode, DWORD hWnd, WORD wAction,
                                     WORD wX, WORD wY, WORD wWidth, WORD wHeight,
                                     WORD wHpixel, WORD wVpixel)
{
    if (wCamMode == WFS_CAM_ROOM)   // 全景摄像
    {
        return m_pDevRoom->Display(hWnd, wAction, wX, wY, wWidth, wHeight);
    } else
    if (wCamMode == WFS_CAM_PERSON) // 人脸摄像
    {
        return m_pDev->Display(hWnd, wAction, wX, wY, wWidth, wHeight);
    } else /* WFS_CAM_HIGHTCAMERA */// 高拍摄像
    {
        return m_pDevHigh->Display(hWnd, wAction, wX, wY, wWidth, wHeight);//, wHpixel, wVpixel);
    }
}

// 按指定模式调用DevCAM::TakePictureEx接口
HRESULT CXFS_CAM::CameraMode_TakePictureEx(WORD wCamMode, LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType,
                                           BOOL bIsContinue, DWORD wTimeOut)
{
    if (wCamMode == WFS_CAM_ROOM)   // 全景摄像
    {
        return m_pDevRoom->TakePictureEx(lpFileName, lpCamData, wPicType, bIsContinue, wTimeOut, wCamMode);
    } else
    if (wCamMode == WFS_CAM_PERSON) // 人脸摄像
    {
        return m_pDev->TakePictureEx(lpFileName, lpCamData, wPicType, bIsContinue, wTimeOut, wCamMode);
    } else /* WFS_CAM_HIGHTCAMERA */// 高拍摄像
    {
        return m_pDevHigh->TakePictureEx(lpFileName, lpCamData, wPicType, bIsContinue, wTimeOut, wCamMode);
    }
}

//--------------------------------窗口处理---------------------------------------
// 显示窗口
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

// 结束显示窗口
void CXFS_CAM::vSlotHideWin()
{
    if (showWin != nullptr)
    {
        showWin->hide();
        delete showWin;
        showWin = nullptr;
    }
}

//-----------------------------共享内存处理----------------------------------------
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

//-----------------------------其他处理----------------------------------------
bool CXFS_CAM::IsDevStatusOK()
{
    return (m_stStatus.fwDevice == WFS_CAM_DEVONLINE) ? true : false;
}

void CXFS_CAM::InitConifig()
{
    LPCSTR ThisModule = "InitConifig";

    CHAR szKeyName[256] = { 0x00 };

    WORD wTmp = 0;

    memset(&m_sCamIniConfig, 0x00, sizeof(ST_CAM_INI_CONFIG));
    memset(&m_stDevInitParam, 0x00, sizeof(ST_CAM_DEV_INIT_PARAM));

    strcpy(m_sCamIniConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 摄像模式支持(缺省人脸设备)
    m_wDeviceModeSup = (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "CameraModeSup", (DWORD)CAM_MODE_PERSON);

    // 人脸摄像设备类型(缺省云从)
    m_wDeviceType = (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DeviceType", (DWORD)CAM_DEV_CLOUDWALK);

    // 全景摄像设备类型(缺省云从)
    m_wDeviceRoomType = (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DeviceRoomType", (DWORD)CAM_DEV_CLOUDWALK);

    // 高拍摄像设备类型(缺省云从)
    m_wDeviceHighType = (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DeviceHighType", (DWORD)CAM_DEV_CLOUDWALK);

    memset(szKeyName, 0x00, sizeof(szKeyName));
    sprintf(szKeyName, "DEVICE_SET_%d", m_wDeviceType);

    if (m_wDeviceType == CAM_DEV_CLOUDWALK ||
        m_wDeviceRoomType == CAM_DEV_CLOUDWALK ||
        m_wDeviceHighType  == CAM_DEV_CLOUDWALK) // 云从摄像配置项(全景+人脸+高拍)
    {
        // 打开方式,0表示序号打开/1表示vid pid打开,缺省0
        m_sCamIniConfig.stCamCwInitParam.wOpenType = (WORD)m_cXfsReg.GetValue(szKeyName, "opentype", (DWORD)CAM_OPEN_DEVIDX);
        // 可见光模组枚举序号,缺省1
        m_sCamIniConfig.stCamCwInitParam.wVisCamIdx = (WORD)m_cXfsReg.GetValue(szKeyName, "visCamIdx", 1);
        // 红外光模组枚举序号，缺省0
        m_sCamIniConfig.stCamCwInitParam.wNisCamIdx = (WORD)m_cXfsReg.GetValue(szKeyName, "nisCamIdx", (DWORD)0);
        // 可见光模组vid/pid
        strcpy(m_sCamIniConfig.stCamCwInitParam.szVisVid, m_cXfsReg.GetValue(szKeyName, "vis_vid", "0C45"));
        strcpy(m_sCamIniConfig.stCamCwInitParam.szVisPid, m_cXfsReg.GetValue(szKeyName, "vis_pid", "C812"));
        // 红外光模组vid/pid
        strcpy(m_sCamIniConfig.stCamCwInitParam.szNisVid, m_cXfsReg.GetValue(szKeyName, "nis_vid", "0C45"));
        strcpy(m_sCamIniConfig.stCamCwInitParam.szNisPid, m_cXfsReg.GetValue(szKeyName, "nis_pid", "C811"));

        // 模型加载方式, 0:文件加载 1: 内存加载
        m_sCamIniConfig.stCamCwInitParam.nModelMode = m_cXfsReg.GetValue(szKeyName, "modelMode", 1);
        // 活体类型, 0:不活检 1: 红外活体 2:结构光活体
        m_sCamIniConfig.stCamCwInitParam.nLivenessMode = m_cXfsReg.GetValue(szKeyName, "livenessMode", 1);
        // 授权类型, 1:芯片授权 2：hasp授权 3:临时授权 4:云从相机绑定授权
        m_sCamIniConfig.stCamCwInitParam.nLicenseType = m_cXfsReg.GetValue(szKeyName, "licenseType", 1);
        // 算法矩阵文件,可以不写，使用默认
        strcpy(m_sCamIniConfig.stCamCwInitParam.szConfigFile, m_cXfsReg.GetValue(szKeyName, "configFile", ""));
        // 人脸检测模型,可以不写，使用默认
        strcpy(m_sCamIniConfig.stCamCwInitParam.szFaceDetectFile, m_cXfsReg.GetValue(szKeyName, "faceDetectFile", ""));
        // 关键点检测模型,可以不写，使用默认
        strcpy(m_sCamIniConfig.stCamCwInitParam.szKeyPointDetectFile, m_cXfsReg.GetValue(szKeyName, "keyPointDetectFile", ""));
        // 关键点跟踪模型,可以不写，使用默认
        strcpy(m_sCamIniConfig.stCamCwInitParam.szKeyPointTrackFile, m_cXfsReg.GetValue(szKeyName, "keyPointTrackFile", ""));
        // 人脸质量评估模型,可以不写，使用默认
        strcpy(m_sCamIniConfig.stCamCwInitParam.szFaceQualityFile, m_cXfsReg.GetValue(szKeyName, "faceQualityFile", ""));
        // 活体模型,可以不写，使用默认
        strcpy(m_sCamIniConfig.stCamCwInitParam.szFaceLivenessFile, m_cXfsReg.GetValue(szKeyName, "faceLivenessFile", ""));
        // 人脸检测模型,可以不写，使用默认
        strcpy(m_sCamIniConfig.stCamCwInitParam.szFaceKeyPointTrackFile, m_cXfsReg.GetValue(szKeyName, "faceKeyPointTrackFile", ""));
        // 是否使用GPU(1/使用GPU,-1使用CPU)
        m_sCamIniConfig.stCamCwInitParam.nGpu = m_cXfsReg.GetValue(szKeyName, "GPU", (DWORD)0);
        if (m_sCamIniConfig.stCamCwInitParam.nGpu == 0)
        {
            m_sCamIniConfig.stCamCwInitParam.nGpu = -1;
        }
        // 是否多线程检测
        m_sCamIniConfig.stCamCwInitParam.nMultiThread = m_cXfsReg.GetValue(szKeyName, "multiThread", 1);

        // 是否打开语音提示支持(0打开/1关闭,缺省0)						// 30-00-00-00(FT#0031)
        m_sCamIniConfig.stCamCwInitParam.bIsVoiceSupp =					// 30-00-00-00(FT#0031)
                ((WORD)m_cXfsReg.GetValue(szKeyName, "VoiceOpen", (DWORD)0) == 0 ? FALSE : TRUE);// 30-00-00-00(FT#0031)

        // 语音提示配置文件(VoiceOpen=1时必须指定,否则VoiceOpen配置无效)// 30-00-00-00(FT#0031)
        strcpy(m_sCamIniConfig.stCamCwInitParam.szVoiceTipFile, m_cXfsReg.GetValue(szKeyName, "VoiceTipFile", ""));// 30-00-00-00(FT#0031)

        // 截取画面帧的分辨率(Width),缺省:1280
        m_sCamIniConfig.stCamCwInitParam.shResoWidth = m_cXfsReg.GetValue(szKeyName, "FrameResoWidth", (DWORD)1280);
        if (m_sCamIniConfig.stCamCwInitParam.shResoWidth < 0)
        {
            m_sCamIniConfig.stCamCwInitParam.shResoWidth = 1280;
        }

        // 截取画面帧的分辨率(Height),缺省:1024
        m_sCamIniConfig.stCamCwInitParam.shResoHeight = m_cXfsReg.GetValue(szKeyName, "FrameResoHeight", (DWORD)1024);
        if (m_sCamIniConfig.stCamCwInitParam.shResoHeight < 0)
        {
            m_sCamIniConfig.stCamCwInitParam.shResoHeight = 1024;
        }

        // 设置SDK版本,缺省0
        m_sCamIniConfig.stCamCwInitParam.wSDKVersion = m_cXfsReg.GetValue(szKeyName, "SDKVersion", (DWORD)0);
        if (m_sCamIniConfig.stCamCwInitParam.wSDKVersion < 0 || m_sCamIniConfig.stCamCwInitParam.wSDKVersion > 1)
        {
            m_sCamIniConfig.stCamCwInitParam.wSDKVersion = 0;
        }

        memcpy(&(m_stDevInitParam.stCamCwInitParam), &(m_sCamIniConfig.stCamCwInitParam),
               sizeof(ST_CAM_CW_INIT_PARAM));
    }

    if (m_wDeviceType == CAM_DEV_TCF261 ||
        m_wDeviceRoomType == CAM_DEV_TCF261 ||
        m_wDeviceHighType == CAM_DEV_TCF261) // 天诚盛业摄像配置项(全景+人脸+高拍)
    {
        // 抓拍图像模式(0活体头像/1全景，缺省0)
        m_sCamIniConfig.stCamTCF261InitParam.wLiveDetectMode = (WORD)m_cXfsReg.GetValue(szKeyName, "LiveDetectMode", (DWORD)0);
        memcpy(&(m_stDevInitParam.stCamTCF261InitParam), &(m_sCamIniConfig.stCamTCF261InitParam),
               sizeof(ST_CAM_TCF261_INIT_PARAM));
    }


    // 摄像数据共享内存名(32位,缺省CamShowSharedMemData)
    strcpy(m_sCamIniConfig.szCamDataSMemName, m_cXfsReg.GetValue("CAMERA_SHAREMEMORY",
                                                                 "ShowSharedMemName", SHAREDMEMNAME_CAMDATA));
    //摄像数据共享内存Size(缺省10M)
    m_sCamIniConfig.ulCamDataSMemSize = (ULONG)m_cXfsReg.GetValue("CAMERA_SHAREMEMORY",
                                                                  "ShowSharedMemSize", (ULONG)SHAREDMEMSIZE_CAMDATA);

    /*if (m_wDeviceType == CAM_DEV_CLOUDWALK ||
        m_wDeviceRoomType == CAM_DEV_CLOUDWALK ||
        m_wDeviceHighType == CAM_DEV_CLOUDWALK) // 云从摄像配置项
    {*/
        m_stDevInitParam.ulCamDataSMemSize = m_sCamIniConfig.ulCamDataSMemSize;
        memcpy(m_stDevInitParam.szCamDataSMemName, m_sCamIniConfig.szCamDataSMemName,
               sizeof(m_sCamIniConfig.szCamDataSMemName));
    //}

    // 摄像刷新时间(秒,缺省30毫秒秒)
    m_sCamIniConfig.unWinRefreshTime = m_cXfsReg.GetValue(szKeyName, "WinRefreshTime", (DWORD)30);
    if (m_sCamIniConfig.unWinRefreshTime < 0)
    {
        m_sCamIniConfig.unWinRefreshTime = 30;
    }


    //--------------------------TakePic命令相关设置参数获取--------------------------
    // TakePictrue/TakePictrueEx超时(单位:秒毫秒,缺省0)		// 30-00-00-00(FT#0031)
    m_sCamIniConfig.wTakePicTimeOut = m_cXfsReg.GetValue("TakePic", "TakePicTimeOut", (DWORD)0);// 30-00-00-00(FT#0031)
    if (m_sCamIniConfig.wTakePicTimeOut < 0)
    {
        m_sCamIniConfig.wTakePicTimeOut = 0;				// 30-00-00-00(FT#0031)
    }

    // 是否支持生成红外图像(0:不支持, 1:支持, 缺省0)
    m_sCamIniConfig.stCamImageMothod.wParam[0] = m_cXfsReg.GetValue("TakePic", "NirImageSup", (DWORD)0);
    if (m_sCamIniConfig.stCamImageMothod.wParam[0] < 0 ||
        m_sCamIniConfig.stCamImageMothod.wParam[0] > 1)
    {
        m_sCamIniConfig.stCamImageMothod.wParam[0] = 0;
    }
    // 设置红外图更名后缀,缺省nir
    if (m_sCamIniConfig.stCamImageMothod.wParam[0] != 0)
    {
        strcpy(m_sCamIniConfig.stCamImageMothod.szNirPicSuffix,
               m_cXfsReg.GetValue(szKeyName, "NirPicSuffix", "_nir"));
        if (strlen(m_sCamIniConfig.stCamImageMothod.szNirPicSuffix) < 1)
        {
            sprintf(m_sCamIniConfig.stCamImageMothod.szNirPicSuffix, "_nir");
        }

    }

    // 指定生成图片的分辨率,缺省: 0
    m_sCamIniConfig.wPicpixel[0] = m_cXfsReg.GetValue("TakePic", "PicHpixel", (DWORD)0);    // 水平分辨率
    m_sCamIniConfig.wPicpixel[1] = m_cXfsReg.GetValue("TakePic", "PicVpixel", (DWORD)0);    // 垂直分辨率

    //--------------------------Reset命令相关设置参数获取--------------------------
    // Reset执行时是否支持关闭摄像窗口(0:不支持,1支持,缺省0)
    m_sCamIniConfig.wResetCloseDiaplay = m_cXfsReg.GetValue("RESET_CONFIG", "ResetCloseDiaplay", (DWORD)0);
    if (m_sCamIniConfig.wResetCloseDiaplay < 0 || m_sCamIniConfig.wResetCloseDiaplay > 1)
    {
        m_sCamIniConfig.wResetCloseDiaplay = 0;
    }

    //--------------------------事件消息相关设置参数获取--------------------------
    // 是否支持上报 活体图像检测错误事件(0不支持/1支持,缺省0)
    m_sCamIniConfig.wEventLiveErrorSup = m_cXfsReg.GetValue("EVENT_CONFIG", "LiveErrorSup", (DWORD)0);
    if (m_sCamIniConfig.wEventLiveErrorSup != 0 && m_sCamIniConfig.wEventLiveErrorSup != 1)
    {
        m_sCamIniConfig.wEventLiveErrorSup = 0;
    }

    // TakePic命令生成格式(1base64/2JPG/4BMP，缺省4)
    m_sCamIniConfig.wTakePicMakeFormatFlag = (WORD)m_cXfsReg.GetValue("CAMERA_INFO",
                                                                        "TakePicMakeFormat", (DWORD)0);

    // TakePic 缺省保存目录
    strcpy(m_sCamIniConfig.szTakePicDefSavePath, m_cXfsReg.GetValue("CAMERA_INFO", "TakePicDefSavePath", ""));
    if (strlen((char*)m_sCamIniConfig.szTakePicDefSavePath) < 2 || m_sCamIniConfig.szTakePicDefSavePath[0]  != '/')
    {
        memset(m_sCamIniConfig.szTakePicDefSavePath, 0x00, sizeof(m_sCamIniConfig.szTakePicDefSavePath));
    }


    // 指定银行，缺省0
    m_sCamIniConfig.wBank = (WORD)m_cXfsReg.GetValue("BANK", "BankNo", (DWORD)BANK_NOALL);
    m_stDevInitParam.wBank = m_sCamIniConfig.wBank;

    if (m_sCamIniConfig.wBank == BANK_PINGAN)
    {
        memset(szKeyName, 0x00, sizeof(szKeyName));
        sprintf(szKeyName, "CAMERA_BANK_%d", m_sCamIniConfig.wBank);

        //双目摄像支持注册文件(缺省/home/projects/wsap/cfg/Camera.ini)
        strcpy(m_sCamIniConfig.stCamSaveImagePab.byCameraSupFile, m_cXfsReg.GetValue(szKeyName, "CameraSupFile", SPCAMERASUPFILE));
        if (strlen((char*)m_sCamIniConfig.stCamSaveImagePab.byCameraSupFile) < 2 ||
            m_sCamIniConfig.stCamSaveImagePab.byCameraSupFile[0]  != '/') {
            memset(m_sCamIniConfig.stCamSaveImagePab.byCameraSupFile, 0x00, sizeof(m_sCamIniConfig.stCamSaveImagePab.byCameraSupFile));
            memcpy(m_sCamIniConfig.stCamSaveImagePab.byCameraSupFile, SPCAMERASUPFILE, strlen(SPCAMERASUPFILE));
        }

        // 双目安装
        QSettings qReadCameraSupFile(m_sCamIniConfig.stCamSaveImagePab.byCameraSupFile, QSettings::IniFormat);
        //qReadCfgFile.setIniCodec(QTextCodec::codecForName("utf-8"));

        if (qReadCameraSupFile.contains("CFG/IsInstallCA") != TRUE)
        {
            m_sCamIniConfig.stCamSaveImagePab.wIsInstallCAM = 3;
        } else
        {
            m_sCamIniConfig.stCamSaveImagePab.wIsInstallCAM = qReadCameraSupFile.value("CFG/IsInstallCAM", (DWORD)0).toInt();
        }

        // 摄像图片信息文件 CameraInfo.ini 全路径
        strcpy(m_sCamIniConfig.stCamSaveImagePab.byImageInfoFile, m_cXfsReg.GetValue(szKeyName, "ImageInfoFile", IMAGEINFOFILE));
        if (strlen((char*)m_sCamIniConfig.stCamSaveImagePab.byImageInfoFile) < 2 ||
            m_sCamIniConfig.stCamSaveImagePab.byImageInfoFile[0] != '/') {
            memset(m_sCamIniConfig.stCamSaveImagePab.byImageInfoFile, 0x00,
                   sizeof(m_sCamIniConfig.stCamSaveImagePab.byImageInfoFile));
            memcpy(m_sCamIniConfig.stCamSaveImagePab.byImageInfoFile, IMAGEINFOFILE, strlen(IMAGEINFOFILE));
        }

        // CameraPic备份保存全路径	BASE64
        strcpy(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBASE64, m_cXfsReg.GetValue(szKeyName, "ImageSavePathBASE64", IMAGESAVEPATH_BASE64));
        if (strlen((char*)m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBASE64) < 2 ||
            m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBASE64[0]  != '/') {
            memset(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBASE64, 0x00,
                   sizeof(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBASE64));
            memcpy(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBASE64, IMAGESAVEPATH_BASE64, strlen(IMAGESAVEPATH_BASE64));
        }

        // CameraPic备份保存全路径 JPG
        strcpy(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathJPG, m_cXfsReg.GetValue(szKeyName, "ImageSavePathJPG", IMAGESAVEPATH_JPG));
        if (strlen((char*)m_sCamIniConfig.stCamSaveImagePab.byImageSavePathJPG) < 2 ||
            m_sCamIniConfig.stCamSaveImagePab.byImageSavePathJPG[0] != '/') {
            memset(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathJPG, 0x00,
                   sizeof(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathJPG));
            memcpy(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathJPG, IMAGESAVEPATH_JPG, strlen(IMAGESAVEPATH_JPG));
        }

        // CameraPic备份保存全路径	BMP
        strcpy(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBMP, m_cXfsReg.GetValue(szKeyName, "ImageSavePathBMP", IMAGESAVEPATH_BMP));
        if (strlen((char*)m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBMP) < 2 ||
            m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBMP[0] != '/') {
            memset(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBMP, 0x00,
                   sizeof(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBMP));
            memcpy(m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBMP, IMAGESAVEPATH_BMP, strlen(IMAGESAVEPATH_BMP));
        }

        // 摄像备份信息文件绝对路径
        strcpy(m_sCamIniConfig.stCamSaveImagePab.byImageSaveInfoPath, m_cXfsReg.GetValue(szKeyName, "ImageSaveInfoPath", IMAGESAVEINFOFILE));
        if (strlen((char*)m_sCamIniConfig.stCamSaveImagePab.byImageSaveInfoPath) < 2 ||
            m_sCamIniConfig.stCamSaveImagePab.byImageSaveInfoPath[0] != '/') {
            memset(m_sCamIniConfig.stCamSaveImagePab.byImageSaveInfoPath, 0x00,
                   sizeof(m_sCamIniConfig.stCamSaveImagePab.byImageSaveInfoPath));
            memcpy(m_sCamIniConfig.stCamSaveImagePab.byImageSaveInfoPath, IMAGESAVEINFOFILE, strlen(IMAGESAVEINFOFILE));
        }

        // 摄像图片备份配置文件全路径
        strcpy(m_sCamIniConfig.stCamSaveImagePab.bySPCamerasNoCfgFile, m_cXfsReg.GetValue(szKeyName, "SPCamerasNoCfgFile", SPCAMERASNOCFG));
        if (strlen((char*)m_sCamIniConfig.stCamSaveImagePab.bySPCamerasNoCfgFile) < 2 ||
            m_sCamIniConfig.stCamSaveImagePab.bySPCamerasNoCfgFile[0] != '/') {
            memset(m_sCamIniConfig.stCamSaveImagePab.bySPCamerasNoCfgFile, 0x00,
                   sizeof(m_sCamIniConfig.stCamSaveImagePab.bySPCamerasNoCfgFile));
            memcpy(m_sCamIniConfig.stCamSaveImagePab.bySPCamerasNoCfgFile, SPCAMERASNOCFG, strlen(SPCAMERASNOCFG));
        }

        QSettings qReadCfgFile(m_sCamIniConfig.stCamSaveImagePab.bySPCamerasNoCfgFile, QSettings::IniFormat);
        //qReadCfgFile.setIniCodec(QTextCodec::codecForName("utf-8"));

        // 是否启用单天累加文件
        wTmp = qReadCfgFile.value("/CFG/IsNeedDailyRecord", (DWORD)1).toInt();
        if (wTmp == 1) {
            m_sCamIniConfig.stCamSaveImagePab.bIsNeedDailyRecord = TRUE;
        } else {
            m_sCamIniConfig.stCamSaveImagePab.bIsNeedDailyRecord = FALSE;
        }

        // CameraPic备份保存天数
        m_sCamIniConfig.stCamSaveImagePab.wSaveTime =  qReadCfgFile.value("/CFG/SavaTime", (DWORD)30).toInt();

        // CameraPic备份保存过期清理
        vDelImageInfoDir();

        memcpy(&(m_stDevInitParam.stCamSaveImagePab), &(m_sCamIniConfig.stCamSaveImagePab),
               sizeof(ST_CAM_SAVEIMAGE_PAB));
    }


    if (m_sCamIniConfig.wBank == BANK_BCS)  // 长沙银行INI参数获取
    {
        memset(szKeyName, 0x00, sizeof(szKeyName));
        sprintf(szKeyName, "CAMERA_BANK_%d", m_sCamIniConfig.wBank);

        // 处理1. 设置TakePic单独生成的人脸图像名,不包含扩展名(不设置表示不改),缺省空
        strcpy(m_sCamIniConfig.stCamMethodBCS.szMT1_PerSonName, m_cXfsReg.GetValue(szKeyName, "MT1_PerSonName", ""));
    }

    //--------------------------招商银行INI参数获取--------------------------
    if (m_sCamIniConfig.wBank == BANK_CMB)
    {
        memset(szKeyName, 0x00, sizeof(szKeyName));
        sprintf(szKeyName, "CAMERA_BANK_%d", m_sCamIniConfig.wBank);

        // 处理1. 设置TakePic生成的红外图绝对路径+红外图名,缺省空(不处理红外图)
        strcpy(m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicPath,
               m_cXfsReg.GetValue(szKeyName, "MT1_NirPicPath", ""));

        // 处理1. 设置红外图更名后缀,缺省nis
        strcpy(m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicSuffix,
               m_cXfsReg.GetValue(szKeyName, "MT1_NirPicSuffix", "_nir"));
    }
}

void CXFS_CAM::InitStatus()
{
    memset(&m_stStatus, 0x00, sizeof(WFSCAMSTATUS));
    m_stStatus.fwDevice      = WFS_CAM_DEVNODEVICE;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.fwMedia[i] = WFS_CAM_MEDIANOTSUPP;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.fwCameras[i] = WFS_CAM_CAMNOTSUPP;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.usPictures[i] = 0;
    m_stStatus.lpszExtra = NULL;
    m_stStatus.wAntiFraudModule = 0;
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
        case ERR_CANCEL:        // (-12)					// 30-00-00-00(FT#0031)
            return WFS_ERR_CANCELED;            // -4		// 30-00-00-00(FT#0031)
        default:
            return WFS_ERR_HARDWARE_ERROR;      // -14
    }
}

WORD CXFS_CAM::ConvertLiveErr2WFS(WORD wError)
{
    switch(wError)
    {
        case LIVE_ERR_OK:                   // 0:活检图像正常
            return WFS_CAM_LIVE_ERR_OK;
        case LIVE_ERR_VIS_NOFACE:           // 1:可见光未检测到人脸
            return WFS_CAM_LIVE_ERR_VIS_NOFACE;
        case LIVE_ERR_FACE_SHELTER:         // 2:人脸有遮挡(五官有遮挡,戴镜帽等)
            return WFS_CAM_LIVE_ERR_FACE_SHELTER;
        case LIVE_ERR_FACE_ANGLE_FAIL:      // 3:人脸角度不满足要求(低头/抬头/侧脸等)
            return WFS_CAM_LIVE_ERR_FACE_ANGLE_FAIL;
        case LIVE_ERR_FACE_EYECLOSE:        // 4:检测到闭眼
            return WFS_CAM_LIVE_ERR_FACE_EYECLOSE;
        case LIVE_ERR_FACE_MOUTHOPEN:       // 5:检测到张嘴
            return WFS_CAM_LIVE_ERR_FACE_MOUTHOPEN;
        case LIVE_ERR_FACE_SHAKE:           // 6:检测到人脸晃动/模糊
            return WFS_CAM_LIVE_ERR_FACE_SHAKE;
        case LIVE_ERR_FACE_MULTIPLE:        // 7:检测到多张人脸
            return WFS_CAM_LIVE_ERR_FACE_MULTIPLE;
        case LIVE_ERR_IS_UNLIVE:            // 8:检测到非活体
            return WFS_CAM_LIVE_ERR_IS_UNLIVE;
        case LIVE_ERR_FACE_TOOFAR:          // 9:人脸离摄像头太远
            return WFS_CAM_LIVE_ERR_FACE_TOOFAR;
        case LIVE_ERR_FACE_TOONEAR:         // 10:人脸离摄像头太近
            return WFS_CAM_LIVE_ERR_FACE_TOONEAR;
        case LIVE_ERR_NIS_NOFACE:           // 11:红外光未检测到人脸
            return WFS_CAM_LIVE_ERR_NIS_NOFACE;
        case LIVE_ERR_UNKNOWN:              // 99:其他/未知错误
            return WFS_CAM_LIVE_ERR_UNKNOWN;
        default:
            return WFS_CAM_LIVE_ERR_UNKNOWN;
    }
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
            sprintf(cTemp, "%s", m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBASE64);
        if (i == 1)
            sprintf(cTemp, "%s", m_sCamIniConfig.stCamSaveImagePab.byImageSavePathJPG);
        if (i == 2)
            sprintf(cTemp, "%s", m_sCamIniConfig.stCamSaveImagePab.byImageSavePathBMP);

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

// TakePicture命令完成后处理
INT CXFS_CAM::InnerTakePicAfterMothod()
{
    THISMODULE(__FUNCTION__);

    // 指定分辨率生成图片
    if (m_sCamIniConfig.wPicpixel[0] > 0 && m_sCamIniConfig.wPicpixel[1] > 0)
    {
        cv::Mat cvSrcMat = cv::imread(szFileNameFromAp);
        cv::Mat cvDstMat;
        cv::resize(cvSrcMat, cvDstMat, cv::Size(m_sCamIniConfig.wPicpixel[0], m_sCamIniConfig.wPicpixel[1]));
        cv::imwrite(szFileNameFromAp, cvDstMat);

        Log(ThisModule, __LINE__, "TakePicture命令完成后处理: 指定分辨率生成图片: %d*%d",
            m_sCamIniConfig.wPicpixel[0], m_sCamIniConfig.wPicpixel[1]);
    }

    return WFS_SUCCESS;
}

// 招商银行特殊处理
INT CXFS_CAM::nBankMethod_CMB()
{
    THISMODULE(__FUNCTION__);

    if (strlen(m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicPath) < 2)
    {
        Log(ThisModule, __LINE__, "招商银行特殊处理(红外图改名): INI指定红外图路径[%s]无效,不处理．",
            m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicPath);
        return WFS_SUCCESS;
    }

    if (FileAccess::is_file_directory_exist(m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicPath) != true)
    {
        Log(ThisModule, __LINE__, "招商银行特殊处理(红外图改名): INI指定红外图路径[%s]不存在,不处理．",
            m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicPath);
        return WFS_SUCCESS;
    }

    // 分解文件名和文件扩展名
    CHAR szFileName[MAX_PATH] = { 0x00 };
    CHAR szFileExt[12] = { 0x00 };
    FileDir::_split_whole_name(szFileNameFromAp, szFileName, szFileExt);

    sprintf(szFileName + strlen(szFileName), "%s%s",
            m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicSuffix, szFileExt);

    if (FileAccess::posix_copy_file(m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicPath, szFileName, false) != true)
    {
        Log(ThisModule, __LINE__, "招商银行特殊处理(红外图改名): 复制文件[%s]->[%s]出错．",
            m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicPath, szFileName);
    } else
    {
        Log(ThisModule, __LINE__, "招商银行特殊处理(红外图改名): 复制文件[%s]->[%s]完成．",
            m_sCamIniConfig.stCamMethodCMB.szMT1_NirPicPath, szFileName);
    }

    return WFS_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////


