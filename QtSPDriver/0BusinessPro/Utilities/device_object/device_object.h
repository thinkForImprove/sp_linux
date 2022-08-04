#ifndef DEVICE_OBJECT_H
#define DEVICE_OBJECT_H

#include <libudev.h>
#include <opencv2/opencv.hpp>
#include <string>
#include "__common_def.h"
#include "device_object_global.h"
#include "QtTypeDef.h"
#include "QtTypeInclude.h"

using namespace std;
using namespace cv;

/***************************************************************************
// 返回值/错误码　宏定义
***************************************************************************/
#define DP_RET_SUCC             0       // 成功/存在/已打开/正常
#define DP_RET_FAIL             -1      // 失败/错误
#define DP_RET_INST_ERR         -2      // 有关实例化失败
#define DP_RET_INPUT_INV        -3      // 无效的入参
#define DP_RET_NOTHAVE          -4      // 不存在
#define DP_RET_OPENFAIL         -5      // Open错误
#define DP_RET_NOTOPEN          -6      // 没有Open
#define DP_RET_OFFLINE          -7      // 已Open但已断线
#define DP_RET_GETIMG_FAIL      -8      // 取图像数据失败
#define DP_RET_IMGDATA_INV      -9      // 图像数据无效
#define DP_RET_MEMAPPLY_FAIL    -10     // 内存申请失败
#define DP_RET_IMG2FILE         -11     // 图像数据保存到文件失败

/***************************************************************************
// 摄像设备参数设置 定义
***************************************************************************/
enum EN_VIDEOMODE
{
    VM_WIDTH        = 0,    // 宽度
    VM_HEIGHT       = 1,    // 高度
    VM_FPS          = 2,    // 帧率(帧/秒)
    VM_BRIGHT       = 3,    // 亮度(-255 ~ 255)(RGB相关)
    VM_CONTRAST     = 4,    // 对比度
    VM_SATURATION   = 5,    // 饱和度
    VM_HUE          = 6,    // 色调
    VM_EXPOSURE     = 7,    // 曝光
};

/***************************************************************************
// 图像帧数据结构体 定义
***************************************************************************/
typedef struct ST_IMAGE_DATA
{
    INT nWidth;                 // 图像宽
    INT nHeight;                // 图像高
    INT nFormat;                // 图像通道数
    UCHAR *ucImgData;           // 图像数据Buffer
    ULONG ulImagDataLen;        // 图像数据BufferSize
    INT nOtherParam[24];        // 其他参数

    ST_IMAGE_DATA()
    {
        memset(this, 0x00, sizeof(ST_IMAGE_DATA));
    }

    void Clear()
    {
        if (ucImgData != nullptr)
        {
            free(ucImgData);
            ucImgData = nullptr;
        }
        memset(this, 0x00, sizeof(ST_IMAGE_DATA));
    }
} STIMAGEDATA, *LPSTIMAGEDATA;


/***************************************************************************
// 类名: CDevicePort
// 功能: 用于设备接口相关索引查找获取
// 备注:
***************************************************************************/
class DEVICE_OBJECT_EXPORT CDevicePort
{
DEFINE_STATIC_VERSION_FUNCTIONS("CDevicePort", "0.0.0.0", TYPE_DYNAMIC)

public:
    static INT SearchVideoIdxFromVidPid(LPCSTR lpcVid, LPCSTR lpcPid);      // 根据Vid+Pid取得摄像设备索引序号
    static INT SearchDeviceNameIsHave(LPCSTR lpcDevName);                   // 查找设备名是否存在
    static INT SearchDeviceVidPidIsHave(LPCSTR lpcVid, LPCSTR lpcPid);      // 查找设备VidPid是否存在
    static INT SearchVideoXIsHave(WORD wID);                                // 查找/dev/videoX是否存在
    static INT SearchVideoIdxIsHave(WORD wIdx);                             // 查找摄像设备索引序号是否存在
    static INT GetComDevName(LPCSTR lpMode, LPSTR lpDevName, INT nDevNameLen);// 取COM串设备名

private:
    static INT SearchDeviceIsHave(WORD wMode, LPCSTR lpcDevName, LPCSTR lpcVid, LPCSTR lpcPid);// 查找设备是否存在
    static DWORD str_to_toupper(LPCSTR lpcSource, DWORD dwSourceSize, LPSTR lpDest, DWORD dwDestSize);
};

/***************************************************************************
// 类名: CDeviceVideo
// 功能: 用于摄像设备的连接、状态/图像帧获取
// 备注: 当前仅使用单目设备
***************************************************************************/
class DEVICE_OBJECT_EXPORT CDeviceVideo
{

    DEFINE_STATIC_VERSION_FUNCTIONS("CDeviceVideo", "0.0.0.0", TYPE_DYNAMIC)
public:
    CDeviceVideo();
    virtual ~CDeviceVideo();

public:
    INT     Open(INT nVideoX);                                  // 打开设备(VideoX)
    INT     Open(LPSTR lpVid, LPSTR lpPid);                     // 打开设备(VID+PID)
    INT     Close();                                            // 关闭设备
    BOOL    IsOpen();                                           // 设备是否Open
    INT     Reset();                                            // 复位设备
    INT     GetStatus();                                        // 取设备状态
    INT     GetVideoWH(INT &nWidth, INT &nHeight);              // 取摄像宽高
    INT     SetVideoWH(INT nWidth, INT nHeight);                // 设置摄像宽高
    INT     SetVideoMode(EN_VIDEOMODE enVM, DOUBLE duData = 0.0);// 设置摄像模式
    DOUBLE  GetVideoMode(EN_VIDEOMODE enVM);                    // 获取摄像模式
    INT     GetVideoImage(LPSTIMAGEDATA lpImgData,
                          INT nWidth = 0, INT nHeight = 0,
                          WORD wFlip = 0);                          // 取图像帧数据
    INT     SaveImageFile(LPSTR lpFileName);                        // 保存图像文件
    INT     GetOpenVideoX();                                        // 取Open成功的Video序号
    LPSTR   GetErrorStr();                                          // 取错误码中文解析(最近一次错误)
    LPSTR   GetErrorStr(INT nErrNo);                                // 取错误码中文解析(指定错误值)

private:
    CSimpleMutex    m_cMutex;
    VideoCapture    m_cvVideoCapt;                                  // 设备句柄
    BOOL            m_bDevOpenOk;                                   // 设备是否Open
    CHAR            m_szDevVidPid[2][16];                           // 保存设备VIDPID
    STIMAGEDATA     m_stImageData;                                  // 保存图像帧数据
    Mat             m_cvMatImg;
    CHAR            m_szErrorStr[1024];
    INT             m_nErrorOLD;
    INT             m_nVideoX;                                      // Open成功的Video序

};


#endif // DEVICE_OBJECT_H
