#include "DevCPR.h"
#include "BT8500M/DevCPR_BT8500M.h"

extern "C" Q_DECL_EXPORT long CreateIDevPTR(LPCSTR lpDevType, IDevPTR *&pDev)
{
    pDev = nullptr;


    if (MCMP_IS0(lpDevType, IDEVCPR_TYPE_BT8500M))
    {
        pDev = new CDevCPR_BT8500M();
    }

    return (pDev != nullptr) ? 0 : -1;
}
