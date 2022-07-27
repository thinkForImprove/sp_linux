/***************************************************************************
* 文件名称: DevImpl_CloudWalk.h
* 文件描述: 封装摄像模块底层指令,提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
***************************************************************************/

#include "DevImpl_CloudWalk.h"

static const CHAR *ThisFile = "DevImpl_CloudWalk.cpp";

CDevImpl_CloudWalk::CDevImpl_CloudWalk()
{
    SetLogFile(LOG_NAME, ThisFile);                 // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CloudWalk::CDevImpl_CloudWalk(LPSTR lpLog)
{
    SetLogFile(LOG_NAME, ThisFile);                 // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CloudWalk::CDevImpl_CloudWalk(LPSTR lpLog, LPCSTR lpDevType)
{
    SetLogFile(lpLog, ThisFile, lpDevType);         // 设置日志文件名和错误发生的文件
    MSET_0(m_szDevType);
    MCPY_NOLEN(m_szDevType, strlen(lpDevType) > 0 ? lpDevType : "YC-0C98");
    Init();
}

// 参数初始化
void CDevImpl_CloudWalk::Init()
{
    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    //strDllName.prepend("CAM/CloudWalk/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_bLoadIntfFail = TRUE;

    // 变量初始化
    MSET_0(m_szDevType);                                    // 设备类型
    m_cwDetector = nullptr;                                 // 设备句柄
    m_bDevOpenOk = FALSE;                                   // 设备Open标记
    m_wSDKVersion = 0;                                      // SDK版本(SP内定义)
    m_bReCon = FALSE;                                       // 是否断线重连状态
    MSET_0(m_szErrStr);                                     // 错误码解析
    memset(m_nRetErrOLD, 0, sizeof(INT) * 12);              // 处理错误值保存

    // 动态库接口函数初始化
    vInitLibFunc();
}

CDevImpl_CloudWalk::~CDevImpl_CloudWalk()
{
    CloseDevice();
    vUnLoadLibrary();
}

BOOL CDevImpl_CloudWalk::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库<%s> fail. ReturnCode:%s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail. ReturnCode:%s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_CloudWalk::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
        vInitLibFunc(); // 动态库接口函数初始化
    }
}

BOOL CDevImpl_CloudWalk::bLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1.1 创建检测对象
    LOAD_LIBINFO_FUNC(pcwEngineCreateDetector, cwEngineCreateDetector, "cwEngineCreateDetector");

    // 1.2 释放检测对象及COM服务
    LOAD_LIBINFO_FUNC(pcwEngineReleaseDetector, cwEngineReleaseDetector, "cwEngineReleaseDetector");

    // 1.3 获取COM服务及算法版本号
    LOAD_LIBINFO_FUNC(pcwEnginGetVersion, cwEnginGetVersion, "cwEngineGetVersion");

    // 1.4 注册相机回调回显函数
    LOAD_LIBINFO_FUNC(pcwEngineRegisterPreviewHandle, cwEngineRegisterPreviewHandle, "cwEngineRegisterPreviewHandle");

    // 1.5 注册检测回调函数对象
    LOAD_LIBINFO_FUNC(pcwEngineRegisterDetectHandle, cwEngineRegisterDetectHandle, "cwEngineRegisterDetectHandle");

    // 1.6 加载检测模型及设置检测参数
    LOAD_LIBINFO_FUNC(pcwEngineInitDetectorParam, cwEngineInitDetectorParam, "cwEngineInitDetectorParam");

    // 1.7 是否进行人脸追踪显示
    LOAD_LIBINFO_FUNC(pcwEngineEnableFaceTrace, cwEngineEnableFaceTrace, "cwEngineEnableFaceTrace");

    // 1.9 依据相机在设备显示的序号开启相机
    LOAD_LIBINFO_FUNC(pcwEngineOpenCamera, cwEngineOpenCamera, "cwEngineOpenCamera");

    // 1.10 依据相机硬件信息开启相机
    LOAD_LIBINFO_FUNC(pcwEngineOpenCameraEx, cwEngineOpenCameraEx, "cwEngineOpenCameraEx");

    // 1.11 关闭当前程序打开的所有相机设备
    LOAD_LIBINFO_FUNC(pcwEngineCloseAllCameras, cwEngineCloseAllCameras, "cwEngineCloseAllCameras");

    // 1.12 枚举当前设备上的所有相机	// 30-00-00-00(FT#0031)
    LOAD_LIBINFO_FUNC(pcwEngineEnumCameras, cwEngineEnumCameras, "cwEngineEnumCameras");

    // 1.13 枚举相机支持的所有分辨率(序号)
    LOAD_LIBINFO_FUNC(pcwEngineEnumResolutions, cwEngineEnumResolutions, "cwEngineEnumResolutions");

    // 1.14 枚举相机支持的所有分辨率(VIDPID)
    LOAD_LIBINFO_FUNC(pcwEngineEnumResolutionsEx, cwEngineEnumResolutionsEx, "cwEngineEnumResolutionsEx");

    // 1.15 开启活体检测
    LOAD_LIBINFO_FUNC(pcwEngineStartLiveDetect, cwEngineStartLiveDetect, "cwEngineStartLiveDetect");

    // 1.16 停止连续活体检测
    LOAD_LIBINFO_FUNC(pcwEngineStopLiveDetect, cwEngineStopLiveDetect, "cwEngineStopLiveDetect");

    // 1.17 抓拍当前画面帧
    LOAD_LIBINFO_FUNC(pcwEngineCaptureCurrentFrame, cwEngineCaptureCurrentFrame, "cwEngineCaptureCurrentFrame");

    // 1.18 保存当前帧至指定文件路径参数
    LOAD_LIBINFO_FUNC(pcwEngineSaveCurrentFrame, cwEngineSaveCurrentFrame, "cwEngineSaveCurrentFrame");

    // 1.19 开启语音提示功能	// 30-00-00-00(FT#0031)
    LOAD_LIBINFO_FUNC(pcwEngineEnableVoiceTip, cwEngineEnableVoiceTip, "cwEngineEnableVoiceTip");

    if (m_wSDKVersion == 1)
    {
        // 1.20获取本次活体检测最佳人脸图像
        LOAD_LIBINFO_FUNC(pcwEngineGetBestFace1, cwEngineGetBestFace1, "cwEngineGetBestFace");

        // 1.21 保存活体最佳人脸到指定路径
        LOAD_LIBINFO_FUNC(pcwEngineSaveBestFace1, cwEngineSaveBestFace1, "cwEngineSaveBestFace");
    } else
    {
        // 1.20获取本次活体检测最佳人脸图像
        LOAD_LIBINFO_FUNC(pcwEngineGetBestFace, cwEngineGetBestFace, "cwEngineGetBestFace");

        // 1.21 保存活体最佳人脸到指定路径
        LOAD_LIBINFO_FUNC(pcwEngineSaveBestFace, cwEngineSaveBestFace, "cwEngineSaveBestFace");
    }

    // 1.22 通过序号得到相机状态
    LOAD_LIBINFO_FUNC(pcwEngineGetCameraStatus, cwEngineGetCameraStatus, "cwEngineGetCameraStatus");

    // 1.23 通过相机硬件信息VID/PID得到相机状态
    LOAD_LIBINFO_FUNC(pcwEngineGetCameraStatusEx, cwEngineGetCameraStatusEx, "cwEngineGetCameraStatusEx");

    // 1.26 将图片转换成base64编码格式
    LOAD_LIBINFO_FUNC(pcwEngineImage2Base64Format, cwEngineImage2Base64Format, "cwEngineImage2Base64Format");

    // 1.28 依照相机在设备上的序号休眠相机
    LOAD_LIBINFO_FUNC(pcwEngineSleepCamera, cwEngineSleepCamera, "cwEngineSleepCamera");

    // 1.29 依照相机硬件信息VID/PID休眠相机
    LOAD_LIBINFO_FUNC(pcwEngineSleepCameraEx, cwEngineSleepCameraEx, "cwEngineSleepCameraEx");

    // 1.32 获得相机固件版本信息，依照相机在设备上的序号
    LOAD_LIBINFO_FUNC(pcwEngineGetFirmwareVersion, cwEngineGetFirmwareVersion, "cwEngineGetFirmwareVersion");

    // 1.33 获得相机固件版本信息，依照相机在设备VID/PID
    LOAD_LIBINFO_FUNC(pcwEngineGetFirmwareVersionEx, cwEngineGetFirmwareVersionEx, "cwEngineGetFirmwareVersionEx");

    return TRUE;
}

