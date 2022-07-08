#include "VHUSBDrive.h"

#ifdef QT_LINUX
#define STR_DLLNAME "/hots/lib/libVHUSB.so"
#else
#define STR_DLLNAME "libVHUSB.dll"
#endif

static const char *ThisFile = "CVHUSBDrive";

CVHUSBDrive::CVHUSBDrive(const char *lpDevName) : m_Log(lpDevName)
{
    memset(m_szDesc, 0x00, sizeof(m_szDesc));
    strcpy(m_szDevName, lpDevName);
    SetLogFile(LOGFILE, ThisFile, "VHUSB");
}

CVHUSBDrive::~CVHUSBDrive()
{
    VHUSBDllRelease();
}

void CVHUSBDrive::VHUSBDllRelease()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    CloseAllConnection();

    if (m_hdll.isLoaded())
    {
        m_hdll.unload();
    }
    m_pInfATMUSB = nullptr;
    m_pFnATMUSB  = nullptr;
    return ;
}

bool CVHUSBDrive::OpenVHUSBConnect(CONNECT_TYPE eConnectType)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STCONNECTPARA Connect = FindConnection(eConnectType);
    if (Connect.econnect != CONNECT_TYPE_UNKNOWN)
        return  true;

    if (!IsVHUSBLoaded())
    {
        if (!VHUSBDllLoad())
            return false;
    }

    STR_DRV strDrv;
    memset(&strDrv, 0, sizeof(STR_DRV));
    STCONNECTPARA stConnPara;
    stConnPara.econnect = eConnectType;
    // 打开连接参数
    strDrv.usParam = USB_PRM_OPEN;
    switch (eConnectType)
    {
    case CONNECT_TYPE_ZERO:
        strDrv.pvDataInBuffPtr = (PVOID)"ZERO";
        strDrv.uiDataInBuffSz = 4;
        break;
    case CONNECT_TYPE_UR:
        strDrv.pvDataInBuffPtr = (PVOID)"UR2";
        strDrv.uiDataInBuffSz = 3;
        break;
    default:
        break;
    }
    strDrv.pvDataOutBuffPtr = &(stConnPara.uUSBHandle);
    strDrv.uiDataOutReqSz = 4;
    strDrv.uiTimer = 5;
    strDrv.pvReserve = nullptr;
    strDrv.pvCallBackPtr = nullptr;
    strDrv.uiWndMsg = 0;
    m_Log.LogAction("OPEN");
    m_Log.NewLine().Log(static_cast<char *>(strDrv.pvDataInBuffPtr));

    long lRet = VHUSBDrvCall(USB_DRV_INF_OPEN, &strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "打开 \"%s\" 的USB连接失败", m_szDevName);
        m_Log.NewLine().Log("OPEN FAILED").EndLine();
        return false;
    }

    if (stConnPara.uUSBHandle == 0)
    {
        m_Log.NewLine().Log("OPEN FAILED").EndLine();
        return false;
    }
    Log(ThisModule, 1, "打开 %s 连接成功, DrvHandle:%d", \
        eConnectType == CONNECT_TYPE_ZERO ? "BV" : "UR", stConnPara.uUSBHandle);
    m_Log.NewLine().Log("OPEN SUCCEED").EndLine();
    m_lConnect.push_back(stConnPara);
    return true;

}

STCONNECTPARA CVHUSBDrive::FindConnection(CONNECT_TYPE eConnectType)
{
    STCONNECTPARA pconnpara;
    if (m_lConnect.size() <= 0)
        return pconnpara;

    list<STCONNECTPARA>::iterator it = m_lConnect.begin();

    for (; it != m_lConnect.end(); it++)
    {
        if ((*it).econnect == eConnectType)
        {
            pconnpara = *it;
            break;
        }
    }
    return pconnpara;
}

bool CVHUSBDrive::CloseVHUSBConnect(CONNECT_TYPE eConnectType)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    STCONNECTPARA connect = FindConnection(eConnectType);
    if (connect.econnect != CONNECT_TYPE_UNKNOWN)
    {
        return CloseConnection(connect) >= 0;
    }

    return  true;
}


