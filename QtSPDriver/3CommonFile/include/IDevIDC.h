/***************************************************************
* 文件名称: IDevIDC.h
* 文件描述: 声明读卡器底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2019年6月15日
* 文件版本: 1.0.0.1
****************************************************************/
#if !defined(SPBuild_OPTIM)     // 非优化工程

#pragma once

#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "IDevCRD.h"
#include "XFSIDC.H"
#include <string.h>
//////////////////////////////////////////////////////////////////////////
#if defined(IDEVIDC_LIBRARY)
#define DEVIDC_EXPORT     Q_DECL_EXPORT
#else
#define DEVIDC_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////

//成功
#define ERR_IDC_SUCCESS         (0)     // 操作成功
//警告
#define ERR_IDC_INSERT_TIMEOUT  (1)     // 进卡超时
#define ERR_IDC_USER_CANCEL     (2)     // 用户取消进卡
#define ERR_IDC_DEVICE_REOPEN   (3)     // 设备重连         //30-00-00-00(FT#0019)

//错误
#define ERR_IDC_COMM_ERR        (-1)    // 通讯错误
#define ERR_IDC_JAMMED          (-2)    // 读卡器堵卡
#define ERR_IDC_OFFLINE         (-3)    // 读卡器脱机
#define ERR_IDC_NOT_OPEN        (-4)    // 读卡器没有打开
#define ERR_IDC_RETAINBINFULL   (-5)    // 回收箱满
#define ERR_IDC_HWERR           (-6)    // 硬件故障
#define ERR_IDC_STATUS_ERR      (-7)    // 读卡器其他状态出错
#define ERR_IDC_UNSUP_CMD       (-31)   // 不支持的指令
#define ERR_IDC_PARAM_ERR       (-32)   // 参数错误
#define ERR_IDC_READTIMEOUT     (-33)   // 读数据超时
#define ERR_IDC_WRITETIMEOUT    (-34)   // 写数据超时
#define ERR_IDC_READERROR       (-35)   // 读卡错
#define ERR_IDC_WRITEERROR      (-36)   // 写卡错
#define ERR_IDC_INVALIDCARD     (-37)   // 无效磁卡，有磁但磁道数据无效
#define ERR_IDC_NOTRACK         (-38)   // 非磁卡，未检测到磁道
#define ERR_IDC_CARDPULLOUT     (-39)   // 接收卡时，卡被拖出
#define ERR_IDC_CARDTOOSHORT    (-40)   // 卡太短
#define ERR_IDC_CARDTOOLONG     (-41)   // 卡太长
#define ERR_IDC_WRITETRACKERROR (-42)   // 写磁道错误
#define ERR_IDC_ACTIVEFAILED    (-43)   // 卡激活失败
#define ERR_IDC_CHIPIOFAILED    (-44)   // CHIIP失败
#define ERR_IDC_RESP_NOT_COMP   (-45)   // 应答数据不完整      //30-00-00-00(FT#0051)
//IC卡相关
#define ERR_IDC_PROTOCOLNOTSUPP (-51)   // 不支持的IC通讯协议
#define ERR_IDC_ICRW_ERROR      (-52)   // IC卡读写时被拖走
#define ERR_IDC_NO_DEVICE       (-53)   // 指定的设备不存在
#define ERR_IDC_OTHER           (-54)   // 其它错误
#define ERR_IDC_USERERR         (-55)   // 检测到卡但进卡失败次数超限
#define ERR_IDC_TAMPER          (-56)   // 防盗嘴异物传感器检测到异物
#define ERR_IDC_CONFLICT        (-57)   // 卡冲突
#define ERR_IDC_MULTICARD       (-58)   // 检测到多张卡
#define ERR_IDC_NOCHIP          (-59)   // 检测到卡无芯片      //30-00-00-00(FT#0058)

#define INFINITE                (0xFFFFFFFF)
#define MAX_LEN_FWVERSION       (255)   // 最大版本信息长度
#define MAX_LEN_ATRINFO         (33)    // 最大ATR信息长度

// 读卡器通信协议
#define CRT_350NJ11_USB         0       // 适用于同型号读卡器,兼容CRT-3500NJ10
#define CRT_591HDR1_COM         1       // 适用于同型号发卡器,兼容读卡器CRT-350NJ10,应答多一位发卡器状态值


/////////////////////////////////////////////////////////////////////////
// 定义卡动作
enum CardAction
{
    CARDACTION_NOACTION   = 1,   // 移动并保持
    CARDACTION_NOMOVEMENT = 2,   // 不移动并保持
    CARDACTION_EJECT      = 3,   // 弹卡
    CARDACTION_RETRACT    = 4,   // 吞卡
};

// 定义抖动功能动作
enum WobbleAction
{
#if defined(SET_BANK_CMBC)
    WOBBLEACTION_STOP     = 0,      // 去除抖动功能
    WOBBLEACTION_START    = 1,      // 使能抖动
    WOBBLEACTION_NOACTION = 2,      // 不做操作
#else
    WOBBLEACTION_START    = 1,      // 使能抖动
    WOBBLEACTION_STOP     = 2,      // 去除抖动功能
    WOBBLEACTION_NOACTION = 3,      // 保持,不做操作
#endif
};

// IC卡激活方式定义，包含冷复位和热复位
enum ICCardReset
{
    ICCARDRESET_COLD    = 1,      // 冷复位
    ICCARDRESET_WARM    = 2,      // 热复位
};

// 读卡器在主机与IC卡之间接收与发送数据所用的协议的定义
enum ICCardProtocol
{
    ICCARD_PROTOCOL_T0  = 1,      // T=0 protocol
    ICCARD_PROTOCOL_T1  = 2,      // T=1 protocol
};

// LED灯类型
enum LedType
{
    LEDTYPE_YELLOW = 0, //黄灯
    LEDTYPE_BLUE   = 1, //蓝灯
    LEDTYPE_GREEN  = 2, //绿灯
    LEDTYPE_RED    = 3, //红灯
};

// LED灯控制
enum LedAction
{
    LEDACTION_OPEN  = 0, //打开
    LEDACTION_CLOSE = 1, //关闭
    LEDACTION_FLASH = 2, //闪烁
};

//定义卡的位置状态
enum IDC_IDCSTAUTS
{
    IDCSTAUTS_NOCARD                = 0, //无卡
    IDCSTAUTS_ENTERING              = 1, //门口
    IDCSTAUTS_INTERNAL              = 2, //内部
    IDCSTAUTS_ICC_PRESS             = 3, //IC卡被压下
    IDCSTAUTS_ICC_ACTIVE            = 4, //IC卡被激活
    IDCSTAUTS_ICC_POWEROFF          = 5, //IC POWEROFF
    IDCSTATUS_UNKNOWN               = 10,//未知状态
};
//////////////////////////////////////////////////////////////////////////
enum EJECTERR
{
    EJECT_UNKNOWN   = 0,
    EJECT_SUCCESS   = 1,
    EJECT_FAILED    = 2,
};

