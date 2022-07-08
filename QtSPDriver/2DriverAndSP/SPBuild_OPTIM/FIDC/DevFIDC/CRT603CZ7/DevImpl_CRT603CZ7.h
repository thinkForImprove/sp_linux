/***************************************************************
* 文件名称：DevImpl_CRT603CZ7.h
* 文件描述：创自非接模块底层指令，提供控制接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_CRT603CZ7_H
#define DEVIMPL_CRT603CZ7_H

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
#define IMP_ERR_DEV_F1                  -1          // An internal consistency check failed(内部一致性检查失败)
#define IMP_ERR_DEV_F2                  -2          // The action was cancelled by an SCardCancel request(该行动因一项取消请求而取消)
#define IMP_ERR_DEV_F3                  -3          // The supplied handle was invalid(提供的句柄无效)
#define IMP_ERR_DEV_F4                  -4          // One or more of the supplied parameters could not be properly interpreted(无法正确解释提供的一个或多个参数)
#define IMP_ERR_DEV_F5                  -5          // Registry startup information is missing or invalid(注册表启动信息丢失或无效)
#define IMP_ERR_DEV_F6                  -6          // Not enough memory available to complete this command(内存不足，无法完成此命令)
#define IMP_ERR_DEV_F7                  -7          // An internal consistency timer has expired(内部一致性计时器已过期)
#define IMP_ERR_DEV_F8                  -8          // The specified reader name is not recognized(无法识别指定的读取器名称)
#define IMP_ERR_DEV_F9                  -9          // The specified reader name is not recognized(无法识别指定的读取器名称)
#define IMP_ERR_DEV_F10                 -10         // The user-specified timeout value has expired(用户指定的超时值已过期)
#define IMP_ERR_DEV_F11                 -11         // The operation requires a Smart Card, but no Smart Card is currently in the device(该操作需要智能卡，但设备中当前没有智能卡)
#define IMP_ERR_DEV_F12                 -12         // The operation requires a Smart Card, but no Smart Card is currently in the device(该操作需要智能卡，但设备中当前没有智能卡)
#define IMP_ERR_DEV_F13                 -13         // The specified smart card name is not recognized(无法识别指定的智能卡名称)
#define IMP_ERR_DEV_F14                 -14         // The system could not dispose of the media in the requested manner(系统无法按请求的方式处置介质)
#define IMP_ERR_DEV_F15                 -15         // The requested protocols are incompatible with the protocol currently in use with the smart card(请求的协议与智能卡当前使用的协议不兼容)
#define IMP_ERR_DEV_F16                 -16         // The reader or smart card is not ready to accept commands(读卡器或智能卡尚未准备好接受命令)
#define IMP_ERR_DEV_F17                 -17         // One or more of the supplied parameters values could not be properly interpreted(无法正确解释提供的一个或多个参数值)
#define IMP_ERR_DEV_F18                 -18         // The action was cancelled by the system, presumably to log off or shut down(该操作被系统取消，可能是为了注销或关闭)
#define IMP_ERR_DEV_F19                 -19         // An internal communications error has been detected(检测到内部通信错误)
#define IMP_ERR_DEV_F20                 -20         // An internal error has been detected, but the source is unknown(检测到内部错误，但来源未知)
#define IMP_ERR_DEV_F21                 -21         // An ATR obtained from the registry is not a valid ATR string(从注册表获取的ATR不是有效的ATR字符串)
#define IMP_ERR_DEV_F22                 -22         // An attempt was made to end a non-existent transaction(试图结束一项不存在的交易)
#define IMP_ERR_DEV_F23                 -23         // The specified reader is not currently available for use(指定的读卡器当前不可用)
#define IMP_ERR_DEV_F24                 -24         // The operation has been aborted to allow the server application to exit(操作已中止，以允许服务器应用程序退出)
#define IMP_ERR_DEV_F25                 -25         // The PCI Receive buffer was too small(PCI接收缓冲区太小)
#define IMP_ERR_DEV_F26                 -26         // The reader driver does not meet minimal requirements for support(读卡器驱动程序不满足最低支持要求)
#define IMP_ERR_DEV_F27                 -27         // The reader driver did not produce a unique reader name(读卡器驱动程序没有生成唯一的读卡器名称)
#define IMP_ERR_DEV_F28                 -28         // The smart card does not meet minimal requirements for support(智能卡不满足最低支持要求)
#define IMP_ERR_DEV_F29                 -29         // The Smart card resource manager is not running(智能卡资源管理器未运行)
#define IMP_ERR_DEV_F30                 -30         // The Smart card resource manager has shut down(智能卡资源管理器已关闭)
#define IMP_ERR_DEV_F31                 -31         // An unexpected card error has occurred(发生了意外的卡错误)
#define IMP_ERR_DEV_F32                 -32         // No Primary Provider can be found for the smart card(找不到智能卡的主要提供商)
#define IMP_ERR_DEV_F33                 -33         // The requested order of object creation is not supported(不支持请求的对象创建顺序)
#define IMP_ERR_DEV_F34                 -34         // This smart card does not support the requested feature(此智能卡不支持请求的功能)
#define IMP_ERR_DEV_F35                 -35         // The identified directory does not exist in the smart card(智能卡中不存在标识的目录)
#define IMP_ERR_DEV_F36                 -36         // The identified file does not exist in the smart card(智能卡中不存在识别的文件)
#define IMP_ERR_DEV_F37                 -37         // The supplied path does not represent a smart card directory(提供的路径不代表智能卡目录)
#define IMP_ERR_DEV_F38                 -38         // The supplied path does not represent a smart card file(提供的路径不代表智能卡文件)
#define IMP_ERR_DEV_F39                 -39         // Access is denied to this file(拒绝访问此文件)
#define IMP_ERR_DEV_F40                 -40         // The smartcard does not have enough memory to store the information(智能卡内存不足，无法存储信息)
#define IMP_ERR_DEV_F41                 -41         // There was an error trying to set the smart card file object pointer(试图设置智能卡文件对象指针时出错)
#define IMP_ERR_DEV_F42                 -42         // The supplied PIN is incorrect(提供的PIN不正确)
#define IMP_ERR_DEV_F43                 -43         // An unrecognized error code was returned from a layered component(分层组件返回了无法识别的错误代码)
#define IMP_ERR_DEV_F44                 -44         // The requested certificate does not exist(请求的证书不存在)
#define IMP_ERR_DEV_F45                 -45         // The requested certificate could not be obtained(无法获取请求的证书)
#define IMP_ERR_DEV_F46                 -46         // Cannot find a smart card reader(找不到智能卡读卡器)
#define IMP_ERR_DEV_F47                 -47         // A communications error with the smart card has been detected, Retry the operation(检测到智能卡存在通信错误, 请重试该操作)
#define IMP_ERR_DEV_F48                 -48         // The requested key container does not exist on the smart card(智能卡上不存在请求的密钥容器)
#define IMP_ERR_DEV_F49                 -49         // The Smart card resource manager is too busy to complete this operation(智能卡资源管理器太忙，无法完成此操作)
#define IMP_ERR_DEV_F101                -101        // The Smart card resource manager is too busy to complete this operation(智能卡资源管理器太忙，无法完成此操作)
#define IMP_ERR_DEV_F102                -102        // The smart card is not responding to a reset(智能卡对重置没有响应)
#define IMP_ERR_DEV_F103                -103        // Power has been removed from the smart card, so that further communication is not possible(智能卡已断电，因此无法进行进一步通信)
#define IMP_ERR_DEV_F104                -104        // The smart card has been reset, so any shared state information is invalid(智能卡已重置，因此任何共享状态信息都无效)
#define IMP_ERR_DEV_F105                -105        // The smart card has been removed, so that further communication is not possible(智能卡已被移除，因此无法进行进一步通信)
#define IMP_ERR_DEV_F106                -106        // Access was denied because of a security violation(由于安全违规，访问被拒绝)
#define IMP_ERR_DEV_F107                -107        // The card cannot be accessed because the wrong PIN was presented(无法访问该卡，因为提供了错误的PIN)
#define IMP_ERR_DEV_F108                -108        // The card cannot be accessed because the maximum number of PIN entry attempts has been reached(无法访问该卡，因为已达到尝试输入PIN码的最大次数)
#define IMP_ERR_DEV_F109                -109        // The end of the smart card file has been reached(已到达智能卡文件的末尾)
#define IMP_ERR_DEV_F110                -110        // The action was cancelled by the user(该操作已被用户取消)
#define IMP_ERR_DEV_F111                -111        // No PIN was presented to the smart card(智能卡上未显示PIN)
#define IMP_ERR_DEV_F112                -112        // The requested item could not be found in the cache(在缓存中找不到请求的项)
#define IMP_ERR_DEV_F113                -113        // The requested cache item is too old and was deleted from the cache(请求的缓存项太旧，已从缓存中删除)
#define IMP_ERR_DEV_F114                -114        // The new cache item exceeds the maximum per-item size defined for the cache(新缓存项超过了为缓存定义的最大每项大小)
#define IMP_ERR_DEV_F99                 -99         // Function returned unknown error code(函数返回未知错误代码)


/*************************************************************************
// 无分类　宏定义
*************************************************************************/
#define LOG_NAME            "DevImpl_CRT603CZ7.log"         // 缺省日志名
#define DLL_DEVLIB_NAME     "libCRT603CZ7.so"               // 缺省动态库名

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
// SDK使用 宏定义
*************************************************************************/
// 返回值: 卡片状态
#define CARD_STAT_HAVE                  1       // 有卡
#define CARD_STAT_NOHAVE                2       // 无卡
#define CARD_STAT_UNKNOWN               9       // 未知状态

