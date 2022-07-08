/***************************************************************
* 文件名称：DevMSR_WBT2000.cpp
* 文件描述：刷折模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevMSR_WBT2000.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QSettings>
#include <sys/stat.h>
#include <dirent.h>
#include "file_access.h"


static const char *ThisFile = "DevMSR_WBT2000.cpp";

//////////////////////////////////////////////////////////////////////////

CDevMSR_WBT2000::CDevMSR_WBT2000() : m_pDevWBT2000(LOG_NAME_DEV)
{
    SetLogFile(LOG_NAME_DEV, ThisFile);     // 设置日志文件名和错误发生的文件
    m_bReCon = FALSE;                       // 是否断线重连状态: 初始F
    memset(&m_stOldStatus, 0x00, sizeof(DEVMSRSTATUS));
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));
}

CDevMSR_WBT2000::~CDevMSR_WBT2000()
{
    m_pDevWBT2000.CloseDevice();
}

void CDevMSR_WBT2000::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    m_pDevWBT2000.CloseDevice();
    return;
}

int CDevMSR_WBT2000::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

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
    INT nExistRet = 0;
    INT nRet = IMP_SUCCESS;
    nExistRet = m_pDevWBT2000.IsDeviceExist(m_stMsrDevInitParam.stDeviceOpenMode,
                                            (m_bReCon == TRUE ? FALSE : TRUE));
    if (nExistRet == 0 || nExistRet == 1)  // 检查设备存在 | 枚举串口成功,指定串口路径为空
    {
        if (m_pDevWBT2000.IsDeviceOpen() != TRUE)   // 检查设备未Open
        {
            nRet = m_pDevWBT2000.OpenDevice(m_stMsrDevInitParam.stDeviceOpenMode);
            if (nRet != IMP_SUCCESS)  // Open失败
            {
                if (m_nRetErrOLD[1] != nRet)
                {
                    Log(ThisModule, __LINE__, "打开设备: Open设备失败.");
                    m_nRetErrOLD[1] = nRet;
                }
                return ERR_MSR_NOT_OPEN;
            }

            // INI未指定设备路径, 需通过设备连接符反推设备路径名
            if (nExistRet == 1)
            {
                LPSTR lpDevPath = m_stMsrDevInitParam.stDeviceOpenMode.szDevPath;
                nRet = m_pDevWBT2000.nGetPortName(lpDevPath, 64);
                if (nRet != IMP_SUCCESS)
                {
                    if (m_nRetErrOLD[1] != nRet)
                    {
                        Log(ThisModule, __LINE__,
                            "打开设备: Open设备成功, INI未指定设备路径, 通过设备连接符反推设备路径名:"
                            " ->nGetPortName() fail, ErrCode: %d.", nRet);
                        m_nRetErrOLD[1] = nRet;
                    }
                    return ERR_MSR_NOT_OPEN;
                }
            }
        }
    } else
    {
        if (nExistRet == -2 || nExistRet == -3)   // 串口不存在 | 枚举串口为0
        {
            if (m_nRetErrOLD[0] != nExistRet)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: ->IsDeviceExist() = %d(串口不存在 | 枚举串口为0), Return: %d",
                    nExistRet, ERR_MSR_NO_DEVICE);
            }
            m_nRetErrOLD[0] = nExistRet;
            return ERR_MSR_NO_DEVICE;
        } else
        if (nExistRet == -1) // 动态库加载失败
        {
            if (m_nRetErrOLD[0] != nExistRet)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: ->IsDeviceExist() = %d(串口不存在 | 枚举串口为0), Return: %d",
                    nExistRet, ERR_MSR_OTHER);
            }
            m_nRetErrOLD[0] = nExistRet;
            return ERR_MSR_OTHER;
        } else
        if (nExistRet == -4) // 无效的连接类型
        {
            if (m_nRetErrOLD[0] != nExistRet)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: ->IsDeviceExist() = %d(INI设置了无效的连接类型), Return: %d",
                    nExistRet, ERR_MSR_OTHER);
            }
            m_nRetErrOLD[0] = nExistRet;
            return ERR_MSR_OTHER;
        }
    }

    if (m_bReCon == FALSE)
    {
        Log(ThisModule, __LINE__, "打开设备: Open设备成功.");
    } else
    {
        Log(ThisModule, __LINE__, "ReOpen设备成功, 设备重连 End......");
    }

    // 设置SDK日志功能
    if (m_stMsrDevInitParam.stDeviceLog.wEnableLog == 0)    // 禁止
    {
        m_pDevWBT2000.nDisableLog();
    } else                                                  // 开启
    {
        m_pDevWBT2000.nEnableLog(m_stMsrDevInitParam.stDeviceLog.wLogLevel);
    }

    //m_pDevWBT2000.bSetup(0, 0);  // 禁止读磁
    m_bReCon = FALSE;           // 断线重连标记置为F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return ERR_MSR_SUCCESS;
}

int CDevMSR_WBT2000::Reset()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    nRet = m_pDevWBT2000.nReset();

    if (nRet != IMP_SUCCESS)
    {
        return ConvertErrCode(nRet);
    }
    //m_pDevWBT2000.bSetup(0, 0);  // 禁止读磁

    return ERR_MSR_SUCCESS;
}

void CDevMSR_WBT2000::Close()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_pDevWBT2000.CloseDevice();

    return;
}

/************************************************************
** 功能：取设备状态
** 输入：无
** 输出：stStatus : 设备状态结构体
** 返回：无
************************************************************/
int CDevMSR_WBT2000::GetStatus(DEVMSRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    char cDeviceStatus[][24] = {    "DEV_ONLINE",
                                    "DEV_OFFLINE",
                                    "DEV_POWEROFF",
                                    "DEV_NODEVICE",
                                    "DEV_HWERROR",
                                    "DEV_USERERROR",
                                    "DEV_BUSY"
    };

    char cMediaStatus[][24] = {    "MEDIA_PRESENT",
                                   "MEDIA_NOTPRESENT",
                                   "MEDIA_JAMMED",
                                   "MEDIA_NOTSUPP",
                                   "MEDIA_UNKNOWN",
                                   "MEDIA_ENTERING",
    };

    // 设备状态初始化为 DEV_OFFLINE,MEDIA_UNKNOWN
    stStatus.clear();

    // 设备不存在
    INT nRet = 0;
    nRet = m_pDevWBT2000.IsDeviceExist(m_stMsrDevInitParam.stDeviceOpenMode, FALSE);
    if (nRet == -2 || nRet == -3)
    {
        stStatus.wDevice = DEV_NODEVICE;
        return ERR_MSR_NO_DEVICE;
    }

    // 设备未Open
    if (m_pDevWBT2000.IsDeviceOpen() != TRUE)
    {
        stStatus.wDevice = DEV_OFFLINE;
        return ERR_MSR_NOT_OPEN;
    }

    stStatus.wDevice = DEV_ONLINE;
    stStatus.wMedia = MEDIA_NOTPRESENT;

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
int CDevMSR_WBT2000::ReadTracks(STTRACK_INFO &stTrackInfo, INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);
   /* #define SWITCH(X, C) \
        switch(X)\
        {\
            case TrackNotRead: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE0: Setup设置不读该磁道];", C); break;\
            case TrackSTX: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE1: 无STX位];", C); break;\
            case TrackETX: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE2: 无ETX位];", C); break;\
            case TrackVRC: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE3: 位VRC校验错误];", C); break;\
            case TrackLRC: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE4: 字节LRC校验错误];", C); break;\
            case TrackBlank: sprintf(szErrCode + strlen(szErrCode), "读磁道%d数据错误[0xE5: 空磁道];", C); break;\
        }*/

    CHAR szErrCode[512] = "";
    UCHAR byTrack1[BUF_SIZE512] = { 0x00 };
    UCHAR byTrack2[BUF_SIZE512] = { 0x00 };
    UCHAR byTrack3[BUF_SIZE512] = { 0x00 };
    INT wTrack1Len = BUF_SIZE512;
    INT wTrack2Len = BUF_SIZE512;
    INT wTrack3Len = BUF_SIZE512;

    // 指定有效读取磁道号，三磁道均有效
    //m_pDevWBT2000.bSetup(0, 7);

    // 读卡数据
    INT nRet = IMP_SUCCESS;
    nRet = m_pDevWBT2000.nReadMGCard(TRACK_123, byTrack1, &wTrack1Len, byTrack2, &wTrack2Len,
                                     byTrack3, &wTrack3Len, nTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡: ->nReadMGCard() fail, Return: %d.",
            ConvertErrCode(nRet));
        //m_pDevWBT2000.bDisable();
        //m_pDevWBT2000.bSetup(0, 0);  // 禁止读磁
        return ConvertErrCode(nRet);
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
                //SWITCH((INT)byTrack1[0], 1);
                //Log(ThisModule, __LINE__, "读卡: bEnable(): 读磁道1数据错误,%s", szErrCode);
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
                //SWITCH((INT)byTrack2[0], 2);
                //Log(ThisModule, __LINE__, "读卡: bEnable(): 读磁道2数据错误,%s", szErrCode);
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
                //SWITCH((INT)byTrack3[0], 3);
                //Log(ThisModule, __LINE__, "读卡: bEnable(): 读磁道3数据错误,%s", szErrCode);
            }
        }
    }

    if (strlen(szErrCode) > 0)
    {
        Log(ThisModule, __LINE__, "读卡: bEnable() succ, 读到的磁道数据结果如下: %s", szErrCode);
    }

    //m_pDevWBT2000.bDisable();
    //m_pDevWBT2000.bSetup(0, 0);  // 禁止读磁

    return ERR_MSR_SUCCESS;
}

