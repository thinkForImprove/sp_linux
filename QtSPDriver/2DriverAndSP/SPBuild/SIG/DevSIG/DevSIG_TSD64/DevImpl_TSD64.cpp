#include "DevImpl_TSD64.h"

static const char *ThisFile = "CDevImpl_TSD64.cpp";

CDevImpl_TSD64::CDevImpl_TSD64()
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

    pSetScreenWidthHeight = nullptr;
    pSetSignWindow = nullptr;
    pSetBackColorParam = nullptr;
    pSetBackgroundPicture = nullptr;
    pSetTextData = nullptr;
    pSetPenMax = nullptr;
    pStartSignatureUseSetting = nullptr;
    pStartSignature = nullptr;
    pStartSignPng = nullptr;
    pStartSignPngPenMax = nullptr;
    pStartSignMsgPen = nullptr;
    pClearSignature = nullptr;
    pHideSignWindow = nullptr;
    pShowSignWindow = nullptr;
    pCloseSignWindow = nullptr;
    pEndSignature = nullptr;
    pGetSignature = nullptr;
    pGetSignData = nullptr;
    pGetSignDataFile = nullptr;
    pGetPngPicture = nullptr;
    pGetPngPictureW2H1 = nullptr;
    pSetDESPrimaryKey = nullptr;
    pSetPrimaryKey = nullptr;
    pResetDev = nullptr;
    pGetDevStatus = nullptr;
    pGetFirmwareVer = nullptr;
    pGetSign = nullptr;
    pImportKey = nullptr;
    pGetKeyVerificationCode = nullptr;
    pMasterKeyToWorkKey = nullptr;
    pGetCompileDate = nullptr;

    bLoadLibrary();
}

CDevImpl_TSD64::~CDevImpl_TSD64()
{
    CloseDevice();
    vUnLoadLibrary();
}


BOOL CDevImpl_TSD64::bLoadLibrary()
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

