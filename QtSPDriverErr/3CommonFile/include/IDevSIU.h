#if !defined(SPBuild_OPTIM)     // 非优化工程

#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "IAllDevPort.h"
//////////////////////////////////////////////////////////////////////////
#if defined(IDEVSIU_LIBRARY)
#define DEVSIU_EXPORT     Q_DECL_EXPORT
#else
#define DEVSIU_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
//错误码说明：
//------------------------ 错误码定义 ----------------------------------
#define ERR_IOMC_SUCCESS                (0)     // 成功，通讯成功，设备操作也成功
#define ERR_IOMC_WARN                   (1)     // 命令执行成功，可是有警告产生了
#define ERR_IOMC_HARDWARE_ERROR         (-1)    // 执行指令异常终止
#define ERR_IOMC_END_STATE              (-2)    // 返回的结束状态有误
#define ERR_IOMC_PARAM                  (-3)    // 参数错误
#define ERR_IOMC_COMM                   (-4)    // 通讯失败
#define ERR_IOMC_DEVBUSY                (-5)    // 设备忙
#define ERR_IOMC_CREATETHREAD           (-6)    // 创建线程失败
#define ERR_IOMC_NO_DEVICE              (-7)    // 如果指定名的设备不存在，创建对象时返回
#define ERR_IOMC_RETURN_DATA_LEN        (-8)    // 数据长度不合法
#define ERR_IOMC_NOLOADDLL              (-9)    // 没有加载通讯库
#define ERR_IOMC_RESPDATA_ERR           (-10)   // 数据解析错误
#define ERR_IOMC_INVALID_COMMAN         (-11)   // 无效命令
#define ERR_IOMC_NOT_OPEN               (-12)   // 没有打开连接
#define ERR_IOMC_NOT_SUPPORT            (-13)   // 不支持功能
#define ERR_IOMC_WRITE_ERROR            (-14)   // 发送数据错误
#define ERR_IOMC_READ_ERROR             (-15)   // 接收数据错误
#define ERR_IOMC_READ_TIMEOUT           (-16)   // 读数据超时


//以下错误是USB相关错误
#define ERR_IOMC_USB_PARAM_ERR          (-101)  // 参数错误
#define ERR_IOMC_USB_CMD_ERR            (-102)  // 指令错误
#define ERR_IOMC_USB_CONN_ERR           (-103)  // 连接错误
#define ERR_IOMC_USB_CANCELLED          (-104)  // 指令取消
#define ERR_IOMC_USB_BUSY               (-105)  // 操作忙
#define ERR_IOMC_USB_NODLL              (-106)  // 驱动库不存在
#define ERR_IOMC_USB_NOFUNC             (-107)  // 驱动库入口函数不存在
#define ERR_IOMC_USB_OTHER              (-108)  // 其他USB错误

//以下错误是固件升级相关错误
#define PDL_NO_UPDATE                   (-201)  // 不用升级固件
#define PDL_DEV_ERR                     (-202)  // 设备异常失败
#define PDL_CONFIG_ERR                  (-203)  // PDL配置错误，或没有找到配置文件
#define PDL_FILE_ERR                    (-204)  // PDL文件错误，或没有找到PDL文件
#define PDL_MD5_ERR                     (-205)  // MD5值校验失败
#define PDL_SWITCH_ERR                  (-206)  // 切换PDL模式失败
#define PDL_SENDHEX_ERR                 (-207)  // 发送固件数据失败
#define PDL_UPDATE_ERR                  (-208)  // 升级固件失败
//////////////////////////////////////////////////////////////////////////
#define DEFSIZE                         (32)
#define IOMC_STATUS_NOCHANGE            (0)
//////////////////////////////////////////////////////////////////////////
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB)
enum DEVICE_STATUS
{
    DEVICE_OFFLINE                          =   0,
    DEVICE_ONLINE                           =   1,
    DEVICE_HWERROR                          =   2,
};
#else
enum DEVICE_STATUS_IOMC                     //30-00-00-00(FS#0004)
{
    DEVICE_OFFLINE                          =   0,
    DEVICE_ONLINE                           =   1,
    DEVICE_HWERROR                          =   2,
};
#endif

