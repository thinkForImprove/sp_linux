#include "DevCAM_TCF261.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>

// CAM 版本号
//BYTE    byDevVRTU[17] = {"DevCAM00000100"};

static const char *ThisFile = "DevCAM_TCF261.cpp";

extern CDevImpl_TCF261_Task cTask;

//////////////////////////////////////////////////////////////////////////

CDevCAM_TCF261::CDevCAM_TCF261() : m_devTCF261(LOG_NAME_DEVCAM)
{
    SetLogFile(LOG_NAME_DEVCAM, ThisFile);  // 设置日志文件名和错误发生的文件

    memset(&m_stStatus, 0x00, sizeof(DEVCAMSTATUS));

    m_bDisplayOpenOK = FALSE;     // 未打开
    m_bCancel = FALSE;	// 30-00-00-00(FT#0031)

    cTask.cDevTCF261 = &m_devTCF261;    // 全局变量，用于抓拍回调函数对类成员的处理
}

CDevCAM_TCF261::~CDevCAM_TCF261()
{
    Close();
}

void CDevCAM_TCF261::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

long CDevCAM_TCF261::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 建立Cam连接
    if (m_devTCF261.IsDeviceOpen() != TRUE)
    {
        if (m_devTCF261.OpenDevice() != TRUE)
        {
            return ERR_OPEN_CAMER;
        }
    }

    return CAM_SUCCESS;
}

long CDevCAM_TCF261::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_devTCF261.CloseDevice();

    return CAM_SUCCESS;
}

long CDevCAM_TCF261::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (m_devTCF261.bCloseWindow() != TRUE)
    {
        return ERR_OTHER;
    }

    return CAM_SUCCESS;
}

long CDevCAM_TCF261::GetDevInfo(char *pInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return CAM_SUCCESS;
}


long CDevCAM_TCF261::GetStatus(DEVCAMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nStat = 0;

    stStatus.fwDevice      = DEVICE_OFFLINE;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.fwMedia[i] = MEDIA_UNKNOWN;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.fwCameras[i] = STATUS_NOTSUPP;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.usPictures[i] = CAM_ROOM;

    if (m_devTCF261.IsDeviceOpen() != TRUE)
    {
        return ERR_OTHER;
    }

    m_devTCF261.bGetStatus(&nStat);
    switch(nStat)
    {
        case FR_RET_SUCC:   // 成功
        case FR_RET_ARDY:   // 已经打开设备
            stStatus.fwDevice   = DEVICE_ONLINE;
            stStatus.fwMedia[1] = MEDIA_OK;
            stStatus.fwCameras[1] = STATUS_OK;
            break;
        case FR_RET_BUSY:   // 设备繁忙
            stStatus.fwDevice   = DEVICE_BUSY;
            stStatus.fwMedia[1] = MEDIA_OK;
            stStatus.fwCameras[1] = STATUS_OK;
            break;
        case FR_RET_NDEV:   // 打开设备失败
        case FR_RET_NINI:   // 未打开设备
        case FR_RET_NLNK:   // 设备断开
            stStatus.fwDevice   = DEVICE_OFFLINE;
            stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            stStatus.fwCameras[1] = CAM_ROOM;
            break;
        default:
            stStatus.fwDevice   = DEVICE_HWERROR;
            stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            stStatus.fwCameras[1] = CAM_ROOM;
            break;
    }

    //UpdateStatus(DEVICE_ONLINE, "000");
    return CAM_SUCCESS;
}

// 命令取消 // 30-00-00-00(FT#0031)
long CDevCAM_TCF261::Cancel()
{
    m_bCancel = TRUE;
}

