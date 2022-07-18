/***************************************************************************
* 文件名称: IDevBCR.h
* 文件描述: 声明条码设备底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2019年6月15日
* 文件版本: 1.0.0.1
***************************************************************************/
#if !defined(SPBuild_OPTIM)     // 非优化工程

#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "string.h"

#if defined(IDEVBCR_LIBRARY)
#  define DEVBCR_EXPORT Q_DECL_EXPORT
#else
#  define DEVBCR_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
enum DEVICE_STATUS
{
    DEVICE_OFFLINE = 0,
    DEVICE_ONLINE  = 1,
    DEVICE_HWERROR = 2,
};
//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_bcr_status
{
    WORD wDevice;               // 设备状态
    char szErrCode[8];          // 三位的错误码

    tag_dev_bcr_status() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_dev_bcr_status)); }
} DEVBCRSTATUS;

//////////////////////////////////////////////////////////////////////////
struct IDevBCR
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
    virtual long Open(LPCSTR lpMode) = 0;
    // 关闭连接
    virtual long Close() = 0;
    // 复位
    virtual long Reset() = 0;
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo) = 0;
    // 取状态
    virtual long GetStatus(DEVBCRSTATUS &stStatus) = 0;
    // 读取二维码
    virtual long ReadBCR(DWORD &dwType, LPSTR lpData, DWORD &dwLen, long lTimeOut) = 0;
    // 取消读取二维码
    virtual long CancelRead() = 0;
};

extern "C" DEVBCR_EXPORT long CreateIDevBCR(LPCSTR lpDevType, IDevBCR *&pDev);
//////////////////////////////////////////////////////////////////////////

#else   // 优化工程

//***************************************************************************
//  以下为IDevBCR接口优化版本
//***************************************************************************

#pragma once

#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "XFSBCR.H"
#include "IDevDEF.h"
#include <string.h>
#include <stdio.h>

//***************************************************************************
//
//****************************************************************************
#if defined(IDEVBCR_LIBRARY)
#define DEVBCR_EXPORT     Q_DECL_EXPORT
#else
#define DEVBCR_EXPORT     Q_DECL_IMPORT
#endif

//***************************************************************************
// 错误码相关定义
//***************************************************************************
// 成功
#define BCR_SUCCESS                 (0)     // 操作成功

// 警告
#define ERR_BCR_USER_CANCEL         (2)     // 用户取消
#define ERR_BCR_DEVICE_REOPEN       (3)     // 设备重连
#define ERR_BCR_CMD_RUNNING         (4)     // 有命令执行中

// 通用错误
#define ERR_BCR_LIBRARY             (-1)    // 动态库加载失败
#define ERR_BCR_PARAM_ERR           (-2)    // 参数错误
#define ERR_BCR_UNSUP_CMD           (-3)    // 不支持的指令
#define ERR_BCR_DATA_FAIL           (-4)    // 数据错误
#define ERR_BCR_TIMEOUT             (-5)    // 超时
#define ERR_BCR_USER_ERR            (-6)    // 用户使用错误
#define ERR_BCR_OTHER               (-98)   // 其它错误(知道错误原因,不细分类别)
#define ERR_BCR_UNKNOWN             (-99)   // 未知错误(不知原因的错误)

// 通信错误
#define ERR_BCR_COMM_ERR            (-100)  // 通讯错误
#define ERR_BCR_READ_ERR            (-101)  // 读数据错误
#define ERR_BCR_WRITE_ERR           (-102)  // 写数据错误
#define ERR_BCR_READ_TIMEOUT        (-103)  // 读数据超时
#define ERR_BCR_WRITE_TIMEOUT       (-104)  // 写数据超时
#define ERR_BCR_RESP_ERR            (-105)  // 应答错误
#define ERR_BCR_RESP_NOT_COMP       (-106)  // 应答数据不完整
#define ERR_BCR_COMM_RUN            (-107)  // 通信命令执行错误

// 设备错误
#define ERR_BCR_DEV_NOTFOUND        (-201)  // 设备不存在
#define ERR_BCR_DEV_NOTOPEN         (-202)  // 设备没有打开
#define ERR_BCR_DEV_OFFLINE         (-203)  // 设备脱机
#define ERR_BCR_DEV_HWERR           (-204)  // 设备硬件故障
#define ERR_BCR_DEV_STAT_ERR        (-205)  // 设备状态出错


//***************************************************************************
// 使用 IDevDEF.h 通用定义部分
// 1. SetData/GetData 数据类型 (0~50为共通使用, 50以上为各模块自行定义)
// 2. GetVersion 数据类型
// 3. 设备打开方式结构体变量, 适用于 SET_DEV_OPENMODE 参数传递
//***************************************************************************


//***************************************************************************
// 设备状态相关定义
//***************************************************************************
//　Status.Device返回状态(引用IDevDEF.h中已定义类型)
typedef EN_DEVICE_STATUS  DEVBCR_DEVICE_STATUS;

//　status.Scanner返回状态(扫描器状态)
enum DEVBCR_SCANNER_STATUS
{
    SCAN_STAT_ON                    = 0,    // 扫描器就绪
    SCAN_STAT_OFF                   = 1,    // 扫描器不可用
    SCAN_STAT_INOP                  = 2,    // 扫描器由于硬件故障不可操作
    SCAN_STAT_UNKNOWN               = 3,    // 扫描器状态未知
};

