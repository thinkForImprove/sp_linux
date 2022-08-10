/***************************************************************
* 文件名称：DevImpl_TMZ.h
* 文件描述：天梦者非接模块底层指令，提供控制接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_TMZ_H
#define DEVIMPL_TMZ_H

#pragma once

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include <QLibrary>

#define FUNC_POINTER_ERROR_RETURN(LPFUNC) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE;    \
        return FALSE;   \
    }

/*************************************************************
// 返回值/错误码　宏定义
// <0 : Impl处理返回
// 0~100: 硬件设备返回
*************************************************************/
// >0: Impl处理返回
#define IMP_SUCCESS                     0           // 成功
#define IMP_ERR_LOAD_LIB                1           // 动态库加载失败
#define IMP_ERR_PARAM_INVALID           2           // 参数无效
#define IMP_ERR_UNKNOWN                 3           // 未知错误
#define IMP_ERR_NOTOPEN                 4           // 设备未Open
// <0: Device返回
#define IMP_DEV_ERR_SUCCESS             0           // 0
#define IMP_DEV_ERR_F31                 -31         // 取消PIN输入
#define IMP_DEV_ERR_F32                 -32         // 键盘超时
#define IMP_DEV_ERR_F33                 -33         // 输入密码长度错误
#define IMP_DEV_ERR_F85                 -85         // 键盘不支持
#define IMP_DEV_ERR_F86                 -86         // 键盘返回数据格式错误
#define IMP_DEV_ERR_F97                 -97         // 无效句柄
#define IMP_DEV_ERR_F98                 -98         // 设备异常断开
#define IMP_DEV_ERR_F99                 -99         // 获取设备状态参数错误
#define IMP_DEV_ERR_F100                -100        // 设置设备状态参数错误
#define IMP_DEV_ERR_F101                -101        // 设置通讯事件错误
#define IMP_DEV_ERR_F113                -113        // 读数据错误
#define IMP_DEV_ERR_F114                -114        // 写数据错误
#define IMP_DEV_ERR_F115                -115        // 命令头错误
#define IMP_DEV_ERR_F116                -116        // 命令尾错误
#define IMP_DEV_ERR_F117                -117        // 数据错位
#define IMP_DEV_ERR_F118                -118        // 校验位错误
#define IMP_DEV_ERR_F119                -119        // 超时错误(上层软件超时返回，没有等待到硬件返回数据)
#define IMP_DEV_ERR_F129                -129        // 数据分配空间错误(内存错误)
#define IMP_DEV_ERR_F130                -130        // 长度错误
#define IMP_DEV_ERR_F131                -131        // 传入数据格式错误
#define IMP_DEV_ERR_F144                -144        // 设备不支持该操作(动态库未加载)
#define IMP_DEV_ERR_F145                -145        // 二代证错误
#define IMP_DEV_ERR_F146                -146        // 无权限(一般为文件操作，可能是设置的路径问题)
#define IMP_DEV_ERR_F147                -147        // 解码库加载失败
#define IMP_DEV_ERR_F148                -148        // 身份证解码错误
#define IMP_DEV_ERR_F149                -149        // 其他错误
#define IMP_DEV_ERR_F161                -161        // T57卡交互数据异常
#define IMP_DEV_ERR_F162                -162        // 无T57卡
#define IMP_DEV_ERR_F163                -163        // T57卡操作作卡片数据出现错误无回应
#define IMP_DEV_ERR_F164                -164        // T57卡参数设置失败
#define IMP_DEV_ERR_F165                -165        // T57卡密码认证没通过
#define IMP_DEV_ERR_F4097               -4097       // 设备功能不支持或参数不支持
#define IMP_DEV_ERR_F4098               -4098       // 命令执行错误
#define IMP_DEV_ERR_F8193               -8193       // 写EEPROM失败
#define IMP_DEV_ERR_F8194               -8194       // 读EEPROM失败
#define IMP_DEV_ERR_F12289              -12289      // 不支持卡类型
#define IMP_DEV_ERR_F12290              -12290      // 无卡
#define IMP_DEV_ERR_F12291              -12291      // 有卡已上电
#define IMP_DEV_ERR_F12292              -12292      // 有卡未上电(或非接有卡状态)
#define IMP_DEV_ERR_F12293              -12293      // 卡上电失败
#define IMP_DEV_ERR_F12294              -12294      // 操作卡片数据无回应,超时(接触式存储卡无响应)
#define IMP_DEV_ERR_F12295              -12295      // 操作卡片数据出现错误
#define IMP_DEV_ERR_F12296              -12296      // 非接卡Halt失败
#define IMP_DEV_ERR_F12297              -12297      // 多张非接卡
#define IMP_DEV_ERR_F16386              -16386      // 设备底层超时未响应返回
#define IMP_DEV_ERR_F20481              -20481      // 磁头未开启
#define IMP_DEV_ERR_F20482              -20482      // 刷卡失败
#define IMP_DEV_ERR_F20483              -20483      // 刷卡超时


