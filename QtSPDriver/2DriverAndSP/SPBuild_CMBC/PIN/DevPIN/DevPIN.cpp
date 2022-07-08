#include "DevPIN.h"
#include "ZT598/DevPIN_ZT598.h"
#include "XZF35/DevPIN_XZF35.h"

//////////////////////////////////////////////////////////////////////////
extern "C" DEVPIN_EXPORT long CreateIDevPIN(LPCSTR lpDevType, IDevPIN *&pDev)
{
    if(lpDevType == nullptr || !strlen(lpDevType)){
        pDev = new CDevPIN_XZF35("XZ");
    } else {
        if(!strcmp(lpDevType, "ZT")){
            pDev = new CDevPIN_ZT598(lpDevType);
        } else if(!strcmp(lpDevType, "XZ")){
            pDev = new CDevPIN_XZF35(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
