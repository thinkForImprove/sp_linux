#pragma once
#include "QtTypeDef.h"
#include <QtCore/qglobal.h>
#include <string.h>
#include <vector>

//错误码说明：
//------------------------ 错误码定义 ----------------------------------
#define ERR_UR_SUCCESS                   (0)    //成功，通讯成功，设备操作也成功，GetLastError返回长度为0的空字串
#define ERR_UR_WARN                      (1)    //命令执行成功，可是有警告产生了，调用GetLastError取具体警告码（14字节）
#define ERR_UR_HARDWARE_ERROR            (-1)   //机芯执行指令异常终止，调用GetLastError取具体错误码（7字节）
#define ERR_UR_END_STATE                 (-2)   //机芯返回的结束状态有误，以下返回码无法通过GetLastError读取具体的机芯错误
#define ERR_UR_PARAM                     (-3)   //参数错误
#define ERR_UR_COMM                      (-4)   //通讯失败
#define ERR_UR_DEVBUSY                   (-5)   //设备忙
#define ERR_UR_CREATETHREAD              (-6)   //创建线程失败
#define ERR_UR_NO_DEVICE                 (-7)   //如果指定名的设备不存在，创建对象时返回
#define ERR_UR_RETURN_DATA_LEN           (-8)   //机芯返回的数据长度不合法
#define ERR_UR_NOLOADDLL                 (-9)   //没有加载通讯库
#define ERR_UR_RESPDATA_ERR              (-10)  //数据解析错误
#define ERR_UR_INVALID_COMMAN            (-11)  //无效命令
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH)
#define ERR_UR_WARN_NOTESREJECT	         (-12)  //警告，出钞口有拒钞
#define ERR_UR_WARN_STOPFEED             (-13)  //警告，拒绝收纳
#define ERR_UR_WARN_TOOMANYRJ            (-14)  //警告，拒钞太多
#define ERR_UR_WARN_FULL                 (-15)  //警告，钞箱满
#define ERR_UR_WARN_BVMEMORYFULL         (-16)  //警告，内存满
#define ERR_UR_WARN_NONOTES              (-17)  //警告，入钞口无钞
#else
//自动精查错误
#define ERR_UR_BV_IMG_FULL               (-12)  //BV画像数据满           //30-00-00-00(FS#0022)
#define ERR_UR_RJ_BOX_FULL               (-13)  //BV画像数据满           //30-00-00-00(FS#0022)
#endif

//以下错误是USB相关错误
#define ERR_UR_USB_PARAM_ERR             (-101) // 参数错误
#define ERR_UR_USB_CMD_ERR               (-102) // 指令错误
#define ERR_UR_USB_CONN_ERR              (-103) // 连接错误
#define ERR_UR_USB_CANCELLED             (-104) // 指令取消
#define ERR_UR_USB_BUSY                  (-105) // 操作忙
#define ERR_UR_USB_NODLL                 (-106) // 驱动库不存在
#define ERR_UR_USB_NOFUNC                (-107) // 驱动库入口函数不存在
#define ERR_UR_USB_OTHER                 (-108) // 其他USB错误

//以下错误是机芯驱动相关错误
#define ERR_UR_DRV_CANCEL_END            (-201) // #2
#define ERR_UR_DRV_CANCEL_NOPST          (-202) //
#define ERR_UR_DRV_CANCEL_CROSS_END      (-203) // #3
#define ERR_UR_DRV_CANCEL_NOTHING        (-204) // #4
#define ERR_UR_DRV_FUNC                  (-205) // #5
#define ERR_UR_DRV_PRM                   (-206) // #6
#define ERR_UR_DRV_DRVHND_DIFFER         (-207) // #7
#define ERR_UR_DRV_DRV_REMOVE            (-208) // #8
#define ERR_UR_DRV_BLD                   (-209) // #9
#define ERR_UR_DRV_INDATA                (-210) // #10
#define ERR_UR_DRV_OUTDATA               (-211) // #11
#define ERR_UR_DRV_INOUTDATA             (-212) // #12
#define ERR_UR_DRV_ENTRY_DEVICE_OVER     (-213) // #13  
#define ERR_UR_DRV_ENTRY_THREAD_OVER     (-214) // #14
#define ERR_UR_DRV_BCC                   (-215) // #15
#define ERR_UR_DRV_INDATA_BUFFSZ         (-216) // #16
#define ERR_UR_DRV_OUTDATA_BUFFSZ        (-217) // #17
#define ERR_UR_DRV_INOUTDATA_BUFFSZ      (-218) // #18
#define ERR_UR_DRV_LINE_TIMEOUT          (-219) // #19
#define ERR_UR_DRV_COMMAND_TIMEOUT       (-220) // #20
#define ERR_UR_DRV_CLOSE                 (-221) // #21
#define ERR_UR_DRV_OPEN_BUSY             (-222) // #22
#define ERR_UR_DRV_SEND_BUSY             (-223) // #23
#define ERR_UR_DRV_RCV_BUSY              (-224) // #24
#define ERR_UR_DRV_EP_DOWN               (-225) // #25
#define ERR_UR_DRV_MEMORY                (-226) // #26
#define ERR_UR_DRV_HANDLE                (-227) // #27
#define ERR_UR_DRV_REG                   (-228) // #28
#define ERR_UR_DRV_DRVCALL               (-229) // #29
#define ERR_UR_DRV_THREAD                (-230) // #30
#define ERR_UR_DRV_POSTMSG               (-231) // #31
#define ERR_UR_DRV_TRACE                 (-232) // #32
#define ERR_UR_DRV_CRYPT                 (-233) // #33
#define ERR_UR_DRV_ALREADYCOMPLETE       (-501) // #33
#define ERR_UR_DRV_USB_SUM               (-502) // #33
#define ERR_UR_DRV_OTHER                 (-301) // 驱动没有定义的错误码

#define ERR_UR_FNC_LIB_OUTDATA_BUFFSZ    (-432) // Buffer size error  Buffer size error for output data

// 最大钱箱个数5+1URJB
#define MAX_CASSETTE_NUM             (6)

// 最大room个数
#define MAX_ROOM_NUM                 (10)           //30-00-00-00(FS#0022)

// 面额总数
#define MAX_DENOMINATION_NUM         (127)

// 维护信息总数
#define MAINTENANCE_INFO_LENGTH      (4096)

// 定义冠字码bmp图片最大数据长度
#define MAX_SNIMAGE_DATA_LENGTH      (40960)
//30-00-00-00(FS#0022) #define MAX_FULLIMAGE_DATA_LENGTH    (120480)
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH)
#define MAX_FULLIMAGE_DATA_LENGTH    (120480)
#else
#define MAX_FULLIMAGE_DATA_LENGTH    (200 * 1024)           //30-00-00-00(FS#0022)
#endif

// 定义冠字码字符串最大长度
#define MAX_SERIAL_LENGTH            (128)

// 冠字码图片信息单次命令获得最大个数
#define MAX_MEDIA_INFO_NUM           (512)

// 固件版本字符串最大长度
#define MAX_FW_DATA_LENGTH           (512)

// 机芯日志数据最大长度
#define MAX_LOG_DATA_LENGTH          (7174 * 255)

// 挖钞命令使用，多少种面额或多少钞箱挖钞
#define MAX_DISP_DENO_NUM            (10)
#define MAX_DISP_ROOM_NUM            (10)

// 用户内存数据最大长度\数组长度
#define MAX_USER_MEMORY_DATA_LENGTH  (128)
#define MAX_USER_MEMORY_DATA_ARRAY_NUM  (5)

// BV警告信息数组长度
#define MAX_BV_WARNING_INFO_NUM      (4)

//下载固件时，最大数据长度
#define FW_BATCH_DATA_MAXSIZE   (7168)



