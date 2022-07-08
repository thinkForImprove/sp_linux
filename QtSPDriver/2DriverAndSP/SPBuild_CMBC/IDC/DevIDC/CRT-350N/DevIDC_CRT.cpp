#include "DevIDC_CRT.h"

static const char *ThisFile = "DevIDC_CRT.cpp";
BOOL g_bExitSkimmingThread = FALSE;         //30-00-00-00(FS#0014)
//extern "C" Q_DECL_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev)
//{
//    pDev = new CDevIDC_CRT(lpDevType);
//    return (pDev != nullptr) ? 0 : -1;
//}

//30-00-00-00(FS#0014)
void *SkimmingCheckThread(void *pDev)
{
    CDevIDC_CRT *pCRTDev = static_cast<CDevIDC_CRT *>(pDev);
    if(pCRTDev == nullptr){
        return nullptr;
    }

    ULONG ulLastCheckTime = 0;           //上次检测时间
    ULONG ulErrorStartTime = 0;          //故障发生起始时间
    ULONG ulNormalStartTime = 0;         //故障恢复起始时间
    CUSBDrive &cUSBDrive = pCRTDev->cUSBDrive();
    BYTE	byDataInLight[44] = {0};     //Sensor发光命令入参
    byDataInLight[1] = 0x2C;		// Size [0][1]
    byDataInLight[3] = 0x01;		// Cntl ID [2][3]
    byDataInLight[5] = 0x04;		// Cntl Len [4][5]
    byDataInLight[6] = 0xA3;		// Cntl CNTL [6][7]
    byDataInLight[7] = 0x01;		//10ms单位指定  //IOMC改善后，修改为10ms单位	//40-01-00-05(UT#00005)
    byDataInLight[8] = 0x07;		// Output Data ID[8][9]
    byDataInLight[9] = 0x01;		// Output Data ID[8][9]
    byDataInLight[11] = 0x22;       // Output Data LNG [10][11]
    byDataInLight[28] = 0x01;       // Port5  Data   出力开始
    byDataInLight[29] = 0x03;       // Port5  Data 出力Port,ON时间,单位:10ms (00时:循环的次数无视) 30ms指定 //40-01-00-05(UT#00005)
    byDataInLight[30] = 0x00;       // Port5  Data 出力Port,OFF时间
    byDataInLight[31] = 0x01;       // 循环次数(ON,OFF循环次数)
    BYTE bySenseGetInput[8] = {0};  // 获取Sensor信息命令入参
    bySenseGetInput[0] = 0x00;										// LEN　2バイト
    bySenseGetInput[1] = 0x08;										//
    bySenseGetInput[2] = 0x00;										// ID　2バイト
    bySenseGetInput[3] = 0x01;										//
    bySenseGetInput[4] = 0x00;										// LNG　2バイト
    bySenseGetInput[5] = 0x04;										//
    bySenseGetInput[6] = 0x0C;										// CMD　2バイト
    bySenseGetInput[7] = 0x01;										//
    STR_DRV      stDrvParam;
    IOMCRESPINFO stIomcRespInfoLight ;	//Sensor发光应答结构体
    IOMCSNSINFO	 stIomcSnsInfo ;        //Sensor信息应答结构体
    long         lRet = 0;
    while(!g_bExitSkimmingThread){
        ULONG ulCurrentTime = CQtTime::GetSysTick();
        if((ulLastCheckTime == 0) ||
           (ulCurrentTime - ulLastCheckTime > pCRTDev->stIDC().dwSkimmingMonitorInterval * 1000)){
            ulLastCheckTime = ulCurrentTime;
            //发送SIU发光命令
            memset(&stDrvParam, 0, sizeof(stDrvParam));
            memset(&stIomcRespInfoLight, 0, sizeof(stIomcRespInfoLight));
            stDrvParam.usParam = USB_PRM_USUALLY;
            stDrvParam.pvDataInBuffPtr = byDataInLight;
            stDrvParam.uiDataInBuffSz = sizeof(byDataInLight);
            stDrvParam.pvDataOutBuffPtr = &stIomcRespInfoLight;
            stDrvParam.uiDataOutReqSz = sizeof(stIomcRespInfoLight);
            stDrvParam.uiTimer = 30;
            lRet = cUSBDrive.USBDrvCall(USB_DRV_FN_DATASENDRCV, &stDrvParam);
            if(lRet != 0){
                pCRTDev->Log("SkimmingCheckThread", __LINE__, "Sensor发光命令失败:%d", lRet);
            } else {
                CQtTime::Sleep(35);
                //获取Sensor状态
                memset(&stIomcSnsInfo, 0, sizeof(stIomcSnsInfo));
                memset(&stDrvParam, 0, sizeof(stDrvParam));
                stDrvParam.usParam = USB_PRM_USUALLY;
                stDrvParam.pvDataInBuffPtr = bySenseGetInput;
                stDrvParam.uiDataInBuffSz = sizeof(bySenseGetInput);
                stDrvParam.pvDataOutBuffPtr = &stIomcSnsInfo;
                stDrvParam.uiDataOutReqSz = sizeof(stIomcSnsInfo);
                stDrvParam.uiTimer = 30;
                lRet = cUSBDrive.USBDrvCall(USB_DRV_FN_DATASENDRCV, &stDrvParam);
                if(lRet != 0){
                    pCRTDev->Log("SkimmingCheckThread", __LINE__, "Sensor状态命令失败:%d", lRet);
                } else {
                    pCRTDev->LogHex("idc_iomc_skimming.log", (LPSTR)stIomcSnsInfo.byAppendInfo, sizeof(stIomcSnsInfo.byAppendInfo));
                    //判断异物状态
                    if((stIomcSnsInfo.byAppendInfo[4 + 4] & 0x10) == 0){
                        //检测到异常发生
                        if(ulErrorStartTime == 0){
                            ulErrorStartTime = CQtTime::GetSysTick();
                        } else {
                            if(CQtTime::GetSysTick() - ulErrorStartTime >
                               (ULONG)pCRTDev->stIDC().dwSkimmingErrorDuration * 1000){
                                //达到持续异常时间，设置为异常状态
                                pCRTDev->setBSkimmingError(TRUE);
                            } else {

                            }
                        }
                        //重置正常起始时间
                        if(ulNormalStartTime != 0){
                            ulNormalStartTime = 0;
                        }
                    } else {
                        if(pCRTDev->bSkimmingError()){
                            if(ulNormalStartTime == 0){
                                ulNormalStartTime = CQtTime::GetSysTick();
                            } else {
                                if(CQtTime::GetSysTick() - ulNormalStartTime >
                                   (ULONG)pCRTDev->stIDC().dwSkimmingNormalDuration * 1000){
                                    pCRTDev->setBSkimmingError(FALSE);
                                }
                            }
                        } else {

                        }
                        //重置异常起始时间
                        if(ulErrorStartTime != 0){
                            ulErrorStartTime = 0;
                        }
                    }
                }
            }
        }

        //防止cpu占用
        CQtTime::Sleep(10);
    }

    return nullptr;
}

CDevIDC_CRT::CDevIDC_CRT(LPCSTR lpDevType)
    : m_nCardStatus(IDCSTATUS_UNKNOWN),
      m_bFraudDetected(FALSE),
      m_bTamperSensorStatus(TANMPER_SENSOR_NOT_AVAILABLE),
      m_dwLastFraudDetectedTime(0),
      m_bICCActived(FALSE),
      m_bICCRelease(TRUE),
      m_bICCJam(FALSE),
      m_bOpen(FALSE),           //30-00-00-00(FT#0019)
      m_cUSBDrive("IDC_IOMC")   //30-00-00-00(FS#0014)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    m_bSkimmingError = FALSE;    //30-00-00-00(FS#0014)
    m_skimmingTid = 0;           //30-00-00-00(FS#0014)
}

CDevIDC_CRT::~CDevIDC_CRT()
{
    Release();
}

void CDevIDC_CRT::Release()
{
    Close();
    if(m_pDev != nullptr)
        m_pDev.Release();
}

INT CDevIDC_CRT::InitConfig()
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

LONG CDevIDC_CRT::ReadConfig()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    AutoLogFuncBeginEnd();

    m_initParam.clear();                          //30-00-00-00(FT#0030)
    m_initParam.append("324000012");              //30-00-00-00(FT#0030)

    QByteArray strFile("/IDCConfig.ini");
#ifdef QT_WIN32
    strFile.prepend(SPETCPATH);
#else
    strFile.prepend(SPETCPATH);