void CDevImpl_TSD64::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_TSD64::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1
    pSetScreenWidthHeight = (psgSetScreenWidthHeight)m_LoadLibrary.resolve("setScreenWidthHeight");
    FUNC_POINTER_ERROR_RETURN(pSetScreenWidthHeight, "psgSetScreenWidthHeight");
    // 2
    pSetSignWindow = (psgSetSignWindow)m_LoadLibrary.resolve("SetSignWindow");
    FUNC_POINTER_ERROR_RETURN(pSetSignWindow, "psgSetSignWindow");
    // 3
    pSetBackColorParam = (psgSetBackColorParam)m_LoadLibrary.resolve("SetBackColorParam");
    FUNC_POINTER_ERROR_RETURN(pSetBackColorParam, "psgSetBackColorParam");
    // 4
    pSetBackgroundPicture = (psgSetBackgroundPicture)m_LoadLibrary.resolve("SetBackgroundPicture");
    FUNC_POINTER_ERROR_RETURN(pSetBackgroundPicture, "psgSetBackgroundPicture");
    // 5
    pSetTextData = (psgSetTextData)m_LoadLibrary.resolve("SetTextData");
    FUNC_POINTER_ERROR_RETURN(pSetTextData, "psgSetTextData");
    // 6
    pSetPenMax = (psgSetPenMax)m_LoadLibrary.resolve("SetPenMax");
    FUNC_POINTER_ERROR_RETURN(pSetPenMax, "psgSetPenMax");
    // 7
    pStartSignatureUseSetting = (psgStartSignatureUseSetting)m_LoadLibrary.resolve("startSignatureUseSetting");
    FUNC_POINTER_ERROR_RETURN(pStartSignatureUseSetting, "psgStartSignatureUseSetting");
    // 8
    pStartSignature = (psgStartSignature)m_LoadLibrary.resolve("startSignature");
    FUNC_POINTER_ERROR_RETURN(pStartSignature, "psgStartSignature");
    // 9
    pStartSignPng = (psgStartSignPng)m_LoadLibrary.resolve("startSignPng");
    FUNC_POINTER_ERROR_RETURN(pStartSignPng, "psgStartSignPng");
    // 10
    pStartSignPngPenMax = (psgStartSignPngPenMax)m_LoadLibrary.resolve("startSignPngPenMax");
    FUNC_POINTER_ERROR_RETURN(pStartSignPngPenMax, "psgStartSignPngPenMax");
    // 11
    pStartSignMsgPen = (psgStartSignMsgPen)m_LoadLibrary.resolve("startSignMsgPen");
    FUNC_POINTER_ERROR_RETURN(pStartSignMsgPen, "psgStartSignMsgPen");
    // 12
    pClearSignature = (psgClearSignature)m_LoadLibrary.resolve("clearSignature");
    FUNC_POINTER_ERROR_RETURN(pClearSignature, "psgClearSignature");
    // 13
    pHideSignWindow = (psgHideSignWindow)m_LoadLibrary.resolve("hideSignWindow");
    FUNC_POINTER_ERROR_RETURN(pHideSignWindow, "psgHideSignWindow");
    // 14
    pShowSignWindow = (psgShowSignWindow)m_LoadLibrary.resolve("showSignWindow");
    FUNC_POINTER_ERROR_RETURN(pShowSignWindow, "psgShowSignWindow");
    // 15
    pCloseSignWindow = (psgCloseSignWindow)m_LoadLibrary.resolve("closeSignWindow");
    FUNC_POINTER_ERROR_RETURN(pCloseSignWindow, "psgCloseSignWindow");
    // 16
    pEndSignature = (psgEndSignature)m_LoadLibrary.resolve("endSignature");
    FUNC_POINTER_ERROR_RETURN(pEndSignature, "psgEndSignature");
    // 17
    pGetSignature = (psgGetSignature)m_LoadLibrary.resolve("getSignature");
    FUNC_POINTER_ERROR_RETURN(pGetSignature, "psgGetSignature");
    // 18
    pGetSignData = (psgGetSignData)m_LoadLibrary.resolve("getSignData");
    FUNC_POINTER_ERROR_RETURN(pGetSignData, "psgGetSignData");
    // 19
    pGetSignDataFile = (psgGetSignDataFile)m_LoadLibrary.resolve("getSignDataFile");
    FUNC_POINTER_ERROR_RETURN(pGetSignDataFile, "psgGetSignDataFile");
    // 20
    pGetPngPicture = (psgGetPngPicture)m_LoadLibrary.resolve("getPngPicture");
    FUNC_POINTER_ERROR_RETURN(pGetPngPicture, "psgGetPngPicture");
    // 21
    pGetPngPictureW2H1 = (psgGetPngPictureW2H1)m_LoadLibrary.resolve("getPngPictureW2H1");
    FUNC_POINTER_ERROR_RETURN(pGetPngPictureW2H1, "psgGetPngPictureW2H1");
    // 22
    pSetDESPrimaryKey = (psgSetDESPrimaryKey)m_LoadLibrary.resolve("setDESPrimaryKey");
    FUNC_POINTER_ERROR_RETURN(pSetDESPrimaryKey, "psgSetDESPrimaryKey");
    // 23
    pSetPrimaryKey = (psgSetPrimaryKey)m_LoadLibrary.resolve("setPrimaryKey");
    FUNC_POINTER_ERROR_RETURN(pSetPrimaryKey, "psgSetPrimaryKey");
    // 24
    pResetDev = (psgResetDev)m_LoadLibrary.resolve("resetDev");
    FUNC_POINTER_ERROR_RETURN(pResetDev, "psgResetDev");
    // 25
    pGetDevStatus = (psgGetDevStatus)m_LoadLibrary.resolve("getDevStatus");
    FUNC_POINTER_ERROR_RETURN(pGetDevStatus, "psgGetDevStatus");
    // 26
    pGetFirmwareVer = (psgGetFirmwareVer)m_LoadLibrary.resolve("getFirmwareVer");
    FUNC_POINTER_ERROR_RETURN(pGetFirmwareVer, "psgGetFirmwareVer");
    // 27
    pGetSign = (psgGetSign)m_LoadLibrary.resolve("getSign");
    FUNC_POINTER_ERROR_RETURN(pGetSign, "psgGetSign");
    // 28
    pImportKey = (psgImportKey)m_LoadLibrary.resolve("importKey");
    FUNC_POINTER_ERROR_RETURN(pImportKey, "psgImportKey");
    // 29
    pGetKeyVerificationCode = (psgGetKeyVerificationCode)m_LoadLibrary.resolve("getKeyVerificationCode");
    FUNC_POINTER_ERROR_RETURN(pGetKeyVerificationCode, "psgGetKeyVerificationCode");
    // 30
    pMasterKeyToWorkKey = (psgMasterKeyToWorkKey)m_LoadLibrary.resolve("masterKeyToWorkKey");
    FUNC_POINTER_ERROR_RETURN(pMasterKeyToWorkKey, "psgMasterKeyToWorkKey");
    // 31
    pGetCompileDate = (psgGetCompileDate)m_LoadLibrary.resolve("getCompileDate");
    FUNC_POINTER_ERROR_RETURN(pGetCompileDate, "psgGetCompileDate");

    return TRUE;
}