// 币种码定义
enum CURRENCY_CODE
{
    CURRENCY_CODE_CNY = 0xA0,
    CURRENCY_CODE_EUR = 0xFE,
    CURRENCY_CODE_RESERVED = 0x00
};

// 拒钞校验程度
enum UNFIT_LEVEL
{
    UNFIT_LEVEL_DEFAULT = 1,        //默认
    UNFIT_LEVEL_NORMAL,             //正常
    UNFIT_LEVEL_SOFT,               //宽松
    UNFIT_LEVEL_STRICT              //严格
};

// BV验钞程度
enum VERIFICATION_LEVEL
{
    VERIFICATION_LEVEL_DEFAULT = 1, //默认
    VERIFICATION_LEVEL_NORMAL,      //正常
    VERIFICATION_LEVEL_STRICT       //严格
};

// 钱箱类型定义
enum CASSETTE_TYPE
{
    CASSETTE_TYPE_UNLOAD,            // 未加载,钞箱内存未使用
    CASSETTE_TYPE_RB,                // 循环箱
    CASSETTE_TYPE_AB,                // AB箱
    CASSETTE_TYPE_URJB,
    //CASSETTE_TYPE_DAB,               // 多功能AB留
    CASSETTE_TYPE_UNKNOWN            // 未知状态
};

// 钱箱操作类型定义
enum CASSETTE_OPERATION
{
    RB_OPERATION_RECYCLE   = 0x01,   // 循环箱：循环
    RB_OPERATION_DEPOSITE  = 0x02,   // 循环箱：存款
    RB_OPERATION_DISPENSE  = 0x03,   // 循环箱：出钞
    AB_OPERATION_DEPREJRET = 0x04,   // 回收箱 指定
    RB_OPERATION_ESCW      = 0x05,   // 收集用，待启用
    RB_OPERATION_UNKNOWN   = 0x00    // 未加载,未识别
};

// 钱箱号定义
enum CASSETTE_NUMBER
{
    CASSETTE_1 = 1,                  // 钱箱1
    CASSETTE_2,                      // 钱箱2
    CASSETTE_3,                      // 钱箱3
    CASSETTE_4,                      // 钱箱4
    CASSETTE_5,                      // 钱箱5
    CASSETTE_6,                      // 钱箱6 指定为URJB箱
    RESERVED
};

// 钱箱线路定义
enum CASSETTE_ERR
{
    CASSETTE_ERR_RESERVED = 0,        // 没有发生错误
    CASSETTE_ERR_LANE5 = 1,           // 钱箱线路5
    CASSETTE_ERR_LANE4 = 2,           // 钱箱线路4
    CASSETTE_ERR_LANE3 = 4,           // 钱箱线路3
    CASSETTE_ERR_LANE2 = 8,           // 钱箱线路2
    CASSETTE_ERR_LANE1 = 16           // 钱箱线路1
};

// 钞箱状态
/*enum CASSETTE_STATUS{
    CASSETTE_STATUS_NORMAL = 0,		  // 正常
    CASSETTE_STATUS_EMPTY,            // 空
    CASSETTE_STATUS_NEAREST_EMPTY,    // 几乎空
    CASSETTE_STATUS_NEAR_EMPTY,       // 将空
    CASSETTE_STATUS_NEAR_FULL,        // 将满
    CASSETTE_STATUS_FULL,             // 满
    CASSETTE_STATUS_UNKNOWN           // 未知或不可用
};
*/ //test#2
enum CASSETTE_STATUS{
    CASSETTE_STATUS_NORMAL = 0,		  // 正常
    CASSETTE_STATUS_FULL,             // 满
    CASSETTE_STATUS_NEAR_FULL,        // 将满
    CASSETTE_STATUS_NEAR_EMPTY,       // 将空
    CASSETTE_STATUS_EMPTY,            // 空
    CASSETTE_STATUS_INOP,            // INOP
    CASSETTE_STATUS_MISSING,            // MISSING
    CASSETTE_STATUS_MANIP,            // MANIP
    CASSETTE_STATUS_NEAREST_EMPTY,    // 几乎空
    CASSETTE_STATUS_UNKNOWN = -1           // 未知或不可用
};//test#2

// Shutter门状态
enum UR2SHUTTER_STATUS
{
    UR2SHUTTER_STATUS_OPEN = 0,          // 打开 Fully Opened
    UR2SHUTTER_STATUS_CLOSED,            // 关闭 Completely closed
    UR2SHUTTER_STATUS_OTHERS,            // 其他情况 Half Opened
    UR2SHUTTER_STATUS_UNKNOWN            // 未知状态
};

//40-00-00-00(FT#0019)
enum UR2INPUTPOS_STATUS{
    UR2INPUTPOS_STATUS_EMPTY = 0,
    UR2INPUTPOS_STATUS_NOTEMPTY,
    UR2INPUTPOS_STATUS_OTHERS,
    UR2INPUTPOS_STATUS_UNKNOWN
};

// 人民币面额代码定义
enum DENOMINATION_CODE {
	DENOMINATION_CODE_00,            // 未指定
//    DENOMINATION_CODE_10_4TH  = 0x01,
//    DENOMINATION_CODE_50_4TH  = 0x02,
//    DENOMINATION_CODE_100_4TH = 0x03,
    DENOMINATION_CODE_100_C = 0x04,
    DENOMINATION_CODE_20_C = 0x05,
    DENOMINATION_CODE_10_C  = 0x06,
    DENOMINATION_CODE_50_C  = 0x07,
    DENOMINATION_CODE_1_B   = 0x08,
    DENOMINATION_CODE_5_C   = 0x09,
    DENOMINATION_CODE_10_B  = 0x0A,
    DENOMINATION_CODE_20_B  = 0x0B,
    DENOMINATION_CODE_50_B    = 0x0C,
    DENOMINATION_CODE_100_D  = 0x0D,
    DENOMINATION_CODE_100_B   = 0x0E,
    DENOMINATION_CODE_5_B     = 0x0F,
    DENOMINATION_CODE_10_D   = 0x10,
    DENOMINATION_CODE_20_D   = 0x11,
    DENOMINATION_CODE_50_D   = 0x12,
    DENOMINATION_CODE_1_D    = 0x13,
    DENOMINATION_CODE_5_D    = 0x14,                    //30-00-00-00(FS#0018)
    DENOMINATION_CODE_ALL       = 0xFF,
};

// 日志信息分类
enum LOG_INFO_TYPE
{
    LOG_INFO_STATISTICAL_TOTAL,      // 存储HCM静态日志数据，用来分析HCM模块的拒钞率和出错率
    LOG_INFO_STATISTICAL_SPECIFIC,
    LOG_INFO_ERRCODE,                // 错误码信息，分析出错频率
    LOG_INFO_WARNINGCODE,            // 警告码信息，分析警告频率
    LOG_INFO_ERRANALYSIS_GENERAL,
    LOG_INFO_OPERATIONAL,
    LOG_INFO_NEAREST_OPERATION,
    LOG_INFO_ERRANALYSIS_INDIVIDUAL,
    LOG_INFO_SENSOR,
    LOG_INFO_INTERNAL_COMMAND,
    LOG_INFO_NOTES_HANDLING,
    LOG_INFO_MOTOR_CONTROL,
    LOG_INFO_SENSORLEVEL_LATEST,
    LOG_INFO_SENSORLEVEL_SPECIFIC
};

// 日志信息周期定义
enum LOG_INFO_TERM
{
    TERM_WHOLE_DATA,                 // 所有日志
    TERM_THIS_MONTH,                 // 本月日志
    TERM_LAST_MONTH,                 // 上个月日志
    MONTH_BEFOR_LAST                 // 上上个月的日志
};

