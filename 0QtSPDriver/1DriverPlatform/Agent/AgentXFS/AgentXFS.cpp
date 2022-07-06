#include "AgentXFS.h"
#include "XFSIDC.H"
#include "XFSLogHelper.h"
//#include "WhiteListCheck.h"
#include "unistd.h"

static const char *ThisFile = "AgentXFS.cpp";
//////////////////////////////////////////////////////////////////////////
//30-00-00-00(FT#0020) static CAgentXFS *gpAgent = nullptr;
static std::mutex gcMutex;
static std::map<HSERVICE, CAgentXFS *> gmapAgent;
//////////////////////////////////////////////////////////////////////////
static void AddAgent(HSERVICE hService, CAgentXFS *pAgent)
{
    std::unique_lock<std::mutex> _auto(gcMutex);
    gmapAgent[hService] = pAgent;
    return;
}
static void DelAgent(HSERVICE hService)
{
    std::unique_lock<std::mutex> _auto(gcMutex);
    auto it = gmapAgent.find(hService);
    if (it != gmapAgent.end())
    {
        delete it->second;
        it->second = nullptr;
        gmapAgent.erase(it);
    }
    return;
}
static CAgentXFS *GetAgent(HSERVICE hService)
{
    std::unique_lock<std::mutex> _auto(gcMutex);
    auto it = gmapAgent.find(hService);
    if (it != gmapAgent.end())
        return it->second;
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#if defined(AGENTXFS_LIBRARY)
#define AGENTXFS_EXPORT Q_DECL_EXPORT
#else
#define AGENTXFS_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
extern "C" {
#ifdef QT_WIN_LINUX_XFS
#ifdef QT_WIN32
#define WINAPI              __stdcall
#else
#define WINAPI              __attribute__((__cdecl__))
#endif
    //////////////////////////////////////////////////////////////////////////
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPOpen(HSERVICE hService, LPSTR lpszLogicalName, HAPP hApp, LPSTR lpszAppID, DWORD dwTraceLevel, DWORD dwTimeOut, HWND hWnd, REQUESTID ReqID, HPROVIDER hProvider, DWORD dwSPIVersionsRequired, LPWFSVERSION lpSPIVersion, DWORD dwSrvcVersionsRequired, LPWFSVERSION lpSrvcVersion)
    {
        THISMODULE(__FUNCTION__);
        if (lpszLogicalName == nullptr)
            return WFS_ERR_INVALID_POINTER;
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
        {
            QDateTime qT = QDateTime::currentDateTime();
            QString strAgentName = QString(lpszLogicalName) + "_" + qT.toString("yyyy-MM-dd hh:mm:ss.zzz");
            gpAgent = new CAgentXFS(lpszLogicalName, strAgentName.toLocal8Bit().constData());
            if (gpAgent == nullptr)
                return WFS_ERR_INTERNAL_ERROR;
            AddAgent(hService, gpAgent);
        }
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
        return gpAgent->WFPOpen(hService, lpszLogicalName, hApp, lpszAppID, dwTraceLevel, dwTimeOut, ReqID, hProvider, dwSPIVersionsRequired, lpSPIVersion, dwSrvcVersionsRequired, lpSrvcVersion);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPClose(HSERVICE hService, HWND hWnd, REQUESTID ReqID)
    {
        THISMODULE(__FUNCTION__);
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
        return gpAgent->WFPClose(hService, ReqID);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPRegister(HSERVICE hService, DWORD dwEventClass, HWND hWndReg, HWND hWnd, REQUESTID ReqID)
    {
        THISMODULE(__FUNCTION__);
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        if (gpAgent->SetHWND(ThisModule, hWndReg, true))
            return WFS_ERR_INVALID_HWND;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
        return gpAgent->WFPRegister(hService, dwEventClass, ReqID);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPDeregister(HSERVICE hService, DWORD dwEventClass, HWND hWndReg, HWND hWnd, REQUESTID ReqID)
    {
        THISMODULE(__FUNCTION__);
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        if (gpAgent->SetHWND(ThisModule, hWndReg, true))
            return WFS_ERR_INVALID_HWND;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
        return gpAgent->WFPDeregister(hService, dwEventClass, ReqID);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPLock(HSERVICE hService, DWORD dwTimeOut, HWND hWnd, REQUESTID ReqID)
    {
        THISMODULE(__FUNCTION__);
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
        return gpAgent->WFPLock(hService, dwTimeOut, ReqID);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPUnlock(HSERVICE hService, HWND hWnd, REQUESTID ReqID)
    {
        THISMODULE(__FUNCTION__);
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
        return gpAgent->WFPUnlock(hService, ReqID);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPGetInfo(HSERVICE hService, DWORD dwCategory, LPVOID lpQueryDetails, DWORD dwTimeOut, HWND hWnd, REQUESTID ReqID)
    {
        THISMODULE(__FUNCTION__);
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
        return gpAgent->WFPGetInfo(hService, dwCategory, lpQueryDetails, dwTimeOut, ReqID);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPExecute(HSERVICE hService, DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, HWND hWnd, REQUESTID ReqID)
    {
        THISMODULE(__FUNCTION__);
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
        return gpAgent->WFPExecute(hService, dwCommand, lpCmdData, dwTimeOut, ReqID);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPCancelAsyncRequest(HSERVICE hService, REQUESTID RequestID)
    {
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        return gpAgent->WFPCancelAsyncRequest(hService, RequestID);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPSetTraceLevel(HSERVICE hService, DWORD dwTraceLevel)
    {
        gpAgent = GetAgent(hService);
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        return gpAgent->WFPSetTraceLevel(hService, dwTraceLevel);
    }
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPUnloadService()
    {
        return WFS_SUCCESS;
    }

    //////////////////////////////////////////////////////////////////////////
#else
#undef AGENTXFS_EXPORT
#define AGENTXFS_EXPORT     __attribute__((visibility("default")))
#define LFSAPI              __attribute__((__cdecl__))
//////////////////////////////////////////////////////////////////////////
#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPOpen(HSERVICE service, LPSTR logical_name, HAPP app_handle, LPSTR app_id, DWORD trace_level, DWORD timeout, LPSTR object_name, REQUESTID req_id, HPROVIDER provider, DWORD spi_versions_required, LPWFSVERSION spi_version, DWORD srvc_versions_required, LPWFSVERSION srvc_version)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPOpen(HSERVICE service, LPSTR logical_name, HAPP app_handle, LPSTR app_id, DWORD trace_level, DWORD timeout, HWND hWnd, REQUESTID req_id, HPROVIDER provider, DWORD spi_versions_required, LPWFSVERSION spi_version, DWORD srvc_versions_required, LPWFSVERSION srvc_version)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPOpen(HSERVICE service, LPSTR logical_name, HAPP app_handle, LPSTR app_id, DWORD trace_level, DWORD timeout, LPSTR object_name, REQUESTID req_id, HPROVIDER provider, DWORD spi_versions_required, LPLFSVERSION spi_version, DWORD srvc_versions_required, LPLFSVERSION srvc_version)
#endif
    {
        THISMODULE(__FUNCTION__);
        if (logical_name == nullptr)
            return WFS_ERR_INVALID_POINTER;
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
        {
            QDateTime qT = QDateTime::currentDateTime();
            QString strAgentName = QString(logical_name) + "_" + qT.toString("yyyy-MM-dd hh:mm:ss.zzz");
            gpAgent = new CAgentXFS(logical_name, strAgentName.toLocal8Bit().constData());
            if (gpAgent == nullptr)
                return WFS_ERR_INTERNAL_ERROR;
            AddAgent(service, gpAgent);
        }
#ifdef QT_LINUX_MANAGER_ZJ
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
#else

        if (gpAgent->SetObjectName(ThisModule, object_name))
            return WFS_ERR_INVALID_HWND;
#endif
#ifdef QT_LINUX_MANAGER_PISA
       return gpAgent->WFPOpen(service, logical_name, app_handle, app_id, trace_level, timeout, req_id, provider, spi_versions_required, (LPWFSVERSION)spi_version, srvc_versions_required, (LPWFSVERSION)srvc_version);
#else
       return gpAgent->WFPOpen(service, logical_name, app_handle, app_id, trace_level, timeout, req_id, provider, spi_versions_required, (LPWFSVERSION)spi_version, srvc_versions_required, (LPWFSVERSION)srvc_version);
#endif
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPClose(HSERVICE service, LPSTR object_name, REQUESTID req_id)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPClose(HSERVICE service, HWND hWnd, REQUESTID req_id)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPClose(HSERVICE service, LPSTR object_name, REQUESTID req_id)
#endif
    {
        THISMODULE(__FUNCTION__);
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
#ifdef QT_LINUX_MANAGER_ZJ
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
#else

        if (gpAgent->SetObjectName(ThisModule, object_name))
            return WFS_ERR_INVALID_HWND;
#endif
        return gpAgent->WFPClose(service, req_id);
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPRegister(HSERVICE service, DWORD event_class, LPSTR object_reg_name, LPSTR object_name, REQUESTID req_id)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPRegister ( HSERVICE service, DWORD event_class, HWND hWndReg, HWND hWnd, REQUESTID req_id)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPRegister(HSERVICE service, DWORD event_class, LPSTR object_reg_name, LPSTR object_name, REQUESTID req_id)
#endif
    {
        THISMODULE(__FUNCTION__);
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
#ifdef QT_LINUX_MANAGER_ZJ
        if (gpAgent->SetHWND(ThisModule, hWndReg, true))
            return WFS_ERR_INVALID_HWND;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
#else
        if (gpAgent->SetObjectName(ThisModule, object_reg_name, true))
            return WFS_ERR_INVALID_HWND;
        if (gpAgent->SetObjectName(ThisModule, object_name))
            return WFS_ERR_INVALID_HWND;
#endif
        return gpAgent->WFPRegister(service, event_class, req_id);
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPDeregister(HSERVICE service, DWORD event_class, LPSTR object_reg_name, LPSTR object_name, REQUESTID req_id)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPDeregister(HSERVICE service, DWORD event_class, HWND hWndReg, HWND hWnd, REQUESTID req_id)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPDeregister(HSERVICE service, DWORD event_class, LPSTR object_reg_name, LPSTR object_name, REQUESTID req_id)
#endif
    {
        THISMODULE(__FUNCTION__);
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
#ifdef QT_LINUX_MANAGER_ZJ
        if (gpAgent->SetHWND(ThisModule, hWndReg, true))
            return WFS_ERR_INVALID_HWND;
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
#else
        if (gpAgent->SetObjectName(ThisModule, object_reg_name, true))
            return WFS_ERR_INVALID_HWND;
        if (gpAgent->SetObjectName(ThisModule, object_name))
            return WFS_ERR_INVALID_HWND;
#endif

        return gpAgent->WFPDeregister(service, event_class, req_id);
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPLock(HSERVICE service, DWORD timeout, LPSTR object_name, REQUESTID req_id)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPLock(HSERVICE service, DWORD timeout, HWND hWnd, REQUESTID req_id)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPLock(HSERVICE service, DWORD timeout, LPSTR object_name, REQUESTID req_id)
#endif
    {
        THISMODULE(__FUNCTION__);
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
#ifdef QT_LINUX_MANAGER_ZJ
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
#else

        if (gpAgent->SetObjectName(ThisModule, object_name))
            return WFS_ERR_INVALID_HWND;
#endif

        return gpAgent->WFPLock(service, timeout, req_id);
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPUnlock(HSERVICE service, LPSTR object_name, REQUESTID req_id)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPUnlock(HSERVICE service, HWND hWnd, REQUESTID req_id)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPUnlock(HSERVICE service, LPSTR object_name, REQUESTID req_id)
#endif
    {
        THISMODULE(__FUNCTION__);
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
#ifdef QT_LINUX_MANAGER_ZJ
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
#else

        if (gpAgent->SetObjectName(ThisModule, object_name))
            return WFS_ERR_INVALID_HWND;
#endif

        return gpAgent->WFPUnlock(service, req_id);
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPGetInfo(HSERVICE service, DWORD category, LPVOID query_details, DWORD timeout, LPSTR object_name, REQUESTID req_id)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPGetInfo(HSERVICE service, DWORD category, LPVOID query_details, DWORD timeout, HWND hWnd, REQUESTID req_id)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPGetInfo(HSERVICE service, DWORD category, LPVOID query_details, DWORD timeout, LPSTR object_name, REQUESTID req_id)
#endif
    {
        THISMODULE(__FUNCTION__);
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
#ifdef QT_LINUX_MANAGER_ZJ
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
#else

        if (gpAgent->SetObjectName(ThisModule, object_name))
            return WFS_ERR_INVALID_HWND;
#endif

        return gpAgent->WFPGetInfo(service, category, query_details, timeout, req_id);
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPExecute(HSERVICE service, DWORD command, LPVOID cmd_data, DWORD timeout, LPSTR object_name, REQUESTID req_id)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPExecute(HSERVICE service, DWORD command, LPVOID cmd_data, DWORD timeout, HWND hWnd, REQUESTID req_id)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPExecute(HSERVICE service, DWORD command, LPVOID cmd_data, DWORD timeout, LPSTR object_name, REQUESTID req_id)
#endif
    {
        THISMODULE(__FUNCTION__);
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
#ifdef QT_LINUX_MANAGER_ZJ
        if (gpAgent->SetHWND(ThisModule, hWnd))
            return WFS_ERR_INVALID_HWND;
#else

        if (gpAgent->SetObjectName(ThisModule, object_name))
            return WFS_ERR_INVALID_HWND;
#endif

        return gpAgent->WFPExecute(service, command, cmd_data, timeout, req_id);
    }
#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPCancelAsyncRequest(HSERVICE service, REQUESTID request_id)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPCancelAsyncRequest(HSERVICE service, REQUESTID request_id)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPCancelAsyncRequest(HSERVICE service, REQUESTID request_id)
#endif
    {
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        return gpAgent->WFPCancelAsyncRequest(service, request_id);
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPSetTraceLevel(HSERVICE service, DWORD trace_level)
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPSetTraceLevel(HSERVICE service, DWORD trace_level)
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPSetTraceLevel(HSERVICE service, DWORD trace_level)
#endif
    {
        CAgentXFS *gpAgent = GetAgent(service); //30-00-00-00(FT#0020)
        if (gpAgent == nullptr)
            return WFS_ERR_INTERNAL_ERROR;
        return gpAgent->WFPSetTraceLevel(service, trace_level);
    }

#ifdef QT_LINUX_MANAGER_PISA
    AGENTXFS_EXPORT HRESULT PISAAPI WFPUnloadService()
#elif QT_LINUX_MANAGER_ZJ
    AGENTXFS_EXPORT HRESULT LFSAPI WFPUnloadService()
#else
    AGENTXFS_EXPORT HRESULT LFSAPI LFPUnloadService()
#endif
    {
        return WFS_SUCCESS;
    }
#endif

}// end extern "C"
//////////////////////////////////////////////////////////////////////////
CAgentXFS::CAgentXFS(LPCSTR lpszLogicalName, LPCSTR lpAgentName):
    m_cCmdMutex(lpszLogicalName + QString("AgentXFS")),
    m_strAgentName(lpAgentName),
    m_cOpenSPMutex("AgentXFS_Open_SP")
{
    m_cXfsReg.SetLogicalName(lpszLogicalName);
    SetLogFile(LOGFILE, ThisFile, m_cXfsReg.GetSPClass());

    m_pSpRMem = nullptr;
    m_pSpWMem = nullptr;

#ifdef QT_WIN_LINUX_XFS
    m_sthWndObject.hWnd = nullptr;
    m_sthWndObject.hWndReg = nullptr;
#endif
}

CAgentXFS::~CAgentXFS()
{

}

//////////////////////////////////////////////////////////////////////////
HRESULT CAgentXFS::WFPOpen(HSERVICE hService, LPSTR lpszLogicalName, HAPP hApp, LPSTR lpszAppID, DWORD dwTraceLevel,
                           DWORD dwTimeOut, REQUESTID ReqID, HPROVIDER hProvider, DWORD dwSPIVersionsRequired,
                           LPWFSVERSION lpSPIVersion, DWORD dwSrvcVersionsRequired, LPWFSVERSION lpSrvcVersion)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);
    CAutoMutex  _auto_sp_mutex(&m_cOpenSPMutex);// 互斥启动SP进程

    if (!lpszLogicalName || !lpSPIVersion || !lpSrvcVersion)
    {
        Log(ThisModule, __LINE__, "无效指针");
        LOG_EXIT_OPEN(WFS_ERR_INVALID_POINTER);
    }

#ifdef QT_LINUX
//    if (!WhiteListCheck())
//    {
//        Log(ThisModule, __LINE__, "SP白名单检测失败");
//        return WFS_ERR_INTERNAL_ERROR;
//    }
#endif

    // 清空Base数据
    m_stCmdBase.clear();
    m_stAgentBase.clear();
    m_cXfsData.ClearCmd();
    m_cXfsData.ClearResult();
    m_cAgentData.Clear();

    // 读取配置
    m_cXfsReg.SetLogicalName(lpszLogicalName);
    strcpy(m_stCmdBase.szLogicalName, lpszLogicalName);
    strcpy(m_stCmdBase.szSPClass, m_cXfsReg.GetSPClass());
    strcpy(m_stCmdBase.szSPName, m_cXfsReg.GetSPName());
    if (strlen(m_stCmdBase.szSPName) == 0)
    {
        Log(ThisModule, __LINE__, "找不到SP服务, lpszLogicalName=%s", lpszLogicalName);
        LOG_EXIT_OPEN(WFS_ERR_SERVICE_NOT_FOUND);
    }

    // 加载库
    if (!LoadDll(ThisModule))
    {
        Log(ThisModule, __LINE__, "加载依赖库失败, lpszLogicalName=%s", lpszLogicalName);
        LOG_EXIT_OPEN(WFS_ERR_INTERNAL_ERROR);
    }

    // 校验版本
    HRESULT hRet = CheckVersion(dwSPIVersionsRequired, lpSPIVersion, dwSrvcVersionsRequired, lpSrvcVersion);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "校验SP支持版本失败, hRet=%d,lpszLogicalName=%s,dwSPIVersionsRequired=%u,dwSrvcVersionsRequired=%u",
            hRet, lpszLogicalName, dwSPIVersionsRequired, dwSrvcVersionsRequired);
        LOG_EXIT_OPEN(hRet);
    }

    // 校验参数
    hRet = CheckParameter(hService, ReqID, ThisModule, true);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "校验参数失败, lpszLogicalName=%s", lpszLogicalName);
        LOG_EXIT_OPEN(hRet);
    }

    // 启动SP进程
    hRet = CreateSPWnd(lpszLogicalName, m_stCmdBase.szSPName, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "启动SP进程失败, lpszLogicalName=%s", lpszLogicalName);
        LOG_EXIT_OPEN(hRet);
    }

    // 延时启动
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // 启动线程
    StartThread();

    // 保存基础命令
    m_stCmdBase.eCmdID = WFS_OPEN;
    m_stCmdBase.hApp = hApp;
    m_stCmdBase.dwTraceLevel = dwTraceLevel;
    m_stCmdBase.dwTimeOut = dwTimeOut;
    m_stCmdBase.hProvider = hProvider;
    strcpy(m_stCmdBase.szAppName, m_strAppName.c_str());
    strcpy(m_stCmdBase.szAppID, m_strAppPID.c_str());

    m_cXfsData.Add(m_stCmdBase);
    m_cAgentData.Add(m_stAgentBase);
    LOG_EXIT_OPEN(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPClose(HSERVICE hService, REQUESTID ReqID)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);

    HRESULT hRet = CheckParameter(hService, ReqID, ThisModule);
    if (hRet != WFS_SUCCESS)
    {
        LOG_EXIT_CLOSE(hRet);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_CLOSE;
    stCmd.dwTimeOut = 0;
    m_cXfsData.Add(stCmd);
    m_cAgentData.Add(m_stAgentBase);
    LOG_EXIT_CLOSE(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPRegister(HSERVICE hService, DWORD dwEventClass, REQUESTID ReqID)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);

    HRESULT hRet = CheckParameter(hService, ReqID, ThisModule);
    if (hRet != WFS_SUCCESS)
    {
        LOG_EXIT_REGISTER(hRet);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_REGISTER;
    stCmd.dwTimeOut = 0;
    stCmd.dwCommand = dwEventClass;
    m_cXfsData.Add(stCmd);
    m_cAgentData.Add(m_stAgentBase);
    LOG_EXIT_REGISTER(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPDeregister(HSERVICE hService, DWORD dwEventClass, REQUESTID ReqID)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);
    HRESULT hRet = CheckParameter(hService, ReqID, ThisModule);
    if (hRet != WFS_SUCCESS)
    {
        LOG_EXIT_DEREGISTER(hRet);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_DEREGISTER;
    stCmd.dwTimeOut = 0;
    stCmd.dwCommand = dwEventClass;
    m_cXfsData.Add(stCmd);
    m_cAgentData.Add(m_stAgentBase);
    LOG_EXIT_DEREGISTER(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPLock(HSERVICE hService, DWORD dwTimeOut, REQUESTID ReqID)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);
    HRESULT hRet = CheckParameter(hService, ReqID, ThisModule);
    if (hRet != WFS_SUCCESS)
    {
        LOG_EXIT_LOCK(hRet);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_LOCK;
    stCmd.dwTimeOut = dwTimeOut;
    m_cXfsData.Add(stCmd);
    m_cAgentData.Add(m_stAgentBase);
    LOG_EXIT_LOCK(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPUnlock(HSERVICE hService, REQUESTID ReqID)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);
    HRESULT hRet = CheckParameter(hService, ReqID, ThisModule);
    if (hRet != WFS_SUCCESS)
    {
        LOG_EXIT_UNLOCK(hRet);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_UNLOCK;
    stCmd.dwTimeOut = 0;
    m_cXfsData.Add(stCmd);
    m_cAgentData.Add(m_stAgentBase);
    LOG_EXIT_UNLOCK(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPGetInfo(HSERVICE hService, DWORD dwCategory, LPVOID lpQueryDetails, DWORD dwTimeOut, REQUESTID ReqID)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);
    HRESULT hRet = CheckParameter(hService, ReqID, ThisModule);
    if (hRet != WFS_SUCCESS)
    {
        LOG_EXIT_GET_INFO(hRet);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_GETINFO;
    stCmd.dwCommand = dwCategory;
    stCmd.dwTimeOut = dwTimeOut;

    hRet = m_pAgent->GetInfo(dwCategory, lpQueryDetails, stCmd.lpCmdData);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "调用GetInfor失败：hRet=%d", hRet);
        m_pIWFM->WFMFreeBuffer(stCmd.lpResult);   // 释放内存
        LOG_EXIT_GET_INFO(hRet);
    }

    m_cXfsData.Add(stCmd);
    m_cAgentData.Add(m_stAgentBase);
    LOG_EXIT_GET_INFO(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPExecute(HSERVICE hService, DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, REQUESTID ReqID)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);
    HRESULT hRet = CheckParameter(hService, ReqID, ThisModule);
    if (hRet != WFS_SUCCESS)
    {
        LOG_EXIT_EXECUTE(hRet);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_EXECUTE;
    stCmd.dwCommand = dwCommand;
    stCmd.dwTimeOut = dwTimeOut;

    hRet = m_pAgent->Execute(dwCommand, lpCmdData, stCmd.lpCmdData);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "调用Execute[dwCommand=%u]失败：hRet=%d", dwCommand, hRet);
        m_pIWFM->WFMFreeBuffer(stCmd.lpResult);   // 释放内存
        LOG_EXIT_EXECUTE(hRet);
    }

    m_cXfsData.Add(stCmd);
    m_cAgentData.Add(m_stAgentBase);
    LOG_EXIT_EXECUTE(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPCancelAsyncRequest(HSERVICE hService, REQUESTID ReqID)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);
    if (hService != m_stCmdBase.hService)
    {
        Log(ThisModule, __LINE__, "无效服务句柄");
        LOG_EXIT_CANCEL_REQUEST(WFS_ERR_INVALID_HSERVICE);
    }
    if (!m_cAppRun.IsAppRunning(m_strSPProgram.c_str(), ""))
    {
        Log(ThisModule, __LINE__, "SP进程已退出：%s", m_strSPProgram.c_str());
        LOG_EXIT_CANCEL_REQUEST(WFS_ERR_CONNECTION_LOST);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_CANCELREQ;
    stCmd.hService = hService;
    stCmd.ReqID = ReqID;
    stCmd.dwTimeOut = 0;
    stCmd.stReqTime = CQtTime::CurrentTime();
    m_cXfsData.Add(stCmd);
    LOG_EXIT_CANCEL_REQUEST(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPSetTraceLevel(HSERVICE hService, DWORD dwTraceLevel)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);
    if (hService != m_stCmdBase.hService)
    {
        Log(ThisModule, __LINE__, "无效服务句柄");
        LOG_EXIT_SET_TRACE_LEVEL(WFS_ERR_INVALID_HSERVICE);
    }
    if (!m_cAppRun.IsAppRunning(m_strSPProgram.c_str(), ""))
    {
        Log(ThisModule, __LINE__, "SP进程已退出：%s", m_strSPProgram.c_str());
        LOG_EXIT_SET_TRACE_LEVEL(WFS_ERR_CONNECTION_LOST);
    }

    SPCMDDATA stCmd;
    stCmd.copy(m_stCmdBase);
    stCmd.eCmdID = WFS_SETTRACELEVEL;
    stCmd.hService = hService;
    stCmd.ReqID = 0;
    stCmd.dwTimeOut = 0;
    stCmd.dwTraceLevel = dwTraceLevel;
    stCmd.stReqTime = CQtTime::CurrentTime();
    m_cXfsData.Add(stCmd);
    LOG_EXIT_SET_TRACE_LEVEL(WFS_SUCCESS);
}

HRESULT CAgentXFS::WFPUnloadService()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cFuncMutex);

    return WFS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
#ifdef QT_WIN_LINUX_XFS
//////////////////////////////////////////////////////////////////////////
HRESULT CAgentXFS::SetHWND(LPCSTR ThisModule, HWND hWnd, bool bReg /*= false*/)
{
    // 判断窗口是否有效
    if (m_pQWin == nullptr)
    {
        if (0 != m_pQWin.Load("QtPostMessage.dll", "CreateIQtPostMessage"))
        {
            Log(ThisModule, __LINE__, "加载库失败: QtPostMessage.dll");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    if (!m_pQWin->IsWindowHWND(hWnd))
    {
        Log(ThisModule, __LINE__, "无效窗口句柄:%s", bReg ? "注册窗口" : "结果窗口");
        return WFS_ERR_INVALID_HWND;
    }
    if (!bReg)
        m_sthWndObject.hWnd = hWnd;
    else
        m_sthWndObject.hWndReg = hWnd;
    return WFS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
#elif QT_LINUX_MANAGER_ZJ
HRESULT CAgentXFS::SetHWND(LPCSTR ThisModule, HWND hWnd, bool bReg /*= false*/)
{
    // 判断窗口是否有效
    if (m_pQWin == nullptr)
    {
        if (0 != m_pQWin.Load("QtPostMessage.dll", "CreateIQtPostMessage"))
        {
            Log(ThisModule, __LINE__, "加载库失败: QtPostMessage.dll");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    //msgget返回的消息队列id可以是0
//    if (hWnd == nullptr)
//        return WFS_ERR_INVALID_HWND;

    if (!bReg)
        m_sthWndObject.hWnd = hWnd;
    else
        m_sthWndObject.hWndReg = hWnd;
    return WFS_SUCCESS;
}
#else
//////////////////////////////////////////////////////////////////////////
HRESULT CAgentXFS::SetObjectName(LPCSTR ThisModule, LPCSTR lpObjectName, bool bReg /*= false*/)
{
    if (m_pQWin == nullptr)
    {
        if (0 != m_pQWin.Load("QtPostMessage.dll", "CreateIQtPostMessage"))
        {
            Log(ThisModule, __LINE__, "加载库失败: QtPostMessage.dll");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    // 判断DBUS名称是否有效
    if (!m_pQWin->IsWindowDBus(lpObjectName))
    {
        Log(ThisModule, __LINE__, "无效窗口句柄:%s", bReg ? "注册窗口" : "结果窗口");
        return WFS_ERR_INVALID_HWND;
    }

    // 保存DBUS名称
    if (!bReg)
        m_sthWndObject.strObjectName = lpObjectName;
    else
        m_sthWndObject.strObjectRegName = lpObjectName;
    return WFS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////
void CAgentXFS::Run_Cmd()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    try
    {
        SPCMDDATA stCmd;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (m_pSpWMem == nullptr)
                continue;

            // 检测是否有新命令
            if (!m_cXfsData.Peek(stCmd))
                continue;

            // 修正超时时间，当为0时，表示无限超时
            stCmd.dwTimeOut = (stCmd.dwTimeOut == 0) ? ((DWORD)(-1)) : stCmd.dwTimeOut;
            // 发送新命令到SP窗口
            AutoMutex(m_cCmdMutex);   // 同类型的XFS互斥
            if (!m_pSpWMem->Write(&stCmd, sizeof(stCmd)))
            {
                Log(ThisModule, __LINE__, "发送命令数据失败:CMDID = %d", stCmd.eCmdID);
                FreeBuffer(stCmd);
                continue;
            }
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_Cmd发送命令线程异常");
    }
    return;
}

void CAgentXFS::Run_Read()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        SPRESULTDATA stResult;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (m_pSpRMem == nullptr)
                continue;

            stResult.clear();
            if (!m_pSpRMem->Read(&stResult, sizeof(stResult)))
                continue;

            // 保存新结果
            m_cXfsData.Add(stResult);
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_Read通信线程异常");
    }
    return;
}

void CAgentXFS::Run_Result()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        bool bComEvent = false;
        SPRESULTDATA stResult;
        AGNETRESULTDATA stAgentResult;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            // 检测是否有新结果
            if (!m_cXfsData.Peek(stResult))
                continue;

            // 发送命令结果到应用窗口
            switch (stResult.uMsgID)
            {
            case WFS_OPEN_COMPLETE:
            case WFS_CLOSE_COMPLETE:
            case WFS_LOCK_COMPLETE:
            case WFS_UNLOCK_COMPLETE:
            case WFS_REGISTER_COMPLETE:
            case WFS_DEREGISTER_COMPLETE:
            case WFS_GETINFO_COMPLETE:
            case WFS_EXECUTE_COMPLETE:
                {
                    bComEvent = true;
                    if (!m_cAgentData.Peek(stAgentResult, stResult))
                    {
                        Log(ThisModule, __LINE__, "无效结果,uMsgID = %d[%s]", stResult.uMsgID, ConvertMSGID(stResult.uMsgID));
                        FreeBuffer(stResult);
                        continue;
                    }
                }
                break;
            case WFS_EXECUTE_EVENT:
            case WFS_SERVICE_EVENT:
            case WFS_USER_EVENT:
            case WFS_SYSTEM_EVENT:
                bComEvent = false;
                break;
            default:
                Log(ThisModule, __LINE__, "未知消息：uMsgID = %d", stResult.uMsgID);
                FreeBuffer(stResult);
                continue;
            }

#ifdef QT_WIN_LINUX_XFS
            HWND hWndResult = bComEvent ? stAgentResult.hWnd : m_sthWndObject.hWndReg;
            XFSLogManagerGetInstance()->LOGXFSMessageInfor(stResult.szAgentName, stResult.uMsgID, stResult.lpResult);

            if (!m_pQWin->IsWindowHWND(hWndResult))
            {
                Log(ThisModule, __LINE__, "无效窗口句柄,uMsgID = %d[%s]", stResult.uMsgID, ConvertMSGID(stResult.uMsgID));
                FreeBuffer(stResult);
                continue;
            }
            if (!m_pQWin->PostMessageHWND(hWndResult, stResult.uMsgID, 0, (LPARAM)stResult.lpResult))
            {
                Log(ThisModule, __LINE__, "发送事件数据失败:uMsgID = %d(%s)", stResult.uMsgID, ConvertMSGID(stResult.uMsgID));
                FreeBuffer(stResult);
                continue;
            }
#elif QT_LINUX_MANAGER_ZJ
            HWND hWndResult = bComEvent ? stAgentResult.hWnd : m_sthWndObject.hWndReg;
            XFSLogManagerGetInstance()->LOGXFSMessageInfor(stResult.szAgentName, stResult.uMsgID, stResult.lpResult);

            if (!m_pQWin->PostMessageHWND(hWndResult, stResult.uMsgID, 0, (LPARAM)stResult.lpResult))
            {
                Log(ThisModule, __LINE__, "发送事件数据失败:uMsgID = %d(%s)", stResult.uMsgID, ConvertMSGID(stResult.uMsgID));
                FreeBuffer(stResult);
                continue;
            }
#else
            string strObjectName = bComEvent ? stAgentResult.szObjectName : m_sthWndObject.strObjectRegName;
            XFSLogManagerGetInstance()->LOGXFSMessageInfor(strObjectName.c_str(), (UINT)stResult.uMsgID, LPWFSRESULT(stResult.lpResult));
            if (!m_pQWin->IsWindowDBus(strObjectName.c_str()))
            {
                Log(ThisModule, __LINE__, "无效窗口句柄,uMsgID = %d[%s]", stResult.uMsgID, ConvertMSGID(stResult.uMsgID));
                FreeBuffer(stResult);
                continue;
            }
            if (!m_pQWin->PostMessagePISA(strObjectName.c_str(), stResult.uMsgID, (LPVOID)stResult.lpResult))
            {
                Log(ThisModule, __LINE__, "发送事件数据失败:uMsgID = %d(%s)", stResult.uMsgID, ConvertMSGID(stResult.uMsgID));
                FreeBuffer(stResult);
                continue;
            }
#endif
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_Result发送结果线程异常");
    }
    return;
}

//////////////////////////////////////////////////////////////////////////

bool CAgentXFS::LoadDll(LPCSTR ThisModule)
{
    if (m_pAgent == nullptr)
    {
        char szAgentDllName[256] = { 0 };
        strcpy(szAgentDllName, m_cXfsReg.GetValue("AgentDllName", ""));
        if (0 != m_pAgent.Load(szAgentDllName, "CreateIAgentBase"))
        {
            Log(ThisModule, __LINE__, "加载库失败: AgentDllName=%s", szAgentDllName);
            return false;
        }
    }
    if (m_pIWFM == nullptr)
    {
        if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
        {
            Log(ThisModule, __LINE__, "加载库失败: WFMShareMenory.dll[%s]",
                m_pIWFM.LastError().toStdString().c_str());
            return false;
        }

        // 设置一次
        m_cXfsData.SetIWFMShareMenory(m_pIWFM);
        m_cAgentData.SetIWFMShareMenory(m_pIWFM);
    }

    return (m_pAgent != nullptr && m_pIWFM != nullptr);
}


HRESULT CAgentXFS::GetResultBuff(LPWFSRESULT &lpResult)
{
    // 申请结果内存,让SPBaseClass申请
    lpResult = nullptr;
    return WFS_SUCCESS;
}

HRESULT CAgentXFS::CheckParameter(HSERVICE hService, REQUESTID ReqID, LPCSTR ThisModule, bool bOpen/* = false*/)
{
    if (bOpen)
    {
        if (!hService)
        {
            Log(ThisModule, __LINE__, "无效服务句柄");
            return WFS_ERR_INVALID_HSERVICE;
        }
        if (!ReqID)
        {
            Log(ThisModule, __LINE__, "无效请求ID=%u", ReqID);
            return WFS_ERR_INVALID_REQ_ID;
        }
    }
    else
    {
        if (hService != m_stCmdBase.hService)
        {
            Log(ThisModule, __LINE__, "无效服务句柄");
            return WFS_ERR_INVALID_HSERVICE;
        }
        // 自有Manager偶发连发２个间隔时间很短的Info命令，先发的Info命令ReqId大，后发的命令ReqId小,导致异常返回，先注释掉错误返回
        // 根本原因时Manager使用的递归锁
        if (ReqID <= m_stCmdBase.ReqID)
        {
            Log(ThisModule, __LINE__, "无效请求ID=%u[上一ID=%u]", ReqID, m_stCmdBase.ReqID);
//            return WFS_ERR_INVALID_REQ_ID;
        }
        if (!m_cAppRun.IsAppRunning(m_strSPProgram.c_str(), ""))
        {
            Log(ThisModule, __LINE__, "SP进程已退出：%s", m_strSPProgram.c_str());
            return WFS_ERR_CONNECTION_LOST;
        }
    }

    // 申请结果内存
    HRESULT hRet = GetResultBuff(m_stCmdBase.lpResult);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "申请结果内存失败:hRet=%d", hRet);
        return hRet;
    }

    // 更新
    m_stCmdBase.hService = hService;
    m_stCmdBase.ReqID = ReqID;
    m_stCmdBase.stReqTime = CQtTime::CurrentTime();
    strcpy(m_stCmdBase.szAgentName, m_strAgentName.c_str());

    // 更新
    m_stAgentBase.hService = hService;
    m_stAgentBase.ReqID = ReqID;
    m_stAgentBase.stReqTime = m_stCmdBase.stReqTime;
    strcpy(m_stAgentBase.szAgentName, m_strAgentName.c_str());
#ifdef QT_WIN_LINUX_XFS
#elif QT_LINUX_MANAGER_ZJ
    m_stAgentBase.hWnd = m_sthWndObject.hWnd;
    m_stAgentBase.hWndReg = m_sthWndObject.hWndReg;
#else
    strcpy(m_stAgentBase.szObjectName, m_sthWndObject.strObjectName.c_str());
    strcpy(m_stAgentBase.szObjectRegName, m_sthWndObject.strObjectRegName.c_str());
#endif

    return WFS_SUCCESS;
}
HRESULT CAgentXFS::CheckVersion(DWORD dwSPIVersionsRequired, LPWFSVERSION lpSPIVersion, DWORD dwSrvcVersionsRequired, LPWFSVERSION lpSrvcVersion)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 查看版本是否支持
    std::string strVersion = m_cXfsReg.GetValue("Version", "");
    Log(ThisModule, 1, "读取SP版本信息:%s", strVersion.c_str());
    if (strVersion.empty())
    {
        Log(ThisModule, __LINE__, "读取SP版本信息失败");
        return WFS_ERR_INTERNAL_ERROR;
    }
    WORD wSPVersion = StringToVersion(strVersion.c_str());
    WORD wSPIVersionLow = HIWORD(dwSPIVersionsRequired);
    WORD wSPIVersionHigh = LOWORD(dwSPIVersionsRequired);
    WORD wSrvcVersionLow = HIWORD(dwSrvcVersionsRequired);
    WORD wSrvcVersionHigh = LOWORD(dwSrvcVersionsRequired);

    memset(lpSPIVersion, 0x00, sizeof(WFSVERSION));
    memset(lpSrvcVersion, 0x00, sizeof(WFSVERSION));
    lpSrvcVersion->wVersion = wSPVersion;
    lpSrvcVersion->wLowVersion = wSPVersion;
    lpSrvcVersion->wHighVersion = wSPVersion;
    lpSPIVersion->wVersion = wSPVersion;
    lpSPIVersion->wLowVersion = wSPVersion;
    lpSPIVersion->wHighVersion = wSPVersion;

    // 因LINUX管理器版本和应用请求的管理不一致，导致同时判断会失败，所以只有WIN版本才判断应用请求版本
#ifdef QT_WIN32
    if (CompareVersion(wSPVersion, wSPIVersionLow) < 0)
    {
        return WFS_ERR_SPI_VER_TOO_HIGH;
    }
    if (CompareVersion(wSPVersion, wSPIVersionHigh) > 0)
    {
        return WFS_ERR_SPI_VER_TOO_LOW;
    }
#endif
//    if (CompareVersion(wSPVersion, wSrvcVersionLow) < 0)
//    {
//        return WFS_ERR_SRVC_VER_TOO_HIGH;
//    }
//    if (CompareVersion(wSPVersion, wSrvcVersionHigh) > 0)
//    {
//        return WFS_ERR_SRVC_VER_TOO_LOW;
//    }

    return WFS_SUCCESS;
}

WORD CAgentXFS::StringToVersion(LPCSTR sVersion)
{
    LPCSTR p = sVersion;
    WORD dwMajor = 0, dwMinor = 0;
    while (isdigit(*p))
    {
        dwMajor = dwMajor * 10 + (WORD)(*p - '0');
        p++;
    }
    if (*p == '.')
        p++;

    while (isdigit(*p))
    {
        dwMinor = dwMinor * 10 + (WORD)(*p - '0');
        p++;
    }

    return MAKEWORD(dwMajor, dwMinor);
}

HRESULT CAgentXFS::CompareVersion(WORD v1, WORD v2)
{
    if (v1 == v2)
        return 0;
    if (LOBYTE(v1) < LOBYTE(v2))
        return -1;
    if (LOBYTE(v1) > LOBYTE(v2))
        return 1;
    if (HIBYTE(v1) > HIBYTE(v2))
        return 1;
    return -1;
}

HRESULT CAgentXFS::CreateSPWnd(LPCSTR lpszLogicalName, LPCSTR lpszSPName, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cCmdMutex);   // 互斥启动SP进程

    std::string strProgram = m_cXfsReg.GetValue("Program", "");
    if (strProgram.empty())
    {
        Log(ThisModule, __LINE__, "无效Program配置项");
        return WFS_ERR_INTERNAL_ERROR;
    }
    // SP单实例服务名
    m_strSPProgram = strProgram;

    // 创建返回通信连接
    if (m_pSpRMem == nullptr)
    {
        m_pSpRMem = new CSPMemoryRW(m_strAgentName.c_str(), true);
        if (m_pSpRMem == nullptr || !m_pSpRMem->IsOpened())
        {
            Log(ThisModule, __LINE__, "创建返回通信连接失败");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }
    // 创建是否在运行标志
    QString strAppName;
#ifdef QT_WIN32
    // 因WIN版本，特殊处理APPName
    wchar_t wszPath[256] = {0};
    GetModuleFileName(nullptr, wszPath, sizeof(wszPath));
    strAppName = QString::fromWCharArray(wszPath);
    strAppName = strAppName.mid(strAppName.lastIndexOf('\\') + 1);
    QCoreApplication::setApplicationName(strAppName);
#else
    strAppName = QCoreApplication::applicationName();
#endif
    DWORD dwCurProcessID = (DWORD)QCoreApplication::applicationPid();
    m_strAppPID = to_string(dwCurProcessID);
    m_strAppName = strAppName.toStdString();

    if (!m_cAppRun.CreateRunnningFlag(m_strAppName.c_str(), m_strAppPID.c_str()))
    {
        Log(ThisModule, __LINE__, "创建是否在运行标志失败");
        return WFS_ERR_INTERNAL_ERROR;
    }

    if(m_cAppRun.IsAppRunning(strProgram.c_str(), "") == TRUE){
        sleep(2);
    }


    if (!m_cAppRun.IsAppRunning(strProgram.c_str(), ""))     // 没找到
    {
        // 启动SP
        if (dwTimeOut == 0) dwTimeOut = 15 * 1000;
        HRESULT hRet = CreateProcessAndWaitForIdle(strProgram.c_str(), lpszLogicalName, dwTimeOut);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "启动SP进程失败：Program=%s[LogicalName=%s,SPName=%s]", strProgram.c_str(), lpszLogicalName, lpszSPName);
            return hRet;
        }

        if (!m_cAppRun.IsAppRunning(strProgram.c_str(), ""))     // 没找到
        {
            Log(ThisModule, __LINE__, "无法找到的SP进程[%s]:SPName=%s", strProgram.c_str(), lpszSPName);
            return WFS_ERR_CONNECTION_LOST;
        }
    }

    // 打开发送通信连接
    QString strRMem = strProgram.c_str() + QString(lpszSPName) + "SP";
    if (m_pSpWMem == nullptr)
    {
        m_pSpWMem = new CSPMemoryRW(strRMem);
        if (m_pSpWMem == nullptr || !m_pSpWMem->IsOpened())
        {
            Log(ThisModule, __LINE__, "打开发送通信连接失败");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    return WFS_SUCCESS;
}

HRESULT CAgentXFS::CreateProcessAndWaitForIdle(LPCSTR lpProgramName, LPCSTR lpszLogicalName, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 命令行
    char szCmdLine[MAX_PATH] = { 0 };
    sprintf(szCmdLine, "%s %s %s", lpProgramName, lpszLogicalName, m_strAppPID.c_str());

    char szEventName[128] = { 0 };
    sprintf(szEventName, SPREADYEVENTNAMEFORMAT, lpszLogicalName, m_strAppPID.c_str());
    CAutoEventEx cEvent(szEventName);

    //创建进程
    if (!m_cAppRun.RunApp(lpProgramName, "", szCmdLine, (long)dwTimeOut))
    {
        Log(ThisModule, __LINE__, "创建SP进程失败:%s", szCmdLine);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 等待SP启动成功并且进入空闲状态
    if (!cEvent.WaitForEvent())
    {
        Log(ThisModule, __LINE__, "等待SP进程失败:%s", szCmdLine);
        return WFS_ERR_INTERNAL_ERROR;
    }
    return WFS_SUCCESS;
}

void CAgentXFS::FreeBuffer(SPCMDDATA &stCmd)
{
    if (stCmd.lpCmdData != nullptr && m_pIWFM != nullptr)
        m_pIWFM->WFMFreeBuffer(stCmd.lpCmdData);
    stCmd.clear();
}

void CAgentXFS::FreeBuffer(SPRESULTDATA &stResult)
{
    if (stResult.lpResult != nullptr && m_pIWFM != nullptr)
        m_pIWFM->WFMFreeBuffer(stResult.lpResult);
    stResult.clear();
}

LPCSTR CAgentXFS::ConvertMSGID(UINT uMSGID)
{
    switch (uMSGID)
    {
    case WFS_OPEN_COMPLETE: return "WFS_OPEN_COMPLETE";
    case WFS_CLOSE_COMPLETE: return "WFS_CLOSE_COMPLETE";
    case WFS_EXECUTE_COMPLETE: return "WFS_EXECUTE_COMPLETE";
    case WFS_GETINFO_COMPLETE: return "WFS_GETINFO_COMPLETE";
    case WFS_REGISTER_COMPLETE: return "WFS_REGISTER_COMPLETE";
    case WFS_EXECUTE_EVENT: return "WFS_EXECUTE_EVENT";
    case WFS_SERVICE_EVENT: return "WFS_SERVICE_EVENT";
    case WFS_USER_EVENT: return "WFS_USER_EVENT";
    case WFS_SYSTEM_EVENT: return "WFS_SYSTEM_EVENT";
    default: return "UNKNOWN MSGID ";
    }
}
