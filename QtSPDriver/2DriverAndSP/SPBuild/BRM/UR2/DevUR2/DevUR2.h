#pragma once

//#include "USB_Hitachi.h"
#include "QtTypeDef.h"
#include "IURDevice.h"
#include "VHUSBDrive.h"
#include "ILogWrite.h"
#include "ErrCodeMap.h"
#include "StlSimpleThread.h"
#include "AutoQtHelpClass.h"
#include "SimpleMutex.h"
#include "LogWriteThread.h"
#include "INIFileReader.h"              //30-00-00-00(FS#0016)

// 验钞模式
enum NOTES_DENO_INFO_TYPE
{
    STACKE_NOTES_DENO_INFO_TYPE = 0x00, // 每种面额钞票的流向、张数信息
    UNFIT_NOTES_DENO_INFO_TYPE,         // 不适合流通的钞票面额、流向、张数信息
    REJECT_SOURCE_NOTES_DENO_INFO_TYPE, // 回收的钞票面额的来源、张数信息
    REJECT_DEST_NOTES_DENO_INFO_TYPE,   // 回收的钞票面额的流向、张数信息
    REJECT_BV_NOTES_DENO_INFO_TYPE,      // 回收的钞票面额的流向、张数信息
    NOTES_DENO_INFO_TYPE_OTHER
};

class CUR2Drv;

typedef struct tag_THREAD_PARM //线程参数
{
    const UCHAR     *pszCmd;        //执行要发送的命令
    int             iCmdLen;        //命令pszCmd长度
    char            *szResp;        //指向响应数据
    DWORD           dwRespLen;      //响应数据长度：输入是缓冲区长度，输出为返回数据长度
    DWORD           dwMinRespLen;   //响应数据的最小长度
    int             iTimeout;       //超时
    int             nRet;           //返回码
    const char      *pszFuncName;   //启动线程的方法
    BOOL            bPDL;           //是否为下载固件
} ThreadParm;

typedef struct tag_PACKET_INFO     //分析响应包数据
{
    USHORT usPacketID;      //包ID
    USHORT usLength;        //包长度
    USHORT usDataPos;       //数据段偏移

    tag_PACKET_INFO()
    {
        usPacketID = 0;
        usLength   = 0;
        usDataPos  = 0;
    }
} ST_PACKET_INFO;

typedef map<USHORT, ST_PACKET_INFO> PACKETMAP;

