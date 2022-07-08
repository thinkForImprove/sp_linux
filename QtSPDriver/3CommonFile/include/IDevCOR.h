#pragma once
/***************************************************************
 * 文件名称：IDevCOR.h
 * 文件描述：声明硬币机底层对外提供的所有的控制指令接口及测试指令接口
 *
 * 版本历史信息
 * 变更说明：建立文件
 * 变更日期：2019年11月7日
 * 文件版本：1.0.0.1
 ****************************************************************/
#include <QtCore/qglobal.h>
#include <string.h>
#include <time.h>
#include "QtTypeDef.h"
#include "IDevBase.h"
//////////////////////////////////////////////////////////////////////////
#if defined(IDEVCOR_LIBRARY)
    #define DEVCOR_EXPORT Q_DECL_EXPORT
#else
    #define DEVCOR_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
#define MAX_DATA_LENGTH (256)  //用户数据存储区每块数据最大长度
//#define MAX_DEV_INFO_LENGTH            (128)      //设备描述最大长度
//#define MAX_FW_VER_LENGTH              (32)       //固件版本最大长度
//#define MAX_DATA_LENGTH                (256)      //用户数据存储区每块数据最大长度
//#define MAX_EXTRA_INFO_LEN             (64)       //错误信息最大长度
//#define MAX_COINCYLINDER_NUM           (6)        //硬币筒个数
//#define MAX_FIRMWAREDATA               (8 * 1024) //固件大小
//#define ERR_COR_SUCCESS                (0)        //成功
//#define ERR_COR_COMM_ERR               (-1)       //通讯错误
//#define ERR_COR_READTIMEOUT            (-2)       //读数据超时
//#define ERR_COR_WRITETIMEOUT           (-3)       //写数据超时
//#define ERR_COR_PARAM_ERR              (-4)       //参数错误
//#define ERR_COR_WRITE_ERR              (-5)       //写数据出错
//#define ERR_COR_READ_ERR               (-6)       //读数据出错
//#define ERR_COR_BUSY                   (-7)       //设备正忙时，不可执行进币或出币等操作
//#define ERR_COR_ACCEPTOR_JAM           (-8)
//#define ERR_COR_ACCEPTOR_ERR           (-9)
//#define ERR_COR_CASHBOX_FULL           (-10)
//#define ERR_COR_CASHBOX_MISSING        (-11)
//#define ERR_COR_CASHNOTENOUGH          (-12)
//#define ERR_COR_PAYOUT_FAIL            (-13)
//#define ERR_COR_NAK                    (-14)
//#define ERR_COR_FILE_MISSING           (-15)
//#define ERR_COR_FILE_EMPTY             (-16)
//#define ERR_COR_LOADFW_FAILED          (-17)
//#define ERR_COR_COUNT_ERR              (-18)
//#define ERR_COR_NOTSUPPORT             (-99)  //不支持此功能
//#define ERR_COR_OTHER                  (-100) //其它错误
#define ERR_COR_INVALIDCURRENCY        DEF_DEV_SOFT_ERROR(1)
#define ERR_COR_INVALIDTELLERID        DEF_DEV_SOFT_ERROR(2)
#define ERR_COR_TOOMANYITEMS           DEF_DEV_SOFT_ERROR(4)
#define ERR_COR_UNSUPPOSITION          DEF_DEV_SOFT_ERROR(5)
#define ERR_COR_SAFEDOOROPEN           DEF_DEV_SOFT_ERROR(6)
#define ERR_COR_SHUTTERNOTOPEN         DEF_DEV_SOFT_ERROR(7)
#define ERR_COR_SHUTTEROPEN            DEF_DEV_SOFT_ERROR(8)
#define ERR_COR_SHUTTERCLOSED          DEF_DEV_SOFT_ERROR(9)
#define ERR_COR_INVALIDCASHUNIT        DEF_DEV_SOFT_ERROR(10)
#define ERR_COR_NOITEMS                DEF_DEV_SOFT_ERROR(11)
#define ERR_COR_EXCHANGEACTIVE         DEF_DEV_SOFT_ERROR(12)
#define ERR_COR_NOEXCHANGEACTIVE       DEF_DEV_SOFT_ERROR(13)
#define ERR_COR_SHUTTERNOTCLOSED       DEF_DEV_SOFT_ERROR(14)
#define ERR_COR_ITEMSTAKEN             DEF_DEV_SOFT_ERROR(15)
#define ERR_COR_CASHINACTIVE           DEF_DEV_SOFT_ERROR(16)
#define ERR_COR_NOCASHINACTIVE         DEF_DEV_SOFT_ERROR(17)
#define ERR_COR_POSITION_NOT_EMPTY     DEF_DEV_SOFT_ERROR(18)
#define ERR_COR_INVALIDRETRACTPOSITION DEF_DEV_SOFT_ERROR(19)
#define ERR_COR_NOTRETRACTAREA         DEF_DEV_SOFT_ERROR(20)
#define ERR_COR_INVALIDDENOMINATION    DEF_DEV_SOFT_ERROR(21)
#define ERR_COR_INVALIDMIXNUMBER       DEF_DEV_SOFT_ERROR(22)
#define ERR_COR_NOCURRENCYMIX          DEF_DEV_SOFT_ERROR(23)
#define ERR_COR_NOTDISPENSABLE         DEF_DEV_SOFT_ERROR(24)
#define ERR_COR_PRERRORNOITEMS         DEF_DEV_SOFT_ERROR(25)
#define ERR_COR_PRERRORITEMS           DEF_DEV_SOFT_ERROR(26)
#define ERR_COR_PRERRORUNKNOWN         DEF_DEV_SOFT_ERROR(27)
#define ERR_COR_INVALIDMIXTABLE        DEF_DEV_SOFT_ERROR(28)
#define ERR_COR_OUTPUTPOS_NOT_EMPTY    DEF_DEV_SOFT_ERROR(29)
#define ERR_COR_NOCASHBOXPRESENT       DEF_DEV_SOFT_ERROR(31)
#define ERR_COR_AMOUNTNOTINMIXTABLE    DEF_DEV_SOFT_ERROR(32)
#define ERR_COR_ITEMSNOTTAKEN          DEF_DEV_SOFT_ERROR(33)
#define ERR_COR_ITEMSLEFT              DEF_DEV_SOFT_ERROR(34)
#define ERR_COR_COINNOTENOUGH          DEF_DEV_SOFT_ERROR(35)
#define ERR_COR_PAYOUT_FAIL            DEF_DEV_SOFT_ERROR(36)
#define ERR_COR_COIN_BOX_FULL          DEF_DEV_SOFT_ERROR(37)
//

