#include "SPBaseClass.h"

static const char *ThisFile = "SPBaseClass.cpp";
//////////////////////////////////////////////////////////////////////////
extern "C" SPBASE_EXPORT long CreateISPBaseClass(LPCSTR lpDevType, ISPBaseClass *&p)
{
    p = new CSPBaseClass(lpDevType);
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CSPBaseClass::CSPBaseClass(LPCSTR lpLogType) : m_pCmdRun(nullptr), m_cXFSMutex("XFS_SP_Mutex"), m_pSpRMem(nullptr)
{
    m_bExecuting = false;
    m_bQuitStartRun = false;
    m_bOpened = false;
    m_bUpdateDevPDLing = true;
    memset(m_szLogicalName, 0x00, sizeof(m_szLogicalName));
    memset(m_szSPName, 0x00, sizeof(m_szSPName));
    memset(m_szExeName, 0x00, sizeof(m_szExeName));
    memset(m_szCurProcessID, 0x00, sizeof(m_szCurProcessID));
    SetLogFile(LOGFILE, ThisFile, lpLogType);

    m_bIsParallelExecCrypt = false;             //30-00-00-00(FS#0002)
    m_bIsExecGetData = false;           //30-00-00-00(FS#0002)
}

CSPBaseClass::~CSPBaseClass() {}

void CSPBaseClass::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_bQuitStartRun = true;
    return;
}

void CSPBaseClass::RegisterICmdRun(ICmdRun *pICmdRun)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_pCmdRun = pICmdRun;
    return;
}

bool CSPBaseClass::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_bQuitStartRun = false;

    // 设置工作目录
    QDir::setCurrent(GetXfsPath());
    // 获取命令行参数
    QString strCmd;
    QStringList arguments = QCoreApplication::arguments();
    for (int i = 1; i < arguments.size(); i++)
    {
        strCmd += arguments[i];
        strCmd += " ";
    }
    // if (strCmd.isEmpty())// 测试
    //    strCmd = "XFS_IDC IDC30 1152 Debug";

    // 命令行，格式：SPName + LogicalName + 进程ID
    std::string strCmdLine = strCmd.toStdString();
    bool bDebug = (std::string::npos != strCmdLine.find("Debug")) ? true : false;
    CAutoSplitByStep cSplit(strCmdLine.c_str(), " ");
    if ((!bDebug && cSplit.Count() != 3) || (bDebug && cSplit.Count() != 4))
    {
        Log(ThisModule, __LINE__, "解析启动命令行失败：%s", strCmdLine.c_str());
        return false;
    }

    m_cXfsReg.SetLogicalName(cSplit.At(1));
    strcpy(m_szExeName, cSplit.At(0));
    strcpy(m_szLogicalName, cSplit.At(1));
    strcpy(m_szCurProcessID, cSplit.At(2));
    strcpy(m_szSPName, m_cXfsReg.GetSPName());

    // 只有唯一实例
    if (m_cAppRun.IsSingleAppRunning(m_szExeName))
    {
        Log(ThisModule, __LINE__, "已启动唯一SP实例：%s", m_szExeName);
        return false;
    }

    // 同名的通知事件
    char szEventName[128] = {0};
    sprintf(szEventName, SPREADYEVENTNAMEFORMAT, m_szLogicalName, m_szCurProcessID);
    CAutoEventEx cNotiyEvent(szEventName);

    // 创建SP通信类
    QString strRMem = m_szExeName + QString(m_szSPName) + "SP";
    m_pSpRMem = new CSPMemoryRW(strRMem, true);
    if (m_pSpRMem == nullptr || !m_pSpRMem->IsOpened())
    {
        Log(ThisModule, __LINE__, "创建SP通信类失败");
        return false;
    }

    // 加载共享内存类
    if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
    {
        Log(ThisModule, __LINE__, "加载库失败: WFMShareMenory.dll");
        return false;
    }
    m_cXfsData.SetIWFMShareMenory(m_pIWFM);
    m_cEventData.SetIWFMShareMenory(m_pIWFM);

    Log(ThisModule, 1, "启动SP成功：SPName = %s", m_szSPName);

    // 创建工作线程
    StartThread();

    // 通知启动完成
    cNotiyEvent.SetEvent();

    // 进入消息循环
    Log(ThisModule, 1, "SP程序进入消息循环：%s", m_szSPName);
    WORD wErrTimes = 0;
    while (true)
    {
        QCoreApplication::processEvents();	// cam要用到事件循环,需要保持exec()随时响应

        // 延时，防止占用CPU
        // std::this_thread::sleep_for(std::chrono::milliseconds(200));
		std::this_thread::sleep_for(std::chrono::milliseconds(20));	// cam摄像刷新保持流畅,刷新实际必须小于30毫秒

        // 调用Close退出
        if (m_bQuitStartRun)
            break;

        // 没有调用Close或异常退出
        if (!bDebug && !m_cXfsApp.IsEmpty())
        {
            AutoMutex(m_cXFSMutex);
            if (m_cXfsApp.IsAllAgentInvalid())
                wErrTimes++;
            else
                wErrTimes = 0;
            if (wErrTimes >= 3)
            {
                Log(ThisModule, __LINE__, "SP[%s]对应的Agent或App已全部退出:wErrTimes=%d", m_szSPName, wErrTimes);
                break;
            }
        }
    }
    Log(ThisModule, __LINE__, "SP程序退出消息循环：%s", m_szSPName);

    // 做一次取消操作
    if (m_pCmdRun)
        m_pCmdRun->OnCancelAsyncRequest();

    // 延时1秒，让Close结果尽可能返回
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // 退出读线程
    m_pSpRMem->Release();

    // 等待工作线程结束
    StopThread();
    return true;
}

void CSPBaseClass::GetSPBaseData(SPBASEDATA &stData)
{
    stData.pMutex = &m_cStatusMutexGet;
    strcpy(stData.szLogicalName, m_szLogicalName);
}

