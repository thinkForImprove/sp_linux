/***************************************************************
* 文件名称：DevImpl_MT50.h
* 文件描述：明泰非接模块底层指令，提供控制接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_MT50_H
#define DEVIMPL_MT50_H

#pragma once

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include <QLibrary>
#include <dlfcn.h>

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
#define IMP_ERR_NODEVICE                5           // 未找到有效设备
// <0: Device返回
// IMP_ERR_DEV_M1卡操作返回码
#define IMP_ERR_DEV_0001H               -1          // 在操作区域内无卡
#define IMP_ERR_DEV_0002H               -2          // 卡片CRC错误
#define IMP_ERR_DEV_0003H               -3          // 数值溢出
#define IMP_ERR_DEV_0004H               -4          // 验证不成功
#define IMP_ERR_DEV_0005H               -5          // 卡片奇偶校验错误
#define IMP_ERR_DEV_0006H               -6          // 与M1卡卡片通讯错误
#define IMP_ERR_DEV_0008H               -8          // 防冲突过程中读系列号错误
#define IMP_ERR_DEV_0009H               -9          // 波特率修改失败
#define IMP_ERR_DEV_000AH               -10         // 卡片没有通过验证
#define IMP_ERR_DEV_000BH               -12         // 从卡片接收到的位数错误
#define IMP_ERR_DEV_000CH               -13         // 从卡片接收到的字节数错误（仅仅读函数有效）
#define IMP_ERR_DEV_0016H               -22         // 调用request函数出错
#define IMP_ERR_DEV_0017H               -23         // 调用select函数出错
#define IMP_ERR_DEV_0018H               -24         // 调用anticoll函数出错
#define IMP_ERR_DEV_0019H               -25         // 调用read函数出错
#define IMP_ERR_DEV_001AH               -26         // 调用write函数出错
#define IMP_ERR_DEV_001BH               -27         // 调用增值函数出错
#define IMP_ERR_DEV_001CH               -28         // 调用减值函数出错
#define IMP_ERR_DEV_001DH               -29         // 调用重载函数出错
#define IMP_ERR_DEV_001EH               -30         // 调用loadkey函数出错
// 其他硬件返回码
#define IMP_ERR_DEV_002AH               -42         // 命令错误
#define IMP_ERR_DEV_002BH               -43         // PC同reader之间通讯错误，比如BCC错误
#define IMP_ERR_DEV_002CH               -44         // PC同reader之间通讯命令码错误(设备不支持这条指令)
#define IMP_ERR_DEV_0041H               -65         // 4442卡错误计数等于0（锁卡）
#define IMP_ERR_DEV_0042H               -66         // 4442卡密码校验失败
#define IMP_ERR_DEV_0043H               -67         // 4442卡读失败
#define IMP_ERR_DEV_0044H               -68         // 4442卡写失败
#define IMP_ERR_DEV_0045H               -69         // 4442卡读写地址越界
#define IMP_ERR_DEV_0060H               -96         // 接收出错
#define IMP_ERR_DEV_0061H               -97         // 输入偏移地址与长度超出读写范围
#define IMP_ERR_DEV_0062H               -98         // 不是102卡
#define IMP_ERR_DEV_0063H               -99         // 密码校验错误
#define IMP_ERR_DEV_0070H               -112        // 接收参数出错
#define IMP_ERR_DEV_0071H               -113        // 校验码出错
#define IMP_ERR_DEV_0072H               -114        // 命令执行失败
#define IMP_ERR_DEV_0073H               -115        // 命令执行超时
#define IMP_ERR_DEV_0074H               -116        // 错误通信模式
#define IMP_ERR_DEV_0075H               -117        // 无磁条卡
#define IMP_ERR_DEV_008AH               -138        // 校验和错误
#define IMP_ERR_DEV_008BH               -139        // 卡型错误
#define IMP_ERR_DEV_008CH               -140        // 拔卡错误
#define IMP_ERR_DEV_008DH               -141        // 通用错误
#define IMP_ERR_DEV_008EH               -142        // 命令头错误
#define IMP_ERR_DEV_008FH               -143        // 数据长度错误
#define IMP_ERR_DEV_0099H               -153        // FLASH读写错误
// 接触式卡操作返回码
#define IMP_ERR_DEV_1001H               -4097       // 不支持接触用户卡
#define IMP_ERR_DEV_1002H               -4098       // 接触用户卡未插到位
#define IMP_ERR_DEV_1003H               -4099       // 接触用户卡已上电
#define IMP_ERR_DEV_1004H               -4100       // 接触用户卡未上电
#define IMP_ERR_DEV_1005H               -4101       // 接触用户卡上电失败
#define IMP_ERR_DEV_1006H               -4102       // 操作接触用户卡数据无回应
#define IMP_ERR_DEV_1007H               -4103       // 操作接触用户卡数据出现错误
// PSAM卡操作返回码
#define IMP_ERR_DEV_2001H               -8193       // 不支持PSAM卡
#define IMP_ERR_DEV_2002H               -8194       // PSAM卡未插到位
#define IMP_ERR_DEV_2003H               -8195       // PSAM卡已上电
#define IMP_ERR_DEV_2004H               -8196       // PSAM卡未上电
#define IMP_ERR_DEV_2005H               -8197       // PSAM卡上电失败
#define IMP_ERR_DEV_2006H               -8198       // 操作PSAM卡数据无回应
#define IMP_ERR_DEV_2007H               -8199       // 操作PSAM卡数据出现错误
// 非接触式卡操作返回码
#define IMP_ERR_DEV_3001H               -12289      // 不支持非接触用户卡
#define IMP_ERR_DEV_3004H               -12292      // 非接触用户卡未激活
#define IMP_ERR_DEV_3005H               -12293      // 非接触用户卡激活失败
#define IMP_ERR_DEV_3006H               -12294      // 操作非接触用户卡无回应（等待超时）
#define IMP_ERR_DEV_3007H               -12295      // 操作非接触用户卡数据出现错误
#define IMP_ERR_DEV_3008H               -12296      // 非接触用户卡halt失败
#define IMP_ERR_DEV_3009H               -12297      // 有多张卡在感应区
// 金融IC卡操作返回码
#define IMP_ERR_DEV_6001H               -24577      // 不支持逻辑操作
#define IMP_ERR_DEV_6020H               -24608      // 卡片类型不对（卡状态6A82）
#define IMP_ERR_DEV_6021H               -24609      // 余额不足（卡状态9401）
#define IMP_ERR_DEV_6022H               -24610      // 卡片功能不支持（卡状态6A81）
#define IMP_ERR_DEV_6023H               -24611      // 扣款失败（卡状态9302）
#define IMP_ERR_DEV_6030H               -24624      // 卡片未启用
#define IMP_ERR_DEV_6031H               -24625      // 卡片不在有效期
#define IMP_ERR_DEV_6032H               -24626      // 交易明细无此记录
#define IMP_ERR_DEV_6033H               -24627      // 交易明细记录未处理完成
#define IMP_ERR_DEV_6040H               -24640      // 需要做防拔处理
#define IMP_ERR_DEV_6041H               -24641      // 防拔处理中出错, 非原来卡
#define IMP_ERR_DEV_6042H               -24642      // 交易中断, 没有资金损失
// 软件返回错误码
#define IMP_ERR_DEV_00A1H               -161        // libusb 返回输入输出错误
#define IMP_ERR_DEV_00A2H               -162        // libusb 参数错误
#define IMP_ERR_DEV_00A3H               -163        // libusb 拒绝访问，可能权限不足，可用管理员权限再试
#define IMP_ERR_DEV_00A4H               -164        // libusb 找不到设备(设备可能已经掉线)
#define IMP_ERR_DEV_00A5H               -165        // libusb 未找到设备
#define IMP_ERR_DEV_00A6H               -166        // libusb 资源忙，可能设备已经被占用
#define IMP_ERR_DEV_00A7H               -167        // libusb 操作超时
#define IMP_ERR_DEV_00A8H               -168        // libusb 溢出错误
#define IMP_ERR_DEV_00A9H               -169        // libusb 管道错误
#define IMP_ERR_DEV_00AAH               -170        // libusb 系统调用中断(可能是由于信号)
#define IMP_ERR_DEV_00ABH               -171        // libusb 内存不足
#define IMP_ERR_DEV_00ACH               -172        // libusb 在这个平台上不支持的操作(linux)
#define IMP_ERR_DEV_00B1H               -177        // 通讯超时
#define IMP_ERR_DEV_00B2H               -178        // 无效的通讯句柄
#define IMP_ERR_DEV_00B3H               -179        // 打开串口错误
#define IMP_ERR_DEV_00B4H               -180        // 串口已经打开
#define IMP_ERR_DEV_00B5H               -181        // 获取通讯端口状态错误
#define IMP_ERR_DEV_00B6H               -182        // 设置通讯端口状态错误
#define IMP_ERR_DEV_00B7H               -183        // 从读写器读取数据出错
#define IMP_ERR_DEV_00B8H               -184        // 向读写器写入数据出错
#define IMP_ERR_DEV_00B9H               -185        // 设置串口通讯波特率错误
#define IMP_ERR_DEV_00C1H               -193        // STX错误
#define IMP_ERR_DEV_00C2H               -194        // ETX错误
#define IMP_ERR_DEV_00C3H               -195        // BCC错误
#define IMP_ERR_DEV_00C4H               -196        // 命令的数据长度大于最大长度
#define IMP_ERR_DEV_00C5H               -197        // 数据值错误
#define IMP_ERR_DEV_00C6H               -198        // 错误的协议类型
#define IMP_ERR_DEV_00C7H               -199        // 错误的设备类型
#define IMP_ERR_DEV_00C8H               -200        // 错误的USB通讯设备类
#define IMP_ERR_DEV_00C9H               -201        // 设备正在通讯中或者是设备已经关闭
#define IMP_ERR_DEV_00CAH               -202        // 设备通讯忙，函数正在操作可能还没返回
#define IMP_ERR_DEV_00CBH               -203        // 接收到的设备返回的数据长度不对
#define IMP_ERR_DEV_00D1H               -209        // 获取身份证
#define IMP_ERR_DEV_00D2H               -210        // 身份证读卡
#define IMP_ERR_DEV_00D3H               -211        // 身份证校验
#define IMP_ERR_DEV_00D4H               -212        // 内存分配失
#define IMP_ERR_DEV_00D5H               -213        // 调用ICONV
#define IMP_ERR_DEV_00D6H               -214        // 调用iconv
#define IMP_ERR_DEV_00D7H               -215        // 调用libwlt.so库出错
#define IMP_ERR_DEV_00D8H               -216        // 传入的WLT错误
#define IMP_ERR_DEV_00D9H               -217        // 打开文件失
#define IMP_ERR_DEV_00DAH               -218        // 文件不存在
#define IMP_ERR_DEV_00DBH               -219        // 磁条卡数据
#define IMP_ERR_DEV_00DCH               -220        // 未识别卡类
#define IMP_ERR_DEV_00DDH               -221        // 无卡
#define IMP_ERR_DEV_00DEH               -222        // 有卡未上电
#define IMP_ERR_DEV_00DFH               -223        // 卡无应答
#define IMP_ERR_DEV_00E0H               -224        // 刷卡命令超时
#define IMP_ERR_DEV_00E1H               -225        // 磁条卡刷卡失败
#define IMP_ERR_DEV_00E2H               -226        // 磁条卡刷卡模式未开启
#define IMP_ERR_DEV_00E3H               -227        // 发送APDU错误

/*************************************************************************
// 无分类　宏定义
*************************************************************************/
#define LOG_NAME            "DevImpl_MT50.log"          // 缺省日志名
#define DLL_DEVLIB_NAME     "libCnSysReader.so"         // 缺省动态库名

// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

// 加载动态库接口
#define LOAD_LIBINFO_FUNC_DL(HANDLE, LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)dlsym(HANDLE, FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        Log(ThisModule, __LINE__, "dlxxx加载动态库接口<%s> Fail. ", FUNC2); \
        return FALSE;   \
    }

/*************************************************************************
// 动态库输出函数接口　定义
*************************************************************************/

//---------------------------设备函数组----------------------------
// 1. 打开USB设备
// 入参: vid: usb设备vid号。
//      pid: usb设备pid号。
//      proid: 通讯协议(0:拆分协议, 1:合并协议)
//             当设备为MT3时只会用合并协议通讯, MT3Y设备按设置的协议通讯
// 返回值: 0:成功, 非0:错误码
typedef int (*pOpenUsbDevice)(int vid, int pid, int proid);


// 2. 打开串口设备
// 入参: serial: 串口设备路径，如：/dev/ttyS1
//      baudrate: 串口波特率
//      devid: 设备(0:MT3Y设备, 1:MT3设备)
//      proid: 通讯协议：0 拆分协议，1 合并协议。
//             当设备为MT3时只会用合并协议通讯, MT3Y设备按设置的协议通讯
// 返回值: 0:成功, 非0:错误码
typedef int (*pOpenSerialDevice)(char *serial_path, int baudrate, int devid, int proid);

