#include "DevSIU_CFES.h"

static const char *ThisFile = "DevSIU_CFES.cpp";
static CSimpleMutex m_cMutex;
static bool g_bExitThread = false;
//////////////////////////////////////////////////////////////////////////

void *ControlSKMLampFlash(void *pDev){
    CDevSIU_CFES *pDevSiuIomc = (CDevSIU_CFES *)pDev;
    while(true){
        for(int i = 0; i < SKM_LAMP_MAX; i++){
            LPSKMLAMPFLASH lpLamp = &(pDevSiuIomc->m_stSkmLampFlash[i]);
            if(lpLamp->iOnTime > 0 && lpLamp->iOffTime > 0){
                ULONG curTime = CQtTime::GetSysTick();
                int iInterval = lpLamp->bLedOn ? lpLamp->iOnTime : lpLamp->iOffTime ;
                if(lpLamp->ulLastOnOffTime == 0 || (curTime - lpLamp->ulLastOnOffTime > iInterval)){
                    lpLamp->bLedOn = !lpLamp->bLedOn;

                    int iStartIndex = (lpLamp->byPortNum - 1) * 4;
                    STR_DRV  stDrv;
                    IOMCCOMMONOUTPUT stIomcCommonOutput;
                    ZeroMemory(&stIomcCommonOutput, sizeof(stIomcCommonOutput));
                    ZeroMemory(&stDrv, sizeof(stDrv));

                    stIomcCommonOutput.wLen = 0x3400;
                    stIomcCommonOutput.wCNTLId = 0x0100;
                    stIomcCommonOutput.WCNTLLen = 0x0400;
                    stIomcCommonOutput.wCNTLCmd = 0x00A3;
                    stIomcCommonOutput.wOutput1Id = 0x0107;
                    stIomcCommonOutput.wOutput1Len = 0x2200;
                    stIomcCommonOutput.byOutput1Data[iStartIndex] = (lpLamp->bLedOn ? 0x01 : 0x02);

                    stIomcCommonOutput.wOutput2Id = 0x0307;
                    stIomcCommonOutput.wOutput2Len = 0x0600;

                    stDrv.usParam          = USB_PRM_NO_RSP;                   // Parameter(Call type): No Resp STD40-00-00-01
                    stDrv.uiDataInBuffSz   = 0x34;                             // Input data size: 28byte
                    stDrv.pvDataInBuffPtr  = &stIomcCommonOutput;              // Input data pointer
                    stDrv.uiTimer          = 10;                               // Command time out

                    pDevSiuIomc->SendCmdData(USB_DRV_FN_DATASEND, &stDrv);

                    //更新最后一次亮灯时间
                    lpLamp->ulLastOnOffTime = curTime;
                }
            }
        }

        //线程是否退出
        if(g_bExitThread){
            break;
        }
        CQtTime::Sleep(10);
    }

    return (void *)0;
}
//////////////////////////////////////////////////////////////////////////
CDevSIU_CFES::CDevSIU_CFES(LPCSTR lpDevType) : m_strDevType(lpDevType),
                                               m_cDev(lpDevType, IOMC_TYPE_CFES),
                                               m_cIOMCShareMemoryS(IOMC_SEND_SHARE_MEMORY_MUTEX_NAME),
                                               m_cIOMCShareMemoryR(IOMC_RECV_SHARE_MEMORY_MUTEX_NAME),
                                               m_cIOMCProcMutex(IOMC_PROC_MUTEX_NAME)
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

	 memset(&m_stSkmLampFlash, 0, sizeof(m_stSkmLampFlash));
    for(int i = 0; i < SKM_LAMP_MAX; i++){
        m_stSkmLampFlash[i].byPortNum = i + 1;
    }
	
    memset(m_byRecvBuff, 0, sizeof(m_byRecvBuff));
    memset(m_bySendBuff, 0, sizeof(m_bySendBuff));
}

CDevSIU_CFES::~CDevSIU_CFES()
{
    ExitSKMLampFlashThread();
    if(0){
        m_cDev.USBClose();
        m_cDev.USBDllRelease();
    }
}

