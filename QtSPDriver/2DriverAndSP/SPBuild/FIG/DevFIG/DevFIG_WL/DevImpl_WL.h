#ifndef DEVIMPL_WL_H
#define DEVIMPL_WL_H

#include <string>
#include <QLibrary>
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "XFSPTR.H"

#define DLL_DEVLIB_NAME        "libFpDriverUSB_WL.so"        // 图像采集接口

#define LOG_NAME                "DevImpl_WL.log"      // 缺省日志名

#define SAVE_IMG_MIN_WIDTH          256    // 采集图像最小宽度
#define SAVE_IMG_MIN_HIGHT          360    // 采集图像最小高度
#define SAVE_IMG_DATA_SIZE_APPEND   1078   // 图像数据附加值

#define SAVE_IMG_BMP    0
#define SAVE_IMG_JPG    1
#define SAVE_IMG_PNG    2

#define SAVE_IMG_BMP_S  "BMP"
#define SAVE_IMG_JPG_S  "JPG"
#define SAVE_IMG_PNG_S  "PNG"

typedef unsigned BSDEVICEID;     // 定义设备ID类型



typedef void (*lFPICancel)();
typedef int (*lFPIPowerOn)();
typedef int (*lFPIPowerOff)();
typedef int (*lFPIDeviceInitRS232)(char* psComFile, int jnBaund, int jnFlag);
//typedef int (*lFPIDeviceInit) (int fd, char* devNode);
typedef int (*lFPIDeviceInit)();
typedef int (*lFPIDeviceClose)();
typedef int (*lFPIGetVersion)(char* psDevVersion,int *lpLength);
typedef int (*lFPIGetDeviceID)(char* psDeviceID);
typedef int (*lFPIFeature)(int nTimeOut, unsigned char* psFpData, int* pnLength);
typedef int (*lFPITemplate)(int nTimeOut,unsigned  char* psFpData, int* pnLength);
typedef int (*lFPIGetImage)(int nTimeOut, char* psImageBuf, int* pnLength, char* psBmpFile);
typedef int (*lFPIGetImageEx)(int nTimeOut, char* psImageBuf, int* pnLength, int* iIsValid, int* iQuality, char* psBmpFile);
typedef int (*lFPICheckFinger)();
typedef int (*lFPICheckImage)(char* psImage, int* pnErrType, char* psErrmsg, char* psSuggest);
typedef int (*lFPIGatherEnroll)(int num, int ss);
typedef int (*lFPISynEnroll)(int num);
typedef int (*lFPIMatch)(unsigned  char* psRegBuf, unsigned  char* psVerBuf, int iLevel);
typedef int (*lFPIFpMatch)(char* psRegBuf, char* psVerBuf, int jnLevel);
typedef int (*lFPIExtract)(char* psImage, unsigned  char* psVerBuf, int* pnLength);
typedef int (*lFPICryptBase64)(int nMode, unsigned char *psInput, int nInLen, unsigned char *psOutput, int *pnOutlen);
typedef int (*lFPIEnroll)(char* psImage1, char* psImage2, char* psImage3, unsigned char* psRegBuf, int* pnLength);
typedef int (*lFPIEnrollX)(char* psTZ1, char* psTZ2, char* psTZ3, char* psRegBuf, int* pnLength);
typedef int (*lFPIGetTemplateByTZ)(char* psTZ1, char* psTZ2, char* psTZ3, char* psRegBuf, int* pnLength);
typedef int (*lFPIGetFeatureAndImage)(int nTimeOut, char* psFpData, int* pnFpDataLen, char* psImageBuf, int* pnImageLen, char* psBmpFile);
typedef int (*lFPIDevDetect) (void);
typedef int (*lFPIGetImageData)(int nTimeOut, int *lpImageWidth, int *lpImageHeight, unsigned  char *psImageBuf);
typedef int (*lFPIImg2Bmp)(int nTimeOut, char* psBmpFile);
typedef int (*lFPIGetImageBuf)(int nTimeOut,char* psBmpFile, int *lpImageWidth, int *lpImageHeight,unsigned char* psImageBuf);
typedef int (*lFPIMatchB64) (unsigned char* psRegBuf,unsigned  char* psVerBuf, int jnLevel);
typedef int (*lFPIBmp2Feature)(int nMode, char* psImgPath1, char* psTZ, int* lpLength);
typedef int (*lFPIBmp2Template)(int nMode, char *psImgPath1, char *psImgPath2, char *psImgPath3, char *psMB, int *lpLength);


#define FUNC_POINTER_ERROR_RETURN(LPFUNC, LPFUNC2) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

class CAutoReleaseFlag
{
public:
    CAutoReleaseFlag(BOOL *);
    ~CAutoReleaseFlag();
private:
    BOOL        *m_pBOOL;
};

