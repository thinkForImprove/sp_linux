#include "LinuxComPort.h"
#include <QFileInfo>
#include <QDir>

static const char *ThisFile = "LinuxComPort.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIDevPort(LPCSTR lpDevName, LPCSTR lpDevType, IDevPort *&pDev)
{
    pDev = new CLinuxComPort(lpDevName, lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CLinuxComPort::CLinuxComPort(LPCSTR lpDevName, LPCSTR lpDevType): m_cLog(lpDevName)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    m_nPort = 0;
    m_SemCancel.ResetSem();
}

CLinuxComPort::~CLinuxComPort()
{
    Close();
}

void CLinuxComPort::Release()
{

}

long CLinuxComPort::Open(LPCSTR lpMode, bool bExclusiveUse/* = true*/)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);

    string strPort;
    struct termios stParam;
    bzero(&stParam, sizeof(stParam));
    stParam.c_cflag |= CLOCAL | CREAD;  // 修改控制模式，使得能够从串口中读取输入数据
    stParam.c_cflag &= ~CSIZE;          // 屏蔽其他标志位

    if (!GetComParam(lpMode, strPort, stParam))
    {
        Log(ThisModule, __LINE__, "GetComParam Error: lpMode=%s", lpMode);
        m_cLog.NewLine().Log("OPEN FAILED PARAM:").Log(lpMode).EndLine();
        return ERR_PORT_PARAM;
    }

    if (IsOpened())
        Close();

    if (!IsPortFileValid(strPort))
    {
        Log(ThisModule, __LINE__, "strPort(%s) is Invalid", strPort.c_str());
        return ERR_PORT_PARAM;
    }

    m_SemCancel.ResetSem();
    m_cLog.NewLine().Log(lpMode);

    bool bOpen = false;
    do
    {
        m_nPort = ::open(strPort.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (-1 == m_nPort)
        {
            Log(ThisModule, -1, "打开串口失败:%s", strPort.c_str());
            break;
        }

        //判断串口文件是否被占用，实现串口独占模式
        if(bExclusiveUse){
            if (0 != ::flock(m_nPort, LOCK_EX | LOCK_NB))
            {
                Log(ThisModule, -1, "尝试独占端口失败:%s", strPort.c_str());
                break;
            }
        }

        // 清除所有ＩＯ的数据
        ::tcflush(m_nPort, TCIOFLUSH);

        //stParam.c_cflag |= CLOCAL | CREAD;  // 修改控制模式，使得能够从串口中读取输入数据
        //stParam.c_cflag &= ~CSIZE;          // 屏蔽其他标志位

        // 设置等待时间和最小接收字符
        stParam.c_cc[VTIME] = 0; // 读取一个字符等待1*(1/10)s
        stParam.c_cc[VMIN] = 0;  // 读取字符的最少个数为1

        //　激活配置 (将修改后的termios数据设置到串口中）
        if (0 != ::tcsetattr(m_nPort, TCSANOW, &stParam))
        {
            Log(ThisModule, -1, "激活端口配置失败:%s", strPort.c_str());
            break;
        }

        bOpen = true;
    } while (false);

    if (!bOpen)
    {
        m_cLog.NewLine().Log("OPEN FAILED").EndLine();
        return ERR_PORT_NOTOPEN;
    }

    m_cLog.NewLine().Log("OPEN SUCCEED").EndLine();
    return ERR_PORT_SUCCESS;
}

long CLinuxComPort::Close()
{
    AutoMutex(m_cMutex);

    if (m_nPort > 0)
        ::close(m_nPort);
    m_nPort = 0;
    return ERR_PORT_SUCCESS;
}

