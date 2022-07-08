#include "DevCRD.h"
#include "CRT-591H/DevCRD_CRT591H.h"

extern "C" Q_DECL_EXPORT long CreateIDevCRD(LPCSTR lpDevType, IDevCRD *&pDev)
{
    pDev = nullptr;

    // 默认为创自发卡模块(CRT591H)
    if(lpDevType == nullptr || !strlen(lpDevType))
    {
        pDev = new CDevCRD_CRT591H();
    } else
    {
        if (memcmp(lpDevType, ICRD_TYPE_CRT591H, strlen(ICRD_TYPE_CRT591H)) == 0 &&
            memcmp(lpDevType, ICRD_TYPE_CRT591H, strlen(lpDevType)) == 0)
        {
            pDev = new CDevCRD_CRT591H();
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}