// 打开窗口(窗口句柄，动作指示:1创建窗口/0销毁窗口, X/Y坐标,窗口宽高)
long CDevCAM_TCF261::Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight)
{
    if (wAction == CAMERA_OPEN)
    {
        if (m_bDisplayOpenOK == TRUE)   // Display已打开窗口，直接return
        {
            return CAM_SUCCESS;
        }

        if (m_devTCF261.bCreateWindow(hWnd, wX, wY, wWidth, wHeight) != TRUE)
        {
            return ERR_OTHER;
        }

        m_bDisplayOpenOK = TRUE;

    } else
    if (wAction == CAMERA_PAUSE)    // 暂停
    {
        if (m_bDisplayOpenOK != TRUE)   // Display已未打开窗口，直接return
        {
            return CAM_SUCCESS;
        }
        if (m_devTCF261.bPauseAndPlaya(true) != TRUE)
        {
            return ERR_OTHER;
        }
    } else
    if (wAction == CAMERA_RESUME)    // 恢复
    {
        if (m_bDisplayOpenOK != TRUE)   // Display已未打开窗口，直接return
        {
            return CAM_SUCCESS;
        }
        if (m_devTCF261.bPauseAndPlaya(false) != TRUE)
        {
            return ERR_OTHER;
        }
    } else
    {
        // 关闭所有相机
        if (m_bDisplayOpenOK == TRUE)   // Display已打开窗口，直接return
        {
            if (m_devTCF261.bCloseWindow() != TRUE)
            {
                return ERR_OTHER;
            }
        }

        m_bDisplayOpenOK = FALSE;
    }

    return CAM_SUCCESS;
}

long CDevCAM_TCF261::TakePicture(WORD wCamera, LPCSTR lpData)
{
    return CAM_SUCCESS;
}

long CDevCAM_TCF261::TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, DWORD dwTimeOut)
{
    std::string stdFileName;
    stdFileName.append(lpFileName);
    stdFileName.append(".");
    if ((wPicType & PIC_BMP) == PIC_BMP)
    {
        stdFileName.append("BMP");
    } else
    if ((wPicType & PIC_JPG) == PIC_JPG)
    {
        stdFileName.append("JPG");
    } else
    {
        stdFileName.append("BASE64");
    }

    m_devTCF261.SetTakePicStop(FALSE);

    if (m_devTCF261.bStartLiveDetecte(m_stCamTCF261InitParam.wLiveDetectMode,
                                      "", stdFileName.c_str()) != TRUE)
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
            if (dwTimeCount >= dwTimeOut && m_devTCF261.GetTakePicStop() != TRUE)
            {
                m_devTCF261.bStopLiveDetecte(); // 停止连续活体检测
                return ERR_TIMEOUT;
            }
        }

        if (m_devTCF261.GetTakePicStop() == TRUE)
        {
            break;
        }

        if (m_bCancel == TRUE)  // 命令取消
        {
            m_devTCF261.bStopLiveDetecte(); // 停止连续活体检测
            return ERR_CANCEL;
        }

        usleep(1000 * 30);
        dwTimeCount += 30;
    }

    // 停止连续活体检测
    m_devTCF261.bStopLiveDetecte();

    return CAM_SUCCESS;
}

long CDevCAM_TCF261::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT:
        {
            m_stCamTCF261InitParam.wLiveDetectMode =
                    ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->stCamTCF261InitParam.wLiveDetectMode;
            break;
        }
        default:
            break;
    }

    return CAM_SUCCESS;
}

long CDevCAM_TCF261::GetData(void *vInitPar, WORD wDataType)
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
void CDevCAM_TCF261::GetVersion(char* szVer, long lSize, ushort usType)
{
    if (usType == 1)
    {
        memcpy(szVer, byDevVRTU, strlen((char*)byDevVRTU) > lSize ? lSize : strlen((char*)byDevVRTU));
    }
    if (usType == 2)
    {
        if (m_devTCF261.IsDeviceOpen() == TRUE)
        {
            if (m_devTCF261.bGetFirmwareVersion(szVer, lSize) != TRUE)
            {
                memset(szVer, 0x00, lSize);
            }
        }
    }
    if (usType == 3)
    {

    }
}

//////////////////////////////////////////////////////////////////////////






