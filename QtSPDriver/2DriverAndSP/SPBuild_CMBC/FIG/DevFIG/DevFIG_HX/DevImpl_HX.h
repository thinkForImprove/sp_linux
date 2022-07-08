#ifndef DEVIMPL_HX_H
#define DEVIMPL_HX_H

#include <string>
#include <QLibrary>
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "XFSPTR.H"

#define DLL_DEVREAD_NAME        "ID_FprCap.so"        // 图像采集接口
#define DLL_DEVCOMPARE_NAME     "ID_Fpr.so"           // 指纹比对接口
#define LOG_NAME                "DevImpl_HX.log"      // 缺省日志名

#define SAVE_IMG_MIN_WIDTH          256    // 采集图像最小宽度
#define SAVE_IMG_MIN_HIGHT          360    // 采集图像最小高度
#define SAVE_IMG_DATA_SIZE_APPEND   1078   // 图像数据附加值

#define SAVE_IMG_BMP    0
#define SAVE_IMG_JPG    1

#define SAVE_IMG_BMP_S  "BMP"
#define SAVE_IMG_JPG_S  "JPG"

typedef unsigned BSDEVICEID;     // 定义设备ID类型

// dll接口函数指针
#define FIGAPI __attribute__((__cdecl__))

// FprCap.so
typedef HRESULT(FIGAPI *FIGLIVESCANINIT)();
typedef HRESULT(FIGAPI *FIGLIVESCANCLOSE)();
typedef HRESULT(FIGAPI *FIGLIVESCANGETCHANNELCOUNT)();
typedef HRESULT(FIGAPI *FIGLIVESCANSETBRIGHT)(int nChannel, int nBright);
typedef HRESULT(FIGAPI *FIGLIVESCANSETCONTRAST)(int nChannel, int nBright);
typedef HRESULT(FIGAPI *FIGLIVESCANGETBRIGHT)(int nChannel, int* pnBright);
typedef HRESULT(FIGAPI *FIGLIVESCANGETCONTRAST)(int nChannel, int* pnContrast);
typedef HRESULT(FIGAPI *FIGLIVESCANGETMAXIMAGESIZE)(int nChannel, int* pnWidth, int* pnHeight);
typedef HRESULT(FIGAPI *FIGLIVESCANGETCAPTWINDOW)(int nChannel, int* pnOriginX, int* pnOriginY, int* pnWidth, int* pnHeight);
typedef HRESULT(FIGAPI *FIGLIVESCANSETCAPTWINDOW)(int nChannel, int nOriginX, int nOriginY, int nWidth, int nHeight);
typedef HRESULT(FIGAPI *FIGLIVESCANSETUP)();
typedef HRESULT(FIGAPI *FIGLIVESCANBEINCAPTURE)(int nChannel);
typedef HRESULT(FIGAPI *FIGLIVESCANGETFPRAWDATA)(int nChannel, unsigned char* pRawData);
typedef HRESULT(FIGAPI *FIGLIVESCANGETFPBMPDATA)(int nChannel, unsigned char* pBmpData);
typedef HRESULT(FIGAPI *FIGLIVESCANENDCAPTURE)(int nChannel);
typedef HRESULT(FIGAPI *FIGLIVESCANISSUPPORTSETUP)();
typedef HRESULT(FIGAPI *FIGLIVESCANGETVERSION)();
typedef HRESULT(FIGAPI *FIGLIVESCANGETDESC)(char pszDesc[1024]);
typedef HRESULT(FIGAPI *FIGLIVESCANGETERRORINFO)(int nErrorNo, char pszErrorInfo[256]);
typedef HRESULT(FIGAPI *FIGLIVESCANSETBUFFERRMPTY)(unsigned char* pImageData, long imageLength);
typedef HRESULT(FIGAPI *FIGLIVESCANGETFIGTEMPLETE)(unsigned char* pcBuf1, unsigned char* pcBuf2, unsigned char* pcBuf3, int nBufLen1, int nBufLen2, int BufLen3, unsigned char* pcBuf4, int* pnBufLen);

// Fpr.so
typedef INT(FIGAPI *FIGFPGETVERSION)(BYTE pszCode[4]);
typedef INT(FIGAPI *FIGFPBEGIN)();
typedef INT(FIGAPI *FIGFPFEATUREEXTRACT)(BYTE cScannerType, BYTE cFingerCode, LPBYTE pFingerImgBuf, LPBYTE pFeatureData);
typedef INT(FIGAPI *FIGFPFEATUREMATCH)(LPBYTE pFeatureData1, LPBYTE pFeatureData2, float * pfSimilarity);
typedef INT(FIGAPI *FIGFPIMAGEMATCH)(LPBYTE pFingerImgBuf, LPBYTE pFeatureData, float * pfSimilarity);
typedef INT(FIGAPI *FIGFPCOMPRESS)(BYTE cScannerType, BYTE cEnrolResult, BYTE cFingerCode, LPBYTE pFingerImgBuf, int nCompressRatio, LPBYTE pCompressedImgBuf, BYTE strBuf[256]);
typedef INT(FIGAPI *FIGFPDECOMPRESS)(LPBYTE pCompressedImgBuf, LPBYTE pFingerImgBuf, BYTE strBuf[256]);
typedef INT(FIGAPI *FIGFPGETQUALITYSCORE)(LPBYTE pFingerImgBuf, LPBYTE pnScore);
typedef INT(FIGAPI *FIGFPGENFEATUREFROMEMPTY1)(BYTE cScannerType, BYTE cFingerCode, LPBYTE pFeatureData);
//typedef INT(FIGAPI *FIGFPGENCOMPRESSDATAFROMEMPTY)();
typedef INT(FIGAPI *FIGFPEND)();

