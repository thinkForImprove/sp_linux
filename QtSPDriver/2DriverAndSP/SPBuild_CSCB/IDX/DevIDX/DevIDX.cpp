#include "DevIDX.h"
#include "DevIDX_BSID81/DevIDX_BSID81.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVIDX_EXPORT long CreateIDevIDX(LPCSTR lpDevType, IDevIDX *&pDev)
{
    pDev = nullptr;

    // BSID81
    if (memcmp(lpDevType, IDEV_TYPE_BSID81, strlen(IDEV_TYPE_BSID81)) == 0 &&
        memcmp(lpDevType, IDEV_TYPE_BSID81, strlen(lpDevType)) == 0)
    {
        pDev = new CDevIDX_BSID81();
    }

    return (pDev != nullptr) ? 0 : -1;
}
