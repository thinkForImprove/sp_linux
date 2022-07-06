#pragma once
/***************************************************************
* 文件名称：IDevIDC.h
* 文件描述：声明读卡器底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
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
#define ERR_IDC_WRITETIMEOUT    (-34)   // 读数据超时
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
//IC卡相关
#define ERR_IDC_PROTOCOLNOTSUPP (-51)   // 不支持的IC通讯协议
#define ERR_IDC_ICRW_ERROR      (-52)   // IC卡读写时被拖走
#define ERR_IDC_NO_DEVICE       (-53)   // 指定的设备不存在
#define ERR_IDC_OTHER           (-54)   // 其它错误
#define ERR_IDC_USERERR         (-55)   // 检测到卡但进卡失败次数超限
#define ERR_IDC_TAMPER          (-56)   // 防盗嘴异物传感器检测到异物
#define ERR_IDC_CONFLICT        (-57)   // ka chongtu

#define INFINITE                (0xFFFFFFFF)
#define MAX_LEN_FWVERSION       (255)   // 最大版本信息长度
#define MAX_LEN_ATRINFO         (33)    // 最大ATR信息长度
//////////////////////////////////////////////////////////////////////////
//定义卡动作
enum CardAction
{
    CARDACTION_NOACTION   = 1,   // 移动并保持
    CARDACTION_NOMOVEMENT = 2,   // 不移动并保持
    CARDACTION_EJECT      = 3,   // 弹卡
    CARDACTION_RETRACT    = 4,   // 吞卡
};

//定义抖动功能动作
enum WobbleAction
{
    WOBBLEACTION_START    = 1,      // 使能抖动
    WOBBLEACTION_STOP     = 2,      // 去除抖动功能
    WOBBLEACTION_NOACTION = 3,      // 保持,不做操作
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
    IDCSTAUTS_NOCARD          = 0, //无卡
    IDCSTAUTS_ENTERING            = 1, //门口
    IDCSTAUTS_INTERNAL            = 2, //内部
    IDCSTAUTS_ICC_PRESS           = 3, //IC卡被压下
    IDCSTAUTS_ICC_ACTIVE          = 4, //IC卡被激活
    IDCSTAUTS_ICC_POWEROFF    = 5, //IC POWEROFF
    IDCSTATUS_UNKNOWN             = 10,//未知状态
};

//////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////
//接口类定义
struct  IDevIDC
{
    // 释放接口
    virtual void Release() = 0;
    /************************************************************
    ** 功能：打开与设备的连接
    ** 输入：pMode: 自定义OPEN参数字串,
            串口： 格式为："COMX: BaudRate,Parity,DataBits,StopBits"(例如："COM2:115200,N,8,1")
            USB:  格式自定义
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Open(const char *pMode) = 0;

    /************************************************************
    ** 功能：设备初始化
    ** 输入: eActFlag : 卡动作
             nNeedWobble：抖动功能
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Init(CardAction eActFlag, WobbleAction nNeedWobble = WOBBLEACTION_NOACTION) = 0;

    /************************************************************
    ** 功能：关闭与设备的连接
    ** 输入：无
    ** 输出：无
    ** 返回：无
    ************************************************************/
    virtual void Close() = 0;

    /************************************************************
    ** 功能：吞卡
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int EatCard() = 0;

    /************************************************************
    ** 功能：弹出卡
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int EjectCard() = 0;

    /************************************************************
    ** 功能：进卡
    ** 输入：ulTimeOut:进卡超时,单位毫秒, 无限超时：INFINITE
             Magnetic：是否是磁条卡
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int AcceptCard(unsigned long ulTimeOut, bool Magnetic = true) = 0;

    /************************************************************
    ** 功能：停止上一次读卡
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CancelReadCard() = 0;

    /************************************************************
    ** 功能：写磁道信息
    ** 输入：stTrackInfo 保存对应磁道信息
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int WriteTracks(const STTRACK_INFO &stTrackInfo) = 0;

    /************************************************************
    ** 功能：读磁道信息
    ** 输入：无
    ** 输出：stTrackInfo 保存对应磁道信息
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ReadTracks(STTRACK_INFO &stTrackInfo) = 0;

    /************************************************************
    ** 功能：检测卡
    ** 输入：无
    ** 输出：IDCstatus
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int DetectCard(IDC_IDCSTAUTS &IDCstatus) = 0;

    /************************************************************
    ** 功能：获取固件版本
    ** 输入：无
    ** 输出：pFWVersion：保存固件版本
             uLen：版本有效信息长度
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetFWVersion(char pFWVersion[MAX_LEN_FWVERSION], unsigned int &uLen) = 0;

    /*--------------------- 以下IC卡操作接口-------------------------*/
    /************************************************************
    ** 功能：压卡
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ICCPress() = 0;

    /************************************************************
    ** 功能：卡释放
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ICCRelease() = 0;

    /************************************************************
    ** 功能：卡激活
    ** 输入：无
    ** 输出：pATRInfo:保存ATR信息
            uATRLen：ATR有效信息长度
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ICCActive(char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen) = 0;

    /************************************************************
    ** 功能：卡激活
    ** 输入：eResetFlag:激活方式
    ** 输出：pATRInfo:保存ATR信息
             uATRLen：ATR有效信息长度
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ICCReset(ICCardReset eResetFlag, char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen) = 0;

    /************************************************************
    ** 功能：反激活
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ICCDeActivation() = 0;

    /************************************************************
    ** 功能：数据接收与发送
    ** 输入：eProFlag:表示读卡器在主机与IC卡之间接收与发送数据所用的协议
    ** 输出：pInOutData:输入数据缓存
             nInOutLen：输入数据有效长度
    ** 输出：pInOutData:输出数据缓存
             nInOutLen：输出数据有效长度
             T0协议时InLen范围为：4~261，OutLen范围为：2~258；
             T1协议时InLen范围为：4~360，OutLen范围为：2~320；
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen) = 0;

    /************************************************************
    ** 功能：设置非接LED
    ** 输入：eFlagLedType:LED灯类型
             eFlagLedAct :LED灯控制
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct) = 0;

    /************************************************************
    ** 功能：设置非接轰鸣器
    ** 输入：蜂鸣时间(s), 默认５秒
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int SetRFIDCardReaderBeep(unsigned long ulTime = 5) = 0;
};

extern "C" DEVIDC_EXPORT long CreateIDevIDC(LPCSTR lpDevType, IDevIDC *&pDev);
