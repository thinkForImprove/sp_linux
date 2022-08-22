#include "USBDrive.h"

static const char *ThisFile = "USBDrive.cpp";
//////////////////////////////////////////////////////////////////////////
CUSBDrive::CUSBDrive(LPCSTR lpDevType, IOMCTYPE eIomcType/* = IOMC_TYPE_HT*/): m_ulUSBHandle(0x00),
                                                                               m_cSysMutex("IOMC_USBDrive_2020"),
                                                                               m_eIomcType(eIomcType)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    m_strDevType = lpDevType;
    memset(m_szDesc, 0x00, sizeof(m_szDesc));
    memset(m_szDevName, 0x00, sizeof(m_szDevName));
    m_pFnATMUSB = nullptr;
    m_pInfATMUSB = nullptr;
    m_bySeqNo = 0x00;
}

CUSBDrive::~CUSBDrive()
{
    USBDllRelease();
}

long CUSBDrive::USBDllLoad()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cSysMutex);

    long lRet = 0;
    if(IOMC_TYPE_HT == m_eIomcType){
        if (m_cUsbDll.isLoaded())
        {
            Log(ThisModule, __LINE__, "已加载VHUSB.dll库");
            return 0;
        }

        QString strDllName;
    #ifdef QT_WIN32
        strDllName = "C:/CFES/BIN/VHUSB.dll";
    #else
        strDllName = "/hots/lib/VHUSB.so";
    #endif
        m_cUsbDll.setFileName(strDllName);
        if (!m_cUsbDll.load())
        {
            Log(ThisModule, __LINE__, "VHUSB.dll加载失败：%s", m_cUsbDll.errorString().toStdString().c_str());
            return -1;
        }

        m_pFnATMUSB  = (HT_FnATMUSB)m_cUsbDll.resolve("FnATMUSB");
        m_pInfATMUSB = (HT_InfATMUSB)m_cUsbDll.resolve("InfATMUSB");
        if (nullptr == m_pFnATMUSB || nullptr == m_pInfATMUSB)
        {
            USBDllRelease();
            Log(ThisModule, __LINE__, "接口加载失败");
            return -2;
        }
    } else {
        if(nullptr == m_pDevPort){
            lRet = m_pDevPort.Load("AllDevPort.so", "CreateIAllDevPort", "IOMC", m_strDevType.c_str());
            if(lRet != 0){
                Log(ThisModule, __LINE__, "加载库AllDevPort.so失败[%s]", m_pDevPort.LastError().toStdString().c_str());
                return -1;
            }
        }
    }

    return 0;
}

void CUSBDrive::USBDllRelease()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(IOMC_TYPE_HT == m_eIomcType){
        if (m_cUsbDll.isLoaded())
        {
            m_cUsbDll.unload();
            m_pInfATMUSB = nullptr;
            m_pFnATMUSB  = nullptr;
        }
    } else {
        m_pDevPort.Release();
    }

    return ;
}

long CUSBDrive::USBOpen(const char *pDevName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    long lRet = 0;
    if (pDevName == nullptr || strlen(pDevName) == 0)
    {
        Log(ThisModule, __LINE__, "无效设备名称");
        return -1;
    }

    if(IOMC_TYPE_HT == m_eIomcType){
        if (m_ulUSBHandle != 0 && qstricmp(pDevName, m_szDevName) == 0)
        {
            Log(ThisModule, __LINE__, "已打开%s的USB连接", m_szDevName);
            return 0;
        }

        STR_DRV strDrv;
        memset(&strDrv, 0x00, sizeof(STR_DRV));
        m_ulUSBHandle = 0x00;
        strcpy(m_szDevName, pDevName);// 对应的USB设备名称

        // 打开连接参数
        strDrv.usParam = USB_PRM_OPEN;
        strDrv.pvDataInBuffPtr = &m_szDevName[0];
        strDrv.uiDataInBuffSz = qstrlen(m_szDevName);
        strDrv.pvDataOutBuffPtr = &m_ulUSBHandle;
        strDrv.uiDataOutReqSz = 4;
        strDrv.uiTimer = 60;

        long lRet = USBDrvCall(USB_DRV_INF_OPEN, &strDrv);
        if (lRet != 0)
        {
            Log(ThisModule, __LINE__, "打开 \"%s\" 的USB连接失败", m_szDevName);
            return lRet;
        }
    } else {
        if(nullptr == m_pDevPort){
            return -1;
        } else {
            lRet = m_pDevPort->Open(pDevName);
            if(lRet != ERR_DEVPORT_SUCCESS){
                return -1;
            }
            m_ulUSBHandle = 1;
        }
    }

    return 0;
}

