//CDMLCUInfor.cpp
//#include "StdAfx.h"
#include "CashUnit.h"
#include <string>
using namespace std;
ULONG CCashUnit::m_ulExchangeCount = 0;

static const char *ThisFile =  "CashUnit.cpp";

//标准构造函数
CCashUnit::CCashUnit(const LPWFSCIMNOTETYPELIST pNoteTypeList, BOOL bCDM):
    m_bCDM(bCDM), m_lpAllNoteTypeList(pNoteTypeList)
{
    InitData();
    SetLogFile(LOGFILE, ThisFile, "CASHUNIT");

    CINIFileReader configfile(string(SPETCPATH) + "/BRMSPConfig.ini");     //30-00-00-00(FT#0004)
    CINIReader     crINI = configfile.GetReaderSection("BRMInfo");         //30-00-00-00(FT#0004)
    m_bIgnoreMax = (BOOL)crINI.GetValue("SetUnitInfoIgnoreMax", 0);        //30-00-00-00(FT#0004)
}

//析构函数
//调用ClearData删除由自己分配的内存
CCashUnit::~CCashUnit()
{
    ClearData();
}


void CCashUnit::BuildCashUnitExtra()
{
    if ((m_fwItemType & WFS_CIM_CITYPINDIVIDUAL) == 0)
    {
        m_CIMCashUnit.lpszExtra = NULL;
        return;
    }

    string s = "lpusNoteIDs=";
    for (USHORT i = 0; i < m_usNoteIDCount; i++)
    {
        char szBuf[30];
        sprintf(szBuf, "%d", m_aryusNoteIDs[i]);
        s += szBuf;
        if (i < m_usNoteIDCount - 1)
            s += ",";
    }

    m_msCIMExtra = NULL;//todo
    m_msCIMExtra.Add(s.c_str());
    m_CIMCashUnit.lpszExtra = (LPSTR)(LPCSTR)m_msCIMExtra;
}

long CCashUnit::SetCashUnitName(LPCSTR lpszCashUnitName)
{
    VERIFY_SET_ITEM_ALLOWED();
    if (lpszCashUnitName == NULL)
    {
        if (m_sCashUnitName.empty())
            return 0;
    }
    else
    {
        if (m_sCashUnitName == lpszCashUnitName)
            return 0;
    }
    SET_CHANGED_DIRTY();
    m_sCashUnitName = lpszCashUnitName == NULL ? "" : lpszCashUnitName;
    return 0;
}

// 函数返回值表示NoteIDs的个数，aryusNoteIDs保存得到的ID
DWORD CCashUnit::GetNoteIDs(USHORT aryusNoteIDs[MAX_NOTEID_SIZE]) const
{
    memcpy(aryusNoteIDs, m_aryusNoteIDs, sizeof(m_aryusNoteIDs));
    return m_usNoteIDCount;
}

long CCashUnit::ClearNoteIDs()
{
    if (m_usNoteIDCount != 0)
    {
        m_usNoteIDCount = 0;
        SET_CHANGED_DIRTY();
    }
    return 0;
}

long CCashUnit::SetNoteIDs(LPUSHORT lpusNoteIDs, USHORT usSize)
{
    DEFMODULE(CU.SetNoteIDs);
    VERIFY_SET_ITEM_ALLOWED();

    //比较ID是否相同，相同则返回
    USHORT usNoteIDs[MAX_NOTEID_SIZE];
    DWORD dwCount = GetNoteIDs(usNoteIDs);
    if (dwCount == usSize &&
        (usSize == 0 || memcmp(lpusNoteIDs, usNoteIDs, sizeof(USHORT) * usSize) == 0))
    {
        return 0;
    }

    if (!NoteIDsAreInNoteTypeList(lpusNoteIDs, usSize))
    {
        Log(ThisModule, -1, "Some of NoteIDs are not in NoteTypeList");
        return WFS_ERR_INVALID_DATA;
    }

    SET_CHANGED_DIRTY();

    m_usNoteIDCount = usSize;
    if (usSize > 0)
    {
        memcpy(m_aryusNoteIDs, lpusNoteIDs, sizeof(USHORT) * usSize);
    }

    char szErrorDesc[250];
    long nRet = VerifyCassInfo(FALSE, szErrorDesc);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "%s", szErrorDesc);
        SetVerifyCUInfoFailedFlag(TRUE);
    }
    else
    {

        SetVerifyCUInfoFailedFlag(FALSE);
    }

    return 0;
}

DWORD CCashUnit::GetNoteNumbers(WFSCIMNOTENUMBER aryNoteNum[MAX_NOTEID_SIZE]) const
{
    if (m_lpNoteNumberList == NULL)
        return 0;
    if (m_lpNoteNumberList->usNumOfNoteNumbers == 0)
        return 0;
    DWORD dwCount = MAX_NOTEID_SIZE;
    if (m_lpNoteNumberList->usNumOfNoteNumbers < dwCount)
    {
        dwCount = m_lpNoteNumberList->usNumOfNoteNumbers;
    }
    for (USHORT i = 0; i < dwCount; i++)
    {
        memcpy(aryNoteNum + i, m_lpNoteNumberList->lppNoteNumber[i], sizeof(WFSCIMNOTENUMBER));
    }
    return dwCount;
}

// 一次性设置所有的"钞票ID-张数"信息。
long CCashUnit::SetNoteNumbers(LPWFSCIMNOTENUMBERLIST lpNoteNumList)
{
    DEFMODULE(CU.SetNoteNumbers);
    if (!SetItemIsAllowed(this, SIT_NOT_IN_EXCHANGE))
        return 0;

    //处理参数为NULL或个数为0的情况
    if (lpNoteNumList == NULL ||
        lpNoteNumList->usNumOfNoteNumbers == 0)
    {
        if (m_lpNoteNumberList == NULL)
        {
            return 0;
        }
        if (m_lpNoteNumberList->usNumOfNoteNumbers == 0)
        {
            ClearNoteNumbers();
            return 0;
        }
        ClearNoteNumbers();
        SET_CHANGED_DIRTY();
        return 0;
    }

    if (!NoteIDOfNoteNumberIsInNoteTypeList(lpNoteNumList))
    {
        Log(ThisModule, -1,
            "Some NoteIDs of NoteNumList are not in NoteTypeList");
        return WFS_ERR_INVALID_DATA;
    }

    //处理参数不为NULL的情况
    if (m_lpNoteNumberList == NULL)
    {
        m_lpNoteNumberList = new WFSCIMNOTENUMBERLIST;
        m_lpNoteNumberList->lppNoteNumber = NULL;
        m_lpNoteNumberList->usNumOfNoteNumbers = 0;
    }
    else
    {
        //个数相同，比较并拷贝数据
        if (m_lpNoteNumberList->usNumOfNoteNumbers == lpNoteNumList->usNumOfNoteNumbers)
        {
            for (USHORT i = 0; i < lpNoteNumList->usNumOfNoteNumbers; i++)
            {
                if (lpNoteNumList->lppNoteNumber[i] == NULL)
                {
                    return WFS_ERR_INVALID_DATA;
                }
                if (memcmp(lpNoteNumList->lppNoteNumber[i],
                           m_lpNoteNumberList->lppNoteNumber[i],
                           sizeof(WFSCIMNOTENUMBER)) == 0) //数据相同，跳过
                    continue;
                *m_lpNoteNumberList->lppNoteNumber[i] = *lpNoteNumList->lppNoteNumber[i];
                SET_CHANGED_DIRTY();
            }
            return 0;
        }
    }

    SET_CHANGED_DIRTY();
    USHORT i = 0;
    //删除老的数据，保留m_lpNoteNumberList指针的内存
    for (i = 0; i < m_lpNoteNumberList->usNumOfNoteNumbers; i++)
    {
        delete m_lpNoteNumberList->lppNoteNumber[i];
    }
    delete [] m_lpNoteNumberList->lppNoteNumber;
    m_lpNoteNumberList->lppNoteNumber = NULL;
    m_lpNoteNumberList->usNumOfNoteNumbers = 0;

    m_lpNoteNumberList->lppNoteNumber = new LPWFSCIMNOTENUMBER[lpNoteNumList->usNumOfNoteNumbers];
    m_lpNoteNumberList->usNumOfNoteNumbers = lpNoteNumList->usNumOfNoteNumbers;
    for (i = 0; i < m_lpNoteNumberList->usNumOfNoteNumbers; i++)
    {
        m_lpNoteNumberList->lppNoteNumber[i] = new WFSCIMNOTENUMBER;
        *m_lpNoteNumberList->lppNoteNumber[i] = *lpNoteNumList->lppNoteNumber[i];
    }
    return 0;
}