#endif
    if (!m_cINI.LoadINIFile(strFile.constData()))
    {
        Log(ThisModule, __LINE__, "CRT:加载配置文件失败：%s", strFile.constData());
        return -1;
    }

    m_stIDC.Clear();
    CINIReader cIR = m_cINI.GetReaderSection("IDCInfo");
    m_stIDC.bWaitInsertCardIntervalTime = (DWORD)cIR.GetValue("WaitInsertCardIntervalTime", "3");
    m_stIDC.bSupportPredictIC = (USHORT)cIR.GetValue("SupportPredictIC", "0");
    m_stIDC.bReTakeIn = (USHORT)cIR.GetValue("ReTakeIn", "1");
    m_stIDC.bFraudEnterCardTimes = (DWORD)cIR.GetValue("FraudEnterCardTimes", "5");
    m_stIDC.bNeedFraudProtect = (BOOL)cIR.GetValue("NeedFraudProtect", "0");
    m_stIDC.bFraudProtectTime = (INT)cIR.GetValue("FraudProtectTime", "4");
    m_stIDC.bTamperSensorSupport = (DWORD)cIR.GetValue("TamperSensorSupport", "0");
    m_stIDC.iESUMode = (int)cIR.GetValue("ESUMode", 1);     //30-00-00-00(FT#0064)
    if(m_stIDC.iESUMode < 0 || m_stIDC.iESUMode > 1){
        m_stIDC.iESUMode = 1;
    }
	
	cIR = m_cINI.GetReaderSection("IDC");           //30-00-00-00(FT#0030)
    m_stIDC.iPowerOffAction = (int)cIR.GetValue("PowerOffAction", 2);           //30-00-00-00(FT#0030)
    switch(m_stIDC.iPowerOffAction){                //30-00-00-00(FT#0030)
    case 1:                                         //30-00-00-00(FT#0030)
        m_initParam.data()[5] = '2';                //30-00-00-00(FT#0030)
        break;                                      //30-00-00-00(FT#0030)
    case 2:                                         //30-00-00-00(FT#0030)
        m_initParam.data()[5] = '0';                //30-00-00-00(FT#0030)
        break;                                      //30-00-00-00(FT#0030)
    case 3:                                         //30-00-00-00(FT#0030)
        m_initParam.data()[5] = '5';                //30-00-00-00(FT#0030)
        break;                                      //30-00-00-00(FT#0030)
    case 4:                                         //30-00-00-00(FT#0030)
        m_initParam.data()[5] = '3';                //30-00-00-00(FT#0030)
        break;                                      //30-00-00-00(FT#0030)
    default:                                        //30-00-00-00(FT#0030)
        break;                                      //30-00-00-00(FT#0030)
    }                                               //30-00-00-00(FT#0030)

    cIR = m_cINI.GetReaderSection("SKIMMING");      //30-00-00-00(FS#0014)
    m_stIDC.bSkimmingSupport = (((int)cIR.GetValue("SkimmingSupp", 0)) == 1);               //30-00-00-00(FS#0014)
    m_stIDC.dwSkimmingMonitorInterval = (UINT)cIR.GetValue("SkimmingMonitorInterval", 3);   //30-00-00-00(FS#0014)
    m_stIDC.dwSkimmingErrorDuration = (UINT)cIR.GetValue("SkimmingErrorDuration", 60);      //30-00-00-00(FS#0014)
    m_stIDC.dwSkimmingNormalDuration = (UINT)cIR.GetValue("SkimmingNormalDuration", 60);    //30-00-00-00(FS#0014)

    return 0;
}

INT CDevIDC_CRT::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    if (m_pDev == nullptr)
    {
        if (0 != m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "IDC", "DevIDC_CRT"))
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
            sprintf(szDevID, "USB:%d,%d", 0x1D6B, 0x0002); //pid :0002 vid :1d6b
            m_strMode = szDevID;
        }
    }
    else
    {
        m_strMode = pMode;
    }

    //读取sp配置文件
    InitConfig();

    long lRes = OpenDevice();
    if(lRes != 0)
        return lRes;

    //30-00-00-00(FS#0014) add start
    if(m_stIDC.bSkimmingSupport){
        //打开IOMC设备
        if(m_cUSBDrive.USBDllLoad() == 0){
            if(m_cUSBDrive.USBOpen("IOMC") == 0){
                //启动异物检测线程
                if(pthread_create(&m_skimmingTid, NULL, SkimmingCheckThread, this) != 0) {
                    Log(ThisModule, __LINE__, "异物检测线程启动失败");
                }
            }
        }
    }
    //30-00-00-00(FS#0014) add end

    m_bOpen = TRUE;                         //30-00-00-00(FT#0019)
    return 0;
}

LONG CDevIDC_CRT::OpenDevice()
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

INT CDevIDC_CRT::Init(CardAction eActFlag, WobbleAction nNeedWobble)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    //VERTIFYISOPEN();
    LOGDEVACTION();
    int nRet = 0;
    BOOL m_bSendflag = TRUE;

//    if (InFraudProtect())
//    {
//        Log(ThisModule, __LINE__, "防逗卡保护中，不做复位。");
//        return ERR_IDC_HWERR;
//    }

    // 驱动内部判断是否退卡成功与卡被取走，用于重进卡判断
    m_nEjectedCard = EJECT_UNKNOWN;
    m_nTakeCard = MEDIA_UNKOWN;

    Log(ThisModule, 1, "调用Init");

    char szReply[2068] = {0};
    //防盗钩已经下压,需要通过防盗钩释放指令清除错误状态才能继续初始化
    if (m_bTamperSensorStatus == TANMPER_SENSOR_ON)
    {
        Log(ThisModule, __LINE__, "防盗钩已下压,清除错误再重新初始化");
        nRet = SendCmd("Cc:", "1", 1, ThisModule);
        if (nRet < 0)
        {
            Log(ThisModule, __LINE__, "SendCmd()失败");
            return nRet;
        }

        nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_WAIT_ACTION, ThisModule);
        if (nRet < 0)
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }
        if (szReply[3] != 'P')
        {
            int iCardReaderError = CardReaderError(szReply + 6, ThisModule); //记录错误日志
            iCardReaderError = HWErrToStdErr(iCardReaderError);
            return iCardReaderError;
        }

        Log(ThisModule, __LINE__, "清除错误已完成");
        return CR_ERROR_TANMPER_BEEN_SET;   //防盗钩已经设置，命令不能执行
    }

    if (eActFlag == CARDACTION_RETRACT) // 3 吞卡
    {
        nRet = UpdateCardStatus();
        if(nRet == ERR_IDC_COMM_ERR)
        {
            Log(ThisModule, __LINE__, "Not Open Dev, try open 1");
            Close();
            OpenDevice();
        }

        if (m_nCardStatus == IDCSTAUTS_NOCARD)
        {
            nRet = SendCmd("C07", (LPCSTR)m_initParam.constData(), m_initParam.size(), ThisModule);      //30-00-00-00(FT#0030)
            m_bSendflag = FALSE;
        }
        else {
            if (EatCard() < 0)
            {
                return ERR_IDC_HWERR;
            }
            m_bSendflag = TRUE;
        }
    }
    else if (eActFlag == CARDACTION_NOACTION) // 1 移动并保持
    {
        nRet = SendCmd("C06", (LPCSTR)m_initParam.constData(), m_initParam.size(), ThisModule);          //30-00-00-00(FT#0030)
        //nRet = SendCmd("C02", nullptr, 0, ThisModule);
        m_bSendflag = FALSE;
    }
    else if (eActFlag == CARDACTION_NOMOVEMENT) // 4 不移动并保持
    {
        nRet = SendCmd("C07", (LPCSTR)m_initParam.constData(), m_initParam.size(), ThisModule);      //30-00-00-00(FT#0030)
        m_bSendflag = FALSE;
    }
    else if ((eActFlag == CARDACTION_EJECT))// 2 弹卡
    {
        nRet = UpdateCardStatus();
        if(nRet == ERR_IDC_COMM_ERR)
        {
            Log(ThisModule, __LINE__, "Not Open Dev, try open 1");
            Close();
            OpenDevice();
        }

        if (m_nCardStatus == IDCSTAUTS_NOCARD)
        {
            nRet = SendCmd("C07", (LPCSTR)m_initParam.constData(), m_initParam.size(), ThisModule);    //30-00-00-00(FT#0030)
            m_bSendflag = FALSE;
        }
        else
        {
            if (EjectCard() < 0)
            {
                return ERR_IDC_HWERR;
            }

            m_bSendflag = TRUE;
        }
    }

    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }

    //没有运行SendCmd, 直接运行GetResponse导致运行时间过长。
    if (!m_bSendflag)
    {
        nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_WAIT_ACTION, ThisModule);
        if (nRet < 0 && (eActFlag != CARDACTION_NOACTION))
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }

        if (szReply[3] != 'P')
        {
            //DLE EOT(10H,04H)
            if (szReply[3] == 0x10 && szReply[4] == 0x04)
            {
                if (m_nCardStatus == IDCSTAUTS_ENTERING)
                {
                    m_nEjectedCard = EJECT_SUCCESS;
                    eActFlag == CARDACTION_EJECT;
                }
                return CR_ERROR_COMMAND_UNABLE_EXEC;
            }

            int iCardReaderError = CardReaderError(szReply + 6, ThisModule); //记录错误日志
            if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
                iCardReaderError == CR_ERROR_JAM)
            {
                return ERR_IDC_JAMMED;
            }

            if (eActFlag != CARDACTION_NOACTION)
                return ERR_IDC_HWERR;

        }
    }


    if (eActFlag == CARDACTION_EJECT)
    {
        m_nEjectedCard = EJECT_SUCCESS;
        m_nTakeCard = MEDIA_NOTAKEN;
    }


    //设置磁干扰模式(该处理必须放在初始化命令之后)
    SetESUMode(m_stIDC.iESUMode);                   //30-00-00-00(FT#0064)

    // add start 30-00-00-00(FT#0065)
    if (nNeedWobble != WOBBLEACTION_NOACTION)
    {
        nRet = SetWobble(nNeedWobble);
        if (nRet != 0)
        {
            Log(ThisModule, 1, "设置抖动功能失败, ret=%d, NeedWobble=%d", nRet, nNeedWobble);
        }
    }
    // add end 30-00-00-00(FT#0064)

    if (m_stIDC.bReTakeIn == 0)
        m_bReTakeIn = FALSE;
    else
    {
        m_bReTakeIn = TRUE;
        //设置重进卡命令超时时间，设备自身默认10毫秒
        //nRet = SetTimeOutMonitor(MONITOR_REINTAKE, 30);
        //if (nRet != 0)
        //   return nRet;
    }

    return ERR_IDC_SUCCESS;
}

void CDevIDC_CRT::Close()
{
    THISMODULE(__FUNCTION__);
    if (m_pDev == nullptr)
        return;

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return;                         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    LOGDEVACTION();
    m_pDev->Close();
    m_bOpen = FALSE;                    //30-00-00-00(FT#0019)

    //30-00-00-00(FS#0014) add start
    if(m_stIDC.bSkimmingSupport){
        if(m_skimmingTid != 0){
            g_bExitSkimmingThread = TRUE;
            pthread_join(m_skimmingTid, NULL);
            m_skimmingTid = 0;
        }
        m_cUSBDrive.USBDllRelease();
    }
    //30-00-00-00(FS#0014) add end
}

