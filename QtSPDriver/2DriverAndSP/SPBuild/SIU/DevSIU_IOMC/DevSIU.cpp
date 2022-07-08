#include "DevSIU.h"
#include "CFESIomc/DevSIU_CFES.h"
#include "HTIomc/DevSIU_IOMC.h"

//////////////////////////////////////////////////////////////////////////
extern "C" DEVSIU_EXPORT long CreateIDevSIU(LPCSTR lpDevType, IDevSIU *&pDev)
{
    if(lpDevType == nullptr || !strlen(lpDevType)){
        pDev = new CDevSIU_IOMC("HT");
    } else {
        if(!strcmp(lpDevType, "CFES")){
            pDev = new CDevSIU_CFES(lpDevType);
        } else {
            pDev = new CDevSIU_IOMC(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
