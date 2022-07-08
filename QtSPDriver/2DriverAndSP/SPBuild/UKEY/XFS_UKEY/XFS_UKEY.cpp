/***************************************************************
* 文件名称：XFS_UKEY.cpp
* 文件描述：UKEY发放模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月24日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_UKEY.h"

// 事件日志
static const char *ThisFile = "XFS_UKEY.cpp";

#define LOG_NAME     "XFS_UKEY.log"

#define SP_TYPE     "UKEY"

//-----------------------------------------------------------------------------------
//-------------------------------------构造/析构--------------------------------------
CXFS_UKEY::CXFS_UKEY()
{
    SetLogFile(LOG_NAME, ThisFile);

    m_nReConRet = CRD_SUCCESS;
}

CXFS_UKEY::~CXFS_UKEY()
{

}


//-----------------------------------------------------------------------------------
// 开始运行SP
long CXFS_UKEY::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseIDC
    if (0 != m_pBase.Load("SPBaseIDC.dll", "CreateISPBaseIDC", SP_TYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseIDC失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();

    return 0;
}

//-----------------------------------------------------------------------------------
//--------------------------------------基本接口---------------------------------------
// Open设备及初始化相关
HRESULT CXFS_UKEY::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    // 获取Manager配置文件中sp配置
    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // 获取INIp配置,初始化IDC和CRD的Status/Caps
    InitConfig();
    InitIDCCaps();
    InitIDCStatus();
    InitCRDCaps();
    InitCRDStatus();

    // 设备驱动动态库验证
    if (strlen(m_stConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败.", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 加载设备驱动动态库
    if (m_pDev == nullptr)
    {
        if (m_pDev.Load(m_stConfig.szDevDllName, "CreateIDevCRD", DEVTYPE_CHG(m_stConfig.nDriverType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DEVTYPE=%d|%s. ReturnCode:%d.",
                m_stConfig.szDevDllName, m_stConfig.nDriverType, DEVTYPE_CHG(m_stConfig.nDriverType),
                m_pDev.LastError().toUtf8().constData());
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = StartOpen();
    if (m_stConfig.usOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

// 关闭设备
HRESULT CXFS_UKEY::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
        m_pDev->Close();
    return WFS_SUCCESS;
}

// 实时状态更新
HRESULT CXFS_UKEY::OnStatus()
{
    INT nRet = UpdateDevStatus();
    /*if (m_stIDCStatus.fwDevice == WFS_IDC_DEVOFFLINE)
    {
        //StartOpen(TRUE);    // 重连
        //UpdateDevStatus();  // 重连后获取状态
    }*/

    return WFS_SUCCESS;
}

// Taken事件处理
HRESULT CXFS_UKEY::OnWaitTaken()
{
    if (m_enWaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    //WaitItemTaken();?????
    return WFS_SUCCESS;
}

// 命令取消
HRESULT CXFS_UKEY::OnCancelAsyncRequest()
{
    /*if (m_pDev != nullptr)
        m_pDev->CancelReadCard();*/
    return WFS_SUCCESS;
}

// 固件升级
HRESULT CXFS_UKEY::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//----------------------------------IDC类型接口(INFO)----------------------------------
// INFO: IDC: Status: 状态
HRESULT CXFS_UKEY::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpStatus = &m_stIDCStatus;
    return WFS_SUCCESS;
}

// INFO: IDC: Capabilities: 能力值
HRESULT CXFS_UKEY::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_stIDCCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_stIDCCaps;
    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//---------------------------------IDC类型接口(EXECUTE)-------------------------------
// CMD: IDC: 退卡
HRESULT CXFS_UKEY::EjectCard(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    return ConvertWfsCRD2IDC(InnerEjecdCard());
}

// CMD: IDC: 吞卡
HRESULT CXFS_UKEY::RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    WORD wPosition = m_stIDCStatus.fwMedia;

    HRESULT hRet = ConvertWfsCRD2IDC(InnerRetainCard(0));

    if (hRet == WFS_SUCCESS)
    {
        lpRetainCardData = new WFSIDCRETAINCARD;
        lpRetainCardData->usCount = m_stIDCStatus.usCards;
        lpRetainCardData->fwPosition = wPosition;
    }

    return hRet;
}