// 读卡器操作模式
enum EN_CARDER_MODE
{
    CARDER_MODE_RF          = 1,    // 正常RF卡模式
    CARDER_MODE_FELICA      = 2,    // Felica模式
    CARDER_MODE_PTOP        = 3,    // 点对点模式
    CARDER_MODE_IDCARD      = 4,    // 二代证模式
    CARDER_MODE_SIMULATION  = 5,    // 卡模拟模式
};

// 版本类型
enum EN_VER_TYPE
{
    VER_PN      = 0,        // P/N信息
    VER_SN      = 1,        // SN信息
    VER_FW      = 2,        // 固件版本信息
    VER_SC      = 3,        // 生成版本信息
    VER_EMID    = 4,        // EMID信息
    VER_SDK     = 5,        // 动态库版本信息
};

// RF卡类型
enum EN_CARDRF_TYPE
{
    CARDRF_NOHAVE   = 0,    // 无卡
    CARDRF_TYPEA    = 1,    // TYPE A类型卡
    CARDRF_TYPEB    = 2,    // TYPE B类型卡
    CARDRF_IDCARD   = 3,    // 身份证
                            // 其他失败
};


/*************************************************************************
// SDK使用 结构体定义
*************************************************************************/
// 二代证信息
typedef struct ST_CRT_DEF_IDINFO
{
    char szName[31];                // 姓名
    char szSex[5];                  // 性别
    char szNation[20];              // 民族
    char szBornDay[10];             // 出生
    char szAddress[128];            // 地址
    char szIDNum[20];               // 身份证编号
    char szIssued[51];              // 签发机关
    char szBeginValidity[10];       // 开始有效日期
    char szEndValidity[10];         // 截止有效日期
    char szImgData[1024];           // 头文件数据
} STCRTDEFIDINFO, *LPSTCRTDEFIDINFO;