long CLinuxComPort::Send(LPCSTR lpData, DWORD dwDataLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);
    if (lpData == nullptr || dwDataLen == 0)
    {
        Log(ThisModule, __LINE__, "参数错误");
        m_cLog.NewLine().Log("SEND[0] FAILED PARAM").EndLine();
        return ERR_PORT_PARAM;
    }
    if (!IsOpened())
    {
        Log(ThisModule, __LINE__, "串口没打开");
        m_cLog.NewLine().Log("SEND[0] FAILED NOTOPEN").EndLine();
        return ERR_PORT_PARAM;
    }

    tcflush(m_nPort, TCIOFLUSH);    //先清除发送接收缓冲区的数据

    // 记录日志
    char szTmp[64] = {0};
    sprintf(szTmp, "SEND[%3u]:", dwDataLen);
    m_cLog.NewLine().Log(szTmp).LogHex(lpData, dwDataLen);

    ULONG dwStartTime = CQtTime::GetSysTick();
    DWORD dwWriteByte = 0;
    DWORD dwWriteTotal = 0;
    DWORD dwWriteLen = 0;

    do
    {
        if ((dwDataLen - dwWriteTotal) > MAX_WRITE_LEN)
        {
            dwWriteLen = MAX_WRITE_LEN;
        }
        else
        {
            dwWriteLen = dwDataLen - dwWriteTotal;
        }

        dwWriteByte = (DWORD)::write(m_nPort, lpData + dwWriteTotal, dwWriteLen);
        if ((-1) == ::tcdrain(m_nPort))
        {
            Log(ThisModule, __LINE__, "发数据错误");
            m_cLog.Log("FAILED WRITE").EndLine();
            return ERR_PORT_WRITE;
        }
        if (CQtTime::GetSysTick() - dwStartTime > dwTimeOut)
        {
            Log(ThisModule, __LINE__, "发数据超时");
            m_cLog.Log("FAILED TIMEOUT").EndLine();
            return ERR_PORT_WTIMEOUT;
        }

        if (dwWriteByte == 0) // 写失败
        {
            Log(ThisModule, __LINE__, "发数据错误");
            m_cLog.Log("FAILED WRITE").EndLine();
            return ERR_PORT_WRITE;
        }

        dwWriteTotal += dwWriteByte;

    } while (dwWriteTotal < dwDataLen);

    if (dwWriteTotal < dwDataLen)
    {
        Log(ThisModule, __LINE__, "发数据错误dwLenWritten=%u, dwLen=%u", dwWriteTotal, dwDataLen);
        char szTmp[64] = {0};
        sprintf(szTmp, "FAILED LEN[S%3u != W%3u]", dwDataLen, dwWriteTotal);
        m_cLog.Log(szTmp).EndLine();
        return ERR_PORT_WRITE;
    }

    m_cLog.EndLine();
    //return dwWriteTotal;
    return ERR_PORT_SUCCESS;                    //30-00-00-00(FT#0068)
}

long CLinuxComPort::Read(LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);
    if (lpReadData == nullptr || dwInOutReadLen == 0)
    {
        Log(ThisModule, __LINE__, "参数错误");
        m_cLog.NewLine().Log("RECV[0] FAILED PARAM").EndLine();
        return ERR_PORT_PARAM;
    }
    if (!IsOpened())
    {
        Log(ThisModule, __LINE__, "串口没打开");
        m_cLog.NewLine().Log("RECV[0] FAILED NOTOPEN").EndLine();
        return ERR_PORT_NOTOPEN;
    }

    if (dwInOutReadLen > MAX_BUFFER_SIZE)
        dwInOutReadLen = MAX_BUFFER_SIZE;

    m_SemCancel.ResetSem();
    DWORD   dwBytesRead = 0;
    ULONG dwStartTime = CQtTime::GetSysTick();
    do
    {
        dwBytesRead = (DWORD)::read(m_nPort, lpReadData, dwInOutReadLen);
        if (dwBytesRead > 0 && dwBytesRead != (DWORD) -1)
        {
            dwInOutReadLen = dwBytesRead;
            break;
        }

        if (CQtTime::GetSysTick() - dwStartTime > dwTimeOut)
        {
            MLog(ThisModule, __LINE__, "读数据超时");
            m_cLog.NewLine().Log("RECV[0]: FAILED TIMEOUT");//.EndLine();// 不调用EndLine，让可取消
            return ERR_PORT_RTIMEOUT;
        }

        if (m_SemCancel.WaitSem(1))
        {
            Log(ThisModule, __LINE__, "取消读数据");
            m_cLog.NewLine().Log("RECV[0]: FAILED CANCELLED").EndLine();
            return ERR_PORT_CANCELED;
        }

        CQtTime::Sleep(10);
    } while (true);

    // 记录日志
    char szTmp[64] = {0};
    sprintf(szTmp, "RECV[%3u]:", dwBytesRead);
    m_cLog.NewLine().Log(szTmp).LogHex(lpReadData, dwBytesRead).EndLine();
    return ERR_PORT_SUCCESS;

}