bool CSPBaseClass::FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    bool bFire = false;
    SPRESULTDATA stResult;
    listAPPSERVICEDATA listApp;
    m_cXfsApp.GetApp(listApp);
    for (auto &it : listApp)
    {
        // 判断是不有此事件和注册窗口是否正常
        if (!IsRegisterMsgID(it.dwEventID, uMsgID))
            continue;

        // 特殊CRS要区分事件
        if (!IsCRSEventClassMatch(dwEventID, it))
            continue;

        // 特殊SIU要区分事件
        if (IsSIUEventClassMatch(dwEventID, it, (LPWFSSIUPORTEVENT)lpData))
            continue;

        // 申请结果内存
        LPWFSRESULT lpResult = nullptr;
        HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSRESULT), WFS_MEM_FLAG, (LPVOID *)&lpResult);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请结果内存失败:hRet=%d[hService=%d,SPName=%s]", hRet, it.hService, it.szSPName);
            continue;
        }
        memset(lpResult, 0x00, sizeof(WFSRESULT));
        // 针对两个特殊的状态事件，赋值逻辑和SP名
        if (uMsgID == WFS_SYSTEM_EVENT && dwEventID == WFS_SYSE_DEVICE_STATUS)
        {
            auto lpStatus = static_cast<LPWFSDEVSTATUS>(lpData);
            if (lpStatus != nullptr)
            {
                lpStatus->lpszPhysicalName = it.szSPName;
            }
        }
        else if (uMsgID == WFS_SYSTEM_EVENT && dwEventID == WFS_SYSE_HARDWARE_ERROR)
        {
            auto lpStatus = static_cast<LPWFSHWERROR>(lpData);
            if (lpStatus != nullptr)
            {
                lpStatus->lpszLogicalName = it.szLogicalName;
                lpStatus->lpszPhysicalName = it.szSPName;
            }
        }

        // 调用对应设备Base赋值
        hRet = m_pCmdRun->FireEvent(uMsgID, dwEventID, lpData, lpResult);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "FireEvent赋值失败:hRet=%d[uMsgID=%d,dwEventID=%d,hService=%d,SPName=%s]", hRet, uMsgID, dwEventID, it.hService,
                it.szSPName);
            m_pIWFM->WFMFreeBuffer(lpResult);
            continue;
        }

        // 赋值
        lpResult->hService = it.hService;
        lpResult->u.dwEventID = dwEventID;
        m_cXfsData.GetLocalTime(lpResult->tsTimestamp);

        // 更新
        stResult.clear();
        stResult.hService = it.hService;
        stResult.uMsgID = uMsgID;
        stResult.lpResult = lpResult;
        strcpy(stResult.szAgentName, it.szAgentName);
        strcpy(stResult.szAppName, it.szAppName);
        strcpy(stResult.szAppID, it.szAppID);
        lpResult = nullptr;  // 此防止释放

        bFire = true;
        m_cXfsData.Add(stResult);
    }
    return bFire;
}

HRESULT CSPBaseClass::Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSDEVSTATUS>(lpData);
        if (lpStatus == nullptr)
        {
            Log(ThisModule, __LINE__, "数据指针为空");
            break;
        }
        LPWFSDEVSTATUS lpNewStatus = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSDEVSTATUS), lpResult, (LPVOID *)&lpNewStatus);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpNewStatus);
        memset(lpNewStatus, 0x00, sizeof(WFSDEVSTATUS));
        lpNewStatus->dwState = lpStatus->dwState;

        LPSTR lpBuff = nullptr;
        ULONG ulSize = sizeof(char) * 256;
        hRet = m_pIWFM->WFMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszPhysicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->WFMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszWorkstationName = lpBuff;
        lpBuff = nullptr;

        // 判断是否有数据
        if (lpStatus->lpszPhysicalName != nullptr)
            strcpy(lpNewStatus->lpszPhysicalName, lpStatus->lpszPhysicalName);
        if (lpStatus->lpszWorkstationName != nullptr)
            strcpy(lpNewStatus->lpszWorkstationName, lpStatus->lpszWorkstationName);

        // 赋值
        lpResult->lpBuffer = lpNewStatus;
        lpNewStatus = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseClass::Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = WFS_ERR_INTERNAL_ERROR;
    CAutoWFMFreeBuffer _auto(m_pIWFM, &hRet);
    do
    {
        auto lpStatus = static_cast<LPWFSHWERROR>(lpData);
        if (lpStatus == nullptr)
        {
            Log(ThisModule, __LINE__, "数据指针为空");
            break;
        }
        LPWFSHWERROR lpNewStatus = nullptr;
        hRet = m_pIWFM->WFMAllocateMore(sizeof(WFSHWERROR), lpResult, (LPVOID *)&lpNewStatus);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpNewStatus);
        memset(lpNewStatus, 0x00, sizeof(WFSHWERROR));
        lpNewStatus->dwAction = lpStatus->dwAction;

        LPSTR lpBuff = nullptr;
        ULONG ulSize = sizeof(char) * 256;
        hRet = m_pIWFM->WFMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszLogicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->WFMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszPhysicalName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->WFMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszWorkstationName = lpBuff;
        lpBuff = nullptr;

        hRet = m_pIWFM->WFMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
            break;
        }
        _auto.push_back(lpBuff);
        memset(lpBuff, 0x00, ulSize);
        lpNewStatus->lpszAppID = lpBuff;
        lpBuff = nullptr;

        if (lpStatus->dwSize > 0)
        {
            // lpbDescription是“ErrorDetail = 00XXXXXXX\x0\x0” (修正了14个字符+错误代码7个字符+空结束2个字符)
            ulSize = lpStatus->dwSize + 2;  // 此特殊处理：多加两位
            hRet = m_pIWFM->WFMAllocateMore(ulSize, lpResult, (LPVOID *)&lpBuff);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
                break;
            }
            _auto.push_back(lpBuff);
            memset(lpBuff, 0x00, ulSize);
            memcpy(lpBuff, lpStatus->lpbDescription, lpStatus->dwSize);
            lpNewStatus->dwSize = ulSize;
            lpNewStatus->lpbDescription = (LPBYTE)lpBuff;
            lpBuff = nullptr;
        }

        // 判断是否有数据
        if (lpStatus->lpszLogicalName != nullptr)
            strcpy(lpNewStatus->lpszLogicalName, lpStatus->lpszLogicalName);
        if (lpStatus->lpszPhysicalName != nullptr)
            strcpy(lpNewStatus->lpszPhysicalName, lpStatus->lpszPhysicalName);
        if (lpStatus->lpszWorkstationName != nullptr)
            strcpy(lpNewStatus->lpszWorkstationName, lpStatus->lpszWorkstationName);
        if (lpStatus->lpszAppID != nullptr)
            strcpy(lpNewStatus->lpszAppID, lpStatus->lpszAppID);

        // 赋值
        lpResult->lpBuffer = lpNewStatus;
        lpNewStatus = nullptr;
    } while (false);
    return hRet;
}

