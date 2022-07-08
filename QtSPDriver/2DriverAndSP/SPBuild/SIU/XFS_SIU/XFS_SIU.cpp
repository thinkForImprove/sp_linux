#include "XFS_SIU.h"

static const char *ThisFile = "XFS_SIU.cpp";
static const char *DEVTYPE  = "SIU";
//增加设备类型时在此处添加
#define DEV_TYPE_MAX_NUM    (2)
const char strDevTypeList[DEV_TYPE_MAX_NUM][MAX_PATH] = {"HT", "CFES"};

//////////////////////////////////////////////////////////////////////////
CXFS_SIU::CXFS_SIU(): m_pMutexGetStatus(nullptr)
{
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    memset(&m_stStatus, 0x00, sizeof(m_stStatus));
    memset(&m_stCaps, 0x00, sizeof(m_stCaps));
    m_bSaftDoorToOperatorSwitch = false;
}


CXFS_SIU::~CXFS_SIU()
{

}

long CXFS_SIU::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseBCR
    if (0 != m_pBase.Load("SPBaseSIU.dll", "CreateISPBaseSIU", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseSIU失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_SIU::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    string strPort = m_cXfsReg.GetValue("CONFIG", "Port", "USB");
    string strDevDll = m_cXfsReg.GetValue("DriverDllName", "");
    if (strDevDll.empty())
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }
    int iDevType = m_cXfsReg.GetValue("DevType", (DWORD)0);
    if(iDevType >= DEV_TYPE_MAX_NUM){
        iDevType = 0;
    }
    // 加载DEV
    long lRet = m_pDev.Load(strDevDll.c_str(), "CreateIDevSIU", strDevTypeList[iDevType]);
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "加载%s失败, lRet=%d,GetLastError=%s", strDevDll.c_str(), lRet, m_pDev.LastError().toStdString().c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    //读取配置项
    ReadConfig();                   //40-00-00-00(FT#0002)

    // 打开连接
    lRet = m_pDev->Open(strPort.c_str());
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败");
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 复位设备
    if (0 != m_pDev->Reset())
    {
        // 复位失败，不用返回故障，只要更新状态为故障就行了
        Log(ThisModule, __LINE__, "设备复位失败！");
    }


    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 更新扩展状态
    char szDevVer[256] = { 0 };
    m_pDev->GetDevInfo(szDevVer);
    UpdateExtra("000", szDevVer);

    // 更新一次状态
    OnStatus();
    UpdateCap();
    EnableEvent();

    // 读取配置
    string strOperatorSwitch = m_cXfsReg.GetValue("CONFIG", "SaftDoorToOperatorSwitch", "0");
    m_bSaftDoorToOperatorSwitch = (strOperatorSwitch == "0") ? false : true;

    Log(ThisModule, 1, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pDev != nullptr)
        m_pDev->Close();
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::OnStatus()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 空闲更新状态
    DEVSIUSTATUS stDevStatus;
    WFSSIUSTATUS stSiuStatus;
    memset(&stSiuStatus, 0x00, sizeof(stSiuStatus));
    m_pDev->GetStatus(stDevStatus);
    // 状态转换
    ConvertStatus(stDevStatus, stSiuStatus);
    // 更新状态和发状态事件
    UpdateStatus(stSiuStatus);
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}


