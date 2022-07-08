#ifndef XFS_CRD_H
#define XFS_CRD_H

#include "XFS_IDC.h"

//////////////////////////////////////////////////////////////////////////
// 发卡模块(CRD)相关声明
//----------------------INI配置变量 结构体 定义----------------------
typedef
struct st_crd_ini_config
{
    CHAR        szCRDDeviceDllName[256];    // 退卡模块动态库名
    BOOL        bCRDDeviceSup;              // 是否支持启动退卡模块,缺省F
    WORD        wCRDDeviceType;             // 退卡模块设备类型,缺省0(CRT-730B)
    CHAR        szCRDDeviceConList[24];     // 退卡模块连接字符串

    st_crd_ini_config()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(st_crd_ini_config));
        bCRDDeviceSup = FALSE;
    }
}STCRDINICONFIG, LPSTCRDINICONFIG;

#define CRDDEVTYPE_CHG(a)  (a == CRD_DEV_CRT591H ? ICRD_TYPE_CRT591H : "")

//----------------------发卡箱 结构体 定义----------------------
// 发卡箱列表结构体定义
#define DISPBOX_COUNT   16          // 最多支持箱数目
#define DISPBOX_TYFK    0           // 发卡箱
#define DISPBOX_TYHS    1           // 回收箱
// 定义获取/设置类型
#define DISPRWT_NUMBER  0           // 索引号
#define DISPRWT_TYPE    1           // 箱类型
#define DISPRWT_INITCNT 2           // 初始数目
#define DISPRWT_COUNT   3           // 当前数目
#define DISPRWT_RETCNT  4           // 进入回收数目
#define DISPRWT_THREH   5           // 报警阀值(HIGH回收箱/LOW发卡箱)
#define DISPRWT_HARDSR  6           // 是否基于硬件传感器生成阀值事件
#define DISPRWT_CARDTP  7           // 箱中卡类型
#define DISPRWT_STATUS  8           // 当前状态
typedef struct st_DispBox
{
    BOOL bIsHave;          // 是否使用中
    USHORT usBoxNo;        // 箱顺序号(1~16)为准,对应INI中[DISPBOX_X]X值,同索引号
    USHORT usNumber;       // 索引号为准(等同于usBoxNo,暂时不用)
    CHAR   szCardName[128];// 卡单元中卡类型
    USHORT usBoxType;      // 箱类型(0发卡箱/1回收箱)
    USHORT usInitCnt;      // 初始数目
    USHORT usCount;        // 当前数目
    USHORT usRetainCnt;    // 进入回收数目
    USHORT usThreshold;    // 报警阀值(HIGH回收箱/LOW发卡箱)
    USHORT usFullThreshold;// FULL报警阀值(回收箱使用)
    USHORT usStatus;       // 当前状态
    BOOL   bHardSensor;    // 是否基于硬件传感器生成阀值事件    
} STDISPPBOX, *LPSTDISPBOX;
typedef struct st_DispBox_List
{
    USHORT usBoxCount;         // 箱数目(根据INI配置可用数目)
    STDISPPBOX stDispBox[DISPBOX_COUNT];
    INT nThresholdDist[DISPBOX_COUNT];// 值=usThreshole-usCount,用于THRESHOLD事件判断

    st_DispBox_List()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_DispBox_List));
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            nThresholdDist[i] = -1000;
        }
    }
    INT nGetDispBoxCount()  // 可用箱数
    {
        INT nCount = 0;
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE)
            {
                nCount ++;
            }
        }

        return nCount;
    }
    BOOL bGetDispBoxIsHave(USHORT usBoxNo)   // 判断箱号是否可用(1-16)
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].usBoxNo == usBoxNo)
            {
                if (stDispBox[i].bIsHave == TRUE)
                {
                    return TRUE;
                } else
                {
                    return FALSE;
                }
            }
        }
        return FALSE;
    }
    INT nGetDispBoxData(USHORT usBoxNo, USHORT usType = DISPRWT_TYPE)  // 指定箱顺序号(1~N)返回数据
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usBoxNo == usBoxNo)
            {
                switch(usType)
                {
                    case DISPRWT_NUMBER     : return stDispBox[i].usBoxNo;      // 索引号
                    case DISPRWT_TYPE       : return stDispBox[i].usBoxType;    // 箱类型
                    case DISPRWT_INITCNT    : return stDispBox[i].usInitCnt;    // 初始数目
                    case DISPRWT_COUNT      : return stDispBox[i].usCount;      // 当前数目
                    case DISPRWT_RETCNT     : return stDispBox[i].usRetainCnt;  // 进入回收数目
                    case DISPRWT_THREH      : return stDispBox[i].usThreshold;  // 报警阀值(HIGH回收箱/LOW发卡箱)
                    case DISPRWT_HARDSR     : return (stDispBox[i].bHardSensor == TRUE ? 1 : 0);  // 是否基于硬件传感器生成阀值事件
                    case DISPRWT_STATUS     : return stDispBox[i].usStatus;     // 当前状态
                    return 0;
                }
            }
        }

        return -1;
    }
    CHAR* nGetDispBoxCardType(USHORT usBoxNo)  // 指定箱顺序号(1~N)返回箱内卡类型
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usBoxNo == usBoxNo)
            {
                return stDispBox[i].szCardName;
            }
        }

        return nullptr;
    }

    INT nSetDispBoxData(USHORT usBoxNo, USHORT usSet, USHORT usType = DISPRWT_TYPE)  // 指定箱顺序号(1~N)设置数据
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usBoxNo == usBoxNo)
            {
                switch(usType)
                {
                    case DISPRWT_NUMBER     :       // 索引号
                        //stDispBox[i].usNumber = usSet;
                        return 0;
                    case DISPRWT_TYPE       :       // 箱类型
                        stDispBox[i].usBoxType = usSet;
                        return 0;
                    case DISPRWT_INITCNT    :       // 初始数目
                        stDispBox[i].usInitCnt = usSet;
                        return 0;
                    case DISPRWT_COUNT      :       // 当前数目
                        stDispBox[i].usCount = usSet;
                        return 0;
                    case DISPRWT_RETCNT     :       // 进入回收数目
                        stDispBox[i].usRetainCnt = usSet;
                        return 0;
                    case DISPRWT_THREH      :       // 报警阀值(HIGH回收箱/LOW发卡箱)
                        stDispBox[i].usThreshold = usSet;
                        return 0;
                    case DISPRWT_HARDSR     :       // 是否基于硬件传感器生成阀值事件
                        stDispBox[i].bHardSensor = (usSet == 0 ? FALSE : TRUE);
                        return 0;
                    case DISPRWT_STATUS     :       // 当前状态
                        stDispBox[i].usStatus = usSet;
                        return 0;
                }
            }
        }

        return -1;
    }

    INT nSetDispBoxDataNum(USHORT usNumber, USHORT usSet, USHORT usType = DISPRWT_TYPE)  // 指定箱顺序号(1~N)设置数据
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usNumber == usNumber)
            {
                switch(usType)
                {
                    case DISPRWT_TYPE       :       // 箱类型
                        stDispBox[i].usBoxType = usSet;
                        return 0;
                    case DISPRWT_INITCNT    :       // 初始数目
                        stDispBox[i].usInitCnt = usSet;
                        return 0;
                    case DISPRWT_COUNT      :       // 当前数目
                        stDispBox[i].usCount = usSet;
                        return 0;
                    case DISPRWT_RETCNT     :       // 进入回收数目
                        stDispBox[i].usRetainCnt = usSet;
                        return 0;
                    case DISPRWT_THREH      :       // 报警阀值(HIGH回收箱/LOW发卡箱)
                        stDispBox[i].usThreshold = usSet;
                        return 0;
                    case DISPRWT_HARDSR     :       // 是否基于硬件传感器生成阀值事件
                        stDispBox[i].bHardSensor = (usSet == 0 ? FALSE : TRUE);
                        return 0;
                }
            }
        }

        return -1;
    }

    INT nSetDispBoxData(USHORT usBoxNo, CHAR *lpSet)  // 指定箱顺序号(1~N)设置数据
    {
        for (INT i = 0; i < usBoxCount; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usBoxNo == usBoxNo)
            {
                memset(stDispBox[i].szCardName, 0x00, sizeof(stDispBox[i].szCardName));
                memcpy(stDispBox[i].szCardName, lpSet, strlen(lpSet));
                return 0;
            }
        }

        return -1;
    }    

    INT GetBoxCardCount(USHORT usBoxNo)       // 指定箱顺序号(1~N)返回卡数目
    {
        for (INT i = 0; i < usBoxCount; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usBoxNo == usBoxNo)
            {
                return stDispBox[i].usCount;
            }
        }

        return -1;
    }
    int GetRetainNum(USHORT usOrder = 0)   // 返回指定第N个回收箱索引号
    {
        USHORT usCount = 1;
        for (INT i = 0; i < CARDBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usBoxType == DISPBOX_TYHS)
            {
                if (usOrder > 0)
                {
                    if (usCount == usOrder)
                    {
                        return stDispBox[i].usBoxNo;
                    } else
                    {
                        usCount ++;
                    }
                }
                return stDispBox[i].usBoxNo;
            }
        }
        return -1;
    }

} STDISPBOXLIST, *LPSTDISPBOXLIST;

