#include "DevCAM_CloudWalk.h"
#include "opencv2/opencv.hpp"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QSettings>
#include <sys/stat.h>
#include <dirent.h>

// CAM 版本号
BYTE    byDevVRTU[17] = {"DevCAM00000100"};

static const char *ThisFile = "DevCAM_CloudWalk.cpp";

//////////////////////////////////////////////////////////////////////////

CDevCAM_CloudWalk::CDevCAM_CloudWalk() : CDevImpl_CloudWalk()
{
    SetLogFile(LOG_NAME_DEVCAM, ThisFile);  // 设置日志文件名和错误发生的文件

    memset(&m_stStatus, 0x00, sizeof(DEVCAMSTATUS));
    memset(&m_stCamOpenType, 0x00, sizeof(ST_CAM_OPENTYPE));
    memset(&m_stCamSaveImageCfg, 0x00, sizeof(ST_CAM_SAVEIMAGE_CFG));

    memset(m_pSaveFileName, 0x00, sizeof(m_pSaveFileName));
    m_wSaveFileType = PIC_JPG;
    m_bIsTakePicExStop = FALSE;

    m_bDisplayOpenOK = FALSE;     // 未打开
    m_qSharedMemData = nullptr;
    m_DevStatusOLD.Clear();
    memset(m_szSharedDataName, 0x00, sizeof(m_szSharedDataName));
    m_ulSharedDataSize = 0;

    wBankNo = 0;
}

CDevCAM_CloudWalk::~CDevCAM_CloudWalk()
{
    Close();
}

void CDevCAM_CloudWalk::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

long CDevCAM_CloudWalk::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 建立Cam连接
    if (IsDeviceOpen() != TRUE)
    {
        if (OpenDevice() != TRUE)
        {
            return ERR_OPEN_CAMER;
        }
    }
    // 检查设备是否链接
    DEVCAMSTATUS stStatus;						// 30-00-00-00(FT#0031)
    GetStatus(stStatus);                        // 30-00-00-00(FT#0031)
    if (stStatus.fwDevice != DEVICE_ONLINE)		// 30-00-00-00(FT#0031)
    {											// 30-00-00-00(FT#0031)
        return ERR_OPEN_CAMER;					// 30-00-00-00(FT#0031)
    }											// 30-00-00-00(FT#0031)

    // 初始化检测器(加载算法模型)
    if (bInitDetectorParam(m_stCamCwInitParam.nModelMode,
                           m_stCamCwInitParam.nLivenessMode,
                           m_stCamCwInitParam.nLicenseType,
                           m_stCamCwInitParam.szConfigFile,
                           m_stCamCwInitParam.szFaceDetectFile,
                           m_stCamCwInitParam.szKeyPointDetectFile,
                           m_stCamCwInitParam.szFaceKeyPointTrackFile,
                           m_stCamCwInitParam.szFaceQualityFile,
                           m_stCamCwInitParam.szFaceLivenessFile,
                           m_stCamCwInitParam.nGpu,
                           m_stCamCwInitParam.nMultiThread) != TRUE)
    {
        return ERR_OPEN_CAMER;
    }

    // 绘制人脸框及显示回调函数
    if (bRegisterPreviewHandle(this) != TRUE)
    {
        return ERR_OPEN_CAMER;
    }

    // 活体检测及回调设置
    if (bRegisterDetectHandle(this) != TRUE)
    {
        return ERR_OPEN_CAMER;
    }

    // 是否进行人脸追踪显示
    if (bEnableFaceTrace(true) != TRUE)
    {
        return ERR_OPEN_CAMER;
    }

    // 连接共享内存(摄像)
    if (bConnSharedMemory(m_szSharedDataName) != TRUE)
    {
        return ERR_OPEN_CAMER;
    }

    return CAM_SUCCESS;
}

long CDevCAM_CloudWalk::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    CloseDevice();
    bDisConnSharedMemory();

    return CAM_SUCCESS;
}

long CDevCAM_CloudWalk::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (bCloseAllCameras() != TRUE)
    {
        return ERR_OTHER;
    }
    // 停止连续活体检测
    bStopLiveDetect();		// 30-00-00-00(FT#0031)

    m_bDisplayOpenOK = FALSE;	// 30-00-00-00(FT#0031)
    m_bIsTakePicExStop = TRUE;	// 30-00-00-00(FT#0031)

    return CAM_SUCCESS;
}

long CDevCAM_CloudWalk::GetDevInfo(char *pInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return CAM_SUCCESS;
}


