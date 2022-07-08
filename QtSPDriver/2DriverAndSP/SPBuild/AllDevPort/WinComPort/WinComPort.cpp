#include "WinComPort.h"

static const char *ThisFile = "WinComPort.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIDevPort(LPCSTR lpDevName, LPCSTR lpDevType, IDevPort *&pDev)
{
    pDev = new CWinComPort(lpDevName, lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CWinComPort::CWinComPort(LPCSTR lpDevName, LPCSTR lpDevType): m_cLog(lpDevName)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    m_hCom = INVALID_HANDLE_VALUE;
}

CWinComPort::~CWinComPort()
{
    Close();
}

void CWinComPort::Release()
{

}

long CWinComPort::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cFuncMutex);
    if (lpMode == nullptr || strnicmp(lpMode, "COM", 3) != 0)
    {
        Log(ThisModule, __LINE__, "参数错误: %s", lpMode);
        m_cLog.NewLine().Log("OPEN FAILED PARAM:").Log(lpMode).EndLine();
        return ERR_PORT_PARAM;
    }

    //打开一个串口
    if (IsOpened())
    {
        if (m_strOpenMode == lpMode)
        {
            m_cLog.NewLine().Log("ALREADY OPEN PARAM:").Log(lpMode).EndLine();
            return ERR_PORT_SUCCESS;
        }
        else
        {
            Close();
        }
    }
    m_strOpenMode = lpMode;
    string::size_type nPos = m_strOpenMode.find(":");
    if (string::npos == nPos)
    {
        Log(ThisModule, __LINE__, "参数错误: %s", lpMode);
        m_cLog.NewLine().Log("OPEN FAILED PARAM:").Log(lpMode).EndLine();
        return ERR_PORT_PARAM;
    }
    m_cLog.NewLine().Log(lpMode);

    string strPort = "\\\\.\\" + m_strOpenMode.substr(0, nPos);
    QString strWPort = QString::fromLocal8Bit(strPort.c_str());
    m_hCom = ::CreateFile(strWPort.toStdWString().c_str(),
                          GENERIC_WRITE | GENERIC_READ,
                          0,                     // comm devices must be opened w/exclusive-access
                          nullptr,
                          OPEN_EXISTING,         // comm devices must use OPEN_EXISTING
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                          nullptr);                 // hTemplate must be NULL for comm devices
    if (m_hCom == INVALID_HANDLE_VALUE)
    {
        Log(ThisModule, __LINE__, "打开串口失败：Port=%s", strPort.c_str());
        m_cLog.NewLine().Log("OPEN FAILED").EndLine();
        return ERR_PORT_FAIL;
    }

    // 设置缓存(4K)
    ::SetupComm(m_hCom, MAX_BUFFER_SIZE, MAX_BUFFER_SIZE);

    // 设置超时时间:总超时＝时间系数× 要求读/写的字符数＋时间常量
    // ReadIntervalTimeout定义两个字符到达的最大时间间隔，单位：毫秒
    // 当读取完一个字符后，超过了ReadIntervalTimeout，仍未读取到下一个字符，就会发生超时
    COMMTIMEOUTS ts;
    ::GetCommTimeouts(m_hCom, &ts);
    ts.ReadIntervalTimeout         = 30;      //读间隔超时
    ts.ReadTotalTimeoutMultiplier  = 0;       //读时间系数
    ts.ReadTotalTimeoutConstant    = 30 * 1000; //读时间常量
    ts.WriteTotalTimeoutMultiplier = 0;       //写时间系数
    ts.WriteTotalTimeoutConstant   = 30 * 1000; //写时间常量
    ::SetCommTimeouts(m_hCom, &ts);

    // 设置串口的设备控制块
    QString strWMode = QString::fromLocal8Bit(lpMode);
    DCB dcb;
    ::GetCommState(m_hCom, &dcb);
    ::BuildCommDCB(strWMode.toStdWString().c_str(), &dcb);
    ::SetCommState(m_hCom, &dcb);
    // 复位信号
    ::ResetEvent(m_cCancleEvent.GetHandle());

    m_cLog.NewLine().Log("OPEN SUCCEED").EndLine();
    m_strOpenMode = lpMode;
    return ERR_PORT_SUCCESS;
}

long CWinComPort::Close()
{
    //THISMODULE(__FUNCTION__);
    AutoMutex(m_cFuncMutex);
    if (m_hCom != INVALID_HANDLE_VALUE)
    {
        CancelRead();
        CloseHandle(m_hCom);
        m_hCom = INVALID_HANDLE_VALUE;
        m_cLog.NewLine().Log("CLOSE SUCCEED").EndLine();
    }
    else
    {
        m_cLog.NewLine().Log("NO OPEN NO CLOSE").EndLine();
    }
    m_strOpenMode = "";
    return ERR_PORT_SUCCESS;
}