INT CDevIDC_CRT::EatCard()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    IDC_IDCSTAUTS IDCstatus;
    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，吞卡取消。");
        return ERR_IDC_HWERR;
    }

    m_nEjectedCard = EJECT_UNKNOWN;
    m_nTakeCard = MEDIA_UNKOWN;

    Log(ThisModule, 1, "调用EatCard");

    int nRet = DetectCard(IDCstatus);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "DetectCard()失败");
        return nRet;
    }
    if (IDCstatus == IDCSTAUTS_NOCARD)
    {
        return CR_ERROR_BEFORE_RETRACT_PULL;    //吞卡时卡拔走
    }

    if (!m_bICCRelease)
    {
        nRet = ICCRelease();
        if (nRet < 0){
            return HWErrToStdErr(nRet);
        }
    }

    char reply[2068] = {0};
    nRet = SendCmd("C05", (LPCSTR)m_initParam.constData(), m_initParam.size(), ThisModule);      //30-00-00-00(FT#0030)
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }

    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
        return HWErrToStdErr(nRet);
    }
    return 0;
}

INT CDevIDC_CRT::EjectCard()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

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
    int nIndex = 0;
    nRet = UpdateCardStatus();
    if (nRet < 0) // 处理卡的错误状态
    {
        return ERR_IDC_HWERR;
    }

    if (!m_bICCRelease)
    {
        nRet = ICCRelease();
        if (nRet < 0){
            return HWErrToStdErr(nRet);
        }
    }

    nRet = SendCmd("C04", (LPCSTR)m_initParam.constData(), m_initParam.size(), ThisModule);      //30-00-00-00(FT#0030)
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        CQtTime::Sleep(500);
        while (reply[3] == 0x00 || nIndex < 100)
        {
            CQtTime::Sleep(100);
            nRet = GetResponse(reply, sizeof(reply), 0, ThisModule);
            if ((reply[6] == 0x31 && reply[7] == 0x30))
                break;
            nIndex++;
        }

        if (nRet < 0)
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }
    }

    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (nRet == CR_ERROR_LONG_CARD ||                                               //30-00-00-00(FT#0043)
            nRet == CR_ERROR_SHORT_CARD)                                                //30-00-00-00(FT#0043)
        {                                                                               //30-00-00-00(FT#0043)
            //异常时使用标准排卡命令C30
            nRet = SendCmd("C30", nullptr, 0, ThisModule);                              //30-00-00-00(FT#0043)
            if (nRet != 0) {                                                            //30-00-00-00(FT#0043)
                Log(ThisModule, __LINE__, "SendCmd()失败");                              //30-00-00-00(FT#0043)
                return nRet;                                                            //30-00-00-00(FT#0043)
            }                                                                           //30-00-00-00(FT#0043)
                                                                                        //30-00-00-00(FT#0043)
            nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);  //30-00-00-00(FT#0043)
            if (nRet < 0){                                                              //30-00-00-00(FT#0043)
                Log(ThisModule, __LINE__, "GetResponse()失败");                          //30-00-00-00(FT#0043)
                return nRet;                                                            //30-00-00-00(FT#0043)
            }                                                                           //30-00-00-00(FT#0043)
            if (reply[3] == 'P')                                                        //30-00-00-00(FT#0043)
                return 0;                                                               //30-00-00-00(FT#0043)
        }                                                                               //30-00-00-00(FT#0043)
                                                                                        //30-00-00-00(FT#0043)
        m_nEjectedCard = EJECT_FAILED;                                                  //30-00-00-00(FT#0043)
        m_nTakeCard = MEDIA_UNKOWN;                                                     //30-00-00-00(FT#0043)
        return HWErrToStdErr(nRet);
    }

    m_nEjectedCard = EJECT_SUCCESS;
    m_nTakeCard = MEDIA_NOTAKEN;
    return 0;
}

INT CDevIDC_CRT::ICCMove()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    char reply[2068] = {0};
    int nRet = 0;

    nRet = UpdateCardStatus();
//    if (nRet < 0) // 处理卡的错误状态
//    {
//        return ERR_IDC_HWERR;
//    }

    if (m_nCardStatus == IDCSTAUTS_NOCARD)
    {
        return ERR_IDC_HWERR;
    }

    nRet = SendCmd("C32", nullptr, 0, ThisModule);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }

    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
        return HWErrToStdErr(nRet);
    }

    return 0;
}

INT CDevIDC_CRT::AcceptCard(ULONG ulTimeOut, bool Magnetic)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
//    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    INT nRet = ERR_IDC_SUCCESS;

    while (TRUE)
    {
        if (!m_SemCancel.WaitForEvent(10))
        {
           break;
        }
    }

//    if (InFraudProtect())
//    {
//        Log(ThisModule, __LINE__, "防逗卡保护中，不能进卡。");
//        return ERR_IDC_HWERR;
//    }
    Log(ThisModule, 1, "调用AcceptCard");

    char  reply[2068]           = {0};
//    DWORD dwRetryTimes          = 0;                    // 检测到没成功进卡重试次数
//    DWORD dwLastFailedTime      = 0;
    ULONG dwStart               = CQtTime::GetSysTick();

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
        return ERR_IDC_TAMPER;
    }

//    if (m_stIDC.bNeedFraudProtect)
//    {

//        if ((dwRetryTimes > 0) && (CQtTime::GetSysTick() - dwLastFailedTime > (m_stIDC.bFraudProtectTime * 1000)))
//        {
//            dwRetryTimes = 0;
//        }
//    }

    nRet = UpdateCardStatus();
    if (nRet < 0) // 处理卡的错误状态
    {
        return ERR_IDC_HWERR;
    }

    if (m_nCardStatus == IDCSTAUTS_INTERNAL)
    {
        return ERR_IDC_SUCCESS;
    } else if (m_nCardStatus == IDCSTAUTS_ICC_PRESS) // IC卡处于压下状态
    {
        // 释放IC卡后返回进卡成功。
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
        if(m_nCardStatus == IDCSTATUS_UNKNOWN){
            Log(ThisModule, __LINE__, "卡状态异常,CardStatus：%d", m_nCardStatus);
            return ERR_IDC_HWERR;
        }
    }

    string strCMD;

