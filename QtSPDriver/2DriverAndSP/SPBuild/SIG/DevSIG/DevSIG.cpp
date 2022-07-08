#include "DevSIG.h"


/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVSIG_EXPORT long CreateIDevSIG(LPCSTR lpDevType, IDevSIG *&pDev)
{
    if (lpDevType == nullptr || !strlen(lpDevType)){
        pDev = new CDevSIG_TSD64("TSD64");
    } else
    {
        if(!strcmp(lpDevType, "TPK")){
            pDev = new CDevSIG_TPK193(lpDevType);
        } else if(!strcmp(lpDevType, "TSD")) {
            pDev = new CDevSIG_TSD64(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