/*************************************************************************
// 动态库输出函数接口　定义
*************************************************************************/
// 1. 打开CRT智能读卡器
// 入参:
// 回参: iListNums,连接的读卡器列表个数
// 返回值: 0成功,非0失败
typedef int (*pCRT_OpenConnect)(int* iListNums);

// 2. 关闭CRT智能读卡器
// 返回值: 0成功,非0失败
typedef int (*pCRT_CloseConnect)();

// 3. 获取智能读卡器列表名字
// 入参: iListNum: 需获取的读卡器序列号(0为起始)
// 回参: szListName: 获取到的读卡器名字
// 返回值: 0成功,非0失败
typedef int (*pCRT_GetReaderListName)(int iListNum, char szListName[]);

// 4. 设置当前工作的读卡器序列号
//    打开时，默认设置为第一个读卡器
// 入参: iListNum:需设置的读卡器序列号(0为起始)
// 返回值: 0成功,非0失败
typedef int (*pCRT_SetReaderName)(int iListNum);

// 5. 获取读卡器上卡状态
// 返回值: 1:有卡, 2:无卡, 9:状态未知
typedef int (*pCRT_GetCardStatus)();

// 6. 弹卡下电,读卡器断开连接
// 返回值: 0成功,非0失败
typedef int (*pCRT_EjectCard)();

