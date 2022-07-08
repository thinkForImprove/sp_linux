#include "DevImpl_SM205BCT.h"

static const char *ThisFile = "CDevImpl_SM205BCT.cpp";

CDevImpl_SM205BCT::CDevImpl_SM205BCT()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件

    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_LoadLibrary.setFileName(strDllName);
    m_bLoadIntfFail = TRUE;

    m_pFPIGetDevVersion = nullptr;
    m_pFPIDetectFinger = nullptr;
    m_pFPICancel = nullptr;
    m_pFPIGetDevSN = nullptr;
    m_pFPIGetFeature = nullptr;
    m_pFPIGetTemplate = nullptr;
    m_pFPIMatch = nullptr;
    m_pFPIGetImageBmp = nullptr;

    bLoadLibrary();
}

CDevImpl_SM205BCT::~CDevImpl_SM205BCT()
{
    CloseDevice();
    vUnLoadLibrary();
}


BOOL CDevImpl_SM205BCT::bLoadLibrary()
{
    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }
    return TRUE;
}

void CDevImpl_SM205BCT::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_SM205BCT::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1
    m_pFPIGetDevVersion = (lpFPIGetDevVersion)m_LoadLibrary.resolve("FPIGetDevVersion");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetDevVersion, "lpFPIGetDevVersion");
    // 2
    m_pFPIDetectFinger = (lpFPIDetectFinger)m_LoadLibrary.resolve("FPIDetectFinger");
    FUNC_POINTER_ERROR_RETURN(m_pFPIDetectFinger, "lpFPIDetectFinger");
    // 3
    m_pFPICancel = (lpFPICancel)m_LoadLibrary.resolve("FPICancel");
    FUNC_POINTER_ERROR_RETURN(m_pFPICancel, "lpFPICancel");
    // 4
    m_pFPIGetDevSN = (lpFPIGetDevSN)m_LoadLibrary.resolve("FPIGetDevSN");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetDevSN, "lpFPIGetDevSN");
    // 5
    m_pFPIGetFeature = (lpFPIGetFeature)m_LoadLibrary.resolve("FPIGetFeature");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetFeature, "lpFPIGetFeature");
    // 6
    m_pFPIGetTemplate = (lpFPIGetTemplate)m_LoadLibrary.resolve("FPIGetTemplate");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetTemplate, "lpFPIGetTemplate");
    // 7
    m_pFPIMatch = (lpFPIMatch)m_LoadLibrary.resolve("FPIMatch");
    FUNC_POINTER_ERROR_RETURN(m_pFPIMatch, "lpFPIMatch");
    // 8
    m_pFPIGetImageBmp = (lpFPIGetImageBmp)m_LoadLibrary.resolve("FPIGetImageBmp");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetImageBmp, "lpFPIGetImageBmp");

    return TRUE;
}


BOOL CDevImpl_SM205BCT::OpenDevice()
{
    return OpenDevice(0);
}

BOOL CDevImpl_SM205BCT::OpenDevice(WORD wType)
{
    //如果已打开，则关闭
    CloseDevice();

    m_bDevOpenOk = FALSE;

    if (bLoadLibrary() != TRUE)
    {
        Log(ThisFile, 1, "加载动态库: OpenDevice() fail. ");
        return FALSE;
    }

    m_bDevOpenOk = TRUE;
    return TRUE;
}

BOOL CDevImpl_SM205BCT::CloseDevice()
{
    vUnLoadLibrary();
    m_bDevOpenOk = FALSE;

    Log(ThisFile, 1, "设备关闭,释放动态库: Close Device, unLoad CloudWalk Dll.");

    return TRUE;
}

BOOL CDevImpl_SM205BCT::IsDeviceOpen()
{
    //return (m_bDevOpenOk == TRUE && m_cwDetector != NULL ? TRUE : FALSE);
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 1. 获取设备版本号
int CDevImpl_SM205BCT::FPIGetDevVersion(int nPortNo, char* lpVersion)
{
    return m_pFPIGetDevVersion(nPortNo, lpVersion);
}
// 2. 手指检测
int CDevImpl_SM205BCT::FPIDetectFinger(int nPortNo)
{
    return m_pFPIDetectFinger(nPortNo);
}
// 3. 取消取指纹
void CDevImpl_SM205BCT::FPICancel()
{
    return m_pFPICancel();
}
// 4. 获取设备序列号
int CDevImpl_SM205BCT::FPIGetDevSN(int nPortNo, char* lpDevSN)
{
    return m_pFPIGetDevSN(nPortNo, lpDevSN);
}
// 5. 取指纹特征
int CDevImpl_SM205BCT::FPIGetFeature(int nPortNo, int nTimeOut, char* lpFinger, unsigned char* lpImage, int* lpImageLen)
{
    return m_pFPIGetFeature(nPortNo, nTimeOut, lpFinger, lpImage, lpImageLen);
}
// 6. 取指纹模板
int CDevImpl_SM205BCT::FPIGetTemplate(int nPortNo, int nTimeOut, char* lpFinger)
{
    return m_pFPIGetTemplate(nPortNo, nTimeOut, lpFinger);
}
// 7. 比对指纹
int CDevImpl_SM205BCT::FPIMatch(char* lpFinger1, char* lpFinger2, int nLevel)
{
    return m_pFPIMatch(lpFinger1, lpFinger2, nLevel);
}
// 8. 保存指纹图片(bmp)
int CDevImpl_SM205BCT::FPIGetImageBmp(unsigned char* lpImageData, unsigned char* lpBmpData)
{
    return m_pFPIGetImageBmp(lpImageData, lpBmpData);
}
//---------------------------------------------
