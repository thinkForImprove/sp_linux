#include "USBDrive.h"

static const char *ThisFile = "USBDrive.cpp";
//////////////////////////////////////////////////////////////////////////
CUSBDrive::CUSBDrive(LPCSTR lpDevType): m_ulUSBHandle(0x00), m_cSysMutex("IOMC_USBDrive_2020")
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    memset(m_szDesc, 0x00, sizeof(m_szDesc));
    memset(m_szDevName, 0x00, sizeof(m_szDevName));
}

CUSBDrive::~CUSBDrive()
{
    USBDllRelease();
}

long CUSBDrive::USBDllLoad()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cSysMutex);

    if (m_cUsbDll.isLoaded())
    {
        Log(ThisModule, __LINE__, "已加载VHUSB.dll库");
        return 0;
    }

    QString strDllName;
#ifdef QT_WIN32
    strDllName = "C:/CFES/BIN/VHUSB.dll";
#else
    strDllName = "/hots/lib/VHUSB.so";
#endif
    m_cUsbDll.setFileName(strDllName);
    if (!m_cUsbDll.load())
    {
        Log(ThisModule, __LINE__, "VHUSB.dll加载失败：%s", m_cUsbDll.errorString().toStdString().c_str());
        return -1;
    }

    m_pFnATMUSB  = (HT_FnATMUSB)m_cUsbDll.resolve("FnATMUSB");
    m_pInfATMUSB = (HT_InfATMUSB)m_cUsbDll.resolve("InfATMUSB");
    if (nullptr == m_pFnATMUSB || nullptr == m_pInfATMUSB)
    {
        USBDllRelease();
        Log(ThisModule, __LINE__, "接口加载失败");
        return -2;
    }

    return 0;
}

void CUSBDrive::USBDllRelease()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_cUsbDll.isLoaded())
    {
        m_cUsbDll.unload();
        m_pInfATMUSB = nullptr;
        m_pFnATMUSB  = nullptr;
    }
    return ;
}

long CUSBDrive::USBOpen(const char *pDevName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (pDevName == nullptr || strlen(pDevName) == 0)
    {
        Log(ThisModule, __LINE__, "无效设备名称");
        return -1;
    }

    if (m_ulUSBHandle != 0 && qstricmp(pDevName, m_szDevName) == 0)
    {
        Log(ThisModule, __LINE__, "已打开%s的USB连接", m_szDevName);
        return 0;
    }

    STR_DRV strDrv;
    memset(&strDrv, 0x00, sizeof(STR_DRV));
    m_ulUSBHandle = 0x00;
    strcpy(m_szDevName, pDevName);// 对应的USB设备名称

    // 打开连接参数
    strDrv.usParam = USB_PRM_OPEN;
    strDrv.pvDataInBuffPtr = &m_szDevName[0];
    strDrv.uiDataInBuffSz = qstrlen(m_szDevName);
    strDrv.pvDataOutBuffPtr = &m_ulUSBHandle;
    strDrv.uiDataOutReqSz = 4;
//    strDrv.uiTimer = 60;
    strDrv.uiTimer = 1;

    long lRet = USBDrvCall(USB_DRV_INF_OPEN, &strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "打开 \"%s\" 的USB连接失败", m_szDevName);
        return lRet;
    }
    return 0;
}

long CUSBDrive::USBClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_ulUSBHandle == 0x00)
    {
        Log(ThisModule, __LINE__, "已关闭USB连接");
        return 0;
    }

    STR_DRV strDrv;
    memset(&strDrv, 0x00, sizeof(STR_DRV));

    // 关闭连接参数
    strDrv.usParam  = USB_PRM_CLOSE;
    strDrv.uiDrvHnd = m_ulUSBHandle;// 对应的句柄
    long lRet = USBDrvCall(USB_DRV_INF_CLOSE, &strDrv);
    if (lRet != 0)
    {
        m_ulUSBHandle = 0;
        Log(ThisModule, __LINE__, "关闭 \"%s\" 的USB连接失败", m_szDevName);
        return lRet;
    }

    m_ulUSBHandle = 0x00;
    memset(m_szDevName, 0x00, sizeof(m_szDevName));
    return 0;
}

bool CUSBDrive::IsOpen()
{
    return (m_ulUSBHandle != 0);
}

long CUSBDrive::USBDrvCall(WORD wCmd, PSTR_DRV pParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cSysMutex);

    if (pParam == nullptr || m_pFnATMUSB == nullptr || m_pInfATMUSB == nullptr)
    {
        Log(ThisModule, __LINE__, "参数指针为NULL，或没有加载USB驱动库");
        return -1;
    }
    if (m_ulUSBHandle == 0x00 && wCmd != USB_DRV_INF_OPEN)
    {
        Log(ThisModule, __LINE__, "没有打开USB连接");
        return -2;
    }

    pParam->uiDrvHnd  = m_ulUSBHandle;
    HT_FnATMUSB pFunc = nullptr;
    switch (wCmd)
    {
    case USB_DRV_FN_DATASEND:
    case USB_DRV_FN_DATARCV:
    case USB_DRV_FN_DATASENDRCV:
    case USB_DRV_FN_USBRESET:
        pFunc = m_pFnATMUSB;
        break;
    case USB_DRV_INF_OPEN:
    case USB_DRV_INF_CLOSE:
    case USB_DRV_INF_INFGET:
    case USB_DRV_INF_SENS:
        pFunc = m_pInfATMUSB;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持命令参数: wCmd = %d", wCmd);
        return -3;
    }

    ULONG uRet = pFunc(wCmd, pParam);
    if (uRet != ERR_DRV_USB_SUCCESS)
    {
        Log(ThisModule, __LINE__, "调用命令返回失败：[wCmd=%d,sParam=%d]lRet=0x%02X[ %s ]", wCmd, pParam->usParam, uRet, GetErrDesc(uRet));
        if(uRet == (ULONG)ERR_DRV_USB_DRV_REMOVE ||
           uRet == (ULONG)ERR_DRV_USB_DRVHND_DIFFER){
            return -5;
        }
        return -4;
    }
    return 0;
}

