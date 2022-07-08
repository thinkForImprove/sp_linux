#ifndef XFS_UKEY_DEC_H
#define XFS_UKEY_DEC_H

#include "XFS_UKEY.h"

//////////////////////////////////////////////////////////////////////////

//-------------------------宏定义------------------------------------
// INI指定银行,用于对应每个银行的特殊处理
#define BANK_SXXH         1     // 陕西信合


// 设备Check 定义: 非ONLINE/BUSY,硬件错误;BUSY,设备未准备好
#define CHECK_DEVICE() \
    if (m_stIDCStatus.fwDevice != WFS_CRD_DEVONLINE && m_stIDCStatus.fwDevice != WFS_CRD_DEVBUSY) \
    { \
        Log(ThisModule, __LINE__, "Device != ONLINE|BUSY, Return: %d.", WFS_ERR_HARDWARE_ERROR); \
        return WFS_ERR_HARDWARE_ERROR; \
    } \
    /*if (m_stStatus.fwDevice == WFS_PTR_DEVBUSY) \
    { \
        Log(ThisModule, __LINE__, "Device == BUSY, Return: %d.", WFS_ERR_DEV_NOT_READY); \
        return WFS_ERR_DEV_NOT_READY; \
    }*/

// 等待用户取媒介标志
enum WAIT_TAKEN_FLAG
{
    WTF_NONE    = 0,    // 不等待
    WTF_TAKEN   = 1,    // 等待
};

enum CONVERTYPE
{
    CONVERT_CRD  = 0,
    CONVERT_IDC = 1,
};

//----------------------INI配置变量 结构体 定义----------------------

// ACT_U6SS39设备特殊设置
typedef
struct st_ACT_U6SS39_cfg
{
    CHAR    szDevParam[5];              // 硬件参数(只用来保存获取的参数)
    st_ACT_U6SS39_cfg()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_ACT_U6SS39_cfg));
    }
} STCFG_ACTU6SS39, *LPSTCFG_ACTU6SS3;

// ini获取
typedef
struct st_ukey_ini_config
{
    CHAR                    szDevDllName[256];                  // DevXXX动态库
    CHAR                    szSDKPath[256];                     // 设备SDK库路径
    USHORT                  usOpenFailRet;                      // Open失败时返回值
    USHORT                  usOpenResetSup;                     // Open时Reset设置
    USHORT                  nDriverType;                        // 设备类型
    CHAR                    szDeviceConList[256];               // 模块连接串
    USHORT                  usBank;                             // 指定银行
    USHORT                  usResetMode;                        // 复位方式(0软复位/1硬复位,缺省0)
    UINT                    uiSndTimeOut;                       // 报文下发超时时间
    UINT                    uiRcvTimeOut;                       // 报文接收超时时间
    USHORT                  usPowerOffMode;                     // 断电后功能模式
    USHORT                  usEjectAgainRead;                   // 介质在出口时是否退回重读(0不支持/1支持,缺省0）
    STCFG_ACTU6SS39         stCfg_ACTU6SS39;                    // ACT-U6-SS39设备配置
    //STDEVINITPAR            stDevInitParam;                     // 设备初始化参数
    st_ukey_ini_config()
    {
        memset(this, 0x00, sizeof(st_ukey_ini_config));
        uiSndTimeOut = 0;                       // 报文下发超时时间
        uiRcvTimeOut = 0;                       // 报文接收超时时间
    }
}STINICONFIG, *LPSTINICONFIG;

//----------------------UKEY箱 结构体 定义----------------------
// UKEY箱列表结构体定义
#define DISPBOX_COUNT           16      // 箱数支持上限(包含存储箱和回收箱)
#define DISPBOX_STORAGE         0       // 存储箱
#define DISPBOX_RETRACT         1       // 回收箱

#define DISPBOX_RETNO           -1
#define DISPBOX_RETOK           0

// 箱状态
#define DISPBOX_NOHAVE          0       // 不存在
#define DISPBOX_ISHAVE          1       // 存在
#define DISPBOX_LOW             2       // 箱将空
#define DISPBOX_EMPTY           3       // 箱空
#define DISPBOX_HIGH            4       // 箱将满
#define DISPBOX_FULL            5       // 箱满
#define DISPBOX_ERR             6       // 故障

