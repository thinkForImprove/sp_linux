/***************************************************************
* 文件名称：DevImpl_WBCS10.h
* 文件描述：刷折模块底层指令，提供控制接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_WBCS10_H
#define DEVIMPL_WBCS10_H

#include <string>

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "wb-cs10.h"
#include "../XFS_MSR/def.h"

// -------------　类型定义　-------------
typedef int DEVICEID;     // 定义设备ID类型

// ------------- 宏定义 -------------
#define DLL_DEVLIB_NAME  "libwbcs10.so"          // 缺省动态库名
#define LOG_NAME         "DevImpl_WBCS10.log"   // 缺省日志名

// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

// ------------- 动态库输出函数　定义 -------------
#ifdef __cplusplus
extern "C" {
#endif

// 1. 库版本信息模块，获取版本信息函数
typedef const char* (*mGetLibVersion)(void);

// 2. 枚举系统中所有串口
typedef int (*mListPortSPath)(char *pathlist, int pathlen);

// 3. 列出操作系统上存在的串口
typedef int (*mListPort)(struct port_name *patharray, int arlen);

// 4. 列出操作系统上连接指定厂商号和产品号的USB HID设备
typedef int (*mListHIDPort)(unsigned short vendor_id, unsigned short product_id);

// 5. 列出操作系统上连接的USB HID设备
typedef int (*mListHIDPortS)(struct hid_port *phidarray, int arraylen);

// 6. 设置指定串口发送接收数据回调函数,监控数据
typedef void (*mSetMonitorPort)(int portfd, monitorcallback readcb, monitorcallback writecb);

// 7. 开启日志,写入文件和显示在错误输出流中
typedef bool (*mEnableLog)(int level, const char *logdir);

// 8. 禁止日志
typedef void (*mDisableLog)();

// 9. 打开指定设备端口
typedef int (*mOpenPort)(const char *devpath, int devparam);

// 10. 关闭指定设备端口
typedef bool (*mClosePort)(int portfd);

// 11. 取消正在执行操作
typedef void (*mCancelPort)(int portfd, int iCancel);

// 12 查询指定设备路径名字
typedef const char* (*mGetPortName)(int portfd);

// 13. 复位设备,获取设备固件版本
typedef int (*mReset)(int portfd, char *strVersion);

// 14. 获取设备状态配置信息
typedef int (*mStatus)(int portfd, unsigned char *pFormat, unsigned char *pTrack, unsigned char *pEnable);

// 15. 设置设备配置
typedef int (*mSetup)(int portfd, unsigned char btFormat, unsigned char btTrack);

// 16. 禁止刷卡,刷卡无效
typedef int (*mDisable)(int portfd);

// 17. 设置串口设备通讯波特率
typedef int (*mSetBPS)(int portfd, int iBaudRate);

// 18. 设置USB键盘模式和磁道数据结尾是否添加回车符'CR'
typedef int (*mSetUSBKB)(int portfd, unsigned char btKB, unsigned char btCR);

// 19. 设置LED模式
typedef int (*mSetLEDMode)(int portfd, unsigned char btMode);

// 20. 控制设备LED
typedef int (*mSetDeviceLED)(int portfd, unsigned char bOnTime, unsigned char bNum);

// 21. 设置蜂鸣器模式
typedef int (*mSetBuzzerMode)(int portfd, unsigned char btMode);

// 22. 控制设备蜂鸣器Buzzer
typedef int (*mSetDeviceBuzzer)(int portfd, unsigned char bOnTime, unsigned char bNum);

// 23. 设置读取磁卡操作模式
typedef int (*mSetDeviceCtrlMode)(int portfd, unsigned char btMode);

// 24. 获取读取磁卡操作模式
typedef int (*mGetDeviceCtrlMode)(int portfd, unsigned char *btMode);

// 25. 允许刷磁卡,等待刷卡读取磁道数据,命令模式下使用(被动模式)
typedef int (*mEnable)(int portfd, bool bFaildContinued, unsigned char *pFormat, unsigned char *pTrack1, int *pInOutLen1,
                       unsigned char *pTrack2, int *pInOutLen2, unsigned char *pTrack3, int *pInOutLen3, int iTimeout);

// 26. 重新读取上次刷卡有效数据,指令模式下使用(被动模式)
typedef int (*mResend)(int portfd, unsigned char *pFormat,  unsigned char *pTrack1, int *pInOutLen1,
                       unsigned char *pTrack2, int *pInOutLen2, unsigned char *pTrack3, int *pInOutLen3);

// 27. 允许刷磁卡,缓存模式下使用(被动模式).
typedef int (*mEnableSwipingCard)(int portfd);

// 28. 获取磁道数据,缓存模式下使用(被动模式)
typedef int (*mGetTrackCache)(int portfd,  unsigned char *pTrack1, int *pInOutLen1,
                              unsigned char *pTrack2, int *pInOutLen2, unsigned char *pTrack3, int *pInOutLen3);

// 29. 清除磁道数据缓存,缓存模式下使用(被动模式)
typedef int (*mClearTrackCache)(int portfd);

// 30. 等待刷卡,读取磁道信息,主动上传模式下使用(主动模式)
typedef int (*mReadTrackAuto)(int portfd, unsigned char *pTrack1, int *pInOutLen1,
                              unsigned char *pTrack2, int *pInOutLen2, unsigned char *pTrack3, int *pInOutLen3,
                              int iTimeout);

#ifdef __cplusplus
}
#endif //__cplusplus


/**************************************************************************
* 命令编辑、发送接收等处理
***************************************************************************/
class CDevImpl_WBCS10 : public CLogManage
{
public:
    CDevImpl_WBCS10();
    CDevImpl_WBCS10(LPSTR lpLog);
    virtual ~CDevImpl_WBCS10();
public:
    BOOL    IsDeviceExist(STDEVICEOPENMODE stDevExist, BOOL bAddLog = TRUE);   // 检查指定设备是否存在/连接
    BOOL    OpenDevice(STDEVICEOPENMODE stOpenMode);       // 打开指定设备
    BOOL    CloseDevice();
    BOOL    IsDeviceOpen();
    BOOL    GetFWVersion(LPSTR lpFwVer);
    INT     nGetHidPortIdx(LPSTR lpVid, LPSTR lpPid);      // 获取HID设备索引
    INT     GetErrCode();
    LPSTR   GetErrorStr(LONG lErrCode);
    INT     SetReConFlag(BOOL bFlag)
    {
        m_bReCon = bFlag;
    }
    void    SetLibPath(LPCSTR lpPath);                     // 设置动态库路径(DeviceOpen前有效)

public: // 接口函数封装
    BOOL    bGetLibVersion(LPSTR lpVersion);            // 1. 库版本信息模块，获取版本信息函数
    //mListPortSPath            ListPortSPath;    		// 2. 枚举系统中所有串口
    //mListPort                 ListPort;         		// 3. 列出操作系统上存在的串口
    UINT    nListHIDPort(LPSTR lpVid, LPSTR lpPid, BOOL bAddLog = TRUE);     // 4. 列出操作系统上连接指定厂商号和产品号的USB HID设备
    //mListHIDPortS             ListHIDPortS;     		// 5. 列出操作系统上连接的USB HID设备
    //mSetMonitorPort           SetMonitorPort;   		// 6. 设置指定串口发送接收数据回调函数,监控数据
    BOOL    bEnableLog(INT nLevel, LPCSTR lpcStrDir);   // 7. 开启日志,写入文件和显示在错误输出流中
    BOOL    bDisableLog();                         		// 8. 禁止日志
    BOOL    bOpenPort(LPCSTR lpDevPath, INT nDevPar);   // 9. 打开指定设备端口
    BOOL    bClosePort();                               // 10. 关闭指定设备端口
    BOOL    bCancelPort(INT nCancel = 1);               // 11. 取消正在执行操作
    LPCSTR  lpGetPortName();                            // 12. 查询指定设备路径名字
    BOOL    bReset();                                   // 13. 复位设备
    BOOL    bReset(LPSTR lpVersion);                    // 13. 复位设备,获取设备固件版本
    BOOL    bStatus(LPUCHAR lpFormat, LPUCHAR lpTrack, LPUCHAR lpyEnable);// 14. 获取设备状态配置信息
    BOOL    bSetup(UCHAR ucFormat, UCHAR ucTrack);      // 15. 设置设备配置
    BOOL    bDisable();                                 // 16. 禁止刷卡,刷卡无效
    //mSetBPS                   SetBPS;           		// 17. 设置串口设备通讯波特率
    //mSetUSBKB                 SetUSBKB;         		// 18. 设置USB键盘模式和磁道数据结尾是否添加回车符'CR'
    //mSetLEDMode               SetLEDMode;       		// 19. 设置LED模式
    //mSetDeviceLED             SetDeviceLED;     		// 20. 控制设备LED
    //mSetBuzzerMode            SetBuzzerMode;    		// 21. 设置蜂鸣器模式
    //mSetDeviceBuzzer          SetDeviceBuzzer;  		// 22. 控制设备蜂鸣器Buzzer
    BOOL    bSetDeviceCtrlMode(UCHAR ucMode);           // 23. 设置读取磁卡操作模式
    //mGetDeviceCtrlMode        GetDeviceCtrlMode;		// 24. 获取读取磁卡操作模式
    BOOL    bEnable(BOOL bFaildCon, LPUCHAR lpFormat,
                    LPUCHAR lpTrack1, LPINT lpInOutLen1,
                    LPUCHAR lpTrack2, LPINT lpInOutLen2,
                    LPUCHAR lpTrack3, LPINT lpInOutLen3, INT nTimeOut);// 25. 允许刷磁卡,等待刷卡读取磁道数据,命令模式下使用(被动模式)
    BOOL    bResend(LPUCHAR lpFormat,
                    LPUCHAR lpTrack1, LPINT lpInOutLen1,
                    LPUCHAR lpTrack2, LPINT lpInOutLen2,
                    LPUCHAR lpTrack3, LPINT lpInOutLen3);// 26. 重新读取上次刷卡有效数据,指令模式下使用(被动模式)
    BOOL    bEnableSwipingCard();		// 27. 允许刷磁卡,缓存模式下使用(被动模式)
    BOOL    bGetTrackCache(LPUCHAR lpTrack1, LPINT lpInOutLen1,
                           LPUCHAR lpTrack2, LPINT lpInOutLen2,
                           LPUCHAR lpTrack3, LPINT lpInOutLen3);// 28. 获取磁道数据,缓存模式下使用(被动模式)
    BOOL    bClearTrackCache();  		// 29. 清除磁道数据缓存,缓存模式下使用(被动模式)
    BOOL    bReadTrackAuto(LPUCHAR lpTrack1, LPINT lpInOutLen1,
                           LPUCHAR lpTrack2, LPINT lpInOutLen2,
                           LPUCHAR lpTrack3, LPINT lpInOutLen3, INT nTimeOu);// 30. 等待刷卡,读取磁道信息,主动上传模式下使用(主动模式)

private:
    DEVICEID        m_DeviceId;
    BOOL            m_bDevOpenOk;
    CHAR            m_szErrStr[1024];
    CHAR            m_szFWVer[256];
    BOOL            m_bReCon;          // 是否断线重连状态
    INT             m_nRetErrOLD[8];   // 处理错误值保存

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
    INT         m_nDevErrCode;


private: // 动态库接口定义
    mGetLibVersion            GetLibVersion;    		// 1. 库版本信息模块，获取版本信息函数
    mListPortSPath            ListPortSPath;    		// 2. 枚举系统中所有串口
    mListPort                 ListPort;         		// 3. 列出操作系统上存在的串口
    mListHIDPort              ListHIDPort;      		// 4. 列出操作系统上连接指定厂商号和产品号的USB HID设备
    mListHIDPortS             ListHIDPortS;     		// 5. 列出操作系统上连接的USB HID设备
    mSetMonitorPort           SetMonitorPort;   		// 6. 设置指定串口发送接收数据回调函数,监控数据
    mEnableLog                EnableLog;        		// 7. 开启日志,写入文件和显示在错误输出流中
    mDisableLog               DisableLog;       		// 8. 禁止日志
    mOpenPort                 OpenPort;         		// 9. 打开指定设备端口
    mClosePort                ClosePort;        		// 10. 关闭指定设备端口
    mCancelPort               CancelPort;       		// 11. 取消正在执行操作
    mGetPortName              GetPortName;      		// 12 查询指定设备路径名字
    mReset                    Reset;            		// 13. 复位设备,获取设备固件版本
    mStatus                   Status;           		// 14. 获取设备状态配置信息
    mSetup                    Setup;            		// 15. 设置设备配置
    mDisable                  Disable;          		// 16. 禁止刷卡,刷卡无效
    mSetBPS                   SetBPS;           		// 17. 设置串口设备通讯波特率
    mSetUSBKB                 SetUSBKB;         		// 18. 设置USB键盘模式和磁道数据结尾是否添加回车符'CR'
    mSetLEDMode               SetLEDMode;       		// 19. 设置LED模式
    mSetDeviceLED             SetDeviceLED;     		// 20. 控制设备LED
    mSetBuzzerMode            SetBuzzerMode;    		// 21. 设置蜂鸣器模式
    mSetDeviceBuzzer          SetDeviceBuzzer;  		// 22. 控制设备蜂鸣器Buzzer
    mSetDeviceCtrlMode        SetDeviceCtrlMode;		// 23. 设置读取磁卡操作模式
    mGetDeviceCtrlMode        GetDeviceCtrlMode;		// 24. 获取读取磁卡操作模式
    mEnable                   Enable;           		// 25. 允许刷磁卡,等待刷卡读取磁道数据,命令模式下使用(被动模式)
    mResend                   Resend;           		// 26. 重新读取上次刷卡有效数据,指令模式下使用(被动模式)
    mEnableSwipingCard        EnableSwipingCard;		// 27. 允许刷磁卡,缓存模式下使用(被动模式)
    mGetTrackCache            GetTrackCache;    		// 28. 获取磁道数据,缓存模式下使用(被动模式)
    mClearTrackCache          ClearTrackCache;  		// 29. 清除磁道数据缓存,缓存模式下使用(被动模式)
    mReadTrackAuto            ReadTrackAuto;    		// 30. 等待刷卡,读取磁道信息,主动上传模式下使用(主动模式)
};


#endif // DEVIMPL_WBCS10_H
