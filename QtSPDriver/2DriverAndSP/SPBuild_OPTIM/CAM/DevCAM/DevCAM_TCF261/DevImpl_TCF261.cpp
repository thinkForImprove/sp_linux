/***************************************************************************
* 文件名称: DevImpl_TCF261.h
* 文件描述: 封装摄像模块底层指令,提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
***************************************************************************/

#include "DevImpl_TCF261.h"
#include "file_access.h"

static const char *ThisFile = "DevImpl_TCF261.cpp";

CDevImpl_TCF261_Task cTask;

CDevImpl_TCF261::CDevImpl_TCF261()
{
    SetLogFile(LOG_NAME, ThisFile);                 // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_TCF261::CDevImpl_TCF261(LPSTR lpLog)
{
    SetLogFile(LOG_NAME, ThisFile);                 // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_TCF261::CDevImpl_TCF261(LPSTR lpLog, LPCSTR lpDevType)
{
    SetLogFile(lpLog, ThisFile, lpDevType);         // 设置日志文件名和错误发生的文件
    MSET_0(m_szDevType);
    MCPY_NOLEN(m_szDevType, strlen(lpDevType) > 0 ? lpDevType : "YC-0C98");
    Init();
}

// 参数初始化
void CDevImpl_TCF261::Init()
{
    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("CAM/TCF261/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_bLoadIntfFail = TRUE;

    // 变量初始化
    MSET_0(m_szDevType);                                        // 设备类型
    m_bDevOpenOk = FALSE;                                       // 设备Open标记
    MSET_0(m_szSDKToolPath);                                    // SDK工具目录
    m_bReCon = FALSE;                                           // 是否断线重连状态
    m_bLiveDetectSucc = FALSE;                                  // 活检成功标记
    m_nLiveErrCode = FR_CALLBACK_RESULT_NOFACE;                 // 活体检测错误码保存
    MSET_0(m_szErrStr);                                         // IMPL错误码解析
    MSET_0(m_szErrStrTCF);                                      // TCF错误码解析
    memset(m_nRetErrOLD, 0, sizeof(INT) * 12);                  // 处理错误值保存

    SetSDKToolPath(m_szLoadDllPath);

    // 动态库接口函数初始化
    vInitLibFunc();
}

CDevImpl_TCF261::~CDevImpl_TCF261()
{
    CloseDevice();
    vUnLoadLibrary();
}



BOOL CDevImpl_TCF261::bLoadLibrary()
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
                Log(ThisModule, __LINE__, "加载动态库<%s> fail, ErrCode: %s, Return: %s.",
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
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail, ErrCode: %s, Return: %s.",
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

void CDevImpl_TCF261::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
        vInitLibFunc(); // 动态库接口函数初始化
    }
}

BOOL CDevImpl_TCF261::bLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1. 初始化SDK
    LOAD_LIBINFO_FUNC(FR_CREATEFACECALLBACK, FR_CreateFaceCallBack, "FR_CreateFaceCallBack");

    // 2. 反初始化/释放SDK
    LOAD_LIBINFO_FUNC(FR_DESTROYFACECALLBACK, FR_DestroyFaceCallBack, "FR_DestroyFaceCallBack");

    // 3. 打开人脸设备
    LOAD_LIBINFO_FUNC(FR_STARTCAMERA, FR_StartCamera, "FR_StartCamera");

    // 4. 关闭人脸设备
    LOAD_LIBINFO_FUNC(FR_STOPCAMERA, FR_StopCamera, "FR_StopCamera");

    // 5. 暂停预览画面
    LOAD_LIBINFO_FUNC(FR_PAUSEANDPLAY, FR_PauseAndPlay, "FR_PauseAndPlay");

    // 6. 抓拍一张图片
    LOAD_LIBINFO_FUNC(FR_TAKEPICTRUE, FR_TakePicture, "FR_TakePicture");

    // 7. 开始人脸活检抓拍
    LOAD_LIBINFO_FUNC(FR_STARTLIVEDETECT, FR_StartLiveDetect, "FR_StartLiveDetect");

    // 8. 停止人脸活检抓拍
    LOAD_LIBINFO_FUNC(FR_STOPLIVEDETECT, FR_StopLiveDetect, "FR_StopLiveDetect");

    // 9. 建立预览窗体
    LOAD_LIBINFO_FUNC(FR_CREATEWINDOW, FR_CreateWindow, "FR_CreateWindow");

    // 10. 关闭预览窗体
    LOAD_LIBINFO_FUNC(FR_CLOSEWINDOW, FR_CloseWindow, "FR_CloseWindow");

    // 11. 比对两张人脸照片
    LOAD_LIBINFO_FUNC(FR_FACECOMPARE, FR_FaceCompare, "FR_FaceCompare");

    // 12. 人脸摄像头状态
    LOAD_LIBINFO_FUNC(FR_GETSTATUS, FR_GetStatus, "FR_GetStatus");

    // 13. 获取设备版本信息
    LOAD_LIBINFO_FUNC(FR_GETFIRMWAREVERSION, FR_GetFirmwareVersion, "FR_GetFirmwareVersion");

    // 14. 未知
    LOAD_LIBINFO_FUNC(FR_HIDETIPS, FR_HideTips, "FR_HideTips");

    return TRUE;
}

void CDevImpl_TCF261::vInitLibFunc()
{
    // 动态库接口函数初始化
    FR_CreateFaceCallBack = nullptr;      // 1. 初始化SDK
    FR_DestroyFaceCallBack = nullptr;     // 2. 反初始化/释放SDK
    FR_StartCamera = nullptr;             // 3. 打开人脸设备
    FR_StopCamera = nullptr;              // 4. 关闭人脸设备
    FR_PauseAndPlay = nullptr;            // 5. 暂停预览画面
    FR_TakePicture = nullptr;             // 6. 抓拍一张图片
    FR_StartLiveDetect = nullptr;         // 7. 开始人脸活检抓拍
    FR_StopLiveDetect = nullptr;          // 8. 停止人脸活检抓拍
    FR_CreateWindow = nullptr;            // 9. 建立预览窗体
    FR_CloseWindow = nullptr;             // 10. 关闭预览窗体
    FR_FaceCompare = nullptr;             // 11. 比对两张人脸照片
    FR_GetStatus = nullptr;               // 12. 人脸摄像头状态
    FR_GetFirmwareVersion = nullptr;      // 13. 获取设备版本信息
    FR_HideTips = nullptr;                // 14. 未知
}

//-------------------------------------------------------------------------
// 打开设备
INT CDevImpl_TCF261::OpenDevice()
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
    // 初始化SDK
    nRet = FR_CreateFaceCallBack(onDetectInfo, m_szSDKToolPath);
    if (nRet != FR_RET_SUCC)
    {
        if (m_nRetErrOLD[1] != nRet)
        {
            Log(ThisModule, __LINE__,
                "打开设备: 初始化SDK: ->FR_CreateFaceCallBack() fail, ErrCode: %s, Return: %s.",
                ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
            m_nRetErrOLD[1] = nRet;
        }
        return ConvertCode_TCF2Impl(nRet);
    }

    // 打开人脸设备
    nRet = FR_StartCamera();
    if (nRet != FR_RET_SUCC)
    {
        if (m_nRetErrOLD[2] != nRet)
        {
            Log(ThisModule, __LINE__,
                "打开设备: 打开人脸设备: ->FR_StartCamera() fail, ErrCode: %s, Return: %s.",
                ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
            m_nRetErrOLD[2] = nRet;
        }
        return ConvertCode_TCF2Impl(nRet);
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
INT CDevImpl_TCF261::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if(m_bDevOpenOk == TRUE)
    {
        FR_StopCamera();
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
BOOL CDevImpl_TCF261::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 获取活检成功标记
BOOL CDevImpl_TCF261::GetIsLiveSucc()
{
    return m_bLiveDetectSucc;
}

// 获取活检错误码
INT CDevImpl_TCF261::GetLiveErrCode()
{
    return m_nLiveErrCode;
}

// 设置活检成功标记
void CDevImpl_TCF261::SetLiveSuccFlag(BOOL bFlag)
{
    m_bLiveDetectSucc = bFlag;
}

// 设置活检错误码
void CDevImpl_TCF261::SetLiveErrCode(INT nCode)
{
    m_nLiveErrCode = nCode;
}

// 设置SDK工具路径
INT CDevImpl_TCF261::SetSDKToolPath(LPSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (lpPath != nullptr)
    {
        CHAR szFileTmp[4][256] = { 0x00 };
        FileDir::_splitpath(lpPath, szFileTmp[0], szFileTmp[1], szFileTmp[2], szFileTmp[3]);
        MSET_0(m_szSDKToolPath);
        sprintf(m_szSDKToolPath, "%s", szFileTmp[1]);
        Log(ThisModule, __LINE__, "设置SDK工具路径: %s", m_szSDKToolPath);
    } else
    {
        Log(ThisModule, __LINE__, "入参为NULL, 不设置SDK工具路径, 当前路径: %s", m_szSDKToolPath);
    }

    return IMP_SUCCESS;
}

// Impl错误码解析
LPSTR CDevImpl_TCF261::ConvertCode_Impl2Str(INT nErrCode)
{
#define IMPL_TCF_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    if (CHK_ERR_ISDEF(nErrCode) == TRUE)
    {
        DEF_ConvertCode_Impl2Str(nErrCode, m_szErrStr);
    } else
    {
        switch(nErrCode)
        {
            // 设备返回错误码
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_SUCCESS, nErrCode, "执行成功");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_RUNFAIL, nErrCode, "执行失败");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_PARAM, nErrCode, "参数错误");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_OPENFAIL, nErrCode, "打开设备失败");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_NOTOPEN, nErrCode, "未打开设备");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_BUSY, nErrCode, "设备繁忙");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_DATA, nErrCode, "图像数据不正确");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_OFFLINE, nErrCode, "设备断开");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_LOADDLL, nErrCode, "加载设备库失败");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_LOADALGO, nErrCode, "加载算法库失败");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_MEMORY, nErrCode, "内存分配失败");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_ISOPEN, nErrCode, "已经打开设备");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_AUTH, nErrCode, "算法授权失败");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_IPC, nErrCode, "IPC错误");
            default :
                sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
                break;
        }
    }

    return (LPSTR)m_szErrStr;
}

