#include "XfsSPIHelper.h"

//static const char *ThisFile = "XfsSPIHelper.cpp";
//////////////////////////////////////////////////////////////////////////
class CAutoMemoryLock
{
public:
    CAutoMemoryLock(QSharedMemory *pMemory) : m_pMemory(pMemory) { if (m_pMemory != nullptr) m_pMemory->lock(); }
    ~CAutoMemoryLock() { if (m_pMemory != nullptr) m_pMemory->unlock(); }
private:
    QSharedMemory *m_pMemory;
};
class CAutoQMutexLock
{
public:
    CAutoQMutexLock(QMutex *pMutex) : m_pMutex(pMutex) { if (m_pMutex != nullptr) m_pMutex->lock(); }
    ~CAutoQMutexLock() { if (m_pMutex != nullptr) m_pMutex->unlock(); }
private:
    QMutex *m_pMutex;
};

//////////////////////////////////////////////////////////////////////////
CXfsAppService::CXfsAppService()
{

}

CXfsAppService::~CXfsAppService()
{

}

void CXfsAppService::Add(const APPSERVICEDATA &stApp)
{
    AutoMutex(m_cMutex);
    m_listAppData.push_back(stApp);
}

bool CXfsAppService::Get(APPSERVICEDATA &stApp, HSERVICE hService)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    CFindHSERVICE cFind(hService);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    memcpy(&stApp, &(*it), sizeof(APPSERVICEDATA));
    return true;
}

bool CXfsAppService::Peek(APPSERVICEDATA &stApp, HSERVICE hService)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    CFindHSERVICE cFind(hService);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    memcpy(&stApp, &(*it), sizeof(APPSERVICEDATA));
    m_listAppData.erase(it);
    return true;
}

bool CXfsAppService::Remove(HSERVICE hService)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    CFindHSERVICE cFind(hService);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    m_listAppData.erase(it);
    return true;
}

bool CXfsAppService::GetApp(listAPPSERVICEDATA &listApp)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    listApp.clear();
    listApp.insert(listApp.begin(), m_listAppData.begin(), m_listAppData.end());
    return true;
}

bool CXfsAppService::IsExistService(HSERVICE hService, LPCSTR lpSPName)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    string strSPName = (lpSPName == nullptr) ? "" : lpSPName;
    CFindHSERVICE cFind(hService, strSPName);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    return true;
}

bool CXfsAppService::IsEmpty()
{
    AutoMutex(m_cMutex);
    return m_listAppData.empty();
}

bool CXfsAppService::IsAllAgentInvalid()
{
    AutoMutex(m_cMutex);
    for (auto &it : m_listAppData)
    {
        if (m_cAppRunning.IsAppRunning(it.szAppName, it.szAppID))
        {
            return false;
        }
    }
    return true;
}

void CXfsAppService::Clear()
{
    AutoMutex(m_cMutex);
    m_listAppData.clear();
}


bool CXfsAppService::UpdateRegister(const SPCMDDATA &stCmd)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    CFindHSERVICE cFind(stCmd.hService);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    it->dwEventID = stCmd.dwCommand;
    return true;
}

bool CXfsAppService::UpdateDeRegister(const SPCMDDATA &stCmd)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    CFindHSERVICE cFind(stCmd.hService);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    it->dwEventID = (it->dwEventID & (~stCmd.dwCommand));
    return true;
}

bool CXfsAppService::AppLock(HSERVICE hService)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    CFindHSERVICE cFind(hService);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    it->bLocked = true;
    return true;
}

bool CXfsAppService::AppUnLock(HSERVICE hService)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    CFindHSERVICE cFind(hService);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    it->bLocked = false;
    return true;
}

bool CXfsAppService::IsAppLock(HSERVICE hService)
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    CFindHSERVICE cFind(hService);
    auto it = find_if(m_listAppData.begin(), m_listAppData.end(), cFind);
    if (it == m_listAppData.end())
        return false;

    return it->bLocked;
}

