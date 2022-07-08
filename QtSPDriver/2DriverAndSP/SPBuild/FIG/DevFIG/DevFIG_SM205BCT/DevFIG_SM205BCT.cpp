#include "DevFIG_SM205BCT.h"

static const char *ThisFile = "DevFIG_TCM042.cpp";
//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////
CDevFIG_SM205BCT::CDevFIG_SM205BCT(LPCSTR lpDevType)
{
    m_nPort = 1;                                    // 端口号0=USB,1=串口1, 2=串口2, ...
    ZeroMemory(m_pszDevName, MAX_PATH);
    ZeroMemory(m_pszDevSN, MAX_EXT);
    SetLogFile(LOGFILE, ThisFile, lpDevType);
}

CDevFIG_SM205BCT::~CDevFIG_SM205BCT()
{

}

int CDevFIG_SM205BCT::Release()
{

}

int CDevFIG_SM205BCT::Open()
{
    int nRet = 0;

    nRet = FPIGetDevSN(m_nPort, (LPSTR)m_pszDevSN);
    if (nRet == -1)
        return ERR_FIG_NOT_OPEN;
    else if (nRet == 0)
        return ERR_FIG_SUCCESS;
    else
        return nRet;
}

void CDevFIG_SM205BCT::Close()
{
    FPICancel();
}

int CDevFIG_SM205BCT::Init()
{
    return ERR_FIG_SUCCESS;
}

int CDevFIG_SM205BCT::GetVersion(STFPRVERSION &stFprVersion)
{
    int nRet = 0;
    nRet = FPIGetDevVersion(m_nPort, stFprVersion.pszDesc);
    if (nRet != 0)
        return ERR_FIG_OTHER;

    return ERR_FIG_SUCCESS;
}

int CDevFIG_SM205BCT::GetFingerTemplateData(LPBYTE pData, LPBYTE pFeature, int *npLen)
{
    return ERR_FIG_SUCCESS;
}

int CDevFIG_SM205BCT::SaveFingerImage(const char *pImgPath, const char *pImgBuf)
{

}


int CDevFIG_SM205BCT::FeatureExtract(LPBYTE pFingerImgBuf, LPBYTE pFingerBmpBuf, LPBYTE pFeatureData, int *nImgLen, DWORD dwTimeOut)
{
    int nRet = 0;

    // 因接口不支持循环检测手指，故先销毁再打开设备
//    FPICancel();

//    nRet = FPIGetDevSN(m_nPort, (LPSTR)m_pszDevSN);
//    if (nRet < 0)
//        return ERR_FIG_NOT_OPEN;

//    nRet = FPIDetectFinger(m_nPort);
//    if (nRet == 0)
    {
        Log(ThisFile, 0, "SM205: FeatureExtracting, Port is %d", m_nPort);
        nRet = FPIGetFeature(m_nPort, dwTimeOut, (LPSTR)pFeatureData, pFingerImgBuf, nImgLen);
        if (nRet == -1)
        {
            Log(ThisFile, 0, "SM205: FPIGetFeature fail. not open. nRet: %d, ImgLen: %d", nRet, *nImgLen);
            return ERR_FIG_NOT_OPEN;
        }
        else if (nRet == -2 || nRet == -3)
        {
            Log(ThisFile, 0, "SM205: FPIGetFeature fail. 可能是手指按耐方式不标准. nRet: %d, ImgLen: %d", nRet, *nImgLen);
            return ERR_FIG_SUCCESS;
        }
        else if (nRet < -3 || (*nImgLen) <= 0)
        {
            Log(ThisFile, 0, "SM205: FPIGetFeature fail. nRet: %d, ImgLen: %d", nRet, *nImgLen);
            return  ERR_FIG_OTHER;
        }

        nRet = FPIGetImageBmp(pFingerImgBuf, pFingerBmpBuf);
        if (nRet < 0)
        {
            Log(ThisFile, 0, "SM205: FPIGetImageBmp fail. nRet: %d, ImgLen: %d", nRet, *nImgLen);
            return ERR_FIG_NOT_OPEN;
        }

        return ERR_FIG_SUCCESS;
    }


//    Log(ThisFile, 0, "SM205: FPIDetectFinger fail. nRet: %d", nRet);
//    return ERR_FIG_OTHER;
}

int CDevFIG_SM205BCT::CompareFPData(LPBYTE pFeatureData1, LPBYTE pFeatureData2)
{
    int nRet = 0;

    nRet = FPIMatch((LPSTR)pFeatureData1, (LPSTR)pFeatureData2, 3);
    if (nRet < 0)
    {
        Log(ThisFile, 0, "match fail. Ret: %d, FeaData1 Len: %d, FeaData2 Len: %d", nRet, strlen((LPCSTR)pFeatureData1), strlen((LPCSTR)pFeatureData2));
        return ERR_FIG_HWERR;
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_SM205BCT::ChkFingerPressed()
{
    int nRet = 0;

    nRet = FPIDetectFinger(m_nPort);
    return nRet;
}
