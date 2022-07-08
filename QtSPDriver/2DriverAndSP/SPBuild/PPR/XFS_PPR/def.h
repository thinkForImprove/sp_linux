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

//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"PPR00010100"};      // XFS_PPR 版本号
static const BYTE byDevVRTU[17] = {"Dev010100"};        // DevPTR 版本号

// 设备类型
#define DEV_MB2                 0               // MB2存折打印机
#define DEV_PRM                 1               // PRM存折打印机

#define IDEV_MB2                "MB2"           // MB2存折打印机
#define IDEV_PRM                "PRM"           // PRM存折打印机

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_PRT_FORMAT          51              // 设置打印格式
#define SET_DEV_RECON           52              // 设置断线重连标记
#define SET_DEV_OPENMODE        53              // 设备打开模式
#define SET_DEV_BEEP            54              // 设置鸣响
#define SET_DEV_PARAM           55              // 设置参数

// GetVersion()使用
#define GET_VER_DEV             1               // DevPPR版本号
#define GET_VER_FW              2               // 固件版本号

// 一个字节(char)按8位获取数据定义
#define     DATA_BIT0       (0x01)  // 第一位
#define     DATA_BIT1       (0x02)
#define     DATA_BIT2       (0x04)
#define     DATA_BIT3       (0x08)
#define     DATA_BIT4       (0x10)
#define     DATA_BIT5       (0x20)
#define     DATA_BIT6       (0x40)
#define     DATA_BIT7       (0x80)

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

//----------------------MB2打印机相关定义-----------------------
typedef struct st_Ini_Config_MB2
{
    WORD    wPrintQuality;      // 打印质量
    WORD    wTrack2Type;
    WORD    wTrack3Type;
    WORD    wScanDPI;           // 扫描分辨率控制(200/300/600)
    WORD    wCisColorMode;      // 扫描光调色模式
    WORD    wGrayMode;          // 扫描灰度模式模式
    WORD    wBrightness;        // 扫描亮度
    WORD    wThresholdLevel;    // 扫描黑白包容度
    WORD    wScanDirection;     // 扫描图像的方向
    st_Ini_Config_MB2()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Ini_Config_MB2));
        wTrack2Type = 0;
        wTrack3Type = 1;
    }
} STINICONFIG_MB2, *LPSTINICONFIG_MB2;

//----------------------PRM打印机相关定义-----------------------
typedef struct st_Ini_Config_PRM
{
    WORD    wPrintDataMode;     // SDK打印命令返回模式
    WORD    wFuncWaitTime;      // SDK接口等待时间
    st_Ini_Config_PRM()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Ini_Config_PRM));
    }
} STINICONFIG_PRM, *LPSTINICONFIG_PRM;

//----------------------打印机鸣响相关定义-----------------------
typedef struct st_Config_Beep
{
    WORD    wSupp;              // 是否支持鸣响
    WORD    wInterval;          // 每次间隔时间(单位:毫秒)
    WORD    wCount;             // 每次鸣响次数
    st_Config_Beep()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Config_Beep));
    }
} STCONFIGBEEP, *LPSTCONFIGBEEP;


#endif // DEF_H
