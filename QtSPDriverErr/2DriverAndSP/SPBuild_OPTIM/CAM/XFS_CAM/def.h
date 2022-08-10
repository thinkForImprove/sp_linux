/***************************************************************************
* 文件名称: def.h
* 文件描述: 用于声明 XFS_XXX 与 DevXXX 共用的变量定义
* 版本历史信息
* 变更说明:
* 变更日期:
* 文件版本: 1.0.0.1
***************************************************************************/
#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

#include "IDevCAM.h"

/***************************************************************************
********************************* 宏定义 ************************************
***************************************************************************/

static const BYTE byXFSVRTU[17] = {"CAM00020100"};      // XFS_HCAM 版本号
static const BYTE byDevVRTU[17] = {"Dev020100"};        // DevHCAM 版本号

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_DEV_VIDEOMODE      51              // 设置设备摄像模式
#define GET_DEV_LIVESTAT       52              // 取活检状态

// 设备类型(用于XFS_XXX)(INI中配置)
#define XFS_YC0C98              1           // 云从双目摄像(YC0C98)
#define XFS_TCF261              2           // 天诚盛业双目摄像(TCF261)
#define XFS_ZLF1000A3           3           // 哲林高拍仪(ZLF1000A3)
#define XFS_JDY5001A0809        4           // 健德源单目(JDY-5001A-0809)

// 设备类型(用于DevXXX转换)
#define IDEV_YC0C98             "YC0C98"            // 云从双目摄像(YC0C98)
#define IDEV_TCF261             "TCF261"            // 天诚盛业双目摄像(TCF261)
#define IDEV_ZLF1000A3          "ZLF1000A3"         // 哲林高拍仪(ZLF1000A3)
#define IDEV_JDY5001A0809       "JDY-5001A-0809"    // JDY-5001A-0809(健德源单目)

// INI中指定[int型]设备类型 转换为 STR型(用于加载DevCAM动态库使用)
#define DEVTYPE2STR(n) \
    (n == XFS_YC0C98 ? IDEV_YC0C98 : \
     (n == XFS_TCF261 ? IDEV_TCF261 : \
      (n == XFS_ZLF1000A3 ? IDEV_ZLF1000A3 : \
        (n == XFS_JDY5001A0809 ? IDEV_JDY5001A0809 : ""))))


// 摄像图片保存类型
#define PIC_BASE64              1           // BASE64格式
#define PIC_JPG                 2           // JPG格式
#define PIC_BMP                 4           // BMP格式

// 摄像数据共享内存名/Size
#define SHAREDMEMNAME_CAMDATA   "CamShowSharedMemData"
#define SHAREDMEMSIZE_CAMDATA   1024 * 1024 * 10



/***************************************************************************
******************************* 结构体定义 **********************************
***************************************************************************/
// 窗口显示标记
enum EN_DISPLAY_STAT
{
    EN_DISP_SHOWING     = 0,    // 显示中
    EN_DISP_ENDING      = 1,    // 结束
    EN_DISP_PAUSE       = 2,    // 暂停
    EN_DISP_CREATE      = 3,    // 创建
};


// 初始化参数结构体(不专门指定用处, 可自由使用)
typedef struct ST_INIT_PARAM
{
    CHAR szParStr[64][128];     // 可设置64组字符串参数
    INT  nParInt[64];           // 可设置64组整型参数
    LONG lParLong[64];          // 可设置64组长整型参数
    FLOAT fParLong[64];         // 可设置64组浮点型参数

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
    INT nOtherParam[32];    // 其他参数(0用于镜像转换模式)

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
    CHAR                    szDevDllName[256];                  // DevCAM动态库名
    WORD                    wDeviceType;                        // 设备类型
    STDEVICEOPENMODE        stDevOpenMode[32];                  // 设备打开模式(0~3参考宏定义LIDX_XXX,4~31保留备用)
    CHAR                    szSDKPath[32][256];                 // 设备SDK路径
    // INI配置Config相关
    WORD                    wDevNotFoundStat;                   // 设备未连接时状态显示
    // INI配置DEVICE_CFG相关
    WORD                    wDeviceRoomType;                    // 环境摄像设备类型
    WORD                    wDevicePersonType;                  // 人脸摄像设备类型
    WORD                    wDeviceExitSlotType;                // 出口槽摄像设备类型
    WORD                    wDeviceExtraType;                   // 扩展摄像设备类型
    WORD                    wDeviceHightType;                   // 高拍摄像设备类型
    WORD                    wDevicePanoraType;                  // 全景摄像设备类型
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
    // INI配置事件相关
    WORD                    wLiveErrorSup;                      // 是否支持上报[活体图像检测错误事件]
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


// 注: STINITPARAM(初始化参数结构体)在CAM模块中各下标为用途
//   szParStr[0]: 摄像数据共享内存名
//   szParStr[10]: 特殊的处理方式1
//   szParStr[11]: 特殊的处理方式2
//   lParLong[0]: 摄像数据共享内存Size
//   nParInt[1]: 指定银行
//   nParInt[2]: Display采用图像帧方式刷新时,取图像帧数据接口错误次数上限

// STDEVICEOPENMODE(设备Open参数结构体)在CAM模块中各下标为用途
//   其他参数下标0~9用于共通, 10及以上用于各设备自行选用
//   nOtherParam[0]: 保存设备类型编号
//   nOtherParam[1]: 摄像刷新时间
//   nOtherParam[2]: 水平分辨率
//   nOtherParam[3]: 垂直分辨率
//   nOtherParam[4]: 镜像转换

// -------------------------------------- END --------------------------------------
