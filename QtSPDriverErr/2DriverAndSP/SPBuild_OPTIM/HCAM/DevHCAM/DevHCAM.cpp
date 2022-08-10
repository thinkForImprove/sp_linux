#include "DevHCAM.h"
#include "QtTypeDef.h"
#include "DevHCAM_JDY5001A0809/DevHCAM_JDY5001A0809.h"
#include "../XFS_HCAM/def.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVCAM_EXPORT long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&pDev)
{
    pDev = nullptr;

    if (MCMP_IS0(lpDevType, IDEV_JDY5001A0809))     // JDY-5001A-0809
    {
        pDev = new CDevHCAM_JDY5001A0809(lpDevType);
    }

    return (pDev != nullptr) ? 0 : -1;
}