// 钞票记录整张图片
enum RECORD_FULLIMAGE_MODE
{
    FULLIMAGE_NO_RECORDS,            // 不记录钞票全幅图片，只记录冠字码图片
    FULLIMAGE_ALL_NOTES,             // 所有钞票均记录全幅图片
    FULLIMAGE_REJECTE_NOTES,         // 只记录回收钞票全幅图片
};

// 动作模式依赖BV验钞是否能识别冠字号
enum REJECT_NOTE_NOSN
{
    ACTION_REJECT_NOTES,             // 当BV未识别出序列号，验钞拒绝该钞票
    ACTION_NO_REJECT_NOTES           // 当BV未识别出序列号，验钞不拒绝该钞票
};

// 钞票指数 10^Power Index
enum NOTE_POWER_INDEX
{
    POWER_INDEX_0 = 0,
    POWER_INDEX_1,
    POWER_INDEX_2,
    POWER_INDEX_3,
    POWER_INDEX_4,
    POWER_INDEX_5
};

// 钞箱位置ID
enum CASSETTE_ROOM_ID
{
    ID_CS       = 0x01,
    ID_ESC      = 0x02,
    ID_URJB     = 0x03,
    ID_ROOM_1   = 0x1A,
    ID_ROOM_2   = 0x2A,
    ID_ROOM_3   = 0x3A,
    ID_ROOM_4   = 0x4A,
    ID_ROOM_5   = 0x5A,
    ID_ROOM_1B  = 0x1B,
    ID_ROOM_1C  = 0x1C,
    ID_ROOM_RESERVED  = 0x00
};

// 验钞模式
enum VALIDATION_MODE
{
    VALIDATION_MODE_REAL = 0x00,     // 实钞模式
    VALIDATION_MODE_TEST = 0xFF      // 测试模式
};

// 钞票传输情况,在一次传输过程中可能有多种情况同时出现
enum FED_NOTE_CONDITION
{
    CONDITION_SKEW         = 1,
    CONDITION_SHORT_NOTE   = 2,
    CONDITION_LONG_NOTE    = 4,
    CONDITION_SHIFT        = 8,
    CONDITION_NOTES_REMAIN = 16,
    CONDITION_MIS_FEED     = 32,
    CONDITION_MOTOR_LOST_CALIBRATION = 64,
    CONDITION_HALF_NOTE    = 128
};

// 指定回收位置或者为指定钞箱可作为以下回收位置
//      2~5钞箱必须设置为DESTINATION_REJCET_DEFAULT
//      1钞箱可设置为DESTINATION_REJCET_CS | DESTINATION_REJCET_DISPENSE | DESTINATION_REJCET_DEPOSIT 
//             	或者 DESTINATION_REJECT_CASH1_COMBINE
//      6钞箱(URJB箱)可设置为DESTINATION_REJCET_CS
enum DESTINATION_REJCET
{
    DESTINATION_REJCET_DEFAULT  = 0x00,          //默认位置，模式使用SET_UNIT_INFO中设置的值
    DESTINATION_REJCET_CS       = 0x40,          //回收至CS
    DESTINATION_REJCET_DISPENSE = 0x20,          //出钞回收位置
    DESTINATION_REJCET_DEPOSIT  = 0x10,          //存钞回收位置
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH)
    DESTINATION_REJECT_CASH1_COMBINE = 0x70,      //钞箱1组合设置
    DESTINATION_REJECT_CASH2_COMBINE = 0xF0      //钞箱1组合设置
#else
    DESTINATION_REJECT_CASH1_COMBINE = 0xF0      //钞箱1组合设置      //30-00-00-00(FS#0022)
#endif
};

// 指定钞票类别
enum NOTE_CATEGORY
{
    NOTE_CATEGORY_UNKNOWN = 0,
    NOTE_CATEGORY_1,                 //Cat.1
    NOTE_CATEGORY_2,                 //Cat.2
    NOTE_CATEGORY_3,                 //Cat.3
    NOTE_CATEGORY_4,                 //Cat.4
    NOTE_CATEGORY_4B,                //Cat.4B
    NOTE_CATEGORY_ALL
};

// 固件下载数据块类型
enum PDL_BLOCK_TYPE
{
    PDL_FIRST_BLOCK,                 //第一段数据
    PDL_MIDDLE_BLOCK,                //中间段数据
    PDL_LAST_BLOCK                   //最后一段数据
};

// 指定用户内存块
enum USER_MEMORY_TARGET
{
    USER_MEMORY_TARGET_RESERVED,    //保留
    USER_MEMORY_TARGET_CASS1,
    USER_MEMORY_TARGET_CASS2,
    USER_MEMORY_TARGET_CASS3,
    USER_MEMORY_TARGET_CASS4,
    USER_MEMORY_TARGET_CASS5,
    USER_MEMORY_TARGET_UR2,
    USER_MEMORY_TARGET_ALLCASS     //读内存时使用
};

// BV记录冠字码图片类别
enum BV_IMAGE_TYPE
{
    BV_IMAGE_NOEXIST   = 0,          //没有图片记录
    BV_IMAGE_FULLIMAGE = 1,          //记录全幅图片
    BV_IMAGE_SNIMAGE   = 2,          //记录SN 图片
    BV_IMAGE_BOTH      = 3           //同时记录SN、全幅图片
};

// 拒钞原因
enum MEDIA_INFORMATION_REJECT_CAUSE
{
    MEDIA_INFORMATION_REJECT_CAUSE_RESERVED          = 0x00,
    MEDIA_INFORMATION_REJECT_CAUSE_SHIFT             = 0x01,
    MEDIA_INFORMATION_REJECT_CAUSE_SKEW              = 0x02,
    MEDIA_INFORMATION_REJECT_CAUSE_EXSKEW            = 0x04,
    MEDIA_INFORMATION_REJECT_CAUSE_LONG              = 0x05,
    MEDIA_INFORMATION_REJECT_CAUSE_SPACING           = 0x07,
    MEDIA_INFORMATION_REJECT_CAUSE_INTERVAL          = 0x08,
    MEDIA_INFORMATION_REJECT_CAUSE_DOUBLE            = 0x09,
    MEDIA_INFORMATION_REJECT_CAUSE_DIMENSTION_ERR    = 0x0A,
    MEDIA_INFORMATION_REJECT_CAUSE_DENO_UNIDENTIFIED = 0x0B,
    MEDIA_INFORMATION_REJECT_CAUSE_VERIFICATION      = 0x0C,
    MEDIA_INFORMATION_REJECT_CAUSE_UNFIT             = 0x0D,
    MEDIA_INFORMATION_REJECT_CAUSE_REJ_RET_SPECFIED  = 0x0E,
    MEDIA_INFORMATION_REJECT_CAUSE_BV_OTHERS         = 0x10,
    MEDIA_INFORMATION_REJECT_CAUSE_SN_BACKLIST       = 0x15,
    MEDIA_INFORMATION_REJECT_CAUSE_DIFF_DENO         = 0x16,
    MEDIA_INFORMATION_REJECT_CAUSE_FOCIBLE_RJ        = 0x17,
    MEDIA_INFORMATION_REJECT_CAUSE_EXCESS            = 0x18,
    MEDIA_INFORMATION_REJECT_CAUSE_FACTOR1           = 0x19,
    MEDIA_INFORMATION_REJECT_CAUSE_FACTOR2           = 0x1A,
    MEDIA_INFORMATION_REJECT_CAUSE_NO_VALIDATION     = 0x1B,
    MEDIA_INFORMATION_REJECT_CAUSE_BV_FORMAT_ERR     = 0x1C
};