void CVHUSBDrive::CloseAllConnection()
{
    if (!m_hdll.isLoaded())
    {
        return;
    }

    for (list<STCONNECTPARA>::iterator it = m_lConnect.begin();
         it != m_lConnect.end(); it++)
    {
        CloseConnection(*it);
    }
    m_lConnect.clear();
}

long CVHUSBDrive::CloseConnection(STCONNECTPARA &stConnPara)
{
    THISMODULE(__FUNCTION__);
    if (stConnPara.uUSBHandle == 0)
    {
        Log(ThisModule, __LINE__, "已关闭USB连接");
        return 0;
    }

    list<STCONNECTPARA>::iterator it = m_lConnect.begin();

    for ( ; it != m_lConnect.end(); it++)
    {
        if ((*it).econnect == stConnPara.econnect)
        {
            m_lConnect.erase(it);
            break;
        }
    }

    STR_DRV strDrv;
    memset(&strDrv, NULL, sizeof(STR_DRV));
    strDrv.pvCallBackPtr = nullptr;
    strDrv.pvDataInBuffPtr = nullptr;
    strDrv.pvDataOutBuffPtr = nullptr;
    strDrv.pvReserve = nullptr;
    // 关闭连接参数
    strDrv.usParam  = USB_PRM_CLOSE;
    strDrv.uiDrvHnd = stConnPara.uUSBHandle;// 对应的句柄

    string strConn("");
    switch (stConnPara.econnect)
    {
    case CONNECT_TYPE_ZERO:
        strConn = "ZERO";
        break;
    case CONNECT_TYPE_UR:
        strConn = "UR2";
        break;
    default:
        break;
    }

    m_Log.LogAction("CLOSE");
    m_Log.NewLine().Log(strConn.c_str());

    long lRet = VHUSBDrvCall(USB_DRV_INF_CLOSE, &strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "关闭 \"%s\" 的USB连接(Hwnd:%d)失败", m_szDevName, stConnPara.uUSBHandle);
        m_Log.NewLine().Log("CLOSE FAILED").EndLine();
        return lRet;
    }
    Log(ThisModule, 1, "关闭 \"%s\" 的USB连接(Hwnd:%d)成功", m_szDevName, stConnPara.uUSBHandle);
    m_Log.NewLine().Log("CLOSE SUCCEED").EndLine();
    return 0;
}

bool CVHUSBDrive::VHUSBDllLoad()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (!QLibrary::isLibrary(STR_DLLNAME))
    {
        QString qstrerr = m_hdll.errorString();
        Log(ThisModule, __LINE__, "VHUSB.dll is not Lib：GetLastError=%s", qstrerr.toStdString().c_str());
        return false;
    }


    m_hdll.setFileName(STR_DLLNAME);
    if (!m_hdll.isLoaded())
    {
        if (!m_hdll.load())
        {
            QString qstrerr = m_hdll.errorString();
            Log(ThisModule, __LINE__, "VHUSB.dll加载失败：GetLastError=%s", qstrerr.toStdString().c_str());
            return false;
        }
    }

    if (nullptr == m_pFnATMUSB || nullptr == m_pInfATMUSB)
    {
        m_pFnATMUSB = (HT_FnATMUSB)m_hdll.resolve("FnATMUSB");
        m_pInfATMUSB = (HT_InfATMUSB)m_hdll.resolve("InfATMUSB");

        if (nullptr == m_pFnATMUSB || nullptr == m_pInfATMUSB)
        {
            VHUSBDllRelease();
            Log(ThisModule, __LINE__, "接口加载失败");
            return false;
        }
    }

    return true;
}

