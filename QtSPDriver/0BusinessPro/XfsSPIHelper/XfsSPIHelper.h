#pragma once

//////////////////////////////////////////////////////////////////////////
#include "QtTypeDef.h"
#include "QtDLLLoader.h"
#include "SimpleMutex.h"
#include "IWFMShareMenory.h"
#include "StlSimpleThread.h"
#include "MultiString.h"
#include "INIFileReader.h"
#include "ILogWrite.h"
#include "AutoQtHelpClass.h"
#include "QtAppRunning.h"
#include "QtShareMemoryRW.h"

//////////////////////////////////////////////////////////////////////////
#include "XFSAPI.H"
#include <QProcess>
#include <QDir>
#include <QFile>
//////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)// 对齐格式和SP头文件一样
//////////////////////////////////////////////////////////////////////////
#define SPREADYEVENTNAMEFORMAT    "StartReadyEvent_%s%s"
//////////////////////////////////////////////////////////////////////////

enum WFSCMDID
{
    WFS_CMD = 100,
    WFS_NULL = 0,
    WFS_OPEN = 1,
    WFS_CLOSE,
    WFS_LOCK,
    WFS_UNLOCK,
    WFS_REGISTER,
    WFS_DEREGISTER,
    WFS_GETINFO,
    WFS_EXECUTE,
    WFS_CANCELREQ,
    WFS_SETTRACELEVEL
};
//////////////////////////////////////////////////////////////////////////

typedef struct tag_sp_cmd_data
{
    char szLogicalName[256];        // 打开的逻辑名
    char szSPClass[256];            // SP类型
    char szSPName[256];             // 打开的SP名
    char szAgentName[256];          // 代理名称
    WFSCMDID eCmdID;                // 命令类型ID
    HSERVICE hService;              // 服务句柄
    HAPP hApp;                      // 应用句柄
    char szAppName[256];            // 应用名称
    char szAppID[256];              // 应用ID
    HPROVIDER hProvider;            // 服务提供者句柄
    REQUESTID ReqID;                // 命令请求ID
    DWORD dwTraceLevel;             // 日志等级
    DWORD dwTimeOut;                // 命令超时时间
    DWORD dwCommand;                // 命令ID
    LPVOID lpCmdData;               // 命令数据
    SYSTEMTIME stReqTime;           // 命令接收时间
    LPWFSRESULT lpResult;           // 结果数据

    tag_sp_cmd_data() { clear(); }
    void copy(const tag_sp_cmd_data &st) { memcpy(this, &st, sizeof(tag_sp_cmd_data)); }
    void clear() { memset(this, 0x00, sizeof(tag_sp_cmd_data)); }
} SPCMDDATA, *LPSPCMDDATA;

typedef struct tag_sp_result_data
{
    char szAgentName[256];          // 代理名称
    char szAppName[256];            // 应用名称
    char szAppID[256];              // 应用ID
    HSERVICE hService;              // 服务句柄
    REQUESTID ReqID;                // 命令请求ID
    SYSTEMTIME stReqTime;           // 命令接收时间
    UINT uMsgID;                    // 结果消息类型
    LPWFSRESULT lpResult;           // 结果数据

    tag_sp_result_data() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_sp_result_data)); }
} SPRESULTDATA, *LPSPRESULTDATA;

typedef struct tag_app_service_data
{
    char szLogicalName[256];        // 打开的逻辑名
    char szSPClass[256];            // SP类型
    char szSPName[256];             // 打开的SP名
    char szAgentName[256];          // 代理名称
    char szAppName[256];            // 应用名称
    char szAppID[256];              // 应用ID
    HSERVICE hService;              // 服务句柄
    DWORD dwEventID;                // 注册事件ID
    bool bLocked;                   // 是否锁定
    SYSTEMTIME stStartTime;         // 服务开始时间

    tag_app_service_data() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_app_service_data)); }
} APPSERVICEDATA;

