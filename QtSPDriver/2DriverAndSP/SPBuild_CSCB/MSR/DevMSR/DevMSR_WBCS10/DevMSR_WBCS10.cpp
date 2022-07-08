#include "DevMSR_WBCS10.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QSettings>
#include <sys/stat.h>

#include <dirent.h>

// MSR 版本号
BYTE    byDevVRTU[17] = {"DevMSR00000100"};

static const char *ThisFile = "DevMSR_WBCS10.cpp";

//////////////////////////////////////////////////////////////////////////

CDevMSR_WBCS10::CDevMSR_WBCS10() : m_pDevWBCS10(LOG_NAME_DEV)
{
    SetLogFile(LOG_NAME_DEV, ThisFile);  // 设置日志文件名和错误发生的文件

    memset(&m_stOldStatus, 0x00, sizeof(DEVMSRSTATUS));
}

CDevMSR_WBCS10::~CDevMSR_WBCS10()
{
    m_pDevWBCS10.CloseDevice();
}

void CDevMSR_WBCS10::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    m_pDevWBCS10.CloseDevice();
    return;
}

int CDevMSR_WBCS10::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 设置动态库底层处理日志
    /*if (m_stMsrDevInitParam.stWBCS10DevLog.wIsEnableDevLog == 1)
    {
        if (strlen(m_stMsrDevInitParam.stWBCS10DevLog.szDeviceLog) < 2)
        {
            m_pDevWBCS10.bEnableLog(m_stMsrDevInitParam.stWBCS10DevLog.wDeviceLogLevel,
                                     nullptr);
        } else
        {
            m_pDevWBCS10.bEnableLog(m_stMsrDevInitParam.stWBCS10DevLog.wDeviceLogLevel,
                                     m_stMsrDevInitParam.stWBCS10DevLog.szDeviceLog);
        }
    } else
    {
        m_pDevWBCS10.bDisableLog();
    }*/

    if (m_stMsrDevInitParam.stDeviceOpenMode.wOpenMode == 0)
    {
        Log(ThisModule, __LINE__, "Open(): Open设备[串口:%s|%d] start...",
            m_stMsrDevInitParam.stDeviceOpenMode.szDevPath, m_stMsrDevInitParam.stDeviceOpenMode.wBaudRate);
    } else
    {
        Log(ThisModule, __LINE__, "Open(): Open设备[USB HID:%s|VID:%s|PID:%s] start...",
            m_stMsrDevInitParam.stDeviceOpenMode.szDevPath, m_stMsrDevInitParam.stDeviceOpenMode.szHidVid,
            m_stMsrDevInitParam.stDeviceOpenMode.szHidPid);
    }

    // 建立MSR连接
    if (m_pDevWBCS10.IsDeviceExist(m_stMsrDevInitParam.stDeviceOpenMode) == TRUE)   // 检查设备是否存在
    {
        if (m_pDevWBCS10.IsDeviceOpen() != TRUE)   // 检查设备未Open
        {
            if (m_pDevWBCS10.OpenDevice(m_stMsrDevInitParam.stDeviceOpenMode) != TRUE)  // Open失败
            {
                Log(ThisModule, __LINE__, "Open(): Open设备失败.");
                return ERR_MSR_NOT_OPEN;
            }
        }
    } else
    {
        Log(ThisModule, __LINE__, "Open(): 设备不存在.");
        return ERR_MSR_NO_DEVICE;
    }
    m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁

    Log(ThisModule, __LINE__, "Open(): Open设备成功.");
    return ERR_MSR_SUCCESS;
}

int CDevMSR_WBCS10::Reset()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (m_pDevWBCS10.bReset() != TRUE)
    {
        return lErrCodeChgToMSR(m_pDevWBCS10.GetErrCode());
        //return ERR_MSR_HWERR;
    }
    m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁

    return ERR_MSR_SUCCESS;
}

void CDevMSR_WBCS10::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_pDevWBCS10.CloseDevice();

    return;
}