//　status.Guid_Lights返回状态(设备指示灯显示状态)
enum DEVBCR_GUIDLIGHTS_STATUS
{
    GUIDL_STAT_NOTAVAILABLE         = 0,    // 不可用
    GUIDL_STAT_OFF                  = 1,    // 关闭
    GUIDL_STAT_SLOWFLASH            = 2,    // 慢闪
    GUIDL_STAT_MEDIUMFLASH          = 3,    // 中闪
    GUIDL_STAT_QUICKFLASH           = 4,    // 快闪
    GUIDL_STAT_CONTINUOUS           = 5,    // 常亮
    GUIDL_STAT_RED                  = 6,    // 红色
    GUIDL_STAT_GREEN                = 7,    // 绿色
    GUIDL_STAT_YELLOW               = 8,    // 黄色
    GUIDL_STAT_BLUE                 = 9,    // 蓝色
    GUIDL_STAT_CYAN                 = 10,   // 蓝绿色
    GUIDL_STAT_MAGENTA              = 11,   // 红紫色
    GUIDL_STAT_WHITE                = 12,   // 白色
};

//　status.Device_Position返回状态(设备位置信息)(引用IDevDEF.h中已定义类型)
typedef EN_DEVICE_POSITION_STATUS  DEVBCR_POSITION_STATUS;

// 设备状态结构体
typedef struct ST_DEV_BCR_STATUS   // 处理后的设备状态
{
    WORD wDevice;               // 设备状态(参考enum DEVBCR_DEVICE_STATUS)
    WORD wBcrScanner;           // 扫描状态(参考enum DEVBCR_SCANNER_STATUS)
    DWORD wGuidLights;          // 指示灯显示状态(参考enum DEVBCR_GUIDLIGHTS_STATUS)
    WORD wPosition;             // 设备位置状态(参考enum DEVBCR_POSITION_STATUS)
    USHORT usPowerTime;         // 设备省电模式恢复秒数
    CHAR szErrCode[32];         // 错误码
    WORD wOtherCode[16];        // 其他状态值,用于非标准WFS/未定义值的返回

    ST_DEV_BCR_STATUS()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(ST_DEV_BCR_STATUS));
        wDevice = DEVICE_STAT_OFFLINE;
        wBcrScanner = SCAN_STAT_UNKNOWN;
        wGuidLights = GUIDL_STAT_NOTAVAILABLE;
        wPosition = DEVPOS_STAT_UNKNOWN;
        usPowerTime = 0;
    }

    int Diff(struct ST_DEV_BCR_STATUS stStat)       // 比较
    {
        if (wDevice != stStat.wDevice ||
            wBcrScanner != stStat.wBcrScanner ||
            wGuidLights != stStat.wGuidLights ||
            wPosition != stStat.wPosition ||
            usPowerTime != stStat.usPowerTime)
        {
            return 1;
        }
        return 0;
    }

    int Copy(struct ST_DEV_BCR_STATUS stStat)       // 复制
    {
        wDevice = stStat.wDevice;
        wBcrScanner = stStat.wBcrScanner;
        wGuidLights = stStat.wGuidLights;
        wPosition = stStat.wPosition;
        usPowerTime = stStat.usPowerTime;
        return 0;
    }

} STDEVBCRSTATUS, *LPSTDEVBCRSTATUS;