//////////////////////////////////////////////////////////////////////////
typedef list<SPCMDDATA>                 listSPCMDDATA;
typedef list<SPRESULTDATA>              listSPRESULTDATA;
typedef list<APPSERVICEDATA>            listAPPSERVICEDATA;

//////////////////////////////////////////////////////////////////////////
class CFindREQID
{
public:
    CFindREQID(REQUESTID uReqID) : m_uReqID(uReqID) {}
    ~CFindREQID() {}
    bool operator()(const WFSRESULT &stVal)
    {
        if (stVal.RequestID == m_uReqID)
            return true;
        else
            return false;
    }
public:
    void SetREQID(REQUESTID uReqID) { m_uReqID = uReqID; }
private:
    REQUESTID m_uReqID;
};
class CFindCMD
{
public:
    CFindCMD(HSERVICE hService, REQUESTID uReqID = 0) : m_hService(hService), m_uReqID(uReqID) {}
    ~CFindCMD() {}
    bool operator()(const SPCMDDATA &stVal)
    {
        if (stVal.hService == m_hService)
        {
            if (m_uReqID == 0)   // 如果请求ID为0,则只比较服务ID
                return true;
            if (stVal.ReqID == m_uReqID)
                return true;
            return false;
        }
        else
            return false;
    }
public:
    void SetCMD(HSERVICE hService, REQUESTID uReqID)
    {
        m_hService = hService;
        m_uReqID = uReqID;
    }
private:
    HSERVICE    m_hService;
    REQUESTID   m_uReqID;
};

class CFindHSERVICE
{
public:
    CFindHSERVICE(HSERVICE hService, string strSPName = "") : m_hService(hService), m_strSPName(strSPName) {}
    ~CFindHSERVICE() {}
    bool operator()(const APPSERVICEDATA &stVal)
    {
        if (stVal.hService == m_hService || m_strSPName == stVal.szSPName)
            return true;
        else
            return false;
    }
public:
    void SetHSERVICE(HSERVICE hService) { m_hService = hService; }
private:
    HSERVICE    m_hService;
    string      m_strSPName;
};

//////////////////////////////////////////////////////////////////////////
class CXfsAppService
{
public:
    CXfsAppService();
    ~CXfsAppService();
public:
    // 添加一个App数据
    void Add(const APPSERVICEDATA &stApp);
    // 读取一个App数据
    bool Get(APPSERVICEDATA &stApp, HSERVICE hService);
    // 读取一个App数据，并从队列中删除
    bool Peek(APPSERVICEDATA &stApp, HSERVICE hService);
    // 从队列中删除App
    bool Remove(HSERVICE hService);
    // 获取全部App
    bool GetApp(listAPPSERVICEDATA &listApp);
    // 是否存在服务
    bool IsExistService(HSERVICE hService, LPCSTR lpSPName);
    // 是否为空
    bool IsEmpty();
    // 全部为无效窗口
    bool IsAllAgentInvalid();
    // 清空命令队列
    void Clear();
    // 更新注册的事件ID
    bool UpdateRegister(const SPCMDDATA &stCmd);
    bool UpdateDeRegister(const SPCMDDATA &stCmd);
    // 添加锁定状态
    bool AppLock(HSERVICE hService);
    // 删除锁定状态
    bool AppUnLock(HSERVICE hService);
    // 判断App是否已锁定
    bool IsAppLock(HSERVICE hService);
    // 判断是否有锁定
    bool IsAnyLocked();
private:
    CSimpleMutex            m_cMutex;
    listAPPSERVICEDATA      m_listAppData;
    CQtAppRunning           m_cAppRunning;
};
//////////////////////////////////////////////////////////////////////////
class CXfsDataManager
{
public:
    CXfsDataManager();
    ~CXfsDataManager();
public:
    // 设置释放类指针
    void SetIWFMShareMenory(IWFMShareMenory *pIWFM);
    // 添加一个命令数据
    void Add(const SPCMDDATA &stCmd);
    // 添加一个结果数据
    void Add(const SPRESULTDATA &stResult);
    // 读取一个命令数据，并从队列中删除
    bool Peek(SPCMDDATA &stCmd);
    // 读取一个结果数据，并从队列中删除
    bool Peek(SPRESULTDATA &stResult);
    // 清空命令队列
    void ClearCmd();
    // 清空结果队列
    void ClearResult();
    // 判断结果队列是否为空
    bool IsResultEmpty();

