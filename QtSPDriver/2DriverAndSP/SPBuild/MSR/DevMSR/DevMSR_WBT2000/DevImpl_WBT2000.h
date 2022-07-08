/***************************************************************
* 文件名称：DevImpl_WBT2000.h
* 文件描述：刷折模块底层指令，提供控制接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_WBT2000_H
#define DEVIMPL_WBT2000_H

#include <string>

#include "IDevMSR.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
//#include "wbt2000.h"
#include "../XFS_MSR/def.h"

// ------------------------------　类型定义　------------------------------
typedef int DEVICEID;     // 定义设备ID类型

// ------------------------------- 宏定义 --------------------------------
#define DLL_DEVLIB_NAME  "libwbt2000.so"            // 缺省动态库名
#define LOG_NAME         "DevImpl_WBT2000.log"      // 缺省日志名

// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

/*************************************************************
// 返回值/错误码　宏定义
// <0 : Impl处理返回
// 0~100: 硬件设备返回
*************************************************************/
// >0: Impl处理返回
#define IMP_SUCCESS                 0                       // 成功
#define IMP_ERR_LOAD_LIB            1                       // 动态库加载失败
#define IMP_ERR_PARAM_INVALID       2                       // 参数无效
#define IMP_ERR_UNKNOWN             3                       // 未知错误
#define IMP_ERR_NOTOPEN             4                       // 设备未Open
// <0: Device返回(参考wbt200.h中 enum APIRETURN)
#define IMP_DEV_ERR_SUCCESS         0                       // 0
#define IMP_DEV_ERR_FAIL            -1                      // -1:命令执行失败
#define IMP_DEV_ERR_PARAMATER       -2                      // -2:参数错误
#define IMP_DEV_ERR_WRITE           -3                      // -3:写数据出错
#define IMP_DEV_ERR_WTIMEOUT        -4                      // -4:写数据超时
#define IMP_DEV_ERR_READ            -5                      // -5:读数据出错
#define IMP_DEV_ERR_RTIMEOUT        -6                      // -6:读数据超时
#define IMP_DEV_ERR_FRAME           -7                      // -7:数据不符合协议
#define IMP_DEV_ERR_UNKNOWN         -8                      // -8:未知错误
#define IMP_DEV_ERR_NOSUPP          -9                      // -9:不支持命令
#define IMP_DEV_ERR_CANCEL          -10                     // -10:命令取消
#define IMP_DEV_ERR_RUNNING         -11                     // -11:命令运行中


// ---------------------------- SDK使用宏定义 ----------------------------
// SDK使用
#define NAME_SIZE   64

// SDK 日志等级
#define LOG_NONE        0
#define LOG_MESSAGE     1
#define LOG_DEBUG       2
#define LOG_WARNING     3
#define LOG_ERROR       4

// mSetHiLoCo使用写磁道模式
#define WTRACK_DK       0           // 低抗写
#define WTRACK_GK       1           // 高抗写

// 磁道模式
#define TRACK_NOT       0           // 不读写磁道
#define TRACK_1         1           // 读写磁道1
#define TRACK_2         2           // 读写磁道2
#define TRACK_3         3           // 读写磁道3
#define TRACK_12        4           // 读写磁道12
#define TRACK_23        5           // 读写磁道23
#define TRACK_13        6           // 读写磁道13
#define TRACK_123       7           // 读写磁道123


// ------------- 结构体　定义 -------------
// 串口设备名字结构
typedef struct PORT_NAME
{
    char path[NAME_SIZE];     // 设备路径("/dev/ttyS0")
} STPORTNAME, *LPPORTNAME;

// 串口设备名字结构
typedef struct HID_PORT
{
  char              path[NAME_SIZE];        // 设备路径
  unsigned short    vendor_id;              // 设备厂商号
  unsigned short    product_id;             // 设备产品号
  int               interface_number;       // 设备接口号，默认0, 如果是复合设备可能有多个接口
} STHIDPORT, *LPHIDPORT;


// ------------- 动态库输出函数　定义 -------------
// 监控发送接受数据回调
typedef void (* monitorcallback)(const void *pdata, int datalen);


