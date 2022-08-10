/***************************************************************
* 文件名称：DevImpl_CRT350N.h
* 文件描述：封装读卡器模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEVIMPL_CRT350N_H
#define DEVIMPL_CRT350N_H


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
#define IMP_SUCCESS                 0               // 成功
#define IMP_ERR_LOAD_LIB            -9901           // 动态库加载失败
#define IMP_ERR_PARAM_INVALID       -9902           // 参数无效
#define IMP_ERR_READERROR           -9903           // 读数据错误
#define IMP_ERR_WRITEERROR          -9904           // 写数据错误
#define IMP_ERR_RCVDATA_INVALID     -9905           // 无效的应答数据
#define IMP_ERR_RCVDATA_NOTCOMP     -9906           // 返回数据不完整
#define IMP_ERR_UNKNOWN             -9909           // 未知错误

// <0 : USB/COM接口处理返回
#define IMP_ERR_DEVPORT_NOTOPEN     ERR_DEVPORT_NOTOPEN      // (-1) 没打开
#define IMP_ERR_DEVPORT_FAIL        ERR_DEVPORT_FAIL         // (-2) 通讯错误
#define IMP_ERR_DEVPORT_PARAM       ERR_DEVPORT_PARAM        // (-3) 参数错误
#define IMP_ERR_DEVPORT_CANCELED    ERR_DEVPORT_CANCELED     // (-4) 操作取消
#define IMP_ERR_DEVPORT_READERR     ERR_DEVPORT_READERR      // (-5) 读取错误
#define IMP_ERR_DEVPORT_WRITE       ERR_DEVPORT_WRITE        // (-6) 发送错误
#define IMP_ERR_DEVPORT_RTIMEOUT    ERR_DEVPORT_RTIMEOUT     // (-7) 操作超时
#define IMP_ERR_DEVPORT_WTIMEOUT    ERR_DEVPORT_WTIMEOUT     // (-8) 操作超时
#define IMP_ERR_DEVPORT_LIBRARY     ERR_DEVPORT_LIBRARY      // (-98) 加载通讯库失败
#define IMP_ERR_DEVPORT_NODEFINED   ERR_DEVPORT_NODEFINED    // (-99) 未知错误

// 0~100: 硬件设备返回
// CRT350N设备返回报文为2位字符串, 转换为INT
// 返回值有0, 与SP通用冲突, 每一个错误码+1以区分
#define IMP_ERR_DEVRET_00           0 + 1           // 命令编码未定义
#define IMP_ERR_DEVRET_01           1 + 1           // 命令参数错误
#define IMP_ERR_DEVRET_02           2 + 1           // 命令无法执行
#define IMP_ERR_DEVRET_03           3 + 1           // 硬件不支持
#define IMP_ERR_DEVRET_04           4 + 1           // 命令数据错误
#define IMP_ERR_DEVRET_05           5 + 1           // IC触点未释放
#define IMP_ERR_DEVRET_06           6 + 1           // 密钥不存在
#define IMP_ERR_DEVRET_07           7 + 1           //
#define IMP_ERR_DEVRET_08           8 + 1           //
#define IMP_ERR_DEVRET_09           9 + 1           //
#define IMP_ERR_DEVRET_10           10 + 1          // 卡堵塞
#define IMP_ERR_DEVRET_11           11 + 1          // Shutter错误
#define IMP_ERR_DEVRET_12           12 + 1          // 传感器错误
#define IMP_ERR_DEVRET_13           13 + 1          // 不规则卡长度(过长)
#define IMP_ERR_DEVRET_14           14 + 1          // 不规则卡长度(过短)
#define IMP_ERR_DEVRET_15           15 + 1          // FRAM错误
#define IMP_ERR_DEVRET_16           16 + 1          // 卡位置移动
#define IMP_ERR_DEVRET_17           17 + 1          // 重进卡时卡堵塞
#define IMP_ERR_DEVRET_18           18 + 1          // SW1,SW2错误
#define IMP_ERR_DEVRET_19           19 + 1          // 卡没有从后端插入
#define IMP_ERR_DEVRET_20           20 + 1          // 读磁卡错误(奇偶校验错)
#define IMP_ERR_DEVRET_21           21 + 1          // 读磁卡错误
#define IMP_ERR_DEVRET_22           22 + 1          // 写磁卡错误
#define IMP_ERR_DEVRET_23           23 + 1          // 读磁卡错误(没有数据内容,只有STX起始符,ETX结束符和LRC)
#define IMP_ERR_DEVRET_24           24 + 1          // 读磁卡错误(没有磁条或没有编码-空白轨道)
#define IMP_ERR_DEVRET_25           25 + 1          // 写磁卡校验错误(品质错误)
#define IMP_ERR_DEVRET_26           26 + 1          // 读磁卡错误(没有SS)
#define IMP_ERR_DEVRET_27           27 + 1          // 读磁卡错误(没有ES)
#define IMP_ERR_DEVRET_28           28 + 1          // 读磁卡错误(LRC错误)
#define IMP_ERR_DEVRET_29           29 + 1          // 写磁卡校验错误(数据不一致)
#define IMP_ERR_DEVRET_30           30 + 1          // 电源掉电
#define IMP_ERR_DEVRET_31           31 + 1          // DSR信号为OFF
#define IMP_ERR_DEVRET_32           32 + 1          //
#define IMP_ERR_DEVRET_40           40 + 1          // 吞卡时卡拔走
#define IMP_ERR_DEVRET_41           41 + 1          // IC触点或触点传感器错误
#define IMP_ERR_DEVRET_42           42 + 1          //
#define IMP_ERR_DEVRET_43           43 + 1          // 无法走到IC卡位
#define IMP_ERR_DEVRET_44           44 + 1          //
#define IMP_ERR_DEVRET_45           45 + 1          // 卡机强制弹卡
#define IMP_ERR_DEVRET_46           46 + 1          // 前端卡未在指定时间内取走
#define IMP_ERR_DEVRET_47           47 + 1          //
#define IMP_ERR_DEVRET_48           48 + 1          //
#define IMP_ERR_DEVRET_49           49 + 1          //
#define IMP_ERR_DEVRET_50           50 + 1          // 回收卡计数溢出
#define IMP_ERR_DEVRET_51           51 + 1          // 马达错误
#define IMP_ERR_DEVRET_52           52 + 1          //
#define IMP_ERR_DEVRET_53           53 + 1          // 数字解码读错误
#define IMP_ERR_DEVRET_54           54 + 1          // 防盗钩移动错误
#define IMP_ERR_DEVRET_55           55 + 1          // 防盗钩已经设置,命令不能执行
#define IMP_ERR_DEVRET_56           56 + 1          // 芯片检测传感器错误
#define IMP_ERR_DEVRET_57           57 + 1          //
#define IMP_ERR_DEVRET_58           58 + 1          // 防盗钩正在移动
#define IMP_ERR_DEVRET_59           59 + 1          //
#define IMP_ERR_DEVRET_60           60 + 1          // IC卡或SAM卡Vcc条件异常
#define IMP_ERR_DEVRET_61           61 + 1          // IC卡或SAM卡ATR通讯错误
#define IMP_ERR_DEVRET_62           62 + 1          // IC卡或SAM卡在当前激活条件下ATR无效
#define IMP_ERR_DEVRET_63           63 + 1          // IC卡或SAM卡通讯过程中无响应
#define IMP_ERR_DEVRET_64           64 + 1          // IC卡或SAM卡通讯错误(除无响应外)
#define IMP_ERR_DEVRET_65           65 + 1          // IC卡或SAM卡未激活
#define IMP_ERR_DEVRET_66           66 + 1          // IC卡或SAM卡不支持(仅对于非EMV激活)
#define IMP_ERR_DEVRET_67           67 + 1          //
#define IMP_ERR_DEVRET_68           68 + 1          //
#define IMP_ERR_DEVRET_69           69 + 1          // IC卡或SAM卡不支持(仅对于EMV激活)
#define IMP_ERR_DEVRET_76           76 + 1          // ESU模块和卡机通讯错误
#define IMP_ERR_DEVRET_95           95 + 1          // ESU模块损坏或无连接
#define IMP_ERR_DEVRET_99           99 + 1          // ESU模块过流
#define IMP_ERR_DEVRET_B0           0xB0 + 1        // 未接收到初始化命令

/*************************************************************
// 无分类　宏定义
*************************************************************/
#define DEV_VID_DEF                     "XXXX"  // 设备缺省VID
#define DEV_PID_DEF                     "XXXX"  // 设备缺省VID

