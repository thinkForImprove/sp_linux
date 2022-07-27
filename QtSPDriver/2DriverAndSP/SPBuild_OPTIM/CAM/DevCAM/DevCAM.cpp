#include "DevCAM.h"
#include "QtTypeDef.h"
#include "../XFS_CAM/def.h"
#include "DevCAM_DEF/DevCAM_DEF.h"
#include "DevCAM_CloudWalk/DevCAM_CloudWalk.h"
#include "DevCAM_JDY5001A0809/DevCAM_JDY5001A0809.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVCAM_EXPORT long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&pDev)
{
    pDev = nullptr;

    if (MCMP_IS0(lpDevType, IDEV_YC0C98))     // YC-0C98
    {
        pDev = new CDevCAM_CloudWalk(lpDevType);
    } else
    if (MCMP_IS0(lpDevType, IDEV_JDY5001A0809))     // JDY-5001A-0809
    {
        pDev = new CDevCAM_JDY5001A0809(lpDevType);
    }

    return (pDev != nullptr) ? 0 : -1;
}
