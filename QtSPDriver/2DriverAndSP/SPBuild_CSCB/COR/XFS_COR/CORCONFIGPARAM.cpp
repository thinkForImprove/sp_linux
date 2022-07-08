#include "CORCONFIGPARAM.h"

int _tag_cor_config_param::LoadParam(LPCSTR lpszFileName)
{
    //const char *ThisModule = "LoadParam";
    THISMODULE(__FUNCTION__);

    int iRet;
    ULONG ulValues[MAX_COINCYLINDER_NUM + 1];

    m_bLoad = m_configfile.LoadINIFile(lpszFileName);
    if (m_bLoad == false)
    {
        Log(ThisModule, -1,
            "m_configfile.LoadINIFile(%s) failed",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // Move to section[DeviceSettings].
    CINIReader cINI = m_configfile.GetReaderSection("DeviceSettings");
    sDevDllName = (LPCSTR)cINI.GetValue("DevDllName", "");
    sPortMode = (LPCSTR)cINI.GetValue("PortMode", "");
    std::string strCylinderNO = (LPCSTR)cINI.GetValue("CylinderNO", "");
    iRet = ConvertMultipleNumber(strCylinderNO.c_str(), ulCylinderNO, MAX_COINCYLINDER_NUM);
    if (iRet < MAX_COINCYLINDER_NUM)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "DeviceSettings", "CylinderNO",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // Move to section[InitValue].
    cINI = m_configfile.GetReaderSection("InitValue");
    std::string strInitialCount = (LPCSTR)cINI.GetValue("InitialCount", "");
    iRet = ConvertMultipleNumber(strInitialCount.c_str(), ulInitialCount, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "InitialCount",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    std::string strCashInCount = (LPCSTR)cINI.GetValue("CashInCount", "");
    iRet = ConvertMultipleNumber(strCashInCount.c_str(), ulCashInCount, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "CashInCount",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    std::string strCount = (LPCSTR)cINI.GetValue("Count", "");
    iRet = ConvertMultipleNumber(strCount.c_str(), ulCount, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "Count",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    std::string strRejectCount = (LPCSTR)cINI.GetValue("RejectCount", "");
    iRet = ConvertMultipleNumber(strRejectCount.c_str(), ulRejectCount, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "RejectCount",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    std::string strMinimum = (LPCSTR)cINI.GetValue("Minimum", "");
    iRet = ConvertMultipleNumber(strMinimum.c_str(), ulMinimum, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "Minimum",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    std::string strMaximum = (LPCSTR)cINI.GetValue("Maximum", "");
    iRet = ConvertMultipleNumber(strMaximum.c_str(), ulMaximum, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "Maximum",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    std::string strPHMaximum = (LPCSTR)cINI.GetValue("PHMaximum", "");
    iRet = ConvertMultipleNumber(strPHMaximum.c_str(), ulPHMaximum, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "PHMaximum",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    memset(ulValues, 0, sizeof(ulValues));
    std::string strCashValue = (LPCSTR)cINI.GetValue("CashValue", "");
    iRet = ConvertMultipleNumber(strCashValue.c_str(), ulValues, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "CashValue",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }
    for (int i = 0; i < MAX_COINCYLINDER_NUM + 1; i++)
    {
        iCashValue[i] = (int32_t)ulValues[i];
    }

    memset(ulValues, 0, sizeof(ulValues));
    std::string strCoinCode = (LPCSTR)cINI.GetValue("CoinCode", "");
    iRet = ConvertMultipleNumber(strCoinCode.c_str(), ulValues, MAX_COINCYLINDER_NUM + 1);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "CoinCode",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }
    for (int i = 0; i < MAX_COINCYLINDER_NUM + 1; i++)
    {
        iCoinCode[i] = (int32_t)ulValues[i];
    }

    std::string strNoteNumber = (LPCSTR)cINI.GetValue("NoteNumber", "");
    CMultiString multiNoteList;
    iRet = SplitMultipleItems(strNoteNumber.c_str(), multiNoteList, SEP_STR_COMMA);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "NoteNumber",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }
    for (int i = 0; i < MAX_COINCYLINDER_NUM + 1; i++)
    {
        std::string strNoteID = multiNoteList.GetAt(i);
        CMultiString multiNoteID;
        iRet = SplitMultipleItems(strNoteID.c_str(), multiNoteID, SEP_STR_SEMICOLON);
        if (iRet <= 0)
        {
            Log(ThisModule, -1,
                "invalid config item([%s] %s) in %s",
                "InitValue", "NoteNumber",
                lpszFileName);
            return WFS_ERR_INTERNAL_ERROR;
        }

        lpNoteNumberListArray[i] = new WFSCIMNOTENUMBERLIST();
        lpNoteNumberListArray[i]->usNumOfNoteNumbers = iRet;
        lpNoteNumberListArray[i]->lppNoteNumber = new LPWFSCIMNOTENUMBER[iRet];
        memset(lpNoteNumberListArray[i]->lppNoteNumber, 0, sizeof(LPWFSCIMNOTENUMBER) * iRet);
        for (int j = 0; j < multiNoteID.GetCount(); j++)
        {
            std::string strIDCount = multiNoteID.GetAt(j);
            CMultiString multiIDCount;
            iRet = SplitMultipleItems(strIDCount.c_str(), multiIDCount, SEP_STR_COLON);
            if (iRet != 2)
            {
                Log(ThisModule, -1,
                    "invalid config item([%s] %s) in %s",
                    "InitValue", "NoteNumber",
                    lpszFileName);
                return WFS_ERR_INTERNAL_ERROR;
            }

            LPWFSCIMNOTENUMBER lpNoteNumber = new WFSCIMNOTENUMBER();
            lpNoteNumberListArray[i]->lppNoteNumber[j] = lpNoteNumber;
            lpNoteNumber->usNoteID = atoi(multiIDCount.GetAt(0));
            lpNoteNumber->ulCount = atoi(multiIDCount.GetAt(1));
        }
    }

    // Cash Unit Identifier.
    multiStrUnitID.Clear();
    std::string strUnitID = (LPCSTR)cINI.GetValue("UnitID", "");
    iRet = SplitMultipleItems(strUnitID.c_str(), multiStrUnitID, SEP_STR_COMMA);
    if (iRet < MAX_COINCYLINDER_NUM + 1)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "InitValue", "UnitID",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // Move to section[UserSettings].
    cINI = m_configfile.GetReaderSection("UserSettings");
    iCoinInTimeOut = (int32_t)cINI.GetValue("CoinInTimeOut", 60);

    // Move to section[CustomizedSettings].
    cINI = m_configfile.GetReaderSection("CustomizedSettings");
    iStartExchangeIgnoreCUNumberList = (int32_t)cINI.GetValue("StartExchangeIgnoreCUNumberList", 1);
    iDiscardCheck_ulCashBox = (int32_t)cINI.GetValue("DiscardCheck_ulCashBox", 1);

    // Move to section[BankNoteTypes].
    cINI = m_configfile.GetReaderSection("BankNoteTypes");
    std::string strNoteTypeList = (LPCSTR)cINI.GetValue("NoteTypeList", "");
    ULONG ulNoteIDs[MAX_SUPP_COIN_TYPE_NUM];
    memset(ulNoteIDs, 0, sizeof(ulNoteIDs));
    iRet = ConvertMultipleNumber(strNoteTypeList.c_str(), ulNoteIDs, MAX_SUPP_COIN_TYPE_NUM);
    if (iRet != MAX_SUPP_COIN_TYPE_NUM)
    {
        Log(ThisModule, -1,
            "invalid config item([%s] %s) in %s",
            "BankNoteTypes", "NoteTypeList",
            lpszFileName);
        return WFS_ERR_INTERNAL_ERROR;
    }

    lpNoteTypeList = new WFSCIMNOTETYPELIST;
    memset(lpNoteTypeList, 0, sizeof(WFSCIMNOTETYPELIST));
    lpNoteTypeList->usNumOfNoteTypes = MAX_SUPP_COIN_TYPE_NUM;
    lpNoteTypeList->lppNoteTypes = new LPWFSCIMNOTETYPE[MAX_SUPP_COIN_TYPE_NUM];
    memset(lpNoteTypeList->lppNoteTypes, 0, sizeof(LPWFSCIMNOTETYPE) * MAX_SUPP_COIN_TYPE_NUM);
    char cNoteID[16];
    for (int i = 0; i < MAX_SUPP_COIN_TYPE_NUM; i++)
    {
        CMultiString multiStr;
        usNoteIDs[i] = ulNoteIDs[i];
        lpNoteTypeList->lppNoteTypes[i] = new WFSCIMNOTETYPE();
        memset(lpNoteTypeList->lppNoteTypes[i], 0, sizeof(WFSCIMNOTETYPE));
        memset(cNoteID, 0, sizeof(cNoteID));
        sprintf(cNoteID, "NOTE%d", usNoteIDs[i]);
        std::string strNoteType = (LPCSTR)cINI.GetValue(cNoteID, "");
        if (strNoteType.size() == 0)
        {
            Log(ThisModule, -1,
                "invalid config item([%s] %s) in %s",
                "BankNoteTypes", cNoteID,
                lpszFileName);
            return WFS_ERR_INTERNAL_ERROR;
        }
        iRet = SplitMultipleItems(strNoteType.c_str(), multiStr);
        if (iRet != MAX_COIN_TYPE_FIELD_NUM)
        {
            Log(ThisModule, -1,
                "invalid config item([%s] %s) in %s",
                "BankNoteTypes", cNoteID,
                lpszFileName);
            return WFS_ERR_INTERNAL_ERROR;
        }

        // NoteID
        lpNoteTypeList->lppNoteTypes[i]->usNoteID = usNoteIDs[i];
        // Currency ID.
        strcpy(lpNoteTypeList->lppNoteTypes[i]->cCurrencyID, multiStr.GetAt(0));
        // The value of single item.
        lpNoteTypeList->lppNoteTypes[i]->ulValues = atoi(multiStr.GetAt(1));
        iRet = ConvertNoteRelease(multiStr.GetAt(2));
        if (iRet < 0)
        {
            Log(ThisModule, -1,
                "invalid config item([%s] %s) in %s",
                "BankNoteTypes", cNoteID,
                lpszFileName);
            return WFS_ERR_INTERNAL_ERROR;
        }
        // Release
        lpNoteTypeList->lppNoteTypes[i]->usRelease= iRet;
        lpNoteTypeList->lppNoteTypes[i]->bConfigured = atoi(multiStr.GetAt(3));
    }

    // CDM currency exp.
    lppCurrencyExp = new LPWFSCDMCURRENCYEXP[2];
    memset(lppCurrencyExp, 0, sizeof(LPWFSCDMCURRENCYEXP) * 2);

    lppCurrencyExp[0] = new WFSCDMCURRENCYEXP;
    strcpy(lppCurrencyExp[0]->cCurrencyID, "CNY");
    lppCurrencyExp[0]->sExponent = -2;

    // CDM mix type.
    lppMixType = new LPWFSCDMMIXTYPE[2];
    memset(lppMixType, 0, sizeof(LPWFSCDMMIXTYPE) * 2);

    lppMixType[0] = new WFSCDMMIXTYPE;
    memset(lppMixType[0], 0, sizeof(WFSCDMMIXTYPE));
    lppMixType[0]->usMixNumber = WFS_CDM_MIX_MINIMUM_NUMBER_OF_BILLS;
    lppMixType[0]->usMixType = WFS_CDM_MIXALGORITHM;
    lppMixType[0]->usSubType = WFS_CDM_MIX_MINIMUM_NUMBER_OF_BILLS;
    lppMixType[0]->lpszName = cEmptyString;

    return WFS_SUCCESS;
}

void _tag_cor_config_param::Clear()
{
    for (int i = 0; i < MAX_COINCYLINDER_NUM + 1; i++)
    {
        if (lpNoteNumberListArray[i] != NULL)
        {
            if (lpNoteNumberListArray[i]->lppNoteNumber != NULL)
            {
                for (int j = 0; j < lpNoteNumberListArray[i]->usNumOfNoteNumbers; j++)
                {
                    if (lpNoteNumberListArray[i]->lppNoteNumber[j] != NULL)
                    {
                        delete lpNoteNumberListArray[i]->lppNoteNumber[j];
                    }
                }
                delete [] lpNoteNumberListArray[i]->lppNoteNumber;
            }

            delete lpNoteNumberListArray[i];
            lpNoteNumberListArray[i] = NULL;
        }
    }

    if (lpNoteTypeList != NULL)
    {
        if (lpNoteTypeList->lppNoteTypes != NULL)
        {
            for (int i = 0; i < lpNoteTypeList->usNumOfNoteTypes; i++)
            {
                if (lpNoteTypeList->lppNoteTypes[i] != NULL)
                {
                    delete lpNoteTypeList->lppNoteTypes[i];
                }
            }
            delete [] lpNoteTypeList->lppNoteTypes;
        }
        delete lpNoteTypeList;
        lpNoteTypeList = NULL;
    }

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

    if (lppMixType != NULL)
    {
        LPWFSCDMMIXTYPE *p = lppMixType;
        while (*p != NULL)
        {
            delete *p;
            p++;
        }
        delete lppMixType;
        lppMixType = NULL;
    }
}

void _tag_cor_config_param::LogCashUnitCount(LPCSTR action, LPCSTR timing)
{
    THISMODULE(__FUNCTION__);

    char cBuf[1024];
    memset(cBuf, 0, sizeof(cBuf));
    MakeMultipleNumberValue(cBuf, ulCount, MAX_COINCYLINDER_NUM + 1);
    Log(ThisModule, -1,
        "cash unit count([%s] %s):%s",
        action,
        timing,
        cBuf);
}

int _tag_cor_config_param::SplitMultipleItems(LPCSTR pValue, CMultiString &ms, string separator)
{
    char szBuf[1024];
    strcpy(szBuf, pValue);
    char *p = strtok(szBuf, separator.c_str());
    while (p != NULL)
    {
        ms.Add(p);
        p = strtok(NULL, separator.c_str());
    }
    return ms.GetCount();
}

int _tag_cor_config_param::ConvertMultipleNumber(LPCSTR pValue, ULONG* ulValues, USHORT usCount)
{
    int iRet = 0;
    CMultiString multiStr;
    if ((pValue == NULL) || (strlen(pValue) == 0))
    {
        return -1;
    }

    iRet = SplitMultipleItems(pValue, multiStr);
    if ((usCount != 0) && (iRet > usCount))
    {
        iRet = usCount;
    }

    for (int i = 0; i < iRet; i++)
    {
        ulValues[i] = atoi(multiStr.GetAt(i));
    }

    return iRet;
}

void _tag_cor_config_param::MakeMultipleNumberValue(char* pValue, ULONG* ulValues, USHORT usCount)
{
    if (usCount < 1)
    {
        return;
    }

    char* pStartPos = NULL;
    int count = sprintf(pValue, "%ld", ulValues[0]);
    for (int i = 1; i < usCount; i++)
    {
        pStartPos = pValue + count;
        count += sprintf(pStartPos, ",%ld", ulValues[i]);
    }

    // append end character.
    pValue[count] = '\0';
}

void _tag_cor_config_param::MakeNoteNumberList(char* buf, int size)
{
    LPWFSCIMNOTENUMBERLIST lpNoteNumberList = NULL;
    LPWFSCIMNOTENUMBER lpNoteNumber = NULL;
    char* pStartPos;
    int len = 0;
    memset(buf, 0, size);
    pStartPos = buf;
    for (int i = 0; i < MAX_COINCYLINDER_NUM + 1; i++)
    {
        lpNoteNumberList = lpNoteNumberListArray[i];
        for (int j = 0; j < lpNoteNumberList->usNumOfNoteNumbers; j++)
        {
            lpNoteNumber = lpNoteNumberList->lppNoteNumber[j];
            pStartPos = buf + len;
            len += sprintf(pStartPos, "%d%s%d", lpNoteNumber->usNoteID,
                           SEP_STR_COLON, lpNoteNumber->ulCount);
            if (j < (lpNoteNumberList->usNumOfNoteNumbers - 1))
            {
                pStartPos = buf + len;
                len += sprintf(pStartPos, "%s", SEP_STR_SEMICOLON);
            }
        }

        if (i < MAX_COINCYLINDER_NUM)
        {
            pStartPos = buf + len;
            len += sprintf(pStartPos, "%s", SEP_STR_COMMA);
        }
    }
}

USHORT _tag_cor_config_param::ConvertNoteRelease(LPCSTR pValue)
{
    USHORT usRet;
    if ((pValue == NULL) || (strlen(pValue) != 1))
    {
        return -1;
    }

    if ((pValue[0] < 'A') || (pValue[0] > 'Z'))
    {
        usRet = 0;
    }
    else
    {
        usRet = pValue[0] - 'A' + 1;
    }

    return usRet;
}

void _tag_cor_config_param::SaveCashUnitInfo(int mode)
{
    CINIWriter cINI = m_configfile.GetWriterSection("InitValue");
    char cBuf[1024];
    memset(cBuf, 0, sizeof(cBuf));

    switch (mode)
    {
    case SAVE_MODE_CIM_COUNT: // cash unit count relative info.
    {
        MakeMultipleNumberValue(cBuf, ulCashInCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("CashInCount", cBuf);
        MakeMultipleNumberValue(cBuf, ulCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Count", cBuf);
        MakeMultipleNumberValue(cBuf, ulMaximum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Maximum", cBuf);
        MakeMultipleNumberValue(cBuf, ulPHMaximum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("PHMaximum", cBuf);
        MakeNoteNumberList(cBuf, sizeof(cBuf));
        cINI.SetValue("NoteNumber", cBuf);
    }
        break;
    case SAVE_MODE_CDM_COUNT:
    {
        MakeMultipleNumberValue(cBuf, ulInitialCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("InitialCount", cBuf);
        MakeMultipleNumberValue(cBuf, ulCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Count", cBuf);
        MakeMultipleNumberValue(cBuf, ulRejectCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("RejectCount", cBuf);
        MakeMultipleNumberValue(cBuf, ulMinimum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Minimum", cBuf);
        MakeMultipleNumberValue(cBuf, ulMaximum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Maximum", cBuf);
        MakeMultipleNumberValue(cBuf, ulPHMaximum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("PHMaximum", cBuf);
        MakeNoteNumberList(cBuf, sizeof(cBuf));
        cINI.SetValue("NoteNumber", cBuf);
    }
        break;
    case SAVE_MODE_CIM_EXCHANGE: // end exchange.
    {
        MakeMultipleNumberValue(cBuf, ulCashInCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("CashInCount", cBuf);
        MakeMultipleNumberValue(cBuf, ulCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Count", cBuf);
        MakeMultipleNumberValue(cBuf, ulMaximum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Maximum", cBuf);
        MakeMultipleNumberValue(cBuf, ulPHMaximum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("PHMaximum", cBuf);
        MakeNoteNumberList(cBuf, sizeof(cBuf));
        cINI.SetValue("NoteNumber", cBuf);
    }
        break;
    case SAVE_MODE_CDM_EXCHANGE:
    {
        MakeMultipleNumberValue(cBuf, ulInitialCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("InitialCount", cBuf);
        MakeMultipleNumberValue(cBuf, ulCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Count", cBuf);
        MakeMultipleNumberValue(cBuf, ulRejectCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("RejectCount", cBuf);
        MakeMultipleNumberValue(cBuf, ulMinimum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Minimum", cBuf);
        MakeMultipleNumberValue(cBuf, ulMaximum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Maximum", cBuf);
        MakeMultipleNumberValue(cBuf, ulPHMaximum, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("PHMaximum", cBuf);
        MakeNoteNumberList(cBuf, sizeof(cBuf));
        cINI.SetValue("NoteNumber", cBuf);
    }
        break;
    case SAVE_MODE_CIM_CASHIN: // cash in
    {
        MakeMultipleNumberValue(cBuf, ulInitialCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("InitialCount", cBuf);
        MakeMultipleNumberValue(cBuf, ulCashInCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("CashInCount", cBuf);
        MakeMultipleNumberValue(cBuf, ulCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Count", cBuf);
        MakeNoteNumberList(cBuf, sizeof(cBuf));
        cINI.SetValue("NoteNumber", cBuf);
    }
        break;
    case SAVE_MODE_CDM_CASHOUT: // dispense/count
    {
        MakeMultipleNumberValue(cBuf, ulCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("Count", cBuf);
        MakeMultipleNumberValue(cBuf, ulRejectCount, MAX_COINCYLINDER_NUM + 1);
        cINI.SetValue("RejectCount", cBuf);
        MakeNoteNumberList(cBuf, sizeof(cBuf));
        cINI.SetValue("NoteNumber", cBuf);
    }
        break;
    default:
        break;
    }
}

void _tag_cor_config_param::ConfigureNoteTypes(LPUSHORT lpusNoteIDs)
{
    CINIWriter cINI = m_configfile.GetWriterSection("BankNoteTypes");
    char cKey[16];
    char cVal[1024];
    char cCurrencyID[4];
    char cRelease;

    if ((lpusNoteIDs != NULL) && (lpNoteTypeList != NULL))
    {
        for (int i = 0; i < lpNoteTypeList->usNumOfNoteTypes; i++)
        {
            BOOL bFind = FALSE;
            LPUSHORT p = lpusNoteIDs;
            while (*p != 0)
            {
                if (*p == lpNoteTypeList->lppNoteTypes[i]->usNoteID)
                {
                    bFind = TRUE;
                    break;
                }
                p++;
            }
            if (bFind)
            {
                lpNoteTypeList->lppNoteTypes[i]->bConfigured = TRUE;
            }
            else
            {
                lpNoteTypeList->lppNoteTypes[i]->bConfigured = FALSE;
            }

            memset(cKey, 0, sizeof(cKey));
            sprintf(cKey, "NOTE%d", lpNoteTypeList->lppNoteTypes[i]->usNoteID);
            memset(cVal, 0, sizeof(cVal));
            if (lpNoteTypeList->lppNoteTypes[i]->usRelease > 0)
            {
                cRelease = lpNoteTypeList->lppNoteTypes[i]->usRelease - 1 + 'A';
            }
            else
            {
                cRelease = '0';
            }

            memset(cCurrencyID, 0, sizeof(cCurrencyID));
            memcpy(cCurrencyID, lpNoteTypeList->lppNoteTypes[i]->cCurrencyID, 3);
            sprintf(cVal, "%s,%d,%c,%d",
                    cCurrencyID,
                    (int32_t)lpNoteTypeList->lppNoteTypes[i]->ulValues,
                    cRelease,
                    (int32_t)lpNoteTypeList->lppNoteTypes[i]->bConfigured);
            cINI.SetValue(cKey, cVal);
        }
    }
}
