#include "DevRPR.h"
//#include "HOTS/devptr_rpr.h"
#include "SNBC-BKC310/DevRPR_SNBC.h"

//DevPTR_RPR *devPTR = nullptr;


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
       pDev = new CDevPTR_SNBC("SNBC");
    } else
    {
        if (MCMP_IS0(lpDevType, DEV_SNBC_BKC310_STR) || // 新北洋BK-C310打印机
            MCMP_IS0(lpDevType, DEV_SNBC_BTNH80_STR))   // 新北洋BT-NH80打印机
        {
            pDev = new CDevPTR_SNBC(lpDevType);
        } /*else
        if (MCMP_IS0(lpDevType, DEV_HOTS_STR))   // HOTS打印机
        {
            devPTR = new DevPTR_RPR();
            if(devPTR)
            {
                pDev = &(devPTR->iPtrDev);
            }
        }*/
    }

    return (pDev != nullptr) ? 0 : -1;
}
