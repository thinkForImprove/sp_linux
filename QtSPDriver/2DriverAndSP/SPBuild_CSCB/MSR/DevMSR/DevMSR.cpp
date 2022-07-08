#include "DevMSR.h"
#include "DevMSR_WBCS10/DevMSR_WBCS10.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVMSR_EXPORT long CreateIDevMSR(LPCSTR lpDevType, IDevMSR *&pDev)
{
    pDev = nullptr;

    // BSID81
    if (memcmp(lpDevType, IDEV_TYPE_WBT2172, strlen(IDEV_TYPE_WBT2172)) == 0 &&
        memcmp(lpDevType, IDEV_TYPE_WBT2172, strlen(lpDevType)) == 0)
    {
        //pDev = new CDevMSR_WBT2172();
        pDev = nullptr;
    } else
    if (memcmp(lpDevType, IDEV_TYPE_WBCS10, strlen(IDEV_TYPE_WBCS10)) == 0 &&
        memcmp(lpDevType, IDEV_TYPE_WBCS10, strlen(lpDevType)) == 0)
    {
        pDev = new CDevMSR_WBCS10();
    }

    return (pDev != nullptr) ? 0 : -1;
}