// TCF设备错误码解析
LPSTR CDevImpl_TCF261::ConvertCode_TCF2Str(INT nErrCode)
{
#define TCF_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStrTCF, "%d|%s", CODE, STR); \
        return m_szErrStrTCF;

    memset(m_szErrStrTCF, 0x00, sizeof(m_szErrStrTCF));

    switch(nErrCode)
    {
        // 设备返回错误码
        TCF_CASE_CODE_STR(FR_RET_SUCC, nErrCode, "执行成功");
        TCF_CASE_CODE_STR(FR_RET_FAIL, nErrCode, "执行失败");
        TCF_CASE_CODE_STR(FR_RET_PARA, nErrCode, "参数错误");
        TCF_CASE_CODE_STR(FR_RET_NDEV, nErrCode, "打开设备失败");
        TCF_CASE_CODE_STR(FR_RET_NINI, nErrCode, "未打开设备");
        TCF_CASE_CODE_STR(FR_RET_BUSY, nErrCode, "设备繁忙");
        TCF_CASE_CODE_STR(FR_RET_DATA, nErrCode, "图像数据不正确");
        TCF_CASE_CODE_STR(FR_RET_NLNK, nErrCode, "设备断开");
        TCF_CASE_CODE_STR(FR_RET_NDRV, nErrCode, "加载设备库失败");
        TCF_CASE_CODE_STR(FR_RET_NALG, nErrCode, "加载算法库失败");
        TCF_CASE_CODE_STR(FR_RET_NMEM, nErrCode, "内存分配失败");
        TCF_CASE_CODE_STR(FR_RET_ARDY, nErrCode, "已经打开设备");
        TCF_CASE_CODE_STR(FR_RET_AUTH, nErrCode, "算法授权失败");
        TCF_CASE_CODE_STR(FP_RET_IPCF, nErrCode, "IPC错误");
        default :
            sprintf(m_szErrStrTCF,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStrTCF;
}
// TCF错误码转Impl错误码
INT CDevImpl_TCF261::ConvertCode_TCF2Impl(INT nErrCode)
{
    switch(nErrCode)
    {
        // 设备返回错误码
        case FR_RET_SUCC:   // 执行成功
        case FR_RET_FAIL:   // 执行失败
        case FR_RET_PARA:   // 参数错误
        case FR_RET_NDEV:   // 打开设备失败
        case FR_RET_NINI:   // 未打开设备
        case FR_RET_BUSY:   // 设备繁忙
        case FR_RET_DATA:   // 图像数据不正确
        case FR_RET_NLNK:   // 设备断开
        case FR_RET_NDRV:   // 加载设备库失败
        case FR_RET_NALG:   // 加载算法库失败
        case FR_RET_NMEM:   // 内存分配失败
        case FR_RET_ARDY:   // 已经打开设备
        case FR_RET_AUTH:   // 算法授权失败
        case FP_RET_IPCF:   // IPC错误
            return nErrCode - IMP_ERR_DEF_NUM;
        default :
            return IMP_ERR_UNKNOWN;
    }
}

//-------------------------------------------------------------------------
//--------------------------- 对外参数设置接口 -------------------------------
//-------------------------------------------------------------------------
// 设置断线重连标记
INT CDevImpl_TCF261::SetReConFlag(BOOL bFlag)
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
INT CDevImpl_TCF261::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, __LINE__, "入参[%s]无效,不设置动态库路径.");
        return IMP_SUCCESS;
    }

    if (lpPath[0] == '/')   // 绝对路径
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s", lpPath);
    } else
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s%s", LINUXPATHLIB, lpPath);
    }

    Log(ThisModule, __LINE__, "设置动态库路径=<%s>.", m_szLoadDllPath);

    SetSDKToolPath(m_szLoadDllPath);

    return IMP_SUCCESS;
}

