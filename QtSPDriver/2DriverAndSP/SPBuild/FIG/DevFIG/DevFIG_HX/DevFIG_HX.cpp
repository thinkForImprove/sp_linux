#include "DevFIG_HX.h"

static const char *ThisFile = "DevFIG_HX.cpp";
//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////
CDevFIG_HX::CDevFIG_HX(LPCSTR lpDevType)
{
    m_pszRawData = new BYTE[FIG_IMAGE_RAW_DATA_SIZE];
    m_pszBmpData = new BYTE[FIG_IMAGE_BMP_DATA_SIZE];
    m_pszError = new CHAR[MAX_EXT];
    m_bIsCapInit = FALSE;                           // 采集接口是否初始化
    m_bIsFprInit = FALSE;                           // 指纹接口是否初始化
    m_bIsSupportSetup = FALSE;                      // 采集器是否支持设置对话框
    memset(CapVersion, 0, sizeof(CapVersion));      // 采集接口版本
    memset(FprVersion, 0, sizeof(FprVersion));      // 指纹接口版本
    memset(pszDesc, 0, sizeof(pszDesc));            // 接口说明
    s_tPrintData.init();                            // 可采集器相关数据
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    nChannel = 0;
}

CDevFIG_HX::~CDevFIG_HX()
{
    if (m_pszRawData != nullptr)
    {
        delete[] m_pszRawData;
        m_pszRawData = nullptr;
    }

    if (m_pszBmpData != nullptr)
    {
        delete[] m_pszBmpData;
        m_pszBmpData = nullptr;
    }

    if (m_pszError != nullptr)
    {
        delete[] m_pszError;
        m_pszError = nullptr;
    }
}

int CDevFIG_HX::Release()
{

}

int CDevFIG_HX::Open()
{
    return ERR_FIG_SUCCESS;
}

void CDevFIG_HX::Close()
{
    if (m_bIsCapInit)
    {
        Cap_Close();
        m_bIsCapInit = FALSE;
    }

    if (m_bIsFprInit)
    {
        Fpr_End();
        m_bIsFprInit = FALSE;
    }
}

