#include "DevDEP_ZDGC.h"

static const char *ThisFile = "DevDEP_ZDGC.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" DEVDEP_EXPORT long CreateIDevDEP(LPCSTR lpDevType, IDevDEP *&pDev)
{
    pDev = new CDevDEP_ZDGC(lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CDevDEP_ZDGC::CDevDEP_ZDGC(LPCSTR lpDevType) : m_bReading(false),
                                               m_cSysMutex("SIU_ACR500")
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
}

CDevDEP_ZDGC::~CDevDEP_ZDGC()
{

}

void CDevDEP_ZDGC::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

long CDevDEP_ZDGC::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (m_pDev == nullptr)
    {
        if (0 != m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "DEP", "DevDEP_ZDGC"))
        {
            Log(ThisModule, __LINE__, "加载库(AllDevPort.dll) 失败");
            return ERR_DEVPORT_LIBRARY;
        }
    }
    LOGDEVACTION();
    CAutoMutex autoMutex(&m_cSysMutex);

    m_strOpenMode = lpMode;
    long lRet = m_pDev->Open(lpMode, false);
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "打开串口失败:%d[%s]", lRet, lpMode);
        return ERR_DEVPORT_NOTOPEN;
    }

    return ERR_DEVPORT_SUCCESS;
}

long CDevDEP_ZDGC::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if (m_pDev->IsOpened())
        m_pDev->Close();
    return ERR_DEVPORT_SUCCESS;
}

long CDevDEP_ZDGC::Reset()
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
        CAutoMutex autoMutex(&m_cSysMutex);
        m_pDev->Open(m_strOpenMode.c_str());
    }

//    char szReadData[512] = { 0 };
//    DWORD dwReadLen = sizeof(szReadData);
//    vector<char> vtData = { 0x7E,0x01,0x30,0x30,0x30,0x30,0x23,0x43,0x55,0x53,0x44,0x45,0x46,0x3B,0x03 };
//    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
//    if (lRet < 0)
//    {
//        Log(ThisModule, __LINE__, "发收复位命令失败：%d", lRet);
//        return lRet;
//    }

    return ERR_DEVPORT_SUCCESS;
}

long CDevDEP_ZDGC::GetDevInfo(char *pInfo)
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

    sprintf(pInfo, "%08sV%07s", "ZDGC", m_stStatus.szVerInfo);// 格式：Firmware(1)版本程序名称8位+版本8位
    return ERR_DEVPORT_SUCCESS;
}


