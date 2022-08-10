#include "DevSIU_IOMC.h"

static const char *ThisFile = "DevSIU_IOMC.cpp";
CSimpleMutex m_cMutex;
bool g_bExitThread = false;
//////////////////////////////////////////////////////////////////////////
extern "C" DEVSIU_EXPORT long CreateIDevSIU(LPCSTR lpDevType, IDevSIU *&pDev)
{
    pDev = new CDevSIU_IOMC(lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}

void *ControlNonContactFlash(void *pDev){
    CDevSIU_IOMC *pDevSiuIomc = (CDevSIU_IOMC *)pDev;
    int iStartIndex = (3 - 1) * 4;
    STR_DRV  stDrv;
    IOMCCOMMONOUTPUT stIomcCommonOutput;
    ZeroMemory(&stIomcCommonOutput, sizeof(stIomcCommonOutput));
    ZeroMemory(&stDrv, sizeof(stDrv));

    stIomcCommonOutput.wLen = 0x2A00;
    stIomcCommonOutput.wCNTLId = 0x0100;
    stIomcCommonOutput.WCNTLLen = 0x0400;
    stIomcCommonOutput.wCNTLCmd = 0x01A3;
    stIomcCommonOutput.wOutput1Id = 0x0107;
    stIomcCommonOutput.wOutput1Len = 0x2200;
    stIomcCommonOutput.byOutput1Data[iStartIndex] = 0x01;
    stIomcCommonOutput.byOutput1Data[iStartIndex + 1] = 0x0A;
    stIomcCommonOutput.byOutput1Data[iStartIndex + 2] = 0x0A;
    stIomcCommonOutput.byOutput1Data[iStartIndex + 3] = 0x01;

    stDrv.usParam          = USB_PRM_NO_RSP;                   // Parameter(Call type): No Resp STD40-00-00-01
    stDrv.uiDataInBuffSz   = 0x2C;                             // Input data size: 28byte
    stDrv.pvDataInBuffPtr  = &stIomcCommonOutput;              // Input data pointer
    stDrv.uiTimer          = 10;                               // Command time out

    while(true){
        long lRet = pDevSiuIomc->SendCmdData(USB_DRV_FN_DATASEND, &stDrv);
        if (lRet != 0)
        {
            return (void *)-1;
        }
        if(g_bExitThread){
            break;
        }
        CQtTime::Sleep(pDevSiuIomc->m_iFlashSleepTime);
    }

    return (void *)0;
}
//////////////////////////////////////////////////////////////////////////
CDevSIU_IOMC::CDevSIU_IOMC(LPCSTR lpDevType): m_cDev(lpDevType), m_cPWDev(lpDevType)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    memset(m_bLightSupport, FALSE, sizeof(m_bLightSupport));
    memset(m_bDoorsSupport, FALSE, sizeof(m_bDoorsSupport));
    memset(m_bIndicatorsSupport, FALSE, sizeof(m_bIndicatorsSupport));
    memset(m_bAuxiliariesSupport, FALSE, sizeof(m_bAuxiliariesSupport));
    m_tid = 0;
    m_ICReaderLEDCtrl = nullptr;                //30-00-00-00(FS#0012)
    m_ICReaderOpenUsbByFD = nullptr;            //30-00-00-00(FS#0012)
    m_ICReaderClose = nullptr;                  //30-00-00-00(FS#0012)
    m_lDevHdl = 0;                              //30-00-00-00(FS#0012)
}

CDevSIU_IOMC::~CDevSIU_IOMC()
{
    ExitNoncontactFlashThread();
    m_cDev.USBClose();
    m_cDev.USBDllRelease();
}

void CDevSIU_IOMC::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

long CDevSIU_IOMC::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    m_strOpenMode = lpMode;

    //读取配置项目

    if (!SetSupportStatus())
    {
        Log(ThisModule, __LINE__, "加载配置文件失败失败");
        return -1;
    }
    ResetAllStatus();

    // 加载USB库，并打开连接
    long lRet = m_cDev.USBDllLoad();
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "加载USB库失败: lRet=%d", lRet);
        return -1;
    }
    lRet = m_cDev.USBOpen(IOMC_DEVICE_NAME);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "打开连接失败: lRet=%d", lRet);
        return -1;
    }

    //30-00-00-00(FS#0012) add start
    if(m_iFIDCType == 1){
//        if(OpenTMZFidc() != 0){
//            return -1;
//        }
    }
    //30-00-00-00(FS#0012) add end
    // 打开电源管理
    /*m_cPWDev.SetFnATMUSB(m_cDev.GetFnATMUSB(), m_cDev.GetInfATMUSB());
    lRet = m_cPWDev.USBOpen(PWRSPLY_DEVICE_NAME);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "打开电源管理失败: lRet=%d", lRet);
        return -1;
    }*/

    return 0;
}

long CDevSIU_IOMC::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if(m_iFIDCType == 1){
//        CloseTMZFidc();                     //30-00-00-00(FS#0012)
    }

    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    ExitNoncontactFlashThread();
    long lRet = m_cDev.USBClose();
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "关闭连接失败: lRet=%d", lRet);
        return -1;
    }
    return 0;
}


long CDevSIU_IOMC::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    if(m_iFIDCType == 1){                           //30-00-00-00(FS#0012)
//        if(m_lDevHdl != 0){                         //30-00-00-00(FS#0012)
//            m_ICReaderLEDCtrl(m_lDevHdl, 0, 0);     //30-00-00-00(FS#0012)
//        }                                           //30-00-00-00(FS#0012)
    } else {                                        //30-00-00-00(FS#0012)
        ExitNoncontactFlashThread();
    }                                               //30-00-00-00(FS#0012)
    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x0800;                                  // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;               // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;             // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_RESET;                  // CMD

    long lRet = SendAndReadCmd(strIomc, &stRespInf, sizeof(stRespInf), ThisModule);
    if (lRet != 0 || stRespInf.wRESP_RES != LOBYTE(strIomc.wCNTL_CMD))
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d[%02X]", lRet, stRespInf.wRESP_RES);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    ResetAllStatus();
    return 0;
}