// 7. 读卡器卡片上电
// 回参: byAtrData:输出 上电成功返回的ATR数据
//      iAtrLen: 输出 上电成功返回的ATR数据长度
// 返回值: 0成功,非0失败
typedef int (*pCRT_ReaderConnect)(BYTE byAtrData[], int* iAtrLen);

// 8. 读卡器发送APDU指令
// 入参: bySendData:需发送的APDU指令（支持acsii码与BCD码）
//      iSendDataLen: 需发送的APDU指令长度
// 回参: byRecvData:输出 APDU通讯后返回的数据
//      iRecvDataLen: 输出 APDU通讯后返回的数据长度
// 返回值: 0成功,非0失败
typedef int (*pCRT_SendApdu)(BYTE bySendData[], int iSendDataLen, BYTE byRecvData[], int* iRecvDataLen);

// 9. 读卡器发送扩展控制命令
// 入参: bySendData:需发送的扩展控制命令数据（支持acsii码与BCD码）
//      iSendDataLen: 需发送的扩展控制命令数据长度
// 回参: byRecvData: 扩展控制命令通讯后返回的数据
//      iRecvDataLen: 扩展控制命令通讯后返回的数据长度
// 返回值: 0成功,非0失败
typedef int (*pCRT_SendControlCMD)(BYTE bySendData[], int iSendDataLen, BYTE byRecvData[], int* iRecvDataLen);

//***********************以下为 发送CRT603具体功能扩展命令****************************
// 10. 读卡器热复位
// 返回值: 0成功,非0失败
typedef int (*pCRT_HotReset)();

// 11. 设置读卡器操作模式
// 入参: iType: 1:正常RF卡模式, 2:Felica模式, 3:点对点模式, 4:二代证模式, 5:卡模拟模式
// 返回值: 0成功,非0失败
typedef int (*pCRT_SetReaderType)(int iType);

// 12. 获取读卡器操作模式
// 返回值: >0:失败, 1:正常RF卡模式, 2:Felica模式, 3:点对点模式, 4:二代证模式, 5:卡模拟模式
typedef int (*pCRT_GetReaderType)();

// 13. 获取读卡器版本信息
// 入参: iVerType: 需获取的版本(0:P/N信息, 1:SN信息, 2:固件版本信息, 3:生成版本信息, 4:EMID信息, 5:动态库版本信息)
// 回参: szVersionInfo: 返回的版本信息
// 返回值: 0成功,非0失败
typedef int (*pCRT_GetVersionInfo)(int iVerType, char szVersionInfo[]);

// 14. 读卡器自动蜂鸣
// 入参: bAutoBeel: 是否自动蜂鸣，true 开启，false 关闭
// 返回值: 0成功,非0失败
typedef int (*pCRT_AutoBeel)(bool bAutoBeel);

// 15. 读卡器蜂鸣设置
// 入参: MultipleTime: 蜂鸣时间(0.25秒的倍数, 比如2, 蜂鸣时间=2*0.25,即0.5秒)(0-20默认为1)
// 返回值: 0成功,非0失败
typedef int (*pCRT_Beel)(int MultipleTime);

// 16. 设置读卡器指示灯模式
// 入参: iType: 指示灯模式(0:自动模式, 1:手动模式)
// 返回值: 0成功,非0失败
typedef int (*pCRT_SetLightMode)(int iType);