// 3. 获取设备版本
// 回参: device_version: 设备版本
//      rlen: 版本长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pGetDeviceVersion)(char *device_version, int *verlen);

// 4. 获取动态库版本
// 回参: so_version: 动态库版本
//      rlen: 版本长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pGetSoVersion)(char *so_version, int *rlen);

// 5. 设备蜂鸣
// 入参: nMsec: 1字节, 一次鸣叫持续时间(单位:100ms)
//      nMsec_end: 1字节, 一次鸣叫停止时间(多次蜂鸣时的间隔时间, 单位:100ms)
//      nTime: 1字节, 蜂鸣器鸣叫次数
// 返回值: 0:成功, 非0:错误码
typedef int (*pDeviceBeep)(int nMsec, int nMsec_end, int nTime);

// 6. 读设备序列号
// 入参: nSnrLen: 序列号长度, 1字节(1~20, 单位:字节)
// 回参: snr_data: 序列号字符串, ASCII码字符串
// 返回值: 0:成功, 非0:错误码
typedef int (*pReadDeviceSNR)(int nSnrLen, char *snr_data);

// 7. 写设备序列号
// 入参: nSnrLen: 序列号长度, 1字节(1~20, 单位:字节)
//      snr_data: 序列号字符串, ASCII码字符串
// 返回值: 0:成功, 非0:错误码
typedef int (*pWriteDeviceSNR)(int nSnrLen, char *snr_data);

// 8. 关闭打开的设备
// 返回值: 0:成功, 非0:错误码
typedef int (*pCloseDevice)();

//---------------------------非接CPU卡----------------------------
// 9. 非接CPU卡状态
// 返回值: 0:成功, 非0:错误码
typedef int (*pPiccStatus)();

// 10. 非接CPU卡上电
// 入参: nMode: 寻卡模式(0:全部卡片都响应该命令, 除了在HALT状态的那些卡片
//                     1:在任何状态的全部卡片都将响应该命令)
// 回参: sSnr: 4字节(卡UID)
//      sAtr: ATR应答数据
//      nAtrLen: ATR应答数据数据长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pPiccPowerOn)(int nMode, unsigned char *sSnr, unsigned char *sAtr, int *nAtrLen);

// 11. 非接CPU卡下电
// 返回值: 0:成功, 非0:错误码
typedef int (*pPiccPowerOff)();

// 12. 非接CPU卡发送APDU
// 入参: sCmd: 发送的卡片APDU指令
//      nCmdLen: APDU指令长度
// 回参: sResp: 卡片返回的APDU应答数据
//      nRespLen: APDU应答数据长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pPiccApdu)(unsigned char *sCmd, int nCmdLen, unsigned char *sResp, int *nRespLen);

//---------------------------接触CPU卡----------------------------
// 13. 接触CPU卡状态
// 入参: nCardSet:
// 返回值: 0:成功, 非0:错误码
typedef int (*pIccStatus)(int nCardSet);

// 14. 指定卡座寻卡上电热复位
//    对指定卡座进行寻卡，寻到卡后便对卡片上电热复位
// 入参: nCardSet: 卡座选择(00H-04H, CPU卡:00H, PSAM1卡:01H, PSAM2卡:02H, PSAM3卡:03H, PSAM4:04H)
// 回参: sAtr: ATR应答数据
//      nAtrLen: ATR应答数据数据长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pIccReset)(int nCardSet, unsigned char *sAtr, int *nAtrLen);

// 15. 指定卡座寻卡上电热复位(指定波特率)
// 入参: nCardSet: 卡座选择(00H-04H, CPU卡:00H, PSAM1卡:01H, PSAM2卡:02H, PSAM3卡:03H, PSAM4:04H)
//      nBaud: 卡片波特率(9600, 19200, 38400, 76800, 115200)
// 回参: sAtr: ATR应答数据
//      nAtrLen: ATR应答数据数据长度
// 返回值: 0:成功, 非0:错误码
// 备注: MT3支持的波特率为: 9600, 19200, 38400, 76800, 115200
//      MT3y支持的波特率为: 9600, 38400, 115200
typedef int (*pIccResetBuad)(int nCardSet, int nBaud, unsigned char *sAtr, int *nAtrLen);

// 16. 指定卡座寻卡上电冷复位
//    对指定卡座进行寻卡，寻到卡后便对卡片上电热复位
// 入参: nCardSet: 卡座选择(00H-04H, CPU卡:00H, PSAM1卡:01H, PSAM2卡:02H, PSAM3卡:03H, PSAM4:04H)
// 回参: sAtr: ATR应答数据
//      nAtrLen: ATR应答数据数据长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pIccPowerOn)(int nCardSet, unsigned char *sAtr, int *nAtrLen);