enum MEDIAERR
{
    MEDIA_UNKOWN    = 0,
    MEDIA_NOTAKEN   = 1,
    MEDIA_TAKEN     = 2,
};

// 保存监控命令类型信息
enum MONITOR_TYPE
{
    MONITOR_INTAKE      = 0,        //进卡
    MONITOR_WITHDRAWAL  = 1,
    MONITOR_REINTAKE    = 2,    //重进卡
};

// 防盗嘴
enum TANMPER_SENSOR
{
    TANMPER_SENSOR_NOT_AVAILABLE    =   0,  // 0 0
    TANMPER_SENSOR_OFF              =   1,
    TANMPER_SENSOR_ON               =   2,

};
//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_idc_status
{
    WORD wDevice;               // 设备状态
    char szErrCode[8];          // 三位的错误码

    tag_dev_idc_status() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_dev_idc_status)); }
} DEVIDCSTATUS;
// 保存磁道信息结构体
typedef struct _track_Detail
{
    char szTrack[128];
    bool bTrackOK;
    _track_Detail()
    {
        memset(szTrack, 0, sizeof(char) * 128);
        bTrackOK = false;
    }
} STTRACKDETAIL, *LPSTTRACKDETAIL;

typedef struct _track_info
{
    STTRACKDETAIL TrackData[3]; //77,38,105
} STTRACK_INFO, *LPSTTRACK_INFO;

typedef struct tag_iomc_idc_param
{
    DWORD       bWaitInsertCardIntervalTime;    //等待插卡间隔时间
    USHORT      bSupportPredictIC;              //IC卡功能支持
    USHORT      bReTakeIn;                      //启用重进卡功能
    DWORD       bFraudEnterCardTimes;           //进卡重试次数
    BOOL        bNeedFraudProtect;              //防逗卡功能
    INT         bFraudProtectTime;              //防逗卡保護時間
    DWORD       bTamperSensorSupport;           //防盜嘴功能支持
    int         iPowerOffAction;                //读卡器掉电时卡的处理方式          //30-00-00-00(FT#0030)
    BOOL        bSkimmingSupport;               //读卡器卡口异物检测支持            //30-00-00-00(FS#0014)
    DWORD       dwSkimmingMonitorInterval;      //异物检测间隔时间,单位: 秒         //30-00-00-00(FS#0014)
    DWORD       dwSkimmingErrorDuration;        //异物检测判故障持续故障时间,单位: 秒 //30-00-00-00(FS#0014)
    DWORD       dwSkimmingNormalDuration;       //异物检测判恢复持续正常时间,单位: 秒 //30-00-00-00(FS#0014)
    int         iESUMode;                       //磁干扰模式　　　　　　　　　　　   //30-00-00-00(FT#0064)
    tag_iomc_idc_param() { Clear(); }
    void Clear()
    {
        memset(this, 0x00, sizeof(tag_iomc_idc_param));
        bSkimmingSupport = FALSE;                                               //30-00-00-00(FS#0014)
        dwSkimmingMonitorInterval = 3;                                          //30-00-00-00(FS#0014)
        dwSkimmingErrorDuration = 60;                                           //30-00-00-00(FS#0014)
        dwSkimmingNormalDuration = 60;                                          //30-00-00-00(FS#0014)
    }
} IOMCIDCPARAM;
//////////////////////////////////////////////////////////////////////////
//接口类定义
struct  IDevIDC
{
    // 释放接口
    virtual void Release() = 0;
    /************************************************************
    ** 功能: 打开与设备的连接
    ** 输入: pMode: 自定义OPEN参数字串,格式自定义
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Open(const char *pMode) = 0;

    /************************************************************
    ** 功能: 设备初始化
    ** 输入: eActFlag : 卡动作
            nNeedWobble: 抖动功能
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Init(CardAction eActFlag, WobbleAction nNeedWobble = WOBBLEACTION_NOACTION)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 关闭与设备的连接
    ** 输入: 无
    ** 输出: 无
    ** 返回: 无
    ************************************************************/
    virtual void Close() = 0;

