#include "DevImpl_HX.h"

static const char *ThisFile = "DevImpl_HX.cpp";

CHX_DevImpl::CHX_DevImpl()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件

    QString strDllName_Read(QString::fromLocal8Bit(DLL_DEVREAD_NAME));
    QString strDllName_Comp(QString::fromLocal8Bit(DLL_DEVCOMPARE_NAME));

    #ifdef Q_OS_WIN
        strDllName_Read.prepend(WINPATH);
    #else
        strDllName_Read.prepend(LINUXPATHLIB);
        strDllName_Comp.prepend(LINUXPATHLIB);
    #endif

    memset(m_szFprCapDllPath, 0x00, sizeof(m_szFprCapDllPath));
    std::string Read = strDllName_Read.toStdString();
    sprintf(m_szFprCapDllPath, "%s", Read.data());

    memset(m_szFprDllPath, 0x00, sizeof(m_szFprDllPath));
    std::string Comp = strDllName_Comp.toStdString();
    sprintf(m_szFprDllPath, "%s", Comp.c_str());

    m_FprCapLibrary.setFileName(strDllName_Read);
    m_FprLibrary.setFileName(strDllName_Comp);

    m_bLoadFprCapIntfFail = TRUE;
    m_bLoadFprIntfFail = TRUE;

    // ID_FprCap.so
    m_pLIVESCAN_Init                        = nullptr;  // 1.  初始化采集器
    m_pLIVESCAN_Close                       = nullptr;  // 2.  释放采集器
    m_pLIVESCAN_GetChannelCount             = nullptr;  // 3.  获得采集器通道数量
    m_pLIVESCAN_SetBright                   = nullptr;  // 4.  设置采集器当前的亮度
    m_pLIVESCAN_SetContrast                 = nullptr;  // 5.  设置采集器当前对比度
    m_pLIVESCAN_GetBright                   = nullptr;  // 6.  获得采集器当前的亮度
    m_pLIVESCAN_GetContrast                 = nullptr;  // 7.  获得采集器当前对比度
    m_pLIVESCAN_GetMaxImageSize             = nullptr;  // 8.  获得采集器采集图像的宽度、高度的最大值
    m_pLIVESCAN_GetCaptWindow               = nullptr;  // 9.  获得当前图像的采集位置、宽度和高度
    m_pLIVESCAN_SetCaptWindow               = nullptr;  // 10. 设置当前图像的采集位置、宽度和高度
    m_pLIVESCAN_Setup                       = nullptr;  // 11. 调用采集器的属性设置对话框
    m_pLIVESCAN_BeginCapture                = nullptr;  // 12. 准备采集一帧图像
    m_pLIVESCAN_GetFPRawData                = nullptr;  // 13. 采集一帧图像
    m_pLIVESCAN_GetFPBmpData                = nullptr;  // 14. 采集一帧 BMP 格式图像数据
    m_pLIVESCAN_EndCapture                  = nullptr;  // 15. 结束采集一帧图像
    m_pLIVESCAN_IsSupportSetup              = nullptr;  // 16. 采集器是否支持设置对话框
    m_pLIVESCAN_GetVersion                  = nullptr;  // 17. 取得接口规范的版本
    m_pLIVESCAN_GetDesc                     = nullptr;  // 18. 获得接口规范的说明
    m_pLIVESCAN_GetErrorInfo                = nullptr;  // 19. 获得采集接口错误信息
    m_pLIVESCAN_SetBufferEmpty              = nullptr;  // 20. 设置存放采集数据的内存块为空

    // ID_Fpr.so
    m_pFPGetVersion                         = nullptr;   // 1. 版本信息获取
    m_pFPBegin                              = nullptr;   // 2. 初始化操作
    m_pFPFeatureExtract                     = nullptr;   // 3. 1 枚指纹图像特征提取
    m_pFPFeatureMatch                       = nullptr;   // 4. 对 2 个指纹特征数据进行比对
    m_pFPImageMatch                         = nullptr;   // 5. 对指纹图像和指纹特征进行比对
    m_pFPCompress                           = nullptr;   // 6. 对指纹图像数据进行压缩
    m_pFPDecompress                         = nullptr;   // 7. 对指纹图像压缩数据进行复现
    m_pFPGetQualityScore                    = nullptr;   // 8. 获取指纹图像的质量值
    m_pFPGenFeatureFromEmpty1               = nullptr;   // 9. 生成"未注册"指纹特征数据
    m_pFPEnd                                = nullptr;   // 10. 结束操作

    LoadDll();
}