//    if (m_stIDC.bNeedFraudProtect)
//    {
//        if (UpdateCardStatus() > 0)
//        {
//            dwRetryTimes++;
//            dwLastFailedTime = CQtTime::GetSysTick();
//        }
//        if (dwRetryTimes >= m_stIDC.bFraudEnterCardTimes)
//        {
//            m_bFraudDetected = TRUE;
//            m_dwLastFraudDetectedTime = CQtTime::GetSysTick();
//            Log(ThisModule, __LINE__, "进入防逗卡保护。");
//            return ERR_IDC_HWERR;
//        }
//    }

    //实现重进卡功能
    if (m_bReTakeIn && m_bCardInGatePosition && m_nTakeCard == MEDIA_NOTAKEN)
    {
        Log(ThisModule, 1, "调用重进卡命令");
        nRet = SendCmd("C40", nullptr, 0, ThisModule);   // 重新吸入卡
        if (nRet < 0)
        {
            Log(ThisModule, __LINE__, "SendCmd()失败");
            return nRet;
        }
        nRet = GetResponse(reply, sizeof(reply), m_stIDC.bWaitInsertCardIntervalTime * 1000, ThisModule);
        if(nRet >= 0){
            nRet = ERR_IDC_SUCCESS;
            if(reply[3] != 'P'){
                Log(ThisModule, __LINE__, "重进卡命令执行失败");
                nRet = CardReaderError(reply + 6, ThisModule);
                nRet = HWErrToStdErr(nRet);
            }
        }
    }
    else
    {       
        // 读卡器功能
        // 只判IC C23
        // 磁或IC C2=
        // 磁且IC C24
        // 配置文件:
        // 1:磁或IC
        // 2:磁且IC
        //异步进卡
        switch(m_stIDC.bSupportPredictIC){
        case 0:
            strCMD = "C:0";
            break;
        case 1:
            strCMD = Magnetic ? "C:=" : "C:3";
            break;
        case 2:
            strCMD = Magnetic ? "C:4" : "C:3";
            break;
        default:
            strCMD = Magnetic ? "C:=" : "C:3";
            break;
        }

        {
            LOGDEVACTION();
            //发送异步进卡命令
            nRet = SendCmd(strCMD.c_str(), nullptr, 0, ThisModule);
            if (nRet < 0)
            {
                Log(ThisModule, __LINE__, "异步进卡命令%s失败", strCMD.c_str());
                return nRet;
            }
            nRet = GetResponse(reply, sizeof(reply), m_stIDC.bWaitInsertCardIntervalTime * 1000, ThisModule);
            if(nRet < 0){
                Log(ThisModule, __LINE__, "异步进卡命令%s接收数据失败", strCMD.c_str());
                return nRet;
            }
            if(reply[3] != 'P'){
                Log(ThisModule, __LINE__, "异步进卡命令执行失败");
                nRet = CardReaderError(reply + 6, ThisModule);
                return HWErrToStdErr(nRet);
            }
        }

        //等待卡插入
        while(TRUE){
            //读卡命令超时
            if ((CQtTime::GetSysTick() - dwStart) >= ulTimeOut)
            {
                nRet =  ERR_IDC_INSERT_TIMEOUT;
                break;
            }

            // 用户取消
            if (m_SemCancel.WaitForEvent(10))
            {
                LOGDEVACTION();
                ForbiddenAcceptCard();
                Log(ThisModule, __LINE__, "检测到取消");
                UpdateCardStatus();
                if (m_nCardStatus == IDCSTAUTS_ENTERING || m_nCardStatus == IDCSTAUTS_INTERNAL)
                    EjectCard();
                return ERR_IDC_USER_CANCEL;
            }

            //检测卡状态
            nRet = GetCardStatusC12();
            if(nRet < 0){
                break;
            }            
            if(nRet == IDCSTAUTS_INTERNAL){
                nRet = ERR_IDC_SUCCESS;
                break;
            }

            CQtTime::Sleep(50);
        }

        //关闭进卡
        LOGDEVACTION();
        ForbiddenAcceptCard();
        //同步进卡
//        switch (m_stIDC.bSupportPredictIC)
//        {
//        case 1:
//            strCMD = Magnetic ? "C2=" : "C23";
//            break;
//        case 2:
//            strCMD = Magnetic ? "C24" : "C23";
//            break;
//        default:
//            strCMD = Magnetic ? "C2=" : "C23";
//            break;
//        }

//        BOOL bSendSuccess = FALSE;
//        //发送同步进卡命令
//        nRet = SendCmd(strCMD.c_str(), nullptr, 0, ThisModule);
//        if (nRet < 0)
//        {
//            Log(ThisModule, __LINE__, "SendCmd()失败");
//            return nRet;
//        }
//        //获取命令返回数据
//        while(TRUE){
//            if ((CQtTime::GetSysTick() - dwStart) >= ulTimeOut)
//            {
//                //取消同步进卡命令
//                StopCmd();
//                return ERR_IDC_INSERT_TIMEOUT;
//            }

//            // 用户取消
//            if (m_SemCancel.WaitForEvent(10))
//            {
//                //取消同步进卡命令
//                StopCmd();
//                Log(ThisModule, __LINE__, "检测到取消");
//                UpdateCardStatus();
//                if (m_nCardStatus == IDCSTAUTS_ENTERING || m_nCardStatus == IDCSTAUTS_INTERNAL)
//                    EjectCard();
//                return ERR_IDC_USER_CANCEL;
//            }

//            nRet = GetResponse(reply, sizeof(reply), m_stIDC.bWaitInsertCardIntervalTime * 1000, ThisModule);
//            if(nRet < 0){
//                continue;
//            }

//            break;
//        }

//        while(TRUE)
//        {
//            if ((CQtTime::GetSysTick() - dwStart) >= ulTimeOut)
//            {
//                return ERR_IDC_INSERT_TIMEOUT;
//            }

//            // 用户取消
//            if (m_SemCancel.WaitForEvent(10))
//            {
//                Log(ThisModule, __LINE__, "检测到取消");
//                UpdateCardStatus();
//                if (m_nCardStatus == IDCSTAUTS_ENTERING || m_nCardStatus == IDCSTAUTS_INTERNAL)
//                    EjectCard();
//                return ERR_IDC_USER_CANCEL;
//            }

//            UpdateCardStatus();
//            if (m_nCardStatus == IDCSTAUTS_ENTERING)
//            {
//                nRet = SendCmd(strCMD.c_str(), nullptr, 0, ThisModule);
//                if (nRet < 0)
//                {
//                    Log(ThisModule, __LINE__, "SendCmd()失败");
//                    return nRet;
//                }

//                CQtTime::Sleep(500);
//                nRet = GetResponse(reply, sizeof(reply), m_stIDC.bWaitInsertCardIntervalTime * 1000, ThisModule);
//                if (reply[3] == 'P')
//                {
//                    bSendSuccess = TRUE;
//                    break;
//                }
//                else if ((nRet == ERR_IDC_READERROR) || (reply[6] == '4' && reply[7] == '5'))
//                {
//                    // 進卡被取走
//                    StopCmd();
//                    return ERR_IDC_CARDPULLOUT;
//                }
//            } else
//            {
//                if (bSendSuccess)
//                    break;
//                else
//                    continue;
//            }
//        }
    }

//    if (nRet < 0 && nRet != ERR_IDC_READTIMEOUT)
//    if(nRet < 0) {
//        Log(ThisModule, __LINE__, "GetResponse()失败");
//        return nRet;
//    } else {
//        if (reply[3] != 'P')
//        {
            //DLE EOT(10H,04H)
//            if (reply[3] == 0x10 && reply[4] == 0x04)
//            {
//                if (m_nCardStatus == IDCSTAUTS_ENTERING)
//                {
//                    m_nEjectedCard = EJECT_SUCCESS;
//                }
//                return CR_ERROR_COMMAND_UNABLE_EXEC;
//            }

//            nRet = CardReaderError(reply + 6, ThisModule);
//            if (nRet == CR_ERROR_CARD_JAM_IN_RETRIEVING) //重进卡时卡堵塞
//                //nRet = ERR_IDC_READTIMEOUT;
//                nRet = ERR_IDC_INSERT_TIMEOUT;
//            else if (CR_ERROR_JAM == nRet)
//            {
//                nRet = ERR_IDC_JAMMED;
//            }
//            else if (CR_ERROR_BEFORE_RETRACT_PULL == nRet)
//            {
//                nRet = ERR_IDC_CARDPULLOUT;
//            }
//            else if (CR_ERROR_LONG_CARD == nRet)
//            {
//                nRet = ERR_IDC_CARDTOOLONG;
//            }
//            else if (CR_ERROR_SHORT_CARD == nRet)
//            {
//                nRet = ERR_IDC_CARDTOOSHORT;
//            }
//            else
//                nRet = ERR_IDC_HWERR;
//        } else {
//            nRet = ERR_IDC_SUCCESS;
//        }
//    }

    return nRet;
}

INT CDevIDC_CRT::CancelReadCard()
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    m_SemCancel.SetEvent();
    if (m_pDev != nullptr)
        m_pDev->CancelRead();
    return 0;
}

INT CDevIDC_CRT::WriteTracks(const STTRACK_INFO &stTrackInfo)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    if (InFraudProtect())
    {
        Log(ThisModule, __LINE__, "防逗卡保护中，写磁卡取消。");
        return ERR_IDC_HWERR;
    }

    Log(ThisModule, 1, "调用WriteTracks");

    for (int i = 1; i < 4; i++)
    {
        string strTrackData = stTrackInfo.TrackData[i - 1].szTrack;
        if (strTrackData.size() == 0)
            continue;
        char reply[2068] = {0};
        char szCmd[255] = {0};
        char szTrack[128] = {0};
        sprintf(szCmd, "C7%d", i);
        sprintf(szTrack, "%s", strTrackData.c_str());

        int nRet = SendCmd(szCmd, szTrack, strTrackData.size(), ThisModule);
        if (nRet != 0)
        {
           Log(ThisModule, __LINE__, "SendCmd()失败");
            return nRet;
        }
        nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
        if (nRet < 0)
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }
        if (reply[3] != 'P')
        {
            nRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
            return HWErrToStdErr(nRet);
        }

        break;
    }
    //m_Log.CancelLog();

    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::ReadTracks(STTRACK_INFO &stTrackInfo)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

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
        sprintf(szCmd, "C6%d", i + 1);
        int nRet = SendCmd(szCmd, nullptr, 0, ThisModule);
        if (nRet != 0)
        {
            Log(ThisModule, __LINE__, "SendCmd()失败");
            return nRet;
        }
        ZeroMemory(reply, sizeof(reply));
        nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
        if(nRet < 0){
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }

        if (reply[3] == 'P')
        {

            stTrackInfo.TrackData[i].bTrackOK = true;
            bAllTrackBad = false;
            DWORD nLen = MAKEWORD(reply[2], reply[1]);
            memcpy(stTrackInfo.TrackData[i].szTrack, reply + 8, nLen - 5);
            //return CR_ERROR_READ_TRACK_ERROR;
        }
        else
        {
            stTrackInfo.TrackData[i].bTrackOK = false;
            //jam判断处理
            nRet = CardReaderError(reply + 6, ThisModule);
            if(nRet == CR_ERROR_JAM){
                return ERR_IDC_JAMMED;
            }
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

INT CDevIDC_CRT::DetectCard(IDC_IDCSTAUTS &IDCstatus)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
//    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    int iRet = UpdateCardStatus();
    IDCstatus = m_nCardStatus;

    if(iRet == ERR_IDC_WRITEERROR || iRet == ERR_IDC_WRITETIMEOUT ||
       iRet == ERR_IDC_READERROR || iRet == ERR_IDC_READTIMEOUT ||
       iRet == ERR_IDC_COMM_ERR){
        return ERR_IDC_OFFLINE;
    }
    return iRet;
}

INT CDevIDC_CRT::GetFWVersion(char pFWVersion[], unsigned int &uLen)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

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
    int nRet = SendCmd("CA6", nullptr, 0, ThisModule);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }
    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule);
        return HWErrToStdErr(nRet);
    }

    ZeroMemory(pFWVersion, sizeof(uLen));
    strcpy(pFWVersion, reply + 8);
    pFWVersion[uLen - 1] = 0x00;
    uLen = strlen(pFWVersion);
    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::SetRecycleCount(LPCSTR pszCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    int nRet = 0;
    char reply[2068] = {0};

    int nCount = atoi(pszCount);
    if (nCount < 0 || nCount > 99)
        return ERR_IDC_PARAM_ERR;

    nRet = SendCmd("CC1", pszCount, 2, ThisModule);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }

    if(reply[3] != 'P'){
        nRet = CardReaderError(reply + 6, ThisModule);
        return HWErrToStdErr(nRet);
    }

    return ERR_IDC_SUCCESS;
}

