//SPUtil.h
#ifndef SP_UTIL_H
#define SP_UTIL_H

#include "IBRMAdapter.h"
#include "ICashUnitManager.h"

//测试出钞箱的状态是否可以出钞
inline BOOL CUCanDispense(CASHUNIT_STATUS eStatus)
{
    if (eStatus == ADP_CASHUNIT_OK ||
        eStatus == ADP_CASHUNIT_LOW ||
        eStatus == ADP_CASHUNIT_HIGH ||
        eStatus == ADP_CASHUNIT_FULL)
        return TRUE;
    return FALSE;
}

//测试进钞箱、回收箱和拒钞箱的状态是否可以接受钞币
inline BOOL CUCanAccept(CASHUNIT_STATUS eStatus)
{
    if (eStatus == ADP_CASHUNIT_OK ||
        eStatus == ADP_CASHUNIT_LOW ||
        eStatus == ADP_CASHUNIT_HIGH ||
        eStatus == ADP_CASHUNIT_EMPTY)
        return TRUE;
    return FALSE;
}

//判断钞箱类型是否可以出钞
//BILL钞箱或循环钞返回TRUE
inline BOOL CUIsOutBox(ADP_CASSETTE_TYPE eType)
{
    return eType == ADP_CASSETTE_BILL || eType == ADP_CASSETTE_RECYCLING;
}

//判断钞箱类型是否可以接受钞票
//REJ、RET、IN、RECYC返回TRUE
inline BOOL CUIsAcceptBox(ADP_CASSETTE_TYPE eType)
{
    return eType == ADP_CASSETTE_RETRACT || eType == ADP_CASSETTE_RECYCLING ||
           eType == ADP_CASSETTE_REJECT || eType == ADP_CASSETTE_CASHIN;
}

//测试钞箱是否为出钞箱并且是否可出钞
BOOL CUCanDispense(ICashUnit *pCU);

//测试钞箱是否为接收钞箱（REJ、RET、IN、RECYC）并且是否可进钞
BOOL CUCanAccept(ICashUnit *pCU, LPWFSCIMNOTETYPELIST pNoteTypeList);

//测试NoteID是否为相同的币种面值
BOOL NoteIDsAreSameCurrencyValue(const LPUSHORT lpusNoteIDs, USHORT usNoteIDCount,
                                 const LPWFSCIMNOTETYPELIST pNoteTypeList,
                                 const char cCurrencyID[3], ULONG ulValue);




#endif //SP_UTIL_H
