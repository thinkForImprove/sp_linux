#include "DevPIN.h"
#include "ZT598/DevPIN_ZT598.h"
#include "XZF35/DevPIN_XZF35.h"
#include "ZTC90/DevPIN_ZTC90.h"             //30-00-00-00(FS#0013)
#include "CFESM01/DevPIN_CFESM01.h"

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
        } else if(!strcmp(lpDevType, "ZTC90")){               //30-00-00-00(FS#0013)
            pDev = new CDevPIN_ZTC90(lpDevType);              //30-00-00-00(FS#0013)
        } else if(!strcmp(lpDevType, "CFES")){                //30-00-00-00(FS#0013)
            pDev = new CDevPIN_CFESM01(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
