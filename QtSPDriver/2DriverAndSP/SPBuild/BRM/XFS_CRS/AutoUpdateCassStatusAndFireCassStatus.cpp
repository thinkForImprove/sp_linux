#include "AutoUpdateCassStatusAndFireCassStatus.h"
#include "XFS_CRS.h"
#include <assert.h>
#include "SPUtil.h"

#define THISFILE    "AutoUpdateCassStatus"

//钞箱类型转为描述
static inline const char *CassType2Desc(ADP_CASSETTE_TYPE eType)
{
    switch (eType)
    {
    case ADP_CASSETTE_BILL: return "OUT ";
    case ADP_CASSETTE_RECYCLING: return "RECY";
    case ADP_CASSETTE_CASHIN: return " IN ";
    case ADP_CASSETTE_RETRACT: return "RETR";
    case ADP_CASSETTE_REJECT: return "REJE";
    case ADP_CASSETTE_UNKNOWN:
    default:
        return "UNKN";
    }
}

//转换钞箱状态为描述
static inline const char *CassStatus2Desc(CASHUNIT_STATUS eStatus)
{
    switch (eStatus)
    {
    case ADP_CASHUNIT_OK: return " OK ";
    case ADP_CASHUNIT_FULL: return "FULL";
    case ADP_CASHUNIT_HIGH: return "HIGH";
    case ADP_CASHUNIT_LOW: return "LOW ";
    case ADP_CASHUNIT_EMPTY: return "EMPT";
    case ADP_CASHUNIT_INOP: return "INOP";
    case ADP_CASHUNIT_MISSING: return "MISS";
    case ADP_CASHUNIT_MANIP: return "MANI";
    case ADP_CASHUNIT_UNKNOWN: return "UN";
    default:
        {
            static char arycBuf[20];
            sprintf(arycBuf, "S%-3d", eStatus);
            return arycBuf;
        }
    }
}

//形成钞箱信息
static const char *FormatCassInfo(ICashUnit *pCU)
{
    static char buf[100];
    if (pCU->IsCDM())
    {
        sprintf(buf, "[%hd]%s%-3d:%sC%dR%d",
                pCU->GetNumber(), CassType2Desc(pCU->GetType()),
                pCU->GetValues(), CassStatus2Desc(pCU->GetStatus()),
                pCU->GetCount(), pCU->GetRejectCount());
    }
    else
    {
        sprintf(buf, "[%hd]%s%-3d:%sC%dI%d",
                pCU->GetNumber(), CassType2Desc(pCU->GetType()),
                pCU->GetValues(), CassStatus2Desc(pCU->GetStatus()),
                pCU->GetCount(), pCU->GetCashInCount());
    }

    return buf;
}

//////////////////////////////////////////////////////////////////////
// CAutoUpdateCassStatusAndFireCassStatus
//////////////////////////////////////////////////////////////////////
CASHUNIT_STATUS CAutoUpdateCassStatusAndFireCassStatus::m_CassStatus[ADP_MAX_CU_SIZE] = {ADP_CASHUNIT_UNKNOWN};

CU2STATUS CAutoUpdateCassStatusAndFireCassStatus::m_mapCU2Status;

CAutoUpdateCassStatusAndFireCassStatus::CAutoUpdateCassStatusAndFireCassStatus(XFS_CRSImp *pSP)
{
    assert(pSP != nullptr);

    m_pSP = pSP;
    if (m_CassStatus[0] == ADP_CASHUNIT_UNKNOWN)
    {
        for (USHORT i = 0; i < ADP_MAX_CU_SIZE; i++)
        {
            char cCassID[128] = {0};
            m_pSP->m_pAdapter->GetCassetteInfo(i + 1, m_CassStatus[i], cCassID);
        }
    }
}

