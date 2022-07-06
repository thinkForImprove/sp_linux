#include "SPUtil.h"
#include <string.h>
//测试钞箱是否为出钞箱并且是否可出钞
BOOL CUCanDispense(ICashUnit *pCU)
{
    if (!CUIsOutBox(pCU->GetType()))
        return FALSE;
    if (!CUCanDispense(pCU->GetStatus()))
        return FALSE;
    if (pCU->GetAppLock())
        return FALSE;
    if (pCU->GetValues() == 0)
        return FALSE;
    if (pCU->GetCount() == 0)
        return FALSE;
    return TRUE;
}

BOOL CUCanAccept(ICashUnit *pCU, LPWFSCIMNOTETYPELIST pNoteTypeList)
{
    if (!CUIsAcceptBox(pCU->GetType()))
        return FALSE;
    if (!CUCanAccept(pCU->GetStatus()))
        return FALSE;
    if (pCU->GetAppLock())
    {
        if (pCU->GetType() == ADP_CASSETTE_RECYCLING ||
            pCU->GetType() == ADP_CASSETTE_CASHIN)
            return FALSE;
    }

    USHORT aryusNoteIDs[MAX_NOTEID_SIZE];
    DWORD dwNoteIDCount = pCU->GetNoteIDs(aryusNoteIDs);
    if (pCU->GetType() == ADP_CASSETTE_RECYCLING)
    {
        if (pCU->GetValues() == 0)
            return FALSE;
        if (dwNoteIDCount < 1)
            return FALSE;
        char cCurrencyID[3];
        pCU->GetCurrencyID(cCurrencyID);
        if (!NoteIDsAreSameCurrencyValue(aryusNoteIDs, (USHORT)dwNoteIDCount, pNoteTypeList,
                                         cCurrencyID, pCU->GetValues()))
            return FALSE;
    }
    else
    {
        if (pCU->GetItemType() == WFS_CIM_CITYPINDIVIDUAL)
        {
            if (dwNoteIDCount == 0)
                return FALSE;
        }
    }

    return TRUE;
}

const LPWFSCIMNOTETYPE FindNoteTypeByNoteID(USHORT usNoteID, const LPWFSCIMNOTETYPELIST pNoteTypeList)
{
    for (int i = 0; i < pNoteTypeList->usNumOfNoteTypes; i++)
    {
        const LPWFSCIMNOTETYPE pNoteType = pNoteTypeList->lppNoteTypes[i];
        if (pNoteType == NULL)
            continue;
        if (pNoteType->usNoteID == usNoteID)
            return pNoteType;
    }
    return NULL;
}

//测试NoteID是否为相同的币种面值
BOOL NoteIDsAreSameCurrencyValue(const LPUSHORT lpusNoteIDs, USHORT usNoteIDCount,
                                 const LPWFSCIMNOTETYPELIST pNoteTypeList,
                                 const char cCurrencyID[3], ULONG ulValue)
{
    for (USHORT i = 0; i < usNoteIDCount; i++)
    {
        const LPWFSCIMNOTETYPE pNoteType = FindNoteTypeByNoteID(lpusNoteIDs[i], pNoteTypeList);
        if (pNoteType->ulValues != ulValue)
            return FALSE;
        if (memcmp(pNoteType->cCurrencyID, cCurrencyID, 3) != 0)
            return FALSE;
    }
    return TRUE;
}
