#pragma once

#include <string>
#include <QFile>
#include <QImage>
#include <QImageWriter>
#include <QPainter>
#include "IDevSIG.h"
#include "QtTypeInclude.h"
#include "errorcode.h"
#include "DevImpl_TSD64.h"

#define LOG_NAME_DEVCAM             "CDevSIG_TSD64.log"
#define RESULT_SUCCESS              0
#define CAM_EXTRA                   (3)

#define TSD64API_ERRORCODE_RETURN(pszErrorInfo) \
if (strcmp(m_pchErrCode, DRV0000000) != 0)\
{\
    Log(ThisModule, __LINE__, pszErrorInfo);\
    return GetErrorCode(m_pchErrCode);\
}

class CDevSIG_TSD64 : public IDevSIG, public CDevImpl_TSD64
{
public:
    CDevSIG_TSD64(LPCSTR lpDevType);
    virtual ~CDevSIG_TSD64();
public:

    // 释放接口
    virtual void Release();
    // 打开连接
    virtual int Open();
    // 关闭连接
    virtual int Close();
    // 复位
    virtual int Reset();
    // 取状态
    virtual int GetStatus(DEVCAMSTATUS &stStatus);
    // 获取固件版本
    virtual int GetFWVersion(LPSTR pFWVersion);
    // 设置属性
    virtual int SetProperty(const SIGCONFIGINFO stSigConfig);
    // 打开窗口
    virtual int Display(WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight, WORD wHpixel, WORD wVpixel, LPSTR pszTexData, WORD wAlwaysShow, DWORD dwTimeout);
    // 获取签名
    virtual int GetSignature(LPSTR pszKey, LPSTR pszEncKey, LPSTR pszPictureFile, LPSTR pszSignatureFile, DWORD wnd, WORD wEncypt, LPSTR pszCamData, LPWSTR pswzUnicodeCamData);
    // 清除签名
    virtual int ClearSignature();

protected:
    BOOL IsDeviceOpen();
    int LoadKey(LPSTR bKey, int iLength, int iIndex, int iDecKeyNum, int iDecMode, int iUse);
    int nSaveDataToImage(LPBYTE pData, LPCSTR filename, int nLen);
    int GetErrorCode(LPSTR pchErrCode);
    int bReadImageData(LPSTR lpImgPath, LPBYTE lpImgData);
    int nImageFormatTransform(std::string pDest, std::string pPicture, LPSTR pFormat);
    std::string getStringLastNChar(std::string str, ULONG lastN);
    int nPngToOtherFormat(QImage pngImage, LPCSTR pDest, LPSTR pFormat);
    int bCreateWindow(WORD wX, WORD wY, WORD wWidth, WORD wHeight, std::string strBackgroundTex, std::string strBackgroundPath);

private:
    CSimpleMutex                m_cMutex;
    DEVCAMSTATUS                m_stStatus;

    CHAR                        m_pSaveFileName[MAX_PATH];
    BOOL                        m_bDevOpenOk;                       // 是否设备已打开
    BOOL                        bDisplyOK;                          // 签名窗口是否已打开
    long                        m_lLength;                          // 数据长度
    CAM_WINDOW                  m_wAction;                          // 窗口操作

    LPSTR                       m_pchErrCode;                       // 错误码(11位)
    SIGCONFIGINFO               m_sSigIniConfig;                    // 存储配置项
};