#define LOG_NAME                        "DevImpl_CRT730B.log"   // 缺省日志名

#define STAND_CMD_LENGHT                3       // 标准命令长度
#define RCV_DATA_POS                    8       // 返回应答的数据位置
#define RCV_CMD_START_POS               3       // 返回应答的命令起始标记位置
#define RCV_CMD_POS                     4       // 返回应答的命令位置
#define RCV_STAT_POS                    6       // 返回应答的状态位置

// 超时时间定义, 单位:毫秒
#define TIMEOUT_ACTION                  60*1000
#define TIMEOUT_NO_ACTION               10*1000
#define TIMEOUT_WAIT_ACTION             5*1000
#define TIMEOUT_WRITEDATA               3*1000

// 数据包/帧长度定义
#define CRT_PACK_MAX_LEN				64      // 一帧(一次)发包长度上限(含包头)
#define CRT_PACK_MAX_PAD_LEN			63      // 一帧(一次)发包长度上限(不含包头)
#define CRT_PACK_MAX_CMP_LEN            58      // 一帧(一次)发包数据长度上限
                                                // 64 - ReportID(1byte) - LEN(2byte) - "CXX"(3byte) = 58
#define CRT_REPORT_ID					0x04    // 一帧数据包头
#define CRT_MULTI_REPORT_ID				0xFF    // 多帧数据包头
const DWORD USB_READ_LENTH = 64;

