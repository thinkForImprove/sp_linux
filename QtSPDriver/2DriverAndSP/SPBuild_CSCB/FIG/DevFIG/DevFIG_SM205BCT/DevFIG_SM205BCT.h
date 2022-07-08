#pragma once

#include "IDevFIG.h"
#include "QtTypeInclude.h"
#include "DevImpl_SM205BCT.h"

class CDevFIG_SM205BCT : public IDevFIG, public CDevImpl_SM205BCT
{
public:
    CDevFIG_SM205BCT(LPCSTR lpDevType);
    virtual ~CDevFIG_SM205BCT();
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
    int                             m_nPort;                    // 端口号0=USB,1=串口1, 2=串口2, ...
    CHAR                            m_pszDevName[MAX_PATH];     // 识别的路径
    CHAR                            m_pszDevSN[MAX_EXT];        // 设备序列号

    std::string                     m_strMode;
    CSimpleMutex                    m_MutexAction;
};
