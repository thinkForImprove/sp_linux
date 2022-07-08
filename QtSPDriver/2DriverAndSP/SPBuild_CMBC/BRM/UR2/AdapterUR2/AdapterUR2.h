#ifndef ADAPTERUR2_H
#define ADAPTERUR2_H


#include "IBRMAdapter.h"
#include "IURDevice.h"
#include "ILogWrite.h"
#include "ErrCodeTranslate.h"
#include "ISaveBRMCustomSNInfo.h"
#include "AutoQtHelpClass.h"
#include "GenerateSN.h"
#include "INIFileReader.h"
#include "ICryptyData.h"
#include "QtDLLLoader.h"
#include <assert.h>
#include <QSettings>
#include <QString>
#include "IAllDevPort.h"        //test#11
#include "QtTypeInclude.h"      //test#11

using namespace std;
#include <string.h>

#define ZeroMemory(a, l)    memset(a, 0, l)

#define ADP_MAX_DENOMINATION_NUM (128)  //日立定义的最大面值个数
#define MAX_ERRCODE_LENGTH       (14)   //最大错误代码长度

//取回收张数扩展信息的键值对(iter的左值、右值分别给strKey、strValue)，完成后iter指向下一个
#define GET_KEYANDVALUE_STRING_BY_ITER(strKey, strValue, iter) \
    strKey = iter->first;\
    assert(strKey.length() > 0);\
    strValue = iter->second;\
    iter++;\


//定义设备需要配置的选项
typedef struct _DevConfigInfo
{
    //出钞验钞模式
    VALIDATION_MODE CashOutNoteType;
    //进钞验钞模式
    VALIDATION_MODE CashInNoteType;
    //是否保存序列号信息
    BOOL bKeepSNInfo;
    //存款时是否检测钞票在CS，TRUE: 检查CS口，如果有钞票返回错误， FALSE: 正常存钞
    BOOL bCheckCSWhenStoreMoney;
    //是否支持安全门状态
    BOOL bSafeDoorStatusSupport;
    //默认钞票回收位置, 1-TS，2-AB，3-URJB(开启URJB箱，如没则回收到AB)
    USHORT usDefaultRetractArea;
    //是否按面额出钞方式出钞
    BOOL DispenseByDeno;
    BOOL SetBVErrWhenConnectAbnormal;
    //是否需要降低验钞废钞率
    SET_VERIFICATION_LEVEL stSetVerificationLevel;
    ST_BV_VERIFICATION_LEVEL stCashCountLevel;
    ST_BV_VERIFICATION_LEVEL stStoreMoneyLevel;
    ST_BV_VERIFICATION_LEVEL stDispenseLevel;
    //BV模式
    ST_BV_DEPENDENT_MODE     stBVDependentMode;

    BOOL bRejectCounterfeitNote;

    //对不适合流通钞票的处理方式，0: 不处理，1：标准方式， 2：更严格验钞方式
    USHORT usUnfitNotesVerifyMode;

    //是否处理99版人民币
    BOOL bGenerateCNY1999;

    DWORD dwVerLevDispense;
    DWORD dwVerLevCashCount;
    DWORD dwVerLevStoreCash;

    BOOL  bCloseShutterForce;
    BOOL  bRB5ToEscSupp;                            //test#24

    //硬件是否支持URJB
    BOOL  bURJBSupp;
    string m_strCashInShutterCom;   //test#11
    std::map<BYTE, BYTE> mapNoteIDGroups;                   //30-00-00-00(FS#0018)
} DEVCONFIGINFO, *LPDEVCONFIGINFO;

//定义设置钞票ID相关信息
typedef struct _adp_notetype
{
    WFSCIMNOTETYPE  CIMNoteType;
    BYTE            usDenoCode;     //标准面额代码 0x01~0x1F代表A~AE
} ADP_NOTETYPE, *LPADP_NOTETYPE;

//定义设置的钱箱相关信息
typedef struct _adp_cass_info
{
    BOOL                bActive;           // 钞箱是否被启用
    ADP_CASSETTE_TYPE   iCassType;         // 钱箱类型
    CHAR                CurrencyID[3];     // 钱箱对应币种
    DWORD               ulValue;           // 面额代码
    USHORT              usDenoCode;        // 币种代码
    DWORD               ulCount;           // 钞票数量
} ADP_CASS_INFO, *LPADP_CASS_INFO;