//***************************************************************************
// 扫码相关定义
//***************************************************************************
// 条码类型相关定义
enum EN_SYM_TYPE
{
    EN_SYM_ALL                      = 0,    // 所有
    EN_SYM_EAN128                   = 1,    // GS1-128
    EN_SYM_EAN8                     = 2,    // EAN-8
    EN_SYM_EAN8_2                   = 3,    // EAN-8 with 2 digit add-on
    EN_SYM_EAN8_5                   = 4,    // EAN-8 with 5 digit add-on
    EN_SYM_EAN13                    = 5,    // EAN13
    EN_SYM_EAN13_2                  = 6,    // EAN-13 with 2 digit add-on
    EN_SYM_EAN13_5                  = 7,    // EAN-13 with 5 digit add-on
    EN_SYM_JAN13                    = 8,    // JAN-13
    EN_SYM_UPCA                     = 9,    // UPC-A
    EN_SYM_UPCE0                    = 10,   // UPC-E
    EN_SYM_UPCE0_2                  = 11,   // UPC-E with 2 digit add-on
    EN_SYM_UPCE0_5                  = 12,   // UPC-E with 5 digit add-on
    EN_SYM_UPCE1                    = 13,   // UPC-E with leading 1
    EN_SYM_UPCE1_2                  = 14,   // UPC-E with leading land 2 digit add-on
    EN_SYM_UPCE1_5                  = 15,   // UPC-E with leading land 5 digit add-on
    EN_SYM_UPCA_2                   = 16,   // UPC-A with2 digit add-on
    EN_SYM_UPCA_5                   = 17,   // UPC-A with 5 digit add-on
    EN_SYM_CODABAR                  = 18,   // CODABAR (NW-7)
    EN_SYM_ITF                      = 19,   // Interleaved 2 of 5 (ITF)
    EN_SYM_11                       = 20,   // CODE 11 (USD-8)
    EN_SYM_39                       = 21,   // CODE 39
    EN_SYM_49                       = 22,   // CODE 49
    EN_SYM_93                       = 23,   // CODE 93
    EN_SYM_128                      = 24,   // CODE 128
    EN_SYM_MSI                      = 25,   // MSI
    EN_SYM_PLESSEY                  = 26,   // PLESSEY
    EN_SYM_STD2OF5                  = 27,   // STANDARD 2 of 5 (INDUSTRIAL 2 of 5 also)
    EN_SYM_STD2OF5_IATA             = 28,   // STANDARD 2 of 5 (IATA Version)
    EN_SYM_PDF_417                  = 29,   // PDF-417
    EN_SYM_MICROPDF_417             = 30,   // MICROPDF-417
    EN_SYM_DATAMATRIX               = 31,   // GS1 DataMatrix
    EN_SYM_MAXICODE                 = 32,   // MAXICODE
    EN_SYM_CODEONE                  = 33,   // CODE ONE
    EN_SYM_CHANNELCODE              = 34,   // CHANNEL CODE
    EN_SYM_TELEPEN_ORIGINAL         = 35,   // Original TELEPEN
    EN_SYM_TELEPEN_AIM              = 36,   // AIM version of TELEPEN
    EN_SYM_RSS                      = 37,   // GS1 DataBar
    EN_SYM_RSS_EXPANDED             = 38,   // Expanded GS1 DataBar
    EN_SYM_RSS_RESTRICTED           = 39,   // Restricted GS1 DataBar
    EN_SYM_COMPOSITE_CODE_A         = 40,   // Composite Code A Component
    EN_SYM_COMPOSITE_CODE_B         = 41,   // Composite Code B Component
    EN_SYM_COMPOSITE_CODE_C         = 42,   // Composite Code C Component
    EN_SYM_POSICODE_A               = 43,   // Posicode Variation A
    EN_SYM_POSICODE_B               = 44,   // Posicode Variation B
    EN_SYM_TRIOPTIC_CODE_39         = 45,   // Trioptic Code 39
    EN_SYM_CODABLOCK_F              = 46,   // Codablock F
    EN_SYM_CODE_16K                 = 47,   // Code 16K
    EN_SYM_QRCODE                   = 48,   // QR Code
    EN_SYM_AZTEC                    = 49,   // Aztec Codes
    EN_SYM_UKPOST                   = 50,   // UK Post
    EN_SYM_PLANET                   = 51,   // US Postal Planet
    EN_SYM_POSTNET                  = 52,   // US Postal Postnet
    EN_SYM_CANADIANPOST             = 53,   // Canadian Post
    EN_SYM_NETHERLANDSPOST          = 54,   // Netherlands Post
    EN_SYM_AUSTRALIANPOST           = 55,   // Australian Post
    EN_SYM_JAPANESEPOST             = 56,   // Japanese Post
    EN_SYM_CHINESEPOST              = 57,   // Chinese Post
    EN_SYM_KOREANPOST               = 58,   // Korean Post
    EN_SYM_UNKNOWN                  = 99,   // 未知
};

// 扫码入参结构体
typedef struct ST_Dev_BCR_ReadIn
{
    WORD wSymType[255];         // 条码类型[可指定多个]
    WORD wSymDataMode;          // 定义条码数据返回模式(0ASCII/1HEX)
    DWORD dwTimeOut;            // 超时时间
    INT nOtherParam[12];        // 其他参数

    ST_Dev_BCR_ReadIn()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(ST_Dev_BCR_ReadIn));
        memset(wSymType, EN_SYM_ALL, sizeof(WORD) * 255);
        wSymDataMode = 1;

    }
} STREADBCRIN, *LPSTREADBCRIN;

// 条码数据模式宏定义
#define SYMD_ASCII      0
#define SYMD_HEX        1

// 扫码回参结构体
typedef struct ST_Dev_BCR_ReadOut
{
    WORD wSymType;              // 条码类型
    CHAR szSymData[4096];       // 条码数据
    LPSTR lpSymData;            // 条码数据内存空间(需主动申请/释放)
    DWORD dwSymDataSize;        // 条码数据有效大小
    WORD wSymDataMode;          // 条码数据模式(0ASCII/1HEX)

    ST_Dev_BCR_ReadOut()
    {
        lpSymData = nullptr;
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(ST_Dev_BCR_ReadOut));
        wSymType = EN_SYM_UNKNOWN;
        if (lpSymData != nullptr)
        {
            free(lpSymData);
        }
        lpSymData = nullptr;
        wSymDataMode = 1;
    }
} STREADBCROUT, *LPSTREADBCROUT;


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
    GLIGHTS_TYPE_BLUDGREEN  = 16,   // 蓝绿灯
    GLIGHTS_TYPE_REDMAGENTA = 32,   // 红紫灯
    GLIGHTS_TYPE_WHITE      = 64,   // 白灯
};

// LED灯控制
enum GUID_LIGHTS_ACTION
{
    GLIGHTS_ACT_OPEN        = 0,    // 打开
    GLIGHTS_ACT_CLOSE       = 1,    // 关闭
    GLIGHTS_ACT_FLASH_SLOW  = 2,    // 闪烁(慢闪)
    GLIGHTS_ACT_FLASH_MED   = 3,    // 闪烁(中闪)
    GLIGHTS_ACT_FLASH_QUICK = 4,    // 闪烁(快闪)
};

// 指示灯控制结构体
typedef struct Dev_BCR_GUID_LIGHTS_CONTROL
{
    WORD            wContMode;      // 控制模式(0自动/1手动)
    GUID_LIGHTS_TYPE enLigType;     // 指示灯类型
    GUID_LIGHTS_ACTION enLigAct;    // 指示灯控制
    WORD            wDelay;         // 间隔时间(单位:毫秒)
    Dev_BCR_GUID_LIGHTS_CONTROL()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_BCR_GUID_LIGHTS_CONTROL));
    }
} STGLIGHTSCONT, *LPSTGLIGHTSCONT;