    // 添加一个Get命令数据
    void AddGet(const SPCMDDATA &stCmd);
    // 添加一个Exe命令数据
    void AddExe(const SPCMDDATA &stCmd);
    // 读取一个Get命令数据，并从队列中删除
    bool PeekGet(SPCMDDATA &stCmd);
    // 读取一个Exe命令数据，并从队列中删除
    bool PeekExe(SPCMDDATA &stCmd);
    // 读取一个Exe命令数据，并从队列中删除
    bool PeekExe(HSERVICE hService, SPCMDDATA &stCmd);
    // 读取一个Exe命令数据，并从队列中删除
    bool PeekExe(HSERVICE hService, REQUESTID ReqID, SPCMDDATA &stCmd);
    // 判断Exe命令数据是否为空
    bool IsExeCmdEmpty();
    // 清空命令队列
    void ClearGetExeCmd();
    // 删除指定服务和ID的命令
    bool RemoveCmd(HSERVICE hService, REQUESTID ReqID);

    // 获取当前时间
    void GetLocalTime(SYSTEMTIME &stTime);
    void GetLocalTime(char *pTime);
private:
    bool RemoveCmd(listSPCMDDATA &listCmd, HSERVICE hService, REQUESTID ReqID);
private:
    CSimpleMutex            m_cMutex;
    CSimpleMutex            m_cCmdMutex;
    listSPCMDDATA           m_listCmd;
    listSPCMDDATA           m_listGetCmd;
    listSPCMDDATA           m_listExeCmd;
    listSPRESULTDATA        m_listResult;
    IWFMShareMenory         *m_pIWFM;
};
//////////////////////////////////////////////////////////////////////////
class CAutoWFMFreeBuffer
{
public:
    CAutoWFMFreeBuffer(IWFMShareMenory *pIWFM, HRESULT *phRet);
    ~CAutoWFMFreeBuffer();
public:
    void push_back(LPVOID lpVoid);
private:
    HRESULT            *m_phRet;
    vector<LPVOID>      m_vtBuff;
    IWFMShareMenory    *m_pIWFM;
};
//////////////////////////////////////////////////////////////////////////
class CAutoSIMFreeBuffer
{
public:
    CAutoSIMFreeBuffer(IWFMShareMenory *pIWFM, HRESULT *phRet);
    ~CAutoSIMFreeBuffer();
public:
    void push_back(LPVOID lpVoid);
private:
    HRESULT            *m_phRet;
    vector<LPVOID>      m_vtBuff;
    IWFMShareMenory    *m_pIWFM;
};
//////////////////////////////////////////////////////////////////////////
class CSPMemoryRW
{
public:
    CSPMemoryRW(QString strKey, bool bCreate = false, int nSize = 10 * 1024);
    ~CSPMemoryRW();
public:
    // 释放读阻塞一次
    void Release();
    // 是否已打开
    bool IsOpened();
    // 内存空间大小
    int Size();
    // 写数据
    bool Write(const void *pBuff, int nBuffSize);
    // 读数据
    bool Read(void *pBuff, int nBuffSize);
private:
    bool              m_bIsOpened;
    CQtShareMemoryRW  m_cMemRW;
};

//////////////////////////////////////////////////////////////////////////
#pragma pack(pop)// 修改回为默认的对齐
//////////////////////////////////////////////////////////////////////////