long CDevSIU_IOMC::GetDevInfo(char *pInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    // 获取版本信息
    IOMCBoardVerRespInfT stVer;
    long lRet = GetBoardVer(stVer);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "获取版本信息失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    sprintf(pInfo, "HTIOMC:%d%d%d%d%d%d%d%d-BVer=%d-SVer=%d", stVer.byDAte[0], stVer.byDAte[1], stVer.byDAte[2],
            stVer.byDAte[3], stVer.byDAte[4], stVer.byDAte[5], stVer.byDAte[6], stVer.byDAte[7],
            stVer.byBOARDVERSION, stVer.bySYSTEMVERSION);
    return 0;
}

long CDevSIU_IOMC::GetStatus(DEVSIUSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    CAutoCopyDevStatus<DEVSIUSTATUS> _auto(&stStatus, &m_stStatus);
    if (!m_cDev.IsOpen())
    {
        UpdateStatus(DEVICE_OFFLINE, 1);
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    if (0 != GetSensStatus())
    {
        // 故障时，保持故障前的状态
        UpdateStatus(DEVICE_HWERROR, 2);
        return -1;
    }

    UpdateDoorsStatus();
    UpdateSensorsStatus();
    UpdateAuxiliariesStatus();
    ResetNotSupportStatus();
    UpdateStatus(DEVICE_ONLINE, 0);
    return 0;
}

long CDevSIU_IOMC::SetDoors(WORD wDoors[DEFSIZE])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    return ERR_IOMC_NOT_SUPPORT;
}

long CDevSIU_IOMC::SetIndicators(WORD wIndicators[DEFSIZE])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    return ERR_IOMC_NOT_SUPPORT;
}

long CDevSIU_IOMC::SetAuxiliaries(WORD wAuxiliaries[DEFSIZE])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    return ERR_IOMC_NOT_SUPPORT;
}

long CDevSIU_IOMC::SetGuidLights(WORD wGuidLights[DEFSIZE])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    for (WORD i = 0; i < DEFSIZE; i++)
    {
        if (!m_bLightSupport[i])// 不支持
            continue;
        if (wGuidLights[i] == 0)// 不改变
            continue;

        if (0 != SetLightsCmd(i, wGuidLights[i]) < 0)
        {
            Log(ThisModule, __LINE__, "设置灯状态失败：GuidLights:%d,wCommand:%d", i, wGuidLights[i]);
            return ERR_IOMC_HARDWARE_ERROR;
        }

        // 更新灯状态
        m_stStatus.wGuidLights[i] = wGuidLights[i];
    }

    return ERR_IOMC_SUCCESS;
}


long CDevSIU_IOMC::GetFirmWareVer(char *pFwVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    DRV_VERtag  stDrvVer;
    IOMC_VERtag stIomcVer;
    long lRet = GetDrvVer(stDrvVer);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "读取驱动版本失败：lRet = %d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }
    lRet = GetIOMCVer(stIomcVer);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "读取固件版本失败：lRet = %d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    char szBuff[64] = {0};
    for (ushort i = 0; i < stDrvVer.wType; i++)
    {
        memcpy(szBuff, &stDrvVer.VerDt[i], sizeof(VERtag));
        sprintf(pFwVer + strlen(pFwVer), "DrvVer[%d]=%s|", i, szBuff);
    }
    for (ushort k = 0; k < stIomcVer.wType; k++)
    {
        memcpy(szBuff, &stIomcVer.VerDt[k], sizeof(VERtag));
        sprintf(pFwVer + strlen(pFwVer), "IomcVer[%d]=%s|", k, szBuff);
    }
    return 0;
}

long CDevSIU_IOMC::UpdateDevPDL()
{
    return PDL_NO_UPDATE;
}

// 设置数据
int CDevSIU_IOMC::SetData(unsigned short usType, void *vData)
{
    switch(usType)
    {
        default:
            break;
    }

    return ERR_IOMC_SUCCESS;
}



// 获取数据
int CDevSIU_IOMC::GetData(unsigned short usType, void *vData)
{
    switch(usType)
    {
        case GET_SKIMMING_CREADER:      // 获取异物检知(读卡器)
        {
            return GetCardReaderSkimming();
        }
        default:
            break;
    }

    return ERR_IOMC_SUCCESS;
}