/////////////////////////////////////IC Card////////////////////////////////
INT CDevIDC_CRT::ICCPress()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    Log(ThisModule, 1, "调用ICCPress");
    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    //判断触点是否已压下
    if(UpdateCardStatus() >= 0){
        if(!m_bICCRelease){
            return ERR_IDC_SUCCESS;
        }
    }

    char reply[2068] = {0};
    int nRet = SendCmd("C@0", nullptr, 0, ThisModule);   //接触触点
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        return HWErrToStdErr(iCardReaderError);
    }

    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::ICCRelease()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    Log(ThisModule, 1, "调用ICCRelease");
    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    char reply[2068] = {0};
    int nRet = SendCmd("C@2", nullptr, 0, ThisModule);  //释放触点
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
        return HWErrToStdErr(iRet);
    }

    //设置芯片为未激活状态
    m_bICCActived = FALSE;

    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::ICCActive(char pATRInfo[], unsigned int &uATRLen)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    if (nullptr == pATRInfo)
    {
        Log(ThisModule, 1, "输出参数pszATRInof为空");
        return ERR_IDC_PARAM_ERR;
    }
    Log(ThisModule, 1, "调用ICCActive");

    char reply[2068] = {0};
    int nRet = SendCmd("CI0", nullptr, 0, ThisModule);   //IC卡冷复位=激活
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        return HWErrToStdErr(iCardReaderError);
    }
    m_bICCActived = TRUE;

    DWORD nLen = MAKEWORD(reply[2], reply[1]);
    uATRLen = nLen - 5; //去掉命令长度3位+去掉RES 2位
    memcpy(pATRInfo, reply + 8, uATRLen); // 返回去掉了消息头、尾以及BCC校验数据的纯ATR信息数据
    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::ICCReset(ICCardReset eResetFlag, char pATRInfo[], unsigned int &uATRLen)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

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
        nRet = SendCmd("CI0", nullptr, 0, ThisModule);
    }
    else if (eResetFlag == ICCARDRESET_WARM)
    {
        nRet = SendCmd("CI8", nullptr, 0, ThisModule);
    }

    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return ERR_IDC_ACTIVEFAILED;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        return HWErrToStdErr(iCardReaderError);
    }

    m_bICCActived = TRUE;
    DWORD nLen = MAKEWORD(reply[2], reply[1]);
    uATRLen = nLen - 5;//去掉命令长度3位+去掉RES 2位 + 1位
    memcpy(pATRInfo, reply + 8, uATRLen); // 返回去掉了消息头(8byte)
    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::ICCDeActivation()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    char reply[2068] = {0};
    int nRet = SendCmd("CI1", nullptr, 0, ThisModule);  //关闭IC卡
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }
    if (reply[3] != 'P')
    {
        int iCardReaderError = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if (iCardReaderError == CR_ERROR_CARD_JAM_IN_RETRIEVING ||
            iCardReaderError == CR_ERROR_JAM)
        {
            return ERR_IDC_JAMMED;
        }

        return ERR_IDC_HWERR;
    }
    m_bICCActived = FALSE;
    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    if (m_bOpen == FALSE)               //30-00-00-00(FT#0019)
    {                                   //30-00-00-00(FT#0019)
        return ERR_IDC_OFFLINE;         //30-00-00-00(FT#0019)
    }                                   //30-00-00-00(FT#0019)

    if ((nullptr == pInOutData) ||
            (nInOutLen < 4) ||
            ((eProFlag != ICCARD_PROTOCOL_T0) && (eProFlag != ICCARD_PROTOCOL_T1)))
    {
        return ERR_IDC_PARAM_ERR;
    }
    Log(ThisModule, 1, "调用ICCChipIO");

    char szCmd[2068] = {0};
    //    if (ICCARD_PROTOCOL_T0 == eProFlag)
    //    {
    //        memcpy(szCmd, "CI3", 3);    //IC 卡 T=0 通讯
    //    }
    //    else if (ICCARD_PROTOCOL_T1 == eProFlag)
    //    {
    //        memcpy(szCmd, "CI4", 3);    //IC 卡 T=1 通讯
    //    }
    memcpy(szCmd, "CI9", 3);

//    int nRet = SendCmd(szCmd, pInOutData, nInOutLen, ThisModule);                //30-00-00-00(FT#0051)
//    if (nRet != 0)                                                               //30-00-00-00(FT#0051)
//    {                                                                            //30-00-00-00(FT#0051)
//        Log(ThisModule, __LINE__, "SendCmd()失败");                               //30-00-00-00(FT#0051)
//        return nRet;                                                             //30-00-00-00(FT#0051)
//    }                                                                            //30-00-00-00(FT#0051)
    char reply[2068] = {0};
//    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);    //30-00-00-00(FT#0051)
//    if (nRet < 0)                                                                 //30-00-00-00(FT#0051)
//    {                                                                             //30-00-00-00(FT#0051)
//        Log(ThisModule, __LINE__, "GetResponse()失败");                            //30-00-00-00(FT#0051)
//        return nRet;                                                              //30-00-00-00(FT#0051)
//    }                                                                             //30-00-00-00(FT#0051)
    int nRet;                                                                       //30-00-00-00(FT#0051)
    for(int i = 0; i < 3; i++){                                                     //30-00-00-00(FT#0051)
        nRet = SendCmd(szCmd, pInOutData, nInOutLen, ThisModule);                   //30-00-00-00(FT#0051)
        if (nRet != 0)                                                              //30-00-00-00(FT#0051)
        {                                                                           //30-00-00-00(FT#0051)
            Log(ThisModule, __LINE__, "SendCmd()失败");                              //30-00-00-00(FT#0051)
            return nRet;                                                            //30-00-00-00(FT#0051)
        }                                                                           //30-00-00-00(FT#0051)
        memset(reply, 0, sizeof(reply));                                            //30-00-00-00(FT#0051)
        nRet = GetResponse(reply, sizeof(reply), 2000, ThisModule);                 //30-00-00-00(FT#0051)
        if (nRet < 0)                                                               //30-00-00-00(FT#0051)
        {                                                                           //30-00-00-00(FT#0051)
            if(nRet != ERR_IDC_RESP_NOT_COMP){                                      //30-00-00-00(FT#0051)
                Log(ThisModule, __LINE__, "GetResponse()失败");                      //30-00-00-00(FT#0051)
                return nRet;                                                        //30-00-00-00(FT#0051)
            } else {                                                                //30-00-00-00(FT#0051)
                //获取数据长度不完整，重新发送命令                                       //30-00-00-00(FT#0051)
            }                                                                       //30-00-00-00(FT#0051)
        } else {                                                                    //30-00-00-00(FT#0051)
            //成功                                                                   //30-00-00-00(FT#0051)
            break;                                                                  //30-00-00-00(FT#0051)
        }                                                                           //30-00-00-00(FT#0051)
    }                                                                               //30-00-00-00(FT#0051)

    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
        if(nRet == CR_ERROR_DATA_ERROR_IN_CMD){
            return ERR_IDC_PROTOCOLNOTSUPP;
        }
        return HWErrToStdErr(nRet);
    }

    //返回61XX的特殊处理
    //40-00-00-00(FT#0005) add start
    if((reply[8] == 0x61) && ((MAKEWORD(reply[2], reply[1])) - 5 == 2)){
        BYTE byGetChipResCmd[5] = {0x00, 0xc0, 0x00, 0x00, 0x00};
        byGetChipResCmd[4] = reply[9];

        nRet = SendCmd(szCmd, (LPSTR)byGetChipResCmd, sizeof(byGetChipResCmd), ThisModule);
        if (nRet != 0)
        {
            Log(ThisModule, __LINE__, "SendCmd()失败");
            return nRet;
        }
        memset(reply, 0, sizeof(reply));
        nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
        if (nRet < 0)
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }
        if (reply[3] != 'P')
        {
            nRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
            if(nRet == CR_ERROR_DATA_ERROR_IN_CMD){
                return ERR_IDC_PROTOCOLNOTSUPP;
            }
            return HWErrToStdErr(nRet);
        }
    }
    //40-00-00-00(FT#0005) add end

    DWORD nLen = 0;
    if (reply[5] == '5')
    {
        //超过1000字节
        DWORD dwOffset = 0;
        do {
            nRet = SendCmd("CI7", nullptr, 0, ThisModule);
            if(nRet != 0){
                nInOutLen = 0;
                return nRet;
            }

            ZeroMemory(reply, sizeof(reply));
            nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
            if(nRet < 0){
                nInOutLen = 0;
                return nRet;
            }

            if (reply[3] != 'P'){
                nInOutLen = 0;
                nRet = CardReaderError(reply + 6, ThisModule);
                return HWErrToStdErr(nRet);
            }

            nLen = (MAKEWORD(reply[2], reply[1])) - 5;
            if(nLen + dwOffset > dwBuffSz){
                nLen = dwBuffSz - dwOffset;
                memcpy(pInOutData + dwOffset, reply + 8, nLen);
                Log(ThisModule, __LINE__, "Chipio实际接收数据大于入参buffer");
                break;
            }
            memcpy(pInOutData + dwOffset, reply + 8, nLen);
            dwOffset += nLen;

        } while (reply[5] != '6');
        nInOutLen = dwOffset;
    } else if (reply[5] == '4')     //不超过1000字节
    {
        nLen = MAKEWORD(reply[2], reply[1]) - 5;
        nInOutLen = qMin(nLen, dwBuffSz);
        memcpy(pInOutData, reply + 8, nInOutLen);
    }

    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct)
{
    Q_UNUSED(eFlagLedAct)
    Q_UNUSED(eFlagLedType)

    return 0;
}

INT CDevIDC_CRT::SetRFIDCardReaderBeep(unsigned long ulTime)
{
    Q_UNUSED(ulTime);

    return 0;
}

//30-00-00-00(FS#0014)
BOOL CDevIDC_CRT::GetSkimmingCheckStatus()
{
    return m_bSkimmingError;
}