// 状态获取类别
#define GET_STAT_ALL                    0       // 获取状态:设备+卡
#define GET_STAT_DEV                    1       // 获取状态:设备
#define GET_STAT_CARD                   2       // 获取状态:卡

// 卡状态(Impl处理使用)
#define IMPL_STAT_CARD_NOTHAVE          0       // 无卡
#define IMPL_STAT_CARD_ISEXPORT         1       // 卡在出口
#define IMPL_STAT_CARD_ISINSIDE         2       // 卡在通道内
#define IMPL_STAT_CARD_ISAFTER_EXPORT   3       // 卡在后出口
#define IMPL_STAT_CARD_INVALID          -1      // 卡状态无效

// IC触点状态
#define IMPL_STAT_MEET_UP               0       // 释放
#define IMPL_STAT_MEET_DOWN             1       // 压下

// 出口闸门状态
#define IMPL_STAT_SHUT_CLOSE            0       // 关闭
#define IMPL_STAT_SHUT_OPEN             1       // 开启

// 是否有芯片
#define IMPL_STAT_CHIPNOTHAVE           0       // 没有芯片
#define IMPL_STAT_CHIPISHAVE            1       // 有芯片

// 读磁方式
#define IMPL_TRACK1                     1       // 1磁道
#define IMPL_TRACK2                     2       // 2磁道
#define IMPL_TRACK3                     4       // 3磁道

// 防盗钩状态
#define IMPL_TAMPER_UNKNOWN             0x00    // 未知状态
#define IMPL_TAMPER_PRESS               0x01    // 防盗钩下压
#define IMPL_TAMPER_RELEASE             0x02    // 防盗钩释放
#define IMPL_TAMPER_PRESS_NORUN         0x04    // 防盗钩下压命令没有执行
#define IMPL_TAMPER_RELEASE_NORUN       0x08    // 防盗钩释放命令没有执行

