#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义
//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"MSR00010100"};      // XFS_MSR 版本号
static const BYTE byDevVRTU[17] = {"Dev010100"};        // DevMSR 版本号

// 设备类型
#define IDEV_TYPE_WBT2172       "0"             // WBT-2172-ZD
#define IDEV_TYPE_WBCS10        "1"             // WB-CS

#define DEV_TYPE_WBT2172         0             // WBT-2172-ZD
#define DEV_TYPE_WBCS10          1             // WB-CS

#define DEVTYPE_CHG(a)  a == 0 ? IDEV_TYPE_WBT2172 : (a == 1 ? IDEV_TYPE_WBCS10 :"")

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_DEV_RECON           51              // 设置断线重连标记

// GetVersion()使用
#define GET_VER_DEVRPR          1               // DevMSR版本号
#define GET_VER_FW              2               // 固件版本号

// 设备打开方式结构体变量 声明
typedef
struct st_Device_OpenMode
{
    WORD    wOpenMode;          // 设备打开方式(0串口/1USB HID)
    CHAR    szDevPath[64+1];    // 设备路径
    WORD    wBaudRate;          // 设备串口波特率
    CHAR    szHidVid[32+1];     // 设备VID
    CHAR    szHidPid[32+1];     // 设备PID

    st_Device_OpenMode()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Device_OpenMode));
    }
} STDEVICEOPENMODE, *LPSTDEVICEOPENMODE;

// 设备SDKlog相关结构体变量 声明
typedef
struct st_Device_Log
{
    WORD    wEnableLog;         // 是否启用动态库底层处理日志记录
    WORD    wLogLevel;          // 日志记录级别
    CHAR    szLogPath[256+1];   // 日志路径

    st_Device_Log()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Device_Log));
    }
} STDEVICELOG, *LPSTDEVICELOG;

// DEVMSR初始化参数结构体
typedef struct st_msr_dev_init_param
{
    STDEVICEOPENMODE    stDeviceOpenMode;   // 设备打开方式
    STDEVICELOG         stDeviceLog;        // 设备Log信息

    st_msr_dev_init_param()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_msr_dev_init_param));
    }
} STMSRDEVINITPARAM, *LPSTMSRDEVINITPARAM;


#endif // DEF_H