HRESULT CSPBaseClass::Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    UINT uLen = GetLenOfSZZ(lpszOldExtra);
    if (uLen == 0)
    {
        Log(ThisModule, __LINE__, "lpszOldExtra格式错误");
        return WFS_ERR_INVALID_DATA;
    }

    HRESULT hRet = m_pIWFM->WFMAllocateMore(sizeof(char) * uLen, lpResult, (LPVOID *)&lpszNewExtra);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "申请数据内存失败:hRet=%d", hRet);
        return hRet;
    }

    memcpy(lpszNewExtra, lpszOldExtra, sizeof(char) * uLen);
    return WFS_SUCCESS;
}

void CSPBaseClass::OnOpen(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = 0;

    do
    {
        // 检测参数
        hRet = CheckParameter(stCmd);
        if (hRet != WFS_SUCCESS)
            break;

        // 检测是否已启动服务
        if (IsHasService(stCmd))
        {
            // 保存服务
            SaveServiceData(stCmd);
            break;
        }

        // 只有第一个服务，才初始化SP
        hRet = m_pCmdRun->OnOpen(stCmd.szLogicalName);
        if (hRet != WFS_SUCCESS)
        {
            m_pCmdRun->OnClose();
            break;
        }

        // 保存服务
        SaveServiceData(stCmd);
    } while (false);

    // 更新结果
    m_bOpened = true;
    stResult.uMsgID = WFS_OPEN_COMPLETE;
    stResult.lpResult->hResult = hRet;
    m_cXfsData.Add(stResult);
    return;
}

void CSPBaseClass::OnClose(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = 0;
    do
    {
        // 检测参数
        hRet = CheckParameter(stCmd);
        if (hRet != WFS_SUCCESS)
            break;

        // 删除服务
        if (m_cXfsApp.Remove(stCmd.hService))
        {
            CancelServerCMD(stCmd.hService);
        }

        // 全部服务关闭后，才关闭SP
        if (m_cXfsApp.IsEmpty())
        {
            m_pCmdRun->OnCancelAsyncRequest();
            hRet = m_pCmdRun->OnClose();
            if (hRet != WFS_SUCCESS)
                break;
        }

    } while (false);

    // 更新结果
    stResult.uMsgID = WFS_CLOSE_COMPLETE;
    stResult.lpResult->hResult = hRet;
    m_cXfsData.Add(stResult);

    // 如果已空服务，则自动退出
    if (m_cXfsApp.IsEmpty())
    {
        m_bQuitStartRun = true;
        Log(ThisModule, __LINE__, "SP[%s]对应的App已全部退出", m_szSPName);
    }
    return;
}

void CSPBaseClass::OnRegister(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = 0;
    do
    {
        // 检测参数
        hRet = CheckParameter(stCmd);
        if (hRet != WFS_SUCCESS)
            break;

        // 更新注册事件ID
        if (!m_cXfsApp.UpdateRegister(stCmd))
        {
            Log(ThisModule, __LINE__, "没找到对应的服务：hService=%d", stCmd.hService);
            hRet = WFS_ERR_SERVICE_NOT_FOUND;
            break;
        }

    } while (false);

    // 更新结果
    stResult.uMsgID = WFS_REGISTER_COMPLETE;
    stResult.lpResult->hResult = hRet;
    m_cXfsData.Add(stResult);
    return;
}

void CSPBaseClass::OnDeregister(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = 0;
    do
    {
        // 检测参数
        hRet = CheckParameter(stCmd);
        if (hRet != WFS_SUCCESS)
            break;

        // 更新注册事件ID
        if (!m_cXfsApp.UpdateDeRegister(stCmd))
        {
            Log(ThisModule, __LINE__, "没找到对应的服务：hService=%d", stCmd.hService);
            hRet = WFS_ERR_SERVICE_NOT_FOUND;
            break;
        }

    } while (false);

    // 更新结果
    stResult.uMsgID = WFS_DEREGISTER_COMPLETE;                      //30-00-00-00(FT#0062)
    stResult.lpResult->hResult = hRet;
    m_cXfsData.Add(stResult);
    return;
}

void CSPBaseClass::OnLock(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = 0;
    do
    {
        // 检测参数
        hRet = CheckParameter(stCmd);
        if (hRet != WFS_SUCCESS)
            break;

        // 检测是否已被锁定
        if (!m_cXfsApp.IsAppLock(stCmd.hService))
        {
            if (m_cXfsApp.IsAnyLocked())
            {
                Log(ThisModule, __LINE__, "SP服务[%s]已被其中一个App锁定", m_szSPName);
                hRet = WFS_ERR_LOCKED;
                break;
            }
        }

        // 更新锁状态
        if (!m_cXfsApp.AppLock(stCmd.hService))
        {
            Log(ThisModule, __LINE__, "没找到对应的服务：hService=%d", stCmd.hService);
            hRet = WFS_ERR_SERVICE_NOT_FOUND;
            break;
        }

        Log(ThisModule, __LINE__, "App[hService=%d]锁定SP服务[%s]", m_szSPName, stCmd.hService);
    } while (false);

    // 更新结果
    stResult.uMsgID = WFS_LOCK_COMPLETE;
    stResult.lpResult->hResult = hRet;
    m_cXfsData.Add(stResult);
    return;
}

void CSPBaseClass::OnUnlock(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    HRESULT hRet = 0;
    do
    {
        // 检测参数
        hRet = CheckParameter(stCmd);
        if (hRet != WFS_SUCCESS)
            break;

        // 检测是否有锁定
        if (!m_cXfsApp.IsAnyLocked())
        {
            Log(ThisModule, __LINE__, "SP服务没有被锁定");
            hRet = WFS_ERR_NOT_LOCKED;
            break;
        }

        if (!m_cXfsApp.IsAppLock(stCmd.hService))
        {
            Log(ThisModule, __LINE__, "SP服务[%s]已被其中一个App锁定", m_szSPName);
            hRet = WFS_ERR_LOCKED;
            break;
        }

        // 更新锁状态
        if (!m_cXfsApp.AppUnLock(stCmd.hService))
        {
            Log(ThisModule, __LINE__, "没找到对应的服务：hService=%d", stCmd.hService);
            hRet = WFS_ERR_SERVICE_NOT_FOUND;
            break;
        }

        Log(ThisModule, __LINE__, "App[hService=%d]解锁定SP服务[%s]", m_szSPName, stCmd.hService);
    } while (false);

    // 更新结果
    stResult.uMsgID = WFS_UNLOCK_COMPLETE;
    stResult.lpResult->hResult = hRet;
    m_cXfsData.Add(stResult);
    return;
}