void CDevImpl_CloudWalk::vInitLibFunc()
{
    // 动态库接口函数初始化
    cwEngineCreateDetector = nullptr;           // 1.1 创建检测对象
    cwEngineReleaseDetector = nullptr;          // 1.2 释放检测对象及COM服务
    cwEnginGetVersion = nullptr;                // 1.3 获取COM服务及算法版本号
    cwEngineRegisterPreviewHandle = nullptr;    // 1.4 注册相机回调回显函数
    cwEngineRegisterDetectHandle = nullptr;     // 1.5 注册检测回调函数对象
    cwEngineInitDetectorParam = nullptr;        // 1.6 加载检测模型及设置检测参数
    cwEngineEnableFaceTrace = nullptr;          // 1.7 是否进行人脸追踪显示
    cwEngineOpenCamera = nullptr;               // 1.9 依据相机在设备显示的序号开启相机
    cwEngineOpenCameraEx = nullptr;             // 1.10 依据相机硬件信息开启相机
    cwEngineCloseAllCameras = nullptr;          // 1.11 关闭当前程序打开的所有相机设备
    cwEngineEnumCameras = nullptr;              // 1.12 枚举当前设备上的所有相机
    cwEngineEnumResolutions = nullptr;          // 1.13 枚举相机支持的所有分辨率(序号)
    cwEngineEnumResolutionsEx = nullptr;        // 1.14 枚举相机支持的所有分辨率(VIDPID)
    cwEngineStartLiveDetect = nullptr;          // 1.15 开启活体检测
    cwEngineStopLiveDetect = nullptr;           // 1.16 停止连续活体检测
    cwEngineCaptureCurrentFrame = nullptr;      // 1.17 抓拍当前画面帧
    cwEngineSaveCurrentFrame = nullptr;         // 1.18 保存当前帧至指定文件路径参数
    cwEngineEnableVoiceTip = nullptr;           // 1.19 开启语音提示功能
    cwEngineGetBestFace = nullptr;              // 1.20 获取本次活体检测最佳人脸图像
    cwEngineSaveBestFace = nullptr;             // 1.21 保存活体最佳人脸到指定路径
    cwEngineGetBestFace1 = nullptr;             // 1.20 获取本次活体检测最佳人脸图像
    cwEngineSaveBestFace1 = nullptr;            // 1.21 保存活体最佳人脸到指定路径
    cwEngineGetCameraStatus = nullptr;          // 1.22 通过序号得到相机状态
    cwEngineGetCameraStatusEx = nullptr;        // 1.23 通过相机硬件信息VID/PID得到相机状态
    cwEngineImage2Base64Format = nullptr;       // 1.26 将图片转换成base64编码格式
    cwEngineSleepCamera = nullptr;              // 1.28 依照相机在设备上的序号休眠相机
    cwEngineSleepCameraEx = nullptr;            // 1.29 依照相机硬件信息VID/PID 休眠相机
    cwEngineGetFirmwareVersion = nullptr;       // 1.32 获得相机固件版本信息，依照相机在设备上的序号
    cwEngineGetFirmwareVersionEx = nullptr;     // 1.33 获得相机固件版本信息，依照相机在设备VID/PID
}