INT CDevIDC_CRT::GetBit(BYTE reply, WORD wposition)
{
    if (wposition > 15)
        return ERR_IDC_PARAM_ERR;

    int nBinary[16] = {0};
    int i = 0;

    int nDecNumber = (int)reply;

    do {
        nBinary[i++] = nDecNumber % 2;
        nDecNumber /= 2;

    } while (nDecNumber != 0);

    return nBinary[wposition];
}

int CDevIDC_CRT::ForbiddenAcceptCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    char reply[2068] = {0};
    int nRet = SendCmd("C:1", nullptr, 0, ThisModule);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "禁止进卡命令失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "禁止进卡命令获取返回数据失败");
        return nRet;
    }
    //Todo:判断返回结果

    return nRet;
}

int CDevIDC_CRT::HWErrToStdErr(int iCardError)
{
    if(iCardError > CR_ERROR_UNDEFINED_COMMAND){
        return iCardError;
    }

    int iRet;
    switch(iCardError){
    case CR_ERROR_JAM:
    case CR_ERROR_CARD_JAM_IN_RETRIEVING:
        iRet = ERR_IDC_JAMMED;
        break;
    case CR_ERROR_COMMAND_DATA_ERROR:
        iRet = ERR_IDC_PARAM_ERR;
        break;
    case CR_ERROR_LONG_CARD:
        iRet = ERR_IDC_CARDTOOLONG;
        break;
    case CR_ERROR_SHORT_CARD:
        iRet = ERR_IDC_CARDTOOSHORT;
        break;
    case CR_ERROR_ICC_OR_SAM_ACTIVE_ATR_INVALID:
        iRet = ERR_IDC_INVALIDCARD;
        break;
    default:
        iRet = ERR_IDC_HWERR;
        break;
    }

    return iRet;
}

int CDevIDC_CRT::GetCardStatusC12()
{
    THISMODULE(__FUNCTION__);

    int nRet = ERR_IDC_SUCCESS;
    char reply[2068] = {0};
    nRet = SendCmd("C12", nullptr, 0, ThisModule);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;
    }
    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule);
        return HWErrToStdErr(nRet);
    }

    if (strncmp(reply + 6, "00", 2) == 0)//无卡
    {
        nRet = IDCSTAUTS_NOCARD;
    }
    else if (strncmp(reply + 6, "01", 2) == 0)//在门口
    {
        nRet = IDCSTAUTS_ENTERING;
    }
    else if (strncmp(reply + 6, "02", 2) == 0)//在内部
    {
        nRet = IDCSTAUTS_INTERNAL;
    }
    else {
        nRet == IDCSTATUS_UNKNOWN;
    }

    return nRet;
}

/*************************
 * @func 设置磁干扰模式
 * @param iMode:0-正常模式　1-节能模式
 ************************/
INT CDevIDC_CRT::SetESUMode(int iMode)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    LOGDEVACTION();
    int nRet = ERR_IDC_SUCCESS;
    char reply[2068] = {0};

    //判断硬件是否支持磁干扰功能
    nRet = SendCmd("Cp1", nullptr, 0, ThisModule);
    if (nRet != 0)
    {
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        return nRet;
    }
    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule);
        return HWErrToStdErr(nRet);
    } else {
        if(reply[8] & 0x20){            //支持磁干扰功能
            nRet = SendCmd("Cp2", iMode == 0 ? "0" : "1", 1, ThisModule);
            if (nRet != 0)
            {
                return nRet;
            }
            memset(reply, 0, sizeof(reply));
            nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
            if (nRet < 0)
            {
                return nRet;
            }
            if (reply[3] != 'P')
            {
                nRet = CardReaderError(reply + 6, ThisModule);
                return HWErrToStdErr(nRet);
            }
        }
    }

    return ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::UpdateCardStatus()
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
    //int nRet = SendCmd("C11", nullptr, 0, ThisModule);
    int nRet = SendCmd("C13", nullptr, 0, ThisModule);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");        
        m_nLastError = nRet;
        return nRet;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");        
        m_nLastError = nRet;
        return nRet;
    }
    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule);
        nRet = HWErrToStdErr(nRet);
        m_nLastError = nRet;
        return nRet;
    }

    if (strncmp(reply + 6, "00", 2) == 0)//无卡
    {
        IDCstatus = IDCSTAUTS_NOCARD;
        if (m_nEjectedCard == EJECT_SUCCESS)
        {
            m_nTakeCard = MEDIA_TAKEN; //取走
        }
    }
    else if (strncmp(reply + 6, "01", 2) == 0)//在门口
    {
        IDCstatus = IDCSTAUTS_ENTERING;
        if (GetBit(reply[9], 3) == 1)
            m_bCardInGatePosition = TRUE; // 卡在门口并且Shutter打开
        else
            m_bCardInGatePosition = FALSE;
    }
    else if (strncmp(reply + 6, "02", 2) == 0)//在内部
    {
        IDCstatus = IDCSTAUTS_INTERNAL;
    }
    else {
        IDCstatus == ERR_IDC_OTHER;
    }

    //获取IC触点状态
    if (GetBit(reply[8], 2) == 1)
        m_bICCRelease = FALSE;
    else {
        m_bICCActived = FALSE;
        m_bICCRelease = TRUE;
    }

    if (m_bICCActived && (IDCstatus == IDCSTAUTS_INTERNAL))
        IDCstatus = IDCSTAUTS_ICC_ACTIVE;

    if (m_nCardStatus != IDCstatus)
    {
        char szLog[255] = {0};
        memcpy(szLog, reply + 6, 2);
        if (IDCstatus == IDCSTAUTS_ENTERING)
            sprintf(szLog + strlen(szLog), "(卡在门口，返回：%d)", nRet);
        else if (IDCstatus == IDCSTAUTS_INTERNAL)
            sprintf(szLog + strlen(szLog), "(卡在内部，返回：%d)", nRet);
        else if (!m_bICCRelease)
            sprintf(szLog + strlen(szLog), "(IC卡触点接触，返回：%d)", nRet);
        else if (m_bICCRelease)
            sprintf(szLog + strlen(szLog), "(IC卡触点释放，返回：%d)", nRet);
        else if (IDCstatus == IDCSTAUTS_ICC_ACTIVE)
            sprintf(szLog + strlen(szLog), "(IC卡被激活，返回：%d)", nRet);
        else
            sprintf(szLog + strlen(szLog), "(没有检测到卡，返回：%d)", nRet);
        Log(ThisModule, 1, "%s", szLog);
    }
    m_nCardStatus = IDCstatus;
    return ERR_IDC_SUCCESS;
}