long CDevCAM_CloudWalk::GetStatus(DEVCAMSTATUS &stStatus)
{
#define MCMP(a, b) \
    (memcmp(a, b, strlen(a)) == 0 && memcmp(a, b, strlen(b)) == 0)

    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    CWCameraState state0, state1;

    stStatus.fwDevice      = DEVICE_OFFLINE;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.fwMedia[i] = MEDIA_UNKNOWN;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.fwCameras[i] = STATUS_NOTSUPP;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.usPictures[i] = CAM_ROOM;

    if (IsDeviceOpen() != TRUE)
    {
        return ERR_OTHER;
    }

    stStatus.fwDevice   = DEVICE_ONLINE;
    stStatus.fwMedia[1] = MEDIA_OK;
    stStatus.fwCameras[1] = STATUS_OK;

    /*if (wOpenType == CAM_OPEN_DEVIDX)
    {
        // 按序号
        if ((bGetCameraStatus(wVisCamIdx, &state0) != TRUE) ||
            (bGetCameraStatus(wNisCamIdx, &state1) != TRUE))
        {
            m_stStatus.fwDevice = DEVICE_HWERROR;
            m_stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            m_stStatus.fwCameras[1] = STATUS_NOTSUPP;
            return ERR_OTHER;
        }
    } else
    {
        if ((bGetCameraStatusEx(m_pVisVid, m_pVisPid, &state0) != TRUE) ||
            (bGetCameraStatusEx(m_pNisVid, m_pNisPid, &state1) != TRUE))
        {
            m_stStatus.fwDevice = DEVICE_HWERROR;
            m_stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            m_stStatus.fwCameras[1] = STATUS_NOTSUPP;
            return ERR_OTHER;
        }
    }
    */

    if (IsDeviceOpen() != TRUE)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (m_nRetErrOLD[0] != ERR_OPEN_CAMER)
        {
            Log(ThisModule, __LINE__, "IsDeviceOpen() != TRUE. Return: %d.", ERR_OPEN_CAMER);
            m_nRetErrOLD[0] = ERR_OPEN_CAMER;
        }
        return ERR_OPEN_CAMER;
    }

    // 枚举当前设备上的所有相机 新增 // 30-00-00-00(FT#0031)
    CWCameraDevice *stCameraLists = nullptr, *stCameraTmp = nullptr;
    if (bEnumCameras(&stCameraLists) != TRUE)
    {
        stStatus.fwDevice   = DEVICE_HWERROR;
        stStatus.fwMedia[1] = MEDIA_UNKNOWN;
        stStatus.fwCameras[1] = STATUS_UNKNOWN;
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (m_nRetErrOLD[0] != ERR_OTHER)
        {
            Log(ThisModule, __LINE__, "枚举当前设备上的所有相机: bEnumCameras() != TRUE. Return: %d.", ERR_OTHER);
            m_nRetErrOLD[0] = ERR_OTHER;
        }
        return ERR_OTHER;
    }

    BOOL bIsVisOk = FALSE, bIsNisOk = FALSE;		// 30-00-00-00(FT#0031)
    stCameraTmp = stCameraLists;					// 30-00-00-00(FT#0031)
    if (m_stCamOpenType.wOpenType == CAM_OPEN_DEVIDX)// 30-00-00-00(FT#0031)
    {
        // 按序号检查枚举的相机,是否包含INI中配置的序号, 新增 // 30-00-00-00(FT#0031)
        while(stCameraTmp != nullptr)
        {
            if (bIsVisOk == FALSE)
            {
                if (stCameraTmp->index == m_stCamOpenType.wVisCamIdx)
                {
                    bIsVisOk = TRUE;
                }
            }
            if (bIsNisOk == FALSE)
            {
                if (stCameraTmp->index == m_stCamOpenType.wNisCamIdx)
                {
                    bIsNisOk = TRUE;
                }
            }
            if (bIsVisOk == TRUE && bIsNisOk == TRUE)
            {
                break;
            }
            stCameraTmp = stCameraTmp->next;
        }

        // 按序号(相机使用状态)
        /*if ((bGetCameraStatus(m_stCamCwInitParam.wVisCamIdx, &state0) != TRUE) ||
            (bGetCameraStatus(m_stCamCwInitParam.wNisCamIdx, &state1) != TRUE))
        {
            m_stStatus.fwDevice = DEVICE_HWERROR;
            m_stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            m_stStatus.fwCameras[1] = STATUS_NOTSUPP;
            return ERR_OTHER;
        }*/
    } else
    {
        // 按VID/PID检查枚举的相机,是否包含INI中配置的VID/PID 新增 // 30-00-00-00(FT#0031)
        while(stCameraTmp != nullptr)
        {
            if (bIsVisOk == FALSE)
            {
                if (MCMP(stCameraTmp->vid, m_stCamOpenType.szVisVid) &&
                    MCMP(stCameraTmp->pid, m_stCamOpenType.szVisPid))
                {
                    bIsVisOk = TRUE;
                }
            }
            if (bIsNisOk == FALSE)
            {
                if (MCMP(stCameraTmp->vid, m_stCamOpenType.szNisVid) &&
                    MCMP(stCameraTmp->pid, m_stCamOpenType.szNisPid))
                {
                    bIsNisOk = TRUE;
                }
            }
            if (bIsVisOk == TRUE && bIsNisOk == TRUE)
            {
                break;
            }
            stCameraTmp = stCameraTmp->next;
        }

        // 按VID+PID(相机使用状态)
        /*if ((bGetCameraStatusEx(m_stCamCwInitParam.szVisVid, m_stCamCwInitParam.szVisPid, &state0) != TRUE) ||
            (bGetCameraStatusEx(m_stCamCwInitParam.szNisVid, m_stCamCwInitParam.szNisPid, &state1) != TRUE))
        {
            m_stStatus.fwDevice = DEVICE_HWERROR;
            m_stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            m_stStatus.fwCameras[1] = STATUS_NOTSUPP;
            return ERR_OTHER;
        }*/
    }

    // 可见光/红外光,有一个序号或VID/PID不存在则OFFLINE // 30-00-00-00(FT#0031)
    if (bIsVisOk == FALSE && bIsNisOk == FALSE)
    {
        stStatus.fwDevice = DEVICE_OFFLINE;
        stStatus.fwMedia[1] = MEDIA_UNKNOWN;
        stStatus.fwCameras[1] = MEDIA_UNKNOWN;
    } else
    if ((bIsVisOk == FALSE && bIsNisOk == TRUE) ||
        (bIsVisOk == TRUE && bIsNisOk == FALSE))
    {
        stStatus.fwDevice = DEVICE_OFFLINE;
        stStatus.fwMedia[1] = MEDIA_UNKNOWN;
        stStatus.fwCameras[1] = MEDIA_UNKNOWN;
    } else
    {
        stStatus.fwDevice   = DEVICE_ONLINE;
        stStatus.fwMedia[1] = MEDIA_OK;
        stStatus.fwCameras[1] = STATUS_OK;
    }

    // 相机使用状态分析
    /*switch(state0)
    {
        case CW_CAMERA_STATE_UNKNOWN:    // 0状态未知
        case CW_CAMERA_STATE_UNINIT:     // 1未初始化
        case CW_CAMERA_STATE_STOPPED:    // 4停止
        case CW_CAMERA_STATE_PAUSED:     // 5暂停
        case CW_CAMERA_STATE_LOST:       // 6丢失
            m_stStatus.fwDevice = DEVICE_HWERROR;
            m_stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            m_stStatus.fwCameras[1] = STATUS_NOTSUPP;
        case CW_CAMERA_STATE_INIT:       // 2初始化
        case CW_CAMERA_STATE_OPENED:     // 3打开
            m_stStatus.fwDevice = DEVICE_ONLINE;
            m_stStatus.fwMedia[1] = MEDIA_OK;
            m_stStatus.fwCameras[1] = STATUS_OK;
            break;
        default:
            break;
    }

    switch(state1)
    {
        case CW_CAMERA_STATE_UNKNOWN:    // 0状态未知
        case CW_CAMERA_STATE_UNINIT:     // 1未初始化
        case CW_CAMERA_STATE_STOPPED:    // 4停止
        case CW_CAMERA_STATE_PAUSED:     // 5暂停
        case CW_CAMERA_STATE_LOST:       // 6丢失
            m_stStatus.fwDevice = DEVICE_HWERROR;
            m_stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            m_stStatus.fwCameras[1] = STATUS_NOTSUPP;
        case CW_CAMERA_STATE_INIT:       // 2初始化
        case CW_CAMERA_STATE_OPENED:     // 3打开
            m_stStatus.fwDevice = DEVICE_ONLINE;
            m_stStatus.fwMedia[1] = MEDIA_OK;
            m_stStatus.fwCameras[1] = STATUS_OK;
            break;
        default:
            break;
    }*/

    //UpdateStatus(DEVICE_ONLINE, "000");

    // 比较两次状态记录LOG
    if (memcmp(&m_DevStatusOLD, &stStatus, sizeof(DEVCAMSTATUS)) != 0)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|Media[1]:%d->%d%s|Cameras[1]:%d->%d%s|",
            m_DevStatusOLD.fwDevice, stStatus.fwDevice, (m_DevStatusOLD.fwDevice != stStatus.fwDevice ? " *" : ""),
            m_DevStatusOLD.fwMedia[1], stStatus.fwMedia[1], (m_DevStatusOLD.fwMedia[1] != stStatus.fwMedia[1]? " *" : ""),
            m_DevStatusOLD.fwCameras[1], stStatus.fwCameras[1], (m_DevStatusOLD.fwCameras[1] != stStatus.fwCameras[1] ? " *" : "")
            );
        memcpy(&m_DevStatusOLD, &stStatus, sizeof(DEVCAMSTATUS));
    }
    return CAM_SUCCESS;
}

