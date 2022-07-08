/***************************************************************
* 文件名称: DevImpl_ZLF1000A3.cpp
* 文件描述: ZL-F1000A3高拍仪模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年2月17日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_ZLF1000A3.h"
#include <dlfcn.h>

static const char *ThisFile = "DevImpl_ZLF1000A3.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[3] != IMP_ERR_DEV_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertErrCodeToStr(IMP_ERR_DEV_NOTOPEN)); \
        } \
        m_nRetErrOLD[3] = IMP_ERR_DEV_NOTOPEN; \
        return IMP_ERR_DEV_NOTOPEN; \
    }

//----------------------------------构造/析构/初始化----------------------------------
#include "DevImpl_ZLF1000A3.h"

CDevImpl_ZLF1000A3::CDevImpl_ZLF1000A3()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_ZLF1000A3::CDevImpl_ZLF1000A3(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_ZLF1000A3::CDevImpl_ZLF1000A3(LPSTR lpLog, LPSTR lpDevStr)
{
    SetLogFile(lpLog, ThisFile, lpDevStr);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_ZLF1000A3::~CDevImpl_ZLF1000A3()
{
    DeviceClose();
    vUnLoadLibrary();
}

// 参数初始化
void CDevImpl_ZLF1000A3::Init()
{
    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("CAM/ZL1000A3/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_bLoadIntfFail = TRUE;

    // 动态库接口函数初始化
    ZLOpenDevice = nullptr;           // 1. 打开摄像头
    ZLGrabImage = nullptr;            // 2. 获取图像
    ZLSaveImage = nullptr;            // 3. 获取图像并保存
    ZLGrabVideoData = nullptr;        // 4. 获取视频图像数据
    ZLCloseDevice = nullptr;          // 5. 关闭摄像头
    ZLGetResolution = nullptr;        // 6. 获取分辨率
    ZLSetResolution = nullptr;        // 7. 设置分辨率
    ZLGetDeviceStatus = nullptr;      // 8. 获取设备状态

    //bLoadLibrary();

    m_vLibInst = nullptr;
}

//----------------------------------SDK接口加载----------------------------------
// 加载动态库
BOOL CDevImpl_ZLF1000A3::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库<%s> Fail. ErrCode:%s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> Succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

// 释放动态库
void CDevImpl_ZLF1000A3::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

// 加载动态库接口函数
BOOL CDevImpl_ZLF1000A3::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1. 打开摄像头
    LOAD_LIBINFO_FUNC(pZLOpenDevice, ZLOpenDevice, "OpenDevice");

    // 2. 获取图像
    LOAD_LIBINFO_FUNC(pZLGrabImage, ZLGrabImage, "GrabImage");

    // 3. 获取图像并保存
    LOAD_LIBINFO_FUNC(pZLSaveImage, ZLSaveImage, "SaveImage");

    // 4. 获取视频图像数据
    LOAD_LIBINFO_FUNC(pZLGrabVideoData, ZLGrabVideoData, "GrabVideoData");

    // 5. 关闭摄像头
    LOAD_LIBINFO_FUNC(pZLCloseDevice, ZLCloseDevice, "CloseDevice");

    // 6. 获取分辨率
    LOAD_LIBINFO_FUNC(pZLGetResolution, ZLGetResolution, "GetResolution");

    // 7. 设置分辨率
    LOAD_LIBINFO_FUNC(pZLSetResolution, ZLSetResolution, "SetResolution");

    // 8. 获取设备状态
    LOAD_LIBINFO_FUNC(pZLGetDeviceStatus, ZLGetDeviceStatus, "GetDeviceStatus");

    return TRUE;
}

// 加载动态库
BOOL CDevImpl_ZLF1000A3::bDlLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_bLoadIntfFail = TRUE;

    if (m_vLibInst == nullptr)
    {
        m_vLibInst = dlopen(m_szLoadDllPath, RTLD_NOW | RTLD_DEEPBIND);
        if (m_vLibInst == nullptr)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库<%s> Fail.", m_szLoadDllPath);
            }
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> Succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bDlLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Fail. ", m_szLoadDllPath);
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

// 释放动态库
void CDevImpl_ZLF1000A3::vDlUnLoadLibrary()
{
    if(m_vLibInst != nullptr)
    {
        dlclose(m_vLibInst);
        m_vLibInst = nullptr;
    }
}

// 加载动态库接口函数
BOOL CDevImpl_ZLF1000A3::bDlLoadLibIntf()
{
    THISMODULE(__FUNCTION__);

    m_bLoadIntfFail = FALSE;

    // 1. 打开摄像头
    LOAD_LIBINFO_FUNC_DL(m_vLibInst, pZLOpenDevice, ZLOpenDevice, "OpenDevice");

    // 2. 获取图像
    LOAD_LIBINFO_FUNC_DL(m_vLibInst, pZLGrabImage, ZLGrabImage, "GrabImage");

    // 3. 获取图像并保存
    LOAD_LIBINFO_FUNC_DL(m_vLibInst, pZLSaveImage, ZLSaveImage, "SaveImage");

    // 4. 获取视频图像数据
    LOAD_LIBINFO_FUNC_DL(m_vLibInst, pZLGrabVideoData, ZLGrabVideoData, "GrabVideoData");

    // 5. 关闭摄像头
    LOAD_LIBINFO_FUNC_DL(m_vLibInst, pZLCloseDevice, ZLCloseDevice, "CloseDevice");

    // 6. 获取分辨率
    LOAD_LIBINFO_FUNC_DL(m_vLibInst, pZLGetResolution, ZLGetResolution, "GetResolution");

    // 7. 设置分辨率
    LOAD_LIBINFO_FUNC_DL(m_vLibInst, pZLSetResolution, ZLSetResolution, "SetResolution");

    // 8. 获取设备状态
    LOAD_LIBINFO_FUNC_DL(m_vLibInst, pZLGetDeviceStatus, ZLGetDeviceStatus, "GetDeviceStatus");

    return TRUE;
}

//----------------------------------SDK封装接口方法----------------------------------
// 1. 打开设备
INT CDevImpl_ZLF1000A3::DeviceOpen(WORD wType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;

    m_wOpenCamType = wType;

    //如果已打开，则关闭
    DeviceClose();

    m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        //if (bLoadLibrary() != TRUE)
        if (bDlLoadLibrary() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                    ConvertErrCodeToStr(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    // 设备连接
    nRet = ZLOpenDevice(m_wOpenCamType);
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[1])
        {
            Log(ThisModule, __LINE__,
                "打开设备: ZLOpenDevice(%d) = %d, Failed, ErrCode: %d, Return: %s.",
                m_wOpenCamType, nRet, ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[1] = nRet;
        }
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__,
            "打开设备: ZLOpenDevice(%d) = %d, Success.", m_wOpenCamType, nRet);
    }

    /*if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }

    m_bReCon = FALSE;   // 是否断线重连状态: 初始F*/
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));
    m_bDevOpenOk = TRUE;

    return IMP_SUCCESS;
}

