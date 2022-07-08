#include "DevImpl_WBCS10.h"

static const char *ThisFile = "DevImpl_WBCS10.cpp";

CDevImpl_WBCS10::CDevImpl_WBCS10()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_WBCS10::CDevImpl_WBCS10(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_WBCS10::Init()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_DeviceId = 0;
    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    memset(m_szFWVer, 0x00, sizeof(m_szFWVer));

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    //strDllName.prepend("MSR/WBCS10/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());//strDllName.data());
    m_LoadLibrary.setFileName(m_szLoadDllPath);//strDllName);
    m_bLoadIntfFail = TRUE;

    m_nDevErrCode = APICMDSuccess;

    // 动态库接口函数初始化
    GetLibVersion = nullptr;    		// 1. 库版本信息模块，获取版本信息函数
    ListPortSPath = nullptr;    		// 2. 枚举系统中所有串口
    ListPort = nullptr;         		// 3. 列出操作系统上存在的串口
    ListHIDPort = nullptr;      		// 4. 列出操作系统上连接指定厂商号和产品号的USB HID设备
    ListHIDPortS = nullptr;     		// 5. 列出操作系统上连接的USB HID设备
    SetMonitorPort = nullptr;   		// 6. 设置指定串口发送接收数据回调函数,监控数据
    EnableLog = nullptr;        		// 7. 开启日志,写入文件和显示在错误输出流中
    DisableLog = nullptr;       		// 8. 禁止日志
    OpenPort = nullptr;         		// 9. 打开指定设备端口
    ClosePort = nullptr;        		// 10. 关闭指定设备端口
    CancelPort = nullptr;       		// 11. 取消正在执行操作
    GetPortName = nullptr;      		// 12 查询指定设备路径名字
    Reset = nullptr;            		// 13. 复位设备,获取设备固件版本
    Status = nullptr;           		// 14. 获取设备状态配置信息
    Setup = nullptr;            		// 15. 设置设备配置
    Disable = nullptr;          		// 16. 禁止刷卡,刷卡无效
    SetBPS = nullptr;           		// 17. 设置串口设备通讯波特率
    SetUSBKB = nullptr;         		// 18. 设置USB键盘模式和磁道数据结尾是否添加回车符'CR'
    SetLEDMode = nullptr;       		// 19. 设置LED模式
    SetDeviceLED = nullptr;     		// 20. 控制设备LED
    SetBuzzerMode = nullptr;    		// 21. 设置蜂鸣器模式
    SetDeviceBuzzer = nullptr;  		// 22. 控制设备蜂鸣器Buzzer
    SetDeviceCtrlMode = nullptr;		// 23. 设置读取磁卡操作模式
    GetDeviceCtrlMode = nullptr;		// 24. 获取读取磁卡操作模式
    Enable = nullptr;           		// 25. 允许刷磁卡,等待刷卡读取磁道数据,命令模式下使用(被动模式)
    Resend = nullptr;           		// 26. 重新读取上次刷卡有效数据,指令模式下使用(被动模式)
    EnableSwipingCard = nullptr;		// 27. 允许刷磁卡,缓存模式下使用(被动模式)
    GetTrackCache = nullptr;    		// 28. 获取磁道数据,缓存模式下使用(被动模式)
    ClearTrackCache = nullptr;  		// 29. 清除磁道数据缓存,缓存模式下使用(被动模式)
    ReadTrackAuto = nullptr;    		// 30. 等待刷卡,读取磁道信息,主动上传模式下使用(主动模式)

    // 加载设备动态库
    if (bLoadLibrary() != TRUE)
    {
        Log(ThisModule, __LINE__, "加载动态库: OpenDevice()->bLoadLibrary() fail. ");
    }
}

CDevImpl_WBCS10::~CDevImpl_WBCS10()
{
    vUnLoadLibrary();
}

