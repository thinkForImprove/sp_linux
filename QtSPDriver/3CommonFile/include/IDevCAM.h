/***************************************************************
* 文件名称：IDevCAM.h
* 文件描述：声明Camera底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/
#if !defined(SPBuild_OPTIM)     // 非优化工程

#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include <string.h>

#if defined(IDEVCAM_LIBRARY)
#define DEVCAM_EXPORT Q_DECL_EXPORT
#else
#define DEVCAM_EXPORT Q_DECL_IMPORT
#endif

//////////////////////////////////////////////////////////////////////////
// 返回码
#define CAM_SUCCESS          (0)
#define ERR_CONFIG           (-1)
#define ERR_OPEN_CAMER       (-2)
#define ERR_INVALID_PARAM    (-3)
#define ERR_LOAD_IMAGE       (-4)
#define ERR_FRAME            (-5)
#define ERR_IMWRITE          (-6)
#define ERR_SAVE_IMAGE       (-7)
#define ERR_OTHER            (-8)
#define ERR_TIMEOUT          (-9)
#define ERR_SHAREDMEM        (-10)
#define ERR_PROCESS          (-11)
#define ERR_CANCEL           (-12)	// 30-00-00-00(FT#0031)
#define ERR_NOTSUPP          (-99)

// SetData/GetData 数据类型
#define DATATYPE_INIT           0   // 初始化数据
#define DATATYPE_PERSON         1   // 增加人脸摄像处理
#define DATATYPE_LIVESTAT       2   // 活检状态
#define DATATYPE_SET_IMAGEPAR   3   // 设置图像参数

// Display Action参数
#define CAMERA_OPEN     0   // 打开
#define CAMERA_CLOSE    1   // 关闭
#define CAMERA_PAUSE    2   // 暂停
#define CAMERA_RESUME   3   // 恢复

// TakePicture 摄像模式
#define TAKEPIC_PERSON  0x01    // 人脸摄像
#define TAKEPIC_ROOM    0x02    // 全景摄像
#define TAKEPIC_FILE    0x04    // 资料补拍 长沙高存特殊要求
#define TAKEPIC_HIGH    0x08    // 高拍摄像

//////////////////////////////////////////////////////////////////////////
#define CAM_CAMERAS_SIZE     (8)
#define MAX_SIZE             (400)
#define MAX_CAMDATA          (4096)
//////////////////////////////////////////////////////////////////////////
enum DEVICE_STATUS
{
    DEVICE_ONLINE                   = 0,
    DEVICE_OFFLINE                  = 1,
    DEVICE_HWERROR                  = 2,
    DEVICE_BUSY                     = 3,
    DEVICE_UNKNOWN                  = 4,	// 30-00-00-00(FT#0031)
    DEVICE_NODEVICE                 = 5,	// 30-00-00-00(FT#0031)
};

enum MEDIA_STATUS
{
    MEDIA_OK                        = 0,
    MEDIA_HIGH                      = 1,
    MEDIA_FULL                      = 2,
    MEDIA_UNKNOWN                   = 3,
    MEDIA_NOTSUPP                   = 4
};

enum CAM_POS
{
    CAM_ROOM                        = 0,
    CAM_PERSON                      = 1,
    CAM_EXITSLOT                    = 2
};

enum CAM_STATUS
{
    STATUS_NOTSUPP                  = 0,
    STATUS_OK                       = 1,
    STATUS_INOP                     = 2,
    STATUS_UNKNOWN                  = 3
};

typedef struct tag_dev_cam_status
{
    WORD   fwDevice;
    WORD   fwMedia[CAM_CAMERAS_SIZE];
    WORD   fwCameras[CAM_CAMERAS_SIZE];
    USHORT usPictures[CAM_CAMERAS_SIZE];
    char   szErrCode[8];    // 三位的错误码

    tag_dev_cam_status() { Clear(); }
    void Clear()
    {
        memset(this, 0x00, sizeof(tag_dev_cam_status));
        fwDevice = DEVICE_NODEVICE;
        for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
            fwMedia[i] = MEDIA_UNKNOWN;
        for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
            fwCameras[i] = MEDIA_UNKNOWN;
        for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
            usPictures[i] = CAM_ROOM;
    }
} DEVCAMSTATUS, *LPDEVCAMSTATUS;



//////////////////////////////////////////////////////////////////////////
//#define DVECAM_NO_VTABLE  __declspec(novtable)
//////////////////////////////////////////////////////////////////////////
struct /*DVECAM_NO_VTABLE*/ IDevCAM
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
    virtual long Open(LPCSTR lpMode) = 0;
    // 关闭连接
    virtual long Close() = 0;
    // 复位
    virtual long Reset() = 0;
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo) = 0;
    // 命令取消
    virtual long Cancel() = 0;	// 30-00-00-00(FT#0031)
    // 取状态
    virtual long GetStatus(DEVCAMSTATUS &stStatus) = 0;
    // 打开窗口(窗口句柄，动作指示:1创建窗口/0销毁窗口, X/Y坐标,窗口宽高)
    virtual long Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wHidth, WORD wHeight)
    {
        return ERR_NOTSUPP;
    }
    // 打开窗口(窗口句柄，动作指示:1创建窗口/0销毁窗口, X/Y坐标,窗口宽高,水平/垂直像素)
    virtual long Display2(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wHidth, WORD wHeight,
                          WORD wHpixel, WORD wVpixel)
    {
        return ERR_NOTSUPP;
    }

    // 拍照 传下来的字符lpData按：”FileName=C:\TEMP\TEST;Text=HELLO;TextPositionX=160;TextPositionY=120;CaptureType=0”格式
    virtual long TakePicture(WORD wCamera, LPCSTR lpData) = 0;
    // 拍照(保存文件名,水印字串，图片类型是否连续检测)(1BASE64/2JPG/4BMP;T连续检测)
