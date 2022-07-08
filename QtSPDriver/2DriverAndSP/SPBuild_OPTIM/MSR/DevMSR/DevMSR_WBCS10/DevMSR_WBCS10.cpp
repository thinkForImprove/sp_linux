/***************************************************************
* 文件名称：DevMSR_WBCS10.cpp
* 文件描述：刷折模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevMSR_WBCS10.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QSettings>
#include <sys/stat.h>
#include <dirent.h>
#include "file_access.h"

static const char *ThisFile = "DevMSR_WBCS10.cpp";

/****************************************************************************
*     对外接口调用处理                                                         *
****************************************************************************/
CDevMSR_WBCS10::CDevMSR_WBCS10() : m_pDevWBCS10(LOG_NAME_DEV)
{
    SetLogFile(LOG_NAME_DEV, ThisFile);     // 设置日志文件名和错误发生的文件
    m_bReCon = FALSE;                       // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));
}

CDevMSR_WBCS10::~CDevMSR_WBCS10()
{
    m_pDevWBCS10.CloseDevice();
}

int CDevMSR_WBCS10::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    m_pDevWBCS10.CloseDevice();
    return IDC_SUCCESS;
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

    if (m_bReCon == FALSE)   // 断线重连时不再记录Log
    {
        if (m_stMsrDevInitParam.stDeviceOpenMode.wOpenMode == 0)
        {
            Log(ThisModule, __LINE__, "打开设备: Open设备[串口:%s|%d] start...",
                m_stMsrDevInitParam.stDeviceOpenMode.szDevPath, m_stMsrDevInitParam.stDeviceOpenMode.wBaudRate);
        } else
        {
            Log(ThisModule, __LINE__, "打开设备: Open设备[USB HID:%s|VID:%s|PID:%s] start...",
                m_stMsrDevInitParam.stDeviceOpenMode.szDevPath, m_stMsrDevInitParam.stDeviceOpenMode.szHidVid,
                m_stMsrDevInitParam.stDeviceOpenMode.szHidPid);
        }
    }

    // 建立MSR连接
    BOOL bRet = FALSE;
    bRet = m_pDevWBCS10.IsDeviceExist(m_stMsrDevInitParam.stDeviceOpenMode,
                                      (m_bReCon == TRUE ? FALSE : TRUE));
    if (bRet == TRUE)   // 检查设备是否存在
    {
        if (m_pDevWBCS10.IsDeviceOpen() != TRUE)   // 检查设备未Open
        {
            bRet = m_pDevWBCS10.OpenDevice(m_stMsrDevInitParam.stDeviceOpenMode);
            if (bRet != TRUE)  // Open失败
            {
                if (m_nRetErrOLD[1] != bRet)
                {
                    Log(ThisModule, __LINE__, "打开设备: 设备不存在.");
                }
                m_nRetErrOLD[1] = bRet;
                Log(ThisModule, __LINE__, "打开设备: Open设备失败.");
                return ERR_IDC_DEV_NOTOPEN;
            }
        }
    } else
    {
        if (m_nRetErrOLD[0] != bRet)
        {
            Log(ThisModule, __LINE__, "打开设备: 设备不存在.");
        }
        m_nRetErrOLD[0] = bRet;
        return ERR_IDC_DEV_NOTFOUND;
    }

    if (m_bReCon == FALSE)
    {
        Log(ThisModule, __LINE__, "打开设备: Open设备成功.");
    } else
    {
        Log(ThisModule, __LINE__, "ReOpen设备成功, 设备重连 End......");
    }

    // 设置设备Log输出
    if (m_stMsrDevInitParam.stDeviceLog.wEnableLog == 0)    // 禁止输出
    {
        m_pDevWBCS10.bDisableLog();
    } else
    {
        m_pDevWBCS10.bDisableLog();
        if (strlen(m_stMsrDevInitParam.stDeviceLog.szLogPath) > 2)
        {
            if (FileAccess::create_directory_by_path(m_stMsrDevInitParam.stDeviceLog.szLogPath) != TRUE)
            {
                Log(ThisModule, __LINE__, "创建设备底层log目录[%s]失败,采用缺省目录.",
                    m_stMsrDevInitParam.stDeviceLog.szLogPath);
                m_pDevWBCS10.bEnableLog(m_stMsrDevInitParam.stDeviceLog.wLogLevel, nullptr);
            } else
            {
                m_pDevWBCS10.bEnableLog(m_stMsrDevInitParam.stDeviceLog.wLogLevel,
                                        m_stMsrDevInitParam.stDeviceLog.szLogPath);
            }
        } else
        {
            Log(ThisModule, __LINE__, "指定设备底层log目录[%s]无效,采用缺省目录.",
                m_stMsrDevInitParam.stDeviceLog.szLogPath);
            m_pDevWBCS10.bEnableLog(m_stMsrDevInitParam.stDeviceLog.wLogLevel, nullptr);
        }
    }

    m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁
    m_bReCon = FALSE;           // 断线重连标记置为F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IDC_SUCCESS;
}

