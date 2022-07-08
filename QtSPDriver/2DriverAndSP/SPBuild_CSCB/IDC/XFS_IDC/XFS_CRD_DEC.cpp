/***************************************************************
* 文件名称：XFS_CRD_DEC.cpp
* 文件描述：CRD-DISPENSE系列标准命令处理口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年6月30日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_IDC.h"

// 验证是否支持发卡模块
#define CRD_SUPP_CHECK() \
    if (m_stCRDINIConfig.bCRDDeviceSup == FALSE) \
    { \
        Log(ThisModule, -1, "CRD Device INI Set NotSupp，Return: %d.", \
            WFS_ERR_UNSUPP_COMMAND); \
        return WFS_ERR_UNSUPP_COMMAND; \
    }

//--------------------------标准命令------------------------------

// INFO: CRD: Status: 状态
HRESULT CXFS_IDC::CRD_GetStatus(LPWFSCRDSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    m_CRDStatus.lpszExtra = (LPSTR)m_cCRDExtra.GetExtra();
    lpStatus = &m_CRDStatus;

    return WFS_SUCCESS;
}

// INFO: CRD: Capabilities: 能力值
HRESULT CXFS_IDC::CRD_GetCapabilities(LPWFSCRDCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    lpCaps = &m_CRDCaps;

    return WFS_SUCCESS;
}

// INFO: CRD: CardUnitInfo: 卡箱信息
HRESULT CXFS_IDC::CRD_GetCardUnitInfo(LPWFSCRDCUINFO &lpCardUnit)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    UpdateDevCRDStatus();   // 更新一次状态
    UpdateRetainCards(MCM_NOACTION, false);     // 40-00-00-00(FT#0013)

    // 通过INI卡箱列表+设备卡箱状态组织应答
    LPWFSCRDCUINFO lpWfsCrdCUUnit = nullptr;
    LPWFSCRDCARDUNIT lpWfsCrdUnit = nullptr;
    INT nCnt = 0;
    USHORT usBoxNo = 0;

    // 卡箱数目
    lpWfsCrdCUUnit = new WFSCRDCUINFO();
    lpCardUnit = lpWfsCrdCUUnit;
    lpWfsCrdCUUnit->usCount = (USHORT)m_stCRDBoxList.nGetDispBoxCount();
    if (lpWfsCrdCUUnit->usCount < 1)
    {
        lpWfsCrdCUUnit->lppList = nullptr;
    } else
    {
        lpWfsCrdCUUnit->lppList = new LPWFSCRDCARDUNIT[lpWfsCrdCUUnit->usCount];
        for(INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            usBoxNo = i + 1;

            if (m_stCRDBoxList.bGetDispBoxIsHave(usBoxNo) != TRUE)
            {
                continue;   // 指定箱号不可用
            }

            lpWfsCrdCUUnit->lppList[i] = new WFSCRDCARDUNIT();
            lpWfsCrdUnit = lpWfsCrdCUUnit->lppList[i];
            memset(lpWfsCrdUnit, 0x00, sizeof(WFSCRDCARDUNIT));
            // 卡单元结构索引号(1~N)
            lpWfsCrdUnit->usNumber = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_NUMBER);
            // 卡单元类型
            nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_TYPE);
            lpWfsCrdUnit->usType = (nCnt == DISPBOX_TYFK ? WFS_CRD_SUPPLYBIN : WFS_CRD_RETAINBIN);
            // 卡单元中卡初始数目
            nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_INITCNT);
            lpWfsCrdUnit->ulInitialCount = (nCnt < 0 ? 0 : nCnt);
            // 卡单元中卡当前数目
            nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_COUNT);
            lpWfsCrdUnit->ulCount = (nCnt < 0 ? 0 : nCnt);
            // 卡单元进入回收箱中卡数目
            nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_RETCNT);
            lpWfsCrdUnit->ulRetainCount = (ULONG)(nCnt < 0 ? 0 : nCnt);
            // 阀值
            nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_THREH);
            lpWfsCrdUnit->ulThreshold = (ULONG)(nCnt < 0 ? 0 : nCnt);
            // 卡箱状态
            lpWfsCrdUnit->usStatus = ConvertCRDUnitInfoStatus(m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_STATUS));
            // 是否基于硬件传感器生成阀值事件
            nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_HARDSR);
            lpWfsCrdUnit->bHardwareSensor = (ULONG)(nCnt == 0 ? false : true);
            // 卡箱中卡类型
            nCnt = strlen(m_stCRDBoxList.nGetDispBoxCardType(usBoxNo));
            if (nCnt < 1)
            {
                lpWfsCrdUnit->lpszCardName = nullptr;
            } else
            {
                lpWfsCrdUnit->lpszCardName = new CHAR[nCnt + 1];
                memset(lpWfsCrdUnit->lpszCardName, 0x00, sizeof(nCnt + 1));
                memcpy(lpWfsCrdUnit->lpszCardName, m_stCRDBoxList.nGetDispBoxCardType(usBoxNo), nCnt);
            }
        }
    }

    return WFS_SUCCESS;
}

// EXEC: CRD: Dispense 发卡
HRESULT CXFS_IDC::CRD_DispenseCard(const LPWFSCRDDISPENSE lpDispense)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    /* Return WFS Event:
     * WFS_EXEE_CRD_CARDUNITERROR: 在分发操作中引起卡单元错误
     * WFS_USRE_CRD_CARDUNITTHRESHOLD: 卡单元已达到阀值
     * WFS_SRVE_CRD_MEDIAREMOVED: 卡已被用户拿走
     */

    INT nCRDRet = CRD_SUCCESS;
    INT nIDCRet = ERR_IDC_SUCCESS;
    INT nUnitNo = 0;
    INT nRet = 0;
    INT nCardCount = 0;  // 保存卡单元数目

    LPWFSCRDCUERROR lpCUError = nullptr;
    STCRDDEVSTATUS stStat;

    if (m_pDev == nullptr)   // 无效/设备未Open
    {
        Log(ThisModule, -1, "DispenseCard: CRD Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
            WFS_ERR_DEV_NOT_READY);
        return WFS_ERR_DEV_NOT_READY;
    }

    if (lpDispense != nullptr)
    {
        nUnitNo = lpDispense->usNumber;
    }

    // 卡箱序列号Check
    if (m_stCRDBoxList.bGetDispBoxIsHave(nUnitNo) != TRUE)
    {
        Log(ThisModule, -1, "DispenseCard: Check BoxNo[%d] Is Invalid, Return: %d.",
            nUnitNo, WFS_ERR_CRD_INVALIDCARDUNIT);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(nUnitNo, WFS_CRD_CARDUNITINVALID);

        return WFS_ERR_CRD_INVALIDCARDUNIT;
    }

    stStat.Clear();
    UpdateDevCRDStatus(&stStat);   // 更新一次状态

    // 检查卡箱状态
    nRet = stStat.GetUnitStatus((USHORT)nUnitNo);
    if (nRet == UNITINFO_STAT_INOP || nRet == UNITINFO_STAT_MISSING || nRet == UNITINFO_STAT_UNKNOWN) // 故障+不存在+无法确定状态
    {
        Log(ThisModule, -1, "DispenseCard: Check BoxNo[%d] Is Error[%d], Return: %d.",
            nUnitNo, nRet, WFS_ERR_CRD_CARDUNITERROR);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(nUnitNo, WFS_CRD_CARDUNITERROR);

        return WFS_ERR_CRD_CARDUNITERROR;
    }

    // 检查卡箱是否有卡
    if ((nCardCount = m_stCRDBoxList.GetBoxCardCount(nUnitNo)) < 1 ||
        stStat.GetUnitStatus(nUnitNo) == UNITINFO_STAT_EMPTY)
    {
        Log(ThisModule, -1, "DispenseCard: Check BoxNo[%d] Is Empty[BoxList:%d,Stat:%d], Return: %d.",
            nUnitNo, nCardCount, UNITINFO_STAT_EMPTY, WFS_ERR_CRD_CARDUNITERROR);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(nUnitNo, WFS_CRD_CARDUNITEMPTY);

        return WFS_ERR_CRD_CARDUNITERROR;
    }

    // 检查卡箱状态
    nRet = stStat.GetUnitStatus(nUnitNo);
    if (nRet == UNITINFO_STAT_INOP ||      // 故障
        nRet == UNITINFO_STAT_MISSING ||   // 不存在
        nRet == UNITINFO_STAT_UNKNOWN)     // 无法确定状态
    {
        Log(ThisModule, -1, "DispenseCard: Check BoxNo[%d] Is Empty[BoxList:%d,Stat:%d], Return: %d.",
            nUnitNo, nRet, UNITINFO_STAT_EMPTY, WFS_ERR_CRD_CARDUNITERROR);

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(nUnitNo, WFS_CRD_CARDUNITERROR);

        return WFS_ERR_CRD_CARDUNITERROR;
    }

    // 发卡前检查读卡器内是否有卡(读卡器命令)
    IDC_IDCSTAUTS enCardstatus = IDCSTATUS_UNKNOWN;
    INT nStatRet =  m_pDev->DetectCard(enCardstatus);
    if (nStatRet >= 0)
    {
        if (enCardstatus != IDCSTAUTS_NOCARD)
        {
            Log(ThisModule, -1, "DispenseCard: Check CardReader Is Have Card: ->DetectCard(), "
                                "CardStat = %d(0:无卡), Return: %d.",
                enCardstatus, WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }
    } else
    {
        Log(ThisModule, -1, "DispenseCard: Check CardReader Is Have Card: ->DetectCard() Fail,"
                            " ErrCode: %d|%s, Return: %d.",
            nStatRet, ProcessReturnCode(nStatRet), WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 下发发卡命令
    nCRDRet = m_pDev->DispenseCard(nUnitNo);
    if (nCRDRet != CRD_SUCCESS)
    {
        Log(ThisModule, -1, "DispenseCard: UnitNo[%d] : ->DispenseCard(%d) Fail. "
                            "RetCode: %d, Return: %d.",
            nUnitNo, nUnitNo, nCRDRet, CRDErr2XFSErrCode(nCRDRet));

        // 上报Event: WFS_EXEE_CRD_CARDUNITERROR
        CRD_FireUnitError_Pack(nUnitNo, WFS_CRD_CARDUNITERROR);

        return CRDErr2XFSErrCode(nCRDRet);
    }

    // 发卡计数-1
    nCardCount = (nCardCount - 1 < 0 ? 0 : nCardCount -1);
    m_stCRDBoxList.nSetDispBoxData(nUnitNo, nCardCount, DISPRWT_COUNT);
    SetDispBoxCfg(nUnitNo, nCardCount, DISPRWT_COUNT);

    return WFS_SUCCESS;
}

// 退卡
HRESULT CXFS_IDC::CRD_EjecdCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    STCRDDEVSTATUS stStat;

    HRESULT hIDCRet = EjectCard(0);
    HRESULT hCRDRet = IDCErr2CRDXFSErrCode(hIDCRet);

    Log(ThisModule, -1, "CRD_EjecdCard: 调用IDC EjectCard(0): ReturnIDC = %d, 转换 CRD = %d, Return: %d.",
        hIDCRet, hCRDRet, hCRDRet);

    if (hCRDRet == WFS_SUCCESS)
    {
        UpdateDevCRDStatus(&stStat);
        if (stStat.wDevice == DEVICE_STAT_ONLINE)
        {
            if (stStat.wCardMoveStat == CAREMOVE_STAT_IDRW_ENTR)    // 卡在读卡器插卡口
            {
                m_CRDWaitTaken = WTF_TAKEN;
            }
        }
    }

    return hCRDRet;
}

// 回收卡
HRESULT CXFS_IDC::CRD_RetainCard(const LPWFSCRDRETAINCARD lpRetainCard)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    if (lpRetainCard == nullptr)
    {
        Log(ThisModule, -1, "CRD_RetainCard: Input Param Is NULL, Return: %d.",
            WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    HRESULT hIDCRet = RetainCard();
    HRESULT hCRDRet = IDCErr2CRDXFSErrCode(hIDCRet);

    Log(ThisModule, -1, "CRD_RetainCard: 调用IDC RetainCard(0): ReturnIDC = %d, 转换 CRD = %d, Return: %d.",
        hIDCRet, hCRDRet, hCRDRet);

    m_CRDWaitTaken = WTF_NONE;

    return hCRDRet;
}

// 复位
HRESULT CXFS_IDC::CRD_Reset(const LPWFSCRDRESET lpResset)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    if (lpResset == nullptr)
    {
        Log(ThisModule, -1, "CRD_Reset: Input Param Is NULL, Return: %d.",
            WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    WORD wAction = WFS_IDC_NOACTION;
    switch(lpResset->usAction)
    {
        case WFS_CRD_EJECT :
            wAction = WFS_IDC_EJECT;
            break;
        case WFS_CRD_RETAIN :
            wAction = WFS_IDC_RETAIN;
            break;
        case WFS_CRD_NOACTION :
            wAction = WFS_IDC_NOACTION;
            break;
    }

    STCRDDEVSTATUS stStat;

    HRESULT hIDCRet = Reset(&wAction);
    HRESULT hCRDRet = IDCErr2CRDXFSErrCode(hIDCRet);

    Log(ThisModule, -1, "CRD_Reset: 调用IDC Reset(%d): ReturnIDC = %d, 转换 CRD = %d, Return: %d.",
        wAction, hIDCRet, hCRDRet, hCRDRet);

    if (hCRDRet == WFS_SUCCESS && wAction == WFS_IDC_EJECT)
    {
        UpdateDevCRDStatus(&stStat);
        if (stStat.wDevice == DEVICE_STAT_ONLINE)
        {
            if (stStat.wCardMoveStat == CAREMOVE_STAT_IDRW_ENTR)    // 卡在读卡器插卡口
            {
                m_CRDWaitTaken = WTF_TAKEN;
            }
        }
    }

    return hCRDRet;
}

// 设置灯状态
HRESULT CXFS_IDC::CRD_SetGuidanceLight(const LPWFSCRDSETGUIDLIGHT lpSetGuidLight)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    return WFS_ERR_UNSUPP_COMMAND;
}

// 设置卡箱信息
HRESULT CXFS_IDC::CRD_SetCardUnitInfo(const LPWFSCRDCUINFO lpCuInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    // 入参Check
    if (lpCuInfo == nullptr)
    {
        Log(ThisModule, -1, "Input Param == NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (lpCuInfo->usCount < 1)
    {
        Log(ThisModule, -1, "Input Param : usCount[%d] < 1, Not Set, Return: %d.",
            lpCuInfo->usCount, WFS_SUCCESS);
        return WFS_SUCCESS;
    }

    if (lpCuInfo->lppList == nullptr)
    {
        Log(ThisModule, -1, "Input Param: usCount[%d] > 0, lppList == NULL, Return: %d.",
            lpCuInfo->usCount, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    for (INT i = 0; i < lpCuInfo->usCount; i ++)
    {
        if (lpCuInfo->lppList[i] == nullptr)
        {
            Log(ThisModule, -1, "Input Param: usCount[%d] > 0, lppList[%d] == NULL, Return: %d.",
                lpCuInfo->usCount, i, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    if (lpCuInfo->usCount > m_stCRDBoxList.nGetDispBoxCount())
    {
        Log(ThisModule, -1, "Input Param: usCount = %d, Supp BoxCount = %d, Return: %d.",
            lpCuInfo->usCount, m_stCRDBoxList.nGetDispBoxCount(), WFS_ERR_SOFTWARE_ERROR);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    for(INT i = 0; i < lpCuInfo->usCount; i ++)
    {
        // 卡单元结构索引号(1~N)
        if (m_stCRDBoxList.bGetDispBoxIsHave(lpCuInfo->lppList[i]->usNumber) != TRUE)
        {
            Log(ThisModule, -1, "Input Param: lppList[%d].usNumber[%d] -> Box Not Supp, Return: %d.",
                i, lpCuInfo->lppList[i]->usNumber, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    // 设置卡箱信息并更新INI
    INT nCnt = 0;
    INT nNumber = 0;
    LPWFSCRDCARDUNIT lpCardUnit = nullptr;
    for(INT i = 0; i < lpCuInfo->usCount; i ++)
    {
        lpCardUnit = (LPWFSCRDCARDUNIT)lpCuInfo->lppList[i];
        nNumber = lpCardUnit->usNumber;

        // 卡单元结构索引号(1~N)
        m_stCRDBoxList.nSetDispBoxData(nNumber, lpCardUnit->usNumber, DISPRWT_NUMBER);
        // 卡单元类型
        nCnt = (lpCardUnit->usType == WFS_CRD_SUPPLYBIN ? DISPBOX_TYFK : DISPBOX_TYHS);
        m_stCRDBoxList.nSetDispBoxData(nNumber, nCnt, DISPRWT_TYPE);
        SetDispBoxCfg(nNumber, nCnt, DISPRWT_TYPE);
        // 卡单元中卡初始数目
        m_stCRDBoxList.nSetDispBoxData(nNumber, lpCardUnit->ulInitialCount, DISPRWT_INITCNT);
        SetDispBoxCfg(nNumber, lpCardUnit->ulInitialCount, DISPRWT_INITCNT);
        // 卡单元中卡当前数目
        m_stCRDBoxList.nSetDispBoxData(nNumber, lpCardUnit->ulCount, DISPRWT_COUNT);
        SetDispBoxCfg(nNumber, lpCardUnit->ulCount, DISPRWT_COUNT);
        // 卡单元进入回收箱中卡数目
        m_stCRDBoxList.nSetDispBoxData(nNumber, lpCardUnit->ulRetainCount, DISPRWT_RETCNT);
        SetDispBoxCfg(nNumber, lpCardUnit->ulRetainCount, DISPRWT_RETCNT);
        // 阀值
        m_stCRDBoxList.nSetDispBoxData(nNumber, lpCardUnit->ulThreshold, DISPRWT_THREH);
        SetDispBoxCfg(nNumber, lpCardUnit->ulThreshold, DISPRWT_THREH);
        // 是否基于硬件传感器生成阀值事件
        nCnt = (lpCardUnit->bHardwareSensor == false ? 0 : 1);
        m_stCRDBoxList.nSetDispBoxData(nNumber, nCnt, DISPRWT_HARDSR);
        SetDispBoxCfg(nNumber, nCnt, DISPRWT_HARDSR);
        // 卡箱中卡类型
        if (lpCardUnit->lpszCardName != nullptr && strlen(lpCardUnit->lpszCardName) > 0)
        {
            m_stCRDBoxList.nSetDispBoxData(nNumber, lpCardUnit->lpszCardName);
            SetDispBoxCfg(nNumber, lpCardUnit->lpszCardName, DISPRWT_CARDTP);
        } else
        {
            m_stCRDBoxList.nSetDispBoxData(nNumber, "");
            SetDispBoxCfg(nNumber, "", DISPRWT_CARDTP);
        }

        CRD_FireUnitInfoChanged_Pack();// 卡单元信息变化检查并上报Event:WFS_SRVE_CRD_CARDUNITINFOCHANGED
    }

    return WFS_SUCCESS;
}

// 激活/停用节能功能
HRESULT CXFS_IDC::CRD_PowerSaveControl(const LPWFSCRDPOWERSAVECONTROL lpPowerCtrl)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CRD_SUPP_CHECK();

    return WFS_ERR_UNSUPP_COMMAND;
}


void CXFS_IDC::InitCRDStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

}

void CXFS_IDC::InitCRDCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

}

// 发卡模块INI配置参数获取
int CXFS_IDC::InitCRDConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    CHAR    szIniAppName[256];

    //--------------------发卡模块INI配置参数获取--------------------
    // 是否支持启用发卡模块: 0不支持/1支持,缺省0
    m_stCRDINIConfig.bCRDDeviceSup = ((WORD)m_cXfsReg.GetValue("CRD_DEVICE_CFG", "CRDDeviceSup", (INT)0) == 1);

    if (m_stCRDINIConfig.bCRDDeviceSup == TRUE)
    {
        // 底层设备控制动态库名(发卡模块)
        strcpy(m_stCRDINIConfig.szCRDDeviceDllName, m_cXfsReg.GetValue("CRDDriverDllName", ""));

        // 发卡模块设备类型(缺省0)
        m_stCRDINIConfig.wCRDDeviceType = (WORD)m_cXfsReg.GetValue("CRD_DEVICE_CFG", "DeviceType", (INT)0);

        CHAR szKeyName[32];
        sprintf(szKeyName, "CRD_DEVICE_SET_%d", m_stCRDINIConfig.wCRDDeviceType);

        if (m_stCRDINIConfig.wCRDDeviceType == CRD_DEV_CRT591H)
        {
            // 1. 设备连接串,缺省(COM:/dev/ttyS0:E,8,1)
            strcpy(m_stCRDINIConfig.szCRDDeviceConList, m_cXfsReg.GetValue(szKeyName, "DeviceConList", "COM:/dev/ttyS0:115200,E,8,1"));
        }

        // --------读卡箱相关参数-------
        // 卡箱数目
        m_stCRDBoxList.usBoxCount = m_cXfsReg.GetValue("CRD_DISPBOX_CONFIG", "DispBoxCount", (INT)0);

        // 读指定卡箱箱相关参数
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            memset(szIniAppName, 0x00, sizeof(szIniAppName));
            sprintf(szIniAppName, "DISPBOX_%d", i + 1);

            // 卡箱顺序号
            m_stCRDBoxList.stDispBox[i].usBoxNo = i + 1;
            // 卡箱是否存在/使用
            m_stCRDBoxList.stDispBox[i].bIsHave =
                    (m_stCRDBoxList.stDispBox[i].usNumber = m_cXfsReg.GetValue(szIniAppName, "Useing", (INT)0) == 0 ? FALSE : TRUE);
            // 索引号[卡箱在UNITINFO中的唯一标识]
            m_stCRDBoxList.stDispBox[i].usNumber = m_cXfsReg.GetValue(szIniAppName, "BoxNumber", (INT)0);
            // 卡箱类型(0发卡箱/1回收箱)
            m_stCRDBoxList.stDispBox[i].usBoxType = m_cXfsReg.GetValue(szIniAppName, "BoxType", (INT)0);
            // 卡箱内卡类型
            strcpy(m_stCRDBoxList.stDispBox[i].szCardName, m_cXfsReg.GetValue(szIniAppName, "CardType", ""));
            // 初始卡数目
            m_stCRDBoxList.stDispBox[i].usInitCnt = m_cXfsReg.GetValue(szIniAppName, "CardInitCnt", (INT)0);
            // 当前卡数目
            m_stCRDBoxList.stDispBox[i].usCount = m_cXfsReg.GetValue(szIniAppName, "CardCount", (INT)0);
            // 进入回收箱数目
            m_stCRDBoxList.stDispBox[i].usRetainCnt = m_cXfsReg.GetValue(szIniAppName, "RetainCnt", (INT)0);
            // 报警阀值(LOW/HIGH)
            m_stCRDBoxList.stDispBox[i].usThreshold = m_cXfsReg.GetValue(szIniAppName, "Threshold", (INT)0);
            // 是否基于硬件传感器生成阀值事件(0否/1是)
            m_stCRDBoxList.stDispBox[i].bHardSensor = m_cXfsReg.GetValue(szIniAppName, "HardSensorEvent", (INT)0);
            m_stCRDBoxList.stDispBox[i].usStatus = UNITINFO_STAT_UNKNOWN; // 卡单元初始状态设置为无法确定
        }
        memcpy(&m_stCRDBoxListOld, &m_stCRDBoxList, sizeof(STDISPBOXLIST));
    }

    return 0;
}

// 发模块(CRD)-取状态
void CXFS_IDC::UpdateDevCRDStatus(LPSTCRDDEVSTATUS lpStat)
{
    THISMODULE(__FUNCTION__);

    INT nCRDRet = CRD_SUCCESS;
    STCRDDEVSTATUS stStatus;

    // 取状态(读卡器IDC与发卡器CRD同一连接方式)
    if((nCRDRet = m_pDev->GetDevStat(stStatus)) == ERR_CRD_UNSUP_CMD) // 不支持
    {
        if (m_pCRDDev == nullptr)
        {
            stStatus.wDevice = DEVICE_STAT_OFFLINE;
            return;
        }
        nCRDRet = m_pCRDDev->GetDevStat(stStatus);
    }

    switch(nCRDRet)
    {
        case ERR_CRD_UNSUP_CMD:     // 不支持
            m_CRDStatus.fwDevice = WFS_CRD_DEVOFFLINE;
            return;
        case ERR_CRD_DATA_INVALID:  // 无效数据
            m_CRDStatus.fwDevice = WFS_CRD_DEVHWERROR;
            return;
    }

    if (lpStat != nullptr)
    {
        memcpy(lpStat, &stStatus, sizeof(STCRDDEVSTATUS));
    }

    //memcpy(&m_stOldStatus, &stStatus, sizeof(STCRDDEVSTATUS));

    m_CRDStatus.fwDevice = ConvertCRDDeviceStatus(stStatus.wDevice);                // 设备状态
    m_CRDStatus.fwDispenser = ConvertCRDTransportStatus(stStatus.wDispensr);        // 单元状态
    m_CRDStatus.fwTransport = ConvertCRDDispensrStatus(stStatus.wTransport);        // 传输状态
    m_CRDStatus.fwMedia = ConvertCRDMediaStatus(stStatus.wMedia);                   // 介质状态
    m_CRDStatus.fwShutter = ConvertCRDShutterStatus(stStatus.wShutter);             // 门状态
    m_CRDStatus.wDevicePosition = ConvertCRDDevicePosStatus(stStatus.wDevicePos);   // 设备位置状态
    m_CRDStatus.wAntiFraudModule = ConvertCRDAntiFraudStatus(stStatus.wAntiFraudMod);// 反欺诈模块状态

    // 更新卡箱状态
    for (INT i = 1; i <= CARDBOX_COUNT; i ++)
    {
        m_stCRDBoxList.nSetDispBoxData(i, stStatus.GetUnitStatus(i), DISPRWT_STATUS);
        if (stStatus.GetUnitStatus(i) == UNITINFO_STAT_EMPTY)   // Sensor检测到卡单元空
        {
            m_stCRDBoxList.nSetDispBoxData(i, 0, DISPRWT_COUNT);// 设置对应卡单元计数=0
        }
    }

    UpdateRetain2BoxDisp();     // IDC回收计数状态更新到结构体  40-00-00-00(FT#0018)

    // ---------------------------Event 处理---------------------------
    if (m_CRDWaitTaken == WTF_TAKEN)    // 准备Taken事件
    {
        if (stStatus.wCardMoveStat == CAREMOVE_STAT_ALL_EMPTY)  // 插卡口无卡
        {
            CRD_FireMediaRemoved();
            m_CRDWaitTaken = WTF_NONE;
        }
    }
    // 设备位置变化
    if (m_CRDStatus.wDevicePosition != WFS_CRD_DEVICEPOSNOTSUPP &&
        m_CRDStatusOld.wDevicePosition != WFS_CRD_DEVICEPOSNOTSUPP)    // 非不支持状态
    {
        if (m_CRDStatus.wDevicePosition != m_CRDStatusOld.wDevicePosition)
        {
            LPWFSCRDDEVICEPOSITION lpWfsDevicePos = new WFSCRDDEVICEPOSITION();
            lpWfsDevicePos->wPosition = m_CRDStatus.wDevicePosition;
            CRD_FireDEvicePosition(lpWfsDevicePos);
        }
    }
    // 卡单元信息变化检查并上报Event:WFS_SRVE_CRD_CARDUNITINFOCHANGED
    CRD_FireUnitInfoChanged_Pack();
    // 卡单元阀值条件复合,上报Event:WFS_USRE_CRD_CARDUNITTHRESHOLD
    CRD_FireUnitThresHold_Pack();

    memcpy(&m_CRDStatusOld, &m_CRDStatus, sizeof(CWfsCRDStatus));   // 当前卡箱状态备份到OLD
}

// 发卡模块(CRD)-加载DevCRD动态库
bool CXFS_IDC::LoadCRDDevDll(LPCSTR ThisModule)
{
    if (m_pCRDDev == nullptr)
    {
        if (0 != m_pCRDDev.Load(m_stCRDINIConfig.szCRDDeviceDllName,
                                "CreateIDevCRD",
                                CRDDEVTYPE_CHG(m_stCRDINIConfig.wCRDDeviceType)))
        {
            Log(ThisModule, __LINE__,
                "加载发卡模块库失败: DriverDllName=%s, CRDDEVTYPE=%d. ReturnCode:%s",
                m_stCRDINIConfig.szCRDDeviceDllName, m_stCRDINIConfig.wCRDDeviceType,
                m_pCRDDev.LastError().toUtf8().constData());
            return false;
        }
    }

    return (m_pCRDDev != nullptr);
}

// 发模块(CRD)-CRD ErrCode 转换为 XFS ErrCode
long CXFS_IDC::CRDErr2XFSErrCode(INT nCRDErrCode)
{
    switch (nCRDErrCode)
    {
        case CRD_SUCCESS:                   return WFS_SUCCESS;
        case ERR_CRD_INSERT_TIMEOUT:        return WFS_ERR_TIMEOUT;             // CRM:进卡超时
        case ERR_CRD_USER_CANCEL:           return WFS_ERR_CANCELED;            // CRM:用户取消
        case ERR_CRD_COMM_ERR:              return WFS_ERR_CONNECTION_LOST;     // CRM:通讯错误
        case ERR_CRD_JAMMED:                return WFS_ERR_CRD_MEDIAJAM;        // CRM:堵卡
        case ERR_CRD_OFFLINE:               return WFS_ERR_CONNECTION_LOST;     // CRM:脱机
        case ERR_CRD_NOT_OPEN:              return WFS_ERR_HARDWARE_ERROR;      // CRM:没有打开
        case ERR_CRD_UNIT_FULL:             return WFS_ERR_CRD_RETAINBINFULL;   // CRM:卡箱满
        case ERR_CRD_UNIT_EMPTY:            return WFS_ERR_CRD_CARDUNITERROR;   // CRM:卡箱空
        case ERR_CRD_UNIT_NOTFOUND:         return WFS_ERR_CRD_INVALIDCARDUNIT; // CRM:卡箱不存在
        case ERR_CRD_HWERR:                 return WFS_ERR_HARDWARE_ERROR;      // CRM:硬件故障
        case ERR_CRD_STATUS_ERR:            return WFS_ERR_HARDWARE_ERROR;      // CRM:状态出错
        case ERR_CRD_UNSUP_CMD:             return WFS_ERR_UNSUPP_COMMAND;      // CRM:不支持的指令
        case ERR_CRD_PARAM_ERR:             return WFS_ERR_INVALID_DATA;        // CRM:参数错误
        case ERR_CRD_READTIMEOUT:           return WFS_ERR_HARDWARE_ERROR;      // CRM:读数据超时
        case ERR_CRD_WRITETIMEOUT:          return WFS_ERR_HARDWARE_ERROR;      // CRM:写数据超时
        case ERR_CRD_READERROR:             return WFS_ERR_HARDWARE_ERROR;      // CRM:读数据错
        case ERR_CRD_WRITEERROR:            return WFS_ERR_HARDWARE_ERROR;      // CRM:写数据错
        case ERR_CRD_DATA_INVALID:          return WFS_ERR_HARDWARE_ERROR;      // CRM:无效数据
        case ERR_CRD_LOAD_LIB:              return WFS_ERR_INTERNAL_ERROR;      // CRM:动态库错误
        case ERR_CRD_OTHER:                 return WFS_ERR_HARDWARE_ERROR;      // CRM:其他错误/未知错误
        default:                            return WFS_ERR_HARDWARE_ERROR;
    }
}

// IDC ErrCode 转换为 CRD XFS ErrCode
long CXFS_IDC::IDCErr2CRDXFSErrCode(long lCode)
{
    if (lCode >= -100)
    {
        return lCode;
    }

    switch (lCode)
    {
        case WFS_ERR_IDC_MEDIAJAM: return WFS_ERR_CRD_MEDIAJAM;
        case WFS_ERR_IDC_NOMEDIA : return WFS_ERR_CRD_NOMEDIA;
        case WFS_ERR_IDC_MEDIARETAINED: return WFS_ERR_CRD_MEDIARETAINED;
        case WFS_ERR_IDC_RETAINBINFULL: return WFS_ERR_CRD_RETAINBINFULL;
        case WFS_ERR_IDC_SHUTTERFAIL: return WFS_ERR_CRD_SHUTTERFAIL;
        case WFS_ERR_IDC_SECURITYFAIL: return WFS_ERR_HARDWARE_ERROR;
    }
}


// 设备状态转换为WFS格式
LONG CXFS_IDC::ConvertCRDDeviceStatus(WORD wStat)
{
    switch (wStat)
    {
        case DEVICE_STAT_ONLINE     : return WFS_CRD_DEVONLINE;         // 设备正常
        case DEVICE_STAT_OFFLINE    : return WFS_CRD_DEVOFFLINE;        // 设备脱机
        case DEVICE_STAT_POWEROFF   : return WFS_CRD_DEVPOWEROFF;       // 设备断电
        case DEVICE_STAT_NODEVICE   : return WFS_CRD_DEVNODEVICE;       // 设备不存在
        case DEVICE_STAT_HWERROR    : return WFS_CRD_DEVHWERROR;        // 设备故障
        case DEVICE_STAT_USERERROR  : return WFS_CRD_DEVUSERERROR;      // 设备存在,但有人阻止设备操作
        case DEVICE_STAT_BUSY       : return WFS_CRD_DEVBUSY;           // 设备读写中
        case DEVICE_STAT_FRAUDAT    : return WFS_CRD_DEVFRAUDATTEMPT;   // 设备存在,但有检测到欺诈企图
        case DEVICE_STAT_POTENTIAL  : return WFS_CRD_DEVPOTENTIALFRAUD; // 设备检测到欺诈企图但可继续使用,应用决定是否脱机
        default: return WFS_CRD_DEVOFFLINE;
    }
}

// 总单元状态转换为WFS格式
LONG CXFS_IDC::ConvertCRDDispensrStatus(WORD wStat)
{
    switch (wStat)
    {
        case DISP_STAT_OK           : return WFS_CRD_DISPCUOK;          // 所有单元正常
        case DISP_STAT_STATE        : return WFS_CRD_DISPCUSTATE;       // 1个/多个单元处于低/空/不工作状态,但仍有一个单元在工作
        case DISP_STAT_STOP         : return WFS_CRD_DISPCUSTOP;        // 所有单元故障/空/停止工作
        case DISP_STAT_UNKNOWN      : return WFS_CRD_DISPCUUNKNOWN;     // 无法确定单元状态
        default: return WFS_CRD_DISPCUUNKNOWN;
    }
}

// 传送模块状态转换为WFS格式
LONG CXFS_IDC::ConvertCRDTransportStatus(WORD wStat)
{
    switch (wStat)
    {
        case TRANS_STAT_OK          : return WFS_CRD_TPOK;              // 正常
        case TRANS_STAT_INOP        : return WFS_CRD_TPINOP;            // 传输不工作(故障/堵塞)
        case TRANS_STAT_UNKNOWN     : return WFS_CRD_TPUNKNOWN;         // 无法确定状态
        case TRANS_STAT_NOTSUPP     : return WFS_CRD_TPNOTSUPPORTED;    // 不支持状态报告
        default: return WFS_CRD_TPNOTSUPPORTED;
    }
}

// 介质状态转换为WFS格式
LONG CXFS_IDC::ConvertCRDMediaStatus(WORD wStat)
{
    switch (wStat)
    {
        case MEDIA_STAT_PRESENT     : return WFS_CRD_MEDIAPRESENT;      // 通道内有介质
        case MEDIA_STAT_NOTPRESENT  : return WFS_CRD_MEDIANOTPRESENT;   // 通道内无介质
        case MEDIA_STAT_JAMMED      : return WFS_CRD_MEDIAJAMMED;       // 通道内有介质且被夹住
        case MEDIA_STAT_NOTSUPP     : return WFS_CRD_MEDIANOTSUPP;      // 不支持检测介质状态
        case MEDIA_STAT_UNKNOWN     : return WFS_CRD_MEDIAUNKNOWN;      // 介质状态未知
        case MEDIA_STAT_EXITING     : return WFS_CRD_MEDIAEXITING;      // 介质在出口插槽(CRD专用)
        default: return WFS_CRD_MEDIAUNKNOWN;
    }
}

// 门状态转换为WFS格式
LONG CXFS_IDC::ConvertCRDShutterStatus(WORD wStat)
{
    switch (wStat)
    {
        case SHUTTER_STAT_CLOSED    : return WFS_CRD_SHTCLOSED;         // 关闭
        case SHUTTER_STAT_OPEN      : return WFS_CRD_SHTOPEN;           // 打开
        case SHUTTER_STAT_JAMMED    : return WFS_CRD_SHTJAMMED;         // 卡住
        case SHUTTER_STAT_UNKNOWN   : return WFS_CRD_SHTUNKNOWN;        // 未知
        case SHUTTER_STAT_NOTSUPP   : return WFS_CRD_SHTNOTSUPPORTED;   // 不支持状态查询
        default: return WFS_CRD_SHTUNKNOWN;
    }
}

// 指定设备位置转换为WFS格式
LONG CXFS_IDC::ConvertCRDDevicePosStatus(WORD wStat)
{
    switch (wStat)
    {
        case DEVPOS_STAT_INPOS      : return WFS_CRD_DEVICEINPOSITION;  // 设备处于正常工作位置
        case DEVPOS_STAT_NOTINPOS   : return WFS_CRD_DEVICENOTINPOSITION;// 设备不在正常工作位置
        case DEVPOS_STAT_UNKNOWN    : return WFS_CRD_DEVICEPOSUNKNOWN;  // 未知
        case DEVPOS_STAT_NOTSUPP    : return WFS_CRD_DEVICEPOSNOTSUPP;  // 不支持状态查询
        default: return WFS_CRD_DEVICEPOSUNKNOWN;
    }
}

// 反欺诈模块状态转换为WFS格式
LONG CXFS_IDC::ConvertCRDAntiFraudStatus(WORD wStat)
{
    switch (wStat)
    {
        case ANFRAUD_STAT_OK        : return WFS_CRD_AFMOK;             // 正常
        case ANFRAUD_STAT_INOP      : return WFS_CRD_AFMINOP;           // 不可用
        case ANFRAUD_STAT_DETECTED  : return WFS_CRD_AFMDEVICEDETECTED; // 检测到外部设备
        case ANFRAUD_STAT_UNKNOWN   : return WFS_CRD_AFMUNKNOWN;        // 未知
        case ANFRAUD_STAT_NOTSUPP   : return WFS_CRD_AFMNOTSUPP;        // 不支持状态查询
        default: return WFS_CRD_AFMUNKNOWN;
    }
}

// 单个单元状态转换为WFS格式
LONG CXFS_IDC::ConvertCRDUnitInfoStatus(WORD wStat)
{
    switch (wStat)
    {
        case UNITINFO_STAT_OK       : return WFS_CRD_STATCUOK;          // 正常   // 原注释标记放开 40-00-00-00(FT#0018)
        case UNITINFO_STAT_LOW      : return WFS_CRD_STATCULOW;         // 介质少
        case UNITINFO_STAT_EMPTY    : return WFS_CRD_STATCUEMPTY;       // 空
        case UNITINFO_STAT_INOP     : return WFS_CRD_STATCUINOP;        // 故障
        case UNITINFO_STAT_MISSING  : return WFS_CRD_STATCUMISSING;     // 不存在
        case UNITINFO_STAT_HIGH     : return WFS_CRD_STATCUHIGH;        // 回收单元将满
        case UNITINFO_STAT_FULL     : return WFS_CRD_STATCUFULL;        // 回收单元满
        case UNITINFO_STAT_UNKNOWN  : return WFS_CRD_STATCUUNKNOWN;     // 无法确定状态
        default: return WFS_CRD_STATCUUNKNOWN;
    }
}

// 设置INI中卡箱单元信息
int CXFS_IDC::SetDispBoxCfg(USHORT usBoxNo, ULONG ulData, USHORT usType)
{
    return SetDispBoxCfg(usBoxNo, (LPSTR)std::to_string(ulData).c_str(), usType);
}

int CXFS_IDC::SetDispBoxCfg(USHORT usBoxNo, LPSTR lpData, USHORT usType)
{
    CHAR    szIniAppName[256] = { 0x00 };

    sprintf(szIniAppName, "DISPBOX_%d", usBoxNo);

    switch(usType)
    {
        case DISPRWT_NUMBER     :       // 索引号
            m_cXfsReg.SetValue(szIniAppName, "BoxNumber", lpData);
            break;
        case DISPRWT_TYPE       :       // 箱类型
            m_cXfsReg.SetValue(szIniAppName, "BoxType", lpData);
            break;
        case DISPRWT_CARDTP       :     // 箱内卡类型
            m_cXfsReg.SetValue(szIniAppName, "CardType", lpData);
            break;
        case DISPRWT_INITCNT    :       // 初始数目
            m_cXfsReg.SetValue(szIniAppName, "CardInitCnt", lpData);
            break;
        case DISPRWT_COUNT      :       // 当前数目
            m_cXfsReg.SetValue(szIniAppName, "CardCount", lpData);
            break;
        case DISPRWT_RETCNT     :       // 进入回收数目
            m_cXfsReg.SetValue(szIniAppName, "RetainCnt", lpData);
            break;
        case DISPRWT_THREH      :       // 报警阀值(HIGH回收箱/LOW发卡箱)
            m_cXfsReg.SetValue(szIniAppName, "Threshold", lpData);
            break;
        case DISPRWT_HARDSR     :       // 是否基于硬件传感器生成阀值事件
            m_cXfsReg.SetValue(szIniAppName, "HardSensorEvent", lpData);
            break;
    }

    return 0;
}

// 更新回收计数和状态到卡单元结构体 40-00-00-00(FT#0018) 新增方法
int CXFS_IDC::UpdateRetain2BoxDisp()
{
    m_stCRDBoxList.nSetDispBoxData(m_stCRDBoxList.GetRetainNum(), m_Status.usCards, DISPRWT_COUNT);
    INT nRetainStat = UNITINFO_STAT_OK;
    switch(m_Status.fwRetainBin)
    {
        case WFS_IDC_RETAINBINOK: nRetainStat = UNITINFO_STAT_OK; break;
        case WFS_IDC_RETAINNOTSUPP: nRetainStat = UNITINFO_STAT_UNKNOWN; break;
        case WFS_IDC_RETAINBINFULL: nRetainStat = UNITINFO_STAT_FULL; break;
        case WFS_IDC_RETAINBINHIGH: nRetainStat = UNITINFO_STAT_HIGH; break;
    }
    m_stCRDBoxList.nSetDispBoxData(m_stCRDBoxList.GetRetainNum(), nRetainStat, DISPRWT_STATUS);
    return 0;
}

// 根据卡单元索引号赋值卡信息结构体
void CXFS_IDC::CardUnitInfoPack(LPWFSCRDCARDUNIT lpUnit, USHORT usBoxNo)
{
    INT nCnt = 0;

    if (lpUnit == nullptr)
    {
        lpUnit = new WFSCRDCARDUNIT();
        if (lpUnit == nullptr)
        {
            return;
        }
    }

    // 卡单元结构索引号(1~N)
    lpUnit->usNumber = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_NUMBER);
    // 卡单元类型
    nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_TYPE);
    lpUnit->usType = (nCnt == DISPBOX_TYFK ? WFS_CRD_SUPPLYBIN : WFS_CRD_RETAINBIN);
    // 卡单元中卡初始数目
    nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_INITCNT);
    lpUnit->ulInitialCount = (nCnt < 0 ? 0 : nCnt);
    // 卡单元中卡当前数目
    nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_COUNT);
    lpUnit->ulCount = (nCnt < 0 ? 0 : nCnt);
    // 卡单元进入回收箱中卡数目
    nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_RETCNT);
    lpUnit->ulRetainCount = (ULONG)(nCnt < 0 ? 0 : nCnt);
    // 阀值
    nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_THREH);
    lpUnit->ulThreshold = (ULONG)(nCnt < 0 ? 0 : nCnt);
    // 卡箱状态
    lpUnit->usStatus = ConvertCRDUnitInfoStatus(m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_STATUS));
    // 是否基于硬件传感器生成阀值事件
    nCnt = m_stCRDBoxList.nGetDispBoxData(usBoxNo, DISPRWT_HARDSR);
    lpUnit->bHardwareSensor = (ULONG)(nCnt == 0 ? false : true);
    // 卡箱中卡类型
    nCnt = strlen(m_stCRDBoxList.nGetDispBoxCardType(usBoxNo));
    if (nCnt < 1)
    {
        lpUnit->lpszCardName = nullptr;
    } else
    {
        lpUnit->lpszCardName = new CHAR[nCnt + 1];
        memset(lpUnit->lpszCardName, 0x00, sizeof(nCnt + 1));
        memcpy(lpUnit->lpszCardName, m_stCRDBoxList.nGetDispBoxCardType(usBoxNo), nCnt);
    }
}

// EVENT封装: WFS_USRE_CRD_CARDUNITTHRESHOLD
void CXFS_IDC::CRD_FireUnitThresHold_Pack()
{
    THISMODULE(__FUNCTION__);

    LPWFSCRDCARDUNIT lpUnit = nullptr;

    for (INT i = 0; i < DISPBOX_COUNT; i ++)
    {
        if (m_stCRDBoxList.stDispBox[i].bIsHave == TRUE)
        {
            USHORT usCount = m_stCRDBoxList.stDispBox[i].usCount;
            USHORT usThreshold = m_stCRDBoxList.stDispBox[i].usThreshold;
            if (usThreshold > 0 &&
                (usCount <= usThreshold) &&    // 阀值条件符合
                ((usThreshold - usCount) != m_stCRDBoxListOld.nThresholdDist[i]))
            {
                if (lpUnit != nullptr)
                {
                    delete lpUnit;
                    lpUnit = nullptr;
                }

                lpUnit = new WFSCRDCARDUNIT();
                if (lpUnit == nullptr)
                {
                    Log(ThisModule, -1, "lpUnit New(WFSCRDCARDUNIT) Result Is NULL, "
                                        "Not Report Event[WFS_USRE_CRD_CARDUNITTHRESHOLD].");
                    return;
                }

                CardUnitInfoPack(lpUnit, m_stCRDBoxList.stDispBox[i].usBoxNo);
                CRD_FireUnitTheshold(lpUnit);
                m_stCRDBoxListOld.nThresholdDist[i] = usThreshold - usCount;
            }
        }
    }
}

// EVENT封装: WFS_SRVE_CRD_CARDUNITINFOCHANGED
void CXFS_IDC::CRD_FireUnitInfoChanged_Pack()
{
    THISMODULE(__FUNCTION__);

    LPWFSCRDCARDUNIT lpUnit = nullptr;

    for (INT i = 0; i < DISPBOX_COUNT; i ++)
    {
        if (m_stCRDBoxList.stDispBox[i].bIsHave == TRUE)
        {
            if (memcmp(&(m_stCRDBoxList.stDispBox[i]), &(m_stCRDBoxListOld.stDispBox[i]),
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
                    Log(ThisModule, -1, "lpUnit New(WFSCRDCARDUNIT) Result Is NULL, "
                                        "Not Report Event[WFS_SRVE_CRD_CARDUNITINFOCHANGED].");
                    return;
                }

                CardUnitInfoPack(lpUnit, m_stCRDBoxList.stDispBox[i].usBoxNo);
                CRD_FireUnitInfoChanged(lpUnit);
                memcpy(&(m_stCRDBoxListOld.stDispBox[i]), &(m_stCRDBoxList.stDispBox[i]),
                       sizeof(STDISPPBOX));
            }
        }
    }
}

// EVENT封装: WFS_EXEE_CRD_CARDUNITERROR
void CXFS_IDC::CRD_FireUnitError_Pack(USHORT usBoxNo, LONG lCode)
{
    THISMODULE(__FUNCTION__);

    LPWFSCRDCUERROR lpCUError = new WFSCRDCUERROR();
    BOOL bNeed = false;
    INT nCnt = 0;

    if (lpCUError == nullptr)
    {
        Log(ThisModule, -1, "LPWFSCRDCUERROR New() Result Is NULL, Not Report Event[WFS_EXEE_CRD_CARDUNITERROR].");
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
            Log(ThisModule, -1, "Input Param[%d] Is NULL, Not Report Event[WFS_EXEE_CRD_CARDUNITERROR].");
            return;
    }

    if (bNeed == TRUE)  // 需要设定UNITINFO
    {
        lpCUError->lpCardUnit = new WFSCRDCARDUNIT();
        if (lpCUError->lpCardUnit == nullptr)
        {
            Log(ThisModule, -1, "LPWFSCRDCUERROR->lpCardUnit New(WFSCRDCARDUNIT) Result Is NULL, "
                                "Not Report Event[WFS_EXEE_CRD_CARDUNITERROR].");
            return;
        }        
        CardUnitInfoPack(lpCUError->lpCardUnit, usBoxNo);
    }

    CRD_FireUnitError(lpCUError);

    return;
}


// 上报 EVENT: WFS_SRVE_CRD_MEDIAREMOVED
void CXFS_IDC::CRD_FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CRD_MEDIAREMOVED, nullptr);
}

