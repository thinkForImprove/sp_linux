#include "DevIDC_EC2G.h"

static const char *ThisFile = "DevIDC_EC2G.cpp";
//////////////////////////////////////////////////////////////////////////
//extern "C" Q_DECL_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev)
//{
//    pDev = new CDevIDC_EC2G(lpDevType);
//    return (pDev != nullptr) ? 0 : -1;
//}

//////////////////////////////////////////////////////////////////////////
CDevIDC_EC2G::CDevIDC_EC2G(LPCSTR lpDevType): m_SemCancel(false)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    InitConfig();
}
CDevIDC_EC2G::~CDevIDC_EC2G()
{
    Release();
}

void CDevIDC_EC2G::Release()
{
    Close();
    if(m_pDev != nullptr)
        m_pDev.Release();
}

LONG CDevIDC_EC2G::ReadConfig()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    AutoLogFuncBeginEnd();
    QByteArray strFile("IDCConfig.ini");
#ifdef QT_WIN32
    strFile.prepend(SPETCPATH);
#else
    strFile.prepend(SPETCPATH);
#endif
    if (!m_cINI.LoadINIFile(strFile.constData()))
    {
        Log(ThisModule, __LINE__, "EC2G:加载配置文件失败：%s", strFile.constData());
        return -1;
    }

    CINIReader cIR = m_cINI.GetReaderSection("IDCInfo");
    m_stIDC.bWaitInsertCardIntervalTime = (DWORD)cIR.GetValue("WaitInsertCardIntervalTime", "3");
    m_stIDC.bSupportPredictIC = (USHORT)cIR.GetValue("SupportPredictIC", "0");
    m_stIDC.bReTakeIn = (USHORT)cIR.GetValue("ReTakeIn", "1");
    m_stIDC.bFraudEnterCardTimes = (DWORD)cIR.GetValue("FraudEnterCardTimes", "5");
    m_stIDC.bNeedFraudProtect = (BOOL)cIR.GetValue("NeedFraudProtect", "0");
    m_stIDC.bFraudProtectTime = (INT)cIR.GetValue("FraudProtectTime", "4");
    m_stIDC.bTamperSensorSupport = (DWORD)cIR.GetValue("TamperSensorSupport", "0");
    return 0;
}

int CDevIDC_EC2G::InitConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = ReadConfig();
    if (nRet < 0)
    {
        return nRet;
    }

    Log(ThisModule, 1, "读取配置文件成功");
    return nRet;
}

int CDevIDC_EC2G::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    if (m_pDev == nullptr)
    {
        if (0 != m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "IDC", "DevIDC_EC2G"))
        {
            Log(ThisModule, __LINE__, "Load(AllDevPort.dll) failed");
            return ERR_IDC_COMM_ERR;
        }
    }

    if (pMode == nullptr)
    {
        if (m_strMode.empty())
        {
            char szDevID[MAX_EXT] = {0};
            sprintf(szDevID, "USB:%d,%d", 0x04A4, 0x00BF);
            m_strMode = szDevID;
        }
    }
    else
    {
        m_strMode = pMode;
    }

    long lRes = OpenDevice();
    if(lRes != 0)
        return lRes;

    return 0;

}

long CDevIDC_EC2G::OpenDevice()
{
    THISMODULE(__FUNCTION__);
    LOGDEVACTION();
    long iRet = m_pDev->Open(m_strMode.c_str());
    if (iRet < 0)
    {
        Log(ThisModule, __LINE__, "Open dev failed ");
        return ConvertErrorCode(iRet);
    }

    Log(ThisModule, 1, "Open dev success ");
    return 0;
}

void CDevIDC_EC2G::Close()
{
    THISMODULE(__FUNCTION__);
    if (m_pDev == nullptr)
        return;

    LOGDEVACTION();
    m_pDev->Close();

}

int CDevIDC_EC2G::Init(CardAction eActFlag, WobbleAction nNeedWobble)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    //VERTIFYISOPEN();
    LOGDEVACTION();

    if(m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null");
        return ERR_DEVPORT_NOTOPEN;
    }

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，不做复位。");
        return ERR_IDC_HWERR;
    }

    // 驱动内部判断是否退卡成功与卡被取走，用于重进卡判断
    m_nEjectedCard = EJECT_UNKNOWN;
    m_nTakeCard = MEDIA_UNKOWN;

    Log(ThisModule, 1, "调用Init");

    int nRet = UpdateCardStatus();
    if(nRet == ERR_IDC_COMM_ERR)
    {
        Log(ThisModule, __LINE__, "Not Open Dev, try open 1");
        Close();
        OpenDevice();
    }
//    if (nRet < 0 && nRet != ERR_IDC_HWERR)
//    {
//        return nRet;
//    }

    if (eActFlag == CARDACTION_RETRACT) // 3 吞卡
    {
        //招行需求复位时不能开启闸门，避免用户卡片插入
        //C01 => C05 No Shutter Control
        nRet = SendCmd("C05", 3, ThisModule);
    }
    else if (eActFlag == CARDACTION_NOACTION) // 1 移动并保持
    {
        //C02 => C06 No Shutter Control
        nRet = SendCmd("C06", 3, ThisModule);
    }
    else if (eActFlag == CARDACTION_NOMOVEMENT) // 4 不移动并保持
    {
        nRet = SendCmd("C070A", 5, ThisModule);
    }
    else // 2 弹卡
    {
        //C00 => C04 No Shutter Control when no card is in
        //C00 => C03 To remove the short card or the long card
        if (nRet <= 0 || !m_bCardTooShortError)
            nRet = SendCmd("C04", 3, ThisModule); // 卡状态未知、无卡或者无短卡，不打开门禁
        else
            nRet = SendCmd("C03", 3, ThisModule); // 有卡并且短卡，打开门禁
    }
    if (nRet < 0)
    {
        return nRet;
    }
    char szReply[2068] = {0};
    nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (szReply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(szReply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }
        return ERR_IDC_HWERR;
    }
    if (eActFlag == CARDACTION_EJECT)
    {
        m_bCardTooShortError = FALSE;
        m_nEjectedCard = EJECT_SUCCESS;
        m_nTakeCard = MEDIA_NOTAKEN;
    }

    if (m_pDev != nullptr)
        m_pDev->SetLogAction("ConnectCRM");

    nRet = SendCmd("CSl1", 4, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, 1, "发送连接CRM命令失败，nRet=%d", nRet);
        return nRet;
    }
    nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, 1, "发送连接CRM命令后，接收响应数据失败，nRet=%d", nRet);
        return nRet;
    }

    nRet = SetICControlInfomationMode();
    if (nRet != 0)
    {
        return nRet;
    }
    SetWobble(nNeedWobble);


    if (m_stIDC.bReTakeIn == 0)
        m_bReTakeIn = FALSE;
    else
    {
        m_bReTakeIn = TRUE;
        //设置重进卡命令超时时间，设备自身默认10毫秒
        nRet = SetTimeOutMonitor(MONITOR_REINTAKE, 30);
        if (nRet != 0)
            return nRet;
    }
    //TransparentCardSetting(FALSE);
    return ERR_IDC_SUCCESS;
}