// 17. 获取读卡器指示灯模式
// 返回值: 0:自动模式, 1:手动模式, 9:模式未知
typedef int (*pCRT_GetLightMode)();

// 18. 设置读卡器指示灯状态
// 入参: iYellowType: 黄色指示灯状态(0:关, 1:开, 2:闪烁)
//      iBlueType: 蓝色指示灯状态(0:关, 1:开, 2:闪烁)
//      iGreenType: 绿色指示灯状态(0:关, 1:开, 2:闪烁)
//      iRedType: 红色指示灯状态(0:关, 1:开, 2:闪烁)
// 返回值: 0成功,非0失败
typedef int (*pCRT_SetLightStatus)(int iYellowType, int iBlueType, int iGreenType,  int iRedType);

// 19. 获取读卡器指示灯状态
// 回参: iYellowType: 黄色指示灯状态(0:关, 1:开, 2:闪烁)
//      iBlueType: 蓝色指示灯状态(0:关, 1:开, 2:闪烁)
//      iGreenType: 绿色指示灯状态(0:关, 1:开, 2:闪烁)
//      iRedType: 红色指示灯状态(0:关, 1:开, 2:闪烁)
// 返回值: 0成功,非0失败
typedef int (*pCRT_GetLightStatus)(int* iYellowType, int* iBlueType, int* iGreenType,  int* iRedType);

// 20. 设置读卡器读取TYPE B卡能力
// 入参: bBan: 是否关闭读TYPE B卡能力(true:关闭, false:打开)
// 返回值: 0成功,非0失败
typedef int (*pCRT_BanTypeBCap)(bool bBan);

//***********************以下为 发送CRT603具体功能APDU命令****************************
// 21. Mifare卡下载密码
// 入参: ilocal: 密码存储位置(0:临时性存储器, 1:非易失存储器)
//      iKeyType: 密钥类型(0:TYPE A类型, 1:TYPE B类型)
//      iKeyNum: 将保存到密钥组号(共0-15组)
//      byInKeyData: 密码信息(共6位, 如0xFF,0xFF,0xFF,0xFF,0xFF,0xFF)
// 返回值: 0成功,非0失败
typedef int (*pCRT_LoadMifareKey)(int ilocal, int iKeyType, int iKeyNum, BYTE byInKeyData[]);

// 22. Mifare卡校验密码
// 入参: iKeyType: 密钥类型(0:TYPE A类型, 1:TYPE B类型)
//      iKeyNum: 已下载好的密钥组号(共0-15组)
//      iBlockNum: 需校验的Mifare卡块号(0为起始位)
// 返回值: 0成功,非0失败
typedef int (*pCRT_CheckMifareKey)(int iKeyType, int iKeyNum, int iBlockNum);

// 23. 非CPU卡读数据操作
// 入参: bFilica: 是否为filica卡读操作(true:是filica, false:非filica)
//      iBlockNum: 需读取的卡块号(0为起始位)
// 回参: byReadData: 读取到的数据
//      iReadDataLen: 读取到的数据长度
// 返回值: 0成功,非0失败
typedef int (*pCRT_Read)(bool bFilica, int iBlockNum, BYTE byReadData[], int* iReadDataLen);

// 24. 非CPU卡写数据操作
// 入参: bFilica: 是否为filica卡读操作(true:是filica, false:非filica)
//      iBlockNum: 需写入的卡块号(0为起始位)
//      byWriteData: 写入到读卡器上的数据
//      iWriteLen: 写入到读卡器上的数据长度
// 返回值: 0成功,非0失败
typedef int (*pCRT_Write)(bool bFilica, int iBlockNum, BYTE byWriteData[], int iWriteLen);

// 25. 获取卡片UID信息
// 回参: szUID: 卡片的UID信息
// 返回值: 0成功,非0失败
typedef int (*pCRT_GetCardUID)(char szUID[]);

//***********************以下为 发送CRT603具体功能SAM卡命令****************************
// 26. SAM卡切换并激活卡座
// 入参: iSlotNum: 需切换激活的卡座号(1-4个, 如: 1表示SAM1卡座)
// 返回值: 0成功,非0失败
typedef int (*pCRT_SAMSlotActivation)(int iSlotNum);

