/***************************************************************
* 文件名称：DevImpl_ACTU6SS39.h
* 文件描述：封装退卡模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEVIMPL_ACTU6SS39_H
#define DEVIMPL_ACTU6SS39_H


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
#define IMP_SUCCESS                 0
// 成功
#define IMP_ERR_LOAD_LIB            101 + 0xFF     // 动态库加载失败
#define IMP_ERR_PARAM_INVALID       102 + 0xFF     // 参数无效
#define IMP_ERR_READERROR           103 + 0xFF     // 读数据错误
#define IMP_ERR_WRITEERROR          104 + 0xFF     // 写数据错误
#define IMP_ERR_UNKNOWN             105 + 0xFF     // 未知错误
#define IMP_ERR_SNDDATA_INVALID     106 + 0xFF     // 无效的下发数据
#define IMP_ERR_RCVDATA_INVALID     107 + 0xFF     // 无效的应答数据

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
#define IMP_ERR_DEVRET_CMD_INVLID               1    // 未定义的命令
#define IMP_ERR_DEVRET_CMD_PARAM                2    // 命令参数错误
#define IMP_ERR_DEVRET_CMD_UKEY_NOHAVE          3    // 卡箱无卡
#define IMP_ERR_DEVRET_CMD_PASS_BLOCK           4    // 通道堵塞
#define IMP_ERR_DEVRET_CMD_SENSOR_FAIL          5    // 传感器坏
#define IMP_ERR_DEVRET_CMD_ELEV_NOBOT           6    // 升降梯不在底部
#define IMP_ERR_DEVRET_CMD_CMD_NORUN            7    // 命令不能执行
#define IMP_ERR_DEVRET_CMD_SCAN_FAIL            8    // 获取扫描信息失败
#define IMP_ERR_DEVRET_CMD_RETAIN_FULL          9    // 回收箱满
#define IMP_ERR_DEVRET_CMD_REPASS_BLOCK         10   // 回收通道堵塞
#define IMP_ERR_DEVRET_CMD_VAL_OVER             11   // 设置值超出范围
#define IMP_ERR_DEVRET_CMD_REBOX_NOHAVE         12   // 无回收箱
#define IMP_ERR_DEVRET_CMD_ELEV_NOSCAN          13   // 升降梯不在扫描位
#define IMP_ERR_DEVRET_CMD_POWER_FAIL           14   // 电压异常,过低或过高
#define IMP_ERR_DEVRET_CMD_ELEC_FAIL            15   // 升降电机模块故障
#define IMP_ERR_DEVRET_CMD_SELF_FAIL            16   // 自适应出错
#define IMP_ERR_DEVRET_CMD_ELEVM_FAIL           17   // 电梯模块故障
#define IMP_ERR_DEVRET_CMD_CAM_FAIL             18   // 凸轮模块故障
#define IMP_ERR_DEVRET_CMD_BOX_NOHAVE           19   // 没有卡箱或卡箱没有到位

/*************************************************************
// 无分类　宏定义
*************************************************************/
#define LOG_NAME                    "DevImpl_ACTU6SS39.log"   // 缺省日志名

/********************************************************
 * 连接方式: COM
 * 通讯方式: 异步,半双工
 * 起始位: 1bit
 * 数据位: 8bits
 * 校验位: 无
 * 停止位: 1bit
 * 默认波特率: 9600bps
 * 数据包格式: 请求: STX+SELEN+CM+PM+SE_DATA+ETX+BCC
 *        成功应答: STX+SELEN+‘P’+CM+PM+RE_DATA+ETX+BCC
 *        失败应答: STX+SELEN+‘N’+CM+PM+RE_DATA+ETX+BCC
 * 通讯流程:
********************************************************/

/*************************************************************
// 帧通信格式 设备请求/应答　宏定义
*************************************************************/

// 报文起始字符(1Bytes)
#define STX     0x02    // 报文开始
#define ETX     0x03    // 报文结束

