/***************************************************************
* 文件名称：DevImpl_CRT780B.h
* 文件描述：声明CRT780B退卡模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

/***************************************************************
* 文件名称：DevImpl_CRT730B.h
* 文件描述：封装退卡模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEVIMPL_CRT730B_H
#define DEVIMPL_CRT730B_H


#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "IAllDevPort.h"

/*************************************************************
// 返回值/错误码　宏定义
// <0 : USB/COM接口处理返回
// 0~100: 硬件设备返回
// > 100: Impl处理返回
*************************************************************/
// > 100: Impl处理返回
#define IMP_SUCCESS             0       // 成功
#define IMP_ERR_LOAD_LIB        101     // 动态库加载失败
#define IMP_ERR_PARAM_INVALID   102     // 参数无效
#define IMP_ERR_READERROR       103     // 读数据错误
#define IMP_ERR_WRITEERROR      104     // 写数据错误
#define IMP_ERR_RCVDATA_INVALID 105     // 无效的应答数据
#define IMP_ERR_STAT_POS_FAIL   106     // 状态返回暂存仓位置异常
#define IMP_ERR_STAT_INVALID    107     // 状态返回无效值
#define IMP_ERR_SLOTNO_INVALID  108     // 指定暂存仓编号无效
#define IMP_ERR_UNKNOWN         109     // 未知错误
#define IMP_ERR_SNDDATA_INVALID 110     // 无效的下发数据

// <0 : USB/COM接口处理返回
#define IMP_ERR_DEVPORT_NOTOPEN      ERR_DEVPORT_NOTOPEN      // (-1) 没打开
#define IMP_ERR_DEVPORT_FAIL         ERR_DEVPORT_FAIL         // (-2) 通讯错误
#define IMP_ERR_DEVPORT_PARAM        ERR_DEVPORT_PARAM        // (-3) 参数错误
#define IMP_ERR_DEVPORT_CANCELED     ERR_DEVPORT_CANCELED     // (-4) 操作取消
#define IMP_ERR_DEVPORT_READERR      ERR_DEVPORT_READERR      // (-5) 读取错误
#define IMP_ERR_DEVPORT_WRITE        ERR_DEVPORT_WRITE        // (-6) 发送错误
#define IMP_ERR_DEVPORT_RTIMEOUT     ERR_DEVPORT_RTIMEOUT     // (-7) 操作超时
#define IMP_ERR_DEVPORT_WTIMEOUT     ERR_DEVPORT_WTIMEOUT     // (-8) 操作超时
#define IMP_ERR_DEVPORT_LIBRARY      ERR_DEVPORT_LIBRARY      // (-98) 加载通讯库失败
#define IMP_ERR_DEVPORT_NODEFINED    ERR_DEVPORT_NODEFINED    // (-99) 未知错误

// 0~100: 硬件设备返回
#define IMP_ERR_DEVRET_CMD_INVALID       1  // 命令编码未定义
#define IMP_ERR_DEVRET_CMD_PARAM         2  // 命令参数错误
#define IMP_ERR_DEVRET_CMD_DATA          3  // 命令数据错误
#define IMP_ERR_DEVRET_CMD_UNABLE        4  // 命令无法执行
#define IMP_ERR_DEVRET_DOOR_ISOPEN       5  // 维护门已经开启（此时不能发送动作指令!）
#define IMP_ERR_DEVRET_POWER_VOLLOW      6  // 电源电压过低(低于20V)
#define IMP_ERR_DEVRET_POWER_VOLHIGH     7  // 电源电压过高(高于28V)
#define IMP_ERR_DEVRET_POWER_NEEDINIT    8  // 电源恢复正常但没有重新初始化
#define IMP_ERR_DEVRET_POWER_FRAM        9  // 模块内部FRAM错误
#define IMP_ERR_DEVRET_SOLT_SENSER       10  // 暂存仓传感器错误
#define IMP_ERR_DEVRET_SLOT_JAM          11  // 升降暂存仓阻塞
#define IMP_ERR_DEVRET_INCARD_JAM        12  // 进卡阻塞
#define IMP_ERR_DEVRET_INCARD_TIMEOUT    13  // 等待进卡超时
#define IMP_ERR_DEVRET_INCARD_TAKE       14  // 进卡时卡被拔走
#define IMP_ERR_DEVRET_EJECT_JAM         15  // 退卡堵塞
#define IMP_ERR_DEVRET_POWER_LOCAT_JAM   16  // 电机定位堵塞
#define IMP_ERR_DEVRET_SLOT_NOTAIM_CK    17  // 暂存仓未对准进卡口
#define IMP_ERR_DEVRET_SLOT_HAVECARD     18  // 目标暂存仓有卡
#define IMP_ERR_DEVRET_SLOT_NOTHAVE      19  // 目标暂存仓无卡
#define IMP_ERR_DEVRET_CINFO_NOSAVE      20  // 卡片信息未保存