//***********************以下为 磁条卡操作命令****************************
// 27. 读所有磁道操作
// 回参: szTrack1: 磁道1数据, ‘Not’未读取到
//      szTrack2: 磁道2数据, ‘Not’未读取到
//      szTrack3: 磁道3数据, ‘Not’未读取到
// 返回值: 0成功,非0失败
typedef int (*pCRT_ReadMagAllTracks)(char szTrack1[], char szTrack2[], char szTrack3[]);

//***********************以下为 获取RF卡类型****************************
// 28. 获取RF卡类型
// 返回值: 0:无卡, 1:TYPE A类型卡, 2:TYPE B类型卡, 3:身份证, 其他:失败
typedef int (*pCRT_GetRFCardType)();

//***********************以下为 读二代证信息命令****************************
// 29. 读二代证信息
// 回参: crtdef_IdInfo: 读取到的二代证信息
// 返回值: 0成功,非0失败
typedef int (*pCRT_ReadIDCardInifo)(LPSTCRTDEFIDINFO crtdef_IdInfo);

// 30. M1卡值操作
// 入参: iMode: 操作模式(1:初始化钱包, 2:增值, 3:减值)
//      iBlock: 操作块区(需绝对地址)
//      iValue: 操作金额
// 返回值: 0成功,非0失败
typedef int (*pCRT_M1ValueProcess)(int iMode, int iBlock, int iValue);

// 31. M1卡查询余额
// 入参: iBlock: 操作块区(需绝对地址)
// 回参: iValue: 输出,查询到的金额值
// 返回值: 0成功,非0失败
typedef int (*pCRT_M1InquireBalance)(int iBlock, int* iValue);

// 32. M1卡备份钱包
// 入参: iBlock: 需备份块区(需绝对地址)
//      iBackBlock: 备份到块区(需绝对地址)
// 返回值: 0成功,非0失败
typedef int (*pCRT_M1BackBlock)(int iBlock, int iBackBlock);

// 33. 获取二代证指纹信息
// 回参: byFinger: 获取到的指纹信息(一般为1024个字节)
//      iFingerLen: 指纹信息长度
// 返回值: 0成功,非0失败
typedef int (*pCRT_GetIDFinger)(BYTE byFinger[], int *iFingerLen);

// 34. 获取二代证DN号
// 回参: szDNNums: 获取到的DN号
// 返回值: 0成功,非0失败
typedef int (*pCRT_GetIDDNNums)(char szDNNums[]);

// 35. 获取身份证盒子SAM ID
// 回参: szSAMID: 获取到的SAM ID
// 返回值: 0成功,非0失败
typedef int (*pCRT_GetSAMID)(char szSAMID[]);

// 36. 读卡器开关射频场
// 入参: iMode: 开关射频场方式(0:开启, 1:关闭)
// 返回值: 0成功,非0失败
typedef int (*pCRT_SwitchRF)(int iMode);

// 37. 获取最后一次错误描叙
// 返回值: 获取的信息
typedef char* (*pCRT_GetLastError)();

// 38. 设置身份证头像保存路径
// 入参: pHeadPath 路径
// 返回值: 0成功,非0失败
typedef int (*pCRT_SetHeadPath)(char* pHeadPath);


/**************************************************************************
* 命令编辑、发送接收等处理
***************************************************************************/
class CDevImpl_CRT603CZ7 : public CLogManage
{
public:
    CDevImpl_CRT603CZ7();
    CDevImpl_CRT603CZ7(LPSTR lpLog);
    ~CDevImpl_CRT603CZ7();

public:
    INT     OpenDevice();                                               // 打开指定设备
    INT     OpenDevice(LPSTR lpcMode);                                  // 打开指定设备(指定参数)
    INT     CloseDevice();                                              // 关闭设备
    INT     IsDeviceOpen();                                             // 设备是否Open成功
    INT     SetReConFlag(BOOL bFlag);                                   // 设置断线重连标记
    INT     SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)
    LPSTR   ConvertCode_IMPL2Str(INT nErrCode);