// 报文长度占位
#define SELEN   2       // 报文长度(2Bytes),SELEN到ETX之间的数据长度

// 传输控制字符(1Bytes)
#define ACK     0x06    // 确认/肯定应答
#define NAK     0x15    // 否认应答
#define ENQ     0x05    // 执行命令请求
#define EOT     0x04    // 取消命令

// 报文中命令执行成功/失败标记
#define CMD_SUCC    "P"     // 命令执行成功:0x50
#define CMD_FAIL    "N"     // 命令执行失败:0x4E

// cm: 底层指令(命令码)
#define CM_RESET   0x40     // "@":复位
#define CM_STAT    0x41     // "A":状态查询
#define CM_STPAM   0x42     // "B":参数设置
#define CM_MOVE    0x43     // "C":移动操作
#define CM_SCAN    0x45     // "E":获取扫描信息
#define CM_STFUN   0x47     // "G":功能设置
#define CM_FUNSTA  0x49     // "I":功能状态查询

// pm: 底层指令(命令参数)
#define PM_0    0x30    // "0"
#define PM_1    0x31    // "1"
#define PM_2    0x32    // "2"
#define PM_3    0x33    // "3"
#define PM_4    0x34    // "4"
#define PM_5    0x35    // "5"
#define PM_6    0x36    // "6"

// 组合下发命令
#define S_CMD_RESET_RETAIN          "@0"    // 复位:回收
#define S_CMD_RESET_NOACTIVE        "@1"    // 复位:无动作,上传软件版本
#define S_CMD_RESET_TRANS           "@2"    // 复位:传动机构
#define S_CMD_RESET_EJECT           "@4"    // 复位:弹卡
#define S_CMD_STAT_SENSOR           "A1"    // 状态查询:读取每个传感器状态
#define S_CMD_STAT_FRONT_HAVE       "A2"    // 状态查询:前端未知是否有UKEY
#define S_CMD_STAT_SCAN_HAVE        "A3"    // 状态查询:扫描位置是否有UKEY
#define S_CMD_STAT_UKEY_BOX         "A4"    // 状态查询:查询UKEY箱状态
#define S_CMD_STAT_RETAIN_BOX       "A5"    // 状态查询:查询回收箱状态
#define S_CMD_STAT_UKBOX_HAVE       "A7"    // 状态查询:查询UKEY箱静止状态,只返回有无UK
#define S_CMD_STPAM_READ            "B0"    // 参数设置:读取
#define S_CMD_STPAM_WRITE           "B1"    // 参数设置:写入/设置
#define S_CMD_MOVE_BOX1_SCAN1       "C0"    // 移动操作:箱1UKEY到扫描位1
#define S_CMD_MOVE_BOX2_SCAN2       "C1"    // 移动操作:箱2UKEY到扫描位1
#define S_CMD_MOVE_UKEY1_FSCAN      "C2"    // 移动操作:箱1UKEY移动到前端扫描位置
#define S_CMD_MOVE_UKEY2_FSCAN      "C3"    // 移动操作:箱2UKEY移动到前端扫描位置
#define S_CMD_MOVE_SCAN2DOOR        "C4"    // 移动操作:UKEY从扫描位置发送到门口
#define S_CMD_MOVE_UKEY_RETAIN      "C5"    // 移动操作:回收UKEY
#define S_CMD_MOVE_UKEY1_DOOR       "C6"    // 移动操作:箱1UKEY直接移动到门口
#define S_CMD_MOVE_UKEY2_DOOR       "C7"    // 移动操作:箱2UKEY直接移动到门口
#define S_CMD_MOVE_DOOR2SCAN        "C8"    // 移动操作:UKEY从出口到扫描位置
#define S_CMD_MOVE_BOX2_SCAN2_N     "C9"    // 移动操作:箱2UKEY到扫描位2,不扫描
#define S_CMD_MOVE_BOX1_SCAN1_N     "C:"    // 移动操作:箱1UKEY到扫描位1,不扫描
#define S_CMD_MOVE_BOX2_SCAN1_N     "C;"    // 移动操作:箱2UKEY到扫描位1,不扫描
#define S_CMD_MOVE_BOX1_SCAN2_N     "C<"    // 移动操作:箱1UKEY到扫描位2,不扫描
#define S_CMD_SCAN_GET_INFO         "E0"    // 扫描信息:获取扫描信息
#define S_CMD_SCAN_CLR_INFO         "E1"    // 扫描信息:清除扫描信息
#define S_CMD_SCAN_START            "E3"    // 扫描信息:开启一次扫码
#define S_CMD_SCAN_DEF              "E4"    // 扫描信息:未知
#define S_CMD_SCAN_QR_OPEN          "E5"    // 扫描信息:打开二维码功能
#define S_CMD_SCAN_QR_CLOSE         "E6"    // 扫描信息:关闭二维码功能
#define S_CMD_SCAN_BAR_OPEN         "E7"    // 扫描信息:打开条码功能
#define S_CMD_SCAN_BAR_CLOSE        "E8"    // 扫描信息:关闭条码功能
#define S_CMD_STFUN_POWFAL_NOR      "G3"    // 功能设置:掉电无动作
#define S_CMD_STFUN_POWFAL_RETAIN   "G8"    // 功能设置:掉电回收
#define S_CMD_STFUN_POWFAL_EJECT    "G9"    // 功能设置:掉电弹出
#define S_CMD_FUNSTA_POWFAL_MODE    "I0"    // 功能状态查询:掉电工作模式