    /************************************************************
    ** 功能: 吞卡
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int EatCard() = 0;

    /************************************************************
    ** 功能: 弹出卡
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int EjectCard() = 0;

    /************************************************************
    ** 功能: 进卡
    ** 输入: ulTimeOut:进卡超时,单位毫秒, 无限超时: INFINITE
             Magnetic: 是否是磁条卡
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int AcceptCard(unsigned long ulTimeOut, bool Magnetic = true) = 0;

    /************************************************************
    ** 功能: 停止上一次读卡
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int CancelReadCard() = 0;

    /************************************************************
    ** 功能: 写磁道信息
    ** 输入: stTrackInfo 保存对应磁道信息
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int WriteTracks(const STTRACK_INFO &stTrackInfo) = 0;

    /************************************************************
    ** 功能: 读磁道信息
    ** 输入: 无
    ** 输出: stTrackInfo 保存对应磁道信息
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ReadTracks(STTRACK_INFO &stTrackInfo) = 0;

    /************************************************************
    ** 功能: 检测卡
    ** 输入: 无
    ** 输出: IDCstatus
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int DetectCard(IDC_IDCSTAUTS &IDCstatus) = 0;

    /************************************************************
    ** 功能: 获取固件版本
    ** 输入: 无
    ** 输出: pFWVersion: 保存固件版本
             uLen: 版本有效信息长度
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int GetFWVersion(char pFWVersion[MAX_LEN_FWVERSION], unsigned int &uLen) = 0;

    /************************************************************
    ** 功能: 设置回收计数器
    ** 输入: pszCount: 设置值(范围在"00" ~ "99"之间)
    ** 输出:
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual INT SetRecycleCount(LPCSTR pszCount) = 0;

    /*--------------------- 以下IC卡操作接口-------------------------*/
    /************************************************************
    ** 功能: 压卡
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ICCPress() = 0;

    /************************************************************
    ** 功能: 卡释放
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ICCRelease() = 0;

    /************************************************************
    ** 功能: 卡激活
    ** 输入: 无
    ** 输出: pATRInfo:保存ATR信息
            uATRLen: ATR有效信息长度
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ICCActive(char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen) = 0;

    /************************************************************
    ** 功能: 卡激活
    ** 输入: eResetFlag:激活方式
    ** 输出: pATRInfo:保存ATR信息
             uATRLen: ATR有效信息长度
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ICCReset(ICCardReset eResetFlag, char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen) = 0;

    /************************************************************
    ** 功能: 移动卡到MM位置
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ICCMove() = 0;

    /************************************************************
    ** 功能: 反激活
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ICCDeActivation() = 0;

    /************************************************************
    ** 功能: 数据接收与发送
    ** 输入: eProFlag:表示读卡器在主机与IC卡之间接收与发送数据所用的协议
    ** 输出: pInOutData:输入数据缓存
             nInOutLen: 输入数据有效长度
    ** 输出: pInOutData:输出数据缓存
             nInOutLen: 输出数据有效长度
             T0协议时InLen范围为: 4~261，OutLen范围为: 2~258；
             T1协议时InLen范围为: 4~360，OutLen范围为: 2~320；
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz) = 0;

    /************************************************************
    ** 功能: 设置非接LED
    ** 输入: eFlagLedType:LED灯类型
             eFlagLedAct :LED灯控制
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct) = 0;

    /************************************************************
    ** 功能: 设置非接轰鸣器
    ** 输入: 蜂鸣时间(s), 默认５秒
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int SetRFIDCardReaderBeep(unsigned long ulTime = 5) = 0;

    /************************************************************
    ** 功能: 获取异物检测状态
    ** 输入: 无
    ** 输出: 无
    ** 返回: TRUE:异物检测触发　FALSE:异物检测未触发
    *************************************************************/
    virtual BOOL GetSkimmingCheckStatus()// = 0;              //30-00-00-00(FS#0014)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /*--------------------- 以下CRD发卡相关操作接口-------------------------*/
    // 适用于读卡器与发卡器使用同一连接方式及接口的情况,否则使用DevCRD操作
    /************************************************************
    ** 功能: 读取设备状态
    ** 输入: 无
    ** 输出: stStat　设备状态信息
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int GetDevStat(STCRDDEVSTATUS &stStat)
    {
        return ERR_CRD_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 发卡
    ** 输入: 无
    ** 输出: nUnitNo　单元编号
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int DispenseCard(const int nUnitNo)
    {
        return ERR_CRD_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 获取版本号
    ** 输入: wType 入参 获取类型(1DevCRD版本/2固件版本/3设备软件版本/4其他)
    ** 输出: szVer 回参 数据
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual void GetVersion(char* szVer, long lSize, WORD wType)
    {
        return;
    }
    /************************************************************
    ** 功能: 设置数据
    ** 输入: vData 入参
    **      wDataType 入参 设置类型(参考宏定义)
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int SetData(void *vData, WORD wDataType = 0)
    {
        return ERR_CRD_UNSUP_CMD;
    }
};

extern "C" DEVIDC_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev);

#else
//****************************************************************************
//  以下为IDevIDC接口优化版本
//****************************************************************************

#pragma once

#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
//#include "IDevCRD.h"
#include "XFSIDC.H"
#include "IDevDEF.h"
#include <string.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
#if defined(IDEVIDC_LIBRARY)
#define DEVIDC_EXPORT     Q_DECL_EXPORT
#else
#define DEVIDC_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////

//****************************************************************************
// 错误码相关定义
//****************************************************************************
// 成功
#define IDC_SUCCESS                 (0)     // 操作成功

// 警告
#define ERR_IDC_INSERT_TIMEOUT      (1)     // 进卡超时
#define ERR_IDC_USER_CANCEL         (2)     // 用户取消进卡
#define ERR_IDC_DEVICE_REOPEN       (3)     // 设备重连
#define ERR_IDC_CMD_RUNNING         (4)     // 有命令执行中
#define ERR_IDC_INCARD_AFTER        (5)     // 后出口进卡

// 通用错误
#define ERR_IDC_LIBRARY             (-1)    // 动态库加载失败
#define ERR_IDC_PARAM_ERR           (-2)    // 参数错误
#define ERR_IDC_UNSUP_CMD           (-3)    // 不支持的指令
#define ERR_IDC_DATA_FAIL           (-4)    // 数据错误
#define ERR_IDC_TIMEOUT             (-5)    // 超时
#define ERR_IDC_USER_ERR            (-6)    // 用户使用错误
#define ERR_IDC_OTHER               (-98)   // 其它错误(知道错误原因,不细分类别)
#define ERR_IDC_UNKNOWN             (-99)   // 未知错误(不知原因的错误)

// 通信错误
#define ERR_IDC_COMM_ERR            (-100)  // 通讯错误
#define ERR_IDC_READ_ERR            (-101)  // 读数据错误
#define ERR_IDC_WRITE_ERR           (-102)  // 写数据错误
#define ERR_IDC_READ_TIMEOUT        (-103)  // 读数据超时
#define ERR_IDC_WRITE_TIMEOUT       (-104)  // 写数据超时
#define ERR_IDC_RESP_ERR            (-105)  // 应答错误
#define ERR_IDC_RESP_NOT_COMP       (-106)  // 应答数据不完整
#define ERR_IDC_COMM_RUN            (-107)  // 通信命令执行错误

// 设备错误
#define ERR_IDC_DEV_NOTFOUND        (-201)  // 设备不存在
#define ERR_IDC_DEV_NOTOPEN         (-202)  // 设备没有打开
#define ERR_IDC_DEV_OFFLINE         (-203)  // 设备脱机
#define ERR_IDC_DEV_HWERR           (-204)  // 设备硬件故障
#define ERR_IDC_DEV_STAT_ERR        (-205)  // 设备状态出错

// 介质错误
#define ERR_IDC_MED_NOTFOUND        (-300)  // 介质未找到
#define ERR_IDC_MED_SHORT           (-301)  // 介质太短
#define ERR_IDC_MED_LONG            (-302)  // 介质太长
#define ERR_IDC_MED_PULLOUT         (-303)  // 接收介质时,介质被强制拖出
#define ERR_IDC_MED_MULTICARD       (-304)  // 检测到多张卡
#define ERR_IDC_MED_CONFLICT        (-305)  // 介质冲突
#define ERR_IDC_MED_IN_OVER         (-306)  // 介质吸入失败次数超限
#define ERR_IDC_MED_JAMMED          (-307)  // 介质阻塞
#define ERR_IDC_MED_STAT_ERR        (-308)  // 介质状态错误
#define ERR_IDC_MED_INV             (-309)  // 无效介质

// 磁条卡错误
#define ERR_IDC_MAG_DATA_INV        (-400)  // 无效磁卡，有磁但磁道数据无效
#define ERR_IDC_MAG_NOTRACK         (-401)  // 非磁卡，未检测到磁道

// 芯片卡错误
#define ERR_IDC_CHIP_NOTINV         (-500)  // 无效芯片卡或卡无芯片
#define ERR_IDC_CHIP_ACTIVE_FAIL    (-501)  // 介质激活失败
#define ERR_IDC_CHIP_IO_FAIL        (-502)  // CHIIP失败
#define ERR_IDC_CHIP_PORT_NOSUP     (-503)  // 不支持的IC通讯协议
#define ERR_IDC_CHIP_RWERROR        (-504)  // 芯片卡读写错误
#define ERR_IDC_CHIP_PULLOUT        (-505)  // 芯片卡操作时被强制拖走

// 回收错误
#define ERR_IDC_RETAIN_NOSUP        (-600)  // 设备不支持回收
#define ERR_IDC_RETAIN_FULL         (-601)  // 设备回收满

// 其他错误
#define ERR_IDC_DETEC_TAMPER        (-9900) // 检知有异物
#define ERR_IDC_DETEC_DESTRUC       (-9901) // 检知有破坏行为
#define ERR_IDC_DETEC_FRAUD         (-9902) // 检知有欺欺诈行为


//***************************************************************************
// 使用 IDevDEF.h 通用定义部分
// 1. SetData/GetData 数据类型 (0~50为共通使用, 50以上为各模块自行定义)
// 2. GetVersion 数据类型
// 3. 设备打开方式结构体变量, 适用于 SET_DEV_OPENMODE 参数传递
//***************************************************************************

//****************************************************************************
// 设备状态相关定义
//****************************************************************************
// 介质控制动作
enum MEDIA_ACTION
{
    MEDIA_NOTACTION                 = 0,    // 无动作
    MEDIA_EJECT                     = 1,    // 介质退出
    MEDIA_RETRACT                   = 2,    // 介质回收
    MEDIA_MOVE                      = 3,    // 介质移动
    MEDIA_ACCEPT_IC                 = 4,    // IC卡进卡
    MEDIA_ACCEPT                    = 5,    // 磁条卡进卡
    MEDIA_ICPRESS                   = 6,    // IC卡压卡
    MEDIA_ICRELEASE                 = 7,    // IC卡释放
    MEDIA_ACCEPT_IDCARD             = 8,    // 身份证进卡
};

//　Status.Device返回状态(引用IDevDEF.h中已定义类型)
typedef EN_DEVICE_STATUS    DEVIDC_DEVICE_STATUS;

//　status.Media返回状态(介质状态)
enum DEVIDC_MEDIA_STATUS
{
    MEDIA_STAT_PRESENT              = 0,    // 通道内有介质
    MEDIA_STAT_NOTPRESENT           = 1,    // 通道内无介质
    MEDIA_STAT_JAMMED               = 2,    // 通道内有介质且被夹住
    MEDIA_STAT_NOTSUPP              = 3,    // 不支持检测通道内是否有介质
    MEDIA_STAT_UNKNOWN              = 4,    // 通道内介质状态未知
    MEDIA_STAT_ENTERING             = 5,    // 介质在出口
    MEDIA_STAT_LATCHED              = 6,    // 介质存在但被锁定
};

//　status.RetainBin返回状态(回收盒状态)
enum DEVIDC_RETBIN_STATUS
{
    RETBIN_STAT_OK                  = 0,    // 未满
    RETBIN_STAT_NOTSUPP             = 1,    // 不支持
    RETBIN_STAT_FULL                = 2,    // 满
    RETBIN_STAT_HIGH                = 3,    // 将满
    RETBIN_STAT_MISSING             = 4,    // 丢失
};

//　status.Security返回状态(安全单元状态)
enum DEVIDC_SECURI_STATUS
{
    SEC_STAT_OPEN                   = 0,    // 已打开可操作
    SEC_STAT_NOTSUPP                = 1,    // 不支持
    SEC_STAT_NOTREADY               = 2,    // 未准备好
};

//　status.ChipPower返回状态(IC芯片状态)
enum DEVIDC_CHIP_STATUS
{
    CHIP_STAT_ONLINE                = 0,    // 存在并通电中
    CHIP_STAT_POWEREDOFF            = 1,    // 存在未通电
    CHIP_STAT_BUSY                  = 2,    // 存在并通电读写中
    CHIP_STAT_HWERROR               = 3,    // 存在且故障中
    CHIP_STAT_NOCARD                = 4,    // 无卡
    CHIP_STAT_NOTSUPP               = 5,    // 不支持
    CHIP_STAT_UNKNOWN               = 6,    // 状态未知
};

// 设备状态结构体
typedef struct ST_DEV_IDC_STATUS   // 处理后的设备状态
{
    WORD wDevice;               // 设备状态(参考enum DEVIDC_DEVICE_STATUS)
    WORD wMedia;                // Media状态(参考enum DEVIDC_MEDIA_STATUS)
    WORD wRetainBin;            // 回收盒状态(参考enum DEVIDC_RETBIN_STATUS)
    WORD wSecurity;             // 安全单元状态(参考enum DEVIDC_SECURI_STATUS)
    WORD wChipPower;            // IC芯片状态(参考enum DEVIDC_CHIP_STATUS)
    WORD wCards;                // 回收卡数
    CHAR szErrCode[32];         // 错误码
    WORD wOtherCode[16];        // 其他状态值,用于非标准WFS/未定义值的返回

    ST_DEV_IDC_STATUS()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(ST_DEV_IDC_STATUS));
        wDevice = DEVICE_STAT_OFFLINE;
        wMedia = MEDIA_STAT_UNKNOWN;
        wRetainBin = RETBIN_STAT_NOTSUPP;
        wSecurity = SEC_STAT_NOTSUPP;
        wChipPower = CHIP_STAT_UNKNOWN;
    }

    int Diff(struct ST_DEV_IDC_STATUS stStat)
    {
        if (wDevice != stStat.wDevice ||
            wMedia != stStat.wMedia ||
            wRetainBin != stStat.wRetainBin ||
            wSecurity != stStat.wSecurity ||
            wChipPower != stStat.wChipPower ||
            wCards != stStat.wCards)
        {
            return 1;
        }
        return 0;
    }

    int Copy(struct ST_DEV_IDC_STATUS stStat)
    {
        wDevice = stStat.wDevice;
        wMedia = stStat.wMedia;
        wRetainBin = stStat.wRetainBin;
        wSecurity = stStat.wSecurity;
        wChipPower = stStat.wChipPower;
        wCards = stStat.wCards;
        return 0;
    }

} STDEVIDCSTATUS, *LPSTDEVIDCSTATUS;


//****************************************************************************
// 读写介质数据相关定义
//****************************************************************************
// MediaRW类别
#define RW_NULL             0x0000  // 空
#define RW_TRACK1           0x0001  // 磁道1
#define RW_TRACK2           0x0002  // 磁道2
#define RW_TRACK3           0x0004  // 磁道3
#define RW_CHIP             0x0008  // 芯片数据
#define RW_FRONTIMAGE       0x0016  // 正面图
#define RW_BACKIMAGE        0x0032  // 背面图

// MediaRWResult
#define RW_RESULT_SUCC      0       // 成功
#define RW_RESULT_INV       1       // 失败(数据无效)
#define RW_RESULT_MISS      2       // 失败(数据空白)
#define RW_RESULT_TLONG     3       // 失败(数据太长)
#define RW_RESULT_TSHORT    4       // 失败(数据太短)
#define RW_RESULT_NOTSUPP   5       // 失败(不支持)
#define RW_RESULT_SRCMISS   6       // 失败(数据源丢失)

// MediaRW模式类别
enum MEDIA_RW_MODE
{
    MEDIA_READ              = 0,    // 读介质
    MEDIA_WRITE             = 1,    // 写介质
};

// Chip操作类别
enum CHIP_RW_MODE
{
    CHIP_POW_COLD           = 1,    // 冷复位
    CHIP_POW_WARM           = 2,    // 热复位
    CHIP_POW_OFF            = 3,    // 断电
    CHIP_IO_T0              = 4,    // T0协议通信
    CHIP_IO_T1              = 5,    // T1协议通信
};

// Media读写数据入参回参结构体
typedef struct Dev_IDC_Media_ReadWrite
{
    struct
    {
        CHAR szData[1024 * 10];
        DWORD wSize;
        WORD wResult;           // 结果(参考宏定义 MediaRW模式类别)
    } stData[6];

    DWORD dwRWType;             // 类别(参考MediaRW类别)
    DWORD dwTimeOut;            // 超时时间
    LONG  lOtherParam[12];      // 其他参数

    Dev_IDC_Media_ReadWrite()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_IDC_Media_ReadWrite));
        stData[0].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[1].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[2].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[3].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[4].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[5].wResult = RW_RESULT_INV;  // 失败(数据无效)
    }
    void DataClear()
    {
        memset(stData, 0x00, sizeof(stData));
        stData[0].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[1].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[2].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[3].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[4].wResult = RW_RESULT_INV;  // 失败(数据无效)
        stData[5].wResult = RW_RESULT_INV;  // 失败(数据无效)
    }
    void DataClear(WORD wIdx)
    {
        if (wIdx >= 0 && wIdx < 6)
        {
            stData[wIdx].wResult = RW_RESULT_INV;  // 失败(数据无效)
            MSET_0(stData[wIdx].szData);
            stData[wIdx].wSize = 0;
        }
    }
    void SetData(WORD wIdx, WORD wResult, LPSTR lpData)
    {
        if (wIdx >= 0 && wIdx < 6)
        {
            stData[wIdx].wResult = wResult;
            MSET_0(stData[wIdx].szData);
            memcpy(stData[wIdx].szData, lpData, strlen(lpData));
            stData[wIdx].wSize = strlen(lpData);
        }
    }
    void SetData(WORD wIdx, WORD wResult, LPSTR lpData, DWORD dwSize)
    {
        if (wIdx >= 0 && wIdx < 6)
        {
            stData[wIdx].wResult = wResult;
            MSET_0(stData[wIdx].szData);
            memcpy(stData[wIdx].szData, lpData, dwSize);
            stData[wIdx].wSize = dwSize;
        }
    }
} STMEDIARW, *LPSTMEDIARW;

// Chip读写数据入参回参结构体
typedef struct Dev_IDC_Chip_ReadWrite
{
    struct
    {
        CHAR szData[1024 * 10];
        DWORD dwSize;
        WORD wResult;
    } stData[2];

    DWORD dwTimeOut;        // 超时时间
    LONG  lOtherParam[12];  // 其他参数

    Dev_IDC_Chip_ReadWrite()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_IDC_Chip_ReadWrite));
    }
} STCHIPRW, *LPSTCHIPRW;


//****************************************************************************
// 灯控制相关定义
//****************************************************************************
// LED灯类型
enum GUID_LIGHTS_TYPE
{
    GLIGHTS_TYPE_YELLOW     = 1,    // 黄灯
    GLIGHTS_TYPE_BLUE       = 2,    // 蓝灯
    GLIGHTS_TYPE_GREEN      = 4,    // 绿灯
    GLIGHTS_TYPE_RED        = 8,    // 红灯
};

// LED灯控制
enum GUID_LIGHTS_ACTION
{
    GLIGHTS_ACT_OPEN        = 0,    // 打开
    GLIGHTS_ACT_CLOSE       = 1,    // 关闭
    GLIGHTS_ACT_FLASH       = 2,    // 闪烁
};

// 指示灯控制结构体
typedef struct Dev_IDC_GUID_LIGHTS_CONTROL
{
    WORD            wContMode;      // 控制模式(0自动/1手动)
    GUID_LIGHTS_TYPE enLigType;     // 指示灯类型
    GUID_LIGHTS_ACTION enLigAct;    // 指示灯控制
    WORD            wDelay;         // 间隔时间(单位:毫秒)
    Dev_IDC_GUID_LIGHTS_CONTROL()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_IDC_GUID_LIGHTS_CONTROL));
    }
} STGLIGHTSCONT, *LPSTGLIGHTSCONT;


//****************************************************************************
// 鸣响控制相关定义
//****************************************************************************
// 鸣响控制结构体
typedef struct Dev_IDC_BEEP_CONTROL
{
    WORD wContMode;         // 控制模式(0自动/1手动)
    WORD wBeepMsec;         // 鸣响一次时间(单位:毫秒)
    WORD wBeepInterval;     // 鸣响频率/鸣响间隔(单位:毫秒)
    WORD wBeepCount;        // 每次鸣响的次数(单位:毫秒)
    Dev_IDC_BEEP_CONTROL()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_IDC_BEEP_CONTROL));
    }
} STBEEPCONT, *LPSTBEEPCONT;


//****************************************************************************
// 接口类定义
//****************************************************************************
struct  IDevIDC
{
    /************************************************************
    ** 功能: 释放接口
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Release()
    {
        return ERR_IDC_UNSUP_CMD;
    }
    /************************************************************
    ** 功能: 打开与设备的连接
    ** 输入: pMode: 自定义OPEN参数字串,格式自定义
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Open(const char *pMode)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 关闭与设备的连接
    ** 输入: 无
    ** 输出: 无
    ** 返回: 无
    ************************************************************/
    virtual int Close()
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
     ** 功能: 取消
     ** 输入: usMode 取消类型
     ** 输出: 无
     ** 返回: 见返回错误码定义
     ************************************************************/
    virtual int Cancel(unsigned short usMode = 0)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 设备复位
    ** 输入: enMediaAct 介质动作(参考 enum MEDIA_ACTION)
    **      usParam    介质参数(根据不同设备要求填写)
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Reset(MEDIA_ACTION enMediaAct, unsigned short usParam = 0)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 取设备状态
    ** 输入: 无
    ** 输出: stStatus 设备状态结构体
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int GetStatus(STDEVIDCSTATUS &stStatus)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 介质控制
    ** 输入: enMediaAct    介质动作
    **      usParam       介质参数,缺省0
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam = 0)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 介质读写
    ** 输入: enRWMode     读写模式(参考 MediaRW模式 enum MEDIA_RW_MODE)
    **      stMediaData   数据入参
    ** 输出: stMediaData   数据回参
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 芯片读写
    ** 输入: enChipMode  读写模式(参考 Chip操作类别 enum CHIP_RW_MODE)
    **      stChipData   数据入参
    ** 输出: stChipData   数据回参
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ChipReadWrite(CHIP_RW_MODE enChipMode, STCHIPRW &stChipData)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 设置数据
    ** 输入: usType  传入数据类型
    **      vData   传入数据
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int SetData(unsigned short usType, void *vData = nullptr)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
     ** 功能: 获取数据
     ** 输入: usType  获取数据类型
     ** 输出: vData   获取数据
     ** 返回: 见返回错误码定义
     ************************************************************/
    virtual int GetData(unsigned short usType, void *vData)
    {
        return ERR_IDC_UNSUP_CMD;
    }

    /************************************************************
     ** 功能: 获取版本
     ** 输入: usType  获取类型(参考 GetVersion 数据类型)
     **      nSize   数据buffer size
     ** 输出: szVer   版本数据
     ** 返回: 见返回错误码定义
     ************************************************************/
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize)
    {
        return ERR_IDC_UNSUP_CMD;
    }
};