// 上报 EVENT: WFS_SRVE_CRD_MEDIADETECTED
void CXFS_IDC::CRD_FireMediaDetected(LPWFSCRDMEDIADETECTED lpMediaDet)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CRD_MEDIADETECTED, lpMediaDet);
}

// 上报 EVENT: WFS_USRE_CRD_CARDUNITTHRESHOLD
void CXFS_IDC::CRD_FireUnitTheshold(LPWFSCRDCARDUNIT lpCardUnit)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_CRD_CARDUNITTHRESHOLD, lpCardUnit);
}

// 上报 EVENT: WFS_SRVE_CRD_CARDUNITINFOCHANGED
void CXFS_IDC::CRD_FireUnitInfoChanged(LPWFSCRDCARDUNIT lpCardUnit)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CRD_CARDUNITINFOCHANGED, lpCardUnit);
}

// 上报 EVENT: WFS_EXEE_CRD_CARDUNITERROR
void CXFS_IDC::CRD_FireUnitError(LPWFSCRDCUERROR lpCardUnitError)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CRD_CARDUNITERROR, lpCardUnitError);
}

// 上报 EVENT: WFS_SRVE_CRD_DEVICEPOSITION
void CXFS_IDC::CRD_FireDEvicePosition(LPWFSCRDDEVICEPOSITION lpDevicePosition)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_CRD_DEVICEPOSITION, lpDevicePosition);
}


//--------------------------方法处理------------------------------