#ifdef __cplusplus
extern "C" {
#endif

// 1. 库版本信息模块，获取版本信息函数
typedef const char* (*mWBTGetLibVersion)(void);

// 2. 枚举系统中所有串口
typedef int (*mWBTListPortSPath)(char *pathlist, int pathlen);

// 3. 列出操作系统上存在的串口
typedef int (*mWBTListPort)(LPPORTNAME patharray, int arlen);

// 4. 设置指定串口发送接收数据回调函数,监控数据
typedef void (*mWBTSetMonitorPort)(int portfd, monitorcallback readcb, monitorcallback writecb);

// 5. 开启日志功能
typedef void (*mWBTEnableLog)(int level);

// 6. 获取日志等级
typedef int (*mWBTGetLogLevel)();

// 7. 禁止日志功能
typedef void (*mWBTDisableLog)();

// 8. 打开指定设备端口
typedef int (*mWBTOpenPort)(const char *devpath,                           // 设备路径(NULL自动搜索)
                         int devparam);                                 // 端口参数(波特率缺省9600)

// 9. 关闭指定设备端口
typedef bool (*mWBTClosePort)(int portfd);

// 10. 取消正在执行操作
typedef void (*mWBTCancelPort)(int portfd,
                            int iCancel);                               // 是否取消(>0取消, <=0不取消)

// 11. 查询指定设备路径名字
typedef const char* (*mWBTGetPortName)(int portfd);

// 12. 软复位
typedef int (*mWBTReset)(int portfd);

// 13. 获取固件版本
typedef int (*mWBTGetFirmVer)(int portfd, char *strVer);

// 14. 设置写磁道模式
typedef int (*mWBTSetHiLoCo)(int portfd,
                          unsigned char type);                          // 参考写磁道模式宏定义

// 15. 设置读磁道模式
typedef int (*mWBTSetReadMGCardMode)(int portfd,
                                  unsigned char type);                  // 参考磁道模式宏定义

// 16. 设置蜂鸣器Buzzer模式
typedef int (*mWBTSetBuzzerMode)(int portfd, unsigned char type);

// 17. 获取蜂鸣器Buzzer模式
typedef int (*mWBTGetBuzzerMode)(int portfd, unsigned char *type);

// 18. 控制蜂鸣器鸣响次数
typedef int (*mWBTSetBuzzer)(int portfd, unsigned char num);

// 19. 设置串口设备通讯波特率
typedef int (*mWBTSetBPS)(int portfd, int iBaudRate);

// 20. 发送命令读取磁道数据(ASCII)
// 注: pInOutLen[0] > 0: 数据正确; == 0, 错误/空白磁道
typedef int (*mWBTReadMGCard)(int portfd,
                              unsigned char ucTrackID,                     // 参考磁道模式宏定义
                              unsigned char *pTrack1, int *pInOutLen1,     // 磁道1Buff及长度(>=256Byte)
                              unsigned char *pTrack2, int *pInOutLen2,     // 磁道2Buff及长度(>=256Byte)
                              unsigned char *pTrack3, int *pInOutLen3,     // 磁道3Buff及长度(>=256Byte)
                              int iTimeOut);                               // 等待刷卡超时(单位:毫秒)

// 21. 自动读取磁道数据(ASCII)
// 注: 与SetReadMGCardMode配合使用
typedef int (*mWBTAutoReadMGCard)(int portfd,
                               unsigned char *pTrack1, int *pInOutLen1, // 磁道1Buff及长度(>=256Byte)
                               unsigned char *pTrack2, int *pInOutLen2, // 磁道2Buff及长度(>=256Byte)
                               unsigned char *pTrack3, int *pInOutLen3, // 磁道3Buff及长度(>=256Byte)
                               int iTimeOut);                           // 等待刷卡超时(单位:毫秒)

// 22. 写磁道
// 注: pInOutLen[0] > 0: 数据正确; == 0, 错误/空白磁道
//     使用ISO7811协议
typedef int (*mWBTWriteMGCard)(int portfd,
                               unsigned char ucTrackID,                     // 参考磁道模式宏定义
                               unsigned char *pTrack1, int *pInOutLen1,     // 磁道1Buff及长度(>=256Byte)
                               unsigned char *pTrack2, int *pInOutLen2,     // 磁道2Buff及长度(>=256Byte)
                               unsigned char *pTrack3, int *pInOutLen3,     // 磁道3Buff及长度(>=256Byte)
                               int iTimeOut);                               // 等待刷卡超时(单位:毫秒)

// 23. 清除磁道
typedef int (*mWBTEraseMGCard)(int portfd,
                               unsigned char ucTrackID,                     // 参考磁道模式宏定义
                               int iTimeOut);                               // 等待刷卡超时(单位:毫秒)

// 24. 取消正在进行的磁道读写操作
typedef int (*mWBTCancelMGCard)(int portfd);

#ifdef __cplusplus
}
#endif //__cplusplus