#define ERR_COR_ACCEPT_ERROR      DEF_DEV_HW_ERR(1)
#define ERR_COR_DISPENSER_FAILED  DEF_DEV_HW_ERR(2)
#define ERR_COR_STACKER_JAMMED    DEF_DEV_HW_ERR(3)
#define ERR_COR_COIN_UNIT_JAMMED  DEF_DEV_HW_ERR(4)
#define ERR_COR_JAMMED            DEF_DEV_HW_ERR(5)
#define ERR_COR_SHUTTER_JAMMED    DEF_DEV_HW_ERR(6)
#define ERR_COR_ACCEPTOR_ILLGGAL  DEF_DEV_HW_ERR(7)
#define ERR_COR_DISPENSER_ILLGGAL DEF_DEV_HW_ERR(8)
#define ERR_COR_CASHUNITERROR     DEF_DEV_HW_ERR(9)
//币种
#define COIN_CN005A          0x0001
#define COIN_CN010B          0x0002
#define COIN_CN010C          0x0004
#define COIN_CN010D          0x0008
#define COIN_CN050B          0x0010
#define COIN_CN050C          0x0020
#define COIN_CN050D          0x0040
#define COIN_CN100B          0x0080
#define COIN_CN100C          0x0100
//#define MAX_COINCYLINDER_NUM 6
#define MAX_COINCYLINDER_NUM 3
#define MAX_SUPP_COIN_TYPE_NUM 3
#define MAX_COIN_TYPE_FIELD_NUM 4
#define MAX_DEV_INFO_LENGTH  128
enum DEV_STATUS
{
    DEV_ONLINE = 0,     //正常
    DEV_OFFLINE = 1,    //设备物理状态改变
    DEV_POWER_OFF = 2,  //不支持
    DEV_NODEVICE = 3,   //不支持
    DEV_HWERROR = 4,    //硬件报错，卡钞，电源线数据线断开等
    DEV_USERERROR = 5,  //钞箱门被打开
    DEV_BUSY = 6
};
enum DEV_INSHUTTER_STATUS
{
    INSHUTTER_CLOSED = 0,
    INSHUTTER_OPEN = 1,
    INSHUTTER_UNKNOWN = 3,
    INSHUTTER_NOTSUPPORTED = 4,
};
//币种代码
enum COIN_CODE
{
    COIN_CODE_00 = 0,      // 未指定
    COIN_CODE_CN010C = 1,  // 1角（2000年 - ）
    COIN_CODE_CN050C = 2,  // 5角（2002年 - ）
    COIN_CODE_CN100B = 3,  // 1元（1991年 - ）
};
enum COIN_UNIT_TYPE
{
    COR_UT_TYPENA = 1,
    COR_UT_TYPEREJECTCASSETTE = 2,
    COR_UT_TYPEBILLCASSETTE = 3,
    COR_UT_TYPECOINCYLINDER = 4,
    COR_UT_TYPECOINDISPENSER = 5,
    COR_UT_TYPERETRACTCASSETTE = 6,
    COR_UT_TYPECOUPON = 7,
    COR_UT_TYPEDOCUMENT = 8,
    COR_UT_TYPEREPCONTAINER = 11,
    COR_UT_TYPERECYCLING = 12
};

