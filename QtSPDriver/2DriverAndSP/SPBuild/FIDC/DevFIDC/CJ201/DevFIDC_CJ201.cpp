#include "DevFIDC_CJ201.h"
#include "AutoQtHelpClass.h"

static const char *ThisFile = "DevFIDC_CJ201.cpp";
//////////////////////////////////////////////////////////////////////////
//extern "C" Q_DECL_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev)
//{
//    pDev = new DevFIDC_CJ201(lpDevType);
//    return (pDev != nullptr) ? 0 : -1;
//}

#define MAX_SEND_LEN (1024)
#define MAX_RES_LEN (1024)
#define TIME_SEND   (3000)
#define TIME_READ   (3000)
//////////////////////////////////////////////////////////////////////////
DevFIDC_CJ201::DevFIDC_CJ201(LPCSTR lpDevType): m_SemCancel(false)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    m_nCardStatus = IDCSTATUS_UNKNOWN;
}

DevFIDC_CJ201::~DevFIDC_CJ201()
{

}

void DevFIDC_CJ201::Release()
{

}


int DevFIDC_CJ201::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    if (m_pDev == nullptr)
    {
        if (0 != m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "FIDC", "DevFIDC_CJ201"))
        {
            Log(ThisModule, __LINE__, "Load(AllDevPort.dll) failed");
            return ERR_IDC_COMM_ERR;
        }
    }
    LOGDEVACTION();

    if (pMode == nullptr)
    {
        if (m_strMode.empty())
        {
            char szDevID[MAX_EXT] = {0};
            sprintf(szDevID, "HID:%d,%d", 0x2D9A, 0x0101);
            m_strMode = szDevID;
        }
    }
    else
    {
        m_strMode = pMode;
    }

    long iRet = m_pDev->Open(m_strMode.c_str());
    if (iRet < 0)
    {
        Log(ThisModule, __LINE__, "Open failed ");
        return ConvertErrorCode(iRet);
    }

    Log(ThisModule, 1, "Open success ");
    m_nCardStatus = IDCSTAUTS_NOCARD;
    return 0;

}

void DevFIDC_CJ201::Close()
{
    THISMODULE(__FUNCTION__);
    if (m_pDev == nullptr)
        return;

    LOGDEVACTION();
    m_pDev->Close();
}

int DevFIDC_CJ201::Init(CardAction eActFlag, WobbleAction nNeedWobble)
{
    int iRet = CloseField();
    m_nCardStatus = IDCSTAUTS_NOCARD;
    IDC_IDCSTAUTS IDCstatus;
    return DetectCard(IDCstatus);
}

int DevFIDC_CJ201::EatCard()
{
    return 0;
}

int DevFIDC_CJ201::EjectCard()
{
    m_nCardStatus = IDCSTAUTS_ENTERING;
    return MoveCard();
}

int DevFIDC_CJ201::AcceptCard(ULONG ulTimeOut, bool Magnetic)
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();
    ULONG ulTimeStart = CQtTime::GetSysTick();
    char pATRInfo[128] = {0};
    unsigned int uATRLen;
    m_nCardStatus = IDCSTAUTS_NOCARD;
    while (true)
    {
        if (m_SemCancel.WaitForEvent(5))
        {
            return ERR_IDC_USER_CANCEL;
        }

        int iRet = ActiveCard(pATRInfo, uATRLen);
        if (iRet == ERR_IDC_SUCCESS)
        {
            m_nCardStatus = IDCSTAUTS_INTERNAL;
            return ERR_IDC_SUCCESS;
        }
        else if (iRet != ERR_IDC_ACTIVEFAILED)
        {
            return iRet;
        }
        if ((CQtTime::GetSysTick() - ulTimeStart) > ulTimeOut)
        {
            return ERR_IDC_INSERT_TIMEOUT;
        }
        CQtTime::Sleep(200);
    }
    return ERR_IDC_HWERR;
}

int DevFIDC_CJ201::CancelReadCard()
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    m_SemCancel.SetEvent();
    if (m_pDev != nullptr)
        m_pDev->CancelRead();
    return 0;
}

int DevFIDC_CJ201::WriteTracks(const STTRACK_INFO &stTrackInfo)
{
    return ERR_IDC_UNSUP_CMD;
}

int DevFIDC_CJ201::ReadTracks(STTRACK_INFO &stTrackInfo)
{
    return ERR_IDC_UNSUP_CMD;
}

