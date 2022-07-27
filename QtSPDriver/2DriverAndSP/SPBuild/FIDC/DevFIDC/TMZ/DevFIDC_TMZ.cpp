#include "DevFIDC_TMZ.h"
#include "AutoQtHelpClass.h"

static const char *ThisFile = "DevFIDC_TMZ.cpp";
#define FIDC_SLOT_NUM 0x31
//////////////////////////////////////////////////////////////////////////
CDevFIDC_TMZ::CDevFIDC_TMZ(LPCSTR lpDevType) : m_SemCancel(false)
{
    SetLogFile("TMZ.log", ThisFile, lpDevType);
    m_eCardStatus = IDCSTATUS_UNKNOWN;
    m_bOpen = false;

    m_nPid = 0x0219;
    m_nVid = 0x23A4;
    m_nProid = 0;
    m_nDetectCardMode = 0;
    m_bIccActive = false;
    m_lDevHdl = 0;
}

CDevFIDC_TMZ::~CDevFIDC_TMZ()
{

    Close();
}

void CDevFIDC_TMZ::Release()
{

}

int CDevFIDC_TMZ::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);   

    //读取配置文件
    ReadIniFile();

    //加载sdk库
    if(!m_TMZDriver.LoadSdkDll()){
        Log(ThisModule, __LINE__, "TMZ Library load fail.");
        return ERR_IDC_NOT_OPEN;
    }

    m_lDevHdl = m_TMZDriver.ICReaderOpenUsbByFD(0);
    if(m_lDevHdl == 0)
    {
        Log(ThisModule, __LINE__, "ICReaderOpenUsbByFD fail.");
        return ERR_IDC_NOT_OPEN;
    }
    m_bOpen = true;
    return 0;
}

int CDevFIDC_TMZ::Init(CardAction eActFlag, WobbleAction nNeedWobble)
{
    int nRet = ERR_IDC_SUCCESS;
    if(eActFlag == CARDACTION_EJECT){
        IDC_IDCSTAUTS eIdcStatus;
        nRet = DetectCard(eIdcStatus);
        if(nRet == ERR_IDC_SUCCESS && eIdcStatus == IDCSTAUTS_INTERNAL){
            nRet = EjectCard();
        }
    }

    return nRet;
}

void CDevFIDC_TMZ::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_bOpen == false)
    {
        return;
    }

    m_TMZDriver.ICReaderClose(m_lDevHdl);
    m_bOpen = false;
    return;
}

int CDevFIDC_TMZ::EatCard()
{
    return 0;
}