typedef struct _ProgramFileInfo
{
    USHORT ucCTLID;
    UINT uLoadAdress;
    UINT uSUM;
    UINT uMaxSize;
    char  szFilePath[MAX_PATH];
    char  szFWType[32];
    char  szFWVersion[32];
    _ProgramFileInfo()
    {
        memset(szFilePath, 0, sizeof(szFilePath));
        memset(szFWType, 0, sizeof(szFWType));
        memset(szFWVersion, 0, sizeof(szFWVersion));
    }
} ProgramFileInfo;

typedef struct _FWVersionInfo
{
    // USHORT usID;
    char sProgramName[17];
    char sVersion[9];
    char sDate[9];
    UINT uSUM;
    USHORT usCTLID;
    _FWVersionInfo() { memset(this, 0, sizeof(_FWVersionInfo)); }
} FWVersionInfo;

enum FWDOWNSTATUS
   {
       FWDOWNSTATUS_UNSET = 0,
       FWDOWNSTATUS_NOTNEEDDL = 1,
       FWDOWNSTATUS_NEEDDL = 2,
       FWDOWNSTATUS_DLING = 3,
       FWDOWNSTATUS_DLED = 4,
   };



typedef map<USHORT, FWVersionInfo> TYPEMAPFWUR;
typedef map<USHORT, ProgramFileInfo> TYPEMAPFWDATA;
//定义获取序列号相关信息
typedef struct _adp_note_serial_info
{
    USHORT usNoteID;
    char arySerialNum[128];
} ADP_NOTE_SERIAL_INFO, *LPADP_NOTE_SERIAL_INFO;

class ErrCodeTranslate;

//适配层接口实现类
class CUR2Adapter: public IBRMAdapter, public CLogManage
{
public:
    CUR2Adapter();
    virtual ~CUR2Adapter();

    //实现IBRMAdapter接口
public:
    virtual void Release()
    {
        //delete this;
    }

    //功能：打开机芯通信连路,建立PC与机芯间的通信
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：串口号、USB端口号等参数由适配层自己决定，不由SP传入
    virtual long Open();

    //功能：关闭机芯
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long Close();

    //功能：设置等待完成的接口
    //输入：pWait：回调接口
    //输出：无
    //返回：无
    //说明：
    // virtual void SetWaitCallback (IWaitComplete *pWait);

    //功能：设置状态改变侦听接口
    // 参数：
    //  pListener：【输入】，回调接口指针，如果NULL，取消侦听
    //返回值：TRUE，设置成功，支持状态侦听；否则，不支持状态侦听
    //virtual BOOL SetStatusChangeListener(IStatusChangeListener *pListener);

    //功能：取设备状态
    //输入：Status：设备状态
    //输出：无
    //返回：无
    //说明：
    virtual long GetStatus(ADPSTATUSINFOR &Status);

    //功能：更新状态，该函数会访问设备，仅在不调用其他动作函数时更新状态
    //输入：无
    //输出：无
    //返回：无
    //说明：
    virtual long UpdateStatus();

    //功能：获取出钞机芯能力
    //输入：无
    //输出：lpCaps：出钞机芯能力结构指针,定义同xfs的CDM部分。
    //返回：XFS通用错误码
    //说明：
    virtual long GetCDMCapabilities(LPADPCDMCAPS lpCaps);

    //功能：获取进钞机芯能力
    //输入：无
    //输出：lpCaps：进钞机芯能力结构指针,定义同xfs的CIM部分。
    //返回：XFS通用错误码
    //说明：
    virtual long GetCIMCapabilities(LPADPCIMCAPS lpCaps);

    //功能：设置BV支持的钞票类型
    //输入：lpList：整机支持的钞票列表,同XFS定义
    //输出：无
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long SetSupportNoteTypes(const LPWFSCIMNOTETYPELIST lpList);

    //功能：设置钞箱是否启用
    //输入：bEnable：大小为5的数组,指示各钞箱是否启用,出钞、存款交易前调用。
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long EnableCashUnit(const BOOL bEnable[ADP_MAX_CU_SIZE]);