void CDevSIU_CFES::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return;
}

long CDevSIU_CFES::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    m_strOpenMode = lpMode;
    long lRet = ERR_IOMC_SUCCESS;

    //读取配置项目
    if (!SetSupportStatus())
    {
        Log(ThisModule, __LINE__, "加载配置文件失败失败");
        return -1;
    }
    ResetAllStatus();

    if(!m_cIOMCShareMemoryS.IsOpened()){
    	if(!m_cIOMCShareMemoryS.Open(IOMC_SEND_SHARE_MEMORY_KEY_NAME)){
    		Log(__FUNCTION__, __LINE__, "IOMC 发送用共享内存[%s]打开失败", IOMC_SEND_SHARE_MEMORY_KEY_NAME);
    		return -1;
    	}
    }

    if(!m_cIOMCShareMemoryR.IsOpened()){
    	if(!m_cIOMCShareMemoryR.Open(IOMC_RECV_SHARE_MEMORY_KEY_NAME)){
    		Log(__FUNCTION__, __LINE__, "IOMC 接收用共享内存[%s]打开失败", IOMC_RECV_SHARE_MEMORY_KEY_NAME);
    		return -1;
    	}
    }    
    //设置闪烁循环模式参数
    SetFlashRecycleParamer();           //40-00-00-00(FT#0001)

    //启动SKM灯闪烁处理线程
    lRet = pthread_create(&m_tid, NULL, ControlSKMLampFlash, this);
    if(lRet != 0){
        Log(ThisModule, __LINE__, "控制SKM灯闪烁线程创建失败：iRet=%d", lRet);
        return -1;
    }

    if(m_iFIDCType == 1){
//        if(OpenTMZFidc() != 0){
//            return -1;
//        }
    }

    return 0;
}

long CDevSIU_CFES::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if(m_iFIDCType == 1){
//        CloseTMZFidc();                     //30-00-00-00(FS#0012)
    }  


    ExitSKMLampFlashThread();
    //关闭所有灯
    Reset();

    return ERR_IOMC_SUCCESS;
}


long CDevSIU_CFES::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    if(m_iFIDCType == 1){                           //30-00-00-00(FS#0012)
//        if(m_lDevHdl != 0){                         //30-00-00-00(FS#0012)
//            m_ICReaderLEDCtrl(m_lDevHdl, 0, 0);     //30-00-00-00(FS#0012)
//        }                                           //30-00-00-00(FS#0012)
    } else {                                        //30-00-00-00(FS#0012)
        //关闭SKM灯闪烁
    	for(int i = 0; i < SKM_LAMP_MAX; i++){
        	if(m_stSkmLampFlash[i].iOnTime > 0 && m_stSkmLampFlash[i].iOffTime > 0){
            	m_stSkmLampFlash[i].iOffTime = 0;
            	m_stSkmLampFlash[i].iOnTime = 0;
            	SetSKMLampCmd(GUIDLIGHT_OFF, i + 1);
        	}
    	}
    }                                               //30-00-00-00(FS#0012)

    memset(m_bySendBuff, 0, sizeof(m_bySendBuff));
    memset(m_byRecvBuff, 0, sizeof(m_byRecvBuff));
    int iOffset = 0;
    UINT uiRecvLen = sizeof(m_byRecvBuff);

    // Sense Status
    iOffset += 2;               //总长度预留
    WORD wCmdId = EN_IOMC_CMD_RESET;
    iOffset += AddPkgData(m_bySendBuff + 2, EN_IOMC_CMDID_CMDCODE, (LPBYTE)&wCmdId, sizeof(wCmdId));

    m_bySendBuff[0] = HIWORD(iOffset);
    m_bySendBuff[1] = LOWORD(iOffset);

    long lRet = SendAndReadCmd(m_bySendBuff, iOffset, m_byRecvBuff, uiRecvLen);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    ResetAllStatus();
    return 0;
}