// 17. 指定卡座寻卡上电冷复位(指定波特率)
// 入参: nCardSet: 卡座选择(00H-04H, CPU卡:00H, PSAM1卡:01H, PSAM2卡:02H, PSAM3卡:03H, PSAM4:04H)
//      nBaud: 卡片波特率(9600, 19200, 38400, 76800, 115200)
// 回参: sAtr: ATR应答数据
//      nAtrLen: ATR应答数据数据长度
// 返回值: 0:成功, 非0:错误码
// 备注: MT3支持的波特率为: 9600, 19200, 38400, 76800, 115200
//      MT3y支持的波特率为: 9600, 38400, 115200
typedef int (*pIccPowerOnBuad)(int nCardSet, int nBaud, unsigned char *sAtr, int *nAtrLen);

// 18. 接触CPU卡下电
// 入参: nCardSet: 卡座选择(00H-04H, CPU卡:00H, PSAM1卡:01H, PSAM2卡:02H, PSAM3卡:03H, PSAM4:04H)
// 返回值: 0:成功, 非0:错误码
typedef int (*pIccPowerOff)(int nCardSet);

// 19. 接触CPU卡发送APDU
// 入参: nCardSet: 卡座选择(00H-04H, CPU卡:00H, PSAM1卡:01H, PSAM2卡:02H, PSAM3卡:03H, PSAM4:04H)
//      sCmd: 发送的卡片APDU指令
//      nCmdLen: APDU指令长度
// 回参: sResp: 卡片返回的APDU应答数据
//      nRespLen: APDU应答数据长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pIccApdu)(int nCardSet,unsigned char *sCmd, int nCmdLen, unsigned char *sResp, int *nRespLen);

// 20. 接触式存储卡类型设置
// 功能: 设置接下来将要操作的存储卡的种类，读写器将初始化该存储卡操作的参数
// 入参: nCardType: 卡类型(1=CPU, 2=24Cxx, 3=4428, 4=4442, 5=AT88SC, 6=inphone, 7=45D041)(默认类型: 1=CPU卡)
// 返回值: 0:成功, 非0:错误码
typedef int (*pContactSelect)(unsigned char nCardType);

// 21. 接触式存储卡类型识别
// 功能: 自动识别4442卡和4428卡命令
// 回参: nCardType:卡类型, 1字节(3=4428, 4=4442)
// 返回值: 0:成功, 非0:错误码
typedef int (*pContactVerify)(unsigned char *nCardType);

//---------------------------4442卡----------------------------
// 22. 检测是否4442卡
// 回参: sCardState: 卡状态(0:4442卡, 非:不是4442卡)
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4442Is42)(unsigned char* sCardState);

// 23. 获取卡片指定地址的数据
// 入参: nAddr: 地址,1字节(0x00--0xFF)
//      nDLen: 数据长度,2字节(1--256)
// 回参: sRecData: 返回的数据, rlen个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4442Read)(unsigned char nAddr, unsigned short nDLen, unsigned char* sRecData);

// 24. 更改卡片指定地址的数据
// 入参: nAddr: 地址,1字节(0x00--0xFF)
//      nDLen: 数据长度,2字节(1--256)
//      sWriteData: 写入的数据, nWLen个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4442Write)(unsigned char nAddr, unsigned short nWLen, unsigned char* sWriteData);

// 25. 读密码
// 回参: sKey: 密码, 3个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4442PwdRead)(unsigned char* sKey);

// 26. 校验密码是否正确
// 入参: sKey: 密码, 3个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4442PwdCheck)(unsigned char* sKey);

// 27. 修改密码
// 入参: sKey: 密码, 3个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4442PwdModify)(unsigned char* sKey);

// 28. 获取卡片保护位数据
// 入参: nLen: 读取保护位数据长度
// 回参: sProBitData: 返回数据
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4442ProbitRead)(unsigned char* nLen, unsigned char* sProBitData);

// 29. 对卡片指定地址的数据进行写保护
// 入参: nAddr: 地址, 1字节(0--31)
//      nWLen: 数据长度,2字节(1--32)
//      sProBitData: 写入的数据, nWLen个字节(此数据需要与卡中原有数据相同, 才可以成功写保护)
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4442ProbitWrite)(unsigned char nAddr, unsigned short nWLen, unsigned char* sProBitData);

// 30. 获取密码校验剩余错误次数
// 回参: nErrCount: 密码校验错误剩余次数, 1个字节
// 返回值: 0:成功, 非0:错误码
// 备注: 初始值为3, 密码校验时, 每错误一次, 计数值减1, 校验成功后, 计数值恢复为3, 如错误次数为0, 卡片被锁死, 只能读不能写
typedef int (*pSle4442ErrcountRead)(unsigned char* nErrCount);

//---------------------------4428卡----------------------------
// 31. 检测是否4428卡
// 回参: sCardState: 卡状态(0:4428卡, 非:不是4428卡)
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4428Is28)(unsigned char* sCardState);

// 32. 获取卡片指定地址的数据
// 入参: nAddr: 地址,1字节(0x00--0xFF)
//      nDLen: 数据长度,2字节(1--256)
// 回参: sRecData: 返回的数据, rlen个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4428Read)(unsigned short nAddr, unsigned short nDLen, unsigned char* sRecData);

// 33. 更改卡片指定地址的数据
// 入参: nAddr: 地址,1字节(0x00--0xFF)
//      nDLen: 数据长度,2字节(1--256)
//      sWriteData: 写入的数据, nWLen个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4428Write)(unsigned short nAddr, unsigned short nWLen, unsigned char* sWriteData);

// 34. 读密码
// 回参: sKey: 密码, 3个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4428PwdRead)(unsigned char* sKey);

// 35. 校验密码是否正确
// 入参: sKey: 密码, 3个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4428PwdCheck)(unsigned char* sKey);