/*************************************************************
// 无分类　宏定义
*************************************************************/
#define DEV_VID_DEF     "XXXX"      // 设备缺省VID
#define DEV_PID_DEF     "XXXX"      // 设备缺省VID

#define LOG_NAME         "DevImpl_CRT730B.log"   // 缺省日志名

#define STAND_CMD_LENGHT            (3)     // 标准命令长度

#define TIMEOUT_ACTION                                          (60*1000)
#define TIMEOUT_NO_ACTION                                       (10*1000)
#define TIMEOUT_WAIT_ACTION         (5*1000)
#define TIMEOUT_WRITEDATA                                       (3*1000)
#define CRT_PACK_MAX_LEN            (65)    // (加包头)一帧数据长度: ReportID(1Byte) + Data(64Byte) = 65
#define CRT_PACK_MAX_PAD_LEN									(63) //  64 - ReportID(1byte) = 63

#define CRT_PACK_MAX_CMP_LEN        (56)    // 一帧数据长度: 64 - STX(1byte)- LEN(2byte) -
                                            //              "CXX"(3byte) - CRC(2byte)
#define CRT_REPORT_ID											(0x04)  // one packet
#define CRT_MULTI_REPORT_ID										(0xFF)  // Multi packet
const DWORD USB_READ_LENTH = 64;

// 初始化方式
#define MODE_INIT_NORMAL         0      // 初始化:正常归位
#define MODE_INIT_EJECT          1      // 初始化:强行退卡
#define MODE_INIT_STORAGE        2      // 初始化:强行暂存
#define MODE_INIT_NOACTION       4      // 初始化:无动作

// 状态获取类别
#define MODE_STAT_ALL            0      // 获取状态:设备+暂存仓+维护门
#define MODE_STAT_DEV            1      // 获取状态:设备
#define MODE_STAT_SLOT           2      // 获取状态:暂存仓
#define MODE_STAT_DOOR           3      // 获取状态:维护门

// 暂存仓移动类别
#define MODE_SLOT_MOVEINIT       0      // 暂存仓移动:回到初始位置
#define MODE_SLOT_MOVEPOS        1      // 暂存仓移动:指定暂存仓对准卡口


// 设备状态(Impl处理使用)
#define IMPL_STAT_CRDSLOT_INIT           0    // 0x30,0x30:暂存仓已回到初始位置
#define IMPL_STAT_CRDSLOT_MOVE_TO_DOOR1  1    // 0x30,0x31:暂存仓1已对准进卡口
#define IMPL_STAT_CRDSLOT_MOVE_TO_DOOR2  2    // 0x30,0x32:暂存仓2已对准进卡口
#define IMPL_STAT_CRDSLOT_MOVE_TO_DOOR3  3    // 0x30,0x33:暂存仓3已对准进卡口
#define IMPL_STAT_CRDSLOT_MOVE_TO_DOOR4  4    // 0x30,0x34:暂存仓4已对准进卡口
#define IMPL_STAT_CRDSLOT_MOVE_TO_DOOR5  5    // 0x30,0x35:暂存仓5已对准进卡口
#define IMPL_STAT_CRDSLOT_POSITION_FAIL  10   // 0x31,0x00:暂存仓位置异常
#define IMPL_STAT_CRDSLOT_INVALID        -1   // 无效/未知状态

// 暂存仓状态(Impl处理使用)
#define IMPL_STAT_CRDSLOT_NOTHAVE       0    // 暂存仓内无卡
#define IMPL_STAT_CRDSLOT_ISHAVE        1    // 暂存仓内有卡
#define IMPL_STAT_CRDSLOT_INVALID       -1   // 暂存仓状态无效

// 维护门状态(Impl处理使用)
#define IMPL_STAT_SAFEDOOR_CLOSE        0    // 维护门关闭
#define IMPL_STAT_SAFEDOOR_OPEN         1    // 维护门开启
#define IMPL_STAT_SAFEDOOR_INVALID      -1   // 维护门状态无效