// CMD: IDC: 吞卡
HRESULT CXFS_UKEY::ResetCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    m_stIDCStatus.usCards = 0;
    m_stUKEYBoxList.SetDispBoxData(m_stUKEYBoxList.GetRetainNum(), 0, DISPRWT_COUNT);
    SetDispBoxCfg(m_stUKEYBoxList.GetRetainNum(), "0", DISPRWT_COUNT);

}

// 读UKEY编号
HRESULT CXFS_UKEY::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    HRESULT hRet = WFS_SUCCESS;

    DWORD dwOption = *(DWORD *)lpReadData;

    m_clCardData.Clear();

    // 参数检查
    if ((dwOption & WFS_IDC_TRACK3) != WFS_IDC_TRACK3)
    {
        Log(ThisModule, __LINE__, "读UKEY编号: 入参[lpReadData=%d] 无效, Return: %d.",
            dwOption, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    hRet = InnerReadRawData(dwOption, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读UKEY编号: ->InnerReadRawData(%d, %d) Fail, Return: %d.",
            dwOption, dwTimeOut, hRet);
        return hRet;
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_clCardData;

    return WFS_SUCCESS;
}

// IDC: 复位
HRESULT CXFS_UKEY::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    return ConvertWfsCRD2IDC(InnerReset(*lpResetIn, 0));
}

//-----------------------------------------------------------------------------------
//----------------------------------CRD类型接口(INFO)----------------------------------
// INFO: CRD: Status: 状态
HRESULT CXFS_UKEY::CRD_GetStatus(LPWFSCRDSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    lpStatus = &m_stCRDStatus;

    return WFS_SUCCESS;
}

// INFO: CRD: Capabilities: 能力值
HRESULT CXFS_UKEY::CRD_GetCapabilities(LPWFSCRDCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpCaps = &m_stCRDCaps;

    return WFS_SUCCESS;
}

// INFO: CRD: CardUnitInfo: 卡箱信息
HRESULT CXFS_UKEY::CRD_GetCardUnitInfo(LPWFSCRDCUINFO &lpCardUnit)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return InnerGetCardUnitInfo(lpCardUnit);
}


//-----------------------------------------------------------------------------------
//---------------------------------IDC类型接口(EXECUTE)-------------------------------
// EXEC: CRD: Dispense 发卡
HRESULT CXFS_UKEY::CRD_DispenseCard(const LPWFSCRDDISPENSE lpDispense)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    if (lpDispense == nullptr)
    {
        Log(ThisModule, __LINE__, "CRD_DispenseCard: Input Param Is NULL, Return: %d.",
            WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    return InnerDispenseCard(lpDispense->usNumber, lpDispense->bPresent);
}

// EXEC: CRD: 退卡
HRESULT CXFS_UKEY::CRD_EjecdCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    return InnerEjecdCard();
}