/*************************************************************
// 枚举 定义
*************************************************************/
// 初始化方式
enum EN_DEV_INIT
{
    INIT_EJECT              = 0,    // 初始化:有卡时弹到前端
    INIT_RETAIN             = 1,    // 初始化:有卡时吞卡
    INIT_REACCEPT           = 2,    // 初始化:有卡时重进卡
    INIT_NOACTION           = 3,    // 初始化:有卡时不移动卡
};

// 进卡方式参数
enum EN_INCARD_PAR
{
    INCARD_NOT              = 0,    // 禁止进卡
    INCARD_ALL              = 1,    // 进卡(所有卡不检查)
    INCARD_MAG              = 2,    // 只允许磁卡
    INCARD_IC               = 3,    // 只允许芯片卡
    INCARD_MAG2IC           = 4,    // 允许进磁卡和芯片卡
    INCARD_MAG3IC           = 5,    // 允许进磁卡或芯片卡
    INCARD_SHAKE            = 6,    // 抖动进卡
};

// 读磁方式


// 磁道操作
enum EN_MAG_OPATION
{
    MAG_NULL                = 0,    // 空值
    MAG_READ_TRACK1         = 1,    // 读1磁道
    MAG_READ_TRACK2         = 2,    // 读2磁道
    MAG_READ_TRACK3         = 4,    // 读3磁道
};

// 芯片操作参数
enum EN_CHIP_OPATION
{
    CHIP_PRESS              = 0,    // 芯片接触/压下
    CHIP_RELEASE            = 1,    // 芯片释放
    CHIP_ACTIVE             = 2,    // 芯片激活/冷复位
    CHIP_DEACTIVE           = 3,    // 芯片关闭
    CHIP_WARM               = 4,    // 芯片热复位
    CHIP_COMMT0             = 5,    // 芯片T0通信
    CHIP_COMMT1             = 6,    // 芯片T1通信
    CHIP_COMMAUTO           = 7,    // 芯片通信(自动选择)
};

// 介质控制参数
enum EN_CARD_OPATION
{
    CARD_EJECT              = 0,    // 介质退出
    CARD_RETAIN             = 1,    // 介质回收
    CARD_MOVE               = 2,    // 介质移动
};

// 防盗钩控制参数
enum EN_TAMPER_OPATION
{
    TAM_SETSTAND            = 0,    // 设置为标准模式
    TAM_SETAUTO             = 1,    // 设置为自动模式
    TAM_SETPRESS            = 2,    // 设置压下
    TAM_SETRELEASE          = 3,    // 设置释放
};

/*************************************************************
// USB帧通信格式 设备请求/应答　宏定义
*************************************************************/
// 帧通信格式:报告ID(1Bytes)+报告数据(63Bytes)，报告数据<=1024Byte
// 通信: 每帧64Byte, 超过64Byte分多个帧传输
#define REPORTID            0x04        // 报告ID=4

// 报告数据格式:传输控制字符+正文长度+正文+循环冗余码,最多300Bytes
// 传输控制字符(1Bytes)
// 正文长度(2Bytes),LEN=高字节+低字节;CRCC
// 循环冗余码(2Bytes),CRCC=高字节+低字节,计算范围=传输控制字符+正文长度+正文

// 传输控制字符(1Bytes)
#define STX                         0xF2        // 指示正文开始
#define ACK                         0x06        // 确认
#define NAK                         0x15        // 否认
#define DLE                         0x10        // + EOT = 取消命令
#define EOT                         0x04        // + DLE = 取消命令

// 正文TEXT格式
// 下发命令: C+cm+pm+Data
// 成功应答命令: P+cm+pm+st1+st0+Data
// 失败应答命令: N+cm+pm+e1+e0+Data
#define COM_SND                     0x43        // "C": 主机下发给模块的命令格式
#define COM_RCV                     0x43        // "P": 模块返回到主机下的成功应答格式
#define COM_RERR                    0x4E        // "E": 模块返回到主机下的失败应答格式