#if defined(SET_BANK_CMBC)
    virtual long TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, DWORD dwTimeOut) = 0;
#else
    virtual long TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue,
                               DWORD wTimeOut, WORD wCamara = TAKEPIC_PERSON) = 0;
#endif

    // 设置数据
    virtual long SetData(void *vData, WORD wDataType = 0) = 0;

    // 获取数据
    virtual long GetData(void *vData, WORD wDataType = 0) = 0;

    // 版本号(1DevCam版本/2固件版本/3其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType) = 0;
};

extern "C" DEVCAM_EXPORT long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&p);

#else // 优化工程

#pragma once

#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "IDevDEF.h"
#include <string.h>

#include <XFSCAM.H>

#if defined(IDEVCAM_LIBRARY)
#define DEVCAM_EXPORT Q_DECL_EXPORT
#else
#define DEVCAM_EXPORT Q_DECL_IMPORT
#endif

//****************************************************************************
// 错误码相关定义
//****************************************************************************
// 成功
#define CAM_SUCCESS                     0

// 警告
#define ERR_CAM_USER_CANCEL             1       // 用户取消

// 通用错误(-1 ~ -99)
#define ERR_CAM_LIBRARY                 -1      // 动态库加载失败
#define ERR_CAM_PARAM_ERR               -2      // 参数错误
#define ERR_CAM_UNSUP_CMD               -3      // 不支持的指令/接口
#define ERR_CAM_DATA_FAIL               -4      // 数据错误
#define ERR_CAM_TIMEOUT                 -5      // 超时
#define ERR_CAM_USER_ERR                -6      // 用户使用错误
#define ERR_CAM_OTHER                   -98     // 其它错误(知道错误原因,不细分类别)
#define ERR_CAM_UNKNOWN                 -99     // 未知错误(不知原因的错误)

// 其他错误
#define ERR_CAM_NOTOPEN                 -100    // 设备未打开
#define ERR_CAM_OPENFAIL                -101    // 设备打开失败
#define ERR_CAM_NODEVICE                -102    // 设备不存在
#define ERR_CAM_OFFLINE                 -103    // 设备断线
#define ERR_CAM_PROCESS                 -104    // 进程/线程错误
#define ERR_CAM_SAVE_IMAGE              -105    // 图像保存错误
#define ERR_CAM_LOAD_IMAGE              -106    // 图像无法加载
#define ERR_CAM_SHAREDMEM               -107    // 共享内存错误
#define ERR_CAM_LIVEDETECT              -108    // 活体检测失败