// 计算CRC使用
#define INIT        0x0000      // Initial value
#define POLINOMIAL  0x1021      // Polynomial X16+X12+X5+1

// 应答数据有效数据位置
#define RCVCMDPOS   3           // STX（1byte)+LEN(2byte):CMD(3byte)
#define RCVSTATPOS  6           // STX（1byte)+LEN(2byte)+CMD(3byte):STAT(2byte)
#define RCVINFOPOS  8           // STX（1byte)+LEN(2byte)+CMD(3byte)+STAT(2byte)
// 提取应答数据中有效信息
#define GETBUFF(BUFF, BUFFLEN, RCVDATA, DATALEN) \
    INT RCVDATASIZE = (DATALEN - RCVINFOPOS) > BUFFLEN ? BUFFLEN : (DATALEN - RCVINFOPOS); \
    memcpy(BUFF, RCVDATA + RCVINFOPOS, RCVDATASIZE);

/*************************************************************
// USB帧通信格式 设备请求/应答　宏定义
*************************************************************/
// 帧通信格式:报告ID(1Bytes)+报告数据(64Bytes)，每帧64+1
#define REPORTID    0x00   // 报告ID=0

// 报告数据格式:传输控制字符+正文长度+正文+循环冗余码,最多300Bytes
// 传输控制字符(1Bytes)
// 正文长度(2Bytes),LEN=高字节+低字节;CRCC
// 循环冗余码(2Bytes),CRCC=高字节+低字节,计算范围=传输控制字符+正文长度+正文

// 传输控制字符(1Bytes)
#define STX     0xF2    // 指示正文开始
#define ACK     0x06    // 确认
#define NAK     0x15    // 否认
#define DLE     0x10    // + EOT = 取消命令
#define EOT     0x04    // + DLE = 取消命令

// 正文TEXT格式
// 下发命令: C+cm+pm+Data
// 成功应答命令: P+cm+pm+st1+st0+Data
// 失败应答命令: N+cm+pm+e1+e0+Data
#define COM_SND     0x43    // "C": 主机下发给模块的命令格式
#define COM_RCV     0x43    // "P": 模块返回到主机下的成功应答格式
#define COM_RERR    0x4E    // "E": 模块返回到主机下的失败应答格式

// cm: 底层指令(命令码)
#define CM_0    0x30    // "0":初始化模块
#define CM_1    0x31    // "1":状态查询
#define CM_2    0x32    // "2":暂存仓移动
#define CM_3    0x33    // "3":同步暂存/退卡
#define CM_4    0x35    // "5":暂存仓信息
#define CM_R    0x52    // "R":模块信息
#define CM_C    0x43    // "C":系统计数值
#define CM_S    0x53    // "S":模块序列号
#define CM_K    0x4B    // "K":复位/跳转

// pm: 底层指令(参数)
#define PM_0    0x30    // "0"
#define PM_1    0x31    // "1"
#define PM_2    0x32    // "2"
#define PM_3    0x33    // "3"
#define PM_4    0x34    // "4"
#define PM_9    0x39    // "9"

// 组合下发命令
#define SND_CMD_INIT_NORMAL         "C00"   // 初始化:正常归位
#define SND_CMD_INIT_EJECT          "C01"   // 初始化:强行退卡
#define SND_CMD_INIT_STORAGE        "C02"   // 初始化:强行暂存
#define SND_CMD_INIT_NOACTION       "C04"   // 初始化:无动作
#define SND_CMD_STAT_DEVICE         "C10"   // 状态查询:模块
#define SND_CMD_STAT_DEVSLOT        "C11"   // 状态查询:模块+暂存仓
#define SND_CMD_STAT_DEVSLOTDOOR    "C14"   // 状态查询:模块+暂存仓+维护门
#define SND_CMD_SLOT_MOVEINIT       "C20"   // 暂存仓移动:回到初始位置
#define SND_CMD_SLOT_MOVEPOS        "C21"   // 暂存仓移动:指定暂存仓对准卡口
#define SND_CMD_CARD_STORAGE        "C30"   // 卡进入暂存仓
#define SND_CMD_CARD_EJECT          "C31"   // 卡退出暂存仓
#define SND_CMD_SLOT_WRITEINFO      "C50"   // 写暂存仓信息
#define SND_CMD_SLOT_READINFO       "C51"   // 读暂存仓信息
#define SND_CMD_SLOT_CLEARINFO      "C52"   // 清除指定暂存仓信息
#define SND_CMD_SLOT_CLEARINFOALL   "C53"   // 清除所有暂存仓信息
#define SND_CMD_SYSE_SOFTVER        "CR0"   // 获取软件版本
#define SND_CMD_SYSE_DEVCAPAB       "CR1"   // 获取设备能力值
#define SND_CMD_COUNT_STORAGE       "CC0"   // 获取存入计数
#define SND_CMD_COUNT_EJECT         "CC1"   // 获取退卡计数
#define SND_CMD_COUNT_CLEAR         "CC9"   // 清除所有计数
#define SND_CMD_GET_SERIALNUM       "CS0"   // 获取序列号
#define SND_CMD_DEV_RESET           "CK0"   // 设备复位
#define SND_CMD_DEV_UPDATA          "CK1"   // 进入固件升级模式


