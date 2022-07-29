/***************************************************************************
* 文件名称: DevImpl_TCF261.h
* 文件描述: 封装摄像模块底层指令,提供控制接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
***************************************************************************/

#pragma once
//#define LINUX

//using namespace std;

#include <string>
#include "QtTypeDef.h"
#include "QtTypeInclude.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "TCF261.h"
#include "../DevCAM_DEF/DevImpl_DEF.h"

/***************************************************************************
// 返回值/错误码　宏定义 (0~-300通用定义, 见DevImpl_DEF.h)
***************************************************************************/
// 设备返回错误码
#define IMP_ERR_DEV_SUCCESS             FR_RET_SUCC - IMP_ERR_DEF_NUM    // 执行成功
#define IMP_ERR_DEV_RUNFAIL             FR_RET_FAIL - IMP_ERR_DEF_NUM    // 执行失败
#define IMP_ERR_DEV_PARAM               FR_RET_PARA - IMP_ERR_DEF_NUM    // 参数错误
#define IMP_ERR_DEV_OPENFAIL            FR_RET_NDEV - IMP_ERR_DEF_NUM    // 打开设备失败
#define IMP_ERR_DEV_NOTOPEN             FR_RET_NINI - IMP_ERR_DEF_NUM    // 未打开设备
#define IMP_ERR_DEV_BUSY                FR_RET_BUSY - IMP_ERR_DEF_NUM    // 设备繁忙
#define IMP_ERR_DEV_DATA                FR_RET_DATA - IMP_ERR_DEF_NUM    // 图像数据不正确
#define IMP_ERR_DEV_OFFLINE             FR_RET_NLNK - IMP_ERR_DEF_NUM    // 设备断开
#define IMP_ERR_DEV_LOADDLL             FR_RET_NDRV - IMP_ERR_DEF_NUM    // 加载设备库失败
#define IMP_ERR_DEV_LOADALGO            FR_RET_NALG - IMP_ERR_DEF_NUM    // 加载算法库失败
#define IMP_ERR_DEV_MEMORY              FR_RET_NMEM - IMP_ERR_DEF_NUM    // 内存分配失败
#define IMP_ERR_DEV_ISOPEN              FR_RET_ARDY - IMP_ERR_DEF_NUM    // 已经打开设备
#define IMP_ERR_DEV_AUTH                FR_RET_AUTH - IMP_ERR_DEF_NUM    // 算法授权失败
#define IMP_ERR_DEV_IPC                 FP_RET_IPCF - IMP_ERR_DEF_NUM    // IPC错误

/***************************************************************************
// 无分类　宏定义
***************************************************************************/
#define LOG_NAME                            "DevImpl_TCF261.log"    // 缺省日志名
#define DLL_DEVLIB_NAME                     "libfacesdk.so"         // 缺省动态库名


// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }


/***************************************************************************
// 封装类: 命令编辑、发送接收等处理。
***************************************************************************/
class CDevImpl_TCF261 : public CLogManage, public CDevImpl_DEF
{
public:
    CDevImpl_TCF261();
    CDevImpl_TCF261(LPSTR lpLog);
    CDevImpl_TCF261(LPSTR lpLog, LPCSTR lpDevType);
    virtual ~CDevImpl_TCF261();

public:
    INT OpenDevice();                                               // 打开设备
    INT CloseDevice();                                              // 关闭设备
    BOOL IsDeviceOpen();                                            // 设备是否Open成功
    BOOL GetIsLiveSucc();                                           // 获取活检成功标记
    INT GetLiveErrCode();                                           // 获取活检错误码
    void SetLiveSuccFlag(BOOL bFlag = TRUE);                        // 设置活检成功标记
    void SetLiveErrCode(INT nCode);                                 // 设置活检错误码
    INT SetSDKToolPath(LPSTR lpPath);                               // 设置SDK工具路径
    LPSTR ConvertCode_Impl2Str(INT nErrCode);                       // Impl错误码解析
    LPSTR ConvertCode_TCF2Str(INT nErrCode);                        // TCF设备错误码解析
    INT ConvertCode_TCF2Impl(INT nErrCode);                         // TCF错误码转Impl错误码

public:    // DevXXX.SetData相关
    INT SetReConFlag(BOOL bFlag);                                   // 设置断线重连标记
    INT SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)