// 用于标记display/TakePicture接口摄像模式入参
#define DEV_CAM_MODE_ROOM               0       //
#define DEV_CAM_MODE_PERSON             1       //
#define DEV_CAM_MODE_EXITSLOT           2       //
#define DEV_CAM_MODE_EXTRA              3       //
#define DEV_CAM_MODE_HIGHT              4       //
#define DEV_CAM_MODE_PANORA             5       //

// LiveError: 活检图像状态事件错误码
#define LIVE_ERR_OK                 0   // 活检图像正常
#define LIVE_ERR_VIS_NOFACE         1   // 可见光未检测到人脸
#define LIVE_ERR_FACE_SHELTER       2   // 人脸有遮挡(五官有遮挡,戴镜帽等)
#define LIVE_ERR_FACE_ANGLE_FAIL    3   // 人脸角度不满足要求(低头/抬头/侧脸等)
#define LIVE_ERR_FACE_EYECLOSE      4   // 检测到闭眼
#define LIVE_ERR_FACE_MOUTHOPEN     5   // 检测到张嘴
#define LIVE_ERR_FACE_SHAKE         6   // 检测到人脸晃动/模糊
#define LIVE_ERR_FACE_MULTIPLE      7   // 检测到多张人脸
#define LIVE_ERR_IS_UNLIVE          8   // 检测到非活体
#define LIVE_ERR_FACE_TOOFAR        9   // 人脸离摄像头太远
#define LIVE_ERR_FACE_TOONEAR       10  // 人脸离摄像头太近
#define LIVE_ERR_NIS_NOFACE         11  // 红外光未检测到人脸
#define LIVE_ERR_UNKNOWN            99  // 其他/未知错误


//***************************************************************************
// 使用 IDevDEF.h 通用定义部分
// 1. SetData/GetData 数据类型 (0~50为共通使用, 50以上为各模块自行定义)
// 2. GetVersion 数据类型
// 3. 设备打开方式结构体变量, 适用于 SET_DEV_OPENMODE 参数传递
//***************************************************************************


//****************************************************************************
// 设备状态相关定义
//****************************************************************************
//　Status.Device返回状态(引用IDevDEF.h中已定义类型)
typedef EN_DEVICE_STATUS    DEVCAM_DEVICE_STATUS;

//　status.Media返回状态(介质状态)
enum DEVCAM_MEDIA_STATUS
{
    CAM_MEDIA_STAT_OK               = 0,    // 介质正常
    CAM_MEDIA_STAT_HIGH             = 1,    // 介质将满
    CAM_MEDIA_STAT_FULL             = 2,    // 介质已满
    CAM_MEDIA_STAT_UNKNOWN          = 3,    // 介质无法确定状态
    CAM_MEDIA_STAT_NOTSUPP          = 4,    // 介质不支持状态检查
};

//　status.Cameras返回状态(相机状态)
enum DEVCAM_CAMEAR_STATUS
{
    CAM_CAMERA_STAT_NOTSUPP         = 0,    // 相机不支持
    CAM_CAMERA_STAT_OK              = 1,    // 相机状态正常
    CAM_CAMERA_STAT_INOP            = 2,    // 相机状态异常
    CAM_CAMERA_STAT_UNKNOWN         = 3,    // 相机状态未知
};