// cm: 底层指令(命令码)
#define CM_30                       0x30        // "0":初始化模块
#define CM_31                       0x31        // "1":查询状态
#define CM_32                       0x32        // "2":同步进卡
#define CM_33                       0x33        // "3":卡移动
#define CM_34                       0x34        // "4":重进卡
#define CM_36                       0x36        // "6":读磁卡
#define CM_37                       0x37        // "7":写磁卡
#define CM_3A                       0x3A        // ":":异步进卡控制
#define CM_3D                       0x3D        // "=":端口输入输出
#define CM_3E                       0x3E        // ">":传感器电压
#define CM_40                       0x40        // "@":IC卡/RF卡走位
#define CM_41                       0x41        // "A":版本号
#define CM_43                       0x43        // "C":回收计数
#define CM_44                       0x44        // "D":数字解码读磁卡
#define CM_49                       0x49        // "I":IC卡/SAM卡控制
#define CM_4B                       0x4B        // "K":切换
#define CM_52                       0x52        // "R":存储卡控制
#define CM_54                       0x54        // "T":监控取卡
#define CM_55                       0x55        // "U":I2C接口存储卡控制
#define CM_56                       0x56        // "V":多磁道读
#define CM_66                       0x66        // "f":日志
#define CM_4D                       0x4D        // "M":AT88SC102卡控制
#define CM_70                       0x70        // "p":ESU(磁干扰)
#define CM_63                       0x63        // "c":防盗钩
#define CM_5A                       0x5A        // "Z":非接触IC卡控制

// pm: 底层指令(参数)
#define PM_0                        0x30        // "0"
#define PM_1                        0x31        // "1"
#define PM_2                        0x32        // "2"
#define PM_3                        0x33        // "3"
#define PM_4                        0x34        // "4"
#define PM_9                        0x39        // "9"

// 组合下发命令
// 初始化
#define SND_CMD_INIT_C00            "C00"       // 有卡时弹到前端
#define SND_CMD_INIT_C01            "C01"       // 有卡时吞卡
#define SND_CMD_INIT_C02            "C02"       // 有卡时重进卡
#define SND_CMD_INIT_C03            "C03"       // 有卡时不移动卡
#define SND_CMD_INIT_C04            "C04"       // 同C00，但回收计数器工作
#define SND_CMD_INIT_C05            "C05"       // 同C01，但回收计数器工作
#define SND_CMD_INIT_C06            "C06"       // 同C02，但回收计数器工作
#define SND_CMD_INIT_C07            "C07"       // 同C03，但回收计数器工作
// 状态查询
#define SND_CMD_STAT_C10            "C10"       // 报告是否有卡和卡的位置
#define SND_CMD_STAT_C11            "C11"       // 报告传感器状态
#define SND_CMD_STAT_C12            "C12"       // 报告传感器状态(扩展)
#define SND_CMD_STAT_C13            "C13"       // 报告传感器状态(扩展)
#define SND_CMD_STAT_C14            "C14"       // 报告传感器状态(扩展)
#define SND_CMD_STAT_C1H40          "C1@"       // 报告传感器状态(扩展)
// 同步进卡
#define SND_CMD_SYNC_INCARD_C20     "C20"       // 前端同步进卡:不检测磁信号
#define SND_CMD_SYNC_INCARD_C21     "C21"       // 前端同步进卡:检测ISO磁道2,磁道3磁信号
#define SND_CMD_SYNC_INCARD_C22     "C22"       // 后端进卡:后端同步等待卡插入
#define SND_CMD_SYNC_INCARD_C23     "C23"       // 前端同步进IC卡:等待进IC卡
#define SND_CMD_SYNC_INCARD_C24     "C24"       // 前端同步进IC卡&磁卡:等待进IC卡&磁卡
#define SND_CMD_SYNC_INCARD_C2H3D   "C2="       // 前端同步进IC卡或磁卡:等待进IC卡或磁卡
// 卡移动
#define SND_CMD_CARD_MOVE_C30       "C30"       // 弹卡:将卡移到前端持卡位
#define SND_CMD_CARD_MOVE_C31       "C31"       // 吞卡:将卡从读卡器后部回收
#define SND_CMD_CARD_MOVE_C32       "C32"       // 设置MM:将卡移到MM起始位
#define SND_CMD_CARD_MOVE_C37       "C37"       // 退卡:将卡移到前端不持卡位
// 重进卡
#define SND_CMD_RE_INCARD_C40       "C40"       // 将卡从前端移动到卡内部