long CUSBDrive::USBClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(IOMC_TYPE_HT == m_eIomcType){
        if (m_ulUSBHandle == 0x00)
        {
            Log(ThisModule, __LINE__, "已关闭USB连接");
            return 0;
        }

        STR_DRV strDrv;
        memset(&strDrv, 0x00, sizeof(STR_DRV));

        // 关闭连接参数
        strDrv.usParam  = USB_PRM_CLOSE;
        strDrv.uiDrvHnd = m_ulUSBHandle;// 对应的句柄
        long lRet = USBDrvCall(USB_DRV_INF_CLOSE, &strDrv);
        if (lRet != 0)
        {
            Log(ThisModule, __LINE__, "关闭 \"%s\" 的USB连接失败", m_szDevName);
            return lRet;
        }

        m_ulUSBHandle = 0x00;
        memset(m_szDevName, 0x00, sizeof(m_szDevName));
    } else {
        if(m_pDevPort != nullptr){
            m_pDevPort->Close();
            m_ulUSBHandle = 0;
        }
    }

    return 0;
}

bool CUSBDrive::IsOpen()
{
    return (m_ulUSBHandle != 0);
}

long CUSBDrive::USBDrvCall(WORD wCmd, PSTR_DRV pParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cSysMutex);

    long lRet = 0;
    if(IOMC_TYPE_HT == m_eIomcType){
        if (pParam == nullptr || m_pFnATMUSB == nullptr || m_pInfATMUSB == nullptr)
        {
            Log(ThisModule, __LINE__, "参数指针为NULL，或没有加载USB驱动库");
            return -1;
        }
        if (m_ulUSBHandle == 0x00 && wCmd != USB_DRV_INF_OPEN)
        {
            Log(ThisModule, __LINE__, "没有打开USB连接");
            return -2;
        }

        pParam->uiDrvHnd  = m_ulUSBHandle;
        HT_FnATMUSB pFunc = nullptr;
        switch (wCmd)
        {
        case USB_DRV_FN_DATASEND:
        case USB_DRV_FN_DATARCV:
        case USB_DRV_FN_DATASENDRCV:
        case USB_DRV_FN_USBRESET:
            pFunc = m_pFnATMUSB;
            break;
        case USB_DRV_INF_OPEN:
        case USB_DRV_INF_CLOSE:
        case USB_DRV_INF_INFGET:
        case USB_DRV_INF_SENS:
            pFunc = m_pInfATMUSB;
            break;
        default:
            Log(ThisModule, __LINE__, "不支持命令参数: wCmd = %d", wCmd);
            return -3;
        }

        ULONG uRet = pFunc(wCmd, pParam);
        if (uRet != ERR_DRV_USB_SUCCESS)
        {
            Log(ThisModule, __LINE__, "调用命令返回失败：[wCmd=%d,sParam=%d]lRet=0x%02X[ %s ]", wCmd, pParam->usParam, uRet, GetErrDesc(uRet));
            return -4;
        }
    } else {
        BYTE bySendData[DEF_WRITE_BUFF_SIZE] = {0};
        int iOffset = 0;
        m_bySeqNo = (m_bySeqNo == 0xFF ? 0x00 : m_bySeqNo);

        LPCMDRESPKG lpCmdResPkg = (LPCMDRESPKG)((LPBYTE)pParam->pvDataInBuffPtr + 2);
        FIRSTPART stFirstPart;
        memset(&stFirstPart, 0, sizeof(stFirstPart));
        stFirstPart.wMLen = EXWORDHL(sizeof(stFirstPart) + pParam->uiDataInBuffSz + 2);
        stFirstPart.bySeq = m_bySeqNo++;
        stFirstPart.byFlg = GetCmdFlag(lpCmdResPkg->wCode);
        if(0 == pParam->uiDataOutReqSz || nullptr == pParam->pvDataOutBuffPtr){
            stFirstPart.byFlg = CMD_FLAG_NORESPONSE;
        }

        memcpy((char *)bySendData, &stFirstPart, sizeof(stFirstPart));
        iOffset += sizeof(stFirstPart);
        memcpy((char *)bySendData + iOffset, pParam->pvDataInBuffPtr, pParam->uiDataInBuffSz);
        iOffset += pParam->uiDataInBuffSz;
        //计算校验码BCC
        WORD wBCC = CalcBCC(bySendData, iOffset);
        memcpy((char *)bySendData + iOffset, &wBCC, sizeof(wBCC));
        iOffset += 2;

        UINT uiRecvLen = pParam->uiDataOutReqSz;
        //超时时间
        pParam->uiTimer = GetCmdTimeout(lpCmdResPkg->wCode);

        BOOL bLog = GetLogEnable(lpCmdResPkg->wCode);
        if(bLog){
            m_pDevPort->SetLogAction(__FUNCTION__);
        }

        lRet = SendAndReadData(bySendData, iOffset, (LPBYTE)pParam->pvDataOutBuffPtr,
                        uiRecvLen, pParam->uiTimer);
        if(bLog){
            m_pDevPort->FlushLog();
        }
        pParam->uiDataOutBuffSz = uiRecvLen;
    }

    return lRet;
}

