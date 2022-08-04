/***************************************************************************
* 文件名称: DevImpl_JDY5001A0809.h
* 文件描述: 封装摄像模块底层指令,提供控制接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
***************************************************************************/
#pragma once

#ifndef DEVIMPL_JDY5001A0809_H
#define DEVIMPL_JDY5001A0809_H

#include <opencv2/opencv.hpp>

#include <libudev.h>

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "QtTypeInclude.h"
#include "../../XFS_CAM/def.h"
#include "../DevCAM_DEF/DevImpl_DEF.h"
#include "device_object.h"

using namespace std;

/***************************************************************************
// 返回值/错误码　宏定义 (0~-300通用定义, 见DevImpl_DEF.h)
***************************************************************************/
#define IMP_ERR_VIDPID_NOTFOUND         -301        // VID/PID未找到
#define IMP_ERR_VIDEOIDX_NOTFOUND       -302        // 未找到摄像设备索引号
#define IMP_ERR_GET_IMGDATA_FAIL        -303        // 取帧图像数据失败
#define IMP_ERR_GET_IMGDATA_ISNULL      -304        // 取帧图像数据为空
#define IMP_ERR_GET_IMGDATA_BUFFER      -305        // 取帧图像数据空间异常
#define IMP_ERR_GET_IMGDATA_CHANGE      -306        // 帧图像数据转换失败
#define IMP_ERR_SAVE_IMAGE_FILE         -307        // 帧图像数据保存到文件失败


/***************************************************************************
// 无分类　宏定义
***************************************************************************/
#define LOG_NAME                    "DevImpl_JDY5001A0809.log"   // 缺省日志名

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
        case DEV_NOTFOUND: return IMP_ERR_NODEVICE; \
        case DEV_NOTOPEN:  return IMP_ERR_OPENFAIL; \
        case DEV_OFFLINE:  return IMP_ERR_OFFLINE; \
        case DEV_UNKNOWN:  return IMP_ERR_UNKNOWN; \
    }

/***************************************************************************
// 封装类: 命令编辑、发送接收等处理。
***************************************************************************/
class CDevImpl_JDY5001A0809 : public CLogManage, public CDevImpl_DEF
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
    INT     ConvertCode_DPErr2Impl(INT nErrCode);                   // DeviceVideo错误码转换为Impl错误码
    LPSTR   ConvertCode_Impl2Str(INT nErrCode);                     // Impl错误码转换解释字符串


private:
    void    Init();                                                 // 参数初始化

private:
    CSimpleMutex    m_cMutex;
    CDeviceVideo    m_clDevVideo;
    CHAR            m_szDevType[64];                                // 设备类型
    VideoCapture    m_cvVideoCapt;                                  // 设备句柄
    BOOL            m_bDevOpenOk;                                   // 设备是否Open
    CHAR            m_szDevVidPid[2][16];                           // 保存设备VIDPID
    INT             m_nRetErrOLD[8];                                // 处理错误值保存(0:USB动态库/1:设备连接/
                                                                    //  2:状态/3/4)
    STIMGDATA       m_stImageData;                                  // 保存图像帧数据
    Mat             m_cvMatImg;
    CHAR            m_szErrStr[1024];                               // IMPL错误解释

};


#endif // DEVIMPL_JDY5001A0809

// -------------------------------------- END --------------------------------------
