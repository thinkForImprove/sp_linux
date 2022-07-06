#ifndef XFSLOGMANAGER_H
#define XFSLOGMANAGER_H

#include "IXFSLogManager.h"
#include "XFSBufferLogger.h"
#include "SimpleMutex.h"

#include <qglobal.h>
#include <QString>
#include <QMutexLocker>
#include <sys/time.h>


//XFS日志管理器接口实现类
class CXFSLogManager : public IXFSLogManager
{
public:
    CXFSLogManager();
    virtual ~CXFSLogManager();

    //需要子类实现的保护类成员虚函数
protected:
    virtual void LogToFile(const char *pStr) = 0;

    //子类可重载的函数
protected:
    //记录当时时间，默认格式为“[HH:mm:ss mmm]”
    virtual CStringBuffer &LogCurrentTime();

    //XFS消息类型转换为描述
    //uiCompleteMsg: XFS完成类消息，如WFS_OPEN_COMPLETE、WFS_CLOSE_COMPLETE、WFS_EXECUTE_COMPLETE
    //返回值：返回消息描述。返回“WFPxxx()”。
    virtual const char *XFSCallMsgType2Desc(UINT uiCompleteMsg);

    //XFS消息类型转换为描述
    //uiMsg: XFS完成类消息，如WFS_OPEN_COMPLETE、WFS_CLOSE_COMPLETE、WFS_EXECUTE_COMPLETE、WFS_SYSTEM_EVENT
    //返回值：返回消息描述。对于完成类，返回“WFPxxx()”，对于事件类，返回事件名，如“WFS_SYSTEM_EVENT”。
    virtual const char *XFSMsgType2Desc(UINT uiMsg);

protected:
    //------------------- 实现IXFSLogManager接口 ----------------------------
    //记录SPI 函数WFPCancelAsyncRequest调用开始时的相关信息
    virtual HRESULT LOGCancelAsyncRequest(HSERVICE service, REQUESTID request_id, long result);

    //记录SPI 函数WFPClose调用开始时的相关信息
    virtual HRESULT LOGClose(HSERVICE service, LPSTR object_name, REQUESTID req_id, long result);

    //记录SPI 函数WFPDeregister调用开始时的相关信息。
    virtual HRESULT LOGDeregister(HSERVICE service, DWORD event_class, LPSTR object_reg_name,
                                  LPSTR object_name, REQUESTID req_id, long result);

    //记录SPI 函数WFPExecute调用开始时的相关信息。
    virtual HRESULT LOGExecute(HSERVICE service, DWORD command, LPVOID cmd_data, DWORD timeout,
                               LPSTR object_name, REQUESTID req_id, long result);

    //记录SPI 函数WFPGetInfo调用开始时的相关信息。
    virtual HRESULT LOGGetInfo(HSERVICE service, DWORD category, LPVOID query_details, DWORD timeout,
                               LPSTR object_name, REQUESTID req_id, long result);

    //记录SPI 函数WFPLock调用开始时的相关信息。
    virtual HRESULT LOGLock(HSERVICE service, DWORD timeout, LPSTR object_name, REQUESTID req_id, long result);

    //记录SPI 函数WFPOpen调用开始时的相关信息。
    virtual HRESULT LOGOpen(HSERVICE service, LPSTR logical_name, HAPP app_handle, LPSTR app_id, DWORD trace_level, DWORD timeout,
                            LPSTR object_name, REQUESTID req_id, HPROVIDER provider, DWORD spi_versions_required,
                            LPWFSVERSION spi_version, DWORD srvc_versions_required, LPWFSVERSION srvc_version, long result);


    //记录SPI 函数WFPRegister调用开始时的相关信息。
    virtual HRESULT LOGRegister(HSERVICE service,  DWORD event_class, LPSTR object_reg_name,
                                LPSTR object_name, REQUESTID req_id, long result);

    //记录SPI 函数WFPSetTraceLevel调用开始时的相关信息。
    virtual HRESULT LOGSetTraceLevel(HSERVICE service, DWORD trace_level, long result);

    //记录SPI 函数WFPUnloadService调用开始时的相关信息。
    virtual HRESULT LOGUnloadService(long result);

    //记录SPI 函数WFPUnlock调用开始时的相关信息。
    virtual HRESULT LOGUnlock(HSERVICE service, LPSTR object_name, REQUESTID req_id, long result);

    //记录SP相关事件信息。
    //uiMsg: WFS_OPEN_COMPLETE ~ WFS_SYSTEM_EVENT
    virtual HRESULT LOGXFSMessageInfor(LPCSTR object_name, UINT uiMsg, LPWFSRESULT result);

    //----------------------- 保护成员函数 -----------------------------------------------
protected:
    //得到错误码的描述
    //lResult：XFS错误码，如：WFS_SUCCESS、WFS_ERR_INTERNAL_ERROR等
    //返回一个字串，如“WFS_SUCCESS”，如未找到，返回错误的数值字串
    const char *GetErrorDesc(long lResult);

    //得到消息描述
    //pMsgType：消息类型，包括：CONST_TYPE_EXECUTE、CONST_TYPE_GET_INFO、CONST_TYPE_MESSAGE
    //dwID：消息ID，如：
    const char *GetMsgDesc(const char *pMsgType, DWORD dwID);

    //测试消息是否是完成消息，完成消息如：WFS_OPEN_COMPLETE、WFS_CLOSE_COMPLETE、WFS_EXECUTE_COMPLETE
    static BOOL MsgIsCompleteMsg(UINT uiMsg);

    //功能：获取系统当前时间（SYSTEMTIME结构体）
    //参数：[out]CurTime：返回当前时间结构体
    void GetLocalTime(SYSTEMTIME &CurTime);

    //开始日志
    inline void StartLog()
    {
        m_StringBuffer.ClearContent().EndLine();
    }

    //结束日志
    inline void EndLog()
    {
        LogToFile(m_StringBuffer);
        m_StringBuffer.ClearContent();
    }

    //----------------------- 私有成员 -----------------------------------------------
protected:
    friend class CAutoStartEndLog;

    CStringBuffer       m_StringBuffer;         //字串缓冲区对象
    CXFSBufferLogger    m_XFSBufferLogger;      //记录WFSRESULT.lpBuffer、执行命令lpCmdData、查找命令lpQueryDetail的对象
    bool                 m_bEnableLog;           //是否启用日志记录
};

#endif // XFSLOGMANAGER_H
