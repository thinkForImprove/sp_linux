/*************************************************************************
 * 明泰非接动态库libCnSysReader.so调用dlopen显式加载时会报异常，所以采用隐式加载方式，
 * 该文件已废弃．
 * ***********************************************************************/
#pragma once
#define DLL_POSSDK_NAME "libtmz.so"
#include "QtTypeDef.h"
#include <QLibrary>

#define FUNC_POINTER_ERROR_RETURN(LPFUNC) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE;    \
        return FALSE;   \
    }

//////////////////////////////////////////////////////////////////////////
//函数指针定义
typedef long (* FNICReaderOpenUsbByFD)(unsigned int uiFD);
typedef int  (* FNICReaderClose)(long icdev);
typedef int  (* FNICReaderGetVer)(long icdev, char *pVer);
typedef int  (* FNICReaderReadDevSnr)(long icdev, unsigned char rlen, char *pSN);

typedef int  (* FNICReaderBeep)(long icdev, unsigned char uMsec, unsigned char uDelay, unsigned char uNum);
typedef int  (* FNICReaderDevStatus)(long icdev, unsigned char *uStatus);
typedef int  (* FNGetCardState)(long icdev, unsigned char uSlot, int* uState);
typedef int  (* FNCPUPowerOn)(long icdev, unsigned char uSlot, int iTime,
                              unsigned char* uType, unsigned char* uSnrLen, unsigned char* pSnr,
                              int* rLen, unsigned char* pData);
typedef int  (* FNCPUPowerOff)(long icdev, unsigned char uSlot);
typedef int  (* FNCPUCardAPDU)(long icdev, unsigned char uSlot, int sLen,
                               unsigned char *pSendData, int* rLen, unsigned char* pData);
typedef int  (* FNICReaderLEDCtrl)(long icdev, unsigned char uLEDCtrl, unsigned char uDelay);
//////////////////////////////////////////////////////////////////
class TMZDriver
{
public:
    TMZDriver();
    ~TMZDriver();
public:
    BOOL LoadSdkDll();
    void UnloadSdkDll();
private:
    BOOL LoadSdkIntf();
private:
    QLibrary m_sdkLibrary;
    BOOL m_bLoadIntfFail;

    char m_szSdkDllPath[MAX_PATH];

public:
    /*-----------------------------FUNCTION POINTER------------------------------*/
    FNICReaderOpenUsbByFD ICReaderOpenUsbByFD;
    FNICReaderClose       ICReaderClose;
    FNICReaderGetVer      ICReaderGetVer;
    FNICReaderReadDevSnr  ICReaderReadDevSnr;
    FNICReaderBeep        ICReaderBeep;
    FNICReaderDevStatus   ICReaderDevStatus;
    FNGetCardState        GetCardState;
    FNCPUPowerOn          CPUPowerOn;
    FNCPUPowerOff         CPUPowerOff;
    FNCPUCardAPDU         CPUCardAPDU;
    FNICReaderLEDCtrl     ICReaderLEDCtrl;
};
