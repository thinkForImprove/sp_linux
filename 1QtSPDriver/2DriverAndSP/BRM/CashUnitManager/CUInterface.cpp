//CUInterface.cpp

//#include "StdAfx.h"
#include "CUInterface.h"
#include "AutoRollback.h"
#include <vector>
using namespace std;
static const char *ThisFile = "CUInterface";
CCUInterface::CCUInterface(BOOL bCDM) : m_bCDM(bCDM)
{
    SetLogFile(LOGFILE, ThisFile, ThisFile);
    InitData();
}

//析构函数
//调用ClearData删除由自己分配的内存
CCUInterface::~CCUInterface()
{
    ClearData();
}

BOOL CCUInterface::IsCDM() const
{
    return m_bCDM;
}


//初始化数据，清除本类接口相关数据
//仅在构造函数中调用
void CCUInterface::InitData()
{
    m_lpCDMCashInfo = NULL;
    m_lpCIMCashInfo = NULL;
    m_itGetCUByPHIndex = m_CUs.end();
}

//清除本类接口相关数据，如有指针，释放其内存
void CCUInterface::ClearData()
{
    if (m_lpCDMCashInfo != NULL)
    {
        if (m_lpCDMCashInfo->lppList != NULL)
        {
            delete [] m_lpCDMCashInfo->lppList;
        }
        delete m_lpCDMCashInfo;
        m_lpCDMCashInfo = NULL;
    }

    if (m_lpCIMCashInfo != NULL)
    {
        if (m_lpCIMCashInfo->lppCashIn != NULL)
        {
            delete [] m_lpCIMCashInfo->lppCashIn;
        }
        delete m_lpCIMCashInfo;
        m_lpCIMCashInfo = NULL;
    }
    m_itGetCUByPHIndex = m_CUs.end();
}

void CCUInterface::CopyFrom(const CCUInterface &src, BOOL bBackup)
{
    if (bBackup)
    {
        vector<CCashUnit *>::iterator it;
        vector<CCashUnit *> *pSrcCUs = (vector<CCashUnit *> *)&src.m_CUs;
        for (it = pSrcCUs->begin(); it != pSrcCUs->end(); it++)
        {
            CCashUnit *pLCU = new CCashUnit((*it)->GetAllNoteTypeList(), TRUE);
            pLCU->CopyFrom(*(*it), bBackup);
            m_CUs.push_back(pLCU);
        }
    }
    else
    {
        assert(src.m_CUs.size() == m_CUs.size());
        for (USHORT i = 0; i < m_CUs.size(); i++)
        {
            m_CUs[i]->CopyFrom(*src.m_CUs[i], bBackup);
        }
    }
}


int CCUInterface::LoadFromConfig(ICashUnitConfig *pConfig,  const LPWFSCIMNOTETYPELIST pNoteTypeList, BOOL bCDM)
{
    DEFMODULE(CUInterface.LoadFromConfig);

    ClearData();

    map<USHORT, USHORT> mapPHIndex2RefCount; //物理钞箱索引到引用次数映射
    CONFIG_TYPE ct = bCDM ? CT_LOG_CDM : CT_LOG_CIM;
    DWORD dwCount = pConfig->GetCUConfigCount(ct);
    for (DWORD i = 0; i < dwCount; i++)
    {
        ICashUnitConfigItem *pCfgItem = pConfig->GetCUConfig(ct, i);
        assert(pCfgItem != NULL);
        CCashUnit *pCU = new CCashUnit(pNoteTypeList, bCDM);
        int iRet = pCU->LoadConfig(pCfgItem);
        if (iRet < 0)
        {
            delete pCU;
            return iRet;
        }

        //校验NUMBER必须依次增加1
        if (pCU->GetNumber() != i + 1)
        {
            Log(ThisModule, -1,
                "number config eror(pCU->GetNumber()[%hd] != %d)",
                pCU->GetNumber(), i + 1);
            delete pCU;
            return WFS_ERR_SOFTWARE_ERROR;
        }
        m_CUs.push_back(pCU);

        USHORT usPHIndex = pCU->GetIndex();
        auto it = mapPHIndex2RefCount.find(usPHIndex);
        if (it == mapPHIndex2RefCount.end())
        {
            mapPHIndex2RefCount[pCU->GetIndex()] = 1;
            //it = mapPHIndex2RefCount.find(usPHIndex);
        }
        else
        {
            mapPHIndex2RefCount[pCU->GetIndex()]++;
            if (pCU->GetType() != ADP_CASSETTE_RETRACT &&
                pCU->GetType() != ADP_CASSETTE_REJECT)
            {
                Log(ThisModule, -1,
                    "物理钞箱(index=%hd)被重复引用",
                    pCU->GetIndex());
                return WFS_ERR_SOFTWARE_ERROR;
            }
        }
    }

    m_itGetCUByPHIndex = m_CUs.end();
    return 0;
}