/*设置指定命令超时时间，仅对进卡、撤卡、重进卡命令有效，返回：=0成功，<0失败*/
int CDevIDC_EC2G::SetTimeOutMonitor(MONITOR_TYPE MonitorType, int nTimeOut)
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    if (nTimeOut < 0 || nTimeOut > 99)
    {
        Log(ThisModule, __LINE__, "超时时间参数异常:%d", nTimeOut);
        return ERR_IDC_PARAM_ERR;
    }

    char szCmd[255] = {0};
    char reply[2068] = {0};
    char t[10] = {0};

    switch (MonitorType)
    {
    case MONITOR_INTAKE:
        {
            memcpy(szCmd, "CW0", 3);
            break;
        }
    case MONITOR_WITHDRAWAL:
        {
            memcpy(szCmd, "CW1", 3);
            break;
        }
    case MONITOR_REINTAKE:
        {
            memcpy(szCmd, "CW2", 3);
            break;
        }
    default:
        return ERR_IDC_PARAM_ERR;
    }

    sprintf(t, "%02d", nTimeOut);
    strcat(szCmd, t);

    int nRet = SendCmd(szCmd, strlen(szCmd), ThisModule);
    if (nRet != 0)
        return nRet;

    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
        return nRet;

    if (reply[3] != 'P')
    {
        CardReaderError(reply + 6, ThisModule); //记录错误日志
        return ERR_IDC_HWERR;
    }

    return 0;
}

// 新增设置读卡器为IC控制信息模式以支持IC卡操作，在初始化函数中调用
int CDevIDC_EC2G::SetICControlInfomationMode()
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    char szReply[2068] = {0};
    // CY1+CMP(2)
    // CMP:
    // 00 ISO/IEC 7816-3
    // 05 EMV4.3
    // 08 MONEO
    // 09 PBOC
    int nRet = SendCmd("CY105", 5, ThisModule);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return ERR_IDC_COMM_ERR;
    }
    nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_NO_ACTION, ThisModule);
    if ((nRet < 0) || (szReply[3] != 'P'))
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }
    return ERR_IDC_SUCCESS;
}

//抖动进卡
int CDevIDC_EC2G::SetWobble(WobbleAction nNeedWobble)
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    int nRet;
    char szReply[2068] = {0};

    if (WOBBLEACTION_START == nNeedWobble)
    {
        m_pDev->SetLogAction(ThisModule);
        nRet = SendCmd("CSL1", 4, ThisModule);
        if (nRet != 0)
        {
            Log(ThisModule, __LINE__, "SendCmd()失败");
            return ERR_IDC_COMM_ERR;
        }

        nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_NO_ACTION, ThisModule);
        if (nRet < 0 || szReply[3] != 'P')
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }
    }
    else if (WOBBLEACTION_STOP == nNeedWobble)
    {
        m_pDev->SetLogAction(ThisModule);
        nRet = SendCmd("CSL0", 4, ThisModule);
        if (nRet != 0)
        {
            Log(ThisModule, __LINE__, "SendCmd()失败");
            return ERR_IDC_COMM_ERR;
        }

        memset(szReply, 0, sizeof(szReply));
        nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_NO_ACTION, ThisModule);
        if (nRet < 0 || szReply[3] != 'P')
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }
    }

    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::EatCard()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，吞卡取消。");
        return ERR_IDC_HWERR;
    }

    m_nEjectedCard = EJECT_UNKNOWN;
    m_nTakeCard = MEDIA_UNKOWN;

    Log(ThisModule, 1, "调用EatCard");

    char reply[2068] = {0};
    int nRet = SendCmd("C41", 3, ThisModule);
    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM ||
            iCardReaderError == CR_ERROR_RETRIEVE_TIMEOUT)
        {
            return ERR_IDC_JAMMED;
        }
        return ERR_IDC_HWERR;
    }
    return 0;
}

int CDevIDC_EC2G::EjectCard()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，弹卡取消。");
        return ERR_IDC_HWERR;
    }

    m_nEjectedCard = EJECT_UNKNOWN;
    m_nTakeCard = MEDIA_UNKOWN;

    Log(ThisModule, 1, "调用EjectCard");

    char reply[2068] = {0};
    int nRet = 0;
    //对于短卡使用C03退卡，对于正常卡使用C00初始化弹卡
    if (m_bCardTooShortError)
    {
        nRet = SendCmd("C03", 3, ThisModule);
    }
    else
    {
        nRet = SendCmd("C00", 3, ThisModule);
    }

    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        m_nEjectedCard = EJECT_FAILED;
        m_nTakeCard = MEDIA_UNKOWN;

        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }
        return ERR_IDC_HWERR;
    }

    m_bCardTooShortError = FALSE;
    m_nEjectedCard = EJECT_SUCCESS;
    m_nTakeCard = MEDIA_NOTAKEN;
    return 0;
}

