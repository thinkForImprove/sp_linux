#include "DevPPR.h"
#include "../XFS_PPR/def.h"
#include "MB2/DevPPR_MB2.h"
#include "PRM/DevPPR_PRM.h"


/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVPTR_EXPORT long CreateIDevPTR(LPCSTR lpDevType, IDevPTR *&pDev)
{
    pDev = nullptr;

    // 缺省
    if(lpDevType == nullptr || strlen(lpDevType) < 1)
    {
       pDev = nullptr;
    } else
    {
        if (MCMP_IS0(lpDevType, IDEV_MB2))
        {
            pDev = new CDevPPR_MB2(lpDevType);
        } else
        if (MCMP_IS0(lpDevType, IDEV_PRM))
        {
            pDev = new CDevPPR_PRM(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