int DevFIDC_CJ201::DetectCard(IDC_IDCSTAUTS &IDCstatus)
{
    THISMODULE(__FUNCTION__);
    //AutoMutex(m_MutexAction);

    VERTIFYISOPEN();
    char pFWVersion[MAX_LEN_FWVERSION] = {0};
    unsigned int uLen = 0;
    int iRet = GetFWVersion(pFWVersion, uLen);
    if (iRet != ERR_IDC_SUCCESS)
    {
        m_nCardStatus =  IDCSTATUS_UNKNOWN;
    }
    else if (m_nCardStatus == IDCSTAUTS_ENTERING)
    {
        char pATRInfo[128] = {0};
        unsigned int uATRLen;
        if (ActiveCard(pATRInfo, uATRLen) == ERR_IDC_SUCCESS)
        {
            m_nCardStatus = IDCSTAUTS_ENTERING;
        }
        else
        {
            m_nCardStatus = IDCSTAUTS_NOCARD;
        }
    }
    IDCstatus =  m_nCardStatus;
    return iRet;
}

int DevFIDC_CJ201::GetFWVersion(char pFWVersion[MAX_LEN_FWVERSION], unsigned int &uLen)
{
    const char *ThisModule = "GetFWVersion";
    int iLenOut = 0;
    char szRecvData[MAX_RES_LEN] = {0};
    int nLenRecv = MAX_RES_LEN;
    int iRet = SendAndRecv(0x89, szRecvData, nLenRecv, ThisModule);

    return iRet;
}


int DevFIDC_CJ201::CloseField()
{
    const char *ThisModule = "CloseField";
    VERTIFYISOPEN();
    int iLenOut = 0;
    char szRecvData[MAX_RES_LEN] = {0};
    int nLenRecv = MAX_RES_LEN;
    int iRet = SendAndRecv(0x4B, szRecvData, nLenRecv, ThisModule);
    return iRet;
}

int DevFIDC_CJ201::MoveCard()
{
    const char *ThisModule = "MoveCard";
    VERTIFYISOPEN();
    int iLenOut = 0;
    char szRecvData[MAX_RES_LEN] = {0};
    int nLenRecv = MAX_RES_LEN;
    int iRet = SendAndRecv(0x4C, szRecvData, nLenRecv, ThisModule);

    return iRet;
}

int DevFIDC_CJ201::OpenLed()
{
    const char *ThisModule = "OpenLed";
    VERTIFYISOPEN();
    int iLenOut = 0;
    char szRecvData[MAX_RES_LEN] = {0};
    int nLenRecv = MAX_RES_LEN;
    int iRet = SendAndRecv(0x49, szRecvData, nLenRecv, ThisModule);
    return iRet;
}

int DevFIDC_CJ201::CloseLed()
{
    const char *ThisModule = "CloseLed";
    VERTIFYISOPEN();
    int iLenOut = 0;
    char szRecvData[MAX_RES_LEN] = {0};
    int nLenRecv = MAX_RES_LEN;
    int iRet = SendAndRecv(0x47, szRecvData, nLenRecv, ThisModule);
    return iRet;
}

