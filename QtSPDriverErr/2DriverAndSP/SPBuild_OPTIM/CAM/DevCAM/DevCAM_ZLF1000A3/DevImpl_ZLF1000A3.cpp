/***************************************************************************
* 文件名称: DevImpl_ZLF1000A3.h
* 文件描述: 封装摄像模块底层指令,提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
***************************************************************************/

#include "DevImpl_ZLF1000A3.h"

#include <dlfcn.h>
#include <qimage.h>
#include "opencv2/opencv.hpp"

static const char *ThisFile = "DevImpl_ZLF1000A3.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[2] != IMP_ERR_DEV_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertCode_Impl2Str(IMP_ERR_DEV_NOTOPEN)); \
        } \
        m_nRetErrOLD[2] = IMP_ERR_DEV_NOTOPEN; \
        return IMP_ERR_DEV_NOTOPEN; \
    }

//-----------------------------构造/析构/初始化-----------------------------
CDevImpl_ZLF1000A3::CDevImpl_ZLF1000A3()
{
    SetLogFile(LOG_NAME, ThisFile);                 // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_ZLF1000A3::CDevImpl_ZLF1000A3(LPSTR lpLog)
{
    SetLogFile(LOG_NAME, ThisFile);                 // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_ZLF1000A3::CDevImpl_ZLF1000A3(LPSTR lpLog, LPCSTR lpDevType)
{
    SetLogFile(lpLog, ThisFile, lpDevType);         // 设置日志文件名和错误发生的文件
    MSET_0(m_szDevType);
    MCPY_NOLEN(m_szDevType, strlen(lpDevType) > 0 ? lpDevType : "ZLF1000A3");
    Init();
}

CDevImpl_ZLF1000A3::~CDevImpl_ZLF1000A3()
{
    m_stImageData.Clear();
    if (m_ucGetVideoDataBuff != nullptr)
    {
        free(m_ucGetVideoDataBuff);
    }
    CloseDevice();
    vDlUnLoadLibrary();
}

// 参数初始化
void CDevImpl_ZLF1000A3::Init()
{
    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("CAM/ZLF1000A3/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    SetDlOpenMode(RTLD_NOW | RTLD_DEEPBIND);               // dlOpen命令模式

    // 变量初始化
    MSET_0(m_szDevType);                                    // 设备类型
    m_bDevOpenOk = FALSE;                                   // 设备Open标记
    m_bReCon = FALSE;                                       // 是否断线重连状态
    memset(m_nRetErrOLD, 0, sizeof(INT) * 12);              // 处理错误值保存
    m_bDrawCutRect = TRUE;                                  // 是否绘制切边区域
    m_nRetImgWidthOLD = 0;                                  // 保存上一次图像宽
    m_nRetImgHeightOLD = 0;                                 // 保存上一次图像高
    m_ucGetVideoDataBuff = nullptr;                         // 图像帧数据保存空间
    STIMGDATA       m_stImageData;                          // 保存图像帧数据
    m_nOpenCamType = ZL_APPLYMODE_DOC;                      // 摄像头类型索引
    MSET_0(m_szErrStr);                                     // IMPL错误码解析

    // 动态库接口函数初始化
    vInitLibFunc();
}

//------------------------------SDK接口加载--------------------------------
// 动态库接口初始化
void CDevImpl_ZLF1000A3::vInitLibFunc()
{
    // 动态库接口函数初始化
    ZLOpenDevice = nullptr;           // 1. 打开摄像头
    ZLGrabImage = nullptr;            // 2. 获取图像
    ZLSaveImage = nullptr;            // 3. 获取图像并保存
    ZLGrabVideoData = nullptr;        // 4. 获取视频图像数据
    ZLCloseDevice = nullptr;          // 5. 关闭摄像头
    ZLGetResolution = nullptr;        // 6. 获取分辨率
    ZLSetResolution = nullptr;        // 7. 设置分辨率
    ZLGetDeviceStatus = nullptr;      // 8. 获取设备状态
}

// 加载动态库接口函数(dlxxx方式)
INT CDevImpl_ZLF1000A3::nDlLoadLibIntf()
{
    // 1. 打开摄像头
    LOAD_LIBINFO_FUNC_DL(pZLOpenDevice, ZLOpenDevice, "OpenDevice");

    // 2. 获取图像
    LOAD_LIBINFO_FUNC_DL(pZLGrabImage, ZLGrabImage, "GrabImage");

    // 3. 获取图像并保存
    LOAD_LIBINFO_FUNC_DL(pZLSaveImage, ZLSaveImage, "SaveImage");

    // 4. 获取视频图像数据
    LOAD_LIBINFO_FUNC_DL(pZLGrabVideoData, ZLGrabVideoData, "GrabVideoData");

    // 5. 关闭摄像头
    LOAD_LIBINFO_FUNC_DL(pZLCloseDevice, ZLCloseDevice, "CloseDevice");

    // 6. 获取分辨率
    LOAD_LIBINFO_FUNC_DL(pZLGetResolution, ZLGetResolution, "GetResolution");

    // 7. 设置分辨率
    LOAD_LIBINFO_FUNC_DL(pZLSetResolution, ZLSetResolution, "SetResolution");

    // 8. 获取设备状态
    LOAD_LIBINFO_FUNC_DL(pZLGetDeviceStatus, ZLGetDeviceStatus, "GetDeviceStatus");

    return SUCCESS;
}

//----------------------------SDK封装接口方法-------------------------------
// 1. 打开设备
INT CDevImpl_ZLF1000A3::OpenDevice()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 如果已打开，则关闭
    if (m_bDevOpenOk == TRUE)
    {
        CloseDevice();
    }

    m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    INT nLoadRet = SUCCESS;
    if (GetLoadLibIsSucc() != TRUE)
    {
        nLoadRet = nDlLoadLibrary(m_szLoadDllPath);
        if (nLoadRet != SUCCESS)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: 加载动态库: nDlLoadLibrary(%s) Failed, ErrCode: %s, Return: %s.",
                    m_szLoadDllPath, GetLibError(LOADDLL_MODE_QLIB),
                    ConvertCode_Impl2Str(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        } else
        {
            Log(ThisModule, __LINE__,
                "打开设备: 加载动态库: nDlLoadLibrary(%s) Succ.", m_szLoadDllPath);
            m_nRetErrOLD[0] = IMP_SUCCESS;
        }
    }

    //-----------------------打开设备-----------------------
    nRet = ZLOpenDevice(m_nOpenCamType);
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[1])
        {
            Log(ThisModule, __LINE__,
                "打开设备: ->ZLOpenDevice(%d) fail, ErrCode: %d, Return: %s.",
                m_nOpenCamType, nRet, ConvertCode_Impl2Str(nRet));
            m_nRetErrOLD[1] = nRet;
        }
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__,
            "打开设备: ->ZLOpenDevice(%d) Success.", m_nOpenCamType, nRet);
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

// 2. 关闭设备
INT CDevImpl_ZLF1000A3::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if(m_bDevOpenOk == TRUE)
    {
        if (ZLCloseDevice(m_nOpenCamType) != IMP_SUCCESS)
        {
            if (ZLCloseDevice(m_nOpenCamType) != IMP_SUCCESS)
            {
                ZLCloseDevice(m_nOpenCamType);
            }
        }
    }

    // 非断线重连时关闭需释放动态库
    if (m_bReCon == FALSE)
    {
        // 释放动态库
        vDlUnLoadLibrary();
        Log(ThisModule, __LINE__, "设备关闭: 释放动态库: Close Device(), unLoad Library.");
    }

    // 设备Open标记=F
    m_bDevOpenOk = FALSE;

    return IMP_SUCCESS;
}