long CVHUSBDrive::VHUSBDrvCall(ULONG wCmd, PSTR_DRV pParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutexStl(m_MutexAction);
    if (m_pFnATMUSB == nullptr || m_pInfATMUSB == nullptr)
    {
        Log(ThisModule, __LINE__, "参数指针为NULL，或没有加载USB驱动库");
        return ERR_UR_USB_CONN_ERR;
    }

    if (pParam->uiDrvHnd == 0 && wCmd != USB_DRV_INF_OPEN)
    {
        Log(ThisModule, __LINE__, "没有打开USB连接");
        return ERR_UR_USB_CONN_ERR;
    }

    HT_FnATMUSB pFunc;
    switch (wCmd)
    {
    case USB_DRV_FN_DATASEND:
    case USB_DRV_FN_DATARCV:
    case USB_DRV_FN_DATASENDRCV:
        //case USB_DRV_FN_USBRESET:
        pFunc = m_pFnATMUSB;
        break;
    case USB_DRV_INF_OPEN:
    case USB_DRV_INF_CLOSE:
    case USB_DRV_INF_INFGET:
        //case USB_DRV_INF_SENS:
        pFunc = m_pInfATMUSB;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持命令参数: wCmd = %d", wCmd);
        return ERR_UR_USB_PARAM_ERR;
    }

    ULONG uRet = pFunc(wCmd, pParam);
    if (uRet != ERR_DRV_USB_SUCCESS)
    {
        Log(ThisModule, __LINE__, "调用命令返回失败：[wCmd=%d,sParam=%d, DrvHand:%d]lRet=0x%02X[ %s ]", \
            wCmd, pParam->usParam, pParam->uiDrvHnd, uRet, GetErrDesc(uRet));
        //return ERR_UR_USB_CMD_ERR;
        return USBDrvError2DriverError(uRet);
    }
    return 0;
}

LPCSTR CVHUSBDrive::GetErrDesc(ULONG ulErrCode)
{
    //THISMODULE(__FUNCTION__);
    memset(m_szDesc, 0x00, sizeof(m_szDesc));
    switch (ulErrCode)
    {
    case ERR_DRV_USB_CANCEL_END:                strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_END"); break;
    case ERR_DRV_USB_CANCEL_NOPST:              strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_NOPST"); break;
    case ERR_DRV_USB_CANCEL_CROSS_END:          strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_CROSS_END"); break;
    case ERR_DRV_USB_CANCEL_NOTHING:            strcpy(m_szDesc, "ERR_DRV_USB_CANCEL_NOTHING"); break;
    case ERR_DRV_USB_FUNC:                      strcpy(m_szDesc, "ERR_DRV_USB_FUNC"); break;
    case ERR_DRV_USB_PRM:                       strcpy(m_szDesc, "ERR_DRV_USB_PRM"); break;
    case ERR_DRV_USB_DRVHND_DIFFER:             strcpy(m_szDesc, "ERR_DRV_USB_DRVHND_DIFFER"); break;
    case ERR_DRV_USB_DRV_REMOVE:                strcpy(m_szDesc, "ERR_DRV_USB_DRV_REMOVE"); break;
    case ERR_DRV_USB_BLD:                       strcpy(m_szDesc, "ERR_DRV_USB_BLD"); break;
    case ERR_DRV_USB_INDATA:                    strcpy(m_szDesc, "ERR_DRV_USB_INDATA"); break;
    case ERR_DRV_USB_OUTDATA:                   strcpy(m_szDesc, "ERR_DRV_USB_OUTDATA"); break;
    case ERR_DRV_USB_INOUTDATA:                 strcpy(m_szDesc, "ERR_DRV_USB_INOUTDATA"); break;
    case ERR_DRV_USB_ENTRY_DEVICE_OVER:         strcpy(m_szDesc, "ERR_DRV_USB_ENTRY_DEVICE_OVER"); break;
    case ERR_DRV_USB_ENTRY_THREAD_OVER:         strcpy(m_szDesc, "ERR_DRV_USB_ENTRY_THREAD_OVER"); break;
    case ERR_DRV_USB_BCC:                       strcpy(m_szDesc, "ERR_DRV_USB_BCC"); break;
    case ERR_DRV_USB_INDATA_BUFFSZ:             strcpy(m_szDesc, "ERR_DRV_USB_INDATA_BUFFSZ"); break;
    case ERR_DRV_USB_OUTDATA_BUFFSZ:            strcpy(m_szDesc, "ERR_DRV_USB_OUTDATA_BUFFSZ"); break;
    case ERR_DRV_USB_INOUTDATA_BUFFSZ:          strcpy(m_szDesc, "ERR_DRV_USB_INOUTDATA_BUFFSZ"); break;
    case ERR_DRV_USB_LINE_TIMEOUT:              strcpy(m_szDesc, "ERR_DRV_USB_LINE_TIMEOUT"); break;
    case ERR_DRV_USB_COMMAND_TIMEOUT:           strcpy(m_szDesc, "ERR_DRV_USB_COMMAND_TIMEOUT"); break;
    case ERR_DRV_USB_CLOSE:                     strcpy(m_szDesc, "ERR_DRV_USB_CLOSE"); break;
    case ERR_DRV_USB_OPEN_BUSY:                 strcpy(m_szDesc, "ERR_DRV_USB_OPEN_BUSY"); break;
    case ERR_DRV_USB_SEND_BUSY:                 strcpy(m_szDesc, "ERR_DRV_USB_SEND_BUSY"); break;
    case ERR_DRV_USB_RCV_BUSY:                  strcpy(m_szDesc, "ERR_DRV_USB_RCV_BUSY"); break;
    case ERR_DRV_USB_EP_DOWN:                   strcpy(m_szDesc, "ERR_DRV_USB_EP_DOWN"); break;
    case ERR_DRV_USB_MEMORY:                    strcpy(m_szDesc, "ERR_DRV_USB_MEMORY"); break;
    case ERR_DRV_USB_HANDLE:                    strcpy(m_szDesc, "ERR_DRV_USB_HANDLE"); break;
    case ERR_DRV_USB_REG:                       strcpy(m_szDesc, "ERR_DRV_USB_REG"); break;
    case ERR_DRV_USB_DRVCALL:                   strcpy(m_szDesc, "ERR_DRV_USB_DRVCALL"); break;
    case ERR_DRV_USB_THREAD:                    strcpy(m_szDesc, "ERR_DRV_USB_THREAD"); break;
    case ERR_DRV_USB_POSTMSG:                   strcpy(m_szDesc, "ERR_DRV_USB_POSTMSG"); break;
    case ERR_DRV_USB_TRACE:                     strcpy(m_szDesc, "ERR_DRV_USB_TRACE"); break;
    case ERR_DRV_USB_ALREADY_COMPLETE:          strcpy(m_szDesc, "ERR_DRV_USB_ALREADY_COMPLETE"); break;
    case ERR_DRV_USB_SUM:                       strcpy(m_szDesc, "ERR_DRV_USB_SUM"); break;
    default:                                    sprintf(m_szDesc, "0x%08.08X", ulErrCode); break;
    }
    return m_szDesc;
}


