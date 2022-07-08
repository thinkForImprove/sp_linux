#include "DevCAM.h"
#include "DevCAM_CloudWalk/DevCAM_CloudWalk.h"
#include "DevCAM_TCF261/DevCAM_TCF261.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVCAM_EXPORT long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&pDev)
{
    pDev = nullptr;

    if (memcmp(lpDevType, IDEV_TYPE_CW1, strlen(IDEV_TYPE_CW1)) == 0 &&
        memcmp(lpDevType, IDEV_TYPE_CW1, strlen(lpDevType)) == 0) // 云从双目摄像
    {
        pDev = new CDevCAM_CloudWalk();
    } else
    if (memcmp(lpDevType, IDEV_TYPE_TCF1, strlen(IDEV_TYPE_TCF1)) == 0 &&
        memcmp(lpDevType, IDEV_TYPE_TCF1, strlen(lpDevType)) == 0) // 天诚盛业双目摄像
    {
        pDev = new CDevCAM_TCF261();
    }

    return (pDev != nullptr) ? 0 : -1;
}
