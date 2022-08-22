#ifndef DEVSIU_H
#define DEVSIU_H
#include "QtTypeInclude.h"

typedef long (* FNICReaderOpenUsbByFD)(unsigned int uiFD);          //30-00-00-00(FS#0012)
typedef int  (* FNICReaderClose)(long icdev);                       //30-00-00-00(FS#0012)
typedef int  (* FNICReaderLEDCtrl)(long icdev, unsigned char uLEDCtrl, unsigned char uDelay);     //30-00-00-00(FS#0012)

#define SKM_LAMP_MAX                        8
#define HEXTOBCD(x)   (((x/10) << 4) + x%10)

#pragma pack(push, 1)
//SKM接口灯闪烁参数结构体
typedef struct _SKMLAMPFLASH{
    BYTE        byPortNum;                      // SKM端口号(1~8)
    BOOL        bLedOn;                         // 灯亮标志
    int         iOnTime;                        // 灯亮时间ms
    int         iOffTime;                       // 灯灭时间ms
    ULONG       ulLastOnOffTime;                // 最后一次亮灭起始时间　0:未记录时间
}SKMLAMPFLASH, *LPSKMLAMPFLASH;
//////////////////////////////////////////////////////////////////////////
#pragma pack(pop)

#endif // DEVSIU_H