long CDevSIU_IOMC::SendCmdData(WORD wCmd, PSTR_DRV pParam)
{
    return m_cDev.USBDrvCall(USB_DRV_FN_DATASEND, pParam);
}
//////////////////////////////////////////////////////////////////////////
long CDevSIU_IOMC::SetLightsCmd(WORD wID, WORD wCmd)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STIOMCFLICKERDATA stFlickerSetData;
    memset(&stFlickerSetData, IOMC_FLK_LED_NOCHANGE, sizeof(stFlickerSetData)); // Set "no change" status

    STR_DRV  strDrv;                               // for USB Driver
    STR_IOMC strIomc;                              // for IOMC parameter
    memset(&strDrv, 0x00, sizeof(STR_DRV)) ;       // USB Driver
    memset(&strIomc, 0x00, sizeof(STR_IOMC));      // IOMC parameter

    BYTE byNewStatus = IOMC_FLK_LED_NOCHANGE;
    switch (wCmd)
    {
    case IOMC_STATUS_NOCHANGE:                      // No change
        byNewStatus = IOMC_FLK_LED_NOCHANGE;
        break;
    case GUIDLIGHT_OFF:                     // Off
        byNewStatus = IOMC_FLK_LED_OFF;
        break;
    case GUIDLIGHT_SLOW_FLASH:              // Flash(slow)
        byNewStatus = IOMC_FLK_LED_FLASH_CYCLE_1;
        break;
    case GUIDLIGHT_MEDIUM_FLASH:                // Flash(medium)
        byNewStatus = IOMC_FLK_LED_FLASH_CYCLE_2;
        break;
    case GUIDLIGHT_QUICK_FLASH:             // Flash(quick)
        byNewStatus = IOMC_FLK_LED_FLASH_CYCLE_3;
        break;
    case GUIDLIGHT_CONTINUOUS:              // On
        byNewStatus = IOMC_FLK_LED_ON;
        break;
    default: return ERR_IOMC_NOT_SUPPORT;
    }
    // Set IOMC parameter
    switch (wID)
    {
    case GUIDLIGHT_CARDUNIT:                                    // Card unit
        stFlickerSetData.Flk[IOMC_FLK_ID_FL_CD] = byNewStatus;
        break;
    case GUIDLIGHT_PINPAD:                                  // PIN pad unit
        stFlickerSetData.Flk[IOMC_FLK_ID_LMP_EPP] = byNewStatus;
        break;
    case GUIDLIGHT_NOTESDISPENSER:                          // Note dispenser
        stFlickerSetData.Flk[IOMC_FLK_ID_LMP_CS] = byNewStatus;
        break;
    case GUIDLIGHT_RECEIPTPRINTER:                          // Receipt printer
        stFlickerSetData.Flk[IOMC_FLK_ID_FL_RT] = byNewStatus;
        break;
    case GUIDLIGHT_BILLACCEPTOR:                                // Note acceptor
        stFlickerSetData.Flk[IOMC_FLK_ID_LMP_CS] = byNewStatus;
        break;
    case GUIDLIGHT_PASSBOOKPRINTER:                     // Passbook unit
        stFlickerSetData.Flk[IOMC_FLK_ID_FL_PB] = byNewStatus;
        break;
    case GUIDLIGHT_FINGER:                                  // 指纹仪
        stFlickerSetData.Flk[IOMC_FLK_ID_FL_BAR] = byNewStatus;
        break;
    case GUIDLIGHT_NONCONTACT:
        return SetNonContactLightCmd(wCmd);
//        stFlickerSetData.Flk[IOMC_FLK_ID_ICRW_FLK] = byNewStatus;
        break;
    case GUIDLIGHT_SCANNER:                               // 扫描仪
    case GUIDLIGHT_CHEQUEUNIT:                            // Cheque unit
    case GUIDLIGHT_DOCUMENTPRINTER:                       // Document printer
    case GUIDLIGHT_COINDISPENSER:                         // Coin dispenser
    case GUIDLIGHT_ENVDEPOSITORY:                           // Envelope depository
    case GUIDLIGHT_COINACCEPTOR:                            // Coin Acceptor
    default:
        return ERR_IOMC_NOT_SUPPORT;
    }

    // Flicker Command
    strIomc.wLEN            = 0x1C00;                           // LEN(28byte)
    strIomc.wCNTL_ID        = EN_IOMC_CMDID_CMDCODE;            // CNTL ID  :command
    strIomc.wCNTL_LNG       = 0x0400;                           // LNG(4byte)
    strIomc.wCNTL_CMD       = EN_IOMC_CMD_FLICKER_CONTROL;      // CMD :Flicker control
    strIomc.wDATA_ID        = EN_IOMC_CMDID_FLKCTRL;            // DATA ID
    strIomc.wDATA_LNG       = EN_IOMC_PKTLNG_FLKCTRL;           // LNG
    memcpy(strIomc.byDATA, &stFlickerSetData, sizeof(stFlickerSetData));    // Copy input data .

    // Set USB driver structure
    strDrv.usParam          = USB_PRM_NO_RSP;                   // Parameter(Call type): No Resp STD40-00-00-01
    strDrv.uiDataInBuffSz   = HIBYTE(strIomc.wLEN);             // Input data size: 28byte
    strDrv.pvDataInBuffPtr  = &strIomc;                         // Input data pointer
    strDrv.uiTimer          = EN_IOMCTOUT_FLICKER_CONTROL;      // Command time out

    long lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASEND, &strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }
    return ERR_IOMC_SUCCESS;
}

long CDevSIU_IOMC::GetSensStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&m_stSensInfo, 0x00, sizeof(m_stSensInfo));

    // Sense Status
    strIomc.wLEN            = 0x0800;                                 // LEN
    strIomc.wCNTL_ID        = EN_IOMC_CMDID_CMDCODE;                  // CNTL ID  : Command
    strIomc.wCNTL_LNG       = 0x0400;                                 // LNG
    strIomc.wCNTL_CMD       = EN_IOMC_CMD_SENSE_STATUS_CONSTRAINT;    // CMD : Sense Status(Constraint)

    long lRet = SendAndReadCmd(strIomc, &m_stSensInfo, sizeof(m_stSensInfo), ThisModule);
    if (lRet != 0 || m_stSensInfo.wRESP_RES != LOBYTE(strIomc.wCNTL_CMD))
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d[%02X]", lRet, m_stSensInfo.wRESP_RES);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    return 0;
}

