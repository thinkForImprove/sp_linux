/******************************************************************************
*                                                                             *
* xfscim.h      XFS - Cash Acceptor (CIM) definitions                         *
*                                                                             *
*               Version 3.20  (March 02 2011)                                 *
*                                                                             *
******************************************************************************/

#ifndef __INC_XFSCIM320__H
#define __INC_XFSCIM320__H

//#ifdef __cplusplus
//extern "C" {
//#endif

//#include <xfsapi.h>

#include "XFSCIM.H"
/* be aware of alignment */
#pragma pack (push, 1)

/* CIM Execute Commands */
#define     WFS_CMD_CIM_SET_CASH_IN_LIMIT       (CIM_SERVICE_OFFSET + 22)

typedef struct _wfs_cim_amount_limit
{
    CHAR                     cCurrencyID[3];
    ULONG                    ulAmount;
} WFSCIMAMOUNTLIMIT, *LPWFSCIMAMOUNTLIMIT;

typedef struct _wfs_cim_cash_in_limit
{
    ULONG                    ulTotalItemsLimit;
    LPWFSCIMAMOUNTLIMIT      lpAmountLimit;
} WFSCIMCASHINLIMIT, *LPWFSCIMCASHINLIMIT;


/* restore alignment */
#pragma pack (pop)

//#ifdef __cplusplus
//}       /*extern "C"*/
//#endif

#endif  /* __INC_XFSCIM__H */
