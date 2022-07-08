#include "DevCRM.h"
#include "CRT-730B/DevCRM_CRT730B.h"
#include "../XFS_IDC/def.h"

extern "C" Q_DECL_EXPORT long CreateIDevCRM(LPCSTR lpDevType, IDevCRM *&pDev)
{
    pDev = nullptr;

    // 默认为创自退卡模块(CRT730B)
    if(lpDevType == nullptr || !strlen(lpDevType))
    {
        pDev = new CDevCRM_CRT730B();
    } else
    {
        if (memcmp(lpDevType, ICRM_TYPE_CRT730B, strlen(ICRM_TYPE_CRT730B)) == 0 &&
            memcmp(lpDevType, ICRM_TYPE_CRT730B, strlen(lpDevType)) == 0)
        {
            pDev = new CDevCRM_CRT730B();
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
