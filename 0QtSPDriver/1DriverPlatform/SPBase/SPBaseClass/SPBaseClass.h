#pragma once

#include "ISPBaseClass.h"
#include "ILogWrite.h"
#include "XfsRegValue.h"
#include "AutoQtHelpClass.h"
#include "QtAppRunning.h"

#include "XFSSIU.H"
#include "XFSCDM.H"
#include "XFSCIM.H"
#include "XFSPIN.H"             //30-00-00-00(FS#0002)

#include <QDir>

//////////////////////////////////////////////////////////////////////////
typedef struct tag_siu_set_event_data
{
    HSERVICE hService;
    WFSSIUENABLE stEvent;
} SIUSETEVENTDATA;
typedef list<SIUSETEVENTDATA>       listSIUSETEVENTDATA;
//////////////////////////////////////////////////////////////////////////
class CFindSIUServiceEvent
{
public:
    CFindSIUServiceEvent(HSERVICE hService) : m_hService(hService) {}
    //~CFindSIUServiceEvent() {}
    bool operator()(const SIUSETEVENTDATA &stVal)
    {
        if (stVal.hService == m_hService)
            return true;
        else
            return false;
    }

public:
    void SetService(HSERVICE hService) { m_hService = hService; }
private:
    HSERVICE m_hService;
};
//////////////////////////////////////////////////////////////////////////
class CSIUSetEventDataManager
{
public:
    CSIUSetEventDataManager() {}
    ~CSIUSetEventDataManager() {}
public:
    // 判断是否为SIU的SET_EVET命令
    bool IsSetEventCmd(const SPCMDDATA &stCmd)
    {
        DWORD dwOffset = (stCmd.dwCommand / 100) * 100;
        if (dwOffset != SIU_SERVICE_OFFSET)
            return false;
        if (strcmp(stCmd.szSPClass, "SIU") != 0)
            return false;
        if (stCmd.dwCommand != WFS_CMD_SIU_ENABLE_EVENTS)
            return false;
        auto lpEnable = static_cast<LPWFSSIUENABLE>(stCmd.lpCmdData);
        if (lpEnable == nullptr)
            return false;

        return true;
    }
    // 添加一个Event数据
    void Add(const SPCMDDATA &stCmd)
    {
        AutoMutex(m_cMutex);
        if (!IsSetEventCmd(stCmd))
            return;

        auto lpEnable = static_cast<LPWFSSIUENABLE>(stCmd.lpCmdData);
        if (lpEnable == nullptr)
            return;

        SIUSETEVENTDATA stEvent;
        stEvent.hService = stCmd.hService;
        memcpy(&stEvent.stEvent, lpEnable, sizeof(WFSSIUENABLE));
        stEvent.stEvent.lpszExtra = nullptr;
        AddEvent(stEvent);
    }

    // 添加一个Event数据，如果相同Ap，则会更新，并保存
    void AddEvent(const SIUSETEVENTDATA &stEvent)
    {
        AutoMutex(m_cMutex);
        CFindSIUServiceEvent cOldEvent(stEvent.hService);
        auto itF = find_if(m_ltEvent.begin(), m_ltEvent.end(), cOldEvent);
        if (itF == m_ltEvent.end())
        {
            m_ltEvent.push_back(stEvent);
            return;
        }
        // 更新
        WFSSIUENABLE stEnable;
        memcpy(&stEnable, &stEvent.stEvent, sizeof(stEnable));
        for (int i = 0; i < WFS_SIU_SENSORS_SIZE; i++)
        {
            if (stEnable.fwSensors[i] != WFS_SIU_NO_CHANGE)
                itF->stEvent.fwSensors[i] = stEnable.fwSensors[i];
        }
        for (int i = 0; i < WFS_SIU_DOORS_SIZE; i++)
        {
            if (stEnable.fwDoors[i] != WFS_SIU_NO_CHANGE)
                itF->stEvent.fwDoors[i] = stEnable.fwDoors[i];
        }
        for (int i = 0; i < WFS_SIU_INDICATORS_SIZE; i++)
        {
            if (stEnable.fwIndicators[i] != WFS_SIU_NO_CHANGE)
                itF->stEvent.fwIndicators[i] = stEnable.fwIndicators[i];
        }
        for (int i = 0; i < WFS_SIU_AUXILIARIES_SIZE; i++)
        {
            if (stEnable.fwAuxiliaries[i] != WFS_SIU_NO_CHANGE)
                itF->stEvent.fwAuxiliaries[i] = stEnable.fwAuxiliaries[i];
        }
        for (int i = 0; i < WFS_SIU_GUIDLIGHTS_SIZE; i++)
        {
            if (stEnable.fwGuidLights[i] != WFS_SIU_NO_CHANGE)
                itF->stEvent.fwGuidLights[i] = stEnable.fwGuidLights[i];
        }
    }
    // 读取一个Event数据
    bool Get(HSERVICE hService, SIUSETEVENTDATA &stEvent)
    {
        AutoMutex(m_cMutex);
        CFindSIUServiceEvent cOldEvent(hService);
        auto it = find_if(m_ltEvent.begin(), m_ltEvent.end(), cOldEvent);
        if (it == m_ltEvent.end())
            return false;

        memcpy(&stEvent, &(*it), sizeof(SIUSETEVENTDATA));
        return true;
    }
private:
    CSimpleMutex            m_cMutex;
    listSIUSETEVENTDATA     m_ltEvent;
};
//////////////////////////////////////////////////////////////////////////
class CSPMemoryRWHelper
{
public:
    CSPMemoryRWHelper() {}
    ~CSPMemoryRWHelper()
    {
        for (auto &it : mapMemoryRW)
        {
            delete it.second;
            it.second = nullptr;
        }
        mapMemoryRW.clear();
    }
public:
    CSPMemoryRW *GetMem(QString strKey)
    {
        auto it = mapMemoryRW.find(strKey);
        if (it != mapMemoryRW.end())
            return it->second;
        CSPMemoryRW *pRW = new CSPMemoryRW(strKey);
        if (pRW != nullptr)
            mapMemoryRW[strKey] = pRW;
        return pRW;
    }
private:
    map<QString, CSPMemoryRW *> mapMemoryRW;
};
//////////////////////////////////////////////////////////////////////////
class CAutoSetExecuteStatus
{
public:
    CAutoSetExecuteStatus(bool *pbExecuting) : m_pbExecuting(pbExecuting) { if (m_pbExecuting != nullptr) *m_pbExecuting = true; }
    ~CAutoSetExecuteStatus() { if (m_pbExecuting != nullptr) *m_pbExecuting = false; }
private:
    bool    *m_pbExecuting;
};
//////////////////////////////////////////////////////////////////////////
class CSPBaseClass : public CLogManage, public CStlSixThread, public ISPBaseClass
{
public:
    CSPBaseClass(LPCSTR lpLogType);
    virtual ~CSPBaseClass();
public:
    // 释放接口
    virtual void Release();
    // 注册回调接口
    virtual void RegisterICmdRun(ICmdRun *pICmdRun);
    // 开始运行
    virtual bool StartRun();
    // 获取SPBase数据
    virtual void GetSPBaseData(SPBASEDATA &stData);
    // 发送事件
    virtual bool FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData);
    // 打包状态改变事件数据，此是通用的
    virtual HRESULT Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData);
    // 打包故障状态事件数据，此是通用的
    virtual HRESULT Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData);
    // 打包扩展状态数据，此是通用的
    virtual HRESULT Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra);