// EXEC: CRD: 回收卡
HRESULT CXFS_UKEY::CRD_RetainCard(const LPWFSCRDRETAINCARD lpRetainCard)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    if (lpRetainCard == nullptr)
    {
        Log(ThisModule, __LINE__, "CRD_RetainCard: Input Param Is NULL, Return: %d.",
            WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    return InnerRetainCard(lpRetainCard->usNumber);
}

// EXEC: CRD: 复位
HRESULT CXFS_UKEY::CRD_Reset(const LPWFSCRDRESET lpResset)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    if (lpResset == nullptr)
    {
        Log(ThisModule, __LINE__, "CRD_Reset: Input Param Is NULL, Return: %d.",
            WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    USHORT usRetainNum = 0;
    if (lpResset->usAction == WFS_CRD_RETAIN)   // 指定回收到第一个回收箱
    {
        if ((usRetainNum = m_stUKEYBoxList.GetRetainNum(1)) == DISPBOX_NOHAVE)
        {
            Log(ThisModule, __LINE__, "CRD_Reset: Input Param Is Retain, Not Found RetranBox, Return: %d.",
                WFS_ERR_CRD_INVALIDRETAINBIN);
            return WFS_ERR_CRD_INVALIDRETAINBIN;
        }
    }

    return InnerReset(lpResset->usAction, usRetainNum);
}

// EXEC: CRD: 设置卡箱信息
HRESULT CXFS_UKEY::CRD_SetCardUnitInfo(const LPWFSCRDCUINFO lpCuInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设备状态Check
    CHECK_DEVICE();

    // 入参Check
    if (lpCuInfo == nullptr)
    {
        Log(ThisModule, __LINE__, "Input Param == NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (lpCuInfo->usCount < 1)
    {
        Log(ThisModule, __LINE__, "Input Param: 箱信息集合数目: usCount[%d] < 1, Not Set, Return: %d.",
            lpCuInfo->usCount, WFS_SUCCESS);
        return WFS_SUCCESS;
    }

    if (lpCuInfo->lppList == nullptr)
    {
        Log(ThisModule, __LINE__, "Input Param: 箱信息集合数目: usCount[%d] > 0, 箱信息集合BUFF: lppList == NULL, Return: %d.",
            lpCuInfo->usCount, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    return InnerSetCardUnitInfo(lpCuInfo);
}


//-----------------------------------------------------------------------------------
//------------------------------------IDC事件消息-------------------------------------

void CXFS_UKEY::IDC_FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_UKEY::IDC_FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_UKEY::IDC_FireCardInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIAINSERTED, nullptr);
}

void CXFS_UKEY::IDC_FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIAREMOVED, nullptr);
}

void CXFS_UKEY::IDC_FireMediaRetained()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIARETAINED, nullptr);
}

void CXFS_UKEY::IDC_FireRetainBinThreshold(WORD wReBin)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_IDC_RETAINBINTHRESHOLD, (LPVOID)&wReBin);
}

void CXFS_UKEY::IDC_FireMediaDetected(WORD ResetOut)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIADETECTED, (LPVOID)&ResetOut);
}

void CXFS_UKEY::IDC_FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData)
{
    WFSIDCTRACKEVENT data;
    data.fwStatus   = wStatus;
    data.lpstrTrack = pTrackName;
    data.lpstrData  = pTrackData;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDTRACKDATA, &data);
}


//-----------------------------------------------------------------------------------
//------------------------------------CRD事件消息-------------------------------------
// 上报 EVENT: WFS_SRVE_CRD_MEDIAREMOVED
void CXFS_UKEY::CRD_FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CRD_MEDIAREMOVED, nullptr);
}

// 上报 EVENT: WFS_SRVE_CRD_MEDIADETECTED
void CXFS_UKEY::CRD_FireMediaDetected(LPWFSCRDMEDIADETECTED lpMediaDet)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CRD_MEDIADETECTED, lpMediaDet);
}

// 上报 EVENT: WFS_USRE_CRD_CARDUNITTHRESHOLD
void CXFS_UKEY::CRD_FireUnitTheshold(LPWFSCRDCARDUNIT lpCardUnit)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_CRD_CARDUNITTHRESHOLD, lpCardUnit);
}

// 上报 EVENT: WFS_SRVE_CRD_CARDUNITINFOCHANGED
void CXFS_UKEY::CRD_FireUnitInfoChanged(LPWFSCRDCARDUNIT lpCardUnit)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CRD_CARDUNITINFOCHANGED, lpCardUnit);
}

// 上报 EVENT: WFS_EXEE_CRD_CARDUNITERROR
void CXFS_UKEY::CRD_FireUnitError(LPWFSCRDCUERROR lpCardUnitError)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CRD_CARDUNITERROR, lpCardUnitError);
}

// 上报 EVENT: WFS_SRVE_CRD_DEVICEPOSITION
void CXFS_UKEY::CRD_FireDEvicePosition(LPWFSCRDDEVICEPOSITION lpDevicePosition)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CRD_DEVICEPOSITION, lpDevicePosition);
}

