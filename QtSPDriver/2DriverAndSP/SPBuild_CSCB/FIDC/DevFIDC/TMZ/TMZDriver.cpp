#include "TMZDriver.h"
#include "QtTypeDef.h"
#include "QtDLLLoader.h"

TMZDriver::TMZDriver()
{
    ICReaderOpenUsbByFD = nullptr;
    ICReaderClose = nullptr;
    ICReaderGetVer = nullptr;
    ICReaderBeep = nullptr;
    ICReaderDevStatus = nullptr;
    GetCardState = nullptr;
    CPUPowerOn = nullptr;
    CPUPowerOff = nullptr;
    CPUCardAPDU = nullptr;
    ICReaderLEDCtrl = nullptr;

    memset(m_szSdkDllPath, 0, sizeof(m_szSdkDllPath));
    QString strDllName(QString::fromLocal8Bit(DLL_POSSDK_NAME));
#ifdef Q_OS_WIN
    strDllName.prepend(WINPATH);
#else
    strDllName.prepend(LINUXPATHLIB);
#endif

    m_sdkLibrary.setFileName(strDllName);
    m_bLoadIntfFail = TRUE;
}

TMZDriver::~TMZDriver()
{
    UnloadSdkDll();
}

BOOL TMZDriver::LoadSdkDll()
{
    if(m_sdkLibrary.isLoaded() != true){
        if(m_sdkLibrary.load() != true){
            return FALSE;
        }
    }

    if(m_bLoadIntfFail){
        if(LoadSdkIntf() != TRUE){
            return FALSE;
        }
    }
    return TRUE;
}

void TMZDriver::UnloadSdkDll()
{
    if(m_sdkLibrary.isLoaded()){
        m_sdkLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL TMZDriver::LoadSdkIntf()
{
    m_bLoadIntfFail = FALSE;

    ICReaderOpenUsbByFD = (FNICReaderOpenUsbByFD)m_sdkLibrary.resolve("ICReaderOpenUsbByFD");
    FUNC_POINTER_ERROR_RETURN(ICReaderOpenUsbByFD);

    ICReaderClose = (FNICReaderClose)m_sdkLibrary.resolve("ICReaderClose");
    FUNC_POINTER_ERROR_RETURN(ICReaderClose);

    ICReaderGetVer = (FNICReaderGetVer)m_sdkLibrary.resolve("ICReaderGetVer");
    FUNC_POINTER_ERROR_RETURN(ICReaderGetVer);

    ICReaderBeep = (FNICReaderBeep)m_sdkLibrary.resolve("ICReaderBeep");
    FUNC_POINTER_ERROR_RETURN(ICReaderBeep);

    ICReaderDevStatus = (FNICReaderDevStatus)m_sdkLibrary.resolve("ICReaderDevStatus");
    FUNC_POINTER_ERROR_RETURN(ICReaderDevStatus);

    GetCardState = (FNGetCardState)m_sdkLibrary.resolve("GetCardState");
    FUNC_POINTER_ERROR_RETURN(GetCardState);

    CPUPowerOn = (FNCPUPowerOn)m_sdkLibrary.resolve("CPUPowerOn");
    FUNC_POINTER_ERROR_RETURN(CPUPowerOn);

    CPUPowerOff = (FNCPUPowerOff)m_sdkLibrary.resolve("CPUPowerOff");
    FUNC_POINTER_ERROR_RETURN(CPUPowerOff);

    CPUCardAPDU = (FNCPUCardAPDU)m_sdkLibrary.resolve("CPUCardAPDU");
    FUNC_POINTER_ERROR_RETURN(CPUCardAPDU);

    ICReaderLEDCtrl = (FNICReaderLEDCtrl)m_sdkLibrary.resolve("ICReaderLEDCtrl");
    FUNC_POINTER_ERROR_RETURN(ICReaderLEDCtrl);

    return TRUE;
}
