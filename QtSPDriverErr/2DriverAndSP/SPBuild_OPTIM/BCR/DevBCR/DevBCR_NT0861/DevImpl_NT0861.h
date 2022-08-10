/*****************************************************************************
* 文件名称：DevImpl_NT0861.h
* 文件描述：封装条码阅读模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年7月13日
* 文件版本：1.0.0.1
******************************************************************************/
#ifndef DEVIMPL_NT0861_H
#define DEVIMPL_NT0861_H


#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "IAllDevPort.h"
#include "DevDef_NT0861.h"

/***************************************************************************
// 返回值/错误码　宏定义
// <0 : USB/COM接口处理返回
// 0~100: 硬件设备返回
// > 100: Impl处理返回
***************************************************************************/
// > 100: Impl处理返回
#define IMP_SUCCESS                 0                       // 成功
#define IMP_ERR_LOAD_LIB            1                       // 动态库加载失败
#define IMP_ERR_PARAM_INVALID       2                       // 参数无效
#define IMP_ERR_READERROR           3                       // 读数据错误
#define IMP_ERR_WRITEERROR          4                       // 写数据错误
#define IMP_ERR_RCVDATA_INVALID     5                       // 无效的应答数据
#define IMP_ERR_RCVDATA_NOTCOMP     6                       // 返回数据不完整
#define IMP_ERR_SNDCMD_INVALID      7                       // 下发命令无效
#define IMP_ERR_UNKNOWN             99                      // 未知错误

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


/***************************************************************************
// 无分类　宏定义
***************************************************************************/

#define LOG_NAME                        "DevImpl_NT0861.log"   // 缺省日志名

// 超时时间定义, 单位:毫秒
#define TIMEOUT_WAIT_ACTION             10*1000         // 命令下发接收超时

// USB/COM动态库句柄检查
#define CHK_DEVHANDLE(h) \
    if (h == nullptr) \
    { \
        Log(ThisModule, __LINE__, "DevHandle == NULL, USB/COM动态库句柄无效, Return: %s", \
            ConvertCode_Impl2Str(IMP_ERR_DEVPORT_NOTOPEN)); \
        return ERR_DEVPORT_NOTOPEN; \
    }


/**************************************************************************
// 枚举 定义
**************************************************************************/
// 设备状态
enum EN_DEVSTAT
{
    DEV_OK          = 0,    // 设备正常
    DEV_NOTFOUND    = 1,    // 设备未找到
    DEV_NOTOPEN     = 2,    // 设备未打开
    DEV_OFFLINE     = 3,    // 设备已打开但断线
    DEV_UNKNOWN     = 4,    // 设备状态未知
};

// 条码类型(对外返回)
enum EN_CODE_MODE
{
    EN_CODE_Codabar                 = 0,        // Codabar
    EN_CODE_Code128                 = 1,        // Code 128
    EN_CODE_Code39                  = 2,        // Code 39
    EN_CODE_Code32                  = 3,        // Code32
    EN_CODE_Code93                  = 4,        // Code 93
    EN_CODE_DataMatrix              = 5,        // Data Matrix
    EN_CODE_InterL2OF5              = 6,        // InterLeaved 2 of 5
    EN_CODE_PDF417                  = 7,        // PDF417
    EN_CODE_QR                      = 8,        // QR
    EN_CODE_UPCA                    = 9,        // UPC-A
    EN_CODE_UPCE                    = 10,        // UPC-E
    EN_CODE_UPCE1                   = 11,       // UPC-E1
    EN_CODE_UPCE8                   = 12,       // UPC-E8
    EN_CODE_UPCE13                  = 13,       // UPC-E13
    EN_CODE_Matrix2OF5              = 14,       // Matrix 2 of 5
    EN_CODE_Indust2OF5              = 15,       // Industrial 2 of 5
    EN_CODE_UNKNOWN                 = 99,       // 未知
    EN_CODE_ALL                     = 100,      // 所有
};