int CDevIDC_EC2G::AcceptCard(ULONG ulTimeOut, bool Magnetic)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();


    while (true)
    {
        if (!m_SemCancel.WaitForEvent(10))
        {
           break;
        }
    }

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，不能进卡。");
        return ERR_IDC_HWERR;
    }

    Log(ThisModule, 1, "调用AcceptCard");
    char reply[2068] = {0};
    int nRet;

    DWORD dwRetryTimes          = 0; // 检测到卡没成功进卡重试次数
    DWORD dwLastFailedTime      = 0;
    ULONG dwStart = CQtTime::GetSysTick();
    m_bCardTooShortError = FALSE;

    while (TRUE)
    {
        if (ulTimeOut != INFINITE)
        {
            if ((CQtTime::GetSysTick() - dwStart) >= ulTimeOut)
            {
                return ERR_IDC_INSERT_TIMEOUT;
            }
        }

        //检测读卡器异形口状态
        if (TamperSensorDetect())
        {
            Log(ThisModule, __LINE__, "读卡器异型口被用手或者其他东西挡住");
            return ERR_IDC_HWERR;
        }

        if (m_stIDC.bNeedFraudProtect)
        {

            if (dwRetryTimes > 0 && CQtTime::GetSysTick() - dwLastFailedTime > m_stIDC.bFraudProtectTime)
            {
                dwRetryTimes = 0;
            }
        }

        nRet = UpdateCardStatus();
        if (nRet < 0) // 处理卡的错误状态
        {
            StopCmd();
            return ERR_IDC_HWERR;
        }

        if (m_nCardStatus == IDCSTAUTS_INTERNAL)
        {
            StopCmd();
            return ERR_IDC_SUCCESS;
        }
        else if (m_nCardStatus == IDCSTAUTS_NOCARD)
        {
            CQtTime::Sleep(100);
        }
        else if (m_nCardStatus == IDCSTAUTS_ENTERING)
        {
            CQtTime::Sleep(100);
        }
        else if (m_nCardStatus == IDCSTAUTS_ICC_PRESS) // IC卡处于压下状态
        {
            // 释放IC卡后返回进卡成功。
            StopCmd();
            nRet = ICCRelease();
            if (nRet != 0)
            {
                return ERR_IDC_HWERR;
            }
            return ERR_IDC_SUCCESS;
        }
        else if ((m_nCardStatus == IDCSTAUTS_ICC_ACTIVE) || (nRet == 5)) //IC卡处于激活状态
        {
            // 去激活IC卡
            StopCmd();
            nRet = ICCDeActivation();
            if (nRet != 0)
            {
                return ERR_IDC_HWERR;
            }
            // 释放IC卡后返回进卡成功。
            nRet = ICCRelease();
            if (nRet != 0)
            {
                return ERR_IDC_HWERR;
            }
            return ERR_IDC_SUCCESS;
        }
        else
        {
            StopCmd();
            Log(ThisModule, __LINE__, "卡状态异常,CardStatus：%d", m_nCardStatus);
            return ERR_IDC_HWERR;
        }

        string strCMD;
        m_pDev->SetLogAction(ThisModule);
        //实现重进卡功能
        if (m_bReTakeIn && m_bCardInGatePosition && m_nTakeCard == MEDIA_NOTAKEN)
        {
            Log(ThisModule, 1, "调用重进卡命令");
            nRet = SendCmd("C40", 3, ThisModule);   // 重新吸入卡
        }
        else
        {
            // 读卡器功能
            // a不判 C20
            // b只判磁 C21
            // c只判IC C25002
            // d判磁或IC C25003
            // e判磁且IC C25004
            // 配置文件
            // 0 判磁:b只判磁、不判磁:a不判（默认）
            // 1 判磁:d判磁或IC、不判磁:c只判IC
            // 2 判磁:e判磁且IC、不判磁:c只判IC
            switch (m_stIDC.bSupportPredictIC)
            {
            case 0:
                strCMD = Magnetic ? "C21" : "C20";
                break;
            case 1:
                strCMD = Magnetic ? "C25003" : "C25002";
                break;
            case 2:
                strCMD = Magnetic ? "C25004" : "C25002";
                break;
            default:
                strCMD = Magnetic ? "C21" : "C20";
                break;
            }
            nRet = SendCmd(strCMD.c_str(), strCMD.length(), ThisModule);
        }
        if (nRet < 0)
        {
            return nRet;
        }

        if (ulTimeOut == INFINITE)
            nRet = GetResponse(reply, sizeof(reply), m_stIDC.bWaitInsertCardIntervalTime * 1000, ThisModule);
        else
        {
            if (CQtTime::GetSysTick() - dwStart >= ulTimeOut)
            {
                StopCmd();
                return ERR_IDC_INSERT_TIMEOUT;
            }
            else
                nRet = GetResponse(reply, sizeof(reply), m_stIDC.bWaitInsertCardIntervalTime * 1000, ThisModule);
        }

        if (m_SemCancel.WaitForEvent(10))
        {
            Log(ThisModule, __LINE__, "检测到取消");
            StopCmd();
            return ERR_IDC_USER_CANCEL;
        }

        if (nRet == ERR_IDC_USER_CANCEL)
        {
            Log(ThisModule, __LINE__, "进卡取消");
            StopCmd();
            return nRet;
        }
        if (nRet < 0 && nRet != ERR_IDC_READTIMEOUT)
        {
            return nRet;
        }
        if (nRet > 0)
        {
            if (reply[3] != 'P')
            {
                nRet = CardReaderError(reply + 6, ThisModule);
                if (nRet == CR_ERRORINSERT_TIMEOUT) //读卡器内部超时
                    nRet = ERR_IDC_READTIMEOUT;
                else if (CR_ERROR_JAM == nRet || CR_ERROR_JAM_IN_REAR_END == nRet)
                {
                    nRet = ERR_IDC_JAMMED;
                }
                else if (CR_ERROR_CARD_DRAWNOUT == nRet)
                {
                    nRet = ERR_IDC_CARDPULLOUT;
                }
                else if (CR_ERROR_LONG_CARD == nRet)
                {
                    nRet = ERR_IDC_CARDTOOLONG;
                }
                else if (CR_ERROR_SHORT_CARD == nRet)
                {
                    nRet = ERR_IDC_CARDTOOSHORT;
                    m_bCardTooShortError = TRUE;
                }
                else
                    nRet = ERR_IDC_HWERR;
            }
            else
            {
                return ERR_IDC_SUCCESS;
            }
        }

        // 超时未插卡
        if (nRet == ERR_IDC_READTIMEOUT)
        {
            StopCmd();

            if (m_stIDC.bNeedFraudProtect)
            {
                if (UpdateCardStatus() > 0)
                {
                    dwRetryTimes++;
                    dwLastFailedTime = CQtTime::GetSysTick();
                }
                if (dwRetryTimes >= m_stIDC.bFraudEnterCardTimes)
                {
                    m_bFraudDetected = TRUE;
                    m_dwLastFraudDetectedTime = CQtTime::GetSysTick();
                    Log(ThisModule, __LINE__, "进入防逗卡保护。");
                    return ERR_IDC_HWERR;
                }
            }

            if (ulTimeOut == INFINITE)
            {
                CQtTime::Sleep(500);
                continue;
            }
            else
            {
                if ((CQtTime::GetSysTick() - dwStart) < ulTimeOut)
                {
                    CQtTime::Sleep(500);
                    continue;
                }
                else
                {
                    return ERR_IDC_INSERT_TIMEOUT;
                }
            }
        }
        else
        {
            return nRet;
        }
    }

    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::CancelReadCard()
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    m_SemCancel.SetEvent();
    if (m_pDev != nullptr)
        m_pDev->CancelRead();
    return 0;
}

