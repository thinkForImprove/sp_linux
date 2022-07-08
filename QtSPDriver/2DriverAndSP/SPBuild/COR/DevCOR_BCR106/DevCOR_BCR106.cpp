#include <QTime>
#include "DevCOR_BCR106.h"
static const char *ThisFile = "DevCOR_BCR106.cpp";
static const char *DLLVersion = "DevCOR_BCR106_V1.0.2";

//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIDevCOR(LPCSTR lpDevType, IDevCOR *&pDev)
{
    pDev = new CDevCOR_BCR106(lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}
CDevCOR_BCR106::CDevCOR_BCR106(LPCSTR lpDevType) : m_SemCancel(false)
{
    THISMODULE(__FUNCTION__);
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    Log(ThisModule, __LINE__, DLLVersion);
    m_lastStatusError = 0;
    m_bIsOpen = FALSE;
    memset(m_pszMode, 0, sizeof(m_pszMode));
    memset(m_iHopperNo, 0, sizeof(m_iHopperNo));
}
CDevCOR_BCR106::~CDevCOR_BCR106() {}
void CDevCOR_BCR106::Release() {}
long CDevCOR_BCR106::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    long lRet = 0;

    if (m_pDev == nullptr)
    {
        if (0 != m_pDev.Load("AllDevPort", "CreateIAllDevPort", "COR", "CDevCOR_BCR106"))
        {
            Log(ThisModule, __LINE__, "Load(AllDevPort) failed");
            return ERR_DEVPORT_READERR;
        }
    }

    LOGDEVACTION();
    if (lpMode == nullptr)
    {
        return SOFT_ERROR_PARAMS;
    }

    // start with [/] indicates configure file as input parameter.
    // else serial port parameters as input parameter.
    if (lpMode[0] == '/')
    {
        CINIFileReader configfile;
        BOOL bLoad = configfile.LoadINIFile(lpMode);
        if (bLoad == false)
        {
            Log(ThisModule, -1,
                "configfile.LoadINIFile(%s) failed",
                lpMode);
            return SOFT_ERROR_LOAD_FILE;
        }

        // Move to section[DeviceSettings].
        CINIReader cINI = configfile.GetReaderSection("DeviceSettings");
        std::string sPortMode = (LPCSTR)cINI.GetValue("PortMode", "");
        std::string strCylinderNO = (LPCSTR)cINI.GetValue("CylinderNO", "");
        int iRet = 0;
        CMultiString multiStr;

        iRet = SplitMultipleItems(strCylinderNO.c_str(), multiStr);
        if (iRet < MAX_COINCYLINDER_NUM)
        {
            Log(ThisModule, -1,
                "invalid config item([%s] %s) in %s",
                "DeviceSettings", "CylinderNO",
                lpMode);
            return SOFT_ERROR_INVALID_DATA;
        }

        for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
        {
            m_iHopperNo[i] = atoi(multiStr.GetAt(i));
        }

        strcpy(m_pszMode, sPortMode.c_str());
    }
    else {
        strcpy(m_pszMode, lpMode);
    }

    lRet = m_pDev->Open(m_pszMode);
    if (lRet < 0)
    {
        memset(m_pszMode, 0, sizeof(m_pszMode));
        Log(ThisModule, __LINE__, "Open failed ");
        return ConvertErrorCode(lRet);
    }

    m_bIsOpen = TRUE;
    Log(ThisModule, 0, "Open success ");
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::Close()
{
    THISMODULE(__FUNCTION__);
    if (m_pDev == nullptr)
    {
        m_bIsOpen = FALSE;
        return DEV_SUCCESS;
    }
    LOGDEVACTION();
    m_pDev->Close();
    m_bIsOpen = FALSE;
    Log(ThisModule, 0, "Close success ");
    return DEV_SUCCESS;
}
BYTE CDevCOR_BCR106::CalcSimpleCheckSum(BYTE pszCmd[MAX_CMD_BUFFER_LEN], DWORD nCmdLen)
{
    BYTE bCheckSum = 0;
    BYTE bSum = 0;
    DWORD k = 0;
    for (k = 0; k < nCmdLen; k++)
    {
        bSum = (bSum + pszCmd[k]) & 0xFF;
    }
    bCheckSum = (BYTE)((256 - (int)bSum) & 0xFF);
    return bCheckSum;
}
long CDevCOR_BCR106::Excute(LPCSTR pszCallFunc, BYTE bCmdIndex, BYTE *pszCmdData, DWORD dwCmdDataLen, BYTE *pszResponse, DWORD &dwInOutRespLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null");
        return ConvertErrorCode(ERR_DEVPORT_NOTOPEN);
    }
    if (dwCmdDataLen > 252)
    {
        return ConvertErrorCode(ERR_DEVPORT_PARAM);
    }
    long lRet = 0;
    BYTE pszSend[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRecv[MAX_CMD_BUFFER_LEN] = {0};
    DWORD nSendLen = 0;
    DWORD nRecvLen = 0;
    DWORD nReqRecvLen = 0;
    //组织和发送命令
    pszSend[nSendLen++] = 55;
    pszSend[nSendLen++] = dwCmdDataLen & 0xFF;
    pszSend[nSendLen++] = 255;
    pszSend[nSendLen++] = bCmdIndex;
    if (dwCmdDataLen > 0 && pszCmdData)
    {
        memcpy(&pszSend[nSendLen], pszCmdData, dwCmdDataLen);
        nSendLen += dwCmdDataLen;
    }
    pszSend[nSendLen] = CalcSimpleCheckSum(pszSend, nSendLen);
    nSendLen++;
    lRet = m_pDev->Send((LPCSTR)pszSend, nSendLen, TIMEOUT_WRITEDATA);
    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "%s: Send()出错，返回%ld", pszCallFunc, lRet);
        return ConvertErrorCode(lRet);
    }
    if (lRet < (long)nSendLen)
    {
        Log(ThisModule, __LINE__, "%s: Send()出错，数据长度校验出错", pszCallFunc);
        return ERR_DEVPORT_READERR;
    }
    //设备会先将接收到的命令原样返回
    nRecvLen = nReqRecvLen = nSendLen;
    lRet = m_pDev->Read((LPSTR)pszRecv, nRecvLen, dwTimeOut);
    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "%s: Read()出错，返回%ld", pszCallFunc, lRet);
        return ConvertErrorCode(lRet);
    }
    if ((nRecvLen != nReqRecvLen) || memcmp(pszRecv, pszSend, nReqRecvLen))
    {
        Log(ThisModule, __LINE__, "%s: Read()出错，应答指令校验错", pszCallFunc);
        return ERR_DEVPORT_READERR;
    }
    memset(pszRecv, 0, sizeof(pszRecv));
    //开始接收应答
    if (dwInOutRespLen > 0)  //当有明确的应答命令长度时，按长度直接读取否则按应答命令中的数据长度读取
    {
        nRecvLen = nReqRecvLen = dwInOutRespLen;
        lRet = m_pDev->Read((LPSTR)pszRecv, nRecvLen, dwTimeOut);
        if (lRet < 0)
        {
            Log(ThisModule, __LINE__, "%s: Read()出错，返回%ld", pszCallFunc, lRet);
            return ConvertErrorCode(lRet);
        }
        if (nRecvLen < nReqRecvLen)
        {
            Log(ThisModule, __LINE__, "%s: Read()出错，未读取到指定长度", pszCallFunc);
            return ERR_DEVPORT_READERR;
        }
    }
    else
    {
        nRecvLen = nReqRecvLen = 4;
        lRet = m_pDev->Read((LPSTR)pszRecv, nRecvLen, dwTimeOut);
        if (lRet < 0)
        {
            Log(ThisModule, __LINE__, "%s: Read()出错，返回%d", pszCallFunc, lRet);
            return ConvertErrorCode(lRet);
        }
        if (nRecvLen < nReqRecvLen)
        {
            Log(ThisModule, __LINE__, "%s: Read()出错，未读取到指定长度", pszCallFunc);
            return ERR_DEVPORT_READERR;
        }
        nRecvLen = nReqRecvLen = pszRecv[1] + 1;
        lRet = m_pDev->Read((LPSTR)(pszRecv + 4), nRecvLen, dwTimeOut);
        if (lRet < 0)
        {
            Log(ThisModule, __LINE__, "%s: Read()出错，返回%d", pszCallFunc, lRet);
            return ConvertErrorCode(lRet);
        }
        if (nRecvLen < nReqRecvLen)
        {
            Log(ThisModule, __LINE__, "%s: Read()出错，未读取到指定长度", pszCallFunc);
            return ERR_DEVPORT_READERR;
        }
    }
    if ((255 != pszRecv[0]) || (55 != pszRecv[2]) || (CalcSimpleCheckSum(pszRecv, nRecvLen + 4) != 0))
    {
        Log(ThisModule, __LINE__, "%s: Read()出错，应答指令校验错", pszCallFunc);
        return ERR_DEVPORT_READERR;
    }
    if (0 != pszRecv[3])
    {
        return ConvertErrorCode((long)pszRecv[3]);
    }
    dwInOutRespLen = pszRecv[1];
    memcpy(pszResponse, &pszRecv[4], dwInOutRespLen);
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::Reset(ulong ulAction, void *pData)
{
    THISMODULE(__FUNCTION__);
    LOGDEVACTION();
    AutoLogFuncBeginEnd();
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    //
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    //
    lRet = Excute(ThisModule, 1, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    //以下做法是避免SP在初始化期间的查询出错
    std::this_thread::sleep_for(std::chrono::seconds(3));
    for (;;)
    {
        ST_DEV_ACTIV_STATUS sas;
        if (0 != (lRet = GetActiveStatus(sas)))
            break;
        if (sas.bInitialising)
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }
        break;
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::Cancel(ulong ulReasion, void *pData)
{
    m_SemCancel.SetEvent();
    return 0;
}
long CDevCOR_BCR106::GetDeviceCode()
{
    return 1;
}
long CDevCOR_BCR106::GetDevInfo(ST_DEV_INFO &stDevInfo)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    DWORD k = 0;
    lRet = Excute(ThisModule, 246, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    memcpy(stDevInfo.pszManufacturer, pszRespond, dwInoutRespLen);
    memset(pszRespond, 0, MAX_CMD_BUFFER_LEN);
    dwInoutRespLen = 0;
    lRet = Excute(ThisModule, 244, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    memcpy(stDevInfo.pszProductName, pszRespond, dwInoutRespLen);
    k = dwInoutRespLen;
    stDevInfo.pszProductName[k] = ' ';
    k++;
    memset(pszRespond, 0, MAX_CMD_BUFFER_LEN);
    dwInoutRespLen = 0;
    lRet = Excute(ThisModule, 192, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    memcpy(&(stDevInfo.pszProductName[k]), pszRespond, dwInoutRespLen);
    memset(stDevInfo.pszCreationDate, 0, MAX_DEV_INFO_LENGTH);
    memset(stDevInfo.pszRepairDate, 0, MAX_DEV_INFO_LENGTH);
    return DEV_SUCCESS;
}

long CDevCOR_BCR106::Rollback(DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM])
{
    return SOFT_ERROR_UNSUPP;
}
long CDevCOR_BCR106::GetFWVersion(char pszFWVersion[MAX_LEN_FWVERSION])
{
    THISMODULE(__FUNCTION__);
    LOGDEVACTION();
    ST_DEV_ACTIV_STATUS sas;
    long lRet = 0;
    if (0 != (lRet = GetActiveStatus(sas)))
        return lRet;
    if (sas.bInitialising)
    {
        return SOFT_ERROR_INOP;
    }
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    lRet = Excute(ThisModule, 241, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    memcpy(pszFWVersion, pszRespond, dwInoutRespLen);
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetDeviceCaps(void *pCaps)
{
    PST_COR_CAPS pCorCap = static_cast<PST_COR_CAPS>(pCaps);
    ST_CDMCAPS *pCDM = pCorCap;
    ST_CIMCAPS *pCIM = pCorCap;
    pCDM->wMaxDispenseItems = 400;
    pCDM->bShutter = FALSE;
    pCDM->bShutterControl = TRUE;
    pCDM->fwRetractAreas = COR_RA_NOTSUPP;
    pCDM->fwRetractTransportActions = 0x0008;
    pCDM->fwRetractStackerActions = 0x0008;
    pCDM->bSafeDoor = FALSE;
    pCDM->bIntermediateStacker = FALSE;
    pCDM->bItemsTakenSensor = FALSE;
    pCDM->fwPositions = COR_POSLEFT;  // WFS_CDM_POSNULL
    pCDM->fwMoveItems = 0;
    pCIM->wMaxCashInItems = 200;
    pCIM->bShutter = FALSE;
    pCIM->bShutterControl = TRUE;
    pCIM->bSafeDoor = FALSE;
    pCIM->fwIntermediateStacker = 0;
    pCIM->bItemsTakenSensor = FALSE;
    pCIM->bItemsInsertedSensor = FALSE;
    pCIM->fwPositions = COR_POSLEFT | COR_POSOUTLEFT;
    pCIM->fwRetractAreas = COR_RA_NOTSUPP;     // WFS_CIM_RA_NOTSUPP
    pCIM->fwRetractTransportActions = 0x0004;  // WFS_CDM_NOTSUPP
    pCIM->fwRetractStackerActions = 0x0004;    // WFS_CDM_NOTSUPP
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetTimeClock(time_t &tClock)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 9;
    lRet = Excute(ThisModule, 115, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (4 != dwInoutRespLen)
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
    }
    tClock = pszRespond[0] + 0x100 * pszRespond[1] + 0x10000 * pszRespond[2] + 0x1000000 * pszRespond[3];
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::SetTimeClock(time_t tClock)
{
    THISMODULE(__FUNCTION__);
    return SOFT_ERROR_UNSUPP;
}
long CDevCOR_BCR106::GetActiveStatus(ST_DEV_ACTIV_STATUS &stDevStatus)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    //
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 7;
    lRet = Excute(ThisModule, 123, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (2 != dwInoutRespLen)
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
    }
    memset(&stDevStatus, 0, sizeof(stDevStatus));
    if (pszRespond[0] & 0x01)
        stDevStatus.bSingulRun = 1;
    if (pszRespond[0] & 0x02)
        stDevStatus.bEscalRun = 1;
    if (pszRespond[0] & 0x04)
        stDevStatus.bMoneyIn = 1;
    if (pszRespond[0] & 0x08)
        stDevStatus.bMoneyOut = 1;
    if (pszRespond[0] & 0x10)
        stDevStatus.bCoinInErr = 1;
    if (pszRespond[0] & 0x20)
        stDevStatus.bCoinOutErr = 1;
    if (pszRespond[0] & 0x40)
        stDevStatus.bInitialising = 1;
    if (pszRespond[0] & 0x80)
        stDevStatus.bEntryFlapOpen = 1;
    if (pszRespond[1] & 0x01)
        stDevStatus.bContReject = 1;
    if (pszRespond[1] & 0x02)
        stDevStatus.bConfChange = 1;
    if (pszRespond[1] & 0x04)
        stDevStatus.bRejeDivert = 1;
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::Poll()
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 254, pszCmd, 0, pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::SelfCheck(BYTE &ucErrorCode, BYTE &ucExInfo)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    lRet = Excute(ThisModule, 232, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (0 == dwInoutRespLen)
    {
        return ERR_DEVPORT_READERR;
    }
    ucErrorCode = pszRespond[0];
    if (2 == dwInoutRespLen)
    {
        ucExInfo = pszRespond[1];
    }
    else
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetCoinCylinderList(ST_COINCYLINDER_INFO pCoinCylinderInfo[MAX_COINCYLINDER_NUM])
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};

    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    ST_DEV_ACTIV_STATUS sas;
    if (0 != (lRet = GetActiveStatus(sas)))
        return lRet;
    if (sas.bInitialising)
    {
        return SOFT_ERROR_INOP;
    }

    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        dwInoutRespLen = 13;
        //pszCmd[0] = (i + 1) & 0xFF;
        pszCmd[0] = m_iHopperNo[i] & 0xFF;
        lRet = Excute(ThisModule, 119, pszCmd, 1, pszRespond, dwInoutRespLen);
        if (DEV_SUCCESS != lRet)
        {
            return lRet;
        }
        if (8 > dwInoutRespLen)
        {
            Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
            return ERR_DEVPORT_READERR;
        }
        //pCoinCylinderInfo[i].iCylinderNO = i + 1;
        pCoinCylinderInfo[i].iCylinderNO = m_iHopperNo[i];
        if (0 == memcmp((BYTE *)"\x00\x00\x00\x00\x00\x00", pszRespond, 6))
        {
            pCoinCylinderInfo[i].iStatus = COR_UNIT_STATCUMISSING;
            pCoinCylinderInfo[i].iCoinCode = COIN_CODE_00;
            pCoinCylinderInfo[i].iCashValue = 0;
            pCoinCylinderInfo[i].lCount = 0;
            pCoinCylinderInfo[i].iType = COR_UT_TYPENA;
        }
        else
        {
            if ((0 == memcmp("CN010C", pszRespond, 6)) || (0 == memcmp("CN010D", pszRespond, 6)))
            {
                pCoinCylinderInfo[i].iCoinCode = COIN_CODE_CN010C;
                pCoinCylinderInfo[i].iCashValue = 10;
            }
            //NOTE! Coin 50 suppot two versions now .
            else if ((0 == memcmp("CN050C", pszRespond, 6)) || (0 == memcmp("CN050B", pszRespond, 6)))
            {
                pCoinCylinderInfo[i].iCoinCode = COIN_CODE_CN050C;
                pCoinCylinderInfo[i].iCashValue = 50;
            }
            else if (0 == memcmp("CN100B", pszRespond, 6))
            {
                pCoinCylinderInfo[i].iCoinCode = COIN_CODE_CN100B;
                pCoinCylinderInfo[i].iCashValue = 100;
            }
            else
            {
                pCoinCylinderInfo[i].iCoinCode = COIN_CODE_00;
            }
            pCoinCylinderInfo[i].iStatus = COR_UNIT_STATCUOK;
            pCoinCylinderInfo[i].lCount = MAKEWORD(pszRespond[6], pszRespond[7]);
            pCoinCylinderInfo[i].iType = COR_UT_TYPECOINCYLINDER;
        }
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::SetCoinCylinderInfo(ST_COINCYLINDER_INFO stCoinCylinderInfo)
{
    THISMODULE(__FUNCTION__);
    LOGDEVACTION();
    AutoLogFuncBeginEnd();
    long lRet = 0;
    ST_COINCYLINDER_INFO stCyliInfo;
    lRet = GetCoinCylinderInfo(stCoinCylinderInfo.iCylinderNO, stCyliInfo);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if ((stCoinCylinderInfo.iCoinCode != stCyliInfo.iCoinCode) || (stCoinCylinderInfo.iCashValue != stCyliInfo.iCashValue))
    {
        Log(ThisModule, __LINE__, "%d号币筒信息错误, 设置币种=%d（参考值=%d）, 设置面额=%d（参考值=%d）", stCoinCylinderInfo.iCylinderNO,
            stCoinCylinderInfo.iCoinCode, stCyliInfo.iCoinCode, stCoinCylinderInfo.iCashValue, stCyliInfo.iCashValue);
        return SOFT_ERROR_PARAMS;
    }
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    dwInoutRespLen = 5;
    pszCmd[0] = stCoinCylinderInfo.iCylinderNO & 0xFF;
    pszCmd[1] = LOBYTE(stCoinCylinderInfo.lCount);
    pszCmd[2] = HIBYTE(stCoinCylinderInfo.lCount);
    lRet = Excute(ThisModule, 120, pszCmd, 3, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::SetCashValueInDev(ULONG ulCashValue)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    pszCmd[0] = LOBYTE(LOWORD(ulCashValue));
    pszCmd[1] = HIBYTE(LOWORD(ulCashValue));
    pszCmd[2] = LOBYTE(HIWORD(ulCashValue));
    pszCmd[3] = HIBYTE(HIWORD(ulCashValue));
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 118, pszCmd, 4, pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::GetCashValueInDev(ULONG &ulCashValue)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 9;
    lRet = Excute(ThisModule, 117, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (4 > dwInoutRespLen)
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
        ;
    }
    ulCashValue = ((ULONG)(MAKEWORD(pszRespond[0], pszRespond[1]) & 0xFFFF)) | (((ULONG)(MAKEWORD(pszRespond[2], pszRespond[3]) & 0xFFFF)) << 16);
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::CoinIn(USHORT usCoinEnable)
{
    THISMODULE(__FUNCTION__);
    //LOGDEVACTION();
    AutoLogFuncBeginEnd();
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    USHORT usInhibit = 0;
    BYTE ucErrCode, ucErrDevNo;
    ST_DEV_ACTIV_STATUS stActive;
    lRet = GetActiveStatus(stActive);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (stActive.bInitialising || stActive.bMoneyIn || stActive.bMoneyOut)
    {
        Log(ThisModule, __LINE__, "设备忙，正在进出币或初始化");
        return SOFT_ERROR_INOP;
    }
    lRet = GetCoinPosition(m_stCoinPos);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (usCoinEnable & COIN_CN005A)
        usInhibit = (usInhibit | m_stCoinPos.usCN005A);
    if (usCoinEnable & COIN_CN010B)
        usInhibit = (usInhibit | m_stCoinPos.usCN010B);
    if (usCoinEnable & COIN_CN010C)
        usInhibit = (usInhibit | m_stCoinPos.usCN010C);
    if (usCoinEnable & COIN_CN010D)
        usInhibit = (usInhibit | m_stCoinPos.usCN010D);
    if (usCoinEnable & COIN_CN050B)
        usInhibit = (usInhibit | m_stCoinPos.usCN050B);
    if (usCoinEnable & COIN_CN050C)
        usInhibit = (usInhibit | m_stCoinPos.usCN050C);
    if (usCoinEnable & COIN_CN050D)
        usInhibit = (usInhibit | m_stCoinPos.usCN050D);
    if (usCoinEnable & COIN_CN100B)
        usInhibit = (usInhibit | m_stCoinPos.usCN100B);
    if (usCoinEnable & COIN_CN100C)
        usInhibit = (usInhibit | m_stCoinPos.usCN100C);
    pszCmd[0] = LOBYTE(usInhibit);
    pszCmd[1] = HIBYTE(usInhibit);
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 231, pszCmd, 2, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    //    delay(500);
    lRet = GetActiveStatus(stActive);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (stActive.bEntryFlapOpen)
    {
        return DEV_SUCCESS;
    }
    lRet = GetErr(ucErrDevNo, ucErrCode);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    switch (ucErrCode)
    {
    case 0:
        return 0;
    case 252:
        Log(ThisModule, __LINE__, "没有币箱");
        return ERR_COR_NOCASHBOXPRESENT;
    case 101:
        Log(ThisModule, __LINE__, "接收口发生阻塞");
        return ERR_COR_NOTDISPENSABLE;
    case 102:
        Log(ThisModule, __LINE__, "检测到非法存入");
        return ERR_COR_ACCEPTOR_ILLGGAL;
    case 103:
        Log(ThisModule, __LINE__, "存入器故障");
        return ERR_COR_ACCEPT_ERROR;
    case 251:
        Log(ThisModule, __LINE__, "币箱满");
        return ERR_COR_COIN_BOX_FULL;
    default:
        Log(ThisModule, __LINE__, "查询到未定义状态:%d", ucErrCode);
    }
    return HARDWAREERRORCODE(0x4000 | ucErrCode);
}
long CDevCOR_BCR106::CoinInEnd()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    pszCmd[0] = 0;
    pszCmd[1] = 0;
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 231, pszCmd, 2, pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::OpenClap()
{
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::CloseClap()
{
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::SetSupportCoinTypes(USHORT usCoinEnable)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_usCoinTypeSupported = usCoinEnable;
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::CashIn(DWORD dwTimeOut, ULONG RecvItems[MAX_COINCYLINDER_NUM], ST_RETRACTBIN_COUNT &stRetract,
                            DEV_CUERROR eCUError[MAX_COINCYLINDER_NUM], OnCashIn fnCallback)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();
    BYTE errorCode, errExtr;
    long lRet;
    if (0 == (lRet = SelfCheck(errorCode, errExtr)))
    {
        if (errorCode != 0)
        {
            return HARDWAREERRORCODE((errExtr << 8) & 0x0F | errorCode);
        }
    }
    while (m_SemCancel.WaitForEvent(100))
        ;
    int i, iNum;
    ST_DEV_ACTIV_STATUS stStatus;
    ST_COINCYLINDER_INFO stInfo_S[MAX_COINCYLINDER_NUM], stInfo_E[MAX_COINCYLINDER_NUM];
    ST_RETRACTBIN_COUNT stMoneyCount;
    ULONG ulMoney_S = 0, ulMoney = 0, ulMoneyIn;
    BYTE iCyliNo = 0, iErrCode = 0;
    stMoneyCount.ulCN010 = 0;
    stMoneyCount.ulCN050 = 0;
    stMoneyCount.ulCN100 = 0;
    memset(RecvItems, 0x00, sizeof(RecvItems));
    stRetract.ulCN010 = 0;
    stRetract.ulCN050 = 0;
    stRetract.ulCN100 = 0;

    lRet = GetCoinCylinderList(stInfo_S);
    memcpy(stInfo_E, stInfo_S, sizeof(stInfo_S));
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    lRet = GetMoneyCount(MONEY_COUNTERS_MONEYIN, ulMoney_S);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    for (i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        eCUError[i] = DEV_CUERROR_OK;
    }
    lRet = CoinIn(m_usCoinTypeSupported);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    QTime qT = QTime::currentTime();
    qT.start();
    while (qT.elapsed() < dwTimeOut)
    {
        if (m_SemCancel.WaitForEvent(100))
        {
            Log(ThisModule, __LINE__, "用户取消存入");
            break;
        }
        lRet = GetActiveStatus(stStatus);
        if (DEV_SUCCESS != lRet)
        {
            Log(ThisModule, __LINE__, "持续检查状态失败(%d)，终结存入", lRet);
            break;
        }
        else if (FALSE == stStatus.bMoneyIn)
        {
            Log(ThisModule, __LINE__, "检测到存入状态终止");
            lRet = GetErr(iCyliNo, iErrCode);
            if (0 != iErrCode)
            {
                if ((iCyliNo > 0) && (iCyliNo <= 6))
                {
                    //iNum = iCyliNo - 1;
                    iNum = MapHopperNo2Index(iCyliNo);
                }
                else if ((100 == iCyliNo) || (250 == iCyliNo))
                {
                    iNum = 0;
                }
                else
                {
                    Log(ThisModule, __LINE__, "获取错误码异常");
                    return HARDWAREERRORCODE(ERROR_HW_ERROR);
                }
                //Log(ThisModule, __LINE__, "存币异常，筒号=%d，错误码=%d", iNum, iErrCode);
                Log(ThisModule, __LINE__, "存币异常，筒号=%d，错误码=%d", iCyliNo, iErrCode);
                switch (iErrCode)
                {
                case 1:
                    eCUError[iNum] = DEV_CUERROR_EMPTY;
                    break;
                case 3:
                case 102:
                    eCUError[iNum] = DEV_CUERROR_RETRYABLE;
                    break;
                case 2:
                case 4:
                case 101:
                case 103:
                    eCUError[iNum] = DEV_CUERROR_FATAL;
                    break;
                case 251:
                    eCUError[iNum] = DEV_CUERROR_FULL;
                    break;
                case 252:
                    eCUError[iNum] = DEV_CUERROR_NOTUSED;
                    break;
                default:
                    eCUError[iNum] = DEV_CUERROR_RETRYABLE;
                    break;
                }
                return HARDWAREERRORCODE(0x4000 | iErrCode);
            }
            break;
        }
        lRet = GetMoneyCount(MONEY_COUNTERS_MONEYIN, ulMoney);
        if (DEV_SUCCESS != lRet)
        {
            return lRet;
        }
        if (ulMoney > ulMoney_S)
        {
            ulMoneyIn = ulMoney - ulMoney_S;
            int type = 0;
            int iLeft = 1;
            switch (ulMoneyIn)
            {
            case 100:
                stMoneyCount.ulCN100 += 1;
                type = COIN_CODE_CN100B;
                break;
            case 50:
                stMoneyCount.ulCN050 += 1;
                type = COIN_CODE_CN050C;
                break;
            case 10:
                stMoneyCount.ulCN010 += 1;
                type = COIN_CODE_CN010C;
                break;
            default:
                Log(ThisModule, __LINE__, "一个未知的面额存入. 面额=%d", (ulMoney - ulMoney_S));
                break;
            }
            ulMoney_S = ulMoney;
            if (fnCallback != nullptr)
            {
                ST_COINCYLINDER_INFO after[MAX_COINCYLINDER_NUM];
                if (0 == GetCoinCylinderList(after))
                {
                    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
                    {
                        if (stInfo_E[i].iCylinderNO != 0 && stInfo_E[i].iStatus < 5 && stInfo_E[i].lCount != after[i].lCount)
                        {
                            iLeft -= after[i].lCount - stInfo_E[i].lCount;
                            fnCallback(stInfo_E[i].iCylinderNO, stInfo_E[i].iCoinCode, after[i].lCount - stInfo_E[i].lCount);
                            stInfo_E[i].lCount = after[i].lCount;
                        }
                    }
                    if (iLeft > 0)
                    {
                        fnCallback(0, type, iLeft);
                    }
                }
            }
        }
    }
    lRet = CoinInEnd();
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    lRet = GetCoinCylinderList(stInfo_E);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    for (i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        if (stInfo_E[i].lCount < stInfo_S[i].lCount)
        {
            Log(ThisModule, __LINE__, "%号币筒计数异常，存币前计数为%ld，存币后计数为%ld", stInfo_E[i].iCylinderNO, stInfo_E[i].lCount, stInfo_S[i].lCount);
            return ERR_COR_CASHUNITERROR;
        }
        RecvItems[i] = (stInfo_E[i].lCount - stInfo_S[i].lCount);
        switch (stInfo_E[i].iCashValue)
        {
        case 100:
            if (stMoneyCount.ulCN100 < RecvItems[i])
            {
                Log(ThisModule, __LINE__, "CN100存币计数异常，软件计数=%ld, 币箱计数=%ld", stMoneyCount.ulCN100, RecvItems[i]);
                stMoneyCount.ulCN100 = 0;
            }
            else
            {
                stMoneyCount.ulCN100 -= RecvItems[i];
            }
            break;
        case 50:
            if (stMoneyCount.ulCN050 < RecvItems[i])
            {
                Log(ThisModule, __LINE__, "CN50存币计数异常，软件计数=%ld, 币箱计数=%ld", stMoneyCount.ulCN050, RecvItems[i]);
                stMoneyCount.ulCN050 = 0;
            }
            else
            {
                stMoneyCount.ulCN050 -= RecvItems[i];
            }
            break;
        case 10:
            if (stMoneyCount.ulCN010 < RecvItems[i])
            {
                Log(ThisModule, __LINE__, "CN10存币计数异常，软件计数=%ld, 币箱计数=%ld", stMoneyCount.ulCN010, RecvItems[i]);
                stMoneyCount.ulCN010 = 0;
            }
            else
            {
                stMoneyCount.ulCN010 -= RecvItems[i];
            }
            break;
        default:
            break;
        }
    }
    stRetract = stMoneyCount;
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetClapStatus(USHORT &usInhibit)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 7;
    lRet = Excute(ThisModule, 231, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    usInhibit = MAKEWORD(pszRespond[0], pszRespond[1]);
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::Payout(ULONG ulCoinValue)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    ST_DEV_ACTIV_STATUS stActive;
    lRet = GetActiveStatus(stActive);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (stActive.bInitialising || stActive.bMoneyIn || stActive.bMoneyOut)
    {
        Log(ThisModule, __LINE__, "设备忙，正在初始化或进出币");
        return SOFT_ERROR_INOP;
    }
    pszCmd[0] = LOBYTE(ulCoinValue);
    pszCmd[1] = HIBYTE(ulCoinValue);
    pszCmd[2] = LOBYTE(HIWORD(ulCoinValue));
    pszCmd[3] = HIBYTE(HIWORD(ulCoinValue));
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 125, pszCmd, 4, pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::GetPayoutValue(ULONG &ulCoinValue)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 14;
    lRet = Excute(ThisModule, 124, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    ulCoinValue = ((ULONG)(MAKEWORD(pszRespond[1], pszRespond[2]) & 0xFFFF)) | (((ULONG)(MAKEWORD(pszRespond[3], pszRespond[4]) & 0xFFFF)) << 8);
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetMoneyCount(MONEY_COUNTERS_TYPE iCounters, ULONG &ulMoneyValue)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 9;
    if (MONEY_COUNTERS_MONEYIN == iCounters)
    {
        lRet = Excute(ThisModule, 128, pszCmd, 0, pszRespond, dwInoutRespLen);
    }
    else if (MONEY_COUNTERS_MONEYOUT == iCounters)
    {
        lRet = Excute(ThisModule, 127, pszCmd, 0, pszRespond, dwInoutRespLen);
    }
    else
    {
        Log(ThisModule, __LINE__, "不支持的参数，iCounters=%d", (int)iCounters);
        return SOFT_ERROR_PARAMS;
    }
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (4 > dwInoutRespLen)
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
    }
    ulMoneyValue = ((ULONG)(MAKEWORD(pszRespond[0], pszRespond[1]) & 0xFFFF)) | (((ULONG)(MAKEWORD(pszRespond[2], pszRespond[3]) & 0xFFFF)) << 16);
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::ClearMoneyCounter()
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 126, pszCmd, 0, pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::PurgeHopper(BYTE bCylinderNO, BYTE bCount)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    pszCmd[0] = bCylinderNO;
    pszCmd[1] = bCount;
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 121, pszCmd, 2, pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::GetErr(BYTE &iDeviceNo, BYTE &bErrorCode)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    //
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 7;
    //
    lRet = Excute(ThisModule, 122, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (2 != dwInoutRespLen)
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
    }
    iDeviceNo = pszRespond[0];
    bErrorCode = pszRespond[1];
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetDataStorageAvai(int &iMemoryType, int &iReadBlocks, int &iReadBlockSize, int &iWriteBlocks, int &iWriteBlockSize)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 10;
    lRet = Excute(ThisModule, 216, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (5 != dwInoutRespLen)
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
    }
    iMemoryType = pszRespond[0];
    iReadBlocks = pszRespond[1];
    iReadBlockSize = pszRespond[2];
    iWriteBlocks = pszRespond[3];
    iWriteBlockSize = pszRespond[4];
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::WriteData(int iBlockNo, BYTE *szData, int iLen)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    if (iBlockNo > 255 || iLen > 251)
    {
        Log(ThisModule, __LINE__, "不支持的参数，块=%d，行=%d", iBlockNo, iLen);
        return SOFT_ERROR_PARAMS;
    }
    pszCmd[0] = iBlockNo & 0xFF;
    memcpy(&pszCmd[1], szData, iLen);
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 214, pszCmd, (iLen + 1), pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::ReadData(int iBlockNo, BYTE szData[MAX_DATA_LENGTH], int &iLen)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    if (iBlockNo > 255)
    {
        Log(ThisModule, __LINE__, "不支持的参数，块号=%d", iBlockNo);
        return SOFT_ERROR_PARAMS;
    }
    pszCmd[0] = iBlockNo & 0xFF;
    lRet = Excute(ThisModule, 215, pszCmd, 1, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (MAX_DATA_LENGTH < dwInoutRespLen)
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
    }
    memcpy(szData, pszRespond, dwInoutRespLen);
    iLen = (int)(dwInoutRespLen);
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::FWLoadStart()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    //判断设备是否支持在线下载
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 141, pszCmd, 0, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (1 != pszRespond[0])
    {
        return SOFT_ERROR_UNSUPP;
    }
    //启动固件下载
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 139, pszCmd, 0, pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::FWLoadSendData(BYTE uclock, BYTE ucLine, const BYTE *szData, BYTE ucDataLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    if (ucDataLen > 128)
    {
        Log(ThisModule, __LINE__, "参数不支持,数据长度=%d", ucDataLen);
        return SOFT_ERROR_PARAMS;
    }
    pszCmd[0] = uclock;
    pszCmd[1] = ucLine;
    memcpy(&pszCmd[2], szData, ucDataLen);
    dwInoutRespLen = 5;
    lRet = Excute(ThisModule, 140, pszCmd, ucDataLen + 2, pszRespond, dwInoutRespLen);
    return lRet;
}
long CDevCOR_BCR106::FWLoadEnd()
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 5;

    lRet = Excute(ThisModule, 138, pszCmd, 0, pszRespond, dwInoutRespLen);
    return lRet;
}

void CDevCOR_BCR106::SetDeviceConfig(void *pConfig) {}

long CDevCOR_BCR106::IsNeedDevicePDL()
{
    return 0;
}

long CDevCOR_BCR106::GetLastError()
{
    return m_lastStatusError;
}
long CDevCOR_BCR106::UpdateDevicePDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    BYTE szDataBuff[128] = {0};
    BYTE ucBlockNum = 0, ucLine = 0;
    BOOL bLastData = FALSE;
    long lDownLoadedLen = 0;
    FILE *fp = fopen("./CORUpdateDevicePDL.bin", "rb");
    if (!fp)
    {
        Log(ThisModule, __LINE__, "打开指定文件失败");
        return SOFT_ERROR_FW_NOFILE;
    }
    //开始下载固件程序
    int iRet = FWLoadStart();
    if (iRet < 0)
    {
        fclose(fp);
        return iRet;
    }
    fseek(fp, 0L, SEEK_END);
    long nLen = ftell(fp);
    if (nLen == 0)
    {
        FWLoadEnd();
        Log(ThisModule, __LINE__, "文件空");
        return SOFT_ERROR_FW_ILLGE;
    }
    fseek(fp, 0, SEEK_SET);
    while (!bLastData)
    {
        int iReadLen = fread(szDataBuff, sizeof(BYTE), 128, fp);
        if (ferror(fp))
        {
            FWLoadEnd();
            Log(ThisModule, __LINE__, "读取文件数据发生错误");
            return ERROR_FW_FAILED;
        }
        if (feof(fp))  // 已到文件末尾
        {
            bLastData = TRUE;
        }
        iRet = FWLoadSendData(ucBlockNum, ucLine, szDataBuff, iReadLen);
        if (iRet < 0)
        {
            fclose(fp);
            FWLoadEnd();
            return iRet;
        }
        if (255 == ucLine)  //每块256行，每行128字节
        {
            ucBlockNum++;
            ucLine = 0;
        }
        else
        {
            ucLine++;
        }
        lDownLoadedLen += iReadLen;
    }
    fclose(fp);
    iRet = FWLoadEnd();
    if (iRet < 0)
    {
        return iRet;
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetStatus(void *pStatus)
{
    THISMODULE(__FUNCTION__);
    long lRet = 0;
    PST_COR_STATUS pCorStatus = static_cast<PST_COR_STATUS>(pStatus);
    pCorStatus->fwDevice = COR_DEVONLINE;
    pCorStatus->fwSafeDoor = COR_SOFTDOORNOTSUPPORTED;
    pCorStatus->fwIntermediateStacker = COR_STACKER_ISNOTSUPPORTED;
    pCorStatus->fwStackerItems = COR_STACKITEM_NOITEMS;
    pCorStatus->fwBanknoteReader = COR_BNRNOTSUPPORTED;
    pCorStatus->fwAcceptor = COR_ACCCUUNKNOWN;
    pCorStatus->fwDispenser = COR_DISPENSER_DISPCUUNKNOWN;
    pCorStatus->bDropBox = FALSE;
    pCorStatus->fwPosition = 0;
    pCorStatus->fwShutter = COR_SHTUNKNOWN;
    pCorStatus->fwPositionStatus = COR_PSNOTSUPPORTED;
    pCorStatus->fwTransport = COR_TPNOTSUPPORTED;
    pCorStatus->fwTransportStatus = COR_TPSTATNOTSUPPORTED;
    ST_DEV_ACTIV_STATUS stActStatus;

    if (!m_bIsOpen)
    {
        lRet = m_pDev->Open(m_pszMode);
        if (DEV_SUCCESS != lRet)
        {
            pCorStatus->fwDevice = COR_DEVOFFLINE;
            pCorStatus->fwShutter = COR_SHTUNKNOWN;
            return DEV_SUCCESS;
        }
        else
        {
            m_bIsOpen = TRUE;
            pCorStatus->fwDevice = COR_DEVONLINE;
        }
    }

    lRet = GetActiveStatus(stActStatus);
    if (DEV_SUCCESS != lRet)
    {
        m_pDev->Close();
        m_bIsOpen = FALSE;
        pCorStatus->fwDevice = COR_DEVOFFLINE;
        pCorStatus->fwShutter = COR_SHTUNKNOWN;
        return DEV_SUCCESS;
    }
    if (stActStatus.bEntryFlapOpen)
    {
        pCorStatus->fwShutter = COR_SHTOPEN;
    }
    else
    {
        pCorStatus->fwShutter = COR_SHTCLOSED;
    }
    if (stActStatus.bInitialising)  // || stActStatus.bMoneyIn || stActStatus.bMoneyOut)
    {
        pCorStatus->fwDispenser = COR_DISPENSER_DISPCUSTOP;
        pCorStatus->fwAcceptor = COR_ACCCUSTOP;
        pCorStatus->fwDevice = COR_DEVBUSY;
        return DEV_SUCCESS;
    }
    pCorStatus->fwDispenser = COR_DISPENSER_DISPOK;
    pCorStatus->fwAcceptor = COR_ACCOK;
    if (stActStatus.bMoneyIn)
        pCorStatus->fwDispenser = COR_DISPENSER_DISPCUSTOP;
    if (stActStatus.bCoinInErr)
        pCorStatus->fwAcceptor = COR_ACCCUSTOP;
    if (stActStatus.bCoinOutErr)
        pCorStatus->fwDispenser = COR_DISPENSER_DISPCUSTOP;
    if (stActStatus.bMoneyOut)
        pCorStatus->fwAcceptor = COR_ACCCUSTOP;
    if (stActStatus.bSingulRun || stActStatus.bEscalRun)
        pCorStatus->fwDevice = COR_DEVBUSY;

    BYTE ucErr = 0, ucExInfo = 0;
    lRet = SelfCheck(ucErr, ucExInfo);
    if (DEV_SUCCESS != lRet)
    {
        pCorStatus->fwDevice = COR_DEVOFFLINE;
        pCorStatus->fwShutter = COR_SHTUNKNOWN;
        return lRet;
    }
    if (0 == ucErr)
    {
        pCorStatus->fwDispenser = COR_DISPENSER_DISPOK;
        pCorStatus->fwAcceptor = COR_ACCOK;
        pCorStatus->fwDevice = COR_DEVONLINE;
    }
    else
    {
        long lErr = (ucExInfo << 8) | ucErr;
        if (m_lastStatusError != lErr)
        {
            char buf[512];
            GetErrorString(lErr, buf);
            Log(ThisModule, __LINE__, "GetDevStatus，Err=%d ExInfo=%d, %s", ucErr, ucExInfo, QString::fromLocal8Bit(buf).toUtf8().toStdString().data());
            m_lastStatusError = lErr;
        }
        if ((51 == ucErr) || (48 == ucErr))
        {
            pCorStatus->fwDispenser = COR_DISPENSER_DISPCUSTOP;
            pCorStatus->fwAcceptor = COR_ACCCUSTOP;
            pCorStatus->fwDevice = COR_DEVUSERERROR;
        }
        else
        {
            pCorStatus->fwDispenser = COR_DISPENSER_DISPCUSTOP;
            pCorStatus->fwAcceptor = COR_ACCCUSTOP;
            pCorStatus->fwDevice = COR_DEVHWERROR;
        }
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::Dispense(const ULONG RequestItems[MAX_COINCYLINDER_NUM], ULONG DispensedItems[MAX_COINCYLINDER_NUM],
                              DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();
    ST_DEV_ACTIV_STATUS stActive;
    ST_COINCYLINDER_INFO stCoinInfo;
    DEV_CUERROR eCUError[MAX_COINCYLINDER_NUM] = {DEV_CUERROR_OK};
    long lRet = 0;
    int i = 0, k = 0;
    lRet = GetActiveStatus(stActive);
    if (DEV_SUCCESS != lRet)
    {
        Log(ThisModule, __LINE__, "请求出币请检测状态失败");
        return lRet;
    }
    if (stActive.bInitialising || stActive.bMoneyIn || stActive.bMoneyOut)
    {
        Log(ThisModule, __LINE__, "设备忙，正在进出币或初始化");
        return SOFT_ERROR_INOP;
    }
    BYTE errorCode, errExtr;
    if (0 == (lRet = SelfCheck(errorCode, errExtr)))
    {
        if (errorCode != 0)
        {
            return HARDWAREERRORCODE((errExtr << 8) & 0x0F | errorCode);
        }
    }
    for (i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        if (0 == RequestItems[i])
        {
            continue;
        }
        lRet = GetCoinCylinderInfo(m_iHopperNo[i], stCoinInfo);
        //lRet = GetCoinCylinderInfo(i + 1, stCoinInfo);
        if (DEV_SUCCESS != lRet)
        {
            //Log(ThisModule, __LINE__, "请求出币请检测币箱(%d)状态失败", i + 1);
            Log(ThisModule, __LINE__, "请求出币请检测币箱(%d)状态失败", m_iHopperNo[i]);
            return lRet;
        }
        if (1 == stCoinInfo.iStatus)
        {
            Log(ThisModule, __LINE__, "%s: 币箱不存在", ThisModule);
            return ERR_COR_NOCASHBOXPRESENT;  //需修改
        }
        if (RequestItems[i] > (ULONG)(stCoinInfo.lCount))
        {
            Log(ThisModule, __LINE__, "%s: 硬币不足", ThisModule);
            return ERR_COR_COINNOTENOUGH;
        }
    }
    for (i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        if (0 == RequestItems[i])
        {
            continue;
        }
        lRet = DispensePart(m_iHopperNo[i], RequestItems[i], DispensedItems[i], arywCUError[i]);
        if (DEV_SUCCESS != lRet)
        {
            return lRet;
        }
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::Count(int iCylinderNo, ULONG DispensedItems[MAX_COINCYLINDER_NUM], ULONG CountedItems[MAX_COINCYLINDER_NUM],
           DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    long lRet = 0;
    BOOL iTag = TRUE;
    ST_DEV_ACTIV_STATUS stActive;

    ST_COINCYLINDER_INFO pCoinCylinderInfo_S[MAX_COINCYLINDER_NUM];
    ST_COINCYLINDER_INFO pCoinCylinderInfo_E[MAX_COINCYLINDER_NUM];
    memset(pCoinCylinderInfo_S, 0, sizeof(pCoinCylinderInfo_S));
    memset(pCoinCylinderInfo_E, 0, sizeof(pCoinCylinderInfo_E));

    lRet = GetCoinCylinderList(pCoinCylinderInfo_S);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }

    // Purge hopper.
    lRet = PurgeHopper((BYTE)iCylinderNo, 0);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    while (iTag)
    {
        lRet = GetActiveStatus(stActive);
        if (DEV_SUCCESS != lRet)
        {
            return lRet;
        }
        if (!stActive.bMoneyOut)
        {
            iTag = FALSE;
            break;
        }
    }

    lRet = GetCoinCylinderList(pCoinCylinderInfo_E);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }

    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        DispensedItems[i] = pCoinCylinderInfo_S[i].lCount - pCoinCylinderInfo_E[i].lCount;
        CountedItems[i] = DispensedItems[i];
    }

    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetCoinCylinderInfo(int iCylinderNo, ST_COINCYLINDER_INFO &CoinCylinderInfo)
{
    THISMODULE(__FUNCTION__);
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    dwInoutRespLen = 13;
    pszCmd[0] = iCylinderNo & 0xFF;
    lRet = Excute(ThisModule, 119, pszCmd, 1, pszRespond, dwInoutRespLen);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    if (8 > dwInoutRespLen)
    {
        Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
        return ERR_DEVPORT_READERR;
    }
    CoinCylinderInfo.iCylinderNO = iCylinderNo;
    CoinCylinderInfo.iStatus = COR_UNIT_STATCUOK;
    CoinCylinderInfo.lCount = MAKEWORD(pszRespond[6], pszRespond[7]);
    if ((0 == memcmp("CN010C", pszRespond, 6)) || (0 == memcmp("CN010D", pszRespond, 6)))
    {
        CoinCylinderInfo.iCoinCode = COIN_CODE_CN010C;
        CoinCylinderInfo.iCashValue = 10;
    }
    else if ((0 == memcmp("CN050C", pszRespond, 6)) || (0 == memcmp("CN050B", pszRespond, 6)))
    {
        CoinCylinderInfo.iCoinCode = COIN_CODE_CN050C;
        CoinCylinderInfo.iCashValue = 50;
    }
    else if (0 == memcmp("CN100B", pszRespond, 6))
    {
        CoinCylinderInfo.iCoinCode = COIN_CODE_CN100B;
        CoinCylinderInfo.iCashValue = 100;
    }
    else
    {
        CoinCylinderInfo.iCoinCode = COIN_CODE_00;
        CoinCylinderInfo.iCashValue = 0;
        CoinCylinderInfo.lCount = 0;
        CoinCylinderInfo.iStatus = COR_UNIT_STATCUMISSING;
        return ERR_COR_CASHUNITERROR;
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::DispensePart(int iCylinderNo, ULONG RequestItems, ULONG &DispensedItems, DEV_CUERROR &arywCUError)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    long lRet = 0;
    ST_DEV_ACTIV_STATUS stActive;
    ST_COINCYLINDER_INFO stCoinInfo;
    long lCount_start = 0, lCount_end = 0;
    BYTE ucCount_req = 0;
    BOOL iTag = TRUE;
    BYTE iCyliNo = 0, iErrCode = 0;
    int i;
    lRet = GetCoinCylinderInfo(iCylinderNo, stCoinInfo);
    if (DEV_SUCCESS != lRet)
    {
        return lRet;
    }
    lCount_start = stCoinInfo.lCount;
    DispensedItems = 0;
    for (i = 0; i <= (int)(RequestItems / 255); i++)  //指令最大支持一次出255个币
    {
        if (((int)(RequestItems / 255) == i) && ((int)(RequestItems % 255) != 0))
        {
            ucCount_req = (RequestItems % 255) & 0xFF;
        }
        else
        {
            ucCount_req = 255;
        }
        lRet = PurgeHopper((BYTE)iCylinderNo, ucCount_req);
        if (DEV_SUCCESS != lRet)
        {
            return lRet;
        }
        while (iTag)
        {
            lRet = GetActiveStatus(stActive);
            if (DEV_SUCCESS != lRet)
            {
                return lRet;
            }
            if (!stActive.bMoneyOut)
            {
                iTag = FALSE;
                break;
            }
        }
        lRet = GetCoinCylinderInfo(iCylinderNo, stCoinInfo);
        if (DEV_SUCCESS != lRet)
        {
            return lRet;
        }
        lCount_end = stCoinInfo.lCount;
        if ((long)(lCount_start - lCount_end - DispensedItems) != (long)ucCount_req)
        {
            Log(ThisModule, __LINE__, "%d号币筒挖币异常：第%d次请求数量=%ld，实际挖出=%ld", iCylinderNo, i + 1, ucCount_req,
                lCount_start - lCount_end - DispensedItems);
            DispensedItems = lCount_start - lCount_end;
            break;
        }
        DispensedItems = lCount_start - lCount_end;
    }
    if (DispensedItems < RequestItems)
    {
        lRet = GetErr(iCyliNo, iErrCode);
        Log(ThisModule, __LINE__, "%d号币筒挖币异常：请求总数量=%ld，实际挖出=%ld,(%d,%d)", iCylinderNo, RequestItems, DispensedItems, iCyliNo, iErrCode);
        if ((0 == iCyliNo) && (0 == iErrCode))
        {
            arywCUError = DEV_CUERROR_OK;
            return ERR_COR_COINNOTENOUGH;
        }
        if (iCyliNo != iCylinderNo)
        {
            arywCUError = DEV_CUERROR_RETRYABLE;
            return HARDWAREERRORCODE(iErrCode | 0x4000);
        }
        switch (iErrCode)
        {
        case 1:
            arywCUError = DEV_CUERROR_EMPTY;
            break;
        case 2:
            arywCUError = DEV_CUERROR_RETRYABLE;
            break;
        case 3:
        case 4:
            arywCUError = DEV_CUERROR_FATAL;
            break;
        default:
            arywCUError = DEV_CUERROR_RETRYABLE;
            break;
        }
        return HARDWAREERRORCODE(iErrCode | 0x4000);
    }
    else
    {
        arywCUError = DEV_CUERROR_OK;
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetCoinPosition(ST_COIN_POSITION &stCoinPosition)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    BYTE pszCmd[MAX_CMD_BUFFER_LEN] = {0};
    BYTE pszRespond[MAX_CMD_BUFFER_LEN] = {0};
    DWORD dwInoutRespLen = 0;
    long lRet = 0;
    BYTE ucPosition = 1, i = 0;
    USHORT usPos1 = 0x0001;
    for (i = 0; i < 16; i++)
    {
        ucPosition = i + 1;
        pszCmd[0] = ucPosition;
        dwInoutRespLen = 11;
        lRet = Excute(ThisModule, 184, pszCmd, 1, pszRespond, dwInoutRespLen);
        if (DEV_SUCCESS != lRet)
        {
            return lRet;
        }
        if (6 > dwInoutRespLen)
        {
            Log(ThisModule, __LINE__, "应答命令数据长度错,长度=%d", dwInoutRespLen);
            return ERR_DEVPORT_READERR;
        }
        if (0 == memcmp("CN005A", pszRespond, 6))
        {
            stCoinPosition.usCN005A = usPos1 << i;
        }
        else if (0 == memcmp("CN010B", pszRespond, 6))
        {
            stCoinPosition.usCN010B = usPos1 << i;
        }
        else if (0 == memcmp("CN010C", pszRespond, 6))
        {
            stCoinPosition.usCN010C = usPos1 << i;
        }
        else if (0 == memcmp("CN010D", pszRespond, 6))
        {
            stCoinPosition.usCN010D = usPos1 << i;
        }
        else if (0 == memcmp("CN050B", pszRespond, 6))
        {
            stCoinPosition.usCN050B = usPos1 << i;
        }
        else if (0 == memcmp("CN050C", pszRespond, 6))
        {
            stCoinPosition.usCN050C = usPos1 << i;
        }
        else if (0 == memcmp("CN050D", pszRespond, 6))
        {
            stCoinPosition.usCN050D = usPos1 << i;
        }
        else if (0 == memcmp("CN100B", pszRespond, 6))
        {
            stCoinPosition.usCN100B = usPos1 << i;
        }
        else if (0 == memcmp("CN100C", pszRespond, 6))
        {
            stCoinPosition.usCN100C = usPos1 << i;
        }
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::GetErrorString(long lErrorCode, char szErrorString[MAX_LEN_BUFFER])
{
    BYTE ucErr = 0, ucExInfo;
    long lRet = IDevCOR::GetErrorString(lErrorCode, szErrorString);
    if (lRet == 0 || lRet == SOFT_ERROR_UNSUPP)
    {
        return lRet;
    }
    switch (REALERRORCODE(lErrorCode))
    {
    case 1:
        strcpy(szErrorString, QString("EEPROM数据校验错误.").toLocal8Bit().data());
        break;
    case 3:
        strcpy(szErrorString, QString("信用传感器故障").toLocal8Bit().data());
        break;
    case 4:
        strcpy(szErrorString, QString("压电传感器故障").toLocal8Bit().data());
        break;
    case 5:
        strcpy(szErrorString, QString("反射式传感器故障").toLocal8Bit().data());
        break;
    case 6:
        strcpy(szErrorString, QString("直径传感器故障").toLocal8Bit().data());
        break;
    case 7:
        strcpy(szErrorString, QString("唤醒传感器故障").toLocal8Bit().data());
        break;
    case 9:
        strcpy(szErrorString, QString("NVRAM校验错误").toLocal8Bit().data());
        break;
    case 10:
        strcpy(szErrorString, QString("硬币分配错误").toLocal8Bit().data());
        break;
    case 13:
        strcpy(szErrorString, QString("硬币计数错误.").toLocal8Bit().data());
        break;
    case 14:
        strcpy(szErrorString, QString("键盘错误.").toLocal8Bit().data());
        break;
    case 15:
        strcpy(szErrorString, QString("按钮错误.").toLocal8Bit().data());
        break;
    case 16:
        strcpy(szErrorString, QString("显示错误.").toLocal8Bit().data());
        break;
    case 17:
        strcpy(szErrorString, QString("硬币审计错误").toLocal8Bit().data());
        break;
    case 18:
        strcpy(szErrorString, QString("拒绝传感器故障").toLocal8Bit().data());
        break;
    case 19:
        strcpy(szErrorString, QString("退币机制故障").toLocal8Bit().data());
        break;
    case 20:
        strcpy(szErrorString, QString("C.O.S机制故障").toLocal8Bit().data());
        break;
    case 21:
        strcpy(szErrorString, QString("边缘传感器故障").toLocal8Bit().data());
        break;
    case 22:
        strcpy(szErrorString, QString("热敏电阻故障").toLocal8Bit().data());
        break;
    case 28:
        strcpy(szErrorString, QString("个性化模块未安装").toLocal8Bit().data());
        break;
    case 29:
        strcpy(szErrorString, QString("个性化模块校验错误").toLocal8Bit().data());
        break;
    case 30:
        strcpy(szErrorString, QString("ROM校验错误").toLocal8Bit().data());
        break;
    case 33:
        strcpy(szErrorString, QString("电源电压超出工作极限").toLocal8Bit().data());
        break;
    case 34:
        strcpy(szErrorString, QString("温度超出工作极限").toLocal8Bit().data());
        break;
    case 35:
        strcpy(szErrorString, QString("D.C.E故障").toLocal8Bit().data());
        break;
    case 37:
        strcpy(szErrorString, QString("票据传输电机故障").toLocal8Bit().data());
        break;
    case 38:
        strcpy(szErrorString, QString("暂存区故障").toLocal8Bit().data());
        break;
    case 39:
        strcpy(szErrorString, QString("票据被卡").toLocal8Bit().data());
        break;
    case 40:
        strcpy(szErrorString, QString("RAM测试失败").toLocal8Bit().data());
        break;
    case 41:
        strcpy(szErrorString, QString("string传感器故障").toLocal8Bit().data());
        break;
    case 42:
        strcpy(szErrorString, QString("接受门打开失败").toLocal8Bit().data());
        break;
    case 43:
        strcpy(szErrorString, QString("接受门关闭失败").toLocal8Bit().data());
        break;
    case 44:
        strcpy(szErrorString, QString("缺少暂存模块").toLocal8Bit().data());
        break;
    case 45:
        strcpy(szErrorString, QString("暂存满").toLocal8Bit().data());
        break;
    case 46:
        strcpy(szErrorString, QString("Flash擦除失败").toLocal8Bit().data());
        break;
    case 47:
        strcpy(szErrorString, QString("Flash写入失败").toLocal8Bit().data());
        break;
    case 50:
        strcpy(szErrorString, QString("电池故障").toLocal8Bit().data());
        break;
    case 51:
        strcpy(szErrorString, QString("门已打开").toLocal8Bit().data());
        break;
    case 52:
        strcpy(szErrorString, QString("微动开关故障").toLocal8Bit().data());
        break;
    case 53:
        strcpy(szErrorString, QString("RTC故障").toLocal8Bit().data());
        break;
    default:
        if (0x4000 & REALERRORCODE(lErrorCode))
        {
            switch (0x3FFF & REALERRORCODE(lErrorCode))
            {
            case 1:
                strcpy(szErrorString, QString("币筒空").toLocal8Bit().data());
                break;
            case 2:
                strcpy(szErrorString, QString("币筒阻塞").toLocal8Bit().data());
                break;
            case 3:
                strcpy(szErrorString, QString("非法币筒").toLocal8Bit().data());
                break;
            case 4:
                strcpy(szErrorString, QString("币筒故障").toLocal8Bit().data());
                break;
            case 101:
                strcpy(szErrorString, QString("接收器异常").toLocal8Bit().data());
                break;
            case 103:
                strcpy(szErrorString, QString("接收器故障").toLocal8Bit().data());
                break;
            case 251:
                strcpy(szErrorString, QString("币筒满").toLocal8Bit().data());
                break;
            case 252:
                strcpy(szErrorString, QString("币筒不存在").toLocal8Bit().data());
                break;
            case 254:
                strcpy(szErrorString, QString("暂存故障").toLocal8Bit().data());
                break;
            case 255:
                strcpy(szErrorString, QString("滚筒松动").toLocal8Bit().data());
                break;
            default:
                strcpy(szErrorString, QString("未定义的故障").toLocal8Bit().data());
            }
        }
        else
        {
            ucErr = REALERRORCODE(lErrorCode) & 0x0FF;
            ucExInfo = REALERRORCODE(lErrorCode) >> 8;
            switch (ucErr)
            {
            case 2:
                sprintf(szErrorString, QString("感应线圈故障. 线圈编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 8:
                strcpy(szErrorString, QString("分拣机出口传感器故障. 传感器编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 23:
                strcpy(szErrorString, QString("支付电机故障, 故障币筒编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 24:
                strcpy(szErrorString, QString("支付超时, 故障币筒编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 25:
                strcpy(szErrorString, QString("支付阻塞, 故障币筒编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 26:
                strcpy(szErrorString, QString("支付传感器故障, 故障币筒编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 27:
                strcpy(szErrorString, QString("料位传感器故障, 故障币筒编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 11:
                strcpy(szErrorString, QString("低电平传感器故障，故障币筒编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 12:
                strcpy(szErrorString, QString("高电平传感器故障，故障币筒编号=%1.").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 31:
                strcpy(szErrorString, QString("缺少从属设备，设备编号=%1").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 32:
                strcpy(szErrorString, QString("内部通讯不良，不良从属设备编号=%1").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 36:
                strcpy(szErrorString, QString("票据验证传感器故障，故障传感器编号=%1").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 48:
                if (2 == ucExInfo)
                {
                    strcpy(szErrorString, QString("硬币箱无响应").toLocal8Bit().data());
                }
                else
                {
                    sprintf(szErrorString, QString("从属设备无响应，设备编号=%1").arg(ucExInfo).toLocal8Bit().data());
                }
                break;

            case 49:
                strcpy(szErrorString, QString("光电传感器故障，传感器编号=%1").arg(ucExInfo).toLocal8Bit().data());
                break;
            case 51:
                strcpy(szErrorString, QString("门(%1)已打开").arg(ucExInfo).toLocal8Bit().data());
                break;

            default:
                strcpy(szErrorString, QString("未定义的故障").toLocal8Bit().data());
                break;
            }
        }
        break;
    }
    return DEV_SUCCESS;
}
long CDevCOR_BCR106::ConvertErrorCode(long lRet)
{
    switch (lRet)
    {
    case DEV_SUCCESS:
        return DEV_SUCCESS;
    case ERR_DEVPORT_PARAM:
        return SOFT_ERROR_PARAMS;
    case ERR_DEVPORT_NOTOPEN:
        return SOFT_ERROR_NOT_OPEN;
    case ERR_DEVPORT_CANCELED:
        return SOFT_USER_CANCEL;
    case ERR_DEVPORT_RTIMEOUT:
        return SOFT_USER_TIMEOUT;
    case ERR_DEVPORT_READERR:
        return ERR_DEVPORT_READERR;
    case ERR_DEVPORT_WRITE:
        return ERR_DEVPORT_READERR;
    case ERR_DEVPORT_WTIMEOUT:
        return ERR_DEVPORT_READERR;
    case ERR_DEVPORT_LIBRARY:
        return SOFT_ERROR_NOT_LOAD;
    case ERR_DEVPORT_NODEFINED:
        return SOFT_ERROR_OTHER;
    case 5:
        return ERR_DEVPORT_READERR;
    default:
        return SOFT_ERROR_OTHER;
    }
}

int CDevCOR_BCR106::SplitMultipleItems(LPCSTR pValue, CMultiString &ms, string separator)
{
    char szBuf[1024];
    strcpy(szBuf, pValue);
    char *p = strtok(szBuf, separator.c_str());
    while (p != NULL)
    {
        ms.Add(p);
        p = strtok(NULL, separator.c_str());
    }
    return ms.GetCount();
}

int CDevCOR_BCR106::MapHopperNo2Index(int iHopperNo)
{
    int nIndex = 0;
    for (int i = 0; i < MAX_COINCYLINDER_NUM; i++)
    {
        if (m_iHopperNo[i] == iHopperNo)
        {
            nIndex = i;
            break;
        }
    }
    return nIndex;
}