class CUR2Drv :
    public IURDevice,
    public CLogManage
{
public:
    CUR2Drv();
    virtual ~CUR2Drv();

    //功能：释放本接口
    virtual void Release();

    //功能：打开USB连接
    //输入：eConnectType, 连接类型
    //输出：
    //返回：0: 成功，非0: 失败
    //说明：打开ZVB和HCM的USB连接
    virtual int OpenURConn();

    //功能：打开BV连接
    //输入：eConnectType, 连接类型
    //输出：
    //返回：0: 成功，非0: 失败
    //说明：打开ZVB和UR的USB连接
    virtual int OpenBVConn();

    //功能：关闭UR连接
    //输入：
    //输出：无
    //返回：0: 成功，非0: 失败
    //说明：关闭ZVB和HCM的USB连接
    virtual int CloseURConn();

    //功能：关闭BV连接
    //输入：
    //输出：无
    //返回：0: 成功，非0: 失败
    //说明：关闭ZVB和HCM的USB连接
    virtual int CloseBVConn();

    //功能1：获取HCM和ZVB固件版本信息并初始化机芯,在上电或Reboot后需要首先发送该命令
    //功能2：获取HCM和ZVB固件版本信息
    //输入：bNeedInitial 为TRUE使用功能1，FALSE使用功能2
    //输出：szFWVersion: 各固件程序的版本信息字符串，由使用该接口的测试程序进行解析
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int GetFWVersion(
    char szFWVersion[MAX_FW_DATA_LENGTH],
    USHORT &usLen,
    BOOL bNeedInitial = FALSE
    );

    //功能：设置校验级别，减少废钞率
    //输入：stSetVerificationLevel：需要减少废钞率设置为TRUE，否则设置为FALSE
    //      建议：cashcount设置为FALSE，storemoney和dispense设置为TRUE以减少废钞率
    virtual int SetVerificationLevel(SET_VERIFICATION_LEVEL stSetVerificationLevel);

    //功能：查询BV序列号信息以及BV支持的币种ID面额额配置表信息
    //输入：无
    //输出：pBVInfo:BV信息结构
    //输出：pArrDenoInfo:面额信息结构数组，数组大小为128;
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    //      在上电或Reboot后需要重新发送该命令
    virtual int GetBanknoteInfo(
    ST_BV_INFO &pBVInfo,
    ST_DENO_INFO pArryDenoInfo[MAX_DENOMINATION_NUM]
    );

    //功能：获得所有钞箱信息
    //输入：无
    //输出：pArrCassInfo:钞箱信息结构数组，数组大小为6;
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：120
    virtual int GetCassetteInfo(
    ST_CASSETTE_INFO pArryCassInfo[MAX_CASSETTE_NUM]
    );

    //功能：设置支持的面额代码
    //输入：pArryDENOCode: 需要支持的面额数组取值为DENOMINATION_CODE,已0结束
    //      例如支持面额 124,pArryDENOCode=12400000.., 支持面额4，pArryDENOCode=40000..,
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：20
    virtual int SetDenominationCode(
    const char pArryDENOCode[MAX_DENOMINATION_NUM]
    );

    //功能：设置HCM钞箱的配置和操作信息，设置命令执行后必须执行复位动作，否则动作指令报错
    //      如果HCM接受设置，设置完成后通过GetUnitInfo命令可读取设置的值，如果HCM不接受该值设置，该值则为0
    //输入：tTime: 系统时间
    //输入：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输入：stOperationalInfo: HCM控制设置
    //输入：stHWConfig: HCM硬件信息设置
    //输入：stCassType: 钞箱类型 ST_CASSETTE_INFO结构, 以及钞箱支持的面额 取值范围DENOMINATION_CODE，DRB箱可设置多种面额，目前一个钞箱只支持一种面额设置
    //输入：bArryAcceptDenoCode: 可接受面额设置 1-127种面额，128面额为不可识别面额，可接受TRUE，不接受FALSE
    //输入：stCashCountLevel: 验钞动作BV校验钞票严格程度
    //输入：stStoreMoneyLevel:存钞动作BV校验钞票严格程度
    //输入：stDispenseLevel:  挖钞动作BV校验钞票严格程度
    //输入：usArryCassCashInPrioritySet:  存钞时钞箱优先级，取值范围 1~10，取值越小优先级越高,例如123456
    //输入：usArryCassCashOutPrioritySet: 挖钞时钞箱优先级，取值范围 1~10，取值越小优先级越高
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int SetUnitInfo(
    int iTotalCashInURJB,
    const ST_OPERATIONAL_INFO &stOperationalInfo,
    const ST_HW_CONFIG &stHWConfig,
    const ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM],
    const BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM],
    const ST_BV_VERIFICATION_LEVEL &stCashCountLevel,
    const ST_BV_VERIFICATION_LEVEL &stStoreMoneyLevel,
    const ST_BV_VERIFICATION_LEVEL &stDispenseLevel,
    const char usArryCassCashInPrioritySet[MAX_ROOM_NUM],                   //30-00-00-00(FS#0022)
    const char usArryCassCashOutPrioritySet[MAX_ROOM_NUM],                  //30-00-00-00(FS#0022)
    const BOOL bURJBSupp
    );

    //功能：获取HCM钞箱的配置和操作信息
    //输入：无
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0X8000
    //输出：bArryDenoCodeBySet: SetUnitInfo命令中的bArryAcceptDenoCode信息
    //输出：bArryAcceptDenoCode: 可接受面额信息 1-127种面额，128面额为不可识别面额，可接受TRUE，不接受FALSE
    //输出：stHWConfig: HCM硬件信息设置
    //输出：stCassType: 钞箱类型 ST_CASSETTE_INFO结构, 以及钞箱支持的面额 取值范围DENOMINATION_CODE，DRB箱可设置多种面额，目前一个钞箱只支持一种面额设置
    //输出：stCashCountLevel: 验钞动作BV校验钞票严格程度
    //输出：stStoreMoneyLevel:存钞动作BV校验钞票严格程度
    //输出：stDispenseLevel:  挖钞动作BV校验钞票严格程度
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：20
    virtual int GetUnitInfo(
    int &iTotalCashInURJB,
    BOOL bArryDenoCodeBySet[MAX_DENOMINATION_NUM],
    BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM],
    ST_OPERATIONAL_INFO &stOperationalInfo,
    ST_HW_CONFIG &stHWConfig,
    ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM],
    ST_BV_VERIFICATION_LEVEL &stCashCountLevel,
    ST_BV_VERIFICATION_LEVEL &stStoreMoneyLevel,
    ST_BV_VERIFICATION_LEVEL &stDispenseLevel
    );

    //功能1 复位命令，检测所有执行器与传感器，并将TS、传输通道钞票移动到CS中.
    //功能2 复位并取消节能模式，需要满足以下取消节能模式条件才能正常返回，否则返回错误。
    //      1.HCM在掉电前成功执行设置节能模式 2.外部Shutter已经关闭 3.HCM CS/TS，传输通道没有钞票残留
    //      4.所有的上部、下部机构、CS/TS单元都在正确位置 5.所有URJB/TS/BV 均关闭
    //      6.上部或下部机构在进入节能模式后没有被访问 7.TS门在进入节能模式后没有被访问
    //功能3 快速复位命令，在HCM状态符合快速复位条件，则不执行机械动作，只检测传感器状态。如果HCM状态不满足快速复位条件或者刚上电后则进行复位命令
    //      快速复位条件
    //      1.CS\TS都为空; 2.没有传感器检测到钞票残留在传输路径上; 3.Shutter门处于关闭状态
    //      4.最后一次Reset命令没有返回错误; 5.StoreMoney、Dispense、CashRollback、Reset命令成功返回或返回警告
    //      6.所有的内部导轨/挡板均在初始位置; 7.TS没有被打开; 8.HCM没有脱离; 9.CS没有检测到强制移动钞票
    //输入：bCancelEnergySaveMode: 是否取消节能模式, TRUE 取消 FLASE: 使用复位命令
    //输入：bQuickRest: 是否使用快速复位, TRUE: 快速复位  FALSE: 复位
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：180   快速复位与取消不能同时使用
    virtual int Reset(
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],
    BOOL bCancelEnergySaveMode = FALSE,
    BOOL bQuickRest = TRUE
    );

    //功能：请求HCM模块中保存的日志数据
    //输入：iLogType: 日志类型，不可取值LOG_INFO_ALLDATA
    //输入：iLogTerm: 日志信息周期
    //输出：pszLogData: 返回日志数据，最大7174 * 255字节
    //输出：iLogDataLen: 返回日志数据长度
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：120
    //      日志数据不会因断电而清除，只会被EraseAllLogData指令清除
    virtual int GetLogData(
    LOG_INFO_TYPE iLogType,
    LOG_INFO_TERM iLogTerm,
    char pszLogData[MAX_LOG_DATA_LENGTH],
    DWORD &iLogDataLen
    );

    //功能：清除HCM模块中所有保存的日志数据
    //输入：无
    //输出：无
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：120
    //      只有在新购买/更换HCM主板时可以使用
    virtual int EraseAllLogData();

    //功能：点钞存入的钞票，将钞票从CS钞口送入ESC暂存箱
    //输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
    //输入：iNumDepositLimit: 存款的最大钞票张数
    //输入：iNumAmountLimit:  ESC最大面值总额=iNumAmountLimit * 10^[iPowerIndex]
    //输入：iPowerIndex: 钞票指数，如果iNumAmountLimit为0,该值忽略
    //输入：bArryAcceptDeno: 可接受的面额:128种面额，TRUE为接受，FALSE为不可接受，NULL为不设置该值使用默认设置
    //输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为1时有效，否则忽略
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
    //输出：nRejectNotes: 表示退回到CS的钞票总数
    //输出：stStackeNotesDenoInfo：每种面额钞票的流向、张数信息
    //输出：usFedNoteCondition: 钞票传输情况取值位运算，FED_NOTE_CONDITION 包含多种情况 CONDITION_SKEW & CONDITION_SHORT_NOTE
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：25张以上的拒钞将返回警告。
    //说明：执行命令的最大时间(秒)：180
    virtual int CashCount(
    VALIDATION_MODE iValidateMode,
    int iNumDepositLimit,
    int iNumAmountLimit,
    NOTE_POWER_INDEX iPowerIndex,
    const BOOL bArryAcceptDeno[MAX_DENOMINATION_NUM],
    const ST_BV_DEPENDENT_MODE stBVDependentMode,
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    int &nRejectNotes,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
    USHORT &usFedNoteCondition,
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
    );

    //功能1：点钞存入的钞票，将钞票从CS钞口送入ESC暂存箱，或者将钞票回收至URJB(通过SET_UNIT_INFO命令设置回收位置或者参数设置)
    //功能2：点钞存入的钞票，将钞票从CS钞票回收至ESC暂存箱
    //输入：bIgnoreESC: 是否忽略ESC检测，TRUE：忽略，FALSE：不忽略，点钞前如果ESC有钞命令返回失败
    //输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
    //输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为1时有效，否则忽略
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
    //输出：nRejectNotes: 表示退回到CS的钞票总数
    //输出：stStackeNotesDenoInfo：每种面额钞票的流向、张数信息
    //输出：usFedNoteCondition: 钞票传输情况取值位运算，FED_NOTE_CONDITION 包含多种情况 CONDITION_SKEW & CONDITION_SHORT_NOTE
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：180
    virtual int CashCountRetract(
    VALIDATION_MODE iValidateMode,
    const ST_BV_DEPENDENT_MODE stBVDependentMode,
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    int &nRejectNotes,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
    USHORT &usFedNoteCondition,
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],
    BOOL bIgnoreESC = FALSE
    );

    //功能：存入钞票，将钞票从ESC暂存箱送入到指定的各钞箱
    //输入：bCheckCS 存款前是否检查CS是否有钞，有钞返回错误
    //输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
    //输入：iDestinationReject: 指定回收位置传入DESTINATION_REJCET_DEFAULT则使用SET_UNIT_INFO中设置的值
    //输入：btProhibitedBox: 表示禁止出钞的钱箱。1字节，0位~4位分别表示钱箱1~6,其余位值恒为0
    //      位值为1表示禁止从对应的钱箱存钞
    //输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为TRUE时有效，否则忽略
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
    //输出：stStackeNotesDenoInfo：每种面额钞票的流向、张数信息
    //输出：stStackeUnfitNotesDenoInfo：不适合流通的钞票面额、流向、张数信息
    //输出：stStackeRejectNotesDenoInfo：回收的钞票面额、流向、张数信息
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：180
    virtual int StoreMoney(
    VALIDATION_MODE iValidateMode,
    BOOL  bCheckCS,
    DESTINATION_REJCET iDestinationReject,
    char btProhibitedBox,
    const ST_BV_DEPENDENT_MODE stBVDependentMode,
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeUnfitNotesDenoInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoInfo,
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
    );

    //功能：取消存款：将暂存箱中的钞票送至出钞口，将钞票返回给用户
    //输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
    //输出：stStackeNotesDenoInf: 表示每种面额钞票的流向、张数信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：240
    virtual int CashRollback(
    VALIDATION_MODE iValidateMode,
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
    );

    //功能1：指定面额挖钞 pArrDispDeno不为NULL使用
    //功能2：指定钞箱挖钞 pArrDispRoom不为NULL使用，冲突时优先使用该功能
    //输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
    //输入：btProhibitedBox: 表示禁止出钞的钱箱。1字节，0位~4位分别表示钱箱1~6,其余位值恒为0
    //      位值为1表示禁止从对应的钱箱存钞
    //输入：pArrDispDeno:  结构数组,表示每种面额的钞票出多少张,数组大小为10 NULL不使用该值
    //输入：pArrDispRoom:  结构数组,表示指定钞箱钞票出多少张,数组大小为10   NULL不使用该值
    //输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为TRUE时有效，否则忽略
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示挖钞时各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
    //输出：pNumDispensedForPerCass: 表示挖钞时各钞箱挖钞张数，钞箱1~6的各钞箱挖钞数,整型数组,数组大小为6
    //输出：stStackeNotesDenoInf: 表示每种面额钞票的流向、张数信息
    //输出：stStackeRejectNotesDenoSourInfo: 表示回收钞的面额的来源信息流向、张数
    //输出：stStackeRejectNotesDenoBVInfo:   表示回收钞的面额的通过BV的流向、张数信息
    //输出：stStackeRejectNotesDenoDestInfo: 表示回收钞的面额的目的信息了流向、张数
    //输出：stDispMissfeedRoom: 表示发生挖钞失败的钱箱以及原因
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：180
    virtual int Dispense(
    VALIDATION_MODE iValidateMode,
    char btProhibitedBox,
    const ST_DISP_DENO pArrDispDeno[MAX_DISP_DENO_NUM],
    const ST_DISP_ROOM pArrDispRoom[MAX_DISP_ROOM_NUM],
    const ST_BV_DEPENDENT_MODE stBVDependentMode,
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    int pNumDispensedForPerCass[MAX_CASSETTE_NUM],
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoSourInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoBVInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoDestInfo,
    ST_DISP_MISSFEED_ROOM &stDispMissfeedRoom,
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
    );

    //功能：回收挖钞后残留在ESC中的钞票，并将钞票送至拒钞箱
    //输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
    //输入：btProhibitedBox: 表示禁止出钞的钱箱。1字节，0位~4位分别表示钱箱1~6,其余位值恒为0
    //      位值为1表示禁止从对应的钱箱存钞
    //输入：iDestinationReject: 指定回收位置传入DESTINATION_REJCET_DEFAULT则使用SET_UNIT_INFO中设置的值
    //输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为TRUE时有效，否则忽略
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示回收时回收至各钞箱的钞票张数,钞箱1~6的各钞箱钞票数,整型数组,数组大小为6
    //输出：stStackeNotesDenoInf: 表示每种面额钞票的流向、张数信息
    //输出：stStackeUnfitNotesDenoInfo:   表示不适合流通的钞票面额、流向、张数信息
    //输出：stStackeRejectNotesDenoInfo:  表示回收钞的面额的目的信息了流向、张数
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：180
    virtual int RetractESCForDispenseRejcet(
    VALIDATION_MODE iValidateMode,
    char btProhibitedBox,
    DESTINATION_REJCET iDestinationReject,
    const ST_BV_DEPENDENT_MODE stBVDependentMode,
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeUnfitNotesDenoInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoInfo,
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
    );

    //功能：回收残留在ESC中的钞票，并将钞票送至存钞箱/拒钞箱/回收箱
    //输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
    //输入：btProhibitedBox: 表示禁止出钞的钱箱。1字节，0位~4位分别表示钱箱1~6,其余位值恒为0
    //      位值为1表示禁止从对应的钱箱存钞
    //输入：iDestinationReject: 指定回收位置传入DESTINATION_REJCET_DEFAULT则使用SET_UNIT_INFO中设置的值
    //输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为TRUE时有效，否则忽略
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示回收时回收至各钞箱的钞票张数,钞箱1~6的各钞箱钞票数,整型数组,数组大小为6
    //输出：stStackeNotesDenoInf: 表示每种面额钞票的流向、张数信息
    //输出：stStackeUnfitNotesDenoInfo:   表示不适合流通的钞票面额、流向、张数信息
    //输出：stStackeRejectNotesDenoInfo:  表示回收钞的面额的目的信息了流向、张数
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：180
    virtual int RetractESC(
    VALIDATION_MODE iValidateMode,
    char btProhibitedBox,
    DESTINATION_REJCET iDestinationReject,
    const ST_BV_DEPENDENT_MODE stBVDependentMode,
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeUnfitNotesDenoInfo,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoInfo,
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
    );

    //功能：关闭Shutter门
    //输入：bForcible:是否TRUE强制关闭SHUTTER门，尝试关闭10次
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int CloseShutter(
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],
    BOOL bForcible = FALSE
    );

    //功能：打开Shutter门
    //输入：无
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int OpenShutter(
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
    );

    //功能：开始下载固件
    //输入：无
    //输出：无
    //返回：0: 成功，非0: 失败
    //说明：进入下载模式成功后，HCM不能响应其他任何命令
    //说明：执行命令的最大时间(秒)：210
    virtual int ProgramDownloadStart();

    //功能：下载固件数据
    //输入：usControlID: 文件ID
    //输入：iPDLBlockType: 固件文件块类型
    //输入：uWritingAddress: 下载地址
    //输入：uSentDataLen: 已下载过的数据长度
    //输入：lpData: 下载的数据
    //输入：usDataLen: 下载数据长度
    //返回：0 成功，>0 警告，<0 失败
    //说明：执行命令的最大时间(秒)：210
    virtual int ProgramDownloadSendData(
    USHORT usControlID,
    PDL_BLOCK_TYPE iPDLBlockType,
    UINT uWritingAddress,
    UINT uSentDataLen,
    const char *lpData,
    USHORT usDataLen);

    //功能：结束下载固件
    //输入：无
    //输出：无
    //返回：0 成功，>0 警告，<0 失败
    //说明：执行命令的最大时间(秒)：210
    virtual int ProgramDownloadEnd();

    //功能：断开与ATM PC端口通讯连接，并重启模块到上电时的初始状态
    //输入：无
    //输出：无
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：210
    virtual int Reboot();

    //功能：维护检测、诊断机芯(暂不实现)
    //      使用该命令前需要进行一次复位，完成检测后需要调用复位恢复正常操作
    //输入：
    //输出：
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：180
    //virtual int Test();

    //功能：写数据到用户内存区域
    //输入：iUserMemoryTaget: 写入的内存区域,不可选择USER_MEMORY_TARGET_ALLCASS
    //输入：szUserMemoryData: 写入的数据，最大写入128字节
    //输入：usDataLen: 数据长度
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int UserMemoryWrite(
    USER_MEMORY_TARGET iUserMemoryTaget,
    const char szUserMemoryData[MAX_USER_MEMORY_DATA_LENGTH],
    USHORT usDataLen);

    //功能：读取用户内存的数据
    //输入：iUserMemoryTaget: 读取的内存区域
    //输出：stUserMemoryData: 用户数据信息结构
    //      USER_MEMORY_TARGET_ALLCASS同时读取5个钱箱内存数据;可读取单个数据，若返回读取内存区域USER_MEMORY_TARGET_RESERVED 则为无效数据
    //输出：usDataArrayCount: 读取后的stUserMemoryData数组有效数据个数, 取值范围0-5
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int UserMemoryRead(
    USER_MEMORY_TARGET iUserMemoryTaget,
    ST_USER_MEMORY_READ stUserMemoryData[MAX_USER_MEMORY_DATA_ARRAY_NUM],
    USHORT &usDataArrayCount
    );

    //功能：查询BV模块信息
    //输入：无
    //输出：stBVWarningInfo: BV警告信息数组
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int QueryBVStatusSense(
    ST_BV_WARNING_INFO stBVWarningInfo[MAX_BV_WARNING_INFO_NUM]
    );

    //功能：获取冠字码相关信息
    //输入：dwIndex: 获取冠字码序号，从0开始
    //输出：lpNoteSerialInfo: 保存冠字码信息的结构指针
    //返回：0：成功，其他：失败
    //说明：获取冠字码数据前需要调用BVCommStart函数，全部获取结束后调用BVCommEnd函数，结束与BV通信
    virtual long GetNoteSeiralInfo(DWORD dwIndex, LPSNOTESERIALINFO lpNoteSerialInfo);

    //功能：获取冠字码全幅图像相关信息
    //输入：dwIndex: 获取冠字码序号，从0开始
    //输出：lpNoteSerialInfoFullImage: 保存冠字码信息的结构指针
    //返回：0：成功，其他：失败
    //说明：获取冠字码数据前需要调用BVCommStart函数，全部获取结束后调用BVCommEnd函数，结束与BV通信
    virtual long GetNoteSeiralInfoFullImage(DWORD dwIndex, LPSNOTESERIALINFOFULLIMAGE lpNoteSerialInfoFullImage);

    //功能：获取钞票流向面额信息，包括到了何处、面值、版次等
    //输入：ucMediaInfoNum: 查询的钞票信息号，取值为1、2、3、4，每次调用自增1，第一次调用值是1
    //输出：usNumNotes: 本次返回的钞票信息数，一次最多返回512个钞票信息
    //输出：usNumTotalNotes: 产生钞票信息总数
    //输出：aryMediaInfo: 返回的钞票信息保存位置数组
    //返回：0:  成功 后面没有数据了
    //      >0: 返回插入aryMediaInfo中的对象个数.如果个数等于512表示后面还有数据，请继续调用本函数获取其余的信息
    //      <0: 命令执行失败
    //说明：执行命令的最大时间(秒)：20
    virtual long GetMediaInformation(
    BYTE byMediaInfoType,                               //30-00-00-00(FS#0022)
    char ucMediaInfoNum,
    USHORT  &usNumNotes,
    USHORT  &usNumTotalNotes,
    ST_MEDIA_INFORMATION_INFO arryMediaInfo[MAX_MEDIA_INFO_NUM]
    );

    //功能：PC与ZERO BV通信开始，通知HCM不访问BV，获取冠字码信息前调用
    //输入：无
    //输出：无
    //0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int BVCommStart();

    //功能：PC与ZERO BV通信结束，获取冠字码信息结束后调用
    //输入：无
    //输出：无
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int BVCommEnd();

    //功能：准备进入一下笔交易
    //      在取款、存款交易结束后调用该命令
    //      HCM2检测所有单元是否在原始位置并且无钞票残留
    //      HCM2关掉传感器以及设备，切换到节能模式
    //输入：无
    //0 成功，>0 警告，<0 失败；见返回码定义
    //说明：
    virtual int PreNextTransaction();

    //功能：开始存取款交易
    //      在取款、存款交易开始前调用该命令
    //      HCM2打开传感器以及设备，从节能模式中恢复
    //输入：无
    //0 成功，>0 警告，<0 失败；见返回码定义
    //说明：
    virtual int StartTransaction();

    //功能：获得设备状态信息
    //输入：无
    //输出：stDevStatusInfo
    //返回：无
    virtual void GetDevStatusInfo(ST_DEV_STATUS_INFO &stDevStatusInfo);

    // 功能：获取错误返回码
    // 输入：无
    // 输出：无
    // 返回：8字节常量字符串
    // 说明：若指令正常结束则返回空字串("")
    //       若异常结束则返回8字节的错误码字符串，如："05130A03"
    //       若警告结束则返回8字节警告码字符串，  如："81xxxxxx"
    virtual void GetLastErrDetail(ST_ERR_DETAIL &stErrDetail);

    // 功能：将响应的错误或警告码转换为对应的错误描述
    // 输入：arycErrCode: 响应的错误代号,请传入GetLastErrCode()返回的字符串
    // 输出：无
    // 返回：成功：返回arycErrCode相关的错误描述，
    //       失败：如arycErrCode错误代号未知则返回空串：""；如参数为NULL返回NULL
    virtual const char *GetErrDesc(const char *arycErrCode);

    //功能：获取所有感应器的亮灭状态
    //输入：iLen: pszLightData所分配的内存空间，该值需大于58
    //输出：pszLightData: 所有感应器的亮灭状态数据，58字节，每1 bit表示一个感应器
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int ReadAllSensorLight(char *pszLightData, int &iLen);

    //功能：调整BV验钞级别
    //输入：ucVerificationLevel : 0~255表示 检验级别，数值越高越宽松
    //输出：无
    //返回：0 成功，>0 警告，<0 失败
    virtual int SetBVVerificationLevel(char ucVerificationLevel) ;

    //功能：对变造币进行拒钞处理
    //输入：
    //输出：
    //返回：0 成功，>0 警告，<0 失败
    virtual int  SetRejectCounterfeitNote();

    //功能：设置不适合流通人民币验钞处理级别
    //输入：CmdType ：设置后即将执行的操作动作; bSevereModel: 是否使用更严格的模式
    //输出：
    //返回：0 成功，>0 警告，<0 失败
    virtual int  SetUnfitNoteVerifyLevel(eCMDType CmdType, BOOL bSevereModel);

    //功能：出钞测试
    virtual int  TestCashUnits(VALIDATION_MODE iValidateMode,
                               BYTE byRoomExclusion[2][5],
                               BYTE byRetryCnt,
                               TESTCASHUNITSOUTPUT &stTestCashUnitsOutput);     //30-00-00-00(FS#0007)
    //功能：自动精查
    virtual int SelfCount(VALIDATION_MODE iValidateMode,
                          bool  bEnableRoom[5][2],
                          ULONG ulNumSelfCountPerCass[MAX_CASSETTE_NUM],
                          ULONG ulNumFeedPerCass[MAX_CASSETTE_NUM],
                          ST_TOTAL_STACKE_NOTES_DENO_INFO &stRejectDenoDstInfo,
                          ST_TOTAL_STACKE_NOTES_DENO_INFO &stRejectDenoSrcInfo,
                          const ST_BV_DEPENDENT_MODE stBVDependentMode,
                          bool  bContinueAfterBVDataFull = false); //30-00-00-00(FS#0022)