// 打开窗口(窗口句柄，动作指示:1创建窗口/0销毁窗口, X/Y坐标,窗口宽高)
long CDevCAM_CloudWalk::Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight)
{
    CWCameraState stCamStatVis, stCamStatNis;

    if (wAction == CAMERA_OPEN)
    {
        if (m_bDisplayOpenOK == TRUE)   // Display已打开窗口，直接return
        {
            return CAM_SUCCESS;
        }

        if (m_stCamOpenType.wOpenType == CAM_OPEN_DEVIDX)
        {
            /*memset(&stCamStatVis, 0x00, sizeof(CWCameraState));
            memset(&stCamStatNis, 0x00, sizeof(CWCameraState));
            if (bGetCameraStatus(wVisCamIdx, &stCamStatVis) != TRUE ||
                bGetCameraStatus(wNisCamIdx, &stCamStatNis) != TRUE)
            {
                return ERR_OTHER;
            }
            if (stCamStatVis == CW_CAMERA_STATE_OPENED &&
                stCamStatNis == CW_CAMERA_STATE_OPENED)
            {
                return CAM_SUCCESS;
            }*/

            // 按序号开启相机
            if (bOpenCamera(m_stCamOpenType.wVisCamIdx, 0, wWidth, wHeight) != TRUE) //
            {
                return ERR_OTHER;
            }
            if (bOpenCamera(m_stCamOpenType.wNisCamIdx, 1, wWidth, wHeight) != TRUE)
            {
                return ERR_OTHER;
            }
        } else
        {
            /*memset(&stCamStatVis, 0x00, sizeof(CWCameraState));
            memset(&stCamStatNis, 0x00, sizeof(CWCameraState));
            if (bGetCameraStatusEx(m_pVisVid, m_pVisPid, &stCamStatVis) != TRUE ||
                bGetCameraStatusEx(m_pNisVid, m_pNisPid, &stCamStatNis) != TRUE)
            {
                return ERR_OTHER;
            }
            if (stCamStatVis == CW_CAMERA_STATE_OPENED &&
                stCamStatNis == CW_CAMERA_STATE_OPENED)
            {
                return CAM_SUCCESS;
            }*/
            // 开启相机(按VID/PID)
            if (bOpenCameraEx(m_stCamOpenType.szVisVid, m_stCamOpenType.szVisPid, 0, wWidth, wHeight) != TRUE)
            {
                return ERR_OTHER;
            }            
            if (bOpenCameraEx(m_stCamOpenType.szNisVid, m_stCamOpenType.szNisPid, 1, wWidth, wHeight) != TRUE)
            {
                return ERR_OTHER;
            }
        }

        // 开始活体检测
        //if (bStartLiveDetect(TRUE) != TRUE)
        //{
        //    return ERR_OTHER;
        //}

        m_bDisplayOpenOK = TRUE;

    } else
    {
        // 关闭所有相机
        if (bCloseAllCameras() != TRUE)
        {
            return ERR_OTHER;
        }
        // 停止连续活体检测
        bStopLiveDetect();

        m_bDisplayOpenOK = FALSE;
    }

    return CAM_SUCCESS;
}