extern "C" DEVIDC_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev);


//****************************************************************************
// IDevIDC定义 设备相关变量转换 类定义(统一转换)
//****************************************************************************
class ConvertVarIDC
{
private:
    CHAR m_szErrStr[1024];
public:
    // 设备状态转换为WFS格式
    WORD ConvertDeviceStatus2WFS(WORD wDevStat)
    {
        switch (wDevStat)
        {
            case DEVICE_STAT_ONLINE     /* 设备正常 */     : return WFS_IDC_DEVONLINE;
            case DEVICE_STAT_OFFLINE    /* 设备脱机 */     : return WFS_IDC_DEVOFFLINE;
            case DEVICE_STAT_POWEROFF   /* 设备断电 */     : return WFS_IDC_DEVPOWEROFF;
            case DEVICE_STAT_NODEVICE   /* 设备不存在 */    : return WFS_IDC_DEVNODEVICE;
            case DEVICE_STAT_HWERROR    /* 设备故障 */     : return WFS_IDC_DEVHWERROR;
            case DEVICE_STAT_USERERROR  /*  */            : return WFS_IDC_DEVUSERERROR;
            case DEVICE_STAT_BUSY       /* 设备读写中 */    : return WFS_IDC_DEVBUSY;
            case DEVICE_STAT_FRAUDAT    /* 设备出现欺诈企图 */: return WFS_IDC_DEVFRAUDATTEMPT;
            defaule: return WFS_IDC_DEVOFFLINE;
        }
    }