long CDevSIU_CFES::GetDevInfo(char *pInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    // 获取版本信息
    VERSIONINFO stVer;
    memset(&stVer, 0, sizeof(stVer));
    long lRet = GetVersion(stVer);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "获取版本信息失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    strcat(pInfo, "HWVer:");
    for(int i = 0; i < sizeof(stVer.stPartVerInfo)/sizeof(stVer.stPartVerInfo[0]); i++){
        if(stVer.stPartVerInfo[i].byType[0] == 0x00){
            continue;
        }
        strcat(pInfo, (char *)stVer.stPartVerInfo[i].byType);
        strcat(pInfo, " ");
    }

    return 0;
}

long CDevSIU_CFES::GetStatus(DEVSIUSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    CAutoCopyDevStatus<DEVSIUSTATUS> _auto(&stStatus, &m_stStatus);
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
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

long CDevSIU_CFES::SetDoors(WORD wDoors[DEFSIZE])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    return ERR_IOMC_NOT_SUPPORT;
}

long CDevSIU_CFES::SetIndicators(WORD wIndicators[DEFSIZE])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    return ERR_IOMC_NOT_SUPPORT;
}

long CDevSIU_CFES::SetAuxiliaries(WORD wAuxiliaries[DEFSIZE])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    return ERR_IOMC_NOT_SUPPORT;
}

long CDevSIU_CFES::SetGuidLights(WORD wGuidLights[DEFSIZE])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
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


long CDevSIU_CFES::GetFirmWareVer(char *pFwVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    //TBD:
    return 0;
}

long CDevSIU_CFES::UpdateDevPDL()
{
    return PDL_NO_UPDATE;
}

#if defined(SPBuild_OPTIM)
// 设置数据
int CDevSIU_CFES::SetData(unsigned short usType, void *vData)
{
    switch(usType)
    {
        default:
            break;
    }

    return ERR_IOMC_SUCCESS;
}



// 获取数据
int CDevSIU_CFES::GetData(unsigned short usType, void *vData)
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
#endif

int CDevSIU_CFES::SetFlickerLed(int iFlickerLedIdx, int iAction)
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

    long lRet = SendAndReadCmd(&strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    return ERR_IOMC_SUCCESS;
}

int CDevSIU_CFES::SetSkimLed(int iFlickerLedIdx, int iAction)
{
    THISMODULE(__FUNCTION__);
    int iStartIndex = iFlickerLedIdx * 4;
    STR_DRV  stDrv;
    IOMCCOMMONOUTPUT stIomcCommonOutput;
    ZeroMemory(&stDrv, sizeof(stDrv));
    ZeroMemory(&stIomcCommonOutput, sizeof(stIomcCommonOutput));

    stIomcCommonOutput.wLen = 0x3400;
    stIomcCommonOutput.wCNTLId = 0x0100;
    stIomcCommonOutput.WCNTLLen = 0x0400;
    stIomcCommonOutput.wCNTLCmd = 0x00A3;
    stIomcCommonOutput.wOutput1Id = 0x0107;
    stIomcCommonOutput.wOutput1Len = 0x2200;
    stIomcCommonOutput.byOutput1Data[iStartIndex] = (iAction == 0) ? 0x02 : 0x01;

    stIomcCommonOutput.wOutput2Id = 0x0307;
    stIomcCommonOutput.wOutput2Len = 0x0600;

    stDrv.usParam          = USB_PRM_NO_RSP;                   // Parameter(Call type): No Resp STD40-00-00-01
    stDrv.uiDataInBuffSz   = 0x34;                             // Input data size: 28byte
    stDrv.pvDataInBuffPtr  = &stIomcCommonOutput;              // Input data pointer
    stDrv.uiTimer          = 10;                               // Command time out

    long lRet = SendAndReadCmd(&stDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "非接灯执行命令失败：wCmd:%d,lRet=%d", iAction, lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }
    return ERR_IOMC_SUCCESS;
}


long CDevSIU_CFES::SendCmdData(WORD wCmd, PSTR_DRV pParam)
{
    AutoMutex(m_cMutex);
//    return m_cDev.USBDrvCall(wCmd, pParam);
    return SendAndReadCmd(pParam);
}
//////////////////////////////////////////////////////////////////////////
long CDevSIU_CFES::SetLightsCmd(WORD wID, WORD wCmd)
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
    {
        if(m_iFIDCType == 1) {
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
            return SetSKMLampCmd(wCmd, 3);
//        stFlickerSetData.Flk[IOMC_FLK_ID_ICRW_FLK] = byNewStatus;
        }
    }
        break;