// st1,st0: 模块状态码
#define CRDSLOT_STAT_INIT           "00"    // 0x30,0x30:暂存仓已回到初始位置
#define CRDSLOT_STAT_MOVE_TO_DOOR1  "01"    // 0x30,0x31:暂存仓1已对准进卡口
#define CRDSLOT_STAT_MOVE_TO_DOOR2  "02"    // 0x30,0x32:暂存仓2已对准进卡口
#define CRDSLOT_STAT_MOVE_TO_DOOR3  "03"    // 0x30,0x33:暂存仓3已对准进卡口
#define CRDSLOT_STAT_MOVE_TO_DOOR4  "04"    // 0x30,0x34:暂存仓4已对准进卡口
#define CRDSLOT_STAT_MOVE_TO_DOOR5  "05"    // 0x30,0x35:暂存仓5已对准进卡口
#define CRDSLOT_STAT_POSITION_FAIL  "10"    // 0x31,0x00:暂存仓位置异常

// 暂存仓有卡/无卡
#define CRDSLOT_STAT_CARD_NOTHAVE   0x00    // 暂存仓内无卡
#define CRDSLOT_STAT_CARD_ISHAVE    0x01    // 暂存仓内有卡

// 维护门开启/关闭
#define SAFEDOOR_STAT_CLOSE     0x00        // 维护门关闭
#define SAFEDOOR_STAT_OPEN      0x01        // 维护门开启

// 错误码
#define DEV_ERR_CMD_INVALID         "00"        // 0x30,0x30:命令编码未定义
#define DEV_ERR_CMD_PARAM           "01"        // 0x30,0x31:命令参数错误
#define DEV_ERR_CMD_DATA            "02"        // 0x30,0x32:命令数据错误
#define DEV_ERR_CMD_UNABLE          "03"        // 0x30,0x33:命令无法执行
#define DEV_ERR_DOOR_ISOPEN         "06"        // 0x30,0x36:维护门已经开启（此时不能发送动作指令!）
#define DEV_ERR_POWER_VOLLOW        "11"        // 0x31,0x31:电源电压过低(低于20V)
#define DEV_ERR_POWER_VOLHIGH       "12"        // 0x31,0x32:电源电压过高(高于28V)
#define DEV_ERR_POWER_NEEDINIT      "13"        // 0x31,0x33:电源恢复正常但没有重新初始化
#define DEV_ERR_POWER_FRAM          "16"        // 0x31,0x36:模块内部FRAM错误
#define DEV_ERR_SOLT_SENSER         "19"        // 0x31,0x39:暂存仓传感器错误
#define DEV_ERR_SLOT_JAM            "20"        // 0x32,0x30:升降暂存仓阻塞
#define DEV_ERR_INCARD_JAM          "22"        // 0x32,0x32:进卡阻塞
#define DEV_ERR_INCARD_TIMEOUT      "23"        // 0x32,0x33:等待进卡超时
#define DEV_ERR_INCARD_TAKE         "24"        // 0x32,0x34:进卡时卡被拔走
#define DEV_ERR_EJECT_JAM           "28"        // 0x32,0x38:退卡堵塞
#define DEV_ERR_POWER_LOCAT_JAM     "29"        // 0x32,0x39:电机定位堵塞
#define DEV_ERR_SLOT_NOTAIM_CK      "30"        // 0x33,0x30:暂存仓未对准进卡口
#define DEV_ERR_SLOT_HAVECARD       "31"        // 0x33,0x31:目标暂存仓有卡
#define DEV_ERR_SLOT_NOTHAVE        "32"        // 0x33,0x32:目标暂存仓无卡
#define DEV_ERR_CINFO_NOSAVE        "35"        // 0x33,0x35:卡片信息未保存