// 取得单种钞票的张数
long CCashUnit::GetNumberByNote(USHORT usNoteID, ULONG &ulCount) const
{
    ulCount = 0;
    if (m_lpNoteNumberList == NULL)
        return WFS_ERR_INVALID_DATA;
    if (m_lpNoteNumberList->usNumOfNoteNumbers == 0)
        return WFS_ERR_INVALID_DATA;
    for (USHORT i = 0; i < m_lpNoteNumberList->usNumOfNoteNumbers; i++)
    {
        if (m_lpNoteNumberList->lppNoteNumber[i]->usNoteID == usNoteID)
        {
            ulCount = m_lpNoteNumberList->lppNoteNumber[i]->ulCount;
            return 0;
        }
    }
    return WFS_ERR_INVALID_DATA;
}

//取得单种钞票的面额
long CCashUnit::GetCurrencyValueByNoteID(USHORT usNoteID, char cCurrency[3], ULONG &ulValue) const
{
    ulValue = 0;
    if (m_lpAllNoteTypeList == NULL)
        return WFS_ERR_INVALID_DATA;
    if (m_lpAllNoteTypeList->usNumOfNoteTypes == 0)
        return WFS_ERR_INVALID_DATA;
    for (USHORT i = 0; i < m_lpAllNoteTypeList->usNumOfNoteTypes; i++)
    {
        if (m_lpAllNoteTypeList->lppNoteTypes[i]->usNoteID == usNoteID)
        {
            ulValue = m_lpAllNoteTypeList->lppNoteTypes[i]->ulValues;
            memcpy(cCurrency, m_lpAllNoteTypeList->lppNoteTypes[i]->cCurrencyID, 3);
            return 0;
        }
    }
    return WFS_ERR_INVALID_DATA;
}

// 设置某种钞票的张数，如果设置的NoteID以前没有,那么就新增
long CCashUnit::SetNumbersByNote(USHORT usNoteID, ULONG ulCount)
{
    DEFMODULE(CU.SetNumbersByNote);
    VERIFY_SET_ITEM_ALLOWED();
    //如果列表为表，分配列表
    if (m_lpNoteNumberList == NULL)
    {
        m_lpNoteNumberList = new WFSCIMNOTENUMBERLIST;
        m_lpNoteNumberList->usNumOfNoteNumbers = 0;
        m_lpNoteNumberList->lppNoteNumber = NULL;
    }

    //查找ID是否存在，如果存在并且张数相同，返回；如果存在但张数不存在，修改并返回
    for (USHORT i = 0; i < m_lpNoteNumberList->usNumOfNoteNumbers; i++)
    {
        if (m_lpNoteNumberList->lppNoteNumber[i]->usNoteID == usNoteID)
        {
            if (m_lpNoteNumberList->lppNoteNumber[i]->ulCount != ulCount)
            {
                SET_CHANGED_DIRTY();
                m_lpNoteNumberList->lppNoteNumber[i]->ulCount = ulCount;
            }
            return 0;
        }
    }
    if (!NoteIDIsInNoteTypeList(usNoteID))
    {
        Log(ThisModule, -1,
            "NoteID(%hd) is not in NoteTypeList", usNoteID);
        return WFS_ERR_INVALID_DATA;
    }

    //处理ID不存在的情况，先分配一个长度加1的列表，拷贝老数据，再把ID和张数回到最后
    LPWFSCIMNOTENUMBER *ppNoteNumber =
    new LPWFSCIMNOTENUMBER[m_lpNoteNumberList->usNumOfNoteNumbers + 1];
    if (m_lpNoteNumberList->lppNoteNumber != NULL)
    {
        memcpy(ppNoteNumber, m_lpNoteNumberList->lppNoteNumber,
               sizeof(LPWFSCIMNOTENUMBER) * m_lpNoteNumberList->usNumOfNoteNumbers);
        delete [] m_lpNoteNumberList->lppNoteNumber;
        m_lpNoteNumberList->lppNoteNumber = NULL;
    }
    ppNoteNumber[m_lpNoteNumberList->usNumOfNoteNumbers] = new WFSCIMNOTENUMBER;
    ppNoteNumber[m_lpNoteNumberList->usNumOfNoteNumbers]->ulCount = ulCount;
    ppNoteNumber[m_lpNoteNumberList->usNumOfNoteNumbers]->usNoteID = usNoteID;
    m_lpNoteNumberList->lppNoteNumber = ppNoteNumber;
    m_lpNoteNumberList->usNumOfNoteNumbers += 1;

    return 0;
}

void CCashUnit::ClearNoteNumbers()
{
    //处理指针为NULL的情况
    if (m_lpNoteNumberList == NULL)
        return;

    //处理个数为0的情况
    if (m_lpNoteNumberList->usNumOfNoteNumbers == 0)
    {
        delete m_lpNoteNumberList;
        m_lpNoteNumberList = NULL;
        return;
    }

    SET_CHANGED_DIRTY();
    //个数不为0的情况
    for (USHORT i = 0; i < m_lpNoteNumberList->usNumOfNoteNumbers; i++)
    {
        delete m_lpNoteNumberList->lppNoteNumber[i];
    }
    delete [] m_lpNoteNumberList->lppNoteNumber;

    m_lpNoteNumberList->lppNoteNumber = NULL;
    m_lpNoteNumberList->usNumOfNoteNumbers = 0;
    delete [] m_lpNoteNumberList;
    m_lpNoteNumberList = NULL;
}

CASHUNIT_STATUS CCashUnit::ComputeStatusFromMinMax() const
{
    CASHUNIT_STATUS eStatus = m_usStatus;

    if (m_usType == ADP_CASSETTE_BILL ||
        m_usType == ADP_CASSETTE_RECYCLING ||
        m_usType == ADP_CASSETTE_RETRACT)
    {
        if (m_ulCount == 0)
        {
            if (eStatus == ADP_CASHUNIT_OK ||
                eStatus == ADP_CASHUNIT_FULL ||
                eStatus == ADP_CASHUNIT_HIGH ||
                eStatus == ADP_CASHUNIT_LOW)
            {
                eStatus = ADP_CASHUNIT_EMPTY;
                return eStatus;
            }
        }
        if (m_ulMinimum > 0)
        {
            if (eStatus == ADP_CASHUNIT_LOW)
            {
                eStatus = ADP_CASHUNIT_OK;
            }
            if ((eStatus == ADP_CASHUNIT_OK || eStatus == ADP_CASHUNIT_FULL || eStatus == ADP_CASHUNIT_HIGH)
                && (m_ulCount <= m_ulMinimum))
            {
                eStatus = ADP_CASHUNIT_EMPTY;
                return eStatus;
            }
        }
    }

    ICashUnitConfigItem *pCfg = m_pCfgItem->GetConfig()->GetCommonConfig();     //30-00-00-00(FT#0013)
//30-00-00-00(FT#0013)    if (m_usType != ADP_CASSETTE_BILL &&
    if(m_usType != ADP_CASSETTE_UNKNOWN &&                                      //30-00-00-00(FT#0013)
       (pCfg->GetCUValue(0, "HighStatusBaseMaximum") == 1))                     //30-00-00-00(FT#0013)
    {
        if (m_ulMaximum > 0 &&
            m_ulCount >= m_ulMaximum)
        {
            if (eStatus == ADP_CASHUNIT_OK ||
                eStatus == ADP_CASHUNIT_LOW)
            {
                eStatus = ADP_CASHUNIT_HIGH;
                return eStatus;
            }
        }
    }

    return eStatus;
}

//初始化数据，清除本类接口相关数据
//仅在构造函数中调用
void CCashUnit::InitData()
{
    m_usPHIndex = 0;
    m_usNumber = 0;
    m_usType = ADP_CASSETTE_UNKNOWN;
    CLEAR_ARRAY(m_cUnitID);
    CLEAR_ARRAY(m_cCurrencyID);
    m_ulValues = 0;
    m_ulInitialCount = 0;
    m_ulCount = 0;
    m_ulRejectCount = 0;
    m_ulMinimum = 0;
    m_ulMaximum = 0;
    m_bAppLock = FALSE;
    m_usStatus = ADP_CASHUNIT_OK;
    m_bVerifyCUInfoFailed = FALSE;
    m_fwItemType = 0;
    m_bHardwareSensors = FALSE;
    m_ulCashInCount = 0;
    m_lpNoteNumberList = NULL;
    m_dwErrorCount  = 0;
    m_bInSetByXFSFormat = FALSE;        //指示是否处于SetByXFSFormat函数调用中
    m_bItemChanged = FALSE;         //指示对象数据是否改变
    m_bExchangeState = FALSE;           //指示是否处于交换状态
    m_usNoteIDCount = 0;
    m_bIgnoreMax = FALSE;           // 指示是否更改最大值 30-00-00-00(FT#0004)

    m_CDMCashUnit.usNumPhysicalCUs = 1;
    m_lppCDMPHCU[0] = &m_CDMPHCU;
    m_CDMCashUnit.lppPhysical = m_lppCDMPHCU;

    m_CIMCashUnit.usNumPhysicalCUs = 1;
    m_lppCIMPHCU[0] = &m_CIMPHCU;
    m_CIMCashUnit.lppPhysical = m_lppCIMPHCU;
    m_CIMCashUnit.lpszExtra = NULL;
    m_CIMPHCU.lpszExtra = NULL;

    //m_msCIMExtra = NULL; //
}

