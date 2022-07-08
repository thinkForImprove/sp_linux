//IXFSLogManager.h
#ifndef IXFS_LOG_MANAGER_H
#define IXFS_LOG_MANAGER_H

#include "XFSAPI.H"

struct IXFSLogManager
{
    //记录SPI 函数WFPCancelAsyncRequest调用开始时的相关信息
    virtual HRESULT LOGCancelAsyncRequest(HSERVICE service, REQUESTID request_id, long result) = 0;

    //记录SPI 函数WFPClose调用开始时的相关信息
    virtual HRESULT LOGClose(HSERVICE service, LPSTR object_name, REQUESTID req_id, long result) = 0;

    //记录SPI 函数WFPDeregister调用开始时的相关信息。
    virtual HRESULT LOGDeregister(HSERVICE service, DWORD event_class, LPSTR object_reg_name,
                                  LPSTR object_name, REQUESTID req_id, long result) = 0;

    //记录SPI 函数WFPExecute调用开始时的相关信息。
    virtual HRESULT LOGExecute(HSERVICE service, DWORD command, LPVOID cmd_data, DWORD timeout,
                               LPSTR object_name, REQUESTID req_id, long result) = 0;

    //记录SPI 函数WFPGetInfo调用开始时的相关信息。
    virtual HRESULT LOGGetInfo(HSERVICE service, DWORD category, LPVOID query_details, DWORD timeout,
                               LPSTR object_name, REQUESTID req_id, long result) = 0;

    //记录SPI 函数WFPLock调用开始时的相关信息。
    virtual HRESULT LOGLock(HSERVICE service, DWORD timeout, LPSTR object_name, REQUESTID req_id, long result) = 0;

    //记录SPI 函数WFPOpen调用开始时的相关信息。
    virtual HRESULT LOGOpen(HSERVICE service, LPSTR logical_name, HAPP app_handle, LPSTR app_id, DWORD trace_level, DWORD timeout,
                            LPSTR object_name, REQUESTID req_id, HPROVIDER provider, DWORD spi_versions_required,
                            LPWFSVERSION spi_version, DWORD srvc_versions_required, LPWFSVERSION srvc_version, long result) = 0;


    //记录SPI 函数WFPRegister调用开始时的相关信息。
    virtual HRESULT LOGRegister(HSERVICE service,  DWORD event_class, LPSTR object_reg_name,
                                LPSTR object_name, REQUESTID req_id, long result) = 0;

    //记录SPI 函数WFPSetTraceLevel调用开始时的相关信息。
    virtual HRESULT LOGSetTraceLevel(HSERVICE service, DWORD trace_level, long result) = 0;

    //记录SPI 函数WFPUnloadService调用开始时的相关信息。
    virtual HRESULT LOGUnloadService(long result) = 0;

    //记录SPI 函数WFPUnlock调用开始时的相关信息。
    virtual HRESULT LOGUnlock(HSERVICE service, LPSTR object_name, REQUESTID req_id, long result) = 0;

    //记录SP相关事件信息。
    //uiMsg: WFS_OPEN_COMPLETE ~ WFS_SYSTEM_EVENT
    virtual HRESULT LOGXFSMessageInfor(LPCSTR object_name, UINT uiMsg, LPWFSRESULT result) = 0;
};

//创建/删除日志记录接口的实例
extern "C" IXFSLogManager *XFSLogManagerGetInstance();
extern "C" void XFSLogManagerDestroyInstance();

#endif //IXFS_LOG_MANAGER_H
