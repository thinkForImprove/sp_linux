#ifndef DEVIMPL_SM205BCT_H
#define DEVIMPL_SM205BCT_H

#include "ILogWrite.h"
#include "QtDLLLoader.h"

#define DLL_DEVLIB_NAME  "libFpDev_ft.so"          // 缺省动态库名
#define LOG_NAME         "DevImpl_SM205BCT.log"           // 缺省日志名

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
// 1. 获取设备版本号
typedef int (CALL_MODE *lpFPIGetDevVersion)(int nPortNo, char* lpVersion);
// 2. 手指检测
typedef int (CALL_MODE *lpFPIDetectFinger)(int nPortNo);
// 3. 取消取指纹
typedef void (CALL_MODE *lpFPICancel)();
// 4. 获取设备序列号
typedef int (CALL_MODE *lpFPIGetDevSN)(int nPortNo, char* lpDevSN);
// 5. 取指纹特征
typedef int (CALL_MODE *lpFPIGetFeature)(int nPortNo, int nTimeOut, char* lpFinger, unsigned char* lpImage, int* lpImageLen);
// 6. 取指纹模板
typedef int (CALL_MODE *lpFPIGetTemplate)(int nPortNo, int nTimeOut, char* lpFinger);
// 7. 比对指纹
typedef int (CALL_MODE *lpFPIMatch)(char* lpFinger1, char* lpFinger2, int nLevel);
// 8. 保存指纹图片(bmp)
typedef int (CALL_MODE *lpFPIGetImageBmp)(unsigned char* lpImageData, unsigned char* lpBmpData);

/*
命令编辑、发送接收等处理。
*/

class CDevImpl_SM205BCT : public CLogManage
{
public:
    CDevImpl_SM205BCT();
    virtual ~CDevImpl_SM205BCT();
public:
    BOOL OpenDevice();
    BOOL OpenDevice(WORD wType);
    BOOL CloseDevice();
    BOOL IsDeviceOpen();

public: // 接口函数封装

    // 1. 获取设备版本号
    int FPIGetDevVersion(int nPortNo, char* lpVersion);
    // 2. 手指检测
    int FPIDetectFinger(int nPortNo);
    // 3. 取消取指纹
    void FPICancel();
    // 4. 获取设备序列号
    int FPIGetDevSN(int nPortNo, char* lpDevSN);
    // 5. 取指纹特征
    int FPIGetFeature(int nPortNo, int nTimeOut, char* lpFinger, unsigned char* lpImage, int* lpImageLen);
    // 6. 取指纹模板
    int FPIGetTemplate(int nPortNo, int nTimeOut, char* lpFinger);
    // 7. 比对指纹
    int FPIMatch(char* lpFinger1, char* lpFinger2, int nLevel);
    // 8. 保存指纹图片(bmp)
    int FPIGetImageBmp(unsigned char* lpImageData, unsigned char* lpBmpData);

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
    lpFPIGetDevVersion                       m_pFPIGetDevVersion;
    lpFPIDetectFinger                        m_pFPIDetectFinger;
    lpFPICancel                              m_pFPICancel;
    lpFPIGetDevSN                            m_pFPIGetDevSN;
    lpFPIGetFeature                          m_pFPIGetFeature;
    lpFPIGetTemplate                         m_pFPIGetTemplate;
    lpFPIMatch                               m_pFPIMatch;
    lpFPIGetImageBmp                         m_pFPIGetImageBmp;
};


#endif // DEVIMPL_SM205BCT_H