// 冠字号版本
enum eNoteEdition
{
    eNoteEdition_1988_010  = 0,
    eNoteEdition_1992_050  = 1,
    eNoteEdition_1992_100  = 2,
    eNoteEdition_1999_100  = 3,
    eNoteEdition_2000_020  = 4,
    eNoteEdition_2001_050  = 5,
    eNoteEdition_2001_010  = 6,
    eNoteEdition_2005_010  = 7,
    eNoteEdition_2005_020  = 8,
    eNoteEdition_2005_050  = 9,
    eNoteEdition_2005_100  = 10,
    eNoteEdition_2015_100  = 11,
    eNoteEdition_unknown = 12
};

//定义操作类型
enum eCMDType
{
    eCMDType_CashCount  = 0,
    eCMDType_StoreMoney  = 1,
    eCMDType_Dispense  = 2,
    eCMDType_Other = 3,
};

//30-00-00-00(FS#0025) add start
enum BVSETTINGCMDTYPE{
    BVSET_CMDTYPE_CASHCOUNT = 0,
    BVSET_CMDTYPE_STOREMONEY = 1,
    BVSET_CMDTYPE_DISPENSE = 2
};
//30-00-00-00(FS#0025) add end

typedef struct tag_SET_VERIFICATION_LEVEL           //设置校验级别-宽松
{
    BOOL bSetForCashCount;                          //验钞
    BOOL bSetForStoreMoney;                         //存钞
    BOOL bSetForDispense;                           //挖钞
} SET_VERIFICATION_LEVEL;

// 错误详情，错误码与错误指示
typedef struct tag_ERR_DETAIL
{
    char   ucErrorCode[8 + 1];                       //错误码或警告码，04 or 05为错误码，8x or 9x为警告码，00 无错误
    int    iCassetteError;                           //钞箱发生错误，取值范围 CASSETTE_ERR 与运算，0 无错误
    USHORT usRecoveryCode;                           //恢复码
    UCHAR  ucPostionCode[16];                        //错误位置
} ST_ERR_DETAIL;

// 设备状态
typedef struct tag_DEV_STATUS_INFO
{
    BOOL  bCashAtShutter;                               // Shutter移动范围内有钞
    BOOL  bCashAtCS;                                    // CS钞口中有钞
    BOOL  bCashAtCSErrPos;                              // 有钞票位于CS出钞口的错误位置
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH)
    BOOL  bCashAtOutCS;                                 // CS出钞口有钞
#endif
    BOOL  bESCFull;                                     // ESC暂存箱满，可能原因为 1.满限制达到 2.物理满
    BOOL  bESCNearFull;                                 // ESC存储200张或更多
    BOOL  bESCEmpty;                                    // ESC暂存箱为空
    BOOL  bURJBFull;                                    // URJB满
    BOOL  bURJBEmpty;                                   // URJB为空
    BOOL  bNotesRJInCashCout;                           // 验钞有拒钞
    BOOL  bHCMUPInPos;                                  // HCM模块(Upper unit)位置正确
    BOOL  bHCMLOWInPos;                                 // HCM模块(Lower unit)位置正确
    BOOL  bURJBOpen;                                    // URJB箱门打开
    BOOL  bRearDoorOpen;                                // HCM模块后门打开
    BOOL  bFrontDoorOpen;                               // HCM模块前门打开
    BOOL  bESCOpen;                                     // ESC门打开
    BOOL  bESCInPos;                                    // ESC位置正确
    BOOL  bESCRearEnd;                                  // ESC stack area rear end
    BOOL  bCSInPos;                                     // CS位置正确
    BOOL  bBVFanErr;                                    // BV风扇报警
    BOOL  bBVOpen;                                      // BV验钞模块打开
    UR2SHUTTER_STATUS  iOutShutterStatus;               // 外部Shutter门状态
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH)
    UR2SHUTTER_STATUS  CashInShutterStatus;             // 入金口Shutter门状态    //test#13
    UR2INPUTPOS_STATUS InPutStatus;                     // 入钞口状态　//40-00-00-00(FT#0019)
#endif
    CASSETTE_STATUS CassStatus[MAX_CASSETTE_NUM - 1];   // 钞箱状态,  AB箱 正常/空/将满/满/UNKNOWN
    BOOL  bCassInPos[MAX_CASSETTE_NUM - 1];             // 钞箱位置正确
    BOOL  bForcedOpenShutter;                           // 强制打开Shutter门
    BOOL  bForcedRemovCashInCS;                         // 强制移动CS内的钞票
    BOOL  bCashLeftInCS;                                // CS残留有钞票
    BOOL  bCashExistInESC;                              // ESC中有钞票，基于原始传感器状态当人工清理ESC后状态不一定被设置
    BOOL  bReqReadStatus;                               // Read Stats 命令发出后该值被设置
    BOOL  bReqGetOPLog;                                 // Get Log Data命令发出后该值被设置
    BOOL  bReqReset;                                    // 要求复位，当HCM检测到错误后该被设置
    BOOL  bBVWarning;                                   // BV存在警告
    BOOL  bDuringEnergy;                                // 处与节能模式
} ST_DEV_STATUS_INFO;

// BV相关信息
typedef struct tag_BV_INFO
{
    // BV功能描述，是否支持获取序列号、全幅图片等
    char szBVSerialNumber[17];               // BV序列号"BVZ10/20SerialNumber" 10/20:HOST-BV Advanced/Basic type
    BOOL bArticle6Support;                   // Article6是否支持，TRUE:支持 FALSE:不支持
    BOOL bBackTracingSupport;                // 是否支持纸币数据的输出，用于回溯跟踪
    BOOL bUnknownNotesNumberInDispSupport;   // 是否支持出钞时估算未知钞票张数，例如未知钞票发生双张错误，计数可能不准确
    BOOL bSNImageReadFunctionSupport;        // 是否支持冠字号图片读取功能
    BOOL bFullSNImageRecordSupport;          // 是否支持记录冠字全幅图片
    BOOL bUnknownNotesEstimationSupport;     // 是否支持每次发生未知钞票估算张数
    BOOL bSNReadSNFunctionSupport;           // 是否支持冠字号信息记录功能
    BOOL bActiveVerificationLevelSupport;    // 是否支持BV验钞校验程度设置
    BOOL bUseUnfitLevelSupport;              // 是否BV校验废钞程度设置
    BOOL bRejectSNSupport;                   // 是否支持拒钞冠字号信息记录功能
} ST_BV_INFO;

// BV验钞校验程度
typedef struct tag_BV_VERIFICATION_LEVEL
{
    UNFIT_LEVEL iUnfitLevel;
    VERIFICATION_LEVEL iVerifiationLevel;

    tag_BV_VERIFICATION_LEVEL()
    {
        iUnfitLevel = UNFIT_LEVEL_DEFAULT;
        iVerifiationLevel = VERIFICATION_LEVEL_DEFAULT;
    }
} ST_BV_VERIFICATION_LEVEL;

// 面额相关信息
typedef struct tag_DENO_INFO
{
    CURRENCY_CODE      iCurrencyCode;    // 币种ISO币种代码，如：USD，CNY等
    int                iCashValue;       // 钞票面额, 10/20/50/100. 内部需要处理为钞票面额的字符串 3个数字+1字母，如：999："999 " 1000：001K 1000000:001M
    BYTE               ucVersion;        // 版本信息
    BYTE               ucIssuingBank;    // 发行银行
    BYTE               ucNoteWidth;      // 钞票宽度
    BYTE               ucNoteLength;     // 钞票长度
} ST_DENO_INFO;