    //功能：指定各出钞箱出钞数,进行点钞
    //输入：aryulItems,各钞箱要挖钞张数
    //输出：aryulDispenseCount：每个钞箱点出的钞数,不含废钞数；
    //输出：aryulRejects：对于取款箱,表示废钞数；对于回收箱,表示回收数,理论上,所有废钞箱、回收箱的Reject数总和应等于出钞箱、循环箱的Reject数之和。
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //输入：bSNAddingMode：冠子号信息是否累计模式，TRUE---累计
    //说明：无。
    virtual long Dispense(const ULONG aryulItems[ADP_MAX_CU_SIZE],
                          ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                          ULONG aryulRejects[ADP_MAX_CU_SIZE],
                          ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE],
                          BOOL bSNAddingMode);

    //功能：开始进入存款模式
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long StartCashIn();

    //功能：验钞
    //输入：无
    //输出：pResult：各钞票种类张数信息,单条数据的格式为"[钞票ID:char6][张数:char6]"样式.
    //               例：ID为1的钞票16张,ID为2的钞票20张："000001000016000002000020",pResult指向内存由调用者分配管理.
    //输出：ulRejCount:废钞张数
    //输出：iErrReject: 拒钞发生原因
    //返回：XFS通用错误码及CIM错误码
    //说明：