// 报文组织相关声明
#define STAND_CMD_LENGHT            2       // 标准命令长度(STX+RELEN)
#define RESP_BUFF_SIZE              1024    // 应答数据Buff Size
#define TIMEOUT_RCVCMD              5*1000  // 命令接收超时时间
#define TIMEOUT_SNDCMD              3*1000  // 命令下发超时时间
#define TIMEOUT_DISPENSECARD        15000   // 发UKEY缺省超时时间
#define TIMEOUT_RESET               30000   // 复位缺省超时时间
#define TIMEOUT_READSCAN            45000   // 扫描缺省超时时间

// 应答数据有效数据位置
#define RCVCMDPOS                   4           // STX（1byte)+LEN(2byte)+"P/N"(1Byte):CMD(2byte)
#define RCVRETPOS                   3           // STX（1byte)+LEN(2byte):"P/N"(1Byte)
#define RCVERRPOS                   6           // STX（1byte)+LEN(2byte)+"P/N"(1Byte)+ CMD(2byte)
#define RCVINFOPOS                  6           // STX（1byte)+LEN(2byte)+"P/N"(1Byte)+ CMD(2byte)

// 提取应答数据中有效信息(DATALEN不包含BCC)
#define GETBUFF(BUFF, BUFFLEN, RCVDATA, DATALEN) \
    memcpy(BUFF, RCVDATA + RCVINFOPOS, \
           ((DATALEN - RCVINFOPOS - 1) > BUFFLEN ? BUFFLEN : (DATALEN - RCVINFOPOS - 1)));

/*************************************************************
// 报文返回错误码(ERR_CD)　宏定义
*************************************************************/
#define ERR_CD_INVLID               0xFF    // 未定义的命令
#define ERR_CD_PARAM                0x01    // 命令参数错误
#define ERR_CD_UKEY_NOHAVE          0x02    // 卡箱无卡
#define ERR_CD_PASS_BLOCK           0x03    // 通道堵塞
#define ERR_CD_SENSOR_FAIL          0x04    // 传感器坏
#define ERR_CD_ELEV_NOBOT           0x05    // 升降梯不在底部
#define ERR_CD_CMD_NORUN            0x06    // 命令不能执行
#define ERR_CD_SCAN_FAIL            0x07    // 获取扫描信息失败
#define ERR_CD_RETAIN_FULL          0x08    // 回收箱满
#define ERR_CD_REPASS_BLOCK         0x09    // 回收通道堵塞
#define ERR_CD_VAL_OVER             0x0A    // 设置值超出范围
#define ERR_CD_REBOX_NOHAVE         0x0B    // 无回收箱
#define ERR_CD_ELEV_NOSCAN          0x0C    // 升降梯不在扫描位
#define ERR_CD_POWER_FAIL           0x10    // 电压异常,过低或过高
#define ERR_CD_ELEC_FAIL            0x12    // 升降电机模块故障
#define ERR_CD_SELF_FAIL            0x13    // 自适应出错
#define ERR_CD_ELEVM_FAIL           0x14    // 电梯模块故障
#define ERR_CD_CAM_FAIL             0x15    // 凸轮模块故障
#define ERR_CD_BOX_NOHAVE           0x17    // 没有卡箱或卡箱没有到位