// 钞箱信息
typedef struct tag_CASSETTE_INFO
{
    CASSETTE_NUMBER     iCassNO;          // 钱箱号，取值1~6
    CASSETTE_TYPE       iCassType;        // 钱箱类型 RB/DRB/AB/DAB/UNKNOW
    DENOMINATION_CODE   iDenoCode;        // 面额代码

    // 钞箱单元信息CASSETTE_ROOM_INFO
    CASSETTE_OPERATION  iCassOper;           // 操作类型, 循环/存款/取款/未识别
    CURRENCY_CODE       iCurrencyCode;       // 钱箱对应, 币种ISO币种代码，如：USD，CNY等
    int                 iCashValue;          // 钞票面额, 10/20/50/100. 内部需要处理为钞票面额的字符串 3个数字+1字母，如：999："999 " 1000：001K 1000000:001M
    BYTE               ucVersion;           // 版本信息
    BYTE               ucIssuingBank;       // 发行银行
    DESTINATION_REJCET  iCassNoteHandInfo;   // 钞箱对钞票操作类型
} ST_CASSETTE_INFO;

// BV依赖模式
typedef struct tag_BV_DEPENDENT_MODE
{
    RECORD_FULLIMAGE_MODE iFullImageMode;   // 是否记录全幅钞票
    REJECT_NOTE_NOSN iRejcetNoteNOSN;       // 冠字号未识别时钞票为拒钞
    BOOL bEnableBVModeSetting;              //是否启用
    tag_BV_DEPENDENT_MODE()
    {
        bEnableBVModeSetting = FALSE;
        iFullImageMode = FULLIMAGE_NO_RECORDS;
        iRejcetNoteNOSN = ACTION_REJECT_NOTES;
    }
} ST_BV_DEPENDENT_MODE;

// 面额存储信息
typedef struct tag_STACKE_NOTES_DENO_INFO
{
    BYTE ucDENOCode;                        //面额代码
    CASSETTE_ROOM_ID iDest;                 //钞票流向目的钞箱
    USHORT usNumberStack;                   //存储的钞票张数
    tag_STACKE_NOTES_DENO_INFO()
    {
        ucDENOCode = DENOMINATION_CODE_00;
        iDest = ID_ROOM_RESERVED;
        usNumberStack = 0;
    }
} ST_STACKE_NOTES_DENO_INFO;

// 每种面额存储信息
typedef struct tag_TOTAL_STACKE_NOTES_DENO_INFO
{
    BYTE ucCount;				            //识别后的面额数 ST_STACKE_NOTES_INFO 有效个数
	ST_STACKE_NOTES_DENO_INFO stStackeNotesInfo[MAX_DENOMINATION_NUM];//面额存储信息
	tag_TOTAL_STACKE_NOTES_DENO_INFO()
	{
		ucCount = 0;
	}

    //30-00-00-00(FS#0022)
    tag_TOTAL_STACKE_NOTES_DENO_INFO operator+(tag_TOTAL_STACKE_NOTES_DENO_INFO stAddend){
        tag_TOTAL_STACKE_NOTES_DENO_INFO stSum;

        stSum = *this;
        for(int i = 0; i < stAddend.ucCount; i++){
            if(stAddend.stStackeNotesInfo[i].usNumberStack > 0){
                BOOL bFind = FALSE;
                for(int j = 0; j < stSum.ucCount; j++){
                    if(stSum.stStackeNotesInfo[j].ucDENOCode == stAddend.stStackeNotesInfo[i].ucDENOCode){
                        stSum.stStackeNotesInfo[j].usNumberStack += stAddend.stStackeNotesInfo[i].usNumberStack;
                        bFind = TRUE;
                        break;
                    }
                }

                if(!bFind){
                    stSum.stStackeNotesInfo[stSum.ucCount] = stAddend.stStackeNotesInfo[i];
                    stSum.ucCount++;
                }
            }
        }

        return stSum;
    }
}ST_TOTAL_STACKE_NOTES_DENO_INFO;

// HCM控制信息
typedef struct tag_OPERATIONAL_INFO
{
    BOOL bArticle6Support;            // Article6是否支持，TRUE:支持 FALSE:不支持(默认值)
    BOOL bActiveVerificationLevel;    // BV验钞校验程度参数是否有效，对于验钞、存钞、出钞影响，TRUE: 有效 FALSE: 参数无效(默认值)
    BOOL bRejectUnfitNotesStore;      // 在存钞时是否将不适合流通的钞票存到废钞箱，如果有循环箱该值设置为TRUE将会报错，TRUE: 使用 FALSE: 不使用(默认值)

    BOOL bReportUnacceptDeno;         // 不可接受面额，面额代码是否作为UNKNOWEN面额返回 TRUE:返回UNKNOWN面额 FALSE:返回BV识别后的面额(默认值)

    BOOL bCashCountErrAsWarning;      // 验钞时发生错误是否当做警告返回，Article6支持时有效 TRUE:返回警告 FALSE:返回错误(默认值)
    BOOL bUseCorrectionFunction;      // 是否使用correction funtion, BV固件支持时参数有效  TRUE:使用(默认值) FALSE:不使用

    BOOL bDispErrAsWarning;           // 挖钞命令中如果发生钞箱空、Miss-feed、连续10张废钞、废钞总数大于等于100张时，是否返回警告。TRUE:返回警告(默认值) FALSE:返回错误
                                      // 当发生"ZERO BV memory Full"时总是返回Warning
    BOOL bShutterCheckBeforeDispense; // Dispense前是否做shutter check动作 TRUE:检查Shutter门(默认值) FALSE: 无Shutter动作

    BOOL bUseSNImageReadFunction;     // 是否使用冠字号图片读取功能 TRUE:使用(默认值) FALSE:不使用

    BOOL bStorDiffSizeNotesInDeposit; // 是否允许同一面额符合尺寸范围的面额钞票存入存款箱  TRUE: 允许  FALSE: 不允许(默认值)
    BOOL bUseSNReadFuncton;           // 是否使用冠字号读取功能 TRUE:使用(默认值) FALSE:不使用
    BOOL bCassMemoryOperation;        // 是否使用钞箱内存信息 TRUE: 使用(默认值) FALSE:不使用

    tag_OPERATIONAL_INFO()
    {
        bArticle6Support            = FALSE;
        bActiveVerificationLevel    = FALSE;
        bRejectUnfitNotesStore      = FALSE;
        bReportUnacceptDeno         = FALSE;
        bCashCountErrAsWarning      = FALSE;
        bUseCorrectionFunction      = FALSE;
        bDispErrAsWarning           = TRUE;
        bShutterCheckBeforeDispense = TRUE;
        bUseSNImageReadFunction     = TRUE;
        bStorDiffSizeNotesInDeposit = FALSE;
        bUseSNReadFuncton           = TRUE;
        bCassMemoryOperation        = TRUE;
    }
} ST_OPERATIONAL_INFO;

// HCM硬件配置
typedef struct tag_HW_CONFIG
{
    BOOL bETType;                     // ET类型(固定值)         TRUE:Type of ET(默认值)  FALSE: Type of ET1
    BOOL bHaveLane1;                  // 是否存在线路/通道1     TRUE:存在(默认值) FALSE: 不存在
    BOOL bHaveLane2;                  // 是否存在线路/通道2     TRUE:存在(默认值) FALSE: 不存在
    BOOL bHaveLane3;                  // 是否存在线路/通道3     TRUE:存在(默认值) FALSE: 不存在
    BOOL bHaveLane4;                  // 是否存在线路/通道4     TRUE:存在(默认值) FALSE: 不存在
    BOOL bHaveLane5;                  // 是否存在线路/通道5     TRUE:存在(默认值) FALSE: 不存在
    BOOL bURJBPoweroffCntlSupp;
    BOOL bUPPoweroffCntlSupp;
    BOOL bSelfCountRB5CfgAutoSwitch;     //是否支持自动精查RB5配置自动切替       //30-00-00-00(FS#0022)
    BOOL bSelfCountRB5SameDenoLimit;     //自动精查钞箱5同金种钞箱限定           //30-00-00-00(FS#0022)
    BOOL bSelfCountDiffDenoAbort;        //自动精查异金种是否中断               //30-00-00-00(FS#0022)
    BYTE byAcceptRoomsPriority[10];      //存入各钞箱优先级                    //30-00-00-00(FS#0022)
    BYTE byDispenseRoomsPriority[10];    //取出各钞箱优先级                    //30-00-00-00(FS#0022)

    tag_HW_CONFIG()
    {
        bETType           = TRUE;
        bURJBPoweroffCntlSupp = FALSE;
        bUPPoweroffCntlSupp = FALSE;
        bHaveLane1        = TRUE;
        bHaveLane2        = TRUE;
        bHaveLane3        = TRUE;
        bHaveLane4        = TRUE;
        bHaveLane5        = TRUE;
        bSelfCountRB5CfgAutoSwitch = TRUE;          //30-00-00-00(FS#0022)
        bSelfCountRB5SameDenoLimit = FALSE;         //30-00-00-00(FS#0022)
        bSelfCountDiffDenoAbort = FALSE;            //30-00-00-00(FS#0022)
        memset((char *)byAcceptRoomsPriority, 0, sizeof(byAcceptRoomsPriority));        //30-00-00-00(FS#0022)
        memset((char *)byDispenseRoomsPriority, 0, sizeof(byDispenseRoomsPriority));    //30-00-00-00(FS#0022)
	}
}ST_HW_CONFIG;