/**************************************************************************
* 命令编辑、发送接收等处理
***************************************************************************/
class CDevImpl_WBT2000 : public CLogManage
{
public:
    CDevImpl_WBT2000();
    CDevImpl_WBT2000(LPSTR lpLog);
    virtual ~CDevImpl_WBT2000();

public:
    BOOL    IsDeviceExist(STDEVICEOPENMODE stDevExist, BOOL bAddLog = TRUE);   // 检查指定设备是否存在/连接
    BOOL    OpenDevice(STDEVICEOPENMODE stOpenMode);                    // 打开指定设备
    BOOL    CloseDevice();
    INT     IsDeviceOpen();
    LPSTR   ConvertErrCodeToStr(INT nErrCode);
    INT     SetReConFlag(BOOL bFlag)
    {
        m_bReCon = bFlag;
    }
    void    SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)

public: // 接口函数封装
    INT     nGetLibVersion(LPSTR lpVersion);                            // 1. 库版本信息模块，获取版本信息函数
    INT     nEnableLog(INT nLevel);                                     // 5. 开启日志功能
    INT     nDisableLog();                                              // 7. 禁止日志功能
    INT     nCancelPort(INT nCancel = 1);                               // 10. 取消正在执行操作
    INT     nGetPortName(LPSTR lpPortName, WORD wSize);      		    // 11. 查询指定设备路径名字
    INT     nReset();                                                   // 12. 复位设备
    INT     nGetFWVersion(LPSTR lpFwVer);                               // 13. 获取固件版本
    INT     nReadMGCard(UCHAR lpFormat,
                        LPUCHAR lpTrack1, LPINT lpInOutLen1,
                        LPUCHAR lpTrack2, LPINT lpInOutLen2,
                        LPUCHAR lpTrack3, LPINT lpInOutLen3,
                        INT nTimeOut);                                  // 20. 发送命令读取磁道数据(ASCII)
private:
    DEVICEID        m_DeviceId;
    BOOL            m_bDevOpenOk;
    CHAR            m_szErrStr[1024];
    CHAR            m_szFWVer[256];
    BOOL            m_bReCon;                                           // 是否断线重连状态
    INT             m_nRetErrOLD[8];                                    // 处理错误值保存

public:
    STPORTNAME      m_stPortList[128];                                  // 枚举串口列表
    WORD            m_wPortListCnt;                                     // 枚举串口列表数目

private:
    void    Init();

private: // 接口加载
    BOOL    bLoadLibrary();
    void    vUnLoadLibrary();
    BOOL    bLoadLibIntf();

private: // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    //INT         m_nDevErrCode;

private: // 动态库接口定义
    mWBTGetLibVersion               GetLibVersion;          // 1. 库版本信息模块，获取版本信息函数
    mWBTListPortSPath               ListPortSPath;    		// 2. 枚举系统中所有串口
    mWBTListPort                    ListPort;         		// 3. 列出操作系统上存在的串口
    mWBTSetMonitorPort              SetMonitorPort;   		// 4. 设置指定串口发送接收数据回调函数,监控数据
    mWBTEnableLog                   EnableLog;        		// 5. 开启日志功能
    mWBTGetLogLevel                 GetLogLevel;            // 6. 获取日志等级
    mWBTDisableLog                  DisableLog;       		// 7. 禁止日志功能
    mWBTOpenPort                    OpenPort;         		// 8. 打开指定设备端口
    mWBTClosePort                   ClosePort;        		// 9. 关闭指定设备端口
    mWBTCancelPort                  CancelPort;       		// 10. 取消正在执行操作
    mWBTGetPortName                 GetPortName;      		// 11. 查询指定设备路径名字
    mWBTReset                       Reset;                  // 12. 软复位
    mWBTGetFirmVer                  GetFirmVer;             // 13. 获取固件版本
    mWBTSetHiLoCo                   SetHiLoCo;              // 14. 设置写磁道模式
    mWBTSetReadMGCardMode           SetReadMGCardMode;      // 15. 设置读磁道模式
    mWBTSetBuzzerMode               SetBuzzerMode;          // 16. 设置蜂鸣器Buzzer模式
    mWBTGetBuzzerMode               GetBuzzerMode;          // 17. 获取蜂鸣器Buzzer模式
    mWBTSetBuzzer                   SetBuzzer;              // 18. 控制蜂鸣器鸣响次数
    mWBTSetBPS                      SetBPS;                 // 19. 设置串口设备通讯波特率
    mWBTReadMGCard                  ReadMGCard;             // 20. 发送命令读取磁道数据(ASCII)
    mWBTAutoReadMGCard              AutoReadMGCard;         // 21. 自动读取磁道数据(ASCII)
    mWBTWriteMGCard                 WriteMGCard;            // 22. 写磁道
    mWBTEraseMGCard                 EraseMGCard;            // 23. 清除磁道
    mWBTCancelMGCard                CancelMGCard;           // 24. 取消正在进行的磁道读写操作
};


#endif // DEVIMPL_WBT2000_H