long CDevCAM_CloudWalk::TakePicture(WORD wCamera, LPCSTR lpData)
{
    return CAM_SUCCESS;
}

long CDevCAM_CloudWalk::TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, WORD wTimeOut)
{
    m_wSaveFileType = wPicType;
    memset(m_pSaveFileName, 0x00, sizeof(m_pSaveFileName));
    sprintf(m_pSaveFileName, "%s", lpFileName);
    m_bIsTakePicExStop = FALSE;

    // 活检前确认
    bStopLiveDetect();

    // 开始活体检测
    if (bStartLiveDetect(isContinue) != TRUE)
    {
        return ERR_OTHER;
    }

    // 根据超时时间，循环验证TakePic是否结束
    WORD wTimeCount = 0;
    while(1)
    {
        QCoreApplication::processEvents();
        if (wTimeCount >= wTimeOut * 1000 && m_bIsTakePicExStop != TRUE)
        {
            return ERR_TIMEOUT;
        }

        if (m_bIsTakePicExStop == TRUE)
        {
            break;
        }

        usleep(1000 * 30);
        wTimeCount += 30;
    }

    // 停止连续活体检测
    bStopLiveDetect();

    return CAM_SUCCESS;
}

long CDevCAM_CloudWalk::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT:
        {
            memcpy(&m_stCamOpenType,
                   &(((LPST_CAM_DEV_INIT_PARAM)vInitPar)->stCamOpenType),
                   sizeof(ST_CAM_OPENTYPE));
            memcpy(&m_stCamCwInitParam,
                   &(((LPST_CAM_DEV_INIT_PARAM)vInitPar)->stCamCwInitParam),
                   sizeof(ST_CAM_CW_INIT_PARAM));
            memcpy(&m_stCamSaveImageCfg,
                   &(((LPST_CAM_DEV_INIT_PARAM)vInitPar)->stCamSaveImageCfg),
                   sizeof(ST_CAM_SAVEIMAGE_CFG));
            memcpy(m_szSharedDataName,
                    ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->szCamDataSMemName,
                   strlen(((LPST_CAM_DEV_INIT_PARAM)vInitPar)->szCamDataSMemName));
            m_ulSharedDataSize = ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->ulCamDataSMemSize;
            break;
        }
        default:
            break;
    }

    return CAM_SUCCESS;
}

