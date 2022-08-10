#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>
#include "IDevIDC.h"

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义
//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"MSR00020100"};      // XFS_MSR 版本号
static const BYTE byDevVRTU[17] = {"Dev020100"};        // DevMSR 版本号

// 设备类型
#define XFS_TYPE_WBT2172            0               // WBT-2172-ZD
#define XFS_TYPE_WBCS10             1               // WB-CS

#define IDEV_TYPE_WBT2172           "0"             // WBT-2172-ZD
#define IDEV_TYPE_WBCS10            "1"             // WB-CS


#define DEVTYPE_CHG(a)  a == XFS_TYPE_WBT2172 ? IDEV_TYPE_WBT2172 : (a == XFS_TYPE_WBCS10 ? IDEV_TYPE_WBCS10 :"")

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)


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