// 出钞时配钞结构
typedef struct tag_DISP_DENO
{
    DENOMINATION_CODE iDenoCode;      // 出钞面额
    int iCashNumber;                  // 出钞张数
    tag_DISP_DENO()
    {
        iDenoCode = DENOMINATION_CODE_00;
        iCashNumber = 0;
    }
} ST_DISP_DENO;

// 出钞时配钞结构
typedef struct tag_DISP_ROOM
{
    CASSETTE_ROOM_ID iCassID;         // 出钞钞箱ID
    int iCashNumber;                  // 出钞张数
    tag_DISP_ROOM()
    {
        iCassID = ID_ROOM_RESERVED;
        iCashNumber = 0;
    }
} ST_DISP_ROOM;

// 钞箱出钞错误信息
typedef struct tag_DISP_MISSFEED_ROOM
{
    BOOL bRoomMissFeed;                //出钞错误
    BOOL bRoomEmpty;                   //钞箱空
    BOOL bRoomContinuousRejects;       //钞箱连续挖出10张废钞
    BOOL bRoomTooManyRejects;          //钞箱连续挖钞100张废钞
    BOOL bArryRoom[MAX_CASSETTE_NUM];  //TRUE表示第几个钞箱单元出错
} ST_DISP_MISSFEED_ROOM;

//BV警告信息
typedef struct _BV_WARNING_INFO
{
    int    iWaringCode;                 //警告码
    USHORT usRecoveryCode;              //修复码
    UCHAR  ucPositionCode[16];          //位置代码
} ST_BV_WARNING_INFO;

// 钞票流向面额信息
typedef struct tag_MEDIA_INFORMATION_INFO
{
    ULONG ulBVInternalCounter;          //BV内部计数
    CASSETTE_ROOM_ID  iMediaInfoOrigin; //钞票来源
    CASSETTE_ROOM_ID  iMediaInfoDest;   //钞票流向
    DENOMINATION_CODE iMediaInfoDnoCode;//钞票面额
    BV_IMAGE_TYPE     iBVImage;         //是否存在冠字图片
    MEDIA_INFORMATION_REJECT_CAUSE iMediaInfoRejectCause;//拒钞发生原因
    NOTE_CATEGORY     iNoteCategory;    //钞票类别

    tag_MEDIA_INFORMATION_INFO()
    {
        ulBVInternalCounter = 0;
        iMediaInfoOrigin  = ID_CS;
        iMediaInfoDest    = ID_CS;
        iMediaInfoDnoCode = DENOMINATION_CODE_00;
        iBVImage          = BV_IMAGE_NOEXIST;
        iMediaInfoRejectCause = MEDIA_INFORMATION_REJECT_CAUSE_RESERVED;
        iNoteCategory     = NOTE_CATEGORY_UNKNOWN;
    }
} ST_MEDIA_INFORMATION_INFO;

// 用户内存的数据读取信息
typedef struct tag_USER_MEMORY_READ
{
    USER_MEMORY_TARGET iUserMemoryTaget;
    char szUserMemoryData[MAX_USER_MEMORY_DATA_LENGTH];
} ST_USER_MEMORY_READ;

// 冠字码信息结构
typedef struct tag_note_serialinfo
{
    unsigned long  dwBVInternalIndex;
    unsigned long  dwImgDataLen;
    char       cSerialNumber[MAX_SERIAL_LENGTH];
    BYTE       cSerialImgData[MAX_SNIMAGE_DATA_LENGTH];
    eNoteEdition    NotesEdition;
    tag_note_serialinfo()
    {
        dwBVInternalIndex = 0;
        dwImgDataLen = 0;
        memset(cSerialNumber, 0, sizeof(char)*MAX_SERIAL_LENGTH);
#if defined(SET_BANK_SXXH)
        memset(cSerialNumber, '*', sizeof(char)*10);
#endif
        memset(cSerialImgData, 0, sizeof(BYTE)*MAX_SNIMAGE_DATA_LENGTH);
        NotesEdition = eNoteEdition_unknown;
    }
} SNOTESERIALINFO, *LPSNOTESERIALINFO;

typedef struct tag_note_serialinfo_full_image
{
    unsigned long  dwBVInternalIndex;
    unsigned long  dwImgDataLen;
    BYTE       cFullImgData[MAX_FULLIMAGE_DATA_LENGTH];
    tag_note_serialinfo_full_image()
    {
        dwImgDataLen = 0;
        memset(cFullImgData, 0, sizeof(char)*MAX_FULLIMAGE_DATA_LENGTH);
    }
} SNOTESERIALINFOFULLIMAGE, *LPSNOTESERIALINFOFULLIMAGE;

// 连接类型定义
enum CONNECT_TYPE
{
    CONNECT_TYPE_UNKNOWN   = 0,
    CONNECT_TYPE_ZERO      = 1,
    CONNECT_TYPE_UR        = 2,
    CONNECT_TYPE_UR_ZERO   = 3,
};

//测试出钞结果数据结构体
enum TESTDISPFAILCAUSE
{
    NO_ERROR               = 0,
    EXECUTE_ABNORMAL       = 1,
    ALL_REJECT             = 2,
    CST_EMPTY              = 3,
    MISS_FEED              = 4,
    DENOMINATION_ERROR     = 5,
    ERROR_OTHER            = 6
};
typedef struct _TESTCASHUNITSOUTPUT
{
    WORD wNumOfCSPerRoom[2][5];                         //各room出钞到CS张数
    WORD wCountToRoom[2][5];                            //各room入钞张数
    WORD wCountFromRoom[2][5];                          //各room出钞张数
    WORD wRBBoxRJCount[2][5];                           //各room拒钞张数
    ST_TOTAL_STACKE_NOTES_DENO_INFO stPerDenomiNumofRJ; //拒钞各面值及张数
    TESTDISPFAILCAUSE eRoomError[2][5];                //各room测试出钞结果
}TESTCASHUNITSOUTPUT, *LPTESTCASHUNITSOUTPUT;

//BlackList
typedef struct tag_blacklist_info
{
    char cCurrency[4];
    char cValue[5];
    char cVersion;
    char cAction;
    char cSerialNumber[17];
} BLACKLIST_INFO, *LPBLACKLIST_INFO;

//------------------------ 接口类定义 ----------------------------------
class IURDevice
{
public:
    //功能：释放本接口
    virtual void Release() = 0;


    //功能：打开UR连接
    //输入：eConnectType, 连接类型
    //输出：
    //返回：0: 成功，非0: 失败
    //说明：打开ZVB和UR的USB连接
    virtual int OpenURConn() = 0;

