#include "DevFIG_TCM042.h"

static const char *ThisFile = "DevFIG_TCM042.cpp";
//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////
CDevFIG_TCM042::CDevFIG_TCM042(LPCSTR lpDevType)
{
    m_nPort = 0;                                    // 接口入参固定值0
    ZeroMemory(m_pszDevName, MAX_PATH);
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    m_wBuildModelPressedTimes = 0;
    m_wBuildModelFailTimes = 0;
    for (int i = 0; i < 3; i++)
    {
        memset(m_ppTemplateData[i], 0x00, FIG_IMAGE_BMP_DATA_SIZE);
    }
}

CDevFIG_TCM042::~CDevFIG_TCM042()
{

}

int CDevFIG_TCM042::Release()
{

}

int CDevFIG_TCM042::Open()
{
    int nRet = 0;

    nRet = FPIFindDevice(m_nPort, (LPSTR)m_pszDevName);
    if (nRet < 0)
        return ERR_FIG_NOT_OPEN;

    m_bOpen = TRUE;
    return ERR_FIG_SUCCESS;
}

void CDevFIG_TCM042::Close()
{
    m_bOpen = FALSE;
}

int CDevFIG_TCM042::Init()
{
    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::GetVersion(STFPRVERSION &stFprVersion)
{
    int nRet = 0;

    if (!m_bOpen)
    {
        nRet = FPIFindDevice(m_nPort, (LPSTR)m_pszDevName);
        if (nRet < 0)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    nRet = FPIGetDevSN(m_nPort, stFprVersion.pszDesc);
    if (nRet < 0)
        return ERR_FIG_HWERR;

    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::GetFingerTemplateData(LPBYTE pData, LPBYTE pFeature, int *npLen)
{
    int nRet = 0;
    int len = 0;

    Log(ThisFile, 0, "第%02d次执行 GetFingerTemplateData ... ", m_wBuildModelPressedTimes + 1);
    if (!m_bOpen)
    {
        nRet = FPIFindDevice(m_nPort, (LPSTR)m_pszDevName);
        if (nRet < 0)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    nRet = FPIGetFeature(m_nPort, (LPSTR)pFeature, (LPSTR)pData, &len);
    if (nRet < 0)
    {
        m_wBuildModelFailTimes++;
        if (m_wBuildModelFailTimes > 3)
        {
            Log(ThisFile, 0, "第%02d次采集指纹错误, 重新建模失败, 失败次数: %d, ErrorCode: %d", m_wBuildModelPressedTimes + 1, m_wBuildModelFailTimes, nRet);
            m_wBuildModelFailTimes = 0;        // 重新建模次数清零
            m_wBuildModelPressedTimes = 0;     // 建模次数清零
            for (int i = 0; i < 3; i++)
            {
                memset(m_ppTemplateData[i], 0x00, FIG_IMAGE_BMP_DATA_SIZE);
            }
            return ERR_FIG_OTHER;
        }
        Log(ThisFile, 0, "第%02d次采集指纹错误，模板合成失败, 重新采集此次指纹, ErrorCode: %d", m_wBuildModelPressedTimes + 1, nRet);
        return ERR_FIG_CAN_RESUME;
    }

    Log(ThisFile, 0, "第%02d次采集指纹成功, 指纹RAW数据的长度: %d", m_wBuildModelPressedTimes + 1, len);
    if (m_wBuildModelPressedTimes < 3)
        memcpy(m_ppTemplateData[m_wBuildModelPressedTimes], pData, len);
    m_wBuildModelPressedTimes++;

    // 第三次合成模板
    if (m_wBuildModelPressedTimes > 3)
    {
        Log(ThisFile, 0, "第%02d次采集指纹,成功采集次数超过3次, 建模失败", m_wBuildModelPressedTimes);
        return ERR_FIG_OTHER;
    } else if (m_wBuildModelPressedTimes == 3) {
        m_wBuildModelFailTimes = 0;
        m_wBuildModelPressedTimes = 0;
        nRet = FPITplFrmImg((LPSTR)m_ppTemplateData[0], (LPSTR)m_ppTemplateData[1], (LPSTR)m_ppTemplateData[2], (LPSTR)pFeature, &len);
        if (nRet < 0 || len <= 0)
        {
            Log(ThisFile, 0, "模板合成失败, 指纹模板数据的长度: %d, ErrorCode: %d", len, nRet);
            return nRet;
        }
        for (int i = 0; i < 3; i++)
        {
            memset(m_ppTemplateData[i], 0x00, FIG_IMAGE_BMP_DATA_SIZE);
        }
        npLen = &len;
        Log(ThisFile, 0, "建模完成, 指纹模板数据的长度: %d", len);
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::SaveFingerImage(const char *pImgPath, const char *pImgBuf)
{
    int nRet = 0;

    if (!m_bOpen)
    {
        nRet = FPIFindDevice(m_nPort, (LPSTR)m_pszDevName);
        if (nRet < 0)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    nRet = FPISaveImage(pImgPath, pImgBuf);
    if (nRet < 0)
    {
        Log(ThisFile, 0, "保存指纹图片失败, FPISaveImage接口返回:<%d>, 图片路径:<%s>", nRet, pImgPath);
        return ERR_FIG_HWERR;
    }

    return ERR_FIG_SUCCESS;
}


int CDevFIG_TCM042::FeatureExtract(LPBYTE pFingerImgBuf, LPBYTE pFingerBmpBuf, LPBYTE pFeatureData, int *nImgLen, DWORD dwTimeOut)
{
    int nRet = 0;
    int len = 0;
    ULONG dwStart = CQtTime::GetSysTick();

    Log(ThisFile, 0, "开始执行 FeatureExtract ... ");

    if (!m_bOpen)
    {
        nRet = FPIFindDevice(m_nPort, (LPSTR)m_pszDevName);
        if (nRet < 0)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    nRet = FPIGetFeature(m_nPort, (LPSTR)pFeatureData, (LPSTR)pFingerImgBuf, &len);
    if (nRet < 0 || len <= 0)
    {
        Log(ThisFile, 0, "采集指纹失败, 指纹RAW数据的长度: %d, ErrorCode: %d", len, nRet);
        return ERR_FIG_HWERR;
    }

    if (CQtTime::GetSysTick() - dwStart > dwTimeOut)
    {
        Log(ThisFile, 0, "采集指纹超时, 指纹RAW数据的长度: %d, ErrorCode: %d", len, nRet);
        return ERR_FIG_TIMEOUT;
    }

    nImgLen = &len;
    Log(ThisFile, 0, "采集指纹成功, 指纹RAW数据的长度: %d", len);
    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::CompareFPData(LPBYTE pFeatureData1, LPBYTE pFeatureData2)
{
    int nRet = 0;

    Log(ThisFile, 0, "开始执行 CompareFPData ... ");

    if (!m_bOpen)
    {
        nRet = FPIFindDevice(m_nPort, (LPSTR)m_pszDevName);
        if (nRet < 0)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    nRet = FPIFpMatch((LPSTR)pFeatureData1, (LPSTR)pFeatureData2, 3);
    if (nRet < 0)
    {
        Log(ThisFile, 0, "match fail.");
        return ERR_FIG_HWERR;
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::ChkFingerPressed()
{
    int nRet = 0;

    if (!m_bOpen)
    {
        nRet = FPIFindDevice(m_nPort, (LPSTR)m_pszDevName);
        if (nRet < 0)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    nRet = FPIChkPressed();
    if (nRet == -5)
        return 1;
    else if (nRet == 0x76dc)
        return 0;
    else if (nRet == -15)
        return ERR_FIG_NOT_OPEN;
    else
        return nRet;

}