    // Media状态转换为WFS格式
    WORD ConvertMediaStatus2WFS(WORD wMediaStat)
    {
        switch (wMediaStat)
        {
            case MEDIA_STAT_PRESENT   /* 通道内有 */               : return WFS_IDC_MEDIAPRESENT;
            case MEDIA_STAT_NOTPRESENT/* 通道内无 */               : return WFS_IDC_MEDIANOTPRESENT;
            case MEDIA_STAT_JAMMED    /* 通道内有且被夹住 */        : return WFS_IDC_MEDIAJAMMED;
            case MEDIA_STAT_NOTSUPP   /* 不支持检测通道内是否有 */   : return WFS_IDC_MEDIANOTPRESENT;
            case MEDIA_STAT_UNKNOWN   /* 通道内状态未知 */          : return WFS_IDC_MEDIAUNKNOWN;
            case MEDIA_STAT_ENTERING  /* 在出口 */                 : return WFS_IDC_MEDIAENTERING;
            case MEDIA_STAT_LATCHED   /* 存在但被锁定 */            : return WFS_IDC_MEDIALATCHED;
            default: return WFS_IDC_MEDIAUNKNOWN;
        }
    }

    // RetainBin状态转换为WFS格式
    WORD ConvertRetainBinStatus2WFS(WORD wRetBinStat)
    {
        switch (wRetBinStat)
        {
            case RETBIN_STAT_OK     /* 未满 */    : return WFS_IDC_RETAINBINOK;
            case RETBIN_STAT_NOTSUPP/* 不支持 */   : return WFS_IDC_RETAINNOTSUPP;
            case RETBIN_STAT_FULL   /* 满 */     : return WFS_IDC_RETAINBINFULL;
            case RETBIN_STAT_HIGH   /* 将满 */    : return WFS_IDC_RETAINBINHIGH;
            case RETBIN_STAT_MISSING/* 丢失 */    : return WFS_IDC_RETAINBINMISSING;
            default: return WFS_IDC_RETAINNOTSUPP;
        }
    }