int CDevMSR_WBCS10::Reset(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (m_pDevWBCS10.bReset() != TRUE)
    {
        return lErrCodeChgToIDC(m_pDevWBCS10.GetErrCode());
    }
    m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁

    return IDC_SUCCESS;
}

int CDevMSR_WBCS10::Close()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    m_pDevWBCS10.CloseDevice();

    return IDC_SUCCESS;
}

// 命令取消
int CDevMSR_WBCS10::Cancel(unsigned short usMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    // 命令取消
    if (m_pDevWBCS10.bCancelPort(1) != TRUE)
    {
        INT nRet = m_pDevWBCS10.GetErrCode();
        Log(ThisModule, __LINE__, "命令取消: Cancel()->bCancelPort(1) fail. Read Error, Return: %d ",
            lErrCodeChgToIDC(nRet));
        return lErrCodeChgToIDC(nRet);
    }
    m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁

    return IDC_SUCCESS;
}

/************************************************************
** 功能：取设备状态
** 输入：无
** 输出：stStatus : 设备状态结构体
** 返回：无
************************************************************/
int CDevMSR_WBCS10::GetStatus(STDEVIDCSTATUS &stStatus)
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
    stStatus.Clear();

    // 设备不存在
    if (m_pDevWBCS10.IsDeviceExist(m_stMsrDevInitParam.stDeviceOpenMode, FALSE) != TRUE)
    {
        stStatus.wDevice = DEVICE_STAT_NODEVICE;
        return ERR_IDC_DEV_NOTFOUND;
    }

    // 设备未Open
    if (m_pDevWBCS10.IsDeviceOpen() != TRUE)
    {
        stStatus.wDevice = DEVICE_STAT_OFFLINE;
        return ERR_IDC_DEV_NOTOPEN;
    }

    // 获取设备状态
    UCHAR ucFormat, ucTrack, ucEnable;
    if (m_pDevWBCS10.bStatus(&ucFormat, &ucTrack, &ucEnable) != TRUE)
    {
        return lErrCodeChgToIDC(m_pDevWBCS10.GetErrCode());
        // return ERR_IDC_HWERR;
    }

    // ----------------Check 状态----------------
    stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
    if (ucEnable == 1)  // 刷卡有效
    {
        stStatus.wDevice = DEVICE_STAT_BUSY;
    } else
    {
        stStatus.wDevice = DEVICE_STAT_ONLINE;
    }

    // 状态变化记录Log
    if (stStatus.Diff(m_stStatusOLD) != 0)
    {
        Log(ThisModule, __LINE__,
            "状态变化: Device[%d|%s->%d|%s%s], Media[%d|%s->%d|%s%s]",
            m_stStatusOLD.wDevice, cDeviceStatus[m_stStatusOLD.wDevice],
            stStatus.wDevice, cDeviceStatus[stStatus.wDevice],
            m_stStatusOLD.wDevice != stStatus.wDevice ? " *" : "",
            m_stStatusOLD.wMedia, cMediaStatus[m_stStatusOLD.wMedia],
            stStatus.wMedia, cMediaStatus[stStatus.wMedia],
            m_stStatusOLD.wMedia != stStatus.wMedia ? " *" : "");
        m_stStatusOLD.Copy(stStatus);
    }

    return IDC_SUCCESS;
}