#ifdef SET_BANK_CCB
    case GUIDLIGHT_SCANNER:                               // 扫描仪
        return SetSKMLampCmd(wCmd, 2);
        break;
    case GUIDLIGHT_CHEQUEUNIT:                            // Cheque unit
        return SetSKMLampCmd(wCmd, 6);
        break;
#else
    case GUIDLIGHT_SCANNER:                               // 扫描仪
    case GUIDLIGHT_CHEQUEUNIT:                            // Cheque unit
#endif
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

//    long lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASEND, &strDrv);
    long lRet = SendAndReadCmd(&strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }
    return ERR_IOMC_SUCCESS;
}

long CDevSIU_CFES::GetSensStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    memset(m_bySendBuff, 0, sizeof(m_bySendBuff));
    memset(m_byRecvBuff, 0, sizeof(m_byRecvBuff));

    UINT uiRecvLen = sizeof(m_byRecvBuff);
    int iOffset = 0;

    //命令编辑
    iOffset += 2;
    WORD wCmdId = EN_IOMC_CMD_SENSE_STATUS_CONSTRAINT;
    iOffset += AddPkgData(m_bySendBuff + 2, EN_IOMC_CMDID_CMDCODE, (LPBYTE)&wCmdId, sizeof(wCmdId));

    m_bySendBuff[0] = HIWORD(iOffset);
    m_bySendBuff[1] = LOWORD(iOffset);

    long lRet = SendAndReadCmd(m_bySendBuff, iOffset, m_byRecvBuff, uiRecvLen);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    memcpy(&m_stSensInfo, m_byRecvBuff, sizeof(m_stSensInfo));

    return 0;
}

void CDevSIU_CFES::UpdateDoorsStatus()
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

void CDevSIU_CFES::UpdateSensorsStatus()
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

    //接近传感器
    if(m_stSensInfo.byFUKA_SENSE_INF[4] & 0x02){
        m_stStatus.wSensors[SENSORS_PROXIMITY] = SENSORS_NOT_PRESENT;
    } else {
        m_stStatus.wSensors[SENSORS_PROXIMITY] = SENSORS_PRESENT;
    }
}

void CDevSIU_CFES::UpdateAuxiliariesStatus()
{

}


bool CDevSIU_CFES::SetSupportStatus()
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

	 m_byOnOffTime[0] = (int)cINI.GetValue("FlashSlowOn", 80);     //40-00-00-00(FT#0001)
    m_byOnOffTime[1] = (int)cINI.GetValue("FlashSlowOff", 50);     //40-00-00-00(FT#0001)
    m_byOnOffTime[2] = (int)cINI.GetValue("FlashMediumOn", 64);     //40-00-00-00(FT#0001)
    m_byOnOffTime[3] = (int)cINI.GetValue("FlashMediumOff", 39);     //40-00-00-00(FT#0001)
    m_byOnOffTime[4] = (int)cINI.GetValue("FlashQuickOn", 14);     //40-00-00-00(FT#0001)
    m_byOnOffTime[5] = (int)cINI.GetValue("FlashQuickOff", 13);     //40-00-00-00(FT#0001)
	
    m_iFIDCType = (int)cINI.GetValue("FIDCType", 0);         //30-00-00-00(FS#0012)
    if(m_iFIDCType < 0 || m_iFIDCType > 1){                  //30-00-00-00(FS#0012)
        m_iFIDCType = 0;                                     //30-00-00-00(FS#0012)
    }                                                        //30-00-00-00(FS#0012)
    return true;
}

void CDevSIU_CFES::ResetAllStatus()
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

void CDevSIU_CFES::ResetNotSupportStatus()
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

void CDevSIU_CFES::UpdateStatus(WORD wDevice, long lErrCode)
{
    m_stStatus.wDevice = wDevice;
    sprintf(m_stStatus.szErrCode, "0%02X", qAbs(lErrCode));
}