/*************************************************************************
// 无分类　宏定义
*************************************************************************/
#define LOG_NAME            "DevImpl_TMZ.log"         // 缺省日志名
#define DLL_DEVLIB_NAME     "libtmz.so"               // 缺省动态库名

// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }


/*************************************************************************
// SDK使用　宏定义
*************************************************************************/
// 接触式用户卡 卡座范围
enum CONT_SLOT_RANGE
{
    CONT_SLOT_01 = 0x01,
    CONT_SLOT_02 = 0x02,
    CONT_SLOT_03 = 0x03,
    CONT_SLOT_04 = 0x04,
    CONT_SLOT_05 = 0x05,
    CONT_SLOT_06 = 0x06,
    CONT_SLOT_07 = 0x07,
    CONT_SLOT_08 = 0x08,
    CONT_SLOT_09 = 0x09,
    CONT_SLOT_10 = 0x0A,
    CONT_SLOT_11 = 0x0B,
    CONT_SLOT_12 = 0x0C,
    CONT_SLOT_13 = 0x0D,
    CONT_SLOT_14 = 0x0E,
    CONT_SLOT_15 = 0x0F,
};

// PSAM卡 卡座范围
enum PSAM_SLOT_RANGE
{
    PSAM_SLOT_01 = 0x11,
    PSAM_SLOT_02 = 0x12,
    PSAM_SLOT_03 = 0x13,
    PSAM_SLOT_04 = 0x14,
    PSAM_SLOT_05 = 0x15,
    PSAM_SLOT_06 = 0x16,
    PSAM_SLOT_07 = 0x17,
    PSAM_SLOT_08 = 0x18,
    PSAM_SLOT_09 = 0x19,
    PSAM_SLOT_10 = 0x1A,
    PSAM_SLOT_11 = 0x1B,
    PSAM_SLOT_12 = 0x1C,
    PSAM_SLOT_13 = 0x1D,
    PSAM_SLOT_14 = 0x1E,
    PSAM_SLOT_15 = 0x1F,
};

// 非接CPU卡 卡座范围
enum FCPU_SLOT_RANGE
{
    FCPU_SLOT_01 = 0x31,
    FCPU_SLOT_02 = 0x32,
    FCPU_SLOT_03 = 0x33,
    FCPU_SLOT_04 = 0x34,
    FCPU_SLOT_05 = 0x35,
    FCPU_SLOT_06 = 0x36,
    FCPU_SLOT_07 = 0x37,
    FCPU_SLOT_08 = 0x38,
    FCPU_SLOT_09 = 0x39,
    FCPU_SLOT_10 = 0x3A,
    FCPU_SLOT_11 = 0x3B,
    FCPU_SLOT_12 = 0x3C,
    FCPU_SLOT_13 = 0x3D,
    FCPU_SLOT_14 = 0x3E,
    FCPU_SLOT_15 = 0x3F,
};

// 只有一个接触大卡座时,0x01，一个非接卡座则为0x31
#define CONT_SLOT_ONLY      0x01
#define FCPU_SLOT_ONLY      0x31

// 返回值: 卡片状态
#define CARD_STAT_TYPE_NOTSUPP          0x3001  // 不支持卡类型
#define CARD_STAT_NOHAVE                0x3002  // 无卡
#define CARD_STAT_POWER_ON              0x3003  // 有卡已上电(非接卡暂无该状态)
#define CARD_STAT_POWER_OFF             0x3004  // 有卡未上电(未激活,或者非接有卡状态)
#define CARD_STAT_POWER_FAIL            0x3005  // 卡上电失败(激活失败)
#define CARD_STAT_DATA_NORESP           0x3006  // 操作卡片数据无回应（超时）
#define CARD_STAT_DATA_ERR              0x3007  // 操作卡片数据出现错误
#define CARD_STAT_MULTIPLE              0x3009  // 有多张卡在感应区

// 返回值: 设备状态
#define DEV_STAT_OK                     0       // 正常
#define DEV_STAT_CONT_ABNOR             1       // 接触卡通道异常
#define DEV_STAT_NOCONT_ABNOR           2       // 非接卡通道异常
#define DEV_STAT_NO_CONT_ABNOR          3       // 接触卡和非接卡通道异常
#define DEV_STAT_SECUR_ABNOR            4       // 安全模块通道异常
#define DEV_STAT_CONT_SECUR_ABNOR       5       // 接触卡和安全模块通道异常
#define DEV_STAT_NOCONT_SECUR_ABNOR     6       // 非接卡和安全模块通道异常
#define DEV_STAT_NO_CONT_SECUR_ABNOR    7       // 接触卡、非接卡和安全模块通道异常