void CUSBDrive::SetFnATMUSB(HT_FnATMUSB pFnATMUSB, HT_InfATMUSB pInfATMUSB)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pFnATMUSB = pFnATMUSB;
    m_pInfATMUSB = pInfATMUSB;
    return;
}

HT_FnATMUSB CUSBDrive::GetFnATMUSB()
{
    return m_pFnATMUSB;
}

HT_InfATMUSB CUSBDrive::GetInfATMUSB()
{
    return m_pInfATMUSB;
}

LPCSTR CUSBDrive::GetErrDesc(ULONG ulErrCode)
{
    memset(m_szDesc, 0x00, sizeof(m_szDesc));
    switch (ulErrCode)
    {
    case ERR_DRV_USB_CANCEL_END: strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_END"); break;
    case ERR_DRV_USB_CANCEL_NOPST: strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_NOPST"); break;
    case ERR_DRV_USB_CANCEL_CROSS_END: strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_CROSS_END"); break;
    case ERR_DRV_USB_CANCEL_NOTHING: strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_NOTHING"); break;
    case ERR_DRV_USB_FUNC: strcpy(m_szDesc, "ERR_DRV_USB_FUNC"); break;
    case ERR_DRV_USB_PRM: strcpy(m_szDesc, "ERR_DRV_USB_PRM"); break;
    case ERR_DRV_USB_DRVHND_DIFFER: strcpy(m_szDesc, "ERR_DRV_USB_DRVHND_DIFFER"); break;
    case ERR_DRV_USB_DRV_REMOVE: strcpy(m_szDesc, "ERR_DRV_USB_DRV_REMOVE"); break;
    case ERR_DRV_USB_BLD: strcpy(m_szDesc, "ERR_DRV_USB_BLD"); break;
    case ERR_DRV_USB_INDATA: strcpy(m_szDesc, "ERR_DRV_USB_INDATA"); break;
    case ERR_DRV_USB_OUTDATA: strcpy(m_szDesc, "ERR_DRV_USB_OUTDATA"); break;
    case ERR_DRV_USB_INOUTDATA: strcpy(m_szDesc, "ERR_DRV_USB_INOUTDATA"); break;
    case ERR_DRV_USB_ENTRY_DEVICE_OVER: strcpy(m_szDesc, "ERR_DRV_USB_ENTRY_DEVICE_OVER"); break;
    case ERR_DRV_USB_ENTRY_THREAD_OVER: strcpy(m_szDesc, "ERR_DRV_USB_ENTRY_THREAD_OVER"); break;
    case ERR_DRV_USB_BCC: strcpy(m_szDesc, "ERR_DRV_USB_BCC"); break;
    case ERR_DRV_USB_INDATA_BUFFSZ: strcpy(m_szDesc, "ERR_DRV_USB_INDATA_BUFFSZ"); break;
    case ERR_DRV_USB_OUTDATA_BUFFSZ: strcpy(m_szDesc, "ERR_DRV_USB_OUTDATA_BUFFSZ"); break;
    case ERR_DRV_USB_INOUTDATA_BUFFSZ: strcpy(m_szDesc, "ERR_DRV_USB_INOUTDATA_BUFFSZ"); break;
    case ERR_DRV_USB_LINE_TIMEOUT: strcpy(m_szDesc, "ERR_DRV_USB_LINE_TIMEOUT"); break;
    case ERR_DRV_USB_COMMAND_TIMEOUT: strcpy(m_szDesc, "ERR_DRV_USB_COMMAND_TIMEOUT"); break;
    case ERR_DRV_USB_CLOSE: strcpy(m_szDesc, "ERR_DRV_USB_CLOSE"); break;
    case ERR_DRV_USB_OPEN_BUSY: strcpy(m_szDesc, "ERR_DRV_USB_OPEN_BUSY"); break;
    case ERR_DRV_USB_SEND_BUSY: strcpy(m_szDesc, "ERR_DRV_USB_SEND_BUSY"); break;
    case ERR_DRV_USB_RCV_BUSY: strcpy(m_szDesc, "ERR_DRV_USB_RCV_BUSY"); break;
    case ERR_DRV_USB_EP_DOWN: strcpy(m_szDesc, "ERR_DRV_USB_EP_DOWN"); break;
    case ERR_DRV_USB_MEMORY: strcpy(m_szDesc, "ERR_DRV_USB_MEMORY"); break;
    case ERR_DRV_USB_HANDLE: strcpy(m_szDesc, "ERR_DRV_USB_HANDLE"); break;
    case ERR_DRV_USB_REG: strcpy(m_szDesc, "ERR_DRV_USB_REG"); break;
    case ERR_DRV_USB_DRVCALL: strcpy(m_szDesc, "ERR_DRV_USB_DRVCALL"); break;
    case ERR_DRV_USB_THREAD: strcpy(m_szDesc, "ERR_DRV_USB_THREAD"); break;
    case ERR_DRV_USB_POSTMSG: strcpy(m_szDesc, "ERR_DRV_USB_POSTMSG"); break;
    case ERR_DRV_USB_TRACE: strcpy(m_szDesc, "ERR_DRV_USB_TRACE"); break;
    default:                sprintf(m_szDesc, "0x%08.08X", ulErrCode); break;
    }
    return m_szDesc;
}