/**************************************************************************
// 封装类: 命令编辑、发送接收等处理。
**************************************************************************/
class CDevImpl_NT0861 : public CLogManage
{
public:
    CDevImpl_NT0861();
    CDevImpl_NT0861(LPSTR lpLog);
    CDevImpl_NT0861(LPSTR lpLog, LPCSTR lpDevType);
    virtual ~CDevImpl_NT0861();

public:
    INT     OpenDevice(LPSTR lpMode);                               // 打开设备
    INT     CloseDevice();                                          // 关闭设备
    INT     Release();                                              // 释放动态库
    BOOL    IsDeviceOpen();                                         // 设备是否Open
    CHAR*   CmdToStr(INT nCmdListNo);                               // 命令转换为解释字符串
    INT     ConvertCode_USB2Impl(long lRet);                        // USB处理错误值转换为Impl返回码/错误码
    INT     ConvertCode_Dev2Impl(CHAR szDeviceErr[3]);              // 硬件错误值转换为Impl返回码/错误码
    LPSTR   ConvertCode_Impl2Str(INT nErrCode);                     // Impl错误码转换解释字符串
    INT     ConvertSymMode(LPSTR lpSymData);                        // 应答条码类型转换为Impl格式标准

public: // 接口函数封装
    INT     DeviceInit(INT nParam = 0);                             // 模块初始化
    INT     GetDeviceStat();                                        // 获取设备状态
    INT     GetVersion(WORD wType, LPSTR lpVerData, WORD wVerSize); // 取版本
    INT     ScanCodeStart(INT nMode = 0);                           // 开始扫码
    INT     ScanCodeEnd(INT nMode = 0);                             // 停止扫码
    INT     GetScanCode(LPSTR lpCodeData, DWORD &dwCodeDataSize,
                        INT &nCodeType, DWORD dwTimeOut);           // 获取扫码数据
    INT     SetSymDistAllow(INT nCodeType);                         // 设置条码识别允许
    INT     SetSymDistForbid(INT nCodeType);                        // 设置条码识别禁止
    INT     SetSymScanMode(INT nMode);                              // 设置扫码识读模式

private:    
    CQtDLLLoader<IAllDevPort>       m_pDev;                         // 硬件接口处理
    string          m_strMode;                                      // 连接设备串
    BOOL            m_bDevOpenOk;                                   // 设备是否Open
    BOOL            m_bReCon;                                       // 是否断线重连状态
    CHAR            m_szDevType[64];                                // 设备类型
    CHAR            m_szErrStr[1024];                               // IMPL错误解释
    CHAR            m_szCmdStr[256];                                // 命令解释
    DWORD           m_dwSndTimeOut;                                 // 命令下发超时时间(毫秒)
    DWORD           m_dwRcvTimeOut;                                 // 命令接收超时时间(毫秒)
    INT             m_nRetErrOLD[8];                                // 处理错误值保存(0:硬件接口动态库/1:设备连接/
                                                                    //  2:设备初始化/3取扫码数据/4)
    CHAR            m_szVersion[4][128];                            // 1固件版本/2PN/3SN/4CN
    CHAR            m_szDevName[128];                               // 设备名

private:
    void Init();

private:    // 命令收发
    INT SendCmd(LPCSTR lpcCmd, INT nCmdLen, DWORD dwTimeOut,
                LPCSTR lpFuncData, BOOL bIsPrtLog = TRUE);          // 下发数据
    INT GetResponse(LPSTR lpRespData, DWORD dwRespDataLen, DWORD dwTimeOut,
                    LPCSTR lpFuncData, BOOL bIsPrtLog = TRUE);      // 接收数据
    DWORD GetCheckSum(LPCSTR lpcData, INT nDataLen, DWORD dwChkInit = CHECKVAL);// 取校验和

private:    // USB命令应答数据处理
    INT SndRcvToChk(INT nCmdListNo, LPSTR lpRcvData, INT &nRcvSize,
                    BOOL bIsCheck = TRUE, LPCSTR lpPrtData = nullptr,
                    BOOL bIsPrtLog = TRUE);                         // 命令收发及检查
    INT RcvDataCheck(INT nCmdListNo, LPSTR lpRcvData, INT &nRcvDataLen,
                     LPCSTR lpPrtData = nullptr, BOOL bIsPrtLog = TRUE);// 应答数据Check

public:    // SetData相关
    INT SetReConFlag(BOOL bFlag);                                   // 设置断线重连标记
    INT SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)
    INT SetSndRcvTimeOut(DWORD dwSnd, DWORD dwRcv);                 // 设置命令收发超时时间

private: // 接口加载
    CSimpleMutex                    m_MutexAction;
    INT                             m_nLastError;

};


#endif // DEVIMPL_NT0861_H

// -------------------------------- END -----------------------------------