BOOL CHX_DevImpl::LoadDll()
{
    if (!m_FprCapLibrary.isLoaded())
    {
        if (!m_FprCapLibrary.load())
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szFprCapDllPath, m_FprCapLibrary.errorString().data());
            return FALSE;
        }
    }

    if (!m_FprLibrary.isLoaded())
    {
        if (!m_FprLibrary.load())
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szFprDllPath, m_FprLibrary.errorString().data());
            return FALSE;
        }
    }

    if (m_bLoadFprCapIntfFail)
    {
        if (!LoadFprCapIntf())
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szFprCapDllPath, m_FprCapLibrary.errorString().data());
            return FALSE;
        }
    }

    if (m_bLoadFprIntfFail)
    {
        if (!LoadFprIntf())
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szFprDllPath, m_FprLibrary.errorString().data());
            return FALSE;
        }
    }

    return TRUE;
}

CHX_DevImpl::~CHX_DevImpl()
{
    Cap_Close();
    UnloadCloudWalkDll();
}

BOOL CHX_DevImpl::LoadFprCapIntf()
{
    m_bLoadFprCapIntfFail = FALSE;

    // 1. 初始化采集器
    m_pLIVESCAN_Init = (FIGLIVESCANINIT)m_FprCapLibrary.resolve("LIVESCAN_Init");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_Init, "LIVESCAN_Init");

    // 2. 释放采集器
    m_pLIVESCAN_Close = (FIGLIVESCANCLOSE)m_FprCapLibrary.resolve("LIVESCAN_Close");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_Close, "LIVESCAN_Close");

    // 3. 获得采集器通道数量
    m_pLIVESCAN_GetChannelCount = (FIGLIVESCANGETCHANNELCOUNT)m_FprCapLibrary.resolve("LIVESCAN_GetChannelCount");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetChannelCount, "LIVESCAN_GetChannelCount");

    // 4. 设置采集器当前的亮度
    m_pLIVESCAN_SetBright = (FIGLIVESCANSETBRIGHT)m_FprCapLibrary.resolve("LIVESCAN_SetBright");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_SetBright, "LIVESCAN_SetBright");

    // 5. 设置采集器当前对比度
    m_pLIVESCAN_SetContrast = (FIGLIVESCANSETCONTRAST)m_FprCapLibrary.resolve("LIVESCAN_SetContrast");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_SetContrast, "LIVESCAN_SetContrast");

    // 6. 获得采集器当前的亮度
    m_pLIVESCAN_GetBright = (FIGLIVESCANGETBRIGHT)m_FprCapLibrary.resolve("LIVESCAN_GetBright");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetBright, "LIVESCAN_GetBright");

    // 7. 获得采集器当前对比度
    m_pLIVESCAN_GetContrast = (FIGLIVESCANGETCONTRAST)m_FprCapLibrary.resolve("LIVESCAN_GetContrast");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetContrast, "LIVESCAN_GetContrast");

    // 8. 获得采集器采集图像的宽度、高度的最大值
    m_pLIVESCAN_GetMaxImageSize = (FIGLIVESCANGETMAXIMAGESIZE)m_FprCapLibrary.resolve("LIVESCAN_GetMaxImageSize");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetMaxImageSize, "LIVESCAN_GetMaxImageSize");

    // 9. 获得当前图像的采集位置、宽度和高度
    m_pLIVESCAN_GetCaptWindow = (FIGLIVESCANGETCAPTWINDOW)m_FprCapLibrary.resolve("LIVESCAN_GetCaptWindow");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetCaptWindow, "LIVESCAN_GetCaptWindow");

    // 10. 设置当前图像的采集位置、宽度和高度
    m_pLIVESCAN_SetCaptWindow = (FIGLIVESCANSETCAPTWINDOW)m_FprCapLibrary.resolve("LIVESCAN_SetCaptWindow");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_SetCaptWindow, "LIVESCAN_SetCaptWindow");

    // 11. 调用采集器的属性设置对话框
    m_pLIVESCAN_Setup = (FIGLIVESCANSETUP)m_FprCapLibrary.resolve("LIVESCAN_Setup");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_Setup, "LIVESCAN_Setup");

    // 12. 准备采集一帧图像
    m_pLIVESCAN_BeginCapture = (FIGLIVESCANBEINCAPTURE)m_FprCapLibrary.resolve("LIVESCAN_BeginCapture");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_BeginCapture, "LIVESCAN_BeginCapture");

    // 13. 采集一帧图像
    m_pLIVESCAN_GetFPRawData = (FIGLIVESCANGETFPRAWDATA)m_FprCapLibrary.resolve("LIVESCAN_GetFPRawData");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetFPRawData, "LIVESCAN_GetFPRawData");

    // 14. 采集一帧 BMP 格式图像数据
    m_pLIVESCAN_GetFPBmpData = (FIGLIVESCANGETFPBMPDATA)m_FprCapLibrary.resolve("LIVESCAN_GetFPBmpData");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetFPBmpData, "LIVESCAN_GetFPBmpData");

    // 15. 结束采集一帧图像
    m_pLIVESCAN_EndCapture = (FIGLIVESCANENDCAPTURE)m_FprCapLibrary.resolve("LIVESCAN_EndCapture");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_EndCapture, "LIVESCAN_EndCapture");

    // 16. 采集器是否支持设置对话框
    m_pLIVESCAN_IsSupportSetup = (FIGLIVESCANISSUPPORTSETUP)m_FprCapLibrary.resolve("LIVESCAN_IsSupportSetup");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_IsSupportSetup, "LIVESCAN_IsSupportSetup");

    // 17. 取得接口规范的版本
    m_pLIVESCAN_GetVersion = (FIGLIVESCANGETVERSION)m_FprCapLibrary.resolve("LIVESCAN_GetVersion");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetVersion, "LIVESCAN_GetVersion");

    // 18. 获得接口规范的说明
    m_pLIVESCAN_GetDesc = (FIGLIVESCANGETDESC)m_FprCapLibrary.resolve("LIVESCAN_GetDesc");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetDesc, "LIVESCAN_GetDesc");

    // 19. 获得采集接口错误信息
    m_pLIVESCAN_GetErrorInfo = (FIGLIVESCANGETERRORINFO)m_FprCapLibrary.resolve("LIVESCAN_GetErrorInfo");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_GetErrorInfo, "LIVESCAN_GetErrorInfo");

    // 20. 设置存放采集数据的内存块为空
    m_pLIVESCAN_SetBufferEmpty = (FIGLIVESCANSETBUFFERRMPTY)m_FprCapLibrary.resolve("LIVESCAN_SetBufferEmpty");
    FUNC_POINTER_ERROR_RETURN_CAP(m_pLIVESCAN_SetBufferEmpty, "LIVESCAN_SetBufferEmpty");
}

