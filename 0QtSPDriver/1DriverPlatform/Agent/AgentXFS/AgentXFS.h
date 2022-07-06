#pragma once
#include "IQtPostMessage.h"
#include <QtCore/qglobal.h>
#include "QtTypeInclude.h"
#include "IAgentBase.h"
#include "QtAppRunning.h"
//////////////////////////////////////////////////////////////////////////
typedef struct tag_hwnd_object_name
{
#ifdef QT_WIN_LINUX_XFS
    HWND hWndReg;                   // 事件接收窗口句柄
    HWND hWnd;                      // 结果接收窗口句柄
#else
#ifdef QT_LINUX_MANAGER_ZJ
    HWND hWndReg;                   // 事件接收窗口句柄
    HWND hWnd;                      // 结果接收窗口句柄
#endif
    std::string strObjectRegName;   // 事件接收DBUS名称
    std::string strObjectName;      // 结果接收DBUS名称
#endif
} HWNDOBJECTNAME;
//////////////////////////////////////////////////////////////////////////
typedef struct tag_agent_result_data
{
    char szAgentName[256];          // 代理名称
    char szAppName[256];            // 应用名称
    char szAppID[256];              // 应用ID
    HSERVICE hService;              // 服务句柄
    REQUESTID ReqID;                // 命令请求ID
    SYSTEMTIME stReqTime;           // 命令接收时间
    UINT uMsgID;                    // 结果消息类型
    LPWFSRESULT lpResult;           // 结果数据

#ifdef QT_WIN_LINUX_XFS
    HWND hWndReg;                   // 事件接收窗口句柄
    HWND hWnd;                      // 结果接收窗口句柄
#elif QT_LINUX_MANAGER_ZJ
    HWND hWndReg;                   // 结果接收窗口句柄
    HWND hWnd;                      // 结果接收窗口句柄
#else
    char szObjectRegName[256];      // 事件接收DBUS名称
    char szObjectName[256];         // 结果接收DBUS名称
#endif

    tag_agent_result_data() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_agent_result_data)); }
    void copy(const SPRESULTDATA &stSpResult) { memcpy(this, &stSpResult, sizeof(SPRESULTDATA)); }
    void copy(const tag_agent_result_data &st) { memcpy(this, &st, sizeof(tag_agent_result_data)); }
} AGNETRESULTDATA, *LPAGNETRESULTDATA;

typedef list<AGNETRESULTDATA>    listAGNETRESULTDATA;
//////////////////////////////////////////////////////////////////////////
class CAgentDataHelper
{
public:
    CAgentDataHelper(): m_pIWFM(nullptr) {}
    ~CAgentDataHelper() {Clear();}
public:
    // 设置释放类指针
    void SetIWFMShareMenory(IWFMShareMenory *pIWFM)
    {
        AutoMutex(m_cMutex);
        m_pIWFM = pIWFM;
    }
    // 添加一个结果数据
    void Add(const AGNETRESULTDATA &stAgent)
    {
        AutoMutex(m_cMutex);
        m_listAgent.push_back(stAgent);
    }
    // 读取一个结果数据，并从队列中删除
    bool Peek(AGNETRESULTDATA &stAgent, const SPRESULTDATA &stResult)
    {
        AutoMutex(m_cMutex);
        if (m_listAgent.empty())
            return false;

        for (auto it = m_listAgent.begin(); it != m_listAgent.end(); it++)
        {
            if (it->hService == stResult.hService &&
                it->ReqID == stResult.ReqID &&
                strcmp(it->szAgentName, stResult.szAgentName) == 0 &&
                memcmp(&it->stReqTime, &stResult.stReqTime, sizeof(SYSTEMTIME)) == 0)
            {
                stAgent = *it;
                stAgent.copy(stResult);
                m_listAgent.erase(it);
                return true;
            }
        }
        return false;
    }

    // 清空代理队列
    void Clear()
    {
        AutoMutex(m_cMutex);
        for (auto &it : m_listAgent)
        {
            if (it.lpResult != nullptr && m_pIWFM != nullptr)
            {
                m_pIWFM->WFMFreeBuffer(it.lpResult);
                it.lpResult = nullptr;
            }
        }

        m_listAgent.clear();
    }
    // 判断结果队列是否为空
    bool IsEmpty()
    {
        AutoMutex(m_cMutex);
        return m_listAgent.empty();
    }
private:
    CSimpleMutex            m_cMutex;
    listAGNETRESULTDATA     m_listAgent;
    IWFMShareMenory        *m_pIWFM;
};
//////////////////////////////////////////////////////////////////////////
class  CAgentXFS : public CLogManage, public CStlThreadThread
{
public:
    CAgentXFS(LPCSTR lpszLogicalName, LPCSTR lpAgentName);
    ~CAgentXFS();
public:
    // SPI接口