//////////////////////////////////////////////////////////////////////////
enum SENSORS_INDEX
{
    SENSORS_OPERATORSWITCH                  =   0,
    SENSORS_TAMPER                          =   1,
    SENSORS_INTTAMPER                       =   2,
    SENSORS_SEISMIC                         =   3,
    SENSORS_HEAT                            =   4,
    SENSORS_PROXIMITY                       =   5,
    SENSORS_AMBLIGHT                        =   6,
    SENSORS_ENHANCEDAUDIO                   =   7,
    SENSORS_CABINET_FAN                     =   15,// 柜体散热风扇
    SENSORS_PANELBOARD                      =   17,// 面板到位检测
    SENSORE_SAFEDOOR_FAN                    =   19,// 保险柜散热风扇
    SENSORS_MODULEINPLACE                   =   21,// 机芯到位检测
    SENSORE_SAFEDOORLOCK                    =   25,// 保险柜门锁报警状态
};

enum SENSORS_STATUS
{
    SENSORS_RUN                             =   0x0001,
    SENSORS_MAINTENANCE                     =   0x0002,
    SENSORS_SUPERVISOR                      =   0x0004,
    SENSORS_OFF                             =   0x0001,
    SENSORS_ON                              =   0x0002,
    SENSORS_NOT_PRESENT                     =   0x0003,
    SENSORS_PRESENT                         =   0x0004
};
//////////////////////////////////////////////////////////////////////////
enum INDICATOR_INDEX
{
    INDICATOR_OPENCLOSE                     = 0,
    INDICATOR_FASCIALIGHT                   = 1, // 框照明灯
    INDICATOR_AUDIO                         = 2,
    INDICATOR_HEATING                       = 3,
};
enum INDICATOR_STATUS
{
    INDICATOR_OFF                           = 0x0001,
    INDICATOR_ON                            = 0x0002,
};
//////////////////////////////////////////////////////////////////////////
enum AUXILIARIE_INDEX
{
    AUXILIARIE_VOLUME                       = 0,
    AUXILIARIE_UPS                          = 1,
    AUXILIARIE_ALARM                        = 2,
    AUXILIARIE_AUDIO                        = 3,
};
enum AUXILIARIE_STATUS
{
    AUXILIARIE_OFF                           = 0x0001,
    AUXILIARIE_ON                            = 0x0002,
};
//////////////////////////////////////////////////////////////////////////
enum GUIDLIGHT_INDEX
{
    GUIDLIGHT_CARDUNIT                      =   0,  //读卡器灯（或发卡机）
    GUIDLIGHT_PINPAD                        =   1,  //密码键盘灯
    GUIDLIGHT_NOTESDISPENSER                =   2,  //出钞指示灯
    GUIDLIGHT_COINDISPENSER                 =   3,  //硬币取款灯/射频卡指示灯
    GUIDLIGHT_RECEIPTPRINTER                =   4,  //凭条打印机灯
    GUIDLIGHT_PASSBOOKPRINTER               =   5,  //存折指示灯
    GUIDLIGHT_ENVDEPOSITORY                 =   6,  //强存/刷磁设备
#if defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN)
    GUIDLIGHT_ENVSCANNER                    =   7,  //票据扫描/支票处理单元
#else
    GUIDLIGHT_CHEQUEUNIT                    =   7,
#endif
    GUIDLIGHT_BILLACCEPTOR                  =   8,  //存款模块灯
    GUIDLIGHT_ENVDISPENSER                  =   9,  //票据发售/UKey
    GUIDLIGHT_DOCUMENTPRINTER               =   10, //A4 激光打印机灯
    GUIDLIGHT_COINACCEPTOR                  =   11, //硬币存款/身份证指示灯
#if defined(SET_BANK_CSCB)
    GUIDLIGHT_UKEY                          =   12, //UKey
#else
    GUIDLIGHT_SCANNER                       =   12, //扫描仪器
#endif
#if defined(SET_BANK_CMBC)
    GUIDLIGHT_ICCARDUNIT                    =   13,
    GUIDLIGHT_RFCARDUNIT                    =   14,
    GUIDLIGHT_TRACKCARDUNIT                 =   15,
    GUIDLIGHT_IDCARDREADER                  =   16,
    GUIDLIGHT_FINGER                        =   17,
    GUIDLIGHT_KEYDISPENSER                  =   18,
    GUIDLIGHT_FORCEACCEPTOR                 =   19,
    GUIDLIGHT_BUNDLEDISPENSE                =   20,
    GUIDLIGHT_CODESCANNER                   =   21,
    GUIDLIGHT_HIGHCAMERA                    =   22
#elif defined(SET_BANK_CSCB)
    GUIDLIGHT_CHECKSCANNER                  =   13, //二维码阅读器指示灯
    GUIDLIGHT_FINGERPRINT                   =   14, //指纹仪灯
    GUIDLIGHT_USBCONTROL                    =   15, //USB 通断电控制
    GUIDLIGHT_CAMERA                        =   16, //摄像头补光灯
    GUIDLIGHT_PASSBOOKREADER                =   17, //刷存折指示灯
    GUIDLIGHT_CARDBOX                       =   18, //发卡盒指示灯
    GUIDLIGHT_RFCARDUNIT                    =   19, //非接读卡器
    GUIDLIGHT_IDCARDREADER                  =   20, //身份证阅读模块灯
//    GUIDLIGHT_USBCONTROL                    =   23, //USB 通断电控制
    GUIDLIGHT_SCANNER                       =   24, //扫描仪指示灯
    GUIDLIGHT_CHEQUEUNIT                    =   25, //氛围灯
    GUIDLIGT_BILLPRINTER                    =   26, //发票打印机
    GUIDLIGHT_HIGHCAMERA                    =   27, //高拍仪指示灯
    GUIDlIGHT_FORCEEJECTIDX                 =   31  //身份证强制退证灯
#elif defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN)
    GUIDLIGHT_FINGERPRINT                   =   13, //指纹仪灯
    GUIDLIGHT_DEPOSITRECPREADER             =   14,	//存单读取
    GUIDLIGHT_DEPOSITRECPPRINTER            =   15,	//存单打印
    GUIDlIGHT_FORCEEJECTIDX                 =   31  //身份证强制退证灯
#else
    GUIDLIGHT_FINGER                        =   14, // 指纹灯
    GUIDLIGHT_NONCONTACT                    =   15, // 非接灯
#endif
};

