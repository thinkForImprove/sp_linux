#include "DevImpl_WL.h"
#include <unistd.h>

static const char *ThisFile = "DevImpl_WL.cpp";


CAutoReleaseFlag::CAutoReleaseFlag(BOOL *b)
{
    m_pBOOL = b;
    *m_pBOOL = TRUE;
}

CAutoReleaseFlag::~CAutoReleaseFlag()
{
    if(m_pBOOL){
        *m_pBOOL = FALSE;
    }
}


CDevImpl_WEL401::CDevImpl_WEL401()
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

    m_pFPICancel = nullptr;
    m_pFPIPowerOn = nullptr;
    m_pFPIPowerOff = nullptr;
    m_pFPIDeviceInitRS232 = nullptr;
//m_pFPIDeviceInit = nullptr;
    m_pFPIDeviceInit = nullptr;
    m_pFPIDeviceClose = nullptr;
    m_pFPIGetVersion = nullptr;
    m_pFPIGetDeviceID = nullptr;
    m_pFPIFeature = nullptr;
    m_pFPITemplate = nullptr;
    m_pFPIGetImage = nullptr;
    m_pFPIGetImageEx = nullptr;
    m_pFPICheckFinger = nullptr;
    m_pFPICheckImage = nullptr;
    m_pFPIGatherEnroll = nullptr;
    m_pFPISynEnroll = nullptr;
    m_pFPIMatch = nullptr;
    m_pFPIFpMatch = nullptr;
    m_pFPIExtract = nullptr;
    m_pFPICryptBase64 = nullptr;
    m_pFPIEnroll = nullptr;
    m_pFPIEnrollX = nullptr;
    m_pFPIGetTemplateByTZ = nullptr;
    m_pFPIGetFeatureAndImage = nullptr;
    m_pFPIDevDetect = nullptr;
    m_pFPIGetImageDat = nullptr;
    m_pFPIImg2Bmp = nullptr;
    m_pFPIGetImageBufar = nullptr;
    m_pFPIMatchB64 = nullptr;
    m_pFPIBmp2Feature = nullptr;
    m_pFPIBmp2Template = nullptr;
    m_bDisableSendCmd = FALSE;
    bLoadLibrary();
}

BOOL CDevImpl_WEL401::bLoadLibrary()
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

void CDevImpl_WEL401::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}
CDevImpl_WEL401::~CDevImpl_WEL401()
{
    FPIDeviceClose();
    vUnLoadLibrary();
}

BOOL CDevImpl_WEL401::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;
    //打开设备
    m_pFPIDeviceInit = (lFPIDeviceInit)m_LoadLibrary.resolve("FPIDeviceInit");
    FUNC_POINTER_ERROR_RETURN(m_pFPIDeviceInit,"FPIDeviceInit");
    //关闭设备
    m_pFPIDeviceClose = (lFPIDeviceClose)m_LoadLibrary.resolve("FPIDeviceClose");
    FUNC_POINTER_ERROR_RETURN(m_pFPIDeviceClose,"FPIDeviceClose");
    //读取指纹仪的厂家和版本信息
    m_pFPIGetVersion = (lFPIGetVersion)m_LoadLibrary.resolve("FPIGetVersion");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetVersion,"FPIGetVersion");
    //读取指纹仪ID信息
    m_pFPIGetDeviceID = (lFPIGetDeviceID)m_LoadLibrary.resolve("FPIGetDeviceID");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetDeviceID,"FPIGetDeviceID");
    //采集指纹特征
    m_pFPIFeature = (lFPIFeature)m_LoadLibrary.resolve("FPIFeature");
    FUNC_POINTER_ERROR_RETURN(m_pFPIFeature,"FPIFeature");
    //采集指纹模板
    m_pFPITemplate = (lFPITemplate)m_LoadLibrary.resolve("FPITemplate");
    FUNC_POINTER_ERROR_RETURN(m_pFPITemplate,"FPITemplate");
    //检测手指
    m_pFPICheckFinger = (lFPICheckFinger)m_LoadLibrary.resolve("FPICheckFinger");
    FUNC_POINTER_ERROR_RETURN(m_pFPICheckFinger,"FPICheckFinger");
    //取消当前操作
    m_pFPICancel = (lFPICancel)m_LoadLibrary.resolve("FPICancel");
    FUNC_POINTER_ERROR_RETURN(m_pFPICancel,"FPICancel");
    //指纹比对
    m_pFPIFpMatch = (lFPIFpMatch)m_LoadLibrary.resolve("FPIFpMatch");
    FUNC_POINTER_ERROR_RETURN(m_pFPIFpMatch,"FPIFpMatch");
    //指纹图像转BMP文件
    m_pFPIImg2Bmp = (lFPIImg2Bmp)m_LoadLibrary.resolve("FPIImg2Bmp");
    FUNC_POINTER_ERROR_RETURN(m_pFPIImg2Bmp,"FPIImg2Bmp");
    //BMP指纹图像生成指纹特征
    m_pFPIBmp2Feature = (lFPIBmp2Feature)m_LoadLibrary.resolve("FPIBmp2Feature");
    FUNC_POINTER_ERROR_RETURN(m_pFPIBmp2Feature,"FPIBmp2Feature");
    // BMP指纹图像生成指纹模板
    m_pFPIBmp2Template = (lFPIBmp2Template)m_LoadLibrary.resolve("FPIBmp2Template");
    FUNC_POINTER_ERROR_RETURN(m_pFPIBmp2Template,"FPIBmp2Template");
    //获取指纹模板
    m_pFPIGetTemplateByTZ = (lFPIGetTemplateByTZ)m_LoadLibrary.resolve("FPIGetTemplateByTZ");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetTemplateByTZ,"FPIGetTemplateByTZ");
    //获取指纹特征并上传图像
    m_pFPIGetFeatureAndImage = (lFPIGetFeatureAndImage)m_LoadLibrary.resolve("FPIGetFeatureAndImage");
    FUNC_POINTER_ERROR_RETURN(m_pFPIGetFeatureAndImage,"FPIGetFeatureAndImage");

    return TRUE;
}