/*************************************************************************
// 动态库输出函数接口　定义
*************************************************************************/
// 1. 打开USB设备
//  返回: 设备句柄(>0)
typedef long (* FNICReaderOpenUsbByFD)(unsigned int uiFD);
// 2. 关闭设备
//  入参: 设备句柄
typedef int  (* FNICReaderClose)(long icdev);
// 3. 获取固件版本(64Byte)
typedef int  (* FNICReaderGetVer)(long icdev, char *pVer);
// 4. 设置鸣响
//  入参: uMsec 鸣响一次时间; uDelay: 鸣响间隔; uNum鸣响次数
typedef int  (* FNICReaderBeep)(long icdev, unsigned char uMsec, unsigned char uDelay, unsigned char uNum);
// 5. 获取设备状态
typedef int  (* FNICReaderDevStatus)(long icdev, unsigned char *uStatus);
// 6. 取卡片状态
typedef int  (* FNGetCardState)(long icdev, unsigned char uSlot, int* uState);
// 7. 卡上电激活
typedef int  (* FNCPUPowerOn)(long icdev, unsigned char uSlot, int iTime,
                              unsigned char* uType, unsigned char* uSnrLen, unsigned char* pSnr,
                              int* rLen, unsigned char* pData);
// 8. 卡断电
typedef int  (* FNCPUPowerOff)(long icdev, unsigned char uSlot);
// 9. 发送APDU
typedef int  (* FNCPUCardAPDU)(long icdev, unsigned char uSlot, int sLen,
                               unsigned char *pSendData, int* rLen, unsigned char* pData);
// 10. LED灯控制
typedef int  (* FNICReaderLEDCtrl)(long icdev, unsigned char uLEDCtrl, unsigned char uDelay);


/**************************************************************************
* 命令编辑、发送接收等处理
***************************************************************************/
class CDevImpl_TMZ : public CLogManage
{
public:
    CDevImpl_TMZ();
    CDevImpl_TMZ(LPSTR lpLog);
    ~CDevImpl_TMZ();

public:
    INT     OpenDevice();                                               // 打开指定设备
    INT     OpenDevice(LPSTR lpcMode);                                  // 打开指定设备(指定参数)
    INT     CloseDevice();
    INT     IsDeviceOpen();
    INT     SetReConFlag(BOOL bFlag);                                   // 设置断线重连标记
    INT     SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)
    LPSTR   ConvertCode_Impl2Str(INT nErrCode);

public: // 接口函数封装
    INT     GetFWVersion(LPSTR lpFwVer);                                // 3. 获取固件版本
    INT     SetDeviceBeep(UCHAR ucMsec, UCHAR ucDelay, UCHAR ucNum);    // 4. 设置鸣响
    INT     GetDeviceStatus(UCHAR &ucStat);                             // 5. 获取设备状态
    INT     GetCardStatus(INT &nStat, UCHAR ucSlot);                    // 6. 取卡片状态
    INT     SetCPUPowerOn(UCHAR ucSlot, INT nTimeOut, UCHAR &ucType,
                          LPUCHAR lpSnrData, UCHAR &ucSnrLen,
                          LPUCHAR lpData, INT &nDataLen);               // 7. 卡上电激活
    INT     SetCPUPowerOff(UCHAR ucSlot);                               // 8. 卡断电
    INT     SendCPUCardAPDU(UCHAR ucSlot, LPUCHAR lpSndData, INT nSndLen,
                            LPUCHAR lpRcvData, INT &nRcvLen);           // 9. 发送APDU
    INT     SetLEDCtrl(UCHAR ucCtrl, UCHAR ucDelay);                    // 10. LED灯控制

private:
    LONG            m_lDevHandle;                                       // 设备句柄
    BOOL            m_bDevOpenOk;                                       // 设备Open标记
    CHAR            m_szErrStr[1024];
    CHAR            m_szFWVer[256];
    BOOL            m_bReCon;                                           // 是否断线重连状态
    INT             m_nRetErrOLD[8];                                    // 处理错误值保存(0:库加载/1:设备连接/
                                                                        //  2:设备状态/3介质状态/4检查设备状态)
private:
    void    Init();

private: // 接口加载
    BOOL    bLoadLibrary();
    void    vUnLoadLibrary();
    BOOL    bLoadLibIntf();
    void    vInitLibFunc();

private: // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;

public: // 动态库接口定义
    FNICReaderOpenUsbByFD           ICReaderOpenUsbByFD;                // 1. 打开USB设备
    FNICReaderClose                 ICReaderClose;                      // 2. 关闭设备
    FNICReaderGetVer                ICReaderGetVer;                     // 3. 获取固件版本
    FNICReaderBeep                  ICReaderBeep;                       // 4. 设置鸣响
    FNICReaderDevStatus             ICReaderDevStatus;                  // 5. 获取设备状态
    FNGetCardState                  GetCardState;                       // 6. 取卡片状态
    FNCPUPowerOn                    CPUPowerOn;                         // 7. 卡上电激活
    FNCPUPowerOff                   CPUPowerOff;                        // 8. 卡断电
    FNCPUCardAPDU                   CPUCardAPDU;                        // 9. 发送APDU
    FNICReaderLEDCtrl               ICReaderLEDCtrl;                    // 10. LED灯控制

};

#endif // DEVIMPL_TMZ_H