//test#13    virtual long ValidateAndCounting(char pResult[256], ULONG &ulRejCount, ADP_ERR_REJECT &iErrReject);
    virtual long ValidateAndCounting(char pResult[256], ULONG &ulRejCount, ADP_ERR_REJECT &iErrReject, ADP_CUERROR arywCUError[],WORD wCashInCnt);         //test#13

    //功能：将进钞钞票存入钞箱
    //输入：无
    //输出  ppNoteCounts：存放各钞箱存入的各种钞票张数,每个数组成员返回内容为各通道钞箱的各种钞票张数列表。
    //      单条格式为："[钞票ID:char6][钞票张数:char6]",
    //      如："000001000030000002000013"代表钞票ID为1的钞票有30张,ID为2的钞票有13张;
    //：     arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long StoreCash(char ppNoteCounts[ADP_MAX_CU_SIZE][256], ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]);

    //功能：退回存款钞票 将存款点钞完的钞票退出到出钞口
    //输入：无
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CIM错误码
    //说明：
    virtual long RollBack(ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]);

    //功能：将Dispense后存在暂存区的钞票送到门口
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long Present();

    //功能：回收 将Present后在门口的钞票,Dispense后存在暂存区的钞票回收到回收箱
    //输入：usRetraceArea：钞票回收的目的位置
    //      usIndex:当usRetraceArea指定为某个钞箱时有效,表示钞箱索引,从1开始,依次增1
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CDM错误码
    //说明：无
    virtual long Retract(ADP_RETRACTEARE usRetractArea, USHORT &usIndex, ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) ;

    //功能：回收拒钞 将Dispense后存在暂存区的钞票回收到废钞箱
    //输入：usIndex,表示钞箱索引,从1开始,依次增1
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long Reject(USHORT usIndex, ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]) ;

    //功能：复位机芯,将机芯置恢复到已知正常状态
    //输入：usRetraceArea：钞票回收的目的位置
    //      usIndex:当usRetraceArea指定为某个钞箱时有效,表示钞箱索引,从1开始,依次增1
    //输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //      bMediaDetected：复位过程中是否检测到介质
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long Reset(ADP_RETRACTEARE &usRetraceArea, USHORT &usIndex,
                       BOOL &bMediaDetected, ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]);

    //功能：取钞票ID列表,即每次有产生读取钞票序列号动作后的序列号
    //输入：无
    //输出：无
    //返回：返回序列号条目数量
    //说明：
    virtual ULONG GetNoteSeiralNumbersCount()
    {
        return 0 /*m_vNotesSerialInfo.size()*/;
    }

    //功能：取钞票每次有产生读取钞票序列号动作后的序列号
    //输入：无
    //输出：dwIndex：第几个条目,从0开始,至总数量-1；
    //      usNoteID：币种代码；
    //      arycSerialNumber[128]：序列号。
    //返回：XFS通用错误码
    //说明：
    virtual long GetNoteSeiralNumbers(ULONG dwIndex, USHORT &usNoteID, char arycSerialNumber[128]);

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
    //说明：设置完成后可能须要立即调用初始化复位方法
    virtual long SetCassetteInfo(WORD wHopperID, const char Currency[3], ADP_CASSETTE_TYPE eType,
                                 ULONG ulValue, WORD *lpusNoteIDs, ULONG ulCount);

    //功能：取钞箱信息
    //输入：wHopperID,钞箱序号,从1开始,依次增1
    //输出：
    //      eStatus:状态（取值定义与XFS相同）
    //      cCassID:物理钞箱ID
    //返回：XFS通用错误码及CIM错误码
    //说明：1)  由于上层没有用到长、宽、厚等信息,适配层可在下面自己做相关校验。
    virtual long GetCassetteInfo(WORD wHopperID, CASHUNIT_STATUS &eStatus, char cCassID[5]);

    //功能：测试钞箱。
    //输入：无
    //输出：aryulDispenseCount：测试钞箱时每个钞箱点出的钞数,不含废钞数；
    //      aryulRejects：对于取款箱,表示废钞数,对于回收箱,表示回收数;
    //      arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long TestCashUnits(ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                               ULONG aryulRejects[ADP_MAX_CU_SIZE], ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]);

    //功能：取固件版本。
    //输入：无
    //输出：cVersion,统计信息,长度包括0结尾不超过512个字符
    //返回：XFS通用错误码
    //说明：
    virtual long GetFirmwareVersion(char cVersion[512]);

    //功能：取最后错误码
    //输入：无
    //输出：无
    //返回：返回最后错误代码
    //说明：
    virtual const char *GetLastError()
    {
        return m_szLastErrCode;
    }

    //功能：获取附加状态
    //输入：pExtra：附加信息设置接口指针,调用该接口完成附加状态的设置
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long GetExtraInfor(IBRMExtra *pExtra);

    //功能：通知机芯进入交换状态,机芯可根据实际情况执行相应的预处理。
    //输入：arybExFlag：将被交换的钱箱。
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long StartExchange(const BOOL arybExFlag[ADP_MAX_CU_SIZE]);

    //功能：通知机芯退出交换状态,机芯可根据实际情况执行相应的收尾处理。
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：
    virtual long EndExchange();

    //功能：在执行复位机芯、Retract及Reject操作时,机芯可能会回收到钞票,通过此方法可获取回收到每个钞箱的钞数。
    //输入：无
    //输出：arywRetractCount 获取回收钞票张数，在执行可能产生回收的操作之后,得到回收的钞数信息。本次操作回收数
    //返回：0---获取成功；
    //      其它---获取失败
    //说明：
    virtual long GetRetractCount(ULONG arywRetractCount[ADP_MAX_CU_SIZE]);

    //功能：开门
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long OpenShutter(BOOL bInShutter = TRUE) ;

    //功能：关门
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：
    virtual long CloseShutter(BOOL bInShutter = TRUE);

    //功能：操作后是否需要复位
    //输入：无
    //输出：无
    //返回：TRUE: 需要复位 ；FALSE: 不需要复位
    //说明：
    virtual BOOL IsNeedReset()
    {
        return m_bIsNeedReset;
    }

    //功能:  判断在钞箱启用条件下，存取款是否可以进行
    //输入:  bCDM:是否是CDM操作。
    //       bCassEnable:钞箱启用条件。TRUE:该钞箱被启用; FALSE: 不被启用
    //返回：TRUE:  存取款可以进行；FALSE: 存取款不可以进行
    virtual BOOL IsOperatable(BOOL bCDM, const BOOL bCassEnable[ADP_MAX_CU_SIZE]);

    //获取回收钞票的ID:张数对信息。
    //输出：pResult：各钞票种类张数信息,单条数据的格式为"[钞票ID:char6][张数:char6]"样式.
    //               例：ID为1的钞票16张,ID为2的钞票20张："000001000016000002000020",pResult指向内存由调用者分配管理.
    //输出：ulRejCount:废钞张数
    //返回值：0：成功；其他：失败
    virtual long GetRetractNoteIdAndCountPairs(char arycResult[256], ULONG &ulRejCount);

    //功能：设置当前执行LFS命令标识ID
    //输入：LFS命令标识ID
    //输出：无
    //返回：无
    //说明：
    virtual void SetLFSCmdId(ADP_LFS_CMD_ID eLfsCmdId) override;           //test#8

    virtual BOOL IsConnected() {                            //40-00-00-00(FT#0015)
        return (m_bConnectNormal && m_bBVConnectNormal);    //40-00-00-00(FT#0015)
    }                                                       //40-00-00-00(FT#0015)

    BOOL IsNeedDownLoadFW();
    long DownLoadFW();
    void SetDownLoadFWFlag(BOOL bFlag);
    long UpdatePostionStatus();
    void  GetDevConfig(ADPDEVINFOR &devinfo);
    int  GetResultFlg();   //test#7
    void SetResultFlg();  //test#7
    int iTooManeyItemsHappenFlg;    //test#7

    //功能：手动清除错误/故障码　lfg
    //输入：无
    //输出：无
    //返回：无
    //说明：无
    void ClearError();

    //内部成员函数