protected:
    //功能：初始化成员变量m_mPacketIDLength，map不为空时将PacketID与包长填充
    //输入：无
    //输出：无
    //返回：无
    //说明：无
    void InitMapPacketID();

    //功能：校验返回数据的合法性，分析提取每个包数据至map中
    //      校验Resp长度与所有packet数据的总和是否相等,每个packet长度是否与协议相等
    //输入：pszRep：返回信息缓冲区首地址
    //输入：iMinPacketCount：解析出的最小数据包个数，取值为协议中固定包个数，为0则不校验解析后的数据包个数
    //输入：ThisModule：调用函数名称
    //输出：mapPacket：存储响应数据中的每个Packet包信息
    //返回：TRUE:数据校验合格， FALSE:数据校验异常
    //说明：无
    BOOL CheckRespEachPacket(
    const char *szResp,
    int &iMinPacketCount,
    PACKETMAP &mapPacket,
    const char *ThisModule);

    //功能：在数据包Map中查找指定ID包数据起始位置
    //输入：mapPacket：存储响应数据中的每个Packet包信息
    //输入：usPacketID：Packet包数据ID
    //输入：ThisModule：调用函数名称
    //输入：bRecordLog：未找到指定ID包时是否记录日志
    //返回：>0 成功，<0 失败；见返回码定义
    //说明：无
    int FindMapPacketDataPosforID(
    const PACKETMAP &mapPacket,
    USHORT usPacketID,
    const char *ThisModule,
    BOOL bRecordLog = TRUE);

    //功能：校验提取RESP数据包中的响应码
    //输入：pszRep：返回信息缓冲区首地址
    //输入：mapPacket：Packet包信息数据
    //输入：ThisModule：调用函数名称
    //输出：无
    //返回：命令响应码
    //说明：无
    bool ParseRESP(
    const char *szResp,
    const PACKETMAP &mapPacket,
    USHORT &usRespCode,
    const char *ThisModule);

    //功能：将命令执行完成后返回信息中机芯错误信息解析出来保存在成员变量
    //输入：szResp：返回信息缓冲区首地址
    //输入：mapPacket：Packet包信息数据
    //输出：stErrDetail：解析后的错误信息
    //返回：0 成功，<0 失败；见返回码定义
    //说明：无
    int SaveErrDetail(
    const char *szResp,
    const PACKETMAP &mapPacket,
    const char *ThisModule);

    //功能：将命令执行完成后返回信息中机芯状态信息解析出来保存在成员变量
    //输入：szResp：返回信息缓冲区首地址
    //输入：mapPacket：Packet包信息数据
    //输出：stDevStatusInfo：解析后的状态信息
    //返回：0 成功，<0 失败；见返回码定义
    //说明：无
    int SaveStatusInfo(
    const char *szResp,
    const PACKETMAP &mapPacket,
    const char *ThisModule);

    //功能：分析暂存钞票数据包信息
    //输入：szResp：返回信息缓冲区首地址
    //输入：mapPacket：Packet包信息数据
    //输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
    //输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
    //输出：pNumStackedToPerCass: 表示各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
    //返回：0 成功，<0 失败；见返回码定义
    //说明：无
    int ParseRespStackedNotes(
    const char *szResp,
    const PACKETMAP &mapPacket,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    const char *ThisModule);

    //功能：分析点钞数据包信息
    //输入：szResp：返回信息缓冲区首地址
    //输入：mapPacket：Packet包信息数据
    //输出：iNumCSFed:  CS点钞数
    //输出：iNumESCFed: ESC点钞数
    //输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
    //返回：0 成功，<0 失败；见返回码定义
    //说明：无
    int ParseRespFedNotes(
    const char *szResp,
    const PACKETMAP &mapPacket,
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    const char *ThisModule);

    //功能：分析每种面额钞票存储张数数据包信息
    //输入：szResp：返回信息缓冲区首地址
    //输入：mapPacket：Packet包信息数据
    //输入：iStackeNotesDenoInfoType：处理数据包类型
    //输出：stStackeNotesDenoInfo：每种面额钞票的流向、张数信息
    //返回：0 成功，<0 失败；见返回码定义
    //说明：无
    int ParseRespNumStackedNotesPerDeno(
    const char *szResp,
    const PACKETMAP &mapPacket,
    const NOTES_DENO_INFO_TYPE iNotesDenoInfoType,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
    const char *ThisModule);

    //功能：检查SetUnitInfo()参数中的钱箱类型与RB/AB 操作是否合法
    //输入：stArrCassType:同SetUnitInfo()参数stCassType
    //返回：合法返回 0；不合法返回 ERR_UR_PARAM
    //说明：无
    bool CheckCSSTpAndRBABOptn(const ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM]);

    //功能：将HCM控制设置参数转化为控制数组，方便数据包处理
    //输入：stOperationalInfo: HCM控制设置
    //输出：bArrayOPInfo HCM控制设置数组
    //返回：无
    //说明：无
    void OPInfoToArray(const ST_OPERATIONAL_INFO stOperationalInfo, char bArrayOPInfo[4]);

    //功能：将控制数组转化HCM控制设置参数，方便数据包处理
    //输入：bArrayOPInfo HCM控制设置数组
    //输出：stOperationalInfo: HCM控制设置
    //返回：无
    //说明：无
    void OPInfoFromArray(const char bArrayOPInfo[4], ST_OPERATIONAL_INFO &stOperationalInfo);

    //功能：执行SendData和RecvData()
    //参数：pParam，指向ThreadParm结构体的指针，具体成员
    unsigned int Run(LPVOID pParam);

    // 功能：执行pszCmd命令，响应数据返回给pszResp
    // 参数：
    // 输入：
    //      dwReqRecvLen：期望接收到的响应数据长度
    //      pszCmd：执行要发送的命令空间的指针
    //      iCmdLen：pszCmd指向命令的有效长度
    //      iTimeout：超时时间
    //      pszFuncName：调用ExecuteCmd()的方法名（用于记录日记）
    //      eConnectType: 连接类型
    //      bPDL：是否为下载固件
    // 输出：
    //      pszResp：执行响应数据空间的指针
    //      dwRespLen：实际的响应数据长度
    // 返回：
    //  ERR_CDMCIM_NOLOADDLL：没有加载通讯库，发生此错误请先调用OpenUSBConn()
    //  ERR_CDMCIM_SUCCESS：成功
    //  ERR_CDMCIM_COMM：实际响应数据长度 < 期望接收到数据长度
    //  其他值为： SendData()或RecvData()出错的返回值
    int ExecuteCmd(
    const char *pszSendData,
    USHORT dwSendLen,
    char *pszRecvData,
    USHORT &dwRecvedLenInOut,
    USHORT dwReqRecvLen,
    USHORT dwRecvTimeout,
    const char *lpFunName,
    CONNECT_TYPE eConnectType = CONNECT_TYPE_UR,
    bool bPDL = false);

    BOOL PercolateDrvErrCode(const ThreadParm &parm);
    //根据错误码判断是否需要重启HCM或系统
    BOOL IsNeetRebootByErrCode(int iRet);
    //根据相关设置重启系统
    BOOL RebootSys();

private:
    void ReadConfig();                  //30-00-00-00(FS#0016)

private:
    //long GetLocalSystemTime(SYSTEMTIME &stSystemTime);
    friend struct CStatusListenThread;


    ST_ERR_DETAIL m_stErrDetail;
    ST_DEV_STATUS_INFO m_stDevStatusInfo;
    std::recursive_mutex m_MutexStatus;
    std::recursive_mutex m_MutexAction;
    CVHUSBDrive *m_pUSBDrv;
    map<USHORT, USHORT> m_mPacketIDLength; //存储<PacketID, Length> 用于校验返回数据有效性。Length为0表示不需要校验该包

    bool m_bUseFuncLib;                     //30-00-00-00(FS#0016)
};