// 命令取消
int CDevMSR_WBT2000::Cancel()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    // 命令取消
    INT nRet = IMP_SUCCESS;
    nRet = m_pDevWBT2000.nCancelPort(1);
    if (nRet != IMP_SUCCESS)
    {
        //INT nRet = m_pDevWBT2000.GetErrCode();
        Log(ThisModule, __LINE__, "命令取消: nCancelPort(1) fail. Return: %d ",
            ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }
    //m_pDevWBT2000.bSetup(0, 0);  // 禁止读磁

    return ERR_MSR_SUCCESS;
}

// 设置数据
int CDevMSR_WBT2000::SetData(void *vData, WORD wDataType)
{
    THISMODULE(__FUNCTION__);

    switch(wDataType)
    {
        case DATATYPE_INIT: // 0/初始化数据
        {
            LPSTMSRDEVINITPARAM lpInit = (LPSTMSRDEVINITPARAM)vData;
            memcpy(&m_stMsrDevInitParam, lpInit, sizeof(STMSRDEVINITPARAM));
            break;
        }
        case DTYPE_LIB_PATH:    // 设置Lib路径
        {
            m_pDevWBT2000.SetLibPath((LPCSTR)vData);
            break;
        }
        case SET_DEV_RECON:   // 设置断线重连标记为T
        {
            if (m_bReCon != TRUE)
            {
                Log(ThisModule, __LINE__, "设备重连 Start......");
            }
            m_bReCon = TRUE;
            m_pDevWBT2000.SetReConFlag(TRUE);
        }
        default:
            break;
    }

    return ERR_MSR_SUCCESS;
}