int DevFIDC_CJ201::ActiveCard(char *szATR, unsigned int &nATRLen)
{
    const char *ThisModule = "ActiveCard";
    VERTIFYISOPEN();
    int iLenOut = 0;
    char szRecvData[MAX_RES_LEN] = {0};
    int nLenRecv = MAX_RES_LEN;
    int iIndex = 0;
    int iRet = 0;
    while (iIndex++ < 10)
    {
        iRet = SendAndRecv(0x42, szRecvData, nLenRecv, ThisModule);

        if (iRet >= 0)
        {
            iRet = AnalyseCmdResult(szRecvData, nLenRecv, szATR, nATRLen);
            if (iRet == ERR_IDC_COMM_ERR)
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }

    m_nCardStatus = iRet == 0 ? IDCSTAUTS_INTERNAL : IDCSTAUTS_NOCARD ;
    return iRet;
}

int DevFIDC_CJ201::SetRecycleCount(LPCSTR pszCount)
{
    return ERR_IDC_SUCCESS;
}

/////////////////////////////////////IC Card////////////////////////////////
int DevFIDC_CJ201::ICCPress()
{
    return ERR_IDC_SUCCESS;
}

int DevFIDC_CJ201::ICCRelease()
{
    return MoveCard();;
}

int DevFIDC_CJ201::ICCActive(char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen)
{
    return ActiveCard(pATRInfo, uATRLen);
}

int DevFIDC_CJ201::ICCReset(ICCardReset eResetFlag, char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen)
{
    int iRet = CloseField();
    return ActiveCard(pATRInfo, uATRLen);
}

int DevFIDC_CJ201::ICCMove()
{
    return 0;
}

int DevFIDC_CJ201::ICCDeActivation()
{
    int iRet = MoveCard();
    return iRet;
}

int DevFIDC_CJ201::ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz)
{
    const char *ThisModule = "ICCChipIO";
    VERTIFYISOPEN();
    if (eProFlag != ICCARD_PROTOCOL_T0)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    int iLenOut = 0;
    char szRecvData[MAX_RES_LEN] = {0};
    int nLenRecv = MAX_RES_LEN;
    //    //int iRet = SendAndRecv(0x43, szRecvData, nLenRecv, ThisModule);
    //    int iRet = SendAndRecv(0x43, pInOutData, nInOutLen, szRecvData, nLenRecv, ThisModule);
    //    if(iRet >= 0)
    //    {
    //       unsigned int nATRLen = 0;
    //       iRet = AnalyseCmdResult(szRecvData, nLenRecv, pInOutData, nATRLen);
    //       nInOutLen = nATRLen;
    //    }

    int iIndex = 0;
    int iRet = 0;
    while (iIndex++ < 10)
    {
        iRet = SendAndRecv(0x43, pInOutData, nInOutLen, szRecvData, nLenRecv, ThisModule);

        if (iRet >= 0)
        {
            unsigned int nATRLen = 0;
            iRet = AnalyseCmdResult(szRecvData, nLenRecv, pInOutData, nATRLen);
            nInOutLen = nATRLen;
            if (iRet == ERR_IDC_COMM_ERR)
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }

    return iRet;
}


int DevFIDC_CJ201::SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct)
{
    if (eFlagLedAct == LEDACTION_OPEN)
    {
        return OpenLed();
    }
    else if (eFlagLedAct == LEDACTION_CLOSE)
    {
        return CloseLed();
    }
    else
    {
        return ERR_IDC_UNSUP_CMD;
    }
}

int DevFIDC_CJ201::SetRFIDCardReaderBeep(unsigned long ulTime)
{
    return ERR_IDC_UNSUP_CMD;
}

//30-00-00-00(FS#0014)
BOOL DevFIDC_CJ201::GetSkimmingCheckStatus()
{
    return FALSE;
}

bool DevFIDC_CJ201::IsFormatCorrect(const char *lpResData, int nDataLen)
{
    const char *ThisModule = "AnalyseCmdResult";
    int nOffset = 0;
    if (lpResData[nOffset++] != 0x02)
    {
        Log(ThisModule, -1, "返回数据格式错误0");
        return false;
    }

    unsigned char X = 0;
    if (lpResData[nOffset] & 0x80)
    {
        X = (lpResData[nOffset] & 0x0F) - 1;
        nOffset++;
    }

    int nATRLen = (((lpResData[nOffset] & 0x0F) << 4) | (lpResData[nOffset + 1] & 0x0F)) + 256 * X;
    nOffset += 2;
    nATRLen--;

    if (lpResData[nOffset++] != 0x33)
    {
        Log(ThisModule, -1, "返回数据格式错误1");
        return false;
    }

    switch (lpResData[nOffset++])
    {
    case 0x30:       break;
    case 0x31:       return ERR_IDC_CHIPIOFAILED;
    case 0x32:       return ERR_IDC_UNSUP_CMD;
    case 0x33:       return ERR_IDC_ACTIVEFAILED;
    default:
        Log(ThisModule, -1, "返回数据格式错误2");
        return false;
    }

    nOffset += nATRLen * 2;

    if (lpResData[nOffset++] != 0x03)
    {
        Log(ThisModule, -1, "返回数据格式错误3");
        return false;
    }

    return true;
}