private:
    //----------- 初始化类函数 ---------------
    //功能：打开机芯连接
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：内部方法
    long OpenUSBConnect(CONNECT_TYPE ct);

    //功能：关闭机芯连接
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：内部方法
    long CloseUSBConnect(CONNECT_TYPE ct);

    //功能：复位机芯
    //输入：无
    //输出：arywCUError： 钞箱错误原因
    //返回：XFS通用错误码
    //说明：内部方法
    long Init(ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]);

    //功能：初始化适配设置 读取相关配置文件等
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：内部方法
    long InitADPConfig();

    //功能：初始化进出钞能力
    //输入：ConfigFileOper：用来进行文件操作的对象
    //输出：无
    //返回：XFS通用错误码
    //说明：内部方法
    long InitCapabilities();

    //功能：根据适配层保存的信息转为机芯所需的配置，并初始化机芯设置
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：内部方法
    long InitSetting();

    //功能：初始化适配器状态
    //输入：无
    //输出：无
    //返回：无
    //说明：内部方法
    void InitADPStatus();

    //功能：设置机芯BV支持的所有币种类型 见文档 Denomination Code Table
    //输入：无
    //输出：无
    //返回：无
    //说明：内部方法
    void InitBVSupportNotesType();

    //----------- 动作类内部成员函数 --------------------------

    //功能：出钞
    //输入：aryCassCount：每个钞箱须挖钞张数
    //      bSNAddingMode：冠子号信息是否累计
    //输出：aryulDispenseCount：实际挖钞张数
    //      aryulRejects：钞箱实际拒钞数
    //      arywCUError:  钞箱出错原因
    //      aryulRejBillPerDenAll:每种面额拒钞数
    //返回：XFS通用错误码及CDM错误码
    //说明：内部方法
    long DispenseFromAllCass(const ULONG aryCassCount[ADP_MAX_CU_SIZE],
                             ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                             ULONG aryulRejects[ADP_MAX_CU_SIZE],
                             ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE],
                             ULONG aryulRejBillPerDenAll[ADP_MAX_DENOMINATION_NUM],
                             BOOL bSNAddingMode);

    //功能：回收Dispense后在出钞口的钞票
    //输入： aryulDispenseCount：每个钞箱的出钞张数
    //输出：aryulRejects:每个钞箱的拒钞张数
    //       iRejBillPerDenAll：每种面额的拒钞张数
    //       arywCUError: 钞箱错误类型
    //返回：XFS通用错误码及CDM错误码
    //说明：内部方法
    long RetractNoteInCSAfterDispense(ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                                      ULONG aryulRejects[ADP_MAX_CU_SIZE],
                                      ULONG iRejBillPerDenAll[ADP_MAX_CU_SIZE],
                                      ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]);

    //功能：回收钞票
    //输入：usRetraceArea：钞票回收的目的位置 1:回收箱，2：通道 3：暂存区，4：废钞箱 5：循环箱 6：出钞口 仅支持1，3，4，6
    //      usIndex: 物理钞箱的序号从1开始
    //输出：arywCuError: 钞箱错误原因
    //返回：XFS通用错误码及CDM错误码
    //说明：内部函数，执行CDM和CIM的回收指令
    long RetractNotes(USHORT usRetraceArea, USHORT usIndex,
                      ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]);


    //功能：分析SP传入的回收位置，并回收存在的钞票
    //输入：usRetractArea：回收目的位置
    //      usRetractIndex: 钞箱序号
    //      arywCUError: 钞箱错误原因
    //输出：无
    //返回：XFS通用错误码
    //说明：内部函数
    long RetractNotesToDesArea(ADP_RETRACTEARE &usRetractArea,
                               USHORT &usRetractIndex,
                               ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]);

    //功能：打开钞门
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：内部函数
 //test#11   long OpenDeviceShutter();

    //功能：打开钞门
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CDM错误码或CIM错误码
    //说明：内部函数
    long OpenDeviceShutter(BOOL bInShutter=FALSE);      //test#11

    //功能：关闭钞门
    //输入：无
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：内部函数
//test#11    long CloseDeviceShutter();
    long CloseDeviceShutter(BOOL bInShutter=FALSE); //test#11

    //功能：取固件版本。
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：内部函数 获取版本信息并写入成员数据
    long QueryFWVersion();

    long SetVerLevel();
    long GetBanknoteInfo();
    long SetDenoCode();
    long GetCassetteInfo();
    long SetUnitInfo();
    long GetUnitInfo();
    long StartBVCommunication();
    long EndBVCommunication();
    //功能：查询机芯状态信息
    //输入：无
    //输出：无
    //返回：XFS通用错误码
    //说明：内部方法
    long QueryDevInfo();

    long RetractNotesToAB(CASSETTE_NUMBER RetractCassNum, ADP_CUERROR arywCuError[ADP_MAX_CU_SIZE]);
    long RetractNotesToESC(ADP_CUERROR arywCuError[ADP_MAX_CU_SIZE], int iRequiredRetractArea);         //test#8