long CDevDEP_ZDGC::GetStatus(DEVDEPSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    CAutoMutex autoMutex(&m_cSysMutex);
    CAutoCopyDevStatus<DEVDEPSTATUS> _auto(&stStatus, &m_stStatus);
    if (!m_pDev->IsOpened())
    {
        UpdateStatus(DEVICE_OFFLINE, "001");
        if (m_strOpenMode.empty())
        {
            Log(ThisFile, __LINE__, "未打开串口");
            return ERR_DEVPORT_NOTOPEN;
        }

        if (0 != m_pDev->Open(m_strOpenMode.c_str()))
        {
            Log(ThisFile, __LINE__, "尝试打开串口失败");
            return ERR_DEVPORT_NOTOPEN;
        }
    }

    char szReadData[256] = { 0 };
    DWORD dwReadLen = sizeof(szReadData);

    // 查询状态
    vector<char> vtData = { 0x02, 0x20, 0x20, 0x20, 0x03, 0x23 };
//    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    long lRet = SendAndReadCmd(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    if (lRet == ERR_DEVPORT_RTIMEOUT || lRet == ERR_DEVPORT_WTIMEOUT)
    {
        Log(ThisModule, __LINE__, "查询状态超时, 返回数据长度: %d", dwReadLen);
        return lRet;
    }
    if (lRet == ERR_DEVPORT_CANCELED)
    {
        Log(ThisModule, __LINE__, "查询状态取消：%d, 返回数据长度: %d", lRet, dwReadLen);
        return lRet;
    }
    if (lRet < 0 || dwReadLen <= 0)
    {
        Log(ThisModule, __LINE__, "发收查询状态命令失败：%d, 返回数据长度: %d, 命令接收码: %04X", lRet, dwReadLen, szReadData[1]);
        return lRet;
    }

    if (szReadData[2] == 0x01)
        m_stStatus.wShutterOpen = SHUTTER_DEP_SHTOPEN;
    else if (szReadData[3] == 0x01)
        m_stStatus.wShutterOpen = SHUTTER_DEP_SHTCLOSED;
    else
        m_stStatus.wShutterOpen = SHUTTER_DEP_SHTJAMMED;

    if (szReadData[11] == 0x01)
        m_stStatus.wDepContainer = CONTAINER_DEP_DEPHIGH;
    else if (szReadData[11] == 0x00)
        m_stStatus.wDepContainer = CONTAINER_DEP_DEPOK;
    else
        m_stStatus.wDepContainer = CONTAINER_DEP_DEPINOP;

    if (szReadData[15] != 0x00 && szReadData[16] != 0x00)
        memcpy(m_stStatus.szVerInfo, szReadData + 15, 2);

    // 记录收取数据日志
    //RecordRecvLog(szReadData, dwReadLen);

    UpdateStatus(DEVICE_ONLINE, "000");
    return ERR_DEVPORT_SUCCESS;
}

long CDevDEP_ZDGC::OpenShutter()
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

    CAutoMutex autoMutex(&m_cSysMutex);
    char szReadData[256] = { 0 };
    DWORD dwReadLen = sizeof(szReadData);

    // 开门
    vector<char> vtData = { 0x02, 0x30, 0x01, 0x31, 0x03, 0x03 };
//    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    long lRet = SendAndReadCmd(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    if (lRet == ERR_DEVPORT_RTIMEOUT || lRet == ERR_DEVPORT_WTIMEOUT)
    {
        Log(ThisModule, __LINE__, "开门超时, 返回数据长度: %d", dwReadLen);
        StopShutter();
        return lRet;
    }
    if (lRet == ERR_DEVPORT_CANCELED)
    {
        Log(ThisModule, __LINE__, "开门取消：%d, 返回数据长度: %d", lRet, dwReadLen);
        StopShutter();
        return lRet;
    }
    if (lRet < 0 || dwReadLen <= 0)
    {
        Log(ThisModule, __LINE__, "发收开门命令失败：%d, 返回数据长度: %d", lRet, dwReadLen);
        StopShutter();
        return lRet;
    }

    // 记录收取数据日志
    //RecordRecvLog(szReadData, dwReadLen);

    return ERR_DEVPORT_SUCCESS;
}


long CDevDEP_ZDGC::CloseShutter()
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

    CAutoMutex autoMutex(&m_cSysMutex);
    char szReadData[256] = { 0 };
    DWORD dwReadLen = sizeof(szReadData);

    // 关门
    vector<char> vtData = { 0x02, 0x30, 0x01, 0x32, 0x03, 0x00 };
//    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    long lRet = SendAndReadCmd(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    if (lRet == ERR_DEVPORT_RTIMEOUT || lRet == ERR_DEVPORT_WTIMEOUT)
    {
        Log(ThisModule, __LINE__, "关门超时, 返回数据长度: %d", dwReadLen);
        StopShutter();
        return lRet;
    }
    if (lRet == ERR_DEVPORT_CANCELED)
    {
        Log(ThisModule, __LINE__, "关门取消：%d, 返回数据长度: %d", lRet, dwReadLen);
        StopShutter();
        return lRet;
    }
    if (lRet < 0 || dwReadLen <= 0)
    {
        Log(ThisModule, __LINE__, "发收关门命令失败：%d, 返回数据长度: %d", lRet, dwReadLen);
        StopShutter();
        return lRet;
    }

    // 记录收取数据日志
    //RecordRecvLog(szReadData, dwReadLen);

    return ERR_DEVPORT_SUCCESS;
}