//-------------------------------------------------------------------------
// 打开设备(加载检测对象)
INT CDevImpl_CloudWalk::OpenDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 如果已打开，则关闭
    if (m_bDevOpenOk == TRUE)
    {
        CloseDevice();
    }

    m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                    ConvertCode_Impl2Str(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    //-----------------------打开设备-----------------------
    // 1. 设备句柄非NULL, 先释放
    if (m_cwDetector != nullptr)
    {
        // 释放检测对象及COM服务
        cwEngineReleaseDetector(m_cwDetector);
        m_cwDetector = nullptr;
    }

    // 2. 创建检测对象
    m_cwDetector = cwEngineCreateDetector();
    if (m_cwDetector == nullptr)
    {
        Log(ThisModule, __LINE__, "打开设备: 创建检测对象: ->cwEngineCreateDetector() fail, Return: %s");
        return IMP_ERR_NOTOPEN;
    }
    Log(ThisModule, __LINE__, "打开设备: 创建检测对象: ->cwEngineCreateDetector() succ. ");

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }
    m_bReCon = FALSE; // 是否断线重连状态: 初始F

    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

// 打开设备
INT CDevImpl_CloudWalk::OpenDevice(STDETECTINITPAR stInitPar)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 如果已打开，则关闭
    if (m_bDevOpenOk == TRUE)
    {
        CloseDevice();
    }

    m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                    ConvertCode_Impl2Str(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    //-----------------------打开设备-----------------------
    // 1. 设备句柄非NULL, 先释放
    if (m_cwDetector != nullptr)
    {
        // 释放检测对象及COM服务
        cwEngineReleaseDetector(m_cwDetector);
        m_cwDetector = nullptr;
    }

    // 2. 创建检测对象
    m_cwDetector = cwEngineCreateDetector();
    if (m_cwDetector == nullptr)
    {
        Log(ThisModule, __LINE__,
            "打开设备: 创建检测对象: ->cwEngineCreateDetector() fail, Return: %s");
        return IMP_ERR_NOTOPEN;
    }

    // 初始化检测器(加载算法模型)
    nRet = InitDetectorParam(stInitPar.nModelMode, stInitPar.nLivenessMode,
                             stInitPar.nLicenseType, stInitPar.szConfigFile,
                             stInitPar.szFaceDetectFile, stInitPar.szKeyPointDetectFile,
                             stInitPar.szFaceKeyPointTrackFile, stInitPar.szFaceQualityFile,
                             stInitPar.szFaceLivenessFile, stInitPar.nGpu,
                             stInitPar.nMultiThread);
    if (nRet != IMP_SUCCESS)
    {
        return nRet;
    }

    // 绘制人脸框及显示回调函数
    nRet = RegisterPreviewHandle(this);
    if (nRet != IMP_SUCCESS)
    {
        return nRet;
    }

    // 活体检测及回调设置
    nRet = RegisterDetectHandle(this);
    if (nRet != IMP_SUCCESS)
    {
        return nRet;
    }

    m_bDevOpenOk = TRUE;

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }
    m_bReCon = FALSE; // 是否断线重连状态: 初始F

    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

// 关闭设备
INT CDevImpl_CloudWalk::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 释放检查对象(设备句柄)
    if (m_cwDetector != nullptr)
    {
        cwEngineStopLiveDetect(m_cwDetector);// 1.2 释放检测对象及COM服务
        cwEngineReleaseDetector(m_cwDetector);// 1.16 停止连续活体检测
        Log(ThisModule, __LINE__, "关闭设备: 停止连续活体检测: ->cwEngineStopLiveDetect().");
        Log(ThisModule, __LINE__, "关闭设备: 释放检测对象: ->cwEngineReleaseDetector().");
        m_cwDetector = nullptr;
    }

    // 非断线重连时关闭需释放动态库
    if (m_bReCon == FALSE)
    {
        // 释放动态库
        vUnLoadLibrary();
        Log(ThisModule, __LINE__, "设备关闭: 释放动态库: Close Device(), unLoad Library.");
    }

    // 设备Open标记=F
    m_bDevOpenOk = FALSE;

    return IMP_SUCCESS;
}

// 设备是否Open成功
BOOL CDevImpl_CloudWalk::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 获取活检成功标记
BOOL CDevImpl_CloudWalk::GetIsLiveSucc()
{
    return m_bLiveDetectSucc;
}

// 获取活检错误码
INT CDevImpl_CloudWalk::GetLiveErrCode()
{
    return m_nLiveErrCode;
}

// 获取视频流数据
INT CDevImpl_CloudWalk::GetViewData(cv::Mat &cvMatView)
{
    cvMatView = m_cvMatView.clone();
    return IMP_SUCCESS;
}