//通知读卡器取消进卡
long CDevIDC_EC2G::StopCmd()
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    char reply[2068] = {0};
    int nRet = m_pDev->Send("\x01\x00\x01\x45", 4, TIMEOUT_NO_ACTION);
    if (nRet != 0)
    {
        return ERR_IDC_HWERR;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
    {
        return ERR_IDC_HWERR;
    }
    if (nRet > 2)
    {
        char szLog[255] = {0};
        for (int i = 0; i < nRet; i++)
        {
            sprintf(szLog + strlen(szLog), "0x%x ", reply[i]);
        }
        memset(reply, 0, sizeof(reply));
    }
    else
    {
        if (reply[3] != 0x45)
        {
            Log(ThisModule, __LINE__, "命令返回数据不符(0x%x, 0x%x)", reply[0], reply[1]);
        }
    }
    return  ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::WriteTracks(const STTRACK_INFO &stTrackInfo)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，写磁卡取消。");
        return ERR_IDC_HWERR;
    }

    Log(ThisModule, 1, "调用WriteTracks");

    //char *pszData[] = {stTrackInfo.szTrack1, stTrackInfo.szTrack2, stTrackInfo.szTrack3};
    //unsigned int nLen[3] = {strlen(stTrackInfo.TrackData[0].szTrack),\
    //                        strlen(stTrackInfo.TrackData[1].szTrack),\
    //                       strlen(stTrackInfo.TrackData[2].szTrack)};
    for (int i = 1; i < 4; i++)
    {
        string strTrackData = stTrackInfo.TrackData[i - 1].szTrack;
        if (strTrackData.size() == 0)
            continue;
        char reply[2068] = {0};
        char szCmd[255] = {0};
        sprintf(szCmd, "C79%d%s", i, strTrackData.c_str());
        int nRet = SendCmd(szCmd, 4 + strTrackData.size(), ThisModule);
        if (nRet != 0)
        {
            return nRet;
        }
        nRet = GetResponse(reply, sizeof(reply), TIMEOUT_ACTION, ThisModule);
        if (nRet < 0)
        {
            return nRet;
        }
        if (reply[3] != 'P')
        {
            nRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
            if (nRet == CR_ERROR_COMMAND_DATA_ERROR)//
            {
                return ERR_IDC_PARAM_ERR;
            }
            else
            {
                return ERR_IDC_WRITEERROR;
            }
        }
    }
    //m_Log.CancelLog();

    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::ReadTracks(STTRACK_INFO &stTrackInfo)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，读磁道取消。");
        return ERR_IDC_HWERR;
    }

    Log(ThisModule, 1, "调用ReadTracks");

    char reply[2068] = {0};
    char szCmd[256] = {0};
    bool bAllTrackBad = true;
    for (int i = 0; i < 3; i++)
    {
        //读123磁道数据
        m_pDev->SetLogAction(ThisModule);
        sprintf(szCmd, "C68%d", i + 1);
        int nRet = SendCmd(szCmd, 4, ThisModule);
        if (nRet != 0)
        {
            return nRet;
        }
        memset(reply, 0, sizeof(reply));
        nRet = GetResponse(reply, sizeof(reply), TIMEOUT_ACTION, ThisModule);
        if (nRet < 0 && (i != 2))
        {
            return nRet;
        }
        if (reply[3] == 'P')
        {
            int iIndex = reply[8] - '1';
            if (iIndex == i)
            {
                //stTrackInfo.bTrack1OK = TRUE;
                stTrackInfo.TrackData[i].bTrackOK = true;
                bAllTrackBad = false;
                //char szReportLen[3] = {0};
                //memcpy(szReportLen,reply+1,2);
                //DWORD nLen = GetReportLen(szReportLen);
                DWORD nLen = MAKEWORD(reply[2], reply[1]);
                memcpy(stTrackInfo.TrackData[i].szTrack, reply + 9, nLen - 6);
            }
        }
        if (reply[3] != 'P')
        {
            int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
            if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
                iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
                iCardReaderError == CR_ERROR_JAM)
            {
                return ERR_IDC_JAMMED;
            }
            else if (CR_ERROR_LONG_CARD == iCardReaderError)
            {
                return ERR_IDC_CARDTOOLONG;
            }
            else if (CR_ERROR_SHORT_CARD == iCardReaderError)
            {
                m_bCardTooShortError = TRUE;
                return ERR_IDC_CARDTOOSHORT;
            }
            else if (iCardReaderError >= CR_ERROR_TRACK_READING_ERROR &&
                     iCardReaderError <= CR_ERROR_READ_ERROR_IN_SS)
            {
                // 读卡器读取动作执行成功，但所有磁道数据无效
                Log(ThisModule, 1, "读卡器读卡动作执行成功，但磁道%d数据无效", i + 1);
            }
            else
                return ERR_IDC_HWERR;
        }

    }

    if (!bAllTrackBad)
    {
        return ERR_IDC_SUCCESS;
    }
    else
    {
        return ERR_IDC_NOTRACK;
    }
}

