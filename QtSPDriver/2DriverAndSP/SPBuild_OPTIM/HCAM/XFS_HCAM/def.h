/***************************************************************
* 文件名称：def.h
* 文件描述：用于声明 XFS_XXX 与 DevXXX 共用的变量定义
* 版本历史信息
* 变更说明：
* 变更日期：
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

#include "IDevCAM.h"

//-------------------------------宏定义------------------------------------------------

static const BYTE byXFSVRTU[17] = {"HCAM00010100"};     // XFS_HCAM 版本号
static const BYTE byDevVRTU[17] = {"Dev010100"};        // DevHCAM 版本号

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_DEV_VIDEOMODE           51              // 设置设备摄像模式

// GetVersion()使用
#define GET_VER_FW                  1               // 固件版本号

//-------------------------------环境摄像模块(HCAM)相关声明-------------------------------
// 设备类型
#define XFS_JDY5001A0809        0                   // JDY-5001A-0809(健德源单目)

#define IDEV_JDY5001A0809       "JDY-5001A-0809"    // JDY-5001A-0809(健德源单目)

// INI中指定[int型]设备类型 转换为 STR型(用于加载DevHCAM动态库使用)
#define DEVTYPE2STR(n) \
    (n == XFS_JDY5001A0809 ? IDEV_JDY5001A0809 : "")

// 摄像保存类型
#define PIC_BASE64  1       // BASE64格式
#define PIC_JPG     2       // JPG格式
#define PIC_BMP     4       // BMP格式

// 银行类型
#define BANK_NOALL  0       // 通用


// 摄像数据共享内存名
#define SHAREDMEMNAME_CAMDATA           "HCamShowSharedMemData"
// 摄像数据共享内存Size
#define SHAREDMEMSIZE_CAMDATA           1024 * 1024 * 10

// 窗口显示标记
enum EN_DISPLAY_STAT
{
    EN_DISP_SHOWING     = 0,    // 显示中
    EN_DISP_ENDING      = 1,    // 结束
    EN_DISP_PAUSE       = 2,    // 暂停
    EN_DISP_CREATE      = 3,    // 创建
};


// 初始化参数结构体
typedef struct ST_INIT_PARAM
{
    CHAR szParStr[32][128];
    INT  nParInt[32];
    LONG lParLong[32];

    ST_INIT_PARAM()
    {
        memset(this, 0x00, sizeof(ST_INIT_PARAM));
    }
} STINITPARAM, *LPSTINITPARAM;


// 摄像窗口参数信息结构体(用于SP创建摄像窗口)
typedef struct st_cam_showwin_info
{
    DWORD   hWnd;       // 窗口ID
    WORD    wX;         // 坐标X
    WORD    wY;         // 坐标Y
    WORD    wWidth;     // 窗口宽
    WORD    wHeight;    // 窗口高
    //WORD    wWinFlag;   // 窗口识别标记
    st_cam_showwin_info()
    {
        memset(this, 0x00, sizeof(st_cam_showwin_info));
    }

} STSHOWWININFO, *LPSTSHOWWININFO;

// 设备摄像参数结构体
#define SET_VM_INV      99999999.00
typedef struct ST_VIDEO_PARAM
{
    DOUBLE duWidth;         // 宽度
    DOUBLE duHeight;        // 高度
    DOUBLE duFPS;           // 帧率(帧/秒)
    DOUBLE duBright;        // 亮度(-255 ~ 255)(RGB相关)
    DOUBLE duContrast;      // 对比度
    DOUBLE duSaturation;    // 饱和度
    DOUBLE duHue;           // 色调
    DOUBLE duExposure;      // 曝光

    ST_VIDEO_PARAM()
    {
        memset(this, SET_VM_INV, sizeof(ST_VIDEO_PARAM));
    }

    void Clear()
    {
        memset(this, SET_VM_INV, sizeof(ST_VIDEO_PARAM));
    }
} STVIDEOPAMAR, *LPSTVIDEOPARAM;

// INI配置变量结构体
typedef struct ST_CAM_INI_CONFIG
{
    CHAR                    szDevDllName[256];                  // DevHCAM动态库名
    WORD                    wDeviceType;                        // 设备类型
    STDEVICEOPENMODE        stDevOpenMode;                      // 设备打开模式
    CHAR                    szSDKPath[256];                     // 设备SDK库路径
    // INI配置Config相关
    WORD                    wDevNotFoundStat;                   // 设备未连接时状态显示
    WORD                    wFrameResoWidth;                    // 截取画面帧的分辨率(Width)
    WORD                    wFrameResoHeight;                   // 截取画面帧的分辨率(Height)
    // INI配置Open相关
    WORD                    wOpenFailRet;                       // Open失败时返回值
    // INI配置共享内存相关
    CHAR                    szCamDataSMemName[32+1];            // 共享内存名
    ULONG                   ulCamDataSMemSize;                  // 共享内存大小(字节)
    // INI配置TakePic命令相关
    DWORD                   dwTakePicTimeOut;                   // 命令超时
    CHAR                    szTakePicDefSavePath[MAX_PATH];     // TakePicture命令缺省保存目录
    // INI配置复位相关
    WORD                    wResetCloseDiaplay;                 // Reset执行时是否支持关闭摄像窗口(0:不支持,1支持,缺省0)
    // INI配置银行相关
    WORD                    wBank;                              // 指定银行
    // INI配置错误码相关
    WORD                    wErrDetailShowSup;                  // 是否支持Status显示错误码
    CHAR                    szErrDetailKeyName[32];             // 当前错误码Key名
    CHAR                    szLastErrDetailKeyName[32];         // 上一次错误码Key名
    // 其他
    UINT                    unWinRefreshTime;                   // 摄像刷新时间(云从使用)
    STINITPARAM             stInitPar;                          // DevXXX初始化参数
    STVIDEOPAMAR            stVideoPar;                         // 设备摄像参数

    ST_CAM_INI_CONFIG()
    {
        memset(this, 0x00, sizeof(ST_CAM_INI_CONFIG));
    }
} STINICONFIG, *LPSTINICONFIG;

#endif /* DEF_H */