private:

    //功能：从设备查询序列号信息
    //输入：enuSSO: 是什么样的操作调用本函数
    //输出：无
    //返回：TRUE:获取成功，FALSE:获取失败
    //说明：内部方法
    BOOL QueryNoteSerialInfo(CThreadGenerateSN *pwork);

    BOOL QueryNoteMediaInfo(CThreadGenerateSN *pwork);
    //------------ 状态更新类函数 --------------

    //功能：根据设备状态更新适配层内部状态
    //输入：iRet: 机芯动作返回的返回值
    //输出：无
    //返回：无
    //说明：内部方法
    void UpdateADPDevStatus(int iRet, int iBVRet = 0);

    //功能：机芯动作后更新钞箱状态
    //输入：无
    //输出：无
    //返回：无
    //说明：内部方法
    void UpdateADPCassStatus(const ST_DEV_STATUS_INFO &stDevStatusInfo);

    //下载固件更新，
    //返回值：>0：不需要更新； =0：更新成功； <0：更新失败
    int DownloadFile();

    //-------------- 数据保存类函数-------------------

    //功能：验钞后保存拒钞数
    //输入： iNumInRJSD：拒钞数
    //输出：无
    //返回：XFS通用错误码及CIM错误码
    //说明：内部方法
    void SaveRejectBill(int iNumInRJSD);

    //功能：验钞后保存拒钞数
    //输入：pNumRejectedPerDenoALL：每种面额的拒钞数
    //输出：无
    //返回：XFS通用错误码及CIM错误码
    //说明：内部方法
    void SaveRejectBill(ULONG pNumRejRetPerDenoALL[ADP_MAX_DENOMINATION_NUM]);

    //功能：每次挖钞或将每种面额的拒钞数累加保存
    //输入：pNumRejectedPerDenoALL：单次挖钞产生的拒钞数
    //      iRejBillPerDenAll：单次挖钞交易产生的拒钞数
    //输出：无
    //返回：XFS通用错误码
    //说明：内部方法
    void SaveRejBillPerDenoALL(const ST_TOTAL_STACKE_NOTES_DENO_INFO &stNumRejRetPerDeno,
                               ULONG pAllRejRetBillPerDen[ADP_MAX_DENOMINATION_NUM]);

    //功能：在可能产生有效错误码的机芯操作后获取错误码
    //输入：iRet：机芯动作返回值
    //输出：无
    //返回：XFS通用错误码
    //说明：内部函数
    void SaveLastErrCode(int iRet, LPCSTR lpModule);

    void SaveLastActionErrCode(int iRet, LPCSTR lpModule);
    //-------------- 其他辅助常函数类 ------------------

    //功能：检查钞箱是否具有出钞能力
    //输入：aryCassItems：每个钞箱须出钞张数
    //输出：arywCUError：钞箱错误类型
    //返回：XFS通用错误码及CDM错误码
    //说明：内部方法
    int IsDeviceDispensable(const ULONG aryCassItems[ADP_MAX_CU_SIZE],
                            ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE]);

    //功能：检查设置钞箱信息后是否需要复位
    //输入： ADPCassInfo：设置的钞箱信息
    //       usCassID:  钞箱号
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：内部方法
    BOOL VertifyIsNeedResetCassChanged(const ADP_CASS_INFO &ADPCassInfo, WORD usCassID) const;

    //功能：检查 SP设置的币种ID 是否已存在可支持币种ID列表中
    //输入：SupportNotID：设置的币种信息
    //输出：无
    //返回：XFS通用错误码
    //说明：内部方法
    BOOL NoteIsSupported(const ADP_NOTETYPE &SupportNotID) const;

    //功能：比较SP设置的币种是否存在，并返回面额代号
    //输入：lpNotesType：币种信息
    //输出：无
    //返回：无
    //说明：内部方法
    USHORT GetHTIDOfNoteType(const LPWFSCIMNOTETYPE lpNotesType) const;

    //功能：测试钞箱配钞
    //输入：无
    //输出：aryCassItems：每个钞箱须出钞张数
    //返回：XFS通用错误码及CDM错误码
    //说明：内部方法
    long TestCashUnitMixNotes(ULONG aryCassItems[ADP_MAX_CU_SIZE]) const;

    //功能：设置验钞级别
    //输入：enCmd，指示机芯动作
    //输出：无
    //返回：XFS通用错误码及CDM错误码
    //说明：内部函数
    long SetVerificationLevel(EnumSaveSerialOperation enCmd);

    //处理不适合流通钞票
    long SetUnfitnoteVerificationLevel(EnumSaveSerialOperation enCmd);

    //处理变造币
    int  SetRejectCounterfeitNote();
    //处理1999版人民币
    long GetBanknoteInfoAndSetDenoCode();
    void GenerateDenominationToUR(ST_DENO_INFO pArryDenoInfo[MAX_DENOMINATION_NUM], char pArryDENOCode[MAX_DENOMINATION_NUM]);
    //内部静态成员函数