//-------------------------------------------------------------------------
//---------------------------- 封装动态库接口 -------------------------------
//-------------------------------------------------------------------------
// 1. 初始化SDK
INT CDevImpl_TCF261::CreateFaceCallBack(CaptureCallBackDetectInfo pDetectInfo, LPCSTR lpParentDir)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_CreateFaceCallBack(pDetectInfo, lpParentDir);
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "初始化SDK: ->FR_CreateFaceCallBack(%s) fail, ErrCode: %s, Return: %s",
            lpParentDir, ConvertCode_TCF2Str(nRet),
            ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 2. 反初始化/释放SDK
INT CDevImpl_TCF261::DestroyFaceCallBack()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_DestroyFaceCallBack();
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "反初始化/释放SDK: ->FR_DestroyFaceCallBack() fail, ErrCode: %s, Return: %s",
            ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 3. 打开人脸设备
INT CDevImpl_TCF261::StartCamera()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_StartCamera();
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "打开人脸设备: ->FR_StartCamera() fail, ErrCode: %s, Return: %s",
            ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 4. 关闭人脸设备
INT CDevImpl_TCF261::StopCamera()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_StopCamera();
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "关闭人脸设备: ->FR_StopCamera() fail, ErrCode: %s, Return: %s",
            ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 5. 暂停预览画面