int CDevIDC_EC2G::UpdateCardStatus()
{
    THISMODULE(__FUNCTION__);
    //VERTIFYISOPEN();

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，检测卡取消。");
        return ERR_IDC_HWERR;
    }

    IDC_IDCSTAUTS IDCstatus = IDCSTATUS_UNKNOWN;
    m_bCardInGatePosition = FALSE;
    char reply[2068] = {0};
    int nRet = SendCmd("C11", 3, ThisModule);
    if (nRet != 0)
    {
        if (m_nLastError == nRet)
        {
            //m_Log.CancelLog();
        }
        m_nLastError = nRet;
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
    {
        if (m_nLastError == nRet)
        {
            //m_Log.CancelLog();
        }
        m_nLastError = nRet;
        return nRet;
    }
    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule);
        if (m_nLastError == nRet)
        {
            //m_Log.CancelLog();
        }
        m_nLastError = nRet;
        return ERR_IDC_HWERR;
    }

    if (strncmp(reply + 6, "01", 2) == 0)//S1
    {
        IDCstatus = IDCSTAUTS_ENTERING; //在门口
        if (reply[13] == '1')
            m_bCardInGatePosition = TRUE; // 卡在门口并且Shutter打开
        else
            m_bCardInGatePosition = FALSE;
    }
    else if (strncmp(reply + 6, "02", 2) == 0)//S2
        IDCstatus = IDCSTAUTS_INTERNAL;  //在内部
    else if (strncmp(reply + 6, "04", 2) == 0)//S3
        IDCstatus = IDCSTAUTS_INTERNAL;  //在内部
    else if (strncmp(reply + 6, "10", 2) == 0)
        IDCstatus = IDCSTAUTS_ICC_PRESS;  //IC Contact Press
    else if (strncmp(reply + 6, "11", 2) == 0)
        IDCstatus = IDCSTAUTS_ICC_ACTIVE;  //ICC Activation Status
    else if (strncmp(reply + 6, "20", 2) == 0)
        IDCstatus = IDCSTAUTS_ICC_ACTIVE;  //IC正在通讯
    else if (strncmp(reply + 6, "21", 2) == 0)
        IDCstatus = IDCSTAUTS_ICC_ACTIVE;  //IC正在通讯
    else if (strncmp(reply + 6, "22", 2) == 0)
        IDCstatus = IDCSTAUTS_ICC_ACTIVE;  //IC正在通讯
    else
    {
        IDCstatus = IDCSTAUTS_NOCARD; //无卡
        if (m_nEjectedCard == EJECT_SUCCESS)
        {
            m_nTakeCard = MEDIA_TAKEN;
        }
    }
    if (m_nCardStatus != IDCstatus)
    {
        char szLog[255] = {0};
        memcpy(szLog, reply + 6, 2);
        if (IDCstatus == IDCSTAUTS_ENTERING)
            sprintf(szLog + strlen(szLog), "(卡在门口，返回：%d)", nRet);
        else if (IDCstatus == IDCSTAUTS_INTERNAL)
            sprintf(szLog + strlen(szLog), "(卡在内部，返回：%d)", nRet);
        else if (IDCstatus == IDCSTAUTS_ICC_PRESS)
            sprintf(szLog + strlen(szLog), "(IC卡被压下，返回：%d)", nRet);
        else if (IDCstatus == IDCSTAUTS_ICC_ACTIVE)
            sprintf(szLog + strlen(szLog), "(IC卡被激活，返回：%d)", nRet);
        else
            sprintf(szLog + strlen(szLog), "(没有检测到卡，返回：%d)", nRet);
        Log(ThisModule, 1, "%s", szLog);
    }
    m_nCardStatus = IDCstatus;
    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::DetectCard(IDC_IDCSTAUTS &IDCstatus)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    LOGDEVACTION();
    int iRet = UpdateCardStatus();
    IDCstatus = m_nCardStatus;
    return iRet;
}

int CDevIDC_EC2G::GetFWVersion(char pFWVersion[MAX_LEN_FWVERSION], unsigned int &uLen)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，获取固件版本取消。");
        return ERR_IDC_HWERR;
    }

    if (nullptr == pFWVersion || uLen < 55)
    {
        return ERR_IDC_PARAM_ERR;
    }

    char reply[512] = {0};
    int nRet = SendCmd("CV0", 3, ThisModule);
    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        return ERR_IDC_HWERR;
    }

    memset(pFWVersion, 0, uLen);
    //memcpy(pszFWVersion, reply+7, nRet-10);
    strcpy(pFWVersion, reply + 8);
    pFWVersion[uLen - 1] = 0x00;
    uLen = strlen(pFWVersion);
    return ERR_IDC_SUCCESS;
}

bool CDevIDC_EC2G::InFraudProtect()
{
    if (m_stIDC.bNeedFraudProtect)
    {
        if (m_bFraudDetected)
        {
            //DWORD dwFraudProtectTime = 4 * 1000;
            if (m_stIDC.bFraudProtectTime == -1)
                return true;
            else if (m_stIDC.bFraudProtectTime == 0)
            {
                m_bFraudDetected = false;
                return false;
            }
            else if (CQtTime::GetSysTick() - m_dwLastFraudDetectedTime < m_stIDC.bFraudProtectTime * 1000)
                return true;
            else
                m_bFraudDetected = false;
        }
    }

    return false;
}

bool CDevIDC_EC2G::TamperSensorDetect()
{
    THISMODULE(__FUNCTION__);

    m_bTamperSensorStatus = TANMPER_SENSOR_NOT_AVAILABLE;
    // 不支持防盗嘴检测
    if (m_stIDC.bTamperSensorSupport == 0)
    {
        Log(ThisModule, __LINE__, "不支持防盗嘴检测");
        return false;
    }

    Log(ThisModule, __LINE__, "支持防盗嘴检测");
    //TamperSensor检测ON状态约1分20秒，OFF状态实时检测
    //NDT_SIUController
    // todo

    if (m_bTamperSensorStatus == TANMPER_SENSOR_ON)
        return true;
    else
        return false;
}

int CDevIDC_EC2G::SetRecycleCount(LPCSTR pszCount)
{
    return ERR_IDC_SUCCESS;
}