// 设备状态结构体
#define CAM_CAMERAS_SIZE    WFS_CAM_CAMERAS_SIZE
typedef struct ST_DEV_CAM_STATUS   // 处理后的设备状态
{
    WORD    wDevice;                            // 设备状态(参考enum DEVCAM_DEVICE_STATUS)
    WORD    wMedia[CAM_CAMERAS_SIZE];           // Media状态(参考enum DEVCAM_MEDIA_STATUS)
    WORD    fwCameras[CAM_CAMERAS_SIZE];        // 相机状态(参考enum DEVCAM_DEVICE_STATUS)
    USHORT  usPictures[CAM_CAMERAS_SIZE];       // 指定存储在相机记录介质上的图片数
    CHAR    szErrCode[8];                       // 8位的错误码
    INT     nOtherCode[32];                     // 其他状态值,用于非标准WFS/未定义值的返回

    ST_DEV_CAM_STATUS()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(ST_DEV_CAM_STATUS));
        wDevice = DEVICE_STAT_OFFLINE;
        for (INT i = 0; i < CAM_CAMERAS_SIZE; i ++)
        {
            wMedia[i] = CAM_MEDIA_STAT_UNKNOWN;
        }
        for (INT i = 0; i < CAM_CAMERAS_SIZE; i ++)
        {
            fwCameras[i] = CAM_MEDIA_STAT_UNKNOWN;
        }
        for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        {
            usPictures[i] = 0;
        }
    }

    int Diff(struct ST_DEV_CAM_STATUS stStat)
    {
        if (wDevice != stStat.wDevice)
        {
            return 1;
        }
        for (INT i = 0; i < CAM_CAMERAS_SIZE; i ++)
        {
            if (stStat.wMedia[i] != wMedia[i] ||
                stStat.fwCameras[i] != fwCameras[i] ||
                stStat.usPictures[i] != usPictures[i])
            {
                return 1;
            }
        }
        return 0;
    }

    int Copy(struct ST_DEV_CAM_STATUS stStat)
    {
        wDevice = stStat.wDevice;
        for (INT i = 0; i < CAM_CAMERAS_SIZE; i ++)
        {
            wMedia[i] = stStat.wMedia[i];
            fwCameras[i] = stStat.fwCameras[i];
            usPictures[i] = stStat.usPictures[i];
        }
        return 0;
    }

} STDEVCAMSTATUS, *LPSTDEVCAMSTATUS;

// Display Action参数
enum EN_DISPLAY_ACTION
{
    CAMERA_OPEN         = 0,    // 打开
    CAMERA_CLOSE        = 1,    // 关闭
    CAMERA_PAUSE        = 2,    // 暂停
    CAMERA_RESUME       = 3,    // 恢复
};

// 事件相关参数(事件ID)
enum EN_EVENT_ID
{
    EN_EVENTID_HWERR    = 1,    // 硬件错误
    EN_EVENTID_SOFTERR  = 2,    // 软件错误
    EN_EVENTID_USERERR  = 3,    // 用户错误
};

// 事件相关参数(错误管理动作)
enum EN_EVENT_ACT
{
    EN_EVENTACT_NOACTION= 1,    // 可自动修复
    EN_EVENTACT_RESET   = 2,    // 重新设置
    EN_EVENTACT_SWERROR = 3,    // 软件错误
    EN_EVENTACT_CONFIG  = 4,    // 配置错误
};

//****************************************************************************
// 命令参数相关定义
//****************************************************************************
// Display/DisplayEx命令入参
typedef struct ST_DISPLAY_PARAM
{
    DWORD   dwTimeOut;          // 超时时间
    DWORD   hWnd;               // 窗口句柄
    WORD    wCamera;            //
    WORD    wAction;            // 窗口控制
    WORD    wX;                 // 窗口左上角X坐标
    WORD    wY;                 // 窗口左上角Y坐标
    WORD    wWidth;             // 窗口高度
    WORD    wHeight;            // 窗口宽度
    WORD    wHpixel;            // 水平像素值
    WORD    wVpixel;            // 垂直像素值
} STDISPLAYPAR, LPSTDISPLAYPA;

// TakePicture/TakePictureEx命令入参
typedef struct ST_TAKEPICTURE_PARAM
{
    DWORD   dwTimeOut;          // 超时时间
    WORD    wCameraAction;      // 拍照模式
    CHAR    szFileName[256];    // 保存文件名
    CHAR    szCamData[256];     // 水印字串
    WORD    wPicType;           // 图片类型
    BOOL    bIsContinue;        // 是否连续监测
} STTAKEPICTUREPAR, LPSTTAKEPICTUREPAR;

//****************************************************************************
// 接口类定义
//****************************************************************************
struct IDevCAM
{
    // 释放接口
    virtual int Release()
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 打开连接
    virtual int Open(LPCSTR lpMode)
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 关闭连接
    virtual int Close()
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 复位
    virtual int Reset()
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 命令取消
    virtual int Cancel(unsigned short usMode = 0)
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 取状态
    virtual int GetStatus(STDEVCAMSTATUS &stStatus)
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 摄像窗口处理(stDisplayIn: 入参参数, vParam: 其他参数)
    virtual int Display(STDISPLAYPAR stDisplayIn, void *vParam = nullptr)
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 摄像拍照处理(stTakePicIn: 入参参数, vParam: 其他参数)
    virtual int TakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam = nullptr)
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 设置数据
    virtual int SetData(unsigned short usType, void *vData = nullptr)
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData)
    {
        return ERR_CAM_UNSUP_CMD;
    }
    // 取版本号
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize)
    {
        return ERR_CAM_UNSUP_CMD;
    }
};