void CUSBDrive::SetFnATMUSB(HT_FnATMUSB pFnATMUSB, HT_InfATMUSB pInfATMUSB)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pFnATMUSB = pFnATMUSB;
    m_pInfATMUSB = pInfATMUSB;
    return;
}

HT_FnATMUSB CUSBDrive::GetFnATMUSB()
{
    return m_pFnATMUSB;
}

HT_InfATMUSB CUSBDrive::GetInfATMUSB()
{
    return m_pInfATMUSB;
}

LPCSTR CUSBDrive::GetErrDesc(ULONG ulErrCode)
{
    memset(m_szDesc, 0x00, sizeof(m_szDesc));
    switch (ulErrCode)
    {
    case ERR_DRV_USB_CANCEL_END: strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_END"); break;
    case ERR_DRV_USB_CANCEL_NOPST: strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_NOPST"); break;
    case ERR_DRV_USB_CANCEL_CROSS_END: strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_CROSS_END"); break;
    case ERR_DRV_USB_CANCEL_NOTHING: strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_NOTHING"); break;
    case ERR_DRV_USB_FUNC: strcpy(m_szDesc, "ERR_DRV_USB_FUNC"); break;
    case ERR_DRV_USB_PRM: strcpy(m_szDesc, "ERR_DRV_USB_PRM"); break;
    case ERR_DRV_USB_DRVHND_DIFFER: strcpy(m_szDesc, "ERR_DRV_USB_DRVHND_DIFFER"); break;
    case ERR_DRV_USB_DRV_REMOVE: strcpy(m_szDesc, "ERR_DRV_USB_DRV_REMOVE"); break;
    case ERR_DRV_USB_BLD: strcpy(m_szDesc, "ERR_DRV_USB_BLD"); break;
    case ERR_DRV_USB_INDATA: strcpy(m_szDesc, "ERR_DRV_USB_INDATA"); break;
    case ERR_DRV_USB_OUTDATA: strcpy(m_szDesc, "ERR_DRV_USB_OUTDATA"); break;
    case ERR_DRV_USB_INOUTDATA: strcpy(m_szDesc, "ERR_DRV_USB_INOUTDATA"); break;
    case ERR_DRV_USB_ENTRY_DEVICE_OVER: strcpy(m_szDesc, "ERR_DRV_USB_ENTRY_DEVICE_OVER"); break;
    case ERR_DRV_USB_ENTRY_THREAD_OVER: strcpy(m_szDesc, "ERR_DRV_USB_ENTRY_THREAD_OVER"); break;
    case ERR_DRV_USB_BCC: strcpy(m_szDesc, "ERR_DRV_USB_BCC"); break;
    case ERR_DRV_USB_INDATA_BUFFSZ: strcpy(m_szDesc, "ERR_DRV_USB_INDATA_BUFFSZ"); break;
    case ERR_DRV_USB_OUTDATA_BUFFSZ: strcpy(m_szDesc, "ERR_DRV_USB_OUTDATA_BUFFSZ"); break;
    case ERR_DRV_USB_INOUTDATA_BUFFSZ: strcpy(m_szDesc, "ERR_DRV_USB_INOUTDATA_BUFFSZ"); break;
    case ERR_DRV_USB_LINE_TIMEOUT: strcpy(m_szDesc, "ERR_DRV_USB_LINE_TIMEOUT"); break;
    case ERR_DRV_USB_COMMAND_TIMEOUT: strcpy(m_szDesc, "ERR_DRV_USB_COMMAND_TIMEOUT"); break;
    case ERR_DRV_USB_CLOSE: strcpy(m_szDesc, "ERR_DRV_USB_CLOSE"); break;
    case ERR_DRV_USB_OPEN_BUSY: strcpy(m_szDesc, "ERR_DRV_USB_OPEN_BUSY"); break;
    case ERR_DRV_USB_SEND_BUSY: strcpy(m_szDesc, "ERR_DRV_USB_SEND_BUSY"); break;
    case ERR_DRV_USB_RCV_BUSY: strcpy(m_szDesc, "ERR_DRV_USB_RCV_BUSY"); break;
    case ERR_DRV_USB_EP_DOWN: strcpy(m_szDesc, "ERR_DRV_USB_EP_DOWN"); break;
    case ERR_DRV_USB_MEMORY: strcpy(m_szDesc, "ERR_DRV_USB_MEMORY"); break;
    case ERR_DRV_USB_HANDLE: strcpy(m_szDesc, "ERR_DRV_USB_HANDLE"); break;
    case ERR_DRV_USB_REG: strcpy(m_szDesc, "ERR_DRV_USB_REG"); break;
    case ERR_DRV_USB_DRVCALL: strcpy(m_szDesc, "ERR_DRV_USB_DRVCALL"); break;
    case ERR_DRV_USB_THREAD: strcpy(m_szDesc, "ERR_DRV_USB_THREAD"); break;
    case ERR_DRV_USB_POSTMSG: strcpy(m_szDesc, "ERR_DRV_USB_POSTMSG"); break;
    case ERR_DRV_USB_TRACE: strcpy(m_szDesc, "ERR_DRV_USB_TRACE"); break;
    default:                sprintf(m_szDesc, "0x%08.08X", ulErrCode); break;
    }
    return m_szDesc;
}