    // Security状态转换为WFS格式
    WORD ConvertSecurityStatus2WFS(WORD wSecStat)
    {
        switch (wSecStat)
        {
            case SEC_STAT_OPEN      /* 已打开可操作 */    : return WFS_IDC_SECOPEN;
            case SEC_STAT_NOTSUPP   /* 不支持 */         : return WFS_IDC_SECNOTSUPP;
            case SEC_STAT_NOTREADY  /* 未准备好 */       : return WFS_IDC_SECNOTREADY;
            default: return WFS_IDC_SECNOTSUPP;
        }
    }

    // ChipPower状态转换为WFS格式
    WORD ConvertChipPowerStatus2WFS(WORD wChipStat)
    {
        switch (wChipStat)
        {
            case CHIP_STAT_ONLINE       /* 存在并通电中 */     : return WFS_IDC_CHIPONLINE;
            case CHIP_STAT_POWEREDOFF   /* 存在未通电 */       : return WFS_IDC_CHIPPOWEREDOFF;
            case CHIP_STAT_BUSY         /* 存在并通电读写中 */  : return WFS_IDC_CHIPBUSY;
            case CHIP_STAT_HWERROR      /* 存在且故障中 */     : return WFS_IDC_CHIPHWERROR;
            case CHIP_STAT_NOCARD       /* 无卡 */            : return WFS_IDC_CHIPNOCARD;
            case CHIP_STAT_NOTSUPP      /* 不支持 */          : return WFS_IDC_CHIPNOTSUPP;
            case CHIP_STAT_UNKNOWN      /* 状态未知 */         : return WFS_IDC_CHIPUNKNOWN;
            default: return WFS_IDC_CHIPNOTSUPP;
        }
    }

