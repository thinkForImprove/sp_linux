#include "DevImpl_TPK193.h"

static const char *ThisFile = "DevSIG_TPK193.cpp";

CDevImpl_TPK193::CDevImpl_TPK193()
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

    m_pGetConnectStatus = nullptr;
    m_pGetDeviceStatus = nullptr;
    m_pResetDevice = nullptr;
    m_pStartSignature = nullptr;
    m_pClearSignature = nullptr;
    m_pEndSignature = nullptr;
    m_pSetPath = nullptr;
    m_pGetSignature = nullptr;
    m_pGetSignatureWin = nullptr;
    m_pGetSignatureNoEnc = nullptr;
    m_pGetPsgData = nullptr;
    m_pSetPenColor = nullptr;
    m_pSetPenWidth = nullptr;
    m_pSetBackgroundColor = nullptr;
    m_pSetBackgroundImage = nullptr;
    m_pLoadKey = nullptr;
    //m_pLoadKeyByName = nullptr;
    m_pCheckKCV = nullptr;
    m_pSetEncypt = nullptr;
    //m_pSetEncyptByName = nullptr;
    m_pShowSignData = nullptr;
    //m_pLogEnable = nullptr;
    //m_pSetlogPath = nullptr;
    m_pGetKCV = nullptr;
    m_pCalKCV = nullptr;

    bLoadLibrary();
}

CDevImpl_TPK193::~CDevImpl_TPK193()
{
    CloseDevice();
    vUnLoadLibrary();
}


BOOL CDevImpl_TPK193::bLoadLibrary()
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

void CDevImpl_TPK193::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_TPK193::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1
    m_pGetConnectStatus = (pGetConnectStatus)m_LoadLibrary.resolve("getConnectStatus");
    FUNC_POINTER_ERROR_RETURN(m_pGetConnectStatus, "pGetConnectStatus");
    // 2
    m_pGetDeviceStatus = (pGetDeviceStatus)m_LoadLibrary.resolve("getDeviceStatus");
    FUNC_POINTER_ERROR_RETURN(m_pGetDeviceStatus, "pGetDeviceStatus");
    // 3
    m_pResetDevice = (pResetDevice)m_LoadLibrary.resolve("resetDevice");
    FUNC_POINTER_ERROR_RETURN(m_pResetDevice, "pResetDevice");
    // 4
    m_pStartSignature = (pStartSignature)m_LoadLibrary.resolve("startSignature");
    FUNC_POINTER_ERROR_RETURN(m_pStartSignature, "pStartSignature");
    // 5
    m_pClearSignature = (pClearSignature)m_LoadLibrary.resolve("clearSignature");
    FUNC_POINTER_ERROR_RETURN(m_pClearSignature, "pClearSignature");
    // 6
    m_pEndSignature = (pEndSignature)m_LoadLibrary.resolve("endSignature");
    FUNC_POINTER_ERROR_RETURN(m_pEndSignature, "pEndSignature");
    // 7
    m_pSetPath = (pSetPath)m_LoadLibrary.resolve("setPath");
    FUNC_POINTER_ERROR_RETURN(m_pSetPath, "pSetPath");
    // 8
    m_pGetSignature = (pGetSignature)m_LoadLibrary.resolve("getSignature");
    FUNC_POINTER_ERROR_RETURN(m_pGetSignature, "pGetSignature");
    // 9
    m_pGetSignatureWin = (pGetSignatureWin)m_LoadLibrary.resolve("getSignatureWin");
    FUNC_POINTER_ERROR_RETURN(m_pGetSignatureWin, "pGetSignatureWin");
    // 10
    m_pGetSignatureNoEnc = (pGetSignatureNoEnc)m_LoadLibrary.resolve("getSignatureNoEnc");
    FUNC_POINTER_ERROR_RETURN(m_pGetSignatureNoEnc, "pGetSignatureNoEnc");
    // 11
    m_pGetPsgData = (pGetPsgData)m_LoadLibrary.resolve("getPsgData");
    FUNC_POINTER_ERROR_RETURN(m_pGetPsgData, "pGetPsgData");
    // 12
    m_pSetPenColor = (pSetPenColor)m_LoadLibrary.resolve("setPenColor");
    FUNC_POINTER_ERROR_RETURN(m_pSetPenColor, "pSetPenColor");
    // 13
    m_pSetPenWidth = (pSetPenWidth)m_LoadLibrary.resolve("setPenWidth");
    FUNC_POINTER_ERROR_RETURN(m_pSetPenWidth, "pSetPenWidth");
    // 14
    m_pSetBackgroundColor = (pSetBackgroundColor)m_LoadLibrary.resolve("setBackgroundColor");
    FUNC_POINTER_ERROR_RETURN(m_pSetBackgroundColor, "pSetBackgroundColor");
    // 15
    m_pSetBackgroundImage = (pSetBackgroundImage)m_LoadLibrary.resolve("setBackgroundImage");
    FUNC_POINTER_ERROR_RETURN(m_pSetBackgroundImage, "pSetBackgroundImage");
    // 16
    m_pLoadKey = (pLoadKey)m_LoadLibrary.resolve("loadKey");
    FUNC_POINTER_ERROR_RETURN(m_pLoadKey, "pLoadKey");
    // 17