// 介质读写
int CDevMSR_WBCS10::MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData)

{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);
    #define SWITCH(X, C) \
        switch(X)\
        {\
            case TrackNotRead: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE0: Setup设置不读该磁道];", C); break;\
            case TrackSTX: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE1: 无STX位];", C); break;\
            case TrackETX: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE2: 无ETX位];", C); break;\
            case TrackVRC: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE3: 位VRC校验错误];", C); break;\
            case TrackLRC: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE4: 字节LRC校验错误];", C); break;\
            case TrackBlank: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE5: 空磁道];", C); break;\
        }

    CHAR szErrCode[512] = "";
    UCHAR byTrack1[512] = { 0x00 };
    UCHAR byTrack2[512] = { 0x00 };
    UCHAR byTrack3[512] = { 0x00 };
    INT wTrack1Len = 512;
    INT wTrack2Len = 512;
    INT wTrack3Len = 512;

    // 指定有效读取磁道号，三磁道均有效
    m_pDevWBCS10.bSetup(0, 7);

    // 读卡数据
    if (m_pDevWBCS10.bEnable(FALSE, 0, byTrack1, &wTrack1Len, byTrack2, &wTrack2Len,
                             byTrack3, &wTrack3Len, stMediaData.dwTimeOut) != TRUE)
    {
        INT nRet = m_pDevWBCS10.GetErrCode();
        Log(ThisModule, __LINE__, "读卡: ->bEnable() fail, Return: %d.",
            lErrCodeChgToIDC(nRet));
        m_pDevWBCS10.bDisable();
        m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁
        return lErrCodeChgToIDC(nRet);
    }

    // Check数据并组织写入应答
    stMediaData.DataClear();
    if ((stMediaData.dwRWType & RW_TRACK1) == RW_TRACK1)  // 需要读1磁数据
    {
        if (wTrack1Len > 0)
        {
            memcpy(stMediaData.stData[0].szData, byTrack1, wTrack1Len);
            stMediaData.stData[0].wSize = wTrack1Len;
            stMediaData.stData[0].wResult = RW_RESULT_SUCC;
        } else  // 数据未读到
        {
            stMediaData.stData[0].wSize = 0;
            stMediaData.stData[0].wResult = RW_RESULT_INV;
            if (wTrack1Len < 1)
            {
                SWITCH((INT)byTrack1[0], 1);
            }
        }
    }
    if ((stMediaData.dwRWType & RW_TRACK2) == RW_TRACK2)  // 需要读2磁数据
    {
        if (wTrack2Len > 0)
        {
            memcpy(stMediaData.stData[1].szData, byTrack2, wTrack2Len);
            stMediaData.stData[1].wSize = wTrack2Len;
            stMediaData.stData[1].wResult = RW_RESULT_SUCC;
        } else  // 数据未读到
        {
            stMediaData.stData[1].wSize = 0;
            stMediaData.stData[1].wResult = RW_RESULT_INV;
            if (wTrack1Len < 1)
            {
                SWITCH((INT)byTrack1[0], 1);
            }
        }
    }
    if ((stMediaData.dwRWType & RW_TRACK3) == RW_TRACK3)  // 需要读3磁数据
    {
        if (wTrack3Len > 0)
        {
            memcpy(stMediaData.stData[2].szData, byTrack3, wTrack3Len);
            stMediaData.stData[2].wSize = wTrack3Len;
            stMediaData.stData[2].wResult = RW_RESULT_SUCC;
        } else  // 数据未读到
        {
            stMediaData.stData[2].wSize = 0;
            stMediaData.stData[2].wResult = RW_RESULT_INV;
            if (wTrack1Len < 1)
            {
                SWITCH((INT)byTrack1[0], 1);
            }
        }
    }

    if (strlen(szErrCode) > 0)
    {
        Log(ThisModule, __LINE__, "读卡: bEnable() succ, 读到的磁道数据结果如下: %s", szErrCode);
    }

    m_pDevWBCS10.bDisable();
    m_pDevWBCS10.bSetup(0, 0);  // 禁止读磁

    return IDC_SUCCESS;
}