void CDevSIU_IOMC::UpdateDoorsStatus()
{
    THISMODULE(__FUNCTION__);
    bitset<8>  bit1(m_stSensInfo.bySEKKYAKU_SENSE_INF[0]);
    bitset<8>  bit2(m_stSensInfo.bySEKKYAKU_SENSE_INF[1]);
    bitset<8>  bit3(m_stSensInfo.bySEKKYAKU_SENSE_INF[2]);
    bitset<8>  bit4(m_stSensInfo.bySEKKYAKU_SENSE_INF[3]);
    MLog(ThisModule, 1, "SENSE状态：%s,%s,%s,%s",
         bit1.to_string().c_str(), bit2.to_string().c_str(), bit3.to_string().c_str(), bit4.to_string().c_str());
    if ((m_stSensInfo.bySEKKYAKU_SENSE_INF[INDEX_UNIT_SW] & MASK_SAFE_DOOR) == 0x00)
    {
        m_stStatus.wDoors[DOOR_SAFE] = DOOR_OPEN;
    }
    else
    {
        m_stStatus.wDoors[DOOR_SAFE] = DOOR_CLOSED;
    }

    if ((m_stSensInfo.bySEKKYAKU_SENSE_INF[INDEX_FRONT_DOOR] & MASK_FRONT_DOOR) == 0x00)
    {
        m_stStatus.wDoors[DOOR_CABINET_FRONT] = DOOR_OPEN;
    }
    else
    {
        m_stStatus.wDoors[DOOR_CABINET_FRONT] = DOOR_CLOSED;
    }
    if ((m_stSensInfo.bySEKKYAKU_SENSE_INF[INDEX_REAR_DOOR] & MASK_REAR_DOOR) == 0x00)
    {
        m_stStatus.wDoors[DOOR_CABINET_REAR] = DOOR_OPEN;
    }
    else
    {
        m_stStatus.wDoors[DOOR_CABINET_REAR] = DOOR_CLOSED;
    }

    if (!m_bFrontType)// 后面机
    {
        if ((m_stStatus.wDoors[DOOR_CABINET_FRONT] == DOOR_CLOSED &&
             m_stStatus.wDoors[DOOR_CABINET_REAR] == DOOR_CLOSED) ||
            (m_bOnlyRearDoor && m_stStatus.wDoors[DOOR_CABINET_REAR] == DOOR_CLOSED))// 只有后柜门传感器的后面机
        {
            m_stStatus.wDoors[DOOR_CABINET] = DOOR_CLOSED;
        }
        else
        {
            m_stStatus.wDoors[DOOR_CABINET] = DOOR_OPEN;
        }
    }
    else
    {
        m_stStatus.wDoors[DOOR_CABINET] = m_stStatus.wDoors[DOOR_CABINET_FRONT];
        m_stStatus.wDoors[DOOR_CABINET_REAR] = DOOR_CLOSED;// 前面机，后柜门一直是关的
    }
}

void CDevSIU_IOMC::UpdateSensorsStatus()
{
    if (m_stStatus.wDoors[DOOR_CABINET] == DOOR_CLOSED)
    {
        m_stStatus.wSensors[SENSORS_OPERATORSWITCH] = SENSORS_RUN;
    }
    else
    {
        m_stStatus.wSensors[SENSORS_OPERATORSWITCH] = SENSORS_SUPERVISOR;
    }

    if ((m_stSensInfo.bySEKKYAKU_SENSE_INF[INDEX_HEAT_SENS_LATCH] & MASK_HEAT_SENS_LATCH) == 0x00)
    {
        m_stStatus.wSensors[SENSORS_HEAT] = SENSORS_OFF;
    }
    else
    {
        m_stStatus.wSensors[SENSORS_HEAT] = SENSORS_ON;
    }
    if ((m_stSensInfo.bySEKKYAKU_SENSE_INF[INDEX_SEIS_SENS_LATCH] & MASK_SEIS_SENS_LATCH) == 0x00)
    {
        m_stStatus.wSensors[SENSORS_SEISMIC] = SENSORS_OFF;
    }
    else
    {
        m_stStatus.wSensors[SENSORS_SEISMIC] = SENSORS_ON;
    }

    if ((m_stSensInfo.bySEKKYAKU_SENSE_INF[INDEX_HEADSET_SENS] & MASK_HEADSET_SENS) == 0x00)
    {
        m_stStatus.wSensors[SENSORS_ENHANCEDAUDIO] = SENSORS_ON;
    }
    else
    {
        m_stStatus.wSensors[SENSORS_ENHANCEDAUDIO] = SENSORS_OFF;
    }
}

void CDevSIU_IOMC::UpdateAuxiliariesStatus()
{

}