public:     // 接口函数封装
    // 1. 初始化SDK
    INT CreateFaceCallBack(CaptureCallBackDetectInfo pDetectInfo, LPCSTR lpParentDir = nullptr);
    // 2. 反初始化/释放SDK
    INT DestroyFaceCallBack();
    // 3. 打开人脸设备
    INT StartCamera();
    // 4. 关闭人脸设备
    INT StopCamera();
    // 5. 暂停预览画面
    INT PauseAndPlaya(BOOL bIsPause);
    // 6. 抓拍一张图片
    INT TakePicture(LPCSTR lpcFilePath);
    // 7. 开始人脸活检抓拍
    // 参数: mode，true全景照片，false，只有活体头部的头像照片
    //      strContent，保留
    //      strFilePath，传入图片文件url地址，图片格式支持bmp，jpeg，base64编码的jpeg
    INT StartLiveDetecte(BOOL bMode, LPCSTR lpcContent, LPCSTR lpcFilePath);
    // 8. 停止人脸活检抓拍
    INT StopLiveDetecte();
    // 9. 建立预览窗体
    INT CreateWindow(ULONG ulWndHandle, INT nX, INT nY, INT nWidth, INT nHeight);
    // 10. 关闭预览窗体
    INT CloseWindow();
    // 11. 比对两张人脸照片
    INT FaceCompare(LPCSTR lpcPicA, LPCSTR lpcPicB, USHORT usScoreIn, USHORT *usScoreOut);
    // 12. 人脸摄像头状态
    INT GetStatus(BOOL bIsPrtLog = TRUE);
    // 13. 获取设备版本信息
    INT GetFirmwareVersion(LPSTR lpVer, WORD wSize);
    // 14. 未知
    INT HideTips(BOOL bIsHide);

private:
    void    Init();

private:    // 变量定义
    CSimpleMutex    m_cMutex;
    CHAR            m_szDevType[64];                                    // 设备类型
    BOOL            m_bDevOpenOk;                                       // 设备Open标记
    CHAR            m_szSDKToolPath[MAX_PATH];                          // SDK工具目录
    BOOL            m_bReCon;                                           // 是否断线重连状态
    BOOL            m_bLiveDetectSucc;                                  // 活检成功标记
    INT             m_nLiveErrCode;                                     // 活体检测错误码保存
    CHAR            m_szErrStr[1024];                                   // IMPL错误码解析
    CHAR            m_szErrStrTCF[1024];                                // TCF错误码解析
    INT             m_nRetErrOLD[12];                                   // 处理错误值保存(0:库加载/1:设备连接[初始化SDK]/
                                                                        //  2:设备连接[打开人脸设备]
                                                                        //  2:设备状态/3介质状态/4检查设备状态/
                                                                        //  5:设置设备列表/6:设备列表)

private: // 接口加载(QLibrary方式)    
    BOOL    bLoadLibrary();                                             // 加载动态库
    void    vUnLoadLibrary();                                           // 释放动态库
    BOOL    bLoadLibIntf();                                             // 加载动态库接口
    void    vInitLibFunc();                                             // 动态库接口初始化

private: // 接口加载(QLibrary方式)
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;

private: // 动态库接口定义
    FR_CREATEFACECALLBACK   FR_CreateFaceCallBack;      // 1. 初始化SDK
    FR_DESTROYFACECALLBACK  FR_DestroyFaceCallBack;     // 2. 反初始化/释放SDK
    FR_STARTCAMERA          FR_StartCamera;             // 3. 打开人脸设备
    FR_STOPCAMERA           FR_StopCamera;              // 4. 关闭人脸设备
    FR_PAUSEANDPLAY         FR_PauseAndPlay;            // 5. 暂停预览画面
    FR_TAKEPICTRUE          FR_TakePicture;             // 6. 抓拍一张图片
    FR_STARTLIVEDETECT      FR_StartLiveDetect;         // 7. 开始人脸活检抓拍
    FR_STOPLIVEDETECT       FR_StopLiveDetect;          // 8. 停止人脸活检抓拍
    FR_CREATEWINDOW         FR_CreateWindow;            // 9. 建立预览窗体
    FR_CLOSEWINDOW          FR_CloseWindow;             // 10. 关闭预览窗体
    FR_FACECOMPARE          FR_FaceCompare;             // 11. 比对两张人脸照片
    FR_GETSTATUS            FR_GetStatus;               // 12. 人脸摄像头状态
    FR_GETFIRMWAREVERSION   FR_GetFirmwareVersion;      // 13. 获取设备版本信息
    FR_HIDETIPS             FR_HideTips;                // 14. 未知
};

class CDevImpl_TCF261_Task
{
public:
    CDevImpl_TCF261 *cDevTCF261;
};

// -------------------------------------- END --------------------------------------
