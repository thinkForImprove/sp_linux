#include "BRMCONFIGPARAM.h"
#include "ILogWrite.h"
#include <assert.h>
#define THISFILE            "BRMCONFIGPARAM"        //记录日志的当前文件名
#define SEP_STR             " \t,"                  //配置文件多项数据的分隔字符
#define ADAPTER_DLL_NAME    "SimBRMAdapter.dll"     //默认适配器DLL文件名

int _tag_brm_config_param::ReadConfigMultipleItem(
LPCSTR pKeyName, LPCSTR pValueName, CMultiString &ms)
{
    char szBuf[1024];
    strcpy(szBuf, m_SPConfigFile.GetString(pKeyName, pValueName, ""));
    char *p = strtok(szBuf, SEP_STR);
    while (p != NULL)
    {
        ms.Add(p);
        p = strtok(NULL, SEP_STR);
    }
    return ms.GetCount();
}

int _tag_brm_config_param::LoadParam(LPCSTR lpszFileName)
{
    const char *ThisModule = "LoadParam";

    int iRet = m_SPConfigFile.Load(lpszFileName);
    if (iRet < 0)
    {
        Log(ThisModule, -1,
            "m_SPConfigFile.Load(%s) failed(iRet = %d)",
            lpszFileName, iRet);
        return iRet;
    }

    sAdapterDLLName = m_SPConfigFile.GetString("BRMInfo", "AdapterDLLName", ADAPTER_DLL_NAME);
    if (sAdapterDLLName.size() == 0)
    {
        Log(ThisModule, -1,
            "配置项AdapterDLLName未正确配置，长度为0");
        return WFS_ERR_INTERNAL_ERROR;
    }

    bExchangeReset = m_SPConfigFile.GetInt("BRMInfo", "ExchangeReset", 1);
    dwAutoRecoverInterval = m_SPConfigFile.GetInt("BRMInfo", "ErrorAutoResetInterval", 0);
    dwAutoRecoverNumber = m_SPConfigFile.GetInt("BRMInfo", "ErrorAutoResetNumber", 1);
    dwUpdateInterval = m_SPConfigFile.GetInt("BRMInfo", "UpdateInterval", 0);
    bErrStackNotEmpty = m_SPConfigFile.GetInt("BRMInfo", "TSNotEmptyToErr", 0);
    bRetractCountMode = m_SPConfigFile.GetInt("BRMInfo", "RetractCountType", 0);
    bEndExchangeAutoClearCount = m_SPConfigFile.GetInt("BRMInfo", "EndExchangeAutoClearCount", 0);
    bCashInBoxFullAccptStop = m_SPConfigFile.GetInt("BRMInfo", "CashInBoxFullAccptStop", 1);
    bSetCIRetractAfterReset = m_SPConfigFile.GetInt("BRMInfo", "SetCIRetractAfterReset", 0);
    bBeepWhenBoxInsertedOrTaken = m_SPConfigFile.GetInt("BRMInfo", "BeepWhenBoxInsertedOrTaken", 0);
    dwStopWhenStackerNotEmpty = m_SPConfigFile.GetInt("BRMInfo", "StopWhenStackerNotEmpty", 1);
    dwStopWhenTransportNotEmpty = m_SPConfigFile.GetInt("BRMInfo", "StopWhenTransportNotEmpty", 1);
    dwStopWhenPositionNotEmpty = m_SPConfigFile.GetInt("BRMInfo", "StopWhenPositionNotEmpty", 1);
    dwStopWhenShutterAbnormal = m_SPConfigFile.GetInt("BRMInfo", "StopWhenShutterAbnormal", 1);
    dwStopWhenSafeDoorOpen = m_SPConfigFile.GetInt("BRMInfo", "StopWhenSafeDoorOpen", 0);
    bCDMStatusBusyWhenCashInActive = m_SPConfigFile.GetInt("BRMInfo", "CDMStatusBusyWhenCashInActive", 0);
    bCheckDoorAbnormalBeforeCashIn = m_SPConfigFile.GetInt("BRMInfo", "CheckDoorAbnormalBeforeCashIn", 0);
    dwCfgNoteTypeCantDepositMode = m_SPConfigFile.GetInt("BRMInfo", "CfgNoteTypeCantDepositMode", 0);
    dwCloseShutterCountBeforeJammed = m_SPConfigFile.GetInt("BRMInfo", "CloseShutterCountBeforeJammed", 2);
    bCheckCSTSEmptyWhenRollBack   = m_SPConfigFile.GetInt("BRMInfo", "CheckCSTSEmptyWhenRollBack", 1);
    bCheckCSEmptyBeforeRetract      = m_SPConfigFile.GetInt("BRMInfo", "CheckCSEmptyBeforeRetract", 1);
    bCheckTSEmptyBeforeReject     = m_SPConfigFile.GetInt("BRMInfo", "CheckTSEmptyBeforeReject", 1);
    bGetInfoAvailableAfterCashOut = m_SPConfigFile.GetInt("BRMInfo", "GetInfoAvailableAfterCashOut", 0);
    bGetInfoAvailableAfterRetractCount = m_SPConfigFile.GetInt("BRMInfo", "GetInfoAvailableAfterRetractCount", 0);
    bGetInfoAvailableAfterCashIn = m_SPConfigFile.GetInt("BRMInfo", "GetInfoAvailableAfterCashIn", 0);
    bGetInfoAvailableAfterCashInEnd = m_SPConfigFile.GetInt("BRMInfo", "GetInfoAvailableAfterCashInEnd", 0);
    bIsSupportIllegalRetractIndex = m_SPConfigFile.GetInt("BRMInfo", "IsSupportIllegalRetractIndex", 0);
    bIsSupportStatusChangeListener = m_SPConfigFile.GetInt("BRMInfo", "IsSupportStatusChangeListener", 1);
    dwModeOfUseRecycleUnitsOfCashInStart = m_SPConfigFile.GetInt("BRMInfo", "ModeOfUseRecycleUnitsOfCashInStart", 0);
    dwCashUnitType = m_SPConfigFile.GetInt("BRMInfo", "CashUnitType", 0);
    dwCassClosedWhenSafeDoorOpen = m_SPConfigFile.GetInt("BRMInfo", "CassClosedWhenSafeDoorOpen", 0);
    dwCassOperWhenStatusChangeWaitTime = m_SPConfigFile.GetInt("BRMInfo", "CassOperWhenStatusChangeWaitTime", 0);
    bGenerRejectPosbyRetractCass = m_SPConfigFile.GetInt("BRMInfo", "GenerRejectPosbyRetractCass", 0);

    bResetOnOpen = m_SPConfigFile.GetInt("BRMInfo", "ResetOnOpen", 1);
    bAsynDownLoadFW = m_SPConfigFile.GetInt("BRMInfo", "AsynDownLoadFW", 0);
    dwMixEqualEmptyMode = m_SPConfigFile.GetInt("BRMInfo", "MixEqualEmptyMode", 0);

    bCashOutShutterControlBySPOnly = m_SPConfigFile.GetInt("BRMInfo", "CashOutShutterControlBySPOnly", 0);  //test#21

    ulSubDispsenseCount = (USHORT)m_SPConfigFile.GetInt("BRMInfo", "SubDispsenseCount", 100);
    if (ulSubDispsenseCount == 0) {
        ulSubDispsenseCount = 100;
    }

    //装载币种
    int i = 0;

    CMultiString msCurrencyList;
    ReadConfigMultipleItem("Currency", "Currency", msCurrencyList);
    if (msCurrencyList.GetCount() == 0)
    {
        Log(ThisModule, -1,
            "配置项[Currency]的Currency未正确配置，个数为0");
        return WFS_ERR_INTERNAL_ERROR;
    }

    lppCurrencyExp = new LPWFSCDMCURRENCYEXP[msCurrencyList.GetCount() + 1];
    memset(lppCurrencyExp, 0, sizeof(LPWFSCDMCURRENCYEXP) * (msCurrencyList.GetCount() + 1));

    for (int i = 0; i < msCurrencyList.GetCount(); i++)
    {
        CMultiString msCurrency;
        ReadConfigMultipleItem("Currency", msCurrencyList.GetAt(i), msCurrency);
        if (msCurrency.GetCount() < 2)
        {
            Log(ThisModule, -1,
                "配置项Currency未正确配置，项数(%d)小于2",
                msCurrency.GetCount());
            return WFS_ERR_INTERNAL_ERROR;
        }
        if (strlen(msCurrency.GetAt(0)) != 3)
        {
            Log(ThisModule, -1,
                "配置项Currency未正确配置，第1项(%s)长度不为3",
                msCurrency.GetAt(0));
            return WFS_ERR_INTERNAL_ERROR;
        }
        if (!isdigit(*msCurrency.GetAt(1)))
        {
            Log(ThisModule, -1,
                "配置项Currency未正确配置，第2项(%s)不为数字",
                msCurrency.GetAt(1));
            return WFS_ERR_INTERNAL_ERROR;
        }
        lppCurrencyExp[i] = new WFSCDMCURRENCYEXP;
        strcpy(lppCurrencyExp[i]->cCurrencyID, msCurrency.GetAt(0));
        lppCurrencyExp[i]->sExponent = (SHORT)atol(msCurrency.GetAt(1));
    }

    //装载钞币类型
    CMultiString msNoteTypeList;
    ReadConfigMultipleItem("MixHeader", "NoteTypes", msNoteTypeList);
    if (msNoteTypeList.GetCount() == 0)
    {
        Log(ThisModule, -1,
            "配置项[MixHeader]的NoteTypes未正确配置，个数为0");
        return WFS_ERR_INTERNAL_ERROR;
    }

    lpNoteTypeList = new WFSCIMNOTETYPELIST();
    lpNoteTypeList->usNumOfNoteTypes = (USHORT)msNoteTypeList.GetCount();
    if (lpNoteTypeList->usNumOfNoteTypes > 0)
    {
        lpNoteTypeList->lppNoteTypes = new LPWFSCIMNOTETYPE[lpNoteTypeList->usNumOfNoteTypes];
        memset(lpNoteTypeList->lppNoteTypes, 0,
               sizeof(LPWFSCIMNOTETYPE) * lpNoteTypeList->usNumOfNoteTypes);
    }

    for (i = 0; i < msNoteTypeList.GetCount(); i++)
    {
        CMultiString msNoteType;
        ReadConfigMultipleItem("MixHeader", msNoteTypeList.GetAt(i), msNoteType);
        if (msNoteType.GetCount() < 4)
        {
            Log(ThisModule, -1,
                "配置项MixHeader.%s未正确配置，项数(%d)小于4",
                msNoteTypeList.GetAt(i), msNoteType.GetCount());
            return WFS_ERR_INTERNAL_ERROR;
        }
        if (strlen(msNoteType.GetAt(0)) != 3)
        {
            Log(ThisModule, -1,
                "配置项MixHeader.%s未正确配置，第1项(%s)长度不为3",
                msNoteTypeList.GetAt(i), msNoteType.GetAt(0));
            return WFS_ERR_INTERNAL_ERROR;
        }
        WFSCIMNOTETYPE NoteType;
        NoteType.usNoteID = (USHORT)atol(msNoteTypeList.GetAt(i));
        strcpy(NoteType.cCurrencyID, msNoteType.GetAt(0));
        NoteType.ulValues = atol(msNoteType.GetAt(1));
        NoteType.usRelease = (USHORT)atol(msNoteType.GetAt(2));
        NoteType.bConfigured = atol(msNoteType.GetAt(3));
        if (NoteType.usNoteID == 0)
        {
            Log(ThisModule, -1,
                "配置项MixHeader未正确配置，第%d项ID(%s)为0",
                i + 1, msNoteTypeList.GetAt(i));
            return WFS_ERR_INTERNAL_ERROR;
        }

        if (NoteType.ulValues == 0)
        {
            Log(ThisModule, -1,
                "配置项MixHeader.%s未正确配置，面值(%d)为0",
                msNoteTypeList.GetAt(i), NoteType.ulValues);
            return WFS_ERR_INTERNAL_ERROR;
        }
        lpNoteTypeList->lppNoteTypes[i] = new WFSCIMNOTETYPE();
        *lpNoteTypeList->lppNoteTypes[i] = NoteType;
    }

    //检验NOTEIDLIST的币种在币种列中已定义
    assert(lpNoteTypeList != NULL);
    assert(lppCurrencyExp != NULL);
    for (i = 0; (USHORT)i < lpNoteTypeList->usNumOfNoteTypes; i++)
    {
        LPWFSCIMNOTETYPE pNoteType = lpNoteTypeList->lppNoteTypes[i];
        assert(pNoteType != NULL);
        LPWFSCDMCURRENCYEXP *lppCurrencyExpTemp = lppCurrencyExp;
        while (*lppCurrencyExpTemp != NULL)
        {
            if (memcmp(pNoteType->cCurrencyID, (*lppCurrencyExpTemp)->cCurrencyID, 3) == 0)
                break;
            lppCurrencyExpTemp++;
        }
        if (*lppCurrencyExpTemp == NULL)
        {
            Log(ThisModule, -1,
                "第%d个NoteType配置错误：币种(%.3s)未在币种指数列表中定义",
                i + 1, pNoteType->cCurrencyID);
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    return 0;
}

void _tag_brm_config_param::SaveNoteTypeList()
{
    assert(lpNoteTypeList != NULL);
    char buf[128];
    for (USHORT i = 0; i < lpNoteTypeList->usNumOfNoteTypes; i++)
    {
        LPWFSCIMNOTETYPE pNoteType = lpNoteTypeList->lppNoteTypes[i];
        assert(pNoteType != NULL);
        char arycNoteID[20];
        sprintf(arycNoteID, "%hd", pNoteType->usNoteID);
        sprintf(buf, "%.3s,%ld,%hd,%d",
                pNoteType->cCurrencyID, pNoteType->ulValues,
                pNoteType->usRelease, pNoteType->bConfigured);
        m_SPConfigFile.SetString("MixHeader", arycNoteID, buf);
    }
}

void _tag_brm_config_param::Clear()
{
    bExchangeReset = FALSE;
    dwUpdateInterval = 0;
    dwAutoRecoverInterval = 0;
    dwAutoRecoverNumber = 1;
    bErrStackNotEmpty = FALSE;
    bRetractCountMode = FALSE;
    bEndExchangeAutoClearCount = FALSE;
    bCashInBoxFullAccptStop = TRUE;
    bSetCIRetractAfterReset = FALSE;
    bBeepWhenBoxInsertedOrTaken = FALSE;
    dwModeOfUseRecycleUnitsOfCashInStart = 0;
    if (lppCurrencyExp != NULL)
    {
        LPWFSCDMCURRENCYEXP *p = lppCurrencyExp;
        while (*p != NULL)
        {
            delete *p;
            p++;
        }
        delete lppCurrencyExp;
        lppCurrencyExp = NULL;
    }
    if (lpNoteTypeList != NULL)
    {
        for (USHORT i = 0; i < lpNoteTypeList->usNumOfNoteTypes; i++)
        {
            delete lpNoteTypeList->lppNoteTypes[i];
        }
        delete [] lpNoteTypeList->lppNoteTypes;
        delete lpNoteTypeList;
        lpNoteTypeList = NULL;
    }
}
