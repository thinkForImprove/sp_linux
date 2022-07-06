#include "MTDriver.h"

static const char *ThisFile = "MTDriver.cpp";
//////////////////////////////////////////////////////////////////////////
CMTDriver::CMTDriver()
{
    SetLogFile("FIDC.MT.Driver.log", ThisFile);
    memset(m_szDesc, 0x00, sizeof(m_szDesc));
    memset(m_szDevName, 0x00, sizeof(m_szDevName));
}

CMTDriver::~CMTDriver()
{
    DriverDllRelease();
}

long CMTDriver::DriverDllLoad()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cSysMutex);

    if (m_mtDll.isLoaded())
    {
        Log(ThisModule, __LINE__, "已加载libCnSysReader.so库");
        return 0;
    }

    QString strDllName = "libCnSysReader.so";
#ifdef QT_WIN32
    strDllName = "C:/CFES/BIN/libCnSysReader.dll";
#else
    strDllName.prepend(LINUXPATHLIB);;
#endif
    m_mtDll.setFileName(strDllName);
    if (!m_mtDll.load())
    {
        Log(ThisModule, __LINE__, "libCnSysReader.so load failed[%s].", m_mtDll.errorString().toUtf8().constData());
        return -1;
    }

     OpenUsbDevice = (FPOpenUsbDevice)m_mtDll.resolve("open_usb_device");
     GetDeviceVersion= (FPGetDeviceVersion)m_mtDll.resolve("close_device");
     PiccPowerOff = (FPPiccPowerOff)m_mtDll.resolve("picc_poweroff");
     CloseDevice = (FPPiccPowerOff)m_mtDll.resolve("close_device");
     PiccApdu = (FPPiccApdu)m_mtDll.resolve("picc_apdu");
     PiccPowerOn = (FPPiccPowerOn)m_mtDll.resolve("picc_poweron");
     PiccStatus =(FPPiccStatus)m_mtDll.resolve("picc_status");
     DeviceBeep = (FPDeviceBeep)m_mtDll.resolve("device_beep");

     if(OpenUsbDevice == nullptr || GetDeviceVersion == nullptr || PiccPowerOff == nullptr ||
        CloseDevice == nullptr || PiccApdu == nullptr || PiccPowerOn == nullptr ||
        PiccStatus == nullptr || DeviceBeep == nullptr)
     {
         Log(ThisModule, __LINE__, "inteface load failed");
         return -2;
     }
     return 0;
}

void CMTDriver::DriverDllRelease()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_mtDll.isLoaded())
    {
        m_mtDll.unload();
        OpenUsbDevice = nullptr;
        CloseDevice = nullptr;
        GetDeviceVersion = nullptr;
        PiccPowerOff = nullptr;
        PiccApdu = nullptr;
        PiccPowerOn = nullptr;
        PiccStatus = nullptr;
        DeviceBeep = nullptr;
    }
    return ;
}

LPCSTR CMTDriver::GetErrDesc(ULONG ulErrCode)
{
    memset(m_szDesc, 0x00, sizeof(m_szDesc));
    switch (ulErrCode)
    {
    default:
        break;
    }
    return m_szDesc;
}
