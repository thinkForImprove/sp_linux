#include "DevFIDC_MT50.h"
#include "AutoQtHelpClass.h"
#include "mtx.h"

static const char *ThisFile = "DevFIDC_MT50.cpp";
//////////////////////////////////////////////////////////////////////////
CDevFIDC_MT50::CDevFIDC_MT50(LPCSTR lpDevType) : m_SemCancel(false)
{
    SetLogFile("FIDC.log", ThisFile, lpDevType);
    m_eCardStatus = IDCSTATUS_UNKNOWN;
    m_bOpen = false;

    m_nPid = 0x0219;
    m_nVid = 0x23A4;
    m_nProid = 0;
    m_nDetectCardMode = 0;
    m_bIccActive = false;
}

CDevFIDC_MT50::~CDevFIDC_MT50()
{

    Close();
}

void CDevFIDC_MT50::Release()
{

}

int CDevFIDC_MT50::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);   

    //读取配置文件
    ReadIniFile();

    int nRet = open_usb_device(m_nVid, m_nPid, m_nProid);
    if(nRet != 0)
    {
        Log(ThisModule, __LINE__, "MT device open failed,nRet:%d", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }
    m_bOpen = true;
    return 0;
}

int CDevFIDC_MT50::Init(CardAction eActFlag, WobbleAction nNeedWobble)
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

void CDevFIDC_MT50::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_bOpen == false)
    {
        return;
    }

    close_device();
    m_bOpen = false;
    return;
}

int CDevFIDC_MT50::EatCard()
{
    return 0;
}

int CDevFIDC_MT50::EjectCard()
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
    int nRet = picc_poweroff();
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