//获取接口错误
#define FUNC_POINTER_ERROR_RETURN_CAP(LPFUNC, LPFUNC2) \
    if(!LPFUNC){   \
        m_bLoadFprCapIntfFail = TRUE; \
        return FALSE;   \
    }

#define FUNC_POINTER_ERROR_RETURN(LPFUNC, LPFUNC2) \
    if(!LPFUNC){   \
        m_bLoadFprIntfFail = TRUE; \
        return FALSE;   \
    }

class CHX_DevImpl : public CLogManage
{
public:
    CHX_DevImpl();
    virtual ~CHX_DevImpl();

public:
    // FprCap.so
    // 初始化采集器
    BOOL Cap_Init();
    // 释放采集器
    BOOL Cap_Close();
    // 获得采集器通道数量
    BOOL Cap_GetChannelCount();
    // 设置采集器当前的亮度
    BOOL Cap_SetBright(int nChannel, int nBright);
    // 设置采集器当前对比度
    BOOL Cap_SetContrast(int nChannel,int nContrast);
    // 获得采集器当前的亮度
    BOOL Cap_GetBright(int nChannel,int* pnBright);
    // 获得采集器当前对比度
    BOOL Cap_GetContrast(int nChannel,int* pnContrast);
    // 获得采集器可采集图像的宽度、高度的最大值
    BOOL Cap_GetMaxImageSize(int nChannel,int* pnWidth, int* pnHeight);
    // 获得采集器当前图像的采集位置、宽度和高度
    BOOL Cap_GetCaptWindow(int nChannel,int* pnOriginX, int* pnOriginY,int* pnWidth, int* pnHeight);
    // 设置采集器当前图像的采集位置、宽度和高度
    BOOL Cap_SetCaptWindow(int nChannel,int nOriginX, int nOriginY,int nWidth, int nHeight);
    // 调用采集器的属性设置对话框
    BOOL Cap_Setup();
    // 准备采集一帧图像
    BOOL Cap_BeginCapture(int nChannel);
    // 采集一帧图像
    BOOL Cap_GetFPRawData(int nChannel,unsigned char* pRawData);
    // 采集一帧 BMP 格式图像
    BOOL Cap_GetFPBmpData(int nChannel, unsigned char *pBmpData);
    // 结束采集一帧图像
    BOOL Cap_EndCapture(int nChannel);
    // 采集器是否支持设置对话框
    BOOL Cap_isSupportSetup();
    // 取得接口规范的版本
    BOOL Cap_GetVersion();
    // 获得接口规范的说明
    BOOL Cap_GetDesc(char pszDesc[1024]);
    // 获得采集接口错误信息
    LPSTR Cap_GetErrorInfo(int nErrorNo);
    // 设置存放采集数据的内存块为空
    BOOL Cap_setBufferEmpty(unsigned char *pImageData, long imageLength);
    // 合成指纹模板
    BOOL Cap_GetFigTemplete(unsigned char* pcBuf1, unsigned char* pcBuf2, unsigned char* pcBuf3, int nBufLen1, int nBufLen2, int BufLen3, unsigned char* pcBuf4, int* pnBufLen);

