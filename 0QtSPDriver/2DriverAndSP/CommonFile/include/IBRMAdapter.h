#ifndef IBRM_ADAPTER_H
#define IBRM_ADAPTER_H

#include "QtTypeDef.h"
#include "XFSCDM.H"
#include "XFSCIM.H"
#include "CashUnitDefine.h"

#define BRM_ADAPTER_NOVTABLE    _declspec(novtable)

#define ADP_MAX_CU_SIZE         (6)

//未定义的币种ID
//用于SetCassetteInfo或StoreCash
#define NOTE_ID_UNKNOWN         ((USHORT)0xFFFF)

enum DEVICE_STATUS
{
    ADP_DEVICE_ONLINE     = 0,    //正常
    ADP_DEVICE_OFFLINE    = 1,    //设备物理状态改变
    ADP_DEVICE_POWER_OFF  = 2,    //不支持
    ADP_DEVICE_NODEVICE   = 3,    //不支持
    ADP_DEVICE_HWERROR    = 4,    //硬件报错，卡钞，电源线数据线断开等
    ADP_DEVICE_USERERROR  = 5,     //钞箱门被打开
    ADP_DEVICE_BUSY       = 6
};

enum SAFEDOOR_STATUS
{
    ADP_SAFEDOOR_CLOSED       =  0,
    ADP_SAFEDOOR_OPEN         =  1,
    ADP_SAFEDOOR_UNKNOWN      =  2,
    ADP_SAFEDOOR_NOTSUPPORTED =  3,
};

enum INSHUTTER_STATUS
{
    ADP_INSHUTTER_CLOSED       =  0,
    ADP_INSHUTTER_OPEN         =  1,
    ADP_INSHUTTER_JAMMED        =  2,
    ADP_INSHUTTER_UNKNOWN      =  3,
    ADP_INSHUTTER_NOTSUPPORTED =  4,
};

enum OUTSHUTTER_STATUS
{
    ADP_OUTSHUTTER_CLOSED = 0,
    ADP_OUTSHUTTER_OPEN = 1,
    ADP_OUTSHUTTER_JAMMED = 2,
    ADP_OUTSHUTTER_UNKNOWN = 3,
    ADP_OUTSHUTTER_NOTSUPPORTED = 4,
};

enum DISPENSER_STATUS
{
    ADP_DISPENSER_OK            = 0,
    ADP_DISPENSER_CUSTATE       = 1,
    ADP_DISPENSER_CUSTOP        = 2,
    ADP_DISPENSER_CUUNKNOWN     = 3,
};

enum INPOS_STATUS
{
    ADP_INPOS_EMPTY        = 0,
    ADP_INPOS_NOTEMPTY     = 1,
    ADP_INPOS_UNKNOWN      = 2,
    ADP_INPOS_NOTSUPPORTED = 3,
};

enum OUTPOS_STATUS
{
    ADP_OUTPOS_EMPTY        = 0,
    ADP_OUTPOS_NOTEMPTY     = 1,
    ADP_OUTPOS_UNKNOWN      = 2,
    ADP_OUTPOS_NOTSUPPORTED = 3,
};

enum SHUTTERPOS_STATUS
{
    ADP_SHUTPOS_EMPTY        = 0,
    ADP_SHUTPOS_NOTEMPTY     = 1,
    ADP_SHUTPOS_UNKNOWN      = 2,
    ADP_SHUTPOS_NOTSUPPORTED = 3,
};

enum STACKER_STATUS
{
    ADP_STACKER_EMPTY        = 0,
    ADP_STACKER_NOTEMPTY     = 1,
    ADP_STACKER_UNKNOWN      = 2,
    ADP_STACKER_NOTSUPPORTED = 3,
};

enum TRANSPORT_STATUS
{
    ADP_TRANSPORT_EMPTY        = 0,
    ADP_TRANSPORT_NOTEMPTY     = 1,
    ADP_TRANSPORT_UNKNOWN      = 2,
    ADP_TRANSPORT_NOTSUPPORTED = 3,
};

enum BANKNOTEREADER_STATUS
{
    ADP_BR_OK           =  0,
    ADP_BR_INOP         =  1,
    ADP_BR_UNKNOWN      =  2,
    ADP_BR_NOTSUPPORTED =  3,
};