extern "C" DEVCAM_EXPORT long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&p);


//****************************************************************************
// IDevCAM定义 设备相关变量转换 类定义(统一转换)
//****************************************************************************
class ConvertVarCAM
{
private:
    CHAR m_szErrStr[1024];
public:
    // 设备状态转换为WFS格式
    WORD ConvertDeviceStatus2WFS(WORD wDevStat)
    {
        switch (wDevStat)
        {
            case DEVICE_STAT_ONLINE     /* 设备正常 */     : return WFS_CAM_DEVONLINE;
            case DEVICE_STAT_OFFLINE    /* 设备脱机 */     : return WFS_CAM_DEVOFFLINE;
            case DEVICE_STAT_POWEROFF   /* 设备断电 */     : return WFS_CAM_DEVPOWEROFF;
            case DEVICE_STAT_NODEVICE   /* 设备不存在 */    : return WFS_CAM_DEVNODEVICE;
            case DEVICE_STAT_HWERROR    /* 设备故障 */     : return WFS_CAM_DEVHWERROR;
            case DEVICE_STAT_USERERROR  /* 用户操作错误 */  : return WFS_CAM_DEVUSERERROR;
            case DEVICE_STAT_BUSY       /* 设备读写中 */    : return WFS_CAM_DEVBUSY;
            case DEVICE_STAT_FRAUDAT    /* 设备出现欺诈企图 */: return WFS_CAM_DEVFRAUDATTEMPT;
            defaule: return WFS_CAM_DEVOFFLINE;
        }
    }

    // Media状态转换为WFS格式
    WORD ConvertMediaStatus2WFS(WORD wMediaStat)
    {
        switch (wMediaStat)
        {
            case CAM_MEDIA_STAT_OK          /* 介质正常 */         : return WFS_CAM_MEDIAOK;
            case CAM_MEDIA_STAT_HIGH        /* 介质将满 */         : return WFS_CAM_MEDIAHIGH;
            case CAM_MEDIA_STAT_FULL        /* 介质已满 */         : return WFS_CAM_MEDIAFULL;
            case CAM_MEDIA_STAT_UNKNOWN     /* 介质无法确定状态 */   : return WFS_CAM_MEDIAUNKNOWN;
            case CAM_MEDIA_STAT_NOTSUPP     /* 介质不支持状态检查 */ : return WFS_CAM_MEDIANOTSUPP;
            default: return WFS_CAM_MEDIAUNKNOWN;
        }
    }

