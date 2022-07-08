#include "DevCAM.h"
#include "QtTypeDef.h"
#include "DevCAM_CloudWalk/DevCAM_CloudWalk.h"
#include "DevCAM_TCF261/DevCAM_TCF261.h"
#include "DevCAM_ZLF1000A3/DevCAM_ZLF1000A3.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVCAM_EXPORT long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&pDev)
{
    pDev = nullptr;

    if (MCMP_IS0(lpDevType, IDEV_YC0C98))       // 云从双目摄像(YC0C98)
    {
        pDev = new CDevCAM_CloudWalk();
    } else
    if (MCMP_IS0(lpDevType, IDEV_TCF261))       // 天诚盛业双目摄像(TCF261)
    {
        pDev = new CDevCAM_TCF261();
    } else
    if (MCMP_IS0(lpDevType, IDEV_ZLF1000A3))    // 哲林高拍仪(ZLF1000A3)
    {
        pDev = new CDevCAM_ZLF1000A3(lpDevType);
    }

    return (pDev != nullptr) ? 0 : -1;
}