enum ADP_CUERROR
{
    ADP_CUERROR_OK            =  0,
    ADP_CUERROR_EMPTY         =  1,
    ADP_CUERROR_FULL          =  2,
    ADP_CUERROR_RETRYABLE     =  3, //可重试类错误
    ADP_CUERROR_FATAL         =  4, //致命错误，不可再用
    ADP_CUERROR_NOTUSED       =  5, //该钞箱未在本操作中使用
};

enum ADP_ERR_REJECT
{
    ADP_ERR_REJECT_OK            =  0,//没有拒钞
    ADP_ERR_REJECT_INVALIDBILL   =  1,//发现非法钞票导致拒钞
    ADP_ERR_REJECT_STACKERFULL   =  2,//暂存箱满
    ADP_ERR_REJECT_NOBILL        =  3,//没有检查到钞票
    ADP_ERR_REJECT_OTHER         =  4,//其他原因
};

//设置回收或传送钞票时的目的位置
enum ADP_RETRACTEARE
{
    ADP_RETRACTEARE_NOSET           =  0,//不设置位置
    ADP_RETRACTEARE_RETRACTBOX      =  1,//回收箱
    ADP_RETRACTEARE_TRANSPORT       =  2,//通道
    ADP_RETRACTEARE_STACKER         =  3,//暂存区
    ADP_RETRACTEARE_REJECTBOX       =  4,//废钞箱
    ADP_RETRACTEARE_RECYCLEBOX      =  5,//循环箱
    ADP_RETRACTEARE_CASHSLOT        =  6,//钞口
};

typedef struct _tag_adp_status_infor
{
    // 整机状态
    DEVICE_STATUS           stDevice;
    // 安全门状态
    SAFEDOOR_STATUS         stSafeDoor;
    // 钞门状态
    INSHUTTER_STATUS            stInShutter;
    OUTSHUTTER_STATUS           stOutShutter;
    // 进钞口状态
    INPOS_STATUS            stInPutPosition;
    //出钞口状态
    OUTPOS_STATUS           stOutPutPosition;
    //钞门处状态
    SHUTTERPOS_STATUS       stShutterPosition;
    // 暂存区状态
    STACKER_STATUS          stStacker;
    // 传输通道状态
    TRANSPORT_STATUS        stTransport;
    // 钞票鉴别模块状态
    BANKNOTEREADER_STATUS   stBanknoteReader;
    _tag_adp_status_infor()
    {
        stDevice = ADP_DEVICE_OFFLINE;
        stSafeDoor = ADP_SAFEDOOR_UNKNOWN;
        stInShutter = ADP_INSHUTTER_UNKNOWN;
        stOutShutter = ADP_OUTSHUTTER_UNKNOWN;
        stInPutPosition = ADP_INPOS_UNKNOWN;
        stOutPutPosition = ADP_OUTPOS_UNKNOWN;
        stStacker = ADP_STACKER_UNKNOWN;
        stTransport = ADP_TRANSPORT_UNKNOWN;
        stBanknoteReader = ADP_BR_UNKNOWN;
        stShutterPosition = ADP_SHUTPOS_UNKNOWN;
    }
    _tag_adp_status_infor(DEVICE_STATUS devStatus)
    {
        stDevice = devStatus;
    }
} ADPSTATUSINFOR, *LPADPSTATUSINFOR;

typedef struct _adp_cdm_caps
{
    WORD wMaxDispenseItems;   // 单次最大点钞张数
    BOOL bShutter;               // 是否有shutter
    BOOL bShutterControl;       //TRUE:the shutter is controlled implicitly by the service provider
    //the shutter must be controlled explicitly by the application using the WFS_CMD_CDM_OPEN_SHUTTER and the WFS_CMD_CDM_CLOSE_SHUTTER
    WORD fwRetractAreas;        //钞币可能的回收位置
    WORD fwRetractTransportActions;
    WORD fwRetractStackerActions;
    BOOL bSafeDoor;              // 是否有安全门
    BOOL bIntermediateStacker;     // 是否有暂存区
    BOOL bItemsTakenSensor;      // 是否能感知门口钞票被取走
    WORD fwPositions;            //可能的输出位组合, 只允许为LEFT、RIGHT等其中一个
    WORD fwMoveItems;           // 同XFS中CDM的相关定义
} ADPCDMCAPS, * LPADPCDMCAPS;

