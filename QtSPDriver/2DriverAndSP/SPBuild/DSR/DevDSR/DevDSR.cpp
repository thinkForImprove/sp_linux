#include "DevDSR.h"
#include "../XFS_DSR/def.h"


/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
** 备注："DSR" device belong to "PTR CLASS"
************************************************************/
extern "C" DEVPTR_EXPORT long CreateIDevPTR(LPCSTR lpDevType, IDevPTR *&pDev)
{
    pDev = nullptr;

    // 缺省
    if(lpDevType == nullptr || strlen(lpDevType) < 1)
    {
       pDev = new CDevDSR_BSD216(lpDevType);
    } else
    {
        pDev = new CDevDSR_BSD216(lpDevType);
    }  

    return (pDev != nullptr) ? 0 : -1;
}