enum COIN_UNIT_STATUS
{
    COR_UNIT_STATCUOK = 0,
    COR_UNIT_STATCUFULL = 1,
    COR_UNIT_STATCUHIGH = 2,
    COR_UNIT_STATCULOW = 3,
    COR_UNIT_STATCUEMPTY = 4,
    COR_UNIT_STATCUINOP = 5,
    COR_UNIT_STATCUMISSING = 6,
    COR_UNIT_STATCUNOVAL = 7,
    COR_UNIT_STATCUNOREF = 8,
    COR_UNIT_STATCUMANIP = 9
};

//硬币筒信息
typedef struct tag_COINCYLINDER_INFO
{
    int iCylinderNO;           // 筒号，取值1~6
    COIN_CODE iCoinCode;       // 币种
    int iCashValue;            // 单币值
    long lCount;               // 现有硬币存量
    COIN_UNIT_STATUS iStatus;  //参见XFSCDM.h/XFSCIM.h
    COIN_UNIT_TYPE iType;
    char szId[5];
    tag_COINCYLINDER_INFO()
    {
        iCylinderNO = 0;
        iCoinCode = COIN_CODE_00;
        iCashValue = 0;
        lCount = 0;
        iStatus = COR_UNIT_STATCUNOVAL;
        iType = COR_UT_TYPENA;
        *(int *)szId = 0;
        *(szId + 4) = 0x00;
    }
} ST_COINCYLINDER_INFO;
//计数类型
enum MONEY_COUNTERS_TYPE
{
    MONEY_COUNTERS_MONEYIN = 1,  //存入金额计数器
    MONEY_COUNTERS_MONEYOUT,     //支出金额计数器
};
//设备信息
typedef struct tag_DEV_INFO
{
    char pszManufacturer[MAX_DEV_INFO_LENGTH];  //厂商信息
    char pszProductName[MAX_DEV_INFO_LENGTH];   //产品名称
    char pszCreationDate[MAX_DEV_INFO_LENGTH];  //制造日期
    char pszRepairDate[MAX_DEV_INFO_LENGTH];    //最后一次维修日期
} ST_DEV_INFO;
enum DEV_CUERROR
{
    DEV_CUERROR_OK = 0,
    DEV_CUERROR_EMPTY = 1,
    DEV_CUERROR_FULL = 2,
    DEV_CUERROR_RETRYABLE = 3,  //可重试类错误
    DEV_CUERROR_FATAL = 4,      //致命错误，不可再用
    DEV_CUERROR_NOTUSED = 5,    //该钞箱未在本操作中使用
};
typedef struct tag_cdm_caps
{
    WORD wMaxDispenseItems;  // 单次最大点钞张数
    BOOL bShutter;           // 是否有shutter
    BOOL bShutterControl;    // TRUE:the shutter is controlled implicitly by the service provider
    // the shutter must be controlled explicitly by the application using the
    // WFS_CMD_CDM_OPEN_SHUTTER and the WFS_CMD_CDM_CLOSE_SHUTTER
    WORD fwRetractAreas;  // 钞币可能的回收位置
    WORD fwRetractTransportActions;
    WORD fwRetractStackerActions;
    BOOL bSafeDoor;             // 是否有安全门
    BOOL bIntermediateStacker;  // 是否有暂存区
    BOOL bItemsTakenSensor;     // 是否能感知门口钞票被取走
    WORD fwPositions;           // 可能的输出位组合, 只允许为LEFT、RIGHT等其中一个
    WORD fwMoveItems;           // 同XFS中CDM的相关定义
} ST_CDMCAPS;
typedef struct tag_cim_caps
{
    WORD wMaxCashInItems;  // 一次交易最大存入钞票张数
    BOOL bShutter;         // 是否有shutter
    BOOL bShutterControl;  // TRUE:the shutter is controlled implicitly by the service provider
    // the shutter must be controlled explicitly by the application using the
    // WFS_CMD_CDM_OPEN_SHUTTER and the WFS_CMD_CDM_CLOSE_SHUTTER
    BOOL bSafeDoor;              // Specifies whether or not the WFS_CMD_CDM_OPEN_SAFE_DOOR command is supported
    WORD fwIntermediateStacker;  // 暂存区能够叠放的钞票张数
    BOOL bItemsTakenSensor;
    BOOL bItemsInsertedSensor;  // 是否能感知门口钞票被放入
    WORD fwPositions;           // 可能的输入、输出位组合, 只允许为LEFT、RIGHT等其中一个, 如为LEFT，必须为INLEFT | OUTLEFT
    WORD fwRetractAreas;
    WORD fwRetractTransportActions;
    WORD fwRetractStackerActions;
} ST_CIMCAPS;
typedef struct tag_RetractBin_Count
{
    ULONG ulCN100;
    ULONG ulCN050;
    ULONG ulCN010;
} ST_RETRACTBIN_COUNT;
typedef struct tag_cor_caps : ST_CIMCAPS, ST_CDMCAPS
{
} ST_COR_CAPS, *PST_COR_CAPS;
enum COR_DEVSTATUS
{
    COR_DEVONLINE = 0,
    COR_DEVOFFLINE = 1,
    COR_DEVPOWEROFF = 2,
    COR_DEVNODEVICE = 3,
    COR_DEVHWERROR = 4,
    COR_DEVUSERERROR = 5,
    COR_DEVBUSY = 6,
};
enum COR_SAFEDOOR
{
    COR_SOFTDOORNOTSUPPORTED = 1,
    COR_SOFTDOOROPEN = 2,
    COR_SOFTDOORCLOSED = 3,
    COR_SOFTDOORUNKNOWN = 5,
};