public: // 接口函数封装
    INT     GetCardStatus();                                            // 5. 获取读卡器上卡状态
    INT     SetEjectCard();                                             // 6. 弹卡下电,读卡器断开连接
    INT     SetReaderPowerOn(LPUCHAR lpData, INT &nDataLen);            // 7. 读卡器卡片上电
    INT     SendReaderAPDU(LPUCHAR lpSndData, INT nSndLen,
                           LPUCHAR lpRcvData, INT &nRcvLen);            // 8. 读卡器发送APDU指令
    INT     ReaderReset();                                              // 10. 读卡器热复位
    INT     SetReaderMode(EN_CARDER_MODE enMode);                       // 11. 设置读卡器操作模式
    INT     GetReaderMode();                                            // 12. 获取读卡器操作模式
    INT     GetReaderVersion(LPSTR lpVerStr, WORD wVerSize,
                             EN_VER_TYPE enVerType = VER_FW);           // 13. 获取读卡器版本信息
    INT     SetBeepAuto(BOOL bIsAuto = TRUE);                           // 14. 读卡器自动蜂鸣设置
    INT     SetBeepCont(INT nMesc);                                     // 15. 读卡器蜂鸣设置
    INT     SetLightAuto(INT nMode);                                    // 16. 设置读卡器指示灯模式
    INT     GetLightAuto();                                             // 17. 获取读卡器指示灯模式
    INT     SetLightStat(INT nYellow, INT nBlue, INT nGreen, INT Red);  // 18. 设置读卡器指示灯状态
    INT     GetLightStat(INT &nYellow, INT &nBlue, INT &nGreen, INT &Red);// 19. 获取读卡器指示灯状态
    INT     SetCloseTypeBCap(BOOL bIsClose = TRUE);                     // 20. 设置关闭读卡器读取TYPE B卡能力
    INT     GetCardRFType();                                            // 28. 获取RF卡类型
    INT     GetIDCardInfo(LPSTCRTDEFIDINFO lpStInfo);                   // 29. 读二代证信息
    INT     GetIDCardFinger(LPBYTE lpFinger, INT &nLen);                // 33. 获取二代证指纹信息
    INT     GetIDCardDN(LPSTR lpDN);                                    // 34. 获取二代证DN号
    INT     GetIDCardSAMID(LPSTR lpSAMID);                              // 35. 获取身份证盒子SAM ID
    INT     SetSwitchRF(INT nMode);                                     // 36. 读卡器开关射频场
    CHAR*   GetLastError();                                             // 37. 获取最后一次错误描叙
    INT     SetIDCardHeadImgPath(LPSTR lpPath);                         // 38. 设置身份证头像保存路径

private:
    WORD            m_wReaderCnt;                                       // 连接的读卡器数目
    CHAR            m_szReaderList[32][128];                            // 连接的读卡器列表
    BOOL            m_bDevOpenOk;
    CHAR            m_szErrStr[1024];
    CHAR            m_szFWVer[256];
    BOOL            m_bReCon;                                           // 是否断线重连状态
    INT             m_nRetErrOLD[8];                                    // 处理错误值保存(0:库加载/1:设备连接/
                                                                        //  2:设备状态/3介质状态/4检查设备状态/
                                                                        //  5:设置设备列表/6:设备列表)
private:
    void    Init();

private: // 接口加载(QLibrary方式)
    BOOL    bLoadLibrary();
    void    vUnLoadLibrary();
    BOOL    bLoadLibIntf();
    void    vInitLibFunc();

private: // 接口加载(QLibrary方式)
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;

private: // 接口加载(dlxxx方式)
    BOOL    bDlLoadLibrary();
    void    vDlUnLoadLibrary();
    BOOL    bDlLoadLibIntf();

private: // 接口加载(dlxxx方式)
    void*   m_vDlHandle;

