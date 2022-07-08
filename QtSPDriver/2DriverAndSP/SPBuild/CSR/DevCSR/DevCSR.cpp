#include "DevCSR.h"
#include "RSCD400M/DevCSR_RSCD400M.h"

extern "C" Q_DECL_EXPORT long CreateIDevPTR(LPCSTR lpDevType, IDevPTR *&pDev)
{
    pDev = nullptr;


    if (MCMP_IS0(lpDevType, IDEVCSR_TYPE_RSCD400M))
    {
        pDev = new CDevCSR_RSCD400M();
    }

    return (pDev != nullptr) ? 0 : -1;
}