    // Cameras状态转换为WFS格式
    WORD ConvertCamerasStatus2WFS(WORD wRetCamerasStat)
    {
        switch (wRetCamerasStat)
        {
            case CAM_CAMERA_STAT_NOTSUPP    /* 相机不支持 */     : return WFS_CAM_CAMNOTSUPP;
            case CAM_CAMERA_STAT_OK         /* 相机状态正常 */   : return WFS_CAM_CAMOK;
            case CAM_CAMERA_STAT_INOP       /* 相机状态异常 */   : return WFS_CAM_CAMINOP;
            case CAM_CAMERA_STAT_UNKNOWN    /* 相机状态未知 */   : return WFS_CAM_CAMUNKNOWN;
            default: return WFS_CAM_CAMUNKNOWN;
        }
    }
    // 错误码转换为WFS格式
    LONG ConvertDevErrCode2WFS(INT nRet)
    {
        switch (nRet)
        {
            // 成功
            case CAM_SUCCESS:               return WFS_SUCCESS;                 // 操作成功->成功
            // 警告
            case ERR_CAM_USER_CANCEL:       return WFS_ERR_CANCELED;            // 用户取消
            // 通用错误
            case ERR_CAM_LIBRARY:           return WFS_ERR_SOFTWARE_ERROR;      // 动态库加载失败
            case ERR_CAM_PARAM_ERR:         return WFS_ERR_INVALID_DATA;        // 参数错误
            case ERR_CAM_UNSUP_CMD:         return WFS_ERR_UNSUPP_COMMAND;      // 不支持的指令
            case ERR_CAM_DATA_FAIL:         return WFS_ERR_HARDWARE_ERROR;      // 数据错误
            case ERR_CAM_USER_ERR:          return WFS_ERR_USER_ERROR;          // 用户使用错误
            case ERR_CAM_OTHER:             return WFS_ERR_HARDWARE_ERROR;      // 其它错误(知道错误原因,不细分类别)
            case ERR_CAM_UNKNOWN:           return WFS_ERR_HARDWARE_ERROR;      // 未知错误(不知原因的错误)
            // 其他错误
            case ERR_CAM_NOTOPEN:           return WFS_ERR_HARDWARE_ERROR;      // 设备未打开
            case ERR_CAM_OPENFAIL:          return WFS_ERR_HARDWARE_ERROR;      // 设备打开失败
            case ERR_CAM_NODEVICE:          return WFS_ERR_HARDWARE_ERROR;      // 设备不存在
            case ERR_CAM_OFFLINE:           return WFS_ERR_HARDWARE_ERROR;      // 设备断线
            case ERR_CAM_PROCESS:           return WFS_ERR_SOFTWARE_ERROR;      // 进程/线程错误
            case ERR_CAM_SAVE_IMAGE:        return WFS_ERR_SOFTWARE_ERROR;      // 图像无法保存
            case ERR_CAM_LOAD_IMAGE:        return WFS_ERR_SOFTWARE_ERROR;      // 图像无法加载
            case ERR_CAM_SHAREDMEM:         return WFS_ERR_SOFTWARE_ERROR;      // 共享内存错误
            case ERR_CAM_LIVEDETECT:        return WFS_ERR_HARDWARE_ERROR;      // 活体检测失败
            default:                        return WFS_ERR_HARDWARE_ERROR;
        }
    }

    CHAR* ConvertDevErrCodeToStr(INT nRet)
    {
        #define CONV_CAM_CODE_STR(RET, STR) \
            sprintf(m_szErrStr, "%d|%s", RET, STR); \
            return m_szErrStr;

        memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

        switch(nRet)
        {
            // 成功
            case CAM_SUCCESS:               CONV_CAM_CODE_STR(nRet, "操作成功")
            // 警告
            case ERR_CAM_USER_CANCEL:       CONV_CAM_CODE_STR(nRet, "用户取消")
            // 通用错误
            case ERR_CAM_LIBRARY:           CONV_CAM_CODE_STR(nRet, "动态库加载失败")
            case ERR_CAM_PARAM_ERR:         CONV_CAM_CODE_STR(nRet, "参数错误")
            case ERR_CAM_UNSUP_CMD:         CONV_CAM_CODE_STR(nRet, "不支持的指令")
            case ERR_CAM_DATA_FAIL:         CONV_CAM_CODE_STR(nRet, "数据错误")
            case ERR_CAM_TIMEOUT:           CONV_CAM_CODE_STR(nRet, "超时")
            case ERR_CAM_USER_ERR:          CONV_CAM_CODE_STR(nRet, "用户使用错误")
            case ERR_CAM_OTHER:             CONV_CAM_CODE_STR(nRet, "其它错误(知道错误原因,不细分类别)")
            case ERR_CAM_UNKNOWN:           CONV_CAM_CODE_STR(nRet, "未知错误(不知原因的错误)")
            // 其他错误
            case ERR_CAM_NOTOPEN:           CONV_CAM_CODE_STR(nRet, "设备未打开")
            case ERR_CAM_OPENFAIL:          CONV_CAM_CODE_STR(nRet, "设备打开失败")
            case ERR_CAM_NODEVICE:          CONV_CAM_CODE_STR(nRet, "设备不存在")
            case ERR_CAM_OFFLINE:           CONV_CAM_CODE_STR(nRet, "设备断线")
            case ERR_CAM_PROCESS:           CONV_CAM_CODE_STR(nRet, "进程/线程错误")
            case ERR_CAM_SAVE_IMAGE:        CONV_CAM_CODE_STR(nRet, "图像无法保存")
            case ERR_CAM_LOAD_IMAGE:        CONV_CAM_CODE_STR(nRet, "图像无法加载")
            case ERR_CAM_SHAREDMEM:         CONV_CAM_CODE_STR(nRet, "共享内存错误")
          case ERR_CAM_LIVEDETECT:          CONV_CAM_CODE_STR(nRet, "活体检测失败")
            default:                        CONV_CAM_CODE_STR(nRet, "未定义错误");
        }

        return m_szErrStr;
    }