enum GUIDLIGHT_STATUS
{
    GUIDLIGHT_OFF                           =   0x0001,
    GUIDLIGHT_SLOW_FLASH                    =   0x0004,
    GUIDLIGHT_MEDIUM_FLASH                  =   0x0008,
    GUIDLIGHT_QUICK_FLASH                   =   0x0010,
    GUIDLIGHT_CONTINUOUS                    =   0x0080,
};
//////////////////////////////////////////////////////////////////////////
enum DOOR_INDEX
{
    DOOR_CABINET                            =   0,
    DOOR_SAFE                               =   1,
    DOOR_VANDALSHIELD                       =   2,
    DOOR_SHUTTER                            =   10, // 闸门
    DOOR_CABINET_FRONT                      =   13, // 前柜门
    DOOR_CABINET_REAR                       =   15, // 后柜门
};

enum DOOR_STATUS
{
    DOOR_CLOSED                             =   0x0001,
    DOOR_OPEN                               =   0x0002,
    DOOR_LOCKED                             =   0x0004,
    DOOR_BOLTED                             =   0x0008,
    DOOR_SERVICE                            =   0x0010,
    DOOR_KEYBOARD                           =   0x0020,
    DOOR_AJAR                               =   0x0040,
    DOOR_JAMMED                             =   0x0080,
};
//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_siu_status
{
    WORD wDevice;
    WORD wSensors[DEFSIZE];
    WORD wDoors[DEFSIZE];
    WORD wIndicators[DEFSIZE];
    WORD wAuxiliaries[DEFSIZE];
    WORD wGuidLights[DEFSIZE];
    char szErrCode[8];              // 三位的错误码

    tag_dev_siu_status() {clear();}
    void clear() {memset(this, 0x00, sizeof(tag_dev_siu_status));} // 默认值为0，表示不支持此项
} DEVSIUSTATUS, *LPDEVSIUSTATUS;

//////////////////////////////////////////////////////////////////////////
struct IDevSIU
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
#if defined(SET_BANK_SXXH) || defined(SET_BANK_JIANGNAN)
	virtual long Open(LPCSTR lpMode, bool bSensorOnly = true) = 0;