//清除本类接口相关数据，如有指针，释放其内存
void CCashUnit::ClearData()
{
    m_sCashUnitName = "";
    m_sPhysicalPositionName = "";

    ClearNoteNumbers();
}

// Convert the information to XFS WFSCDMCASHUNIT
LPWFSCDMCASHUNIT CCashUnit::BuildByXFSCDMFormat()
{
    //逻辑钞箱
    m_CDMCashUnit.usNumber = m_usNumber;
    m_CDMCashUnit.usType = (USHORT)CassTypeLocal2XFS(m_usType, TRUE);
    m_CDMCashUnit.lpszCashUnitName = (char *)m_sCashUnitName.c_str();
    memcpy(m_CDMCashUnit.cUnitID, m_cUnitID, 5);
    memcpy(m_CDMCashUnit.cCurrencyID, m_cCurrencyID, 3);
    m_CDMCashUnit.ulValues = m_ulValues;
    m_CDMCashUnit.ulInitialCount = m_ulInitialCount;
    m_CDMCashUnit.ulCount = m_ulCount;
    m_CDMCashUnit.ulRejectCount = m_ulRejectCount;
    m_CDMCashUnit.ulMinimum = m_ulMinimum;
    m_CDMCashUnit.ulMaximum = m_ulMaximum;
    m_CDMCashUnit.bAppLock = m_bAppLock;
    m_CDMCashUnit.usStatus = StatusLocal2XFS(GetStatus());

    //物理钞箱
    memcpy(m_CDMPHCU.cUnitID, m_cUnitID, 5);
    m_CDMPHCU.lpPhysicalPositionName = (char *)m_sPhysicalPositionName.c_str();
    m_CDMPHCU.ulInitialCount = m_ulInitialCount;
    m_CDMPHCU.ulCount = m_ulCount;
    m_CDMPHCU.ulRejectCount = m_ulRejectCount;
    m_CDMPHCU.ulMaximum = m_ulMaximum;
    m_CDMPHCU.usPStatus = StatusLocal2XFS(GetStatus());
    m_CDMPHCU.bHardwareSensor = GetHardwareSensors();

    return &m_CDMCashUnit;
}

// Convert the information to XFS WFSCIMCASHIN
LPWFSCIMCASHIN CCashUnit::BuildByXFSCIMFormat()
{
    //逻辑钞箱
    m_CIMCashUnit.usNumber = m_usNumber;
    m_CIMCashUnit.fwType = (USHORT)CassTypeLocal2XFS(m_usType, FALSE);
    memcpy(m_CIMCashUnit.cUnitID, m_cUnitID, 5);
    memcpy(m_CIMCashUnit.cCurrencyID, m_cCurrencyID, 3);
    m_CIMCashUnit.ulValues = m_ulValues;
    m_CIMCashUnit.ulCashInCount = m_ulCashInCount;
    m_CIMCashUnit.ulCount = m_ulCount;
    m_CIMCashUnit.ulMaximum = m_ulMaximum;
    m_CIMCashUnit.bAppLock = m_bAppLock;
    m_CIMCashUnit.usStatus = StatusLocal2XFS(GetStatus());
    m_CIMCashUnit.fwItemType = m_fwItemType;
    m_CIMCashUnit.lpNoteNumberList = m_lpNoteNumberList;
    BuildCashUnitExtra();

    //物理钞箱
    memcpy(m_CIMPHCU.cUnitID, m_cUnitID, 5);
    m_CIMPHCU.lpPhysicalPositionName = (char *)m_sPhysicalPositionName.c_str();
    m_CIMPHCU.ulCashInCount = m_ulCashInCount;
    m_CIMPHCU.ulCount = m_ulCount;
    m_CIMPHCU.ulMaximum = m_ulMaximum;
    m_CIMPHCU.usPStatus = StatusLocal2XFS(GetStatus());
    m_CIMPHCU.bHardwareSensors = GetHardwareSensors();
    return &m_CIMCashUnit;
}

long CCashUnit::SetXFSData(LPWFSCDMCASHUNIT pData)
{
    DEFMODULE(CU.SetXFSCDMData);
    SetInSetByXFSFormat(TRUE); //设置在SetByXFSFormat函数调用中标志
    long iRet = InnerSetXFSData(pData);
    SetInSetByXFSFormat(FALSE); //清除在SetByXFSFormat函数调用中标志
    return iRet;
}

long CCashUnit::SetXFSData(LPWFSCIMCASHIN pData)
{
    DEFMODULE(CU.SetXFSCIMData);
    SetInSetByXFSFormat(TRUE); //设置在SetByXFSFormat函数调用中标志
    long iRet = InnerSetXFSData(pData);
    SetInSetByXFSFormat(FALSE); //清除在SetByXFSFormat函数调用中标志

    return iRet;
}

//设置XFS数据结构，保存与本类相关的数据
long CCashUnit::InnerSetXFSData(LPWFSCDMCASHUNIT pData)
{
    DEFMODULE(CU.InnerSetXFSCDMData);

    ADP_CASSETTE_TYPE eType = CassTypeXFS2Local(pData->usType, TRUE);
    if (m_bExchangeState &&
        eType != m_usType)
    {
        if ((eType != ADP_CASSETTE_BILL) && (eType != ADP_CASSETTE_RECYCLING) &&
            (eType != ADP_CASSETTE_REJECT) && (eType != ADP_CASSETTE_RETRACT))
        {
            Log(ThisModule, -1, "CDM:钞箱类型(%d)不属于XFS规范所定义的", eType);
            return WFS_ERR_INVALID_DATA;
        }

        BOOL bTypeAllowed =
        ((m_usType == ADP_CASSETTE_BILL && eType == ADP_CASSETTE_RECYCLING) ||
         (m_usType == ADP_CASSETTE_RECYCLING && eType == ADP_CASSETTE_BILL));
        if (!bTypeAllowed)
        {
            Log(ThisModule, -1,
                "CDM: usType(%hd) -> pData->usType(%hd) not allowed, usNumber=%hd",
                m_usType, eType, m_usNumber);
            return WFS_ERR_UNSUPP_DATA;
        }
        m_usType = eType;
        SET_CHANGED_DIRTY();
    }

    if (pData->usNumPhysicalCUs != 1 ||
        pData->lppPhysical == NULL ||
        pData->lppPhysical[0] == NULL)
    {
        Log(ThisModule, -1,
            "CDM: pData->usNumPhysicalCUs(%hd)不为1或pData->lppPhysical(0x%08X)为NULL或pData->lppPhysical[0](0x%08X) == NULL, usNumber=%hd",
            pData->usNumPhysicalCUs, pData->lppPhysical, pData->lppPhysical[0], m_usNumber);
        return WFS_ERR_INVALID_DATA;
    }
    ICashUnitConfigItem *pCfg = m_pCfgItem->GetConfig()->GetCommonConfig();
    //处理物理钞箱数全为0的情况：复制逻辑钞箱数到物理钞箱数
    BOOL bUseLogicalCount = pData->lppPhysical[0]->ulRejectCount == 0 &&
                            pData->lppPhysical[0]->ulCount == 0;

    if (pCfg->GetCUValue(0, "CalcBoxCountByLogicalNumber") == 1)
    {
        bUseLogicalCount = TRUE;
    }

    BOOL bAllowedSetValue = ((m_usType != ADP_CASSETTE_REJECT) &&
                             (m_usType != ADP_CASSETTE_RETRACT));
    BOOL bSetCountToInitialCountWhenCDMEndExchange = FALSE;
    if (m_bExchangeState)
    {
        if (pCfg != NULL)
        {
            bSetCountToInitialCountWhenCDMEndExchange = pCfg->GetCUValue(0, "SetCountToInitialCountWhenCDMEndExchange");
        }
    }
    VERIFY_FUNC_CALL(SetCashUnitName(pData->lpszCashUnitName));
    VERIFY_FUNC_CALL(SetInitialCount(
                     bUseLogicalCount ?
                     (bSetCountToInitialCountWhenCDMEndExchange ? pData->ulCount : pData->ulInitialCount) :
                     (bSetCountToInitialCountWhenCDMEndExchange ? pData->lppPhysical[0]->ulCount : pData->lppPhysical[0]->ulInitialCount)
                     ));
    VERIFY_FUNC_CALL(SetRejectCount(
                     bUseLogicalCount ? pData->ulRejectCount : pData->lppPhysical[0]->ulRejectCount));
    VERIFY_FUNC_CALL(SetMinimum(pData->ulMinimum));
    VERIFY_FUNC_CALL(SetUnitID(pData->cUnitID));
    VERIFY_FUNC_CALL(SetCurrencyID(pData->cCurrencyID));
    VERIFY_FUNC_CALL(SetValues(bAllowedSetValue ? pData->ulValues : 0));
    VERIFY_FUNC_CALL(SetCount(
                     bUseLogicalCount ? pData->ulCount : pData->lppPhysical[0]->ulCount));
    if (!m_bIgnoreMax)                                      //30-00-00-00(FT#0004)
    {                                                       //30-00-00-00(FT#0004)
        VERIFY_FUNC_CALL(SetMaximum(pData->ulMaximum));     //30-00-00-00(FT#0004)
    }                                                       //30-00-00-00(FT#0004)
    VERIFY_FUNC_CALL(SetAppLock(pData->bAppLock));

    if (!m_bExchangeState)
    {
        return 0;
    }
    char szErrorDesc[250];
    BOOL bErrorByItemTypeNoteID = FALSE;
    long nRet = VerifyCassInfo(FALSE, szErrorDesc, &bErrorByItemTypeNoteID);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "%s", szErrorDesc);
        if (!bErrorByItemTypeNoteID)
            return nRet;
        SetVerifyCUInfoFailedFlag(TRUE);
    }
    else
    {

        SetVerifyCUInfoFailedFlag(FALSE);
    }

    return 0;
}