// 36. 修改密码
// 入参: sKey: 密码, 3个字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4428PwdModify)(unsigned char* sKey);

// 37. 获取卡片保护位数据
// 入参: nLen: 读取保护位数据长度
// 回参: sProBitData: 返回数据
//      sRecData: 返回数据
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4428ProbitRead)(unsigned short nAddr, unsigned short nDLen, unsigned char* sRecData);

// 38. 对卡片指定地址的数据进行写保护
// 入参: nAddr: 地址, 1字节(0--31)
//      nWLen: 数据长度,2字节(1--32)
//      sProBitData: 写入的数据, nWLen个字节(此数据需要与卡中原有数据相同, 才可以成功写保护)
// 返回值: 0:成功, 非0:错误码
typedef int (*pSle4428ProbitWrite)(unsigned short nAddr, unsigned short nWLen, unsigned char* sWriteData);

// 39. 获取密码校验剩余错误次数
// 回参: nErrCount: 密码校验错误剩余次数, 1个字节
// 返回值: 0:成功, 非0:错误码
// 备注: 初始值为3, 密码校验时, 每错误一次, 计数值减1, 校验成功后, 计数值恢复为3, 如错误次数为0, 卡片被锁死, 只能读不能写
typedef int (*pSle4428ErrcountRead)(unsigned char* nErrCount);

//---------------------------M1卡----------------------------
// 40. 射频复位
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFReset)();

// 41. 激活非接触式存储卡
// 功能: 查寻卡是否进入感应区, 并激活进入感应区的非接触式存储卡
// 回参: nMode:寻卡模式(0: 全部卡片都响应该命令, 除了在HALT状态的那些卡片, 1: 在任何状态的全部卡片都将应该命令)
// 回参: sSnr: 4字节, 卡片UID
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFCard)(unsigned char nMode, unsigned char *sSnr);

// 42. 非接触式存储卡认证扇区
// 功能: 对非接触式存储卡的某一个扇区进行认证
// 入参: nMode: 认证模式(0:KEYA 模式, 1:KEYB 模式)
//      nBlockaddr: 块号(绝对地址)
//      sNkey: 认证的密码, 6字节
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFAuthenticationKey)(unsigned char nMode, unsigned char nBlockaddr, unsigned char *sNkey);

// 43. 读取数据
// 功能: 获取非接触存储卡指定块地址的数据
// 入参: nAdr: 块地址
// 回参: sReadData: 读出的数据
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFRead)(unsigned char nAdr, unsigned char *sReadData);

// 44. 写入数据
// 功能: 设置非接触存储卡指定块地址的数据
// 入参: nAdr: 块地址
// 回参: sWriteData: 写入的数据
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFWrite)(unsigned char nAdr, unsigned char *sWriteData);

// 45. 初始化块值
// 功能: 设置非接触存储卡指定块地址的数值
// 入参: nAdr: 块地址
//      ulValue: 传入的值
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFInitval)(unsigned char nAdr, unsigned long ulValue);

// 46. 读块值
// 功能: 获取非接触存储卡指定块地址的数值
// 入参: nAdr: 块地址
// 回参: ulValue: 读出的值
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFReadVal)(unsigned char nAdr, unsigned long *ulValue);

// 47. 加值
// 功能: 对非接触存储卡指定块地址的数据进行加值操作
// 入参: nAdr: 块地址
//      ulValue: 传入的值
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFIncrement)(unsigned char nAdr, unsigned long ulValue);

// 48. 减值
// 功能: 对非接触存储卡指定块地址的数据进行减值操作
// 入参: nAdr: 块地址
//      ulValue: 传入的值
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFDecrement)(unsigned char nAdr, unsigned long ulValue);

// 49.
// 入参: nAdr: 块地址
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFTransfer)(unsigned char nAdr);

// 50.
// 入参: nAdr: 块地址
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFRestore)(unsigned char nAdr);

// 51. 将卡片状态设置为halt
// 返回值: 0:成功, 非0:错误码
typedef int (*pRFTerminal)();

//---------------------------磁条卡----------------------------
// 52. 设置磁条卡模式
// 入参: nmode: 磁条卡模式, 如下:
//          0x00: 模拟键盘输出模式；
//          0x01: 命令输出模式；
//          0x40: 开关输出模式，启动磁条刷卡。发送此命令后，清空缓存数据，开始等待磁条刷卡；
//          0x41: 开关输出模式，关闭磁条刷卡。发送此命令后，清空缓存数据，结束等待磁条刷卡；
//          当mode最高位1时，磁条数据去掉‘；‘和'?’起始结束符后输出。也就是下面参数值，磁条数据去掉起始和结束符后输出：
//          0x80: 模拟键盘输出模式;
//          0x81: 命令输出模式；
//          0xC0: 开关输出模式，启动磁条刷卡。发送此命令后，清空缓存数据，开始等待磁条刷卡；
//          0xC1: 开关输出模式，关闭磁条刷卡。发送此命令后，清空缓存数据，结束等待磁条刷卡；
//          为了保证向下兼容，默认mode=0x01; 设置成功后，掉电保存。
// 返回值: 0:成功, 非0:错误码
typedef int (*pSetMagneticMode)(int nmode);

// 53. 读取磁条卡数据
// 入参: timeout: 刷卡超时时间(单位:S)
// 回参: track1_len: 1磁道数据长度
//      track2_len: 2磁道数据长度
//      track3_len: 3磁道数据长度
//      track1_data: 1磁道数据
//      track2_data: 2磁道数据
//      track3_data: 3磁道数据
// 返回值: 0:成功, 非0:错误码
typedef int (*pMagneticRead)(int timeout, int* track1_len, int* track2_len, int* track3_len,
                             char* track1_data, char* track2_data, char* track3_data);

//---------------------------二代证----------------------------
// 54. 获取身份证安全模块ID
// 回参: ModeIDLen: 返回模块ID的长度
//      ModeID: 模块ID(ASCII码)
// 返回值: 0:成功, 非0:错误码
typedef int (*pIDCardModuleId)(int *ModeIDLen,char *ModeID);