/************************************************************
** 功能：取设备状态
** 输入：无
** 输出：stStatus : 设备状态结构体
** 返回：无
************************************************************/
int CDevMSR_WBCS10::GetStatus(DEVMSRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    char cDeviceStatus[][24] = {  "DEV_ONLINE",
                                "DEV_OFFLINE",
                                "DEV_POWEROFF",
                                "DEV_NODEVICE",
                                "DEV_HWERROR",
                                "DEV_USERERROR",
                                "DEV_BUSY"
    };

    char cMediaStatus[][24] = {  "MEDIA_PRESENT",
                               "MEDIA_NOTPRESENT",
                               "MEDIA_JAMMED",
                               "MEDIA_NOTSUPP",
                               "MEDIA_UNKNOWN",
                               "MEDIA_ENTERING",
    };

    // 设备状态初始化为 DEV_OFFLINE,MEDIA_UNKNOWN
    stStatus.clear();

    // 设备不存在
    if (m_pDevWBCS10.IsDeviceExist(m_stMsrDevInitParam.stDeviceOpenMode, FALSE) != TRUE)
    {
        stStatus.wDevice = DEV_NODEVICE;
        return ERR_MSR_NO_DEVICE;
    }

    // 设备未Open
    if (m_pDevWBCS10.IsDeviceOpen() != TRUE)
    {
        stStatus.wDevice = DEV_OFFLINE;
        return ERR_MSR_NOT_OPEN;
    }

    // 获取设备状态
    UCHAR ucFormat, ucTrack, ucEnable;
    if (m_pDevWBCS10.bStatus(&ucFormat, &ucTrack, &ucEnable) != TRUE)
    {
        return lErrCodeChgToMSR(m_pDevWBCS10.GetErrCode());
        // return ERR_MSR_HWERR;
    }

    // ----------------Check 状态----------------
    stStatus.wMedia = MEDIA_NOTPRESENT;
    if (ucEnable == 1)  // 刷卡有效
    {
        stStatus.wDevice = DEV_BUSY;
    } else
    {
        stStatus.wDevice = DEV_ONLINE;
    }

    // 状态变化记录Log
    if (memcmp(&stStatus, &m_stOldStatus, sizeof(DEVMSRSTATUS)) != 0)
    {
        Log(ThisModule, __LINE__,
            "状态变化: Device[%d|%s->%d|%s%s], Media[%d|%s->%d|%s%s]",
            m_stOldStatus.wDevice, cDeviceStatus[m_stOldStatus.wDevice],
            stStatus.wDevice, cDeviceStatus[stStatus.wDevice],
            m_stOldStatus.wDevice != stStatus.wDevice ? " *" : "",
            m_stOldStatus.wMedia, cMediaStatus[m_stOldStatus.wMedia],
            stStatus.wMedia, cMediaStatus[stStatus.wMedia],
            m_stOldStatus.wMedia != stStatus.wMedia ? " *" : "");
        memcpy(&m_stOldStatus, &stStatus, sizeof(DEVMSRSTATUS));
    }

    return ERR_MSR_SUCCESS;
}

// 读卡
int CDevMSR_WBCS10::ReadTracks(STTRACK_INFO &stTrackInfo, INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);
    #define SWITCH(X) \
        memset(szErrCode, 0x00, sizeof(szErrCode));\
        switch(X)\
        {\
            case TrackNotRead: sprintf(szErrCode, "0xE0: Setup设置不读该磁道");\
            case TrackSTX: sprintf(szErrCode, "0xE1: 无STX位"); break;\
            case TrackETX: sprintf(szErrCode, "0xE2: 无ETX位"); break;\
            case TrackVRC: sprintf(szErrCode, "0xE3: 位VRC校验错误"); break;\
            case TrackLRC: sprintf(szErrCode, "0xE4: 字节LRC校验错误"); break;\
            case TrackBlank: sprintf(szErrCode, "0xE5: 空磁道"); break;\
        }

    CHAR szErrCode[128] = "";
    UCHAR byTrack1[BUF_SIZE512] = { 0x00 };
    UCHAR byTrack2[BUF_SIZE512] = { 0x00 };
    UCHAR byTrack3[BUF_SIZE512] = { 0x00 };
    INT wTrack1Len = BUF_SIZE512;
    INT wTrack2Len = BUF_SIZE512;
    INT wTrack3Len = BUF_SIZE512;

    // 指定有效读取磁道号，三磁道均有效
    m_pDevWBCS10.bSetup(0, 7);

    // 读卡数据
    if (m_pDevWBCS10.bEnable(FALSE, 0, byTrack1, &wTrack1Len, byTrack2, &wTrack2Len, byTrack3, &wTrack3Len, nTimeOut) != TRUE)
    {
        INT nRet = m_pDevWBCS10.GetErrCode();
        Log(ThisModule, __LINE__, "读卡: ReadTracks()->bReadTrackAuto() 刷卡读卡 fail. Read Error, Return: %d ",
            lErrCodeChgToMSR(nRet));
        m_pDevWBCS10.bDisable();
        m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁
        return lErrCodeChgToMSR(nRet);
        //return ERR_MSR_READERROR;     // 读卡错误
    }

    // Check数据并组织写入应答
    if (stTrackInfo.TrackData[0].bTrackOK == true)  // 需要读1磁数据
    {
        if (wTrack1Len > 0)
        {
            memcpy(stTrackInfo.TrackData[0].szTrack, byTrack1, wTrack1Len);
        } else  // 数据未读到
        {
            stTrackInfo.TrackData[0].bTrackOK = false;
            if (wTrack1Len < 1)
            {
                SWITCH((INT)byTrack1[0]);
                Log(ThisModule, __LINE__, "读卡: bEnable(): 读磁道1数据错误,%s", szErrCode);
            }
        }
    }
    if (stTrackInfo.TrackData[1].bTrackOK == true)  // 需要读2磁数据
    {
        if (wTrack2Len > 0)
        {
            memcpy(stTrackInfo.TrackData[1].szTrack, byTrack2, wTrack2Len);
        } else  // 数据未读到
        {
            stTrackInfo.TrackData[1].bTrackOK = false;
            if (wTrack2Len < 1)
            {
                SWITCH((INT)byTrack2[0]);
                Log(ThisModule, __LINE__, "读卡: bEnable(): 读磁道2数据错误,%s", szErrCode);
            }
        }
    }
    if (stTrackInfo.TrackData[2].bTrackOK == true)  // 需要读3磁数据
    {
        if (wTrack3Len > 0)
        {
            memcpy(stTrackInfo.TrackData[2].szTrack, byTrack3, wTrack3Len);
        } else  // 数据未读到
        {
            stTrackInfo.TrackData[2].bTrackOK = false;
            if (wTrack3Len < 1)
            {
                SWITCH((INT)byTrack3[0]);
                Log(ThisModule, __LINE__, "读卡: bEnable(): 读磁道3数据错误,%s", szErrCode);
            }
        }
    }

    m_pDevWBCS10.bDisable();
    m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁

    return ERR_MSR_SUCCESS;
}

