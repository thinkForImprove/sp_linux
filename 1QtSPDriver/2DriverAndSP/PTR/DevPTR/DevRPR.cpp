#include "DevRPR.h"
#include "HOTS/devptr_rpr.h"
#include "SNBC-BKC310/DevRPR_SNBC.h"

DevPTR_RPR *devPTR = nullptr;


/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
long DEVPTR_EXPORT CreateIDevPTR(LPCSTR lpDevType, IDevPTR *&pDev)
{
    pDev = nullptr;

    //默认为新北洋凭条打印机
    if(lpDevType == nullptr || !strlen(lpDevType)){
       pDev = new CDevPTR_SNBC("SNBC");
    } else {
        if(!strcmp(lpDevType, "HOTS")){
            devPTR = new DevPTR_RPR();
            if(devPTR){
                pDev = &(devPTR->iPtrDev);
            }
        } else /*if(!strcmp(lpDevType, "SNBC")){*/
        if (!strcmp(lpDevType, BKC310_DEV_MODEL_NAME) ||    // 30-00-00-00(FT#0052)
            !strcmp(lpDevType, BTNH80_DEV_MODEL_NAME))      // 30-00-00-00(FT#0052)
        {
            pDev = new CDevPTR_SNBC(lpDevType);
        }
    }

    return (pDev != nullptr) ? 0 : -1;
}
