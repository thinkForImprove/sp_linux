//XFSLogHelper.h

#ifndef XFS_LOG_HELPER_H
#define XFS_LOG_HELPER_H

#include "IXFSLogManager.h"

#define LOG_EXIT_EXECUTE(hResult)   \
    {\
        XFSLogManagerGetInstance()->LOGExecute(hService, dwCommand, lpCmdData, \
                                               dwTimeOut, (LPSTR)m_strAgentName.c_str(), ReqID, hResult); \
        return hResult;\
    }

#define LOG_EXIT_CANCEL_REQUEST(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGCancelAsyncRequest(hService, ReqID, hResult);\
        return hResult;\
    }

#define LOG_EXIT_CLOSE(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGClose(hService, (LPSTR)m_strAgentName.c_str(), ReqID, hResult); \
        return hResult;\
    }

#define LOG_EXIT_DEREGISTER(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGDeregister(hService, dwEventClass, \
                                                  (LPSTR)m_strAgentName.c_str(), (LPSTR)m_strAgentName.c_str(), ReqID, hResult); \
        return hResult;\
    }

#define LOG_EXIT_GET_INFO(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGGetInfo(hService, dwCategory, lpQueryDetails, \
                                               dwTimeOut, (LPSTR)m_strAgentName.c_str(), ReqID, hResult);\
        return hResult;\
    }

#define LOG_EXIT_LOCK(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGLock(hService, dwTimeOut, (LPSTR)m_strAgentName.c_str(), ReqID, hResult);\
        return hResult;\
    }

#define LOG_EXIT_OPEN(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGOpen(hService, lpszLogicalName, hApp, \
                                            lpszAppID, dwTraceLevel, dwTimeOut, (LPSTR)m_strAgentName.c_str(), ReqID, \
                                            hProvider, dwSPIVersionsRequired, lpSPIVersion, \
                                            dwSrvcVersionsRequired, lpSrvcVersion, hResult); \
        return hResult;\
    }

#define LOG_EXIT_REGISTER(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGRegister(hService, dwEventClass, (LPSTR)m_strAgentName.c_str(),  (LPSTR)m_strAgentName.c_str(), ReqID, hResult);\
        return hResult;\
    }

#define LOG_EXIT_SET_TRACE_LEVEL(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGSetTraceLevel(hService, dwTraceLevel, hResult);\
        return hResult;\
    }

#define LOG_EXIT_UNLOAD_SERVICE(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGUnloadService(hResult); \
        return hResult;\
    }

#define LOG_EXIT_UNLOCK(hResult) \
    {\
        XFSLogManagerGetInstance()->LOGUnlock(hService, (LPSTR)m_strAgentName.c_str(), ReqID, hResult); \
        return hResult;\
    }

//#define LOG_EXIT_(hResult) XFSLogManagerGetInstance()->LOGXFSMessageInfor( hWnd, UINT uiMsg, LPWFSRESULT lpWFSResult); return hResult

#endif //XFS_LOG_HELPER_H
