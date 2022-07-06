#include "AllDevPort.h"

static const char *ThisFile = "AllDevPort.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIAllDevPort(LPCSTR lpDevName, LPCSTR lpDevType, IAllDevPort *&pPort)
{
    pPort = new CAllDevPort(lpDevName, lpDevType);
    return (pPort != nullptr) ? 0 : -1;
}

//////////////////////////////////////////////////////////////////////////
CAllDevPort::CAllDevPort(LPCSTR lpDevName, LPCSTR lpDevType):
    m_strDevName(lpDevName), m_strDevType(lpDevType)
{
    SetLogFile(LOGFILE, ThisFile);
}

CAllDevPort::~CAllDevPort()
{

}

void CAllDevPort::Release()
{
    if (m_pPort != nullptr)
        m_pPort->Release();
}

long CAllDevPort::Open(LPCSTR lpMode)
{
    DEVPORTTYPE eType;
    QString strMode = QString::fromLocal8Bit(lpMode);
    QStringList strList = strMode.split(":");
    if (strList.size() != 2 && strList.size() != 3)
        return ERR_DEVPORT_PARAM;

    if (qstrnicmp("COM", strList[0].toLocal8Bit(), 3) == 0)
        eType = DPTYPE_COM;
    else if (qstrnicmp("HID", strList[0].toLocal8Bit(), 3) == 0)
        eType = DPTYPE_HID;
    else if (qstrnicmp("USB", strList[0].toLocal8Bit(), 3) == 0)
        eType = DPTYPE_USB;
    else
        return ERR_DEVPORT_PARAM;

    if (eType != DPTYPE_COM)
    {
        strMode = strList[1];
        QStringList vtVidPid = strMode.split(",");
        if (vtVidPid.size() != 2)
            return ERR_DEVPORT_PARAM;

        QString strVID = vtVidPid[0];
        QString strPID = vtVidPid[1];
        ULONG lVID = HexToLen(strVID.toStdString().c_str());
        ULONG lPID = HexToLen(strPID.toStdString().c_str());
        strMode.sprintf("%lu,%lu", lVID, lPID);
    }

    if (!LoadDll(eType))
        return ERR_DEVPORT_LIBRARY;
    return m_pPort->Open(strMode.toLocal8Bit());
}

long CAllDevPort::Close()
{
    if (m_pPort == nullptr)
        return ERR_DEVPORT_LIBRARY;
    return m_pPort->Close();
}

long CAllDevPort::Send(LPCSTR lpData, DWORD dwDataLen, DWORD dwTimeOut)
{
    if (m_pPort == nullptr)
        return ERR_DEVPORT_LIBRARY;
    return m_pPort->Send(lpData, dwDataLen, dwTimeOut);
}

long CAllDevPort::Read(LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    if (m_pPort == nullptr)
        return ERR_DEVPORT_LIBRARY;
    return m_pPort->Read(lpReadData, dwInOutReadLen, dwTimeOut);
}

long CAllDevPort::SendAndRead(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwInOutReadLen, DWORD dwTimeOut)
{
    if (m_pPort == nullptr)
        return ERR_DEVPORT_LIBRARY;
    return m_pPort->SendAndRead(lpSendData, dwSendLen, lpReadData, dwInOutReadLen, dwTimeOut);
}

void CAllDevPort::CancelRead()
{
    if (m_pPort != nullptr)
        m_pPort->CancelRead();
}

void CAllDevPort::ClearBuffer()
{
    if (m_pPort != nullptr)
        m_pPort->ClearBuffer();
}

bool CAllDevPort::IsOpened()
{
    if (m_pPort != nullptr)
        return m_pPort->IsOpened();
    return false;
}

void CAllDevPort::SetLogAction(LPCSTR lpActName)
{
    m_strActName = lpActName;
    if (m_pPort != nullptr)
        m_pPort->SetLogAction(lpActName);
}

void CAllDevPort::FlushLog()
{
    if (m_pPort != nullptr)
        m_pPort->FlushLog();
}

void CAllDevPort::CancelLog()
{
    if (m_pPort != nullptr)
        m_pPort->CancelLog();
}

bool CAllDevPort::LoadDll(DEVPORTTYPE eType)
{
    QString strDllName;
    switch (eType)
    {
    case DPTYPE_COM:
        {
#ifdef QT_WIN32
            strDllName = "WinComPort.dll";
#else
            strDllName = "LinuxComPort.so";
#endif
        }
        break;
    case DPTYPE_HID:
        {
#ifdef QT_WIN32
            strDllName = "WinHIDPort.dll";
#else
            strDllName = "LinuxHIDPort.so";
#endif
        }
        break;
    case DPTYPE_USB:
        {
#ifdef QT_WIN32
            strDllName = "WinUSBPort.dll";
#else
            strDllName = "LinuxUSBPort.so";
#endif
        }
        break;
    default:
        break;
    }
    if (strDllName.isEmpty())
        return false;
    if(m_pPort != nullptr){
        return true;
    }
    long lRet = m_pPort.Load(strDllName.toStdString().c_str(), "CreateIDevPort", m_strDevType.c_str(), m_strDevName.c_str());
    if (m_pPort != nullptr && !m_strActName.empty())
        m_pPort->SetLogAction(m_strActName.c_str());
    return (0 == lRet);
}

ULONG CAllDevPort::HexToLen(const char *pHex)
{
    UINT uHex = strlen(pHex);
    UINT uVal = 0;
    ULONG ulLen = 0;
    for (UINT i = 0; i < uHex; i++)
    {
        if ((pHex[i] >= 'A') && (pHex[i] <= 'F'))
        {
            uVal = pHex[i] - 'A' + 10;
        } else if(pHex[i] >= 'a' && pHex[i] <= 'f') {
            uVal = pHex[i] - 'a' + 10;
        } else {
            uVal = pHex[i] - '0';
        }

        uVal  = uVal / 16 * 10 + uVal % 16;
        ulLen = ulLen * 16 + uVal;
    }

    return ulLen;
}