long CVHUSBDrive::USBDrvError2DriverError(ULONG ulError)
{
    THISMODULE(__FUNCTION__);
    switch (ulError)
    {
    case ERR_DRV_USB_CANCEL_END:                  return ERR_UR_DRV_CANCEL_END;
    case ERR_DRV_USB_CANCEL_NOPST:                return ERR_UR_DRV_CANCEL_NOPST;
    case ERR_DRV_USB_CANCEL_CROSS_END:            return ERR_UR_DRV_CANCEL_CROSS_END;
    case ERR_DRV_USB_CANCEL_NOTHING:              return ERR_UR_DRV_CANCEL_NOTHING;
    case ERR_DRV_USB_FUNC:                        return ERR_UR_DRV_FUNC;
    case ERR_DRV_USB_PRM:                         return ERR_UR_DRV_PRM;
    case ERR_DRV_USB_DRVHND_DIFFER:               return ERR_UR_DRV_DRVHND_DIFFER;
    case ERR_DRV_USB_DRV_REMOVE:                  return ERR_UR_DRV_DRV_REMOVE;
    case ERR_DRV_USB_BLD:                         return ERR_UR_DRV_BLD;
    case ERR_DRV_USB_INDATA:                      return ERR_UR_DRV_INDATA;
    case ERR_DRV_USB_OUTDATA:                     return ERR_UR_DRV_OUTDATA;
    case ERR_DRV_USB_INOUTDATA:                   return ERR_UR_DRV_INOUTDATA;
    case ERR_DRV_USB_ENTRY_DEVICE_OVER:           return ERR_UR_DRV_ENTRY_DEVICE_OVER;
    case ERR_DRV_USB_ENTRY_THREAD_OVER:           return ERR_UR_DRV_ENTRY_THREAD_OVER;
    case ERR_DRV_USB_BCC:                         return ERR_UR_DRV_BCC;
    case ERR_DRV_USB_INDATA_BUFFSZ:               return ERR_UR_DRV_INDATA_BUFFSZ;
    case ERR_DRV_USB_OUTDATA_BUFFSZ:              return ERR_UR_DRV_OUTDATA_BUFFSZ;
    case ERR_DRV_USB_INOUTDATA_BUFFSZ:            return ERR_UR_DRV_INOUTDATA_BUFFSZ;
    case ERR_DRV_USB_LINE_TIMEOUT:                return ERR_UR_DRV_LINE_TIMEOUT;
    case ERR_DRV_USB_COMMAND_TIMEOUT:             return ERR_UR_DRV_COMMAND_TIMEOUT;
    case ERR_DRV_USB_CLOSE:                       return ERR_UR_DRV_CLOSE;
    case ERR_DRV_USB_OPEN_BUSY:                   return ERR_UR_DRV_OPEN_BUSY;
    case ERR_DRV_USB_SEND_BUSY:                   return ERR_UR_DRV_SEND_BUSY;
    case ERR_DRV_USB_RCV_BUSY:                    return ERR_UR_DRV_RCV_BUSY;
    case ERR_DRV_USB_EP_DOWN:                     return ERR_UR_DRV_EP_DOWN;
    case ERR_DRV_USB_MEMORY:                      return ERR_UR_DRV_MEMORY;
    case ERR_DRV_USB_HANDLE:                      return ERR_UR_DRV_HANDLE;
    case ERR_DRV_USB_REG:                         return ERR_UR_DRV_REG;
    case ERR_DRV_USB_DRVCALL:                     return ERR_UR_DRV_DRVCALL;
    case ERR_DRV_USB_THREAD:                      return ERR_UR_DRV_THREAD;
    case ERR_DRV_USB_POSTMSG:                     return ERR_UR_DRV_POSTMSG;
    case ERR_DRV_USB_TRACE:                       return ERR_UR_DRV_TRACE;
    case ERR_DRV_USB_ALREADY_COMPLETE:            return ERR_UR_DRV_ALREADYCOMPLETE;
    case ERR_DRV_USB_SUM:                         return ERR_UR_DRV_USB_SUM;
    default:
        Log(ThisModule, -1, "USB Driver error code(%d[0x%08.08X]) not defined", ulError, ulError);
        return ERR_UR_DRV_OTHER;
    }
}