long CCashUnit::InnerSetXFSData(LPWFSCIMCASHIN pData)
{
    DEFMODULE(CU.InnerSetXFSCIMData);

    ADP_CASSETTE_TYPE eType = CassTypeXFS2Local(pData->fwType, FALSE);
    if (m_bExchangeState &&  //交换状态才设置状态
        eType != m_usType)
    {
        if ((eType != ADP_CASSETTE_CASHIN) && (eType != ADP_CASSETTE_RECYCLING) &&
            (eType != ADP_CASSETTE_RETRACT))
        {
            Log(ThisModule, -1, "CIM:钞箱(%hd)的类型(%d)不属于XFS规范所定义的", pData->usNumber, eType);
            return WFS_ERR_INVALID_DATA;
        }

        BOOL bTypeAllowed =
        ((m_usType == ADP_CASSETTE_CASHIN && eType == ADP_CASSETTE_RECYCLING) ||
         (m_usType == ADP_CASSETTE_RECYCLING && eType == ADP_CASSETTE_CASHIN));
        if (!bTypeAllowed)
        {
            Log(ThisModule, -1,
                "CIM: usType(%hd) -> pData->fwType(%d) not allowed, , usNumber=%hd",
                m_usType, pData->fwType, m_usNumber);
            return WFS_ERR_UNSUPP_DATA;
        }
        m_usType = eType;
        SET_CHANGED_DIRTY();
    }

    if (pData->usNumPhysicalCUs != 1 ||
        pData->lppPhysical == NULL ||
        pData->lppPhysical[0] == NULL)
    {
        Log(ThisModule, -1,
            "CIM: pData->usNumPhysicalCUs(%hd)不为1或pData->lppPhysical(0x%08X)为NULL或pData->lppPhysical[0](0x%08X) == NULL, usNumber=%hd",
            pData->usNumPhysicalCUs, pData->lppPhysical, pData->lppPhysical[0], m_usNumber);
        return WFS_ERR_INVALID_DATA;
    }

    //处理物理钞箱数全为0的情况：复制逻辑钞箱数到物理钞箱数
    BOOL bUseLogicalCount = pData->lppPhysical[0]->ulCashInCount == 0 &&
                            pData->lppPhysical[0]->ulCount == 0;
    BOOL bAllowedSetValue = (m_usType != ADP_CASSETTE_RETRACT);
    VERIFY_FUNC_CALL(SetItemType(pData->fwItemType));
    VERIFY_FUNC_CALL(SetUnitID(pData->cUnitID));
    VERIFY_FUNC_CALL(SetCurrencyID(pData->cCurrencyID));
    VERIFY_FUNC_CALL(SetValues(bAllowedSetValue ? pData->ulValues : 0));
    VERIFY_FUNC_CALL(SetCashInCount(
                     bUseLogicalCount ? pData->ulCashInCount : pData->lppPhysical[0]->ulCashInCount));
    VERIFY_FUNC_CALL(SetCount(
                     bUseLogicalCount ? pData->ulCount : pData->lppPhysical[0]->ulCount));
    if (!m_bIgnoreMax)                                      //30-00-00-00(FT#0004)
    {                                                       //30-00-00-00(FT#0004)
        VERIFY_FUNC_CALL(SetMaximum(pData->ulMaximum));     //30-00-00-00(FT#0004)
    }                                                       //30-00-00-00(FT#0004)
    VERIFY_FUNC_CALL(SetAppLock(pData->bAppLock));
    VERIFY_FUNC_CALL(SetNoteNumbers(pData->lpNoteNumberList));

    if (!m_bExchangeState)
    {
        return 0;
    }

    char szErrorDesc[250];
    BOOL bErrorByItemTypeNoteID = FALSE;
    long nRet = VerifyCassInfo(FALSE, szErrorDesc, &bErrorByItemTypeNoteID);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "%s", szErrorDesc);
        if (!bErrorByItemTypeNoteID)
            return nRet;
        SetVerifyCUInfoFailedFlag(TRUE);
    }
    else
    {
        SetVerifyCUInfoFailedFlag(FALSE);
    }

    return 0;
}

int CCashUnit::LoadConfig(ICashUnitConfigItem *pCfgItem)
{
    return InnerLoadConfig(pCfgItem);
}

int CCashUnit::InnerLoadConfig(ICashUnitConfigItem *pCfgItem)
{
    DEFMODULE(CU.LoadConfig);
    ClearData(); //装入配置前先清除老的数据

    CConfigurable::LoadConfig(pCfgItem);

    //logical cassette
    m_usNumber = m_pCfgItem->GetCUValue(ADP_CASSETTE_UNKNOWN, "NUMBER");
    if (m_usNumber == 0)
    {
        Log(ThisModule, -1,
            "Cassette Number configure error(Number=%hd)",
            m_usNumber);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    m_usPHIndex = (USHORT)m_pCfgItem->GetCUValue(0, "PHCU");
    if (m_usPHIndex == 0)
    {
        Log(ThisModule, -1,
            "Cassette PHCU configure error(PHCU=%hd)",
            m_usPHIndex);
        return WFS_ERR_SOFTWARE_ERROR;
    }
    VERIFY_FUNC_CALL(SetAppLock(m_pCfgItem->GetCUValue(0, "LOCK")));

    ADP_CASSETTE_TYPE eType = (ADP_CASSETTE_TYPE)m_pCfgItem->GetCUValue(ADP_CASSETTE_UNKNOWN, "TYPE");
    if (eType < ADP_CASSETTE_BILL ||
        eType >= ADP_CASSETTE_UNKNOWN)
    {
        Log(ThisModule, -1,
            "Cassette type configure error(Type=%d)",
            eType);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    VERIFY_FUNC_CALL(SetType(eType));
    VERIFY_FUNC_CALL(SetInitialCount(pCfgItem->GetCUValue(0, "INITCOUNT")));
    VERIFY_FUNC_CALL(SetCashInCount(pCfgItem->GetCUValue(0, "CINCOUNT")));
    VERIFY_FUNC_CALL(SetCount(pCfgItem->GetCUValue(0, "COUNT")));
    VERIFY_FUNC_CALL(SetRejectCount(pCfgItem->GetCUValue(0, "REJCOUNT")));

    VERIFY_FUNC_CALL(SetItemType(pCfgItem->GetCUValue(0, "ITEMTYPE")));

    //physical cassette
    ICashUnitConfigItem *pPhCUConfig = GetPHCfgItemByIndex(m_usPHIndex, pCfgItem);
    if (pPhCUConfig == NULL)
    {
        Log(ThisModule, -1,
            "%s:逻辑钞箱(%hd)对应的物理钞箱的索引(%hd)不存在",
            m_bCDM ? "CDM" : "CIM", m_usNumber, m_usPHIndex);
        return WFS_ERR_SOFTWARE_ERROR;
    }

    BOOL bAllowedSetValue = ((m_usType != ADP_CASSETTE_REJECT) &&
                             (m_usType != ADP_CASSETTE_RETRACT));
    strncpy(m_cUnitID, (char *)pPhCUConfig->GetCUValue("", "ID"), 5);
    strncpy(m_cCurrencyID, pPhCUConfig->GetCUValue("   ", "CURRENCY"), 3);
    VERIFY_FUNC_CALL(bAllowedSetValue ? SetValues(pPhCUConfig->GetCUValue(0, "VALUE")) : 0);

    VERIFY_FUNC_CALL(SetMaximum(pPhCUConfig->GetCUValue(0, "MAX")));
    VERIFY_FUNC_CALL(SetMinimum(pPhCUConfig->GetCUValue(0, "MIN")));

    VERIFY_FUNC_CALL(SetStatus((CASHUNIT_STATUS)pPhCUConfig->GetCUValue(0, "STATUS"), TRUE));
    VERIFY_FUNC_CALL(SetHardwareSensors(pPhCUConfig->GetCUValue(0, "SENSOR")));
    SetErrorCount(pPhCUConfig->GetCUValue(0, "ERRORCOUNT"));

    m_sPhysicalPositionName = pPhCUConfig->GetCUValue("", "PPN");

    //note number
    VERIFY_FUNC_CALL(LoadNoteNumber());
    VERIFY_FUNC_CALL(LoadNoteIDs());

    char szErrorDesc[250];
    if (VerifyCassInfo(TRUE, szErrorDesc) < 0)
    {
        Log(ThisModule, -1, "%s", szErrorDesc);
        m_bVerifyCUInfoFailed = TRUE;
    }

    return 0;
}

//是否处于设置XFS状态函数（SetByXFSFormat）调用中
BOOL CCashUnit::IsInSetByXFSFormat() const
{
    return m_bInSetByXFSFormat;
}

const LPWFSCIMNOTETYPELIST CCashUnit::GetAllNoteTypeList() const
{
    return m_lpAllNoteTypeList;
}

void CCashUnit::SetVerifyCUInfoFailedFlag(BOOL bFailed)
{
    if (m_bVerifyCUInfoFailed != bFailed)
    {
        CASHUNIT_STATUS eOldStatus = GetStatus();   //保存老状态
        m_bVerifyCUInfoFailed = bFailed;
        CASHUNIT_STATUS eNewStatus = GetStatus();
        if (eOldStatus != eNewStatus)
        {
            SetItemChanged();
        }
    }
}

ICashUnitConfigItem *CCashUnit::GetPHCfgItemByIndex(USHORT usPHIndex, ICashUnitConfigItem *pCfgItem) const
{
    ICashUnitConfig *pConfig = pCfgItem->GetConfig();
    DWORD dwPHCUConfigCount = pConfig->GetCUConfigCount(CT_LOG_PH);
    for (UINT i = 0; i < dwPHCUConfigCount; i++)
    {
        ICashUnitConfigItem *pTemp = pConfig->GetCUConfig(CT_LOG_PH, i);
        if (usPHIndex == pTemp->GetCUValue(0, "INDEX"))
        {
            return pTemp;
        }
    }
    return NULL;
}

int CCashUnit::SaveConfig()
{
    DEFMODULE(CU.SaveConfig);
    VERIFY_DIRTY();
    //logical cassette
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_usNumber, "NUMBER"));
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_usPHIndex, "PHCU"));
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_bAppLock, "LOCK"));
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_usType, "TYPE"));

    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_ulInitialCount, "INITCOUNT"));
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_ulRejectCount, "REJCOUNT"));
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_ulCount, "COUNT"));
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_ulCashInCount, "CINCOUNT"));

    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(m_fwItemType, "ITEMTYPE"));
    //physical cassette
    ICashUnitConfigItem *pPhCUConfig = GetPHCfgItemByIndex(m_usPHIndex, m_pCfgItem);

    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_usType, "TYPE"));

    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_cUnitID, "ID"));

    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_cCurrencyID, "CURRENCY"));
    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_ulValues, "VALUE"));

    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_ulMaximum, "MAX"));
    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_ulMinimum, "MIN"));

    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_usStatus, "STATUS"));


    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_bHardwareSensors, "SENSOR"));
    VERIFY_FUNC_CALL(pPhCUConfig->SetCUValue(m_dwErrorCount, "ERRORCOUNT"));

    //note number
    VERIFY_FUNC_CALL(SaveNoteNumber());
    VERIFY_FUNC_CALL(SaveNoteIDs());

    SetDirty(FALSE);

    return 0;
}

