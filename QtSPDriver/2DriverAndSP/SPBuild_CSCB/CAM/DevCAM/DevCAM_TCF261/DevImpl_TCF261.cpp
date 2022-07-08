#include "DevImpl_TCF261.h"

static const char *ThisFile = "DevImpl_TCF261.cpp";

CDevImpl_TCF261_Task cTask;

//-------------------------------------------------------------------

CDevImpl_TCF261::CDevImpl_TCF261()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_TCF261::CDevImpl_TCF261(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_TCF261::Init()
{
    QString strDllName(QString::fromLocal8Bit(DLL_TCF261_NAME));
    //strDllName.prepend("TCF261/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szDllPath, 0x00, sizeof(m_szDllPath));
    sprintf(m_szDllPath, "%s", strDllName.toStdString().c_str());
    m_qDLLLibrary.setFileName(m_szDllPath);
    m_bLoadIntfFail = TRUE;

    memset(m_szSDKToolPath, 0x00, sizeof(m_szSDKToolPath));
    sprintf(m_szSDKToolPath, "%sTCF261", LINUXPATHLIB);

    m_bIsTakePicExStop = FALSE;

    FR_CreateFaceCallBack = NULL;      // 1. 初始化SDK
    FR_DestroyFaceCallBack = NULL;     // 2. 反初始化/释放SDK
    FR_StartCamera = NULL;             // 3. 打开人脸设备
    FR_StopCamera = NULL;              // 4. 关闭人脸设备
    FR_PauseAndPlay = NULL;            // 5. 暂停预览画面
    FR_TakePicture = NULL;             // 6. 抓拍一张图片
    FR_StartLiveDetect = NULL;         // 7. 开始人脸活检抓拍
    FR_StopLiveDetect = NULL;          // 8. 停止人脸活检抓拍
    FR_CreateWindow = NULL;            // 9. 建立预览窗体
    FR_CloseWindow = NULL;             // 10. 关闭预览窗体
    FR_FaceCompare = NULL;             // 11. 比对两张人脸照片
    FR_GetStatus = NULL;               // 12. 人脸摄像头状态
    FR_GetFirmwareVersion = NULL;      // 13. 获取设备版本信息
    FR_HideTips = NULL;                // 14. 未知

    LoadDeviceDll();
}

CDevImpl_TCF261::~CDevImpl_TCF261()
{
    CloseDevice();
    UnLoadDeviceDll();
}


BOOL CDevImpl_TCF261::LoadDeviceDll()
{
    if (m_qDLLLibrary.isLoaded() != true)
    {
        if (m_qDLLLibrary.load() != true)
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szDllPath, m_qDLLLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }

    if (m_bLoadIntfFail)
    {
        if (LoadDeviceIntf() != TRUE)
        {
            Log(ThisFile, 1, "加载动态库接口<%s> fail. ReturnCode:%s.",
                m_szDllPath, m_qDLLLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }
    return TRUE;
}

void CDevImpl_TCF261::UnLoadDeviceDll()
{
    if (m_qDLLLibrary.isLoaded())
    {
        m_qDLLLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_TCF261::LoadDeviceIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1. 初始化SDK
    FR_CreateFaceCallBack = (FR_CREATEFACECALLBACK)m_qDLLLibrary.resolve("FR_CreateFaceCallBack");
    FUNC_POINTER_ERROR_RETURN(FR_CreateFaceCallBack, "FR_CreateFaceCallBack");

    // 2. 反初始化/释放SDK
    FR_DestroyFaceCallBack = (FR_DESTROYFACECALLBACK)m_qDLLLibrary.resolve("FR_DestroyFaceCallBack");
    FUNC_POINTER_ERROR_RETURN(FR_DestroyFaceCallBack, "FR_DestroyFaceCallBack");

    // 3. 打开人脸设备
    FR_StartCamera = (FR_STARTCAMERA)m_qDLLLibrary.resolve("FR_StartCamera");
    FUNC_POINTER_ERROR_RETURN(FR_StartCamera, "FR_StartCamera");

    // 4. 关闭人脸设备
    FR_StopCamera = (FR_STOPCAMERA)m_qDLLLibrary.resolve("FR_StopCamera");
    FUNC_POINTER_ERROR_RETURN(FR_StopCamera, "FR_StopCamera");

    // 5. 暂停预览画面
    FR_PauseAndPlay = (FR_PAUSEANDPLAY)m_qDLLLibrary.resolve("FR_PauseAndPlay");
    FUNC_POINTER_ERROR_RETURN(FR_PauseAndPlay, "FR_PauseAndPlay");

    // 6. 抓拍一张图片
    FR_TakePicture = (FR_TAKEPICTRUE)m_qDLLLibrary.resolve("FR_TakePicture");
    FUNC_POINTER_ERROR_RETURN(FR_TakePicture, "FR_TakePicture");

    // 7. 开始人脸活检抓拍
    FR_StartLiveDetect = (FR_STARTLIVEDETECT)m_qDLLLibrary.resolve("FR_StartLiveDetect");
    FUNC_POINTER_ERROR_RETURN(FR_StartLiveDetect, "FR_StartLiveDetect");

    // 8. 停止人脸活检抓拍
    FR_StopLiveDetect = (FR_STOPLIVEDETECT)m_qDLLLibrary.resolve("FR_StopLiveDetect");
    FUNC_POINTER_ERROR_RETURN(FR_StopLiveDetect, "FR_StopLiveDetect");

    // 9. 建立预览窗体
    FR_CreateWindow = (FR_CREATEWINDOW)m_qDLLLibrary.resolve("FR_CreateWindow");
    FUNC_POINTER_ERROR_RETURN(FR_CreateWindow, "FR_CreateWindow");

    // 10. 关闭预览窗体
    FR_CloseWindow = (FR_CLOSEWINDOW)m_qDLLLibrary.resolve("FR_CloseWindow");
    FUNC_POINTER_ERROR_RETURN(FR_CloseWindow, "FR_CloseWindow");

    // 11. 比对两张人脸照片
    FR_FaceCompare = (FR_FACECOMPARE)m_qDLLLibrary.resolve("FR_FaceCompare");
    FUNC_POINTER_ERROR_RETURN(FR_FaceCompare, "FR_FaceCompare");

    // 12. 人脸摄像头状态
    FR_GetStatus = (FR_GETSTATUS)m_qDLLLibrary.resolve("FR_GetStatus");
    FUNC_POINTER_ERROR_RETURN(FR_GetStatus, "FR_GetStatus");

    // 13. 获取设备版本信息
    FR_GetFirmwareVersion = (FR_GETFIRMWAREVERSION)m_qDLLLibrary.resolve("FR_GetFirmwareVersion");
    FUNC_POINTER_ERROR_RETURN(FR_GetFirmwareVersion, "FR_GetFirmwareVersion");

    // 14. 未知
    FR_HideTips = (FR_HIDETIPS)m_qDLLLibrary.resolve("FR_HideTips");
    FUNC_POINTER_ERROR_RETURN(FR_HideTips, "FR_HideTips");

    return TRUE;
}


BOOL CDevImpl_TCF261::OpenDevice()
{
    //如果已打开，则关闭
    if (m_bDevOpenOk == TRUE)
    {
        CloseDevice();
    }

    m_bDevOpenOk = FALSE;

    if (LoadDeviceDll() != TRUE)
    {
        Log(ThisFile, 1, "加载动态库: OpenDevice()->LoadDeviceDll() fail. ");
        return FALSE;
    }

    // 初始化SDK
    if (bCreateFaceCallBack(onDetectInfo, m_szSDKToolPath) != TRUE)
    {
        Log(ThisFile, 1, "初始化SDK: OpenDevice()->bCreateFaceCallBack() is fail. ");
        return FALSE;
    }
    Log(ThisFile, 1, "初始化SDK: OpenDevice()->bCreateFaceCallBack() is succ. ");

    // 打开人脸设备
    if (bStartCamera() != TRUE)
    {
        Log(ThisFile, 1, "打开人脸设备: OpenDevice()->bStartCamera() is fail. ");
        return FALSE;
    }
    Log(ThisFile, 1, "打开人脸设备: OpenDevice()->bStartCamera() is succ. ");

    m_bDevOpenOk = TRUE;

    return TRUE;
}

BOOL CDevImpl_TCF261::CloseDevice()
{
    if(m_bDevOpenOk == TRUE)
    {
        bStopCamera();
        //bDestroyFaceCallBack();
    }

    UnLoadDeviceDll();
    m_bDevOpenOk = FALSE;

    Log(ThisFile, 1, "设备关闭,释放动态库: Close Device, unLoad TCF261 Dll.");

    return TRUE;
}

BOOL CDevImpl_TCF261::IsDeviceOpen()
{
    //return (m_bDevOpenOk == TRUE && m_cwDetector != NULL ? TRUE : FALSE);
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}


void CDevImpl_TCF261::SetTakePicStop(BOOL bFlag)
{
    m_bIsTakePicExStop = bFlag;
}

BOOL CDevImpl_TCF261::GetTakePicStop()
{
    return m_bIsTakePicExStop;
}

// 1. 初始化SDK
BOOL CDevImpl_TCF261::bCreateFaceCallBack(CaptureCallBackDetectInfo pDetectInfo, LPCSTR lpParentDir)
{
    int nRet = 0;

    nRet = FR_CreateFaceCallBack(pDetectInfo, lpParentDir);
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "初始化SDK: bCreateFaceCallBack()->FR_CreateFaceCallBack(%s) fail. ReturnCode:%s",
            lpParentDir, GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 2. 反初始化/释放SDK
BOOL CDevImpl_TCF261::bDestroyFaceCallBack()
{
    int nRet = 0;

    nRet = FR_DestroyFaceCallBack();
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "反初始化/释放SDK: bDestroyFaceCallBack()->FR_DestroyFaceCallBack() fail. ReturnCode:%s",
            GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 3. 打开人脸设备
BOOL CDevImpl_TCF261::bStartCamera()
{
    int nRet = 0;

    nRet = FR_StartCamera();
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "打开人脸设备: bStartCamera()->FR_StartCamera() fail. ReturnCode:%s",
            GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 4. 关闭人脸设备
BOOL CDevImpl_TCF261::bStopCamera()
{
    int nRet = 0;

    nRet = FR_StopCamera();
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "关闭人脸设备: bStopCamera()->FR_StopCamera() fail. ReturnCode:%s",
            GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 5. 暂停预览画面
BOOL CDevImpl_TCF261::bPauseAndPlaya(bool bIsPause)
{
    int nRet = 0;

    nRet = FR_PauseAndPlay(bIsPause);
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "暂停预览画面: bPauseAndPlaya()->FR_PauseAndPlay(%d|%s) fail. ReturnCode:%s",
            bIsPause, bIsPause == true ? "暂停" : "恢复", GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 6. 抓拍一张图片
BOOL CDevImpl_TCF261::bTakePicture(LPCSTR lpcFilePath)
{
    int nRet = 0;

    nRet = FR_TakePicture(lpcFilePath);
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "抓拍一张图片: bTakePicture()->FR_TakePicture(%s) fail. ReturnCode:%s",
            lpcFilePath, GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 7. 开始人脸活检抓拍
// 参数: mode，true全景照片，false，只有活体头部的头像照片
//      strContent，保留
//      strFilePath，传入图片文件url地址，图片格式支持bmp，jpeg，base64编码的jpeg
BOOL CDevImpl_TCF261::bStartLiveDetecte(BOOL bMode, LPCSTR lpcContent, LPCSTR lpcFilePath)
{
    int nRet = 0;

    nRet = FR_StartLiveDetect(bMode, lpcContent, lpcFilePath);
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "开始人脸活检抓拍: bStartLiveDetect()->FR_StartLiveDetect(%d, %s, %s) fail. ReturnCode:%s",
            bMode, lpcContent, lpcFilePath, GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 8. 停止人脸活检抓拍
BOOL CDevImpl_TCF261::bStopLiveDetecte()
{
    int nRet = 0;

    nRet = FR_StopLiveDetect();
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "停止人脸活检抓拍: bStopLiveDetect()->FR_StopLiveDetect() fail. ReturnCode:%s",
            GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 9. 建立预览窗体
BOOL CDevImpl_TCF261::bCreateWindow(ULONG ulWndHandle, INT nX, INT nY, INT nWidth, INT nHeight)
{
    int nRet = 0;

    nRet = FR_CreateWindow(ulWndHandle, nX, nY, nWidth, nHeight);
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "建立预览窗体: bCreateWindow(%08X, %d, %d, %d, %d)->FR_CreateWindow() fail. ReturnCode:%s",
            ulWndHandle, nX, nY, nWidth, nHeight, GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 10. 关闭预览窗体
BOOL CDevImpl_TCF261::bCloseWindow()
{
    int nRet = 0;

    nRet = FR_CloseWindow();
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "关闭预览窗体: bCloseWindow()->FR_CloseWindow() fail. ReturnCode:%s",
            GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

// 11. 比对两张人脸照片
BOOL CDevImpl_TCF261::bFaceCompare(LPCSTR lpcPicA, LPCSTR lpcPicB, USHORT usScoreIn, USHORT *usScoreOut)
{
    int nRet = 0;
    USHORT usScoreTmp = 0;

    nRet = FR_FaceCompare(lpcPicA, lpcPicB, usScoreIn, usScoreTmp);
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "比对两张人脸照片: bFaceCompare()->FR_FaceCompare(%s, %s, %d, %d) fail. ReturnCode:%s",
            lpcPicA, lpcPicB, usScoreIn, usScoreTmp, GetErrorStr(nRet));
        return FALSE;
    }

    *usScoreOut = usScoreTmp;

    return TRUE;
}

// 12. 人脸摄像头状态
BOOL CDevImpl_TCF261::bGetStatus(INT *nStatOut)
{
    FR_GetStatus(nStatOut);
    /*if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "人脸摄像头状态: bGetStatus()->FR_GetStatus(%s, %s, %d, %d) fail. ReturnCode:%s",
            lpcPicA, lpcPicB, usScoreIn, *usScoreOut, GetErrorStr(nRet));
        return FALSE;
    }*/

    return TRUE;
}

// 13. 获取设备版本信息
BOOL CDevImpl_TCF261::bGetFirmwareVersion(LPSTR lpVer, WORD wSize)
{
    int nRet = 0;
    CHAR    szVer[64] = { 0x00 };

    nRet = FR_GetFirmwareVersion(szVer);
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            "获取设备版本信息: bGetFirmwareVersion()->FR_GetFirmwareVersion() fail. ReturnCode:%s",
            GetErrorStr(nRet));
        return FALSE;
    }

    memcpy(lpVer, szVer, (wSize < 64 ? wSize : 64));

    return TRUE;
}

// 14. 未知
BOOL CDevImpl_TCF261::bHideTips(bool bIsHide)
{
    int nRet = 0;

    nRet = FR_HideTips(bIsHide);
    if(nRet != FR_RET_SUCC)
    {
        Log(ThisFile, 1,
            ": bHideTips()->FR_HideTips(%d) fail. ReturnCode:%s",
            bIsHide, GetErrorStr(nRet));
        return FALSE;
    }

    return TRUE;
}

LPSTR CDevImpl_TCF261::GetErrorStr(INT nErrCode)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(nErrCode)
    {
        case FR_RET_SUCC: /* 0   */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "执行成功");
            break;
        case FR_RET_FAIL: /* -1  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "执行失败");
            break;
        case FR_RET_PARA: /* -2  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "参数错误");
            break;
        case FR_RET_NDEV: /* -3  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "打开设备失败");
            break;
        case FR_RET_NINI: /* -4  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "未打开设备");
            break;
        case FR_RET_BUSY: /* -5  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设备繁忙");
            break;
        case FR_RET_DATA: /* -6  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "图像数据不正确");
            break;
        case FR_RET_NLNK: /* -7  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设备断开");
            break;
        case FR_RET_NDRV: /* -8  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "加载设备库失败");
            break;
        case FR_RET_NALG: /* -9  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "加载算法库失败");
            break;
        case FR_RET_NMEM: /* -10 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "内存分配失败");
            break;
        case FR_RET_ARDY: /* -11 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "已经打开设备");
            break;
        case FR_RET_AUTH: /* -12 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "算法授权失败");
            break;
        case FP_RET_IPCF: /* -13 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "IPC错误");
            break;
        case FR_CALLBACK_RESULT_SUCC     : /* 100 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "活体检测成功");
            break;
        case FR_CALLBACK_RESULT_FAIL     : /* 101 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "活体检测失败");
            break;
        case FR_CALLBACK_RESULT_TIMEOUT  : /* 102 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "活体检测超时");
            break;
        case FR_CALLBACK_RESULT_ERROR    : /* 103 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "从设备取图失败");
            break;
        case FR_CALLBACK_RESULT_CANCEL   : /* 104 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "活体检测取消");
            break;
        case FR_CALLBACK_RESULT_NOFACE   : /* 105 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "没有检测到人脸");
            break;
        case FR_CALLBACK_RESULT_MULTIFADE: /* 106 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "检测到多个人脸");
            break;
        case FR_CALLBACK_RESULT_HEADPOS  : /* 107 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "头部姿态不正常");
            break;
        case FR_CALLBACK_RESULT_EMOTION  : /* 108 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "有闭眼张嘴等表情动作");
            break;
        case FR_CALLBACK_RESULT_MOTIVE   : /* 109 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "有运动模糊");
            break;
        case FR_CALLBACK_RESULT_BRIGHT   : /* 110 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "亮度不符合");
            break;
        case FR_CALLBACK_RESULT_NOTCENTER: /* 111 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "人脸未居中");
            break;
        case FR_CALLBACK_RESULT_UNKNOW   : /* 112 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "未知错误");
            break;
        default :
            sprintf(m_szErrStr, "%d|%s", nErrCode, "未知错误");
            break;
    }

    return (LPSTR)m_szErrStr;
}

// 回调函数(活检抓拍后返回)
int onDetectInfo(int *nResultID, void *lpParam)
{
    if (*nResultID == FR_CALLBACK_RESULT_SUCC)
    {
        cTask.cDevTCF261->SetTakePicStop(TRUE);
    }

    return 1;
}

//---------------------------------------------