// 3. 获取设备状态
INT CDevImpl_ZLF1000A3::GetDevStatus()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLGetDeviceStatus(m_nOpenCamType);
    if (nRet != IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[3])
        {
            Log(ThisModule, __LINE__, "获取设备状态: ZLGetDeviceStatus(%d) fail. Return: %s.",
                m_nOpenCamType, ConvertCode_Impl2Str(nRet));
            m_nRetErrOLD[3] = nRet;
        }
        return nRet;
    }

    return nRet;
}

// 4. 获取图像
INT CDevImpl_ZLF1000A3::GetImage(LPINT lpnDataBuf, INT nDataBufLen,
                                 LPINT lpnImgWidth, LPINT lpnImgHeight,
                                 INT nImgType/* = IMG_TYPE_COLOR */,
                                 INT nRotate/* = IMG_ROT_0 */, BOOL bAutoCut/* = TRUE */)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLGrabImage(m_nOpenCamType, lpnDataBuf, nDataBufLen,
                           lpnImgWidth, lpnImgHeight, nRotate,
                           bAutoCut, nImgType);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取图像: ZLGrabImage(%d, %d, %d, %d, %d, %d, %d) fail. Return: %s.",
            m_nOpenCamType, nDataBufLen, *lpnImgWidth, *lpnImgHeight, nRotate,
            bAutoCut, nImgType, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 5. 获取图像并保存