// 定义获取/设置类型
#define DISPRWT_SEQNO           0       // 索引号
#define DISPRWT_TYPE            1       // 箱类型
#define DISPRWT_INITCNT         2       // 初始数目
#define DISPRWT_COUNT           3       // 当前数目
#define DISPRWT_RETCNT          4       // 进入回收数目
#define DISPRWT_THREH           5       // 报警阀值(HIGH回收箱/LOWUKEY箱)
#define DISPRWT_HARDSR          6       // 是否基于硬件传感器生成阀值事件
#define DISPRWT_THINGTP         7       // 箱中介质类型
#define DISPRWT_STATUS          8       // 当前状态
#define DISPRWT_MAXCNT          9       // 可容纳最大数目

typedef
struct st_DispBox
{
    BOOL bIsHave;          // 是否使用中
    USHORT usNumber;       // 索引号为准(1~16)对应INI中[DISPBOX_X]X值
    CHAR   szThingName[128];// 卡单元中卡类型
    USHORT usBoxType;      // 箱类型(0UKEY箱/1回收箱)
    USHORT usInitCnt;      // 初始数目
    USHORT usMaxCnt;       // 可容纳最大数目
    USHORT usCount;        // 当前数目
    USHORT usRetainCnt;    // 进入回收数目
    USHORT usThreshold;    // 报警阀值(HIGH回收箱/LOWUKEY箱)
    USHORT usFullThreshold;// FULL报警阀值(回收箱使用)
    INT    usStatus;       // 当前状态
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
    INT GetDispBoxCount()  // 可用箱数
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
    INT GetDispBoxOrderIsHave(USHORT usOrder)   // 判断的箱顺序号是否可用(0-15)
    {
        if (usOrder > 15)
        {
            return DISPBOX_RETNO;
        }

        if (stDispBox[usOrder].bIsHave == TRUE)
        {
            return DISPBOX_RETOK;
        } else
        {
            return DISPBOX_RETNO;
        }

        return DISPBOX_RETNO;
    }
    INT GetDispBoxOrderIsNumber(USHORT usOrder)   // 指定箱顺序号返回箱索引号
    {
        return stDispBox[usOrder].usNumber;
    }    
    INT GetDispBoxNumberIsOrder(USHORT usNumber)   // 指定箱索引号返回箱顺序号
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].usNumber == usNumber)
            {
                return i;
            }
        }
        return DISPBOX_RETNO;
    }
    INT GetDispBoxIsHave(USHORT usNumber, USHORT usType = DISPBOX_STORAGE)   // 判断指定类型的箱索引号是否可用(1-16)
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].usNumber == usNumber)
            {
                if (stDispBox[i].bIsHave == TRUE)
                {
                    if (stDispBox[i].usBoxType == usType)
                    {
                        return DISPBOX_RETOK;
                    } else
                    {
                        return DISPBOX_RETNO;
                    }
                } else
                {
                    return DISPBOX_RETNO;
                }
            }
        }
        return DISPBOX_RETNO;
    }
    INT GetDispBoxData(USHORT usNumber, USHORT usType = DISPRWT_TYPE)  // 指定箱索引号(1~N)返回数据
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usNumber == usNumber)
            {
                switch(usType)
                {
                    case DISPRWT_SEQNO      : return i;                         // 顺序号
                    case DISPRWT_TYPE       : return stDispBox[i].usBoxType;    // 箱类型
                    case DISPRWT_INITCNT    : return stDispBox[i].usInitCnt;    // 初始数目
                    case DISPRWT_COUNT      : return stDispBox[i].usCount;      // 当前数目
                    case DISPRWT_RETCNT     : return stDispBox[i].usRetainCnt;  // 进入回收数目
                    case DISPRWT_THREH      : return stDispBox[i].usThreshold;  // 报警阀值(HIGH回收箱/LOWUKEY箱)
                    case DISPRWT_HARDSR     : return (stDispBox[i].bHardSensor == TRUE ? 1 : 0);  // 是否基于硬件传感器生成阀值事件
                    case DISPRWT_STATUS     : return stDispBox[i].usStatus;     // 当前状态
                    case DISPRWT_MAXCNT     : return stDispBox[i].usMaxCnt;     // 可容纳最大数目
                    return DISPBOX_RETOK;
                }
            }
        }

        return DISPBOX_RETNO;
    }
    CHAR* GetDispBoxCardType(USHORT usNumber)  // 指定箱索引号(1~N)返回箱内卡类型
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usNumber == usNumber)
            {
                return stDispBox[i].szThingName;
            }
        }

        return nullptr;
    }

    INT SetDispBoxData(USHORT usNumber, USHORT usSet, USHORT usType = DISPRWT_TYPE)  // 指定箱索引号(1~N)设置数据
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
                    case DISPRWT_THREH      :       // 报警阀值(HIGH回收箱/LOWUKEY箱)
                        stDispBox[i].usThreshold = usSet;
                        return 0;
                    case DISPRWT_HARDSR     :       // 是否基于硬件传感器生成阀值事件
                        stDispBox[i].bHardSensor = (usSet == 0 ? FALSE : TRUE);
                        return 0;
                    case DISPRWT_STATUS     :       // 当前状态
                        stDispBox[i].usStatus = usSet;
                        return DISPBOX_RETOK;
                }
            }
        }

        return DISPBOX_RETNO;
    }

    INT SetDispBoxData(USHORT usNumber, CHAR *lpSet)  // 指定箱索引号(1~N)设置卡/UKEY类型
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usNumber == usNumber)
            {
                memset(stDispBox[i].szThingName, 0x00, sizeof(stDispBox[i].szThingName));
                memcpy(stDispBox[i].szThingName, lpSet, strlen(lpSet));
                return DISPBOX_RETOK;
            }
        }

        return DISPBOX_RETNO;
    }    

    INT GetBoxCardCount(USHORT usNumber)       // 指定箱索引号(1~N)返回卡数目
    {
        for (INT i = 0; i < DISPBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usNumber == usNumber)
            {
                return stDispBox[i].usCount;
            }
        }

        return DISPBOX_RETNO;
    }

    int GetRetainNum(USHORT usOrder = 0)   // 返回指定第N个回收箱索引号
    {
        USHORT usCount = 1;
        for (INT i = 0; i < CARDBOX_COUNT; i ++)
        {
            if (stDispBox[i].bIsHave == TRUE && stDispBox[i].usBoxType == DISPBOX_RETRACT)
            {
                if (usOrder > 0)
                {
                    if (usCount == usOrder)
                    {
                        return stDispBox[i].usNumber;
                    } else
                    {
                        usCount ++;
                    }
                }
                return stDispBox[i].usNumber;
            }
        }
        return DISPBOX_RETNO;
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
        memset(dwGuidLights, WFS_CRD_GUIDANCE_NOT_AVAILABLE,
               sizeof(dwGuidLights) * WFS_CRD_GUIDANCE_NOT_AVAILABLE);// 指示灯状态:初始全不可用
        wDevicePosition = WFS_CRD_DEVICEINPOSITION; // 设备未知:初始正常
        usPowerSaveRecoveryTime = 0;            // 节能模式转正常描述:初始0(不支持)
        wAntiFraudModule = WFS_CRD_AFMNOTSUPP;  // 反欺诈模块状态:初始不支持
    }