    HRESULT WFPOpen(HSERVICE hService, LPSTR lpszLogicalName, HAPP hApp, LPSTR lpszAppID, DWORD dwTraceLevel, DWORD dwTimeOut, REQUESTID ReqID, HPROVIDER hProvider, DWORD dwSPIVersionsRequired, LPWFSVERSION lpSPIVersion, DWORD dwSrvcVersionsRequired, LPWFSVERSION lpSrvcVersion);
    HRESULT WFPClose(HSERVICE hService, REQUESTID ReqID);
    HRESULT WFPRegister(HSERVICE hService, DWORD dwEventClass, REQUESTID ReqID);
    HRESULT WFPDeregister(HSERVICE hService, DWORD dwEventClass,  REQUESTID ReqID);
    HRESULT WFPLock(HSERVICE hService, DWORD dwTimeOut, REQUESTID ReqID);
    HRESULT WFPUnlock(HSERVICE hService, REQUESTID ReqID);
    HRESULT WFPGetInfo(HSERVICE hService, DWORD dwCategory, LPVOID lpQueryDetails, DWORD dwTimeOut, REQUESTID ReqID);
    HRESULT WFPExecute(HSERVICE hService, DWORD dwCommand, LPVOID lpCmdData, DWORD dwTimeOut, REQUESTID ReqID);
    HRESULT WFPCancelAsyncRequest(HSERVICE hService, REQUESTID ReqID);
    HRESULT WFPSetTraceLevel(HSERVICE hService, DWORD dwTraceLevel);
    HRESULT WFPUnloadService();
#ifdef QT_WIN_LINUX_XFS
    HRESULT SetHWND(LPCSTR ThisModule, HWND hWnd, bool bReg = false);
#elif QT_LINUX_MANAGER_ZJ
    HRESULT SetHWND(LPCSTR ThisModule, HWND hWnd, bool bReg = false);
#else
    HRESULT SetObjectName(LPCSTR ThisModule, LPCSTR lpObjectName, bool bReg = false);
#endif
protected:
    virtual void Run_Cmd();                                     // 命令发送处理线程
    virtual void Run_Read();                                    // 读取结果处理线程
    virtual void Run_Result();                                  // 结果消息处理线程
protected:

    // 加载库
    bool LoadDll(LPCSTR ThisModule);
    // 申请结果内存
    HRESULT GetResultBuff(LPWFSRESULT &lpResult);
    // 校验参数
    HRESULT CheckParameter(HSERVICE hService, REQUESTID ReqID, LPCSTR ThisModule, bool bOpen = false);
    // 检验版本
    HRESULT CheckVersion(DWORD dwSPIVersionsRequired, LPWFSVERSION lpSPIVersion, DWORD dwSrvcVersionsRequired, LPWFSVERSION lpSrvcVersion);
    // 转换字串版本为数字版本, 输入：sVersion : 字串版本，如3.00
    // 返回：数字版本，LOBYTE为主版本，HIBYTE为次版本
    WORD StringToVersion(LPCSTR sVersion);
    // 比较版本, 输入：v1, v2 : 版本
    // 返回：-1, v1 < v2; 0, v1 == v2; 1 v1 > v2
    HRESULT CompareVersion(WORD v1, WORD v2);

    // 创建SP进程:lpszLogicalName服务逻辑名,lpszSPName服务名,dwTimeOut超时
    HRESULT CreateSPWnd(LPCSTR lpszLogicalName, LPCSTR lpszSPName, DWORD dwTimeOut);
    // 创建SP服务进程并等待它进程服务状态:lpProgramName服务进程名,lpszLogicalName逻辑名
    HRESULT CreateProcessAndWaitForIdle(LPCSTR lpProgramName, LPCSTR lpszLogicalName, DWORD dwTimeOut);

    // 释放缓存
    void FreeBuffer(SPCMDDATA &stCmd);
    void FreeBuffer(SPRESULTDATA &stResult);

    // 转换消息为字符
    LPCSTR ConvertMSGID(UINT uMSGID);
private:
    CXfsDataManager                 m_cXfsData;
    CXfsRegValue                    m_cXfsReg;
    SPCMDDATA                       m_stCmdBase;
    AGNETRESULTDATA                 m_stAgentBase;
    CSimpleMutex                    m_cFuncMutex;
    CQtSimpleMutexEx                m_cCmdMutex;
    CQtSimpleMutexEx                m_cOpenSPMutex;
    CQtDLLLoader<IAgentBase>        m_pAgent;
    CQtDLLLoader<IWFMShareMenory>   m_pIWFM;
    CQtDLLLoader<IQtPostMessage>    m_pQWin;
    CSPMemoryRW                    *m_pSpWMem;
    CSPMemoryRW                    *m_pSpRMem;
    std::string                     m_strAgentName;
    std::string                     m_strSPProgram;
    std::string                     m_strAppName;
    std::string                     m_strAppPID;
    HWNDOBJECTNAME                  m_sthWndObject;
    CAgentDataHelper                m_cAgentData;
    CQtAppRunning                   m_cAppRun;
};