INT CDevImpl_TCF261::PauseAndPlaya(BOOL bIsPause)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_PauseAndPlay(bIsPause);
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "暂停预览画面: ->FR_PauseAndPlay(%d|%s) fail, ErrCode: %s, Return: %s",
            bIsPause, bIsPause == TRUE ? "暂停" : "恢复",
            ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 6. 抓拍一张图片
INT CDevImpl_TCF261::TakePicture(LPCSTR lpcFilePath)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_TakePicture(lpcFilePath);
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "抓拍一张图片: ->FR_TakePicture(%s) fail, ErrCode: %s, Return: %s",
            lpcFilePath, ConvertCode_TCF2Str(nRet),
            ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 7. 开始人脸活检抓拍
// 参数: mode，true全景照片，false，只有活体头部的头像照片
//      strContent，保留
//      strFilePath，传入图片文件url地址，图片格式支持bmp，jpeg，base64编码的jpeg
INT CDevImpl_TCF261::StartLiveDetecte(BOOL bMode, LPCSTR lpcContent, LPCSTR lpcFilePath)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    m_bLiveDetectSucc = FALSE;
    m_nLiveErrCode = FR_CALLBACK_RESULT_UNKNOW;

    nRet = FR_StartLiveDetect(bMode, lpcContent, lpcFilePath);
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "开始人脸活检抓拍: ->FR_StartLiveDetect(%d, %s, %s) fail, ErrCode: %s, Return: %s",
            bMode, lpcContent, lpcFilePath, ConvertCode_TCF2Str(nRet),
            ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 8. 停止人脸活检抓拍