bool CDevSIU_IOMC::SetSupportStatus()
{
    THISMODULE(__FUNCTION__);
    m_iFIDCType = 0;                        //30-00-00-00(FS#0012)

    QString strINIFile("SIUConfig.ini");
#ifdef QT_WIN32
    strINIFile.prepend("C:/CFES/ETC/");
#else
    strINIFile.prepend("/usr/local/CFES/ETC/");
#endif
    if (!m_cINI.LoadINIFile(strINIFile.toLocal8Bit().data()))
    {
        Log(ThisModule, __LINE__, "加载配置文件失败:%s", strINIFile.toLocal8Bit().data());
        return false;
    }
    CINIReader cINI = m_cINI.GetReaderSection("SIUInfo");
    m_bFrontType = (bool)cINI["FrontType"];
    m_bOnlyRearDoor = (bool)cINI["OnlyRearDoor"];
    QByteArray vtSensor = (LPCSTR)cINI["SensorsSupport"];
    QByteArray vtDoor = (LPCSTR)cINI["DoorsSupport"];
    QByteArray vtIndicator = (LPCSTR)cINI["IndicatorsSupport"];
    QByteArray vtAuxiliarie = (LPCSTR)cINI["AuxiliariesSupport"];
    QByteArray vtGuidLight = (LPCSTR)cINI["GuidLightsSupport"];
    if (vtSensor.length() != DEFSIZE || vtDoor.length() != DEFSIZE || vtIndicator.length() != DEFSIZE ||
        vtAuxiliarie.length() != DEFSIZE || vtGuidLight.length() != DEFSIZE)
    {
        Log(ThisModule, __LINE__, "配置支持数据长度不对:%s", strINIFile.toLocal8Bit().data());
        return false;
    }
    for (int i = 0; i < vtSensor.size() && i < DEFSIZE; i++)
    {
        m_bSensorsSupport[i] = (vtSensor.at(i) == '1') ? TRUE : FALSE;
    }
    for (int i = 0; i < vtDoor.size() && i < DEFSIZE; i++)
    {
        m_bDoorsSupport[i] = (vtDoor.at(i) == '1') ? TRUE : FALSE;
    }
    for (int i = 0; i < vtIndicator.size() && i < DEFSIZE; i++)
    {
        m_bIndicatorsSupport[i] = (vtIndicator.at(i) == '1') ? TRUE : FALSE;
    }
    for (int i = 0; i < vtAuxiliarie.size() && i < DEFSIZE; i++)
    {
        m_bAuxiliariesSupport[i] = (vtAuxiliarie.at(i) == '1') ? TRUE : FALSE;
    }
    for (int i = 0; i < vtGuidLight.size() && i < DEFSIZE; i++)
    {
        m_bLightSupport[i] = (vtGuidLight.at(i) == '1') ? TRUE : FALSE;
    }

    m_iSlowFlashSleepTime = (int)cINI.GetValue("SlowFlashSleepTime", 1000);
    m_iMediumFlashSleepTime = (int)cINI.GetValue("MediumFlashSleepTime", 600);
    m_iQuickFlashSleepTime = (int)cINI.GetValue("QuickFlashSleepTime", 300);
    m_iFIDCType = (int)cINI.GetValue("FIDCType", 0);         //30-00-00-00(FS#0012)
    if(m_iFIDCType < 0 || m_iFIDCType > 1){                  //30-00-00-00(FS#0012)
        m_iFIDCType = 0;                                     //30-00-00-00(FS#0012)
    }                                                        //30-00-00-00(FS#0012)
    return true;
}

void CDevSIU_IOMC::ResetAllStatus()
{
    m_stStatus.clear();
    for (int i = 0; i < DEFSIZE; i++)
    {
        if (m_bSensorsSupport[i]) m_stStatus.wSensors[i] = SENSORS_OFF;
        if (m_bDoorsSupport[i]) m_stStatus.wDoors[i] = DOOR_CLOSED;
        if (m_bIndicatorsSupport[i]) m_stStatus.wIndicators[i]  = INDICATOR_OFF;
        if (m_bAuxiliariesSupport[i]) m_stStatus.wAuxiliaries[i] = AUXILIARIE_OFF;
        if (m_bLightSupport[i]) m_stStatus.wGuidLights[i] = GUIDLIGHT_OFF;

        if (!m_bSensorsSupport[i]) m_stStatus.wSensors[i] = 0;
        if (!m_bDoorsSupport[i]) m_stStatus.wDoors[i] = 0;
        if (!m_bIndicatorsSupport[i]) m_stStatus.wIndicators[i] = 0;
        if (!m_bAuxiliariesSupport[i]) m_stStatus.wAuxiliaries[i] = 0;
        if (!m_bLightSupport[i]) m_stStatus.wGuidLights[i] = 0;
    }
}

void CDevSIU_IOMC::ResetNotSupportStatus()
{
    for (int i = 0; i < DEFSIZE; i++)
    {
        if (!m_bSensorsSupport[i]) m_stStatus.wSensors[i] = 0;
        if (!m_bDoorsSupport[i]) m_stStatus.wDoors[i] = 0;
        if (!m_bIndicatorsSupport[i]) m_stStatus.wIndicators[i] = 0;
        if (!m_bAuxiliariesSupport[i]) m_stStatus.wAuxiliaries[i] = 0;
        if (!m_bLightSupport[i]) m_stStatus.wGuidLights[i] = 0;
    }
}

void CDevSIU_IOMC::UpdateStatus(WORD wDevice, long lErrCode)
{
    m_stStatus.wDevice = wDevice;
    sprintf(m_stStatus.szErrCode, "0%02X", qAbs(lErrCode));
}

long CDevSIU_IOMC::GetBoardVer(IOMCBoardVerRespInfT &stBoardVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    // 获取版本信息
    STR_IOMC strIomc;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stBoardVer, 0x00, sizeof(IOMCBoardVerRespInfT));

    // Sense Status
    strIomc.wLEN = 0x0800;                                  // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;               // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;             // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_DATA_READ;              // CMD

    long lRet = SendAndReadCmd(strIomc, &stBoardVer, sizeof(IOMCBoardVerRespInfT), ThisModule);
    if (lRet != 0 || stBoardVer.wRESP_RES != LOBYTE(strIomc.wCNTL_CMD))
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d[%02X]", lRet, stBoardVer.wRESP_RES);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    return 0;
}

