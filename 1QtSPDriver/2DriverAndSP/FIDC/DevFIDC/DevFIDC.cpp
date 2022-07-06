#include "DevFIDC.h"
#include "CJ201/DevFIDC_CJ201.h"
#include "MT50/DevFIDC_MT50.h"
#include "TMZ/DevFIDC_TMZ.h"

//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev)
{
    //默认为明泰非接(MT50)
    if(lpDevType == nullptr || !strlen(lpDevType)){
        pDev = new CDevFIDC_MT50("MT50");
    } else {
        if(!strcmp(lpDevType, "CJ201")){
            pDev = new DevFIDC_CJ201(lpDevType);
        } else if(!strcmp(lpDevType, "MT50")) {
            pDev = new CDevFIDC_MT50(lpDevType);
        } else if(!strcmp(lpDevType, "TMZ")){
            pDev = new CDevFIDC_TMZ(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