// 读磁卡
#define SND_CMD_READ_CARD_C60       "C60"       // 移动卡:移动读卡
#define SND_CMD_READ_CARD_C61       "C61"       // 读ISO磁道#1:传输ISO磁道#1数据
#define SND_CMD_READ_CARD_C62       "C62"       // 读ISO磁道#2:传输ISO磁道#2数据
#define SND_CMD_READ_CARD_C63       "C63"       // 读ISO磁道#3:传输ISO磁道#3数据
#define SND_CMD_READ_CARD_C65       "C65"       // 数据读所有磁道:传输所有磁道数据
#define SND_CMD_READ_CARD_C66       "C66"       // 清除磁卡缓存:清除缓存的读写磁卡数据
#define SND_CMD_READ_CARD_C67       "C67"       // 读磁道状态:磁道数据缓冲区状态
#define SND_CMD_READ_CARD_C69       "C69"       // ISO磁道#1其他方式读:传输ISO磁道#1数据
#define SND_CMD_READ_CARD_C6H3A     "C6:"       // ISO磁道#2其他方式读:传输ISO磁道#2数据
#define SND_CMD_READ_CARD_C6H3B     "C6;"       // ISO磁道#3其他方式读:传输ISO磁道#3数据

// 写磁卡


// 异步进卡控制
#define SND_CMD_ASYNC_INCARD_CH3A0  "C:0"       // 使能进卡,不检测磁信号(所有卡)
#define SND_CMD_ASYNC_INCARD_CH3A1  "C:1"       // 禁止进卡
#define SND_CMD_ASYNC_INCARD_CH3A2  "C:2"       // 使能进卡,检测磁信号(只允许磁卡)
#define SND_CMD_ASYNC_INCARD_CH3A3  "C:3"       // 使能进卡,检测IC芯片
#define SND_CMD_ASYNC_INCARD_CH3A4  "C:4"       // 使能进卡,检查IC芯片和磁信号
#define SND_CMD_ASYNC_INCARD_CH3AD  "C:="       // 使能进卡,检查IC芯片或磁信号
#define SND_CMD_ASYNC_INCARD_CH3AX  "C:X"       // 抖动进卡设置
// 端口输入输出
// 传感器电压
// IC卡/RF卡走位
#define SND_CMD_CHIP_PRESS          "C@0"       // 走位/触点接触
#define SND_CMD_CHIP_RELEASE        "C@2"       // 触点释放
// 版本号
#define SND_CMD_READ_VER_CA1        "CA1"       // 读版本号:读取应用程序版本号
#define SND_CMD_READ_VER_CA2        "CA2"       // 读版本号:读取EMV2000版本
#define SND_CMD_READ_VER_CA3        "CA3"       // 读版本号:读取EMV证书编号
#define SND_CMD_READ_VER_CA4        "CA4"       // 读版本号:读取GIE-CB 证书编号
#define SND_CMD_READ_VER_CA5        "CA5"       // 读版本号:读取IFM编号
#define SND_CMD_READ_VER_CA6        "CA6"       // 读版本号:读取ICC控制器版本