long CDevCAM_CloudWalk::GetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT:
            break;
        default:
            break;
    }

    return CAM_SUCCESS;
}

// 版本号(1DevCam版本/2固件版本/3其他)
void CDevCAM_CloudWalk::GetVersion(char* szVer, long lSize, ushort usType)
{
    if (usType == 1)
    {
        memcpy(szVer, byDevVRTU, strlen((char*)byDevVRTU) > lSize ? lSize : strlen((char*)byDevVRTU));
    }
    if (usType == 2)
    {
        if (m_stCamOpenType.wOpenType == 0)
        {
            if (bGetFirmwareVersion(m_stCamOpenType.wVisCamIdx, szVer + strlen(szVer)) == TRUE)
            {
                if (strlen(szVer) > 0)
                {
                    sprintf(szVer, "%s", " ; ");
                }
            }

            bGetFirmwareVersion(m_stCamOpenType.wNisCamIdx, szVer + strlen(szVer));
        }

        if (m_stCamOpenType.wOpenType == 1)
        {
            if (bGetFirmwareVersionEx(m_stCamOpenType.szVisVid, m_stCamOpenType.szVisPid, szVer + strlen(szVer)) == TRUE)
            {
                if (strlen(szVer) > 0)
                {
                    sprintf(szVer, "%s", " ; ");
                }
            }
            bGetFirmwareVersionEx(m_stCamOpenType.szNisVid, m_stCamOpenType.szNisPid, szVer + strlen(szVer));
        }
    }
    if (usType == 3)
    {
        bGetVersion(szVer, lSize);
    }
}

// 绘制图像框
void CDevCAM_CloudWalk::preview(const uchar* data, int width, int height, int channels, int type)
{
    cv::Mat mat(height, width, CV_8UC3, (char*)data);
    cv::cvtColor(mat, mat, CV_BGR2RGB);

    if (m_qSharedMemData->isAttached())
    {
        m_qSharedMemData->lock();

        int w, h, c;
        unsigned char* ucTmp = nullptr;

        w = mat.cols;
        h = mat.rows;
        c = mat.channels();

        //ucTmp = new unsigned char(w * h * c + 5 + 1);
        ucTmp = (unsigned char *)malloc(sizeof(unsigned char) * ((w * h * c + 5 + 1)));
        if (ucTmp == nullptr)
        {
            ucTmp = (unsigned char *)malloc(sizeof(unsigned char) * ((w * h * c + 5 + 1)));
            if (ucTmp == nullptr)
            {
                ucTmp = (unsigned char *)malloc(sizeof(unsigned char) * ((w * h * c + 5 + 1)));
                if (ucTmp == nullptr)
                {
                    return;
                }
            }
        }
        memset(ucTmp, 0x00, sizeof(w * h * c + 5 + 1));

        sprintf((char*)ucTmp, "%c%c%c%c%c", w / 255, w % 255, h / 255, h % 255, c);
        memcpy(ucTmp + 5, mat.data, w * h * c);
        memcpy(m_qSharedMemData->data(), ucTmp, w * h * c + 5);
        //delete ucTmp;
        if (ucTmp != nullptr)
        {
            free(ucTmp);
        }
        ucTmp = nullptr;
        m_qSharedMemData->unlock();
    }
}

//
void CDevCAM_CloudWalk::showFace(const long error_code, const long timestamp, const CWLiveFaceDetectInformation_t* pFaceInformations, int nFaceNumber)
{

}

