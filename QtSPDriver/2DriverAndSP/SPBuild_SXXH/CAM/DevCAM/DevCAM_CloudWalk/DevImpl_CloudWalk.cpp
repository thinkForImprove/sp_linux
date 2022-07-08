#include "DevImpl_CloudWalk.h"

static const char *ThisFile = "DevImpl_CloudWalk.cpp";

CDevImpl_CloudWalk::CDevImpl_CloudWalk()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CloudWalk::CDevImpl_CloudWalk(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_CloudWalk::Init()
{
    QString strDllName(QString::fromLocal8Bit(DLL_CLOUDWALK_NAME));
    //strDllName.prepend("CloudWalk/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szCloudWalkDllPath, 0x00, sizeof(m_szCloudWalkDllPath));
    sprintf(m_szCloudWalkDllPath, "%s", strDllName.toStdString().c_str());
    m_CloudWalkLibrary.setFileName(m_szCloudWalkDllPath);
    m_bLoadIntfFail = TRUE;

    cwEngineCreateDetector = NULL;         // 1.1 创建检测对象
    cwEngineReleaseDetector = NULL;        // 1.2 释放检测对象及COM服务
    cwEnginGetVersion = NULL;              // 1.3 获取COM服务及算法版本号
    cwEngineRegisterPreviewHandle = NULL;  // 1.4 注册相机回调回显函数
    cwEngineRegisterDetectHandle = NULL;   // 1.5 注册检测回调函数对象
    cwEngineInitDetectorParam = NULL;      // 1.6 加载检测模型及设置检测参数
    cwEngineEnableFaceTrace = NULL;        // 1.7 是否进行人脸追踪显示
    cwEngineOpenCamera = NULL;             // 1.9 依据相机在设备显示的序号开启相机
    cwEngineOpenCameraEx = NULL;           // 1.10 依据相机硬件信息开启相机
    cwEngineCloseAllCameras = NULL;        // 1.11 关闭当前程序打开的所有相机设备    
    cwEngineEnumCameras = NULL;            // 1.12 枚举当前设备上的所有相机	// 30-00-00-00(FT#0031)
    cwEngineStartLiveDetect = NULL;        // 1.15 开启活体检测
    cwEngineStopLiveDetect = NULL;         // 1.16 停止连续活体检测
    cwEngineCaptureCurrentFrame = NULL;    // 1.17 抓拍当前画面帧
    cwEngineSaveCurrentFrame = NULL;       // 1.18 保存当前帧至指定文件路径参数
    cwEngineEnableVoiceTip = NULL;         // 1.19 开启语音提示功能	// 30-00-00-00(FT#0031)
    cwEngineGetBestFace = NULL;            // 1.20 获取本次活体检测最佳人脸图像
    cwEngineSaveBestFace = NULL;           // 1.21 保存活体最佳人脸到指定路径
    cwEngineGetCameraStatus = NULL;        // 1.22 通过序号得到相机状态
    cwEngineGetCameraStatusEx = NULL;      // 1.23 通过相机硬件信息VID/PID得到相机状态
    cwEngineImage2Base64Format = NULL;     // 1.26 将图片转换成base64编码格式
    cwEngineGetFirmwareVersion = NULL;     // 1.32 获得相机固件版本信息，依照相机在设备上的序号
    cwEngineGetFirmwareVersionEx = NULL;   // 1.33 获得相机固件版本信息，依照相机在设备VID/PID

    m_cwDetector = NULL;

    LoadCloudWalkDll();
}

CDevImpl_CloudWalk::~CDevImpl_CloudWalk()
{
    CloseDevice();
    UnloadCloudWalkDll();
}


BOOL CDevImpl_CloudWalk::LoadCloudWalkDll()
{
    if (m_CloudWalkLibrary.isLoaded() != true)
    {
        if (m_CloudWalkLibrary.load() != true)
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szCloudWalkDllPath, m_CloudWalkLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }

    if (m_bLoadIntfFail)
    {
        if (LoadCloudWalkIntf() != TRUE)
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szCloudWalkDllPath, m_CloudWalkLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }
    return TRUE;
}

void CDevImpl_CloudWalk::UnloadCloudWalkDll()
{
    if (m_CloudWalkLibrary.isLoaded())
    {
        m_CloudWalkLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_CloudWalk::LoadCloudWalkIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1.1 创建检测对象
    cwEngineCreateDetector = (pcwEngineCreateDetector)m_CloudWalkLibrary.resolve("cwEngineCreateDetector");
    FUNC_POINTER_ERROR_RETURN(cwEngineCreateDetector, "cwEngineCreateDetector");

    // 1.2 释放检测对象及COM服务
    cwEngineReleaseDetector = (pcwEngineReleaseDetector)m_CloudWalkLibrary.resolve("cwEngineReleaseDetector");
    FUNC_POINTER_ERROR_RETURN(cwEngineReleaseDetector, "cwEngineReleaseDetector");

    // 1.3 获取COM服务及算法版本号
    cwEnginGetVersion = (pcwEnginGetVersion)m_CloudWalkLibrary.resolve("cwEngineGetVersion");
    FUNC_POINTER_ERROR_RETURN(cwEnginGetVersion, "cwEngineGetVersion");

    // 1.4 注册相机回调回显函数
    cwEngineRegisterPreviewHandle = (pcwEngineRegisterPreviewHandle)m_CloudWalkLibrary.resolve("cwEngineRegisterPreviewHandle");
    FUNC_POINTER_ERROR_RETURN(cwEngineRegisterPreviewHandle, "cwEngineRegisterPreviewHandle");

    // 1.5 注册检测回调函数对象
    cwEngineRegisterDetectHandle = (pcwEngineRegisterDetectHandle)m_CloudWalkLibrary.resolve("cwEngineRegisterDetectHandle");
    FUNC_POINTER_ERROR_RETURN(cwEngineRegisterDetectHandle, "cwEngineRegisterDetectHandle");

    // 1.6 加载检测模型及设置检测参数
    cwEngineInitDetectorParam = (pcwEngineInitDetectorParam)m_CloudWalkLibrary.resolve("cwEngineInitDetectorParam");
    FUNC_POINTER_ERROR_RETURN(cwEngineInitDetectorParam, "cwEngineInitDetectorParam");

    // 1.7 是否进行人脸追踪显示
    cwEngineEnableFaceTrace = (pcwEngineEnableFaceTrace)m_CloudWalkLibrary.resolve("cwEngineEnableFaceTrace");
    FUNC_POINTER_ERROR_RETURN(cwEngineEnableFaceTrace, "cwEngineEnableFaceTrace");

    // 1.9 依据相机在设备显示的序号开启相机
    cwEngineOpenCamera = (pcwEngineOpenCamera)m_CloudWalkLibrary.resolve("cwEngineOpenCamera");
    FUNC_POINTER_ERROR_RETURN(cwEngineOpenCamera, "cwEngineOpenCamera");

    // 1.10 依据相机硬件信息开启相机
    cwEngineOpenCameraEx = (pcwEngineOpenCameraEx)m_CloudWalkLibrary.resolve("cwEngineOpenCameraEx");
    FUNC_POINTER_ERROR_RETURN(cwEngineOpenCameraEx, "cwEngineOpenCameraEx");

    // 1.11 关闭当前程序打开的所有相机设备
    cwEngineCloseAllCameras = (pcwEngineCloseAllCameras)m_CloudWalkLibrary.resolve("cwEngineCloseAllCameras");
    FUNC_POINTER_ERROR_RETURN(cwEngineCloseAllCameras, "cwEngineCloseAllCameras");

    // 1.12 枚举当前设备上的所有相机	// 30-00-00-00(FT#0031)
    cwEngineEnumCameras = (pcwEngineEnumCameras)m_CloudWalkLibrary.resolve("cwEngineEnumCameras");
    FUNC_POINTER_ERROR_RETURN(cwEngineEnumCameras, "cwEngineEnumCameras");

    // 1.15 开启活体检测
    cwEngineStartLiveDetect = (pcwEngineStartLiveDetect)m_CloudWalkLibrary.resolve("cwEngineStartLiveDetect");
    FUNC_POINTER_ERROR_RETURN(cwEngineStartLiveDetect, "cwEngineStartLiveDetect");

    // 1.16 停止连续活体检测
    cwEngineStopLiveDetect = (pcwEngineStopLiveDetect)m_CloudWalkLibrary.resolve("cwEngineStopLiveDetect");
    FUNC_POINTER_ERROR_RETURN(cwEngineStopLiveDetect, "cwEngineStopLiveDetect");

    // 1.17 抓拍当前画面帧
    cwEngineCaptureCurrentFrame = (pcwEngineCaptureCurrentFrame)m_CloudWalkLibrary.resolve("cwEngineCaptureCurrentFrame");
    FUNC_POINTER_ERROR_RETURN(cwEngineCaptureCurrentFrame, "cwEngineCaptureCurrentFrame");

    // 1.18 保存当前帧至指定文件路径参数
    cwEngineSaveCurrentFrame = (pcwEngineSaveCurrentFrame)m_CloudWalkLibrary.resolve("cwEngineSaveCurrentFrame");
    FUNC_POINTER_ERROR_RETURN(cwEngineSaveCurrentFrame, "cwEngineSaveCurrentFrame");

    // 1.19 开启语音提示功能	// 30-00-00-00(FT#0031)
    cwEngineEnableVoiceTip = (pcwEngineEnableVoiceTip)m_CloudWalkLibrary.resolve("cwEngineEnableVoiceTip");
    FUNC_POINTER_ERROR_RETURN(cwEngineEnableVoiceTip, "cwEngineEnableVoiceTip");

    // 1.20获取本次活体检测最佳人脸图像
    cwEngineGetBestFace = (pcwEngineGetBestFace)m_CloudWalkLibrary.resolve("cwEngineGetBestFace");
    FUNC_POINTER_ERROR_RETURN(cwEngineGetBestFace, "cwEngineGetBestFace");

    // 1.21 保存活体最佳人脸到指定路径
    cwEngineSaveBestFace = (pcwEngineSaveBestFace)m_CloudWalkLibrary.resolve("cwEngineSaveBestFace");
    FUNC_POINTER_ERROR_RETURN(cwEngineSaveBestFace, "cwEngineSaveBestFace");

    // 1.22 通过序号得到相机状态
    cwEngineGetCameraStatus = (pcwEngineGetCameraStatus)m_CloudWalkLibrary.resolve("cwEngineGetCameraStatus");
    FUNC_POINTER_ERROR_RETURN(cwEngineGetCameraStatus, "cwEngineGetCameraStatus");

    // 1.23 通过相机硬件信息VID/PID得到相机状态
    cwEngineGetCameraStatusEx = (pcwEngineGetCameraStatusEx)m_CloudWalkLibrary.resolve("cwEngineGetCameraStatusEx");
    FUNC_POINTER_ERROR_RETURN(cwEngineGetCameraStatusEx, "cwEngineGetCameraStatusEx");

    // 1.26 将图片转换成base64编码格式
    cwEngineImage2Base64Format = (pcwEngineImage2Base64Format)m_CloudWalkLibrary.resolve("cwEngineImage2Base64Format");
    FUNC_POINTER_ERROR_RETURN(cwEngineImage2Base64Format, "cwEngineImage2Base64Format");

    // 1.32 获得相机固件版本信息，依照相机在设备上的序号
    cwEngineGetFirmwareVersion = (pcwEngineGetFirmwareVersion)m_CloudWalkLibrary.resolve("cwEngineGetFirmwareVersion");
    FUNC_POINTER_ERROR_RETURN(cwEngineGetFirmwareVersion, "cwEngineGetFirmwareVersion");

    // 1.33 获得相机固件版本信息，依照相机在设备VID/PID
    cwEngineGetFirmwareVersionEx = (pcwEngineGetFirmwareVersionEx)m_CloudWalkLibrary.resolve("cwEngineGetFirmwareVersionEx");
    FUNC_POINTER_ERROR_RETURN(cwEngineGetFirmwareVersionEx, "cwEngineGetFirmwareVersionEx");

    return TRUE;
}


BOOL CDevImpl_CloudWalk::OpenDevice()
{
    return OpenDevice(0);
}

BOOL CDevImpl_CloudWalk::OpenDevice(WORD wType)
{
    //如果已打开，则关闭
    CloseDevice();

    m_bDevOpenOk = FALSE;

    if (LoadCloudWalkDll() != TRUE)
    {
        Log(ThisFile, 1, "加载动态库: OpenDevice()->LoadCloudWalkDll() fail. ");
        return FALSE;
    }

    if (m_cwDetector != nullptr)				// 30-00-00-00(FT#0031)
    {
        // 释放检测对象及COM服务
        cwEngineReleaseDetector(m_cwDetector);	// 30-00-00-00(FT#0031)
        m_cwDetector = nullptr;					// 30-00-00-00(FT#0031)
    }

    m_cwDetector = cwEngineCreateDetector();
    if (m_cwDetector == NULL)
    {
        Log(ThisFile, 1, "创建检测对象: OpenDevice()->cwEngineCreateDetector(): Create Detector Handle fail. ");
        return FALSE;
    }
    Log(ThisFile, 1, "创建检测对象: OpenDevice()->cwEngineCreateDetector(): Create Detector Handle succ. ");

    m_bDevOpenOk = TRUE;

    return TRUE;
}

BOOL CDevImpl_CloudWalk::CloseDevice()
{
    if (m_cwDetector != NULL)
    {
        cwEngineStopLiveDetect(m_cwDetector);// 1.2 释放检测对象及COM服务
        cwEngineReleaseDetector(m_cwDetector);// 1.16 停止连续活体检测
        Log(ThisFile, 1, "停止连续活体检测: Close Device()->cwEngineStopLiveDetect().");
        Log(ThisFile, 1, "释放检测对象: Close Device()->cwEngineReleaseDetector().");
        m_cwDetector = NULL;
    }

    UnloadCloudWalkDll();
    m_bDevOpenOk = FALSE;

    Log(ThisFile, 1, "设备关闭,释放动态库: Close Device, unLoad CloudWalk Dll.");

    return TRUE;
}

BOOL CDevImpl_CloudWalk::IsDeviceOpen()
{
    //return (m_bDevOpenOk == TRUE && m_cwDetector != NULL ? TRUE : FALSE);
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 1.3 获取COM服务及算法版本号
BOOL CDevImpl_CloudWalk::bGetVersion(LPSTR lpVer, long lSize)
{
    char *tmp = nullptr;
    // COM版本
    tmp = cwEnginGetVersion(m_cwDetector, 0);
    if (tmp != nullptr)
    {
        memcpy(lpVer, tmp, strlen(tmp) > lSize ? lSize : strlen(tmp));
    } else
    {
        Log(ThisFile, 1, "获取COM版本函数:bGetVersion()->cwEnginGetVersion() fail. ");
    }
    // 中间版本
    tmp = nullptr;
    tmp = cwEnginGetVersion(m_cwDetector, 1);
    if (tmp != nullptr)
    {
        if (strlen(lpVer) < lSize && strlen(tmp) > 0 && strlen(lpVer) > 0)
        {
           sprintf(lpVer + strlen(lpVer), "%s", " ; ");
           memcpy(lpVer + strlen(lpVer), tmp,
                  strlen(tmp) > (lSize - strlen(lpVer)) ? (lSize - strlen(lpVer)) : strlen(tmp));
        }
    } else
    {
        Log(ThisFile, 1, "获取中间版本函数:bGetVersion()->cwEnginGetVersion() fail. ");
    }
    // 算法版本
    tmp = nullptr;
    tmp = cwEnginGetVersion(m_cwDetector, 2);
    if (tmp != nullptr)
    {
        if (strlen(lpVer) < lSize && strlen(tmp) > 0 && strlen(lpVer) > 0)
        {
           sprintf(lpVer + strlen(lpVer), "%s", " ; ");
           memcpy(lpVer + strlen(lpVer), tmp,
                  strlen(tmp) > (lSize - strlen(lpVer)) ? (lSize - strlen(lpVer)) : strlen(tmp));
        }
    } else
    {
        Log(ThisFile, 1, "获取算法版本函数:bGetVersion()->cwEnginGetVersion() fail. ");
    }

    return TRUE;
}

//  1.4 注册相机回调回显函数(绘制人脸框及显示回调函数)
BOOL CDevImpl_CloudWalk::bRegisterPreviewHandle(HANDLE hWnd)
{
    cw_preview_observer ober;
    ober.handle = hWnd;
    ober.func = onPreviewCallback;
    LONG code =  cwEngineRegisterPreviewHandle(m_cwDetector, &ober);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "注册相机回调函数:bRegisterPreviewHandle()->cwEngineRegisterPreviewHandle() fail. ReturnCode:%s", GetErrorStr(code));
        return FALSE;
    }
    return TRUE;
}

BOOL CDevImpl_CloudWalk::bRegisterPreviewHandle(HANDLE hWnd, preview_func fuPreView)
{
    cw_preview_observer ober;
    ober.handle = hWnd;
    ober.func = fuPreView;
    LONG code =  cwEngineRegisterPreviewHandle(m_cwDetector, &ober);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "注册相机回调函数:bRegisterPreviewHandle()->cwEngineRegisterPreviewHandle() fail. ReturnCode:%s", GetErrorStr(code));
        return FALSE;
    }
    return TRUE;
}

