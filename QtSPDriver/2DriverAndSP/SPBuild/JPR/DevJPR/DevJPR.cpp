#include "DevJPR.h"
#include "../XFS_JPR/def.h"
#include "BT_T080AII/DevJPR_BTT080AII.h"


/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
long DEVPTR_EXPORT CreateIDevPTR(LPCSTR lpDevType, IDevPTR *&pDev)
{
    pDev = nullptr;

    // 缺省为新北洋凭条打印机
    if(lpDevType == nullptr || strlen(lpDevType) < 1)
    {
       pDev = nullptr;
    } else
    {
        if (MCMP_IS0(lpDevType, IDEV_BTT080AII))   // 新北洋BT-T080AII打印机
        {
            pDev = new CDevJPR_BTT080AII(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