public: // 动态库接口定义
    pCRT_OpenConnect                    CRT_OpenConnect;            // 1. 打开CRT智能读卡器
    pCRT_CloseConnect                   CRT_CloseConnect;           // 2. 关闭CRT智能读卡器
    pCRT_GetReaderListName              CRT_GetReaderListName;      // 3. 获取智能读卡器列表名字
    pCRT_SetReaderName                  CRT_SetReaderName;          // 4. 设置当前工作的读卡器序列号
    pCRT_GetCardStatus                  CRT_GetCardStatus;          // 5. 获取读卡器上卡状态
    pCRT_EjectCard                      CRT_EjectCard;              // 6. 弹卡下电,读卡器断开连接
    pCRT_ReaderConnect                  CRT_ReaderConnect;          // 7. 读卡器卡片上电
    pCRT_SendApdu                       CRT_SendApdu;               // 8. 读卡器发送APDU指令
    pCRT_SendControlCMD                 CRT_SendControlCMD;         // 9. 读卡器发送扩展控制命令
    pCRT_HotReset                       CRT_HotReset;               // 10. 读卡器热复位
    pCRT_SetReaderType                  CRT_SetReaderType;          // 11. 设置读卡器操作模式
    pCRT_GetReaderType                  CRT_GetReaderType;          // 12. 获取读卡器操作模式
    pCRT_GetVersionInfo                 CRT_GetVersionInfo;         // 13. 获取读卡器版本信息
    pCRT_AutoBeel                       CRT_AutoBeel;               // 14. 读卡器自动蜂鸣
    pCRT_Beel                           CRT_Beel;                   // 15. 读卡器蜂鸣设置
    pCRT_SetLightMode                   CRT_SetLightMode;           // 16. 设置读卡器指示灯模式
    pCRT_GetLightMode                   CRT_GetLightMode;           // 17. 获取读卡器指示灯模式
    pCRT_SetLightStatus                 CRT_SetLightStatus;         // 18. 设置读卡器指示灯状态
    pCRT_GetLightStatus                 CRT_GetLightStatus;         // 19. 获取读卡器指示灯状态
    pCRT_BanTypeBCap                    CRT_BanTypeBCap;            // 20. 设置读卡器读取TYPE B卡能力
    pCRT_LoadMifareKey                  CRT_LoadMifareKey;          // 21. Mifare卡下载密码
    pCRT_CheckMifareKey                 CRT_CheckMifareKey;         // 22. Mifare卡校验密码
    pCRT_Read                           CRT_Read;                   // 23. 非CPU卡读数据操作
    pCRT_Write                          CRT_Write;                  // 24. 非CPU卡写数据操作
    pCRT_GetCardUID                     CRT_GetCardUID;             // 25. 获取卡片UID信息
    pCRT_SAMSlotActivation              CRT_SAMSlotActivation;      // 26. SAM卡切换并激活卡座
    pCRT_ReadMagAllTracks               CRT_ReadMagAllTracks;       // 27. 读所有磁道操作
    pCRT_GetRFCardType                  CRT_GetRFCardType;          // 28. 获取RF卡类型
    pCRT_ReadIDCardInifo                CRT_ReadIDCardInifo;        // 29. 读二代证信息
    pCRT_M1ValueProcess                 CRT_M1ValueProcess;         // 30. M1卡值操作
    pCRT_M1InquireBalance               CRT_M1InquireBalance;       // 31. M1卡查询余额
    pCRT_M1BackBlock                    CRT_M1BackBlock;            // 32. M1卡备份钱包
    pCRT_GetIDFinger                    CRT_GetIDFinger;            // 33. 获取二代证指纹信息
    pCRT_GetIDDNNums                    CRT_GetIDDNNums;            // 34. 获取二代证DN号
    pCRT_GetSAMID                       CRT_GetSAMID;               // 35. 获取身份证盒子SAM ID
    pCRT_SwitchRF                       CRT_SwitchRF;               // 36. 读卡器开关射频场
    pCRT_GetLastError                   CRT_GetLastError;           // 37. 获取最后一次错误描叙
    pCRT_SetHeadPath                    CRT_SetHeadPath;            // 38. 设置身份证头像保存路径

};

#endif // DEVIMPL_CRT603CZ7_H