/*************************************************************
// 对外接口提供参数　宏定义
*************************************************************/
// 复位动作模式
#define MODE_RESET_NOACTIVE         0       // 复位无动作
#define MODE_RESET_RETAIN           1       // 复位回收
#define MODE_RESET_EJECT            2       // 复位弹出
#define MODE_RESET_TRANS            4       // 复位传动机构

// 介质移动模式
#define MODE_MOVE_BOX1_SCAN1        0        // 移动操作:箱1UKEY到扫描位1
#define MODE_MOVE_BOX2_SCAN2        1        // 移动操作:箱2UKEY到扫描位1
#define MODE_MOVE_UKEY1_FSCAN       2        // 移动操作:箱1UKEY移动到前端扫描位置
#define MODE_MOVE_UKEY2_FSCAN       3        // 移动操作:箱2UKEY移动到前端扫描位置
#define MODE_MOVE_SCAN2DOOR         4        // 移动操作:UKEY从扫描位置发送到门口
#define MODE_MOVE_UKEY_RETAIN       5        // 移动操作:回收UKEY
#define MODE_MOVE_UKEY1_DOOR        6        // 移动操作:箱1UKEY直接移动到门口
#define MODE_MOVE_UKEY2_DOOR        7        // 移动操作:箱2UKEY直接移动到门口
#define MODE_MOVE_DOOR2SCAN         8        // 移动操作:UKEY从出口到扫描位置
#define MODE_MOVE_BOX2_SCAN2_N      9        // 移动操作:箱2UKEY到扫描位2,不扫描
#define MODE_MOVE_BOX1_SCAN1_N      10       // 移动操作:箱1UKEY到扫描位1,不扫描
#define MODE_MOVE_BOX2_SCAN1_N      11       // 移动操作:箱2UKEY到扫描位1,不扫描
#define MODE_MOVE_BOX1_SCAN2_N      12       // 移动操作:箱1UKEY到扫描位2,不扫描

// 介质扫描模式
#define MODE_SCAN_GET_INFO          0       // 扫描信息:获取扫描信息
#define MODE_SCAN_CLR_INFO          1       // 扫描信息:清除扫描信息
#define MODE_SCAN_START             2       // 扫描信息:开启一次扫码
#define MODE_SCAN_DEF               3       // 扫描信息:未知
#define MODE_SCAN_QR_OPEN           4       // 扫描信息:打开二维码功能
#define MODE_SCAN_QR_CLOSE          5       // 扫描信息:关闭二维码功能
#define MODE_SCAN_BAR_OPEN          6       // 扫描信息:打开条码功能
#define MODE_SCAN_BAR_CLOSE         7       // 扫描信息:关闭条码功能

// 功能设置/获取模式
#define MODE_ST_POWOFF_NOACT        0       // 掉电后处理:无动作
#define MODE_ST_POWOFF_RETAIN       1       // 掉电后处理:回收
#define MODE_ST_POWOFF_EJECT        2       // 掉电后处理:弹出

//
#define MODE_SCAN_PARAM             0       // 扫描时托盘移动参数


