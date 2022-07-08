#include "DevDPR.h"
#include "P3018D/DevDPR_P3018D.h"

extern "C" Q_DECL_EXPORT long CreateIDevPTR(LPCSTR lpDevType, IDevPTR *&pDev)
{
    pDev = nullptr;


    if (MCMP_IS0(lpDevType, IDEVCSR_TYPE_P3018D))
    {
        pDev = new CDevDPR_P3018D();
    }

    return (pDev != nullptr) ? 0 : -1;
}
