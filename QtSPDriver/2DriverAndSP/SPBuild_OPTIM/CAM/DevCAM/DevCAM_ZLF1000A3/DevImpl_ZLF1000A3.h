/***************************************************************************
* 文件名称: DevImpl_ZLF1000A3.h
* 文件描述: 封装摄像模块底层指令,提供控制接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
***************************************************************************/

#ifndef DEVIMPL_ZLF1000A3_H
#define DEVIMPL_ZLF1000A3_H

#pragma once

#include <string>
#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "../DevCAM_DEF/DevImpl_DEF.h"

/***************************************************************************
// 返回值/错误码　宏定义 (0~-300通用定义, 见DevImpl_DEF.h)
***************************************************************************/
// 设备返回错误码
#define IMP_ERR_DEV_NOTOPEN              1              // 设备未打开
#define IMP_ERR_DEV_GETVDATA             2              // 获取视频/图像数据失败
#define IMP_ERR_DEV_PARAM                3              // SDK接口参数错误


/***************************************************************************
// 无分类　宏定义
***************************************************************************/
#define LOG_NAME                            "DevImpl_ZLF1000A3.log" // 缺省日志名
#define DLL_DEVLIB_NAME                     "libVideoPro.so"        // 缺省动态库名

// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

// dlxxx加载动态库接口
#define LOAD_LIBINFO_FUNC_DL(HANDLE, LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)dlsym(HANDLE, FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        Log(ThisModule, __LINE__, "dlxxx方式加载动态库接口<%s> Fail. ", FUNC2); \
        return FALSE;   \
    }

// 摄像类型定义
#define ZL_APPLYMODE_DOC        0       // 文档摄像头
#define ZL_APPLYMODE_PER        1       // 人脸摄像头

/***************************************************************************
// 结构体/枚举 宏定义
***************************************************************************/
// GetDeviceStatus()接口返回状态值
enum EVideoDeviceState
{
    DEVICE_OK = 0,              // 启动且正在运行
    DEVICE_NOT_FOUND = 200,     // 未找到设备
    DEVICE_OPEN_ERROR,          // 打开设备时出错
    DEVICE_CONNECTED,           // 设备已连接未打开
    DEVICE_OPENED,              // 设备已打开
    DEVICE_ERROR,               // 其他错误
};

// 图像选装角度
enum EN_IMG_Rotate
{
    IMG_ROT_0   = 0,            // 旋转0度(不旋转)
    IMG_ROT_90  = 90,           // 旋转90度
    IMG_ROT_180 = 180,          // 旋转180度
    IMG_ROT_270 = 270,          // 旋转270度
};

// 图像类型
enum EN_IMG_TYPE
{
    IMG_TYPE_COLOR  = 0,        // 彩色图
    IMG_TYPE_GRAY   = 1,        // 灰度图
    IMG_TYPE_BINARY = 2,        // 二值图
};

/***************************************************************************
// 动态库输出函数接口　定义
***************************************************************************/
/* 1. 打开摄像头
 * SDK接口名: OpenDevice
 * 参数: index: 摄像头索引(0文档头, 1人像头)
 * 返回值: 0: 成功, 1: 设备未连接, 2: 设备启动失败
*/
typedef long (*pZLOpenDevice)(int index);

/* 2. 获取图像
 * SDK接口名: GrabImage
 * 参数: index: 摄像头索引(0文档头, 1人像头)
 *      iDataBuffer: 保存视频图像数据的内存地址(数据格式为
 *                   依次存储图像中第0行至最后一行中每个像素的BGRA值)
 *      iLen:  iDataBuffer的长度，iLen必须大于等于图像的长x宽
 *      width:  返回图像的宽度
 *      height: 返回图像的高度
 *      iRotate: 旋转角度(0/90/180/270)
 *      bAutoCut: 是否进行图像去黑边处理
 *      iType: 图像类型(0彩色, 1灰度图像, 2二值图像)
 * 返回值: 0: 成功
 *        1: 设备未开启, 先调用函数OpenDevice打开摄像头
 *        2: 获取视频数据失败, 检查设备是否连接正常
 *        3: 获取视频数据失败, iDataBuffer太小或者其他参数有问题
*/
typedef long (*pZLGrabImage)(int index, int *iDataBuffer, int iLen,
                           int *width, int *height, int iRotate,
                           bool bAutoCut, int iType);

