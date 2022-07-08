#pragma once

#include "IDevFIG.h"
#include "QtTypeInclude.h"
#include "DevImpl_TCM042.h"

class CDevFIG_TCM042 : public IDevFIG, public CDevImpl_TCM042
{
public:
    CDevFIG_TCM042(LPCSTR lpDevType);
    virtual ~CDevFIG_TCM042();
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
    // 采集指纹数据
    virtual int CollectFPData();
    // 提取指纹特征
    virtual int FeatureExtract(LPBYTE pFingerImgBuf, LPBYTE pFingerBmpBuf, LPBYTE pFeatureData, int *nImgLen, DWORD dwTimeOut);
    // 比对指纹特征
    virtual int CompareFPData(LPBYTE pFeatureData1, LPBYTE pFeatureData2);
    // 检查是否有手指按捺
    virtual int ChkFingerPressed();

private:
    int                             m_nPort;                    // 接口入参固定值0
    CHAR                            m_pszDevName[MAX_PATH];     // 识别的路径

    std::string                     m_strMode;
    CSimpleMutex                    m_MutexAction;
};
