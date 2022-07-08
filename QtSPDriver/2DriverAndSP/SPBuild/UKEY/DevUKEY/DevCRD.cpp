#include "DevCRD.h"
#include "ACT-U6SS39/DevCRD_ACTU6SS39.h"
#include "ACT-U6SG5/DevCRD_ACTU6SG5.h"
#include "../XFS_UKEY/def.h"

extern "C" Q_DECL_EXPORT long CreateIDevCRD(LPCSTR lpDevType, IDevCRD *&pDev)
{
    pDev = nullptr;

    if (MCMP_IS0(lpDevType, IDEVUKEY_TYPE_ACTU6SS39))
    {
        pDev = new CDevCRD_ACTU6SS39();
    } else
    if (MCMP_IS0(lpDevType, IDEVUKEY_TYPE_ACTU6SG5))
    {
        pDev = new CDevCRD_ACTU6SG5();
    }

    return (pDev != nullptr) ? 0 : -1;
}


