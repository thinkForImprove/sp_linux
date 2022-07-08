#include "DevBCR_N3680.h"

static const char *ThisFile = "DevBCR_N3680.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" DEVBCR_EXPORT long CreateIDevBCR(LPCSTR lpDevType, IDevBCR *&pDev)
{
    pDev = new CDevBCR_N3680(lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CDevBCR_N3680::CDevBCR_N3680(LPCSTR lpDevType) : m_bReading(false)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
}

CDevBCR_N3680::~CDevBCR_N3680()
{

}

void CDevBCR_N3680::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

long CDevBCR_N3680::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (m_pDev == nullptr)
    {
        if (0 != m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "BCR", "DevBCR_N3680"))
        {
            Log(ThisModule, __LINE__, "加载库(AllDevPort.dll) 失败");
            return ERR_DEVPORT_LIBRARY;
        }
    }
    LOGDEVACTION();

    m_strOpenMode = lpMode;
    long lRet = m_pDev->Open(lpMode);
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "打开串口失败:%d[%s]", lRet, lpMode);
        return ERR_DEVPORT_NOTOPEN;
    }
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_N3680::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if (m_pDev->IsOpened())
        m_pDev->Close();
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_N3680::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if (!m_pDev->IsOpened())
    {
        Log(ThisModule, __LINE__, "未打开串口");
        return ERR_DEVPORT_NOTOPEN;
    }
    // 如果故障，重新打开一次串口，因是USB转串口
    if (m_stStatus.wDevice == DEVICE_HWERROR)
    {
        m_pDev->Close();
        m_pDev->Open(m_strOpenMode.c_str());
    }

    char szReadData[512] = { 0 };
    DWORD dwReadLen = sizeof(szReadData);
    vector<char> vtData = { 0x16, 0x4D, 0x0D, 0x54, 0x45, 0x52, 0x4D, 0x49, 0x44, 0x31, 0x33, 0x30, 0x2E };
    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "发收复位命令失败：%d", lRet);
        return lRet;
    }
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_N3680::GetDevInfo(char *pInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if (!m_pDev->IsOpened())
    {
        Log(ThisModule, __LINE__, "未打开串口");
        return ERR_DEVPORT_NOTOPEN;
    }

    char szFVer[64] = { "1.0.0.1" };
    sprintf(pInfo, "%08sV%07s", "N3680", szFVer);// 格式：Firmware(1)版本程序名称8位+版本8位
    return ERR_DEVPORT_SUCCESS;
}


long CDevBCR_N3680::GetStatus(DEVBCRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    CAutoCopyDevStatus<DEVBCRSTATUS> _auto(&stStatus, &m_stStatus);
    if (!m_pDev->IsOpened())
    {
        UpdateStatus(DEVICE_OFFLINE, "001");
        if (m_strOpenMode.empty())
        {
            Log(ThisModule, __LINE__, "未打开串口");
            return ERR_DEVPORT_NOTOPEN;
        }

        if (0 != m_pDev->Open(m_strOpenMode.c_str()))
        {
            Log(ThisModule, __LINE__, "尝试打开串口失败");
            return ERR_DEVPORT_NOTOPEN;
        }
    }

    char szReadData[4096] = { 0 };;
    DWORD dwReadLen = sizeof(szReadData);
    vector<char> vtData = { 0x16, 0x4D, 0x0D, 0x63, 0x62, 0x72, 0x65, 0x6E, 0x61, 0x2A, 0x2E };
    for (int i = 0;;)
    {
        long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
        if (lRet >= 0)
            break;

        Log(ThisModule, __LINE__, "发收状态命令失败：%d[i=%d]", lRet, i);
        if (++i >= 0)// 测试，修改为一次
        {
            UpdateStatus(DEVICE_HWERROR, "002");
            return lRet;
        }
    }

    UpdateStatus(DEVICE_ONLINE, "000");
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_N3680::ReadBCR(DWORD &dwType, LPSTR lpData, DWORD &dwLen, long lTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    CAutoSetReadingStatus _auto(&m_bReading);
    if (!m_pDev->IsOpened())
    {
        Log(ThisModule, __LINE__, "未打开串口");
        return ERR_DEVPORT_NOTOPEN;
    }

    char szReadData[4096] = { 0 };
    DWORD dwReadLen = sizeof(szReadData);
    vector<char> vtData = { 0x16, 0x54, 0x0D };

    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, (DWORD)lTimeOut);
    if (lRet == ERR_DEVPORT_RTIMEOUT || lRet == ERR_DEVPORT_WTIMEOUT)
    {
        Log(ThisModule, __LINE__, "扫码超时");
        CancelReadBCRCmd();
        return lRet;
    }
    if (lRet == ERR_DEVPORT_CANCELED)
    {
        Log(ThisModule, __LINE__, "扫码返回取消：%d", lRet);
        CancelReadBCRCmd();
        return lRet;
    }
    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "发收扫码命令失败：%d", lRet);
        CancelReadBCRCmd();
        return lRet;
    }

    // 解析码值
    dwType = 0;// 不支持类型检测
    if (dwLen > dwReadLen)
        dwLen = dwReadLen;
    memcpy(lpData, szReadData, dwLen);
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_N3680::CancelRead()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);// 取消接口不用互斥

    if (m_bReading)// 防止多取消，造成取状态失败
    {
        m_bReading = false;
        m_pDev->CancelRead();
    }
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_N3680::CancelReadBCRCmd()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    vector<char> vtCancelData = { 0x16, 0x55, 0x0D };
    long lRet = m_pDev->Send(vtCancelData.data(), vtCancelData.size(), DEF_TIMEOUT);
    if (lRet != 0)                  //30-00-00-00(FT#0068)
    {
        Log(ThisModule, __LINE__, "取消扫码命令失败：%d", lRet);
        return lRet;
    }
    return ERR_DEVPORT_SUCCESS;
}

void CDevBCR_N3680::UpdateStatus(WORD wDevice, std::string strErrCode)
{
    m_stStatus.clear();
    m_stStatus.wDevice = wDevice;
    strcpy(m_stStatus.szErrCode, strErrCode.c_str());
}