// 获取视频流数据
INT CDevImpl_CloudWalk::GetVideoData(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, DWORD dwParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    Mat cvMatImg_RGB;
    INT nBuffSize = 0;
    BOOL bIsChange = FALSE;

    m_GetDataRunning = TRUE;

    if (m_cvMatView.empty())
    {
        m_GetDataRunning = FALSE;
        return IMP_ERR_UNKNOWN;
    }

    cvMatImg_RGB = m_cvMatView.clone();

    // 图像数据转换
    if (nWidth > 0 && nHeight > 0)
    {
        cv::resize(cvMatImg_RGB, cvMatImg_RGB, cv::Size(nWidth, nHeight));
        bIsChange = TRUE;
    }

    if (cvMatImg_RGB.cols != m_stImageData.nWidth ||
        cvMatImg_RGB.rows != m_stImageData.nHeight ||
        cvMatImg_RGB.channels() != m_stImageData.nFormat)
    {
        m_stImageData.Clear();

        nBuffSize = cvMatImg_RGB.cols * cvMatImg_RGB.rows * cvMatImg_RGB.channels() + 1 + 5;

        m_stImageData.ucImgData = (UCHAR*)malloc(sizeof(UCHAR) * nBuffSize);
        if (m_stImageData.ucImgData == nullptr)
        {
            m_GetDataRunning = FALSE;
            return IMP_ERR_UNKNOWN;
        }
        m_stImageData.nWidth = cvMatImg_RGB.cols;
        m_stImageData.nHeight = cvMatImg_RGB.rows;
        m_stImageData.nFormat = cvMatImg_RGB.channels();
        memset(m_stImageData.ucImgData, 0x00, nBuffSize);
    } else
    {
        nBuffSize = m_stImageData.ulImagDataLen + 1;
    }

    sprintf((CHAR*)m_stImageData.ucImgData, "%c%c%c%c%c",
            m_stImageData.nWidth / 255, m_stImageData.nWidth % 255,
            m_stImageData.nHeight / 255, m_stImageData.nHeight % 255,
            m_stImageData.nFormat);
    m_stImageData.ulImagDataLen = nBuffSize - 1;
    MCPY_LEN(m_stImageData.ucImgData + 5, cvMatImg_RGB.data, nBuffSize - 1 - 5);

    lpImgData->nWidth = m_stImageData.nWidth;
    lpImgData->nHeight = m_stImageData.nHeight;
    lpImgData->nFormat = m_stImageData.nFormat;
    lpImgData->ulImagDataLen = m_stImageData.ulImagDataLen;
    lpImgData->ucImgData = m_stImageData.ucImgData;

    lpImgData->nOtherParam[0] = 0;
    if (bIsChange == TRUE)
    {
        lpImgData->nOtherParam[0] = 1;
        lpImgData->nOtherParam[1] = cvMatImg_RGB.cols;
        lpImgData->nOtherParam[2] = cvMatImg_RGB.rows;
    }

    m_GetDataRunning = FALSE;

    return IMP_SUCCESS;
}

//-------------------------------------------------------------------------
//---------------------------- 封装动态库接口 -------------------------------
//-------------------------------------------------------------------------
// 1.3 获取COM服务及算法版本号
INT CDevImpl_CloudWalk::GetVersion(LPSTR lpVer, long lSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR *szVerTmp = nullptr;

    // COM版本
    szVerTmp = cwEnginGetVersion(m_cwDetector, 0);
    if (szVerTmp != nullptr)
    {
        memcpy(lpVer, szVerTmp, strlen(szVerTmp) > lSize ? lSize : strlen(szVerTmp));
    } else
    {
        Log(ThisModule, __LINE__,
            "获取COM版本: ->cwEnginGetVersion(%d, %d) fail", m_cwDetector, 0);
    }

    // 中间版本
    szVerTmp = nullptr;
    szVerTmp = cwEnginGetVersion(m_cwDetector, 1);
    if (szVerTmp != nullptr)
    {
        if (strlen(lpVer) < lSize && strlen(szVerTmp) > 0 && strlen(lpVer) > 0)
        {
           sprintf(lpVer + strlen(lpVer), "%s", " ; ");
           memcpy(lpVer + strlen(lpVer), szVerTmp,
                  strlen(szVerTmp) > (lSize - strlen(lpVer)) ? (lSize - strlen(lpVer)) : strlen(szVerTmp));
        }
    } else
    {
        Log(ThisModule, __LINE__,
            "获取中间版本函数: ->cwEnginGetVersion(%d, %d) fail", m_cwDetector, 1);
    }

    // 算法版本
    szVerTmp = nullptr;
    szVerTmp = cwEnginGetVersion(m_cwDetector, 2);
    if (szVerTmp != nullptr)
    {
        if (strlen(lpVer) < lSize && strlen(szVerTmp) > 0 && strlen(lpVer) > 0)
        {
           sprintf(lpVer + strlen(lpVer), "%s", " ; ");
           memcpy(lpVer + strlen(lpVer), szVerTmp,
                  strlen(szVerTmp) > (lSize - strlen(lpVer)) ? (lSize - strlen(lpVer)) : strlen(szVerTmp));
        }
    } else
    {
        Log(ThisModule, __LINE__,
            "获取算法版本: ->cwEnginGetVersion(%d, %d) fail", m_cwDetector, 1);
    }

    return IMP_SUCCESS;
}

//  1.4 注册相机回调回显函数(绘制人脸框及显示回调函数)
INT CDevImpl_CloudWalk::RegisterPreviewHandle(HANDLE hWnd)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    cw_preview_observer ober;
    ober.handle = hWnd;
    ober.func = onPreviewCallback;
    lRet =  cwEngineRegisterPreviewHandle(m_cwDetector, &ober);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "注册相机回调函数: ->cwEngineRegisterPreviewHandle() fail, Return: %s",
            ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }
    return IMP_SUCCESS;
}

