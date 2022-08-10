#include "DevFIDC.h"
#include "../XFS_FIDC/def.h"
//#include "CJ201/DevFIDC_CJ201.h"
#include "MT50/DevFIDC_MT50.h"
#include "TMZ/DevFIDC_TMZ.h"
#include "CRT603CZ7/DevFIDC_CRT603CZ7.h"

//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev)
{
    pDev = nullptr;

    if (MCMP_IS0(lpDevType, IDEV_TMZ))
    {
        pDev = new CDevFIDC_TMZ(lpDevType);
    } else
    if (MCMP_IS0(lpDevType, IDEV_CRT603CZ7))
    {
        pDev = new CDevFIDC_CRT603CZ7(lpDevType);
    } else
    if (MCMP_IS0(lpDevType, IDEV_MT50))
    {
        pDev = new CDevFIDC_MT50(lpDevType);
    }

    //默认为明泰非接(MT50)
    /*if(lpDevType == nullptr || !strlen(lpDevType)){
        pDev = new CDevFIDC_MT50("MT50");
    } else {
        if(!strcmp(lpDevType, "CJ201")){
            pDev = new DevFIDC_CJ201(lpDevType);
        } else if(!strcmp(lpDevType, "MT50")) {
            pDev = new CDevFIDC_MT50(lpDevType);
        } else if(!strcmp(lpDevType, "TMZ")){
            pDev = new CDevFIDC_TMZ(lpDevType);
        }
    }*/

    return (pDev != nullptr) ? 0 : -1;
}