long CWinComPort::Send(LPCSTR lpData, DWORD dwDataLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cFuncMutex);
    if (lpData == nullptr || dwDataLen == 0)
    {
        Log(ThisModule, __LINE__, "参数错误");
        m_cLog.NewLine().Log("SEND[0] FAILED PARAM").EndLine();
        return ERR_PORT_PARAM;
    }
    if (m_hCom == INVALID_HANDLE_VALUE)
    {
        Log(ThisModule, __LINE__, "串口没打开");
        m_cLog.NewLine().Log("SEND[0] FAILED NOTOPEN").EndLine();
        return ERR_PORT_PARAM;
    }

    //保证缓冲区的所有字符都被发送
    //该函数只受流量控制的支配，不受超时控制的支配，它在所有的写操作完成后才返回。
    ::FlushFileBuffers(m_hCom);

    //清除所有读写有关的状态(如:输入, 输出缓存, 终止所有发送, 接收)
    ::PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);

    // 设置超时时间
    SetTimeOut(dwTimeOut);

    // 记录日志
    char szTmp[64] = {0};
    sprintf(szTmp, "SEND[%3u]:", dwDataLen);
    m_cLog.NewLine().Log(szTmp).LogHex(lpData, dwDataLen);

    // 开始发数据
    DWORD dwWriteLen = 0;
    OVERLAPPED Overlapped;
    ZeroMemory(&Overlapped, sizeof(Overlapped));
    Overlapped.hEvent = m_cOverlappedWriteEvent.GetHandle();
    ::ResetEvent(Overlapped.hEvent);
    if (::WriteFile(m_hCom, lpData, dwDataLen, &dwWriteLen, &Overlapped))
    {
        if (dwWriteLen < dwDataLen)
        {
            Log(ThisModule, __LINE__, "发数据错误dwLenWritten=%u, dwLen=%u", dwWriteLen, dwDataLen);
            char szTmp[64] = {0};
            sprintf(szTmp, "FAILED LEN[S%3u != W%3u]", dwDataLen, dwWriteLen);
            m_cLog.Log(szTmp).EndLine();
            return ERR_PORT_WRITE;
        }
        m_cLog.EndLine();
        return dwWriteLen;
    }
    else
    {
        if (::GetLastError() != ERROR_IO_PENDING)//异步写数据操作还没完成
        {
            Log(ThisModule, __LINE__, "发数据错误");
            m_cLog.Log("FAILED NODEFINED").EndLine();
            return ERR_PORT_NODEFINED;
        }

        DWORD dwRet = ::WaitForSingleObject(Overlapped.hEvent, dwTimeOut);
        if (dwRet == WAIT_OBJECT_0)
        {
            //当串口写操作进行完毕后，Overlapped 的 hEvent 事件会变为有信号
            ::GetOverlappedResult(m_hCom, &Overlapped, &dwWriteLen, FALSE);
            if (dwWriteLen < dwDataLen)
            {
                Log(ThisModule, __LINE__, "发数据错误dwLenWritten=%u, dwLen=%u", dwWriteLen, dwDataLen);
                char szTmp[64] = {0};
                sprintf(szTmp, "FAILED LEN[S%3u != W%3u]", dwDataLen, dwWriteLen);
                m_cLog.Log(szTmp).EndLine();
                return ERR_PORT_WRITE;
            }

            m_cLog.EndLine();
            return dwWriteLen;
        }
        else if (dwRet == WAIT_TIMEOUT)
        {
            Log(ThisModule, __LINE__, "发数据超时");
            m_cLog.Log("FAILED TIMEOUT").EndLine();
            return ERR_PORT_WTIMEOUT;
        }
        else
        {
            Log(ThisModule, __LINE__, "发数据错误");
            m_cLog.Log("FAILED NODEFINED").EndLine();
            return ERR_PORT_NODEFINED;
        }
    }
}

