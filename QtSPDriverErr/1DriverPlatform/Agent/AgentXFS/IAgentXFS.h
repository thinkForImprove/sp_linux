#pragma once



//////////////////////////////////////////////////////////////////////////
#if defined(AGENTXFS_LIBRARY)
#define AGENTXFS_EXPORT Q_DECL_EXPORT
#else
#define AGENTXFS_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
#define WINAPI                              __cdecl //  __stdcall
#define LFSAPI                                  //__attribute__((__cdecl__))
//////////////////////////////////////////////////////////////////////////
extern "C" {

#ifdef Q_OS_WIN
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPOpen(HSERVICE hService, LPSTR lpszLogicalName, HAPP hApp, LPSTR lpszAppID, DWORD dwTraceLevel, DWORD dwTimeOut, HWND hWnd, REQUESTID ReqID, HPROVIDER hProvider, DWORD dwSPIVersionsRequired, LPWFSVERSION lpSPIVersion, DWORD dwSrvcVersionsRequired, LPWFSVERSION lpSrvcVersion);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPClose(HSERVICE hService, HWND hWnd, REQUESTID ReqID);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPRegister(HSERVICE hService, DWORD dwEventClass, HWND hWndReg, HWND hWnd, REQUESTID ReqID);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPDeregister(HSERVICE hService, DWORD dwEventClass, HWND hWndReg, HWND hWnd, REQUESTID ReqID);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPLock(HSERVICE hService, DWORD dwTimeOut, HWND hWnd, REQUESTID ReqID);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPUnlock(HSERVICE hService, HWND hWnd, REQUESTID ReqID);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPGetInfo(HSERVICE hService, DWORD dwCategory, LPVOID lpQueryDetails, DWORD dwTimeOut, HWND hWnd, REQUESTID ReqID);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPExecute(HSERVICE hService, DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, HWND hWnd, REQUESTID ReqID);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPCancelAsyncRequest(HSERVICE hService, REQUESTID RequestID);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPSetTraceLevel(HSERVICE hService, DWORD dwTraceLevel);
    AGENTXFS_EXPORT HRESULT extern WINAPI WFPUnloadService();
#else
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPCancelAsyncRequest(HSERVICE service, REQUESTID request_id);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPClose(HSERVICE service, LPSTR object_name, REQUESTID req_id);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPDeregister(HSERVICE service, DWORD event_class, LPSTR object_reg_name, LPSTR object_name, REQUESTID req_id);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPExecute(HSERVICE service, DWORD command, LPVOID cmd_data, DWORD timeout, LPSTR object_name, REQUESTID req_id);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPGetInfo(HSERVICE service, DWORD category, LPVOID query_details, DWORD timeout, LPSTR object_name, REQUESTID req_id);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPLock(HSERVICE service, DWORD timeout, LPSTR object_name, REQUESTID req_id);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPOpen(HSERVICE service, LPSTR logical_name, HAPP app_handle, LPSTR app_id, DWORD trace_level, DWORD timeout, LPSTR object_name, REQUESTID req_id, HPROVIDER provider, DWORD spi_versions_required, LPLFSVERSION spi_version, DWORD srvc_versions_required, LPLFSVERSION srvc_version);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPRegister(HSERVICE service,  DWORD event_class, LPSTR object_reg_name, LPSTR object_name, REQUESTID req_id);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPSetTraceLevel(HSERVICE service, DWORD trace_level);
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPUnloadService();
    AGENTXFS_EXPORT HRESULT extern LFSAPI LFPUnlock(HSERVICE service, LPSTR object_name, REQUESTID req_id);
#endif

}



