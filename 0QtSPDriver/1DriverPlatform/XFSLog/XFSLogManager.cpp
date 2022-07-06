#include "XFSLogManager.h"
#include "INIFileReader.h"

CQtSimpleMutexEx g_cMutexEx("NDT_XFS_LOG_MANAGER_MUTEX");

//#define LOCKIT CQtSimpleMutexEx __auto_unlock_mutex("NDT_XFS_LOG_MANAGER_MUTEX")
#define LOCKIT \
    AutoMutex(g_cMutexEx)
//    CQtSimpleMutexEx mutexEx("NDT_XFS_LOG_MANAGER_MUTEX"); \
//    AutoMutex(mutexEx)
#define AUTO_LOCK_AND_START_LOG()                                                                                                                                        \
    LOCKIT;                                                                                                                                                              \
    CAutoStartEndLog __auto_start_end_log(this);
#ifdef QT_WIN32
#define INIPATH "C:/CFES/ETC/XFSLogManager.ini"
#else
#define INIPATH "/usr/local/CFES/ETC/XFSLogManager.ini"
#endif

//自动开始和结束日志的辅助类
class CAutoStartEndLog
{
public:
    CAutoStartEndLog(CXFSLogManager *pXFSLogManager)
    {
        m_pXFSLogManager = pXFSLogManager;
        m_pXFSLogManager->StartLog();
    }
    ~CAutoStartEndLog()
    {
        m_pXFSLogManager->EndLog();
    }

private:
    CXFSLogManager *m_pXFSLogManager;
};

CXFSLogManager::CXFSLogManager() : m_XFSBufferLogger(m_StringBuffer)
{
    CINIFileReader ReaderConfigFile;
    ReaderConfigFile.LoadINIFile(INIPATH);
    CINIReader cINI = ReaderConfigFile.GetReaderSection("default");
    m_bEnableLog    = (DWORD)cINI.GetValue("Enable", 1) != 0;
}

CXFSLogManager::~CXFSLogManager()
{
}

