/***************************************************************
* 文件名称: DevImpl_JDY5001A0809.h
* 文件描述: 封装摄像模块底层指令,提供控制接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
****************************************************************/
#pragma once

#ifndef DEVIMPL_JDY5001A0809_H
#define DEVIMPL_JDY5001A0809_H

#include <opencv2/opencv.hpp>

#include <libudev.h>

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "../../XFS_HCAM/def.h"
#include "device_port.h"

using namespace std;
using namespace cv;

/*************************************************************
// 返回值/错误码　宏定义
*************************************************************/
#define IMP_SUCCESS                 0               // 成功
#define IMP_ERR_LOAD_LIB            1               // 动态库加载错误
#define IMP_ERR_PARAM_INVALID       2               // 参数无效
#define IMP_ERR_UNKNOWN             99              // 未知错误

#define IMP_ERR_DEVICE_NOTFOUND     -1              // 设备未找到
#define IMP_ERR_VIDPID_NOTFOUND     -2              // VID/PID未找到
#define IMP_ERR_VIDEOIDX_NOTFOUND   -3              // 未找到摄像设备索引号
#define IMP_ERR_DEVICE_OPENFAIL     -4              // 设备打开失败/未打开
#define IMP_ERR_DEVICE_OFFLINE      -5              // 设备断线
#define IMP_ERR_GET_IMGDATA_FAIL    -6              // 取帧图像数据失败
#define IMP_ERR_GET_IMGDATA_ISNULL  -7              // 取帧图像数据为空
#define IMP_ERR_GET_IMGDATA_BUFFER  -8              // 取帧图像数据空间异常
#define IMP_ERR_GET_IMGDATA_CHANGE  -9              // 帧图像数据转换失败
#define IMP_ERR_SAVE_IMAGE_FILE     -10             // 帧图像数据保存到文件失败
#define IMP_ERR_SET_VMODE           -11             // 摄像参数设置错误
#define IMP_ERR_SET_RESO            -12             // 分辨率设置失败
#define IMP_ERR_SHAREDMEM_RW        -13             // 共享内存读写失败


/*************************************************************
// 无分类　宏定义
*************************************************************/
#define LOG_NAME                        "DevImpl_JDY5001A0809.log"   // 缺省日志名

enum EN_DEVSTAT
{
    DEV_OK          = 0,    // 设备正常
    DEV_NOTFOUND    = 1,    // 设备未找到
    DEV_NOTOPEN     = 2,    // 设备未打开
    DEV_OFFLINE     = 3,    // 设备已打开但断线
    DEV_UNKNOWN     = 4,    // 设备状态未知
};

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
} STIMGDATA, *LPSTIMGDATA;

// 图像镜像模式转换
enum EN_CLIPMODE
{
    EN_CLIP_NO      = 0,        // 不转换
    EN_CLIP_LR      = 1,        // 左右转换
    EN_CLIP_UD      = 2,        // 上下转换
    EN_CLIP_UDLR    = 3,        // 上下左右转换
};

#define CHECK_DEVICE_STAT(S) \
    switch(S) \
    { \
        case DEV_OK:       break; \
        case DEV_NOTFOUND: return IMP_ERR_DEVICE_NOTFOUND; \
        case DEV_NOTOPEN:  return IMP_ERR_DEVICE_OPENFAIL; \
        case DEV_OFFLINE:  return IMP_ERR_DEVICE_OFFLINE; \
        case DEV_UNKNOWN:  return IMP_ERR_UNKNOWN; \
    }

/*************************************************************
// 封装类: 命令编辑、发送接收等处理。
*************************************************************/
class CDevImpl_JDY5001A0809 : public CLogManage
{
public:
    CDevImpl_JDY5001A0809();
    CDevImpl_JDY5001A0809(LPSTR lpLog);
    CDevImpl_JDY5001A0809(LPSTR lpLog, LPCSTR lpDevType);
    virtual ~CDevImpl_JDY5001A0809();

public:
    //INT     OpenDevice(LPSTR lpMode);                               // 打开设备(设备路径)
    INT     OpenDevice(LPSTR lpVid, LPSTR lpPid);                   // 打开设备(VID+PID)
    INT     CloseDevice();                                          // 关闭设备
    BOOL    IsDeviceOpen();                                         // 设备是否Open
    INT     ResetDevice();                                          // 复位设备
    INT     GetDeviceStatus();                                      // 取设备状态
    INT     GetVideoCaptWH(INT &nWidth, INT &nHeight);              // 取摄像宽高
    INT     SetVideoCaptWH(INT nWidth, INT nHeight);                // 设置摄像宽高
    INT     SetVideoCaptMode(STVIDEOPAMAR stVideoPar);              // 设置摄像模式
    INT     GetVideoCaptMode(STVIDEOPAMAR &stVideoPar);             // 获取摄像模式
    INT     SetVideoCaptMode(EN_VIDEOMODE enVM, DOUBLE duData = 0.0);// 设置摄像模式
    DOUBLE  GetVideoCaptMode(EN_VIDEOMODE enVM);                    // 获取摄像模式
    INT     GetVideoImage(LPSTIMGDATA lpImgData,
                          INT nWidth = 0, INT nHeight = 0,
                          WORD wFlip = 0);                          // 取图像帧数据
    INT     SaveImageFile(LPSTR lpFileName);                        // 保存图像文件
    LPSTR   ConvertCode_Impl2Str(INT nErrCode);                     // Impl错误码转换解释字符串


private:
    void    Init();                                                 // 参数初始化

private:
    CHAR            m_szDevType[64];                                // 设备类型
    VideoCapture    m_cvVideoCapt;                                  // 设备句柄
    BOOL            m_bDevOpenOk;                                   // 设备是否Open
    CHAR            m_szDevVidPid[2][16];                           // 保存设备VIDPID
    INT             m_nRetErrOLD[8];                                // 处理错误值保存(0:USB动态库/1:设备连接/
                                                                    //  2:设备初始化/3/4)
    STIMGDATA       m_stImageData;                                  // 保存图像帧数据
    Mat             m_cvMatImg;
    CHAR            m_szErrStr[1024];                               // IMPL错误解释

};


#endif // DEVIMPL_JDY5001A0809