void CSPBaseClass::OnGetInfo(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;
    do
    {
        // 查询状态，也要互斥
        AutoMutex(m_cStatusMutexGet);

        // 检测参数
        hRet = CheckParameter(stCmd);
        if (hRet != WFS_SUCCESS)
            break;

        //用于区分不同的逻辑名,但是SP相同且SP类型相同的情况
        strcpy(m_szLogicalName, stCmd.szLogicalName);
        // 获取信息
        hRet = m_pCmdRun->GetInfo(stCmd.dwCommand, stCmd.lpCmdData, stResult.lpResult);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "查询[dwCategory=%u, SPName=%s]失败：hRet = %d", stCmd.dwCommand, stCmd.szSPName, hRet);
            break;
        }

    } while (false);

    // 更新结果
    stResult.uMsgID = WFS_GETINFO_COMPLETE;
    stResult.lpResult->hResult = hRet;
    m_cXfsData.Add(stResult);
    return;
}

void CSPBaseClass::OnExecute(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
//30-00-00-00(FT#0070)    CAutoSetExecuteStatus _auto_status(&m_bExecuting);

    HRESULT hRet = 0;
    do
    {
        // 执行动作和查状态，也要互斥查状态
        AutoMutex(m_cStatusMutexExe);

        // 检测参数
        hRet = CheckParameter(stCmd);
        if (hRet != WFS_SUCCESS)
            break;

        // 用于区分不同的逻辑名,但是SP相同且SP类型相同的情况
        strcpy(m_szLogicalName, stCmd.szLogicalName);

        // 判断是否在做PDL
        if (IsUpdateDevPDL())
        {
            hRet = WFS_ERR_DEV_NOT_READY;
            Log(ThisModule, __LINE__, "设备正在执行PDL，不执行动作类命令，返回失败：hRet = %d", stCmd.dwCommand, stCmd.szSPName, hRet);
            break;
        }
        // 执行操作
        hRet = m_pCmdRun->Execute(stCmd.dwCommand, stCmd.lpCmdData, stCmd.dwTimeOut, stResult.lpResult);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "执行[dwCommand=%u, SPName=%s]失败：hRet = %d", stCmd.dwCommand, stCmd.szSPName, hRet);
            break;
        }

        // 针对SIU的事件命令特殊处理
        if (m_cSIUSetEvent.IsSetEventCmd(stCmd))
            m_cSIUSetEvent.Add(stCmd);

    } while (false);

    // 执行完成后，更新一次状态
    if (!IsUpdateDevPDL())
        OnStatus();

    // 更新结果
    stResult.uMsgID = WFS_EXECUTE_COMPLETE;
    stResult.lpResult->hResult = hRet;
    m_cXfsData.Add(stResult);
    return;
}

void CSPBaseClass::OnCancelAsyncRequest(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    do
    {
        // 先清空
        FreeBuffer(stResult);

        // 检测参数
        if (!stCmd.hService)
        {
            Log(ThisModule, __LINE__, "无效服务句柄");
            break;
        }
        if (!m_pCmdRun)
        {
            Log(ThisModule, __LINE__, "无效命令数据执行指针:m_pCmdRun=NULL");
            break;
        }

        // 执行取消，分指定取消和全部取消
        SPCMDDATA stOldCmd;
        if (stCmd.ReqID == 0)  // 全部取消
        {
            while (true)
            {
                // 检测是否有新命令
                if (!m_cXfsData.PeekExe(stOldCmd))
                    break;

                // 如果申请结果内存失败，则SP无返回，App只能自己超时
                stResult.clear();
                HRESULT hRet = GetResultBuff(stOldCmd, stResult);
                if (WFS_SUCCESS != hRet)
                {
                    Log(ThisModule, __LINE__, "获取结果内存失败:hRet=%d, eCmdID=%d", hRet, stOldCmd.eCmdID);
                    FreeBuffer(stOldCmd);
                    continue;
                }

                // 更新结果
                stResult.uMsgID = WFS_EXECUTE_COMPLETE;
                stResult.lpResult->hResult = WFS_ERR_CANCELED;
                m_cXfsData.Add(stResult);
            }

            if(IsExecuting()){
                m_pCmdRun->OnCancelAsyncRequest();
            }
        }
        else
        {
            // 检测是否有新命令
            if (m_cXfsData.PeekExe(stCmd.hService, stCmd.ReqID, stOldCmd))
            {
                // 如果申请结果内存失败，则SP无返回，App只能自己超时
                stResult.clear();
                HRESULT hRet = GetResultBuff(stOldCmd, stResult);
                if (WFS_SUCCESS != hRet)
                {
                    Log(ThisModule, __LINE__, "获取结果内存失败:hRet=%d, eCmdID=%d", hRet, stOldCmd.eCmdID);
                    FreeBuffer(stOldCmd);
                    break;
                }

                // 更新结果
                stResult.uMsgID = WFS_EXECUTE_COMPLETE;
                stResult.lpResult->hResult = WFS_ERR_CANCELED;
                m_cXfsData.Add(stResult);
            }
            else
            {
                // 是当前命令，因测试工具和使用原因，此不用再判断是否为当前ID
                // if (stCmd.ReqID == m_ExecutingReqID)
                if(IsExecuting())
                {
                    m_pCmdRun->OnCancelAsyncRequest();
                    break;
                }
            }
        }
    } while (false);

    stResult.clear();
    return;
}

void CSPBaseClass::OnSetTraceLevel(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    // 清结果内存
    stResult.hService = stCmd.hService;
    FreeBuffer(stResult);
    return;
}

void CSPBaseClass::OnUnloadService(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    // 清结果内存
    stResult.hService = stCmd.hService;
    FreeBuffer(stResult);
    return;
}

void CSPBaseClass::OnStatus()
{
    // THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cStatusMutexExe);
    if (m_pCmdRun != nullptr)
        m_pCmdRun->OnStatus();
    return;
}