// 活体检测结果获取
void CDevCAM_CloudWalk::showLive(int errCode, const float* scores, const float* distances, int nFaceNumber)
{
    if (m_bIsTakePicExStop == TRUE)
    {
        return;
    }

    // 检测到活体，保存图片文件并关闭活体检测
    if(errCode == CW_FACE_LIVENESS_IS_LIVE) // 检测到活体
    {
        Log(ThisFile, 1, "showLive()->检测到活体,SavePicType=%d...", m_wSaveFileType);
        CWLiveImage live = { 0x00 };
        LONG code = bGetBestFace(2.0f, FALSE/*T指定Base64编码格式输出*/, &live);
        if (code == TRUE)
        {
            Log(ThisFile, 1, "showLive()->bGetBestFace()获取人脸数据成功");

            CHAR szFileNameJPG[MAX_PATH] = { 0x00 };
            CHAR szFileNameBASE64[MAX_PATH] = { 0x00 };
            CHAR szFileNameBMP[MAX_PATH] = { 0x00 };
            BOOL bSaveJPG = FALSE;
            BOOL bSaveBASE64 = FALSE;
            BOOL bSaveBMP = FALSE;

            if ((m_wSaveFileType & PIC_JPG) == PIC_JPG ||
                (m_wSaveFileType & PIC_BASE64) == PIC_BASE64)
            {
                sprintf(szFileNameJPG, "%s.JPG", m_pSaveFileName);

                cv::Mat mat(live.height, live.width, CV_8UC3, live.data);
                if (cv::imwrite(szFileNameJPG, mat) == TRUE)
                {
                    Log(ThisFile, 1, "showLive()->生成图片[%s]成功", szFileNameJPG);
                    bSaveJPG = TRUE;

                    if ((m_wSaveFileType & PIC_BASE64) == PIC_BASE64)
                    {
                        sprintf(szFileNameBASE64, "%s.BASE64", m_pSaveFileName);
                        std::string szBase64;
                        szBase64.clear();

                        if (bImage2Base64Format(szFileNameJPG, &szBase64) == TRUE)
                        {
                            if (szBase64.size() < 1)
                            {
                                Log(ThisFile, 1, "showLive()->bImage2Base64Format(): [%s]转BASE64 Data失败，返回data<1", szFileNameJPG);
                                bSaveBASE64 = FALSE;
                            } else
                            {
                                Log(ThisFile, 1, "showLive()->bImage2Base64Format(): [%s]转BASE64 Data成功", szFileNameJPG);
                                if (bWriteFile(szFileNameBASE64, (char*)szBase64.c_str(), szBase64.size()) != TRUE)
                                {
                                    Log(ThisFile, 1, "showLive()->bWriteFile() BASE64数据写入[%s]失败, errno: %d|%s",
                                                      szFileNameBASE64, errno, strerror(errno));
                                    bSaveBASE64 = FALSE;
                                } else
                                {
                                    bSaveBASE64 = TRUE;
                                }
                            }
                        } else
                        {
                            Log(ThisFile, 1, "showLive()->bImage2Base64Format(): [%s]转BASE64 Data失败", szFileNameJPG);
                            bSaveBASE64 = FALSE;
                        }
                    }
                } else
                {
                    Log(ThisFile, 1, "showLive()->生成图片[%s]失败", szFileNameJPG);
                    bSaveJPG = FALSE;
                }
            }

            if ((m_wSaveFileType & PIC_BMP) == PIC_BMP)
            {
                sprintf(szFileNameBMP, "%s.BMP", m_pSaveFileName);

                cv::Mat mat(live.height, live.width, CV_8UC3, live.data);
                if (cv::imwrite(szFileNameBMP, mat) == TRUE)
                {
                    Log(ThisFile, 1, "showLive()->生成图片[%s]成功", szFileNameBMP);
                    bSaveBMP = TRUE;
                } else
                {
                    Log(ThisFile, 1, "showLive()->生成图片[%s]失败", szFileNameBMP);
                    bSaveBMP = TRUE;
                }
            }

            if (((m_wSaveFileType & PIC_BASE64) == PIC_BASE64 && bSaveBASE64 == FALSE) ||
                ((m_wSaveFileType & PIC_JPG) == PIC_JPG && bSaveJPG == FALSE) ||
                ((m_wSaveFileType & PIC_BMP) == PIC_BMP && bSaveBMP == FALSE))
            {
                return;
            } else
            {
                if (m_stCamSaveImageCfg.bIsNeedDailyRecord == TRUE)
                {
                    vWriteImageInfo(m_pSaveFileName);
                }
                m_bIsTakePicExStop = TRUE;
            }
        }
        else
        {
            Log(ThisFile, 1, "showLive()->bGetBestFace()获取人脸数据失败");
            return;
        }
    }
}