// 55. 获取身份证UID
// 回参: UIDLen: 返回UID的长度
//      UID: 身份证UID
// 返回值: 0:成功, 非0:错误码
typedef int (*pIDCardUid)(int *UIDLen,char *UID);

// 56. 读取身份证原始数据
// 入参: read_finger_data: 是否需要读取指纹数据(0:不读, 1:读取)
// 回参: textIDCMsg: 返回身份证的文字数据信息
//      textIDCMsgLen: 文字数据信息长度(256)
//      photoIDCMsg: 返回身份证的照片数据信息
//      photoIDCMsgLen: 照片数据信息长度(1024)
//      fingerIDCMsg: 返回身份证的指纹数据信息
//      fingerIDCMsgLen: 指纹数据信息长度(1024)
// 返回值: 0:成功, 非0:错误码
typedef int (*pIDCardReadBase)(int read_finger_data, unsigned char *textIDCMsg,
                               unsigned int  *textIDCMsgLen, unsigned char *photoIDCMsg,
                               unsigned int  *photoIDCMsgLen, unsigned char *fingerIDCMsg,
                               unsigned int  *fingerIDCMsgLen);

// 57. 解析身份证文字信息
// 功能	根据传入的文字信息原始数据解析身份证文字信息
// 入参: sCodeFormat: 按照什么编码解析文字信息(如:UTF-8/TRANSLIT/IGNORE)
//      textIDCMsg: 返回身份证的原始文字数据信息
//      textIDCMsgLen: 文字数据信息长度(256)
// 回参: cardType: 返回的证件类型(0:中国大陆身份证, 1:外国人永久居住证, 2:港澳台居民居住证)
//      cardMessage: 解析的证件信息, 各项数据以'|'号分隔, 如下:
//        中国大陆身份证:
//          姓名|性别|民族|出生日期|住址|公民身份证号码|签发机关|有效期起始日期|有效期截止日期|预留数据|
//        外国人永久居住证：
//          英文姓名|性别|永久居留证号码|国籍所在地区|中文姓名|有效期起始日期|有效期截止日期|出生日期|证件版本号|
//          当次申请受理机关代码|证件类型标识|预留数据|
//        港澳台居民居住证：
//          姓名|性别|出生日期|住址|公民身份证号码|签发机关|有效期起始日期|有效期截止日期|通行证号码|签发次数|证件类型标识|
// 返回值: 0:成功, 非0:错误码
typedef int (*pParseIDCardText)(char *sCodeFormat, unsigned char *textIDCMsg,
                                unsigned int textIDCMsgLen, int  *cardType,
                                char *cardMessage);

// 58. 读取磁条卡数据
// 功能: 根据传入的照片信息原始数据解析身份证件中的照片数据
// 入参: photoIDCMsg: 身份证件中的照片数据
//      photoIDCMsgLen: 身份证件中的照片数据的长度
//      photo_save_full_path: 需要保存文件全路径(如: "/home/zp.bmp")
// 回参: photo_base64: 照片文件的base64码
//      photo_base64_len: base64码长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pParseIDCardPhoto)(unsigned char *photoIDCMsg, unsigned int photoIDCMsgLen,
                                 char *photo_save_full_path, char *photo_base64,
                                 long  *photo_base64_len);

//---------------------------工具函数----------------------------
// 59.
typedef char* (*pMyStrUpr)(char *str/*,int len*/);

// 60.
typedef char* (*pMyStrLwr)(char *str/*,int len*/);

// 61.
typedef void (*pSplitPath)(char *path,char *dir, char *fname, char *ext);

// 62.
typedef int (*pMakePath)(char *path,char *dir, char *fname, char *ext);

// 63.
typedef int (*pBmpGenerate)(unsigned char *pSrcBmpdata, char *pBMPFile, int Width, int Height);

// 64.
typedef int (*pImgGenerate)(unsigned char *pSrcBmpdata, char *pBMPFile,
                            int Width, int Height,int ColorType/*0:RGB,1:BGR*/);

// 65. 字符编码转换
// 功能: 将字符编码从一个编码转到另一个编码
// 入参: from_charset: 需转换的编码的格式
//      to_charset: 转换后编码的格式
//      inbuf: 需转换的编码数据
//      inlen: 需转换的编码的长度
// 回参: outbuf: 转换后的编码数据
//      outlen: 转换后的编码的长度
// 返回值: 0:成功, 非0:错误码
// 备注: inlen,outlen的类型必须是size_t类型，不然虽然能转换成功，但是会报84错误
typedef int (*pCodeConvert)(char *from_charset, char *to_charset, char *inbuf,
                            size_t inlen, char *outbuf, size_t outlen);

// 66. 将16进制数转换为ASCII字符
// 功能: 将十六进制的数字数组转换为十六进制的字符数组
// 入参: hex: 输入要转换的HEX数据
// 回参: asc: 转换后的字符串
//      length: 16进制数据的字节长度
// 返回值: 0:成功, 非0:错误码
// 备注: 转换后的字符串长度为 2*length
typedef int (*pHex2Asc)(unsigned char *hex, unsigned char *asc, unsigned long length);

// 67. 将ASCII字符转换为16进制
// 功能: 将十六进制的字符数组转换为十六进制的数字数组
// 入参: asc: ASCII字符
// 回参: hex: 存放转换后的字符
//      length: 十六进制字符的字节长度(strlen(asc)/2)
// 返回值: 0:成功, 非0:错误码
typedef int (*pAsc2Hex)(unsigned char *asc, unsigned char *hex, unsigned long length);

// 68. 将Base64字符转换为16进制数
// 功能: 将Base64字符数组转换为十六进制的字符数组
// 入参: base64: Base64字符数组
// 回参: hex: 存放转换后的字符
//      base64len: Base64字符数组的字节长度
// 返回值: 0:成功, 非0:错误码
typedef long (*pBase642Hex)(unsigned char *base64,unsigned char *hex,unsigned long base64len);