HRESULT CSPBaseClass::OnWaitTaken()
{
    // THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cStatusMutexExe);
    if (m_pCmdRun != nullptr)
        return m_pCmdRun->OnWaitTaken();
    return WFS_ERR_CANCELED;
}

void CSPBaseClass::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CAutoSetExecuteStatus _auto_status(&m_bUpdateDevPDLing);
    if (m_pCmdRun != nullptr)
        m_pCmdRun->OnUpdateDevPDL();
    return;
}

//////////////////////////////////////////////////////////////////////////
void CSPBaseClass::Run_Read()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        SPCMDDATA stCmd;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (m_bQuitStartRun)
                break;

            if (!m_pSpRMem->Read(&stCmd, sizeof(stCmd)))
                continue;

            // 保存新命令
            m_cXfsData.Add(stCmd);
            stCmd.clear();
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_Read线程异常");
    }
}
void CSPBaseClass::Run_Cmd()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        SPCMDDATA stCmd;
        SPRESULTDATA stResult;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // 检测是否有新命令
            if (!m_cXfsData.Peek(stCmd))
                continue;

            // 此两种命令，不用获取结果内存
            if (stCmd.eCmdID != WFS_GETINFO && stCmd.eCmdID != WFS_EXECUTE)
            {
                stResult.clear();
                HRESULT hRet = GetResultBuff(stCmd, stResult);
                if (WFS_SUCCESS != hRet)
                {
                    Log(ThisModule, __LINE__, "获取结果内存失败:hRet=%d, eCmdID=%d", hRet, stCmd.eCmdID);
                    FreeBuffer(stCmd);
                    continue;
                }
            }

            // 执行
            switch (stCmd.eCmdID)
            {
            case WFS_OPEN:
                OnOpen(stCmd, stResult);
                break;
            case WFS_CLOSE:
                OnClose(stCmd, stResult);
                break;
            case WFS_LOCK:
                OnLock(stCmd, stResult);
                break;
            case WFS_UNLOCK:
                OnUnlock(stCmd, stResult);
                break;
            case WFS_REGISTER:
                OnRegister(stCmd, stResult);
                break;
            case WFS_DEREGISTER:
                OnDeregister(stCmd, stResult);
                break;
            case WFS_CANCELREQ:
                OnCancelAsyncRequest(stCmd, stResult);
                break;
            case WFS_SETTRACELEVEL:
                OnSetTraceLevel(stCmd, stResult);
                break;
            case WFS_GETINFO:
                m_cXfsData.AddGet(stCmd);
                break;
            case WFS_EXECUTE:
                AddExeCmd(stCmd);
                break;
            default:
                break;
            }

            // 此两种命令，在执行线程中释放
            if (stCmd.eCmdID != WFS_GETINFO && stCmd.eCmdID != WFS_EXECUTE)
            {
                FreeBuffer(stCmd);
            }
            stResult.clear();
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_Cmd线程异常");
    }
}

void CSPBaseClass::Run_GetInfo()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        SPCMDDATA stCmd;
        SPRESULTDATA stResult;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // 检测是否有新命令
            if (!m_cXfsData.PeekGet(stCmd))
                continue;

            // 获取结果内存
            stResult.clear();
            HRESULT hRet = GetResultBuff(stCmd, stResult);
            if (WFS_SUCCESS != hRet)
            {
                Log(ThisModule, __LINE__, "获取结果内存失败:hRet=%d, eCmdID=%d", hRet, stCmd.eCmdID);
                FreeBuffer(stCmd);
                continue;
            }

            // 执行
            if (stCmd.eCmdID == WFS_GETINFO)
                OnGetInfo(stCmd, stResult);

            // 清缓存
            FreeBuffer(stCmd);
            stResult.clear();
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_GetInfo线程异常");
    }
}

void CSPBaseClass::Run_Execute()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        SPCMDDATA stCmd;
        SPRESULTDATA stResult;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(m_bIsParallelExecCrypt){                                 //30-00-00-00(FS#0002)
                continue;                                       //30-00-00-00(FS#0002)
            }                                                   //30-00-00-00(FS#0002)

            // 检测结果队列是否为空，防止事件错乱
            if (!m_cEventData.IsResultEmpty() || !m_cXfsData.IsResultEmpty())
                continue;

            // 检测是否有新命令
            if (!m_cXfsData.PeekExe(stCmd))
                continue;

            m_bExecuting = true;                                //30-00-00-00(FT#0070)
            // 获取结果内存
            stResult.clear();
            HRESULT hRet = GetResultBuff(stCmd, stResult);
            if (WFS_SUCCESS != hRet)
            {
                Log(ThisModule, __LINE__, "获取结果内存失败:hRet=%d, eCmdID=%d", hRet, stCmd.eCmdID);
                FreeBuffer(stCmd);
                m_bExecuting = false;                         //30-00-00-00(FT#0070)
                m_pCmdRun->OnClearCancelSemphoreCount();      //30-00-00-00(FT#0070)
                continue;
            }

            if(stCmd.dwCommand == WFS_CMD_PIN_GET_DATA){                //30-00-00-00(FS#0002)
                m_bIsExecGetData = true;                                //30-00-00-00(FS#0002)
            }                                                           //30-00-00-00(FS#0002)
            // 执行
            if (stCmd.eCmdID == WFS_EXECUTE){                            //30-00-00-00(FT#0070)
                OnExecute(stCmd, stResult);
                m_bExecuting = false;                                    //30-00-00-00(FT#0070)
                //清除当前命令Cancel标志
                m_pCmdRun->OnClearCancelSemphoreCount();                 //30-00-00-00(FT#0070)
            }                                                            //30-00-00-00(FT#0070)
            // 清缓存
            FreeBuffer(stCmd);
            stResult.clear();

            if(stCmd.dwCommand == WFS_CMD_PIN_GET_DATA){                //30-00-00-00(FS#0002)
                m_bIsExecGetData = false;                               //30-00-00-00(FS#0002)
            }                                                           //30-00-00-00(FS#0002)
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_Execute线程异常");
    }
}

