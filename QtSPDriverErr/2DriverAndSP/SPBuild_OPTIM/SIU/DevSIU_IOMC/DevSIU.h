#ifndef DEVSIU_H
#define DEVSIU_H
#include "QtTypeInclude.h"

typedef long (* FNICReaderOpenUsbByFD)(unsigned int uiFD);          //30-00-00-00(FS#0012)
typedef int  (* FNICReaderClose)(long icdev);                       //30-00-00-00(FS#0012)
typedef int  (* FNICReaderLEDCtrl)(long icdev, unsigned char uLEDCtrl, unsigned char uDelay);     //30-00-00-00(FS#0012)

#define SKM_LAMP_MAX                        8

#pragma pack(push, 1)
//SKM接口灯闪烁参数结构体
typedef struct _SKMLAMPFLASH{
    BYTE        byPortNum;                      // SKM端口号(1~8)
    int         iFlashInterval;                 // 闪烁间隔时间    0:不闪烁
    ULONG       ulLastOnTime;                   // 闪烁最后一次亮灯时间　0:未亮过灯
}SKMLAMPFLASH, *LPSKMLAMPFLASH;
//////////////////////////////////////////////////////////////////////////
#pragma pack(pop)

#endif // DEVSIU_H