// 设置数据
int CDevMSR_WBCS10::SetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    switch(usType)
    {
        case SET_DEV_INIT:      // 0/初始化数据
        {
            if (vData != nullptr)
            {
                LPSTMSRDEVINITPARAM lpInit = (LPSTMSRDEVINITPARAM)vData;
                memcpy(&m_stMsrDevInitParam, lpInit, sizeof(STMSRDEVINITPARAM));
            }
            break;
        }
        case SET_LIB_PATH:      // 设置Lib路径
        {
            if (vData != nullptr)
            {
                m_pDevWBCS10.SetLibPath((LPCSTR)vData);
            }
            break;
        }
        case SET_DEV_RECON:     // 设置断线重连标记为T
        {
            if (m_bReCon != TRUE)
            {
                Log(ThisModule, __LINE__, "设备重连 Start......");
            }
            m_bReCon = TRUE;
            m_pDevWBCS10.SetReConFlag(TRUE);
        }
        default:
            break;
    }

    return IDC_SUCCESS;
}

// 获取数据
int CDevMSR_WBCS10::GetData(unsigned short usType, void *vData)
{
    switch(usType)
    {
        default:
            break;
    }

    return IDC_SUCCESS;
}

// 获取版本号
int CDevMSR_WBCS10::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    if (usType == GET_VER_FW)
    {
        m_pDevWBCS10.GetFWVersion(szVer);
    } else
    if (usType == GET_VER_LIB)
    {
        m_pDevWBCS10.bGetLibVersion(szVer);
    }

    return IDC_SUCCESS;
}

// WBCS10错误码转换为DevMSR错误码
INT CDevMSR_WBCS10::lErrCodeChgToIDC(LONG lDevCode)
{
    switch(lDevCode)
    {
        case APICMDSuccess:     /* 0 */         // 命令执行成功
            return IDC_SUCCESS;                         // return 操作成功
        case APICMDFailure:     /* -1 */        // 命令执行失败
            return ERR_IDC_DEV_HWERR;                   // return 硬件故障
        case APICMDParmError:   /* -2 */        // API参数错误
            return ERR_IDC_PARAM_ERR;                   // return 参数错误
        case APIWriteError:     /* -3 */        // 写数据出错
            return ERR_IDC_WRITE_ERR;                   // return 写卡错
        case APIWriteTimeOut:   /* -4 */        // 写数据超时
            return ERR_IDC_WRITE_TIMEOUT;               // return 写数据超时
        case APIReadError:      /* -5 */        // 读数据出错
            return ERR_IDC_READ_ERR;                    // return 读卡错
        case APIReadTimeOut:    /* -6 */        // 读数据超时
            return ERR_IDC_READ_TIMEOUT;                // return 读数据超时
        case APIFrameError:     /* -7 */        // 数据不符合协议
            return ERR_IDC_DEV_HWERR;                   // return 硬件故障
        case APIUnknownError:   /* -8 */        // 未知错误
            return ERR_IDC_OTHER;                       // return 其它错误
        case APINotSupportCMD:  /* -9 */        // 不支持命令
            return ERR_IDC_UNSUP_CMD;                   // return 不支持的指令
        case APICMDCancel:      /* -10 */        // 命令取消
            return ERR_IDC_USER_CANCEL;                 // return 用户取消进卡
        case APICMDInProgress:  /* -11 */        // 命令运行中
            return ERR_IDC_CMD_RUNNING;                 // return 命令执行中
        default :
            return ERR_IDC_OTHER;                       //
    }
}

//////////////////////////////////////////////////////////////////////////