    // Fpr.so
    // 版本信息获取
    BOOL Fpr_GetVersion(BYTE pszCode[4]);
    // 初始化操作
    BOOL Fpr_Begin();
    // 1 枚指纹图像特征提取
    BOOL Fpr_FeatureExtract(BYTE cScannerType,              //输入参数 指纹采集器类型代码
                            BYTE cFingerCode,               //输入参数 指位代码
                            LPBYTE pFingerImgBuf,           //输入参数 指纹图像数据指针图像格式为 256(width)*360(height) 8 位灰度图像
                            LPBYTE pFeatureData);           //输出参数 指纹图像特征数据指针
    // 对 2 个指纹特征数据进行比对
    BOOL Fpr_FeatureMatch(LPBYTE pFeatureData1,             //输入参数 指纹特征数 据 1
                          LPBYTE pFeatureData2,             //输入参数 指纹特征数据 2
                          float * pfSimilarity);            //输出参数 匹配相似度值 0.00-1.00
    // 对指纹图像和指纹特征进行比对
    BOOL Fpr_ImageMatch(LPBYTE pFingerImgBuf,               //输入参数 指纹图像数据指针 图像格式为 256(width)*360(height)
                        LPBYTE pFeatureData,                //输入参数 指纹特征数据
                        float * pfSimilarity);              //输出参数 指纹图像特征数据指针
    // 对指纹图像数据进行压缩
    BOOL Fpr_Compress( BYTE cScannerType,                   //输入参数 指纹采集器类型代 码
                       BYTE cEnrolResult,
                       BYTE cFingerCode,                    //输入参数 指位代码
                       LPBYTE pFingerImgBuf,                //输入参数 指纹图像数据指针 图像格式为 256(width)*360(height)
                       int nCompressRatio,                  //输入参数 压缩倍数
                       LPBYTE pCompressedImgBuf,            //输出参数 指纹图像压缩数据指针 空间为 20K 字节
                       BYTE strBuf[256]);                   //输出参数 返回-4 时的错误信息
    // 对指纹图像压缩数据进行复现
    BOOL Fpr_Decompress( LPBYTE pCompressedImgBuf,          //输入参数 指纹图像压缩 数据指针 已分配好 20K 字节空间
                         LPBYTE pFingerImgBuf,              //输入参数 指纹图像数据指针 大小 256*360 字节
                         BYTE strBuf[256]);                 //输出参数 返回-4 时的错误信息
    // 获取指纹图像的质量值
    BOOL Fpr_GetQualityScore( LPBYTE pFingerImgBuf,         //输入参数 指纹图像数据指 针 图像格式为 256(width)*360(height)
                              LPBYTE bScore);                 //输出参数 图像质量值 00H - 64H
    // 生成"未注册"指纹特征数据
    BOOL Fpr_GenFeatureFromEmpty1(BYTE cScannerType,        //输入参数 指纹 采集器类型代码
                                  BYTE cFingerCode,         //输入参数 指位代码
                                  LPBYTE pFeatureData);     //输出参数 已由调用者分配 512 字节空间

    // 结束操作
    BOOL Fpr_End();

private:  // 内部函数
    BOOL LoadDll();
    BOOL LoadFprCapIntf();
    BOOL LoadFprIntf();
    void UnloadCloudWalkDll();

private:  // 接口加载
    char        m_szFprCapDllPath[MAX_PATH];
    char        m_szFprDllPath[MAX_PATH];
    QLibrary    m_FprCapLibrary;
    QLibrary    m_FprLibrary;
    BOOL        m_bLoadFprCapIntfFail;
    BOOL        m_bLoadFprIntfFail;

private: // 动态库接口定义

    // FprCap.so
    FIGLIVESCANINIT                 m_pLIVESCAN_Init;
    FIGLIVESCANCLOSE                m_pLIVESCAN_Close;
    FIGLIVESCANGETCHANNELCOUNT      m_pLIVESCAN_GetChannelCount;
    FIGLIVESCANSETBRIGHT            m_pLIVESCAN_SetBright;
    FIGLIVESCANSETCONTRAST          m_pLIVESCAN_SetContrast;
    FIGLIVESCANGETBRIGHT            m_pLIVESCAN_GetBright;
    FIGLIVESCANGETCONTRAST          m_pLIVESCAN_GetContrast;
    FIGLIVESCANGETMAXIMAGESIZE      m_pLIVESCAN_GetMaxImageSize;
    FIGLIVESCANGETCAPTWINDOW        m_pLIVESCAN_GetCaptWindow;
    FIGLIVESCANSETCAPTWINDOW        m_pLIVESCAN_SetCaptWindow;
    FIGLIVESCANSETUP                m_pLIVESCAN_Setup;
    FIGLIVESCANBEINCAPTURE          m_pLIVESCAN_BeginCapture;
    FIGLIVESCANGETFPRAWDATA         m_pLIVESCAN_GetFPRawData;
    FIGLIVESCANGETFPBMPDATA         m_pLIVESCAN_GetFPBmpData;
    FIGLIVESCANENDCAPTURE           m_pLIVESCAN_EndCapture;
    FIGLIVESCANISSUPPORTSETUP       m_pLIVESCAN_IsSupportSetup;
    FIGLIVESCANGETVERSION           m_pLIVESCAN_GetVersion;
    FIGLIVESCANGETDESC              m_pLIVESCAN_GetDesc;
    FIGLIVESCANGETERRORINFO         m_pLIVESCAN_GetErrorInfo;
    FIGLIVESCANSETBUFFERRMPTY       m_pLIVESCAN_SetBufferEmpty;
    FIGLIVESCANGETFIGTEMPLETE       m_pLIVESCAN_GetFigTemplete;

    //Fpr.so
    FIGFPGETVERSION                 m_pFPGetVersion;
    FIGFPBEGIN                      m_pFPBegin;
    FIGFPFEATUREEXTRACT             m_pFPFeatureExtract;
    FIGFPFEATUREMATCH               m_pFPFeatureMatch;
    FIGFPIMAGEMATCH                 m_pFPImageMatch;
    FIGFPCOMPRESS                   m_pFPCompress;
    FIGFPDECOMPRESS                 m_pFPDecompress;
    FIGFPGETQUALITYSCORE            m_pFPGetQualityScore;
    FIGFPGENFEATUREFROMEMPTY1       m_pFPGenFeatureFromEmpty1;
    FIGFPEND                        m_pFPEnd;
};
#endif // DEVIMPL_HX_H
