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

    return ERR_FIG_SUCCESS;
}

void CDevFIG_TCM042::Close()
{
}

int CDevFIG_TCM042::Init()
{
    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::GetVersion(STFPRVERSION &stFprVersion)
{
    int nRet = 0;
    nRet = FPIGetDevSN(m_nPort, stFprVersion.pszDesc);
    if (nRet < 0)
        return ERR_FIG_HWERR;

    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::CollectFPData()
{

    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::FeatureExtract(LPBYTE pFingerImgBuf, LPBYTE pFingerBmpBuf, LPBYTE pFeatureData, int *nImgLen, DWORD dwTimeOut)
{
    int nRet = 0;
    char szBuf[1024] = {0};

    if ((*nImgLen) != 0)
    {
        nRet = FPIGetFeature(m_nPort, (LPSTR)pFeatureData, (LPSTR)pFingerImgBuf, nImgLen);
        if (nRet < 0 || (*nImgLen) <= 0)
        {
            for (DWORD i = 0; i < 685; i++)
            {
                sprintf(szBuf + i, "%02.02X ", (BYTE)(pFeatureData[i]));
            }

            Log(ThisFile, 0, "FPIGetFeature fail. ImgLen: %d, Port: %d, FeatureData: %02.02X ",
                (*nImgLen), m_nPort, szBuf);

            return  ERR_FIG_HWERR;
        }
    } else
    {
        nRet = FPIGetFeature(m_nPort, (LPSTR)pFeatureData, (LPSTR)pFingerBmpBuf, nImgLen);
        if (nRet < 0 || (*nImgLen) <= 0)
        {
            for (DWORD i = 0; i < 685; i++)
            {
                sprintf(szBuf + i, "%02.02X ", (BYTE)(pFeatureData[i]));
            }

            Log(ThisFile, 0, "FPIGetFeature fail. ImgLen: %d, Port: %d, FeatureData: %02.02X ",
                (*nImgLen), m_nPort, szBuf);

            return  ERR_FIG_HWERR;
        }
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_TCM042::CompareFPData(LPBYTE pFeatureData1, LPBYTE pFeatureData2)
{
    int nRet = 0;
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

    nRet = FPIChkPressed();
    return nRet;
}






