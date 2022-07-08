#include "DevIDX.h"
#include "DevIDX_BSID81/DevIDX_BSID81.h"
#include "DevIDX_DMTM1M/DevIDX_DMTM1M.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVIDC_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev)
{
    pDev = nullptr;

    // BSID81
    if (MCMP_IS0(lpDevType, IDEV_TYPE_BSID81))
    {
        pDev = new CDevIDX_BSID81();
    } else
    if (MCMP_IS0(lpDevType, IDEV_TYPE_DMTM1M))
    {
        pDev = new CDevIDX_DMTM1M();
    }

    return (pDev != nullptr) ? 0 : -1;
}