int CDevFIDC_TMZ::EjectCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }
    m_eCardStatus = IDCSTAUTS_ENTERING;

    //card poweroff
    int nRet = m_TMZDriver.CPUPowerOff(m_lDevHdl, FIDC_SLOT_NUM);
    if(nRet != 0)
    {
        Log(ThisModule, __LINE__, "Power off failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }
    m_eCardStatus = IDCSTAUTS_ICC_POWEROFF;
    m_bIccActive = false;
    return 0;
}

int CDevFIDC_TMZ::AcceptCard(ULONG ulTimeOut, bool Magnetic)
{
    THISMODULE(__FUNCTION__);

    if(m_bOpen == false){
        return ERR_IDC_OFFLINE;
    }

    while(m_SemCancel.WaitForEvent(1));

    ULONG ulTimeStart = CQtTime::GetSysTick();
    m_eCardStatus = IDCSTAUTS_NOCARD;
    while (true)
    {
        if (m_SemCancel.WaitForEvent(5))
        {
            return ERR_IDC_USER_CANCEL;
        }

        IDC_IDCSTAUTS eIDCstatus;
        int nRet = DetectCard(eIDCstatus);
        if (nRet == ERR_IDC_SUCCESS)
        {
            if(eIDCstatus == IDCSTAUTS_INTERNAL || eIDCstatus == IDCSTAUTS_ENTERING){
                return ERR_IDC_SUCCESS;
            }
        } else {
            return nRet;
        }

        if ((CQtTime::GetSysTick() - ulTimeStart) > ulTimeOut)
        {
            return ERR_IDC_INSERT_TIMEOUT;
        }
        CQtTime::Sleep(10);
    }
    return ERR_IDC_HWERR;
}

int CDevFIDC_TMZ::CancelReadCard()
{
    if(m_bOpen == false){
        return ERR_IDC_SUCCESS;
    }

    m_SemCancel.SetEvent();
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_TMZ::WriteTracks(const STTRACK_INFO &stTrackInfo)
{
    return ERR_IDC_UNSUP_CMD;
}

int CDevFIDC_TMZ::ReadTracks(STTRACK_INFO &stTrackInfo)
{
    return ERR_IDC_UNSUP_CMD;
}

int CDevFIDC_TMZ::DetectCard(IDC_IDCSTAUTS &IDCstatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }

    int iCardStatus;
    int nRet = m_TMZDriver.GetCardState(m_lDevHdl, FIDC_SLOT_NUM, &iCardStatus);
    if(nRet != 0){
        Log(ThisModule, __LINE__, "Get ic card status:%d.", nRet);
        m_bIccActive = false;
        m_eCardStatus = IDCSTATUS_UNKNOWN;
        return ERR_IDC_OFFLINE;
    }

    switch(iCardStatus){
    case CARD_POWER_OFF:
        if(!m_bIccActive){
            m_eCardStatus = IDCSTAUTS_ENTERING;
        } else {
            m_eCardStatus = IDCSTAUTS_INTERNAL;
        }
        break;
    case CARD_POWER_ON:
        m_eCardStatus = IDCSTAUTS_INTERNAL;
        break;
    case CARD_POWER_ON_FAIL:
    case HANDLE_CARD_DATA_ERR:
    case HANDLE_CARD_DATA_TIMEOUT:
        m_bIccActive = false;
        m_eCardStatus =  IDCSTATUS_UNKNOWN;
        break;
    case NO_CARD:
    case CARD_TYPE_NOT_SUPP:
    default:
        m_bIccActive = false;
        m_eCardStatus =  IDCSTAUTS_NOCARD;
        break;
    }

    IDCstatus =  m_eCardStatus;
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_TMZ::GetFWVersion(char pFWVersion[10][MAX_LEN_FWVERSION], unsigned int &uLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }

    if (nullptr == pFWVersion || uLen == 0)
    {
        return ERR_IDC_PARAM_ERR;
    }
    //GetVersion
    string strVerInfo;
    char szTemp[64];
    int nLen = 0;
    ZeroMemory(szTemp, sizeof(szTemp));

    int nRet = m_TMZDriver.ICReaderGetVer(m_lDevHdl, szTemp);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "Get version failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }
    char DEV[MAX_LEN_FWVERSION] = "Ver:";
    memcpy(DEV+4, szTemp, 22);
    strcpy(pFWVersion[0], DEV);
    pFWVersion[0][uLen - 1] = 0x00;
    uLen = strlen(pFWVersion[0]);

    char szTemp1[64] = {0};
    nRet = m_TMZDriver.ICReaderReadDevSnr(m_lDevHdl, 20, szTemp);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "Get version failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }
    char Snr[MAX_LEN_FWVERSION] = "Snr:";
    memcpy(Snr+4, szTemp1, 22);
    strcpy(pFWVersion[1], Snr);
    pFWVersion[1][sizeof(pFWVersion[1]) - 1] = 0x00;
    uLen = uLen + strlen(pFWVersion[1]);
/*
    strVerInfo.append(szTemp);

    memset(pFWVersion, 0, uLen);
    uLen = qMin((size_t)strVerInfo.size(), (size_t)uLen - 1);
    memcpy(pFWVersion, strVerInfo.c_str(), uLen);
*/
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_TMZ::SetRecycleCount(LPCSTR pszCount)
{
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_TMZ::ICCPress()
{
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_TMZ::ICCRelease()
{
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_TMZ::ICCActive(char pATRInfo[], unsigned int &uATRLen)
{    
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }
    unsigned char byCardType = 0;
    unsigned char bySnr[4] = {0};
    unsigned char bySnrLen = sizeof(bySnr);

    int nRet = m_TMZDriver.CPUPowerOn(m_lDevHdl, (BYTE)FIDC_SLOT_NUM, 100, &byCardType, &bySnrLen, bySnr,
                                      (int *)&uATRLen, (LPBYTE)pATRInfo);
    if(nRet != 0){
        Log(ThisModule, __LINE__, "Power on failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }

    m_bIccActive = true;
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_TMZ::ICCReset(ICCardReset eResetFlag, char pATRInfo[], unsigned int &uATRLen)
{
    return ICCActive(pATRInfo, uATRLen);
}

int CDevFIDC_TMZ::ICCMove()
{
    return ERR_IDC_UNSUP_CMD;
}

int CDevFIDC_TMZ::ICCDeActivation()
{

    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }

    //poweroff
    int nRet = m_TMZDriver.CPUPowerOff(m_lDevHdl, FIDC_SLOT_NUM);
    if(nRet != 0)
    {
        Log(ThisModule, __LINE__, "Power off failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }

    m_bIccActive = false;
    return 0;
}

int CDevFIDC_TMZ::ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);;

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }

    if (eProFlag != ICCARD_PROTOCOL_T0 && eProFlag != ICCARD_PROTOCOL_T1)
    {
        return ERR_IDC_PARAM_ERR;
    }

    char szRecvData[1024] = {0};
    int nLenRecv = 1024;

    int nRet = m_TMZDriver.CPUCardAPDU(m_lDevHdl, FIDC_SLOT_NUM, nInOutLen, (LPBYTE)pInOutData,
                                       &nLenRecv, (LPBYTE)szRecvData);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "IC Card chipio failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }

    nInOutLen = nLenRecv;
    memcpy(pInOutData, szRecvData, nInOutLen);
    return nRet;
}

int CDevFIDC_TMZ::SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }

    BYTE byLedCtrl = 0;             //0-关，1-开，2-闪烁
    BYTE byFlashInterval = 1000/50;
    switch(eFlagLedAct){
    case LEDACTION_CLOSE:
        byLedCtrl = 0;
        break;
    case LEDACTION_OPEN:
        byLedCtrl = 1;
        break;
    case LEDACTION_FLASH:
    {
        byLedCtrl = 2;
        switch(eFlagLedType){
        case LEDTYPE_YELLOW:                //慢闪
            byFlashInterval = 1000/50;
            break;
        case LEDTYPE_BLUE:                  //中闪
            byFlashInterval = 600/50;
            break;
        case LEDTYPE_GREEN:                 //快闪
            byFlashInterval = 300/50;
            break;
        default:
            return ERR_IDC_PARAM_ERR;
            break;
        }
    }
        break;
    default:
        return ERR_IDC_PARAM_ERR;
        break;
    }
    int nRet = m_TMZDriver.ICReaderLEDCtrl(m_lDevHdl, byLedCtrl, byFlashInterval);
    if(nRet != 0){
        Log(ThisModule, __LINE__, "Led control failed,nRet:%d.", nRet);
        return ConvertErrorCode(nRet);
    }

    return ERR_IDC_SUCCESS;
}

int CDevFIDC_TMZ::SetRFIDCardReaderBeep(unsigned long ulTime)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }


    int nRet = m_TMZDriver.ICReaderBeep(m_lDevHdl, 1, 10, ulTime);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "Device beep failed,nRet=%d.", nRet);
        return ConvertErrorCode(nRet);
    }
    return ERR_IDC_SUCCESS;
}