void CSPBaseClass::Run_Result()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        SPRESULTDATA stResult;
        CSPMemoryRWHelper cMemRW;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // 空闲时，再把事件结果放到结果队列中
            if (!IsExecuting() && !m_cEventData.IsResultEmpty() && m_cXfsData.IsResultEmpty())
            {
                if (m_cEventData.Peek(stResult))
                    m_cXfsData.Add(stResult);
            }

            // 检测是否有新结果
            if (!m_cXfsData.Peek(stResult))
                continue;

            // 普通事件延后处理，执行中的事件，正常发送
            if (stResult.uMsgID == WFS_SERVICE_EVENT || stResult.uMsgID == WFS_USER_EVENT || stResult.uMsgID == WFS_SYSTEM_EVENT)
            {
                if (stResult.lpResult != nullptr && (stResult.lpResult->u.dwEventID == 102 || stResult.lpResult->u.dwEventID == 106))
                {
                    //同步发送指纹的Insert和Taken事件
                }
                else if (IsExecuting() || !m_cXfsData.IsResultEmpty())
                {
                    m_cEventData.Add(stResult);
                    continue;
                }
            }

            // 发送命令结果到Agent窗口
            if (!m_cAppRun.IsAppRunning(stResult.szAppName, stResult.szAppID))
            {
                Log(ThisModule, __LINE__, "无效Agent窗口[%s][szAppName=%s,szAppID=%s],uMsgID = %d", stResult.szAgentName, stResult.szAppName,
                    stResult.szAppID, stResult.uMsgID);
                FreeBuffer(stResult);
                continue;
            }

            CSPMemoryRW *pSpWMem = cMemRW.GetMem(stResult.szAgentName);
            if (pSpWMem == nullptr || !pSpWMem->IsOpened())
            {
                Log(ThisModule, __LINE__, "连接Agent窗口失败");
                FreeBuffer(stResult);
                continue;
            }

            if (!pSpWMem->Write(&stResult, sizeof(stResult)))
            {
                Log(ThisModule, __LINE__, "发送数据到Agent=%s失败:uMsgID=%s", stResult.szAgentName, ConvertMSGID(stResult.uMsgID));
                FreeBuffer(stResult);
                continue;
            }
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_Result线程异常");
    }
}

void CSPBaseClass::Run_Status()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        DWORD dwUpdateStatusTime = stoul(m_cXfsReg.GetValue("CONFIG", "UpdateStatusTime", "2000"));
        if (dwUpdateStatusTime < 200)
            dwUpdateStatusTime = 200;

        DWORD dwWaitTakenTime = stoul(m_cXfsReg.GetValue("CONFIG", "WaitTakenTime", "200"));
        if (dwWaitTakenTime < 200)
            dwWaitTakenTime = 200;

        bool bUpdatePDL = false;
        bool bUpdateStatus = false;
        bool bWaitTaken = false;
        QTime qUTime = QTime::currentTime();
        QTime qWTime = QTime::currentTime();
        while (true)
        {
            // 等待OnOpen结束
            if (!m_bOpened)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                if (m_bQuitRun)
                    break;
                continue;
            }
            // 调用一次升级固件
            if (!bUpdatePDL)
            {
                bUpdatePDL = true;
                OnUpdateDevPDL();
            }

            // 按配置间隔更新一次状态
            if (bUpdateStatus)
            {
                bUpdateStatus = false;
                qUTime = QTime::currentTime();
                qUTime.start();
            }
            if (bWaitTaken)
            {
                bWaitTaken = false;
                qWTime = QTime::currentTime();
                qWTime.start();
            }
            while (!m_bQuitRun)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                if (qUTime.elapsed() > (int)dwUpdateStatusTime)
                    bUpdateStatus = true;
                if (qWTime.elapsed() > (int)dwWaitTakenTime)
                    bWaitTaken = true;
                if (bUpdateStatus || bWaitTaken)
                    break;
            }
            // 延时后判断，提高退出速度和减少一次查状态
            if (m_bQuitRun)
                break;

            // 检测是否已App服务
            if (m_cXfsApp.IsEmpty())
                continue;

            // 检测是否有命令
            if (!m_cXfsData.IsExeCmdEmpty())
                continue;

            // 检测是否有命令在执行
            if (IsExecuting())
                continue;

            // 没有执行命令时，检测状态
            if (bWaitTaken)
            {
                HRESULT hRet = OnWaitTaken();
                if (WFS_ERR_CANCELED != hRet)  // 和OnStatus互斥调用
                {
                    continue;
                }
            }
            if (bUpdateStatus)
            {
                OnStatus();
            }
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_Status线程异常");
    }
}

//30-00-00-00(FS#0002) add start
void CSPBaseClass::Run_ParallelExecute()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    try
    {
        SPCMDDATA stCmd;
        SPRESULTDATA stResult;
        while (!m_bQuitRun)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            //检查是否WFS_CMD_PIN_GET_DATA命令正在执行
            if(!m_bIsExecGetData){
                continue;
            }

            // 检测结果队列是否为空，防止事件错乱
            if (!m_cEventData.IsResultEmpty() || !m_cXfsData.IsResultEmpty())
                continue;

            // 检测是否有Crypt命令
            if (!m_cXfsData.PeekExe((DWORD)WFS_CMD_PIN_CRYPT, stCmd))
                continue;

            m_bIsParallelExecCrypt = true;
            // 获取结果内存
            stResult.clear();
            HRESULT hRet = GetResultBuff(stCmd, stResult);
            if (WFS_SUCCESS != hRet)
            {
                Log(ThisModule, __LINE__, "获取结果内存失败:hRet=%d, eCmdID=%d, dwCommand:%u", hRet, stCmd.eCmdID, stCmd.dwCommand);
                FreeBuffer(stCmd);
                m_bIsParallelExecCrypt = false;
                continue;
            }

            // 执行
            do
            {
                // 检测参数
                hRet = CheckParameter(stCmd);
                if (hRet != WFS_SUCCESS)
                    break;

                // 判断是否在做PDL
                if (IsUpdateDevPDL())
                {
                    hRet = WFS_ERR_DEV_NOT_READY;
                    Log(ThisModule, __LINE__, "设备正在执行PDL，不执行动作类命令，返回失败：hRet = %d", stCmd.dwCommand, stCmd.szSPName, hRet);
                    break;
                }
                // 执行操作
                hRet = m_pCmdRun->Execute(stCmd.dwCommand, stCmd.lpCmdData, stCmd.dwTimeOut, stResult.lpResult);
                if (hRet != WFS_SUCCESS)
                {
                    Log(ThisModule, __LINE__, "执行[dwCommand=%u, SPName=%s]失败：hRet = %d", stCmd.dwCommand, stCmd.szSPName, hRet);
                    break;
                }
            } while (false);

            //该处不再取状态，取状态和执行系命令是互斥的，会造成卡死
            // 执行完成后，更新一次状态
            //                if (!IsUpdateDevPDL())
            //                    OnStatus();

            // 更新结果
            stResult.uMsgID = WFS_EXECUTE_COMPLETE;
            stResult.lpResult->hResult = hRet;
            m_cXfsData.Add(stResult);

            // 清缓存
            FreeBuffer(stCmd);
            stResult.clear();

            m_bIsParallelExecCrypt = false;
        }
    }
    catch (...)
    {
        Log(ThisModule, __LINE__, "Run_ParallelExecute线程异常");
    }

    return;
}
//30-00-00-00(FS#0002) add end