int DevFIDC_CJ201::AnalyseCmdResult(const char *lpResData, int nDataLen, char *lpATRData, unsigned int &nATRLen)
{
    const char *ThisModule = "AnalyseCmdResult";
    int nOffset = 0;
    if (lpResData[nOffset++] != 0x02)
    {
        Log(ThisModule, -1, "返回数据格式错误0");
        return ERR_IDC_COMM_ERR;
    }

    unsigned char X = 0;
    if (lpResData[nOffset] & 0x80)
    {
        X = (lpResData[nOffset] & 0x0F) - 1;
        nOffset++;
    }

    nATRLen = (((lpResData[nOffset] & 0x0F) << 4) | (lpResData[nOffset + 1] & 0x0F)) + 256 * X;
    nOffset += 2;
    nATRLen--;

    if (lpResData[nOffset++] != 0x33)
    {
        Log(ThisModule, -1, "返回数据格式错误1");
        return ERR_IDC_COMM_ERR;
    }

    switch (lpResData[nOffset++])
    {
    case 0x30:       break;
    case 0x31:       return ERR_IDC_CHIPIOFAILED;
    case 0x32:       return ERR_IDC_UNSUP_CMD;
    case 0x33:       return ERR_IDC_ACTIVEFAILED;
    case 0x36:       return ERR_IDC_CONFLICT;
    default:
        Log(ThisModule, -1, "返回数据格式错误2");
        return ERR_IDC_COMM_ERR;
    }

    for (int i = 0; i < nATRLen; i++)
    {
        lpATRData[i] = ((lpResData[nOffset + i * 2] & 0x0F) << 4) | (lpResData[nOffset + i * 2 + 1] & 0x0F);
    }
    nOffset += nATRLen * 2;

    if (lpResData[nOffset++] != 0x03)
    {
        Log(ThisModule, -1, "返回数据格式错误3");
        return ERR_IDC_COMM_ERR;
    }

    return ERR_IDC_SUCCESS;
}

int DevFIDC_CJ201::SendAndRecv(char cCmd, char *lpRecvData, int &nLenOut, LPCSTR lpModel)
{
    char szData[MAX_EXT] = {0x01};
    int nDataLen = 0x01;
    return SendAndRecv(cCmd, szData, nDataLen, lpRecvData, nLenOut, lpModel);
}

int DevFIDC_CJ201::SendAndRecv(char cCmd, const char *lpSendData, int nDataLen, \
                               char *lpRecvData, int &nLenOut, LPCSTR ThisModule)
{
    //THISMODULE(__FUNCTION__);

    LOGDEVACTION();

    if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null");
        return ERR_IDC_NOT_OPEN;
    }

    char szBuff[MAX_SEND_LEN] = {0};
    char LRC = 0;
    DWORD nOffset = 0;
    szBuff[0] = 0x02;
    szBuff[1] = cCmd;
    szBuff[2] = (0X30 | ((((char)nDataLen) >> 4) & 0X0F));
    szBuff[3] = (0X30 | ((char)nDataLen & 0X0F));
    nOffset += 4;
    //comdata
    LRC = nDataLen;
    for (int i = 0 ; i < nDataLen; i++)
    {
        szBuff[4 + i * 2]   = (0X30 | ((((char)lpSendData[i]) >> 4) & 0X0F));
        szBuff[4 + i * 2 + 1] = (0X30 | ((char)lpSendData[i] & 0X0F));
        LRC = LRC ^ lpSendData[i];
    }
    nOffset += nDataLen * 2;

    szBuff[nOffset] = 0x03;
    LRC = LRC ^ szBuff[nOffset];
    nOffset++;

    szBuff[nOffset] = (0X30 | ((((unsigned char)LRC) >> 4) & 0X0F));
    szBuff[nOffset + 1] = (0X30 | ((unsigned char)LRC & 0X0F));
    nOffset += 2;

    int iIndex = 0;
    DWORD dwRecvLen = nLenOut;
    do
    {
        //int nRet = m_pDev->SendAndRead(szBuff, nOffset, lpRecvData, dwRecvLen, 3000);
        int nRet = SendAndRecvData(szBuff, nOffset, lpRecvData, dwRecvLen, 3000);

        nLenOut = dwRecvLen;
        if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
        {
            if (nRet == ERR_DEVPORT_WRITE)
                Log(ThisModule, -1, "%s: 读卡器没有响应", ThisModule);
            else
                Log(ThisModule, -1, "%s: SendData()出错，返回%d", ThisModule, nRet);
        }
        m_nLastError = nRet;
        if (nRet < 0)
        {
            if ((nRet == ERR_DEVPORT_NOTOPEN) && (iIndex < 3))
            {
                iIndex++;
                Log(ThisModule, -1, "读卡器重试打开第%d次", iIndex);
                Close();
                int nRetTemp = Open(nullptr);
                if (nRetTemp == ERR_DEVPORT_SUCCESS)
                {
                    continue;
                }
            }
        }

        return ConvertErrorCode(nRet);

    } while (TRUE);
}