BOOL CDevImpl_WBCS10::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_WBCS10::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_WBCS10::bLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1. 库版本信息模块，获取版本信息函数
    GetLibVersion = (mGetLibVersion)m_LoadLibrary.resolve("GetLibVersion");
    FUNC_POINTER_ERROR_RETURN(GetLibVersion, "GetLibVersion");

    // 2. 枚举系统中所有串口
    ListPortSPath = (mListPortSPath)m_LoadLibrary.resolve("ListPortSPath");
    FUNC_POINTER_ERROR_RETURN(ListPortSPath, "ListPortSPath");
    // 3. 列出操作系统上存在的串口
    ListPort = (mListPort)m_LoadLibrary.resolve("ListPort");
    FUNC_POINTER_ERROR_RETURN(ListPort, "ListPort");

    // 4. 列出操作系统上连接指定厂商号和产品号的USB HID设备
    ListHIDPort = (mListHIDPort)m_LoadLibrary.resolve("ListHIDPort");
    FUNC_POINTER_ERROR_RETURN(ListHIDPort, "ListHIDPort");

    // 5. 列出操作系统上连接的USB HID设备
    ListHIDPortS = (mListHIDPortS)m_LoadLibrary.resolve("ListHIDPortS");
    FUNC_POINTER_ERROR_RETURN(ListHIDPortS, "ListHIDPortS");

    // 6. 设置指定串口发送接收数据回调函数,监控数据
    SetMonitorPort = (mSetMonitorPort)m_LoadLibrary.resolve("SetMonitorPort");
    FUNC_POINTER_ERROR_RETURN(SetMonitorPort, "SetMonitorPort");

    // 7. 开启日志,写入文件和显示在错误输出流中
    EnableLog = (mEnableLog)m_LoadLibrary.resolve("EnableLog");
    FUNC_POINTER_ERROR_RETURN(EnableLog, "EnableLog");

    // 8. 禁止日志
    DisableLog = (mDisableLog)m_LoadLibrary.resolve("DisableLog");
    FUNC_POINTER_ERROR_RETURN(DisableLog, "DisableLog");

    // 9. 打开指定设备端口
    OpenPort = (mOpenPort)m_LoadLibrary.resolve("OpenPort");
    FUNC_POINTER_ERROR_RETURN(OpenPort, "OpenPort");

    // 10. 关闭指定设备端口
    ClosePort = (mClosePort)m_LoadLibrary.resolve("ClosePort");
    FUNC_POINTER_ERROR_RETURN(ClosePort, "ClosePort");

    // 11. 取消正在执行操作
    CancelPort = (mCancelPort)m_LoadLibrary.resolve("CancelPort");
    FUNC_POINTER_ERROR_RETURN(CancelPort, "CancelPort");

    // 12 查询指定设备路径名字
    GetPortName = (mGetPortName)m_LoadLibrary.resolve("GetPortName");
    FUNC_POINTER_ERROR_RETURN(GetPortName, "GetPortName");

    // 13. 复位设备,获取设备固件版本
    Reset = (mReset)m_LoadLibrary.resolve("Reset");
    FUNC_POINTER_ERROR_RETURN(Reset, "Reset");

    // 14. 获取设备状态配置信息
    Status = (mStatus)m_LoadLibrary.resolve("Status");
    FUNC_POINTER_ERROR_RETURN(Status, "Status");

    // 15. 设置设备配置
    Setup = (mSetup)m_LoadLibrary.resolve("Setup");
    FUNC_POINTER_ERROR_RETURN(Setup, "Setup");

    // 16. 禁止刷卡,刷卡无效
    Disable = (mDisable)m_LoadLibrary.resolve("Disable");
    FUNC_POINTER_ERROR_RETURN(Disable, "Disable");

    // 17. 设置串口设备通讯波特率
    SetBPS = (mSetBPS)m_LoadLibrary.resolve("SetBPS");
    FUNC_POINTER_ERROR_RETURN(SetBPS, "SetBPS");

    // 18. 设置USB键盘模式和磁道数据结尾是否添加回车符'CR'
    SetUSBKB = (mSetUSBKB)m_LoadLibrary.resolve("SetUSBKB");
    FUNC_POINTER_ERROR_RETURN(SetUSBKB, "SetUSBKB");

    // 19. 设置LED模式
    SetLEDMode = (mSetLEDMode)m_LoadLibrary.resolve("SetLEDMode");
    FUNC_POINTER_ERROR_RETURN(SetLEDMode, "SetLEDMode");

    // 20. 控制设备LED
    SetDeviceLED = (mSetDeviceLED)m_LoadLibrary.resolve("SetDeviceLED");
    FUNC_POINTER_ERROR_RETURN(SetDeviceLED, "SetDeviceLED");

    // 21. 设置蜂鸣器模式
    SetBuzzerMode = (mSetBuzzerMode)m_LoadLibrary.resolve("SetBuzzerMode");
    FUNC_POINTER_ERROR_RETURN(SetBuzzerMode, "SetBuzzerMode");

    // 22. 控制设备蜂鸣器Buzzer
    SetDeviceBuzzer = (mSetDeviceBuzzer)m_LoadLibrary.resolve("SetDeviceBuzzer");
    FUNC_POINTER_ERROR_RETURN(SetDeviceBuzzer, "SetDeviceBuzzer");

    // 23. 设置读取磁卡操作模式
    SetDeviceCtrlMode = (mSetDeviceCtrlMode)m_LoadLibrary.resolve("SetDeviceCtrlMode");
    FUNC_POINTER_ERROR_RETURN(SetDeviceCtrlMode, "SetDeviceCtrlMode");

    // 24. 获取读取磁卡操作模式
    GetDeviceCtrlMode = (mGetDeviceCtrlMode)m_LoadLibrary.resolve("GetDeviceCtrlMode");
    FUNC_POINTER_ERROR_RETURN(GetDeviceCtrlMode, "GetDeviceCtrlMode");

    // 25. 允许刷磁卡,等待刷卡读取磁道数据,命令模式下使用(被动模式)
    Enable = (mEnable)m_LoadLibrary.resolve("Enable");
    FUNC_POINTER_ERROR_RETURN(Enable, "Enable");

    // 26. 重新读取上次刷卡有效数据,指令模式下使用(被动模式)
    Resend = (mResend)m_LoadLibrary.resolve("Resend");
    FUNC_POINTER_ERROR_RETURN(Resend, "Resend");

    // 27. 允许刷磁卡,缓存模式下使用(被动模式)
    EnableSwipingCard = (mEnableSwipingCard)m_LoadLibrary.resolve("EnableSwipingCard");
    FUNC_POINTER_ERROR_RETURN(EnableSwipingCard, "EnableSwipingCard");

    // 28. 获取磁道数据,缓存模式下使用(被动模式)
    GetTrackCache = (mGetTrackCache)m_LoadLibrary.resolve("GetTrackCache");
    FUNC_POINTER_ERROR_RETURN(GetTrackCache, "GetTrackCache");

    // 29. 清除磁道数据缓存,缓存模式下使用(被动模式)
    ClearTrackCache = (mClearTrackCache)m_LoadLibrary.resolve("ClearTrackCache");
    FUNC_POINTER_ERROR_RETURN(ClearTrackCache, "ClearTrackCache");

    // 30. 等待刷卡,读取磁道信息,主动上传模式下使用(主动模式)
    ReadTrackAuto = (mReadTrackAuto)m_LoadLibrary.resolve("ReadTrackAuto");
    FUNC_POINTER_ERROR_RETURN(ReadTrackAuto, "ReadTrackAuto");

    return TRUE;
}