//////////////////////////////////////////////////////////////////////////

HRESULT CSPBaseClass::CheckParameter(const SPCMDDATA &stCmd)
{
    THISMODULE(__FUNCTION__);
    if (!m_pCmdRun)
    {
        Log(ThisModule, __LINE__, "无效命令数据执行指针:m_pCmdRun=NULL");
        return WFS_ERR_INVALID_POINTER;
    }

    if (!stCmd.hService)
    {
        Log(ThisModule, __LINE__, "无效服务句柄");
        return WFS_ERR_INVALID_HSERVICE;
    }

    if (!m_cAppRun.IsAppRunning(stCmd.szAppName, stCmd.szAppID))
    {
        Log(ThisModule, __LINE__, "Agent窗口异常或已退出");
        return WFS_ERR_SOFTWARE_ERROR;
    }

    // 判断是否超时
    SYSTEMTIME stCurT = CQtTime::CurrentTime();
    SYSTEMTIME stSpan = CQtTime::DiffTime(stCurT, stCmd.stReqTime);
    DWORD dwSpanTime = (stSpan.wMinute * 60 + stSpan.wSecond) * 1000;  // 转换为毫秒
    if (stSpan.wHour > 0 || dwSpanTime > stCmd.dwTimeOut)
    {
        Log(ThisModule, __LINE__, "uMsg等待执行超时: dwTimeOut=%u[已超时时间：%u[wHour=%d]]", stCmd.dwTimeOut, dwSpanTime, stSpan.wHour);
        return WFS_ERR_TIMEOUT;
    }

    return WFS_SUCCESS;
}

HRESULT CSPBaseClass::GetResultBuff(const SPCMDDATA &stCmd, SPRESULTDATA &stResult)
{
    // THISMODULE(__FUNCTION__);
    // 申请结果内存
    LPWFSRESULT lpResult = nullptr;
    HRESULT hRet = m_pIWFM->WFMAllocateBuffer(sizeof(WFSRESULT), WFS_MEM_FLAG, (LPVOID *)&lpResult);
    if (hRet != WFS_SUCCESS)
        return hRet;
    memset(lpResult, 0x00, sizeof(WFSRESULT));

    // 更新
    lpResult->hService = stCmd.hService;
    lpResult->RequestID = stCmd.ReqID;
    lpResult->u.dwCommandCode = stCmd.dwCommand;
    lpResult->tsTimestamp = CQtTime::CurrentTime();

    stResult.lpResult = lpResult;
    stResult.hService = stCmd.hService;
    stResult.ReqID = stCmd.ReqID;
    stResult.stReqTime = stCmd.stReqTime;
    strcpy(stResult.szAgentName, stCmd.szAgentName);
    strcpy(stResult.szAppName, stCmd.szAppName);
    strcpy(stResult.szAppID, stCmd.szAppID);
    lpResult = nullptr;  // 此防止释放
    return WFS_SUCCESS;
}

void CSPBaseClass::AddExeCmd(SPCMDDATA &stCmd)
{
    THISMODULE(__FUNCTION__);

    // 执行操作，要检测是否已被其他服务锁定
    if (!IsOtherServiceLocked(stCmd))
    {
        m_cXfsData.AddExe(stCmd);
        return;
    }
    // 如果申请结果内存失败，则SP无返回，App只能自己超时
    SPRESULTDATA stResult;
    HRESULT hRet = GetResultBuff(stCmd, stResult);
    if (WFS_SUCCESS != hRet)
    {
        Log(ThisModule, __LINE__, "获取结果内存失败:hRet=%d, eCmdID=%d", hRet, stCmd.eCmdID);
        FreeBuffer(stCmd);
        return;
    }

    // 更新结果
    stResult.uMsgID = WFS_EXECUTE_COMPLETE;
    stResult.lpResult->hResult = WFS_ERR_LOCKED;
    m_cXfsData.Add(stResult);
    FreeBuffer(stCmd);
    return;
}

void CSPBaseClass::SaveServiceData(const SPCMDDATA &stCmd)
{
    APPSERVICEDATA stApp;
    stApp.hService = stCmd.hService;
    stApp.stStartTime = CQtTime::CurrentTime();
    strcpy(stApp.szLogicalName, stCmd.szLogicalName);
    strcpy(stApp.szSPClass, stCmd.szSPClass);
    strcpy(stApp.szSPName, stCmd.szSPName);
    strcpy(stApp.szAgentName, stCmd.szAgentName);
    strcpy(stApp.szAppName, stCmd.szAppName);
    strcpy(stApp.szAppID, stCmd.szAppID);

    m_cXfsApp.Add(stApp);
    return;
}

bool CSPBaseClass::IsHasService(const SPCMDDATA &stCmd)
{
    return m_cXfsApp.IsExistService(stCmd.hService, stCmd.szSPName);
}

bool CSPBaseClass::IsExecuting()
{
    return m_bExecuting;
}

bool CSPBaseClass::IsOtherServiceLocked(const SPCMDDATA &stCmd)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_cXfsApp.IsAppLock(stCmd.hService))
    {
        return false;
    }

    if (!m_cXfsApp.IsAnyLocked())
    {
        return false;
    }

    Log(ThisModule, __LINE__, "SP服务[%s]已被其中一个App锁定", m_szSPName);
    return true;
}

