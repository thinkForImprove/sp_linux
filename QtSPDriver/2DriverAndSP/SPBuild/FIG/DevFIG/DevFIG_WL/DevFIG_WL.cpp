#include "DevFIG_WL.h"
#include "file_access.h"
static const char *ThisFile = "DevFIG_WL.cpp";

#define FPBMP_TEMP_PATH  "/tmp/fp_wl.bmp"

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////
CDevFIG_WL::CDevFIG_WL(LPCSTR lpDevType)
{
    m_pnTimeOut = 10;
    ZeroMemory(m_pszDevName, MAX_PATH);
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    m_wBuildModelPressedTimes = 0;
    m_wBuildModelFailTimes = 0;
    for (int i = 0; i < 3; i++)
    {
        memset(m_ppTemplateData[i], 0x00, FIG_IMAGE_BMP_DATA_SIZE);
    }
}

CDevFIG_WL::~CDevFIG_WL()
{

}

int CDevFIG_WL::Release()
{

}

int CDevFIG_WL::Open()
{
    CAutoMutex autoMutex(&m_cMutex);
    Log(ThisFile,1,"WL");
    int nRet = 0;
    nRet = FPIDeviceInit();

    if (nRet != TRUE)
        return ERR_FIG_NOT_OPEN;
    m_bOpen = TRUE;
    return ERR_FIG_SUCCESS;
}

void CDevFIG_WL::Close()
{   
    CAutoMutex autoMutex(&m_cMutex);
    int nRet = 0;

    nRet = FPIDeviceClose();
    Log(ThisFile,1,"FPIDeviceClose   ret = %d", nRet);
    m_bOpen = FALSE;
}

int CDevFIG_WL::Init()
{
    return ERR_FIG_SUCCESS;
}


int CDevFIG_WL::GetVersion(STFPRVERSION &stFprVersion)
{
    if (!m_bOpen)
    {
        Log(ThisFile, 1, "Dev not init");
        return ERR_FIG_NOT_OPEN;
    }

    CAutoMutex autoMutex(&m_cMutex);
    int len = 0;
//    int nRet = FPIGetDeviceID(stFprVersion.pszDesc);
    int nRet = FPIGetVersion(stFprVersion.pszDesc,&len);
    if (nRet < 0)
    {
        Log(ThisFile, 1, "GetVersion Fail");
        return ERR_FIG_OTHER;
    }
    return ERR_FIG_SUCCESS;
}