    // 错误码转换为WFS格式
    LONG ConvertDevErrCode2WFS(INT nRet)
    {
        switch (nRet)
        {
            // 成功
            case IDC_SUCCESS:               return WFS_SUCCESS;                 // 操作成功->成功
            // 警告
            case ERR_IDC_INSERT_TIMEOUT:    return WFS_ERR_TIMEOUT;             // 进卡超时->超时
            case ERR_IDC_USER_CANCEL:       return WFS_ERR_CANCELED;            // 用户取消进卡->取消
            case ERR_IDC_DEVICE_REOPEN:     return WFS_ERR_CONNECTION_LOST;     // 设备重连
            case ERR_IDC_CMD_RUNNING:       return WFS_SUCCESS;                 // 有命令执行中
            // 通用错误
            case ERR_IDC_LIBRARY:           return WFS_ERR_SOFTWARE_ERROR;      // 动态库加载失败
            case ERR_IDC_PARAM_ERR:         return WFS_ERR_INVALID_DATA;        // 参数错误
            case ERR_IDC_UNSUP_CMD:         return WFS_ERR_UNSUPP_COMMAND;      // 不支持的指令
            case ERR_IDC_DATA_FAIL:         return WFS_ERR_HARDWARE_ERROR;      // 数据错误
            case ERR_IDC_USER_ERR:          return WFS_ERR_USER_ERROR;          // 用户使用错误
            case ERR_IDC_OTHER:             return WFS_ERR_HARDWARE_ERROR;      // 其它错误(知道错误原因,不细分类别)
            case ERR_IDC_UNKNOWN:           return WFS_ERR_HARDWARE_ERROR;      // 未知错误(不知原因的错误)
            // 通信错误
            case ERR_IDC_COMM_ERR:          return WFS_ERR_CONNECTION_LOST;     // 通讯错误
            case ERR_IDC_READ_ERR:          return WFS_ERR_HARDWARE_ERROR;      // 读数据错误
            case ERR_IDC_WRITE_ERR:         return WFS_ERR_HARDWARE_ERROR;      // 写数据错误
            case ERR_IDC_READ_TIMEOUT:      return WFS_ERR_TIMEOUT;             // 读数据超时
            case ERR_IDC_WRITE_TIMEOUT:     return WFS_ERR_TIMEOUT;             // 写数据超时
            case ERR_IDC_RESP_ERR:          return WFS_ERR_IDC_ATRNOTOBTAINED;  // 应答错误
            case ERR_IDC_RESP_NOT_COMP:     return WFS_ERR_IDC_ATRNOTOBTAINED;  // 应答数据不完整
            case ERR_IDC_COMM_RUN:          return WFS_ERR_HARDWARE_ERROR;      // 通信命令执行错误
            // 设备错误
            case ERR_IDC_DEV_NOTFOUND:      return WFS_ERR_HARDWARE_ERROR;      // 设备不存在
            case ERR_IDC_DEV_NOTOPEN:       return WFS_ERR_HARDWARE_ERROR;      // 设备没有打开
            case ERR_IDC_DEV_OFFLINE:       return WFS_ERR_CONNECTION_LOST;     // 设备脱机
            case ERR_IDC_DEV_HWERR:         return WFS_ERR_HARDWARE_ERROR;      // 设备硬件故障
            case ERR_IDC_DEV_STAT_ERR:      return WFS_ERR_HARDWARE_ERROR;      // 设备状态错误
            // 介质错误
            case ERR_IDC_MED_NOTFOUND:      return WFS_ERR_IDC_NOMEDIA;         // 介质未找到
            case ERR_IDC_MED_SHORT:         return WFS_ERR_IDC_CARDTOOSHORT;    // 介质太短
            case ERR_IDC_MED_LONG:          return WFS_ERR_IDC_CARDTOOLONG;     // 介质太长
            case ERR_IDC_MED_PULLOUT:       return WFS_ERR_USER_ERROR;          // 接收介质时,介质被强制拖出
            case ERR_IDC_MED_MULTICARD:     return WFS_ERR_IDC_INVALIDMEDIA;    // 检测到多张卡
            case ERR_IDC_MED_CONFLICT:      return WFS_ERR_IDC_INVALIDMEDIA;    // 介质冲突
            case ERR_IDC_MED_IN_OVER:       return WFS_ERR_USER_ERROR;          // 介质吸入失败次数超限
            case ERR_IDC_MED_JAMMED:        return WFS_ERR_IDC_MEDIAJAM;        // 介质阻塞
            case ERR_IDC_MED_STAT_ERR:      return WFS_ERR_HARDWARE_ERROR;      // 介质状态错误
            case ERR_IDC_MED_INV:           return WFS_ERR_IDC_INVALIDMEDIA;    // 无效介质
            // 磁条卡错误
            case ERR_IDC_MAG_DATA_INV:      return WFS_ERR_IDC_INVALIDMEDIA;    // 无效磁卡,有磁但磁道数据无效
            case ERR_IDC_MAG_NOTRACK:       return WFS_ERR_IDC_INVALIDMEDIA;    // 非磁卡，未检测到磁道
            // 芯片卡错误
            case ERR_IDC_CHIP_NOTINV:       return WFS_ERR_IDC_INVALIDMEDIA;    // 无效芯片卡或卡无芯片
            case ERR_IDC_CHIP_ACTIVE_FAIL:  return WFS_ERR_HARDWARE_ERROR;      // 卡激活失败
            case ERR_IDC_CHIP_IO_FAIL:      return WFS_ERR_HARDWARE_ERROR;      // CHIIP失败
            case ERR_IDC_CHIP_PORT_NOSUP:   return WFS_ERR_IDC_PROTOCOLNOTSUPP; // 不支持的IC通讯协议
            case ERR_IDC_CHIP_RWERROR:      return WFS_ERR_HARDWARE_ERROR;      // 芯片卡读写错误
            case ERR_IDC_CHIP_PULLOUT:      return WFS_ERR_USER_ERROR;          // 芯片卡操作时被强制拖走
            // 回收错误
            case ERR_IDC_RETAIN_NOSUP:      return WFS_ERR_IDC_NORETRACTBIN;    // 设备不支持回收
            case ERR_IDC_RETAIN_FULL:       return WFS_ERR_IDC_RETAINBINFULL;   // 设备回收满
            // 其他错误
            case ERR_IDC_DETEC_TAMPER:      return WFS_ERR_HARDWARE_ERROR;      // 检知有异物
            case ERR_IDC_DETEC_DESTRUC:     return WFS_ERR_HARDWARE_ERROR;      // 检知有破坏行为
            case ERR_IDC_DETEC_FRAUD:       return WFS_ERR_HARDWARE_ERROR;      // 检知有欺欺诈行为
            default:                        return WFS_ERR_HARDWARE_ERROR;
        }
    }

