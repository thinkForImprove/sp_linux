#include "LinuxHIDPort.h"

static const char *ThisFile = "LinuxHIDPort";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIDevPort(LPCSTR lpDevName, LPCSTR lpDevType, IDevPort *&pDev)
{
    pDev = new CLinuxHIDPort(lpDevName, lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CLinuxHIDPort::CLinuxHIDPort(LPCSTR lpDevName, LPCSTR lpDevType): m_cLog(lpDevName)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
}

CLinuxHIDPort::~CLinuxHIDPort()
{

}

void CLinuxHIDPort::Release()
{

}

long CLinuxHIDPort::Open(LPCSTR lpMode)
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

    do
    {
        m_pdev = hid_open(stParam.usVID, stParam.usPID, nullptr);

        if (m_pdev == nullptr)
        {
            m_cLog.NewLine().Log("OPEN FAILED").EndLine();
            Log(ThisModule, ERR_PORT_NOTOPEN, "open device failed");
            break;
        }

        m_bIsOpen = true;
        // Set the hid_read() function to be non-blocking.
        hid_set_nonblocking(m_pdev, 1);
        m_cLog.NewLine().Log("OPEN SUCCEED").EndLine();
    } while (false);

    if (!m_bIsOpen)
    {
        m_cLog.NewLine().Log("OPEN FAILED").EndLine();
        return ERR_PORT_NOTOPEN;
    }
    fflush(stdout);
    return ERR_PORT_SUCCESS;
}

long CLinuxHIDPort::Close()
{
    //THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);
    if (m_pdev != nullptr)
    {
        hid_close(m_pdev);
        m_bIsOpen = false;
        hid_exit();
        m_pdev = nullptr;
        m_cLog.NewLine().Log("CLOSE SUCCEED").EndLine();
    }
    else
    {
        m_cLog.NewLine().Log("NO OPEN").EndLine();
    }
    m_bIsOpen = false;
    return ERR_PORT_SUCCESS;
}

long CLinuxHIDPort::Send(LPCSTR lpData, DWORD dwDataLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);
    if (lpData == nullptr || dwDataLen <= 0)
    {
        Log(ThisModule, __LINE__, "param error");
        m_cLog.NewLine().Log("SEND[0] FAILED PARAM").EndLine();
        return ERR_PORT_PARAM;
    }
    if (m_pdev == nullptr)
    {
        Log(ThisModule, __LINE__, "usb not open");
        m_cLog.NewLine().Log("SEND[0] FAILED NOTOPEN").EndLine();
        return ERR_PORT_NOTOPEN;
    }

    if (dwDataLen == INFINITE)
        dwDataLen = strlen(lpData);

    // 记录日志
    char szTmp[64] = {0};
    sprintf(szTmp, "SEND[%3u]:", dwDataLen);
    m_cLog.NewLine().Log(szTmp).LogHex(lpData, dwDataLen);

    //int nRet = libusb_control_transfer(m_hComm, uRequestType, uRequest, wValue, 0x00, (BYTE*)lpData, dwDataLen, dwTimeOut);
    int nRet = hid_write(m_pdev, (const unsigned char *)lpData, dwDataLen);
    fflush(stdout);
    usleep(100 * 1000);
    if (nRet == (int)dwDataLen)
    {
        return ERR_PORT_SUCCESS;
    }
    else if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "Send Data error nRet=%d", nRet);
        m_cLog.Log("FAILED WRITE").EndLine();
        return ERR_PORT_WRITE;
    }
    else if (nRet != (int)dwDataLen)
    {
        Log(ThisModule, __LINE__, "Send Data len error nRet=%d", nRet);
        char szTmp[64] = {0};
        sprintf(szTmp, "FAILED LEN[S%3u != W%3u]", dwDataLen, nRet);
        m_cLog.Log(szTmp).EndLine();
        return ERR_PORT_WRITE;
    }

    Log(ThisModule, __LINE__, "Send Data error nRet=%d", nRet);
    m_cLog.Log("FAILED NODEFINED").EndLine();
    return ERR_PORT_NODEFINED;
}