enum COR_RETRACTAREA
{
    COR_RA_RETRACT = 0x0001,
    COR_RA_TRANSPORT = 0x0002,
    COR_RA_STACKER = 0x0004,
    COR_RA_BILLCASSETTES = 0x0008,
    COR_RA_NOTSUPP = 0x0010,
};

enum COR_DISPENER_STATUS
{
    COR_DISPENSER_DISPOK = 0,
    COR_DISPENSER_DISPCUSTATE = 1,
    COR_DISPENSER_DISPCUSTOP = 2,
    COR_DISPENSER_DISPCUUNKNOWN = 3,
};
enum COR_STACKER
{
    COR_STACKER_ISEMPTY = 0,
    COR_STACKER_ISNOTEMPTY = 1,
    COR_STACKER_ISNOTEMPTYCUST = 2,
    COR_STACKER_ISNOTEMPTYUNK = 3,
    COR_STACKER_ISUNKNOWN = 4,
    COR_STACKER_ISNOTSUPPORTED = 5,
};
enum COR_ACCEPTOR
{
    COR_ACCOK = 0,
    COR_ACCCUSTATE = 1,
    COR_ACCCUSTOP = 2,
    COR_ACCCUUNKNOWN = 3,
};
enum COR_STACKITEM
{
    COR_STACKITEM_CUSTOMERACCESS = 0,
    COR_STACKITEM_NOCUSTOMERACCESS = 1,
    COR_STACKITEM_ACCESSUNKNOWN = 2,
    COR_STACKITEM_NOITEMS = 4,
};
enum COR_BANKREAD
{
    COR_BNROK = 0,
    COR_BNRINOP = 1,
    COR_BNRUNKNOWN = 2,
    COR_BNRNOTSUPPORTED = 3,
};
enum COR_POSITION
{
    COR_POSNULL = 0x0000,
    COR_POSLEFT = 0x0001,
    COR_POSRIGHT = 0x0002,
    COR_POSCENTER = 0x0004,
    COR_POSTOP = 0x0008,
    COR_POSBOTTOM = 0x0010,
    COR_POSFRONT = 0x0020,
    COR_POSREAR = 0x0040,
    COR_POSOUTLEFT = 0x0080,
    COR_POSOUTRIGHT = 0x0100,
    COR_POSOUTCENTER = 0x0200,
    COR_POSOUTTOP = 0x0400,
    COR_POSOUTBOTTOM = 0x0800,
    COR_POSOUTFRONT = 0x1000,
    COR_POSOUTREAR = 0x2000,
};
enum COR_SHUTTER
{
    COR_SHTCLOSED = 0,
    COR_SHTOPEN = 1,
    COR_SHTJAMMED = 2,
    COR_SHTUNKNOWN = 3,
    COR_SHTNOTSUPPORTED = 4,
};
enum COR_POSITIONSTATUS
{
    COR_PSEMPTY = 0,
    COR_PSNOTEMPTY = 1,
    COR_PSUNKNOWN = 2,
    COR_PSNOTSUPPORTED = 3,
};
enum COR_TRANSPORT
{
    COR_TPOK = 0,
    COR_TPINOP = 1,
    COR_TPUNKNOWN = 2,
    COR_TPNOTSUPPORTED = 3,
};
enum COR_TRANSPORtSTATUS
{
    COR_TPSTATEMPTY = 0,
    COR_TPSTATNOTEMPTY = 1,
    COR_TPSTATNOTEMPTYCUST = 2,
    COR_TPSTATNOTEMPTY_UNK = 3,
    COR_TPSTATNOTSUPPORTED = 4,
};
typedef struct tag_cor_status
{
    COR_DEVSTATUS fwDevice;
    COR_SAFEDOOR fwSafeDoor;
    COR_DISPENER_STATUS fwDispenser;
    COR_STACKER fwIntermediateStacker;
    COR_ACCEPTOR fwAcceptor;
    COR_STACKITEM fwStackerItems;
    COR_BANKREAD fwBanknoteReader;
    BOOL bDropBox;
    WORD fwPosition;
    COR_SHUTTER fwShutter;
    COR_POSITIONSTATUS fwPositionStatus;
    COR_TRANSPORT fwTransport;
    COR_TRANSPORtSTATUS fwTransportStatus;
} ST_COR_STATUS, *PST_COR_STATUS;