    CHAR* ConvertDevErrCodeToStr(INT nRet)
    {
        #define CONV_IDC_CODE_STR(RET, STR) \
            sprintf(m_szErrStr, "%d|%s", RET, STR); \
            return m_szErrStr;

        memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

        switch(nRet)
        {
            // 成功
            case IDC_SUCCESS:               CONV_IDC_CODE_STR(nRet, "操作成功")
            // 警告
            case ERR_IDC_INSERT_TIMEOUT:    CONV_IDC_CODE_STR(nRet, "进卡超时")
            case ERR_IDC_USER_CANCEL:       CONV_IDC_CODE_STR(nRet, "用户取消进卡")
            case ERR_IDC_DEVICE_REOPEN:     CONV_IDC_CODE_STR(nRet, "设备重连")
            case ERR_IDC_CMD_RUNNING:       CONV_IDC_CODE_STR(nRet, "有命令执行中")
            // 通用错误
            case ERR_IDC_LIBRARY:           CONV_IDC_CODE_STR(nRet, "动态库加载失败")
            case ERR_IDC_PARAM_ERR:         CONV_IDC_CODE_STR(nRet, "参数错误")
            case ERR_IDC_UNSUP_CMD:         CONV_IDC_CODE_STR(nRet, "不支持的指令")
            case ERR_IDC_DATA_FAIL:         CONV_IDC_CODE_STR(nRet, "数据错误")
            case ERR_IDC_TIMEOUT:           CONV_IDC_CODE_STR(nRet, "超时")
            case ERR_IDC_USER_ERR:          CONV_IDC_CODE_STR(nRet, "用户使用错误")
            case ERR_IDC_OTHER:             CONV_IDC_CODE_STR(nRet, "其它错误(知道错误原因,不细分类别)")
            case ERR_IDC_UNKNOWN:           CONV_IDC_CODE_STR(nRet, "未知错误(不知原因的错误)")
            // 通信错误
            case ERR_IDC_COMM_ERR:          CONV_IDC_CODE_STR(nRet, "通讯错误")
            case ERR_IDC_READ_ERR:          CONV_IDC_CODE_STR(nRet, "读数据错误")
            case ERR_IDC_WRITE_ERR:         CONV_IDC_CODE_STR(nRet, "写数据错误")
            case ERR_IDC_READ_TIMEOUT:      CONV_IDC_CODE_STR(nRet, "读数据超时")
            case ERR_IDC_WRITE_TIMEOUT:     CONV_IDC_CODE_STR(nRet, "写数据超时")
            case ERR_IDC_RESP_ERR:          CONV_IDC_CODE_STR(nRet, "应答错误")
            case ERR_IDC_RESP_NOT_COMP:     CONV_IDC_CODE_STR(nRet, "应答数据不完整")
            case ERR_IDC_COMM_RUN:          CONV_IDC_CODE_STR(nRet, "通信命令执行错误")
            // 设备错误
            case ERR_IDC_DEV_NOTFOUND:      CONV_IDC_CODE_STR(nRet, "设备不存在")
            case ERR_IDC_DEV_NOTOPEN:       CONV_IDC_CODE_STR(nRet, "设备没有打开")
            case ERR_IDC_DEV_OFFLINE:       CONV_IDC_CODE_STR(nRet, "设备脱机")
            case ERR_IDC_DEV_HWERR:         CONV_IDC_CODE_STR(nRet, "设备硬件故障")
            case ERR_IDC_DEV_STAT_ERR:      CONV_IDC_CODE_STR(nRet, "设备状态错误")
            // 介质错误
            case ERR_IDC_MED_NOTFOUND:      CONV_IDC_CODE_STR(nRet, "介质未找到")
            case ERR_IDC_MED_SHORT:         CONV_IDC_CODE_STR(nRet, "介质太短")
            case ERR_IDC_MED_LONG:          CONV_IDC_CODE_STR(nRet, "介质太长")
            case ERR_IDC_MED_PULLOUT:       CONV_IDC_CODE_STR(nRet, "接收介质时,介质被强制拖出")
            case ERR_IDC_MED_MULTICARD:     CONV_IDC_CODE_STR(nRet, "检测到多张卡")
            case ERR_IDC_MED_CONFLICT:      CONV_IDC_CODE_STR(nRet, "介质冲突")
            case ERR_IDC_MED_IN_OVER:       CONV_IDC_CODE_STR(nRet, "介质吸入失败次数超限")
            case ERR_IDC_MED_JAMMED:        CONV_IDC_CODE_STR(nRet, "介质阻塞")
            case ERR_IDC_MED_STAT_ERR:      CONV_IDC_CODE_STR(nRet, "介质状态错误")
            case ERR_IDC_MED_INV:           CONV_IDC_CODE_STR(nRet, "无效介质")
            // 磁条卡错误
            case ERR_IDC_MAG_DATA_INV:      CONV_IDC_CODE_STR(nRet, "无效磁卡，有磁但磁道数据无效")
            case ERR_IDC_MAG_NOTRACK:       CONV_IDC_CODE_STR(nRet, "非磁卡，未检测到磁道")
            // 芯片卡错误
            case ERR_IDC_CHIP_NOTINV:       CONV_IDC_CODE_STR(nRet, "无效芯片卡或卡无芯片")
            case ERR_IDC_CHIP_ACTIVE_FAIL:  CONV_IDC_CODE_STR(nRet, "介质激活失败")
            case ERR_IDC_CHIP_IO_FAIL:      CONV_IDC_CODE_STR(nRet, "CHIIP失败")
            case ERR_IDC_CHIP_PORT_NOSUP:   CONV_IDC_CODE_STR(nRet, "不支持的IC通讯协议")
            case ERR_IDC_CHIP_RWERROR:      CONV_IDC_CODE_STR(nRet, "芯片卡读写错误")
            case ERR_IDC_CHIP_PULLOUT:      CONV_IDC_CODE_STR(nRet, "芯片卡操作时被强制拖走")
            // 回收错误
            case ERR_IDC_RETAIN_NOSUP:      CONV_IDC_CODE_STR(nRet, "设备不支持回收")
            case ERR_IDC_RETAIN_FULL:       CONV_IDC_CODE_STR(nRet, "设备回收满")
            // 其他错误
            case ERR_IDC_DETEC_TAMPER:      CONV_IDC_CODE_STR(nRet, "检知有异物")
            case ERR_IDC_DETEC_DESTRUC:     CONV_IDC_CODE_STR(nRet, "检知有破坏行为")
            case ERR_IDC_DETEC_FRAUD:       CONV_IDC_CODE_STR(nRet, "检知有欺欺诈行为")
            default:                        CONV_IDC_CODE_STR(nRet, "未定义错误");
        }

        return m_szErrStr;
    }

    // MediaRWResult(介质读写结果)转换为WFS格式
    WORD ConvertMediaRWResult2WFS(WORD wRet)
    {
        switch (wRet)
        {
            case RW_RESULT_SUCC     /* 成功 */           : return WFS_IDC_DATAOK;
            case RW_RESULT_INV      /* 失败(数据无效) */  : return WFS_IDC_DATAINVALID;
            case RW_RESULT_MISS     /* 失败(数据空白) */  : return WFS_IDC_DATAMISSING;
            case RW_RESULT_TLONG    /* 失败(数据太长) */  : return WFS_IDC_DATATOOLONG;
            case RW_RESULT_TSHORT   /* 失败(数据太短) */  : return WFS_IDC_DATATOOSHORT;
            case RW_RESULT_NOTSUPP  /* 失败(不支持) */    : return WFS_IDC_DATASRCNOTSUPP;
            case RW_RESULT_SRCMISS  /* 失败(数据源丢失) */ : return WFS_IDC_DATASRCMISSING;
            default: return WFS_IDC_DATAOK;
        }
    }

    // WFS_IDX_XXX 与 MediaRW类别转换
    DWORD ConvertWFS2MediaRW(DWORD dwXFS)
    {
        DWORD dwRet = 0;
        if ((dwXFS & WFS_IDC_TRACK1) == WFS_IDC_TRACK1)
        {
            dwRet |= RW_TRACK1;
        }
        if ((dwXFS & WFS_IDC_TRACK2) == WFS_IDC_TRACK2)
        {
            dwRet |= RW_TRACK2;
        }
        if ((dwXFS & WFS_IDC_TRACK3) == WFS_IDC_TRACK3)
        {
            dwRet |= RW_TRACK3;
        }
        if ((dwXFS & WFS_IDC_CHIP) == WFS_IDC_CHIP)
        {
            dwRet |= RW_CHIP;
        }
        if ((dwXFS & WFS_IDC_FRONTIMAGE) == WFS_IDC_FRONTIMAGE)
        {
            dwRet |= RW_FRONTIMAGE;
        }
        if ((dwXFS & WFS_IDC_BACKIMAGE) == WFS_IDC_BACKIMAGE)
        {
            dwRet |= RW_BACKIMAGE;
        }

        return dwRet;
    }
};

#endif  // !defined(SPBuild_OPTIM)



