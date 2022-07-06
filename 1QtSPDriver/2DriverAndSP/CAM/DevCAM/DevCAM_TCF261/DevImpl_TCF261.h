#pragma once
#define LINUX

#include <string>

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "TCF261.h"

using namespace std;

#define DLL_TCF261_NAME     "libfacesdk.so"
#define LOG_NAME            "TCF261.log"

#ifndef CALL_MODE
#ifdef _WIN32
#define CALL_MODE	__stdcall
#else
#define CALL_MODE
#endif
#endif

#define FUNC_POINTER_ERROR_RETURN(LPFUNC, LPFUNC2) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE; \
        Log(ThisFile, 1, "动态库接口<%s>加载 fail. ", LPFUNC2); \
        return FALSE;   \
    }

class CDevImpl_TCF261;

class CDevImpl_TCF261_Task
{
public:
    CDevImpl_TCF261 *cDevTCF261;
};

/*
命令编辑、发送接收等处理。
*/
class CDevImpl_TCF261 : public CLogManage
{
public:
    CDevImpl_TCF261();
    CDevImpl_TCF261(LPSTR lpLog);
    virtual ~CDevImpl_TCF261();
public:
    void Init();
    BOOL OpenDevice();
    BOOL CloseDevice();
    BOOL IsDeviceOpen();
    LPSTR GetErrorStr(INT lErrCode);
    void SetTakePicStop(BOOL bFlag = TRUE);
    BOOL GetTakePicStop();

public: // 接口函数封装
    // 1. 初始化SDK
    BOOL bCreateFaceCallBack(CaptureCallBackDetectInfo pDetectInfo, LPCSTR lpParentDir = NULL);
    // 2. 反初始化/释放SDK
    BOOL bDestroyFaceCallBack();
    // 3. 打开人脸设备
    BOOL bStartCamera();
    // 4. 关闭人脸设备
    BOOL bStopCamera();
    // 5. 暂停预览画面
    BOOL bPauseAndPlaya(bool bIsPause);
    // 6. 抓拍一张图片
    BOOL bTakePicture(LPCSTR lpcFilePath);
    // 7. 开始人脸活检抓拍
    // 参数: mode，true全景照片，false，只有活体头部的头像照片
    //      strContent，保留
    //      strFilePath，传入图片文件url地址，图片格式支持bmp，jpeg，base64编码的jpeg
    BOOL bStartLiveDetecte(BOOL bMode, LPCSTR lpcContent, LPCSTR lpcFilePath);
    // 8. 停止人脸活检抓拍
    BOOL bStopLiveDetecte();
    // 9. 建立预览窗体
    BOOL bCreateWindow(ULONG ulWndHandle, INT nX, INT nY, INT nWidth, INT nHeight);
    // 10. 关闭预览窗体
    BOOL bCloseWindow();
    // 11. 比对两张人脸照片
    BOOL bFaceCompare(LPCSTR lpcPicA, LPCSTR lpcPicB, USHORT usScoreIn, USHORT *usScoreOut);
    // 12. 人脸摄像头状态
    BOOL bGetStatus(INT *nStatOut);
    // 13. 获取设备版本信息
    BOOL bGetFirmwareVersion(LPSTR lpVer, WORD wSize);
    // 14. 未知
    BOOL bHideTips(bool bIsHide);

private: // 接口加载
    BOOL LoadDeviceDll();
    void UnLoadDeviceDll();
    BOOL LoadDeviceIntf();

private: // 接口加载
    char        m_szDllPath[MAX_PATH];  // 设备动态库路径
    char        m_szSDKToolPath[MAX_PATH];  // SDK工具目录
    QLibrary    m_qDLLLibrary;
    BOOL        m_bLoadIntfFail;

private:
    BOOL        m_bIsTakePicExStop;         // 抓拍是否结束
    BOOL        m_bDevOpenOk;
    CHAR        m_szErrStr[1024];

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