//------------------------------------------------------------------------------------------------------
// 检查指定设备是否存在
BOOL CDevImpl_WBCS10::IsDeviceExist(STDEVICEOPENMODE stDevExis, BOOL bAddLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    int nCount = 0;

    if (stDevExis.wOpenMode == 0)   // 串口
    {
        struct port_name stPortList[128];
        memset(stPortList, 0x00, sizeof(struct port_name) * 128);
        nCount = ListPort(stPortList, 128);     // 3. 列出操作系统上存在的串口
        if (nCount > 0)
        {
            // 检查串口是否存在
            for(int i = 0; i < nCount; i ++)
            {
                if (memcmp(stPortList[i].path, stDevExis.szDevPath, strlen(stDevExis.szDevPath)) == 0 &&
                    memcmp(stPortList[i].path, stDevExis.szDevPath, strlen(stPortList[i].path)) == 0)
                {
                    return TRUE;
                }
            }
            if (bAddLog == TRUE)
            {
                Log(ThisModule, __LINE__,
                    "检查指定设备是否存在: IsDeviceExist() fail. 指定串口[%s]不存在", stDevExis.szDevPath);
            }
            return FALSE;
        } else
        {
            if (bAddLog == TRUE)
            {
                Log(ThisModule, __LINE__,
                    "检查指定设备是否存在: IsDeviceExist() -> ListPort() fail. 列出操作系统上存在的串口数<1, nCount = %d ", nCount);
            }
            return FALSE;
        }

    } else
    if (stDevExis.wOpenMode == 1)   // USB HID
    {
        nCount = nListHIDPort(stDevExis.szHidVid, stDevExis.szHidPid, bAddLog);      		// 4. 列出操作系统上连接指定厂商号和产品号的USB HID设备
        if (nCount < 1)
        {
            if (bAddLog == TRUE)
            {
                Log(ThisModule, __LINE__,
                    "检查指定设备是否存在: IsDeviceExist() -> ListHIDPort() fail. 指定[VID=%s,PID=%s]的USB HID设备<1, nCount = %d ",
                    stDevExis.szHidVid, stDevExis.szHidPid, nCount);
            }
            return FALSE;
        }
    } else
    {
        if (bAddLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "检查指定设备是否存在: IsDeviceExist() fail. 无效的连接方式, stDevExis.wOpenMode = %d", stDevExis.wOpenMode);
        }
        return FALSE;
    }

    return TRUE;
}