typedef struct _adp_cim_caps
{
    WORD wMaxCashInItems;      // 一次交易最大存入钞票张数
    BOOL bShutter;               // 是否有shutter
    BOOL bShutterControl;        //TRUE:the shutter is controlled implicitly by the service provider
    //the shutter must be controlled explicitly by the application using the WFS_CMD_CDM_OPEN_SHUTTER and the WFS_CMD_CDM_CLOSE_SHUTTER
    BOOL bSafeDoor;              //Specifies whether or not the WFS_CMD_CDM_OPEN_SAFE_DOOR command is supported
    WORD fwIntermediateStacker;   // 暂存区能够叠放的钞票张数
    BOOL bItemsTakenSensor;
    BOOL bItemsInsertedSensor;    // 是否能感知门口钞票被放入
    WORD fwPositions;             //可能的输入、输出位组合, 只允许为LEFT、RIGHT等其中一个, 如为LEFT，必须为INLEFT | OUTLEFT
    WORD fwRetractAreas;
    WORD fwRetractTransportActions;
    WORD fwRetractStackerActions;
} ADPCIMCAPS, * LPADPCIMCAPS;

//test#8 add start
enum ADP_LFS_CMD_ID{
    ADP_CASHIN          = 0,
    ADP_CASHINEND       = 1,
    ADP_CIM_RETRACT     = 2,
    ADP_CIM_RESET       = 3,
    ADP_DISPENSE        = 4,
    ADP_CDM_RETRACT     = 5,
    ADP_CDM_RESET       = 6,
    ADP_OTHER           = 7
};
//test#8 add end

/*
 *
enum BRMTYPE
{
    BRMTYPE_URT = 0,
    BRMTYPE_NMD100 = 1,
};
*/
struct ADPDEVINFOR
{
    BOOL bHaveInShutter;
    BOOL bHaveOutShutter;
};


struct IBRMExtra
{
    virtual void AddExtra(LPCSTR lpszkey, LPCSTR lpszValue) = 0;
};

struct  IBRMAdapter
{

    //释放对象
    virtual void Release() = 0;

    //功能：打开机芯通信连路,建立PC与机芯间的通信
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：串口号、USB端口号等参数由适配层自己决定，不由SP传入
    virtual long Open() = 0;

    //功能：关闭机芯
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long Close() = 0;


    //功能：设置等待完成的接口
    //输入：pWait：回调接口
    //输出：无
    //返回：无
    //说明：
    //virtual void SetWaitCallback (/*IWaitComplete *pWait*/) = 0;

    //功能：设置状态改变侦听接口
    // 参数：
    //  pListener：【输入】，回调接口指针，如果NULL，取消侦听
    //返回值：TRUE，设置成功，支持状态侦听；否则，不支持状态侦听
    //virtual BOOL SetStatusChangeListener(/*IStatusChangeListener *pListener*/) = 0;

    //功能：取设备状态
    //输入：Status：设备状态
    //输出：无
    //返回：无
    //说明：
    virtual long GetStatus(ADPSTATUSINFOR &Status) = 0;

    //功能：更新状态，该函数会访问设备，仅在不调用其他动作函数时更新状态
    //输入：无
    //输出：无
    //返回：无
    //说明：
    virtual long UpdateStatus() = 0;

    //功能：更新钞口状态
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long UpdatePostionStatus() = 0;

    //功能：获取出钞机芯能力
    //输入：无
    //输出：lpCaps：出钞机芯能力结构指针,定义同xfs的CDM部分。
    //返回：XFS通用错误码
    //说明：
    virtual long GetCDMCapabilities(LPADPCDMCAPS lpCaps) = 0;

    //功能：获取进钞机芯能力
    //输入：无
    //输出：lpCaps：进钞机芯能力结构指针,定义同xfs的CIM部分。
    //返回：XFS通用错误码
    //说明：
    virtual long GetCIMCapabilities(LPADPCIMCAPS lpCaps) = 0;

    //功能：设置BV支持的钞票类型
    //输入：lpList：整机支持的钞票列表,同XFS定义
    //输出：无
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long SetSupportNoteTypes(const LPWFSCIMNOTETYPELIST lpList) = 0;

    //功能：设置钞箱是否启用
    //输入：bEnable：大小为5的数组,指示各钞箱是否启用,出钞、存款交易前调用。
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long EnableCashUnit(const BOOL bEnable[ADP_MAX_CU_SIZE]) = 0;