HRESULT CXFS_SIU::GetStatus(LPWFSSIUSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //状态
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_stStatus;
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::GetCapabilities(LPWFSSIUCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 能力
    m_stCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_stCaps;
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::SetEnableEvent(const LPWFSSIUENABLE lpEvent)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutexStl(m_cEnableMutex);
    if (lpEvent == nullptr)
        return WFS_ERR_INVALID_POINTER;

    // 保存
    if (m_ltEnable.empty())
    {
        WFSSIUENABLE stEvent;
        memcpy(&stEvent, lpEvent, sizeof(stEvent));
        stEvent.lpszExtra = nullptr;// 暂时不支持扩展状态
        m_ltEnable.push_back(stEvent);
        return WFS_SUCCESS;
    }
    //更新
    for (auto &it : m_ltEnable)
    {
        UINT i = 0;
        for (i = 0; i < WFS_SIU_SENSORS_SIZE; i++)
        {
            if (lpEvent->fwSensors[i] != WFS_SIU_NO_CHANGE)
                it.fwSensors[i] = lpEvent->fwSensors[i];
        }

        for (i = 0; i < WFS_SIU_DOORS_SIZE; i++)
        {
            if (lpEvent->fwDoors[i] != WFS_SIU_NO_CHANGE)
                it.fwDoors[i] = lpEvent->fwDoors[i];
        }
        for (i = 0; i < WFS_SIU_INDICATORS_SIZE; i++)
        {
            if (lpEvent->fwIndicators[i] != WFS_SIU_NO_CHANGE)
                it.fwIndicators[i] = lpEvent->fwIndicators[i];
        }
        for (i = 0; i < WFS_SIU_AUXILIARIES_SIZE; i++)
        {
            if (lpEvent->fwAuxiliaries[i] != WFS_SIU_NO_CHANGE)
                it.fwAuxiliaries[i] = lpEvent->fwAuxiliaries[i];
        }
        for (i = 0; i < WFS_SIU_GUIDLIGHTS_SIZE; i++)
        {
            if (lpEvent->fwGuidLights[i] != WFS_SIU_NO_CHANGE)
                it.fwGuidLights[i] = lpEvent->fwGuidLights[i];
        }
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::SetDoor(const LPWFSSIUSETDOOR lpDoor)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (lpDoor == nullptr)
        return WFS_ERR_INVALID_POINTER;

    // 参数检测
    if (lpDoor->wDoor < WFS_SIU_CABINET ||
        lpDoor->wDoor > WFS_SIU_DOORS_MAX)
    {
        Log(ThisModule, __LINE__, "无效门ID=%d", lpDoor->wDoor);
        return WFS_ERR_INVALID_DATA;
    }
    if (lpDoor->fwCommand < WFS_SIU_OFF ||
        lpDoor->fwCommand > WFS_SIU_CONTINUOUS)
    {
        Log(ThisModule, __LINE__, "无效门命令：ID=%d,Cmd=%d", lpDoor->wDoor, lpDoor->fwCommand);
        return WFS_ERR_INVALID_DATA;
    }
    // 检测是否支持门
    if (!IsSupportDoor(lpDoor))
    {
        Log(ThisModule, __LINE__, "不支持门ID=%d", lpDoor->wDoor);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 设置
    WORD wszDoors[DEFSIZE]  = { 0 };
    wszDoors[lpDoor->wDoor] = lpDoor->fwCommand;
    long lRet = m_pDev->SetDoors(wszDoors);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "设置门失败，ID=%d", lpDoor->wDoor);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::SetIndicator(const LPWFSSIUSETINDICATOR lpIndicator)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (lpIndicator == nullptr)
        return WFS_ERR_INVALID_POINTER;

    // 参数检测
    if (lpIndicator->wIndicator < WFS_SIU_OPENCLOSE ||
        lpIndicator->wIndicator > WFS_SIU_INDICATORS_MAX)
    {
        Log(ThisModule, __LINE__, "无效ID=%d", lpIndicator->wIndicator);
        return WFS_ERR_INVALID_DATA;
    }

    if (lpIndicator->fwCommand < WFS_SIU_OFF ||
        lpIndicator->fwCommand > WFS_SIU_CONTINUOUS)
    {
        Log(ThisModule, __LINE__, "无效命令：ID=%d,Cmd=%d", lpIndicator->wIndicator, lpIndicator->fwCommand);
        return WFS_ERR_INVALID_DATA;
    }
    // 检测是否支
    if (!IsSupportIndicator(lpIndicator))
    {
        Log(ThisModule, __LINE__, "不支持ID=%d", lpIndicator->wIndicator);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 设置灯
    WORD wIndicator[DEFSIZE] = { 0 };
    wIndicator[lpIndicator->wIndicator] = lpIndicator->fwCommand;
    long lRet = m_pDev->SetIndicators(wIndicator);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "设置指示符失败，ID=%d", lpIndicator->wIndicator);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::SetGuidLight(const LPWFSSIUSETGUIDLIGHT lpGuidLight)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (lpGuidLight == nullptr)
        return WFS_ERR_INVALID_POINTER;

    // 参数检测
    if (lpGuidLight->wGuidLight < WFS_SIU_CARDUNIT ||
        lpGuidLight->wGuidLight > WFS_SIU_GUIDLIGHTS_MAX)
    {
        Log(ThisModule, __LINE__, "无效灯ID=%d", lpGuidLight->wGuidLight);
        return WFS_ERR_INVALID_DATA;
    }
    if (lpGuidLight->fwCommand < WFS_SIU_OFF ||
        lpGuidLight->fwCommand > WFS_SIU_CONTINUOUS)
    {
        Log(ThisModule, __LINE__, "无效灯命令:ID=%d,Cmd=%d", lpGuidLight->wGuidLight, lpGuidLight->fwCommand);
        return WFS_ERR_INVALID_DATA;
    }
    // 检测是否支持灯
    if (!IsSupportGuidLight(lpGuidLight))
    {
        Log(ThisModule, __LINE__, "不支持灯ID=%d", lpGuidLight->wGuidLight);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 设置灯
    WORD wszGuidLights[DEFSIZE] = {0};
    wszGuidLights[lpGuidLight->wGuidLight] = lpGuidLight->fwCommand;
    long lRet = m_pDev->SetGuidLights(wszGuidLights);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "设置灯失败，ID=%d", lpGuidLight->wGuidLight);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_SIU::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = m_pDev->Reset();
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "复位失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

void CXFS_SIU::EnableEvent()
{
    AutoMutexStl(m_cEnableMutex);
    //  WORD            fwSensors[WFS_SIU_SENSORS_SIZE];
    //  WORD            fwDoors[WFS_SIU_DOORS_SIZE];
    //  WORD            fwIndicators[WFS_SIU_INDICATORS_SIZE];
    //  WORD            fwAuxiliaries[WFS_SIU_AUXILIARIES_SIZE];
    //  WORD            fwGuidLights[WFS_SIU_GUIDLIGHTS_SIZE];

    std::string strEnableEvent = m_cXfsReg.GetValue("CONFIG", "AutoEnableEvent", "1");
    std::string strEnableGuidLights = m_cXfsReg.GetValue("CONFIG", "AutoEnableGuidLights", "0");
    if (strEnableEvent != "1")
        return;

    WFSSIUENABLE stEnable;
    memset(&stEnable, 0x00, sizeof(stEnable));
    stEnable.lpszExtra = nullptr;
    for (auto &it : stEnable.fwSensors)
    {
        it = WFS_SIU_ENABLE_EVENT;
    }
    for (auto &it : stEnable.fwDoors)
    {
        it = WFS_SIU_ENABLE_EVENT;
    }
    for (auto &it : stEnable.fwIndicators)
    {
        it = WFS_SIU_ENABLE_EVENT;
    }
    for (auto &it : stEnable.fwAuxiliaries)
    {
        it = WFS_SIU_ENABLE_EVENT;
    }

    for (auto &it : stEnable.fwGuidLights)
    {
        it = (strEnableGuidLights == "1") ? WFS_SIU_ENABLE_EVENT : WFS_SIU_DISABLE_EVENT;
    }

    m_ltEnable.push_back(stEnable);
    return;
}

//////////////////////////////////////////////////////////////////////////
void CXFS_SIU::ConvertStatus(const DEVSIUSTATUS &stDevStatus, WFSSIUSTATUS &stSiuStatus)
{
    WORD wStatus = 0;
    // 设备状态
    UpdateExtra(stDevStatus.szErrCode);
    switch (stDevStatus.wDevice)
    {
    case DEVICE_OFFLINE:        wStatus = WFS_SIU_DEVOFFLINE;       break;
    case DEVICE_ONLINE:         wStatus = WFS_SIU_DEVONLINE;        break;
    default:                    wStatus = WFS_SIU_DEVHWERROR;       break;
    }
    stSiuStatus.fwDevice = wStatus;

    // 门
    for (int i = 0; i < WFS_SIU_DOORS_SIZE && i < DEFSIZE; i++)
    {
        switch (stDevStatus.wDoors[i])
        {
        case DOOR_CLOSED:           wStatus = WFS_SIU_CLOSED;           break;
        case DOOR_OPEN:             wStatus = WFS_SIU_OPEN;             break;
        case DOOR_JAMMED:           wStatus = WFS_SIU_JAMMED;           break;
        default:                    wStatus = WFS_SIU_NOT_AVAILABLE;    break;
        }
        stSiuStatus.fwDoors[i] = wStatus;
    }
    // 灯
    for (int i = 0; i < WFS_SIU_GUIDLIGHTS_SIZE && i < DEFSIZE; i++)
    {
        switch (stDevStatus.wGuidLights[i])
        {
        case GUIDLIGHT_OFF:                 wStatus = WFS_SIU_OFF;                  break;
        case GUIDLIGHT_SLOW_FLASH:          wStatus = WFS_SIU_SLOW_FLASH;           break;
        case GUIDLIGHT_MEDIUM_FLASH:        wStatus = WFS_SIU_MEDIUM_FLASH;         break;
        case GUIDLIGHT_QUICK_FLASH:         wStatus = WFS_SIU_QUICK_FLASH;          break;
        case GUIDLIGHT_CONTINUOUS:          wStatus = WFS_SIU_CONTINUOUS;           break;
        default:                            wStatus = WFS_SIU_NOT_AVAILABLE;        break;
        }
        stSiuStatus.fwGuidLights[i] = wStatus;
    }
    // 传感器
    // 维护开关特殊处理
//40-00-00-00(FT#0002)    stSiuStatus.fwSensors[WFS_SIU_OPERATORSWITCH] = WFS_SIU_SUPERVISOR;
    stSiuStatus.fwSensors[WFS_SIU_OPERATORSWITCH] =                                                 //40-00-00-00(FT#0002)
            (m_iMaintenanceStatusValue == 0) ? WFS_SIU_SUPERVISOR : WFS_SIU_MAINTENANCE;            //40-00-00-00(FT#0002)
    if (m_bSaftDoorToOperatorSwitch)// 使用安全门作为维护开关
    {
        if (stSiuStatus.fwDoors[WFS_SIU_SAFE] == WFS_SIU_CLOSED)
            stSiuStatus.fwSensors[WFS_SIU_OPERATORSWITCH] = WFS_SIU_RUN;
    }
    else
    {
        if (stSiuStatus.fwDoors[WFS_SIU_CABINET] == WFS_SIU_CLOSED)
            stSiuStatus.fwSensors[WFS_SIU_OPERATORSWITCH] = WFS_SIU_RUN;
    }

    for (int i = 1; i < WFS_SIU_SENSORS_SIZE && i < DEFSIZE; i++)
    {
        switch (stDevStatus.wSensors[i])
        {
        case SENSORS_OFF:           wStatus = WFS_SIU_OFF;              break;
        case SENSORS_ON:            wStatus = WFS_SIU_ON;               break;
        case SENSORS_NOT_PRESENT:   wStatus = WFS_SIU_NOT_PRESENT;      break;
        case SENSORS_PRESENT:       wStatus = WFS_SIU_PRESENT;          break;
        default:                    wStatus = WFS_SIU_NOT_AVAILABLE;    break;
        }
        stSiuStatus.fwSensors[i] = wStatus;
    }

    // 指示符
    for (int i = 0; i < WFS_SIU_INDICATORS_SIZE && i < DEFSIZE; i++)
    {
        switch (stDevStatus.wIndicators[i])
        {
        case INDICATOR_OFF:         wStatus = WFS_SIU_OFF;              break;
        case INDICATOR_ON:          wStatus = WFS_SIU_ON;               break;
        default:                    wStatus = WFS_SIU_NOT_AVAILABLE;    break;
        }
        stSiuStatus.fwIndicators[i] = wStatus;
    }
    // 辅助器
    for (int i = 0; i < WFS_SIU_AUXILIARIES_SIZE && i < DEFSIZE; i++)
    {
        switch (stDevStatus.wAuxiliaries[i])
        {
        case AUXILIARIE_OFF:        wStatus = WFS_SIU_OFF;              break;
        case AUXILIARIE_ON:         wStatus = WFS_SIU_ON;               break;
        default:                    wStatus = WFS_SIU_NOT_AVAILABLE;    break;
        }
        stSiuStatus.fwAuxiliaries[i] = wStatus;
    }
}

bool CXFS_SIU::UpdateStatus(const WFSSIUSTATUS &stStatus)
{
    AutoMutex(*m_pMutexGetStatus);
    // 设备状态，判断状态是否有变化
    if (m_stStatus.fwDevice != stStatus.fwDevice)
    {
        m_pBase->FireStatusChanged(stStatus.fwDevice);
        // 故障时，也要上报故障事件
        if (stStatus.fwDevice == WFS_SIU_DEVHWERROR)
            m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_cExtra.GetErrDetail("ErrorDetail"));
    }

    // 传感器
    WFSSIUPORTEVENT stEvent;
    memset(&stEvent, 0x00, sizeof(stEvent));
    stEvent.wPortType = WFS_SIU_SENSORS;
    for (WORD i = 0; i < WFS_SIU_SENSORS_SIZE; i++)
    {
        if (m_stStatus.fwSensors[i] != stStatus.fwSensors[i])
        {
            stEvent.wPortIndex = i;
            stEvent.wPortStatus = stStatus.fwSensors[i];
            FirePortEvent(&stEvent);// 发送改变状态
        }
    }

    // 指示符
    memset(&stEvent, 0x00, sizeof(stEvent));
    stEvent.wPortType = WFS_SIU_INDICATORS;
    for (WORD i = 0; i < WFS_SIU_INDICATORS_SIZE; i++)
    {
        if (m_stStatus.fwIndicators[i] != stStatus.fwIndicators[i])
        {
            stEvent.wPortIndex = i;
            stEvent.wPortStatus = stStatus.fwIndicators[i];
            FirePortEvent(&stEvent);// 发送改变状态
        }
    }
    // 辅助器
    memset(&stEvent, 0x00, sizeof(stEvent));
    stEvent.wPortType = WFS_SIU_AUXILIARIES;
    for (WORD i = 0; i < WFS_SIU_AUXILIARIES_SIZE; i++)
    {
        if (m_stStatus.fwAuxiliaries[i] != stStatus.fwAuxiliaries[i])
        {
            stEvent.wPortIndex = i;
            stEvent.wPortStatus = stStatus.fwAuxiliaries[i];
            FirePortEvent(&stEvent);// 发送改变状态
        }
    }
    // 门
    memset(&stEvent, 0x00, sizeof(stEvent));
    stEvent.wPortType = WFS_SIU_DOORS;
    for (WORD i = 0; i < WFS_SIU_DOORS_SIZE; i++)
    {
        if (m_stStatus.fwDoors[i] != stStatus.fwDoors[i])
        {
            stEvent.wPortIndex = i;
            stEvent.wPortStatus = stStatus.fwDoors[i];
            FirePortEvent(&stEvent);// 发送改变状态
        }
    }

    // 灯
    memset(&stEvent, 0x00, sizeof(stEvent));
    stEvent.wPortType = WFS_SIU_GUIDLIGHTS;
    for (WORD i = 0; i < WFS_SIU_GUIDLIGHTS_SIZE; i++)
    {
        if (m_stStatus.fwGuidLights[i] != stStatus.fwGuidLights[i])
        {
            stEvent.wPortIndex = i;
            stEvent.wPortStatus = stStatus.fwGuidLights[i];
            FirePortEvent(&stEvent);// 发送改变状态
        }
    }

    // 保存新状态，暂时不支持扩展状态
    memcpy(&m_stStatus, &stStatus, sizeof(m_stStatus));    
    return true;
}

bool CXFS_SIU::UpdateCap()
{
    AutoMutex(*m_pMutexGetStatus);
    m_stCaps.wClass = WFS_SERVICE_CLASS_SIU;
    WORD wType      = 0;
    // 传感器
//40-00-00-00(FT#0002)    m_stCaps.fwSensors[WFS_SIU_OPERATORSWITCH] = WFS_SIU_RUN | WFS_SIU_SUPERVISOR;
    m_stCaps.fwSensors[WFS_SIU_OPERATORSWITCH] = WFS_SIU_RUN |                              //40-00-00-00(FT#0002)
            ((m_iMaintenanceStatusValue == 0) ? WFS_SIU_SUPERVISOR : WFS_SIU_MAINTENANCE);  //40-00-00-00(FT#0002)
    for (int i = 1; i < WFS_SIU_SENSORS_SIZE; i++)
    {
        if (m_stStatus.fwSensors[i] != WFS_SIU_NOT_AVAILABLE)
        {
            wType |= WFS_SIU_SENSORS;
            m_stCaps.fwSensors[i] = WFS_SIU_AVAILABLE;
        }
    }

    // 指示符
    for (int i = 0; i < WFS_SIU_INDICATORS_SIZE; i++)
    {
        if (m_stStatus.fwIndicators[i] != WFS_SIU_NOT_AVAILABLE)
        {
            wType |= WFS_SIU_INDICATORS;
            m_stCaps.fwIndicators[i] = WFS_SIU_AVAILABLE;
        }
    }
    // 辅助器
    for (int i = 0; i < WFS_SIU_AUXILIARIES_SIZE; i++)
    {
        if (m_stStatus.fwAuxiliaries[i] != WFS_SIU_NOT_AVAILABLE)
        {
            wType |= WFS_SIU_AUXILIARIES;
            m_stCaps.fwAuxiliaries[i] = WFS_SIU_AVAILABLE;
        }
    }
    // 门
    for (int i = 0; i < WFS_SIU_DOORS_SIZE; i++)
    {
        if (m_stStatus.fwDoors[i] != WFS_SIU_NOT_AVAILABLE)
        {
            wType |= WFS_SIU_DOORS;
            m_stCaps.fwDoors[i] = WFS_SIU_CLOSED | WFS_SIU_OPEN;
        }
    }

    // 灯
    for (int i = 0; i < WFS_SIU_GUIDLIGHTS_SIZE; i++)
    {
        if (m_stStatus.fwGuidLights[i] != WFS_SIU_NOT_AVAILABLE)
        {
            wType |= WFS_SIU_GUIDLIGHTS;
            m_stCaps.fwGuidLights[i] = WFS_SIU_AVAILABLE;
        }
    }

    m_stCaps.fwType    = wType;    
    return true;
}

bool CXFS_SIU::IsDevStatusOK()
{
    return (m_stStatus.fwDevice == WFS_SIU_DEVONLINE) ? true : false;
}

bool CXFS_SIU::IsSupportDoor(const LPWFSSIUSETDOOR lpDoor)
{
    if (m_stCaps.fwDoors[lpDoor->wDoor] == WFS_SIU_NOT_AVAILABLE)
        return false;
    return true;
}

bool CXFS_SIU::IsSupportIndicator(const LPWFSSIUSETINDICATOR lpIndicator)
{
    if (m_stCaps.fwIndicators[lpIndicator->wIndicator] == WFS_SIU_NOT_AVAILABLE)
        return false;
    return true;
}

bool CXFS_SIU::IsSupportGuidLight(const LPWFSSIUSETGUIDLIGHT lpLight)
{
    if (m_stCaps.fwGuidLights[lpLight->wGuidLight] == WFS_SIU_NOT_AVAILABLE)
        return false;
    return true;
}

bool CXFS_SIU::FirePortEvent(const LPWFSSIUPORTEVENT lpEvent)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    if (IsEnableEvent(lpEvent))
    {
        return m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_SIU_PORT_STATUS, lpEvent);
    }
    //Log(ThisModule, __LINE__, "没有启用对应事件：wPortType=%d, wPortIndex=%d", lpEvent->wPortType, lpEvent->wPortIndex);
    return false;
}