// 打开指定设备
BOOL CDevImpl_WBCS10::OpenDevice(STDEVICEOPENMODE stOpenMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    //如果已打开，则关闭
    if (m_bDevOpenOk == TRUE)
    {
        CloseDevice();
    }

    m_bDevOpenOk = FALSE;

    // 加载设备动态库
    if (bLoadLibrary() != TRUE)
    {
        Log(ThisModule, __LINE__, "加载动态库: OpenDevice()->bLoadLibrary() fail. ");
        return FALSE;
    }

    // 打开设备
    if (stOpenMode.wOpenMode == 0)   // 串口
    {
        if (bOpenPort(stOpenMode.szDevPath, stOpenMode.wBaudRate) != TRUE)
        {
            Log(ThisModule, __LINE__, "打开串口设备: OpenDevice()->bOpenPort(%s, %d) fail. ", stOpenMode.szDevPath, stOpenMode.wBaudRate);
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "打开串口设备: OpenDevice()->bOpenPort(%s, %d) succ. ", stOpenMode.szDevPath, stOpenMode.wBaudRate);
        }
    } else
    if (stOpenMode.wOpenMode == 1)   // USB HID
    {
        int nIdx = nGetHidPortIdx(stOpenMode.szHidVid, stOpenMode.szHidPid);
        if (bOpenPort(stOpenMode.szDevPath, nIdx) != TRUE)
        {
            Log(ThisModule, __LINE__, "打开USB设备: OpenDevice()->bOpenPort(%s, %d) fail. ", stOpenMode.szDevPath, nIdx);
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "打开USB设备: OpenDevice()->bOpenPort(%s, %d) succ. ", stOpenMode.szDevPath, nIdx);
        }
    } else
    {
        Log(ThisModule, __LINE__, "打开设备: OpenDevice() fail. 无效的连接方式, stDevExis.wOpenMode = %d|0串口,1USB HID", stOpenMode.wOpenMode);
        return FALSE;
    }

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    if (bSetDeviceCtrlMode(1) != TRUE)
    {
        Log(ThisModule, __LINE__, "设置读卡操作模式: OpenDevice()->bSetDeviceCtrlMode(%d) fail.", 1);
        return FALSE;
    } else
    {
        Log(ThisModule, __LINE__, "设置读卡操作模式: OpenDevice()->bSetDeviceCtrlMode(%d) succ.", 1);
    }

    if (bSetup(0, 7) != TRUE)
    {
        Log(ThisModule, __LINE__, "设置磁道数据模式: OpenDevice()->bSetup(%d, %d) fail.", 0, 7);
        return FALSE;
    } else
    {
        Log(ThisModule, __LINE__, "设置磁道数据模式: OpenDevice()->bSetup(%d, %d) succ.", 0, 7);
    }

    if (bDisable() != TRUE)
    {
        Log(ThisModule, __LINE__, "初始设置禁止刷卡: OpenDevice()->bDisable() fail.", 0, 7);
        return FALSE;
    } else
    {
        Log(ThisModule, __LINE__, "设置磁道数据模式: OpenDevice()->bDisable() succ.", 0, 7);
    }

    return TRUE;
}