void CUSBDrive::SetIomcType(const IOMCTYPE &eIomcType)
{
    m_eIomcType = eIomcType;
}

long CUSBDrive::SendAndReadData(LPBYTE lpbySendData, UINT uiSendLen, LPBYTE lpbyRecvData, UINT &uiRecvLen,
                                   UINT uiTimeout)
{
    if(nullptr == lpbySendData || uiSendLen <= 0){
        return ERR_DEVPORT_PARAM;
    }

    long lRet = ERR_DEVPORT_SUCCESS;
    //数据发送：分割为64字节包发送，最后不足64字节，用0补足
    int iPkgCnt = (uiSendLen + (HID_REPORT_SIZE - 1))/HID_REPORT_SIZE;
    BYTE byPkgData[HID_REPORT_SIZE] = {0};
    for(int i = 0; i < iPkgCnt; i++){
        memset(byPkgData, 0, sizeof(byPkgData));
        int iCopySize = (i == iPkgCnt - 1) ? (uiSendLen - HID_REPORT_SIZE * i) : HID_REPORT_SIZE;
        mempcpy(byPkgData, lpbySendData + i * HID_REPORT_SIZE, iCopySize);
        lRet = m_pDevPort->Send((LPCSTR)byPkgData, HID_REPORT_SIZE, DEF_WRITE_TIMEOUT);
        if(lRet != ERR_DEVPORT_SUCCESS){
            return ERR_DEVPORT_WRITE;
        }
    }

    //如果输出buffer为空，不读取返回数据
    if(nullptr == lpbyRecvData || 0 == uiRecvLen){
//        return lRet;
        //硬件仍然会返回数据
    }

    //数据接收
    char cReadBuffer[DEF_READ_BUFF_SIZE] = {0};
    UINT uiReadLen = sizeof(cReadBuffer);
    lRet = RecvData((LPBYTE)cReadBuffer, uiReadLen, uiTimeout);
    if(lRet != ERR_DEVPORT_SUCCESS){
        return lRet;
    }

    //接收数据检查
    lRet = CheckRecvData((LPBYTE)cReadBuffer, uiReadLen);

    //截取有效部分返回
    if(ERR_DEVPORT_SUCCESS == lRet){
        if(lpbyRecvData != nullptr && uiReadLen > 0){
            uiRecvLen = uiReadLen - sizeof(FIRSTPART) - 2;
            memcpy(lpbyRecvData, cReadBuffer + sizeof(FIRSTPART), uiRecvLen);
        }
    }

    return lRet;
}