long CDevSIU_CFES::GetVersion(VERSIONINFO &stVersion)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
//    if (!m_cDev.IsOpen())
    if(!IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return ERR_IOMC_NOT_OPEN;
    }

    memset(m_bySendBuff, 0, sizeof(m_bySendBuff));
    memset(m_byRecvBuff, 0, sizeof(m_byRecvBuff));

    UINT uiRecvLen = sizeof(m_byRecvBuff);
    int iOffset = 0;

    //命令编辑
    iOffset += 2;
    WORD wCmdId = EN_IOMC_CMD_VERSION_SENSE;
    iOffset += AddPkgData(m_bySendBuff + 2, EN_IOMC_CMDID_CMDCODE, (LPBYTE)&wCmdId, sizeof(wCmdId));

    m_bySendBuff[0] = HIWORD(iOffset);
    m_bySendBuff[1] = LOWORD(iOffset);

    long lRet = SendAndReadCmd(m_bySendBuff, iOffset,  m_byRecvBuff, uiRecvLen);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    memcpy(&stVersion, m_byRecvBuff + RES_DATA_OFFSET, sizeof(stVersion));

    return 0;
}

long CDevSIU_CFES::SetSKMLampCmd(WORD wCmd,  BYTE byPortNum)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(byPortNum > SKM_LAMP_MAX || byPortNum < 1){
        return ERR_IOMC_PARAM;
    }

    switch(wCmd){
    case IOMC_STATUS_NOCHANGE:
        break;
    case GUIDLIGHT_OFF:
    case GUIDLIGHT_CONTINUOUS:
    {
        //停止闪烁
        m_stSkmLampFlash[byPortNum - 1].iOffTime = 0;
        m_stSkmLampFlash[byPortNum - 1].iOnTime = 0;

        int iStartIndex = (byPortNum - 1) * 4;
        STR_DRV  stDrv;
        IOMCCOMMONOUTPUT stIomcCommonOutput;
        ZeroMemory(&stDrv, sizeof(stDrv));
        ZeroMemory(&stIomcCommonOutput, sizeof(stIomcCommonOutput));

        stIomcCommonOutput.wLen = 0x3400;
        stIomcCommonOutput.wCNTLId = 0x0100;
        stIomcCommonOutput.WCNTLLen = 0x0400;
        stIomcCommonOutput.wCNTLCmd = 0x00A3;
        stIomcCommonOutput.wOutput1Id = 0x0107;
        stIomcCommonOutput.wOutput1Len = 0x2200;
        stIomcCommonOutput.byOutput1Data[iStartIndex] = wCmd == GUIDLIGHT_OFF ? 0x02 : 0x01;

        stIomcCommonOutput.wOutput2Id = 0x0307;
        stIomcCommonOutput.wOutput2Len = 0x0600;

        stDrv.usParam          = USB_PRM_NO_RSP;                   // Parameter(Call type): No Resp STD40-00-00-01
        stDrv.uiDataInBuffSz   = 0x34;                             // Input data size: 28byte
        stDrv.pvDataInBuffPtr  = &stIomcCommonOutput;              // Input data pointer
        stDrv.uiTimer          = 10;                               // Command time out

        //        long lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASEND, &stDrv);
        long lRet = SendAndReadCmd(&stDrv);
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
        m_stSkmLampFlash[byPortNum - 1].iOnTime = m_byOnOffTime[0] * 10;
        m_stSkmLampFlash[byPortNum - 1].iOffTime = m_byOnOffTime[1] * 10;
        if(wCmd == GUIDLIGHT_MEDIUM_FLASH){
            m_stSkmLampFlash[byPortNum - 1].iOnTime = m_byOnOffTime[2] * 10;
            m_stSkmLampFlash[byPortNum - 1].iOffTime = m_byOnOffTime[3] * 10;
        } else if(wCmd == GUIDLIGHT_QUICK_FLASH){
            m_stSkmLampFlash[byPortNum - 1].iOnTime = m_byOnOffTime[4] * 10;
            m_stSkmLampFlash[byPortNum - 1].iOffTime = m_byOnOffTime[5] * 10;
        }

        m_stSkmLampFlash[byPortNum - 1].ulLastOnOffTime = 0;
        m_stSkmLampFlash[byPortNum - 1].bLedOn = FALSE;
    }
        break;
    default:
        break;
    }

    return ERR_IOMC_SUCCESS;
}