void CDevDEP_ZDGC::UpdateStatus(WORD wDevice, std::string strErrCode)
{
    m_stStatus.clear();
    m_stStatus.wDevice = wDevice;
    strcpy(m_stStatus.szErrCode, strErrCode.c_str());
}

void CDevDEP_ZDGC::RecordRecvLog(LPSTR pData, int nLen)
{
    // 记录收取数据日志
    char szBuf[1024] = {0};
    if (nLen > 1024)
    {
        Log(ThisFile, 0, "记录收取数据日志失败, 字符串长度太长, Len=%d > 1024", nLen);
        return;
    }

    for (DWORD i = 0; i < nLen; i++)
    {
        sprintf(szBuf + i, "%02.02X ", (BYTE)(pData[i]));
    }

    Log(ThisFile, 0, "RECV[%3u]:%s", nLen, szBuf);
}

long CDevDEP_ZDGC::StopShutter()
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

    CAutoMutex autoMutex(&m_cSysMutex);
    char szReadData[256] = { 0 };
    DWORD dwReadLen = sizeof(szReadData);

    // 停止闸门
    vector<char> vtData = { 0x02, 0x30, 0x01, 0x33, 0x03, 0x01 };
//    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    long lRet = SendAndReadCmd(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    if (lRet == ERR_DEVPORT_RTIMEOUT || lRet == ERR_DEVPORT_WTIMEOUT)
    {
        Log(ThisModule, __LINE__, "停止闸门超时, 返回数据长度: %d", dwReadLen);
        return lRet;
    }
    if (lRet == ERR_DEVPORT_CANCELED)
    {
        Log(ThisModule, __LINE__, "停止闸门取消：%d, 返回数据长度: %d", lRet, dwReadLen);
        return lRet;
    }
    if (lRet < 0 || dwReadLen <= 0)
    {
        Log(ThisModule, __LINE__, "发收停止闸门命令失败：%d, 返回数据长度: %d", lRet, dwReadLen);
        return lRet;
    }

    // 记录收取数据日志
    RecordRecvLog(szReadData, dwReadLen);

    return ERR_DEVPORT_SUCCESS;
}

long CDevDEP_ZDGC::SendAndReadCmd(LPCSTR lpSendData, int iSendSize,
                                  LPSTR lpRecvData, DWORD &dwInOutData, int iTimeOut)
{
    THISMODULE(__FUNCTION__);
    if(lpSendData == nullptr || iSendSize == 0 ||
       lpRecvData == nullptr || dwInOutData == 0){
        return false;
    }

    long lRet = 0;
    lRet = m_pDev->Send(lpSendData, iSendSize, iTimeOut);
    if(lRet != iSendSize){
        Log(ThisModule, __LINE__, "数据发送失败");
        return lRet;
    }

    int iRecvDataLen = 0;
    ULONG ulStartTime = CQtTime::GetSysTick();
    while(true){
        DWORD iBuffLeftLen = dwInOutData - iRecvDataLen;
        lRet = m_pDev->Read(lpRecvData + iRecvDataLen, iBuffLeftLen, 1000);
        if(lRet < 0){
            Log(ThisModule, __LINE__, "接收数据时发生错误:%d", lRet);
            break;
        }

        iRecvDataLen += iBuffLeftLen;
        if(iRecvDataLen >= ACR500_REPLY_LEN){
            break;
        }

        //判断超时
        if(CQtTime::GetSysTick() - ulStartTime >= iTimeOut){
            Log(ThisModule, __LINE__, "接收数据超时");
            lRet = ERR_DEVPORT_RTIMEOUT;
            break;
        }
    }

    dwInOutData = iRecvDataLen;
    return lRet;
}