/* 3. 获取图像并保存
 * 调用OpenDevice打开摄像头设备之后，要等待一会才能获取到图像数据
 * SDK接口名: SaveImage
 * 参数: index: 摄像头索引(0文档头, 1人像头)
 *      szFilePath: 保存图像的文件路径
 *      iRotate: 旋转角度(0/90/180/270)
 *      bAutoCut: 是否进行图像去黑边处理
 *      iType: 图像类型(0彩色, 1灰度图像, 2二值图像)
 * 返回值: 0: 成功
 *        1: 设备未开启, 先调用函数OpenDevice打开摄像头
 *        2: 获取视频数据失败, 检查设备是否连接正常
 *        3: 保存图像识别，检测图像路径是否存在
*/
typedef long (*pZLSaveImage)(int index, const char *szFilePath, int iRotate,
                           bool bAutoCut, int iType);

/* 4. 获取视频图像数据
 * 调用OpenDevice打开摄像头设备之后，要等待一会才能获取到图像数据
 * SDK接口名: GrabVideoData
 * 参数: index: 摄像头索引(0文档头, 1人像头)
 *      iDataBuffer: 保存视频图像数据的内存地址
 *                   数据格式为依次存储图像中第0行至最后一行中每个像素的BGRA值
 *      iLen:  iDataBuffer的长度，iLen必须大于等于图像的长x宽
 *      width:  返回图像的宽度
 *      height: 返回图像的高度
 *      timestamp: 当前视频帧的时间戳，单位毫秒
 *      drawCutRect: 是否绘制切边区域
 * 返回值: 0: 成功
 *        1: 设备未开启, 先调用函数OpenDevice打开摄像头
 *        2: 获取视频数据失败, 检查设备是否连接正常
 *        3: 保存图像识别，检测图像路径是否存在
 * 备注: 调用OpenDevice打开摄像头设备之后，要等待一会才能获取到图像数据，在此期间调用
 *      GrabVideoData函数将return 0，但是通过入参返回的其他参数值为0,当输入的iDataBuffer
 *      为NULL，或者iDataBuffer的长度小于视频图像数据的长度，将不拷贝数据，同时
 *      width, height和timestamp设置成相应的值
*/
typedef long (*pZLGrabVideoData)(int index, int *iDataBuffer, int iLen, int *width, int *height,
                               long *timestamp, bool drawCutRect);

/* 5. 关闭摄像头
 * SDK接口名: CloseDevice
 * 参数: index: 摄像头索引(0文档头, 1人像头)
 * 返回值: 0: 成功
 *        1: 设备未连接
*/
typedef long (*pZLCloseDevice)(int index);

/* 6. 获取分辨率
 * SDK接口名: GetResolution
 * 参数: index: 摄像头索引(0文档头, 1人像头)
 *      resolutions: 输入的数组必修足够大, 顺序保存每个分辨率对应的宽高
 *      number: 作为输入代表resolutions的大小, 作为输出为分辨率的个数x2
 * 返回值: 分辨率个数
*/
typedef long (*pZLGetResolution)(int index, unsigned int resolution[30][2]);

/* 7. 设置分辨率
 * SDK接口名: SetResolution
 * 参数: index: 摄像头索引(0文档头, 1人像头)
 *      width：宽度
 *      height：高度
 * 返回值: 0: 成功
          非0: 失败
*/
typedef long (*pZLSetResolution)(int index, int width, int height);

/* 8. 获取设备状态
 * SDK接口名: GetDeviceStatus(int iDeviceIdx)
 * 参数: index: 摄像头索引(0文档头, 1人像头)
 * 返回值: 设备状态(参考 enum EVideoDeviceState)
*/
typedef long (*pZLGetDeviceStatus)(int iDeviceIdx);

/***************************************************************************
// 封装类: 命令编辑、发送接收等处理。
***************************************************************************/
class CDevImpl_ZLF1000A3 : public CLogManage, public CDevImpl_DEF
{
public:
    CDevImpl_ZLF1000A3();
    CDevImpl_ZLF1000A3(LPSTR lpLog);
    CDevImpl_ZLF1000A3(LPSTR lpLog, LPCSTR lpDevType);
    virtual ~CDevImpl_ZLF1000A3();

public: // 动态库接口封装
    INT OpenDevice();                                           // 1. 打开设备
    INT CloseDevice();                                          // 2. 关闭设备
    INT GetDevStatus();                                         // 3. 获取设备状态
    INT GetImage(LPINT lpnDataBuf, INT nDataBufLen,
                 LPINT lpnImgWidth, LPINT lpnImgHeight,
                 INT nImgType = IMG_TYPE_COLOR,
                 INT nRotate = IMG_ROT_0, BOOL bAutoCut = TRUE);// 4. 获取图像
    INT SaveImage(LPCSTR lpcFilePath, INT nImgType = IMG_TYPE_COLOR,
                  INT nRotate = IMG_ROT_0, BOOL bAutoCut = TRUE);// 5. 获取图像并保存
    INT GetVideoData(LPINT lpnDataBuf, INT nDataBufLen,
                     LPINT lpnImgWidth, LPINT lpnImgHeight,
                     LPLONG lplTimesTamp, BOOL dDrawCutRect = TRUE);// 6. 获取视频图像数据
    INT GetResolut(UINT unResoBuf[30][2]);                      // 7. 获取分辨率
    INT SetResolut(INT nWidth, INT nHeight);                    // 8. 设置分辨率