//    m_pLoadKeyByName = (pLoadKeyByName)m_LoadLibrary.resolve("loadKeyByName");
//    FUNC_POINTER_ERROR_RETURN(m_pLoadKeyByName, "pLoadKeyByName");
    // 18
    m_pCheckKCV = (pCheckKCV)m_LoadLibrary.resolve("checkKCV");
    FUNC_POINTER_ERROR_RETURN(m_pCheckKCV, "pCheckKCV");
    // 19
    m_pSetEncypt = (pSetEncypt)m_LoadLibrary.resolve("setEncypt");
    FUNC_POINTER_ERROR_RETURN(m_pSetEncypt, "pSetEncypt");
    // 20
//    m_pSetEncyptByName = (pSetEncyptByName)m_LoadLibrary.resolve("setEncyptByName");
//    FUNC_POINTER_ERROR_RETURN(m_pSetEncyptByName, "pSetEncyptByName");
    // 21
    m_pShowSignData = (pShowSignData)m_LoadLibrary.resolve("showSignData");
    FUNC_POINTER_ERROR_RETURN(m_pShowSignData, "pShowSignData");
    // 22
//    m_pLogEnable = (pLogEnable)m_LoadLibrary.resolve("logEnable");
//    FUNC_POINTER_ERROR_RETURN(m_pLogEnable, "pLogEnable");
    // 23
//    m_pSetlogPath = (pSetlogPath)m_LoadLibrary.resolve("setlogPath");
//    FUNC_POINTER_ERROR_RETURN(m_pSetlogPath, "pSetlogPath");
    // 24
    m_pGetKCV = (pGetKCV)m_LoadLibrary.resolve("getKCV");
    FUNC_POINTER_ERROR_RETURN(m_pGetKCV, "pGetKCV");
    // 25
    m_pCalKCV = (pCalKCV)m_LoadLibrary.resolve("calKCV");
    FUNC_POINTER_ERROR_RETURN(m_pCalKCV, "pCalKCV");

    return TRUE;
}


BOOL CDevImpl_TPK193::OpenDevice()
{
    return OpenDevice(0);
}

BOOL CDevImpl_TPK193::OpenDevice(WORD wType)
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

BOOL CDevImpl_TPK193::CloseDevice()
{
    vUnLoadLibrary();
    m_bDevOpenOk = FALSE;

    Log(ThisFile, 1, "设备关闭,释放动态库: Close Device, unLoad CloudWalk Dll.");

    return TRUE;
}