//-----------------------------------------------------------------------------------
//------------------------------------CRD事件消息 封装---------------------------------
// EVENT封装: WFS_USRE_CRD_CARDUNITTHRESHOLD
void CXFS_UKEY::CRD_FireUnitThresHold_Pack()
{
    THISMODULE(__FUNCTION__);

    LPWFSCRDCARDUNIT lpUnit = nullptr;
    USHORT usCount = 0;             // 当前数目
    USHORT usThreshold = 0;         // 阀值
    USHORT usBoxType = 0;           // 箱类型
    USHORT usBoxStat = 0;           // 箱状态
    BOOL   bHardSorThres;           // 是否基于硬件传感器生成阀值事件
    BOOL   bThresholdOk = FALSE;    // 上报阀值事件标记

    for (INT i = 0; i < DISPBOX_COUNT; i ++)
    {
        if (m_stUKEYBoxList.stDispBox[i].bIsHave == TRUE)
        {
            bHardSorThres = m_stUKEYBoxList.stDispBox[i].bHardSensor;
            usBoxType = m_stUKEYBoxList.stDispBox[i].usBoxType;
            bThresholdOk = FALSE;

            if (bHardSorThres == TRUE)  // 基于硬件生成阀值事件
            {
                usBoxStat = m_stUKEYBoxList.stDispBox[i].usStatus;
                if (usBoxType == DISPBOX_STORAGE && usBoxStat == DISPBOX_LOW &&     // 阀值条件1符合: 存储箱+少
                    (usThreshold - usCount) != m_stUKEYBoxListOld.nThresholdDist[i])// 阀值条件2符合: 阀值事件未上报
                {
                    bThresholdOk = TRUE;
                } else
                if (usBoxType == DISPBOX_STORAGE && usBoxStat == DISPBOX_HIGH &&    // 阀值条件符合: 回收箱+将满
                    (usCount - usThreshold) != m_stUKEYBoxListOld.nThresholdDist[i])// 阀值条件2符合: 阀值事件未上报
                {
                    bThresholdOk = TRUE;
                }
            } else  // 基于计数生成阀值事件
            {
                usCount = m_stUKEYBoxList.stDispBox[i].usCount;
                usThreshold = m_stUKEYBoxList.stDispBox[i].usThreshold;
                if (usThreshold > 0)    // 阀值有效
                {
                    if (usBoxType == DISPBOX_STORAGE)   // 存储箱
                    {
                        if (usCount <= usThreshold &&               // 阀值条件1符合: 当前数据 <= 阀值
                            (usThreshold - usCount) != m_stUKEYBoxListOld.nThresholdDist[i])   // 阀值条件2符合: 阀值事件未上报
                        {
                            bThresholdOk = TRUE;
                        }
                    } else
                    if (usBoxType == DISPBOX_RETRACT)   // 回收箱
                    {
                        if (usCount >= usThreshold &&               // 阀值条件1符合: 当前数据 <= 阀值
                            (usCount - usThreshold) != m_stUKEYBoxListOld.nThresholdDist[i])   // 阀值条件2符合: 阀值事件未上报
                        {
                            bThresholdOk = TRUE;
                        }
                    }
                }
            }

            // 上报阀值事件标记:T
            if (bThresholdOk == TRUE)
            {
                if (lpUnit != nullptr)
                {
                    delete lpUnit;
                    lpUnit = nullptr;
                }

                lpUnit = new WFSCRDCARDUNIT();
                if (lpUnit == nullptr)
                {
                    Log(ThisModule, __LINE__, "lpUnit New(WFSCRDCARDUNIT) Result Is NULL, "
                                        "Not Report Event[WFS_USRE_CRD_CARDUNITTHRESHOLD].");
                    return;
                }

                CardUnitInfoPack(lpUnit, m_stUKEYBoxList.stDispBox[i].usNumber);
                CRD_FireUnitTheshold(lpUnit);
                m_stUKEYBoxListOld.nThresholdDist[i] =
                        (usBoxType == DISPBOX_STORAGE ? usThreshold - usCount : usCount - usThreshold);
            }
        }
    }
}