BOOL CDevImpl_WBCS10::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 关闭设备
    ClosePort(m_DeviceId);

    // 释放动态库
    vUnLoadLibrary();

    // 设备Open标记=F
    m_bDevOpenOk = FALSE;

    Log(ThisModule, __LINE__, "设备关闭,释放动态库: Close Device(%d), unLoad Library.", m_DeviceId);

    return TRUE;
}

BOOL CDevImpl_WBCS10::IsDeviceOpen()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisModule, __LINE__, "IsDeviceOpen(): Device Not Open.");
    }

    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

BOOL CDevImpl_WBCS10::GetFWVersion(LPSTR lpFwVer)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "获取固件版本: GetFWVersion() Device Not Open.");
        return FALSE;
    }

    if (strlen(m_szFWVer) < 1)
    {
        if (bReset() != TRUE)
        {
            Log(ThisModule, __LINE__, "获取固件版本: GetFWVersion()->bReset() fail.");
            return FALSE;
        }
    }

    memcpy(lpFwVer, m_szFWVer, strlen(m_szFWVer));
    return TRUE;
}

// 获取HID设备索引
INT  CDevImpl_WBCS10::nGetHidPortIdx(LPSTR lpVid, LPSTR lpPid)
{
    int nIdx = 0;

    return nIdx;
}

INT CDevImpl_WBCS10::GetErrCode()
{
    return m_nDevErrCode;
}


//------封装动态库接口------------------------------------------------------------------------------------
// 1. 库版本信息模块，获取版本信息函数
BOOL CDevImpl_WBCS10::bGetLibVersion(LPSTR lpVersion)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "复位设备: bGetLibVersion() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    lpVersion = (LPSTR)GetLibVersion();
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "复位设备: bGetLibVersion()->GetLibVersion() fail. Return Version: %s",
            lpVersion);
        return FALSE;
    }

    return TRUE;
}

// 4. 列出操作系统上连接指定厂商号和产品号的USB HID设备
UINT CDevImpl_WBCS10::nListHIDPort(LPSTR lpVid, LPSTR lpPid, BOOL bAddLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_nDevErrCode = APICMDSuccess;
    USHORT usVid = 0;
    USHORT usPid = 0;
    bool bOk = false;
    UINT uiCount = 0;

    usVid = QString(lpVid).toInt(&bOk, 16);
    if (bOk != true)
    {
        m_nDevErrCode = APICMDParmError;
        if (bAddLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "枚举指定USB HID设备数目: nListHIDPort() VID HEX Char(%s) -> USHORT fail. ReturnCode:%s",
                lpVid, GetErrorStr(m_nDevErrCode));
        }
        return 0;
    }

    usPid = QString(lpPid).toInt(&bOk, 16);
    if (bOk != true)
    {
        m_nDevErrCode = APICMDParmError;
        if (bAddLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "枚举指定USB HID设备数目: nListHIDPort() PID HEX Char(%s) -> USHORT fail. ReturnCode:%s",
                lpPid, GetErrorStr(m_nDevErrCode));
        }
        return 0;
    }

    uiCount = ListHIDPort(usVid, usPid);
    if (bAddLog == TRUE)
    {
        Log(ThisModule, __LINE__,
            "枚举指定USB HID设备数目: nListHIDPort()->ListHIDPort(%s|%d, %s|%d) . Return Count:%d",
            lpVid, usVid, lpPid, usPid, uiCount);
    }

    return uiCount;
}