// 回收计数
#define SND_CMD_GET_RETAINCNT       "CC0"       // 读取回收计数
#define SND_CMD_SET_RETAINCNT       "CC1"       // 设置回收计数
// 数字解码读磁卡
// IC卡/SAM卡控制
#define SND_CMD_CHIP_ACTION_CI0     "CI0"       // 芯片卡激活
#define SND_CMD_CHIP_DEACTION_CI1   "CI1"       // 芯片卡关闭
#define SND_CMD_CHIP_GETSTAT_CI2    "CI2"       // 芯片卡状态查询
#define SND_CMD_CHIP_COMMT0_CI3     "CI3"       // 芯片卡T0通讯
#define SND_CMD_CHIP_COMMT1_CI4     "CI4"       // 芯片卡T1通讯
#define SND_CMD_CHIP_COMMKZ1_CI5    "CI5"       // 芯片卡通讯扩展1(发送命令1000Byte及以下)
#define SND_CMD_CHIP_COMMKZ2_CI6    "CI6"       // 芯片卡通讯扩展2(接收应答1000Byte及以下)
#define SND_CMD_CHIP_COMMKZ3_CI7    "CI7"       // 芯片卡通讯扩展3(接收应答1000Byte以上)
#define SND_CMD_CHIP_RESETWARM_CI8  "CI8"       // 芯片卡热复位
#define SND_CMD_CHIP_COMMAUTO_CI9   "CI9"       // 芯片卡自动通讯
// 切换
// 存储卡控制
// 监控取卡
// I2C接口存储卡控制
// 多磁道读
// 日志
// AT88SC102卡控制
// ESU(磁干扰)
// 防盗钩
#define SND_CMD_TAMPER_GETSTAT_Cc8  "Cc8"       // 获取防盗钩状态
#define SND_CMD_TAMPER_SETWORK_Cc9  "Cc9"       // 设置防盗钩工作模式
#define SND_CMD_TAMPER_CONT_CcH3A   "Cc:"       // 控制防盗钩动作

// 非接触IC卡控制



// st1,st0: 读卡器状态码
#define CARDER_STAT_NOCARD          "00"    // 0x30,0x30:读卡器内无卡
#define CARDER_STAT_CARD_IS_EXPORT  "01"    // 0x30,0x31:卡在出口
#define CARDER_STAT_CARD_IS_INSIDE  "02"    // 0x30,0x32:卡在内部