int CDevFIG_WL::GetFingerTemplateData(LPBYTE pData, LPBYTE pFeature, int *npLen)
{
    int nRet = -5;
    int len = 0;

    Log(ThisFile, 0, "第 [%02d] 次执行 GetFingerTemplateData ... ", m_wBuildModelPressedTimes + 1);
    if (!m_bOpen)
    {
        nRet = FPIDeviceInit();
        if (nRet != TRUE)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    nRet = FPIFeature(m_pnTimeOut, pData, &len);
    if (nRet != 0)
    {
        m_wBuildModelFailTimes++;
        if (m_wBuildModelFailTimes > 3)
        {
            Log(ThisFile, 0, "第 [%02d] 次采集指纹 TZ 错误, 重新建模失败, 失败次数: %d, ErrorCode: %d", m_wBuildModelPressedTimes + 1, m_wBuildModelFailTimes, nRet);
            m_wBuildModelFailTimes = 0;        // 重新建模次数清零
            m_wBuildModelPressedTimes = 0;     // 建模次数清零
            memset(m_ppTemplateData, 0x00, sizeof(m_ppTemplateData));
            return ERR_FIG_OTHER;
        }
        Log(ThisFile, 0, "第 [%02d] 次采集指纹 TZ 错误，模板合成失败, 重新采集此次指纹, ErrorCode: %d", m_wBuildModelPressedTimes + 1, nRet);
        return ERR_FIG_CAN_RESUME;
    }

    Log(ThisFile, 0, "第 [%02d] 次采集指纹 TZ 成功, 指纹RAW数据的长度: %d", m_wBuildModelPressedTimes + 1, len);

    if (m_wBuildModelPressedTimes < 3)
        memcpy(m_ppTemplateData[m_wBuildModelPressedTimes], pData, len);

    m_wBuildModelPressedTimes++;

    // 第三次合成模板
    if (m_wBuildModelPressedTimes > 3)
    {
        Log(ThisFile, 0, "第 [%02d] 次采集指纹,成功采集次数超过3次, 建模失败", m_wBuildModelPressedTimes);
        return ERR_FIG_OTHER;
    }
    else if (m_wBuildModelPressedTimes == 3)
    {
        m_wBuildModelFailTimes = 0;
        m_wBuildModelPressedTimes = 0;
        nRet = FPIGetTemplateByTZ((LPSTR)m_ppTemplateData[0], (LPSTR)m_ppTemplateData[1], (LPSTR)m_ppTemplateData[2], (LPSTR)pFeature, &len);
        if (nRet < 0 || len <= 0)
        {
            Log(ThisFile, 0, "模板合成失败, 指纹模板数据的长度: %d, ErrorCode: %d", len, nRet);
            return nRet;
        }

        memset(m_ppTemplateData, 0x00, sizeof(m_ppTemplateData));

        npLen = &len;
        Log(ThisFile, 0, "指纹建模　success, 指纹模板数据的长度: %d", len);
    }

    return ERR_FIG_SUCCESS;
}

int CDevFIG_WL::SaveFingerImage(const char *pImgPath, const char *pImgBuf)
{
    Log(ThisFile, 1, "start SaveFingerImage");

#if 1
    int nRet = 0;

    if(!FileAccess::create_directory_by_path(pImgPath, true))
    {
       Log(ThisFile,1,"[%s] mkdir fail", FPBMP_TEMP_PATH);
       return -1;
    }
    QFile file(FPBMP_TEMP_PATH);
    if (!file.exists())
    {
        Log(ThisFile,1,"[%s] not exist", FPBMP_TEMP_PATH);
        return ERR_FIG_HWERR;
    }
    QFile OldFile(pImgPath);
    if (OldFile.exists())
    {
        Log(ThisFile,-1,"delete old file %s",pImgPath);
        OldFile.remove();
    }
    if (!file.rename(pImgPath))
    {
        Log(ThisFile,1,"[%s] rename to [%s] failed", FPBMP_TEMP_PATH, pImgPath);
        return ERR_FIG_HWERR;
    }

    return ERR_FIG_SUCCESS;
#else
    int nRet = 0;
    if (!m_bOpen)
    {
        nRet = FPIDeviceInit();
        if (nRet != TRUE)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }
    int nWidth = FIG_IMAGE_WIDTH_INTI_SIZE;
    int nHeight = FIG_IMAGE_HEIGHT_INIT_SIZE;
    Log(ThisFile,1,"nREt = %d, pImgPath = %s",nRet,pImgPath);
    nRet = FPIGetImageBuf(m_pnTimeOut,(LPSTR)pImgPath,&nWidth,&nHeight,(LPBYTE)pImgBuf);
    Log(ThisFile,1,"nREt = %d, pImgPath = %s",nRet,pImgPath);
    if(nRet < 0)
    {
        Log(ThisFile, 0, "保存指纹图片失败, 接口返回:<%d>, 图片路径:<%s>", nRet, pImgPath);
        return ERR_FIG_HWERR;
    }
    return ERR_FIG_SUCCESS;

#endif
}

int CDevFIG_WL::FeatureExtract(LPBYTE pFingerImgBuf, LPBYTE pFingerBmpBuf, LPBYTE pFeatureData, int *nImgLen, DWORD dwTimeOut)
{
    int nRet = 0;
    int len = 0;
    int pnImageLen = 0;

    ULONG dwStart = CQtTime::GetSysTick();

    if(!m_bOpen)
    {
        nRet = FPIDeviceInit();
        if (nRet != TRUE)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    Log(ThisFile, 0, "开始执行 %s ... ", __FUNCTION__);

    nRet = FPIGetFeatureAndImage(m_pnTimeOut, (char *)pFeatureData, &len, (char *)pFingerImgBuf, &pnImageLen, FPBMP_TEMP_PATH);
    if (((CQtTime::GetSysTick() - dwStart) > (m_pnTimeOut * 1000)))
    {
        Log(ThisFile, 0, "采集指纹 TZ 超时, 指纹RAW数据的长度: %d, ErrorCode: %d", len, nRet);
        return ERR_FIG_TIMEOUT;
    }
    if (nRet < 0 || len <= 0)
    {
        Log(ThisFile, 0, "采集指纹失败, 指纹RAW数据的长度: %d, ErrorCode: %d", len, nRet);
        return nRet;
    }



    nImgLen = &len;
    Log(ThisFile, 0, "采集指纹 TZ 成功, 指纹RAW数据的长度: %d", len);
    return ERR_FIG_SUCCESS;
}

int CDevFIG_WL::CompareFPData(LPBYTE pFeatureData1, LPBYTE pFeatureData2)
{
    int nRet = 0;

    Log(ThisFile, 0, "开始执行 CompareFPData ... ");

    if(!m_bOpen)
    {
        nRet = FPIDeviceInit();
        if (nRet != TRUE)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }

    nRet = FPIFpMatch((LPSTR)pFeatureData1,(LPSTR)pFeatureData2,3);
    if(nRet < 0)
    {
        Log(ThisFile, 0, "match fail. %d", nRet);
        return nRet;
    }

    Log(ThisFile, 0, "match ok .");

    return ERR_FIG_SUCCESS;
}

int CDevFIG_WL::ChkFingerPressed()
{
    CAutoMutex autoMutex(&m_cMutex);
    int nRet = 0;
    if(!m_bOpen)
    {
        nRet = FPIDeviceInit();
        if (nRet != TRUE)
            return ERR_FIG_NOT_OPEN;
        m_bOpen = TRUE;
    }
    nRet = FPICheckFinger();
    if( nRet == 0)
        return 0;
    else if(nRet == 1)
        return 1;
    else
        return nRet;
}