private:
    static void CassType2String(ADP_CASSETTE_TYPE iType, char szCassType[128]);
    static void CassStatus2String(CASHUNIT_STATUS stCassStatus, char szCassStatus[128]);
    static string CassID2String(CASSETTE_ROOM_ID iCassID);
    LPCSTR ConvertStatusToStr(CASSETTE_STATUS boxstatus);
    //功能：用bit位设置启用或禁止的钞箱
    //输入：bEnable：FALSE表示禁止
    //输出：usProhibited：1字节，0位~4位分别表示钱箱1~5,其余位值恒为0。位值为1表示禁用该钱箱
    //返回：无
    //说明：内部方法
    static void GetProhibitedCashUnitBit(const BOOL bEnable[MAX_CASSETTE_NUM], BYTE &usProhibited);
    //删除字符数组中的指定字符
    static void DeleteChar(char *src, char ch);
    bool RemoveDir(const char *pDirPath) const;
    //成员变量
protected:
    //加载驱动库有关的变量
    IURDevice *m_pUR2Dev;
private:
    //通过错误码获取错误信息
    ErrCodeTranslate *m_pErrCodeTrans;

    //回收计数（包含面值、张数信息）
    ULONG m_aryulRetractCountPerDeno[ADP_MAX_DENOMINATION_NUM];
    void InitBVSupportNotesDeno(ST_DENO_INFO pArryDenoInfo[MAX_DENOMINATION_NUM]);
    void WriteNoteVersionInfoToIni();               //30-00-00-00(FT#0002)
    void GenerateDenoArray(BYTE pArryDENOCode[MAX_DENOMINATION_NUM]);
    long InitUnitSetting();

    BOOL InnerNeedDownLoad();
    void AccumulatePerDenomCount(ST_TOTAL_STACKE_NOTES_DENO_INFO &stResult, ST_TOTAL_STACKE_NOTES_DENO_INFO &stAppend);  //40-00-00-00(FT#0016)