// Convert the information to XFS WFSCIMCASHINFO
LPWFSCIMCASHINFO CCUInterface::BuildByXFSCIMFormat()
{
    DWORD dwCount = m_CUs.size();
    if (m_lpCIMCashInfo == NULL)
    {
        m_lpCIMCashInfo = new WFSCIMCASHINFO();
        m_lpCIMCashInfo->lppCashIn = NULL;
    }

    if (m_lpCIMCashInfo->lppCashIn != NULL)
    {
        delete [] m_lpCIMCashInfo->lppCashIn;
        m_lpCIMCashInfo->lppCashIn = NULL;
    }

    if (dwCount > 0)
    {
        m_lpCIMCashInfo->lppCashIn = new LPWFSCIMCASHIN[dwCount];
    }

    m_lpCIMCashInfo->usCount = (USHORT)dwCount;
    for (DWORD i = 0; i < dwCount; i++)
    {
        m_lpCIMCashInfo->lppCashIn[i] = m_CUs[i]->BuildByXFSCIMFormat();
    }

    return m_lpCIMCashInfo;
}

// Convert the information to XFS WFSCDMCASHINFO
LPWFSCDMCUINFO CCUInterface::BuildByXFSCDMFormat()
{
    DWORD dwCount = m_CUs.size();
    if (m_lpCDMCashInfo == NULL)
    {
        m_lpCDMCashInfo = new WFSCDMCUINFO();
        m_lpCDMCashInfo->usTellerID = 0;
        m_lpCDMCashInfo->lppList = NULL;
    }

    if (m_lpCDMCashInfo->lppList != NULL)
    {
        delete [] m_lpCDMCashInfo->lppList;
        m_lpCDMCashInfo->lppList = NULL;
    }

    if (dwCount > 0)
    {
        m_lpCDMCashInfo->lppList = new LPWFSCDMCASHUNIT[dwCount];
    }

    m_lpCDMCashInfo->usCount = (USHORT)dwCount;
    for (DWORD i = 0; i < dwCount; i++)
    {
        m_lpCDMCashInfo->lppList[i] = m_CUs[i]->BuildByXFSCDMFormat();
    }

    return m_lpCDMCashInfo;
}

// Set the information from XFS WFSCIMCASHINFO
long CCUInterface::SetByXFSCIMFormat(const LPWFSCIMCASHINFO lpCUInfor)
{
    DEFMODULE(CUInterface.SetByXFSCIMFormat);

    if (lpCUInfor == NULL)
    {
        Log(ThisModule, -1,
            "lpCUInfor = NULL");
        return WFS_ERR_INVALID_POINTER;
    }

    if (lpCUInfor->usCount != m_CUs.size())
    {
        Log(ThisModule, -1,
            "lpCUInfor->usCount(%hd) != m_CUs.size()(%d)",
            lpCUInfor->usCount, m_CUs.size());
        return WFS_ERR_INVALID_DATA;
    }

    long lRet = 0;
    CAutoRollback<CCUInterface> _auto_rollback(this, &lRet);
    for (USHORT i = 0; i < lpCUInfor->usCount; i++)
    {
        LPWFSCIMCASHIN pCU = lpCUInfor->lppCashIn[i];
        lRet = m_CUs[i]->SetXFSData(pCU);
        if (lRet < 0)
        {
            return lRet;
        }
    }

    return 0;
}

