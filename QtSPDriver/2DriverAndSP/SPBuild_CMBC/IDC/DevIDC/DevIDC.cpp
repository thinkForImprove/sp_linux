#include "DevIDC.h"
#include "EC2G/DevIDC_EC2G.h"
#include "CRT-350N/DevIDC_CRT.h"

extern "C" Q_DECL_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev)
{
    //默认为创自读卡器(ＣＲＴ)
    if(lpDevType == nullptr || !strlen(lpDevType)){
        pDev = new CDevIDC_CRT("CRT");
    } else {
        if(!strcmp(lpDevType, "EC2G")){
            pDev = new CDevIDC_EC2G(lpDevType);
        } else if(!strcmp(lpDevType, "CRT")) {
            pDev = new CDevIDC_CRT(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