/////////////////////////////////////IC Card////////////////////////////////
int CDevIDC_EC2G::ICCPress()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    Log(ThisModule, 1, "调用ICCPress");

    char reply[2068] = {0};
    int nRet = SendCmd("CC0", 3, ThisModule);
    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }
        return ERR_IDC_HWERR;
    }

    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::ICCRelease()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    Log(ThisModule, 1, "调用ICCRelease");

    char reply[2068] = {0};
    int nRet = SendCmd("CC1", 3, ThisModule);
    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }

        return ERR_IDC_HWERR;
    }
    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::ICCActive(char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (nullptr == pATRInfo)
    {
        Log(ThisModule, 1, "输出参数pszATRInof为空");
        return ERR_IDC_PARAM_ERR;
    }
    Log(ThisModule, 1, "调用ICCActive");

    char reply[2068] = {0};
    int nRet = SendCmd("CC2", 3, ThisModule);
    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }
        if (iCardReaderError == CR_ERROR_ICC_ACTIVE_ERROR)
        {
            return ERR_IDC_INVALIDCARD;
        }

        return ERR_IDC_HWERR;
    }
    m_bICCActived = TRUE;
    //char szReportLen[3] = {0};
    //memcpy(szReportLen,reply+1,2);
    //DWORD nLen = GetReportLen(szReportLen);
    DWORD nLen = MAKEWORD(reply[2], reply[1]);
    uATRLen = nLen - 5; //去掉命令长度3位+去掉RES 2位
    memcpy(pATRInfo, reply + 8, uATRLen); // 返回去掉了消息头、尾以及BCC校验数据的纯ATR信息数据
    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::ICCReset(ICCardReset eResetFlag, char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (nullptr == pATRInfo)
    {
        Log(ThisModule, 1, "输出参数pATRInfo为空");
        return ERR_IDC_PARAM_ERR;
    }
    Log(ThisModule, 1, "调用ICCReset");

    char reply[2068] = {0};
    int nRet = -1;
    if (eResetFlag == ICCARDRESET_COLD)
    {
        nRet = SendCmd("CE0000", 6, ThisModule);
    }
    else if (eResetFlag == ICCARDRESET_WARM)
    {
        nRet = SendCmd("CE1000", 6, ThisModule);
    }

    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }

        return ERR_IDC_HWERR;
    }
    //char szReportLen[3] = {0};
    //memcpy(szReportLen,reply+1,2);
    //DWORD nLen = GetReportLen(szReportLen);
    m_bICCActived = TRUE;
    DWORD nLen = MAKEWORD(reply[2], reply[1]);
    uATRLen = nLen - 6;//去掉命令长度3位+去掉RES 2位 + 1位
    memcpy(pATRInfo, reply + 9, uATRLen); // 返回去掉了消息头
    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::ICCMove()
{
    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::ICCDeActivation()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    Log(ThisModule, 1, "调用ICCDeActivation");

    char reply[2068] = {0};
    int nRet = SendCmd("CC3", 3, ThisModule);
    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }

        return ERR_IDC_HWERR;
    }
    m_bICCActived = FALSE;
    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if ((nullptr == pInOutData) ||
        (nInOutLen < 4) ||
        ((eProFlag != ICCARD_PROTOCOL_T0) && (eProFlag != ICCARD_PROTOCOL_T1)))
    {
        return ERR_IDC_PARAM_ERR;
    }
    Log(ThisModule, 1, "调用ICCChipIO");

    char szCmd[2068] = {0};
    if (ICCARD_PROTOCOL_T0 == eProFlag)
    {
        memcpy(szCmd, "CF0", 3);
    }
    else if (ICCARD_PROTOCOL_T1 == eProFlag)
    {
        memcpy(szCmd, "CF1", 3);
    }

    memcpy(szCmd + 3, pInOutData, nInOutLen);
    int nRet = SendCmd(szCmd, nInOutLen + 3, ThisModule);
    if (nRet != 0)
    {
        return nRet;
    }
    char reply[2068] = {0};
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_NO_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM_IN_REAR_END ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }
        // IC卡已经激活但在通讯时返回指令序列错误，说明是支持T0协议的卡用T1通讯或支持T1协议的卡用T0通讯
        if (iCardReaderError == CR_ERROR_COMMAND_SEQUENCE_ERROR && m_bICCActived)
        {
            return ERR_IDC_PROTOCOLNOTSUPP;
        }

        return ERR_IDC_HWERR;
    }
    //char szReportLen[3] = {0};
    //memcpy(szReportLen,reply + 1,2);
    //DWORD nLen = GetReportLen(szReportLen);
    DWORD nLen = MAKEWORD(reply[2], reply[1]);
    nInOutLen = nLen - 5;
    memcpy(pInOutData, reply + 8, nInOutLen);
    return ERR_IDC_SUCCESS;
}

int  CDevIDC_EC2G::TransparentCardSetting(bool bEnableDetect)
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    char szCmd[20] = {0};
    char reply[2068] = {0};

    if (bEnableDetect)
    {
        memcpy(szCmd, "CSj1", 4);
    }
    else
    {
        memcpy(szCmd, "CSj0", 4);
    }

    int nRet = SendCmd(szCmd, strlen(szCmd), ThisModule);
    if (nRet != 0)
        return nRet;

    nRet = GetResponse(reply, sizeof(reply), 30000, ThisModule);
    if (nRet < 0)
        return nRet;

    if (reply[3] != 'P')
    {
        CardReaderError(reply + 6, ThisModule); //记录错误日志
        return ERR_IDC_HWERR;
    }

    Log(ThisModule, 1, "透明卡设置成功");
    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct)
{
    return 0;
}

int CDevIDC_EC2G::SetRFIDCardReaderBeep(unsigned long ulTime)
{
    return 0;
}