/*************************************************************
// 设备状态　宏定义
*************************************************************/
// 箱状态
#define STAT_FULL                   0           // 满
#define STAT_LOW                    1           // 少
#define STAT_EMPTY                  2           // 空
#define STAT_NOHAVE                 3           // 无
#define STAT_ISHAVE                 4           // 有
#define STAT_UNKNOWN                5           // 未知

// 设备状态
#define DEV_STAT_ONLINE             0
#define DEV_STAT_OFFLINE            1
#define DEV_STAT_HWERR              2

/*************************************************************
// 封装类: 命令编辑、发送接收等处理。
*************************************************************/
class CDevImpl_ACTU6SS39 : public CLogManage
{
public:
    CDevImpl_ACTU6SS39();
    CDevImpl_ACTU6SS39(LPSTR lpLog);
    virtual ~CDevImpl_ACTU6SS39();
    void Init();

public: //
    INT     DeviceOpen(LPSTR lpMode);                               // 打开设备
    INT     DeviceClose();                                          // 关闭设备
    INT     Release();                                              // 释放动态库
    BOOL    IsDeviceOpen();                                         // 检查设备打开标记
    void    SetTimeOut(UINT uiSndTimeOut = 0, UINT uiRcvTimeOut = 0);// 设置超时时间

public: // 接口函数封装
    INT     DeviceReset(USHORT usMode);                             // 1. 复位
    INT     GetDeviceStat(USHORT usMode, INT nStat[12]);            // 2. 获取设备状态
    INT     MoveMedia(USHORT usMode);                               // 3. 介质移动
    INT     ScanMedia(USHORT usMode, LPSTR lpRcvData, LPINT lpRcvDataLen);// 4. 介质扫描
    INT     SetDeviceParam(USHORT usMode, INT nParam[12]);          // 5. 设置设备参数
    INT     GetDeviceParam(USHORT usMode, INT nParam[12]);          // 6. 获取设备参数
    INT     GetDeviceFW(LPSTR lpDevFW, USHORT usLen);               // 7. 获取设备固件版本

private: // USB命令收发
    INT CmdSendRecv(LPCSTR lpcsSndCmd, LPCSTR lpcSndData, INT nDataLen,
                    LPSTR lpRcvData, LPINT lpRcvDataLen, LPSTR lpDesc = nullptr,
                    UINT uiRcvTimeOut = TIMEOUT_RCVCMD, UINT uiSndTimeOut = TIMEOUT_SNDCMD,
                    BOOL bLog = TRUE);
    INT SendCmd(LPCSTR lpcCmd, LPCSTR lpcData, INT nLen, UINT uiTimeout, BOOL bSingle = FALSE); // 主下发数据
    INT GetResponse(LPSTR lpzResponse, INT nLen, UINT uiTimeout, INT &dwOutLen); // 主接收数据
    BYTE GetDataBCC(LPSTR lpData, USHORT usDataSize);                           // 计算BCC
    INT RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT &nRcvDataLen, BOOL bLog = TRUE);    // 应答数据检查

private: // 错误码转换
    INT     ConvertErrorCode(INT nRet);                             // DEVICE/USB/COM错误码转换为Impl错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);                          // IMPL错误码解释
    CHAR*   CmdToStr(LPSTR lpCmd);                                  // 下发命令码解释

private:
    string                          m_strMode;
    CQtDLLLoader<IAllDevPort>       m_pDev;     // USB接口处理
    BOOL                            m_bDevOpenOk;
    CSimpleMutex                    m_MutexAction;
    INT                             m_nLastError;
    CHAR                            m_szCmdStr[1024];
    CHAR                            m_szErrStr[1024];
    WORD                            m_wSignalCom;
    UINT                            m_uiSndTimeOut;
    UINT                            m_uiRcvTimeOut;
    INT                             m_nGetStatRetList[8];           // GetDevStat接口返回值列表,记录上一次返回值
    CHAR                            m_szGetStatRetList[8][32];      // GetDevStat接口返回数据列表,记录上一次返回值
};


#endif // DEVIMPL_ACTU6SS39_H