// 命令取消
int CDevMSR_WBCS10::Cancel()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    // 命令取消
    if (m_pDevWBCS10.bCancelPort(1) != TRUE)
    {
        INT nRet = m_pDevWBCS10.GetErrCode();
        Log(ThisModule, __LINE__, "命令取消: Cancel()->bCancelPort(1) fail. Read Error, Return: %d ",
            lErrCodeChgToMSR(nRet));
        return lErrCodeChgToMSR(nRet);
    }
    m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁

    return ERR_MSR_SUCCESS;
}

// 设置数据
int CDevMSR_WBCS10::SetData(void *vData, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT: // 0/初始化数据
        {
            LPSTMSRDEVINITPARAM lpInit = (LPSTMSRDEVINITPARAM)vData;
            memcpy(&m_stMsrDevInitParam, lpInit, sizeof(STMSRDEVINITPARAM));
            break;
        }
        default:
                break;
    }

    return ERR_MSR_SUCCESS;
}

// 获取数据
int CDevMSR_WBCS10::GetData(void *vData, WORD wDataType)
{
    switch(wDataType)
    {
        default:
            break;
    }

    return ERR_MSR_SUCCESS;
}

// 获取版本号(1Dev版本/2固件版本/3其他)
void CDevMSR_WBCS10::GetVersion(LPSTR szVer, long lSize, ushort usType)
{
    if (usType == 1)
    {
        memcpy(szVer, byDevVRTU, strlen((char*)byDevVRTU) > lSize ? lSize : strlen((char*)byDevVRTU));
    }
    if (usType == 2)
    {
        m_pDevWBCS10.GetFWVersion(szVer);
    }
    if (usType == 3)
    {
        m_pDevWBCS10.bGetLibVersion(szVer);
    }
}

// WBCS10错误码转换为DevMSR错误码
LONG CDevMSR_WBCS10::lErrCodeChgToMSR(LONG lDevCode)
{
    switch(lDevCode)
    {
        case APICMDSuccess:     /* 0 */         // 命令执行成功
            return ERR_MSR_SUCCESS;                     // return 操作成功
        case APICMDFailure:     /* -1 */        // 命令执行失败
            return ERR_MSR_HWERR;                       // return 硬件故障
        case APICMDParmError:   /* -2 */        // API参数错误
            return ERR_MSR_PARAM_ERR;                   // return 参数错误
        case APIWriteError:     /* -3 */        // 写数据出错
            return ERR_MSR_WRITEERROR;                  // return 写卡错
        case APIWriteTimeOut:   /* -4 */        // 写数据超时
            return ERR_MSR_WRITETIMEOUT;                // return 写数据超时
        case APIReadError:      /* -5 */        // 读数据出错
            return ERR_MSR_READERROR;                   // return 读卡错
        case APIReadTimeOut:    /* -6 */        // 读数据超时
            return ERR_MSR_READTIMEOUT;                 // return 读数据超时
        case APIFrameError:     /* -7 */        // 数据不符合协议
            return ERR_MSR_HWERR;                       // return 硬件故障
        case APIUnknownError:   /* -8 */        // 未知错误
            return ERR_MSR_OTHER;                       // return 其它错误
        case APINotSupportCMD:  /* -9 */        // 不支持命令
            return ERR_MSR_UNSUP_CMD;                   // return 不支持的指令
        case APICMDCancel:      /* -10 */        // 命令取消
            return ERR_MSR_USER_CANCEL;                 // return 用户取消进卡
        case APICMDInProgress:  /* -11 */        // 命令运行中
            return ERR_MSR_CMD_RUNNING;                 // return 命令执行中
        default :
            return ERR_MSR_OTHER;                       //
    }
}

//////////////////////////////////////////////////////////////////////////






