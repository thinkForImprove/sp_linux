#include "DevCRM.h"
#include "QtTypeDef.h"
#include "../XFS_IDC/def.h"
#include "CRT-730B/DevCRM_CRT730B.h"

extern "C" Q_DECL_EXPORT long CreateIDevCRM(LPCSTR lpDevType, IDevCRM *&pDev)
{
    pDev = nullptr;

    // 默认为创自退卡模块(CRT730B)
    if(lpDevType == nullptr || !strlen(lpDevType))
    {
        pDev = new CDevCRM_CRT730B();
    } else
    {
        if (MCMP_IS0(lpDevType, IDEV_CRM_CRT730B))
        {
            pDev = new CDevCRM_CRT730B();
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