int CDevFIDC_MT50::AcceptCard(ULONG ulTimeOut, bool Magnetic)
{
    THISMODULE(__FUNCTION__);

    if(m_bOpen == false){
        return ERR_IDC_OFFLINE;
    }

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

int CDevFIDC_MT50::CancelReadCard()
{
    if(m_bOpen == false){
        return ERR_IDC_SUCCESS;
    }

    m_SemCancel.SetEvent();
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_MT50::WriteTracks(const STTRACK_INFO &stTrackInfo)
{
    return ERR_IDC_UNSUP_CMD;
}

int CDevFIDC_MT50::ReadTracks(STTRACK_INFO &stTrackInfo)
{
    return ERR_IDC_UNSUP_CMD;
}

int CDevFIDC_MT50::DetectCard(IDC_IDCSTAUTS &IDCstatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }

    int nRet = picc_status();
    Log(ThisModule, __LINE__, "Get ic card status:%d.", nRet);
    switch(qAbs(nRet)){
    case CARD_NO_POWER_ON:
        if(m_bIccActive){
            m_eCardStatus = IDCSTAUTS_INTERNAL;
        } else {
            m_eCardStatus = IDCSTAUTS_ENTERING;
        }
        break;
    case LIBUSB_DEVICE_OFFLINE:
    case LIBUSB_NO_DEVICE:
    case LIBUSB_ACCESS_DENY:
    case LIBUSB_ACT_TIMEOUT:
        m_bIccActive = false;
        m_eCardStatus =  IDCSTATUS_UNKNOWN;
        break;
    case NO_CARD:
    default:
        m_bIccActive = false;
        m_eCardStatus =  IDCSTAUTS_NOCARD;
        break;
    }

    IDCstatus =  m_eCardStatus;
    return m_eCardStatus == IDCSTATUS_UNKNOWN ? ERR_IDC_OFFLINE : ERR_IDC_SUCCESS;
}

int CDevFIDC_MT50::GetFWVersion(char pFWVersion[], unsigned int &uLen)
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

    int nRet = get_device_version(szTemp ,&nLen);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "Get version failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }

    strVerInfo.append(szTemp, nLen);
    nRet = get_so_version(szTemp, &nLen);
    if(nRet != 0){
        Log(ThisModule, __LINE__, "Get so version failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }

    strVerInfo.append("|");
    strVerInfo.append(szTemp, nLen);

    memset(pFWVersion, 0, uLen);
    uLen = qMin((size_t)strVerInfo.size(), (size_t)uLen - 1);
    memcpy(pFWVersion, strVerInfo.c_str(), uLen);

    return ERR_IDC_SUCCESS;
}

int CDevFIDC_MT50::SetRecycleCount(LPCSTR pszCount)
{
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_MT50::ICCPress()
{
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_MT50::ICCRelease()
{
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_MT50::ICCActive(char pATRInfo[], unsigned int &uATRLen)
{    
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }
    unsigned char bySnr[4] = {0};

    int nRet = picc_poweron(m_nDetectCardMode, bySnr, (LPBYTE)pATRInfo, (int *)&uATRLen);
    if(nRet != 0){
        Log(ThisModule, __LINE__, "Power on failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }

    m_bIccActive = true;
    return ERR_IDC_SUCCESS;
}

int CDevFIDC_MT50::ICCReset(ICCardReset eResetFlag, char pATRInfo[], unsigned int &uATRLen)
{
    return ICCActive(pATRInfo, uATRLen);
}

int CDevFIDC_MT50::ICCMove()
{
    return ERR_IDC_UNSUP_CMD;
}

int CDevFIDC_MT50::ICCDeActivation()
{

    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }

    //poweroff
    int nRet = picc_poweroff();
    if(nRet != 0)
    {
        Log(ThisModule, __LINE__, "Power off failed,nRet:%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }

    m_bIccActive = false;
    return 0;
}

int CDevFIDC_MT50::ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz)
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

    int nRet = picc_apdu((LPBYTE)pInOutData, nInOutLen, (LPBYTE)szRecvData, &nLenRecv);
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

int CDevFIDC_MT50::SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct)
{
     return ERR_IDC_UNSUP_CMD;
}

int CDevFIDC_MT50::SetRFIDCardReaderBeep(unsigned long ulTime)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    if (m_bOpen == false)
    {
        return ERR_IDC_OFFLINE;
    }


    int nRet = device_beep(ulTime,100,1);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "Device beep failed,nRet=%d.", nRet);
        UpdateErrorCode(nRet, 1);
        return ConvertErrorCode(nRet);
    }
    return ERR_IDC_SUCCESS;
}

//30-00-00-00(FS#0014)
BOOL CDevFIDC_MT50::GetSkimmingCheckStatus()
{
    return FALSE;
}
int CDevFIDC_MT50::ReadIniFile()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    QString strINIFile("FIDCConfig.ini");
    #ifdef QT_WIN3
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
void CDevFIDC_MT50::UpdateErrorCode(long lErrCode, BYTE byErrType/* = 0*/)
{
    if(byErrType == 1){
        sprintf(m_stDevIdcStatus.szErrCode, "1%04X", qAbs(lErrCode));
    } else {
        sprintf(m_stDevIdcStatus.szErrCode, "0%04X", qAbs(lErrCode));
    }
}

int CDevFIDC_MT50::ConvertErrorCode(int nRetCode)
{
    int nRet;
    switch(qAbs(nRetCode)){
    case LIBUSB_DEVICE_OFFLINE:
    case LIBUSB_NO_DEVICE:
    case LIBUSB_DEVICE_BUSY:
        nRet = ERR_IDC_OFFLINE;
        break;
    case LIBUSB_ACT_TIMEOUT:
        nRet = ERR_IDC_COMM_ERR;
        break;
    case UNT_CARD_NOT_SUPPORT:              //30-00-00-00(FT#0041)
    case UNT_CARD_ACTIVATE_FAIL:            //30-00-00-00(FT#0041)
    case CARD_NO_RESPONSE:                  //30-00-00-00(FT#0041)
        nRet = ERR_IDC_INVALIDCARD;         //30-00-00-00(FT#0041)
        break;                              //30-00-00-00(FT#0041)
    case UNT_CARD_MORE_ONE:                 //30-00-00-00(FT#0041)
        nRet = ERR_IDC_MULTICARD;           //30-00-00-00(FT#0041)
        break;                              //30-00-00-00(FT#0041)
    case UNT_CARD_NOT_ACTIVATED:
    case UNT_CARD_NOT_ACK:
    case UNT_CARD_DATA_ERROR:
    case UNT_CARD_HALT_FAIL:    
    case LIUSB_IN_OUT_ERROR:
    case LIBUSB_PARAM_ERROR:
    case LIBUSB_ACCESS_DENY:
//    case LIBUSB_DEVICE_OFFLINE:
//    case LIBUSB_NO_DEVICE:
//    case LIBUSB_DEVICE_BUSY:
//    case LIBUSB_ACT_TIMEOUT:
    case LIBUSB_OVERFLOW:
    case LIBUSB_PIPE_ERROR:
    case LIBUSB_SYS_CALL_ABORT:
    case LIBUSB_NO_MEMORY:
    case LIBUSB_PLATFORM_ERROR:
    case COMM_TIMEOUT:
    case INVALID_COMM_HANDLE:
    case OPEN_PORT_ERROR:
    case PORT_ALREADY_OPEN:
    case GET_PORT_STATUS_FAIL:
    case SET_PORT_STATUS_FAIL:
    case READ_FROM_READER_ERROR:
    case WRITE_TO_READER_ERROR:
    case SET_PORT_BPS_ERROR:
    case STX_ERROR:
    case ETX_ERROR:
    case BCC_ERROR:
    case CMD_DATA_MORE_MAX_LEN:
    case DATA_ERROR:
    case ROTOCOL_TYPE_ERROR:
    case DEVICE_TYPE_ERROR:
    case ERROR_USB_CLASS_TYPE:
    case DEVICE_COMM_OR_CLOSE:
    case DEVICE_COMM_BUSY:
    case RECEIVE_DATA_LEN_ERROR:
    case CALL_LIBWLT_ERROR:
    case WLT_ERROR:
    case OPEN_FOLDER_FAIL:
    case FILE_NOT_EXIST:
    case TRACK_CARD_DATA:
    case UNKNOWN_CARD_TYPE:
    case NO_CARD:
    case CARD_NO_POWER_ON:
    case SWIPE_CARD_TIMEOUT:
    case SWIPE_TRACK_CARD_FAIL:
    case SWIPE_TRACK_CARD_NO_OPEN:
    case SEND_APDU_ERROR:
    default:
        nRetCode = ERR_IDC_OTHER;
        break;
    }

    return nRet;
}