void CDevIDC_EC2G::GetErrorDesc(int  nErrorCode, char *pszDesc)
{
    switch (nErrorCode)
    {
    case  CR_ERROR_ERROR_COMMAND            :
        strcpy(pszDesc, "00 读卡器发送错误命令");
        break;
    case  CR_ERROR_COMMAND_SEQUENCE_ERROR   :
        strcpy(pszDesc, "01 读卡器发送命令序列出错");
        break;
    case  CR_ERROR_COMMAND_DATA_ERROR       :
        strcpy(pszDesc, "02 读卡器命令参数出错");
        break;
    case  CR_ERROR_WRITE_TRACK_ERROR        :
        strcpy(pszDesc, "03 读卡器指定磁道未被设置写数据");
        break;
    case  CR_ERROR_RECEIVE_BUFFER_OVERFLOW  :
        strcpy(pszDesc, "04 读卡器接收缓冲区满（不支持）");
        break;

    case  CR_ERROR_JAM                      :
        strcpy(pszDesc, "10 读卡器堵卡");
        break;
    case  CR_ERROR_SHUTTER_ERROR            :
        strcpy(pszDesc, "11 读卡器门异常");
        break;
    case  CR_ERROR_SENSOR_ERROR             :
        strcpy(pszDesc, "12 读卡器传感器异常（不支持）");
        break;
    case  CR_ERROR_MOTOR_ERROR              :
        strcpy(pszDesc, "13 读卡器主机异常");
        break;
    case  CR_ERROR_CARD_DRAWNOUT            :
        strcpy(pszDesc, "14 卡处理时被拖出");
        break;
    case  CR_ERROR_CARD_JAM_IN_RETRIEVING   :
        strcpy(pszDesc, "15 读卡器回收时堵卡");
        break;
    case  CR_ERROR_JAM_IN_REAR_END          :
        strcpy(pszDesc, "16 卡堵在尾端与接触部位的连接处");
        break;
    case  CR_ERROR_BPI_ENCODER_ERROR        :
        strcpy(pszDesc, "17 读卡器 75bpi-encoder 异常（不支持）");
        break;
    case  CR_ERROR_POWER_DOWN               :
        strcpy(pszDesc, "18 读卡器断电");
        break;
    case  CR_ERROR_NEED_RESET               :
        strcpy(pszDesc, "19 读卡器需要初始化");
        break;
    case  CR_ERROR_ACT_ERROR                :
        strcpy(pszDesc, "1E 读卡器ACT错误");
        break;
    case  CR_ERROR_ACT_ERROR2               :
        strcpy(pszDesc, "1F 读卡器ACT错误");
        break;
    case  CR_ERROR_ACT_ERROR3               :
        strcpy(pszDesc, "1G 读卡器ACT错误（ACT echo is detected）");
        break;
    case  CR_ERROR_ACT_ERROR4               :
        strcpy(pszDesc, "1H 读卡器需要初始化（HOST WDT超时）");
        break;

    case  CR_ERROR_LONG_CARD                :
        strcpy(pszDesc, "20 读卡器检测到卡太长，门不能被关闭");
        break;
    case  CR_ERROR_SHORT_CARD               :
        strcpy(pszDesc, "21 读卡器检测到卡太短");
        break;

    case  CR_ERROR_POSITION_CHANGE          :
        strcpy(pszDesc, "32 读卡器检测到卡位置改变");
        break;
    case  CR_ERROR_EEPROM_ERROR             :
        strcpy(pszDesc, "33 读卡器闪存数据错误");
        break;
    case  CR_ERROR_NO_STRIPE_ERROR          :
        strcpy(pszDesc, "34 读卡器没有检测到磁条卡");
        break;

    case  CR_ERROR_READ_ERROR_IN_SS         :
        strcpy(pszDesc, "40 读卡器读错误（SS错误）");
        break;
    case  CR_ERROR_READ_ERROR_IN_ES         :
        strcpy(pszDesc, "41 读卡器读错误（ES错误）");
        break;
    case  CR_ERROR_READ_ERROR_IN_VRC        :
        strcpy(pszDesc, "42 读卡器读错误（VRC错误）");
        break;
    case  CR_ERROR_READ_ERROR_IN_LRC        :
        strcpy(pszDesc, "43 读卡器读错误（LRC错误）");
        break;
    case  CR_ERROR_NODETECT_MAGNETIC_STRIPE :
        strcpy(pszDesc, "44 读卡器读错误（无编码）");
        break;
    case  CR_ERROR_READ_ERROR_IN_SS_ES_LRC  :
        strcpy(pszDesc, "45 读卡器读错误（无数据）");
        break;
    case  CR_ERROR_READ_JITTER_ERROR        :
        strcpy(pszDesc, "46 读卡器读错误（抖动错误）");
        break;
    case  CR_ERROR_TRACK_READING_ERROR      :
        strcpy(pszDesc, "49 读卡器读磁道设置错误（指定的磁道未被读取）");
        break;

    case  CR_ERROR_WRITE_ERROR_IN_SS        :
        strcpy(pszDesc, "50 读卡器写错误（SS错误）");
        break;
    case  CR_ERROR_WRITE_ERROR_IN_ES        :
        strcpy(pszDesc, "51 读卡器写错误（ES错误）");
        break;
    case  CR_ERROR_WRITE_ERROR_IN_VRC       :
        strcpy(pszDesc, "52 读卡器写错误（VRC错误）");
        break;
    case  CR_ERROR_WRITE_ERROR_IN_LRC       :
        strcpy(pszDesc, "53 读卡器写错误（LRC错误）");
        break;
    case  CR_ERROR_WRITE_ERROR_NO_ENCODE    :
        strcpy(pszDesc, "54 读卡器写错误（无编码）");
        break;
    case  CR_ERROR_WRITE_ERROR_VERIFICATION :
        strcpy(pszDesc, "55 读卡器写错误（数据不一致）");
        break;
    case  CR_ERROR_WRITE_ERROR_IN_JITTER    :
        strcpy(pszDesc, "56 读卡器写错误（抖动错误）");
        break;
    case  CR_ERROR_WRITE_ERROR_MAGNETIC_LEVEL :
        strcpy(pszDesc, "5B 读卡器写错误（磁等级错误）");
        break;

    case  CR_ERROR_CARD_TAKEOUT_RETRIEVEDED :
        strcpy(pszDesc, "60 读卡器超时错误（重进卡无法完全执行，卡可能被拖走）");
        break;
    case  CR_ERRORINSERT_TIMEOUT            :
        strcpy(pszDesc, "61 读卡器超时错误（等待插入时间溢出）");
        break;
    case  CR_ERROR_TAKEOUT_TIMEOUT          :
        strcpy(pszDesc, "62 读卡器超时错误（等待取卡时间溢出）");
        break;
    case  CR_ERROR_RETRIEVE_TIMEOUT         :
        strcpy(pszDesc, "63 读卡器超时错误（回收卡时间溢出）");
        break;
    case  CR_ERROR_CARD_HELD_BY_FORCE       :
        strcpy(pszDesc, "64 读卡器超时错误（初始化时检测到门口有卡）");
        break;

    case  CR_ERROR_FW_ERROR                 :
        strcpy(pszDesc, "70 读卡器固件错误（不完整的程序）");
        break;
    case  CR_ERROR_FW_ERROR2                :
        strcpy(pszDesc, "71 读卡器固件错误（在下载完成后复位初始化等待中，即未收到初始化命令）");
        break;

    case  CR_ERROR_ICC_CONTROL_BOARD_ERROR  :
        strcpy(pszDesc, "80 读卡器ICC错误（IC卡无法接收）");
        break;
    case  CR_ERROR_ICC_SOLENOID_ERROR       :
        strcpy(pszDesc, "81 读卡器ICC错误（IC卡螺线管错误）");
        break;
    case  CR_ERROR_ICC_ACTIVE_ERROR         :
        strcpy(pszDesc, "82 读卡器ICC错误（IC卡激活错误）");
        break;
    case  CR_ERROR_ICC_DEACTIVE_ERROR       :
        strcpy(pszDesc, "83 读卡器ICC错误（IC卡反激活错误）（不支持）");
        break;
    case  CR_ERROR_ICC_COMMUNICATION_ERROR  :
        strcpy(pszDesc, "84 读卡器ICC错误（IC卡通信错误）");
        break;
    case  CR_ERROR_ICC_FORCED_COMMAND       :
        strcpy(pszDesc, "85 读卡器ICC错误（IC卡接收时被强制中断）");
        break;
    case  CR_ERROR_ICC_RECEIVE_DATA_ERROR   :
        strcpy(pszDesc, "86 读卡器ICC错误（IC卡接收数据错误）");
        break;
    case  CR_ERROR_ICC_ATR_DATA_ERROR       :
        strcpy(pszDesc, "87 读卡器ICC错误（IC卡不支持，ATR数据不支持");
        break;
    case  CR_ERROR_ICC_CARD_MOVEMENT        :
        strcpy(pszDesc, "88 读卡器ICC错误（IC卡在压下IC触点时被移动）");
        break;
    case  CR_ERROR_ICC_DIS_OF_VER_CODE      :
        strcpy(pszDesc, "89 读卡器ICC错误（IC卡验证码不一致）");
        break;
    case  CR_ERROR_ICC_INAP_VER_CARD        :
        strcpy(pszDesc, "8A 读卡器ICC错误（IC卡不适合验证卡）");
        break;

    case  CR_ERROR_SAM_ERROR                :
        strcpy(pszDesc, "A0 读卡器SAM错误（SAM卡无法接收）");
        break;
    case  CR_ERROR_SAM_ERROR2               :
        strcpy(pszDesc, "A2 读卡器SAM错误（SAM卡激活错误）");
        break;
    case  CR_ERROR_SAM_ERROR3               :
        strcpy(pszDesc, "A4 读卡器SAM错误（SAM卡通信错误）");
        break;
    case  CR_ERROR_SAM_ERROR4               :
        strcpy(pszDesc, "A5 读卡器SAM错误（SAM卡接收时被强制中断）");
        break;
    case  CR_ERROR_SAM_ERROR5               :
        strcpy(pszDesc, "A6 读卡器SAM错误（SAM卡接收数据错误）");
        break;
    case  CR_ERROR_SAM_ERROR6               :
        strcpy(pszDesc, "A7 读卡器SAM错误（SAM卡不支持，ATR数据不支持）");
        break;
    case  CR_ERROR_SAM_ERROR7               :
        strcpy(pszDesc, "A9 读卡器SAM错误（SAM卡芯片被拔出）");
        break;

    case  CR_ERROR_EASD_ERROR               :
        strcpy(pszDesc, "K0 读卡器E-ASD错误（E-ASD功能被损坏）");
        break;

    default:
        sprintf(pszDesc, "未找到(%d)对应的错误描述", nErrorCode);
        break;
    }
}

