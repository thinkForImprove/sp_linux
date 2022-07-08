#include "DevCAM_CloudWalk.h"
#include "opencv2/opencv.hpp"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QSettings>
#include <sys/stat.h>

#include <dirent.h>

// CAM 版本号
//BYTE    byDevVRTU[17] = {"DevCAM00000100"};

static const char *ThisFile = "DevCAM_CloudWalk.cpp";

//////////////////////////////////////////////////////////////////////////

CDevCAM_CloudWalk::CDevCAM_CloudWalk() : CDevImpl_CloudWalk(LOG_NAME_DEVCAM)
{
    SetLogFile(LOG_NAME_DEVCAM, ThisFile);  // 设置日志文件名和错误发生的文件

    memset(&m_stStatus, 0x00, sizeof(DEVCAMSTATUS));
    memset(&m_stCamSaveImagePab, 0x00, sizeof(ST_CAM_SAVEIMAGE_PAB));

    memset(m_pSaveFileName, 0x00, sizeof(m_pSaveFileName));// 人脸摄像保存文件名
    m_wSaveFileType = PIC_JPG;
    m_bIsTakePicExStop = FALSE;

    m_bDisplayOpenOK = FALSE;     // 未打开
    m_bCancel = FALSE;		// 30-00-00-00(FT#0031)

    m_qSharedMemData = nullptr;

    memset(m_szSharedDataName, 0x00, sizeof(m_szSharedDataName));
    m_ulSharedDataSize = 0;

    m_wTakePicMode = TAKEPIC_PERSON;                     // TakePic摄像模式(缺省人脸)
    m_wTakePicMode_Set = 0x00;
    memset(m_pSaveFileNameRoom, 0x00, sizeof(m_pSaveFileNameRoom)); // 全景摄像保存文件名
    m_stDisplayReso.width = 640;
    m_stDisplayReso.height = 480;

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

    // 建立CloudWalk检测对象	
    if (IsDeviceOpen() != TRUE)
    {
        if (OpenDevice() != TRUE)
        {
            return ERR_OPEN_CAMER;
        }
    }

    // 检查设备是否链接
    DEVCAMSTATUS stStatus;						// 30-00-00-00(FT#0031)
    GetStatus(stStatus);						// 30-00-00-00(FT#0031)
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

    // 语音提示开启 新增 // 30-00-00-00(FT#0031)
    if (m_stCamCwInitParam.bIsVoiceSupp == TRUE && strlen(m_stCamCwInitParam.szVoiceTipFile) > 0)
    {
        bEnableVoiceTip(TRUE, m_stCamCwInitParam.szVoiceTipFile);
    } else
    {
        bEnableVoiceTip(FALSE, nullptr);
    }

    // 枚举当前设备上的相机所支持的分辨率并记录Log
    bGetbEnumReso();

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

    // 关闭所有相机
    bCloseAllCameras();		// 30-00-00-00(FT#0031)
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
        return ERR_OPEN_CAMER;
    }

    // 枚举当前设备上的所有相机 新增 // 30-00-00-00(FT#0031)
    CWCameraDevice *stCameraLists = nullptr, *stCameraTmp = nullptr;
    if (bEnumCameras(&stCameraLists) != TRUE)
    {
        stStatus.fwDevice   = DEVICE_HWERROR;
        stStatus.fwMedia[1] = MEDIA_UNKNOWN;
        stStatus.fwCameras[1] = STATUS_UNKNOWN;
        return ERR_OTHER;
    }

    BOOL bIsVisOk = FALSE, bIsNisOk = FALSE;		// 30-00-00-00(FT#0031)
    stCameraTmp = stCameraLists;					// 30-00-00-00(FT#0031)
    if (m_stCamCwInitParam.wOpenType == CAM_OPEN_DEVIDX)// 30-00-00-00(FT#0031)
    {
        // 按序号检查枚举的相机,是否包含INI中配置的序号, 新增 // 30-00-00-00(FT#0031)
        while(stCameraTmp != nullptr)
        {
            if (bIsVisOk == FALSE)
            {
                if (stCameraTmp->index == m_stCamCwInitParam.wVisCamIdx)
                {
                    bIsVisOk = TRUE;
                }
            }
            if (bIsNisOk == FALSE)
            {
                if (stCameraTmp->index == m_stCamCwInitParam.wNisCamIdx)
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
                if (MCMP(stCameraTmp->vid, m_stCamCwInitParam.szVisVid) &&
                    MCMP(stCameraTmp->pid, m_stCamCwInitParam.szVisPid))
                {
                    bIsVisOk = TRUE;
                }
            }
            if (bIsNisOk == FALSE)
            {
                if (MCMP(stCameraTmp->vid, m_stCamCwInitParam.szNisVid) &&
                    MCMP(stCameraTmp->pid, m_stCamCwInitParam.szNisPid))
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
        stStatus.fwDevice = DEVICE_NODEVICE;
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
    return CAM_SUCCESS;
}

// 命令取消 新增 // 30-00-00-00(FT#0031)
long CDevCAM_CloudWalk::Cancel()
{
    m_bCancel = TRUE;
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

        if (m_stCamCwInitParam.wOpenType == CAM_OPEN_DEVIDX)
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
            if (bOpenCamera(m_stCamCwInitParam.wVisCamIdx, 0, wWidth, wHeight) != TRUE) //
            {
                return ERR_OTHER;
            }
            if (bOpenCamera(m_stCamCwInitParam.wNisCamIdx, 1, wWidth, wHeight) != TRUE)
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
            if (bOpenCameraEx(m_stCamCwInitParam.szVisVid, m_stCamCwInitParam.szVisPid, 0, wWidth, wHeight) != TRUE)
            {
                return ERR_OTHER;
            }            
            if (bOpenCameraEx(m_stCamCwInitParam.szNisVid, m_stCamCwInitParam.szNisPid, 1, wWidth, wHeight) != TRUE)
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

        m_stDisplayReso.width = wWidth;
        m_stDisplayReso.height = wHeight;

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

long CDevCAM_CloudWalk::TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, DWORD dwTimeOut, WORD wCamara)
{
    if (wCamara == TAKEPIC_FILE)    // 资料补拍(保证与全景/人脸无交集)
    {
        m_wTakePicMode = TAKEPIC_FILE;
        memset(m_pSaveFileName, 0x00, sizeof(m_pSaveFileName));
        sprintf(m_pSaveFileName, "%s", lpFileName);
    } else
    {
        if (wCamara == TAKEPIC_ROOM)    // 全景摄像
        {
            memset(m_pSaveFileNameRoom, 0x00, sizeof(m_pSaveFileNameRoom));
            sprintf(m_pSaveFileNameRoom, "%s", lpFileName);
            m_wTakePicMode = TAKEPIC_ROOM;
        } else
        {
            memset(m_pSaveFileName, 0x00, sizeof(m_pSaveFileName));
            sprintf(m_pSaveFileName, "%s", lpFileName);
            m_wTakePicMode = TAKEPIC_PERSON;    // 摄像模式缺省人脸
        }

        m_wTakePicMode = (m_wTakePicMode | m_wTakePicMode_Set);
    }

    m_wSaveFileType = wPicType;
    m_bIsTakePicExStop = FALSE;
    m_bCancel = FALSE;		// 30-00-00-00(FT#0031)

    // 活检前确认
    bStopLiveDetect();

    // 开始活体检测
    if (bStartLiveDetect(isContinue) != TRUE)
    {
        return ERR_OTHER;
    }

    // 根据超时时间，循环验证TakePic是否结束
    DWORD dwTimeCount = 0;
    while(1)
    {
        QCoreApplication::processEvents();
        if (dwTimeOut > 0)   // >0有超时;否则不超时
        {
            if (dwTimeCount >= dwTimeOut && m_bIsTakePicExStop != TRUE)
            {                
                bStopLiveDetect();// 停止连续活体检测
                return ERR_TIMEOUT;
            }
        }

        if (m_bIsTakePicExStop == TRUE)
        {
            break;
        }

        if (m_bCancel == TRUE)  // 命令取消			// 30-00-00-00(FT#0031)
        {
            bStopLiveDetect();// 停止连续活体检测	// 30-00-00-00(FT#0031)
            return ERR_CANCEL;						// 30-00-00-00(FT#0031)
        }

        usleep(1000 * 30);
        dwTimeCount += 30;
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
            memcpy(&m_stCamCwInitParam,
                   &(((LPST_CAM_DEV_INIT_PARAM)vInitPar)->stCamCwInitParam),
                   sizeof(ST_CAM_CW_INIT_PARAM));
            memcpy(&m_stCamSaveImagePab,
                   &(((LPST_CAM_DEV_INIT_PARAM)vInitPar)->stCamSaveImagePab),
                   sizeof(ST_CAM_SAVEIMAGE_PAB));
            memcpy(m_szSharedDataName,
                    ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->szCamDataSMemName,
                   strlen(((LPST_CAM_DEV_INIT_PARAM)vInitPar)->szCamDataSMemName));
            m_ulSharedDataSize = ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->ulCamDataSMemSize;
            wBankNo = ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->wBank;
            m_TakePicFileReso.width = ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->stCamCwInitParam.shResoWidth;
            m_TakePicFileReso.height = ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->stCamCwInitParam.shResoHeight;
            break;
        }
        case DATATYPE_PERSON:   // 增加人脸摄像处理
        {
            memset(m_pSaveFileName, 0x00, sizeof(m_pSaveFileName));
            sprintf(m_pSaveFileName, "%s", (char*)vInitPar);
            m_wTakePicMode_Set = TAKEPIC_PERSON;    // 摄像模式缺省人脸
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
        if (m_stCamCwInitParam.wOpenType == 0)
        {
            if (bGetFirmwareVersion(m_stCamCwInitParam.wVisCamIdx, szVer + strlen(szVer)) == TRUE)
            {
                if (strlen(szVer) > 0)
                {
                    sprintf(szVer, "%s", " ; ");
                }
            }

            bGetFirmwareVersion(m_stCamCwInitParam.wNisCamIdx, szVer + strlen(szVer));
        }

        if (m_stCamCwInitParam.wOpenType == 1)
        {
            if (bGetFirmwareVersionEx(m_stCamCwInitParam.szVisVid, m_stCamCwInitParam.szVisPid, szVer + strlen(szVer)) == TRUE)
            {
                if (strlen(szVer) > 0)
                {
                    sprintf(szVer, "%s", " ; ");
                }
            }
            bGetFirmwareVersionEx(m_stCamCwInitParam.szNisVid, m_stCamCwInitParam.szNisPid, szVer + strlen(szVer));
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

    if (m_wTakePicMode == TAKEPIC_FILE)    // 资料补拍
    {
        Log(ThisFile, 1, "showLive()->未检测到活体:资料补拍,SavePicType=%d|%d|%d...",
            m_wSaveFileType, m_TakePicFileReso.width, m_TakePicFileReso.height);
        LONG code;
        CWLiveImage live_FILE = { 0x00 };
        BOOL bRet = FALSE;

        INT nCount = 0;
        while(true)
        {
            code = bCaptureCurrentFrame(/*m_stDisplayReso*/m_TakePicFileReso, FALSE/*T指定Base64编码格式输出*/, &live_FILE);
            if (code == TRUE)
            {
                break;
            } else
            if (code != TRUE && nCount >= 3)
            {
                Log(ThisFile, 1, "showLive()->bCaptureCurrentFrame()3次获取当前帧(资料补拍)失败");
                m_bIsTakePicExStop = TRUE;
                return;
            }
            nCount ++;
        }
        if (showLiveToImage(live_FILE, m_pSaveFileName, m_wSaveFileType) == TRUE)
        {
            m_bIsTakePicExStop = TRUE;
        }
    }

    // 检测到活体，保存图片文件并关闭活体检测
    if(errCode == CW_FACE_LIVENESS_IS_LIVE) // 检测到活体
    {
        Log(ThisFile, 1, "showLive()->检测到活体,SavePicType=%d...", m_wSaveFileType);

        LONG code;
        CWLiveImage live = { 0x00 };        // 人脸摄像数据
        CWLiveImage live_ROOM = { 0x00 };   // 全景摄像数据
        BOOL bRet = FALSE;

        if ((m_wTakePicMode & TAKEPIC_PERSON) == TAKEPIC_PERSON)    // 人脸摄像
        {
            code = bGetBestFace(2.0f, FALSE/*T指定Base64编码格式输出*/, &live);
            if (code == TRUE)
            {
                Log(ThisFile, 1, "showLive()->bGetBestFace()获取人脸数据成功");
            } else
            {
                Log(ThisFile, 1, "showLive()->bGetBestFace()获取人脸数据失败");
                return;
            }

            bRet = showLiveToImage(live, m_pSaveFileName, m_wSaveFileType);
        }

        if ((m_wTakePicMode & TAKEPIC_ROOM) == TAKEPIC_ROOM)    // 全景摄像
        {
            code = bCaptureCurrentFrame(m_stDisplayReso, FALSE/*T指定Base64编码格式输出*/, &live_ROOM);
            if (code == TRUE)
            {
                Log(ThisFile, 1, "showLive()->bCaptureCurrentFrame()获取全景数据成功");
            } else
            {
                Log(ThisFile, 1, "showLive()->bCaptureCurrentFrame()获取全景数据失败");
                return;
            }

            bRet = showLiveToImage(live_ROOM, m_pSaveFileNameRoom, m_wSaveFileType);
        }

        if (bRet == TRUE)
        {
            m_bIsTakePicExStop = TRUE;

            if (wBankNo == BANK_PINGAN && m_stCamSaveImagePab.bIsNeedDailyRecord == TRUE)
            {
                vWriteImageInfo(m_pSaveFileName);
            }
        }
    }
}

// 活体检测结果生成图像
BOOL CDevCAM_CloudWalk::showLiveToImage(CWLiveImage live, LPSTR lpSaveFileName, WORD wSaveType)
{
    CHAR szFileNameJPG[MAX_PATH] = { 0x00 };
    CHAR szFileNameBASE64[MAX_PATH] = { 0x00 };
    CHAR szFileNameBMP[MAX_PATH] = { 0x00 };
    BOOL bSaveJPG = FALSE;
    BOOL bSaveBASE64 = FALSE;
    BOOL bSaveBMP = FALSE;

    if ((wSaveType & PIC_JPG) == PIC_JPG ||
        (wSaveType & PIC_BASE64) == PIC_BASE64)
    {
        sprintf(szFileNameJPG, "%s", lpSaveFileName);

        cv::Mat mat(live.height, live.width, CV_8UC3, live.data);
        if (cv::imwrite(szFileNameJPG, mat) == TRUE)
        {
            Log(ThisFile, 1, "showLive()->生成图片[%s]成功", szFileNameJPG);
            bSaveJPG = TRUE;

            if ((wSaveType & PIC_BASE64) == PIC_BASE64)
            {
                sprintf(szFileNameBASE64, "%s", lpSaveFileName);
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

    if ((wSaveType & PIC_BMP) == PIC_BMP)
    {
        sprintf(szFileNameBMP, "%s", lpSaveFileName);

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

    if (((wSaveType & PIC_BASE64) == PIC_BASE64 && bSaveBASE64 == FALSE) ||
        ((wSaveType & PIC_JPG) == PIC_JPG && bSaveJPG == FALSE) ||
        ((wSaveType & PIC_BMP) == PIC_BMP && bSaveBMP == FALSE))
    {
        return FALSE;
    }

    return TRUE;
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
            sprintf(cSourceImageFile, "%s", lpImageFile);
            sprintf(cImageSaveName, "%s/%s/%s.BASE64",
                    m_stCamSaveImagePab.byImageSavePathBASE64, cDirDate, cDirTime);
            bWriteFlag = FALSE;
        } else
        if ((m_wSaveFileType & PIC_JPG) == PIC_JPG && i == 1)
        {	// 生成JPG
            sprintf(cSourceImageFile, "%s", lpImageFile);
            sprintf(cImageSaveName, "%s/%s/%s.JPG",
                    m_stCamSaveImagePab.byImageSavePathJPG, cDirDate, cDirTime);
            bWriteFlag = TRUE;
        } else
        if ((m_wSaveFileType & PIC_BMP) == PIC_BMP && i == 2)
        {	// 生成BMP
            sprintf(cSourceImageFile, "%s", lpImageFile);
            sprintf(cImageSaveName, "%s/%s/%s.BMP",
                    m_stCamSaveImagePab.byImageSavePathBMP, cDirDate, cDirTime);
            bWriteFlag = TRUE;
        } else {
            continue;
        }

        sprintf(cImageInfoSaveName, "%s/CamImage_%s.txt",
                m_stCamSaveImagePab.byImageSaveInfoPath, cDirDate);
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
            if (bPathCheckAndCreate((LPSTR)m_stCamSaveImagePab.byImageInfoFile, FALSE) == FALSE) {
                Log(ThisFile, 1, "vWriteImageInfo()->bPathCheckAndCreate()目录[%s]建立失败, errno: %d|%s",
                            m_stCamSaveImagePab.byImageInfoFile, errno, strerror(errno));
                return;
            } else
            {
                // 写 通用重复写 Camera Info 文件，只保留最近一次 TakePic 生成图片信息
                QSettings qWCameraInfo(m_stCamSaveImagePab.byImageInfoFile, QSettings::IniFormat);
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

// 枚举当前设备上的相机所支持的分辨率
BOOL CDevCAM_CloudWalk::bGetbEnumReso()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CWResolution *stResoLists = nullptr, *stResoListTmp = nullptr;
    std::string  stdReso = "";
    if (m_stCamCwInitParam.wOpenType == CAM_OPEN_DEVIDX)
    {
        // 按序号枚举当前设备上的相机所支持的分辨率
        if (bEnumResolutions(m_stCamCwInitParam.wVisCamIdx, &stResoLists) != TRUE)
        {
            Log(ThisFile, 1, "按序号[%s]枚举当前设备上的相机所支持的分辨率失败,不记录Log", m_stCamCwInitParam.wVisCamIdx);
            return FALSE;
        }

        stResoListTmp = stResoLists;
        while(stResoListTmp != nullptr)
        {
            stdReso.append("W:");
            stdReso.append(std::to_string(stResoListTmp->width));
            stdReso.append(" H:");
            stdReso.append(std::to_string(stResoListTmp->height));
            stdReso.append("|");
            stResoListTmp = stResoListTmp->next;
        }
        Log(ThisFile, 1, "按序号[%s]枚举当前设备上的相机所支持的分辨率如下: %s",
            m_stCamCwInitParam.wVisCamIdx, stdReso.c_str());
    } else
    {
        // 按VID/PID枚举当前设备上的相机所支持的分辨率
        CWResolution *stResoLists = nullptr, *stResoListTmp = nullptr;
        if (bEnumResolutionsEx(m_stCamCwInitParam.szVisVid, m_stCamCwInitParam.szVisPid, &stResoLists) != TRUE)
        {
            Log(ThisFile, 1, "按VID[%s]/PID[%s]枚举当前设备上的相机所支持的分辨率失败,不记录Log",
                m_stCamCwInitParam.szVisVid, m_stCamCwInitParam.szVisPid);
            return FALSE;
        }

        stResoListTmp = stResoLists;
        std::string  stdReso = "";
        while(stResoListTmp != nullptr)
        {
            stdReso.append("W:");
            stdReso.append(std::to_string(stResoListTmp->width));
            stdReso.append(" H:");
            stdReso.append(std::to_string(stResoListTmp->height));
            stdReso.append("|");
            stResoListTmp = stResoListTmp->next;
        }
        Log(ThisFile, 1, "按VID[%s]/PID[%s]枚举当前设备上的相机所支持的分辨率如下: %s",
            m_stCamCwInitParam.szVisVid, m_stCamCwInitParam.szVisPid, stdReso.c_str());
    }

    return TRUE;
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






