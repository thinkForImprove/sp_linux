/***************************************************************
* 文件名称：DevImpl_WBT2000.cpp
* 文件描述：刷折模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_WBT2000.h"
#include <unistd.h>

static const char *ThisFile = "DevImpl_WBT2000.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[7] != IMP_ERR_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertCode_Impl2Str(IMP_ERR_NOTOPEN)); \
        } \
        m_nRetErrOLD[7] = IMP_ERR_NOTOPEN; \
        return IMP_ERR_NOTOPEN; \
    }

CDevImpl_WBT2000::CDevImpl_WBT2000()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_WBT2000::CDevImpl_WBT2000(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_WBT2000::Init()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_DeviceId = 0;
    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    memset(m_szFWVer, 0x00, sizeof(m_szFWVer));

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("MSR/WBT2000/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());//strDllName.data());
    //m_LoadLibrary.setFileName(m_szLoadDllPath);//strDllName);
    m_bLoadIntfFail = TRUE;

    m_bReCon = FALSE;                       // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    memset(m_stPortList, 0x00, sizeof(STPORTNAME) * 128);
    m_wPortListCnt = 0;

    // 动态库接口函数初始化
    vInitLibFunc();
}

CDevImpl_WBT2000::~CDevImpl_WBT2000()
{
    vUnLoadLibrary();
}

BOOL CDevImpl_WBT2000::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

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

void CDevImpl_WBT2000::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
        vInitLibFunc();
    }
}

BOOL CDevImpl_WBT2000::bLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1. 库版本信息模块，获取版本信息函数
    LOAD_LIBINFO_FUNC(mWBTGetLibVersion, GetLibVersion, "GetLibVersion");

    // 2. 枚举系统中所有串口
    LOAD_LIBINFO_FUNC(mWBTListPortSPath, ListPortSPath, "ListPortSPath");

    // 3. 列出操作系统上存在的串口
    LOAD_LIBINFO_FUNC(mWBTListPort, ListPort, "ListPort");

    // 4. 设置指定串口发送接收数据回调函数,监控数据
    LOAD_LIBINFO_FUNC(mWBTSetMonitorPort, SetMonitorPort, "SetMonitorPort");

    // 5. 开启日志,写入文件和显示在错误输出流中
    LOAD_LIBINFO_FUNC(mWBTEnableLog, EnableLog, "EnableLog");

    // 6. 获取日志等级
    LOAD_LIBINFO_FUNC(mWBTGetLogLevel, GetLogLevel, "GetLogLevel");

    // 7. 禁止日志功能
    LOAD_LIBINFO_FUNC(mWBTDisableLog, DisableLog, "DisableLog");

    // 8. 打开指定设备端口
    LOAD_LIBINFO_FUNC(mWBTOpenPort, OpenPort, "OpenPort");

    // 9. 关闭指定设备端口
    LOAD_LIBINFO_FUNC(mWBTClosePort, ClosePort, "ClosePort");

    // 10. 取消正在执行操作
    LOAD_LIBINFO_FUNC(mWBTCancelPort, CancelPort, "CancelPort");

    // 11 查询指定设备路径名字
    LOAD_LIBINFO_FUNC(mWBTGetPortName, GetPortName, "GetPortName");

    // 12. 软复位
    LOAD_LIBINFO_FUNC(mWBTReset, Reset, "Reset");

    // 13. 获取固件版本
    LOAD_LIBINFO_FUNC(mWBTGetFirmVer, GetFirmVer, "GetFirmVer");

    // 14. 设置写磁道模式
    LOAD_LIBINFO_FUNC(mWBTSetHiLoCo, SetHiLoCo, "SetHiLoCo");

    // 15. 设置读磁道模式
    LOAD_LIBINFO_FUNC(mWBTSetReadMGCardMode, SetReadMGCardMode, "SetReadMGCardMode");

    // 16. 设置蜂鸣器Buzzer模式
    LOAD_LIBINFO_FUNC(mWBTSetBuzzerMode, SetBuzzerMode, "SetBuzzerMode");

    // 17. 获取蜂鸣器Buzzer模式
    LOAD_LIBINFO_FUNC(mWBTGetBuzzerMode, GetBuzzerMode, "GetBuzzerMode");

    // 18. 控制蜂鸣器鸣响次数
    LOAD_LIBINFO_FUNC(mWBTSetBuzzer, SetBuzzer, "SetBuzzer");

    // 19. 设置串口设备通讯波特率
    LOAD_LIBINFO_FUNC(mWBTSetBPS, SetBPS, "SetBPS");

    // 20. 发送命令读取磁道数据(ASCII)
    LOAD_LIBINFO_FUNC(mWBTReadMGCard, ReadMGCard, "ReadMGCard");

    // 21. 自动读取磁道数据(ASCII)
    LOAD_LIBINFO_FUNC(mWBTAutoReadMGCard, AutoReadMGCard, "AutoReadMGCard");

    // 22. 写磁道
    LOAD_LIBINFO_FUNC(mWBTWriteMGCard, WriteMGCard, "WriteMGCard");

    // 23. 清除磁道
    LOAD_LIBINFO_FUNC(mWBTEraseMGCard, EraseMGCard, "EraseMGCard");

    // 24. 取消正在进行的磁道读写操作
    LOAD_LIBINFO_FUNC(mWBTCancelMGCard, CancelMGCard, "CancelMGCard");

    return TRUE;
}

void CDevImpl_WBT2000::vInitLibFunc()
{
    GetLibVersion = nullptr;            // 1. 库版本信息模块，获取版本信息函数
    ListPortSPath = nullptr;    		// 2. 枚举系统中所有串口
    ListPort = nullptr;         		// 3. 列出操作系统上存在的串口
    SetMonitorPort = nullptr;   		// 4. 设置指定串口发送接收数据回调函数,监控数据
    EnableLog = nullptr;        		// 5. 开启日志,写入文件和显示在错误输出流中
    GetLogLevel = nullptr;              // 6. 获取日志等级
    DisableLog = nullptr;       		// 7. 禁止日志功能
    OpenPort = nullptr;         		// 8. 打开指定设备端口
    ClosePort = nullptr;        		// 9. 关闭指定设备端口
    CancelPort = nullptr;       		// 10. 取消正在执行操作
    GetPortName = nullptr;      		// 11 查询指定设备路径名字
    Reset = nullptr;                    // 12. 软复位
    GetFirmVer = nullptr;               // 13. 获取固件版本
    SetHiLoCo = nullptr;                // 14. 设置写磁道模式
    SetReadMGCardMode = nullptr;        // 15. 设置读磁道模式
    SetBuzzerMode = nullptr;            // 16. 设置蜂鸣器Buzzer模式
    GetBuzzerMode = nullptr;            // 17. 获取蜂鸣器Buzzer模式
    SetBuzzer = nullptr;                // 18. 控制蜂鸣器鸣响次数
    SetBPS = nullptr;                   // 19. 设置串口设备通讯波特率
    ReadMGCard = nullptr;               // 20. 发送命令读取磁道数据(ASCII)
    AutoReadMGCard = nullptr;           // 21. 自动读取磁道数据(ASCII)
    WriteMGCard = nullptr;              // 22. 写磁道
    EraseMGCard = nullptr;              // 23. 清除磁道
    CancelMGCard = nullptr;             // 24. 取消正在进行的磁道读写操作
}

//------------------------------------------------------------------------------------------------------
// 检查指定设备是否存在
// 返回值: -1动态库加载失败; -2串口不存在; -3枚举串口为0; -4无效的连接类型
//        1枚举串口成功,指定串口路径为空
INT CDevImpl_WBT2000::IsDeviceExist(STDEVICEOPENMODE stDevExis, BOOL bAddLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            if (bAddLog == TRUE)
            {
                Log(ThisModule, __LINE__, "加载动态库: OpenDevice()->bLoadLibrary() fail. , Return: -1");
            }
            return -1;
        }
    }

    if (stDevExis.wOpenMode == 0)   // 串口
    {        
        memset(m_stPortList, 0x00, sizeof(STPORTNAME) * 128);
        m_wPortListCnt = ListPort(m_stPortList, 128);     // 3. 列出操作系统上存在的串口
        if (m_wPortListCnt > 0)
        {
            // 串口列表输出
            if (bAddLog == TRUE)
            {
                std::string stdOut = "枚举串口列表如下: ";
                for(int i = 0; i < m_wPortListCnt; i ++)
                {
                    stdOut.append(m_stPortList[i].path);
                    stdOut.append("|");
                }
                Log(ThisModule, __LINE__, "%s", stdOut.c_str());
            }

            // 指定串口路径为空
            if (strlen(stDevExis.szDevPath) < 1)
            {
                if (bAddLog == TRUE)
                {
                    Log(ThisModule, __LINE__,
                        "检查指定设备是否存在: 指定串口[%s]为空, Return: 1", stDevExis.szDevPath);
                }
                return 1;
            }

            // 检查串口是否存在
            for(int i = 0; i < m_wPortListCnt; i ++)
            {
                if (MCMP_IS0(m_stPortList[i].path, stDevExis.szDevPath))
                {
                    return 0;
                }
            }
            if (bAddLog == TRUE)
            {
                Log(ThisModule, __LINE__,
                    "检查指定设备是否存在: IsDeviceExist() fail. 指定串口[%s]不存在, Return: -2", stDevExis.szDevPath);
            }
            return -2;
        } else
        {
            if (bAddLog == TRUE)
            {
                Log(ThisModule, __LINE__,
                    "检查指定设备是否存在: IsDeviceExist() -> ListPort() fail. 列出操作系统上存在的串口数<1, "
                    "nCount = %d, Return: -3", m_wPortListCnt);
            }
            return -3;
        }
    } else
    {
        if (bAddLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "检查指定设备是否存在: IsDeviceExist() fail. 无效的连接方式, stDevExis.wOpenMode = %d, Return: -4",
                stDevExis.wOpenMode);
        }
        return -4;
    }

    return TRUE;
}

// 打开指定设备
INT CDevImpl_WBT2000::OpenDevice(STDEVICEOPENMODE stOpenMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    //如果已打开，则关闭
    if (m_bDevOpenOk == TRUE)
    {
        CloseDevice();
    }

    m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            Log(ThisModule, __LINE__, "加载动态库: OpenDevice()->bLoadLibrary() fail. ");
            return FALSE;
        }
    }

    // 打开设备
    if (stOpenMode.wOpenMode == 0)   // 串口
    {
        if (strlen(stOpenMode.szDevPath) < 1)
        {
            m_DeviceId = OpenPort(nullptr, 0);
            if (m_DeviceId <= 0)
            {
                Log(ThisModule, __LINE__,
                    "打开指定设备端口(串口): 未指定串口路径: ->OpenPort(%s, %d) fail. DeviceId: %d, Return: %s",
                    nullptr, 0, m_DeviceId, ConvertCode_Impl2Str(IMP_ERR_NOTOPEN));
                return IMP_ERR_NOTOPEN;
            } else
            {
                Log(ThisModule, __LINE__,
                    "打开指定设备端口(串口): 未指定串口路径: ->OpenPort(%s, %d) Succ. DeviceId: %d",
                    nullptr, 0, m_DeviceId);
            }
        } else
        {
            m_DeviceId = OpenPort(stOpenMode.szDevPath, stOpenMode.wBaudRate);
            if (m_DeviceId <= 0)
            {
                Log(ThisModule, __LINE__, "打开指定设备端口(串口): ->OpenPort(%s, %d) fail. DeviceId: %d, Return: %s",
                    stOpenMode.szDevPath, stOpenMode.wBaudRate, m_DeviceId, ConvertCode_Impl2Str(IMP_ERR_NOTOPEN));
                return IMP_ERR_NOTOPEN;
            } else
            {
                Log(ThisModule, __LINE__, "打开指定设备端口(串口): ->OpenPort(%s, %d) Succ. DeviceId: %d",
                    stOpenMode.szDevPath, stOpenMode.wBaudRate, m_DeviceId);
            }
        }
    } else
    {
        Log(ThisModule, __LINE__, "打开设备: OpenDevice() fail. 无效的连接方式, stDevExis.wOpenMode = %d|0串口,1USB HID",
            stOpenMode.wOpenMode);
        return IMP_ERR_NOTOPEN;
    }

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    //bSetDeviceCtrlMode(1);  // 初始读卡操作模式(指令模式被动)
    //bSetup(0, 0);           // 初始设置不读磁
    //bDisable();             // 初始设置禁止刷卡

    m_bReCon = FALSE;                       // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

INT CDevImpl_WBT2000::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 关闭设备
    if (m_DeviceId > 0)
    {
        ClosePort(m_DeviceId);
    }

    if (m_bReCon == FALSE)  // 非断线重连时关闭需释放动态库
    {
        // 释放动态库
        vUnLoadLibrary();
    }

    // 设备Open标记=F
    m_bDevOpenOk = FALSE;

    if (m_bReCon == FALSE)
    {
        Log(ThisModule, __LINE__, "设备关闭,释放动态库: Close Device(%d), unLoad Library.", m_DeviceId);
    }

    return IMP_SUCCESS;
}

BOOL CDevImpl_WBT2000::IsDeviceOpen()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 设置动态库路径(DeviceOpen前有效)
void CDevImpl_WBT2000::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, __LINE__, "入参[%s]无效,不设置动态库路径.");
        return;
    }

    if (lpPath[0] == '/')   // 绝对路径
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s", lpPath);
    } else
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s%s", LINUXPATHLIB, lpPath);
    }

    Log(ThisModule, __LINE__, "设置动态库路径=<%s>.", m_szLoadDllPath);
}


//------封装动态库接口------------------------------------------------------------------------------------
// 1. 库版本信息模块，获取版本信息函数
INT CDevImpl_WBT2000::nGetLibVersion(LPSTR lpVersion)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    LPSTR lpVer = (LPSTR)GetLibVersion();
    memcpy(lpVersion, lpVer, strlen(lpVer));
    Log(ThisModule, __LINE__, "获取库版本: GetLibVersion() = %s. ", lpVersion);

    return IMP_SUCCESS;
}

// 5. 开启日志功能
INT CDevImpl_WBT2000::nEnableLog(INT nLevel)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    EnableLog(nLevel);
    Log(ThisModule, __LINE__, "开启日志功能: EnableLog(%d) 无返回值. ", nLevel);
    return IMP_SUCCESS;
}

// 7. 禁止日志功能
INT CDevImpl_WBT2000::nDisableLog()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;
    DisableLog();
    Log(ThisModule, __LINE__, "禁止日志功能: DisableLog() succ. ");
    return IMP_SUCCESS;
}

// 10. 取消正在执行操作
INT CDevImpl_WBT2000::nCancelPort(INT nCancel)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    CancelPort(m_DeviceId, nCancel);
    Log(ThisModule, __LINE__, "取消正在执行操作: CancelPort(%d, %d). ", m_DeviceId, nCancel);
    return IMP_SUCCESS;
}

// 11. 查询指定设备路径名字
INT CDevImpl_WBT2000::nGetPortName(LPSTR lpPortName, WORD wSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    LPSTR lpStr = (LPSTR)GetPortName(m_DeviceId);
    if (lpStr == nullptr)
    {
        Log(ThisModule, __LINE__, "查询指定设备路径名字: GetPortName(%d) = nullptr. ", m_DeviceId);
        return IMP_ERR_NOTOPEN;
    }
    memcpy(lpPortName, lpStr, strlen(lpStr) > wSize ? wSize : strlen(lpStr));

    return IMP_SUCCESS;
}

// 12. 复位设备
INT CDevImpl_WBT2000::nReset()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = Reset(m_DeviceId);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "复位设备: Reset(%d) fail. Return: %s",
            m_DeviceId, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 13. 获取固件版本
BOOL CDevImpl_WBT2000::nGetFWVersion(LPSTR lpFwVer)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = GetFirmVer(m_DeviceId, lpFwVer);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取固件版本: GetFirmVer(%d, %s) fail. Return: %s",
            m_DeviceId, lpFwVer, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 20. 发送命令读取磁道数据(ASCII)
INT CDevImpl_WBT2000::nReadMGCard(UCHAR lpFormat, LPUCHAR lpTrack1,
                                  LPINT lpInOutLen1, LPUCHAR lpTrack2,
                                  LPINT lpInOutLen2, LPUCHAR lpTrack3,
                                  LPINT lpInOutLen3, INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = ReadMGCard(m_DeviceId, lpFormat, lpTrack1, lpInOutLen1,
                      lpTrack2,  lpInOutLen2, lpTrack3, lpInOutLen3, nTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "发送命令读取磁道数据(ASCII): ReadMGCard(%d, %02X, %s, %d, %s, %d, %s, %d, %d) fail. Return: %s",
            m_DeviceId, lpFormat, lpTrack1, *lpInOutLen1, lpTrack2, *lpInOutLen2,
            lpTrack3, *lpInOutLen3, nTimeOut, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

LPSTR CDevImpl_WBT2000::ConvertCode_Impl2Str(INT nErrCode)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nErrCode)
    {
        case IMP_ERR_LOAD_LIB:          /* 1  */
                sprintf(m_szErrStr, "%d|%s", nErrCode, "动态库加载失败");
                break;
        case IMP_ERR_PARAM_INVALID:     /* 2  */
                sprintf(m_szErrStr, "%d|%s", nErrCode, "参数无效");
                break;
        case IMP_ERR_UNKNOWN:           /* 3  */
                sprintf(m_szErrStr, "%d|%s", nErrCode, "未知错误");
                break;
        case IMP_ERR_NOTOPEN:           /* 4  */
                sprintf(m_szErrStr, "%d|%s", nErrCode, "设备未Open");
                break;
        // <0: Device返回
        case IMP_DEV_ERR_SUCCESS:       /* 0  */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "command execute ok,命令执行成功");
            break;
        case IMP_DEV_ERR_FAIL:          /* -1 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "command execute error,命令执行失败");
            break;
        case IMP_DEV_ERR_PARAMATER:     /* -2 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "API parameter error,API参数错误");
            break;
        case IMP_DEV_ERR_WRITE:         /* -3 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "write data error,写数据出错");
            break;
        case IMP_DEV_ERR_WTIMEOUT:      /* -4 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "write data timeout,写数据超时");
            break;
        case IMP_DEV_ERR_READ:          /* -5 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "read data error,读数据出错");
            break;
        case IMP_DEV_ERR_RTIMEOUT:      /* -6 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "read data timeout,读数据超时");
            break;
        case IMP_DEV_ERR_FRAME:         /* -7 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "data not conform to protocol,数据不符合协议");
            break;
        case IMP_DEV_ERR_UNKNOWN:       /* -8 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "unknown error,未知错误");
            break;
        case IMP_DEV_ERR_NOSUPP:        /* -9 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "not support command,不支持命令");
            break;
        case IMP_DEV_ERR_CANCEL:        /* -10 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "command cancel,命令取消");
            break;
        case IMP_DEV_ERR_RUNNING:       /* -11 */
            sprintf(m_szErrStr, "%d|%s", nErrCode, "command in progress,命令运行中");
            break;
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }


    return (LPSTR)m_szErrStr;
}

//---------------------------------------------