#else
    virtual long Open(LPCSTR lpMode) = 0;
#endif
    // 关闭连接
    virtual long Close() = 0;
    // 复位
    virtual long Reset() = 0;
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo) = 0;
    // 取状态
    virtual long GetStatus(DEVSIUSTATUS &stStatus) = 0;
    // 控制门
    virtual long SetDoors(WORD wDoors[DEFSIZE]) = 0;
    // 控制指示符
    virtual long SetIndicators(WORD wIndicators[DEFSIZE]) = 0;
    // 控制辅助器
    virtual long SetAuxiliaries(WORD wAuxiliaries[DEFSIZE]) = 0;
    // 控制灯
    virtual long SetGuidLights(WORD wGuidLights[DEFSIZE]) = 0;
    // 获取设备固件版本
    virtual long GetFirmWareVer(char *pFwVer) = 0;
    // 执行固件升级
    virtual long UpdateDevPDL() = 0;

    //测试Flicker,Skim灯下标接口
    virtual int SetFlickerLed(int iFlickerLedIdx, int iAction){
        return ERR_IOMC_INVALID_COMMAN;
    }
    virtual int SetSkimLed(int iFlickerLedIdx, int iAction){
        return ERR_IOMC_INVALID_COMMAN;
    }
};

extern "C" DEVSIU_EXPORT long CreateIDevSIU(LPCSTR lpDevType, IDevSIU *&p);

#else

#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "IAllDevPort.h"
//////////////////////////////////////////////////////////////////////////
#if defined(IDEVSIU_LIBRARY)
#define DEVSIU_EXPORT     Q_DECL_EXPORT
#else
#define DEVSIU_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
//错误码说明：
//------------------------ 错误码定义 ----------------------------------
#define ERR_IOMC_SUCCESS                (0)     // 成功，通讯成功，设备操作也成功
#define ERR_IOMC_WARN                   (1)     // 命令执行成功，可是有警告产生了
#define ERR_IOMC_HARDWARE_ERROR         (-1)    // 执行指令异常终止
#define ERR_IOMC_END_STATE              (-2)    // 返回的结束状态有误
#define ERR_IOMC_PARAM                  (-3)    // 参数错误
#define ERR_IOMC_COMM                   (-4)    // 通讯失败
#define ERR_IOMC_DEVBUSY                (-5)    // 设备忙
#define ERR_IOMC_CREATETHREAD           (-6)    // 创建线程失败
#define ERR_IOMC_NO_DEVICE              (-7)    // 如果指定名的设备不存在，创建对象时返回
#define ERR_IOMC_RETURN_DATA_LEN        (-8)    // 数据长度不合法
#define ERR_IOMC_NOLOADDLL              (-9)    // 没有加载通讯库
#define ERR_IOMC_RESPDATA_ERR           (-10)   // 数据解析错误
#define ERR_IOMC_INVALID_COMMAN         (-11)   // 无效命令
#define ERR_IOMC_NOT_OPEN               (-12)   // 没有打开连接
#define ERR_IOMC_NOT_SUPPORT            (-13)   // 不支持功能
#define ERR_IOMC_WRITE_ERROR            (-14)   // 发送数据错误
#define ERR_IOMC_READ_ERROR             (-15)   // 接收数据错误
#define ERR_IOMC_READ_TIMEOUT           (-16)   // 读数据超时

//以下错误是USB相关错误
#define ERR_IOMC_USB_PARAM_ERR          (-101)  // 参数错误
#define ERR_IOMC_USB_CMD_ERR            (-102)  // 指令错误
#define ERR_IOMC_USB_CONN_ERR           (-103)  // 连接错误
#define ERR_IOMC_USB_CANCELLED          (-104)  // 指令取消
#define ERR_IOMC_USB_BUSY               (-105)  // 操作忙
#define ERR_IOMC_USB_NODLL              (-106)  // 驱动库不存在
#define ERR_IOMC_USB_NOFUNC             (-107)  // 驱动库入口函数不存在
#define ERR_IOMC_USB_OTHER              (-108)  // 其他USB错误