// 7. 开启日志,写入文件和显示在错误输出流中
BOOL CDevImpl_WBCS10::bEnableLog(INT nLevel, LPCSTR lpcStrDir)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_nDevErrCode = APICMDSuccess;
    if (EnableLog(nLevel, lpcStrDir) != TRUE)
    {
        Log(ThisModule, __LINE__, "开启日志,写入文件和显示在错误输出流中: bEnableLog()->EnableLog(%d, %s) fail. ",
            nLevel, lpcStrDir);
        return FALSE;
    }

    Log(ThisModule, __LINE__, "开启日志,写入文件和显示在错误输出流中: bEnableLog()->EnableLog(%d, %s) succ. ",
        nLevel, lpcStrDir);


    return TRUE;
}

// 8. 禁止日志
BOOL CDevImpl_WBCS10::bDisableLog()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_nDevErrCode = APICMDSuccess;
    DisableLog();
    Log(ThisModule, __LINE__, "禁止日志: bDisableLog()->DisableLog() succ. ");
    return TRUE;
}

// 9. 打开指定设备端口
BOOL CDevImpl_WBCS10::bOpenPort(LPCSTR lpDevPath, INT nDevPar)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_nDevErrCode = APICMDSuccess;
    m_DeviceId = 0;
    if (strlen(lpDevPath) < 1)
    {
        m_DeviceId = OpenPort(nullptr, nDevPar);
    } else
    {
        m_DeviceId = OpenPort(lpDevPath, nDevPar);
    }

    if (m_DeviceId <= 0)
    {
        Log(ThisModule, __LINE__, "打开指定设备端口: bOpenPort()->OpenPort(%s, %d) fail. ReturnCode:%s",
            strlen(lpDevPath) < 0 ? nullptr : lpDevPath, nDevPar, GetErrorStr(m_DeviceId));
        return FALSE;
    }
    Log(ThisModule, __LINE__, "打开指定设备端口: bOpenPort()->OpenPort(%s, %d) succ. DeviceId = %d", lpDevPath, nDevPar, m_DeviceId);

    return TRUE;
}

// 10. 关闭指定设备端口
BOOL CDevImpl_WBCS10::bClosePort()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_nDevErrCode = APICMDSuccess;
    if (ClosePort(m_DeviceId) == FALSE)
    {
        Log(ThisModule, __LINE__, "关闭指定设备端口: bClosePort()->ClosePort(%d) fail. ", m_DeviceId);
        return FALSE;
    }
    Log(ThisModule, __LINE__, "关闭指定设备端口: bClosePort()->ClosePort(%d) succ. ", m_DeviceId);

    return TRUE;
}

// 11. 取消正在执行操作
BOOL CDevImpl_WBCS10::bCancelPort(INT nCancel)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "取消正在执行操作: vCancelPort() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    CancelPort(m_DeviceId, nCancel);
    Log(ThisModule, __LINE__, "取消正在执行操作: vCancelPort()->CancelPort(%d, %d). ", m_DeviceId, nCancel);
    return TRUE;
}

// 12. 查询指定设备路径名字
LPCSTR CDevImpl_WBCS10::lpGetPortName()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "查询指定设备路径名字: bGetPortName() Device Not Open.");
        return FALSE;
    }

    LPCSTR lpcRtnStr = nullptr;
    lpcRtnStr = GetPortName(m_DeviceId);
    Log(ThisModule, __LINE__, "查询指定设备路径名字: bGetPortName()->GetPortName(%d). ReturnStr: %s", m_DeviceId, lpcRtnStr);
    return lpcRtnStr;
}

// 13. 复位设备
BOOL CDevImpl_WBCS10::bReset()
{
    CHAR szVer[128] = { 0x00 };
    return bReset(szVer);
}

// 13. 复位设备,获取设备固件版本
BOOL CDevImpl_WBCS10::bReset(LPSTR lpVersion)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "复位设备: bReset() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = Reset(m_DeviceId, lpVersion);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "复位设备: bReset()->Reset(%d) fail. ReturnCode:%s",
            m_DeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }
    memset(m_szFWVer, 0x00, sizeof(m_szFWVer));
    memcpy(m_szFWVer, lpVersion, strlen(lpVersion));

    return TRUE;
}