long CUSBDrive::RecvData(LPBYTE lpbyRecvData, UINT &uiRecvLen, UINT uiTimeout)
{
    if(nullptr == lpbyRecvData || uiRecvLen <= 0){
        return ERR_DEVPORT_PARAM;
    }

    long lRet = ERR_DEVPORT_SUCCESS;
    int iOffset = 0;
    int iTotalDataLen = 0;
    ULONG ulStartTime = CQtTime::GetSysTick();
    char cReadBuffer[DEF_READ_BUFF_SIZE] = {0};
    while(true){
        DWORD dwReadLen = 64;
        lRet = m_pDevPort->Read(cReadBuffer + iOffset, dwReadLen, DEF_SINGLE_READ_TIMEOUT);
        if(lRet < 0 && lRet != ERR_DEVPORT_RTIMEOUT){
            Log(__FUNCTION__, __LINE__, "读数据错误:%d", lRet);
            lRet = ERR_DEVPORT_READERR;
            break;
        }

        if(dwReadLen > 0){
            iOffset += dwReadLen;
            if(iTotalDataLen == 0 && iOffset >= 2){
                iTotalDataLen = MAKEWORD(cReadBuffer[1], cReadBuffer[0]);
            }
            if(iOffset >= iTotalDataLen){
                lRet = ERR_DEVPORT_SUCCESS;
                break;
            }
        }

        //检查超时
        if(uiTimeout > 0 && (CQtTime::GetSysTick() - ulStartTime > uiTimeout)){
            Log(__FUNCTION__, __LINE__, "读数据超时，超时时间:%dms", uiTimeout);
            lRet = ERR_DEVPORT_RTIMEOUT;
            break;
        }
        CQtTime::Sleep(1);
    }

    if(ERR_DEVPORT_SUCCESS == lRet){
        if(iOffset > uiRecvLen){
            Log(__FUNCTION__, __LINE__, "实际接收数据长度[%d]大于接收buffer长度[%d]", iOffset, uiRecvLen);
            lRet = ERR_DEVPORT_PARAM;
        } else {
            memcpy((char *)lpbyRecvData, cReadBuffer, iOffset);
            uiRecvLen = iOffset;
        }
    }

    return lRet;
}