long CVHUSBDrive::SendAndRecvData(
CONNECT_TYPE    eConnect,
const char *pszSendData,
DWORD dwSendLen,
DWORD dwSendTimeout,
char  *pszRecvData,
DWORD &dwRecvedLenInOut,
DWORD dwRecvTimeout,
const char *pActionName,
BOOL bPDL)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    STCONNECTPARA Connect = FindConnection(eConnect);
    if (Connect.econnect == CONNECT_TYPE_UNKNOWN)
    {
        Log(ThisModule, ERR_UR_USB_CONN_ERR, "Find connect %d failed", eConnect);
        return  ERR_UR_USB_CONN_ERR;
    }

    if (Connect.uUSBHandle == 0)
    {
        Log(ThisModule, ERR_UR_USB_CONN_ERR, "Connect.uUSBHandle:%d", Connect.uUSBHandle);
        return  ERR_UR_USB_CONN_ERR;
    }

    BOOL bIgnoreLog = FALSE;
    if(bPDL|| (eConnect == CONNECT_TYPE_ZERO))
        bIgnoreLog = TRUE;
    //LOGACTION(pActionName);	//记录设备日志
    m_Log.LogAction(pActionName);

    //记录发送的数据
    char temp[30] = {0};
    sprintf(temp, "SEND[%3d]:", dwSendLen);
    if (bIgnoreLog)
        m_Log.NewLine().Log(temp).EndLine();
    else
        m_Log.NewLine().Log(temp).LogHex(pszSendData, dwSendLen).EndLine();


    //发送数据
    ULONG uifncNo;
    STR_DRV drv;
    memset(&drv, 0, sizeof(STR_DRV));
    drv.uiDrvHnd = Connect.uUSBHandle;
    drv.usParam = bPDL ? USB_PRM_PDL : USB_PRM_USUALLY;
    drv.pvDataInBuffPtr = (void *)pszSendData;
    drv.uiDataInBuffSz = dwSendLen;
    drv.uiTimer = dwSendTimeout;
    drv.pvReserve = nullptr;
    drv.pvDataOutBuffPtr = nullptr;
    drv.pvCallBackPtr = nullptr;
    if (bPDL || (Connect.econnect == CONNECT_TYPE_ZERO)) //PDL必须使用USB_DRV_FN_DATASENDRCV 和zero-bv通讯须使用USB_DRV_FN_DATASENDRCV
    {
        drv.pvDataOutBuffPtr = pszRecvData;
        memset(drv.pvDataOutBuffPtr, 0, dwRecvedLenInOut);
        drv.uiDataOutReqSz = dwRecvedLenInOut;
        drv.uiTimer += dwRecvTimeout;
        uifncNo = USB_DRV_FN_DATASENDRCV;
    }
    else
    {
        uifncNo = USB_DRV_FN_DATASEND;
    }

    long lRet = VHUSBDrvCall(uifncNo, &drv);
    if (ERR_DRV_USB_SUCCESS != lRet)
    {
        Log(ThisModule, -1, "%s: 数据发送失败ulRet=%08X:%s", pActionName, lRet, GetErrDesc(static_cast<ULONG>(lRet)));
        m_Log.NewLine().Log("SEND FAILED").EndLine();
        //return USBDrvError2DriverError(static_cast<ULONG>(lRet));
        return lRet;
    }

    //接收数据 、 对于设置校验级别，降低废钞率命令，只需要发送成功即可
    if (bPDL || (Connect.econnect == CONNECT_TYPE_ZERO)) //PDL情况下不再接收数据
    {

        if (strcmp(pActionName, "GetNoteSeiralInfo") != 0)
        {
            dwRecvedLenInOut = MAKEWORD(pszRecvData[1], pszRecvData[0]);
        }

        char temp[30];
        sprintf(temp, "RECV[%3d]:", dwRecvedLenInOut);
        if(bIgnoreLog)
        {
            m_Log.NewLine().Log(temp).EndLine();
        }
        else
        {
            m_Log.NewLine().Log(temp).LogHex(pszRecvData, dwRecvedLenInOut).EndLine();
        }
        return ERR_DRV_USB_SUCCESS;
    }
    return RecvData(Connect.uUSBHandle, pszRecvData, dwRecvedLenInOut, dwRecvTimeout, pActionName, bPDL, !bIgnoreLog);
}