public:
    bool Diff(CWfsCRDStatus clStat)
    {
        if (this->fwDevice != clStat.fwDevice ||
            this->fwDispenser != clStat.fwDispenser ||
            this->fwTransport != clStat.fwTransport ||
            this->fwMedia != clStat.fwMedia ||
            this->fwShutter != clStat.fwShutter ||
            this->wDevicePosition != clStat.wDevicePosition ||
            this->usPowerSaveRecoveryTime != clStat.usPowerSaveRecoveryTime ||
            this->wAntiFraudModule != clStat.wAntiFraudModule)
        {
            return true;
        }
        for(int i = 0; i < WFS_CRD_GUIDANCE_NOT_AVAILABLE; i ++)
        {
            if (this->dwGuidLights[i] != clStat.dwGuidLights[i])
            {
                return true;
            }
        }
        return false;
    }
    void Copy(CWfsCRDStatus clStat)
    {
        this->fwDevice = clStat.fwDevice;
        this->fwDispenser = clStat.fwDispenser;
        this->fwTransport = clStat.fwTransport;
        this->fwMedia = clStat.fwMedia;
        this->fwShutter = clStat.fwShutter;
        this->wDevicePosition = clStat.wDevicePosition;
        this->usPowerSaveRecoveryTime = clStat.usPowerSaveRecoveryTime;
        this->wAntiFraudModule = clStat.wAntiFraudModule;
        for(int i = 0; i < WFS_CRD_GUIDANCE_NOT_AVAILABLE; i ++)
        {
            this->dwGuidLights[i] = clStat.dwGuidLights[i];
        }
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
        bCardTakenSensor= true;                // 指定发卡器是否检测take
        fwDispenseTo    = WFS_CRD_DISPTO_TRANSPORT;// 指定发卡器中卡发出最终位置
        memset(dwGuidLights, WFS_CRD_GUIDANCE_NOT_AVAILABLE, WFS_CRD_GUIDLIGHTS_SIZE);
        bPowerSaveControl= false;               // 节能控制是否可用
        bPowerSaveControl= false;               // 反欺诈模块是否可用
        lpszExtra       = nullptr;
    }
};