//--------------CRD(Card Dispenser)系列状态类 定义--------------
// CRD(Card Dispenser)系列状态类
class CWfsCRDStatus : public WFSCRDSTATUS
{
public:
    CWfsCRDStatus()
    {
        fwDevice    = WFS_CRD_DEVONLINE;        // 设备状态:初始ONLINE
        fwDispenser = WFS_CRD_DISPCUOK;         // 单元状态:初始全部OK
        fwTransport = WFS_CRD_TPOK;             // 传送模块:初始OK
        fwMedia     = WFS_CRD_MEDIANOTPRESENT;  // 介质状态:初始设备通道无卡
        fwShutter   = WFS_CRD_SHTNOTSUPPORTED;  // 门状态:初始不支持
        lpszExtra   = nullptr;
        memset(dwGuidLights, WFS_CRD_GUIDANCE_NOT_AVAILABLE, sizeof(dwGuidLights));// 指示灯状态:初始全不可用
        wDevicePosition = WFS_CRD_DEVICEINPOSITION; // 设备未知:初始正常
        usPowerSaveRecoveryTime = 0;            // 节能模式转正常描述:初始0(不支持)
        wAntiFraudModule = WFS_CRD_AFMNOTSUPP;  // 反欺诈模块状态:初始不支持
    }
};
// CRD(Card Dispenser)系列能力值类
class CWfsCRDCaps : public WFSCRDCAPS
{
public:
    CWfsCRDCaps()
    {
        wClass          = WFS_SERVICE_CLASS_CRD;
        bCompound       = true;                 // 发卡器是否复合读卡器的一部分
        fwPowerOnOption = WFS_CRD_NOACTION;     // 指定发卡器上电时对卡的操作
        fwPowerOffOption= WFS_CRD_NOACTION;     // 指定发卡器断电时对卡的操作
        bCardTakenSensor= false;                // 指定发卡器是否检测take
        fwDispenseTo    = WFS_CRD_DISPTO_TRANSPORT;// 指定发卡器中卡发出最终位置
        memset(dwGuidLights, WFS_CRD_GUIDANCE_NOT_AVAILABLE, WFS_CRD_GUIDLIGHTS_SIZE);
        bPowerSaveControl= false;               // 节能控制是否可用
        bPowerSaveControl= false;               // 反欺诈模块是否可用
        lpszExtra       = nullptr;
    }
};

#endif /* XFS_CRD_H */