BOOL CDevImpl_TPK193::IsDeviceOpen()
{
    //return (m_bDevOpenOk == TRUE && m_cwDetector != NULL ? TRUE : FALSE);
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}
// 1. 获取设备连接状态
int CDevImpl_TPK193::GetConnectStatus()
{
    return m_pGetConnectStatus();
}
// 2. 获取设备状态
int CDevImpl_TPK193::GetDeviceStatus()
{
    return m_pGetDeviceStatus();
}
// 3. 重置设备
int CDevImpl_TPK193::ResetDevice()
{
    return m_pResetDevice();
}
// 4. 启动签名
int CDevImpl_TPK193::StartSignature(int x, int y, int width, int height)
{
    return m_pStartSignature(x, y, width, height);
}
// 5. 清除签名
int CDevImpl_TPK193::ClearSign()
{
    return m_pClearSignature();
}
// 6. 结束签名
int CDevImpl_TPK193::EndSignature()
{
    return m_pEndSignature();
}
// 7. 设置路径
int CDevImpl_TPK193::SetPath(char *psgPath, char *imagePath)
{
    return m_pSetPath(psgPath, imagePath);
}
// 8. 获取签名(encrypt)
int CDevImpl_TPK193::GetSign(unsigned char *pSignData, int *pLen, int iWorkKey)
{
    return m_pGetSignature(pSignData, pLen, iWorkKey);
}
// 9. 获取签名(encrypt)
int CDevImpl_TPK193::GetSignatureWin(unsigned char *pSignData, int iMK, unsigned char *pucWK, int iAlgorithm, int iWKlen)
{
    return m_pGetSignatureWin(pSignData, iMK, pucWK, iAlgorithm, iWKlen);
}
// 10. 获取签名(plain)
int CDevImpl_TPK193::GetSignatureNoEnc(unsigned char *pSignData)
{
    return m_pGetSignatureNoEnc(pSignData);
}
// 11. 获取签名数据
unsigned char *CDevImpl_TPK193::GetPsgData(int &datalen)
{
    return m_pGetPsgData(datalen);
}
// 12. 设置签名笔迹的颜色
int CDevImpl_TPK193::SetPenColor(int colorflag)
{
    return m_pSetPenColor(colorflag);
}
// 13. 设置签名笔迹的宽度
int CDevImpl_TPK193::SetPenWidth(int width)
{
    return m_pSetPenWidth(width);
}
// 14. 设置Background颜色
int CDevImpl_TPK193::SetBackgroundColor(int colorflag)
{
    return m_pSetBackgroundColor(colorflag);
}
// 15. 设置Background图片
int CDevImpl_TPK193::SetBackgroundImage(const char *path)
{
    return m_pSetBackgroundImage(path);
}
// 16. 灌注密钥
int CDevImpl_TPK193::LoadKey(char *keydata, int datalen, int keyuse, int index, int algorithm,int decodekey, int checkmode)
{
    return m_pLoadKey(keydata, datalen, keyuse, index, algorithm,decodekey, checkmode);
}
//// 17. 灌注密钥（按名称）
//int CDevImpl_TPK193::LoadKeyByName(char *keydata, int datalen, int keyuse, char* keyname, int algorithm, char* decodekey, int checkmode)
//{
//    return m_pLoadKeyByName(keydata, datalen, keyuse, keyname, algorithm, decodekey, checkmode);
//}
// 18. 校验KCV
int CDevImpl_TPK193::CheckKCV(unsigned char*kcv, int len)
{
    return m_pCheckKCV(kcv, len);
}
// 19. 设置加密方式
int CDevImpl_TPK193::SetEncypt(int ec_algorithm, int ec_master_key, int ec_work_key)
{
    return m_pSetEncypt(ec_algorithm, ec_master_key, ec_work_key);
}
//// 20. 设置加密方式（按名称）
//int CDevImpl_TPK193::SetEncyptByName(int ec_algorithm, char* ec_master_key, char* ec_work_key)
//{
//    return m_pSetEncyptByName(ec_algorithm, ec_master_key, ec_work_key);
//}
// 21. 还原签名数据
int CDevImpl_TPK193::ShowSignData(unsigned char* ucData, int iLength, int iWorkKey, int iMainKey, char *pImgPath)
{
    return m_pShowSignData(ucData, iLength, iWorkKey, iMainKey, pImgPath);
}
// 22. 日志设置
//int CDevImpl_TPK193::LogEnable(bool bEnable)
//{
//    return m_pLogEnable(bEnable);
//}
// 23. 日志设置
//int CDevImpl_TPK193::SetlogPath(char* pstr)
//{
//    return m_pSetlogPath(pstr);
//}
// 24. 获取KCV
int CDevImpl_TPK193::GetKCV(unsigned char* pPlainData, int iPlainDataLen, int index, int algorithm, int checkmode, unsigned char*kcv)
{
    return m_pGetKCV(pPlainData, iPlainDataLen, index, algorithm, checkmode, kcv);
}
// 25. 计算KCV
int CDevImpl_TPK193::CalKCV(int index, unsigned char* ucKcv)
{
    return m_pCalKCV(index, ucKcv);
}
//---------------------------------------------
