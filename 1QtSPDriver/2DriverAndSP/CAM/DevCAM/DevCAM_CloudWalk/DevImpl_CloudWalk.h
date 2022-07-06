#pragma once
#define LINUX

#include <string>
#include "opencv2/opencv.hpp"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "cloudwalk.h"

using namespace std;

#define DLL_CLOUDWALK_NAME  "libcwlivdetengine.so"

#define LOG_NAME     "CloudWalk.log"

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
        return FALSE;   \
    }

/*
命令编辑、发送接收等处理。
*/

class CDevImpl_CloudWalk : public CLogManage
{
public:
    CDevImpl_CloudWalk();
    virtual ~CDevImpl_CloudWalk();
public:
    BOOL OpenDevice();
    BOOL OpenDevice(WORD wType);
    BOOL CloseDevice();
    BOOL IsDeviceOpen();
    LPSTR GetErrorStr(LONG lErrCode);

public: // 接口函数封装
    // 1.3 获取COM服务及算法版本号
    BOOL bGetVersion(LPSTR lpVer, long lSize);
    // 1.4 注册相机回调回显函数(绘制人脸框及显示回调函数)
    BOOL bRegisterPreviewHandle(HANDLE hWnd);
    BOOL bRegisterPreviewHandle(HANDLE hWnd, preview_func fuPreView);
    // 1.5 注册检测回调函数对象(活体检测及回调设置)
    BOOL bRegisterDetectHandle(HANDLE hWnd);
    BOOL bRegisterDetectHandle(HANDLE hWnd,
                               face_det_func fuFaceDet,
                               live_det_func fuLiveDet);
    // 1.6 加载检测模型及设置检测参数,初始化检测器(加载算法模型)
    BOOL bInitDetectorParam(short modelMode, short livenessMode, short licenseType, char* configFile,
                            char* faceDetectFile, char* faceKeyPointDetectFile, char* faceKeyPointTrackFile,
                            char* faceQualityFile, char* faceLivenessFile, short gpu, short multiThread);
    // 1.7 是否进行人脸追踪显示
    BOOL bEnableFaceTrace(BOOL bEnable);
    // 1.9 依据相机在设备显示的序号开启相机
    BOOL bOpenCamera(SHORT deviceIndex, SHORT deviceMode, SHORT width, SHORT height);
    // 1.10 依据相机硬件信息开启相机(VID/PID)
    BOOL bOpenCameraEx(LPSTR vid, LPSTR pid, SHORT deviceMode, SHORT width, SHORT height);
    // 1.11 关闭当前程序打开的所有相机设备
    BOOL bCloseAllCameras();
    // 1.15 开启活体检测
    BOOL bStartLiveDetect(BOOL isContinue);
    //  1.16 停止连续活体检测
    BOOL bStopLiveDetect();
    // 1.20 获取本次活体检测最佳人脸图像
    BOOL bGetBestFace(float ratio, BOOL isBase64, CWLiveImage* image);
    // 1.21 保存活体最佳人脸到指定路径
    BOOL bSaveBestFace(LPSTR fileName, float ratio);
    // 1.22 通过序号得到相机状态
    BOOL bGetCameraStatus(short index, CWCameraState* status);
    // 1.23 通过相机硬件信息VID/PID得到相机状态
    BOOL bGetCameraStatusEx(LPSTR vid, LPSTR pid, CWCameraState* state);
    // 1.26 将图片转换成base64编码格式
    BOOL bImage2Base64Format(LPSTR imageFilePath,  std::string* strBase64);//LPSTR strBase64);
    // 1.32 获得相机固件版本信息，依照相机在设备上的序号
    BOOL bGetFirmwareVersion(short deviceIndex, LPSTR lpFmVer);
    // 1.33 获得相机固件版本信息，依照相机在设备VID/PID
    BOOL bGetFirmwareVersionEx(LPSTR vid, LPSTR pid, LPSTR lpFmVer);

private:
    HANDLE      m_cwDetector;
    BOOL        m_bDevOpenOk;
    CHAR        m_szErrStr[1024];

private: // 接口加载
    BOOL LoadCloudWalkDll();
    void UnloadCloudWalkDll();
    BOOL LoadCloudWalkIntf();

private: // 接口加载
    char        m_szCloudWalkDllPath[MAX_PATH];
    QLibrary    m_CloudWalkLibrary;
    BOOL        m_bLoadIntfFail;

private: // 动态库接口定义
    pcwEngineCreateDetector         cwEngineCreateDetector;         // 1.1 创建检测对象
    pcwEngineReleaseDetector        cwEngineReleaseDetector;        // 1.2 释放检测对象及COM服务
    pcwEnginGetVersion              cwEnginGetVersion;              // 1.3 获取COM服务及算法版本号
    pcwEngineRegisterPreviewHandle  cwEngineRegisterPreviewHandle;  // 1.4 注册相机回调回显函数
    pcwEngineRegisterDetectHandle   cwEngineRegisterDetectHandle;   // 1.5 注册检测回调函数对象
    pcwEngineInitDetectorParam      cwEngineInitDetectorParam;      // 1.6 加载检测模型及设置检测参数
    pcwEngineEnableFaceTrace        cwEngineEnableFaceTrace;        // 1.7 是否进行人脸追踪显示
    pcwEngineOpenCamera             cwEngineOpenCamera;             // 1.9 依据相机在设备显示的序号开启相机
    pcwEngineOpenCameraEx           cwEngineOpenCameraEx;           // 1.10 依据相机硬件信息开启相机
    pcwEngineCloseAllCameras        cwEngineCloseAllCameras;        // 1.11 关闭当前程序打开的所有相机设备
    pcwEngineStartLiveDetect        cwEngineStartLiveDetect;        // 1.15 开启活体检测
    pcwEngineStopLiveDetect         cwEngineStopLiveDetect;         // 1.16 停止连续活体检测
    pcwEngineGetBestFace            cwEngineGetBestFace;            // 1.20 获取本次活体检测最佳人脸图像
    pcwEngineSaveBestFace           cwEngineSaveBestFace;           // 1.21 保存活体最佳人脸到指定路径
    pcwEngineGetCameraStatus        cwEngineGetCameraStatus;        // 1.22 通过序号得到相机状态
    pcwEngineGetCameraStatusEx      cwEngineGetCameraStatusEx;      // 1.23 通过相机硬件信息VID/PID得到相机状态    
    pcwEngineImage2Base64Format     cwEngineImage2Base64Format;     // 1.26 将图片转换成base64编码格式
    pcwEngineGetFirmwareVersion     cwEngineGetFirmwareVersion;     // 1.32 获得相机固件版本信息，依照相机在设备上的序号
    pcwEngineGetFirmwareVersionEx   cwEngineGetFirmwareVersionEx;   // 1.33 获得相机固件版本信息，依照相机在设备VID/PID

};
