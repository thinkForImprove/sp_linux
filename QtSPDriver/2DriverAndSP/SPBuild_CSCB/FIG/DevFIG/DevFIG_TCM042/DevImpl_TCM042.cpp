#include "DevImpl_TCM042.h"

static const char *ThisFile = "CDevImpl_TCM042.cpp";

CDevImpl_TCM042::CDevImpl_TCM042()
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

    m_pFPIFindDevice = nullptr;
    m_pFPIGetFeature = nullptr;
    m_pFPIGetTemplate = nullptr;
    m_pFPIGetDevSN = nullptr;
    m_pFPIFpMatch = nullptr;
    m_pFPITplFrmImg = nullptr;
    m_pFPIFeaFrmImg = nullptr;
    m_pFPISaveImage = nullptr;
    m_pFPIChkPressed = nullptr;

    bLoadLibrary();
}

CDevImpl_TCM042::~CDevImpl_TCM042()
{
    CloseDevice();
    vUnLoadLibrary();
}


BOOL CDevImpl_TCM042::bLoadLibrary()
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

void CDevImpl_TCM042::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_TCM042::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1
    m_pFPIFindDevice = (pFPIFindDevice)m_LoadLibrary.resolve("FPIFindDevice");
    FUNC_POINTER_ERROR_RETURN(m_pFPIFindDevice, "pFPIFindDevice");
    // 2
    m_pFPIGetFeature = (pFPIGetFeature)m_LoadLibrary.resolve("FPIGetFeature");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetFeature, "pFPIGetFeature");
    // 3
    m_pFPIGetTemplate = (pFPIGetTemplate)m_LoadLibrary.resolve("FPIGetTemplate");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetTemplate, "pFPIGetTemplate");
    // 4
    m_pFPIGetDevSN = (pFPIGetDevSN)m_LoadLibrary.resolve("FPIGetDevSN");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetDevSN, "pFPIGetDevSN");
    // 5
    m_pFPIFpMatch = (pFPIFpMatch)m_LoadLibrary.resolve("FPIFpMatch");
    FUNC_POINTER_ERROR_RETURN(m_pFPIFpMatch, "pFPIFpMatch");
    // 6
    m_pFPITplFrmImg = (pFPITplFrmImg)m_LoadLibrary.resolve("FPITplFrmImg");
    FUNC_POINTER_ERROR_RETURN(m_pFPITplFrmImg, "pFPITplFrmImg");
    // 7
    m_pFPIFeaFrmImg = (pFPIFeaFrmImg)m_LoadLibrary.resolve("FPIFeaFrmImg");
    FUNC_POINTER_ERROR_RETURN(m_pFPIFeaFrmImg, "pFPIFeaFrmImg");
    // 8
    m_pFPISaveImage = (pFPISaveImage)m_LoadLibrary.resolve("FPISaveImage");
    FUNC_POINTER_ERROR_RETURN(m_pFPISaveImage, "pFPISaveImage");
    // 9
    m_pFPIChkPressed = (pFPIChkPressed)m_LoadLibrary.resolve("FPIChkPressed");
    FUNC_POINTER_ERROR_RETURN(m_pFPIChkPressed, "pFPIChkPressed");

    return TRUE;
}


BOOL CDevImpl_TCM042::OpenDevice()
{
    return OpenDevice(0);
}

BOOL CDevImpl_TCM042::OpenDevice(WORD wType)
{
    //如果已打开，则关闭
    CloseDevice();

    m_bDevOpenOk = FALSE;

    if (bLoadLibrary() != TRUE)
    {
        Log(ThisFile, 1, "加载动态库: OpenDevice()->LoadCloudWalkDll() fail. ");
        return FALSE;
    }

    m_bDevOpenOk = TRUE;
    return TRUE;
}

BOOL CDevImpl_TCM042::CloseDevice()
{
    vUnLoadLibrary();
    m_bDevOpenOk = FALSE;

    Log(ThisFile, 1, "设备关闭,释放动态库: Close Device, unLoad CloudWalk Dll.");

    return TRUE;
}

BOOL CDevImpl_TCM042::IsDeviceOpen()
{
    //return (m_bDevOpenOk == TRUE && m_cwDetector != NULL ? TRUE : FALSE);
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 1.1
int CDevImpl_TCM042::FPIFindDevice(int nPort, char * pszDevName)
{
    return m_pFPIFindDevice(nPort, pszDevName);
}
// 1.2
int CDevImpl_TCM042::FPIGetFeature(int nPort, char * pszVer, char *pImgBuf, int *ImgLen)
{
    return m_pFPIGetFeature(nPort, pszVer, pImgBuf, ImgLen);
}
// 1.3
int CDevImpl_TCM042::FPIGetTemplate(int nPort, char *pszReg)
{
    return m_pFPIGetTemplate(nPort, pszReg);
}
// 1.4
int CDevImpl_TCM042::FPIGetDevSN(int nPort, char * pszDevSN)
{
    return m_pFPIGetDevSN(nPort, pszDevSN);
}
// 1.5
int CDevImpl_TCM042::FPIFpMatch(char * pszReg, char * pszVer, int nMatchLevel)
{
    return m_pFPIFpMatch(pszReg, pszVer, nMatchLevel);
}
// 1.6
int CDevImpl_TCM042::FPITplFrmImg(char *pImgBuf1, char * pImgBuf2, char *pImgBuf3, char *pRegBuf, int *pnRegLen)
{
    return m_pFPITplFrmImg(pImgBuf1, pImgBuf2, pImgBuf3, pRegBuf, pnRegLen);
}
// 1.7
int CDevImpl_TCM042::FPIFeaFrmImg(char *pImgBuf, char *pVerBuf, int *pnVerLen)
{
    return m_pFPIFeaFrmImg(pImgBuf, pVerBuf, pnVerLen);
}
// 1.8
int CDevImpl_TCM042::FPISaveImage(const char *pImgPath, const char *pImgBuf)
{
    return m_pFPISaveImage(pImgPath, pImgBuf);
}
// 1.9
int CDevImpl_TCM042::FPIChkPressed()
{
    return m_pFPIChkPressed();
}
//---------------------------------------------