/*************************************************************
// 封装类: 命令编辑、发送接收等处理。
*************************************************************/
class CDevImpl_CRT350N : public CLogManage
{
public:
    CDevImpl_CRT350N();
    CDevImpl_CRT350N(LPSTR lpLog);
    CDevImpl_CRT350N(LPSTR lpLog, LPCSTR lpDevType);
    virtual ~CDevImpl_CRT350N();

public:
    INT     OpenDevice(LPSTR lpMode, LPCSTR lpcInit = nullptr,
                       INT nInitLen = 0);                           // 打开设备
    INT     CloseDevice();                                          // 关闭设备
    INT     Release();                                              // 释放动态库
    BOOL    IsDeviceOpen();                                         // 设备是否Open
    CHAR*   CmdToStr(LPSTR lpCmd);                                  // 命令转换为解释字符串
    INT     ConvertCode_USB2Impl(long lRet);                        // USB处理错误值转换为Impl返回码/错误码
    INT     ConvertCode_Dev2Impl(CHAR szDeviceErr[3]);              // 硬件错误值转换为Impl返回码/错误码
    LPSTR   ConvertCode_Impl2Str(INT nErrCode);                     // Impl错误码转换解释字符串

public: // 接口函数封装
    INT     DeviceInit(EN_DEV_INIT enInit, LPSTR lpParam, WORD wParLen,
                       BOOL bIsRetainEna);                          // 1. 模块初始化
    INT     GetDeviceStat(WORD wMode, INT nStat[12]);               // 2. 获取设备状态
    INT     SetDevRetainCnt(WORD wSize = 0);                        // 3. 设置回收计数
    INT     GetVersion(WORD wType, LPSTR lpVerStr, WORD wVerSize);  // 4. 取版本
    INT     CardAcceptASync(EN_INCARD_PAR enInCard, INT nParam = 0);// 5. 异步进卡
    INT     MagneticRead(DWORD dwMag, LPSTR lpData, INT nSize);     // 6. 读磁处理
    INT     MagneticRead();                                         // 7. 磁道处理
    INT     ChipOperation(EN_CHIP_OPATION enAtion, LPSTR lpRcvData = nullptr,
                          INT *nRcvDataSize = nullptr);             // 8. 芯片操作
    INT     ChipProtocol(EN_CHIP_OPATION enAtion, LPSTR lpSnd, WORD wSndSize,
                         LPSTR lpRcv, DWORD &dwRcvSize);            // 9. 通信操作
    INT     MediaOperation(EN_CARD_OPATION enAtion, WORD wParam = 0);// 10. 介质控制
    INT     GetTamperStat(INT &nStat);                              // 11. 获取防盗钩状态
    INT     SetTamperOperAtion(EN_TAMPER_OPATION enAtion);          // 12. 设置防盗钩动作
    INT     CardReAccept();                                         // 13. 重进卡

private:    
    CQtDLLLoader<IAllDevPort>       m_pDev;                         // USB接口处理
    string          m_strMode;                                      // 连接USB串
    BOOL            m_bDevOpenOk;                                   // 设备是否Open
    BOOL            m_bReCon;                                       // 是否断线重连状态
    WORD            m_wPredictIC;                                   // 进卡检查模式
    CHAR            m_szDevType[64];                                // 设备类型
    CHAR            m_szErrStr[1024];                               // IMPL错误解释
    CHAR            m_szCmdStr[256];                                // 命令解释
    DWORD           m_dwSndTimeOut;                                 // 命令下发超时时间(毫秒)
    DWORD           m_dwRcvTimeOut;                                 // 命令接收超时时间(毫秒)
    INT             m_nRetErrOLD[8];                                // 处理错误值保存(0:USB动态库/1:设备连接/
                                                                    //  2:设备初始化/3/4)
    CHAR            m_szResetParam[11 + 1];                         // 复位辅助参数

private:
    void Init();

private:    // USB命令收发
    INT SendCmd(LPCSTR lpcCmd, LPCSTR lpcData, INT nLen, LPCSTR lpFuncData,
                BOOL bIsPrtLog = TRUE);                             // 主下发数据
    INT GetResponse(LPSTR lpResponse, INT nLen, LPCSTR lpFuncData, BOOL bIsPrtLog = TRUE);// 主接收数据
    INT SendSinglePacket(LPCSTR lpcCmd, LPCSTR lpcCmdPar, INT nParLen,
                         LPCSTR lpFuncData, BOOL bIsPrtLog = TRUE); // 下发单帧数据
    INT SendMultiPacket(LPCSTR lpcCmd, LPCSTR lpcCmdPar, INT nParLen,
                        LPCSTR lpFuncData, BOOL bIsPrtLog = TRUE);  // 下发多帧数据

private:    // USB命令应答数据处理
    INT SndRcvToChk(LPCSTR lpcSndCmd, LPCSTR lpcSndPar, INT nParSize,
                    LPSTR lpRcvData, INT &nRcvSize, LPCSTR lpPrtData = nullptr,
                    BOOL bIsPrtLog = TRUE);                         // 命令收发及检查
    INT RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT nRcvDataLen,
                     LPCSTR lpPrtData = nullptr, BOOL bIsPrtLog = TRUE);// 应答数据Check
    INT RcvCardStatChg(LPCSTR lpStat);                              // 应答数据状态转换

public:    // DevIDC.SetData相关
    INT SetReConFlag(BOOL bFlag);                                   // 设置断线重连标记
    INT SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)
    INT SetPredictIC(WORD wPreIC);                                  // 设置进卡检查模式
    INT SetSndRcvTimeOut(DWORD dwSnd, DWORD dwRcv);                 // 设置命令收发超时时间

private: // 接口加载
    CSimpleMutex                    m_MutexAction;
    INT                             m_nLastError;

};


#endif // DEVIMPL_CRT350N_H