//以下错误是固件升级相关错误
#define PDL_NO_UPDATE                   (-201)  // 不用升级固件
#define PDL_DEV_ERR                     (-202)  // 设备异常失败
#define PDL_CONFIG_ERR                  (-203)  // PDL配置错误，或没有找到配置文件
#define PDL_FILE_ERR                    (-204)  // PDL文件错误，或没有找到PDL文件
#define PDL_MD5_ERR                     (-205)  // MD5值校验失败
#define PDL_SWITCH_ERR                  (-206)  // 切换PDL模式失败
#define PDL_SENDHEX_ERR                 (-207)  // 发送固件数据失败
#define PDL_UPDATE_ERR                  (-208)  // 升级固件失败


//-----------------------------------------------------------------------

// SetData/GetData 数据类型
#define SET_DEV_INIT            0       // 设置初始化数据
#define SET_LIB_PATH            1       // 设置动态库路径
#define SET_DEV_OPENMODE        2       // 设置设备打开模式
#define SET_GLIGHT_CONTROL      3       // 指示灯控制
#define SET_BEEP_CONTROL        4       // 设备鸣响控制
#define SET_DEV_RECON           5       // 设置断线重连标记
#define GET_SKIMMING_CREADER    6       // 获取异物检知(读卡器)

//////////////////////////////////////////////////////////////////////////
#define DEFSIZE                         (32)
#define IOMC_STATUS_NOCHANGE            (0)
//////////////////////////////////////////////////////////////////////////
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB)
enum DEVICE_STATUS
{
    DEVICE_OFFLINE                          =   0,
    DEVICE_ONLINE                           =   1,
    DEVICE_HWERROR                          =   2,
};
#else
enum DEVICE_STATUS_IOMC                     //30-00-00-00(FS#0004)
{
    DEVICE_OFFLINE                          =   0,
    DEVICE_ONLINE                           =   1,
    DEVICE_HWERROR                          =   2,
};
#endif

//////////////////////////////////////////////////////////////////////////
enum SENSORS_INDEX
{
    SENSORS_OPERATORSWITCH                  =   0,
    SENSORS_TAMPER                          =   1,
    SENSORS_INTTAMPER                       =   2,
    SENSORS_SEISMIC                         =   3,
    SENSORS_HEAT                            =   4,
    SENSORS_PROXIMITY                       =   5,
    SENSORS_AMBLIGHT                        =   6,
    SENSORS_ENHANCEDAUDIO                   =   7,
    SENSORS_CABINET_FAN                     =   15,// 柜体散热风扇
    SENSORS_PANELBOARD                      =   17,// 面板到位检测
    SENSORE_SAFEDOOR_FAN                    =   19,// 保险柜散热风扇
    SENSORS_MODULEINPLACE                   =   21,// 机芯到位检测
    SENSORE_SAFEDOORLOCK                    =   25,// 保险柜门锁报警状态
};

