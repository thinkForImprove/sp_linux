#include "LinuxUSBPort.h"

static const char *ThisFile = "LinuxUSBPort.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIDevPort(LPCSTR lpDevName, LPCSTR lpDevType, IDevPort *&pDev)
{
    pDev = new CLinuxUSBPort(lpDevName, lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CLinuxUSBPort::CLinuxUSBPort(LPCSTR lpDevName, LPCSTR lpDevType): m_cLog(lpDevName)
{
    m_pUsb_handle = NULL;
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    m_SemCancel.ResetSem();
}

CLinuxUSBPort::~CLinuxUSBPort()
{
    if(m_pUsb_handle != NULL)
    {
        m_pUsb_handle = NULL;
    }
}

void CLinuxUSBPort::Release()
{

}

long CLinuxUSBPort::Open(LPCSTR lpMode, bool bExclusiveUse/* = true*/)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);

    m_bIsOpen = false;
    STUSBPARAM stParam;
    if (0 != GetUSBParam(lpMode, stParam))
    {
        Log(ThisModule, __LINE__, "GetUSBParam Error: lpMode=%s", lpMode);
        m_cLog.NewLine().Log("OPEN FAILED PARAM:").Log(lpMode).EndLine();
        return ERR_PORT_PARAM;
    }

    QString strMode;
    strMode.sprintf("VID:%04X, PID:%04X", stParam.usVID, stParam.usPID);
    m_cLog.NewLine().Log(strMode.toStdString().c_str());

    if (IsOpened())
        Close();

    usb_device *devh = usb_open(stParam.usVID, stParam.usPID, NULL);
    if(devh != NULL)
    {
        m_pUsb_handle = devh;
        usb_set_nonblocking(devh,0);
        m_bIsOpen = true;
    }
    else
    {
        m_pUsb_handle = NULL;
        return FALSE;
    }

    m_SemCancel.ResetSem();
    return ERR_PORT_SUCCESS;
}

long CLinuxUSBPort::Close()
{
    //THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);

    // 释放句柄，否则会崩溃
    if (IsOpened())
    {
        if(m_pUsb_handle == NULL)
        {
            return FALSE;
        }

        usb_close(m_pUsb_handle);
        m_bIsOpen = false;
        if (m_pUsb_handle != NULL)
            m_pUsb_handle = NULL;
    }

    return ERR_PORT_SUCCESS;
}

long CLinuxUSBPort::Send(LPCSTR lpData, DWORD dwDataLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);
    Q_UNUSED(dwTimeOut)

    if(m_pUsb_handle == NULL)
    {
        Log(ThisModule, __LINE__, "usb not open");
        m_cLog.NewLine().Log("SEND[0] FAILED NOTOPEN").EndLine();
        return ERR_PORT_NOTOPEN;
    }

    if(dwDataLen == 0 || lpData == NULL)
    {
        Log(ThisModule, __LINE__, "param error");
        m_cLog.NewLine().Log("SEND[0] FAILED PARAM").EndLine();
        return ERR_PORT_PARAM;
    }

    // 记录日志
    char szTmp[64] = {0};
    sprintf(szTmp, "SEND[%3u]:", dwDataLen);
    m_cLog.NewLine().Log(szTmp).LogHex(lpData, dwDataLen).EndLine();

    int nRetLen = usb_write(m_pUsb_handle, (BYTE *)lpData, dwDataLen);
    if(nRetLen < 0)
    {
        Log(ThisModule, __LINE__, "Send Data len error nRetLen=%d", nRetLen);
        sprintf(szTmp, "FAILED LEN[S%3u != W%3u]", dwDataLen, nRetLen);
        m_cLog.NewLine().Log(szTmp).EndLine();
        return ERR_PORT_WRITE;
    }

    return ERR_PORT_SUCCESS;
}

long CLinuxUSBPort::Read(LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    if(m_pUsb_handle == NULL)
    {
        return -1;
    }

    if(lpReadData == NULL)
    {
        return -1;
    }
    if(dwInOutReadLen < USB_READ_LENTH){
        return -1;
    }

    int nRetLen = 0;
    BYTE actBuf[64];

    memset(actBuf, 0x00, sizeof(actBuf));

    nRetLen = usb_read_timeout(m_pUsb_handle, actBuf, USB_READ_LENTH, dwTimeOut);

    if(nRetLen > 0){
        memcpy(lpReadData, (LPSTR)actBuf, nRetLen);
    } else if(nRetLen < 0) {
        return -1;
    }

//    if (nRetLen > dwInOutReadLen)
//    {
//        memcpy(lpReadData, (LPSTR)actBuf, dwInOutReadLen);
//        dwInOutReadLen = strlen(lpReadData) + 1;
//    } else if (nRetLen <= dwInOutReadLen && nRetLen > 0)
//    {
//        memcpy(lpReadData, (LPSTR)actBuf, nRetLen);   //防止溢出
//        dwInOutReadLen = strlen(lpReadData) + 1;
//    } else
//    {
//        return FALSE;
//    }

    // 记录日志
    char szTmp[64] = {0};
    sprintf(szTmp, "RECV[%3u]:", nRetLen);
    m_cLog.NewLine().Log(szTmp).LogHex(lpReadData, nRetLen).EndLine();

    return nRetLen;
}

long CLinuxUSBPort::SendAndRead(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);

    try
    {
        long lRet = Send(lpSendData, dwSendLen, dwTimeOut);
        if (lRet <= 0)
            return lRet;
        return Read(lpReadData, dwInOutReadLen, dwTimeOut);
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "收发数据异常失败");
        return ERR_PORT_NODEFINED;
    }
}

void CLinuxUSBPort::CancelRead()
{
    m_SemCancel.SetSem();
}

void CLinuxUSBPort::ClearBuffer()
{

}

bool CLinuxUSBPort::IsOpened()
{
    AutoMutex(m_cMutex);
    return m_bIsOpen;
}

void CLinuxUSBPort::SetLogAction(LPCSTR lpActName)
{
    m_cLog.LogAction(lpActName);
}

void CLinuxUSBPort::FlushLog()
{
    m_cLog.FlushLog();
}

void CLinuxUSBPort::CancelLog()
{
    m_cLog.CancelLog();
}

long CLinuxUSBPort::GetUSBParam(LPCSTR lpMode, STUSBPARAM &stParam)
{
    QString strMode(lpMode);
    QStringList strList = strMode.split(",");
    if (strList.size() != 2)
        return ERR_PORT_PARAM;

    stParam.usVID = (uint16_t)strList[0].toUInt();
    stParam.usPID = (uint16_t)strList[1].toUInt();
    return ERR_PORT_SUCCESS;
}