INT CDevImpl_ZLF1000A3::SaveImage(LPCSTR lpcFilePath, INT nImgType/* = IMG_TYPE_COLOR */,
                                  INT nRotate/* = IMG_ROT_0 */, BOOL bAutoCut/* = TRUE */)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLSaveImage(m_nOpenCamType, lpcFilePath, nRotate, bAutoCut, nImgType);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取图像并保存: ZLSaveImage(%d, %s, %d, %d, %d) fail. Return: %s.",
            m_nOpenCamType, lpcFilePath, nRotate, bAutoCut, nImgType,
            ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 6. 获取视频图像数据
INT CDevImpl_ZLF1000A3::GetVideoData(LPINT lpnDataBuf, INT nDataBufLen,
                                     LPINT lpnImgWidth, LPINT lpnImgHeight,
                                     LPLONG lplTimesTamp, BOOL dDrawCutRect/* = TRUE */)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLGrabVideoData(m_nOpenCamType, lpnDataBuf, nDataBufLen,
                               lpnImgWidth, lpnImgHeight, lplTimesTamp, dDrawCutRect);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取视频图像数据: ZLGrabVideoData(%d, %d, %d, %d, %d, %d) fail. Return: %s.",
            m_nOpenCamType, nDataBufLen, *lpnImgWidth, *lpnImgHeight, *lplTimesTamp,
            dDrawCutRect, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 7. 获取分辨率
INT CDevImpl_ZLF1000A3::GetResolut(UINT unResoBuf[30][2])
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLGetResolution(m_nOpenCamType, unResoBuf);

    return nRet;
}

// 8. 设置分辨率
INT CDevImpl_ZLF1000A3::SetResolut(INT nWidth, INT nHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLSetResolution(m_nOpenCamType, nWidth, nHeight);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设置分辨率: ZLSetResolution(%d, %d, %d) fail. Return: %s.",
            m_nOpenCamType, nWidth, nHeight, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 取图像帧数据
INT CDevImpl_ZLF1000A3::GetVideoImage(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, WORD wFlip)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = IMP_SUCCESS;
    INT nRetImgWidth = 0;
    INT nRetImgHeight = 0;
    INT nRetImgChannel = 4;
    INT nRetImgSize = 0;
    LONG nRetImgTimesTamp = 0;
    INT nBuffSize = 0;

    // 取图像宽高
    nRet = ZLGrabVideoData(m_nOpenCamType, nullptr, 0, &nRetImgWidth,
                           &nRetImgHeight, &nRetImgTimesTamp, m_bDrawCutRect);
    if (nRet != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[4] != nRet)
        {
            Log(ThisModule, __LINE__,
                "获取视频图像数据宽高: ->ZLGrabVideoData(%d, NULL, %d, %d, %d, %d, %d) fail. Return: %s.",
                m_nOpenCamType, 0, nRetImgWidth, nRetImgHeight, nRetImgTimesTamp,
                m_bDrawCutRect, ConvertCode_Impl2Str(nRet));
            m_nRetErrOLD[4] = nRet;
        }

        return nRet;
    }

    if (nRetImgWidth < 1 || nRetImgHeight < 1)
    {
        return IMP_ERR_INTEREXEC;
    }

    if (m_ucGetVideoDataBuff == nullptr)
    {
        // 初始:根据图像宽高申请图像帧保存内存空间
        nRetImgSize = nRetImgWidth * nRetImgHeight * nRetImgChannel;
        m_ucGetVideoDataBuff = (UCHAR*)malloc(sizeof(UCHAR) * (nRetImgSize + 1));
        if (m_ucGetVideoDataBuff == nullptr)
        {
            return IMP_ERR_MEMORY;
        }
        m_nRetImgWidthOLD = nRetImgWidth;
        m_nRetImgHeightOLD = nRetImgHeight;
    } else
    {
        // 图像帧保存内存空间已申请, 如图像宽高与申请的宽高不一致, 重新申请
        if (m_nRetImgWidthOLD != nRetImgWidth || m_nRetImgHeightOLD != nRetImgHeight)
        {
            free(m_ucGetVideoDataBuff);
            m_ucGetVideoDataBuff == nullptr;

            nRetImgSize = nRetImgWidth * nRetImgHeight * nRetImgChannel;
            m_ucGetVideoDataBuff = (UCHAR*)malloc(sizeof(UCHAR) * (nRetImgSize + 1));
            if (m_ucGetVideoDataBuff == nullptr)
            {
                return IMP_ERR_MEMORY;
            }
            m_nRetImgWidthOLD = nRetImgWidth;
            m_nRetImgHeightOLD = nRetImgHeight;
        }
        nRetImgSize = nRetImgWidth * nRetImgHeight * nRetImgChannel;
    }

    // 取图像帧数据
    nRet = ZLGrabVideoData(m_nOpenCamType, (LPINT)m_ucGetVideoDataBuff,
                           nRetImgSize, &nRetImgWidth, &nRetImgHeight,
                           &nRetImgTimesTamp, m_bDrawCutRect);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取视频图像数据: ->ZLGrabVideoData(%d, %d, %d, %d, %d, %d) fail. Return: %s.",
            m_nOpenCamType, nRetImgSize, nRetImgWidth, nRetImgHeight,
            nRetImgTimesTamp, m_bDrawCutRect, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    if (nRetImgWidth < 1 || nRetImgHeight < 1)
    {
        return IMP_ERR_INTEREXEC;
    }

    QImage qImgBuffer = QImage((uchar*)m_ucGetVideoDataBuff, nRetImgWidth, nRetImgHeight, QImage::Format_ARGB32);
    cv::Mat cvMatImg(qImgBuffer.height(), qImgBuffer.width(), CV_8UC4,
                     (void*)qImgBuffer.constBits(), qImgBuffer.bytesPerLine());
    if (nWidth != cvMatImg.cols || nHeight != cvMatImg.rows)
    {
        cv::resize(cvMatImg, cvMatImg, cv::Size(nWidth, nHeight));
    }


    if (cvMatImg.cols != m_stImageData.nWidth ||
        cvMatImg.rows != m_stImageData.nHeight ||
        cvMatImg.channels() != m_stImageData.nFormat)
    {
        m_stImageData.Clear();

        nBuffSize = cvMatImg.cols * cvMatImg.rows * cvMatImg.channels() + 1 + 5;

        m_stImageData.ucImgData = (UCHAR*)malloc(sizeof(UCHAR) * nBuffSize);
        if (m_stImageData.ucImgData == nullptr)
        {
            return IMP_ERR_MEMORY;
        }
        m_stImageData.nWidth = cvMatImg.cols;
        m_stImageData.nHeight = cvMatImg.rows;
        m_stImageData.nFormat = cvMatImg.channels();
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
    MCPY_LEN(m_stImageData.ucImgData + 5, cvMatImg.data, nBuffSize - 1 - 5);

    lpImgData->nWidth = m_stImageData.nWidth;
    lpImgData->nHeight = m_stImageData.nHeight;
    lpImgData->nFormat = m_stImageData.nFormat;
    lpImgData->ulImagDataLen = m_stImageData.ulImagDataLen;
    lpImgData->ucImgData = m_stImageData.ucImgData;

    lpImgData->nOtherParam[0] = 0;

    return IMP_SUCCESS;
}