// Set the information from XFS WFSCIMCASHINFO
long CCUInterface::SetByXFSCDMFormat(const LPWFSCDMCUINFO lpCUInfor)
{
    DEFMODULE(CUInterface.SetByXFSCDMFormat);


    if (lpCUInfor == NULL)
    {
        Log(ThisModule, -1,
            "lpCUInfor = NULL");
        return WFS_ERR_INVALID_POINTER;
    }

    if (lpCUInfor->usCount != m_CUs.size())
    {
        Log(ThisModule, -1,
            "lpCUInfor->usCount(%hd) != m_CUs.size()(%d)",
            lpCUInfor->usCount, m_CUs.size());
        return WFS_ERR_INVALID_DATA;
    }

    long lRet = 0;
    CAutoRollback<CCUInterface> _auto_rollback(this, &lRet);
    for (USHORT i = 0; i < lpCUInfor->usCount; i++)
    {
        LPWFSCDMCASHUNIT pCU = lpCUInfor->lppList[i];
        lRet = m_CUs[i]->SetXFSData(pCU);
        if (lRet < 0)
        {
            return lRet;
        }
    }

    return 0;
}

long CCUInterface::SyncDataByPhysicalIndex(ICUInterface *pSrc, BOOL bModifyStats)
{
    const char *ThisModule = "SyncDataByPhysicalIndex";
    USHORT usCount = pSrc->GetCUCount();
    USHORT i = 0;
    USHORT j = 0;
    for (i = 0; i < usCount; i++)
    {
        ICashUnit *pSrcCU = pSrc->GetCUByNumber(i + 1);
        //查找物理索引相同的钞箱
        ICashUnit *pDestCU = NULL;
        for (j = 0; j < GetCUCount(); j++)
        {
            pDestCU = GetCUByNumber(j + 1);
            if (pDestCU->GetIndex() == pSrcCU->GetIndex())
                break;
        }

        //如果没有找到，如果源钞箱是循环箱，则增加
        if (j == GetCUCount())
        {
            if (pSrcCU->GetType() != ADP_CASSETTE_RECYCLING || //不是循环箱，跳过
                !pSrcCU->GetExchangeState())                   //不是交换状态，跳过
            {
                continue;
            }
            //增加
            CCashUnit *pCU = new CCashUnit(((CCashUnit *)pSrcCU)->GetAllNoteTypeList(), IsCDM());
            pCU->m_pCfgItem = ((CCashUnit *)pSrcCU)->m_pCfgItem->GetConfig()->AddCUConfig(
                              IsCDM() ? CT_LOG_CDM : CT_LOG_CIM);
            pCU->SetIndex(pSrcCU->GetIndex()); //指向相同的物理钞箱
            pCU->SetNumber(GetCUCount() + 1);  //放到最后一个钞箱位置
            pCU->SetType(ADP_CASSETTE_RECYCLING);
            pDestCU = pCU;
            m_CUs.push_back(pCU);
        }
        else //找到
        {
            //如果源钞箱是交换状态、并且自己是循环箱并且对方不是循环箱，则删除
            if (pSrcCU->GetExchangeState() &&
                pSrcCU->GetType() != ADP_CASSETTE_RECYCLING &&
                pDestCU->GetType() == ADP_CASSETTE_RECYCLING)
            {
                //delete it
                CCashUnit *pCU = (CCashUnit *)pDestCU;
                pCU->m_pCfgItem->GetConfig()->DeleteCUConfig(
                IsCDM() ? CT_LOG_CDM : CT_LOG_CIM,
                pCU->m_pCfgItem);
                m_CUs.erase(m_CUs.begin() + j);
                delete pCU;

                //修改钞箱NUMBER
                for (UINT iCUIndex = 0; iCUIndex < m_CUs.size(); iCUIndex++)
                {
                    CCashUnit *pCU = m_CUs[iCUIndex];
                    if (pCU->GetNumber() != iCUIndex + 1)
                    {
                        pCU->SetNumber(iCUIndex + 1);
                    }
                }
                continue;
            }

            //如果源钞箱不是回收箱或循环箱，跳过
            if (pSrcCU->GetType() != ADP_CASSETTE_RECYCLING &&
                pSrcCU->GetType() != ADP_CASSETTE_RETRACT)
                continue;
            //如果源钞箱和目标钞箱类型不一致，不更新
            if (pSrcCU->GetType() != pDestCU->GetType())
                continue;
        }

        //同步数据
        pDestCU->SetCashUnitName(pSrcCU->GetCashUnitName());
        pDestCU->SetCount(pSrcCU->GetCount());
        char cCurrencyID[3];
        pSrcCU->GetCurrencyID(cCurrencyID);
        pDestCU->SetCurrencyID(cCurrencyID);
        pDestCU->SetHardwareSensors(pSrcCU->GetHardwareSensors());
        pDestCU->SetPhysicalPositionName(pSrcCU->GetPhysicalPositionName());
        char cUnitID[5];
        pSrcCU->GetUnitID(cUnitID);
        pDestCU->SetUnitID(cUnitID);
        pDestCU->SetValues(pSrcCU->GetValues());

        //add by tucy
        if (bModifyStats)
        {
            pDestCU->SetStatus(pSrcCU->GetStatus(), TRUE);
        }

        char szLog[252];
        long lVerifyResult = ((CCashUnit *)pDestCU)->VerifyCassInfo(TRUE, szLog);
        ((CCashUnit *)pDestCU)->SetVerifyCUInfoFailedFlag(lVerifyResult < 0);
        if (lVerifyResult < 0)
        {
            Log(ThisModule, -1,  "%s", szLog);
        }
    }

    m_itGetCUByPHIndex = m_CUs.end();
    return 0;
}