//-------------------------------- 接口函数 -----------------
//记录SPI 函数WFPCancelAsyncRequest调用开始时的相关信息
HRESULT CXFSLogManager::LOGCancelAsyncRequest(HSERVICE service, REQUESTID request_id, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("CancelAsyncRequest() returned %s", GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("RequestId: 0x%08.8X(%d)", request_id, request_id).EndLine();
    return 0;
}

//记录SPI 函数WFPClose调用开始时的相关信息
HRESULT CXFSLogManager::LOGClose(HSERVICE service, LPSTR object_name, REQUESTID req_id, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s returned %s", XFSCallMsgType2Desc(WFS_CLOSE_COMPLETE), GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectName: %s", object_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("ReqID: 0x%08.8X(%d)", req_id, req_id).EndLine();
    return 0;
}

//记录SPI 函数WFPDeregister调用开始时的相关信息。
HRESULT CXFSLogManager::LOGDeregister(HSERVICE service, DWORD event_class, LPSTR object_reg_name, LPSTR object_name, REQUESTID req_id, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s returned %s", XFSCallMsgType2Desc(WFS_DEREGISTER_COMPLETE), GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("EventClass: 0x%08.8X(%d)", event_class, event_class).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectName: %s", object_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectRegName: %s", object_reg_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("ReqID: 0x%08.8X(%d)", req_id, req_id).EndLine();
    return 0;
}

//记录SPI 函数WFPExecute调用开始时的相关信息。
HRESULT CXFSLogManager::LOGExecute(HSERVICE service, DWORD command, LPVOID cmd_data, DWORD timeout, LPSTR object_name, REQUESTID req_id, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s returned %s", XFSCallMsgType2Desc(WFS_EXECUTE_COMPLETE), GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("Command: %s(%d)", GetMsgDesc(CONST_TYPE_EXECUTE, command), command).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectName: %s", object_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("Timeout: 0x%08.8X(%d)", timeout, timeout).EndLine();
    m_StringBuffer.AddTab(1).AddF("ReqID: 0x%08.8X(%d)", req_id, req_id).EndLine();
    m_XFSBufferLogger.Log(MT_EX, command, "CmdData", (LPCTSTR)cmd_data);
    return 0;
}

//记录SPI 函数WFPGetInfo调用开始时的相关信息。
HRESULT CXFSLogManager::LOGGetInfo(HSERVICE service, DWORD category, LPVOID query_details, DWORD timeout, LPSTR object_name, REQUESTID req_id, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s returned %s", XFSCallMsgType2Desc(WFS_GETINFO_COMPLETE), GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("Category: %s(%d)", GetMsgDesc(CONST_TYPE_GET_INFO, category), category).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjecName: %s", object_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("Timeout: 0x%08.8X(%d)", timeout, timeout).EndLine();
    m_StringBuffer.AddTab(1).AddF("ReqID: 0x%08.8X(%d)", req_id, req_id).EndLine();
    m_XFSBufferLogger.Log(MT_GI, category, "QueryDetails", (LPCTSTR)query_details);
    return 0;
}

//记录SPI 函数WFPLock调用开始时的相关信息。
HRESULT CXFSLogManager::LOGLock(HSERVICE service, DWORD timeout, LPSTR object_name, REQUESTID req_id, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s returned %s", XFSCallMsgType2Desc(WFS_LOCK_COMPLETE), GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectName: %s", object_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("Timeout: 0x%08.8X(%d)", timeout, timeout).EndLine();
    m_StringBuffer.AddTab(1).AddF("ReqID: 0x%08.8X(%d)", req_id, req_id).EndLine();
    return 0;
}

//记录SPI 函数WFPOpen调用开始时的相关信息。
HRESULT CXFSLogManager::LOGOpen(HSERVICE service, LPSTR logical_name, HAPP app_handle, LPSTR app_id, DWORD trace_level, DWORD timeout, LPSTR object_name,
                                REQUESTID req_id, HPROVIDER provider, DWORD spi_versions_required, LPWFSVERSION spi_version, DWORD srvc_versions_required,
                                LPWFSVERSION srvc_version, long result)
{
    Q_UNUSED(spi_version);
    Q_UNUSED(srvc_versions_required);
    Q_UNUSED(srvc_version);
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s returned %s", XFSCallMsgType2Desc(WFS_OPEN_COMPLETE), GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("LogicalMame: %s", logical_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("AppHandle: 0x%08.8X(%d)", app_handle, app_handle).EndLine();
    m_StringBuffer.AddTab(1).AddF("AppId: %s", app_id).EndLine();
    m_StringBuffer.AddTab(1).AddF("TraceLevel: 0x%08.8X(%d)", trace_level, trace_level).EndLine();
    m_StringBuffer.AddTab(1).AddF("Timeout: 0x%08.8X(%d)", timeout, timeout).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectName: %s", object_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("Provider: 0x%08.8X(%d)", provider, provider).EndLine();
    m_StringBuffer.AddTab(1).AddF("SpiVersionsRequired: 0x%08.8X(%d)", spi_versions_required, spi_versions_required).EndLine();
    m_StringBuffer.AddTab(1).AddF("SrvcVersionsRequired: 0x%08.8X(%d)", srvc_versions_required, srvc_versions_required).EndLine();
    m_StringBuffer.AddTab(1).AddF("ReqID: 0x%08.8X(%d)", req_id, req_id).EndLine();

    return 0;
}

//记录SPI 函数WFPRegister调用开始时的相关信息。
HRESULT CXFSLogManager::LOGRegister(HSERVICE service, DWORD event_class, LPSTR object_reg_name, LPSTR object_name, REQUESTID req_id, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s returned %s", XFSCallMsgType2Desc(WFS_REGISTER_COMPLETE), GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("EventClass: 0x%08.8X(%d)", event_class, event_class).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectName: %s", object_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectRegName: %s", object_reg_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("ReqID: 0x%08.8X(%d)", req_id, req_id).EndLine();
    return 0;
}

//记录SPI 函数WFPSetTraceLevel调用开始时的相关信息。
HRESULT CXFSLogManager::LOGSetTraceLevel(HSERVICE service, DWORD trace_level, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("WFPSetTraceLevel() returned %s", GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("TraceLevel: 0x%08.8X(%d)", trace_level, trace_level).EndLine();
    return 0;
}

//记录SPI 函数WFPUnloadService调用开始时的相关信息。
HRESULT CXFSLogManager::LOGUnloadService(long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("WFPUnloadService() returned %s", GetErrorDesc(result)).EndLine();
    return 0;
}

//记录SPI 函数WFPUnlock调用开始时的相关信息。
HRESULT CXFSLogManager::LOGUnlock(HSERVICE service, LPSTR object_name, REQUESTID req_id, long result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s returned %s", XFSCallMsgType2Desc(WFS_UNLOCK_COMPLETE), GetErrorDesc(result)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", service, service).EndLine();
    m_StringBuffer.AddTab(1).AddF("ObjectName: %s", object_name).EndLine();
    m_StringBuffer.AddTab(1).AddF("ReqID: 0x%08.8X(%d)", req_id, req_id).EndLine();
    return 0;
}

//记录SP相关事件信息。
HRESULT CXFSLogManager::LOGXFSMessageInfor(LPCSTR object_name, UINT uiMsg, LPWFSRESULT result)
{
    if (!m_bEnableLog)
    {
        return 0;
    }
    AUTO_LOCK_AND_START_LOG();
    LogCurrentTime().AddF("%s %s with %s", XFSMsgType2Desc(uiMsg), MsgIsCompleteMsg(uiMsg) ? "Completed" : "Event", GetErrorDesc(result->hResult)).EndLine();
    m_StringBuffer.AddTab(1).AddF("Service: 0x%04.4X(%d)", result->hService, result->hService).EndLine();
    if (uiMsg == WFS_EXECUTE_COMPLETE)
    {
        m_StringBuffer.AddTab(1).AddF("Command: %s(%d)", GetMsgDesc(CONST_TYPE_EXECUTE, result->u.dwCommandCode), result->u.dwCommandCode).EndLine();
    }
    else if (uiMsg == WFS_GETINFO_COMPLETE)
    {
        m_StringBuffer.AddTab(1).AddF("Category: %s(%d)", GetMsgDesc(CONST_TYPE_GET_INFO, result->u.dwCommandCode), result->u.dwCommandCode).EndLine();
    }
    else if (uiMsg == WFS_EXECUTE_EVENT || uiMsg == WFS_SERVICE_EVENT || uiMsg == WFS_USER_EVENT || uiMsg == WFS_SYSTEM_EVENT)
    {
        m_StringBuffer.AddTab(1).AddF("EventID: %s(%d)", GetMsgDesc(CONST_TYPE_MESSAGE, result->u.dwEventID), result->u.dwEventID).EndLine();
    }
    m_StringBuffer.AddTab(1).AddF("ObjectName: %s", object_name).EndLine();
    SYSTEMTIME &st = result->tsTimestamp;
    m_StringBuffer.AddTab(1)
    .AddF("TimeStamp: %04.4d-%02.2d-%02.2d %02.2d:%02.2d:%02.2d.%03.3d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds)
    .EndLine();
    m_StringBuffer.AddTab(1).AddF("RequestId: 0x%08.8X(%d)", result->RequestID, result->RequestID).EndLine();

    if (uiMsg == WFS_EXECUTE_COMPLETE)
    {
        m_XFSBufferLogger.Log(MT_EC, result->u.dwCommandCode, "buffer", (LPCTSTR)result->lpBuffer);
    }
    else if (uiMsg == WFS_GETINFO_COMPLETE)
    {
        m_XFSBufferLogger.Log(MT_GC, result->u.dwCommandCode, "buffer", (LPCTSTR)result->lpBuffer);
    }
    else if (uiMsg == WFS_EXECUTE_EVENT || uiMsg == WFS_SERVICE_EVENT || uiMsg == WFS_USER_EVENT || uiMsg == WFS_SYSTEM_EVENT)
    {
        m_XFSBufferLogger.Log(MT_EVENT, result->u.dwCommandCode, "buffer", (LPCTSTR)result->lpBuffer);
    }
    else
    {
        m_StringBuffer.AddTab(1).AddF("buffer: 0x%08.8X(%d)", result->lpBuffer, result->lpBuffer).EndLine();
    }
    return 0;
}

//功能：获取系统当前时间（SYSTEMTIME结构体）
//参数：[out]CurTime：返回当前时间结构体
void CXFSLogManager::GetLocalTime(SYSTEMTIME &CurTime)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t tt             = time(NULL);
    tm     temp           = *localtime(&tt);
    CurTime.wYear         = 1900 + temp.tm_year;
    CurTime.wMonth        = 1 + temp.tm_mon;
    CurTime.wDayOfWeek    = temp.tm_wday;
    CurTime.wDay          = temp.tm_mday;
    CurTime.wHour         = temp.tm_hour;
    CurTime.wMinute       = temp.tm_min;
    CurTime.wSecond       = temp.tm_sec;
    CurTime.wMilliseconds = tv.tv_usec / 1000;
}

//------------------------ 内部函数 --------------------------
CStringBuffer &CXFSLogManager::LogCurrentTime()
{
    SYSTEMTIME SysTime;
    GetLocalTime(SysTime);

    m_StringBuffer.AddF("[%02.2d:%02.2d:%02.2d %03.3d] ", SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wMilliseconds);
    return m_StringBuffer;
}

const char *CXFSLogManager::GetErrorDesc(long lResult)
{
    const char *pDesc = m_XFSBufferLogger.ConstValue2Str(CONST_TYPE_ERROR_CODE, lResult);
    if (pDesc != NULL)
        return pDesc;
    static char szDesc[20];
    sprintf(szDesc, "%d", lResult);
    return szDesc;
}

const char *CXFSLogManager::GetMsgDesc(const char *pMsgType, DWORD dwID)
{
    const char *pDesc = m_XFSBufferLogger.ConstValue2Str(pMsgType, dwID);
    if (pDesc != NULL)
        return pDesc;
    static char szDesc[100];
    sprintf(szDesc, "0x%08.8X", dwID);
    return szDesc;
}

const char *CXFSLogManager::XFSCallMsgType2Desc(UINT uiMsg)
{
    return XFSMsgType2Desc(uiMsg);
}

const char *CXFSLogManager::XFSMsgType2Desc(UINT uiMsg)
{
    switch (uiMsg)
    {
    case WFS_OPEN_COMPLETE: return "WFPOpen()";
    case WFS_CLOSE_COMPLETE: return "WFPClose()";
    case WFS_LOCK_COMPLETE: return "WFPLock()";
    case WFS_UNLOCK_COMPLETE: return "WFPUnlock()";
    case WFS_REGISTER_COMPLETE: return "WFPRegister()";
    case WFS_DEREGISTER_COMPLETE: return "WFPDeregister()";
    case WFS_GETINFO_COMPLETE: return "WFPGetInfo()";
    case WFS_EXECUTE_COMPLETE: return "WFPExecute()";
    case WFS_EXECUTE_EVENT: return "WFS_EXECUTE_EVENT";
    case WFS_SERVICE_EVENT: return "WFS_SERVICE_EVENT";
    case WFS_USER_EVENT: return "WFS_USER_EVENT";
    case WFS_SYSTEM_EVENT: return "WFS_SYSTEM_EVENT";
    default:
        {
            static char szBuf[20];
            sprintf(szBuf, "%u", uiMsg);
            return szBuf;
        }
    }
}

BOOL CXFSLogManager::MsgIsCompleteMsg(UINT uiMsg)
{
    switch (uiMsg)
    {
    case WFS_OPEN_COMPLETE:
    case WFS_CLOSE_COMPLETE:
    case WFS_LOCK_COMPLETE:
    case WFS_UNLOCK_COMPLETE:
    case WFS_REGISTER_COMPLETE:
    case WFS_DEREGISTER_COMPLETE:
    case WFS_GETINFO_COMPLETE:
    case WFS_EXECUTE_COMPLETE: return TRUE;
    default: return FALSE;
    }
}
