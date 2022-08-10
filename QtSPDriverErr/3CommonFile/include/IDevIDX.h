#pragma once
/***************************************************************
* 文件名称：IDevIDX.h
* 文件描述：声明身份证底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年2月3日
* 文件版本：1.0.0.1
****************************************************************/
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////////
#if defined(IDEVIDX_LIBRARY)
#define DEVIDX_EXPORT     Q_DECL_EXPORT
#else
#define DEVIDX_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////

//成功
#define ERR_IDX_SUCCESS         (0)     // 操作成功
//警告
#define ERR_IDX_INSERT_TIMEOUT  (1)     // 进卡超时
#define ERR_IDX_USER_CANCEL     (2)     // 用户取消进卡
//错误
#define ERR_IDX_COMM_ERR        (-1)    // 通讯错误
#define ERR_IDX_JAMMED          (-2)    // 读卡器堵卡
#define ERR_IDX_OFFLINE         (-3)    // 读卡器脱机
#define ERR_IDX_NOT_OPEN        (-4)    // 读卡器没有打开
#define ERR_IDX_RETAINBINFULL   (-5)    // 回收箱满
#define ERR_IDX_HWERR           (-6)    // 硬件故障
#define ERR_IDX_STATUS_ERR      (-7)    // 读卡器其他状态出错
#define ERR_IDX_UNSUP_CMD       (-31)   // 不支持的指令
#define ERR_IDX_PARAM_ERR       (-32)   // 参数错误
#define ERR_IDX_READTIMEOUT     (-33)   // 读数据超时
#define ERR_IDX_WRITETIMEOUT    (-34)   // 写数据超时
#define ERR_IDX_READERROR       (-35)   // 读卡错
#define ERR_IDX_WRITEERROR      (-36)   // 写卡错
#define ERR_IDX_INVALIDCARD     (-37)   // 无效磁卡，有磁但磁道数据无效
#define ERR_IDX_NOTRACK         (-38)   // 非磁卡，未检测到磁道
#define ERR_IDX_CARDPULLOUT     (-39)   // 接收卡时，卡被拖出
#define ERR_IDX_CARDTOOSHORT    (-40)   // 卡太短
#define ERR_IDX_CARDTOOLONG     (-41)   // 卡太长
#define ERR_IDX_WRITETRACKERROR (-42)   // 写磁道错误
#define ERR_IDX_ACTIVEFAILED    (-43)   // 卡激活失败
#define ERR_IDX_CHIPIOFAILED    (-44)   // CHIIP失败
//IC卡相关
#define ERR_IDX_PROTOCOLNOTSUPP (-51)   // 不支持的IC通讯协议
#define ERR_IDX_ICRW_ERROR      (-52)   // IC卡读写时被拖走
#define ERR_IDX_NO_DEVICE       (-53)   // 指定的设备不存在
#define ERR_IDX_OTHER           (-54)   // 其它错误
#define ERR_IDX_USERERR         (-55)   // 检测到卡但进卡失败次数超限
#define ERR_IDX_TAMPER          (-56)   // 防盗嘴异物传感器检测到异物
#define ERR_IDX_CONFLICT        (-57)   // 卡冲突

#define INFINITE                (0xFFFFFFFF)
#define MAX_LEN_FWVERSION       (255)   // 最大版本信息长度
#define MAX_LEN_ATRINFO         (33)    // 最大ATR信息长度
//////////////////////////////////////////////////////////////////////////

// SetData/GetData 数据类型
#define DATATYPE_INIT               0   // 初始化数据
#define DATATYPE_ResetDevice        1   // 复位设备(BSID81)
#define DATATYPE_SetDevIsNotOpen    2   // 设置设备Open标记=NotOpen

#define BUF_SIZE8       8
#define BUF_SIZE32      32
#define BUF_SIZE64      64
#define BUF_SIZE128     128
#define BUF_SIZE256     256
#define BUF_SIZE512     512

// 设备类型
#define IDEV_TYPE_BSID81    "0"     // BS-ID81


//////////////////////////////////////////////////////////////////////////
// 定义卡动作
enum CardAction
{
    CARDACTION_NOACTION     = 0,   // 无动作
    CARDACTION_EJECT        = 1,   // 退卡
    CARDACTION_RETAIN       = 2,   // 吞卡
};

// 定义卡的位置状态
enum CardPosition
{
    CARDPOS_NOCARD                = 0, // 无卡
    CARDPOS_ENTERING              = 1, // 门口
    CARDPOS_INTERNAL              = 2, // 内部
    CARDPOS_SCANNING              = 3, // 扫描位置
    CARDPOS_UNKNOWN               = 4, // 未知状态
};


//////////////////////////////////////////////////////////////////////////


enum DEVIDX_DEVICE_STATUS  //　适用DEVIDX返回状态
{
    DEV_ONLINE      = 0,    // 设备正常
    DEV_OFFLINE     = 1,    // 设备脱机
    DEV_POWEROFF    = 2,    // 设备断电
    DEV_NODEVICE    = 3,    // 设备不存在
    DEV_HWERROR     = 4,    // 设备故障
    DEV_USERERROR   = 5,    //
    DEV_BUSY        = 6     // 设备读写中
};

enum DEVIDX_MEDIA_STATUS   //　适用DEVIDX返回状态
{
    MEDIA_PRESENT       = 0,    // 介质在通道内
    MEDIA_NOTPRESENT    = 1,    // 介质不在设备里	卡不在读卡器内，也不在入卡口。
    MEDIA_JAMMED        = 2,    // 卡被夹住，需要操作员的干预。
    MEDIA_NOTSUPP       = 3,    // 卡状态不支持(例如刷卡式读卡器).
    MEDIA_UNKNOWN       = 4,    // 卡的状态未知
    MEDIA_ENTERING      = 5     // 卡位于的入卡口位置。
};