BOOL CHX_DevImpl::LoadFprIntf()
{
    m_bLoadFprIntfFail = FALSE;

    // 1. 版本信息获取
    m_pFPGetVersion = (FIGFPGETVERSION)m_FprLibrary.resolve("FP_GetVersion");
    FUNC_POINTER_ERROR_RETURN(m_pFPGetVersion, "FP_GetVersion");

    // 2. 初始化操作
    m_pFPBegin = (FIGFPBEGIN)m_FprLibrary.resolve("FP_Begin");
    FUNC_POINTER_ERROR_RETURN(m_pFPBegin, "FP_Begin");

    // 3. 1 枚指纹图像特征提取
    m_pFPFeatureExtract = (FIGFPFEATUREEXTRACT)m_FprLibrary.resolve("FP_FeatureExtract");
    FUNC_POINTER_ERROR_RETURN(m_pFPFeatureExtract, "FP_FeatureExtract");

    // 4. 对 2 个指纹特征数据进行比对
    m_pFPFeatureMatch = (FIGFPFEATUREMATCH)m_FprLibrary.resolve("FP_FeatureMatch");
    FUNC_POINTER_ERROR_RETURN(m_pFPFeatureMatch, "FP_FeatureMatch");

    // 5. 对指纹图像和指纹特征进行比对
    m_pFPImageMatch = (FIGFPIMAGEMATCH)m_FprLibrary.resolve("FP_ImageMatch");
    FUNC_POINTER_ERROR_RETURN(m_pFPImageMatch, "FP_ImageMatch");

    // 6. 对指纹图像数据进行压缩
    m_pFPCompress = (FIGFPCOMPRESS)m_FprLibrary.resolve("FP_Compress");
    FUNC_POINTER_ERROR_RETURN(m_pFPCompress, "FP_Compress");

    // 7. 对指纹图像压缩数据进行复现
    m_pFPDecompress = (FIGFPDECOMPRESS)m_FprLibrary.resolve("FP_Decompress");
    FUNC_POINTER_ERROR_RETURN(m_pFPDecompress, "FP_Decompress");

    // 8. 获取指纹图像的质量值
    m_pFPGetQualityScore = (FIGFPGETQUALITYSCORE)m_FprLibrary.resolve("FP_GetQualityScore");
    FUNC_POINTER_ERROR_RETURN(m_pFPGetQualityScore, "FP_GetQualityScore");

    // 9. 生成"未注册"指纹特征数据
    m_pFPGenFeatureFromEmpty1 = (FIGFPGENFEATUREFROMEMPTY1)m_FprLibrary.resolve("FP_GenFeatureFromEmpty1");
    FUNC_POINTER_ERROR_RETURN(m_pFPGenFeatureFromEmpty1, "FP_GenFeatureFromEmpty1");

    // 10. 结束操作
    m_pFPEnd = (FIGFPEND)m_FprLibrary.resolve("FP_End");
    FUNC_POINTER_ERROR_RETURN(m_pFPEnd, "FP_End");
}