INT CDevImpl_CloudWalk::RegisterPreviewHandle(HANDLE hWnd, preview_func fuPreView)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    cw_preview_observer ober;
    ober.handle = hWnd;
    ober.func = fuPreView;
    lRet =  cwEngineRegisterPreviewHandle(m_cwDetector, &ober);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "注册相机回调函数: ->cwEngineRegisterPreviewHandle() fail, Return: %s",
            ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }
    return IMP_SUCCESS;
}

// 1.5 注册检测回调函数对象(活体检测及回调设置)
INT CDevImpl_CloudWalk::RegisterDetectHandle(HANDLE hWnd)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    cw_detect_observer ober;
    ober.handle = hWnd;
    ober.face_func = onFaceDetCallback;
    ober.live_func = onLiveDetCallback;
    lRet = cwEngineRegisterDetectHandle(m_cwDetector, &ober);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "注册检测回调函数: ->cwEngineRegisterDetectHandle() fail, Return: %s", ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.5 注册检测回调函数对象(活体检测及回调设置)
INT CDevImpl_CloudWalk::RegisterDetectHandle(HANDLE hWnd,
                                              face_det_func fuFaceDet,
                                              live_det_func fuLiveDet)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    cw_detect_observer ober;
    ober.handle = hWnd;
    ober.face_func = onFaceDetCallback;
    ober.live_func = fuLiveDet;

    lRet = cwEngineRegisterDetectHandle(m_cwDetector, &ober);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "注册检测回调函数: ->cwEngineRegisterDetectHandle() fail, Return: %s", ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.6 加载检测模型及设置检测参数,初始化检测器(加载算法模型)
INT CDevImpl_CloudWalk::InitDetectorParam(short modelMode,                 // 模型加载方式(0/文件加载,1/内存加载)
                                           short livenessMode,              // 活体检测方式(0/单目活体,1/红外活体,2/结构光后置,3TOF光)
                                           short licenseType,               // 授权类型(1/云从相机芯片授权,2/云从hasp授权,3/云从cw临时授权码授权,4/云从绑定授权码授权)
                                           CHAR* configFile,                // 活体配置文件
                                           CHAR* faceDetectFile,            // 人脸检测模型
                                           CHAR* faceKeyPointDetectFile,    // 关键点检测模型
                                           CHAR* faceKeyPointTrackFile,     // 关键点跟踪模型
                                           CHAR* faceQualityFile,           // 人脸质量评估模型
                                           CHAR* faceLivenessFile,          // 活体模型
                                           short gpu,                       // 是否使用GPU(1/使用GPU,1使用CPU)
                                           short multiThread)               // 是否使用多线程(1/多线程,0/单线程)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineInitDetectorParam(m_cwDetector, (short)modelMode, (short)livenessMode,
                                     (short)licenseType, configFile, faceDetectFile,
                                     faceKeyPointDetectFile, faceKeyPointTrackFile,
                                     faceQualityFile, faceLivenessFile,
                                     gpu, (short)multiThread);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000 && lRet != IMP_ERR_DEV_75000118)
    {
        CHAR* p = ConvertCode_Impl2Str(lRet);
        Log(ThisModule, __LINE__,
            "加载检测模型: ->cwEngineInitDetectorParam(%d, %d ,%d ,%s ,%s ,%s ,%s ,%s ,%s ,%d ,%d) fail, Return: %s",
            (short)modelMode, (short)livenessMode, (short)licenseType, configFile, faceDetectFile,
            faceKeyPointDetectFile, faceKeyPointTrackFile, faceQualityFile, faceLivenessFile,
            gpu, (short)multiThread, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.7 是否进行人脸追踪显示
INT CDevImpl_CloudWalk::EnableFaceTrace(BOOL bEnable)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineEnableFaceTrace(m_cwDetector, bEnable);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "设置人脸追踪显示: ->cwEngineEnableFaceTrace(%d) fail, Return: %s",
            bEnable, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.9 依据相机在设备显示的序号开启相机
INT CDevImpl_CloudWalk::OpenCamera(SHORT deviceIndex, // 相机序号,相机在设备显示的序号
                                    SHORT deviceMode,  // 相机类型(0/可见图像,1/红外图像,2/深度图像,3/TOF图像,4/欧菲舜宇结构光后置,5/华捷艾米结构光后置,6/奥比中光结构光后置)
                                    SHORT width,       // 请求图像的宽度
                                    SHORT height)      // 请求图像的高度
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineOpenCamera(m_cwDetector, deviceIndex, deviceMode, width, height);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "按序号开启相机: ->cwEngineOpenCamera(%d, %d, %d, %d) fail, Return: %s",
            deviceIndex, deviceMode, width, height, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.10 依据相机硬件信息开启相机(VID/PID)
INT CDevImpl_CloudWalk::OpenCameraEx(LPSTR vid,         // 相机VID
                                       LPSTR pid,         // 相机PID
                                       SHORT deviceMode,  // 相机类型(0/可见图像,1/红外图像,2/深度图像,3/TOF图像,4/欧菲舜宇结构光后置,5/华捷艾米结构光后置,6/奥比中光结构光后置)
                                       SHORT width,       // 请求图像的宽度
                                       SHORT height)      // 请求图像的高度
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineOpenCameraEx(m_cwDetector, vid, pid, deviceMode, width, height);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "按VID/PID开启相机: ->cwEngineOpenCameraEx(%s, %s, %d, %d, %d) fail, Return: %s",
            vid, pid, deviceMode, width, height, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.11 关闭当前程序打开的所有相机设备
INT CDevImpl_CloudWalk::CloseAllCameras()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineCloseAllCameras(m_cwDetector);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "关闭所有相机设备: ->cwEngineCloseAllCameras() fail, Return: %s",
            ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.12 枚举当前设备上的所有相机
INT CDevImpl_CloudWalk::EnumCameras(CWCameraDevice **cameraLists)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineEnumCameras(m_cwDetector, cameraLists);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "枚举当前设备上的所有相机: ->cwEngineEnumCameras() fail, Return: %s",
            ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.13 枚举相机支持的所有分辨率(序号)
INT CDevImpl_CloudWalk::EnumResolutions(SHORT deviceIndex, CWResolution** resolutionLists)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineEnumResolutions(m_cwDetector, deviceIndex, resolutionLists);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "枚举相机支持的所有分辨率(序号): ->cwEngineEnumResolutions() fail, Return: %s",
            ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.14 枚举相机支持的所有分辨率(VIDPID)
INT CDevImpl_CloudWalk::EnumResolutionsEx(LPSTR vid, LPSTR pid, CWResolution** resolutionLists)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineEnumResolutionsEx(m_cwDetector, vid, pid, resolutionLists);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "枚举相机支持的所有分辨率(VIDPID): ->cwEngineEnumResolutionsEx() fail, Return: %s",
            ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.15 开启活体检测
INT CDevImpl_CloudWalk::StartLiveDetect(BOOL isContinue)  // 是否连续检测(TRUE连续,FALSE单次)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineStartLiveDetect(m_cwDetector, isContinue);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "开启活体检测: ->cwEngineStartLiveDetect(%d) fail, Return: %s",
            isContinue, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }
    m_bLiveDetectSucc = FALSE;  // 活检成功标记:F

    return IMP_SUCCESS;
}