long CWinComPort::Read(LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cFuncMutex);
    if (lpReadData == nullptr || dwInOutReadLen == 0)
    {
        Log(ThisModule, __LINE__, "参数错误");
        m_cLog.NewLine().Log("RECV[0] FAILED PARAM").EndLine();
        return ERR_PORT_PARAM;
    }
    if (m_hCom == INVALID_HANDLE_VALUE)
    {
        Log(ThisModule, __LINE__, "串口没打开");
        m_cLog.NewLine().Log("RECV[0] FAILED NOTOPEN").EndLine();
        return ERR_PORT_NOTOPEN;
    }

    // 修正读取最大长度
    if (dwInOutReadLen > MAX_BUFFER_SIZE)
        dwInOutReadLen = MAX_BUFFER_SIZE;

    // 设置超时时间
    SetTimeOut(dwTimeOut, true);

    // 开始读数据
    DWORD dwReadLen = 0;
    OVERLAPPED Overlapped;
    ZeroMemory(&Overlapped, sizeof(Overlapped));
    Overlapped.hEvent = m_cOverlappedReadEvent.GetHandle();
    ::ResetEvent(Overlapped.hEvent);
    ::ResetEvent(m_cCancleEvent.GetHandle());
    if (::ReadFile(m_hCom, lpReadData, dwInOutReadLen, &dwReadLen, &Overlapped))
    {
        if (dwReadLen == 0)
        {
            Log(ThisModule, __LINE__, "读数据失败");
            m_cLog.NewLine().Log("RECV[0]: FAILED READERR").EndLine();
            return ERR_PORT_READERR;
        }
        dwInOutReadLen = dwReadLen;
        return dwReadLen;
    }
    else
    {
        if (::GetLastError() != ERROR_IO_PENDING)//异步读数据操作还没完成
        {
            Log(ThisModule, __LINE__, "读数据错误");
            m_cLog.NewLine().Log("RECV[0]: FAILED NODEFINED").EndLine();
            return ERR_PORT_NODEFINED;
        }

        HANDLE szhEvent[] = {Overlapped.hEvent, m_cCancleEvent.GetHandle()};
        DWORD dwCount     = sizeof(szhEvent) / sizeof(szhEvent[0]);
        DWORD dwRet = ::WaitForMultipleObjects(dwCount, szhEvent, FALSE, dwTimeOut);
        if (dwRet == WAIT_OBJECT_0)
        {
            //当串口读操作进行完毕后(包含超时), Overlapped的hEvent事件会变为有信号
            ::GetOverlappedResult(m_hCom, &Overlapped, &dwReadLen, FALSE);
            if (dwReadLen == 0)
            {
                Log(ThisModule, __LINE__, "读数据失败");
                m_cLog.NewLine().Log("RECV[0]: FAILED READERR").EndLine();
                return ERR_PORT_READERR;
            }

            dwInOutReadLen = dwReadLen;

            // 记录日志
            char szTmp[64] = {0};
            sprintf(szTmp, "RECV[%3u]:", dwReadLen);
            m_cLog.NewLine().Log(szTmp).LogHex(lpReadData, dwReadLen).EndLine();
            return ERR_PORT_SUCCESS;
        }
        else if (dwRet == WAIT_OBJECT_0 + 1)
        {
            Log(ThisModule, __LINE__, "取消读数据");
            m_cLog.NewLine().Log("RECV[0]: FAILED CANCELLED").EndLine();
            return ERR_PORT_CANCELED;
        }
        else if (dwRet == WAIT_TIMEOUT)
        {
            Log(ThisModule, __LINE__, "读数据超时");
            m_cLog.NewLine().Log("RECV[0]: FAILED TIMEOUT").EndLine();
            return ERR_PORT_RTIMEOUT;
        }
        else
        {
            Log(ThisModule, __LINE__, "读数据错误");
            m_cLog.NewLine().Log("RECV[0]: FAILED NODEFINED").EndLine();
            return ERR_PORT_NODEFINED;
        }
    }
}

long CWinComPort::SendAndRead(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cFuncMutex);
    try
    {
        long lRet = Send(lpSendData, dwSendLen, dwTimeOut);
        if (lRet <= 0)
            return lRet;
        return Read(lpReadData, dwInOutReadLen, dwTimeOut);
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "收发数据异常失败:GetLastError=%u", GetLastError());
        return ERR_PORT_NODEFINED;
    }
}

void CWinComPort::CancelRead()
{
    ::SetEvent(m_cCancleEvent.GetHandle());
    return;
}

void CWinComPort::ClearBuffer()
{
    AutoMutex(m_cFuncMutex);
    if (m_hCom == INVALID_HANDLE_VALUE)
        return;
    ::PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

bool CWinComPort::IsOpened()
{
    AutoMutex(m_cFuncMutex);
    return (m_hCom != INVALID_HANDLE_VALUE);
}

void CWinComPort::SetLogAction(LPCSTR lpActName)
{
    AutoMutex(m_cFuncMutex);
    m_cLog.LogAction(lpActName);
    return;
}

void CWinComPort::FlushLog()
{
    AutoMutex(m_cFuncMutex);
    m_cLog.FlushLog();
    return;
}
void CWinComPort::CancelLog()
{
    AutoMutex(m_cFuncMutex);
    m_cLog.CancelLog();
}
void CWinComPort::SetTimeOut(DWORD dwTimeOut, bool bReadTime)
{
    DWORD dwError = 0;
    COMSTAT Comstat;
    ::ClearCommError(m_hCom, &dwError, &Comstat);

    // 修正超时时间
    dwTimeOut = (dwTimeOut == 0) ? 3000 : dwTimeOut;

    //总超时＝时间系数× 要求读/写的字符数＋时间常量
    COMMTIMEOUTS ts;
    ::GetCommTimeouts(m_hCom, &ts);
    if (bReadTime)
    {
        ts.ReadTotalTimeoutMultiplier = 0;         //读时间系数
        ts.ReadTotalTimeoutConstant   = dwTimeOut; //读时间常量
    }
    else
    {
        ts.WriteTotalTimeoutMultiplier  = 0;        //写时间系数
        ts.WriteTotalTimeoutConstant    = dwTimeOut;//写时间常量
    }
    ::SetCommTimeouts(m_hCom, &ts);                 //设置超时
    return;
}