void CHX_DevImpl::UnloadCloudWalkDll()
{
    if (m_FprCapLibrary.isLoaded())
    {
        m_FprCapLibrary.unload();
        m_bLoadFprCapIntfFail = TRUE;
    }

    if (m_FprLibrary.isLoaded())
    {
        m_FprLibrary.unload();
        m_bLoadFprIntfFail = TRUE;
    }
}

// FprCap.so
// 1. 初始化采集器
BOOL CHX_DevImpl::Cap_Init()
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_Init();
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_Init fail, ErrorInfo :%s", Cap_GetErrorInfo(nRet));
        return FALSE;
    }

    return TRUE;
}

// 2. 释放采集器
BOOL CHX_DevImpl::Cap_Close()
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_Close();
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_Close fail");
        return FALSE;
    }

    return TRUE;
}

// 3. 获得采集器通道数量
BOOL CHX_DevImpl::Cap_GetChannelCount()
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetChannelCount();
    if (nRet <= 0)
    {
        Log(ThisFile, 1, "LIVESCAN_GetChannelCount fail");
        return FALSE;
    }

    return nRet;
}

// 4. 设置采集器当前的亮度
BOOL CHX_DevImpl::Cap_SetBright(int nChannel, int nBright)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_SetBright(nChannel, nBright);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_SetBright fail");
        return FALSE;
    }

    return TRUE;
}

// 5. 设置采集器当前对比度
BOOL CHX_DevImpl::Cap_SetContrast(int nChannel,int nContrast)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_SetContrast(nChannel, nContrast);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_SetContrast fail");
        return FALSE;
    }

    return TRUE;
}

// 6. 获得采集器当前的亮度
BOOL CHX_DevImpl::Cap_GetBright(int nChannel,int* pnBright)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetBright(nChannel, pnBright);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_GetBright fail");
        return FALSE;
    }

    return TRUE;
}