inline long GetDataStart(LPCSTR lpData, DWORD dwDataLen)
{
    DWORD Index = 0;
    while (Index < dwDataLen)
    {
        if (*(lpData + Index) == 0x02)
        {
            return Index;
        }
        Index++;
    }
    return -1;
}

int DevFIDC_CJ201::SendAndRecvData(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwReadLen, DWORD dwTimeOut)
{

    int iRet = m_pDev->Send(lpSendData, dwSendLen, TIME_SEND);
    if (iRet < 0)
    {
        return iRet;
    }
#define MAX_LEN_ONCE (64)
    char szBuff[MAX_LEN_ONCE] = {0};
    DWORD dwBuffSize = MAX_LEN_ONCE;
    dwReadLen = 0;
    long dwStart = -1;
    long dwDataTotalLen = 0;
    while (true)
    {
        memset(szBuff, 0, sizeof(szBuff));
        dwBuffSize = MAX_LEN_ONCE;
        if (dwDataTotalLen > 0)
        {
            DWORD dwLeft = dwDataTotalLen - dwReadLen;
            dwBuffSize =  dwLeft > MAX_LEN_ONCE ? MAX_LEN_ONCE : dwLeft;
        }
        iRet = m_pDev->Read(szBuff, dwBuffSize, TIME_READ);
        if (iRet < 0 || dwBuffSize <= 0)
        {
            break;
        }

        if (dwStart < 0)
        {
            dwStart = GetDataStart(szBuff, dwBuffSize);
            if (dwStart >= 0)
            {
                unsigned char X = 0;
                int nOffset = 1;
                if (szBuff[nOffset] & 0x80)
                {
                    X = (szBuff[nOffset] & 0x0F) - 1;
                    nOffset++;
                }
                dwDataTotalLen = (((szBuff[nOffset] & 0x0F) << 4) | (szBuff[nOffset + 1] & 0x0F)) + 256 * X;
                dwDataTotalLen *= 2;
                dwDataTotalLen += 6;

                DWORD dwValidDataLen =  dwBuffSize - dwStart;
                if (dwValidDataLen >= dwDataTotalLen)
                {
                    dwValidDataLen = dwDataTotalLen;
                }
                memcpy(lpReadData + dwReadLen, szBuff + dwStart, dwValidDataLen);
                dwReadLen += dwValidDataLen;
            }
        }
        else
        {
            memcpy(lpReadData + dwReadLen, szBuff, dwBuffSize);
            dwReadLen += dwBuffSize;
        }

        if ((dwReadLen >= dwDataTotalLen) && (dwDataTotalLen > 0))
        {
            break;
        }

        //usleep(50*1000);
    }

    return iRet;
}

int DevFIDC_CJ201::ConvertErrorCode(long iRet)
{
    switch (iRet)
    {
    case ERR_DEVPORT_SUCCESS:        return ERR_IDC_SUCCESS;
    case ERR_DEVPORT_PARAM:          return ERR_IDC_PARAM_ERR;
    case ERR_DEVPORT_NOTOPEN:        return ERR_IDC_COMM_ERR;
    case ERR_DEVPORT_CANCELED:       return ERR_IDC_USER_CANCEL; // 特殊错误，用户取消
    case ERR_DEVPORT_RTIMEOUT:       return ERR_IDC_READTIMEOUT; // 特殊错误，超时未插卡
    case ERR_DEVPORT_READERR:        return ERR_IDC_READERROR;
    case ERR_DEVPORT_WRITE:          return ERR_IDC_WRITEERROR;
    case ERR_DEVPORT_WTIMEOUT:       return ERR_IDC_WRITETIMEOUT;
    case ERR_DEVPORT_LIBRARY:        return ERR_IDC_OTHER;
    case ERR_DEVPORT_NODEFINED:      return ERR_IDC_OTHER;
    default:                         return ERR_IDC_OTHER;
    }
}