BOOL CDevImpl_TSD64::OpenDevice()
{
    return OpenDevice(0);
}

BOOL CDevImpl_TSD64::OpenDevice(WORD wType)
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

BOOL CDevImpl_TSD64::CloseDevice()
{
    vUnLoadLibrary();
    m_bDevOpenOk = FALSE;

    Log(ThisFile, 1, "设备关闭,释放动态库: Close Device, unLoad CloudWalk Dll.");

    return TRUE;
}

BOOL CDevImpl_TSD64::IsDeviceOpen()
{
    //return (m_bDevOpenOk == TRUE && m_cwDetector != NULL ? TRUE : FALSE);
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 1.1
void CDevImpl_TSD64::vSetScreenWidthHeight(int offsetX, int offsetY, unsigned int screenWidth, unsigned int screenHeight, char *pchErrCode)
{
    pSetScreenWidthHeight(offsetX, offsetY, screenWidth, screenHeight, pchErrCode);
}
// 1.2
void CDevImpl_TSD64::vSetSignWindow(int x, int y, int w, int h, char *pchErrCode)
{
    pSetSignWindow(x, y, w, h, pchErrCode);
}
// 1.3
void CDevImpl_TSD64::vSetBackColorParam(int Transparency, unsigned long backColor, int useBackColor, char *pchErrCode)
{
    pSetBackColorParam(Transparency, backColor, useBackColor, pchErrCode);
}
// 1.4
void CDevImpl_TSD64::vSetBackgroundPicture(char *photoPath, bool bPicAlwaysShow, char* pchErrCode)
{
    pSetBackgroundPicture(photoPath, bPicAlwaysShow, pchErrCode);
}
// 1.5
void CDevImpl_TSD64::vSetTextData(int Left, int Top, const char* string, const char* fontName, int fontSize, unsigned long textColor, bool bTxtAlwaysShow, char* pchErrCode)
{
    pSetTextData(Left, Top, string, fontName, fontSize, textColor, bTxtAlwaysShow, pchErrCode);
}
// 1.6
void CDevImpl_TSD64::vSetPenMax(int MaxPressurePixel, char *pchErrCode)
{
    pSetPenMax(MaxPressurePixel, pchErrCode);
}
// 1.7
void CDevImpl_TSD64::vStartSignatureUseSetting(char* pchErrCode)
{
    pStartSignatureUseSetting(pchErrCode);
}
// 1.8
void CDevImpl_TSD64::vStartSignature(int x, int y, int w, int h, char *pchErrCode)
{
    pStartSignature(x, y, w, h, pchErrCode);
}
// 1.9
void CDevImpl_TSD64::vStartSignPng(int x, int y, int w, int h, char *photoPath, bool bAlwaysShow, char *pchErrCode)
{
    pStartSignPng(x, y, w, h, photoPath, bAlwaysShow, pchErrCode);
}
// 1.10
void CDevImpl_TSD64::vStartSignPngPenMax(int x, int y, int w, int h, unsigned int PenMax, char *photoPath, bool bAlwaysShow, char* pchErrCode)
{
    pStartSignPngPenMax(x, y, w, h, PenMax, photoPath, bAlwaysShow, pchErrCode);
}
// 1.11
void CDevImpl_TSD64::vStartSignMsgPen(int x, int y, int w, int h, unsigned int PenMax, const char *pcMsg, bool bAlwaysShow, int iX, int iY, int iFontHeight, unsigned long ulColor, const char *pccFont, char *pchErrCode)
{
    pStartSignMsgPen(x, y, w, h, PenMax, pcMsg, bAlwaysShow, iX, iY, iFontHeight, ulColor, pccFont, pchErrCode);
}
// 1.12
void CDevImpl_TSD64::vClearSignature(char* pchErrCode)
{
    pClearSignature(pchErrCode);
}
// 1.13
void CDevImpl_TSD64::vHideSignWindow(char *pchErrCode)
{
    pHideSignWindow(pchErrCode);
}
// 1.14
void CDevImpl_TSD64::vShowSignWindow(char *pchErrCode)
{
    pShowSignWindow(pchErrCode);
}
// 1.15
void CDevImpl_TSD64::vCloseSignWindow(char* pchErrCode)
{
    pCloseSignWindow(pchErrCode);
}
// 1.16
void CDevImpl_TSD64::vEndSignature(char* pchErrCode)
{
    pEndSignature(pchErrCode);
}
// 1.17
void CDevImpl_TSD64::vGetSignature(unsigned char *psignData, long *psignDataLen, int iIndex, unsigned char *pbWorkKey, char *photoPath, char *pchErrCode)
{
    pGetSignature(psignData, psignDataLen, iIndex, pbWorkKey, photoPath, pchErrCode);
}
// 1.18
void CDevImpl_TSD64::vGetSignData(unsigned char *psignData, long *psignDataLen, char* pchErrCode)
{
    pGetSignData(psignData, psignDataLen, pchErrCode);
}
// 1.19
void CDevImpl_TSD64::vGetSignDataFile(char *signFilePath, int type, char *pchErrCode)
{
    pGetSignDataFile(signFilePath, type, pchErrCode);
}
// 1.20
void CDevImpl_TSD64::vGetPngPicture(char *photoPath, double multiple, char* pchErrCode)
{
    pGetPngPicture(photoPath, multiple, pchErrCode);
}
// 1.21
void CDevImpl_TSD64::vGetPngPictureW2H1(char *photoPath, double multiple, char *pchErrCode)
{
    pGetPngPictureW2H1(photoPath, multiple, pchErrCode);
}
// 1.22
void CDevImpl_TSD64::vSetDESPrimaryKey(char *pPriKeys, int number, char* pchErrCode)
{
    pSetDESPrimaryKey(pPriKeys, number, pchErrCode);
}
// 1.23
void CDevImpl_TSD64::vSetPrimaryKey(char *pPriKeys, int iLength, int iIndex, char *pchErrCode)
{
    pSetPrimaryKey(pPriKeys, iLength, iIndex, pchErrCode);
}
// 1.24
void CDevImpl_TSD64::vResetDev(char *pchErrCode)
{
    pResetDev(pchErrCode);
}
// 1.25
void CDevImpl_TSD64::vGetDevStatus(char* pchErrCode)
{
    pGetDevStatus(pchErrCode);
}
// 1.26
void CDevImpl_TSD64::vGetFirmwareVer(char *strVer, char* pchErrCode)
{
    pGetFirmwareVer(strVer, pchErrCode);
}
// 1.27
void CDevImpl_TSD64::vGetSign(unsigned char *psignData, int iEncryptType, char* pchErrCode)
{
    pGetSign(psignData, iEncryptType, pchErrCode);
}
// 1.28
void CDevImpl_TSD64::vImportKey(char *pKey, int iLength, int iIndex, int iDecKeyNum, int iDecMode, int iUse, char *pchErrCode)
{
    pImportKey(pKey, iLength, iIndex, iDecKeyNum, iDecMode, iUse, pchErrCode);
}
// 1.29
void CDevImpl_TSD64::vGetKeyVerificationCode(char* pKVC, int *iLength, int iIndex, int iEncMode, char* pchErrCode)
{
    pGetKeyVerificationCode(pKVC, iLength, iIndex, iEncMode, pchErrCode);
}
// 1.30
void CDevImpl_TSD64::vMasterKeyToWorkKey(int iIndex, char *pchErrCode)
{
    pMasterKeyToWorkKey(iIndex, pchErrCode);
}
// 1.31
void CDevImpl_TSD64::vGetCompileDate(char *strDate, char *pchErrCode)
{
    pGetCompileDate(strDate, pchErrCode);
}
//---------------------------------------------