INT CDevImpl_TCF261::StopLiveDetecte()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_StopLiveDetect();
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "停止人脸活检抓拍: ->FR_StopLiveDetect() fail, ErrCode: %s, Return: %s",
            ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 9. 建立预览窗体
INT CDevImpl_TCF261::CreateWindow(ULONG ulWndHandle, INT nX, INT nY, INT nWidth, INT nHeight)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_CreateWindow(ulWndHandle, nX, nY, nWidth, nHeight);
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "建立预览窗体: ->FR_CreateWindow(%08X, %d, %d, %d, %d) fail, ErrCode: %s, Return: %s",
            ulWndHandle, nX, nY, nWidth, nHeight, ConvertCode_TCF2Str(nRet),
            ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 10. 关闭预览窗体
INT CDevImpl_TCF261::CloseWindow()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_CloseWindow();
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "关闭预览窗体: ->FR_CloseWindow() fail, ErrCode: %s, Return: %s",
            ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 11. 比对两张人脸照片
INT CDevImpl_TCF261::FaceCompare(LPCSTR lpcPicA, LPCSTR lpcPicB, USHORT usScoreIn, USHORT *usScoreOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;
    USHORT usScoreTmp = 0;

    nRet = FR_FaceCompare(lpcPicA, lpcPicB, usScoreIn, usScoreTmp);
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "比对两张人脸照片: FR_FaceCompare(%s, %s, %d, %d) fail, ErrCode: %s, Return: %s",
            lpcPicA, lpcPicB, usScoreIn, usScoreTmp, ConvertCode_TCF2Str(nRet),
            ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    *usScoreOut = usScoreTmp;

    return IMP_SUCCESS;
}

// 12. 人脸摄像头状态
INT CDevImpl_TCF261::GetStatus(BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    FR_GetStatus(&nRet);

    return nRet;
}

// 13. 获取设备版本信息
INT CDevImpl_TCF261::GetFirmwareVersion(LPSTR lpVer, WORD wSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    CHAR    szVer[64] = { 0x00 };

    nRet = FR_GetFirmwareVersion(szVer);
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "获取设备版本信息: ->FR_GetFirmwareVersion() fail, ErrCode: %s, Return: %s",
            ConvertCode_TCF2Str(nRet), ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    memcpy(lpVer, szVer, (wSize < 64 ? wSize : 64));

    return IMP_SUCCESS;
}

// 14. 未知
INT CDevImpl_TCF261::HideTips(BOOL bIsHide)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = FR_RET_SUCC;

    nRet = FR_HideTips(bIsHide);
    if (nRet != FR_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            ": ->FR_HideTips(%d) fail, ErrCode: %s, Return: %s",
            bIsHide, ConvertCode_TCF2Str(nRet),
            ConvertCode_Impl2Str(ConvertCode_TCF2Impl(nRet)));
        return ConvertCode_TCF2Impl(nRet);
    }

    return IMP_SUCCESS;
}

// 回调函数(活检抓拍后返回)
int onDetectInfo(int *nResultID, void *lpParam)
{
    if (*nResultID == FR_CALLBACK_RESULT_SUCC)
    {
        cTask.cDevTCF261->SetLiveSuccFlag(TRUE);
    } else
    {
        cTask.cDevTCF261->SetLiveErrCode(*nResultID);
    }

    return 1;
}

// -------------------------------------- END --------------------------------------