enum SENSORS_STATUS
{
    SENSORS_RUN                             =   0x0001,
    SENSORS_MAINTENANCE                     =   0x0002,
    SENSORS_SUPERVISOR                      =   0x0004,
    SENSORS_OFF                             =   0x0001,
    SENSORS_ON                              =   0x0002,
    SENSORS_NOT_PRESENT                     =   0x0003,
    SENSORS_PRESENT                         =   0x0004
};
//////////////////////////////////////////////////////////////////////////
enum INDICATOR_INDEX
{
    INDICATOR_OPENCLOSE                     = 0,
    INDICATOR_FASCIALIGHT                   = 1, // 框照明灯
    INDICATOR_AUDIO                         = 2,
    INDICATOR_HEATING                       = 3,
};
enum INDICATOR_STATUS
{
    INDICATOR_OFF                           = 0x0001,
    INDICATOR_ON                            = 0x0002,
};
//////////////////////////////////////////////////////////////////////////
enum AUXILIARIE_INDEX
{
    AUXILIARIE_VOLUME                       = 0,
    AUXILIARIE_UPS                          = 1,
    AUXILIARIE_ALARM                        = 2,
    AUXILIARIE_AUDIO                        = 3,
};
enum AUXILIARIE_STATUS
{
    AUXILIARIE_OFF                           = 0x0001,
    AUXILIARIE_ON                            = 0x0002,
};
//////////////////////////////////////////////////////////////////////////
enum GUIDLIGHT_INDEX
{
    GUIDLIGHT_CARDUNIT                      =   0,  //读卡器灯（或发卡机）
    GUIDLIGHT_PINPAD                        =   1,  //密码键盘灯
    GUIDLIGHT_NOTESDISPENSER                =   2,  //出钞指示灯
    GUIDLIGHT_COINDISPENSER                 =   3,  //硬币取款灯/射频卡指示灯
    GUIDLIGHT_RECEIPTPRINTER                =   4,  //凭条打印机灯
    GUIDLIGHT_PASSBOOKPRINTER               =   5,  //存折指示灯
    GUIDLIGHT_ENVDEPOSITORY                 =   6,  //强存/刷磁设备
#if defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN)
    GUIDLIGHT_ENVSCANNER                    =   7,  //票据扫描/支票处理单元
#else
    GUIDLIGHT_CHEQUEUNIT                    =   7,
#endif
    GUIDLIGHT_BILLACCEPTOR                  =   8,  //存款模块灯
    GUIDLIGHT_ENVDISPENSER                  =   9,  //票据发售/UKey
    GUIDLIGHT_DOCUMENTPRINTER               =   10, //A4 激光打印机灯
    GUIDLIGHT_COINACCEPTOR                  =   11, //硬币存款/身份证指示灯
#if defined(SET_BANK_CSCB)
    GUIDLIGHT_UKEY                          =   12, //UKey
#else
    GUIDLIGHT_SCANNER                       =   12, //扫描仪器
#endif
#if defined(SET_BANK_CMBC)
    GUIDLIGHT_ICCARDUNIT                    =   13,
    GUIDLIGHT_RFCARDUNIT                    =   14,
    GUIDLIGHT_TRACKCARDUNIT                 =   15,
    GUIDLIGHT_IDCARDREADER                  =   16,
    GUIDLIGHT_FINGER                        =   17,
    GUIDLIGHT_KEYDISPENSER                  =   18,
    GUIDLIGHT_FORCEACCEPTOR                 =   19,
    GUIDLIGHT_BUNDLEDISPENSE                =   20,
    GUIDLIGHT_CODESCANNER                   =   21,
    GUIDLIGHT_HIGHCAMERA                    =   22
#elif defined(SET_BANK_CSCB)
    GUIDLIGHT_CHECKSCANNER                  =   13, //二维码阅读器指示灯
    GUIDLIGHT_FINGERPRINT                   =   14, //指纹仪灯
    GUIDLIGHT_USBCONTROL                    =   15, //USB 通断电控制
    GUIDLIGHT_CAMERA                        =   16, //摄像头补光灯
    GUIDLIGHT_PASSBOOKREADER                =   17, //刷存折指示灯
    GUIDLIGHT_CARDBOX                       =   18, //发卡盒指示灯
    GUIDLIGHT_RFCARDUNIT                    =   19, //非接读卡器
    GUIDLIGHT_IDCARDREADER                  =   20, //身份证阅读模块灯
//    GUIDLIGHT_USBCONTROL                    =   23, //USB 通断电控制
    GUIDLIGHT_SCANNER                       =   24, //扫描仪指示灯
    GUIDLIGHT_CHEQUEUNIT                    =   25, //氛围灯
    GUIDLIGT_BILLPRINTER                    =   26, //发票打印机
    GUIDLIGHT_HIGHCAMERA                    =   27, //高拍仪指示灯
    GUIDlIGHT_FORCEEJECTIDX                 =   31  //身份证强制退证灯
#elif defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN)
    GUIDLIGHT_FINGERPRINT                   =   13, //指纹仪灯
    GUIDLIGHT_DEPOSITRECPREADER             =   14,	//存单读取
    GUIDLIGHT_DEPOSITRECPPRINTER            =   15,	//存单打印
    GUIDlIGHT_FORCEEJECTIDX                 =   31  //身份证强制退证灯
#else
    GUIDLIGHT_FINGER                        =   14, // 指纹灯
    GUIDLIGHT_NONCONTACT                    =   15, // 非接灯
#endif
};