void CDevIDC_CRT::GetErrorDesc(int  nErrorCode, char *pszDesc)
{
    switch (nErrorCode)
    {
    case  CR_ERROR_UNDEFINED_COMMAND:
        strcpy(pszDesc, "00 读卡器接收到没有定义的命令");
        break;
    case  CR_ERROR_COMMAND_DATA_ERROR:
        strcpy(pszDesc, "01 读卡器命令参数错误");
        break;
    case   CR_ERROR_COMMAND_UNABLE_EXEC:
        strcpy(pszDesc, "02 读卡器命令无法执行（如在读卡器内无卡时执行读卡命令）");
        break;
    case  CR_ERROR_HARDWARE_UNAVAILABLE:
        strcpy(pszDesc, "03 读卡器执行命令需要的功能(硬件)不可用或有故障");
        break;
    case  CR_ERROR_DATA_ERROR_IN_CMD:
        strcpy(pszDesc, "04 读卡器命令中的数据错误");
        break;
    case  CR_ERROR_NO_RELEASE_CONTACT_MOVE:
        strcpy(pszDesc, "05 移动卡时没有释放触点");
        break;
    case  CR_ERROR_NO_KEY:
        strcpy(pszDesc, "06 没有加密功能需要的密钥");
        break;

    case  CR_ERROR_JAM:
        strcpy(pszDesc, "10 读卡器堵卡");
        break;
    case  CR_ERROR_SHUTTER_ERROR:
        strcpy(pszDesc, "11 读卡器门异常");
        break;
    case  CR_ERROR_SENSOR_ERROR:
        strcpy(pszDesc, "12 读卡器传感器异常");
        break;
    case  CR_ERROR_LONG_CARD:
        strcpy(pszDesc, "13 读卡器检测到卡太长，门不能被关闭");
        break;
    case  CR_ERROR_SHORT_CARD:
        strcpy(pszDesc, "14 读卡器检测到卡太短");
        break;
    case  CR_ERROR_WRITE_FRAM_ERROR:
        strcpy(pszDesc, "15 读卡器FRAM错误");
        break;
    case  CR_ERROR_POSITION_CHANGE:
        strcpy(pszDesc, "16 卡位置移动");
        break;
    case  CR_ERROR_CARD_JAM_IN_RETRIEVING:
        strcpy(pszDesc, "17 重进卡时卡堵塞");
        break;
    case  CR_ERROR_SW1_SW2_ERROR:
        strcpy(pszDesc, "18 SW1, SW2 错误");
        break;
    case  CR_ERROR_CARD_NO_INSERTED_BACK:
        strcpy(pszDesc, "19 卡没有从后端插入");
        break;

    case  CR_ERROR_READ_TRACK_PARITY_ERROR:
        strcpy(pszDesc, "20 读磁卡错误 (奇偶校验错)");
        break;
    case  CR_ERROR_READ_TRACK_ERROR:
        strcpy(pszDesc, "21 读磁卡错误");
        break;
    case  CR_ERROR_WRITE_TRACK_ERROR:
        strcpy(pszDesc, " 22 写磁卡错误");
        break;
    case  CR_ERROR_READ_NO_DATA_ERROR:
        strcpy(pszDesc, "23 读磁卡错误 (没有数据内容，只有 STX 起始符，ETX 结束符和 LRC)");
        break;
    case  CR_ERROR_READ_NO_TRACK:
        strcpy(pszDesc, "24 读磁卡错误 (没有磁条或没有编码-空白轨道)");
        break;
    case  CR_ERROR_WRITE_CHECK_ERROR:
        strcpy(pszDesc, "25 写磁卡校验错误 (品质错误)");
        break;
    case  CR_ERROR_READ_ERROR_IN_SS:
        strcpy(pszDesc, "26 读磁卡错误（没有 SS）");
        break;
    case  CR_ERROR_READ_ERROR_IN_ES:
        strcpy(pszDesc, "27 读卡器读错误（ES错误)");
        break;
    case  CR_ERROR_READ_ERROR_IN_LRC:
        strcpy(pszDesc, "28 读卡器读错误（LRC错误）");
        break;
    case  CR_ERROR_WRITE_ERROR_VERIFICATION:
        strcpy(pszDesc, "29 读卡器写错误（数据不一致）");
        break;

    case  CR_ERROR_POWER_FAILURE:
        strcpy(pszDesc, "30 读卡器电源掉电");
        break;
    case  CR_ERROR_DSR_IS_OFF:
        strcpy(pszDesc, "31 读卡器DSR 信号为 OFF");
        break;

    case  CR_ERROR_BEFORE_RETRACT_PULL:
        strcpy(pszDesc, "40 吞卡时卡拔走");
        break;
    case  CR_ERROR_IC_CONTACT_SENNOR_ERROR:
        strcpy(pszDesc, "41 IC触点或触点传感器错误");
        break;
    case  CR_ERROR_UNABLE_REACH_IC_POSITION:
        strcpy(pszDesc, "43 无法走到 IC 卡位");
        break;
    case  CR_ERROR_ENFORCE_EJECT_CARD:
        strcpy(pszDesc, "45 卡机强制弹卡");
        break;
    case  CR_ERROR_RETRIEVE_TIMEOUT:
        strcpy(pszDesc, "46 前端卡未在指定时间内取走");
        break;

    case  CR_ERROR_COUNT_OVERFLOW:
        strcpy(pszDesc, "50 回收卡计数溢出");
        break;
    case  CR_ERROR_MOTOR_ERROR:
        strcpy(pszDesc, "51 马达错误");
        break;
    case  CR_ERROR_DIGITAL_DECODING_READ:
        strcpy(pszDesc, "53 数字解码读错误");
        break;
    case  CR_ERROR_TANMPER_MOVE_ERROR:
        strcpy(pszDesc, "54 防盗钩移动错误");
        break;
    case  CR_ERROR_TANMPER_BEEN_SET:
        strcpy(pszDesc, " 55 防盗钩已经设置，命令不能执行");
        break;
    case  CR_ERROR_CHIP_TEST_SENSOR_ERROR:
        strcpy(pszDesc, " 56 芯片检测传感器错误");
        break;
    case  CR_ERROR_TANMPER_IS_MOVING:
        strcpy(pszDesc, "5B 防盗钩正在移动");
        break;

    case  CR_ERROR_ICC_OR_SAM_VCC_ABNORMAL:
        strcpy(pszDesc, "60 IC 卡或 SAM 卡 Vcc 条件异常");
        break;
    case  CR_ERROR_ICC_OR_SAM_ATR_COMM_ERROR:
        strcpy(pszDesc, "61 IC 卡或 SAM 卡 ATR 通讯错误");
        break;
    case  CR_ERROR_ICC_OR_SAM_ACTIVE_ATR_INVALID:
        strcpy(pszDesc, "62 IC 卡或 SAM 卡在当前激活条件下 ATR 无效");
        break;
    case  CR_ERROR_ICC_OR_SAM_COMM_NO_RESPONSE:
        strcpy(pszDesc, "63 IC 卡或 SAM 卡通讯过程中无响应");
        break;
    case  CR_ERROR_ICC_OR_SAM_COMM_ERROR:
        strcpy(pszDesc, "64 IC 卡或 SAM 卡通讯错误（除无响应外）");
        break;
    case  CR_ERROR_ICC_OR_SAM_CARD_INACTIVATED:
        strcpy(pszDesc, "65 IC 卡或 SAM 卡未激活");
        break;
    case  CR_ERROR_ICC_OR_SAM_NOT_SUPPORT:
        strcpy(pszDesc, "66 IC 卡或 SAM 卡不支持（仅对于非 EMV 激活）");
        break;
    case  CR_ERROR_ICC_OR_SAM_NOT_SUPPORT_EMV:
        strcpy(pszDesc, "69 IC 卡或 SAM 卡不支持（仅对于 EMV 激活）");
        break;

    case  CR_ERROR_ESU_AND_IC_COMM_ERROR:
        strcpy(pszDesc, "76 ESU模块和卡机通讯错误");
        break;

    case  CR_ERROR_ESU_NO_CONNECTION:
        strcpy(pszDesc, "95 ESU 模块损坏或无连接");
        break;
    case  CR_ERROR_ESU_OVERCURRENT:
        strcpy(pszDesc, "99 ESU 模块过流");
        break;

    case  CR_ERROR_NO_RECEIVE_INIT_CMD:
        strcpy(pszDesc, "B0 未接收到初始化命令");
        break;
    default:
        sprintf(pszDesc, "未找到(%d)对应的错误描述", nErrorCode);
        break;
    }
}

bool CDevIDC_CRT::InFraudProtect()
{
    if (m_stIDC.bNeedFraudProtect)
    {
        if (m_bFraudDetected)
        {
            if (m_stIDC.bFraudProtectTime == -1)
                return true;
            else if (m_stIDC.bFraudProtectTime == 0)
            {
                m_bFraudDetected = FALSE;
                return false;
            }
            else if ((CQtTime::GetSysTick() - m_dwLastFraudDetectedTime) < (m_stIDC.bFraudProtectTime * 1000))
                return true;
            else
                m_bFraudDetected = FALSE;
        }
    }

    return false;
}

INT CDevIDC_CRT::CardReaderError(const char *pCode, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    const char *ErrorCode[] = {"00", "01", "02", "03", "04", "05", "06",
                               "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                               "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
                               "30", "31",
                               "40", "41", "43", "45", "46",
                               "50", "51", "53", "54", "55", "56", "5B",
                               "60", "61", "62", "63", "64", "65", "66", "69",
                               "76",
                               "95", "99",
                               "B0"
                              };
    for (size_t i = 0; i < sizeof(ErrorCode) / sizeof(ErrorCode[0]); i++)
    {
        if (strncmp(ErrorCode[i], pCode, 2) == 0)
        {
            int nRet = (int)(CR_ERROR_UNDEFINED_COMMAND - i);
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

//通知读卡器取消进卡
LONG CDevIDC_CRT::StopCmd()
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    char reply[2068] = {0};
    char cDltEot[2] = {0x10, 0x04};

    //int nRet = m_pDev->Send("\x01\x00\x01\x45", 4, TIMEOUT_WAIT_ACTION);   // diff: time
    int nRet = SendCmd(cDltEot, nullptr, 0, ThisModule); // 禁止前端和后端进卡(DISABLE 模式)
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return ERR_IDC_HWERR;
    }
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return ERR_IDC_HWERR;
    }
    if (nRet > 2)
    {
        char szLog[255] = {0};
        for (int i = 0; i < nRet; i++)
        {
            sprintf(szLog + strlen(szLog), "0x%x ", reply[i]);
            Log(ThisModule, __LINE__, "命令返回数据:%s", szLog);
        }
        ZeroMemory(reply, sizeof(reply));
    }
    else
    {
        if (reply[3] != 'P')
        {
            Log(ThisModule, __LINE__, "命令返回数据不符(0x%x, 0x%x)", reply[0], reply[1]);
        }
    }
    return  ERR_IDC_SUCCESS;
}

INT CDevIDC_CRT::SendCmd(const char *pszCmd, const char *lpData, int nLen, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null");
        return ERR_DEVPORT_NOTOPEN;
    }

    int nRet = 0;

    if (nLen >= 0 && nLen <= CRT_PACK_MAX_CMP_LEN)
    {
        // one packet
        nRet = SendSinglePacket(pszCmd, lpData, nLen, ThisModule);
        if (nRet < 0)
        {
            return nRet;
        }

    } else if ((nLen > CRT_PACK_MAX_CMP_LEN) && (lpData != nullptr))
    {
        // Multi packet
        nRet = SendMultiPacket(pszCmd, lpData, nLen, ThisModule);
        if (nRet < 0)
        {
            return nRet;
        }
    }

    return ConvertErrorCode(nRet);
}

INT CDevIDC_CRT::SendSinglePacket(const char* pszCmd, const char *lpData, int nLen, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    char buf[2068] = {0};
    int nRet = 0;
    BYTE dwReportID = {0};
    int nbufSize = 0;

    // one packet
    dwReportID = CRT_REPORT_ID;
    buf[0] = (char)dwReportID;
    buf[1] = (char)((nLen + 3) / 256);
    buf[2] = (char)((nLen + 3) % 256);
    //TEXT命令
    memcpy(buf + 3, pszCmd, 3);

    if (lpData != nullptr)
    {
        memcpy(buf + 6, lpData, nLen);
    }

    nbufSize = nLen + 6;

    nRet = m_pDev->Send(buf, nbufSize, TIMEOUT_WAIT_ACTION);

    if (nRet != ERR_DEVPORT_SUCCESS)
    {
        Log(ThisModule, __LINE__, "%s: SendData()出错，返回%d", pszCallFunc, nRet);
        m_nLastError = nRet;
    }

    return ConvertErrorCode(nRet);                                           //30-00-00-00(FT#0019)
}