bool CXFS_SIU::IsEnableEvent(const LPWFSSIUPORTEVENT lpEvent)
{
    AutoMutexStl(m_cEnableMutex);
    // 主要对应哪个AP控制是在SPBaseClass中，此主要是控制配置的默认是否能发事件
    bool bRet = false;
    for (auto &it : m_ltEnable)
    {
        if (bRet)
            break;
        switch (lpEvent->wPortType)
        {
        case WFS_SIU_SENSORS:
            {
                if (lpEvent->wPortIndex >= WFS_SIU_SENSORS_SIZE)
                    break;
                if (it.fwSensors[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
                    bRet = true;
            }
            break;
        case WFS_SIU_DOORS:
            {
                if (lpEvent->wPortIndex >= WFS_SIU_DOORS_SIZE)
                    break;
                if (it.fwDoors[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
                    bRet = true;
            }
            break;
        case WFS_SIU_INDICATORS:
            {
                if (lpEvent->wPortIndex >= WFS_SIU_INDICATORS_SIZE)
                    break;
                if (it.fwIndicators[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
                    bRet = true;
            }
            break;
        case WFS_SIU_AUXILIARIES:
            {
                if (lpEvent->wPortIndex >= WFS_SIU_AUXILIARIES_SIZE)
                    break;
                if (it.fwAuxiliaries[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
                    bRet = true;
            }
            break;
        case WFS_SIU_GUIDLIGHTS:
            {
                if (lpEvent->wPortIndex >= WFS_SIU_GUIDLIGHTS_SIZE)
                    break;
                if (it.fwGuidLights[lpEvent->wPortIndex] == WFS_SIU_ENABLE_EVENT)
                    bRet = true;
            }
            break;
        default:
            break;
        }
    }
    return bRet;
}

void CXFS_SIU::UpdateExtra(string strErrCode, string strDevVer/* = ""*/)
{
    if (strErrCode == "000")
        strErrCode.insert(0, "000000");// 没故障时显示
    else
        strErrCode.insert(0, "001101");// 固化的前六位

    m_cExtra.AddExtra("ErrorDetail", strErrCode.c_str());

    if (!strDevVer.empty())
    {
        char szSPVer[64] = { 0 };
        char szVer[64] = { "0001001" };
        //GetFileVersion(szVer);
        sprintf(szSPVer, "%08sV%07s", "SIU_V303", szVer);
        m_cExtra.AddExtra("VRTCount", "2");
        m_cExtra.AddExtra("VRTDetail[00]", szSPVer);                // SP版本程序名称8位+版本8位
        m_cExtra.AddExtra("VRTDetail[01]", strDevVer.c_str());      // Firmware(1)版本程序名称8位+版本8位

        // 增加安装包版本信息
        //CRegOperator cReg(HKEY_CURRENT_USER, "Software\\CFES\\XFS");
        //m_cExtra.AddExtra("InstallVer", cReg.GetRegValueSTRING("InstallVer", "Unknown"));
    }
}

//40-00-00-00(FT#0002)
void CXFS_SIU::ReadConfig()
{
    THISMODULE(__FUNCTION__);
    m_iMaintenanceStatusValue = 0;

    QString strINIFile("SIUConfig.ini");
#ifdef QT_WIN32
    strINIFile.prepend("C:/CFES/ETC/");
#else
    strINIFile.prepend("/usr/local/CFES/ETC/");
#endif
    CINIFileReader cIniFileReader;
    if (!cIniFileReader.LoadINIFile(strINIFile.toLocal8Bit().data()))
    {
        Log(ThisModule, __LINE__, "加载配置文件失败:%s", strINIFile.toLocal8Bit().data());
        return;
    }
    CINIReader cIniReader = cIniFileReader.GetReaderSection("SIUInfo");
    m_iMaintenanceStatusValue = (int)cIniReader.GetValue("MaintenanceStatusValue", 0);

    return;
}

//////////////////////////////////////////////////////////////////////////