BOOL CDevImpl_WEL401::FPIDeviceInit()
{
    int nRet = -1;
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    nRet = m_pFPIDeviceInit();
    if (nRet != 0)
    {
        Log(ThisFile, 1, "LIVESCAN_Init fail-----");
        return FALSE;
    }
    Log(ThisFile, 1, "LIVESCAN_Init success");
    return TRUE;
}

BOOL CDevImpl_WEL401::FPIDeviceClose()
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIDeviceClose();
}

BOOL CDevImpl_WEL401::FPIGetVersion(char *psDevVersion, int *lpLength)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIGetVersion(psDevVersion, lpLength);
}

BOOL CDevImpl_WEL401::FPIGetDeviceID(char *psDeviceID)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIGetDeviceID(psDeviceID);
}

BOOL CDevImpl_WEL401::FPIFeature(int nTimeOut, unsigned char *psFpData, int *pnLength)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIFeature(nTimeOut, psFpData, pnLength);
}

BOOL CDevImpl_WEL401::FPITemplate(int nTimeOut, unsigned char *psFpData, int *pnLength)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPITemplate(nTimeOut, psFpData, pnLength);
}

BOOL CDevImpl_WEL401::FPIGetImageBuf(int nTimeOut, char *psBmpFile, int *lpImageWidth, int *lpImageHeight, unsigned char *psImageBuf)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIGetImageBufar(nTimeOut, psBmpFile, lpImageWidth, lpImageHeight, psImageBuf);
}

BOOL CDevImpl_WEL401::FPICheckFinger()
{
    if(m_bDisableSendCmd)
    {
        return TRUE;
    }
    return m_pFPICheckFinger();
}

void CDevImpl_WEL401::FPICancel()
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPICancel();
}

BOOL CDevImpl_WEL401::FPIFpMatch(char *psRegBuf, char *psVerBuf, int jnLevel)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIFpMatch(psRegBuf, psVerBuf, jnLevel);
}

BOOL CDevImpl_WEL401::FPIImg2Bmp(int nTimeOut, char *psBmpFile)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIImg2Bmp(nTimeOut, psBmpFile);
}

BOOL CDevImpl_WEL401::FPIBmp2Feature(int nMode, char *psImgPath1, char *psTZ, int *lpLength)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIBmp2Feature(nMode, psImgPath1, psTZ, lpLength);
}

BOOL CDevImpl_WEL401::FPIBmp2Template(int nMode, char *psImgPath1, char *psImgPath2, char *psImgPath3, char *psMB, int *lpLength)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIBmp2Template(nMode, psImgPath1, psImgPath2, psImgPath3, psMB, lpLength);
}

BOOL CDevImpl_WEL401::FPIGetTemplateByTZ(char *psTZ1, char *psTZ2, char *psTZ3, char *psRegBuf, int *pnLength)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIGetTemplateByTZ(psTZ1, psTZ2, psTZ3, psRegBuf, pnLength);
}

BOOL CDevImpl_WEL401::FPIGetFeatureAndImage(int nTimeOut, char *psFpData, int *pnFpDataLen, char *psImageBuf, int *pnImageLen, char *psBmpFile)
{
    CAutoReleaseFlag AutoReleaseFlag(&m_bDisableSendCmd);
    return m_pFPIGetFeatureAndImage(nTimeOut, psFpData, pnFpDataLen, psImageBuf, pnImageLen, psBmpFile);
}