public:
    // SP对应接口
    void OnOpen(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnClose(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnRegister(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnDeregister(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnLock(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnUnlock(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnGetInfo(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnExecute(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnCancelAsyncRequest(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnSetTraceLevel(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnUnloadService(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    void OnStatus();
    HRESULT OnWaitTaken();
    void OnUpdateDevPDL();
public:
    virtual void Run_Read();                                    // 数据读取线程
    virtual void Run_Cmd();                                     // 接收命令处理线程
    virtual void Run_GetInfo();                                 // Get命令处理线程
    virtual void Run_Execute();                                 // Exe命令处理线程
    virtual void Run_Result();                                  // 结果消息处理线程
    virtual void Run_Status();                                  // 状态更新线程
    virtual void Run_ParallelExecute();                         // Exe并行命令处理线程      //30-00-00-00(FS#0002)

protected:
    // 检测参数
    HRESULT CheckParameter(const SPCMDDATA &stCmd);
    // 申请结果内存
    HRESULT GetResultBuff(const SPCMDDATA &stCmd, SPRESULTDATA &stResult);
    // 判断并保存执行命令
    void AddExeCmd(SPCMDDATA &stCmd);
    // 保存服务信息
    void SaveServiceData(const SPCMDDATA &stCmd);
    // 检测是否已启动服务
    bool IsHasService(const SPCMDDATA &stCmd);
    // 检测是否在执行
    bool IsExecuting();
    // 检测是否有服务锁定SP
    bool IsOtherServiceLocked(const SPCMDDATA &stCmd);
    // 检测事件消息类型是否有注册
    bool IsRegisterMsgID(DWORD dwEventID, DWORD dwMsgID);
    // 检测CRS的事件是否对应
    bool IsCRSEventClassMatch(DWORD dwEventID, const APPSERVICEDATA &stAppData);
    // 检测SIU的事件是否对应
    bool IsSIUEventClassMatch(DWORD dwEventID, const APPSERVICEDATA &stAppData, const LPWFSSIUPORTEVENT lpEvent);
    // 读取环境变化中工作目录
    LPCSTR GetXfsPath();
    // 释放缓存
    void FreeBuffer(SPCMDDATA &stCmd);
    void FreeBuffer(SPRESULTDATA &stResult);
    // 是否允许发SIU事件
    bool IsSIUEnableEvent(WFSSIUENABLE stEnable, const LPWFSSIUPORTEVENT lpEvent);
    // 消息转换为字符
    LPCSTR ConvertMSGID(UINT uMSGID);
    // 取消命令
    long CancelServerCMD(HSERVICE hService);
    // 检测是否在升级固件
    bool IsUpdateDevPDL();
private:
    ICmdRun                        *m_pCmdRun;
    char                            m_szLogicalName[MAX_PATH];
    char                            m_szSPName[MAX_PATH];
    char                            m_szExeName[MAX_PATH];
    char                            m_szCurProcessID[MAX_PATH];
    CXfsAppService                  m_cXfsApp;
    CXfsDataManager                 m_cXfsData;
    CXfsDataManager                 m_cEventData;
    CXfsRegValue                    m_cXfsReg;
    SPCMDDATA                       m_stCmdBase;
    CSimpleMutex                    m_cStatusMutexExe;
    CSimpleMutex                    m_cStatusMutexGet;
    CQtSimpleMutexEx                m_cXFSMutex;
    bool                            m_bQuitStartRun;
    bool                            m_bExecuting;
    bool                            m_bOpened;
    bool                            m_bUpdateDevPDLing;
    std::string                     m_strXfsPath;
    CSIUSetEventDataManager         m_cSIUSetEvent;
    CQtAppRunning                   m_cAppRun;
    CSPMemoryRW                     *m_pSpRMem;
    CQtDLLLoader<IWFMShareMenory>   m_pIWFM;

    bool                            m_bIsParallelExecCrypt; //30-00-00-00(FS#0002)
    bool                            m_bIsExecGetData;       //30-00-00-00(FS#0002)

};