// 69. 将16进制数转换为Base64
// 功能: 将十六进制的字符数组转换为Base64的字符数组
// 入参: hex: HEX字符数组
// 回参: base64: 存放转换后的字符
//      hexlen: HEX字符数组的字节长度
// 返回值: 0:成功, 非0:错误码
typedef long (*pHex2Base64)(unsigned char *hex, unsigned char *base64, unsigned long hexlen);

// 70. 解析二代证数据为RGB数据
// 功能: 将读取的二代证照片数据解析为RGB数据
// 入参: wlt_data: wlt文件数据,1024字节
//      VendrCode: 厂商代码
//      bsava_photo: 是否生成bmp图片(0:不生成, 1:生成)
//      photo_save_full_path: bmp文件全路径(如:"/home/zp.bmp")
// 回参: rgb_data: 解析的RGB数据，102*126*3字节, 可根据需求生成BMP或者JPG, 图像数据BGR格式, 需要将B、R值互换
// 返回值: 0:成功, 非0:错误码
typedef int (*pWlt2RGB)(char *wlt_data, char *rgb_data, int VendorCode,
                        int bsava_photo, char *photo_save_full_path);

// 71. 获取照片文件的base64码
// 功能: 将读取的二代证照片数据解析为RGB数据
// 入参: photo_save_full_path: bmp文件全路径(如:"/home/zp.bmp")
//      VendrCode: 厂商代码
//      bsava_photo: 是否生成bmp图片(0:不生成, 1:生成)
//      photo_save_full_path: bmp文件全路径(如:"/home/zp.bmp")
// 回参: base64: 照片文件的base64码
//      base64_len: base64码长度
// 返回值: 0:成功, 非0:错误码
typedef int (*pGetBase64Data)(char *photo_save_full_path, char *base64, long *base64_len);

// 72.
typedef int (*pGetErrmsg)(int errcode, char *errmsg);

//---------------------------日志函数----------------------------
// 73. 开启日志
// 入参: cSavePath: 日志文件保存路径(如:"/mnt/sdcard")
//      cLogFileName: 日志文件保存的文件夹
//      iMaxSize: 保存日志最大的大小 单位 Mb
//      iKeepingDate: 日志保存的天数
// 返回值: 0:成功, 非0:错误码
typedef int (*pEnabledLog)(char *cSavePath, char *cLogFileName, int iMaxSize, int iKeepingDate);

// 74. 关闭日志
// 返回值: 0:成功, 非0:错误码
typedef int (*pDisenabledLog)();


/**************************************************************************
* 命令编辑、发送接收等处理                                                    *
***************************************************************************/
class CDevImpl_MT50 : public CLogManage
{
public:
    CDevImpl_MT50();
    CDevImpl_MT50(LPSTR lpLog);
    ~CDevImpl_MT50();

public:
    INT     OpenDeviceUSB(INT nVid, INT nPid, INT nProtocol);           // 打开指定设备(USB方式)
    INT     CloseDevice();                                              // 关闭设备
    INT     IsDeviceOpen();                                             // 设备是否Open成功
    INT     SetReConFlag(BOOL bFlag);                                   // 设置断线重连标记
    INT     SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)
    LPSTR   ConvertCode_Impl2Str(INT nErrCode);

public: // 接口函数封装
    INT     GetCardStatus();                                            // 获取读卡器上卡状态
    INT     GetFWVersion(LPSTR lpVerStr, WORD wVerSize);                // 获取读卡器固件版本信息
    INT     SetReaderPowerOff();                                        // 读卡器卡片下电
    INT     SetReaderPowerOn(INT nMode, LPUCHAR lpData, INT &nDataLen); // 读卡器卡片上电
    INT     SendReaderAPDU(LPUCHAR lpSndData, INT nSndLen,
                           LPUCHAR lpRcvData, INT &nRcvLen);            // 读卡器发送APDU指令
    INT     SetReaderBeep(INT nMsec, INT nInterval, INT nCount);        // 设置读卡器鸣响

private:
    BOOL            m_bDevOpenOk;
    CHAR            m_szErrStr[1024];
    CHAR            m_szFWVer[256];
    BOOL            m_bReCon;                                           // 是否断线重连状态
    INT             m_nRetErrOLD[8];                                    // 处理错误值保存(0:库加载/1:设备连接/
                                                                        //  2:设备状态/3介质状态/4检查设备状态)

private:
    void    Init();

private: // 接口加载(QLibrary方式)
    BOOL    bLoadLibrary();
    void    vUnLoadLibrary();
    BOOL    bLoadLibIntf();
    void    vInitLibFunc();

private: // 接口加载(dlxxx方式)
    BOOL    bDlLoadLibrary();
    void    vDlUnLoadLibrary();
    BOOL    bDlLoadLibIntf();

private: // 接口加载(QLibrary方式)
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;

private: // 接口加载(dlxxx方式)
    void*   m_vDlHandle;