long CDevSIU_IOMC::GetDrvVer(DRV_VERtag &stDrvVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    // 获取版本信息
    VERtag stVer;
    STR_DRV strDrv;
    BYTE byVRTBuff[64] = {0};
    memset(&strDrv, 0x00, sizeof(STR_DRV));

    // USB driver parameter
    strDrv.usParam = USB_PRM_VRT;                       // Param: usually
    strDrv.pvDataOutBuffPtr = byVRTBuff;                // Output data pointer
    strDrv.uiDataOutReqSz = sizeof(byVRTBuff);          // Output data area size
    long lRet = m_cDev.USBDrvCall(USB_DRV_INF_INFGET, &strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    memset(&stDrvVer, 0x00, sizeof(DRV_VERtag));
    VERtag *lpVer = (VERtag *)&byVRTBuff[0];
    while (lpVer->NAME[0] != NULL)
    {
        memcpy(stDrvVer.VerDt[stDrvVer.wType++].NAME, lpVer, sizeof(VERtag)) ;
        if (stDrvVer.wType >= sizeof(stDrvVer.VerDt) / sizeof(stDrvVer.VerDt[0]))
            break;
        lpVer++;
    }
    return 0;
}

long CDevSIU_IOMC::GetIOMCVer(IOMC_VERtag &stIOMCVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    // 获取版本信息
    STR_IOMC strIomc;
    IOMCVrtInfT  stVerInfo;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stVerInfo, 0x00, sizeof(IOMCVrtInfT));

    // Sense Status
    strIomc.wLEN = 0x0800;                                  // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;               // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;             // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_VERSION_SENSE;          // CMD

    long lRet = SendAndReadCmd(strIomc, &stVerInfo, sizeof(IOMCVrtInfT), ThisModule);
    if (lRet != 0 || stVerInfo.wRESP_RES != LOBYTE(strIomc.wCNTL_CMD))
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d[%02X]", lRet, stVerInfo.wRESP_RES);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    memset(&stIOMCVer, 0x00, sizeof(IOMC_VERtag));
    ushort uVerNum = sizeof(stVerInfo.VRT) / sizeof(stVerInfo.VRT[0]);
    for (ushort i = 0; i < uVerNum; i++)
    {
        memcpy(stIOMCVer.VerDt[i].NAME, &stVerInfo.VRT[i].cKatashiki[8], 7);    //STD40-01-00-00
        memcpy(stIOMCVer.VerDt[i].byV, &stVerInfo.VRT[i].cVER[2], 6);           //STD40-00-00-01
        stIOMCVer.wType++;
    }

    return 0;
}

long CDevSIU_IOMC::SetNonContactLightCmd(WORD wCmd)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_iFIDCType == 1){
//        if(m_ICReaderLEDCtrl == nullptr || m_lDevHdl == 0){
//            return ERR_IOMC_HARDWARE_ERROR;
//        } else {
//            int iRet;
//            switch(wCmd){
//            case IOMC_STATUS_NOCHANGE:
//                break;
//            case GUIDLIGHT_OFF:
//                iRet = m_ICReaderLEDCtrl(m_lDevHdl, 0, 0);
//                break;
//            case GUIDLIGHT_CONTINUOUS:
//                iRet = m_ICReaderLEDCtrl(m_lDevHdl, 1, 0);
//                break;
//            case GUIDLIGHT_SLOW_FLASH:
//                iRet = m_ICReaderLEDCtrl(m_lDevHdl, 2, m_iSlowFlashSleepTime/50);
//                break;
//            case GUIDLIGHT_MEDIUM_FLASH:
//                iRet = m_ICReaderLEDCtrl(m_lDevHdl, 2, m_iMediumFlashSleepTime/50);
//                break;
//            case GUIDLIGHT_QUICK_FLASH:
//                iRet = m_ICReaderLEDCtrl(m_lDevHdl, 2, m_iQuickFlashSleepTime/50);
//                break;
//            default:
//                break;
//            }
//            if(iRet != 0){
//                return ERR_IOMC_HARDWARE_ERROR;
//            }
//        }
    } else {
        ExitNoncontactFlashThread();

        switch(wCmd){
        case IOMC_STATUS_NOCHANGE:
            break;
        case GUIDLIGHT_OFF:
        case GUIDLIGHT_CONTINUOUS:
        {
            int iStartIndex = (3 - 1) * 4;
            STR_DRV  stDrv;
            IOMCCOMMONOUTPUT stIomcCommonOutput;
            ZeroMemory(&stDrv, sizeof(stDrv));
            ZeroMemory(&stIomcCommonOutput, sizeof(stIomcCommonOutput));

            stIomcCommonOutput.wLen = 0x2A00;
            stIomcCommonOutput.wCNTLId = 0x0100;
            stIomcCommonOutput.WCNTLLen = 0x0400;
            stIomcCommonOutput.wCNTLCmd = 0x01A3;
            stIomcCommonOutput.wOutput1Id = 0x0107;
            stIomcCommonOutput.wOutput1Len = 0x2200;
            stIomcCommonOutput.byOutput1Data[iStartIndex] = wCmd == GUIDLIGHT_OFF ? 0x02 : 0x01;

            stDrv.usParam          = USB_PRM_NO_RSP;                   // Parameter(Call type): No Resp STD40-00-00-01
            stDrv.uiDataInBuffSz   = 0x2C;                             // Input data size: 28byte
            stDrv.pvDataInBuffPtr  = &stIomcCommonOutput;              // Input data pointer
            stDrv.uiTimer          = 10;                               // Command time out

            long lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASEND, &stDrv);
            if (lRet != 0)
            {
                Log(ThisModule, __LINE__, "非接灯执行命令失败：wCmd:%d,lRet=%d", wCmd, lRet);
                return ERR_IOMC_HARDWARE_ERROR;
            }
            return ERR_IOMC_SUCCESS;
        }
            break;
        case GUIDLIGHT_SLOW_FLASH:
        case GUIDLIGHT_MEDIUM_FLASH:
        case GUIDLIGHT_QUICK_FLASH:
        {
            m_iFlashSleepTime = m_iSlowFlashSleepTime;
            if(wCmd == GUIDLIGHT_MEDIUM_FLASH){
                m_iFlashSleepTime = m_iMediumFlashSleepTime;
            } else if(wCmd == GUIDLIGHT_QUICK_FLASH){
                m_iFlashSleepTime = m_iQuickFlashSleepTime;

            }
            g_bExitThread = false;
            int iRet = pthread_create(&m_tid, NULL, ControlNonContactFlash, this);
            if(iRet != 0){
                Log(ThisModule, __LINE__, "控制非接灯闪烁时线程创建失败：iRet=%d", iRet);
                return ERR_IOMC_HARDWARE_ERROR;
            }
        }
            break;
        default:
            break;
        }
    }

    return ERR_IOMC_SUCCESS;
}