void CCashUnit::CopyFrom(const CCashUnit &src, BOOL bBackup)
{

    if (src.m_lpNoteNumberList != NULL)
    {
        SetNoteNumbers(src.m_lpNoteNumberList);
    }
    CConfigurable::CopyFrom(src);

    m_usPHIndex = src.m_usPHIndex;
    m_usNumber = src.m_usNumber;
    m_usType = src.m_usType;
    memcpy(m_cUnitID, src.m_cUnitID, 5);
    memcpy(m_cCurrencyID, src.m_cCurrencyID, 3);
    m_ulValues = src.m_ulValues;
    m_ulInitialCount = src.m_ulInitialCount;
    m_ulCount = src.m_ulCount;
    m_ulRejectCount = src.m_ulRejectCount;
    m_ulMinimum = src.m_ulMinimum;
    m_ulMaximum = src.m_ulMaximum;
    m_bAppLock = src.m_bAppLock;
    m_usStatus = src.m_usStatus;
    m_bVerifyCUInfoFailed = src.m_bVerifyCUInfoFailed;
    m_fwItemType = src.m_fwItemType;
    m_ulCashInCount = src.m_ulCashInCount;
    m_bInSetByXFSFormat = src.m_bInSetByXFSFormat;  //指示是否处于SetByXFSFormat函数调用中
    m_bItemChanged = src.m_bItemChanged;            //指示对象数据是否改变
    m_bExchangeState = src.m_bExchangeState;        //指示是否处于交换状态
    m_dwErrorCount = src.m_dwErrorCount;            //错误次数
    memcpy(m_aryusNoteIDs, src.m_aryusNoteIDs, sizeof(m_aryusNoteIDs));
    m_usNoteIDCount = src.m_usNoteIDCount;
}

//Load Note Number
long CCashUnit::LoadNoteNumber()
{
    DEFMODULE(CU.LoadNoteNumber);

    const char *pNMList = m_pCfgItem->GetCUValue("", "NNLIST");
    char szBuf[1024];
    strcpy(szBuf, pNMList);
    char *p = strtok(szBuf, "|");
    while (p != NULL &&
           *p != '\0')
    {
        USHORT usNoteID = 0;
        ULONG ulCount = 0;
        int iRet = sscanf(p, "%hd:%ld", &usNoteID, &ulCount);
        if (iRet != 2 || usNoteID == 0)
        {
            Log(ThisModule, -1,
                "sscanf NoteID and Count error(iRet=%d, usNoteID=%hd)",
                iRet, usNoteID);
            return WFS_ERR_SOFTWARE_ERROR;
        }
        VERIFY_FUNC_CALL(SetNumbersByNote(usNoteID, ulCount));
        p = strtok(NULL, "|");
    }
    return 0;
}

//Save Note Numbers
long CCashUnit::SaveNoteNumber()
{
    DEFMODULE(CU.SaveNoteNumber);
    char szBuf[1024];
    szBuf[0] = 0;

    WFSCIMNOTENUMBER nn[MAX_NOTEID_SIZE];
    DWORD dwNoteNumbersCount = GetNoteNumbers(nn);
    for (DWORD i = 0; i < dwNoteNumbersCount; i++)
    {
        char szTemp[40];
        sprintf(szTemp, "%hd:%ld", nn[i].usNoteID, nn[i].ulCount);
        if (i != 0)
        {
            strcat(szBuf, "|");
        }
        strcat(szBuf, szTemp);
    }
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(szBuf, "NNLIST"));
    return 0;
}

//Load Note Number
long CCashUnit::LoadNoteIDs()
{
    DEFMODULE(CU.LoadNoteIDs);

    USHORT usNoteIDCount = 0;
    USHORT aryusNoteIDs[MAX_NOTEID_SIZE];

    const char *pNoteIDs = m_pCfgItem->GetCUValue("", "NOTEIDS");
    char szBuf[1024];
    strcpy(szBuf, pNoteIDs);
    char *p = strtok(szBuf, "|");
    while (p != NULL &&
           *p != '\0')
    {
        USHORT usNoteID = 0;
        int iRet = sscanf(p, "%hd", &usNoteID);
        if (iRet != 1 || usNoteID == 0)
        {
            Log(ThisModule, -1,
                "sscanf NoteID From NOTEIDS error(iRet=%d, usNoteID=%hd)",
                iRet, usNoteID);
            return WFS_ERR_SOFTWARE_ERROR;
        }
        aryusNoteIDs[usNoteIDCount] = usNoteID;
        usNoteIDCount++;
        p = strtok(NULL, "|");
    }

    VERIFY_FUNC_CALL(SetNoteIDs(aryusNoteIDs, usNoteIDCount));

    return 0;
}

//Save Note Numbers
long CCashUnit::SaveNoteIDs()
{
    DEFMODULE(CU.SaveNoteIDs);
    char szBuf[1024];
    szBuf[0] = 0;

    for (USHORT i = 0; i < m_usNoteIDCount; i++)
    {
        char szTemp[40];
        sprintf(szTemp, "%ld", m_aryusNoteIDs[i]);
        if (i != 0)
        {
            strcat(szBuf, "|");
        }
        strcat(szBuf, szTemp);
    }
    VERIFY_FUNC_CALL(m_pCfgItem->SetCUValue(szBuf, "NOTEIDS"));
    return 0;
}