    //功能：指定各出钞箱出钞数,进行点钞
    //输入：aryulItems,各钞箱要挖钞张数
    //输出：aryulDispenseCount：每个钞箱点出的钞数,不含废钞数；
    //输出：aryulRejects：对于取款箱,表示废钞数；对于回收箱,表示回收数,理论上,所有废钞箱、回收箱的Reject数总和应等于出钞箱、循环箱的Reject数之和。
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //说明：无
    virtual long Dispense(const ULONG aryulItems[ADP_MAX_CU_SIZE],
                          ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                          ULONG aryulRejects[ADP_MAX_CU_SIZE],
                          ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]) = 0;

    //功能：开始进入存款模式
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long StartCashIn() = 0;

    //功能：验钞
    //输入：无
    //输出：pResult：各钞票种类张数信息,单条数据的格式为"[钞票ID:char6][张数:char6]"样式.
    //               例：ID为1的钞票16张,ID为2的钞票20张："000001000016000002000020",pResult指向内存由调用者分配管理.
    //输出：ulRejCount:废钞张数
    //输出：iErrReject: 拒钞发生原因
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long ValidateAndCounting(char pResult[256], ULONG &ulRejCount, ADP_ERR_REJECT &iErrReject) = 0;

    //功能：将进钞钞票存入钞箱
    //输入：无
    //输出  ppNoteCounts：存放各钞箱存入的各种钞票张数,每个数组成员返回内容为各通道钞箱的各种钞票张数列表。单条格式为："[钞票ID:char6][钞票张数:char6]",
    //      如："000001000030000002000013"代表钞票ID为1的钞票有30张,ID为2的钞票有13张;
    //      arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long StoreCash(char ppNoteCounts[ADP_MAX_CU_SIZE][256], ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]) = 0;

    //功能：退回存款钞票 将存款点钞完的钞票退出到出钞口
    //输入：无
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long RollBack(ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]) = 0;

    //功能：将Dispense后存在暂存区的钞票送到门口
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long Present() = 0;

    //功能：回收 将Present后在门口的钞票,Dispense后存在暂存区的钞票回收到回收箱
    //输入：usRetraceArea：钞票回收的目的位置
    //      usIndex:当usRetraceArea指定为某个钞箱时有效,表示钞箱索引,从1开始,依次增1
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CDM错误码
    //说明：无
    virtual long Retract(ADP_RETRACTEARE usRetractArea, USHORT &usIndex, ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]) = 0;

    //功能：回收拒钞 将Dispense后存在暂存区的钞票回收到废钞箱
    //输入：usIndex,表示钞箱索引,从1开始,依次增1
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long Reject(USHORT usIndex, ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]) = 0;

    //功能：复位机芯,将机芯置恢复到已知正常状态
    //输入：usRetraceArea：钞票回收的目的位置
    //      usIndex:当usRetraceArea指定为某个钞箱时有效,表示钞箱索引,从1开始,依次增1
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //      bMediaDetected：复位过程中是否检测到介质
    //      usRetraceArea：当SP没有指定具体回收位置时，返回适配层实际回收的位置和索引
    //      usIndex:当SP没有指定具体回收位置时，返回适配层实际回收的索引
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long Reset(ADP_RETRACTEARE &usRetraceArea, USHORT &usIndex,
                       BOOL &bMediaDetected, ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]) = 0;

    //功能：取钞票ID列表,即每次有产生读取钞票序列号动作后的序列号
    //输入：无
    //输出：无
    //返回：返回序列号条目数量
    //说明：
    virtual ULONG GetNoteSeiralNumbersCount() = 0;

    //功能：取钞票每次有产生读取钞票序列号动作后的序列号
    //输入：dwIndex：第几个条目,从0开始,至总数量-1；
    //输出：
    //      usNoteID：币种代码；
    //      arycSerialNumber[128]：序列号。
    //返回：XFS通用错误码
    //说明：
    virtual long GetNoteSeiralNumbers(ULONG dwIndex, USHORT &usNoteID, char arycSerialNumber[128]) = 0;


    //功能：设钞箱信息
    //输入：wHopperID:钞箱序号,从1开始,依次增1,最大ADP_MAX_CU_SIZE
    //      Currency:币种
    //      ulValue：面额
    //      eType：钞箱类型;
    //      lpusNoteIDs:以0结尾的整数数组,表示钞箱支持的钞票类型ID列表,
    //          如果要收纳所有不可接受的币种,那么ID列表最后一个的值为NOTE_ID_UNKNOWN。
    //          REJECT、出钞箱，该值可为NULL
    //      ulCount: 当前张数
    //输出：无
    //返回：XFS通用错误码及CIM错误码
    //说明：设置完成后须立即调用初始化复位方法
    virtual long SetCassetteInfo(WORD wHopperID, const char Currency[3], ADP_CASSETTE_TYPE eType,
                                 ULONG ulValue, LPUSHORT lpusNoteIDs, ULONG ulCount) = 0;

    //功能：取钞箱信息
    //输入：wHopperID,钞箱序号,从1开始,依次增1
    //输出：
    //      eStatus:状态（取值定义与XFS相同）
    //      cCassID:物理钞箱ID
    //返回：XFS通用错误码及CIM错误码
    //说明：1)  由于上层没有用到长、宽、厚等信息,适配层可在下面自己做相关校验。
    virtual long GetCassetteInfo(WORD wHopperID, CASHUNIT_STATUS &eStatus, char cCassID[5]) = 0;

    //功能：测试钞箱。
    //输入：无
    //输出：aryulDispenseCount：测试钞箱时每个钞箱点出的钞数,不含废钞数；
    //      aryulRejects：对于取款箱,表示废钞数,对于回收箱,表示回收数;
    //      arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CDM错误码
    //说明：无
    virtual long TestCashUnits(ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                               ULONG aryulRejects[ADP_MAX_CU_SIZE], ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]) = 0;

    //功能：取固件版本。
    //输入：无
    //输出：cVersion,统计信息,长度包括0结尾不超过512个字符
    //返回：XFS通用错误码
    //说明：
    virtual long GetFirmwareVersion(char cVersion[512]) = 0;

    //功能：取最后错误码
    //输入：无
    //输出：无
    //返回：返回最后错误代码
    //说明：
    virtual const char *GetLastError() = 0;

    //功能：获取附加状态
    //输入：pExtra：附加信息设置接口指针,调用该接口完成附加状态的设置
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long GetExtraInfor(IBRMExtra *pExtra) = 0;

    //功能：通知机芯进入交换状态,机芯可根据实际情况执行相应的预处理。
    //输入：arybExFlag：将被交换的钱箱。
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long StartExchange(const BOOL arybExFlag[ADP_MAX_CU_SIZE]) = 0;

    //功能：通知机芯退出交换状态,机芯可根据实际情况执行相应的收尾处理。
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long EndExchange() = 0;

    //功能：在执行复位机芯、Retract及Reject操作时,机芯可能会回收到钞票,通过此方法可获取回收到每个钞箱的钞数。
    //输入：无
    //输出：arywRetractCount 获取回收钞票张数，在执行可能产生回收的操作之后,得到回收的钞数信息。
    //返回：0---获取成功；
    //      其它---获取失败
    //说明：
    virtual long GetRetractCount(ULONG aryulRetractCount[ADP_MAX_CU_SIZE]) = 0;

    //功能：开门
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long OpenShutter(BOOL bInShutter) = 0;

    //功能：关门
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long CloseShutter(BOOL bInShutter) = 0;

    //功能：操作后是否需要复位
    //输入：无
    //输出：无
    //返回：TRUE: 需要复位 ；FALSE: 不需要复位
    //说明：
    virtual BOOL IsNeedReset() = 0;

    //功能:  判断在钞箱启用条件下，存取款是否可以进行
    //输入:  bCDM:是否是CDM操作。
    //       bCassEnable:钞箱启用条件。TRUE:该钞箱被启用; FALSE: 不被启用
    //返回：TRUE:  存取款可以进行；FALSE: 存取款不可以进行
    virtual BOOL IsOperatable(BOOL bCDM, const BOOL bCassEnable[ADP_MAX_CU_SIZE]) = 0;

    //virtual BOOL GetNotesRetractCountsExtraFirstStr(string &strKey, string &strValue) = 0;

    //virtual BOOL GetNotesRetractCountsExtraNextStr(string &strKey, string &strValue) = 0;

    virtual long GetRetractNoteIdAndCountPairs(char arycResult[256], ULONG &ulRejCount) = 0;

    virtual BOOL IsNeedDownLoadFW() = 0;

    virtual long DownLoadFW() = 0;

    virtual void SetDownLoadFWFlag(BOOL bFlag) = 0;

    virtual void  GetDevConfig(ADPDEVINFOR &devinfo) = 0;

    virtual int  GetResultFlg() = 0;  //test#7
    virtual void SetResultFlg() = 0;  //test#7

    virtual void SetLFSCmdId(ADP_LFS_CMD_ID eLfsCmdId) = 0;           //test#8
};//struct BRM_ADAPTER_NOVTABLE IBRMAdapter

extern "C" int CreateBRMAdapter(IBRMAdapter *&pAdapter);

#endif //IBRM_ADAPTER_H
