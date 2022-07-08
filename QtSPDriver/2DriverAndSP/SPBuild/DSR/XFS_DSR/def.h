/***************************************************************
* 文件名称：def.h
* 文件描述：用于声明 XFS_XXX 与 DevXXX 共用的变量定义
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

//----------------------无分类定义-----------------------
// XFS_DSR 版本号
const BYTE byVRTU_XFS[17] = {"HWDSRSTE01000100"};

// DevDSR 版本号
const BYTE  byVRTU_DEV[17]  = "DevDSR01000000";

#define DEV_BSD216              0               // SNBC BS-D216型嵌入式A4扫描仪
#define IDEV_BSD216             "BSD216"

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_PRT_FORMAT          51              // 设置打印格式
#define SET_DEV_RECON           52              // 设置断线重连标记
#define SET_DEV_OPENMODE        53              // 设备打开模式
#define SET_DEV_BEEP            54              // 设置鸣响
#define SET_DEV_PARAM           55              // 设置参数

// GetVersion()使用
#define GET_VER_DEV             1               // DevDSR版本号
#define GET_VER_FW              2               // 固件版本号

// 设备打开模式
typedef struct st_DevOpen_mode
{
    INT nOpenMode;          // 打开模式
    INT nOpenParam;         // 模式参数
    CHAR szOpenParam[64];   // 模式参数
    st_DevOpen_mode()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_DevOpen_mode));
    }
} STDEVOPENMODE, *LPSTDEVOPENMODE;

#endif // DEF_H