//****************************************************************************
// 鸣响控制相关定义
//****************************************************************************
// 鸣响控制结构体
typedef struct Dev_BCR_BEEP_CONTROL
{
    WORD wContMode;         // 控制模式(0自动/1手动)
    WORD wBeepMsec;         // 鸣响一次时间(单位:毫秒)
    WORD wBeepInterval;     // 鸣响频率/鸣响间隔(单位:毫秒)
    WORD wBeepCount;        // 每次鸣响的次数(单位:毫秒)
    Dev_BCR_BEEP_CONTROL()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_BCR_BEEP_CONTROL));
    }
} STBEEPCONT, *LPSTBEEPCONT;


//****************************************************************************
// 接口类定义
//****************************************************************************
struct IDevBCR
{
    /************************************************************
    ** 功能: 释放接口
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Release()
    {
        return ERR_BCR_UNSUP_CMD;
    }
    /************************************************************
    ** 功能: 打开与设备的连接
    ** 输入: pMode: 自定义OPEN参数字串,格式自定义
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Open(const char *pMode)
    {
        return ERR_BCR_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 关闭与设备的连接
    ** 输入: 无
    ** 输出: 无
    ** 返回: 无
    ************************************************************/
    virtual int Close()
    {
        return ERR_BCR_UNSUP_CMD;
    }

