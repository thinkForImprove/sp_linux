#include "DevBCR_NLN1.h"
#include <QStringList>

static const char *ThisFile = "DevBCR_NLN1.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" DEVBCR_EXPORT long CreateIDevBCR(LPCSTR lpDevType, IDevBCR *&pDev)
{
    pDev = new CDevBCR_NLN1(lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CDevBCR_NLN1::CDevBCR_NLN1(LPCSTR lpDevType) : m_bReading(false)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
}

CDevBCR_NLN1::~CDevBCR_NLN1()
{

}

void CDevBCR_NLN1::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

long CDevBCR_NLN1::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (m_pDev == nullptr)
    {
        if (0 != m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "BCR", "DevBCR_NLN1"))
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

    //设置读码模式为电平模式
    char szReadData[512] = { 0 };
    DWORD dwReadLen = sizeof(szReadData);
    vector<char> vtData = { 0x7E,0x01,0x30,0x30,0x30,0x30,0x40,0x53,0x43,0x4E,0x4D,0x4F,0x44,0x30,0x3B,0x03 };
    lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "设置读码模式为电平模式失败：%d", lRet);
        return lRet;
    }
    
    //设置单次扫码超时时间
//    vtData = {0x7E,0x01,0x30,0x30,0x30,0x30,0x40,0x4F,0x52,0x54,0x53,0x45,0x54,0x39,0x39,0x39,0x39,0x3B,0x03};
//    lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
//    if (lRet < 0)
//    {
//        Log(ThisModule, __LINE__, "设置读码超时时间失败：%d", lRet);
//        return lRet;
//    }

    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_NLN1::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if (m_pDev->IsOpened())
        m_pDev->Close();
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_NLN1::Reset()
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
    vector<char> vtData = { 0x7E,0x01,0x30,0x30,0x30,0x30,0x23,0x43,0x55,0x53,0x44,0x45,0x46,0x3B,0x03 };
    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "发收复位命令失败：%d", lRet);
        return lRet;
    }
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_NLN1::GetDevInfo(char *pInfo)
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

    char szFVer[64] = { "1.00.007" };
    sprintf(pInfo, "%08sV%07s", "NLS-N1", szFVer);// 格式：Firmware(1)版本程序名称8位+版本8位
    return ERR_DEVPORT_SUCCESS;
}


long CDevBCR_NLN1::GetStatus(DEVBCRSTATUS &stStatus)
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

    if(m_strOpenMode.size() > 0){
        QString strParams = m_strOpenMode.c_str();
        QStringList paramList = strParams.split(":");
        if(paramList.size() > 1){
            if(!QFile::exists(paramList.at(1).toUtf8().data())){
                return ERR_DEVPORT_NOTOPEN;
            }
        }
    }

    char szReadData[4096] = { 0 };;
    DWORD dwReadLen = sizeof(szReadData);
    vector<char> vtData = { '?' };
 /*   for (int i = 0;;)
    {
        long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT);
        if (szReadData[0] == 0x21)
            break;

        Log(ThisModule, __LINE__, "发收状态命令失败：%d[i=%d]", lRet, i);
        if (++i >= 0)// 测试，修改为一次
        {
            UpdateStatus(DEVICE_HWERROR, "002");
            return lRet;
        }
    }*/

    UpdateStatus(DEVICE_ONLINE, "000");
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_NLN1::ReadBCR(DWORD &dwType, LPSTR lpData, DWORD &dwLen, long lTimeOut)
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

    //#SCNTRG1  读二维码
    ULONG dwCmdStart = CQtTime::GetSysTick();
    while(true){
        memset(szReadData, NULL, sizeof(szReadData));
        dwReadLen = sizeof(szReadData);
        vector<char> vtData = { 0x7E,0x01,0x30,0x30,0x30,0x30,0x23,0x53,0x43,0x4E,0x54,0x52,0x47,0x31,0x3B,0x03 };
        long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, DEF_TIMEOUT*4);

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

        memset(szReadData, NULL, sizeof(szReadData));
        dwReadLen = sizeof(szReadData);
        ULONG dwOnceReadStart = CQtTime::GetSysTick();
        long lOnceReadTimeout = 5000;
        while(true)
        {
            if (CQtTime::GetSysTick() - dwOnceReadStart > lOnceReadTimeout){
                lRet = ERR_DEVPORT_RTIMEOUT;
                break;
            }

            lRet = m_pDev->Read(szReadData, dwReadLen, (DWORD)lOnceReadTimeout);

            if(lRet == ERR_DEVPORT_CANCELED){
                return lRet;
            }
            if(lRet == 0 && dwReadLen > 0){
                break;
            }
            CQtTime::Sleep(10);
        }

        if(lRet == 0){
            break;
        } else {
            CancelReadBCRCmd();
        }

        if(CQtTime::GetSysTick() - dwCmdStart> lTimeOut){
            return ERR_DEVPORT_RTIMEOUT;
        }
    }
