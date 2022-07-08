#pragma once

#include "IDevFIG.h"
#include "QtTypeInclude.h"
#include "IAllDevPort.h"
#include "DevFIG_HX/DevImpl_HX.h"
#include "../../XFS_FIG/FIGData.h"
//#include "ID_Fpr.h"
//#include "ID_FprCap.h"
//#include "HS_Errors.h"

class CDevFIG_HX : public IDevFIG, public CHX_DevImpl
{
public:
    CDevFIG_HX(LPCSTR lpDevType);
    virtual ~CDevFIG_HX();
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

protected:
    // 获取通道号
    int GetChannelCount();
    // 设置存放采集数据的内存块为空
    int setBufferEmpty(LPBYTE pImageData, long imageLength);
    // 获取采集器当前的数据
    int GetFPPrintData(STPRINTDATA &s_tCurPrintData);
    // 设置采集器当前的数据
    int SetFPPrintData(STPRINTDATA stPrintData);
    // 调用采集器的属性设置对话框
    bool Setup();
    // 获得采集接口错误信息
    char* GetErrorInfo(int nErrorNo);
    // 获取指纹图像的质量值
    int GetQualityScore(LPBYTE pFingerImgBuf, LPBYTE pszScore);
    // 采集指纹数据
    int CollectFPData();

private:
    STPRINTDATA                     s_tPrintData;       // 可采集器相关数据
    BOOL                            m_bIsCapInit;       // 采集接口是否初始化
    BOOL                            m_bIsFprInit;       // 指纹接口是否初始化
    BOOL                            m_bIsSupportSetup;  // 采集器是否支持设置对话框
    BYTE                            CapVersion[10];     // 采集接口版本
    BYTE                            FprVersion[10];     // 指纹接口版本
    BYTE                            pszDesc[1024];      // 接口说明
    LPBYTE                          m_pszRawData;       // 采集原始指纹特征数据
    LPBYTE                          m_pszBmpData;       // 采集指纹图像特征数据
    int                             nChannel;           // 通道号
    LPSTR                           m_pszError;         // 错误信息
    BOOL                            m_bOpen;
    BOOL                            m_bIsPressed;       // 指纹是否按下

    std::string                     m_strMode;
    CQtDLLLoader<IAllDevPort>       m_pDev;
    CSimpleMutex                    m_MutexAction;

    BYTE                            m_ppTemplateData[3][MAX_FEATURE_SIZE];// 保存指纹RAW数据,三合一模板数据
    WORD                            m_wBuildModelPressedTimes;    // 模板按奈次数
    WORD                            m_wBuildModelFailTimes;       // 建模失败次数
};
