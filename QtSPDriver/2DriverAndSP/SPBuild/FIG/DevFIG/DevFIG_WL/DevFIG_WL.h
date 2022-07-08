#pragma once

#include "IDevFIG.h"
#include "QtTypeInclude.h"
#include "DevFIG_WL.h"
#include "DevImpl_WL.h"


class CDevFIG_WL : public IDevFIG, public CDevImpl_WEL401
{
public:
    CDevFIG_WL(LPCSTR lpDevType);
    virtual ~CDevFIG_WL();
public:
    virtual int Release();
    // 打开与设备的连接
    virtual int Open();
    // 关闭与设备的连接
    virtual void Close();
    // 设备初始化
    virtual int Init();
    // 获取采集接口版本
    virtual int GetVersion(STFPRVERSION &stFprVersion);
    // 获取指纹模板
    virtual int GetFingerTemplateData(LPBYTE pData, LPBYTE pFeature, int *npLen);
    // 存储指纹图片
    virtual int SaveFingerImage(const char* pImgPath, const char* pImgBuf);
    // 提取指纹特征
    virtual int FeatureExtract(LPBYTE pFingerImgBuf, LPBYTE pFingerBmpBuf, LPBYTE pFeatureData, int *nImgLen, DWORD dwTimeOut);
    // 比对指纹特征
    virtual int CompareFPData(LPBYTE pFeatureData1, LPBYTE pFeatureData2);
    // 检查是否有手指按捺
    virtual int ChkFingerPressed();


private:
    int                             m_pnTimeOut;
    CHAR                            m_pszDevName[MAX_PATH];     // 识别的路径
    BOOL                            m_bOpen = FALSE;

    std::string                     m_strMode;
    CSimpleMutex                    m_MutexAction;
    BYTE                            m_ppTemplateData[3][FIG_IMAGE_BMP_DATA_SIZE];// 保存指纹RAW数据,三合一模板数据
    WORD                            m_wBuildModelPressedTimes;    // 模板按奈次数
    WORD                            m_wBuildModelFailTimes;       // 建模失败次数
    CSimpleMutex                    m_cMutex;
};