void CDevSIU_CFES::ExitSKMLampFlashThread()
{
    if(m_tid != 0){
        g_bExitThread = true;
        pthread_join(m_tid, NULL);
        m_tid = 0;
    }
}

long CDevSIU_CFES::SendAndReadCmd(LPBYTE lpbySendData, UINT uiSendLen, LPVOID lpRespData, UINT &uiRespLen)
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    long lRet;
    STR_DRV  stDrv;
    memset(&stDrv, 0, sizeof(stDrv));

    // USB driver parameter
    stDrv.uiDataInBuffSz = uiSendLen;                      // Input data size
    stDrv.pvDataInBuffPtr = lpbySendData;                  // Input data pointer
    stDrv.pvDataOutBuffPtr = lpRespData;                   // Output data pointer
    stDrv.uiDataOutReqSz = uiRespLen;                      // Output data area size
    stDrv.uiTimer = 0;                                     // Timeout

//    lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASENDRCV, &stDrv);
    lRet = SendAndReadCmd(&stDrv);
    uiRespLen = stDrv.uiDataOutBuffSz;
    return lRet;
}

long CDevSIU_CFES::AddPkgData(LPBYTE lpData, WORD wPkgId, LPBYTE lpbyPkgData, int iPkgDataLen)
{
    if(nullptr == lpData || nullptr == lpbyPkgData){
        return 0;
    }

    WORD wLen = 2 + iPkgDataLen;
    lpData[0] = LOBYTE(wPkgId);
    lpData[1] = HIBYTE(wPkgId);
    lpData[2] = HIBYTE(wLen);
    lpData[3] = LOBYTE(wLen);

    memcpy(lpData + 4, lpbyPkgData, iPkgDataLen);
    return wLen + 2;
}

//30-00-00-00(FS#0012)
long CDevSIU_CFES::CloseTMZFidc()
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

//30-00-00-00(FS#0012)
long CDevSIU_CFES::OpenTMZFidc()
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