int CDevIDC_EC2G::CardReaderError(const char *pCode, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    const char *ErrorCode[] = {"00", "01", "02", "03", "04",
                               "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1E", "1F", "1G", "1H",
                               "20", "21",
                               "32", "33", "34",
                               "40", "41", "42", "43", "44", "45", "46", "49",
                               "50", "51", "52", "53", "54", "55", "56", "5B",
                               "60", "61", "62", "63", "64",
                               "70", "71",
                               "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A",
                               "A0", "A2", "A4", "A5", "A6", "A7", "A9",
                               "K0"
                              };
    for (size_t i = 0; i < sizeof(ErrorCode) / sizeof(ErrorCode[0]); i++)
    {
        if (strncmp(ErrorCode[i], pCode, 2) == 0)
        {
            int nRet = (int)(CR_ERROR_ERROR_COMMAND - i);
            if (nRet != m_nLastError)
            {
                char szErrorDesc[255] = {0};
                GetErrorDesc(nRet, szErrorDesc);
                Log(ThisModule, __LINE__, "%s:%s", pszCallFunc, szErrorDesc);
            }
            return nRet;
        }

    }
    return ERR_IDC_SUCCESS;
}

int CDevIDC_EC2G::SendCmd(const char *pszCmd, int nLen, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null");
        return ERR_DEVPORT_NOTOPEN;
    }

    char buf[2068] = {0};
    //发送的数据格式
    //ReportID(1 byte) + Len(2 byte表示命令长度) + 命令(IDN + CMD + Parameter) + PAD(数据填充)
    int nSendLen = 0;
    DWORD dwReportID = GetReportLenFromLength(nLen + 3, nSendLen);
    //ReportID
    buf[0] = dwReportID;
    //Len
    buf[1] = (char)(nLen / 256);
    buf[2] = (char)(nLen % 256);
    //命令
    memcpy(buf + 3, pszCmd, nLen);

    int nRet = 0;
    for (int iIndex = 0; iIndex < 4; iIndex++)
    {

        nRet = m_pDev->Send(buf, nSendLen, TIMEOUT_ACTION);
        if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
        {
            if (nRet == ERR_DEVPORT_WRITE)
                Log(ThisModule, __LINE__, "%s: 读卡器没有响应", pszCallFunc);
            else
                Log(ThisModule, __LINE__, "%s: SendData()出错，返回%d", pszCallFunc, nRet);
        }
        m_nLastError = nRet;
        if (nRet < 0)
        {
            if (nRet == ERR_DEVPORT_NOTOPEN)
            {
                Close();
                int nRetTemp = Open("");
                if (nRetTemp == ERR_DEVPORT_SUCCESS && iIndex < 3)
                {
                    continue;
                }
            }
            return ConvertErrorCode(nRet);
        }
        else
            break;
    }
    return ConvertErrorCode(nRet);
}

/*功能：读取读卡器的返回数据
  参数：pszReponse返回数据的缓冲区，nLen缓冲区长度，nTimeout超时(毫秒)，pWaitCancel是IWaitCancel指针
  返回：>0数据长度，<0错误，不会返回0
*/
int CDevIDC_EC2G::GetResponse(char *pszResponse, int nLen, int nTimeout, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);

    DWORD ulInOutLen = nLen;
    int nRet = m_pDev->Read(pszResponse, ulInOutLen, nTimeout);
    if (nRet < 0)
    {
        return ConvertErrorCode(nRet);
    }

    //char szReportLen[3] = {0};
    //memcpy(szReportLen,pszResponse+1,2);
    //DWORD nRecvLen = GetReportLen(szReportLen);
    DWORD nRecvLen = MAKEWORD(pszResponse[2], pszResponse[1]);
    if (nRecvLen != 0)
        return nRecvLen + 3;
    else
    {
        Log(ThisModule, __LINE__, "%s: 读卡器返回数据错误", pszCallFunc);
        return ERR_IDC_READERROR;
    }
}

//////////
DWORD CDevIDC_EC2G::GetReportLenFromLength(int nInLen, int &nOutSendLen)
{
    if (nInLen <= 11)
    {
        nOutSendLen = 11;
        return 0x02;
    }
    else if (nInLen > 11 && nInLen <= 19)
    {
        nOutSendLen = 19;
        return 0x03;
    }
    else if (nInLen > 19 && nInLen <= 35)
    {
        nOutSendLen = 35;
        return 0x04;
    }
    else if (nInLen > 35 && nInLen <= 75)
    {
        nOutSendLen = 75;
        return 0x05;
    }
    else if (nInLen > 75 && nInLen <= 139)
    {
        nOutSendLen = 139;
        return 0x06;
    }
    else if (nInLen > 139 && nInLen <= 267)
    {
        nOutSendLen = 267;
        return 0x07;
    }
    else if (nInLen > 267 && nInLen <= 371)
    {
        nOutSendLen = 371;
        return 0x08;
    }
    else
    {
        nOutSendLen = 2068;
        return 0x09;
    }
    return 0;
}

int CDevIDC_EC2G::ConvertErrorCode(long iRet)
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