class CDevImpl_WEL401 : public CLogManage
{
public:
    CDevImpl_WEL401();
    virtual ~CDevImpl_WEL401();


public:
    void FPICancel();
    BOOL FPIPowerOn();
    BOOL FPIPowerOff();
    BOOL FPIDeviceInitRS232(char* psComFile, int jnBaund, int jnFlag);
    //BOOL FPIDeviceInit (int fd, char* devNode);
    BOOL FPIDeviceInit ();
    BOOL FPIDeviceClose();
    BOOL FPIGetVersion(char* psDevVersion,int *lpLength);
    BOOL FPIGetDeviceID(char* psDeviceID);
    BOOL FPIFeature(int nTimeOut, unsigned char* psFpData, int* pnLength);
    BOOL FPITemplate(int nTimeOut,unsigned  char* psFpData, int* pnLength);
    BOOL FPIGetImage(int nTimeOut, char* psImageBuf, int* pnLength, char* psBmpFile);
    BOOL FPIGetImageEx(int nTimeOut, char* psImageBuf, int* pnLength, int* iIsValid, int* iQuality, char* psBmpFile);
    BOOL FPICheckFinger();
    BOOL FPICheckImage(char* psImage, int* pnErrType, char* psErrmsg, char* psSuggest);
    BOOL FPIGatherEnroll(int num, int ss);
    BOOL FPISynEnroll(int num);
    BOOL FPIMatch(unsigned  char* psRegBuf, unsigned  char* psVerBuf, int iLevel);
    BOOL FPIFpMatch(char* psRegBuf, char* psVerBuf, int jnLevel);
    BOOL FPIExtract(char* psImage, unsigned  char* psVerBuf, int* pnLength);
    BOOL FPICryptBase64(int nMode, unsigned char *psInput, int nInLen, unsigned char *psOutput, int *pnOutlen);
    BOOL FPIEnroll(char* psImage1, char* psImage2, char* psImage3, unsigned char* psRegBuf, int* pnLength);
    BOOL FPIEnrollX(char* psTZ1, char* psTZ2, char* psTZ3, char* psRegBuf, int* pnLength);
    BOOL FPIGetTemplateByTZ(char* psTZ1, char* psTZ2, char* psTZ3, char* psRegBuf, int* pnLength);
    BOOL FPIGetFeatureAndImage(int nTimeOut, char* psFpData, int* pnFpDataLen, char* psImageBuf, int* pnImageLen, char* psBmpFile);
    BOOL FPIDevDetect (void);
    BOOL FPIGetImageData(int nTimeOut, int *lpImageWidth, int *lpImageHeight, unsigned  char *psImageBuf);
    BOOL FPIImg2Bmp(int nTimeOut, char* psBmpFile);
    BOOL FPIGetImageBuf(int nTimeOut,char* psBmpFile, int *lpImageWidth, int *lpImageHeight,unsigned char* psImageBuf);
    BOOL FPIMatchB64 (unsigned char* psRegBuf,unsigned  char* psVerBuf, int jnLevel);
    BOOL FPIBmp2Feature(int nMode, char* psImgPath1, char* psTZ, int* lpLength);
    BOOL FPIBmp2Template(int nMode, char *psImgPath1, char *psImgPath2, char *psImgPath3, char *psMB, int *lpLength);

private:  // 内部函数
    BOOL bLoadLibrary();
    void vUnLoadLibrary();
    BOOL bLoadLibIntf();

private:  // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    INT         m_nDevErrCode;
    INT         m_nStatErrCode;

    BOOL        m_bDisableSendCmd;   // TRUE: 禁止给指纹仪发送命令

private: // 动态库接口定义
    lFPICancel m_pFPICancel;
    lFPIPowerOn m_pFPIPowerOn;
    lFPIPowerOff m_pFPIPowerOff;
    lFPIDeviceInitRS232 m_pFPIDeviceInitRS232;
//lFPIDeviceInit m_pFPIDeviceInit;
    lFPIDeviceInit m_pFPIDeviceInit;
    lFPIDeviceClose m_pFPIDeviceClose;
    lFPIGetVersion m_pFPIGetVersion;
    lFPIGetDeviceID m_pFPIGetDeviceID;
    lFPIFeature m_pFPIFeature;
    lFPITemplate m_pFPITemplate;
    lFPIGetImage m_pFPIGetImage;
    lFPIGetImageEx m_pFPIGetImageEx;
    lFPICheckFinger m_pFPICheckFinger;
    lFPICheckImage m_pFPICheckImage;
    lFPIGatherEnroll m_pFPIGatherEnroll;
    lFPISynEnroll m_pFPISynEnroll;
    lFPIMatch m_pFPIMatch;
    lFPIFpMatch m_pFPIFpMatch;
    lFPIExtract m_pFPIExtract;
    lFPICryptBase64  m_pFPICryptBase64;
    lFPIEnroll m_pFPIEnroll;
    lFPIEnrollX m_pFPIEnrollX;
    lFPIGetTemplateByTZ m_pFPIGetTemplateByTZ;
    lFPIGetFeatureAndImage m_pFPIGetFeatureAndImage;
    lFPIDevDetect m_pFPIDevDetect;
    lFPIGetImageData m_pFPIGetImageDat;
    lFPIImg2Bmp m_pFPIImg2Bmp;
    lFPIGetImageBuf m_pFPIGetImageBufar;
    lFPIMatchB64 m_pFPIMatchB64;
    lFPIBmp2Feature m_pFPIBmp2Feature;
    lFPIBmp2Template m_pFPIBmp2Template;

};
#endif // DEVIMPL_HX_H
