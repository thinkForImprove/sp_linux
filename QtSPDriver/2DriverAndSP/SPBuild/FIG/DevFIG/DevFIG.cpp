#include "DevFIG.h"

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVFIG_EXPORT long CreateIDevFIG(LPCSTR lpDevType, IDevFIG *&pDev)
{
    if (lpDevType == nullptr || !strlen(lpDevType)){
        pDev = new CDevFIG_HX("HX");
    } else {
        if(!strcmp(lpDevType, IDEV_TYPE_HX)){
            pDev = new CDevFIG_HX(lpDevType);
        } else if(!strcmp(lpDevType, IDEV_TYPE_TCM)) {
            pDev = new CDevFIG_TCM042(lpDevType);
        } else if (!strcmp(lpDevType, IDEV_TYPE_SM205)) {
            pDev = new CDevFIG_SM205BCT(lpDevType);
        } else if (!strcmp(lpDevType, IDEV_TYPE_WEL401)) {
            pDev = new CDevFIG_WL(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