    /************************************************************
     ** 功能: 取消
     ** 输入: usMode 取消类型
     ** 输出: 无
     ** 返回: 见返回错误码定义
     ************************************************************/
    virtual int Cancel(unsigned short usMode = 0)
    {
        return ERR_BCR_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 设备复位
    ** 输入: usParam 复位参数(根据不同设备要求填写)
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Reset(unsigned short usParam = 0)
    {
        return ERR_BCR_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 取设备状态
    ** 输入: 无
    ** 输出: stStatus 设备状态结构体
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int GetStatus(STDEVBCRSTATUS &stStatus)
    {
        return ERR_BCR_UNSUP_CMD;
    }

    /************************************************************
    ** 功能: 扫码
    ** 输入: stReadIn
    ** 输出: stReadOut
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ReadBCR(STREADBCRIN stReadIn, STREADBCROUT &stReadOut)
    {
        return ERR_BCR_UNSUP_CMD;
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
        return ERR_BCR_UNSUP_CMD;
    }

    /************************************************************
     ** 功能: 获取数据
     ** 输入: usType  获取数据类型
     ** 输出: vData   获取数据
     ** 返回: 见返回错误码定义
     ************************************************************/
    virtual int GetData(unsigned short usType, void *vData)
    {
        return ERR_BCR_UNSUP_CMD;
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
        return ERR_BCR_UNSUP_CMD;
    }
};

extern "C" DEVBCR_EXPORT long CreateIDevBCR(LPCSTR lpDevType, IDevBCR *&pDev);


//***************************************************************************
// IDevBCR定义 设备相关变量转换 类定义(统一转换)
//***************************************************************************
class ConvertVarBCR
{
private:
    CHAR m_szErrStr[1024];
public:
    // 设备状态转换为WFS格式
    WORD ConvertDeviceStatus2WFS(WORD wDevStat)
    {
        switch (wDevStat)
        {
            case DEVICE_STAT_ONLINE     /* 设备正常 */     : return WFS_BCR_DEVONLINE;
            case DEVICE_STAT_OFFLINE    /* 设备脱机 */     : return WFS_BCR_DEVOFFLINE;
            case DEVICE_STAT_POWEROFF   /* 设备断电 */     : return WFS_BCR_DEVPOWEROFF;
            case DEVICE_STAT_NODEVICE   /* 设备不存在 */    : return WFS_BCR_DEVNODEVICE;
            case DEVICE_STAT_HWERROR    /* 设备故障 */     : return WFS_BCR_DEVHWERROR;
            case DEVICE_STAT_USERERROR  /* 用户操作故障 */  : return WFS_BCR_DEVUSERERROR;
            case DEVICE_STAT_BUSY       /* 设备读写中 */    : return WFS_BCR_DEVBUSY;
            case DEVICE_STAT_FRAUDAT    /* 设备出现欺诈企图 */: return WFS_BCR_DEVFRAUDATTEMPT;
            default: return WFS_BCR_DEVOFFLINE;
        }
    }

    // Scanner状态转换为WFS格式
    WORD ConvertScannerStatus2WFS(WORD wStat)
    {
        switch (wStat)
        {
            case SCAN_STAT_ON           /* 扫描器就绪 */     : return WFS_BCR_SCANNERON;
            case SCAN_STAT_OFF          /* 扫描器不可用 */    : return WFS_BCR_SCANNEROFF;
            case SCAN_STAT_INOP         /* 扫描器由于硬件故障不可操作 */ : return WFS_BCR_SCANNERINOP;
            case SCAN_STAT_UNKNOWN      /* 扫描器状态未知 */   : return WFS_BCR_SCANNERUNKNOWN;
            default: return WFS_BCR_SCANNERUNKNOWN;
        }
    }

    // Device_Position状态转换为WFS格式
    WORD ConvertDevPosStatus2WFS(WORD wStat)
    {
        switch (wStat)
        {
            case DEVPOS_STAT_INPOS      /* 设备处于可正常操作位置或固定位置 */  : return WFS_BCR_DEVICEINPOSITION;
            case DEVPOS_STAT_NOTINPOS   /* 设备从可正常操作位置或固定位置移除 */: return WFS_BCR_DEVICENOTINPOSITION;
            case DEVPOS_STAT_UNKNOWN    /* 设备位置未知 */                  : return WFS_BCR_DEVICEPOSUNKNOWN;
            case DEVPOS_STAT_NOTSUPP    /* 设备不支持位置检测 */             : return WFS_BCR_DEVICEPOSUNKNOWN;
            default: return WFS_BCR_DEVICEPOSUNKNOWN;
        }
    }

    // 错误码转换为WFS格式
    LONG ConvertDevErrCode2WFS(INT nRet)
    {
        switch (nRet)
        {
            // 成功
            case BCR_SUCCESS:               return WFS_SUCCESS;                 // 操作成功->成功
            // 警告
            case ERR_BCR_USER_CANCEL:       return WFS_ERR_CANCELED;            // 用户取消->取消
            case ERR_BCR_DEVICE_REOPEN:     return WFS_ERR_CONNECTION_LOST;     // 设备重连
            case ERR_BCR_CMD_RUNNING:       return WFS_SUCCESS;                 // 有命令执行中
            // 通用错误
            case ERR_BCR_LIBRARY:           return WFS_ERR_SOFTWARE_ERROR;      // 动态库加载失败
            case ERR_BCR_PARAM_ERR:         return WFS_ERR_INVALID_DATA;        // 参数错误
            case ERR_BCR_UNSUP_CMD:         return WFS_ERR_UNSUPP_COMMAND;      // 不支持的指令
            case ERR_BCR_DATA_FAIL:         return WFS_ERR_HARDWARE_ERROR;      // 数据错误
            case ERR_BCR_TIMEOUT:           return WFS_ERR_TIMEOUT;             // 超时
            case ERR_BCR_USER_ERR:          return WFS_ERR_USER_ERROR;          // 用户使用错误
            case ERR_BCR_OTHER:             return WFS_ERR_HARDWARE_ERROR;      // 其它错误(知道错误原因,不细分类别)
            case ERR_BCR_UNKNOWN:           return WFS_ERR_HARDWARE_ERROR;      // 未知错误(不知原因的错误)
            // 通信错误
            case ERR_BCR_COMM_ERR:          return WFS_ERR_CONNECTION_LOST;     // 通讯错误
            case ERR_BCR_READ_ERR:          return WFS_ERR_HARDWARE_ERROR;      // 读数据错误
            case ERR_BCR_WRITE_ERR:         return WFS_ERR_HARDWARE_ERROR;      // 写数据错误
            case ERR_BCR_READ_TIMEOUT:      return WFS_ERR_TIMEOUT;             // 读数据超时
            case ERR_BCR_WRITE_TIMEOUT:     return WFS_ERR_TIMEOUT;             // 写数据超时
            case ERR_BCR_COMM_RUN:          return WFS_ERR_HARDWARE_ERROR;      // 通信命令执行错误
            // 设备错误
            case ERR_BCR_DEV_NOTFOUND:      return WFS_ERR_HARDWARE_ERROR;      // 设备不存在
            case ERR_BCR_DEV_NOTOPEN:       return WFS_ERR_HARDWARE_ERROR;      // 设备没有打开
            case ERR_BCR_DEV_OFFLINE:       return WFS_ERR_CONNECTION_LOST;     // 设备脱机
            case ERR_BCR_DEV_HWERR:         return WFS_ERR_HARDWARE_ERROR;      // 设备硬件故障
            case ERR_BCR_DEV_STAT_ERR:      return WFS_ERR_HARDWARE_ERROR;      // 设备状态错误
            default:                        return WFS_ERR_HARDWARE_ERROR;
        }
    }

    CHAR* ConvertDevErrCodeToStr(INT nRet)
    {
        #define CONV_BCR_CODE_STR(RET, STR) \
            sprintf(m_szErrStr, "%d|%s", RET, STR); \
            return m_szErrStr;

        memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

        switch(nRet)
        {
            // 成功
            case BCR_SUCCESS:               CONV_BCR_CODE_STR(nRet, "操作成功")
            // 警告
            case ERR_BCR_USER_CANCEL:       CONV_BCR_CODE_STR(nRet, "用户取消")
            case ERR_BCR_DEVICE_REOPEN:     CONV_BCR_CODE_STR(nRet, "设备重连")
            case ERR_BCR_CMD_RUNNING:       CONV_BCR_CODE_STR(nRet, "有命令执行中")
            // 通用错误
            case ERR_BCR_LIBRARY:           CONV_BCR_CODE_STR(nRet, "动态库加载失败")
            case ERR_BCR_PARAM_ERR:         CONV_BCR_CODE_STR(nRet, "参数错误")
            case ERR_BCR_UNSUP_CMD:         CONV_BCR_CODE_STR(nRet, "不支持的指令")
            case ERR_BCR_DATA_FAIL:         CONV_BCR_CODE_STR(nRet, "数据错误")
            case ERR_BCR_TIMEOUT:           CONV_BCR_CODE_STR(nRet, "超时")
            case ERR_BCR_USER_ERR:          CONV_BCR_CODE_STR(nRet, "用户使用错误")
            case ERR_BCR_OTHER:             CONV_BCR_CODE_STR(nRet, "其它错误(知道错误原因,不细分类别)")
            case ERR_BCR_UNKNOWN:           CONV_BCR_CODE_STR(nRet, "未知错误(不知原因的错误)")
            // 通信错误
            case ERR_BCR_COMM_ERR:          CONV_BCR_CODE_STR(nRet, "通讯错误")
            case ERR_BCR_READ_ERR:          CONV_BCR_CODE_STR(nRet, "读数据错误")
            case ERR_BCR_WRITE_ERR:         CONV_BCR_CODE_STR(nRet, "写数据错误")
            case ERR_BCR_READ_TIMEOUT:      CONV_BCR_CODE_STR(nRet, "读数据超时")
            case ERR_BCR_WRITE_TIMEOUT:     CONV_BCR_CODE_STR(nRet, "写数据超时")
            case ERR_BCR_RESP_ERR:          CONV_BCR_CODE_STR(nRet, "应答错误")
            case ERR_BCR_RESP_NOT_COMP:     CONV_BCR_CODE_STR(nRet, "应答数据不完整")
            case ERR_BCR_COMM_RUN:          CONV_BCR_CODE_STR(nRet, "通信命令执行错误")
            // 设备错误
            case ERR_BCR_DEV_NOTFOUND:      CONV_BCR_CODE_STR(nRet, "设备不存在")
            case ERR_BCR_DEV_NOTOPEN:       CONV_BCR_CODE_STR(nRet, "设备没有打开")
            case ERR_BCR_DEV_OFFLINE:       CONV_BCR_CODE_STR(nRet, "设备脱机")
            case ERR_BCR_DEV_HWERR:         CONV_BCR_CODE_STR(nRet, "设备硬件故障")
            case ERR_BCR_DEV_STAT_ERR:      CONV_BCR_CODE_STR(nRet, "设备状态错误")
            default:                        CONV_BCR_CODE_STR(nRet, "未定义错误");
        }

        return m_szErrStr;
    }

    // DevBCR条码类型转换为WFS类型
    INT ConvertDevSymDevToWFS(INT nSym)
    {
        switch(nSym)
        {
            case EN_SYM_EAN128              : return WFS_BCR_SYM_EAN128;            // GS1-128
            case EN_SYM_EAN8                : return WFS_BCR_SYM_EAN8;              // EAN-8
            case EN_SYM_EAN8_2              : return WFS_BCR_SYM_EAN8_2;            // EAN-8 with 2 digit add-on
            case EN_SYM_EAN8_5              : return WFS_BCR_SYM_EAN8_5;            // EAN-8 with 5 digit add-on
            case EN_SYM_EAN13               : return WFS_BCR_SYM_EAN13;             // EAN13
            case EN_SYM_EAN13_2             : return WFS_BCR_SYM_EAN13_2;           // EAN-13 with 2 digit add-on
            case EN_SYM_EAN13_5             : return WFS_BCR_SYM_EAN13_5;           // EAN-13 with 5 digit add-on
            case EN_SYM_JAN13               : return WFS_BCR_SYM_JAN13;             // JAN-13
            case EN_SYM_UPCA                : return WFS_BCR_SYM_UPCA;              // UPC-A
            case EN_SYM_UPCE0               : return WFS_BCR_SYM_UPCE0;             // UPC-E
            case EN_SYM_UPCE0_2             : return WFS_BCR_SYM_UPCE0_2;           // UPC-E with 2 digit add-on
            case EN_SYM_UPCE0_5             : return WFS_BCR_SYM_UPCE0_5;           // UPC-E with 5 digit add-on
            case EN_SYM_UPCE1               : return WFS_BCR_SYM_UPCE1;             // UPC-E with leading 1
            case EN_SYM_UPCE1_2             : return WFS_BCR_SYM_UPCE1_2;           // UPC-E with leading land 2 digit add-on
            case EN_SYM_UPCE1_5             : return WFS_BCR_SYM_UPCE1_5;           // UPC-E with leading land 5 digit add-on
            case EN_SYM_UPCA_2              : return WFS_BCR_SYM_UPCA_2;            // UPC-A with2 digit add-on
            case EN_SYM_UPCA_5              : return WFS_BCR_SYM_UPCA_5;            // UPC-A with 5 digit add-on
            case EN_SYM_CODABAR             : return WFS_BCR_SYM_CODABAR;           // CODABAR (NW-7)
            case EN_SYM_ITF                 : return WFS_BCR_SYM_ITF;               // Interleaved 2 of 5 (ITF)
            case EN_SYM_11                  : return WFS_BCR_SYM_11;                // CODE 11 (USD-8)
            case EN_SYM_39                  : return WFS_BCR_SYM_39;                // CODE 39
            case EN_SYM_49                  : return WFS_BCR_SYM_49;                // CODE 49
            case EN_SYM_93                  : return WFS_BCR_SYM_93;                // CODE 93
            case EN_SYM_128                 : return WFS_BCR_SYM_128;               // CODE 128
            case EN_SYM_MSI                 : return WFS_BCR_SYM_MSI;               // MSI
            case EN_SYM_PLESSEY             : return WFS_BCR_SYM_PLESSEY;           // PLESSEY
            case EN_SYM_STD2OF5             : return WFS_BCR_SYM_STD2OF5;           // STANDARD 2 of 5 (INDUSTRIAL 2 of 5 also)
            case EN_SYM_STD2OF5_IATA        : return WFS_BCR_SYM_STD2OF5_IATA;      // STANDARD 2 of 5 (IATA Version)
            case EN_SYM_PDF_417             : return WFS_BCR_SYM_PDF_417;           // PDF-417
            case EN_SYM_MICROPDF_417        : return WFS_BCR_SYM_MICROPDF_417;      // MICROPDF-417
            case EN_SYM_DATAMATRIX          : return WFS_BCR_SYM_DATAMATRIX;        // GS1 DataMatrix
            case EN_SYM_MAXICODE            : return WFS_BCR_SYM_MAXICODE;          // MAXICODE
            case EN_SYM_CODEONE             : return WFS_BCR_SYM_CODEONE;           // CODE ONE
            case EN_SYM_CHANNELCODE         : return WFS_BCR_SYM_CHANNELCODE;       // CHANNEL CODE
            case EN_SYM_TELEPEN_ORIGINAL    : return WFS_BCR_SYM_TELEPEN_ORIGINAL;  // Original TELEPEN
            case EN_SYM_TELEPEN_AIM         : return WFS_BCR_SYM_TELEPEN_AIM;       // AIM version of TELEPEN
            case EN_SYM_RSS                 : return WFS_BCR_SYM_RSS;               // GS1 DataBar
            case EN_SYM_RSS_EXPANDED        : return WFS_BCR_SYM_RSS_EXPANDED;      // Expanded GS1 DataBar
            case EN_SYM_RSS_RESTRICTED      : return WFS_BCR_SYM_RSS_RESTRICTED;    // Restricted GS1 DataBar
            case EN_SYM_COMPOSITE_CODE_A    : return WFS_BCR_SYM_COMPOSITE_CODE_A;  // Composite Code A Component
            case EN_SYM_COMPOSITE_CODE_B    : return WFS_BCR_SYM_COMPOSITE_CODE_B;  // Composite Code B Component
            case EN_SYM_COMPOSITE_CODE_C    : return WFS_BCR_SYM_COMPOSITE_CODE_C;  // Composite Code C Component
            case EN_SYM_POSICODE_A          : return WFS_BCR_SYM_POSICODE_A;        // Posicode Variation A
            case EN_SYM_POSICODE_B          : return WFS_BCR_SYM_POSICODE_B;        // Posicode Variation B
            case EN_SYM_TRIOPTIC_CODE_39    : return WFS_BCR_SYM_TRIOPTIC_CODE_39;  // Trioptic Code 39
            case EN_SYM_CODABLOCK_F         : return WFS_BCR_SYM_CODABLOCK_F;       // Codablock F
            case EN_SYM_CODE_16K            : return WFS_BCR_SYM_CODE_16K;          // Code 16K
            case EN_SYM_QRCODE              : return WFS_BCR_SYM_QRCODE;            // QR Code
            case EN_SYM_AZTEC               : return WFS_BCR_SYM_AZTEC;             // Aztec Codes
            case EN_SYM_UKPOST              : return WFS_BCR_SYM_UKPOST;            // UK Post
            case EN_SYM_PLANET              : return WFS_BCR_SYM_PLANET;            // US Postal Planet
            case EN_SYM_POSTNET             : return WFS_BCR_SYM_POSTNET;           // US Postal Postnet
            case EN_SYM_CANADIANPOST        : return WFS_BCR_SYM_CANADIANPOST;      // Canadian Post
            case EN_SYM_NETHERLANDSPOST     : return WFS_BCR_SYM_NETHERLANDSPOST;   // Netherlands Post
            case EN_SYM_AUSTRALIANPOST      : return WFS_BCR_SYM_AUSTRALIANPOST;    // Australian Post
            case EN_SYM_JAPANESEPOST        : return WFS_BCR_SYM_JAPANESEPOST;      // Japanese Post
            case EN_SYM_CHINESEPOST         : return WFS_BCR_SYM_CHINESEPOST;       // Chinese Post
            case EN_SYM_KOREANPOST          : return WFS_BCR_SYM_KOREANPOST;        // Korean Post
            default                         : return WFS_BCR_SYM_UNKNOWN;           // 未知
        }
    }

    // DevBCR条码类型转换为WFS类型
    INT ConvertWFSSymDevToDev(INT nWFSSym)
    {
        switch(nWFSSym)
        {
            case WFS_BCR_SYM_EAN128             : return EN_SYM_EAN128;             // GS1-128
            case WFS_BCR_SYM_EAN8               : return EN_SYM_EAN8;               // EAN-8
            case WFS_BCR_SYM_EAN8_2             : return EN_SYM_EAN8_2;             // EAN-8 with 2 digit add-on
            case WFS_BCR_SYM_EAN8_5             : return EN_SYM_EAN8_5;             // EAN-8 with 5 digit add-on
            case WFS_BCR_SYM_EAN13              : return EN_SYM_EAN13;              // EAN13
            case WFS_BCR_SYM_EAN13_2            : return EN_SYM_EAN13_2;            // EAN-13 with 2 digit add-on
            case WFS_BCR_SYM_EAN13_5            : return EN_SYM_EAN13_5;            // EAN-13 with 5 digit add-on
            case WFS_BCR_SYM_JAN13              : return EN_SYM_JAN13;              // JAN-13
            case WFS_BCR_SYM_UPCA               : return EN_SYM_UPCA;               // UPC-A
            case WFS_BCR_SYM_UPCE0              : return EN_SYM_UPCE0;              // UPC-E
            case WFS_BCR_SYM_UPCE0_2            : return EN_SYM_UPCE0_2;            // UPC-E with 2 digit add-on
            case WFS_BCR_SYM_UPCE0_5            : return EN_SYM_UPCE0_5;            // UPC-E with 5 digit add-on
            case WFS_BCR_SYM_UPCE1              : return EN_SYM_UPCE1;              // UPC-E with leading 1
            case WFS_BCR_SYM_UPCE1_2            : return EN_SYM_UPCE1_2;            // UPC-E with leading land 2 digit add-on
            case WFS_BCR_SYM_UPCE1_5            : return EN_SYM_UPCE1_5;            // UPC-E with leading land 5 digit add-on
            case WFS_BCR_SYM_UPCA_2             : return EN_SYM_UPCA_2;             // UPC-A with2 digit add-on
            case WFS_BCR_SYM_UPCA_5             : return EN_SYM_UPCA_5;             // UPC-A with 5 digit add-on
            case WFS_BCR_SYM_CODABAR            : return EN_SYM_CODABAR;            // CODABAR (NW-7)
            case WFS_BCR_SYM_ITF                : return EN_SYM_ITF;                // Interleaved 2 of 5 (ITF)
            case WFS_BCR_SYM_11                 : return EN_SYM_11;                 // CODE 11 (USD-8)
            case WFS_BCR_SYM_39                 : return EN_SYM_39;                 // CODE 39
            case WFS_BCR_SYM_49                 : return EN_SYM_49;                 // CODE 49
            case WFS_BCR_SYM_93                 : return EN_SYM_93;                 // CODE 93
            case WFS_BCR_SYM_128                : return EN_SYM_128;                // CODE 128
            case WFS_BCR_SYM_MSI                : return EN_SYM_MSI;                // MSI
            case WFS_BCR_SYM_PLESSEY            : return EN_SYM_PLESSEY;            // PLESSEY
            case WFS_BCR_SYM_STD2OF5            : return EN_SYM_STD2OF5;            // STANDARD 2 of 5 (INDUSTRIAL 2 of 5 also)
            case WFS_BCR_SYM_STD2OF5_IATA       : return EN_SYM_STD2OF5_IATA;       // STANDARD 2 of 5 (IATA Version)
            case WFS_BCR_SYM_PDF_417            : return EN_SYM_PDF_417;            // PDF-417
            case WFS_BCR_SYM_MICROPDF_417       : return EN_SYM_MICROPDF_417;       // MICROPDF-417
            case WFS_BCR_SYM_DATAMATRIX         : return EN_SYM_DATAMATRIX;         // GS1 DataMatrix
            case WFS_BCR_SYM_MAXICODE           : return EN_SYM_MAXICODE;           // MAXICODE
            case WFS_BCR_SYM_CODEONE            : return EN_SYM_CODEONE;            // CODE ONE
            case WFS_BCR_SYM_CHANNELCODE        : return EN_SYM_CHANNELCODE;        // CHANNEL CODE
            case WFS_BCR_SYM_TELEPEN_ORIGINAL   : return EN_SYM_TELEPEN_ORIGINAL;   // Original TELEPEN
            case WFS_BCR_SYM_TELEPEN_AIM        : return EN_SYM_TELEPEN_AIM;        // AIM version of TELEPEN
            case WFS_BCR_SYM_RSS                : return EN_SYM_RSS;                // GS1 DataBar
            case WFS_BCR_SYM_RSS_EXPANDED       : return EN_SYM_RSS_EXPANDED;       // Expanded GS1 DataBar
            case WFS_BCR_SYM_RSS_RESTRICTED     : return EN_SYM_RSS_RESTRICTED;     // Restricted GS1 DataBar
            case WFS_BCR_SYM_COMPOSITE_CODE_A   : return EN_SYM_COMPOSITE_CODE_A;   // Composite Code A Component
            case WFS_BCR_SYM_COMPOSITE_CODE_B   : return EN_SYM_COMPOSITE_CODE_B;   // Composite Code B Component
            case WFS_BCR_SYM_COMPOSITE_CODE_C   : return EN_SYM_COMPOSITE_CODE_C;   // Composite Code C Component
            case WFS_BCR_SYM_POSICODE_A         : return EN_SYM_POSICODE_A;         // Posicode Variation A
            case WFS_BCR_SYM_POSICODE_B         : return EN_SYM_POSICODE_B;         // Posicode Variation B
            case WFS_BCR_SYM_TRIOPTIC_CODE_39   : return EN_SYM_TRIOPTIC_CODE_39;   // Trioptic Code 39
            case WFS_BCR_SYM_CODABLOCK_F        : return EN_SYM_CODABLOCK_F;        // Codablock F
            case WFS_BCR_SYM_CODE_16K           : return EN_SYM_CODE_16K;           // Code 16K
            case WFS_BCR_SYM_QRCODE             : return EN_SYM_QRCODE;             // QR Code
            case WFS_BCR_SYM_AZTEC              : return EN_SYM_AZTEC;              // Aztec Codes
            case WFS_BCR_SYM_UKPOST             : return EN_SYM_UKPOST;             // UK Post
            case WFS_BCR_SYM_PLANET             : return EN_SYM_PLANET;             // US Postal Planet
            case WFS_BCR_SYM_POSTNET            : return EN_SYM_POSTNET;            // US Postal Postnet
            case WFS_BCR_SYM_CANADIANPOST       : return EN_SYM_CANADIANPOST;       // Canadian Post
            case WFS_BCR_SYM_NETHERLANDSPOST    : return EN_SYM_NETHERLANDSPOST;    // Netherlands Post
            case WFS_BCR_SYM_AUSTRALIANPOST     : return EN_SYM_AUSTRALIANPOST;     // Australian Post
            case WFS_BCR_SYM_JAPANESEPOST       : return EN_SYM_JAPANESEPOST;       // Japanese Post
            case WFS_BCR_SYM_CHINESEPOST        : return EN_SYM_CHINESEPOST;        // Chinese Post
            case WFS_BCR_SYM_KOREANPOST         : return EN_SYM_KOREANPOST;         // Korean Post
            default:                              return EN_SYM_UNKNOWN;            // 未知
        }
    }
};

#endif  // !defined(SPBuild_OPTIM)

// -------------------------------- END -----------------------------------

