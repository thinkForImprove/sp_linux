#pragma once
/***************************************************************
* 文件名称：IDevCAM.h
* 文件描述：声明Camera底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include <string.h>

#if defined(IDEVCAM_LIBRARY)
#define DEVCAM_EXPORT Q_DECL_EXPORT
#else
#define DEVCAM_EXPORT Q_DECL_IMPORT
#endif

#define IDEV_TYPE_CW1 "CW1"   // 云从双目摄像



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

// SetData/GetData 数据类型
#define DATATYPE_INIT   0   // 初始化数据

//////////////////////////////////////////////////////////////////////////
#define CAM_CAMERAS_SIZE     (8)
#define MAX_SIZE             (400)
#define MAX_CAMDATA          (4096)
//////////////////////////////////////////////////////////////////////////
enum DEVICE_STATUS
{
    DEVICE_ONLINE                   = 0,
    DEVICE_OFFLINE                  = 1,
    DEVICE_HWERROR                  = 2
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
        for (auto &it : fwMedia)
        {
            it = MEDIA_NOTSUPP;// 初始不支持
        }
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
    // 取状态
    virtual long GetStatus(DEVCAMSTATUS &stStatus) = 0;
    // 打开窗口(窗口句柄，动作指示:1创建窗口/0销毁窗口, X/Y坐标,窗口宽高)
    virtual long Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wHidth, WORD wHeight) = 0;
    // 拍照 传下来的字符lpData按：”FileName=C:\TEMP\TEST;Text=HELLO;TextPositionX=160;TextPositionY=120;CaptureType=0”格式
    virtual long TakePicture(WORD wCamera, LPCSTR lpData) = 0;
    // 拍照(保存文件名,水印字串，图片类型是否连续检测)(1BASE64/2JPG/4BMP;T连续检测)
    virtual long TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, WORD wTimeOut) = 0;

    // 设置数据
    virtual long SetData(void *vData, WORD wDataType = 0) = 0;

    // 获取数据
    virtual long GetData(void *vData, WORD wDataType = 0) = 0;

    // 版本号(1DevCam版本/2固件版本/3其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType) = 0;
};

extern "C" DEVCAM_EXPORT long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&p);
//extern "C" DEVCAM_EXPORT long CreateIDevCAMFrame(LPCSTR lpDevType, IDevCAM *&p, void* vHWnd);
//////////////////////////////////////////////////////////////////////////
