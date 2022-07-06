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
enum DEVICE_STATUS_IOMC                     //30-00-00-00(FS#0004)
{
    DEVICE_OFFLINE                          =   0,
    DEVICE_ONLINE                           =   1,
    DEVICE_HWERROR                          =   2,
};
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
    GUIDLIGHT_CARDUNIT                      =   0,
    GUIDLIGHT_PINPAD                        =   1,
    GUIDLIGHT_NOTESDISPENSER                =   2,
    GUIDLIGHT_COINDISPENSER                 =   3,
    GUIDLIGHT_RECEIPTPRINTER                =   4,
    GUIDLIGHT_PASSBOOKPRINTER               =   5,
    GUIDLIGHT_ENVDEPOSITORY                 =   6,
    GUIDLIGHT_CHEQUEUNIT                    =   7,
    GUIDLIGHT_BILLACCEPTOR                  =   8,
    GUIDLIGHT_ENVDISPENSER                  =   9,
    GUIDLIGHT_DOCUMENTPRINTER               =   10,
    GUIDLIGHT_COINACCEPTOR                  =   11,
    GUIDLIGHT_SCANNER                       =   12,
    GUIDLIGHT_FINGER                        =   14, // 指纹灯
    GUIDLIGHT_NONCONTACT                    =   15, // 非接灯
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
    virtual long Open(LPCSTR lpMode) = 0;
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
};

extern "C" DEVSIU_EXPORT long CreateIDevSIU(LPCSTR lpDevType, IDevSIU *&p);
//////////////////////////////////////////////////////////////////////////
