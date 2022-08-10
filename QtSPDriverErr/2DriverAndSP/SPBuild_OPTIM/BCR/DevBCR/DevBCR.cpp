#include "DevBCR.h"
#include "DevBCR_NT0861/DevBCR_NT0861.h"
#include "../XFS_BCR/def.h"

extern "C" Q_DECL_EXPORT long CreateIDevBCR(LPCSTR lpDevType, IDevBCR *&pDev)
{
    pDev = nullptr;

    if (MCMP_IS0(lpDevType, IDEV_NT0861_STR))
    {
        pDev = new CDevBCR_NT0861(lpDevType);
    }

    return (pDev != nullptr) ? 0 : -1;
}