INT CDevIDC_CRT::SendMultiPacket(const char *pszCmd, const char *lpData, int nLen, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    //发送的数据格式
    //首包：ReportID(1 byte) + LEN(2 byte) + CMD(3 byte) + TEXT(58 byte)
    //后续包：ReportID(1 byte) + TEXT(63 byte)

    char buf[64] = {0};
    int nPackCount = 0;
    int nRet = 0;
    // 分包
    int nDataLenExceptFirstPacket = nLen - (CRT_PACK_MAX_CMP_LEN);
    nPackCount = 1 + nDataLenExceptFirstPacket / CRT_PACK_MAX_PAD_LEN +
                 (nDataLenExceptFirstPacket % CRT_PACK_MAX_PAD_LEN != 0 ? 1 : 0);

    int nDataOffset = 0;
    for(int nWriteIdx = 0; nWriteIdx < nPackCount; nWriteIdx++){
        memset(buf, 0, sizeof(buf));
        // frist
        if(nWriteIdx == 0){
            buf[0] = (char)CRT_MULTI_REPORT_ID;
            buf[1] = (char)((nLen + 3) / 256);
            buf[2] = (char)((nLen + 3) % 256);
            memcpy(buf + 3, pszCmd, 3);
            memcpy(buf + 6, lpData, CRT_PACK_MAX_CMP_LEN);
            nDataOffset += CRT_PACK_MAX_CMP_LEN;
            // last
        } else if(nWriteIdx == nPackCount - 1){
            buf[0] = (char)CRT_REPORT_ID;
            memcpy(buf + 1, lpData + nDataOffset, nLen - nDataOffset);
            // middle
        } else {
            buf[0] = (char)CRT_MULTI_REPORT_ID;
            memcpy(buf + 1, lpData + nDataOffset, CRT_PACK_MAX_PAD_LEN);
            nDataOffset += CRT_PACK_MAX_PAD_LEN;
        }

        nRet = m_pDev->Send(buf, CRT_PACK_MAX_LEN, TIMEOUT_WAIT_ACTION);

        if (nRet != ERR_DEVPORT_SUCCESS)
        {
            Log(ThisModule, __LINE__, "%s: SendData()出错，返回%d", pszCallFunc, nRet);
            m_nLastError = nRet;
            break;
        }
    }
    return ConvertErrorCode(nRet);
}

/*
  功能：读取读卡器的返回数据
  参数：pszResponse返回数据的缓冲区，nLen缓冲区长度，nTimeout超时(毫秒)，pWaitCancel是IWaitCancel指针
  返回：>0数据长度，=0错误
*/
INT CDevIDC_CRT::GetResponse(char *pszResponse, int nLen, int nTimeout, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();
    Q_UNUSED(nLen)

    char pszReply[64];
    int nRet = 0;
    int nIndex = 0;
    int timeout = nTimeout;
    ULONG ulStart = CQtTime::GetSysTick();              //30-00-00-00(FT#0033)

    if (m_pDev == nullptr)
        return ERR_DEVPORT_NOTOPEN;

    while(TRUE)
    {
        DWORD ulInOutLen = USB_READ_LENTH;
        memset(pszReply, 0, sizeof(pszReply));
        nRet = m_pDev->Read(pszReply, ulInOutLen, timeout);
        if (nRet < 0)                                 //30-00-00-00(FT#0033)
        {
            break;
        } else if (nRet > 0) {                        //30-00-00-00(FT#0033)
            if (nIndex == 0)
            {
               // 第一个包
               nIndex = nRet;
               memcpy(pszResponse, pszReply, nIndex);
               timeout = 50;
            } else
            {
                // 多个包
                memcpy(pszResponse + nIndex, pszReply + 1, nRet - 1);
                nIndex += nRet - 1;
            }
            if (pszReply[0] == 0x04)
            {
                break;
            }
        }

        //判断超时
        if(CQtTime::GetSysTick() - ulStart > nTimeout){             //30-00-00-00(FT#0033)
            break;                                                  //30-00-00-00(FT#0033)
        }                                                           //30-00-00-00(FT#0033)
    }

    DWORD nRecvLen = MAKEWORD(pszResponse[2], pszResponse[1]);
    if (nRecvLen != 0) {
        if(nIndex < nRecvLen + 3){
            Log(ThisModule, __LINE__, "%s: 读卡器返回数据接收不完整", pszCallFunc);
//            return ERR_IDC_READERROR;                             //30-00-00-00(FT#0051)
            return ERR_IDC_RESP_NOT_COMP;                           //30-00-00-00(FT#0051)
        }
        return nRecvLen + 3;
    } else {
        Log(ThisModule, __LINE__, "%s: 读卡器返回数据错误", pszCallFunc);
        return ERR_IDC_READERROR;
    }
}

INT CDevIDC_CRT::SendReadCmd(LPCSTR pszCmd, LPCSTR lpData, LPSTR pszResponse, int &nInOutReadLen, int nTimeout, LPCSTR pszCallFunc)
{
    int nRet = ERR_IDC_SUCCESS;
    int len = 0;

    nRet = SendCmd(pszCmd, lpData, strlen(lpData), pszCallFunc);
    if (nRet < 0)
    {
        Log(pszCallFunc, __LINE__, "SendReadCmd()->SendCmd()失败");
        return ERR_IDC_COMM_ERR;
    }

    len = GetResponse(pszResponse, nInOutReadLen, nTimeout, pszCallFunc);
    if (len < 0)
    {
        Log(pszCallFunc, __LINE__, "SendReadCmd()->GetResponse()失败");
        return len;
    }

    if (pszResponse[3] != 'P')
    {
        nRet = CardReaderError(pszResponse + 6, pszCallFunc); //记录错误日志
        return HWErrToStdErr(nRet);
    }

    nInOutReadLen = len;
    return ERR_IDC_SUCCESS;
}

/*设置指定命令超时时间，仅对进卡、撤卡、重进卡命令有效，返回：=0成功，<0失败*/
INT CDevIDC_CRT::SetTimeOutMonitor(MONITOR_TYPE MonitorType, int nTimeOut)
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
            memcpy(szCmd, "C2=", 3);
            break;
        }
    case MONITOR_WITHDRAWAL:
        {
            memcpy(szCmd, "C37", 3);
            break;
        }
    case MONITOR_REINTAKE:
        {
            memcpy(szCmd, "C40", 3);
            break;
        }
    default:
        return ERR_IDC_PARAM_ERR;
    }

    sprintf(t, "%02d", nTimeOut);
    strcat(szCmd, t);

    int nRet = SendCmd(szCmd, nullptr, 0, ThisModule);
    if (nRet != 0)
        Log(ThisModule, __LINE__, "SendCmd()失败");
        return nRet;

    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0)
        Log(ThisModule, __LINE__, "GetResponse()失败");
        return nRet;

    if (reply[3] != 'P')
    {
        nRet = CardReaderError(reply + 6, ThisModule); //记录错误日志
        return HWErrToStdErr(nRet);
    }

    return 0;
}

bool CDevIDC_CRT::TamperSensorDetect()
{
    THISMODULE(__FUNCTION__);

    // 不支持防盗嘴检测
    if (m_stIDC.bTamperSensorSupport == 0)
    {
        Log(ThisModule, __LINE__, "不支持防盗嘴检测");
        return false;
    }

    Log(ThisModule, __LINE__, "支持防盗嘴检测");

    //TamperSensor检测ON状态约1分20秒，OFF状态实时检测
    int nRet = SendCmd("Cc8", nullptr, 0, ThisModule);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "获取防盗钩状态失败");
        return false;
    }

    char reply[2068] = {0};
    nRet = GetResponse(reply, sizeof(reply), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRet < 0 || reply[3] != 'P')
    {
        Log(ThisModule, __LINE__, "GetResponse()失败,reply:%s", reply);
        return nRet;
    }

    if (GetBit(reply[8], 0) == 0)
    {
        //防盗钩未释放
        m_bTamperSensorStatus = TANMPER_SENSOR_ON;
        Log(ThisModule, __LINE__, "防盗钩未释放 : %d", m_bTamperSensorStatus);
        return true;
    } else {
        //防盗钩已释放
        m_bTamperSensorStatus = TANMPER_SENSOR_OFF;
        Log(ThisModule, __LINE__, "防盗钩已释放 : %d", m_bTamperSensorStatus);
        return false;
    }
}

//抖动进卡
INT CDevIDC_CRT::SetWobble(WobbleAction nNeedWobble)
{
    THISMODULE(__FUNCTION__);
    VERTIFYISOPEN();

    int nRet;
    char szReply[2068] = {0};

    if (WOBBLEACTION_START == nNeedWobble)
    {
        m_pDev->SetLogAction(ThisModule);
        nRet = SendCmd("C:X", "1", 1, ThisModule);
        if (nRet != 0)
        {
            Log(ThisModule, __LINE__, "SendCmd()失败");
            return ERR_IDC_COMM_ERR;
        }

        nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_WAIT_ACTION, ThisModule);
        if (nRet < 0 || szReply[3] != 'P')
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }
    }
    else if (WOBBLEACTION_STOP == nNeedWobble)
    {
        m_pDev->SetLogAction(ThisModule);
        nRet = SendCmd("C:X", "0", 1, ThisModule);
        if (nRet != 0)
        {
            Log(ThisModule, __LINE__, "SendCmd()失败");
            return ERR_IDC_COMM_ERR;
        }

        ZeroMemory(szReply, sizeof(szReply));
        nRet = GetResponse(szReply, sizeof(szReply), TIMEOUT_WAIT_ACTION, ThisModule);
        if (nRet < 0 || szReply[3] != 'P')
        {
            Log(ThisModule, __LINE__, "GetResponse()失败");
            return nRet;
        }
    }

    return ERR_IDC_SUCCESS;
}

DWORD CDevIDC_CRT::GetReportLenFromLength(int nInLen, int &nOutSendLen)
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

INT CDevIDC_CRT::ConvertErrorCode(long iRet)
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