int CDevFIG_HX::Init()
{
    int nRet = Cap_Init();
    if (nRet != TRUE)
    {
        Log(ThisFile, 1, "LIVESCAN_Init Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }
    m_bIsCapInit  = TRUE;

    nRet = Fpr_Begin();
    if (nRet != TRUE)
    {
        Log(ThisFile, 1, "FP_Begin Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    m_bIsFprInit = TRUE;

    nRet = Cap_isSupportSetup();
    if (nRet < 0)
    {
        Log(ThisFile, 1, "LIVESCAN_isSupportSetup Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }
    else if (nRet == 0)
    {
        m_bIsSupportSetup = FALSE;
    }
    else
    {
        m_bIsSupportSetup = TRUE;
    }

    nRet = GetChannelCount();
    if (nRet < 0)
    {
        Log(ThisFile, 1, "LIVESCAN_GetChannelCount Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    nChannel = nRet;

    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::GetChannelCount()
{
    if (!m_bIsCapInit)
    {
        Log(ThisFile, 1, "FprCap Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    int nRet = Cap_GetChannelCount();
    if (nRet < 0)
    {
        Log(ThisFile, 1, "LIVESCAN_GetChannelCount Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    return nRet;
}

int CDevFIG_HX::GetVersion(STFPRVERSION &stFprVersion)
{
    if (!m_bIsCapInit)
    {
        Log(ThisFile, 1, "FprCap Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    // 取得接口规范的版本
    int nRet = Cap_GetVersion();
    if (nRet < 0)
    {
        Log(ThisFile, 1, "LIVESCAN_GetVersion Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    if ((nRet % 100) > 0)
    {
        stFprVersion.dCapVersion = 1.01;
    }

    stFprVersion.pszDesc[0] = 0xFF;    // 获得接口规范的说明
    nRet = Cap_GetDesc(stFprVersion.pszDesc);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "LIVESCAN_GetDesc Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    // 获取指纹接口版本
    BYTE pszVersion[4] = {0};
    nRet = Fpr_GetVersion(pszVersion);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "FP_GetVersion Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    memcpy(stFprVersion.FprVersion, pszVersion, sizeof(pszVersion));

    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::GetFingerTemplateData(LPBYTE pData, LPBYTE pFeature, int *npLen)
{
    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::SaveFingerImage(const char *pImgPath, const char *pImgBuf)
{

}

int CDevFIG_HX::CollectFPData()
{
    if (!m_bIsCapInit)
    {
        Log(ThisFile, 1, "FprCap Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    int nRet = -1;
    // 准备采集一帧图像
    nRet = Cap_BeginCapture(nChannel);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    // 采集一帧图像
    nRet = Cap_GetFPRawData(nChannel, m_pszRawData);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "GetFPRawData fail. ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

//    // 采集一帧 BMP 格式图像
    nRet = Cap_GetFPBmpData(nChannel, m_pszBmpData);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    // 结束采集一帧图像
    nRet = Cap_EndCapture(nChannel);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
        return ERR_FIG_OTHER;
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::FeatureExtract(LPBYTE pFingerImgBuf, LPBYTE pFingerBmpBuf, LPBYTE pFeatureData, int *nImgLen, DWORD dwTimeOut)
{
    int nRet = 0;
    BYTE bScore = 0x00;

    if (!m_bIsFprInit)
    {
        nRet = Fpr_Begin();
        if (nRet < 0)
        {
            Log(ThisFile, 1, "FP_Begin Fail, ErrCode :%d, ErrInfo :%s", nRet, GetErrorInfo(nRet));
            return ERR_FIG_OTHER;
        }
        m_bIsFprInit = TRUE;
    }

    if ((*nImgLen) != 0)
    {
        memcpy(pFingerImgBuf, m_pszRawData, *nImgLen);
        if (pFingerImgBuf == nullptr)
            return ERR_FIG_OTHER;

        memcpy(pFingerBmpBuf, m_pszBmpData, FIG_IMAGE_BMP_DATA_SIZE);
        if (pFingerBmpBuf == nullptr)
            return ERR_FIG_OTHER;

        // 1 枚指纹图像特征提取
        nRet = Fpr_FeatureExtract(0/*0x62*/, 0/*0x10*/, pFingerImgBuf, pFeatureData);
        if (nRet < 0)
        {
            Log(ThisFile, 1, "");
            return ERR_FIG_OTHER;
        }
    } else
    {
        // 1 枚指纹图像特征提取
        nRet = Fpr_FeatureExtract(0/*0x62*/, 0/*0x10*/, pFingerBmpBuf, pFeatureData);
        if (nRet < 0)
        {
            Log(ThisFile, 1, "");
            return ERR_FIG_OTHER;
        }

        // 获取指纹图像的质量值 < 0x60 : 无效
        nRet = GetQualityScore(pFingerBmpBuf, &bScore);
        if (nRet == 0 && bScore > 60)
        {
            return ERR_FIG_SUCCESS;
        } else
        {
            return ERR_FIG_OTHER;
        }
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::CompareFPData(LPBYTE pFeatureData1, LPBYTE pFeatureData2)
{
    if (!m_bIsFprInit)
    {
        Log(ThisFile, 1, "Fpr Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    float fSimilarity = 0.00;                                       // 匹配相似度值

    // 对 2 个指纹特征数据进行比对，得到匹配相似度值
    int nRet = Fpr_FeatureMatch(pFeatureData1, pFeatureData2, &fSimilarity);
    if (nRet < 0 || fSimilarity < 0.60)
    {
        Log(ThisFile, 1, "");
        return ERR_FIG_OTHER;
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::GetFPPrintData(STPRINTDATA &s_tCurPrintData)
{
    if (!m_bIsCapInit)
    {
        Log(ThisFile, 1, "FprCap Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    // 获得采集器当前的亮度
    int nRet = Cap_GetBright(nChannel, &s_tCurPrintData.nBright);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "FprCap Get Bright Fail, nBright is %d", s_tCurPrintData.nBright);
        return ERR_FIG_OTHER;
    }

    // 获得采集器当前对比度
    nRet = Cap_GetContrast(nChannel, &s_tCurPrintData.nContrast);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "FprCap Get Contrast Fail, nContrast is %d", s_tCurPrintData.nContrast);
        return ERR_FIG_OTHER;
    }

    // 获得采集器可采集图像的宽度、高度的最大值
    nRet = Cap_GetMaxImageSize(nChannel, &s_tPrintData.nWidth, &s_tPrintData.nHeight);
    if (nRet < 0)
    {
         Log(ThisFile, 1, "FprCap Get Max Image Size Fail, Width is %d, Height is %d",
             s_tPrintData.nWidth, s_tPrintData.nHeight);
         return ERR_FIG_OTHER;
    }
    if (s_tPrintData.nWidth < FIG_IMAGE_WIDTH_INTI_SIZE || s_tPrintData.nHeight < FIG_IMAGE_HEIGHT_INIT_SIZE)
    {
        return ERR_FIG_PARAM_ERR;
    }

    // 获得采集器当前图像的采集位置、宽度和高度
    nRet = Cap_GetCaptWindow(nChannel, &s_tCurPrintData.nOriginX, &s_tCurPrintData.nOriginY,
                                         &s_tCurPrintData.nWidth, &s_tCurPrintData.nHeight);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "FprCap Get Capt Window Fail, OriginX is %d, OriginY is %d, Width is %d, Height is %d",
            s_tCurPrintData.nOriginX, s_tCurPrintData.nOriginY, s_tCurPrintData.nWidth, s_tCurPrintData.nHeight);
        return ERR_FIG_OTHER;
    }

    if ((s_tPrintData.nWidth < s_tCurPrintData.nWidth) || (s_tPrintData.nHeight < s_tCurPrintData.nHeight))
    {
        Log(ThisFile, 1, "s_tPrintData < s_tCurPrintData fail.");
        return ERR_FIG_PARAM_ERR;
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::SetFPPrintData(STPRINTDATA stPrintData)
{
    if (!m_bIsCapInit)
    {
        Log(ThisFile, 1, "FprCap Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    // 设置采集器当前的亮度
    if (stPrintData.nBright <= 0 && stPrintData.nBright >= 255)
    {
        return ERR_FIG_PARAM_ERR;
    }

    int nRet = Cap_SetBright(nChannel, stPrintData.nBright);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "FprCap Set Bright Fail, Bright is %d", stPrintData.nBright);
        return ERR_FIG_OTHER;
    }

    // 设置采集器当前对比度
    if (stPrintData.nContrast <= 0 && stPrintData.nContrast >= 255)
    {
        return ERR_FIG_PARAM_ERR;
    }

    nRet = Cap_SetContrast(nChannel, stPrintData.nContrast);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "FprCap Set Contrast Fail, Contrast is %d", stPrintData.nContrast);
        return ERR_FIG_OTHER;
    }

    // 设置采集器当前图像的采集位置、宽度和高度
    if ((stPrintData.nWidth < FIG_IMAGE_WIDTH_INTI_SIZE) || (stPrintData.nHeight < FIG_IMAGE_HEIGHT_INIT_SIZE))
    {
        Log(ThisFile, 1, "Set Current FP Width or Height too small");
        return ERR_FIG_PARAM_ERR;
    }

//    nRet = Cap_SetCaptWindow(nChannel, stPrintData.nOriginX, stPrintData.nOriginY, stPrintData.nWidth, stPrintData.nHeight);
//    if (nRet < 0)
//    {
//        Log(ThisFile, 1, "FprCap Set CaptWindow Fail, OriginX is %d, OriginY is %d, Width is %d, Height is %d",
//            stPrintData.nOriginX, stPrintData.nOriginY, stPrintData.nWidth, stPrintData.nHeight);
//        return ERR_FIG_OTHER;
//    }

    return ERR_FIG_SUCCESS;
}

bool CDevFIG_HX::Setup()
{
    if (!m_bIsCapInit)
    {
        Log(ThisFile, 1, "FprCap Dev not init");
        return false;
    }

    if (m_bIsSupportSetup)
    {
        int nRet = Cap_Setup();
        if (nRet < 0)
        {
            return false;
        }
        return true;
    }
    else
        return false;
}

char* CDevFIG_HX::GetErrorInfo(int nErrorNo)
{
    return m_pszError = Cap_GetErrorInfo(nErrorNo);
}

int CDevFIG_HX::setBufferEmpty(LPBYTE pImageData, long imageLength)
{
    if (!m_bIsCapInit)
    {
        Log(ThisFile, 1, "FprCap Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    int nRet = Cap_setBufferEmpty(pImageData, imageLength);
    if (nRet < 0)
    {
        return ERR_FIG_OTHER;
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::GetQualityScore(LPBYTE pFingerImgBuf, LPBYTE pszScore)
{
    if (!m_bIsFprInit)
    {
        Log(ThisFile, 1, "Fpr Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    int nRet = Fpr_GetQualityScore(pFingerImgBuf, pszScore);
    if (nRet < 0)
    {
        return ERR_FIG_OTHER;
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_HX::ChkFingerPressed()
{
    int nRet = 0;
    BYTE cQualityScore = 0;

    setBufferEmpty(m_pszBmpData, FIG_IMAGE_BMP_DATA_SIZE);
    setBufferEmpty(m_pszRawData, FIG_IMAGE_RAW_DATA_SIZE);

    // 采集一帧图像
    nRet = CollectFPData();
    if (nRet < 0)
    {
        return ERR_FIG_OTHER;
    }

    // 获取指纹图像的质量值 < 0x60 : 无效
    nRet = GetQualityScore(m_pszRawData, &cQualityScore);
    if (nRet < 0)
    {
        return ERR_FIG_OTHER;
    } else if (nRet == 0 && cQualityScore < 60)
    {
        return 1;
    }

    return ERR_FIG_SUCCESS;
}







