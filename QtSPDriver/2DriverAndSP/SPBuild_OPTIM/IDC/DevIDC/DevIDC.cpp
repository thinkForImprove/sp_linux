#include "DevIDC.h"
//#include "EC2G/DevIDC_EC2G.h"
#include "DevIDC_CRT350N/DevIDC_CRT350N.h"
#include "../XFS_IDC/def.h"

extern "C" Q_DECL_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev)
{
    //默认为创自读卡器(ＣＲＴ)
    /*if(lpDevType == nullptr || !strlen(lpDevType)){
        pDev = new CDevIDC_CRT("CRT");
    } else {
        if(!strcmp(lpDevType, "EC2G")){
            pDev = new CDevIDC_EC2G(lpDevType);
        } else if(!strcmp(lpDevType, "CRT")) {
            pDev = new CDevIDC_CRT(lpDevType);
        }
    }*/

    pDev = nullptr;

    if (MCMP_IS0(lpDevType, IDEV_CRT350N_STR))
    {
        pDev = new CDevIDC_CRT350N(lpDevType);
    }

    return (pDev != nullptr) ? 0 : -1;
}