// 7. 获得采集器当前对比度
BOOL CHX_DevImpl::Cap_GetContrast(int nChannel,int* pnContrast)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetContrast(nChannel, pnContrast);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_GetContrast fail");
        return FALSE;
    }

    return TRUE;
}

// 8. 获得采集器可采集图像的宽度、高度的最大值
BOOL CHX_DevImpl::Cap_GetMaxImageSize(int nChannel,int* pnWidth, int* pnHeight)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetMaxImageSize(nChannel, pnWidth, pnHeight);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_GetMaxImageSize fail");
        return FALSE;
    }

    return TRUE;
}

// 9. 获得采集器当前图像的采集位置、宽度和高度
BOOL CHX_DevImpl::Cap_GetCaptWindow(int nChannel,int* pnOriginX, int* pnOriginY,int* pnWidth, int* pnHeight)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetCaptWindow(nChannel, pnOriginX, pnOriginY, pnWidth, pnHeight);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_GetCaptWindow fail");
        return FALSE;
    }

    return TRUE;
}

// 10. 设置采集器当前图像的采集位置、宽度和高度
BOOL CHX_DevImpl::Cap_SetCaptWindow(int nChannel,int nOriginX, int nOriginY,int nWidth, int nHeight)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetCaptWindow(nChannel, &nOriginX, &nOriginY, &nWidth, &nHeight);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_GetCaptWindow fail");
        return FALSE;
    }

    return TRUE;
}

// 11. 调用采集器的属性设置对话框
BOOL CHX_DevImpl::Cap_Setup()
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_Setup();
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_Setup fail");
        return FALSE;
    }

    return TRUE;
}

// 12. 准备采集一帧图像
BOOL CHX_DevImpl::Cap_BeginCapture(int nChannel)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_BeginCapture(nChannel);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_BeginCapture fail");
        return FALSE;
    }

    return TRUE;
}

// 13. 采集一帧图像
BOOL CHX_DevImpl::Cap_GetFPRawData(int nChannel,unsigned char* pRawData)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetFPRawData(nChannel, pRawData);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_GetFPRawData fail, ret: %d", nRet);
        return FALSE;
    }

    return TRUE;
}

// 14. 采集一帧 BMP 格式图像
BOOL CHX_DevImpl::Cap_GetFPBmpData(int nChannel, unsigned char *pBmpData)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetFPBmpData(nChannel, pBmpData);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_GetFPBmpData fail");
        return FALSE;
    }

    return TRUE;
}

// 15. 结束采集一帧图像
BOOL CHX_DevImpl::Cap_EndCapture(int nChannel)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_EndCapture(nChannel);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_EndCapture fail");
        return FALSE;
    }

    return TRUE;
}

// 16. 采集器是否支持设置对话框
BOOL CHX_DevImpl::Cap_isSupportSetup()
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_IsSupportSetup();
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_isSupportSetup fail");
        return FALSE;
    }

    return TRUE;
}

// 17. 取得接口规范的版本
BOOL CHX_DevImpl::Cap_GetVersion()
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetVersion();
    if (nRet < 0)
    {
        Log(ThisFile, 1, "LIVESCAN_GetVersion fail");
        return FALSE;
    }

    return nRet;
}

// 18. 获得接口规范的说明
BOOL CHX_DevImpl::Cap_GetDesc(char pszDesc[1024])
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_GetDesc(pszDesc);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_GetDesc fail");
        return FALSE;
    }

    return TRUE;
}

// 19. 获得采集接口错误信息
LPSTR CHX_DevImpl::Cap_GetErrorInfo(int nErrorNo)
{
    int nRet = 0;
    char pszErrorInfo[256] = {0};

    if (!LoadDll())
        return nullptr;

    nRet = m_pLIVESCAN_GetErrorInfo(nErrorNo, pszErrorInfo);
    if (nRet == 1 || nRet == -6)
        return pszErrorInfo;
    else
    {
        Log(ThisFile, 1, "LIVESCAN_GetErrorInfo fail");
        return nullptr;
    }
}