    //功能：打开BV连接
    //输入：eConnectType, 连接类型
    //输出：
    //返回：0: 成功，非0: 失败
    //说明：打开ZVB和UR的USB连接
    virtual int OpenBVConn() = 0;

    //功能：关闭UR连接
    //输入：
    //输出：无
    //返回：0: 成功，非0: 失败
    //说明：关闭ZVB和HCM的USB连接
    virtual int CloseURConn() = 0;

    //功能：关闭BV连接
    //输入：
    //输出：无
    //返回：0: 成功，非0: 失败
    //说明：关闭ZVB和HCM的USB连接
    virtual int CloseBVConn() = 0;

    //功能1：获取HCM和ZVB固件版本信息并初始化机芯,在上电或Reboot后需要首先发送该命令
    //功能2：获取HCM和ZVB固件版本信息
    //输入：bNeedInitial 为TRUE使用功能1，FALSE使用功能2
    //输出：szFWVersion: 各固件程序的版本信息字符串，由使用该接口的测试程序进行解析
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int GetFWVersion(
    char szFWVersion[MAX_FW_DATA_LENGTH],
    USHORT &usLen,
    BOOL bNeedInitial = FALSE) = 0;

    //功能：设置校验级别，减少废钞率
    //输入：stSetVerificationLevel：需要减少废钞率设置为TRUE，否则设置为FALSE
    //      建议：bSetForCashCount设置为FALSE，bSetForStoreMoney和bSetForDispense设置为TRUE以减少废钞率
    virtual int SetVerificationLevel(SET_VERIFICATION_LEVEL stSetVerificationLevel) = 0;

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
    ) = 0;

    //功能：获得所有钞箱信息
    //输入：无
    //输出：pArrCassInfo:钞箱信息结构数组，数组大小为6;
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：120
    virtual int GetCassetteInfo(ST_CASSETTE_INFO pArryCassInfo[MAX_CASSETTE_NUM]) = 0;

    //功能：设置支持的面额代码
    //输入：pArryDENOCode: 需要支持的面额数组取值为DENOMINATION_CODE,已0结束
    //      例如支持面额 124,pArryDENOCode=12400000.., 支持面额4，pArryDENOCode=40000..,
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：20
    virtual int SetDenominationCode(
    const char pArryDENOCode[MAX_DENOMINATION_NUM]) = 0;

    //功能：设置HCM钞箱的配置和操作信息，设置命令执行后必须执行复位动作，否则动作指令报错
    //      如果HCM接受设置，设置完成后通过GetUnitInfo命令可读取设置的值，如果HCM不接受该值设置，该值则为0
    //输入：tTime: 系统时间
    //输入：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
    //输入：stOperationalInfo: UR控制设置
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
    virtual int SetUnitInfo(int iTotalCashInURJB,
                            const ST_OPERATIONAL_INFO &stOperationalInfo,
                            const ST_HW_CONFIG &stHWConfig,
                            const ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM],
                            const BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM],
                            const ST_BV_VERIFICATION_LEVEL &stCashCountLevel,
                            const ST_BV_VERIFICATION_LEVEL &stStoreMoneyLevel,
                            const ST_BV_VERIFICATION_LEVEL &stDispenseLevel,
                            const char usArryCassCashInPrioritySet[MAX_ROOM_NUM],        //30-00-00-00(FS#0022)
                            const char usArryCassCashOutPrioritySet[MAX_ROOM_NUM],       //30-00-00-00(FS#0022)
                            const BOOL bURJBSupp) = 0;

    //功能：获取HCM钞箱的配置和操作信息
    //输入：无
    //输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
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
    ) = 0;

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
    ) = 0;

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
    ) = 0;

    //功能：清除HCM模块中所有保存的日志数据
    //输入：无
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：120
    //      只有在新购买/更换HCM主板时可以使用
    virtual int EraseAllLogData() = 0;

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
#if defined(SET_BANK_CMBC)
    BOOL bRB5ToEscSupp,                                         //test#24
#endif
    int &iTotalCashInURJB,
    int &iNumStackedToCS,
    int &iNumStackedToESC,
    int pNumStackedToPerCass[MAX_CASSETTE_NUM],
    int &iNumCSFed,
    int &iNumESCFed,
    int pNumPerCassFed[MAX_CASSETTE_NUM],
    int &nRejectNotes,
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH)
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeUnfitNotesDenoInfo,    //test#13
    ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoInfo,   //test#13
    BYTE  ucProhibited,                                             //test#13
#endif
    USHORT &usFedNoteCondition,
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
#if defined(SET_BANK_SXXH)
	,
    BOOL bFoceMode = FALSE
#endif
    ) = 0;

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
    ) = 0;

    //功能：存入钞票，将钞票从ESC暂存箱送入到指定的各钞箱
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
    ) = 0;

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
    ) = 0;

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
    ) = 0;

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
    ) = 0;

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
    ) = 0;

    //功能：关闭Shutter门
    //输入：bForcible:是否TRUE强制关闭SHUTTER门，尝试关闭10次
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int CloseShutter(char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],
                             BOOL bForcible = FALSE
                            ) = 0;

    //功能：打开Shutter门
    //输入：无
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int OpenShutter(char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]) = 0;

#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH)
    //功能：关闭Shutter门
    //输入：bForcible:是否TRUE强制关闭SHUTTER门，尝试关闭10次
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int CloseCashInShutter(char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],     //test#11
        BOOL bForcible = FALSE                                                         //test#11
        ) = 0;

    //功能：打开入金口Shutter门
    //输入：无
    //输出：pMaintenanceInfo: 维护信息
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int OpenCashInShutter(char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]) = 0;  //test#11

    //功能：获取入金口Shutter门状态
    //输入：无
    //输出：无
    //返回：0 门关闭，1 门打开
    //说明：执行命令的最大时间(秒)：120
    virtual int GetCashInShutterStatus() = 0;                       //test#13