long CVHUSBDrive::RecvData(DWORD uUSBHandle, char *pszRecvData, DWORD &dwRecvedLenInOut, DWORD dwRecvTimeout,
                           const char *pActionName, bool bPDL, bool bWriteLog)
{
    THISMODULE(__FUNCTION__);

    STR_DRV drv;
    memset(&drv, 0, sizeof(STR_DRV));
    drv.uiDrvHnd = uUSBHandle;
    drv.usParam = bPDL ? USB_PRM_PDL : USB_PRM_USUALLY;
    drv.pvDataOutBuffPtr = pszRecvData;
    memset(drv.pvDataOutBuffPtr, 0, dwRecvedLenInOut);
    drv.uiDataOutReqSz = dwRecvedLenInOut;
    drv.uiTimer = dwRecvTimeout;
    long lRet = VHUSBDrvCall(USB_DRV_FN_DATARCV, &drv);

    if (ERR_DRV_USB_SUCCESS != lRet)
    {
        if (bWriteLog)
        {
            Log(ThisModule, -1, "%s:接收数据失败ulRet=%08X:%s",  pActionName, lRet, GetErrDesc(lRet));
            m_Log.NewLine().Log("RECV FAILED").EndLine();
        }
        //return USBDrvError2DriverError(static_cast<ULONG>(lRet));
        return lRet;
    }
    dwRecvedLenInOut = drv.uiDataOutBuffSz;
    //记录接收到的数据

    char temp[30];
    sprintf(temp, "RECV[%3d]:", dwRecvedLenInOut);

    if(bWriteLog)
    {
        m_Log.NewLine().Log(temp).LogHex(pszRecvData, dwRecvedLenInOut).EndLine();
    }
    else
    {
        m_Log.NewLine().Log(temp).EndLine();
    }
    return ERR_DRV_USB_SUCCESS;
}