// EVENT封装: WFS_SRVE_CRD_CARDUNITINFOCHANGED
void CXFS_UKEY::CRD_FireUnitInfoChanged_Pack()
{
    THISMODULE(__FUNCTION__);

    LPWFSCRDCARDUNIT lpUnit = nullptr;

    for (INT i = 0; i < DISPBOX_COUNT; i ++)
    {
        if (m_stUKEYBoxList.stDispBox[i].bIsHave == TRUE)
        {
            if (memcmp(&(m_stUKEYBoxList.stDispBox[i]), &(m_stUKEYBoxListOld.stDispBox[i]),
                       sizeof(STDISPPBOX)) != 0)    // 有效卡单元发生变化
            {
                if (lpUnit != nullptr)
                {
                    delete lpUnit;
                    lpUnit = nullptr;
                }

                lpUnit = new WFSCRDCARDUNIT();
                if (lpUnit == nullptr)
                {
                    Log(ThisModule, __LINE__, "lpUnit New(WFSCRDCARDUNIT) Result Is NULL, "
                                        "Not Report Event[WFS_SRVE_CRD_CARDUNITINFOCHANGED].");
                    return;
                }

                CardUnitInfoPack(lpUnit, m_stUKEYBoxList.stDispBox[i].usNumber);
                CRD_FireUnitInfoChanged(lpUnit);
                memcpy(&(m_stUKEYBoxListOld.stDispBox[i]), &(m_stUKEYBoxList.stDispBox[i]),
                       sizeof(STDISPPBOX));
            }
        }
    }
}

// EVENT封装: WFS_EXEE_CRD_CARDUNITERROR
void CXFS_UKEY::CRD_FireUnitError_Pack(USHORT usBoxNum, LONG lCode)
{
    THISMODULE(__FUNCTION__);

    LPWFSCRDCUERROR lpCUError = new WFSCRDCUERROR();
    BOOL bNeed = false;
    INT nCnt = 0;

    if (lpCUError == nullptr)
    {
        Log(ThisModule, __LINE__, "LPWFSCRDCUERROR New() Result Is NULL, Not Report Event[WFS_EXEE_CRD_CARDUNITERROR].");
        return;
    }

    switch(lCode)
    {
        case WFS_CRD_CARDUNITINVALID:       // 指定卡单元ID/序列号无效
            lpCUError->wFailure = WFS_CRD_CARDUNITINVALID;
            lpCUError->lpCardUnit = nullptr;
            break;
        case WFS_CRD_CARDUNITEMPTY:// 指定卡单元为空
            lpCUError->wFailure = WFS_CRD_CARDUNITEMPTY;
            lpCUError->lpCardUnit = nullptr;
            bNeed = TRUE;
            break;
        case WFS_CRD_CARDUNITERROR:// 指定卡单元故障
            lpCUError->wFailure = WFS_CRD_CARDUNITERROR;
            lpCUError->lpCardUnit = nullptr;
            bNeed = TRUE;
            break;
        default:
            Log(ThisModule, __LINE__, "Input Param[%d] Is NULL, Not Report Event[WFS_EXEE_CRD_CARDUNITERROR].");
            return;
    }

    if (bNeed == TRUE)  // 需要设定UNITINFO
    {
        lpCUError->lpCardUnit = new WFSCRDCARDUNIT();
        if (lpCUError->lpCardUnit == nullptr)
        {
            Log(ThisModule, __LINE__, "LPWFSCRDCUERROR->lpCardUnit New(WFSCRDCARDUNIT) Result Is NULL, "
                                "Not Report Event[WFS_EXEE_CRD_CARDUNITERROR].");
            return;
        }
        CardUnitInfoPack(lpCUError->lpCardUnit, usBoxNum);
    }

    CRD_FireUnitError(lpCUError);

    return;
}


