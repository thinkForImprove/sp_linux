/******************************************************************************
*                                                                             *
* xfscim.h      XFS - Cash Dispenser (CDM) definitions                        *
*                                                                             *
*               Version 3.20  (March 02 2011)                                 *
*                                                                             *
******************************************************************************/

#ifndef __INC_XFSCDM320__H
#define __INC_XFSCDM320__H

//#ifdef __cplusplus
//extern "C" {
//#endif

//#include <xfsapi.h>

#include "XFSCDM.H"
/* be aware of alignment */
#pragma pack (push, 1)

/* CDM Execute Commands */
#define     WFS_CMD_CDM_SET_GUIDANCE_LIGHT            (CDM_SERVICE_OFFSET + 24)

typedef struct _wfs_cdm_set_guidlight 
{
    WORD                  wGuidLight;
    DWORD                 dwCommand;
} WFSCDMSETGUIDLIGHT, *LPWFSCDMSETGUIDLIGHT;

/* restore alignment */
#pragma pack (pop)

//#ifdef __cplusplus
//}       /*extern "C"*/
//#endif

#endif  /* __INC_XFSCDM320__H */