typedef void (*OnCashIn)(DWORD, DWORD, DWORD);
struct IDevCOR : IDevBase
{
    //获取币筒信息
    virtual long GetCoinCylinderList(ST_COINCYLINDER_INFO pCoinCylinderInfo[MAX_COINCYLINDER_NUM]) = 0;
    //设置币筒存量数
    virtual long SetCoinCylinderInfo(ST_COINCYLINDER_INFO pCoinCylinderInfo) = 0;
    //开启投币口和币筒
    virtual long OpenClap() = 0;
    //关闭投币口
    virtual long CloseClap() = 0;
    //设置验钞支持的钞票类型
    virtual long SetSupportCoinTypes(USHORT usCoinEnable) = 0;
    //存款 RecvItems返回各币筒收到的硬币数量
    virtual long CashIn(DWORD dwTimeOut, ULONG RecvItems[MAX_COINCYLINDER_NUM], ST_RETRACTBIN_COUNT &stRetract, DEV_CUERROR eCUError[MAX_COINCYLINDER_NUM],
                        OnCashIn fnCallback = nullptr) = 0;
    //功能：指定各出币筒出币数进行出币
    virtual long Dispense(const ULONG RequestItems[MAX_COINCYLINDER_NUM], ULONG DispensedItems[MAX_COINCYLINDER_NUM],
                          DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM]) = 0;
    virtual long Count(int iCylinderNo, ULONG DispensedItems[MAX_COINCYLINDER_NUM], ULONG CountedItems[MAX_COINCYLINDER_NUM],
                       DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM]) = 0;
    virtual long Rollback(DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM]) = 0;
    virtual long CoinIn(USHORT usCoinEnable) = 0;
    virtual long CoinInEnd() = 0;
    //按金额请求支出硬币。 ulCoinValue为总金额，单位为货币最小单位量  如中国硬币为“分”
    virtual long Payout(ULONG ulCoinValue) = 0;
    //已支出金额
    virtual long GetPayoutValue(ULONG &ulCoinValue) = 0;
    //获取存入或支出的总金额
    virtual long GetMoneyCount(MONEY_COUNTERS_TYPE iCounters, ULONG &ulMoneyValue) = 0;
    //清空存入和支出的金额计数
    virtual long ClearMoneyCounter() = 0;
    //获取数据存储区参数
    virtual long GetDataStorageAvai(int &iMemoryType, int &iReadBlocks, int &iReadBlockSize, int &iWriteBlocks, int &iWriteBlockSize) = 0;
    //写入数据
    virtual long WriteData(int iBlockNo, BYTE *szData, int iLen) = 0;
    //读取数据
    virtual long ReadData(int iBlockNo, BYTE szData[MAX_DATA_LENGTH], int &iLen) = 0;
    //获取设备厂商、型号等相关信息
    virtual long GetDevInfo(ST_DEV_INFO &stDevInfo) = 0;

    virtual long GetLastError() override { return 0; }
    long GetErrorString(long lErrorCode, char szErrorString[MAX_LEN_BUFFER]) override
    {
        if (szErrorString != nullptr)
        {
            const char *pDesc = nullptr;
            switch (ERRORCODE(lErrorCode))
            {
            case ERR_COR_INVALIDCURRENCY:
                pDesc = "无效的币种";
                break;
            case ERR_COR_INVALIDTELLERID:
                pDesc = "无效的TellID";
                break;
            case ERR_COR_TOOMANYITEMS:
                pDesc = "硬币太多";
                break;
            case ERR_COR_UNSUPPOSITION:
                pDesc = "不支持的位置";
                break;
            case ERR_COR_SAFEDOOROPEN:
                pDesc = "安全门没有关闭";
                break;
            case ERR_COR_SHUTTERNOTOPEN:
                pDesc = "Shutter没有打开";
                break;
            case ERR_COR_SHUTTEROPEN:
                pDesc = "Shutter打开";
                break;
            case ERR_COR_SHUTTERCLOSED:
                pDesc = "Shutter关闭";
                break;
            case ERR_COR_INVALIDCASHUNIT:
                pDesc = "无效的币筒";
                break;
            case ERR_COR_NOITEMS:
                pDesc = "没有硬币";
                break;
            case ERR_COR_EXCHANGEACTIVE:
                pDesc = "正在交互过程中";
                break;
            case ERR_COR_NOEXCHANGEACTIVE:
                pDesc = "没有进入交换过程";
                break;
            case ERR_COR_SHUTTERNOTCLOSED:
                pDesc = "Shutter没有关闭";
                break;
            case ERR_COR_ITEMSTAKEN:
                pDesc = "硬币被取走";
                break;
            case ERR_COR_CASHINACTIVE:
                pDesc = "正在存入";
                break;
            case ERR_COR_NOCASHINACTIVE:
                pDesc = "不能进入存币状态";
                break;
            case ERR_COR_POSITION_NOT_EMPTY:
                pDesc = "位置非空";
                break;
            case ERR_COR_INVALIDRETRACTPOSITION:
                pDesc = "无效的回收位置";
                break;
            case ERR_COR_NOTRETRACTAREA:
                pDesc = "没有回收区域";
                break;
            case ERR_COR_INVALIDDENOMINATION:
                pDesc = "无效的配币";
                break;
            case ERR_COR_INVALIDMIXNUMBER:
                pDesc = "无效的配币";
                break;
            case ERR_COR_NOCURRENCYMIX:
                pDesc = "不存在的配币算法";
                break;
            case ERR_COR_NOTDISPENSABLE:
                pDesc = "不能出币";
                break;
            case ERR_COR_PRERRORNOITEMS:
                pDesc = "送币时，没有硬币";
                break;
            case ERR_COR_PRERRORITEMS:
                pDesc = "送币失败";
                break;
            case ERR_COR_PRERRORUNKNOWN:
                pDesc = "送币结果未知";
                break;
            case ERR_COR_INVALIDMIXTABLE:
                pDesc = "无效的MIX表";
                break;
            case ERR_COR_OUTPUTPOS_NOT_EMPTY:
                pDesc = "出币未知未空";
                break;
            case ERR_COR_NOCASHBOXPRESENT:
                pDesc = "没有分配币筒";
                break;
            case ERR_COR_AMOUNTNOTINMIXTABLE:
                pDesc = "金额错误";
                break;
            case ERR_COR_ITEMSNOTTAKEN:
                pDesc = "硬币未取走";
                break;
            case ERR_COR_ITEMSLEFT:
                pDesc = "有硬币剩余";
                break;
            case ERR_COR_COINNOTENOUGH:
                pDesc = "硬币不足";
                break;
            case ERR_COR_PAYOUT_FAIL:
                pDesc = "出币失败";
                break;
            case ERR_COR_COIN_BOX_FULL:
                pDesc = "币筒满";
                break;
            case ERR_COR_ACCEPT_ERROR:
                pDesc = "接收器故障";
                break;
            case ERR_COR_DISPENSER_FAILED:
                pDesc = "出币故障";
                break;
            case ERR_COR_STACKER_JAMMED:
                pDesc = "STACKER 阻塞";
                break;
            case ERR_COR_COIN_UNIT_JAMMED:
                pDesc = "币筒阻塞";
                break;
            case ERR_COR_JAMMED:
                pDesc = "机器阻塞";
                break;
            case ERR_COR_SHUTTER_JAMMED:
                pDesc = "Shutter阻塞";
                break;
            case ERR_COR_ACCEPTOR_ILLGGAL:
                pDesc = "接收硬币非法";
                break;
            case ERR_COR_DISPENSER_ILLGGAL:
                pDesc = "发现非法的出币";
                break;
            case ERR_COR_CASHUNITERROR:
                pDesc = "币筒故障";
                break;
            default:
                pDesc = Dev_Error_Descript(lErrorCode);
                break;
            }
            if (pDesc != nullptr)
            {
                strcpy(szErrorString, QString(pDesc).toLocal8Bit().data());
                return 0;
            }
            return SOFT_ERROR_UNKNOWNCODE;
        }
        return SOFT_ERROR_PARAMS;
    }
};
extern "C" DEVCOR_EXPORT long CreateIDevCOR(const char *pszDevType, IDevCOR *&pDevice);