// 2. 关闭设备
INT CDevImpl_ZLF1000A3::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    if (m_bDevOpenOk == TRUE)
    {
        if (ZLCloseDevice(m_wOpenCamType) != IMP_SUCCESS)
        {
            if (ZLCloseDevice(m_wOpenCamType) != IMP_SUCCESS)
            {
                ZLCloseDevice(m_wOpenCamType);
            }
        }
    }

    vUnLoadLibrary();
    m_bDevOpenOk = FALSE;

    /*if (m_bReCon != TRUE)   // 断线重连状态时不释放动态库
    {
        vUnLoadLibrary();
    }

    m_bDevOpenOk = FALSE;

    if (m_bReCon != TRUE)   // 断线重连状态时不记录LOG
    {
        Log(ThisModule, __LINE__, "关闭设备: Close Device, unLoad PossdkDll.");
    }*/

    return IMP_SUCCESS;
}

// 3. 获取设备状态
INT CDevImpl_ZLF1000A3::GetDevStatus()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLGetDeviceStatus(m_wOpenCamType);
    if (nRet != IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[3])
        {
            Log(ThisModule, __LINE__, "获取设备状态: ZLGetDeviceStatus(%d) fail. Return: %s.",
                m_wOpenCamType, ConvertErrCodeToStr(nRet));
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
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLGrabImage(m_wOpenCamType, lpnDataBuf, nDataBufLen,
                           lpnImgWidth, lpnImgHeight, nRotate,
                           bAutoCut, nImgType);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取图像: ZLGrabImage(%d, %d, %d, %d, %d, %d, %d) fail. Return: %s.",
            m_wOpenCamType, nDataBufLen, *lpnImgWidth, *lpnImgHeight, nRotate,
            bAutoCut, nImgType, ConvertErrCodeToStr(nRet));
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
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLSaveImage(m_wOpenCamType, lpcFilePath, nRotate, bAutoCut, nImgType);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取图像并保存: ZLSaveImage(%d, %s, %d, %d, %d) fail. Return: %s.",
            m_wOpenCamType, lpcFilePath, nRotate, bAutoCut, nImgType,
            ConvertErrCodeToStr(nRet));
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
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLGrabVideoData(m_wOpenCamType, lpnDataBuf, nDataBufLen,
                               lpnImgWidth, lpnImgHeight, lplTimesTamp, dDrawCutRect);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取视频图像数据: ZLGrabVideoData(%d, %d, %d, %d, %d, %d) fail. Return: %s.",
            m_wOpenCamType, nDataBufLen, *lpnImgWidth, *lpnImgHeight, *lplTimesTamp,
            dDrawCutRect, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 7. 获取分辨率