bool CXfsAppService::IsAnyLocked()
{
    AutoMutex(m_cMutex);
    if (m_listAppData.empty())
        return false;

    for (auto &it : m_listAppData)
    {
        // 只有App是OK时，才判断，防止锁定App异常退出后，其他App无法使用
        //if (IsWindow(it.hWndAgent))
        {
            if (it.bLocked)
            {
                return true;
            }
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
CXfsDataManager::CXfsDataManager(): m_pIWFM(nullptr)
{

}

CXfsDataManager::~CXfsDataManager()
{

}

void CXfsDataManager::SetIWFMShareMenory(IWFMShareMenory *pIWFM)
{
    AutoMutex(m_cMutex);
    m_pIWFM = pIWFM;
}

void CXfsDataManager::Add(const SPCMDDATA &stCmd)
{
    AutoMutex(m_cMutex);
    m_listCmd.push_back(stCmd);
}

void CXfsDataManager::Add(const SPRESULTDATA &stResult)
{
    AutoMutex(m_cMutex);
    m_listResult.push_back(stResult);
}

bool CXfsDataManager::Peek(SPCMDDATA &stCmd)
{
    AutoMutex(m_cMutex);
    if (m_listCmd.empty())
        return false;

    stCmd.copy(m_listCmd.front());
    m_listCmd.pop_front();
    return true;
}

bool CXfsDataManager::Peek(SPRESULTDATA &stResult)
{
    AutoMutex(m_cMutex);
    if (m_listResult.empty())
        return false;

    stResult = m_listResult.front();
    m_listResult.pop_front();
    return true;
}

void CXfsDataManager::ClearCmd()
{
    AutoMutex(m_cMutex);
    for (auto &it : m_listCmd)
    {
        if (it.lpCmdData != nullptr && m_pIWFM != nullptr)
        {
            m_pIWFM->WFMFreeBuffer(it.lpCmdData);
            it.lpCmdData = nullptr;
        }
    }

    m_listCmd.clear();
}

void CXfsDataManager::ClearResult()
{
    AutoMutex(m_cMutex);
    for (auto &it : m_listResult)
    {
        if (it.lpResult != nullptr && m_pIWFM != nullptr)
        {
            m_pIWFM->WFMFreeBuffer(it.lpResult);
            it.lpResult = nullptr;
        }
    }

    m_listResult.clear();
}


bool CXfsDataManager::IsResultEmpty()
{
    AutoMutex(m_cMutex);
    return m_listResult.empty();
}

void CXfsDataManager::AddGet(const SPCMDDATA &stCmd)
{
    AutoMutex(m_cCmdMutex);
    m_listGetCmd.push_back(stCmd);
}

void CXfsDataManager::AddExe(const SPCMDDATA &stCmd)
{
    AutoMutex(m_cCmdMutex);
    m_listExeCmd.push_back(stCmd);
}

bool CXfsDataManager::PeekGet(SPCMDDATA &stCmd)
{
    AutoMutex(m_cCmdMutex);
    if (m_listGetCmd.empty())
        return false;

    stCmd.copy(m_listGetCmd.front());
    m_listGetCmd.pop_front();
    return true;
}

bool CXfsDataManager::PeekExe(SPCMDDATA &stCmd)
{
    AutoMutex(m_cCmdMutex);
    if (m_listExeCmd.empty())
        return false;

    stCmd.copy(m_listExeCmd.front());
    m_listExeCmd.pop_front();
    return true;
}

bool CXfsDataManager::PeekExe(HSERVICE hService, SPCMDDATA &stCmd)
{
    AutoMutex(m_cCmdMutex);
    if (m_listExeCmd.empty())
        return false;

    CFindCMD cFind(hService);
    auto it = find_if(m_listExeCmd.begin(), m_listExeCmd.end(), cFind);
    if (it != m_listExeCmd.end())
    {
        stCmd.copy(*it);
        m_listExeCmd.erase(it);
        return true;
    }
    return false;
}

bool CXfsDataManager::PeekExe(HSERVICE hService, REQUESTID ReqID, SPCMDDATA &stCmd)
{
    AutoMutex(m_cCmdMutex);
    if (m_listExeCmd.empty())
        return false;

    CFindCMD cFind(hService, ReqID);
    auto it = find_if(m_listExeCmd.begin(), m_listExeCmd.end(), cFind);
    if (it != m_listExeCmd.end())
    {
        stCmd.copy(*it);
        m_listExeCmd.erase(it);
        return true;
    }
    return false;
}

bool CXfsDataManager::IsExeCmdEmpty()
{
    AutoMutex(m_cCmdMutex);
    return m_listExeCmd.empty();
}

void CXfsDataManager::ClearGetExeCmd()
{
    AutoMutex(m_cCmdMutex);
    for (auto &it : m_listGetCmd)
    {
        if (it.lpCmdData != nullptr && m_pIWFM != nullptr)
        {
            m_pIWFM->WFMFreeBuffer(it.lpCmdData);
            it.lpCmdData = nullptr;
        }
    }
    for (auto &it : m_listExeCmd)
    {
        if (it.lpCmdData != nullptr && m_pIWFM != nullptr)
        {
            m_pIWFM->WFMFreeBuffer(it.lpCmdData);
            it.lpCmdData = nullptr;
        }
    }

    m_listGetCmd.clear();
    m_listExeCmd.clear();
}


bool CXfsDataManager::RemoveCmd(HSERVICE hService, REQUESTID ReqID)
{
    // 只清执行Cmd
    AutoMutex(m_cCmdMutex);
    return RemoveCmd(m_listExeCmd, hService, ReqID);

    /*
        // 清三种Cmd
        bool bRemove = false;
        {
        AutoMutex(m_cMutex);
        if (RemoveCmd(m_listCmd, hService, ReqID))
            bRemove = true;
        }
        {
        AutoMutex(m_cCmdMutex);
        if (RemoveCmd(m_listGetCmd, hService, ReqID))
            bRemove = true;
        if (RemoveCmd(m_listExeCmd, hService, ReqID))
            bRemove = true;
        }
        return bRemove;
    */
}

bool CXfsDataManager::RemoveCmd(listSPCMDDATA &listCmd, HSERVICE hService, REQUESTID ReqID)
{
    // 清Cmd
    bool bRemove = false;
    if (listCmd.empty())
        return bRemove;

    if (ReqID == 0)   // 清除指定服务的全部命令
    {
        for (auto it = listCmd.begin(); it != listCmd.end();)
        {
            if (it->hService == hService)
            {
                if (it->lpCmdData != nullptr && m_pIWFM != nullptr)
                {
                    m_pIWFM->WFMFreeBuffer(it->lpCmdData);
                    it->lpCmdData = nullptr;
                }
                it = listCmd.erase(it);
                bRemove = true;
            }
            else
            {
                it++;
            }
        }
    }
    else
    {
        CFindCMD cFind(hService, ReqID);
        auto it = find_if(listCmd.begin(), listCmd.end(), cFind);
        if (it != listCmd.end())
        {
            if (it->lpCmdData != nullptr && m_pIWFM != nullptr)
            {
                m_pIWFM->WFMFreeBuffer(it->lpCmdData);
                it->lpCmdData = nullptr;
            }
            listCmd.erase(it);
            bRemove = true;
        }
    }

    return bRemove;
}

void CXfsDataManager::GetLocalTime(SYSTEMTIME &stTime)
{
    QDateTime qDT = QDateTime::currentDateTime();
    QTime qTime = qDT.time();
    QDate qDate = qDT.date();

    memset(&stTime, 0x00, sizeof(SYSTEMTIME));
    stTime.wYear = (WORD)qDate.year();
    stTime.wMonth = (WORD)qDate.month();
    stTime.wDay = (WORD)qDate.day();
    stTime.wDayOfWeek = (WORD)qDate.dayOfWeek();
    stTime.wHour = (WORD)qTime.hour();
    stTime.wMinute = (WORD)qTime.minute();
    stTime.wSecond = (WORD)qTime.second();
    stTime.wMilliseconds = (WORD)qTime.msec();
    return;
}

void CXfsDataManager::GetLocalTime(char *pTime)
{
    if (pTime == nullptr)
        return;

    QDateTime qDT = QDateTime::currentDateTime();
    QTime qTime = qDT.time();
    QDate qDate = qDT.date();
    sprintf(pTime, "%04d-%02d-%02d %02d:%02d:%02d.%03d", qDate.year(), qDate.month(), qDate.day(),
            qTime.hour(), qTime.minute(), qTime.second(), qTime.msec());
    return;
}

//////////////////////////////////////////////////////////////////////////

CAutoWFMFreeBuffer::CAutoWFMFreeBuffer(IWFMShareMenory *pIWFM, HRESULT *phRet) :
    m_phRet(phRet), m_pIWFM(pIWFM)
{

}

CAutoWFMFreeBuffer::~CAutoWFMFreeBuffer()
{
    if (m_phRet != nullptr && *m_phRet != WFS_SUCCESS)
    {
        for (auto &it : m_vtBuff)
        {
            if (it != nullptr && m_pIWFM != nullptr)
            {
                m_pIWFM->WFMFreeBuffer(it);
            }
        }
    }
    m_vtBuff.clear();
}

void CAutoWFMFreeBuffer::push_back(LPVOID lpVoid)
{
    m_vtBuff.push_back(lpVoid);
}
//////////////////////////////////////////////////////////////////////////

CAutoSIMFreeBuffer::CAutoSIMFreeBuffer(IWFMShareMenory *pIWFM, HRESULT *phRet) :
    m_phRet(phRet), m_pIWFM(pIWFM)
{

}

CAutoSIMFreeBuffer::~CAutoSIMFreeBuffer()
{
    if (m_phRet != nullptr && *m_phRet != WFS_SUCCESS)
    {
        for (auto &it : m_vtBuff)
        {
            if (it != nullptr && m_pIWFM != nullptr)
            {
                m_pIWFM->SIMFreeBuffer(it);
            }
        }
    }
    m_vtBuff.clear();
}

void CAutoSIMFreeBuffer::push_back(LPVOID lpVoid)
{
    m_vtBuff.push_back(lpVoid);
}
//////////////////////////////////////////////////////////////////////////
CSPMemoryRW::CSPMemoryRW(QString strKey, bool bCreate/* = false*/, int nSize/* = 10 * 1024*/):
    m_cMemRW(strKey + "_SemMutex")
{
    m_bIsOpened = m_cMemRW.Open(strKey, bCreate, nSize);
}

CSPMemoryRW::~CSPMemoryRW()
{
}

void CSPMemoryRW::Release()
{
    if (!m_bIsOpened)
        return;
    m_cMemRW.Release();
}

bool CSPMemoryRW::IsOpened()
{
    if (!m_bIsOpened)
        return false;
    return m_cMemRW.IsOpened();
}

int CSPMemoryRW::Size()
{
    if (!m_bIsOpened)
        return 0;
    return m_cMemRW.Size();
}

bool CSPMemoryRW::Write(const void *pBuff, int nBuffSize)
{
    if (!m_bIsOpened)
        return false;
    return m_cMemRW.Write(pBuff, nBuffSize);
}

bool CSPMemoryRW::Read(void *pBuff, int nBuffSize)
{
    if (!m_bIsOpened)
        return false;
    return m_cMemRW.Read(pBuff, nBuffSize);
}

//////////////////////////////////////////////////////////////////////////