//-------------------------继承命令类 定义-------------------------
// 避免 命令主处理类过多冗余
class CXFS_CMDFUNC: public ICmdFunc
{
public:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT OnClose()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT OnStatus()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT OnWaitTaken()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT OnCancelAsyncRequest()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT OnUpdateDevPDL()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    // IDC类型接口
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT GetFormList(LPSTR &lpFormList)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    // 执行命令
    virtual HRESULT ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT WriteTrack(const LPWFSIDCWRITETRACK lpWriteData)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT EjectCard(DWORD dwTimeOut)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT ResetCount()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT SetKey(const LPWFSIDCSETKEY lpKeyData)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT WriteRawData(const LPWFSIDCCARDDATA *lppCardData)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT Reset(LPWORD lpResetIn)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
#ifdef CARD_REJECT_GD_MODE
    //读卡器新增扩展部分
    virtual HRESULT ReduceCount()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT SetCount(LPWORD lpwCount)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT IntakeCardBack()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    //退卡部分
    virtual HRESULT CMEjectCard(LPCSTR lpszCardNo)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CMSetCardData(LPCSTR lpszCardNo)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CMRetainCard()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CMStatus(BYTE lpucQuery[118], BYTE lpucStatus[118])
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CMReduceCount()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CMSetCount(LPWORD lpwCount)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CMEmptyCard(LPCSTR lpszCardBox)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024])
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CMReset()
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
#endif

    // 增加 CRD系列命令(Card Dispenser) START
    /* CRD Info Commands */
    virtual HRESULT CRD_GetStatus(LPWFSCRDSTATUS &lpStatus)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CRD_GetCapabilities(LPWFSCRDCAPS &lpCaps)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CRD_GetCardUnitInfo(LPWFSCRDCUINFO &lpCardUnit)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    /* CRD Execute Commands */
    virtual HRESULT CRD_DispenseCard(const LPWFSCRDDISPENSE lpDispense)                 // 发卡
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CRD_EjecdCard()                                                     // 退卡
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CRD_RetainCard(const LPWFSCRDRETAINCARD lpRetainCard)               // 回收卡
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CRD_Reset(const LPWFSCRDRESET lpResset)                             // 复位
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CRD_SetGuidanceLight(const LPWFSCRDSETGUIDLIGHT lpSetGuidLight)     // 设置灯状态
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CRD_SetCardUnitInfo(const LPWFSCRDCUINFO lpCuInfo)                  // 设置卡箱信息
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT CRD_PowerSaveControl(const LPWFSCRDPOWERSAVECONTROL lpPowerCtrl)    // 激活/停用节能功能
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    // 增加 CRD系列命令(Card Dispenser) END
};


#endif /* XFS_UKEY_DEC_H */