//30-00-00-00(FS#0014)
BOOL CDevFIDC_TMZ::GetSkimmingCheckStatus()
{
    return FALSE;
}
int CDevFIDC_TMZ::ReadIniFile()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    QString strINIFile("FIDCConfig.ini");
    #ifdef QT_WIN32
        strINIFile.prepend("C:/CFES/ETC/");
    #else
        strINIFile.prepend("/usr/local/CFES/ETC/");
    #endif

    // read FIDCConfig.ini
    if(!m_cINI.LoadINIFile(strINIFile.toLocal8Bit().data()))
    {
        Log(ThisModule, __LINE__, "Load FIDCConfig.ini failed");
        return ERR_IDC_OTHER;
    }

    CINIReader cINI = m_cINI.GetReaderSection("FIDCInfo");
    m_nVid = (int)cINI.GetValue("Vid", 0x23A4);
    m_nPid = (int)cINI.GetValue("Pid", 0x0219);
    m_nProid = (int)cINI.GetValue("Proid", 0);
    m_nDetectCardMode = (int)cINI.GetValue("DetectCardMode", 0);

    return 0;
}

/*
 *@param : byErrType:1:驱动错误 0:其他错误
 */
void CDevFIDC_TMZ::UpdateErrorCode(long lErrCode, BYTE byErrType/* = 0*/)
{
    if(byErrType == 1){
        sprintf(m_stDevIdcStatus.szErrCode, "1%04X", qAbs(lErrCode));
    } else {
        sprintf(m_stDevIdcStatus.szErrCode, "0%04X", qAbs(lErrCode));
    }
}

int CDevFIDC_TMZ::ConvertErrorCode(int nRetCode)
{
    int nRet;
    switch(qAbs(nRetCode)){   
    case DEVICE_DISCONNECT:
        nRet = ERR_IDC_OFFLINE;
        break;
    case MULTI_CARD:
        nRet = ERR_IDC_MULTICARD;
        break;
    case CARD_POWER_ON_FAIL:
    case CARD_TYPE_NOT_SUPP:
    case HANDLE_CARD_DATA_TIMEOUT:
        nRet = ERR_IDC_INVALIDCARD;
        break;
    case INVALID_HANDLE:
    case FUNC_OR_PARAM_NOT_SUPP:
    case CMD_EXEC_ERR:
    case WRITE_EPPROM_FAIL:
    case READ_EPPROM_FAIL:
    case NO_CARD:
    case CARD_POWER_ON:
    case CARD_POWER_OFF:
    case HANDLE_CARD_DATA_ERR:
    case CARD_HALT_FAIL: 
    default:
        nRet = ERR_IDC_OTHER;
        break;
    }

    return nRet;
}