/*************************************************************
// 封装类: 命令编辑、发送接收等处理。
*************************************************************/
class CDevImpl_CRT730B : public CLogManage
{
public:
    CDevImpl_CRT730B();
    CDevImpl_CRT730B(LPSTR lpLog);
    virtual ~CDevImpl_CRT730B();

public:
    INT     DeviceOpen(LPSTR lpMode);   // 打开设备
    INT     DeviceClose();              // 关闭设备
    INT     Release();                  // 释放动态库
    BOOL    IsDeviceOpen();             //
    INT     ConvertErrorCode(long lRet);// 转换为Impl返回码/错误码
    INT     ConvertErrorCode(CHAR szDeviceErr[3]);
    CHAR*   ConvertErrCodeToStr(long lRet);
    CHAR*   CmdToStr(LPSTR lpCmd);
    INT     RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT &nRcvDataLen);
    INT     RcvDeviceStatChg(LPCSTR lpDeviceStat);
    INT     RcvCardSlotStatChg(LPCSTR lpStat, INT nSlotNo);
    INT     RcvDoorStatChg(LPCSTR lpStat);

public: // 接口函数封装
    INT     DeviceInit(WORD wInitMode);                             // 1. 模块初始化
    INT     GetDeviceStat(WORD wMode, INT nStat[12]);               // 2. 获取设备状态
    INT     CardSlotMove(WORD wMode, WORD wSlotNo);                 // 3. 暂存仓移动
    INT     CardStorage();                                          // 4. 卡存入暂存仓
    INT     CardEject();                                            // 5. 卡退出暂存仓
    INT     WriteCardSlotInfo(WORD wSlotNo, LPSTR lpInfo);          // 6. 写指定暂存仓信息
    INT     ReadCardSlotInfo(WORD wSlotNo, LPSTR lpInfo, WORD wInfoSize);// 7. 读指定暂存仓信息
    INT     ClearCardSoltInfo(WORD wSlotNo = 0);                    // 8. 清除暂存仓信息
    INT     GetSoftVersion(LPSTR lpVersion);                        // 9. 获取软件版本信息
    INT     GetDeviceCapab(INT nDevCap[9]);                         // 10. 获取设备能力信息
    INT     GetCardStorageCount(INT &nCount);                       // 11. 获取存卡计数
    INT     GetCardEjectCount(INT &nCount);                         // 12. 获取退卡计数
    INT     ClearCardSoltCount();                                   // 13. 清除所有计数
    INT     GetDeviceSerialNumber(LPSTR lpSerialNum);               // 14. 获取设备序列号
    INT     DeviceReset();                                          // 15. 设备复位
    INT     DeviceUpdate();                                         // 16. 进入固件升级模式

private:
    string                          m_strMode;
    CQtDLLLoader<IAllDevPort>       m_pDev;     // USB接口处理
    BOOL                            m_bDevOpenOk;

private:
    void Init();

private: // USB命令收发
    INT SendCmd(const char *pszCmd, const char *lpData, int nLen);     // 主下发数据
    INT GetResponse(char *pszResponse, int nLen, int nTimeout, INT &dwOutLen);        // 主接收数据
    INT SendSinglePacket(const char* pszCmd, const char *lpData, int nLen);    // 下发单帧数据
    INT SendMultiPacket(const char *pszCmd, const char *lpData, int nLen);     // 下发多帧数据
    INT SendSinglePacket0(const char* pszCmd, const char *lpData, int nLen);   // 下发单帧数据(含包头)
    INT SendMultiPacket0(const char *pszCmd, const char *lpData, int nLen);    // 下发多帧数据(含包头)
    USHORT  GetDataCRC(UCHAR *ucData, USHORT usDataSize, USHORT usInitCrc = INIT);
    USHORT  Calc_CRC(USHORT usCrc, USHORT usCh);

private: // 接口加载
    CSimpleMutex                    m_MutexAction;
    INT                             m_nLastError;
    CHAR                            m_szCmdStr[1024];
    CHAR                            m_szErrStr[1024];
};


#endif // DEVIMPL_CRT730B_H