void CDevSIU_IOMC::ExitNoncontactFlashThread()
{
    if(m_tid != 0){
        g_bExitThread = true;
        pthread_join(m_tid, NULL);
        m_tid = 0;
    }
}

long CDevSIU_IOMC::SendAndReadCmd(const STR_IOMC &strIomc, LPVOID lpRespInf, UINT uRespSize, const char *ThisModule)
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    STR_DRV  strDrv;
    STR_IOMC stData;
    memset(&strDrv, 0x00, sizeof(STR_DRV));
    memcpy(&stData, &strIomc, sizeof(STR_IOMC));

    // USB driver parameter
    strDrv.usParam = USB_PRM_USUALLY;                       // Param: usually
    strDrv.uiDataInBuffSz = HIBYTE(stData.wLEN);            // Input data size
    strDrv.pvDataInBuffPtr = &stData;                       // Input data pointer
    strDrv.pvDataOutBuffPtr = lpRespInf;                    // Output data pointer
    strDrv.uiDataOutReqSz = uRespSize;                      // Output data area size
    strDrv.uiTimer = EN_IOMCTOUT_DATA_READ;                 // Timeout

    return m_cDev.USBDrvCall(USB_DRV_FN_DATASENDRCV, &strDrv);;
}

//30-00-00-00(FS#0012)
long CDevSIU_IOMC::CloseTMZFidc()
{
    if(m_lDevHdl != 0 && m_ICReaderClose){
        m_ICReaderClose(m_lDevHdl);
        m_lDevHdl = 0;
    }
    if(m_cLibrary.isLoaded()){
        m_cLibrary.unload();
        m_ICReaderOpenUsbByFD = nullptr;
        m_ICReaderLEDCtrl = nullptr;
        m_ICReaderClose = nullptr;
    }

    return 0;
}

// 获取读卡器异物检知结果
INT CDevSIU_IOMC:: GetCardReaderSkimming()
{
    THISMODULE(__FUNCTION__);

    BYTE	byDataInLight[44] = { 0x00 };    //Sensor发光命令入参
    byDataInLight[1] = 0x2C;        // Size [0][1]
    byDataInLight[3] = 0x01;        // Cntl ID [2][3]
    byDataInLight[5] = 0x04;        // Cntl Len [4][5]
    byDataInLight[6] = 0xA3;        // Cntl CNTL [6][7]
    byDataInLight[7] = 0x01;        //10ms单位指定  //IOMC改善后，修改为10ms单位	//40-01-00-05(UT#00005)
    byDataInLight[8] = 0x07;        // Output Data ID[8][9]
    byDataInLight[9] = 0x01;        // Output Data ID[8][9]
    byDataInLight[11] = 0x22;       // Output Data LNG [10][11]
    byDataInLight[28] = 0x01;       // Port5  Data   出力开始
    byDataInLight[29] = 0x03;       // Port5  Data 出力Port,ON时间,单位:10ms (00时:循环的次数无视) 30ms指定 //40-01-00-05(UT#00005)
    byDataInLight[30] = 0x00;       // Port5  Data 出力Port,OFF时间
    byDataInLight[31] = 0x01;       // 循环次数(ON,OFF循环次数)

    BYTE bySenseGetInput[8] = {0};  // 获取Sensor信息命令入参
    bySenseGetInput[0] = 0x00;      // LEN　2バイト
    bySenseGetInput[1] = 0x08;      //
    bySenseGetInput[2] = 0x00;      // ID　2バイト
    bySenseGetInput[3] = 0x01;      //
    bySenseGetInput[4] = 0x00;      // LNG　2バイト
    bySenseGetInput[5] = 0x04;      //
    bySenseGetInput[6] = 0x0C;      // CMD　2バイト
    bySenseGetInput[7] = 0x01;      //

    STR_DRV      stDrvParam;
    IOMCRESPINFO stIomcRespInfoLight ;	//Sensor发光应答结构体
    IOMCSNSINFO	 stIomcSnsInfo ;        //Sensor信息应答结构体
    long         lRet = 0;

    // 发送SIU发光命令
    memset(&stDrvParam, 0, sizeof(stDrvParam));
    memset(&stIomcRespInfoLight, 0, sizeof(stIomcRespInfoLight));
    stDrvParam.usParam = USB_PRM_USUALLY;
    stDrvParam.pvDataInBuffPtr = byDataInLight;
    stDrvParam.uiDataInBuffSz = sizeof(byDataInLight);
    stDrvParam.pvDataOutBuffPtr = &stIomcRespInfoLight;
    stDrvParam.uiDataOutReqSz = sizeof(stIomcRespInfoLight);
    stDrvParam.uiTimer = 30;
    lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASENDRCV, &stDrvParam);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "Sensor发光命令执行失败:%d", lRet);
        return -1;
    } else
    {
        // 获取Sensor状态
        memset(&stIomcSnsInfo, 0, sizeof(stIomcSnsInfo));
        memset(&stDrvParam, 0, sizeof(stDrvParam));
        stDrvParam.usParam = USB_PRM_USUALLY;
        stDrvParam.pvDataInBuffPtr = bySenseGetInput;
        stDrvParam.uiDataInBuffSz = sizeof(bySenseGetInput);
        stDrvParam.pvDataOutBuffPtr = &stIomcSnsInfo;
        stDrvParam.uiDataOutReqSz = sizeof(stIomcSnsInfo);
        stDrvParam.uiTimer = 30;
        lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASENDRCV, &stDrvParam);
        if(lRet != 0)
        {
            Log(ThisModule, __LINE__, "Sensor状态命令执行失败:%d", lRet);
            return -2;
        } else
        {
            // 判断异物状态
            if((stIomcSnsInfo.byAppendInfo[4 + 4] & 0x10) == 0)
            {
                return 1;
            } else
            {
                return 0;
            }
        }
    }

    return 0;
}

