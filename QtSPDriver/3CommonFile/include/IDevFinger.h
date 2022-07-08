#pragma once

#define DVEFINGER_NO_VTABLE  __declspec(novtable)
//////////////////////////////////////////////////////////////////////////
#define IMGBUFFSIZE         (512*512)       // 指纹图像数据大小
//////////////////////////////////////////////////////////////////////////
struct DVEFINGER_NO_VTABLE IDevFinger
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
    virtual long Open(const char *lpMode) = 0;
    // 关闭连接
    virtual long Close() = 0;
    // 初始化设备
    virtual long Init() = 0;
    // 复位
    virtual long Reset() = 0;
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo) = 0;
    // 取状态
    virtual long GetStatus() = 0;
    // 取消读取
    virtual long CancelRead() = 0;
    // 等待松开手指
    virtual long WaitReleaseFinger(long lTimeOut) = 0;
    // 读取采集指纹图像，数据为Base64编码，返回值-1表示超时
    virtual long ReadImage(const char *pRandom, char *pImage, char *pMac, long lTimeOut) = 0;
    // 指纹图像转指纹特征数据，数据为Base64编码
    virtual long GetFeatureByImage(const char *pImage, char *pFeature) = 0;
    // 指纹图像转指纹模板数据，数据为Base64编码
    virtual long GetTemplateByImage(const char *pImage1, const char *pImage2, const char *pImage3, char *pTemplate) = 0;
    // 指纹比对，iSecurityLevel->安全级别（1~5，默认为 3），数据为Base64编码
    virtual long TFVerify(const char *pTemplate, const char *pFeature, int iSecurityLevel = 3) = 0;
    // 数据 BASE64 编码/解码
    virtual long CryptBase64(const char *pIn, int nInLen, char *pOut, int *nOutLen, bool bCrypt = true) = 0;
    // 根据长度生成随机数
    virtual long GetRandom(char *pRandom, int nBit = 32) = 0;
    // 下载厂家密钥，数据为十六进制数
    virtual long LoadDFK(const char *pDevID, const char *pKeyName, const char *pKeyVer, const char *pKey) = 0;
    // 下载主密钥，数据为十六进制数
    virtual long LoadDMK(const char *pDevID, const char *pKeyName, const char *pKeyVer, const char *pKey, const char *pMAC) = 0;
    // 下载工作密钥，数据为十六进制数
    virtual long LoadWorkKey(const char *pDevID, int nKeyType, const char *pKeyName, const char *pKeyVer, const char *pKey, const char *pMAC) = 0;

};


extern "C" long CreateIDevFinger(LPCSTR lpDevType, IDevFinger *&p);
//////////////////////////////////////////////////////////////////////////