#endif

    //功能：开始下载固件
    //输入：无
    //返回：0: 成功，非0: 失败
    //说明：进入下载模式成功后，UR不能响应其他任何命令
    //说明：执行命令的最大时间(秒)：210
    virtual int ProgramDownloadStart() = 0;

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
    USHORT usDataLen) = 0;

    //功能：结束下载固件
    //输入：无
    //返回：0 成功，>0 警告，<0 失败
    //说明：执行命令的最大时间(秒)：210
    virtual int ProgramDownloadEnd() = 0;

    //功能：断开与ATM PC端口通讯连接，并重启模块到上电时的初始状态
    //输入：无
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：210
    virtual int Reboot() = 0;

    //功能：维护检测、诊断机芯(暂不实现)
    //      使用该命令前需要进行一次复位，完成检测后需要调用复位恢复正常操作
    //输入：
    //输出：
    //返回：0: 成功，非0: 失败
    //说明：执行命令的最大时间(秒)：180
    //virtual int Test() = 0;

    //功能：写数据到用户内存区域
    //输入：iUserMemoryTaget: 写入的内存区域,不可选择USER_MEMORY_TARGET_ALLCASS
    //输入：szUserMemoryData: 写入的数据，最大写入128字节
    //输入：usDataLen: 数据长度
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：120
    virtual int UserMemoryWrite(
    USER_MEMORY_TARGET iUserMemoryTaget,
    const char szUserMemoryData[MAX_USER_MEMORY_DATA_LENGTH],
    USHORT usDataLen) = 0;

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
    ) = 0;

    //功能：查询BV模块信息
    //输入：无
    //输出：stBVWarningInfo: BV警告信息数组
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int QueryBVStatusSense(
    ST_BV_WARNING_INFO stBVWarningInfo[MAX_BV_WARNING_INFO_NUM]
    ) = 0;

    //功能：获取冠字码相关信息
    //输入：dwIndex: 获取冠字码序号，从0开始
    //输出：lpNoteSerialInfo: 保存冠字码信息的结构指针
    //返回：0：成功，其他：失败
    //说明：获取冠字码数据前需要调用BVCommStart函数，全部获取结束后调用BVCommEnd函数，结束与BV通信
    virtual long GetNoteSeiralInfo(DWORD dwIndex, LPSNOTESERIALINFO lpNoteSerialInfo) = 0;

    //功能：获取冠字码全幅图像相关信息
    //输入：dwIndex: 获取冠字码序号，从0开始
    //输出：lpNoteSerialInfoFullImage: 保存冠字码信息的结构指针
    //返回：0：成功，其他：失败
    //说明：获取冠字码数据前需要调用BVCommStart函数，全部获取结束后调用BVCommEnd函数，结束与BV通信
    virtual long GetNoteSeiralInfoFullImage(DWORD dwIndex, LPSNOTESERIALINFOFULLIMAGE lpNoteSerialInfoFullImage) = 0;

    //功能：获取钞票流向面额信息，包括到了何处、面值、版次等
    //输入：ucMediaInfoNum: 查询的钞票信息号，取值为1、2、3、4，每次调用自增1，第一次调用值是1
    //输出：usNumNotes: 本次返回的钞票信息数，一次最多返回512个钞票信息
    //输出：usNumTotalNotes: 产生钞票信息总数
    //输出：aryMediaInfo: 返回的钞票信息保存位置数组
    //返回：0:  成功 后面没有数据了
    //      >0: 返回插入aryMediaInfo中的对象个数.如果个数等于512表示后面还有数据，请继续调用本函数获取其余的信息
    //      <0: 命令执行失败
    //说明：执行命令的最大时间(秒)：20
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH)
    virtual long GetMediaInformation(
        char ucMediaInfoNum,
		USHORT	&usNumNotes,
		USHORT  &usNumTotalNotes,
		ST_MEDIA_INFORMATION_INFO arryMediaInfo[MAX_MEDIA_INFO_NUM]
		) = 0;
#else
    virtual long GetMediaInformation(
        BYTE byMediaInfoType,                               //30-00-00-00(FS#0022)
        char ucMediaInfoNum,
        USHORT	&usNumNotes,
        USHORT  &usNumTotalNotes,
        ST_MEDIA_INFORMATION_INFO arryMediaInfo[MAX_MEDIA_INFO_NUM]
        ) = 0;
#endif

    //功能：PC与ZERO BV通信开始，通知HCM不访问BV，获取冠字码信息前调用
    //输入：无
    //0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int BVCommStart() = 0;

    //功能：PC与ZERO BV通信结束，获取冠字码信息结束后调用
    //输入：无
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int BVCommEnd() = 0;

    //功能：准备进入一下笔交易
    //      在取款、存款交易结束后调用该命令
    //      HCM2检测所有单元是否在原始位置并且无钞票残留
    //      HCM2关掉传感器以及设备，切换到节能模式
    //输入：无
    //0 成功，>0 警告，<0 失败；见返回码定义
    //说明：
    virtual int PreNextTransaction() = 0;

    //功能：开始存取款交易
    //      在取款、存款交易开始前调用该命令
    //      HCM2打开传感器以及设备，从节能模式中恢复
    //输入：无
    //0 成功，>0 警告，<0 失败；见返回码定义
    //说明：
    virtual int StartTransaction() = 0;

    //功能：获得设备状态信息
    //输入：无
    //输出：stDevStatusInfo
    //返回：无
    virtual void GetDevStatusInfo(ST_DEV_STATUS_INFO &stDevStatusInfo) = 0;

    // 功能：获取错误返回码
    // 输入：无
    // 输出：无
    // 返回：8字节常量字符串
    // 说明：若指令正常结束则返回空字串("")
    //       若异常结束则返回8字节的错误码字符串，如："05130A03"
    //       若警告结束则返回8字节警告码字符串，  如："81xxxxxx"
    virtual void GetLastErrDetail(ST_ERR_DETAIL &stErrDetail) = 0;

    // 功能：将响应的错误或警告码转换为对应的错误描述
    // 输入：arycErrCode: 响应的错误代号,请传入GetLastErrCode()返回的字符串
    // 输出：无
    // 返回：成功：返回arycErrCode相关的错误描述，
    //       失败：如arycErrCode错误代号未知则返回空串：""；如参数为NULL返回NULL
    virtual const char *GetErrDesc(const char *arycErrCode) = 0;

    //功能：获取所有感应器的亮灭状态
    //输入：iLen: pszLightData所分配的内存空间，该值需大于58
    //输出：pszLightData: 所有感应器的亮灭状态数据，58字节，每1 bit表示一个感应器
    //返回：0 成功，>0 警告，<0 失败；见返回码定义
    //说明：执行命令的最大时间(秒)：20
    virtual int ReadAllSensorLight(char *pszLightData, int& iLen) = 0;

    //功能：调整BV验钞级别
    //输入：ucVerificationLevel : 0~255表示 检验级别，数值越高越宽松
    //输出：无
    //返回：0 成功，>0 警告，<0 失败
    virtual int SetBVVerificationLevel(char ucVerificationLevel)  = 0;

    //功能：对变造币进行拒钞处理
    //输入：
    //输出：
    //返回：0 成功，>0 警告，<0 失败
    virtual int  SetRejectCounterfeitNote() = 0;

    //功能：设置不适合流通人民币验钞处理级别
    //输入：CmdType ：设置后即将执行的操作动作; bSevereModel: 是否使用更严格的模式
    //输出：
    //返回：0 成功，>0 警告，<0 失败
    virtual int  SetUnfitNoteVerifyLevel(eCMDType CmdType, BOOL bSevereModel) = 0;

    //功能：出钞测试
    virtual int  TestCashUnits(VALIDATION_MODE iValidateMode,
                               BYTE byRoomExclusion[2][5],
                               BYTE byRetryCnt,
                               TESTCASHUNITSOUTPUT &stTestCashUnitsOutput)// = 0;     //30-00-00-00(FS#0007)
    {
        return 0;
    }

    virtual int SelfCount(VALIDATION_MODE iValidateMode,
                          bool bExclusionRoom[5][2],
                          ULONG ulNumSelfCountPerCass[MAX_CASSETTE_NUM],
                          ULONG ulNumFeedPerCass[MAX_CASSETTE_NUM],
                          ST_TOTAL_STACKE_NOTES_DENO_INFO &stRejectDenoDstInfo,
                          ST_TOTAL_STACKE_NOTES_DENO_INFO &stRejectDenoSrcInfo,
                          const ST_BV_DEPENDENT_MODE stBVDependentMode,
                          bool bContinueAfterBVDataFull = false){ 				//30-00-00-00(FS#0022)
        return 0;
    }
	//功能：获取入金口Com值
    virtual int  SetComVal(const char * strCashInShutterPort)// = 0;
	{
        return 0;
    }

    //功能：设置BV鉴别设定
    virtual int SetZeroBVSettingInfo(BYTE byCmdType, BYTE byBVSettingLevel, LPCSTR lpBVSettingInfo) = 0;

    //功能：BlackList
    virtual int SetBlackList(std::vector<BLACKLIST_INFO>& vctBlackListInfoList) = 0;
protected:

};

//功能：创建设备对象
//参数：pDevice：返回设备实例
//返回：>=0成功，<0失败
extern "C" Q_DECL_EXPORT int  CreateURDevice(const char *pName, IURDevice *&pDevice);

#if defined(DEVUR2_LIBRARY)
#  define DEVUR2SHARED_EXPORT Q_DECL_EXPORT
#else
#  define DEVUR2SHARED_EXPORT Q_DECL_IMPORT
#endif