CAutoUpdateCassStatusAndFireCassStatus::~CAutoUpdateCassStatusAndFireCassStatus()
{
    const char *ThisModule = "Destructor";

    assert(m_pSP->m_pCUManager != nullptr);
    //assert(m_pSP->m_pAdapter != nullptr);

    //Get ADP Cassette Info
    BOOL bHasBoxInsertOrTaken = FALSE;
    BRMCASHUNITINFOR aryCUInfor[ADP_MAX_CU_SIZE];
    USHORT i;
    for (i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        //HOPPER: from 1 to ADP_MAX_CU_SIZE
        CASHUNIT_STATUS eStatus;
        char cCassID[128] = {0};
        m_pSP->m_pAdapter->GetCassetteInfo(i + 1, eStatus, cCassID);
        aryCUInfor[i].SetStatus(eStatus);
        aryCUInfor[i].SetUnitID(cCassID);
        if ((eStatus == ADP_CASHUNIT_MISSING || m_CassStatus[i] == ADP_CASHUNIT_MISSING) &&
            eStatus != m_CassStatus[i])
        {
            bHasBoxInsertOrTaken = TRUE;
        }

        m_CassStatus[i] = eStatus; //保存钞箱状态
    }
    if (bHasBoxInsertOrTaken &&
        m_pSP->m_Param.IsBeepWhenBoxInsertedOrTaken())
    {
        //MessageBeep(MB_ICONEXCLAMATION); todo
    }

    //Fire Cassette Status Changed and Threshold Event
    //并记录钞箱信息
    string sLog;    //钞箱信息
    for (int iInterface = 0; iInterface < 2; iInterface++)
    {
        BOOL bCassInfoChanged = FALSE;  //钞箱状态是否改变
        ICUInterface *pInterface;
        if (iInterface == 0)
        {
            sLog = "CDM钞箱状态:";
            pInterface = m_pSP->m_pCUManager->GetCUInterface_CDM();
        }
        else
        {
            sLog = "CIM钞箱状态:";
            pInterface = m_pSP->m_pCUManager->GetCUInterface_CIM();
        }

        for (i = 0; i < pInterface->GetCUCount(); i++)
        {
            //设置适配层钞箱状态到钞箱管理模块中
            ICashUnit *pCU = pInterface->GetCUByNumber(i + 1);
            assert(pCU != NULL);
            if (pCU->GetExchangeState())    //交换状态清钞箱错误次数
            {
                pCU->SetErrorCount(0);
            }
            USHORT usIndex = pCU->GetIndex();
            if (usIndex > 0 && usIndex <= ADP_MAX_CU_SIZE)
            {
                usIndex--;
                CASHUNIT_STATUS eStatus = aryCUInfor[usIndex].GetStatus();
                pCU->SetStatus(eStatus);
                pCU->SetUnitID((char *)aryCUInfor[usIndex].GetUnitID());
            }

            //FIRE钞箱状态改变
            if (pCU->IsItemChanged())
            {
                bCassInfoChanged = TRUE;

                BOOL bNeedFireEvent = FALSE;

                // 广农商应用对CUInfoChanged事件的处理比较敏感，在不恰当的时机发送CUInfoChanged事件可能会导致应用程序重启机器。
                if (m_pSP->m_bInCmdExecute)     // 在某命令执行过程中
                {
                    if (pCU->IsCDM() &&
                        (m_pSP->m_dwCurrentCommand == WFS_CMD_CDM_SET_CASH_UNIT_INFO    ||
                         m_pSP->m_dwCurrentCommand == WFS_CMD_CDM_END_EXCHANGE           ||
                         m_pSP->m_dwCurrentCommand == WFS_CMD_CDM_CALIBRATE_CASH_UNIT    ||
                         m_pSP->m_dwCurrentCommand == WFS_CMD_CDM_TEST_CASH_UNITS))
                    {
                        bNeedFireEvent = TRUE;
                    }
                    else if (!pCU->IsCDM() &&
                             m_pSP->m_dwCurrentCommand == WFS_CMD_CIM_CASH_IN_END    ||
                             m_pSP->m_dwCurrentCommand == WFS_CMD_CIM_SET_CASH_UNIT_INFO ||
                             m_pSP->m_dwCurrentCommand == WFS_CMD_CIM_END_EXCHANGE       ||
                             m_pSP->m_dwCurrentCommand == WFS_CMD_CIM_CONFIGURE_CASH_IN_UNITS)
                    {
                        bNeedFireEvent = TRUE;
                    }
                }
                else
                {
                    // 空闲的时候可以发
                    bNeedFireEvent = TRUE;
                }

                if (bNeedFireEvent)
                {
                    if (pCU->IsCDM())
                    {
                        m_pSP->CDMFireCUInfoChanged(pCU->BuildByXFSCDMFormat());
                        //                      if (ADP_CASSETTE_RECYCLING == pCU->GetType())
                        //                          m_pSP->CIMFireCUInfoChanged(pCU->BuildByXFSCIMFormat()); //by tucy 20160727  多余的事件发送，会导致部分数据错误，所以取消
                    }
                    else
                    {
                        m_pSP->CIMFireCUInfoChanged(pCU->BuildByXFSCIMFormat());
                        //                      if (ADP_CASSETTE_RECYCLING == pCU->GetType())
                        //                          m_pSP->CDMFireCUInfoChanged(pCU->BuildByXFSCDMFormat());
                    }
                }
            }

            //FIRE钞箱Threshold事件
            CASHUNIT_STATUS eStatus = pCU->GetStatus();
            CU2STATUSIT it = m_mapCU2Status.find(pCU);
            if (it != m_mapCU2Status.end())
            {
                if ((eStatus == ADP_CASHUNIT_HIGH && it->second == ADP_CASHUNIT_OK) ||
                    (eStatus == ADP_CASHUNIT_LOW  && it->second == ADP_CASHUNIT_OK) // 当钞箱状态变为将满或少钞时才发
                    /*it->second != ADP_CASHUNIT_INOP && it->second != ADP_CASHUNIT_MISSING && it->second != ADP_CASHUNIT_MANIP &&
                    eStatus != ADP_CASHUNIT_INOP && eStatus != ADP_CASHUNIT_MISSING && eStatus != ADP_CASHUNIT_MANIP &&
                    eStatus != ADP_CASHUNIT_OK*/)
                {
                    if (pCU->IsCDM())
                        m_pSP->CDMFireCashUnitThreshold(pCU->BuildByXFSCDMFormat());
                    else
                        m_pSP->CIMFireCashUnitThreshold(pCU->BuildByXFSCIMFormat());
                }
            }
            m_mapCU2Status[pCU] = pCU->GetStatus();

            sLog += FormatCassInfo(pCU);
        }
        if (bCassInfoChanged) //钞箱信息改变后重新记录
        {
            m_pSP->Log(ThisModule, 0, sLog.c_str());
        }
    }

    m_pSP->UpdateNoteTypeListDepositable();
}