// 20. 设置存放采集数据的内存块为空
BOOL CHX_DevImpl::Cap_setBufferEmpty(unsigned char *pImageData, long imageLength)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pLIVESCAN_SetBufferEmpty(pImageData, imageLength);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "LIVESCAN_setBufferEmpty fail");
        return FALSE;
    }

    return TRUE;
}

// Fpr.so
// 1. 版本信息获取
BOOL CHX_DevImpl::Fpr_GetVersion(BYTE pszCode[4])
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPGetVersion(pszCode);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_GetVersion Fail.");
        return FALSE;
    }

    return TRUE;
}

// 2. 初始化操作
BOOL CHX_DevImpl::Fpr_Begin()
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPBegin();
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_Begin Fail. ErrorInfo : %s", Cap_GetErrorInfo(nRet));
        return FALSE;
    }

    return TRUE;
}

// 3. 1 枚指纹图像特征提取
BOOL CHX_DevImpl::Fpr_FeatureExtract(BYTE cScannerType, BYTE cFingerCode, LPBYTE pFingerImgBuf, LPBYTE pFeatureData)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPFeatureExtract(cScannerType, cFingerCode, pFingerImgBuf, pFeatureData);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_FeatureExtract Fail.");
        return FALSE;
    }

    return TRUE;
}

//4. 对 2 个指纹特征数据进行比对
BOOL CHX_DevImpl::Fpr_FeatureMatch(LPBYTE pFeatureData1, LPBYTE pFeatureData2, float * pfSimilarity)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPFeatureMatch(pFeatureData1, pFeatureData2, pfSimilarity);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_FeatureMatch Fail.");
        return FALSE;
    }

    return TRUE;
}

// 5. 对指纹图像和指纹特征进行比对
BOOL CHX_DevImpl::Fpr_ImageMatch(LPBYTE pFingerImgBuf, LPBYTE pFeatureData, float * pfSimilarity)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPImageMatch(pFingerImgBuf, pFeatureData, pfSimilarity);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_ImageMatch Fail.");
        return FALSE;
    }

    return TRUE;
}

// 6. 对指纹图像数据进行压缩
BOOL CHX_DevImpl::Fpr_Compress(BYTE cScannerType, BYTE cEnrolResult, BYTE cFingerCode, LPBYTE pFingerImgBuf, int nCompressRatio, LPBYTE pCompressedImgBuf, BYTE strBuf[256])
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPCompress(cScannerType, cEnrolResult, cFingerCode, pFingerImgBuf, nCompressRatio, pCompressedImgBuf, strBuf);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_Compress Fail.");
        return FALSE;
    }

    return TRUE;
}

// 7. 对指纹图像压缩数据进行复现
BOOL CHX_DevImpl::Fpr_Decompress(LPBYTE pCompressedImgBuf, LPBYTE pFingerImgBuf, BYTE strBuf[256])
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPDecompress(pCompressedImgBuf, pFingerImgBuf, strBuf);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_Decompress Fail.");
        return FALSE;
    }

    return TRUE;
}

// 8. 获取指纹图像的质量值
BOOL CHX_DevImpl::Fpr_GetQualityScore(LPBYTE pFingerImgBuf, LPBYTE bScore)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPGetQualityScore(pFingerImgBuf, bScore);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_GetQualityScore Fail.");
        return FALSE;
    }

    return TRUE;
}

// 9. 生成"未注册"指纹特征数据
BOOL CHX_DevImpl::Fpr_GenFeatureFromEmpty1(BYTE cScannerType, BYTE cFingerCode, LPBYTE pFeatureData)
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPGenFeatureFromEmpty1(cScannerType, cFingerCode, pFeatureData);
    if (nRet != 1)
    {
        Log(ThisFile, 1, "FP_GenFeatureFromEmpty1 Fail.");
        return FALSE;
    }

    return TRUE;
}

// 10. 结束操作
BOOL CHX_DevImpl::Fpr_End()
{
    int nRet = 0;

    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;

    nRet = m_pFPEnd();
    if (nRet != 1)
    {
        Log(ThisFile, 1, "Fp_End Fail.");
        return FALSE;
    }

    return TRUE;
}