    INT GetVideoImage(LPSTIMGDATA lpImgData,
                      INT nWidth = 0, INT nHeight = 0,
                      WORD wFlip = 0);                          // 取图像帧数据

public:
    BOOL IsDeviceOpen();
    LPSTR ConvertCode_Impl2Str(INT nErrCode);                       // Impl错误码解析

public:    // DevXXX.SetData相关
    INT SetReConFlag(BOOL bFlag);                                   // 设置断线重连标记
    INT SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)
    INT SetApplyMode(INT nMode);                                    // 设置设备使用模式
    INT SetDrawCutRect(BOOL bIsDCRect);                             // 设置图像帧是否绘制切边区域

private:
    void Init();

private:
    CSimpleMutex    m_Mutex;                                            // 执行互斥
    CHAR            m_szDevType[64];                                    // 设备类型
    BOOL            m_bDevOpenOk;                                       // 设备Open标记
    BOOL            m_bReCon;                                           // 是否断线重连状态
    INT             m_nRetErrOLD[12];                                   // 处理错误值保存(0:库加载/1:设备连接/
                                                                        //              2:设备状态/3:Open标记检查/
                                                                        //              4介质吸入)
    BOOL            m_bDrawCutRect;                                     // 是否绘制切边区域
    INT             m_nRetImgWidthOLD;                                  // 保存上一次图像宽
    INT             m_nRetImgHeightOLD;                                 // 保存上一次图像高
    UCHAR           *m_ucGetVideoDataBuff;                              // 图像帧数据保存空间
    STIMGDATA       m_stImageData;                                      // 保存图像帧数据
    INT             m_nOpenCamType;                                     // 摄像头类型索引
    CHAR            m_szErrStr[1024];                                   // IMPL错误码解析

private: // 接口加载(QLibrary方式)
    BOOL    bLoadLibrary();                                             // 加载动态库(QLibrary方式)
    void    vUnLoadLibrary();                                           // 释放动态库(QLibrary方式)
    BOOL    bLoadLibIntf();                                             // 加载动态库接口(QLibrary方式)
    void    vInitLibFunc();                                             // 动态库接口初始化

private: // 接口加载(dlxxx方式)
    BOOL bDlLoadLibrary();                                              // 加载动态库(dlxxx方式)
    void vDlUnLoadLibrary();                                            // 释放动态库(dlxxx方式)
    BOOL bDlLoadLibIntf();                                              // 加载动态库接口(dlxxx方式)

private: // 接口加载
    QLibrary    m_LoadLibrary;                                          // QLibrary方式库连接句柄
    void*       m_vLibInst;                                             // dlxxx方式库连接句柄
    INT         m_nDlOpenMode;                                          // dlOpen命令模式
    char        m_szLoadDllPath[MAX_PATH];                              // 动态库路径
    BOOL        m_bLoadIntfFail;                                        // 动态库及接口加载是否有错误

private: // 动态库接口定义
    pZLOpenDevice               ZLOpenDevice;                           // 1. 打开摄像头
    pZLGrabImage                ZLGrabImage;                            // 2. 获取图像
    pZLSaveImage                ZLSaveImage;                            // 3. 获取图像并保存
    pZLGrabVideoData            ZLGrabVideoData;                        // 4. 获取视频图像数据
    pZLCloseDevice              ZLCloseDevice;                          // 5. 关闭摄像头
    pZLGetResolution            ZLGetResolution;                        // 6. 获取分辨率
    pZLSetResolution            ZLSetResolution;                        // 7. 设置分辨率
    pZLGetDeviceStatus          ZLGetDeviceStatus;                      // 8. 获取设备状态
};

#endif // DEVIMPL_ZLF1000A3_H

// -------------------------------------- END --------------------------------------