// 1.5 注册检测回调函数对象(活体检测及回调设置)
BOOL CDevImpl_CloudWalk::bRegisterDetectHandle(HANDLE hWnd)
{
    cw_detect_observer ober;
    ober.handle = hWnd;
    ober.face_func = onFaceDetCallback;
    ober.live_func = onLiveDetCallback;
    LONG code = cwEngineRegisterDetectHandle(m_cwDetector, &ober);
    if (code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "注册检测回调函数: bRegisterDetectHandle()->cwEngineRegisterDetectHandle() fail. ReturnCode:%s", GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

BOOL CDevImpl_CloudWalk::bRegisterDetectHandle(HANDLE hWnd,
                                               face_det_func fuFaceDet,
                                               live_det_func fuLiveDet)
{
    cw_detect_observer ober;
    ober.handle = hWnd;
    ober.face_func = onFaceDetCallback;
    ober.live_func = fuLiveDet;
    LONG code = cwEngineRegisterDetectHandle(m_cwDetector, &ober);
    if (code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "注册检测回调函数: bRegisterDetectHandle()->cwEngineRegisterDetectHandle() fail. ReturnCode:%s", GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.6 加载检测模型及设置检测参数,初始化检测器(加载算法模型)
BOOL CDevImpl_CloudWalk::bInitDetectorParam(short modelMode,                 // 模型加载方式(0/文件加载,1/内存加载)
                                           short livenessMode,              // 活体检测方式(0/单目活体,1/红外活体,2/结构光后置,3TOF光)
                                           short licenseType,               // 授权类型(1/云从相机芯片授权,2/云从hasp授权,3/云从cw临时授权码授权,4/云从绑定授权码授权)
                                           char* configFile,                // 活体配置文件
                                           char* faceDetectFile,            // 人脸检测模型
                                           char* faceKeyPointDetectFile,    // 关键点检测模型
                                           char* faceKeyPointTrackFile,     // 关键点跟踪模型
                                           char* faceQualityFile,           // 人脸质量评估模型
                                           char* faceLivenessFile,          // 活体模型
                                           short gpu,                       // 是否使用GPU(1/使用GPU,1使用CPU)
                                           short multiThread)               // 是否使用多线程(1/多线程,0/单线程)
{
    LONG code = cwEngineInitDetectorParam(m_cwDetector, (short)modelMode, (short)livenessMode,
                                          (short)licenseType, configFile, faceDetectFile,
                                          faceKeyPointDetectFile, faceKeyPointTrackFile,
                                          faceQualityFile, faceLivenessFile,
                                          gpu, (short)multiThread);
    if(code != CW_LIVENESS_SUCCESSFUL && code != CW_LIVENESS_REINIT_DETECTOR_ERROR)	// 30-00-00-00(FT#0031)
    {
        char* p = GetErrorStr(code);
        Log(ThisFile, 1,
            "加载检测模型: bInitDetectorParam()->cwEngineInitDetectorParam(%d, %d ,%d ,%s ,%s ,%s ,%s ,%s ,%s ,%d ,%d) fail. ReturnCode:%s",
            (short)modelMode, (short)livenessMode, (short)licenseType, configFile, faceDetectFile,
            faceKeyPointDetectFile, faceKeyPointTrackFile, faceQualityFile, faceLivenessFile,
            gpu, (short)multiThread, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.7 是否进行人脸追踪显示
BOOL CDevImpl_CloudWalk::bEnableFaceTrace(BOOL bEnable)
{
    LONG code = cwEngineEnableFaceTrace(m_cwDetector, bEnable);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "设置人脸追踪显示: bEnableFaceTrace()->cwEngineEnableFaceTrace(%d) fail. ReturnCode:%s",
            bEnable, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.9 依据相机在设备显示的序号开启相机
BOOL CDevImpl_CloudWalk::bOpenCamera(SHORT deviceIndex, // 相机序号,相机在设备显示的序号
                                    SHORT deviceMode,  // 相机类型(0/可见图像,1/红外图像,2/深度图像,3/TOF图像,4/欧菲舜宇结构光后置,5/华捷艾米结构光后置,6/奥比中光结构光后置)
                                    SHORT width,       // 请求图像的宽度
                                    SHORT height)      // 请求图像的高度
{
    LONG code = cwEngineOpenCamera(m_cwDetector, deviceIndex, deviceMode, width, height);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "按序号开启相机: bOpenCamera()->cwEngineOpenCamera(%d, %d, %d, %d) fail. ReturnCode:%s",
            deviceIndex, deviceMode, width, height, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.10 依据相机硬件信息开启相机(VID/PID)
BOOL CDevImpl_CloudWalk::bOpenCameraEx(LPSTR vid,         // 相机VID
                                       LPSTR pid,         // 相机PID
                                       SHORT deviceMode,  // 相机类型(0/可见图像,1/红外图像,2/深度图像,3/TOF图像,4/欧菲舜宇结构光后置,5/华捷艾米结构光后置,6/奥比中光结构光后置)
                                       SHORT width,       // 请求图像的宽度
                                       SHORT height)      // 请求图像的高度
{
    LONG code = cwEngineOpenCameraEx(m_cwDetector, vid, pid, deviceMode, width, height);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "按VID/PID开启相机: bOpenCameraEx()->cwEngineOpenCameraEx(%s, %s, %d, %d, %d) fail. ReturnCode:%s",
            vid, pid, deviceMode, width, height, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.11 关闭当前程序打开的所有相机设备
BOOL CDevImpl_CloudWalk::bCloseAllCameras()
{
    LONG code = cwEngineCloseAllCameras(m_cwDetector);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "关闭所有相机设备: bCloseAllCameras()->cwEngineCloseAllCameras() fail. ReturnCode:%s", GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.12 枚举当前设备上的所有相机 // 30-00-00-00(FT#0031)
BOOL CDevImpl_CloudWalk::bEnumCameras(CWCameraDevice **cameraLists)
{
    LONG code = cwEngineEnumCameras(m_cwDetector, cameraLists);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "枚举当前设备上的所有相机: bEnumCameras()->cwEngineEnumCameras() fail. ReturnCode:%s", GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.15 开启活体检测
BOOL CDevImpl_CloudWalk::bStartLiveDetect(BOOL isContinue)  // 是否连续检测(TRUE连续,FALSE单次)
{
    LONG code = cwEngineStartLiveDetect(m_cwDetector, isContinue);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "开启活体检测: bStartLiveDetect()->cwEngineStartLiveDetect(%d) fail. ReturnCode:%s", isContinue, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

//  1.16 停止连续活体检测
BOOL CDevImpl_CloudWalk::bStopLiveDetect()
{
    LONG code = cwEngineStopLiveDetect(m_cwDetector);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "停止连续活体检测: bCloseAllCameras()->cwEngineStopLiveDetect() fail. ReturnCode:%s", GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.17 抓拍当前画面帧
BOOL CDevImpl_CloudWalk::bCaptureCurrentFrame(CWSize size, BOOL isBase64, CWLiveImage* image)
{
    LONG code = cwEngineCaptureCurrentFrame(m_cwDetector, size, isBase64, image);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "抓拍当前画面帧: bCaptureCurrentFrame()->cwEngineCaptureCurrentFrame(%d, %d,%d) fail. ReturnCode:%s",
            size.width, size.height, isBase64, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}
// 1.18 保存当前帧至指定文件路径参数
BOOL CDevImpl_CloudWalk::bSaveCurrentFrame(LPSTR fileName)
{
    LONG code = cwEngineSaveCurrentFrame(m_cwDetector, fileName);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "保存当前帧至指定文件路径参数: bSaveCurrentFrame()->cwEngineSaveCurrentFrame(%s) fail. ReturnCode:%s",
            fileName, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.19 开启语音提示功能	// 30-00-00-00(FT#0031)
BOOL CDevImpl_CloudWalk::bEnableVoiceTip(BOOL bEnable, LPCSTR lpTipFile)
{
    LONG code = cwEngineEnableVoiceTip(m_cwDetector, bEnable, lpTipFile);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "开启语音提示功能: bEnableVoiceTip()->cwEngineEnableVoiceTip(%d, %s) fail. ReturnCode:%s",
            bEnable, lpTipFile, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.20 获取本次活体检测最佳人脸图像
BOOL CDevImpl_CloudWalk::bGetBestFace(float ratio, BOOL isBase64, CWLiveImage* image)
{
    LONG code = cwEngineGetBestFace(m_cwDetector, ratio, isBase64, image);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "获取活体检测最佳人脸图像: bGetBestFace()->cwEngineGetBestFace(%f, %d) fail. ReturnCode:%s",
            ratio, isBase64, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.21 保存活体最佳人脸到指定路径
BOOL CDevImpl_CloudWalk::bSaveBestFace(LPSTR fileName, float ratio)
{
    LONG code = cwEngineSaveBestFace(m_cwDetector, fileName, ratio);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "存活体最佳人脸: bSaveBestFace()->cwEngineSaveBestFace(%s, %d) fail. ReturnCode:%s",
            fileName, ratio, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.22 通过序号得到相机状态
BOOL CDevImpl_CloudWalk::bGetCameraStatus(short index, CWCameraState* status)
{
    LONG code = cwEngineGetCameraStatus(m_cwDetector, (const short)index, status);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "按序号获得相机状态: bGetCameraStatus()->cwEngineGetCameraStatus(%d) fail. ReturnCode:%s",
            index, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.23 通过相机硬件信息VID/PID得到相机状态
BOOL CDevImpl_CloudWalk::bGetCameraStatusEx(LPSTR vid,         // 相机VID
                                           LPSTR pid,          // 相机PID
                                           CWCameraState* state)
{
    LONG code = cwEngineGetCameraStatusEx(m_cwDetector, (const char*)vid, (const char*)pid, state);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "按VID/PID获得相机状态: bGetCameraStatusEx()->cwEngineGetCameraStatusEx(%s, %s) fail. ReturnCode:%s",
            vid, pid, GetErrorStr(code));
        return FALSE;
    }

    return TRUE;
}

// 1.26 将图片转换成base64编码格式
BOOL CDevImpl_CloudWalk::bImage2Base64Format(LPSTR imageFilePath,   // 图像全路径
                                             std::string* strBase64)
                                            //LPSTR strBase64)       // base64编码格式字符串
{
    std::string str;
    LONG code = cwEngineImage2Base64Format(m_cwDetector, imageFilePath, &str);
    if(code != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisFile, 1, "图片转成base64: bImage2Base64Format()->cwEngineImage2Base64Format(%s, %s) fail. ReturnCode:%s",
            imageFilePath, strBase64->c_str(), GetErrorStr(code));
        return FALSE;
    }
    strBase64->append(str.c_str());

    return TRUE;
}

// 1.32 获得相机固件版本信息，依照相机在设备上的序号
BOOL CDevImpl_CloudWalk::bGetFirmwareVersion(short deviceIndex, LPSTR lpFmVer)
{
    char *szFmVer = NULL;
    szFmVer = (char*)cwEngineGetFirmwareVersion(m_cwDetector, deviceIndex);
    if(szFmVer == NULL)
    {
        Log(ThisFile, 1, "通过序号获取固件版本: bGetFirmwareVersion()->cwEngineGetFirmwareVersion(%d) fail. ReturnCode: is null",
            deviceIndex);
        return FALSE;
    }

    lpFmVer = szFmVer;

    return TRUE;
}

// 1.33 获得相机固件版本信息，依照相机在设备VID/PID
BOOL CDevImpl_CloudWalk::bGetFirmwareVersionEx(LPSTR vid, LPSTR pid, LPSTR lpFmVer)
{
    char *szFmVer = NULL;
    szFmVer = (char*)cwEngineGetFirmwareVersionEx(m_cwDetector, vid, pid);
    if(szFmVer == NULL)
    {
        Log(ThisFile, 1, "通过VID/PID获取固件版本: bGetFirmwareVersionEx()->cwEngineGetFirmwareVersionEx(%s, %s) fail. ReturnCode: is null",
            vid, pid);
        return FALSE;
    }

    lpFmVer = szFmVer;
    return TRUE;
}

LPSTR CDevImpl_CloudWalk::GetErrorStr(LONG lErrCode)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(lErrCode)
    {
        case 75002000:
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "成功");
            break;
        case 75002001 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "输入参数错误");
            break;
        case 75002002 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "相机不存在");
            break;
        case 75002003 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "相机打开失败");
            break;
        case 75002004 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "相机关闭失败");
            break;
        case 75002005 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "非云从相机/设备不存在");
            break;
        case 75002006 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "读取芯片授权码失败");
            break;
        case 75002007 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "创建相机句柄失败");
            break;
        case 75002008 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "相机句柄为空");
            break;
        case 75002009 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "不支持的相机类型");
            break;
        case 75002010 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "参数指针为空");
            break;
        case 75002011 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "成像参数读取失败");
            break;
        case 75002012 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "成像参数设置失败");
            break;
        case 75002013 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "相机参数读取失败");
            break;
        case 75002014 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "相机参数设置失败");
            break;
        case 75002015 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "枚举相机设备失败");
            break;
        case 75002016 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "枚举相机分辨率失败");
            break;
        case 75002017 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "相机运行中");
            break;
        case 75002018 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "操作未知的错误");
            break;
        case 75002019 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "设置相机休眠失败");
            break;
        case 75002020 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "设置相机唤醒失败");
            break;
        case 75002021 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "相机设备断开");
            break;
        case 75000108 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "输入参数错误");
            break;
        case 75000109 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "无效的句柄");
            break;
        case 75000110 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "读取芯片授权码失败");
            break;
        case 75000111 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测器未初始化");
            break;
        case 75000112 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "未知错误catch");
            break;
        case 75000113 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "设置人脸检测回调函数错误");
            break;
        case 75000114 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "设置活体检测回调函数错误");
            break;
        case 75000115 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "设置日志信息回调函数错误");
            break;
        case 75000116 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "抠取人脸图像错误");
            break;
        case 75000117 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "创建com服务错误");
            break;
        case 75000118 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "重复创建检测器");
            break;
        case 75000119 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "base64解码错误");
            break;
        case 75000120 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到换脸");
            break;
        case 75000121 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "创建检测器错误");
            break;
        case 26121000 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "成功or合法");
            break;
        case 26121001 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "空图像");
            break;
        case 26121002 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "图像格式不支持");
            break;
        case 26121003 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "没有人脸");
            break;
        case 26121004 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "ROI设置失败");
            break;
        case 26121005 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "最小最大人脸设置失败");
            break;
        case 26121006 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "数据范围错误");
            break;
        case 26121007 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "未授权");
            break;
        case 26121008 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "尚未初始化");
            break;
        case 26121009 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "加载检测模型失败");
            break;
        case 26121010 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "加载关键点模型失败");
            break;
        case 26121011 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "加载质量评估模型失败");
            break;
        case 26121012 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "加载活体检测模型失败");
            break;
        case 26121013 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测失败");
            break;
        case 26121014 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "提取关键点失败");
            break;
        case 26121015 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "对齐人脸失败");
            break;
        case 26121016 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "质量评估失败");
            break;
        case 26121017 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "活体检测错误");
            break;
        case 26121018 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "时间戳不匹配");
            break;
        case 26121019 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "获取检测参数失败");
            break;
        case 26121020 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "设置检测参数失败");
            break;
        case 26121021 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "获取版本信息失败");
            break;
        case 26121022 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "删除文件或文件夹失败");
            break;
        case 26121030 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "人脸检测默认值");
            break;
        case 26121031 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "color图像通过检测");
            break;
        case 26121032 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "人脸距离太近");
            break;
        case 26121033 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "人脸距离太远");
            break;
        case 26121034 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "人脸角度不满足要求");
            break;
        case 26121035 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "人脸清晰度不满足要求");
            break;
        case 26121036 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到闭眼，仅在设置参数时eyeopen为true且检测到闭眼");
            break;
        case 26121037 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到张嘴，仅在设置参数时mouthopen为true且检测到多人时返回");
            break;
        case 26121038 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到人脸过亮，仅在设置参数时brightnessexc为true且人脸过亮时返回");
            break;
        case 26121039 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到人脸过暗，仅在设置参数时brightnessins为true且人脸过暗时返回");
            break;
        case 26121040 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到人脸置信度过低，仅在设置参数时confidence为true且人脸置信度过低时返回");
            break;
        case 26121041 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到人脸存在遮挡，仅在设置参数时occlusion为true且检测到遮挡时返回");
            break;
        case 26121042 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到黑框眼镜，仅在设置参数时blackspec为true且检测到黑框眼镜时返回");
            break;
        case 26121043 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "检测到墨镜，仅在设置参数时sunglass为true且检测到墨镜时返回");
            break;
        case 26121080 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "活体检测默认值");
            break;
        case 26121081 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "活体");
            break;
        case 26121082 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "非活体");
            break;
        case 26121083 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "人脸肤色检测未通过");
            break;
        case 26121084 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "可见光和红外人脸不匹配");
            break;
        case 26121085 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "红外输入没有人脸");
            break;
        case 26121086 :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "可见光输入没有人脸");
            break;
        default :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "未知错误");
            break;
    }

    return (LPSTR)m_szErrStr;
}

//---------------------------------------------