void CDevCAM_CloudWalk::vWriteImageInfo(LPSTR lpImageFile)
{
    WORD wLevel4_Count = 0;
    WORD wLevel3_Count = 0;
    WORD wLevel2_Count = 0;
    CHAR cAppName[128] = { 0x00 };
    CHAR cKeyName[128] = { 0x00 };

    CHAR cDirDate[8+1] = { 0x00 };
    CHAR cDirTime[6+1] = { 0x00 };
    CHAR cDirDateTime[30] = { 0x00 };

    CHAR cImageSaveName[MAX_PATH] = { 0x00 };
    CHAR cImageInfoSaveName[MAX_PATH] = { 0x00 };

    BOOL bWriteFlag = FALSE;

    CHAR cInfoData[MAX_PATH] = { 0x00 };
    CHAR cSourceImageFile[MAX_PATH] = { 0x00 };


    time_t		time_tmp = time(NULL);
    struct tm*	tm_now = localtime(&time_tmp);
    sprintf(cDirDate, "%04d%02d%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
    sprintf(cDirTime, "%02d%02d%02d", tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
    sprintf(cDirDateTime,"%04d-%02d-%02d %02d:%02d:%02d",
            tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

    // 循环验证 写 CameraInfo
    for (int i = 0; i < 3; i ++)
    {
        memset(cSourceImageFile, 0x00, sizeof(cSourceImageFile));
        memset(cImageSaveName, 0x00, sizeof(cImageSaveName));
        memset(cImageInfoSaveName, 0x00, sizeof(cImageInfoSaveName));
        if ((m_wSaveFileType & PIC_BASE64) == PIC_BASE64 && i == 0)
        {	// 生成BASE64
            sprintf(cSourceImageFile, "%s.BASE64", lpImageFile);
            sprintf(cImageSaveName, "%s/%s/%s.BASE64",
                    m_stCamSaveImageCfg.byImageSavePathBASE64, cDirDate, cDirTime);
            bWriteFlag = FALSE;
        } else
        if ((m_wSaveFileType & PIC_JPG) == PIC_JPG && i == 1)
        {	// 生成JPG
            sprintf(cSourceImageFile, "%s.JPG", lpImageFile);
            sprintf(cImageSaveName, "%s/%s/%s.JPG",
                    m_stCamSaveImageCfg.byImageSavePathJPG, cDirDate, cDirTime);
            bWriteFlag = TRUE;
        } else
        if ((m_wSaveFileType & PIC_BMP) == PIC_BMP && i == 2)
        {	// 生成BMP
            sprintf(cSourceImageFile, "%s.BMP", lpImageFile);
            sprintf(cImageSaveName, "%s/%s/%s.BMP",
                    m_stCamSaveImageCfg.byImageSavePathBMP, cDirDate, cDirTime);
            bWriteFlag = TRUE;
        } else {
            continue;
        }

//        sprintf(cImageInfoSaveName, "%s/CamImage_%s.txt",
//                m_stCamSaveImageCfg.byImageSaveInfoPath, cDirDate);
        sprintf(cImageInfoSaveName, "%s/%s/CamImage_%s.txt",
                m_stCamSaveImageCfg.byImageSaveInfoPath, cDirDate, cDirDate);
        // 备份目录(指定YYYYMMDD)不存在则新建；3次不成功，返回
        if (bPathCheckAndCreate((LPSTR)cImageSaveName, FALSE) == FALSE) {
            if (bPathCheckAndCreate((LPSTR)cImageSaveName, FALSE) == FALSE) {
                if (bPathCheckAndCreate((LPSTR)cImageSaveName, FALSE) == FALSE) {
                    Log(ThisFile, 1, "vWriteImageInfo()->bPathCheckAndCreate()目录[%s]建立失败, errno: %d|%s",
                        cImageSaveName, errno, strerror(errno));
                    return;
                }
            }
        }

        // TakePic 生成的图片文件 Copy 到备份目录下,文件名为YYYYMMDDHHmmss；Copy 3次失败，返回
        if (QFile::copy(cSourceImageFile, cImageSaveName) != TRUE)
        {
            Log(ThisFile, 1, "vWriteImageInfo()->copy[%s->%s]失败", cSourceImageFile, cImageSaveName);
            return;
        }

        // 写摄像备份信息文件
        if (bWriteFlag == TRUE)
        {
            bWriteFlag = FALSE;
            // 写摄像备份信息文件
            memset(cInfoData, 0x00, sizeof(cInfoData));
            sprintf(cInfoData, "0,0,%s,0,0,0|%s;0\n", cImageSaveName, cDirTime);
            if (bWriteFile(cImageInfoSaveName, cInfoData, strlen(cInfoData)) != TRUE)
            {
                Log(ThisFile, 1, "vWriteImageInfo()->写[%s][%s]失败, errno: %d|%s",
                            cImageInfoSaveName, cInfoData, errno, strerror(errno));
                return;
            }


            // 通用重复写 Camera Info 文件路径 不存在则新建
            if (bPathCheckAndCreate((LPSTR)m_stCamSaveImageCfg.byImageInfoFile, FALSE) == FALSE) {
                Log(ThisFile, 1, "vWriteImageInfo()->bPathCheckAndCreate()目录[%s]建立失败, errno: %d|%s",
                            m_stCamSaveImageCfg.byImageInfoFile, errno, strerror(errno));
                return;
            } else
            {
                // 写 通用重复写 Camera Info 文件，只保留最近一次 TakePic 生成图片信息
                QSettings qWCameraInfo(m_stCamSaveImageCfg.byImageInfoFile, QSettings::IniFormat);
                qWCameraInfo.setValue("/Camera_info/LEVEL4_COUNT", wLevel4_Count + 1);
                qWCameraInfo.setValue("/Camera_info/LEVEL3_COUNT", wLevel3_Count);
                qWCameraInfo.setValue("/Camera_info/LEVEL2_COUNT", wLevel2_Count);
                qWCameraInfo.setValue("/Camera_info/OperationTime", cDirDateTime);

                memset(cAppName, 0x00, sizeof(cAppName));
                sprintf(cAppName, "/LEVEL4_%03d/Index", wLevel4_Count + 1);
                qWCameraInfo.setValue(cAppName, 0);

                memset(cAppName, 0x00, sizeof(cAppName));
                sprintf(cAppName, "/LEVEL4_%03d/Value", wLevel4_Count + 1);
                qWCameraInfo.setValue(cAppName, 0);

                memset(cAppName, 0x00, sizeof(cAppName));
                sprintf(cAppName, "/LEVEL4_%03d/SerialNumber", wLevel4_Count + 1);
                qWCameraInfo.setValue(cAppName, 0);
                memset(cAppName, 0x00, sizeof(cAppName));
                sprintf(cAppName, "/LEVEL4_%03d/ImageFile", wLevel4_Count + 1);
                qWCameraInfo.setValue(cAppName, cImageSaveName);
                memset(cAppName, 0x00, sizeof(cAppName));
                sprintf(cAppName, "/LEVEL4_%03d/NoteVersion", wLevel4_Count + 1);
                qWCameraInfo.setValue(cAppName, 0);
                memset(cAppName, 0x00, sizeof(cAppName));
                sprintf(cAppName, "/LEVEL4_%03d/Currency", wLevel4_Count + 1);
                qWCameraInfo.setValue(cAppName, 0);
            }
        }
    }

    return;
}

BOOL CDevCAM_CloudWalk::bWriteFile(LPSTR szPath, LPSTR szData, int iDataSize)
{
    if(iDataSize == 0){
        return TRUE;
    }

    if (bPathCheckAndCreate(szPath, FALSE) == FALSE)
        return FALSE;

    FILE *fp = NULL;
    if ((fp = fopen(szPath, "a+")) == NULL)
    {
        return FALSE;
    }
    //fseek(fp, 0, SEEK_END);
    fwrite(szData, 1, iDataSize, fp);
    fclose(fp);

    return TRUE;
}

// lpspath: 绝对路径； bFolder:是否不包含文件名的目录路径
BOOL CDevCAM_CloudWalk::bPathCheckAndCreate(LPSTR lpsPath, BOOL bFolder)
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

BOOL CDevCAM_CloudWalk::bConnSharedMemory(LPSTR lpSharedName)
{

    m_qSharedMemData = new QSharedMemory(lpSharedName);
    if (m_qSharedMemData == nullptr)
    {
        Log(ThisFile, 1, "bConnSharedMemory()->创建共享内存[%s]实例失败", lpSharedName);
        return FALSE;
    }
    if (m_qSharedMemData->attach(QSharedMemory::ReadWrite) != true)    // 读写方式关联
    {
        if (m_qSharedMemData->isAttached() != true)   // 判断共享内存的关联状态
        {
            Log(ThisFile, 1, "bConnSharedMemory()->关联共享内存[%s]无法关联", lpSharedName);
            return FALSE;
        }
    }

    if (m_qSharedMemData->size() < 1)
    {
        Log(ThisFile, 1, "bConnSharedMemory()->关联共享内存[%s] size[%s] < 1",
                          m_qSharedMemData->size(), lpSharedName);
        return FALSE;
    }

    return TRUE;
}

void CDevCAM_CloudWalk::bDisConnSharedMemory()
{
    if (m_qSharedMemData != nullptr)
    {
        if (m_qSharedMemData->isAttached())   // 检测程序当前是否关联共享内存
            m_qSharedMemData->detach();      // 解除关联
        delete m_qSharedMemData;
        m_qSharedMemData = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////

// 视频流显示回调，已绘制好人脸框
void onPreviewCallback(void* handle, const short mode, const char* frame,
                       const short width, const short height, const short channel)
{
    if(nullptr == handle || 1 == mode)
    {
        return;
    }

    auto Ptr = CvtPointer(handle, CDevCAM_CloudWalk);
    if(nullptr == Ptr) {return ;}
    Ptr->preview((const uchar*)frame, width, height, channel);
}

// 人脸检测回调
void onFaceDetCallback(void* handle, const long error_code, const long timestamp,
                       const CWLiveFaceDetectInformation* infos, const short face_num)
{
    if(nullptr == handle)
    {
        return ;
    }

    auto Ptr = CvtPointer(handle, CDevCAM_CloudWalk);
    if(nullptr == Ptr)
    {
       return ;
    }
    Ptr->showFace(error_code, timestamp, infos, face_num);
}

// 活体检测回调
void onLiveDetCallback(void* handle, const long error_code, const float* scores,
                       const float* distances, const short face_num)
{
    if(nullptr == handle)
    {
        return ;
    }

    auto Ptr = CvtPointer(handle, CDevCAM_CloudWalk);
    if(nullptr == Ptr){
        return ;
    }

    Ptr->showLive(error_code, scores, distances, face_num);
}


//////////////////////////////////////////////////////////////////////////