enum GUIDLIGHT_STATUS
{
    GUIDLIGHT_OFF                           =   0x0001,
    GUIDLIGHT_SLOW_FLASH                    =   0x0004,
    GUIDLIGHT_MEDIUM_FLASH                  =   0x0008,
    GUIDLIGHT_QUICK_FLASH                   =   0x0010,
    GUIDLIGHT_CONTINUOUS                    =   0x0080,
};
//////////////////////////////////////////////////////////////////////////
enum DOOR_INDEX
{
    DOOR_CABINET                            =   0,
    DOOR_SAFE                               =   1,
    DOOR_VANDALSHIELD                       =   2,
    DOOR_SHUTTER                            =   10, // 闸门
    DOOR_CABINET_FRONT                      =   13, // 前柜门
    DOOR_CABINET_REAR                       =   15, // 后柜门
};

enum DOOR_STATUS
{
    DOOR_CLOSED                             =   0x0001,
    DOOR_OPEN                               =   0x0002,
    DOOR_LOCKED                             =   0x0004,
    DOOR_BOLTED                             =   0x0008,
    DOOR_SERVICE                            =   0x0010,
    DOOR_KEYBOARD                           =   0x0020,
    DOOR_AJAR                               =   0x0040,
    DOOR_JAMMED                             =   0x0080,
};
//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_siu_status
{
    WORD wDevice;
    WORD wSensors[DEFSIZE];
    WORD wDoors[DEFSIZE];
    WORD wIndicators[DEFSIZE];
    WORD wAuxiliaries[DEFSIZE];
    WORD wGuidLights[DEFSIZE];
    char szErrCode[8];              // 三位的错误码

    tag_dev_siu_status() {clear();}
    void clear() {memset(this, 0x00, sizeof(tag_dev_siu_status));} // 默认值为0，表示不支持此项
} DEVSIUSTATUS, *LPDEVSIUSTATUS;

//////////////////////////////////////////////////////////////////////////
struct IDevSIU
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
#if defined(SET_BANK_SXXH) || defined(SET_BANK_JIANGNAN)
        virtual long Open(LPCSTR lpMode, bool bSensorOnly = true) = 0;
#else
    virtual long Open(LPCSTR lpMode) = 0;
#endif
    // 关闭连接
    virtual long Close() = 0;
    // 复位
    virtual long Reset() = 0;
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo) = 0;
    // 取状态
    virtual long GetStatus(DEVSIUSTATUS &stStatus) = 0;
    // 控制门
    virtual long SetDoors(WORD wDoors[DEFSIZE]) = 0;
    // 控制指示符
    virtual long SetIndicators(WORD wIndicators[DEFSIZE]) = 0;
    // 控制辅助器
    virtual long SetAuxiliaries(WORD wAuxiliaries[DEFSIZE]) = 0;
    // 控制灯
    virtual long SetGuidLights(WORD wGuidLights[DEFSIZE]) = 0;
    // 获取设备固件版本
    virtual long GetFirmWareVer(char *pFwVer) = 0;
    // 执行固件升级
    virtual long UpdateDevPDL() = 0;


    /************************************************************
    ** 功能：设置数据
    ** 输入：vData 入参
    **      wDataType 入参 设置类型(参考宏定义)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int SetData(unsigned short usType, void *vData = nullptr)
    {
        return ERR_IOMC_INVALID_COMMAN;
    }

    /************************************************************
    ** 功能：获取数据
    ** 输入：wDataType 入参 获取类型(参考宏定义)
    ** 输出：vData 回参 数据
    **
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetData(unsigned short usType, void *vData)
    {
        return ERR_IOMC_INVALID_COMMAN;
    }

    /************************************************************
    ** 功能：获取版本号
    ** 输入：wType 入参 获取类型
    ** 输出：szVer 回参 数据
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize)
    {
        return ERR_IOMC_INVALID_COMMAN;
    }

    //测试Flicker,Skim灯下标接口
    virtual int SetFlickerLed(int iFlickerLedIdx, int iAction){
        return ERR_IOMC_INVALID_COMMAN;
    }
    virtual int SetSkimLed(int iFlickerLedIdx, int iAction){
        return ERR_IOMC_INVALID_COMMAN;
    }
};

extern "C" DEVSIU_EXPORT long CreateIDevSIU(LPCSTR lpDevType, IDevSIU *&p);

#endif // SPBuild_OPTIM
//////////////////////////////////////////////////////////////////////////