enum READTRACK_MODE
{
    RT_DEF              = 0x0000,    // 缺省
    RT_TRACK1           = 0x0001,
    RT_TRACK2           = 0x0002,
    RT_TRACK3           = 0x0004,
    RT_CHIP             = 0x0008,
    RT_FRONTIMAGE       = 0x0010,
    RT_BACKIMAGE        = 0x0020,
};

//////////////////////////////////////////////////////////////////////////
typedef struct Dev_IDX_Status   // 处理后的设备状态
{
    WORD wDevice;               // 设备状态(参考enum DEVIDX_DEVICE_STATUS)
    WORD wMedia;                // Media状态(参考enum DEVIDX_MEDIA_STATUS)
    WORD wForceBackCard;        // 强制退证按钮是否按下(0未按下/1按下)
    char szErrCode[8];          // 三位的错误码
    Dev_IDX_Status()
    {
        clear();
    }
    void clear()
    {
        memset(this, 0x00, sizeof(Dev_IDX_Status));
        wDevice = DEV_OFFLINE;
        wMedia = MEDIA_UNKNOWN;
    }
} DEVIDXSTATUS, *LPDEVIDXSTATUS;

// 保存磁道信息结构体
typedef struct IDX_Data_Detail_Track
{
    char szTrack[1024 * 10];
    bool bTrackOK;
    IDX_Data_Detail_Track()
    {
        memset(szTrack, 0x00, sizeof(char) * 1024);
        bTrackOK = false;
    }
} STTRACKDETAIL, *LPSTTRACKDETAIL;

// 3个磁道数据
typedef struct IDX_Track_Info
{
    STTRACKDETAIL TrackData[3];
} STTRACK_INFO, *LPSTTRACK_INFO;

//////////////////////////////////////////////////////////////////////////
//接口类定义
struct  IDevIDX
{
    /************************************************************
    ** 功能：释放接口
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
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
    ** 输入: nActFlag : 卡动作(参考enum CardAction)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Reset(const int nActFlag) = 0;

    /************************************************************
    ** 功能：关闭与设备的连接
    ** 输入：无
    ** 输出：无
    ** 返回：无
    ************************************************************/
    virtual void Close() = 0;

    /************************************************************
    ** 功能：取设备状态
    ** 输入：无
    ** 输出：stStatus : 设备状态结构体
    ** 返回：无
    ************************************************************/
    virtual int GetStatus(DEVIDXSTATUS &stStatus) = 0;

    /************************************************************
    ** 功能：取卡位置
    ** 输入：无
    ** 输出：enCardPos 卡位置参考(enum CardPosition)
    ** 返回：
    ************************************************************/
    virtual int GetCardPosition(CardPosition &enCardPos) = 0;

    /************************************************************
    ** 功能：吞卡
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int RetainCard() = 0;

    /************************************************************
    ** 功能：退卡
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int EjectCard() = 0;

    /************************************************************
    ** 功能：进卡
    ** 输入：ulTimeOut:进卡超时,单位毫秒, 0无超时
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int AcceptCard(unsigned long ulTimeOut) = 0;

   /************************************************************
    ** 功能：读磁道信息
    ** 输入：wTimeOut 超时时间，单位:秒(0一直等待,缺省0)
    **      enMode  辅助参数,缺省0(参考enum READTRACK_MODE)
    ** 输出：stTrackInfo 保存对应磁道信息
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ReadTracks(STTRACK_INFO &stTrackInfo, unsigned int unMode = RT_DEF) = 0;

    /************************************************************
    ** 功能：命令取消
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Cancel() = 0;

    /************************************************************
     ** 功能：设置数据
     ** 输入：vData 传入数据
     **      wDataType 传入数据类型
     ** 输出：无
     ** 返回：见返回错误码定义
     ************************************************************/
    virtual int SetData(void *vData, WORD wDataType = 0) = 0;

    /************************************************************
     ** 功能：获取数据
     ** 输入：wDataType 获取数据类型
     ** 输出：vData 获取数据
     ** 返回：见返回错误码定义
     ************************************************************/
    virtual int GetData(void *vData, WORD wDataType = 0) = 0;

    /************************************************************
     ** 功能：获取版本
     ** 输入：lSize 数据buffer size
     **      usType 获取类型(1DevIdx版本/2固件版本/3其他)
     ** 输出：szVer 版本数据
     ** 返回：见返回错误码定义
     ************************************************************/
    virtual void GetVersion(char* szVer, long lSize, ushort usType) = 0;

    /************************************************************
    ** 功能：检测卡
    ** 输入：无
    ** 输出：IDCstatus
    ** 返回：见返回错误码定义
    ************************************************************/
    //virtual int DetectCard(IDC_IDCSTAUTS &IDCstatus) = 0;

    /************************************************************
    ** 功能：获取固件版本
    ** 输入：无
    ** 输出：pFWVersion：保存固件版本
             uLen：版本有效信息长度
    ** 返回：见返回错误码定义
    ************************************************************/
   // virtual int GetFWVersion(char pFWVersion[MAX_LEN_FWVERSION], unsigned int &uLen) = 0;

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
    //virtual int ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen) = 0;
};

extern "C" DEVIDX_EXPORT long CreateIDevIDX(LPCSTR lpDevType, IDevIDX *&pDev);