//  1.16 停止连续活体检测
INT CDevImpl_CloudWalk::StopLiveDetect()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineStopLiveDetect(m_cwDetector);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "停止连续活体检测: ->cwEngineStopLiveDetect() fail, Return: %s", ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.17 抓拍当前画面帧
INT CDevImpl_CloudWalk::CaptureCurrentFrame(CWSize size, BOOL isBase64, CWLiveImage* image)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineCaptureCurrentFrame(m_cwDetector, size, isBase64, image);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "抓拍当前画面帧: ->cwEngineCaptureCurrentFrame(%d, %d,%d) fail, Return: %s",
            size.width, size.height, isBase64, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}
// 1.18 保存当前帧至指定文件路径参数
INT CDevImpl_CloudWalk::SaveCurrentFrame(LPSTR fileName)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineSaveCurrentFrame(m_cwDetector, fileName);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "保存当前帧至指定文件路径参数: b->cwEngineSaveCurrentFrame(%s) fail, Return: %s",
            fileName, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.19 开启语音提示功能
INT CDevImpl_CloudWalk::EnableVoiceTip(BOOL bEnable, LPCSTR lpTipFile)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineEnableVoiceTip(m_cwDetector, bEnable, lpTipFile);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "开启语音提示功能: ->cwEngineEnableVoiceTip(%d, %s) fail, Return: %s",
            bEnable, lpTipFile, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.20 获取本次活体检测最佳人脸图像
INT CDevImpl_CloudWalk::GetBestFace(float ratio, BOOL isBase64, CWLiveImage* image, INT nType)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    if (m_wSDKVersion == 1)
    {
        lRet = cwEngineGetBestFace1(m_cwDetector, ratio, isBase64, nType, image);
    } else
    {
        lRet = cwEngineGetBestFace(m_cwDetector, ratio, isBase64, image);
    }

    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "获取活体检测最佳人脸图像: ->cwEngineGetBestFace(%f, %d, %d) fail, Return: %s",
            ratio, isBase64, nType, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.21 保存活体最佳人脸到指定路径
INT CDevImpl_CloudWalk::SaveBestFace(LPSTR fileName, float ratio)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    /*LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineSaveBestFace(m_cwDetector, fileName, ratio);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__, "存活体最佳人脸: bSaveBestFace()->cwEngineSaveBestFace(%s, %d) fail, Return: %s",
            fileName, ratio, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }*/

    return IMP_SUCCESS;
}

// 1.22 通过序号得到相机状态
INT CDevImpl_CloudWalk::GetCameraStatus(short index, CWCameraState* status)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineGetCameraStatus(m_cwDetector, (const short)index, status);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "按序号获得相机状态: ->cwEngineGetCameraStatus(%d) fail, Return: %s",
            index, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.23 通过相机硬件信息VID/PID得到相机状态
INT CDevImpl_CloudWalk::GetCameraStatusEx(LPSTR vid, LPSTR pid, CWCameraState* state)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineGetCameraStatusEx(m_cwDetector, (const CHAR*)vid, (const CHAR*)pid, state);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "按VID/PID获得相机状态: ->cwEngineGetCameraStatusEx(%s, %s) fail, Return: %s",
            vid, pid, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.26 将图片转换成base64编码格式
INT CDevImpl_CloudWalk::Image2Base64Format(LPSTR imageFilePath,   // 图像全路径
                                             std::string* strBase64)
                                            //LPSTR strBase64)       // base64编码格式字符串
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    std::string str;
    lRet = cwEngineImage2Base64Format(m_cwDetector, imageFilePath, &str);
    if (lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "图片转成base64: ->cwEngineImage2Base64Format(%s, %s) fail, Return: %s",
            imageFilePath, strBase64->c_str(), ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }
    strBase64->append(str.c_str());

    return IMP_SUCCESS;
}