INT CDevImpl_ZLF1000A3::GetResolut(UINT unResoBuf[30][2])
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLGetResolution(m_wOpenCamType, unResoBuf);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取分辨率: ZLGetResolution(%d) fail. Return: %s.",
            m_wOpenCamType, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 8. 设置分辨率
INT CDevImpl_ZLF1000A3::SetResolut(INT nWidth, INT nHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = ZLSetResolution(m_wOpenCamType, nWidth, nHeight);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设置分辨率: ZLSetResolution(%d, %d, %d) fail. Return: %s.",
            m_wOpenCamType, nWidth, nHeight, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

//----------------------------------其他接口方法----------------------------------

BOOL CDevImpl_ZLF1000A3::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

LPSTR CDevImpl_ZLF1000A3::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(nRet)
    {
        case IMP_SUCCESS:
            sprintf(m_szErrStr,  "%d|%s", nRet, "成功");
            break;
        case IMP_ERR_LOAD_LIB:
            sprintf(m_szErrStr,  "%d|%s", nRet, "动态库加载失败");
            break;
        case IMP_ERR_PARAM_INVALID:
            sprintf(m_szErrStr,  "%d|%s", nRet, "参数无效");
            break;
        case IMP_ERR_DEV_NOTOPEN:
            sprintf(m_szErrStr,  "%d|%s", nRet, "设备未打开");
            break;
        case IMP_ERR_DEV_GETVDATA:
            sprintf(m_szErrStr,  "%d|%s", nRet, "获取视频/图像数据失败");
            break;
        case IMP_ERR_DEV_PARAM:
            sprintf(m_szErrStr,  "%d|%s", nRet, "SDK接口参数错误");
        default :
            sprintf(m_szErrStr,  "%d|%s", nRet, "未知错误");
            break;
    }

    return (LPSTR)m_szErrStr;
}

//------------------------------------END-------------------------------------