// 获取数据
int CDevMSR_WBT2000::GetData(void *vData, WORD wDataType)
{
    switch(wDataType)
    {
        default:
            break;
    }

    return ERR_MSR_SUCCESS;
}

// 获取版本号(1Dev版本/2固件版本/3其他)
void CDevMSR_WBT2000::GetVersion(LPSTR szVer, long lSize, ushort usType)
{
    if (usType == 1)
    {
        memcpy(szVer, byDevVRTU, strlen((char*)byDevVRTU) > lSize ? lSize : strlen((char*)byDevVRTU));
    }
    if (usType == 2)
    {
        m_pDevWBT2000.nGetFWVersion(szVer);
    }
    if (usType == 3)
    {
        m_pDevWBT2000.nGetLibVersion(szVer);
    }
}

// WBT2000错误码转换为DevMSR错误码
INT CDevMSR_WBT2000::ConvertErrCode(INT nDevCode)
{
    switch(nDevCode)
    {
        case IMP_ERR_LOAD_LIB:          /* 1 */         // 动态库加载失败
            return ERR_MSR_OTHER;                           // return 其它错误
        case IMP_ERR_PARAM_INVALID:     /* 2 */         // 参数无效
            return ERR_MSR_PARAM_ERR;                       // return 参数错误
        case IMP_ERR_UNKNOWN:           /* 3 */         // 未知错误
            return ERR_MSR_OTHER;                           // return 其它错误
        case IMP_ERR_NOTOPEN:           /* 4 */         // 设备未Open
            return ERR_MSR_NOT_OPEN;                        // return
        case IMP_DEV_ERR_SUCCESS:       /* 0 */         // 命令执行成功
            return ERR_MSR_SUCCESS;                         // return 操作成功
        case IMP_DEV_ERR_FAIL:          /* -1 */        // 命令执行失败
            return ERR_MSR_HWERR;                           // return 硬件故障
        case IMP_DEV_ERR_PARAMATER:     /* -2 */        // API参数错误
            return ERR_MSR_PARAM_ERR;                       // return 参数错误
        case IMP_DEV_ERR_WRITE:         /* -3 */        // 写数据出错
            return ERR_MSR_WRITEERROR;                      // return 写卡错
        case IMP_DEV_ERR_WTIMEOUT:      /* -4 */        // 写数据超时
            return ERR_MSR_WRITETIMEOUT;                    // return 写数据超时
        case IMP_DEV_ERR_READ:          /* -5 */        // 读数据出错
            return ERR_MSR_READERROR;                       // return 读卡错
        case IMP_DEV_ERR_RTIMEOUT:      /* -6 */        // 读数据超时
            return ERR_MSR_READTIMEOUT;                     // return 读数据超时
        case IMP_DEV_ERR_FRAME:         /* -7 */        // 数据不符合协议
            return ERR_MSR_HWERR;                           // return 硬件故障
        case IMP_DEV_ERR_UNKNOWN:       /* -8 */        // 未知错误
            return ERR_MSR_OTHER;                           // return 其它错误
        case IMP_DEV_ERR_NOSUPP:        /* -9 */        // 不支持命令
            return ERR_MSR_UNSUP_CMD;                       // return 不支持的指令
        case IMP_DEV_ERR_CANCEL:        /* -10 */       // 命令取消
            return ERR_MSR_USER_CANCEL;                     // return 用户取消进卡
        case IMP_DEV_ERR_RUNNING:       /* -11 */       // 命令运行中
            return ERR_MSR_CMD_RUNNING;                     // return 命令执行中
        default :
            return ERR_MSR_OTHER;                       //


#define IMP_SUCCESS                 0                       // 成功



#define           APINotSupportCMD        // -9:不支持命令
#define           APICMDCancel            // -10:命令取消
#define          APICMDInProgress        // -11:命令运行中
    }
}

//////////////////////////////////////////////////////////////////////////