// Count of Logical cash unit
USHORT CCUInterface::GetCUCount() const
{
    return (USHORT)m_CUs.size();
}

// Get Logical cash unit information by Number
ICashUnit *CCUInterface::GetCUByNumber(USHORT usNumber)
{
    vector<CCashUnit *>::iterator it;
    for (it = m_CUs.begin(); it != m_CUs.end(); it++)
    {
        if ((*it)->GetNumber() == usNumber)
        {
            return *it;
        }
    }
    return NULL;
}

// Get First Logical cash unit information by physical Index
ICashUnit *CCUInterface::GetFirstCUByPHIndex(USHORT usPHIndex)
{
    for (m_itGetCUByPHIndex = m_CUs.begin(); m_itGetCUByPHIndex != m_CUs.end(); m_itGetCUByPHIndex++)
    {
        if ((*m_itGetCUByPHIndex)->GetIndex() == usPHIndex)
        {
            return *m_itGetCUByPHIndex;
        }
    }
    return NULL;
}

// Get Next Logical cash unit information by physical Index
// return NULL if not existing
ICashUnit *CCUInterface::GetNextCUByPHIndex(USHORT usPHIndex)
{
    if (m_itGetCUByPHIndex == m_CUs.end())
        return NULL;
    m_itGetCUByPHIndex++;
    for (; m_itGetCUByPHIndex != m_CUs.end(); m_itGetCUByPHIndex++)
    {
        if ((*m_itGetCUByPHIndex)->GetIndex() == usPHIndex)
        {
            return *m_itGetCUByPHIndex;
        }
    }
    return NULL;
}

int CCUInterface::SaveToConfig()
{
    DEFMODULE(CUIFImpl.SaveToConfig);
    DWORD dwCount = m_CUs.size();
    for (DWORD i = 0; i < dwCount; i++)
    {
        VERIFY_FUNC_CALL(m_CUs[i]->SaveConfig());
    }

    return 0;
}

BOOL CCUInterface::IsDirty() const
{
    DWORD dwCount = m_CUs.size();
    for (DWORD i = 0; i < dwCount; i++)
    {
        if (m_CUs[i]->IsDirty())
            return TRUE;
    }

    return FALSE;
}