private:
    //配置数据
    //初始化配置信息
    DEVCONFIGINFO m_sDevConfigInfo;
    //机芯操作配置
    ST_OPERATIONAL_INFO m_stOperationalInfo;
    //硬件配置
    ST_HW_CONFIG m_stHWConfig;
    //出钞能力
    ADPCDMCAPS m_sCDMCapabilities;
    //进钞能力
    ADPCIMCAPS  m_sCIMCapabilities;

    //须SP设置的成员数据
    //适配层钱箱配置信息
    ADP_CASS_INFO m_aryADPCassSettingInfo[ADP_MAX_CU_SIZE];
    //禁止出钞钱箱组合
    BOOL m_arybEnableCassUnit[MAX_CASSETTE_NUM];
    //SP设置的被支持的钞票类型数组
    map<BYTE, WFSCIMNOTETYPE> m_mapSupportNotID;
    //机芯是否处于交换状态
    BOOL m_bExchangeActive;

    //须返回SP的数据
    //执行可能产生回收动作的指令后回收的钞数
    ULONG m_aryRetractCount[ADP_MAX_CU_SIZE];
    //状态改变后是否需要复位
    BOOL m_bIsNeedReset;

    //机芯状态
    ADPSTATUSINFOR m_sDevStatus;
    //每3字节对应应用列表表示的ID拒钞或回收数，最后3字节表示不可识别的拒钞或回收数
    char m_szRejectBill[48];
    //版本信息
    char m_szFWVersion[MAX_FW_DATA_LENGTH];     //日立循环机芯的固件版本
    char m_szSPVersion[512];     //日立SP各主要组件的版本

    //内部使用数据
    //机芯BV支持的所有钞票类型
    map<BYTE, WFSCIMNOTETYPE> m_mapBVSupportNoteID;
    //BRM模块是否被初始化过
    BOOL m_bBRMInitialized;
    //适配层当前钱箱状态信息
    CASHUNIT_STATUS m_aryADPCassStatusInfo[ADP_MAX_CU_SIZE];
    //结束交换时钱箱状态信息
    CASHUNIT_STATUS m_aryCassStatusWhenEndEx[ADP_MAX_CU_SIZE];

    //最后一次机芯操作产生的错误码
    char m_szLastErrCode[MAX_ERRCODE_LENGTH + 1];
    //最后一次涉及机芯动作类操作产生的错误码
    char m_szLastErrCodeAction[MAX_ERRCODE_LENGTH + 1];

    USHORT m_usRecoveryCode;                            //恢复码
    UCHAR m_ucPostionCode[16];                         //错误位置

    //是否记录错误码于日志文件中，民生银行专用
    BOOL m_bRecordErrCodeCMBC;

    //是否处于出钞后钞票仍在暂存区的状态
    BOOL m_bCashOutBeforePresent;
    //Shutter是否处于打开或关闭时的阻塞状态
    BOOL m_bOpenShutterJammed;
    BOOL m_bCloseShutterJammed;
    //USB通信是否正常
    BOOL m_bConnectNormal;

    //BV连接是否正常
    BOOL m_bBVConnectNormal;

    BOOL m_bCassVerifyFail[ADP_MAX_CU_SIZE];


    set<ULONG> m_sLastestInternalCounts;

    //IStatusChangeListener *m_pListener;
    string  m_strConfigFile;
    CINIFileReader m_ReaderConfigFile;

    //Get cash acceptor
    string  m_strCashAcceptorFile;               //test#5
    CINIFileReader m_ReaderCashAcceptorFile;     //test#5
    //下载固件相关成员变量
    char m_szFWFilesPath[MAX_PATH];
    //validator ID todo

    TYPEMAPFWUR m_mapURFWVerInfo;
    TYPEMAPFWDATA m_mapProFilesInfo;

    FWDOWNSTATUS m_FWDStatus;

    CQtDLLLoader<ICryptData>	      m_pCryptData;
    CQtDLLLoader<IAllDevPort>         m_pDev;                   //test#11
    //DWORD m_dwWaitGSNTimeOut;//等待处理冠字码的超时时间
    //CGenerateSN m_GenSN;
    friend class ErrCodeTranslate;
    friend class CAssistGenerateSN;

    LPCSTR iAmountLimitForAP;       //test#5
    int iAmountLimitForAPTmp;       //test#5

    ADP_LFS_CMD_ID m_eLfsCmdId;     //test#8
    char cFirmHardInfo;             //test#13

    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfoTemp;          //test#13
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeUnfitNotesDenoInfoTemp;     //test#13
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoInfoTemp;    //test#13
};

/*
class CAssistGenerateSN
{
public:
    CAssistGenerateSN(CThreadGenerateSN*  pGenSNThread, DWORD dwTimeOut)
    {
        m_pwork = pGenSNThread;
        m_dwTimeOut = dwTimeOut;
    }
    ~CAssistGenerateSN()
    {
        m_pwork->DoWork();
        m_pwork->WaitExit(m_dwTimeOut);
    };

private:
    DWORD m_dwTimeOut;
    CThreadGenerateSN* m_pwork;
};
*/


#endif