bool CSPBaseClass::IsRegisterMsgID(DWORD dwEventID, DWORD dwMsgID)
{
    // DWORD dwEventID = SERVICE_EVENTS | USER_EVENTS | SYSTEM_EVENTS | EXECUTE_EVENTS;
    DWORD dwMstEventID = 0;
    switch (dwMsgID)
    {
    case WFS_EXECUTE_EVENT:
        dwMstEventID = EXECUTE_EVENTS;
        break;
    case WFS_SERVICE_EVENT:
        dwMstEventID = SERVICE_EVENTS;
        break;
    case WFS_USER_EVENT:
        dwMstEventID = USER_EVENTS;
        break;
    case WFS_SYSTEM_EVENT:
        dwMstEventID = SYSTEM_EVENTS;
        break;
    default:
        break;
    }
    // 判断是不有此事件和注册窗口是否正常
    if ((dwEventID & dwMstEventID) == dwMstEventID)
        return true;
    else
        return false;
}

bool CSPBaseClass::IsCRSEventClassMatch(DWORD dwEventID, const APPSERVICEDATA &stAppData)
{
    DWORD dwOffset = (dwEventID / 100) * 100;
    if (dwOffset == CDM_SERVICE_OFFSET)
    {
        if (strcmp(stAppData.szSPClass, "CDM") != 0)
            return false;
    }
    else if (dwOffset == CIM_SERVICE_OFFSET)
    {
        if (strcmp(stAppData.szSPClass, "CIM") != 0)
            return false;
    }

    return true;
}

bool CSPBaseClass::IsSIUEventClassMatch(DWORD dwEventID, const APPSERVICEDATA &stAppData, const LPWFSSIUPORTEVENT lpEvent)
{
    // 判断是否为SIU事件
    DWORD dwOffset = (dwEventID / 100) * 100;
    if (dwOffset != SIU_SERVICE_OFFSET)
        return false;
    if (strcmp(stAppData.szSPClass, "SIU") != 0)
        return false;
    if (dwEventID != WFS_SRVE_SIU_PORT_STATUS)
        return false;

    // 判断是否已启用
    SIUSETEVENTDATA stSIU;
    if (!m_cSIUSetEvent.Get(stAppData.hService, stSIU))
        return false;

    if (IsSIUEnableEvent(stSIU.stEvent, lpEvent))
        return false;

    return true;
}

LPCSTR CSPBaseClass::GetXfsPath()
{
#ifdef Q_OS_WIN
    m_strXfsPath = WINPATH;
#else
    m_strXfsPath = LINUXPATH;
#endif
    return m_strXfsPath.c_str();
}

void CSPBaseClass::FreeBuffer(SPCMDDATA &stCmd)
{
    if (stCmd.lpCmdData != nullptr)
        m_pIWFM->WFMFreeBuffer(stCmd.lpCmdData);
    stCmd.clear();
    return;
}

void CSPBaseClass::FreeBuffer(SPRESULTDATA &stResult)
{
    if (stResult.lpResult != nullptr)
        m_pIWFM->WFMFreeBuffer(stResult.lpResult);
    stResult.clear();
    return;
}

bool CSPBaseClass::IsSIUEnableEvent(WFSSIUENABLE stEnable, const LPWFSSIUPORTEVENT lpEvent)
{
    bool bRet = false;
    switch (lpEvent->wPortType)
    {
    case WFS_SIU_SENSORS:
    {
        if (lpEvent->wPortIndex >= WFS_SIU_SENSORS_SIZE)
            break;
        if (stEnable.fwSensors[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
            bRet = true;
    }
    break;
    case WFS_SIU_DOORS:
    {
        if (lpEvent->wPortIndex >= WFS_SIU_DOORS_SIZE)
            break;
        if (stEnable.fwDoors[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
            bRet = true;
    }
    break;
    case WFS_SIU_INDICATORS:
    {
        if (lpEvent->wPortIndex >= WFS_SIU_INDICATORS_SIZE)
            break;
        if (stEnable.fwIndicators[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
            bRet = true;
    }
    break;
    case WFS_SIU_AUXILIARIES:
    {
        if (lpEvent->wPortIndex >= WFS_SIU_AUXILIARIES_SIZE)
            break;
        if (stEnable.fwAuxiliaries[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
            bRet = true;
    }
    break;
    case WFS_SIU_GUIDLIGHTS:
    {
        if (lpEvent->wPortIndex >= WFS_SIU_GUIDLIGHTS_SIZE)
            break;
        if (stEnable.fwGuidLights[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
            bRet = true;
    }
    break;
    default:
        break;
    }
    return bRet;
}

//////////////////////////////////////////////////////////////////////////
LPCSTR CSPBaseClass::ConvertMSGID(UINT uMSGID)
{
    switch (uMSGID)
    {
    case WFS_OPEN_COMPLETE:
        return "WFS_OPEN_COMPLETE";
    case WFS_CLOSE_COMPLETE:
        return "WFS_CLOSE_COMPLETE";
    case WFS_EXECUTE_COMPLETE:
        return "WFS_EXECUTE_COMPLETE";
    case WFS_GETINFO_COMPLETE:
        return "WFS_GETINFO_COMPLETE";
    case WFS_REGISTER_COMPLETE:
        return "WFS_REGISTER_COMPLETE";
    case WFS_EXECUTE_EVENT:
        return "WFS_EXECUTE_EVENT";
    case WFS_SERVICE_EVENT:
        return "WFS_SERVICE_EVENT";
    case WFS_USER_EVENT:
        return "WFS_USER_EVENT";
    case WFS_SYSTEM_EVENT:
        return "WFS_SYSTEM_EVENT";
    default:
        return "UNKNOWN MSGID ";
    }
}

long CSPBaseClass::CancelServerCMD(HSERVICE hService)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    SPCMDDATA stCmd;
    SPRESULTDATA stResult;
    while (true)
    {
        // 检测是否有新命令
        if (!m_cXfsData.PeekExe(hService, stCmd))
            break;

        // 如果申请结果内存失败，则SP无返回，App只能自己超时
        stResult.clear();
        HRESULT hRet = GetResultBuff(stCmd, stResult);
        if (WFS_SUCCESS != hRet)
        {
            Log(ThisModule, __LINE__, "获取结果内存失败:hRet=%d, eCmdID=%d", hRet, stCmd.eCmdID);
            FreeBuffer(stCmd);
            continue;
        }

        // 更新结果
        stResult.uMsgID = WFS_EXECUTE_COMPLETE;
        stResult.lpResult->hResult = WFS_ERR_CANCELED;
        m_cXfsData.Add(stResult);
    }
    return 0;
}

bool CSPBaseClass::IsUpdateDevPDL()
{
    return m_bUpdateDevPDLing;
}