//40-00-00-00(FT#0001)
long CDevSIU_CFES::SetFlashRecycleParamer()
{
    THISMODULE(__FUNCTION__);

    memset(m_bySendBuff, 0, sizeof(m_bySendBuff));
    memset(m_byRecvBuff, 0, sizeof(m_byRecvBuff));

    UINT uiRecvLen = sizeof(m_byRecvBuff);
    int iOffset = 0;

    //命令编辑
    iOffset += 2;
    WORD wCmdId = EN_IOMC_CMD_DATA_WRITE;
    iOffset += AddPkgData(m_bySendBuff + iOffset, EN_IOMC_CMDID_CMDCODE, (LPBYTE)&wCmdId, sizeof(wCmdId));

    //时间
    BYTE byTime[8] = {0};
    SYSTEMTIME currentTime = CQtTime::CurrentTime();
    byTime[0] = HEXTOBCD(currentTime.wYear/100);
    byTime[1] = HEXTOBCD(currentTime.wYear%100);
    byTime[2] = HEXTOBCD(currentTime.wMonth);
    byTime[3] = HEXTOBCD(currentTime.wDay);
    byTime[4] = HEXTOBCD(currentTime.wDayOfWeek % 7);
    byTime[5] = HEXTOBCD(currentTime.wHour);
    byTime[6] = HEXTOBCD(currentTime.wMinute);
    byTime[7] = HEXTOBCD(currentTime.wSecond);
    iOffset += AddPkgData(m_bySendBuff + iOffset, EN_IOMC_CMDID_DATETIMESET, byTime, sizeof(byTime));

    BYTE byFlkcycleMode[2 * 8];
    memset(byFlkcycleMode, 0, sizeof(byFlkcycleMode));
    memcpy(byFlkcycleMode, m_byOnOffTime, sizeof(m_byOnOffTime));
    iOffset += AddPkgData(m_bySendBuff + iOffset, EN_IOMC_CMDID_FLKCYCLE, byFlkcycleMode, sizeof(byFlkcycleMode));

    BYTE byTemp[12] = {0};
    byTemp[0] = 70;
    byTemp[1] = 30;
    byTemp[2] = 40;
    byTemp[3] = 20;
    iOffset += AddPkgData(m_bySendBuff + iOffset, EN_IOMC_CMDID_LMCLEDCYCLE, byTemp, 4);
    iOffset += AddPkgData(m_bySendBuff + iOffset, EN_IOMC_CMDID_STLMPCYCLE, byTemp, 4);
    iOffset += AddPkgData(m_bySendBuff + iOffset, EN_IOMC_CMDID_LMCBZRCYCLE, byTemp, 4);

    memset(byTemp, 0, sizeof(byTemp));
    byTemp[0] = 0xFF;
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x4A07, byTemp, 2);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x4207, byTemp, 2);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x4607, byTemp, 2);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x4507, byTemp, 8);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x5007, byTemp, 2);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x5107, byTemp, 2);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x5207, byTemp, 2);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x5E07, byTemp, 2);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x6007, byTemp, 4);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x1507, byTemp, 4);
    iOffset += AddPkgData(m_bySendBuff + iOffset, 0x4E07, byTemp, 6);

    m_bySendBuff[0] = HIWORD(iOffset);
    m_bySendBuff[1] = LOWORD(iOffset);

    long lRet = SendAndReadCmd(m_bySendBuff, iOffset, m_byRecvBuff, uiRecvLen);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return ERR_IOMC_HARDWARE_ERROR;
    }

    return ERR_IOMC_SUCCESS;
}

long CDevSIU_CFES::SendAndReadCmd(PSTR_DRV pParam)
{
    //进程间互斥，读写原子操作，防止返回数据被其他进程误读
    CAutoMutex autoMutex(&m_cIOMCProcMutex);

    long lRet = ERR_IOMC_SUCCESS;

    if(!IsOpen()){
        return ERR_IOMC_NOT_OPEN;
    }

    //向共享内存写入命令
    if(!m_cIOMCShareMemoryS.Write(pParam->pvDataInBuffPtr, pParam->uiDataInBuffSz)){
        MLog(__FUNCTION__, __LINE__, "IOMC共享内存写入命令数据失败");
        return ERR_IOMC_WRITE_ERROR;
    }

    //从共享内存读取返回数据
    ULONG ulStart = CQtTime::GetSysTick();
    char cReadBuff[1024];
    memset(cReadBuff, 0, sizeof(cReadBuff));
    while(true){
        if(m_cIOMCShareMemoryR.Read(cReadBuff, sizeof(cReadBuff))){
            WORD wLen = MAKEWORD(cReadBuff[1], cReadBuff[0]);
            if(0 == wLen){          //数据长度为0时为命令执行异常
                return ERR_IOMC_USB_CMD_ERR;
            }
            if(pParam->pvDataOutBuffPtr != nullptr && pParam->uiDataOutReqSz != 0){
                memcpy((LPSTR)pParam->pvDataOutBuffPtr, cReadBuff, qMin((UINT)wLen, pParam->uiDataOutReqSz));
                pParam->uiDataOutBuffSz = wLen;
            }
            break;
        }

        if(CQtTime::GetSysTick() - ulStart > 30*1000){
            return ERR_IOMC_READ_TIMEOUT;
        }
    }

    return lRet;
}

BOOL CDevSIU_CFES::IsOpen()
{
    return m_cIOMCShareMemoryR.IsOpened() && m_cIOMCShareMemoryS.IsOpened();
}

// 获取读卡器异物检知结果
INT CDevSIU_CFES:: GetCardReaderSkimming()
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
//    lRet = m_cDev.USBDrvCall(USB_DRV_FN_DATASENDRCV, &stDrvParam);
    lRet = SendAndReadCmd(&stDrvParam);
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