//    vector<char> vtData = { 0x7E,0x01,0x30,0x30,0x30,0x30,0x23,0x53,0x43,0x4E,0x54,0x52,0x47,0x31,0x3B,0x03 };
////    long lRet = m_pDev->SendAndRead(vtData.data(), vtData.size(), szReadData, dwReadLen, (DWORD)lTimeOut);
//    long lRet = m_pDev->Send(vtData.data(), vtData.size(), (DWORD)lTimeOut);

//    lRet = m_pDev->Read(szReadData, dwReadLen, (DWORD)lTimeOut);
//    memset(szReadData, NULL, sizeof(szReadData));
//    dwReadLen = sizeof(szReadData);

//    if (lRet == ERR_DEVPORT_RTIMEOUT || lRet == ERR_DEVPORT_WTIMEOUT)
//    {
//        Log(ThisModule, __LINE__, "扫码超时");
//        CancelReadBCRCmd();
//        return lRet;
//    }
//    if (lRet == ERR_DEVPORT_CANCELED)
//    {
//        Log(ThisModule, __LINE__, "扫码返回取消：%d", lRet);
//        CancelReadBCRCmd();
//        return lRet;
//    }
//    if (lRet < 0)
//    {
//        Log(ThisModule, __LINE__, "发收扫码命令失败：%d", lRet);
//        CancelReadBCRCmd();
//        return lRet;
//    }

//    ULONG dwStart = CQtTime::GetSysTick();
//    while(true)
//    {
//        if (CQtTime::GetSysTick() - dwStart > lTimeOut)
//            break;

//        lRet = m_pDev->Read(szReadData, dwReadLen, (DWORD)lTimeOut);

//        if(lRet == 0 && dwReadLen == 0)
//            continue;

//        if(lRet == -4 || lRet == -7){
//            return lRet;
//        }
//        if(lRet == 0 && dwReadLen != 0){
//            break;

//        }
//        CQtTime::Sleep(10);

//    }

    // 解析码值
    dwType = 0;// 不支持类型检测
    if (dwLen > dwReadLen)
        dwLen = dwReadLen;
    memcpy(lpData, szReadData, dwLen);
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_NLN1::CancelRead()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);// 取消接口不用互斥

    if (m_bReading)// 防止多取消，造成取状态失败
    {
        m_bReading = false;
        m_pDev->CancelRead();
    }
    CancelReadBCRCmd();
    return ERR_DEVPORT_SUCCESS;
}

long CDevBCR_NLN1::CancelReadBCRCmd()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    vector<char> vtCancelData = { 0x7E,0x01,0x30,0x30,0x30,0x30,0x40,0x53,0x43,0x4E,0x54,0x52,0x47,0x30,0x3B,0x03 };
    long lRet = m_pDev->Send(vtCancelData.data(), vtCancelData.size(), DEF_TIMEOUT);
    if (lRet != 0)          //30-00-00-00(FT#0068)
    {
        Log(ThisModule, __LINE__, "取消扫码命令失败：%d", lRet);
        return lRet;
    }
    return ERR_DEVPORT_SUCCESS;
}

void CDevBCR_NLN1::UpdateStatus(WORD wDevice, std::string strErrCode)
{
    m_stStatus.clear();
    m_stStatus.wDevice = wDevice;
    strcpy(m_stStatus.szErrCode, strErrCode.c_str());
}