// 1.28 依照相机在设备上的序号休眠相机
INT CDevImpl_CloudWalk::SleepCamera(SHORT index, BOOL bIsSleep)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineSleepCamera(m_cwDetector, index, bIsSleep);
    if(lRet != CW_LIVENESS_SUCCESSFUL)
    {
        Log(ThisModule, __LINE__,
            "依照相机在设备上的序号休眠相机: ->cwEngineSleepCamera(%d, %d) fail, Return: %s",
            index, bIsSleep, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.29 通过相机硬件信息VID/PID休眠相机
INT CDevImpl_CloudWalk::SleepCameraEx(LPSTR vid, LPSTR pid, BOOL bIsSleep)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LONG lRet = IMP_ERR_DEV_SUCCESS;

    lRet = cwEngineSleepCameraEx(m_cwDetector, (LPCSTR)vid, (LPCSTR)pid, bIsSleep);
    if(lRet != IMP_ERR_DEV_SUCCESS && lRet != IMP_ERR_DEV_26121000)
    {
        Log(ThisModule, __LINE__,
            "通过相机硬件信息VID/PID休眠相机: ->cwEngineSleepCameraEx(%s, %s, %d) fail, Return: %s",
            vid, pid, bIsSleep, ConvertCode_Impl2Str(lRet));
        return (INT)lRet;
    }

    return IMP_SUCCESS;
}

// 1.32 获得相机固件版本信息，依照相机在设备上的序号
INT CDevImpl_CloudWalk::GetFirmwareVersion(short deviceIndex, LPSTR lpFmVer)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR *szFmVer = nullptr;
    szFmVer = (CHAR*)cwEngineGetFirmwareVersion((HANDLE)m_cwDetector, deviceIndex);
    if(szFmVer == nullptr)
    {
        Log(ThisModule, __LINE__,
            "通过序号获取固件版本: ->cwEngineGetFirmwareVersion(%d) fail, Return: %s",
            deviceIndex);
        return IMP_ERR_INTEREXEC;
    }

    lpFmVer = szFmVer;

    return IMP_SUCCESS;
}

// 1.33 获得相机固件版本信息，依照相机在设备VID/PID
INT CDevImpl_CloudWalk::GetFirmwareVersionEx(LPSTR vid, LPSTR pid, LPSTR lpFmVer)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR *szFmVer = nullptr;
    szFmVer = (CHAR*)cwEngineGetFirmwareVersionEx(m_cwDetector, vid, pid);
    if(szFmVer == nullptr)
    {
        Log(ThisModule, __LINE__,
            "通过VID/PID获取固件版本: ->cwEngineGetFirmwareVersionEx(%s, %s) fail, Return: %s",
            vid, pid);
        return IMP_ERR_INTEREXEC;
    }

    lpFmVer = szFmVer;

    return IMP_SUCCESS;
}

// 当前SDK是否支持红外图片生成
BOOL CDevImpl_CloudWalk::GetNirImgSup()
{
    if (m_wSDKVersion == 1)
    {
        return TRUE;
    }

    return FALSE;
}

// 错误码解析
LPSTR CDevImpl_CloudWalk::ConvertCode_Impl2Str(LONG lErrCode)
{
#define YC0C98_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%ld|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    if (CHK_ERR_ISDEF(lErrCode) == TRUE)
    {
        DEF_ConvertCode_Impl2Str(lErrCode, m_szErrStr);
    } else
    {
        switch(lErrCode)
        {
            // 设备返回错误码
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_SUCCESS, lErrCode, "成功");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002001, lErrCode, "输入参数错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002002, lErrCode, "相机不存在");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002003, lErrCode, "相机打开失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002004, lErrCode, "相机关闭失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002005, lErrCode, "非云从相机");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002006, lErrCode, "读取芯片授权码失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002007, lErrCode, "创建相机句柄失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002008, lErrCode, "相机句柄为空");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002009, lErrCode, "不支持的相机类型");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002010, lErrCode, "参数指针为空");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002011, lErrCode, "成像参数读取失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002012, lErrCode, "成像参数设置失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002013, lErrCode, "相机参数读取失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002014, lErrCode, "相机参数设置失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002015, lErrCode, "枚举相机设备失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002016, lErrCode, "枚举相机分辨率失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002017, lErrCode, "相机运行中");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002018, lErrCode, "操作未知的错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002019, lErrCode, "设置相机休眠失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002020, lErrCode, "设置相机唤醒失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75002021, lErrCode, "相机设备断开");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000108, lErrCode, "输入参数错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000109, lErrCode, "无效的句柄");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000110, lErrCode, "读取芯片授权码失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000111, lErrCode, "检测器未初始化");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000112, lErrCode, "未知错误catch");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000113, lErrCode, "设置人脸检测回调函数错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000114, lErrCode, "设置活体检测回调函数错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000115, lErrCode, "设置日志信息回调函数错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000116, lErrCode, "抠取人脸图像错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000117, lErrCode, "创建com服务错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000118, lErrCode, "重复创建检测器");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000119, lErrCode, "base64解码错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000120, lErrCode, "检测到换脸");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000121, lErrCode, "创建检测器错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_75000123, lErrCode, "检测到多张人脸");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121000, lErrCode, "成功or合法");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121001, lErrCode, "空图像");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121002, lErrCode, "图像格式不支持");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121003, lErrCode, "没有人脸");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121004, lErrCode, "ROI设置失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121005, lErrCode, "最小最大人脸设置失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121006, lErrCode, "数据范围错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121007, lErrCode, "未授权");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121008, lErrCode, "尚未初始化");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121009, lErrCode, "加载检测模型失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121010, lErrCode, "加载关键点模型失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121011, lErrCode, "加载质量评估模型失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121012, lErrCode, "加载活体检测模型失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121013, lErrCode, "检测失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121014, lErrCode, "提取关键点失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121015, lErrCode, "对齐人脸失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121016, lErrCode, "质量评估失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121017, lErrCode, "活体检测错误");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121018, lErrCode, "时间戳不匹配");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121019, lErrCode, "获取检测参数失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121020, lErrCode, "设置检测参数失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121021, lErrCode, "获取版本信息失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121022, lErrCode, "删除文件或文件夹失败");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121030, lErrCode, "人脸检测默认值");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121031, lErrCode, "color图像通过检测");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121032, lErrCode, "人脸距离太近");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121033, lErrCode, "人脸距离太远");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121034, lErrCode, "人脸角度不满足要求");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121035, lErrCode, "人脸清晰度不满足要求");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121036, lErrCode, "检测到闭眼，仅在设置参数时eyeopen为true且检测到闭眼");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121037, lErrCode, "检测到张嘴，仅在设置参数时mouthopen为true且检测到多人时返回");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121038, lErrCode, "检测到人脸过亮，仅在设置参数时brightnessexc为true且人脸过亮时返回");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121039, lErrCode, "检测到人脸过暗，仅在设置参数时brightnessins为true且人脸过暗时返回");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121040, lErrCode, "检测到人脸置信度过低，仅在设置参数时confidence为true且人脸置信度过低时返回");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121041, lErrCode, "检测到人脸存在遮挡，仅在设置参数时occlusion为true且检测到遮挡时返回");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121042, lErrCode, "检测到黑框眼镜，仅在设置参数时blackspec为true且检测到黑框眼镜时返回");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121043, lErrCode, "检测到墨镜，仅在设置参数时sunglass为true且检测到墨镜时返回");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121044, lErrCode, "检测到口罩,仅在设置参数时proceduremask>-1且检测到口罩时返回");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121080, lErrCode, "活体检测默认值");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121081, lErrCode, "活体");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121082, lErrCode, "非活体");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121083, lErrCode, "人脸肤色检测未通过");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121084, lErrCode, "可见光和红外人脸不匹配");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121085, lErrCode, "红外输入没有人脸");
            YC0C98_CASE_CODE_STR(IMP_ERR_DEV_26121086, lErrCode, "可见光输入没有人脸");
            default :
                sprintf(m_szErrStr,  "%d|%s", lErrCode, "未知Code");
                break;
        }
    }



    return (LPSTR)m_szErrStr;
}

