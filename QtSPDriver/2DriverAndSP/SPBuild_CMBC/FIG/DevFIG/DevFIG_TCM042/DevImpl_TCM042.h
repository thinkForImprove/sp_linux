#ifndef DEVIMPL_TCM042_H
#define DEVIMPL_TCM042_H

#include "ILogWrite.h"
#include "QtDLLLoader.h"

#define DLL_DEVLIB_NAME  "libusbfp_phytium.so"          // 缺省动态库名
#define LOG_NAME         "DevImpl_TCM042.log"           // 缺省日志名

#define SAVE_IMG_BMP    0
#define SAVE_IMG_JPG    1
#define SAVE_IMG_PNG    2

#define SAVE_IMG_BMP_S  "BMP"
#define SAVE_IMG_JPG_S  "JPG"
#define SAVE_IMG_PNG_S  "PNG"

typedef unsigned BSDEVICEID;     // 定义设备ID类型

#define FUNC_POINTER_ERROR_RETURN(LPFUNC, LPFUNC2) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

#ifndef CALL_MODE
#ifdef _WIN32
#define CALL_MODE	__stdcall
#else
#define CALL_MODE
#endif
#endif

//----接口函数定义-------------------------------------------------------------------------------
// 1. 检测有没有连接指纹
typedef int (CALL_MODE *pFPIFindDevice)(int nPort, char * pszDevName);
// 2. 获取指纹特征数据
typedef int (CALL_MODE *pFPIGetFeature)(int nPort, char * pszVer, char *pImgBuf, int *ImgLen);
// 3. 获取指纹模板数据
typedef int (CALL_MODE *pFPIGetTemplate)(int nPort, char *pszReg);
// 4. 获取指纹仪SN序号
typedef int (CALL_MODE *pFPIGetDevSN)(int nPort, char * pszDevSN);
// 5. 指纹比对
typedef int (CALL_MODE *pFPIFpMatch)(char * pszReg, char * pszVer, int nMatchLevel);
// 6. 由三张指纹RAW数据或者特征值合成模板
typedef int (CALL_MODE *pFPITplFrmImg)(char *pImgBuf1, char * pImgBuf2, char *pImgBuf3, char *pRegBuf, int *pnRegLen);
// 7. 合成特征
typedef int (CALL_MODE *pFPIFeaFrmImg)(char *pImgBuf, char *pVerBuf, int *pnVerLen);
// 8. 保存指纹图片
typedef int (CALL_MODE *pFPISaveImage)(const char *pImgPath, const char *pImgBuf);
// 9. 判断手指是否按捺
typedef int (CALL_MODE *pFPIChkPressed)();
/*
命令编辑、发送接收等处理。
*/

class CDevImpl_TCM042 : public CLogManage
{
public:
    CDevImpl_TCM042();
    virtual ~CDevImpl_TCM042();
public:
    BOOL OpenDevice();
    BOOL OpenDevice(WORD wType);
    BOOL CloseDevice();
    BOOL IsDeviceOpen();

public: // 接口函数封装

    // 1. 检测有没有连接指纹
    int FPIFindDevice(int nPort, char * pszDevName);
    // 2. 获取指纹特征数据
    int FPIGetFeature(int nPort, char * pszVer, char *pImgBuf, int *ImgLen);
    // 3. 获取指纹模板数据
    int FPIGetTemplate(int nPort, char *pszReg);
    // 4. 获取指纹仪SN序号
    int FPIGetDevSN(int nPort, char * pszDevSN);
    // 5. 指纹比对
    int FPIFpMatch(char * pszReg, char * pszVer, int nMatchLevel);
    // 6. 由三张指纹RAW数据或者特征值合成模板
    int FPITplFrmImg(char *pImgBuf1, char * pImgBuf2, char *pImgBuf3, char *pRegBuf, int *pnRegLen);
    // 7. 合成特征
    int FPIFeaFrmImg(char *pImgBuf, char *pVerBuf, int *pnVerLen);
    // 8. 保存指纹图片
    int FPISaveImage(const char *pImgPath, const char *pImgBuf);
    // 9. 判断手指是否按捺
    int FPIChkPressed();

private:
    BSDEVICEID      m_bsDeviceId;
    BOOL            m_bDevOpenOk;
    CHAR            m_szErrStr[1024];

private:
    void Init();

private: // 接口加载
    BOOL bLoadLibrary();
    void vUnLoadLibrary();
    BOOL bLoadLibIntf();

private: // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    INT         m_nDevErrCode;
    INT         m_nStatErrCode;


private: // 动态库接口定义
    pFPIFindDevice                      m_pFPIFindDevice;
    pFPIGetFeature                      m_pFPIGetFeature;
    pFPIGetTemplate                     m_pFPIGetTemplate;
    pFPIGetDevSN                        m_pFPIGetDevSN;
    pFPIFpMatch                         m_pFPIFpMatch;
    pFPITplFrmImg                       m_pFPITplFrmImg;
    pFPIFeaFrmImg                       m_pFPIFeaFrmImg;
    pFPISaveImage                       m_pFPISaveImage;
    pFPIChkPressed                      m_pFPIChkPressed;
};


#endif // DEVIMPL_TCM042_H