public: // 动态库接口定义
    pOpenUsbDevice              MT_OpenUsbDevice;           // 1. 打开USB设备
    pOpenSerialDevice           MT_OpenSerialDevice;        // 2. 打开串口设备
    pGetDeviceVersion           MT_GetDeviceVersion;        // 3. 获取设备版本
    pGetSoVersion               MT_GetSoVersion;            // 4. 获取动态库版本
    pDeviceBeep                 MT_DeviceBeep;              // 5. 设备蜂鸣
    pReadDeviceSNR              MT_ReadDeviceSNR;           // 6. 读设备序列号
    pWriteDeviceSNR             MT_WriteDeviceSNR;          // 7. 写设备序列号
    pCloseDevice                MT_CloseDevice;             // 8. 关闭打开的设备
    pPiccStatus                 MT_PiccStatus;              // 9. 非接CPU卡状态
    pPiccPowerOn                MT_PiccPowerOn;             // 10. 非接CPU卡上电
    pPiccPowerOff               MT_PiccPowerOff;            // 11. 非接CPU卡下电
    pPiccApdu                   MT_PiccApdu;                // 12. 非接CPU卡发送APDU
    pIccStatus                  MT_IccStatus;               // 13. 接触CPU卡状态
    pIccReset                   MT_IccReset;                // 14. 指定卡座寻卡上电热复位
    pIccResetBuad               MT_IccResetBuad;            // 15. 指定卡座寻卡上电热复位(指定波特率)
    pIccPowerOn                 MT_IccPowerOn;              // 16. 指定卡座寻卡上电冷复位
    pIccPowerOnBuad             MT_IccPowerOnBuad;          // 17. 指定卡座寻卡上电冷复位(指定波特率)
    pIccPowerOff                MT_IccPowerOff;             // 18. 接触CPU卡下电
    pIccApdu                    MT_IccApdu;                 // 19. 接触CPU卡发送APDU
    pContactSelect              MT_ContactSelect;           // 20. 接触式存储卡类型设置
    pContactVerify              MT_ContactVerify;           // 21. 接触式存储卡类型识别
    pSle4442Is42                MT_Sle4442Is42;             // 22. 检测是否4442卡
    pSle4442Read                MT_Sle4442Read;             // 23. 获取卡片指定地址的数据
    pSle4442Write               MT_Sle4442Write;            // 24. 更改卡片指定地址的数据
    pSle4442PwdRead             MT_Sle4442PwdRead;          // 25. 读密码
    pSle4442PwdCheck            MT_Sle4442PwdCheck;         // 26. 校验密码是否正确
    pSle4442PwdModify           MT_Sle4442PwdModify;        // 27. 修改密码
    pSle4442ProbitRead          MT_Sle4442ProbitRead;       // 28. 获取卡片保护位数据
    pSle4442ProbitWrite         MT_Sle4442ProbitWrite;      // 29. 对卡片指定地址的数据进行写保护
    pSle4442ErrcountRead        MT_Sle4442ErrcountRead;     // 30. 获取密码校验剩余错误次数
    pSle4428Is28                MT_Sle4428Is28;             // 31. 检测是否4428卡
    pSle4428Read                MT_Sle4428Read;             // 32. 获取卡片指定地址的数据
    pSle4428Write               MT_Sle4428Write;            // 33. 更改卡片指定地址的数据
    pSle4428PwdRead             MT_Sle4428PwdRead;          // 34. 读密码
    pSle4428PwdCheck            MT_Sle4428PwdCheck;         // 35. 校验密码是否正确
    pSle4428PwdModify           MT_Sle4428PwdModify;        // 36. 修改密码
    pSle4428ProbitRead          MT_Sle4428ProbitRead;       // 37. 获取卡片保护位数据
    pSle4428ProbitWrite         MT_Sle4428ProbitWrite;      // 38. 对卡片指定地址的数据进行写保护
    pSle4428ErrcountRead        MT_Sle4428ErrcountRead;     // 39. 获取密码校验剩余错误次数
    pRFReset                    MT_RFReset;                 // 40. 射频复位
    pRFCard                     MT_RFCard;                  // 41. 激活非接触式存储卡
    pRFAuthenticationKey        MT_RFAuthenticationKey;     // 42. 非接触式存储卡认证扇区
    pRFRead                     MT_RFRead;                  // 43. 读取数据
    pRFWrite                    MT_RFWrite;                 // 44. 写入数据
    pRFInitval                  MT_RFInitval;               // 45. 初始化块值
    pRFReadVal                  MT_RFReadVal;               // 46. 读块值
    pRFIncrement                MT_RFIncrement;             // 47. 加值
    pRFDecrement                MT_RFDecrement;             // 48. 减值
    pRFTransfer                 MT_RFTransfer;              // 49.
    pRFRestore                  MT_RFRestore;               // 50.
    pRFTerminal                 MT_RFTerminal;              // 51. 将卡片状态设置为halt
    pSetMagneticMode            MT_SetMagneticMode;         // 52. 设置磁条卡模式
    pMagneticRead               MT_MagneticRead;            // 53. 读取磁条卡数据
    pIDCardModuleId             MT_IDCardModuleId;          // 54. 获取身份证安全模块ID
    pIDCardUid                  MT_IDCardUid;               // 55. 获取身份证UID
    pIDCardReadBase             MT_IDCardReadBase;          // 56. 读取身份证原始数据
    pParseIDCardText            MT_ParseIDCardText;         // 57. 解析身份证文字信息
    pParseIDCardPhoto           MT_ParseIDCardPhoto;        // 58. 读取磁条卡数据
    pMyStrUpr                   MT_MyStrUpr;                // 59.
    pMyStrLwr                   MT_MyStrLwr;                // 60.
    pSplitPath                  MT_SplitPath;               // 61.
    pMakePath                   MT_MakePath;                // 62.
    pBmpGenerate                MT_BmpGenerate;             // 63.
    pImgGenerate                MT_ImgGenerate;             // 64.
    pCodeConvert                MT_CodeConvert;             // 65. 字符编码转换
    pHex2Asc                    MT_Hex2Asc;                 // 66. 将16进制数转换为ASCII字符
    pAsc2Hex                    MT_Asc2Hex;                 // 67. 将ASCII字符转换为16进制
    pBase642Hex                 MT_Base642Hex;              // 68. 将Base64字符转换为16进制数
    pHex2Base64                 MT_Hex2Base64;              // 69. 将16进制数转换为Base64
    pWlt2RGB                    MT_Wlt2RGB;                 // 70. 解析二代证数据为RGB数据
    pGetBase64Data              MT_GetBase64Data;           // 71. 获取照片文件的base64码
    pGetErrmsg                  MT_GetErrmsg;               // 72.
    pEnabledLog                 MT_EnabledLog;              // 73. 开启日志
    pDisenabledLog              MT_DisenabledLog;           // 74. 关闭日志
};

#endif // DEVIMPL_MT50_H