long CLinuxHIDPort::Read(LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);
    if (lpReadData == nullptr || dwInOutReadLen == 0)
    {
        Log(ThisModule, -1, "param error");
        m_cLog.NewLine().Log("RECV[0] FAILED PARAM").EndLine();
        return ERR_PORT_PARAM;
    }
    if (m_pdev == nullptr)
    {
        Log(ThisModule, -1, "usb not open");
        m_cLog.NewLine().Log("RECV[0] FAILED NOTOPEN").EndLine();
        return ERR_PORT_NOTOPEN;
    }

#define MAX_READ_LEN_ONCE  (256)

    DWORD dwReadLen = 0;
    //int nRet = libusb_bulk_transfer(m_hComm, CTRL_IN,((BYTE *)lpReadData), dwInOutReadLen, &nReadLen, dwTimeOut);

    int nRet = 0;
    int nRetryTime = 0;
    int nReadLen = 0;

    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &endTime);
	if(dwTimeOut > 0){
		endTime.tv_sec += dwTimeOut / 1000;
    	endTime.tv_nsec += dwTimeOut % 1000 * 1000000;
		if(endTime.tv_nsec > 1000 * 1000000){
			endTime.tv_sec++;
			endTime.tv_nsec -= 1000 * 1000000;
		}
	}
    
    do
    {
        DWORD dwLeft = dwInOutReadLen - dwReadLen;
        if(dwLeft <= 0){
            break;
        }
        int nMaxReadLen = dwLeft < MAX_READ_LEN_ONCE ? dwLeft  : MAX_READ_LEN_ONCE;
        //nRet = hid_read_timeout(m_pdev, (unsigned char*)(lpReadData + dwReadLen), nMaxReadLen, dwTimeOut);
        nRet = hid_read(m_pdev, (unsigned char *)(lpReadData + dwReadLen), nMaxReadLen);
        fflush(stdout);

        if(nRet > 0){
            dwReadLen += nRet;
        } else if(nRet == 0){
            if(dwReadLen > 0){
                break;
            }
        } else {
            //出错
            break;
        }

        //判断是否到超时时间
        struct timespec curTime;
        clock_gettime(CLOCK_REALTIME, &curTime);
        if(curTime.tv_sec > endTime.tv_sec ||
           (curTime.tv_sec == endTime.tv_sec && curTime.tv_nsec > endTime.tv_nsec)){
            break;
        }
        usleep(10);
    } while (true);

    dwInOutReadLen = dwReadLen;
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "Read Data error Len = 0");
        m_cLog.NewLine().Log("RECV[0]: FAILED READERR").EndLine();
        return ERR_PORT_READERR;
    }

    if (dwInOutReadLen != 0)
    {
        // 记录日志
        char szTmp[64] = {0};
        sprintf(szTmp, "RECV[%3u]:", dwInOutReadLen);
        m_cLog.NewLine().Log(szTmp).LogHex(lpReadData, dwInOutReadLen).EndLine();
    } else {
        return ERR_PORT_RTIMEOUT;
    }
    return ERR_PORT_SUCCESS;
}

long CLinuxHIDPort::SendAndRead(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);

    try
    {
        long lRet = Send(lpSendData, dwSendLen, dwTimeOut);
        if (lRet < 0)
            return lRet;
        return Read(lpReadData, dwInOutReadLen, dwTimeOut);
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "收发数据异常失败");
        return ERR_PORT_NODEFINED;
    }
}

void CLinuxHIDPort::CancelRead()
{

}

void CLinuxHIDPort::ClearBuffer()
{

}

bool CLinuxHIDPort::IsOpened()
{
    AutoMutex(m_cMutex);
    return m_bIsOpen;
}

void CLinuxHIDPort::SetLogAction(LPCSTR lpActName)
{
    m_cLog.LogAction(lpActName);
}

void CLinuxHIDPort::FlushLog()
{
    m_cLog.FlushLog();
}

void CLinuxHIDPort::CancelLog()
{
    m_cLog.CancelLog();
}

long CLinuxHIDPort::GetUSBParam(LPCSTR lpMode, STUSBPARAM &stParam)
{
    QString strMode(lpMode);
    QStringList strList = strMode.split(",");
    if (strList.size() != 2)
        return ERR_PORT_PARAM;

    stParam.usVID = (uint16_t)strList[0].toUInt();
    stParam.usPID = (uint16_t)strList[1].toUInt();
    return ERR_PORT_SUCCESS;
}