//----------------------------------对外参数设置接口----------------------------------
// 设置断线重连标记
INT CDevImpl_CloudWalk::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

// 设置动态库路径(DeviceOpen前有效)
INT CDevImpl_CloudWalk::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径

    return IMP_SUCCESS;
}

// 设置SDK版本
void CDevImpl_CloudWalk::SetSDKVersion(WORD wVer)
{
    if (m_wSDKVersion == 0 && wVer != 0)
    {
        Log(ThisFile, __LINE__, "Set SDK Version: %d->%d", m_wSDKVersion, wVer);
        m_wSDKVersion = wVer;
    }
}

//----------------------------------视频数据处理----------------------------------
// 视频流显示回调，已绘制好人脸框
void onPreviewCallback(void* handle, const short mode, const char* frame,
                       const short width, const short height, const short channel)
{
    if(nullptr == handle || 1 == mode)
    {
        return;
    }

    auto Ptr = CvtPointer(handle, CDevImpl_CloudWalk);
    if(nullptr == Ptr) {return ;}
    Ptr->preview((const uchar*)frame, width, height, channel);
}

// 人脸检测回调
void onFaceDetCallback(void* handle, const long error_code, const long timestamp,
                       const CWLiveFaceDetectInformation* infos, const short face_num)
{
    if(nullptr == handle)
    {
        return ;
    }

    auto Ptr = CvtPointer(handle, CDevImpl_CloudWalk);
    if(nullptr == Ptr)
    {
       return ;
    }
    Ptr->showFace(error_code, timestamp, infos, face_num);
}

// 活体检测回调
void onLiveDetCallback(void* handle, const long error_code, const float* scores,
                       const float* distances, const short face_num)
{
    if(nullptr == handle)
    {
        return ;
    }

    auto Ptr = CvtPointer(handle, CDevImpl_CloudWalk);
    if(nullptr == Ptr){
        return ;
    }

    Ptr->showLive(error_code, scores, distances, face_num);
}

// 视频流显示回调
void CDevImpl_CloudWalk::preview(const uchar* data, int width, int height, int channels, int type)
{
    AutoMutex(m_cMutex);

    if (m_GetDataRunning == TRUE)
    {
        return;
    }
    // 视频流回调暂停

    cv::Mat mat(height, width, CV_8UC3, (char*)data);
    cv::cvtColor(mat, mat, CV_BGR2RGB);
    m_cvMatView = mat.clone();
}

//
void CDevImpl_CloudWalk::showFace(const long error_code, const long timestamp, const CWLiveFaceDetectInformation_t* pFaceInformations, int nFaceNumber)
{

}

// 活体检测结果获取
void CDevImpl_CloudWalk::showLive(int errCode, const float* scores, const float* distances, int nFaceNumber)
{
    // 已检测到活体, 不再处理
    if (m_bLiveDetectSucc == TRUE)
    {
        return;
    }

    // 检测到活体
    if (errCode == IMP_ERR_DEV_26121081)
    {
        m_bLiveDetectSucc = TRUE;
    } else
    {
        m_nLiveErrCode = errCode;   // 活检错误码保存
    }
}

// -------------------------------------- END --------------------------------------