long CUSBDrive::CheckRecvData(LPBYTE lpRecvData, UINT uiRecvLen)
{
    if(nullptr == lpRecvData || uiRecvLen <= 0){
        return ERR_DEVPORT_PARAM;
    }

    long lRet = ERR_DEVPORT_SUCCESS;
    WORD wBCC = CalcBCC(lpRecvData, uiRecvLen - 2);
    WORD wRespBCC = MAKEWORD(lpRecvData[uiRecvLen -2], lpRecvData[uiRecvLen - 1]);
    if(wBCC != wRespBCC){
        Log(__FUNCTION__, __LINE__, "应答数据BCC校验错误[0x%X:0x%X]", wRespBCC, wBCC);
        lRet = ERR_DEVPORT_NODEFINED;
    } else {
        //检查应答码
        LPIOMCRESPCOMMON lpIomcRespCommPart = (LPIOMCRESPCOMMON)lpRecvData;
        BYTE byRespCode = HIBYTE(lpIomcRespCommPart->stRespValidComm.stRespPkg.wCode);
        if(byRespCode > 0x6F){
            Log(__FUNCTION__, __LINE__, "命令执行返回错误[0x%X]", byRespCode);
            lRet = ERR_DEVPORT_NODEFINED;
        }
    }

    return lRet;
}

WORD CUSBDrive::CalcBCC(LPBYTE lpbyData, UINT uiDataLen)
{
    BYTE byFirstByte = 0;
    BYTE bySecondByte = 0;
    for(int i = 0; i < uiDataLen; i++){
        if(i % 2 == 0){
           byFirstByte ^= lpbyData[i];
        } else {
           bySecondByte ^= lpbyData[i];
        }
    }

    return MAKEWORD(byFirstByte, bySecondByte);
}

BYTE CUSBDrive::GetCmdFlag(WORD wCmdId)
{
    BYTE byCmdFlag = CMD_FLAG_NORMAL;
    switch(wCmdId){
    case EN_IOMC_CMD_STOP:
        byCmdFlag = CMD_FLAG_STOP;
        break;
    case EN_IOMC_CMD_PDL_CHECK:
    case EN_IOMC_CMD_PDL_START:
    case EN_IOMC_CMD_PDL_DATA_TRANSMISSION:
    case EN_IOMC_CMD_PDL_END:
        byCmdFlag = CMD_FLAG_PDL;
        break;
    default:
        break;
    }

    return byCmdFlag;
}

BOOL CUSBDrive::GetLogEnable(WORD wCmdId)
{
    BOOL bEnableLog = TRUE;
    switch(wCmdId){
    case EN_IOMC_CMD_SENSE_STATUS_IDLE:
    case EN_IOMC_CMD_SENSE_STATUS_CONSTRAINT:
    case EN_IOMC_CMD_SCAN_REFRESH:
    case EN_IOMC_CMD_PW_SCAN_FLAG:
        bEnableLog = FALSE;
        break;
    default:
        break;
    }

    return bEnableLog;
}

UINT CUSBDrive::GetCmdTimeout(WORD wCmdId)
{
    UINT uiTimeout = 0;

    switch(wCmdId){
    case EN_IOMC_CMD_DATA_WRITE:
        uiTimeout = 60 * 1000;
        break;
    case EN_IOMC_CMD_DATA_READ:
    case EN_IOMC_CMD_TIMER_INFO_SET:
    case EN_IOMC_CMD_SERIAL_NO_WRITE:
    case EN_IOMC_CMD_SERIAL_NO_READ:
    case EN_IOMC_CMD_FLASH_WRITE:
    case EN_IOMC_CMD_FLASH_CLEAR:
    case EN_IOMC_CMD_FLASH_READ:
    case EN_IOMC_CMD_BUZZER:
    case EN_IOMC_CMD_PDL_CHECK:
    case EN_IOMC_CMD_PDL_DATA_TRANSMISSION:
        uiTimeout = 30 * 1000;
        break;
    case EN_IOMC_CMD_LOG_GET_NOT_CLEAR:
    case EN_IOMC_CMD_LOG_GET_CLEAR:
    case EN_IOMC_CMD_LOG_GET_ALL_LOG_CLEAR:
        uiTimeout = 120 * 1000;
        break;
    case EN_IOMC_CMD_PDL_START:
    case EN_IOMC_CMD_PDL_END:
        uiTimeout = 100 * 1000;
        break;
    default:
        uiTimeout = 10 * 1000;
        break;
    }
    return uiTimeout;
}