long CLinuxComPort::SendAndRead(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
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

void CLinuxComPort::CancelRead()
{
    m_SemCancel.SetSem();
}

void CLinuxComPort::ClearBuffer()
{
    AutoMutex(m_cMutex);
    tcflush(m_nPort, TCIOFLUSH);
}

bool CLinuxComPort::IsOpened()
{
    AutoMutex(m_cMutex);
    return (m_nPort != 0);
}

void CLinuxComPort::SetLogAction(LPCSTR lpActName)
{
    AutoMutex(m_cMutex);
    m_cLog.LogAction(lpActName);
}

void CLinuxComPort::FlushLog()
{
    AutoMutex(m_cMutex);
    m_cLog.FlushLog();
}

void CLinuxComPort::CancelLog()
{
    AutoMutex(m_cMutex);
    m_cLog.CancelLog();
}

bool CLinuxComPort::IsPortFileValid(string &strPort)
{
    QFileInfo fileinfo(strPort.c_str());
    if (fileinfo.exists())
    {
        return true;
    }

    string strDir = strPort.substr(0, strPort.rfind('/'));
    string strName = strPort.substr(strPort.rfind('/') + 1);
    if (strName.length() < 3)
    {
        return false;
    }

    QDir qDir(QString::fromStdString(strDir));
    foreach (QFileInfo mfi, qDir.entryInfoList())
    {
        QString strFileName = mfi.fileName();
        if (mfi.isDir() || strFileName == "." || strFileName == "..")
        {
            continue;
        }

        int iIndex = strFileName.indexOf(QString::fromStdString(strName));
        if (iIndex >= 0)
        {
            strPort = strDir + "/" + strFileName.toStdString();
            return true;
        }
    }

    return false;
}

bool CLinuxComPort::GetComParam(LPCSTR lpMode, string &strPort, struct termios &stParam)
{
    // 格式lpMode: 串口=COM:/dev/ttyUSB0:9600,N,8,1
    QString strMode = QString::fromLocal8Bit(lpMode);
    QStringList strList = strMode.split(QRegExp("[:,]"), QString::SkipEmptyParts);
    if (strList.size() != 6)
        return false;
    if (strList[0] != "COM")
        return false;

    strPort = strList[1].toStdString();
    // 设置波特率
    switch (strList[2].toUInt())
    {
    case 2400:
        cfsetispeed(&stParam, B2400);
        cfsetospeed(&stParam, B2400);
        break;
    case 4800:
        cfsetispeed(&stParam, B4800);
        cfsetospeed(&stParam, B4800);
        break;
    case 9600:
        cfsetispeed(&stParam, B9600);
        cfsetospeed(&stParam, B9600);
        break;
    case 19200:
        cfsetispeed(&stParam, B19200);
        cfsetospeed(&stParam, B19200);
        break;
    case 57600:                                     //test#13
        cfsetispeed(&stParam, B57600);              //test#13
        cfsetospeed(&stParam, B57600);              //test#13
        break;                                      //test#13
    case 38400:
        cfsetispeed(&stParam, B38400);
        cfsetospeed(&stParam, B38400);
        break;
    case 115200:
        cfsetispeed(&stParam, B115200);
        cfsetospeed(&stParam, B115200);
        break;
    default:
        return false;
    }

    //设置校验位
    switch (strList[3].toLocal8Bit().at(0))
    {
    case 'n':
    case 'N': // 无奇偶校验位。
        stParam.c_cflag &= ~PARENB;
        stParam.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O':// 设置为奇校验
        stParam.c_cflag |= (PARODD | PARENB);
        stParam.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E':// 设置为偶校验
        stParam.c_cflag |= PARENB;
        stParam.c_cflag &= ~PARODD;
        stParam.c_iflag |= INPCK;
        break;
    case 's':
    case 'S': // 设置为空格
        stParam.c_cflag &= ~PARENB;
        stParam.c_cflag &= ~CSTOPB;
        break;
    default:
        return false;
    }

    // 设置数据位
    switch (strList[4].toUInt())
    {
    case 5:
        stParam.c_cflag |= CS5;
        break;
    case 6:
        stParam.c_cflag |= CS6;
        break;
    case 7:
        stParam.c_cflag |= CS7;
        break;
    case 8:
        stParam.c_cflag |= CS8;
        break;
    default:
        return false;
    }

    // 设置停止位
    switch (strList[5].toUInt())
    {
    case 1:
        stParam.c_cflag &= ~CSTOPB;
        break;
    case 2:
        stParam.c_cflag |= CSTOPB;
        break;
    default:
        return false;
    }
    return true;
}
