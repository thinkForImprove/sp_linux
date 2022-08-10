#include "DevMSR.h"
#include "DevMSR_WBCS10/DevMSR_WBCS10.h"
#include "DevMSR_WBT2000/DevMSR_WBT2000.h"
#include "../XFS_MSR/def.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVIDC_EXPORT long CreateIDevMSR(LPCSTR lpDevType, IDevIDC *&pDev)
{
    pDev = nullptr;

    if (MCMP_IS0(lpDevType, IDEV_TYPE_WBT2172))
    {
        pDev = new CDevMSR_WBT2000();
    } else
    if (MCMP_IS0(lpDevType, IDEV_TYPE_WBCS10))
    {
        pDev = new CDevMSR_WBCS10();
    }

    return (pDev != nullptr) ? 0 : -1;
}
