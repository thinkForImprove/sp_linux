#include "DevPPR.h"
#include "MB2/DevPPR_MB2.h"

extern "C" Q_DECL_EXPORT long CreateIDevPTR(LPCSTR lpDevType, IDevPPR *&pDev)
{
    pDev = nullptr;
    // 缺省为新北洋凭条打印机
    if(lpDevType == nullptr || strlen(lpDevType) < 1)
    {
       pDev = new CDevPPR_MB2("MB2");
    } else
    {
        if (memcmp(lpDevType, DEV_PYCX_MB2_PPR, strlen(DEV_PYCX_MB2_PPR)) == 0)
        {
            pDev = new CDevPPR_MB2(lpDevType);
        }
    }
    return (pDev != nullptr) ? 0 : -1;
}