    // 事件ID转换为WFS格式
    WORD ConvertEventID2WFS(WORD wEventID)
    {
        switch (wEventID)
        {
            case EN_EVENTID_HWERR       /* 硬件错误 */      : return WFS_SYSE_HARDWARE_ERROR;
            case EN_EVENTID_SOFTERR     /* 软件错误 */      : return WFS_SYSE_SOFTWARE_ERROR;
            case EN_EVENTID_USERERR     /* 用户错误 */      : return WFS_SYSE_USER_ERROR;
            default: return WFS_SYSE_HARDWARE_ERROR;
        }
    }

    // 事件Action转换为WFS格式
    WORD ConvertEventAct2WFS(WORD wEventAct)
    {
        switch (wEventAct)
        {
            case EN_EVENTACT_NOACTION   /* 可自动修复 */     : return WFS_ERR_ACT_NOACTION;
            case EN_EVENTACT_RESET      /* 重新设置 */      : return WFS_ERR_ACT_RESET;
            case EN_EVENTACT_SWERROR    /* 软件错误 */      : return WFS_ERR_ACT_SWERROR;
            case EN_EVENTACT_CONFIG     /* 配置错误 */      : return WFS_ERR_ACT_CONFIG;
            default: return WFS_ERR_ACT_RESET;
        }
    }

    // 活检状态转换为WFS格式
    WORD ConvertLiveErr2WFS(WORD wError)
    {
        switch(wError)
        {
            case LIVE_ERR_OK:                   // 0:活检图像正常
                return WFS_CAM_LIVE_ERR_OK;
            case LIVE_ERR_VIS_NOFACE:           // 1:可见光未检测到人脸
                return WFS_CAM_LIVE_ERR_VIS_NOFACE;
            case LIVE_ERR_FACE_SHELTER:         // 2:人脸有遮挡(五官有遮挡,戴镜帽等)
                return WFS_CAM_LIVE_ERR_FACE_SHELTER;
            case LIVE_ERR_FACE_ANGLE_FAIL:      // 3:人脸角度不满足要求(低头/抬头/侧脸等)
                return WFS_CAM_LIVE_ERR_FACE_ANGLE_FAIL;
            case LIVE_ERR_FACE_EYECLOSE:        // 4:检测到闭眼
                return WFS_CAM_LIVE_ERR_FACE_EYECLOSE;
            case LIVE_ERR_FACE_MOUTHOPEN:       // 5:检测到张嘴
                return WFS_CAM_LIVE_ERR_FACE_MOUTHOPEN;
            case LIVE_ERR_FACE_SHAKE:           // 6:检测到人脸晃动/模糊
                return WFS_CAM_LIVE_ERR_FACE_SHAKE;
            case LIVE_ERR_FACE_MULTIPLE:        // 7:检测到多张人脸
                return WFS_CAM_LIVE_ERR_FACE_MULTIPLE;
            case LIVE_ERR_IS_UNLIVE:            // 8:检测到非活体
                return WFS_CAM_LIVE_ERR_IS_UNLIVE;
            case LIVE_ERR_FACE_TOOFAR:          // 9:人脸离摄像头太远
                return WFS_CAM_LIVE_ERR_FACE_TOOFAR;
            case LIVE_ERR_FACE_TOONEAR:         // 10:人脸离摄像头太近
                return WFS_CAM_LIVE_ERR_FACE_TOONEAR;
            case LIVE_ERR_NIS_NOFACE:           // 11:红外光未检测到人脸
                return WFS_CAM_LIVE_ERR_NIS_NOFACE;
            case LIVE_ERR_UNKNOWN:              // 99:其他/未知错误
                return WFS_CAM_LIVE_ERR_UNKNOWN;
            default:
                return WFS_CAM_LIVE_ERR_UNKNOWN;
        }
    }
};

#endif
//////////////////////////////////////////////////////////////////////////