//--------------------------其他接口方法-----------------------------
BOOL CDevImpl_ZLF1000A3::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// Impl错误码解析
LPSTR CDevImpl_ZLF1000A3::ConvertCode_Impl2Str(INT nErrCode)
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
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_NOTOPEN, nErrCode, "设备未打开");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_GETVDATA, nErrCode, "获取视频/图像数据失败");
            IMPL_TCF_CASE_CODE_STR(IMP_ERR_DEV_PARAM, nErrCode, "SDK接口参数错误");
            default :
                sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
                break;
        }
    }

    return (LPSTR)m_szErrStr;
}


//----------------------------对外参数设置接口-------------------------------
// 设置断线重连标记
INT CDevImpl_ZLF1000A3::SetReConFlag(BOOL bFlag)
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
INT CDevImpl_ZLF1000A3::SetLibPath(LPCSTR lpPath)
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

    return IMP_SUCCESS;
}

// 设置设备使用模式
INT CDevImpl_ZLF1000A3::SetApplyMode(INT nMode)
{
    THISMODULE(__FUNCTION__);

    if (nMode == ZL_APPLYMODE_DOC || nMode == ZL_APPLYMODE_PER)
    {
        m_nOpenCamType = nMode;
        Log(ThisModule, __LINE__, "设置设备使用模式为: %d|%s",
            nMode, nMode == ZL_APPLYMODE_DOC ? "文档模式" : "人脸模式");
    } else
    {
        m_nOpenCamType = ZL_APPLYMODE_DOC;
        Log(ThisModule, __LINE__,
            "设置设备使用模式入参 %d != %d/%d, 使用缺省模式: %d|%s",
            nMode, ZL_APPLYMODE_DOC, nMode == ZL_APPLYMODE_PER,
            ZL_APPLYMODE_DOC, "文档模式");
    }

    return IMP_SUCCESS;
}

// 设置图像帧是否绘制切边区域
INT CDevImpl_ZLF1000A3::SetDrawCutRect(BOOL bIsDCRect)
{
    THISMODULE(__FUNCTION__);

    m_bDrawCutRect = bIsDCRect;
    Log(ThisModule, __LINE__, "设置图像帧是否绘制切边区域为: %s",
        bIsDCRect == TRUE ? "TRUE" : "FALSE");

    return IMP_SUCCESS;
}

//------------------------------------END-------------------------------------