long CCashUnit::VerifyCassInfo(BOOL bVerifyType, char szErrorDesc[250], BOOL *pbItemTypeNoteIDError) const
{
    if (pbItemTypeNoteIDError != NULL)
        *pbItemTypeNoteIDError = FALSE;
    //校验回收箱的币种
    if (m_usType == ADP_CASSETTE_RETRACT && strcmp(m_cCurrencyID, "   ") != 0)
    {
        if (szErrorDesc != NULL)
        {
            sprintf(szErrorDesc,
                    "%s CURRENCY(%s) has error(cassette type=%d, Number=%hd)",
                    m_bCDM ? "CDM" : "CIM", m_cCurrencyID, m_usType, m_usNumber);
        }
        return WFS_ERR_INVALID_DATA;
    }
    //校验BILL、循环、进钞箱的面值、币种有效
    if (m_usType == ADP_CASSETTE_BILL ||
        m_usType == ADP_CASSETTE_RECYCLING ||
        m_usType == ADP_CASSETTE_CASHIN)
    {
        if (strlen(m_cCurrencyID) != 3 ||
            strcmp(m_cCurrencyID, "   ") == 0)
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s CURRENCY(%s) has error(cassette type=%d, Number=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_cCurrencyID, m_usType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
        if (m_ulValues == 0)
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s VALUE(%d) has error(cassette type=%d, usNumber=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_ulValues, m_usType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }

        if (!NoteTypeIsInNoteTypeList(m_cCurrencyID, m_ulValues))
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s VALUE(%d) Or CURRENCY(%s) has error(cassette type=%d, usNumber=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_ulValues, m_cCurrencyID, m_usType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }

    //检验钞箱物理类型和逻辑类型相符
    ICashUnitConfigItem *pPhCUConfig = GetPHCfgItemByIndex(m_usPHIndex, m_pCfgItem);
    assert(pPhCUConfig != NULL);

    ADP_CASSETTE_TYPE ePHType = (ADP_CASSETTE_TYPE)pPhCUConfig->GetCUValue(0, "TYPE");
    if (bVerifyType)
    {
        if (m_usType == ADP_CASSETTE_RETRACT ||
            m_usType == ADP_CASSETTE_REJECT)
        {
            if ((ePHType != m_usType &&
                 ePHType != ADP_CASSETTE_UNKNOWN &&
                 !m_bCDM) ||
                (ePHType != ADP_CASSETTE_RETRACT &&
                 ePHType != ADP_CASSETTE_REJECT &&
                 m_bCDM))
            {
                if (szErrorDesc != NULL)
                {
                    sprintf(szErrorDesc,
                            "%s 钞箱类型不一致(LogType=%d, PhType=%d, Number=%hd)",
                            m_bCDM ? "CDM" : "CIM", m_usType, ePHType, m_usNumber);
                }
                return WFS_ERR_INVALID_DATA;
            }
        }
        else if (m_usType != ePHType)
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s 钞箱类型不一致(LogType=%d, PhType=%d, Number=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_usType, ePHType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }

    //检验钞箱类型与设备类型相符
    if (m_bCDM) //CDM
    {
        if (m_usType == ADP_CASSETTE_CASHIN)
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "CDM can't have CASHIN cassette(cassette type=%d, Number=%hd)",
                        m_usType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }

    }
    else    //CIM
    {
        if (m_usType == ADP_CASSETTE_BILL ||
            m_usType == ADP_CASSETTE_REJECT)
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "CIM can't have BILL or REJECT cassette(cassette type=%d, Number=%hd)",
                        m_usType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }

    if (m_bCDM)
    {
        return 0;
    }

    if (pbItemTypeNoteIDError != NULL)
        *pbItemTypeNoteIDError = TRUE;
    //校验NoteNumber、ItemType与钞箱类型的一致性
    if ((m_fwItemType & WFS_CIM_CITYPINDIVIDUAL) != 0)
    {
        if (m_usNoteIDCount == 0) //ITEM类型为WFS_CIM_CITYPINDIVIDUAL时NoteID个数不能为0
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s cassette MUST have NoteID when m_fwItemType == 0x%X(cassette type=%d, Number=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_fwItemType, m_usType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }

    if (m_usType == ADP_CASSETTE_BILL)
    {
        if (m_fwItemType != 0)
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s BILL cassette ItemType MUST be 0 (cassette type=%d, Number=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_usType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }
    else if (m_usType == ADP_CASSETTE_RECYCLING)
    {
        if (m_fwItemType != WFS_CIM_CITYPINDIVIDUAL)
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s RECYCLING cassette ItemType MUST be WFS_CIM_CITYPINDIVIDUAL(cassette type=%d, Number=%hd, ItemType=%d)",
                        m_bCDM ? "CDM" : "CIM", m_usType, m_usNumber, m_fwItemType);
            }
            return WFS_ERR_INVALID_DATA;
        }
        if (m_usNoteIDCount < 1) //循环箱不能少于一种NoteType
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s RECYCLING cassette's NoteIDCount < 1(cassette type=%d, Number=%hd, NoteIDCount=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_usType, m_usNumber, m_usNoteIDCount);
            }
            return WFS_ERR_INVALID_DATA;
        }
        if (!NoteIDsAreSameCurrencyValue())
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "RECYCLING cassette's CurrencyID or Value is different with CurrentcyID[%0.3s] or Value[%d] , or not defined",
                        m_cCurrencyID, m_ulValues);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }
    else
    {
        if (m_fwItemType == 0)
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s cassette ItemType CAN'T be 0 (cassette type=%d, Number=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_usType, m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }

    //检验NoteNumberList在AllNoteTypeList中
    if (m_lpNoteNumberList != NULL)
    {
        if (!NoteIDOfNoteNumberIsInNoteTypeList(m_lpNoteNumberList))
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s cassette NoteIDsOfNoteNumber are not in note type list(usNumber=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }

    //检验NoteID在AllNoteTypeList中
    if (m_usNoteIDCount > 0)
    {
        if (!NoteIDsAreInNoteTypeList((LPUSHORT)m_aryusNoteIDs, m_usNoteIDCount))
        {
            if (szErrorDesc != NULL)
            {
                sprintf(szErrorDesc,
                        "%s cassette NoteIDs are not in note type list(usNumber=%hd)",
                        m_bCDM ? "CDM" : "CIM", m_usNumber);
            }
            return WFS_ERR_INVALID_DATA;
        }
    }

    return 0;
}

BOOL CCashUnit::CurrencyIDIsInNoteTypeList(const char *pCurrencyID) const
{
    if (memcmp(pCurrencyID, "   ", 3) == 0)
        return TRUE;

    for (USHORT i = 0; i < m_lpAllNoteTypeList->usNumOfNoteTypes; i++)
    {
        assert(m_lpAllNoteTypeList->lppNoteTypes[i] != NULL);
        if (memcmp(pCurrencyID, m_lpAllNoteTypeList->lppNoteTypes[i]->cCurrencyID, 3) == 0)
            return TRUE;
    }

    return FALSE;
}

BOOL CCashUnit::NoteTypeIsInNoteTypeList(const char *pCurrencyID, ULONG ulValues) const
{
    if (memcmp(pCurrencyID, "   ", 3) == 0)
        return TRUE;

    for (USHORT i = 0; i < m_lpAllNoteTypeList->usNumOfNoteTypes; i++)
    {
        assert(m_lpAllNoteTypeList->lppNoteTypes[i] != NULL);
        if ((memcmp(pCurrencyID, m_lpAllNoteTypeList->lppNoteTypes[i]->cCurrencyID, 3) == 0)
            && (ulValues == m_lpAllNoteTypeList->lppNoteTypes[i]->ulValues))
            return TRUE;
    }

    return FALSE;
}


BOOL CCashUnit::NoteIDIsInNoteTypeList(USHORT usNoteID) const
{
    for (USHORT i = 0; i < m_lpAllNoteTypeList->usNumOfNoteTypes; i++)
    {
        assert(m_lpAllNoteTypeList->lppNoteTypes[i] != NULL);
        if (usNoteID == m_lpAllNoteTypeList->lppNoteTypes[i]->usNoteID)
            return TRUE;
    }

    return FALSE;
}

BOOL CCashUnit::NoteIDOfNoteNumberIsInNoteTypeList(
LPWFSCIMNOTENUMBERLIST lpNoteNumberList) const
{
    assert(m_lpAllNoteTypeList != NULL);
    for (USHORT i = 0; i < lpNoteNumberList->usNumOfNoteNumbers; i++)
    {
        LPWFSCIMNOTENUMBER pNoteNumber = lpNoteNumberList->lppNoteNumber[i];
        if (!NoteIDIsInNoteTypeList(pNoteNumber->usNoteID))
            return FALSE;
    }

    return TRUE;
}

BOOL CCashUnit::NoteIDsAreInNoteTypeList(LPUSHORT lpusNoteIDs, USHORT usSize) const
{
    for (USHORT i = 0; i < usSize; i++)
    {
        if (!NoteIDIsInNoteTypeList(lpusNoteIDs[i]))
            return FALSE;
    }
    return TRUE;
}

BOOL CCashUnit::NoteIDsAreSameCurrencyValue() const
{
    for (USHORT i = 0; i < m_usNoteIDCount; i++)
    {
        char cCurrency[3];
        ULONG ulValue;
        if (GetCurrencyValueByNoteID(m_aryusNoteIDs[i], cCurrency, ulValue) < 0)
            return FALSE;
        if (m_ulValues != ulValue ||
            memcmp(m_cCurrencyID, cCurrency, 3) != 0)
        {
            return FALSE;
        }
    }
    return TRUE;
}

CASHUNIT_STATUS CCashUnit::ComputeStatusFromCfgAndMinMax() const
{
    CASHUNIT_STATUS eStatus = ComputeStatusFromMinMax();
    return ComputeStatusFromCfg(eStatus);
}

CASHUNIT_STATUS CCashUnit::ComputeStatusFromCfg(CASHUNIT_STATUS eStatus) const
{
    ICashUnitConfigItem *pCfg = m_pCfgItem->GetConfig()->GetCommonConfig();
    if (pCfg == NULL)
        return eStatus;

    int nErrorRetryNum = pCfg->GetCUValue(1, "BoxErrorRetryNum");
    if (m_dwErrorCount > (DWORD)nErrorRetryNum &&
        (eStatus == ADP_CASHUNIT_OK ||
         eStatus == ADP_CASHUNIT_HIGH ||
         eStatus == ADP_CASHUNIT_LOW ||
         eStatus == ADP_CASHUNIT_FULL))
    {
        return ADP_CASHUNIT_INOP;
    }

    if (m_usType == ADP_CASSETTE_CASHIN ||
        m_usType == ADP_CASSETTE_RETRACT ||
        m_usType == ADP_CASSETTE_REJECT)
    {
        if ((eStatus == ADP_CASHUNIT_LOW || eStatus == ADP_CASHUNIT_EMPTY) &&
            !pCfg->GetCUValue(0, "InBoxAllowedLowEmptyStatus"))
        {
            return ADP_CASHUNIT_OK;
        }

        ULONG uInBoxFullCount = pCfg->GetCUValue(0, "InBoxFullCount");
        if (((eStatus == ADP_CASHUNIT_OK)
             || (eStatus == ADP_CASHUNIT_LOW)
             || (eStatus == ADP_CASHUNIT_HIGH))
            && (uInBoxFullCount > 0)
            && (m_ulCount >= uInBoxFullCount))
        {
            return ADP_CASHUNIT_FULL;
        }
    }
    else if (m_usType == ADP_CASSETTE_BILL)
    {
        if ((eStatus != ADP_CASHUNIT_MISSING)
            && (eStatus != ADP_CASHUNIT_INOP)
            && (eStatus != ADP_CASHUNIT_MANIP)
            && m_ulCount <= (ULONG)pCfg->GetCUValue(0, "CDMBoxLeftCount"))
        {
            return ADP_CASHUNIT_EMPTY;
        }

        if ((eStatus == ADP_CASHUNIT_HIGH || eStatus == ADP_CASHUNIT_FULL) &&
            !pCfg->GetCUValue(0, "OutBoxAllowedHighFullStatus"))
        {
            eStatus = ADP_CASHUNIT_OK;
        }

        if ((pCfg->GetCUValue(0, "CDMBoxLowStatusBySensor") == 0)//启用物理传感器检测LOW状态
            && (eStatus == ADP_CASHUNIT_LOW))
        {
            eStatus = ADP_CASHUNIT_OK;
        }

        if ((eStatus == ADP_CASHUNIT_OK || eStatus == ADP_CASHUNIT_FULL || eStatus == ADP_CASHUNIT_HIGH) &&
            m_ulCount <= (ULONG)pCfg->GetCUValue(0, "CDMBoxLowStatusCount"))
        {
            return ADP_CASHUNIT_LOW;
        }

    }
    else if (eStatus != ADP_CASHUNIT_MISSING && m_usType == ADP_CASSETTE_RECYCLING && m_bCDM)
    {
        if (m_ulCount <= (ULONG)pCfg->GetCUValue(0, "RecyclingBoxLeftCount"))
        {
            return ADP_CASHUNIT_EMPTY;
        }

        if ((eStatus == ADP_CASHUNIT_HIGH || eStatus == ADP_CASHUNIT_FULL) &&
            (0 == pCfg->GetCUValue(1, "RecyclingBoxAllowedHighFullStatus")))
        {
            eStatus = ADP_CASHUNIT_OK;
        }
    }
    else if (eStatus != ADP_CASHUNIT_MISSING  && m_usType == ADP_CASSETTE_RECYCLING && !m_bCDM)
    {
        ULONG ulCount = (ULONG)pCfg->GetCUValue(0, "RecyclingBoxMaxCount");
        if ((m_ulCount >= ulCount) && (ulCount != 0) && eStatus != ADP_CASHUNIT_EMPTY)
        {
            eStatus = ADP_CASHUNIT_FULL;
        }

        if ((eStatus == ADP_CASHUNIT_LOW || eStatus == ADP_CASHUNIT_EMPTY) &&
            (0 == pCfg->GetCUValue(1, "RecyclingBoxAllowedLowEmptyStatus")))
        {
            eStatus = ADP_CASHUNIT_OK;
        }
    }

    return eStatus;
}

void CCashUnit::GetCurrencyID(CHAR cCurrencyID[3]) const
{
    memcpy(cCurrencyID, m_cCurrencyID, 3);
}

long CCashUnit::SetCurrencyID(CHAR cCurrencyID[3])
{
    VERIFY_SET_ITEM_ALLOWED();
    if (m_usType == ADP_CASSETTE_RETRACT || m_usType == ADP_CASSETTE_REJECT)
    {
        if (memcmp(cCurrencyID, "   ", 3) != 0)
            return WFS_ERR_INVALID_DATA;
    }
    else
    {
        if (memcmp(cCurrencyID, "   ", 3) == 0)
        {
            return WFS_ERR_UNSUPP_DATA;
        }
        // todo
        //if (!CurrencyCodeFitISOList(cCurrencyID))
        //{
        //return WFS_ERR_INVALID_DATA;
        //}

    }

    if (memcmp(cCurrencyID, m_cCurrencyID, 3) != 0)
    {
        if (!CurrencyIDIsInNoteTypeList(cCurrencyID))
        {
            return WFS_ERR_UNSUPP_DATA;
        }
        memcpy(m_cCurrencyID, cCurrencyID, 3);
        SET_CHANGED_DIRTY();
    }
    return 0;
}

void CCashUnit::SetIndex(USHORT usIndex)
{
    assert(m_usPHIndex == 0);
    m_usPHIndex = usIndex;
    SET_CHANGED_DIRTY();
}

void CCashUnit::SetNumber(USHORT usNumber)
{
    if (m_usNumber != usNumber)
    {
        m_usNumber = usNumber;
        SET_CHANGED_DIRTY();
    }
}


USHORT CCashUnit::GetNumber() const
{
    return m_usNumber;
}

//Physical Box Index
USHORT CCashUnit::GetIndex() const
{
    return m_usPHIndex;
}

BOOL CCashUnit::IsCDM() const
{
    return m_bCDM;
}

//identifier
// Physical Position Name
LPCSTR CCashUnit::GetPhysicalPositionName() const
{
    return m_sPhysicalPositionName.c_str();
}

long CCashUnit::SetPhysicalPositionName(LPCSTR lpPhysicalPositionName)
{
    VERIFY_SET_ITEM_ALLOWED();
    if (m_sPhysicalPositionName != lpPhysicalPositionName)
    {
        m_sPhysicalPositionName = lpPhysicalPositionName;
        SET_CHANGED_DIRTY();
    }
    return 0;
}

// Unit ID
void CCashUnit::GetUnitID(CHAR cUnitID[5]) const
{
    memcpy(cUnitID, this->m_cUnitID, 5);
}

long CCashUnit::SetUnitID(CHAR cUnitID[5])
{
    VERIFY_SET_ITEM_ALLOWED();
    if (memcmp(cUnitID, this->m_cUnitID, 5) != 0)
    {
        memcpy(this->m_cUnitID, cUnitID, 5);
        SET_CHANGED_DIRTY();
    }
    return 0;
}

// Type
ADP_CASSETTE_TYPE CCashUnit::GetType() const
{
    return m_usType;
}

long CCashUnit::SetType(ADP_CASSETTE_TYPE usType)
{
    DEFMODULE(CU.SetType);
    VERIFY_FUNC_CALL(VerifyDataMinMax(usType, ADP_CASSETTE_BILL, ADP_CASSETTE_REJECT));
    SETITEM_IN_EXCHANGE(usType);
}

ULONG CCashUnit::GetValues() const
{
    return m_ulValues;
}

long CCashUnit::SetValues(ULONG ulValues)
{
    DEFMODULE(LCUImpl.SetValues);
    VERIFY_FUNC_CALL(VerifyData(ulValues, 0, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 10000, -1));
    SETITEM_IN_EXCHANGE(ulValues);
}

// Cash Unit Name
LPCSTR CCashUnit::GetCashUnitName() const
{
    return m_sCashUnitName.c_str();
}

DWORD CCashUnit::GetItemType() const
{
    return m_fwItemType;
}

long CCashUnit::SetItemType(DWORD fwItemType)
{
    DEFMODULE(CIMLCU.SetItemType);
    VERIFY_FUNC_CALL(VerifyData(fwItemType,
                                0,
                                WFS_CIM_CITYPALL,
                                WFS_CIM_CITYPUNFIT,
                                WFS_CIM_CITYPALL | WFS_CIM_CITYPUNFIT,
                                WFS_CIM_CITYPINDIVIDUAL,
                                WFS_CIM_CITYPINDIVIDUAL | WFS_CIM_CITYPALL,
                                WFS_CIM_CITYPINDIVIDUAL | WFS_CIM_CITYPUNFIT,
                                WFS_CIM_CITYPINDIVIDUAL | WFS_CIM_CITYPUNFIT | WFS_CIM_CITYPALL,
                                -1));
    SETITEM_IN_EXCHANGE(fwItemType);
}

LPWFSCIMNOTENUMBERLIST CCashUnit::GetNoteNumbers() const
{
    return m_lpNoteNumberList;
}

ULONG CCashUnit::GetCount() const
{
    return m_ulCount;
}

long CCashUnit::SetCount(ULONG ulCount)
{
    SETITEM_NOT_IN_EXCHANGE(ulCount);
}

// Initial Count
ULONG CCashUnit::GetInitialCount() const
{
    return m_ulInitialCount;
}

long CCashUnit::SetInitialCount(ULONG ulInitialCount)
{
    SETITEM_NOT_IN_EXCHANGE(ulInitialCount);
}

// Cash In Count
ULONG CCashUnit::GetCashInCount() const
{
    return m_ulCashInCount;
}

long CCashUnit::SetCashInCount(ULONG ulCashInCount)
{
    ICashUnitConfigItem *pCfg = m_pCfgItem->GetConfig()->GetCommonConfig();
    if (!IsCDM() &&
        m_usType == ADP_CASSETTE_RETRACT &&
        pCfg != NULL &&
        pCfg->GetCUValue(0, "SetCashInCountOfRetractBoxToZero"))
    {
        ulCashInCount = 0;
    }

    SETITEM_NOT_IN_EXCHANGE(ulCashInCount);
}

// Reject Count
ULONG CCashUnit::GetRejectCount() const
{
    return m_ulRejectCount;
}

long CCashUnit::SetRejectCount(ULONG ulRejectCount)
{
    SETITEM_NOT_IN_EXCHANGE(ulRejectCount);
}

//min max
// Maximum
ULONG CCashUnit::GetMaximum() const
{
    return m_ulMaximum;
}

long CCashUnit::SetMaximum(ULONG ulMaximum)
{
    SETITEM_NOT_IN_EXCHANGE(ulMaximum);
}

// Minimum
ULONG CCashUnit::GetMinimum() const
{
    return m_ulMinimum;
}
long CCashUnit::SetMinimum(ULONG ulMinimum)
{
    SETITEM_NOT_IN_EXCHANGE(ulMinimum);
}

// Status
CASHUNIT_STATUS CCashUnit::GetStatus() const
{
    if (m_bExchangeState)
        return ComputeStatusFromCfgAndMinMax();
    ICashUnitConfigItem *pCfg = m_pCfgItem->GetConfig()->GetCommonConfig();
    int nDontSetManipWhenAnyCUInExchange = 1;
    int nUseManipInCUStats = 1;
    if (pCfg != NULL)
    {
        nUseManipInCUStats = pCfg->GetCUValue(1, "UseManipInCUStats");
        nDontSetManipWhenAnyCUInExchange = pCfg->GetCUValue(1, "DontSetManipWhenAnyCUInExchange");
    }
    if ((nUseManipInCUStats == 1) &&
        (nDontSetManipWhenAnyCUInExchange != 1 || m_ulExchangeCount == 0))
    {
        if (m_usStatus == ADP_CASHUNIT_MANIP /*||
            m_usStatus == ADP_CASHUNIT_MISSING*/)
            return ADP_CASHUNIT_MANIP;
    }
    if (m_bVerifyCUInfoFailed)
    {
        return ADP_CASHUNIT_INOP;
    }

    return ComputeStatusFromCfgAndMinMax();
}

long CCashUnit::SetStatus(CASHUNIT_STATUS usStatus, BOOL bForce)
{
    DEFMODULE(LCUImpl.SetStatus);
    VERIFY_FUNC_CALL(VerifyDataMinMax(usStatus, 0, 9));

    if (bForce)
    {
        m_usStatus = usStatus;
        return 0;
    }
    if (usStatus != m_usStatus) //如果新状态不同于老状态
    {
        CASHUNIT_STATUS eOldStatus = GetStatus();   //保存老状态
        if (eOldStatus == ADP_CASHUNIT_MANIP && !m_bExchangeState)
        {
            return 0;
        }

        if (!m_bExchangeState)
        {
            BOOL bCanSetManIP = FALSE;
            ICashUnitConfigItem *pCfg = m_pCfgItem->GetConfig()->GetCommonConfig();
            if (m_ulExchangeCount == 0)
            {
                bCanSetManIP = TRUE;
            }
            else
            {
                int nDontSetManipWhenAnyCUInExchange = 1;
                if (pCfg != NULL)
                {
                    nDontSetManipWhenAnyCUInExchange = pCfg->GetCUValue(1, "DontSetManipWhenAnyCUInExchange");
                }
                if (nDontSetManipWhenAnyCUInExchange != 1)
                {
                    bCanSetManIP = TRUE;
                }
            }

            int nUseManipInCUStats = 1;
            if (pCfg != NULL)
            {
                nUseManipInCUStats = pCfg->GetCUValue(1, "UseManipInCUStats");
            }
            if (bCanSetManIP && (nUseManipInCUStats == 1))
            {
//30-00-00-00(FT#0003)                if (eOldStatus != ADP_CASHUNIT_MISSING
//30-00-00-00(FT#0003)                    && (usStatus == ADP_CASHUNIT_MISSING
//30-00-00-00(FT#0003)                        /*|| usStatus == ADP_CASHUNIT_UNKNOWN*/))
//30-00-00-00(FT#0003)                {
//30-00-00-00(FT#0003)                    usStatus = ADP_CASHUNIT_MANIP;
//30-00-00-00(FT#0003)                }

                if ((eOldStatus == ADP_CASHUNIT_MISSING)
                    && usStatus != ADP_CASHUNIT_MISSING)
                {
                    usStatus = ADP_CASHUNIT_MANIP;
                }
            }
        }
        m_usStatus = usStatus;  //给状态赋值
        SetDirty(TRUE);
        CASHUNIT_STATUS eNewStatus = GetStatus();
        if (eOldStatus != eNewStatus)
        {
            SetItemChanged();
        }
    }
    return 0;
}

// Hardware Sensors
BOOL CCashUnit::GetHardwareSensors() const
{
    return m_bHardwareSensors;
}

long CCashUnit::SetHardwareSensors(BOOL bHardwareSensors)
{
    DEFMODULE(CU.SetHardwareSensors);
    SETITEM_IN_EXCHANGE(bHardwareSensors);
}

// AppLock
BOOL CCashUnit::GetAppLock() const
{
    return m_bAppLock;
}

long CCashUnit::SetAppLock(BOOL bAppLock)
{
    SETITEM_NOT_IN_EXCHANGE(bAppLock);
}

DWORD CCashUnit::GetErrorCount() const
{
    return m_dwErrorCount;
}

void CCashUnit::SetErrorCount(DWORD dwErrorCount)
{
    if (m_dwErrorCount != dwErrorCount)
    {
        m_dwErrorCount = dwErrorCount;
        SetDirty();
    }
}

// Exchange state
BOOL CCashUnit::GetExchangeState() const
{
    return m_bExchangeState;
}

long CCashUnit::SetExchangeState(BOOL bEx)
{
    if (!m_bExchangeState && bEx)
        m_ulExchangeCount++;
    else if (m_bExchangeState && !bEx)
        m_ulExchangeCount--;
    m_bExchangeState = bEx;
    return 0;
}

//设置数据改变标志
void CCashUnit::SetItemChanged()
{
    m_bItemChanged = TRUE;
}

// Check for changes of any item
// 如果任何数据元素有改变,那么返回一次TRUE.
//在没有调用新的设置操作导致数据改变的情况发生前,重复调用将返回FALSE.
BOOL CCashUnit::IsItemChanged()
{
    BOOL bItemChanged = m_bItemChanged;
    m_bItemChanged = FALSE;
    return bItemChanged;
}

//设置是否在SetByXFSFormat函数中的标志
void CCashUnit::SetInSetByXFSFormat(BOOL bIn)
{
    m_bInSetByXFSFormat = bIn;
}