// 14. 获取设备状态配置信息
BOOL CDevImpl_WBCS10::bStatus(LPUCHAR lpFormat, LPUCHAR lpTrack, LPUCHAR lpEnable)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "获取设备状态配置信息: bStatus() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = Status(m_DeviceId, lpFormat, lpTrack, lpEnable);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "获取设备状态配置信息: bStatus()->Status(%d, %02X, %02X, %02X) fail. ReturnCode:%s",
            m_DeviceId, *lpFormat, *lpTrack, *lpEnable, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 15. 设置设备配置
BOOL CDevImpl_WBCS10::bSetup(UCHAR ucFormat, UCHAR ucTrack)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "设置设备配置: bSetup() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = Setup(m_DeviceId, ucFormat, ucTrack);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "设置设备配置: bSetup()->Setup(%d, %02X, %02X) fail. ReturnCode:%s",
            m_DeviceId, ucFormat, ucTrack, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 16. 禁止刷卡,刷卡无效
BOOL CDevImpl_WBCS10::bDisable()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "禁止刷卡: bDisable() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = Disable(m_DeviceId);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "禁止刷卡: bDisable()->bDisable(%d) fail. ReturnCode:%s",
            m_DeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 23. 设置读取磁卡操作模式
BOOL CDevImpl_WBCS10::bSetDeviceCtrlMode(UCHAR ucMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "设置设备配置: bSetDeviceCtrlMode() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = SetDeviceCtrlMode(m_DeviceId, ucMode);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "设置设备配置: bSetDeviceCtrlMode()->SetDeviceCtrlMode(%d, %d) fail. ReturnCode:%s",
            m_DeviceId, ucMode, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 25. 允许刷磁卡,等待刷卡读取磁道数据,命令模式下使用(被动模式)
BOOL CDevImpl_WBCS10::bEnable(BOOL bFaildCon, LPUCHAR lpFormat,
                               LPUCHAR lpTrack1, LPINT lpInOutLen1,
                               LPUCHAR lpTrack2, LPINT lpInOutLen2,
                               LPUCHAR lpTrack3, LPINT lpInOutLen3, INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "允许刷磁卡,等待刷卡读取磁道数据,命令模式下使用(被动模式): bEnable() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = Enable(m_DeviceId, bFaildCon, lpFormat, lpTrack1, lpInOutLen1, lpTrack2,  lpInOutLen2, lpTrack3, lpInOutLen3, nTimeOut);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "允许刷磁卡,等待刷卡读取磁道数据,命令模式下使用(被动模式): bEnable()->Enable(%d, %d, %02X, %s, %d, %s, %d, %s, %d, %d) fail. ReturnCode:%s",
            m_DeviceId, bFaildCon, lpFormat, lpTrack1, *lpInOutLen1, lpTrack2, *lpInOutLen2, lpTrack3, *lpInOutLen3, nTimeOut, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 26. 重新读取上次刷卡有效数据,指令模式下使用(被动模式)
BOOL CDevImpl_WBCS10::bResend(LPUCHAR lpFormat,
                               LPUCHAR lpTrack1, LPINT lpInOutLen1,
                               LPUCHAR lpTrack2, LPINT lpInOutLen2,
                               LPUCHAR lpTrack3, LPINT lpInOutLen3)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "重新读取上次刷卡有效数据,指令模式下使用(被动模式): bResend() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = Resend(m_DeviceId, lpFormat, lpTrack1, lpInOutLen1, lpTrack2,  lpInOutLen2, lpTrack3, lpInOutLen3);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "重新读取上次刷卡有效数据,指令模式下使用(被动模式): bResend()->Resend(%d, %02X, %s, %d, %s, %d, %s, %d) fail. ReturnCode:%s",
            m_DeviceId, lpFormat, lpTrack1, *lpInOutLen1, lpTrack2, *lpInOutLen2, lpTrack3, *lpInOutLen3, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 27. 允许刷磁卡,缓存模式下使用(被动模式)
BOOL CDevImpl_WBCS10::bEnableSwipingCard()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "允许刷磁卡,缓存模式下使用(被动模式): bEnableSwipingCard() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = EnableSwipingCard(m_DeviceId);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "允许刷磁卡,缓存模式下使用(被动模式): bEnableSwipingCard()->EnableSwipingCard(%d) fail. ReturnCode:%s",
            m_DeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 28. 获取磁道数据,缓存模式下使用(被动模式)
BOOL CDevImpl_WBCS10::bGetTrackCache(LPUCHAR lpTrack1, LPINT lpInOutLen1,
                                      LPUCHAR lpTrack2, LPINT lpInOutLen2,
                                      LPUCHAR lpTrack3, LPINT lpInOutLen3)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "获取磁道数据,缓存模式下使用(被动模式): bGetTrackCache() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = GetTrackCache(m_DeviceId, lpTrack1, lpInOutLen1, lpTrack2,  lpInOutLen2, lpTrack3, lpInOutLen3);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "获取磁道数据,缓存模式下使用(被动模式): bGetTrackCache()->GetTrackCache(%d, , %s, %d, %s, %d, %s, %d) fail. ReturnCode:%s",
            m_DeviceId, lpTrack1, *lpInOutLen1, lpTrack2, *lpInOutLen2, lpTrack3, *lpInOutLen3, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 29. 清除磁道数据缓存,缓存模式下使用(被动模式)
BOOL CDevImpl_WBCS10::bClearTrackCache()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "清除磁道数据缓存,缓存模式下使用(被动模式): bClearTrackCache() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = ClearTrackCache(m_DeviceId);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "清除磁道数据缓存,缓存模式下使用(被动模式): bClearTrackCache()->ClearTrackCache(%d) fail. ReturnCode:%s",
            m_DeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 30. 等待刷卡,读取磁道信息,主动上传模式下使用(主动模式)
BOOL CDevImpl_WBCS10::bReadTrackAuto(LPUCHAR lpTrack1, LPINT lpInOutLen1,
                                      LPUCHAR lpTrack2, LPINT lpInOutLen2,
                                      LPUCHAR lpTrack3, LPINT lpInOutLen3, INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "等待刷卡,读取磁道信息: bReadTrackAuto() Device Not Open.");
        return FALSE;
    }

    m_nDevErrCode = APICMDSuccess;
    m_nDevErrCode = ReadTrackAuto(m_DeviceId, lpTrack1, lpInOutLen1, lpTrack2,  lpInOutLen2, lpTrack3, lpInOutLen3, nTimeOut);
    if (m_nDevErrCode != 0)
    {
        Log(ThisModule, __LINE__, "等待刷卡,读取磁道信息: bReadTrackAuto()->ReadTrackAuto(%d, %s, %d, %s, %d, %s, %d, %d) fail. ReturnCode:%s",
            m_DeviceId, lpTrack1, *lpInOutLen1, lpTrack2, *lpInOutLen2, lpTrack3, *lpInOutLen3, nTimeOut, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

LPSTR CDevImpl_WBCS10::GetErrorStr(LONG lErrCode)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(lErrCode)
    {
        case APICMDSuccess:     /* 0  */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "command execute ok,命令执行成功");
            break;
        case APICMDFailure:     /* -1 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "command execute error,命令执行失败");
            break;
        case APICMDParmError:   /* -2 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "API parameter error,API参数错误");
            break;
        case APIWriteError:     /* -3 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "write data error,写数据出错");
            break;
        case APIWriteTimeOut:   /* -4 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "write data timeout,写数据超时");
            break;
        case APIReadError:      /* -5 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "read data error,读数据出错");
            break;
        case APIReadTimeOut:    /* -6 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "read data timeout,读数据超时");
            break;
        case APIFrameError:     /* -7 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "data not conform to protocol,数据不符合协议");
            break;
        case APIUnknownError:   /* -8 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "unknown error,未知错误");
            break;
        case APINotSupportCMD:  /* -9 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "not support command,不支持命令");
            break;
        case APICMDCancel:      /* -10 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "command cancel,命令取消");
            break;
        case APICMDInProgress:  /* -11 */
            sprintf(m_szErrStr, "%d|%s", lErrCode, "command in progress,命令运行中");
            break;
        default :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}

//---------------------------------------------