int CDevSIU_IOMC::SetFlickerLed(int iFlickerLedIdx, int iAction)
{
    THISMODULE(__FUNCTION__);
    STIOMCFLICKERDATA stFlickerSetData;
    memset(&stFlickerSetData, IOMC_FLK_LED_NOCHANGE, sizeof(stFlickerSetData)); // Set "no change" status

    STR_DRV  strDrv;                               // for USB Driver
    STR_IOMC strIomc;                              // for IOMC parameter
    memset(&strDrv, 0x00, sizeof(STR_DRV)) ;       // USB Driver
    memset(&strIomc, 0x00, sizeof(STR_IOMC));      // IOMC parameter

    BYTE byNewStatus = ((iAction == 0) ? IOMC_FLK_LED_OFF : IOMC_FLK_LED_ON);
    stFlickerSetData.Flk[iFlickerLedIdx] = byNewStatus;

    // Flicker Command
    strIomc.wLEN            = 0x1C00;                           // LEN(28byte)
    strIomc.wCNTL_ID        = EN_IOMC_CMDID_CMDCODE;            // CNTL ID  :command
    strIomc.wCNTL_LNG       = 0x0400;                           // LNG(4byte)
    strIomc.wCNTL_CMD       = EN_IOMC_CMD_FLICKER_CONTROL;      // CMD :Flicker control
    strIomc.wDATA_ID        = EN_IOMC_CMDID_FLKCTRL;            // DATA ID
    strIomc.wDATA_LNG       = EN_IOMC_PKTLNG_FLKCTRL;           // LNG
    memcpy(strIomc.byDATA, &stFlickerSetData, sizeof(stFlickerSetData));    // Copy input data .

    // Set USB driver structure
    strDrv.usParam          = USB_PRM_NO_RSP;                   // Parameter(Call type): No Resp STD40-00-00-01
    strDrv.uiDataInBuffSz   = HIBYTE(strIomc.wLEN);             // Input data size: 28byte
    strDrv.pvDataInBuffPtr  = &strIomc;                         // Input data pointer
    strDrv.uiTimer          = EN_IOMCTOUT_FLICKER_CONTROL;      // Command time out

    long lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASEND, &strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    return ERR_IOMC_SUCCESS;
}

int CDevSIU_IOMC::SetSkimLed(int iFlickerLedIdx, int iAction)
{
    THISMODULE(__FUNCTION__);
    int iStartIndex = iFlickerLedIdx * 4;
    STR_DRV  stDrv;
    IOMCCOMMONOUTPUT stIomcCommonOutput;
    ZeroMemory(&stDrv, sizeof(stDrv));
    ZeroMemory(&stIomcCommonOutput, sizeof(stIomcCommonOutput));

    stIomcCommonOutput.wLen = 0x2A00;
    stIomcCommonOutput.wCNTLId = 0x0100;
    stIomcCommonOutput.WCNTLLen = 0x0400;
    stIomcCommonOutput.wCNTLCmd = 0x01A3;
    stIomcCommonOutput.wOutput1Id = 0x0107;
    stIomcCommonOutput.wOutput1Len = 0x2200;
    stIomcCommonOutput.byOutput1Data[iStartIndex] = (iAction == 0) ? 0x02 : 0x01;

    stDrv.usParam          = USB_PRM_NO_RSP;                   // Parameter(Call type): No Resp STD40-00-00-01
    stDrv.uiDataInBuffSz   = 0x2C;                             // Input data size: 28byte
    stDrv.pvDataInBuffPtr  = &stIomcCommonOutput;              // Input data pointer
    stDrv.uiTimer          = 10;                               // Command time out

    long lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASEND, &stDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "非接灯执行命令失败：wCmd:%d,lRet=%d", iAction, lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }
    return ERR_IOMC_SUCCESS;
}

//30-00-00-00(FS#0012)
long CDevSIU_IOMC::OpenTMZFidc()
{
    THISMODULE(__FUNCTION__);
    string strDllFile = "/usr/local/CFES/lib/libtmz.so";
    m_cLibrary.setFileName(strDllFile.c_str());
    if(!m_cLibrary.load()){
        Log(ThisModule, __LINE__, "%s load fail[%s]!", strDllFile.c_str(), m_cLibrary.errorString().toStdString().c_str());
        return -1;
    }
    m_ICReaderLEDCtrl = (FNICReaderLEDCtrl)m_cLibrary.resolve("ICReaderLEDCtrl");
    m_ICReaderOpenUsbByFD = (FNICReaderOpenUsbByFD)m_cLibrary.resolve("ICReaderOpenUsbByFD");
    m_ICReaderClose = (FNICReaderClose)m_cLibrary.resolve("ICReaderClose");
    if(m_ICReaderLEDCtrl == nullptr ||
       m_ICReaderOpenUsbByFD == nullptr || m_ICReaderClose == nullptr){
        Log(ThisModule, __LINE__, "Interface ICReaderLEDCtrl|ICReaderOpenUsbByFD|ICReaderClose get fail!");
        return -1;
    }

    m_lDevHdl = m_ICReaderOpenUsbByFD(0);
    if(m_lDevHdl == 0){
        Log(ThisModule, __LINE__, "ICReaderOpenUsbByFD fail!");
        return -1;
    }

    return 0;
}
