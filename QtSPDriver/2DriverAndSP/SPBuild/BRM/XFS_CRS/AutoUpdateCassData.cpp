#include "AutoUpdateCassData.h"
#include "XFS_CRS.h"

#define THISFILE    "AutoUpdateCassData"

CAutoUpdateCassData::CAutoUpdateCassData(XFS_CRSImp *pSP,
                                         ULONG ulDispenseCounts[ADP_MAX_CU_SIZE],
                                         ULONG ulRejectCounts[ADP_MAX_CU_SIZE],
                                         ULONG ulRetractCount[ADP_MAX_CU_SIZE],
                                         NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE],
                                         BOOL bCDMOperation,
                                         BOOL bSubstractRejectCountFromCount)
{
    m_pSP = pSP;

    m_bCDMOperation = bCDMOperation;
    m_pulDispenseCounts = ulDispenseCounts;
    m_pulRejectCounts = ulRejectCounts;
    m_pulRetractCounts = ulRetractCount;
    m_pNoteIDCountArray = NoteIDCountArray;
    m_bSubstractRejectCountFromCount = bSubstractRejectCountFromCount;
}

CAutoUpdateCassData::~CAutoUpdateCassData()
{
    UpdateCountToCUManager();
}

void CAutoUpdateCassData::UpdateCountToCUManager()
{
    const char *ThisModule = "UpdateCountToCUManager";

    USHORT usRejIndex = 1;
    BOOL bHaveRejBox = m_pSP->FindCassetteADPIndex(FCAIF_REJECT, TRUE, usRejIndex) >= 0;
    for (int iCUInterface = 0; iCUInterface < 2; iCUInterface++)
    {
        ICUInterface *pCUInterface;
        ICUInterface *pCUInterfacePeer;
        if (iCUInterface == 0)
        {
            pCUInterface = m_pSP->m_pCUManager->GetCUInterface_CIM();
            pCUInterfacePeer = m_pSP->m_pCUManager->GetCUInterface_CDM();
        }
        else
        {
            pCUInterface = m_pSP->m_pCUManager->GetCUInterface_CDM();
            pCUInterfacePeer = m_pSP->m_pCUManager->GetCUInterface_CIM();
        }

        vector<USHORT> FireCountChangedArray;
        USHORT usCount = pCUInterface->GetCUCount();
        for (USHORT i = 0; i < usCount; i++)
        {
            ICashUnit *pCU = pCUInterface->GetCUByNumber(i + 1);
            assert(pCU != NULL);
            if (pCU == NULL)
                continue;
            USHORT usIndex = pCU->GetIndex();
            if (usIndex < 1 || usIndex > ADP_MAX_CU_SIZE)
            {
                assert(FALSE);
                continue;
            }
            usIndex--;

            BOOL bChanged = FALSE;
            if (m_pulDispenseCounts != NULL && m_pulDispenseCounts[usIndex] > 0)
            {
                bChanged = TRUE;
                if (pCU->GetCount() > m_pulDispenseCounts[usIndex])
                {
                    pCU->SetCount(pCU->GetCount() - m_pulDispenseCounts[usIndex]);
                }
                else
                {
                    pCU->SetCount(0);
                }
                if (pCU->GetType() == ADP_CASSETTE_RECYCLING)
                {
                    USHORT aryusNoteIDs[MAX_NOTEID_SIZE];
                    DWORD dwNoteIDCount = pCU->GetNoteIDs(aryusNoteIDs);
                    ULONG ulDispenseCount = m_pulDispenseCounts[usIndex];
                    for (DWORD dwNoteIDIndex = 0;
                         dwNoteIDIndex < dwNoteIDCount && ulDispenseCount > 0;
                         dwNoteIDIndex++)
                    {
                        ULONG ulCount = 0;
                        pCU->GetNumberByNote(aryusNoteIDs[dwNoteIDIndex], ulCount);
                        if (ulCount >= ulDispenseCount)
                        {
                            ulCount -= ulDispenseCount;
                            ulDispenseCount = 0;
                        }
                        else
                        {
                            ulDispenseCount -= ulCount;
                            ulCount = 0;
                        }
                        pCU->SetNumbersByNote(aryusNoteIDs[dwNoteIDIndex], ulCount);

                    }
                }
            }

            if (m_pulRejectCounts != NULL && m_pulRejectCounts[usIndex] > 0)
            {
                bChanged = TRUE;
                if (pCU->GetType() == ADP_CASSETTE_RETRACT || pCU->GetType() == ADP_CASSETTE_REJECT)
                {
                    if ((m_pNoteIDCountArray == NULL) &&
                        ((pCU->GetType() == ADP_CASSETTE_RETRACT && !bHaveRejBox) || pCU->GetType() == ADP_CASSETTE_REJECT))
                    {
                        pCU->SetCount(pCU->GetCount() + m_pulRejectCounts[usIndex]);
                    }
                }
                else
                {
                    pCU->SetRejectCount(pCU->GetRejectCount() + m_pulRejectCounts[usIndex]);
                    if (m_bSubstractRejectCountFromCount)
                    {
                        if (pCU->GetCount() > m_pulRejectCounts[usIndex])
                            pCU->SetCount(pCU->GetCount() - m_pulRejectCounts[usIndex]);
                        else
                            pCU->SetCount(0);
                    }
                }
            }

            if (m_pulRetractCounts != NULL && m_pulRetractCounts[usIndex] > 0)
            {
                bChanged = TRUE;
                if (pCU->GetType() == ADP_CASSETTE_RETRACT)
                {
                    pCU->SetCount(pCU->GetCount() + m_pulRetractCounts[usIndex]);
                    pCU->SetCashInCount(pCU->GetCashInCount() + m_pulRetractCounts[usIndex]);
                }
                else if ((pCU->GetType() == ADP_CASSETTE_BILL)
                         || (pCU->GetType() == ADP_CASSETTE_RECYCLING)
                         || (pCU->GetType() == ADP_CASSETTE_CASHIN))
                {
                    if (pCU->GetCount() >= m_pulRetractCounts[usIndex])
                    {
                        pCU->SetCount(pCU->GetCount() - m_pulRetractCounts[usIndex]);
                    }
                    else
                    {
                        pCU->SetCount(0);
                    }
                }
            }

            if (m_pNoteIDCountArray != NULL)
            {
                ULONG ulTotal = 0;
                for (NOTEID_COUNT_ARRAY::iterator it = m_pNoteIDCountArray[usIndex].begin();
                     it != m_pNoteIDCountArray[usIndex].end(); it++)
                {
                    ulTotal += it->second;
                    //add to note number of CU
//30-00-00-00(FS#0021)                    if (it->first != NOTE_ID_UNKNOWN)
//30-00-00-00(FS#0021)                    {
                        ULONG ulCount = 0;
                        if (pCU->GetNumberByNote(it->first, ulCount) < 0)
                            ulCount = 0;
                        pCU->SetNumbersByNote(it->first, ulCount + it->second);
//30-00-00-00(FS#0021)                    }
                }

                if (ulTotal > 0)
                {
                    bChanged = TRUE;
                    pCU->SetCount(pCU->GetCount() + ulTotal);
                    pCU->SetCashInCount(pCU->GetCashInCount() + ulTotal);
                    m_pSP->Log("UpdateCountToCUManager", 1, "Cass%d(Index%d) SetCountAdd: %d", i, usIndex, ulTotal);
                }
            }

            ICashUnit *pCUPeer = pCUInterfacePeer->GetFirstCUByPHIndex(pCU->GetIndex());
            if (bChanged && pCUPeer != NULL)
            {
                FireCountChangedArray.push_back(pCUPeer->GetNumber());
            }
        }

        //Fire WFS_SRVE_XXX_COUNTS_CHANGED
        if (FireCountChangedArray.size() > 0)
        {

            if (m_bCDMOperation)
            {
                if (!pCUInterfacePeer->IsCDM())
                {
                    //m_pSP->CIMFireCountsChanged(FireCountChangedArray.begin(), (USHORT)FireCountChangedArray.size());
                }
            }
            else // CIM Operation
            {
                if (pCUInterfacePeer->IsCDM())
                {
                    //m_pSP->CDMFireCountsChanged(FireCountChangedArray.begin(), (USHORT)FireCountChangedArray.size());
                }
            }
        }
    }

    m_pSP->UpdateADPCassInfoFromCUManager();
}
