/***************************************************************
* 文件名称：DevImpl_TMZ.cpp
* 文件描述：天梦者非接模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_TMZ.h"
#include "QtTypeDef.h"
#include "QtDLLLoader.h"

static const char *ThisFile = "DevImpl_WBT2000.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[4] != IMP_ERR_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertCode_Impl2Str(IMP_ERR_NOTOPEN)); \
        } \
        m_nRetErrOLD[4] = IMP_ERR_NOTOPEN; \
        return IMP_ERR_NOTOPEN; \
    }

CDevImpl_TMZ::CDevImpl_TMZ()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_TMZ::CDevImpl_TMZ(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_TMZ::Init()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_lDevHandle = 0;                                       // 设备句柄
    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    memset(m_szFWVer, 0x00, sizeof(m_szFWVer));

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("FIDC/TMZ/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_bLoadIntfFail = TRUE;

    m_bReCon = FALSE;                       // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    // 动态库接口函数初始化
    vInitLibFunc();
}

CDevImpl_TMZ::~CDevImpl_TMZ()
{
    vUnLoadLibrary();
}

BOOL CDevImpl_TMZ::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库<%s> fail. ReturnCode:%s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
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
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail. ReturnCode:%s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_TMZ::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
        vInitLibFunc();
    }
}

BOOL CDevImpl_TMZ::bLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1. 打开USB设备
    LOAD_LIBINFO_FUNC(FNICReaderOpenUsbByFD, ICReaderOpenUsbByFD, "ICReaderOpenUsbByFD");

    // 2. 关闭设备
    LOAD_LIBINFO_FUNC(FNICReaderClose, ICReaderClose, "ICReaderClose");

    // 3. 获取固件版本
    LOAD_LIBINFO_FUNC(FNICReaderGetVer, ICReaderGetVer, "ICReaderGetVer");

    // 4. 设置鸣响
    LOAD_LIBINFO_FUNC(FNICReaderBeep, ICReaderBeep, "ICReaderBeep");

    // 5. 获取设备状态
    LOAD_LIBINFO_FUNC(FNICReaderDevStatus, ICReaderDevStatus, "ICReaderDevStatus");

    // 6. 取卡片状态
    LOAD_LIBINFO_FUNC(FNGetCardState, GetCardState, "GetCardState");

    // 7. 卡上电激活
    LOAD_LIBINFO_FUNC(FNCPUPowerOn, CPUPowerOn, "CPUPowerOn");

    // 8. 卡断电
    LOAD_LIBINFO_FUNC(FNCPUPowerOff, CPUPowerOff, "CPUPowerOff");

    // 9. 发送APDU
    LOAD_LIBINFO_FUNC(FNCPUCardAPDU, CPUCardAPDU, "CPUCardAPDU");

    // 10. LED灯控制
    LOAD_LIBINFO_FUNC(FNICReaderLEDCtrl, ICReaderLEDCtrl, "ICReaderLEDCtrl");

    return TRUE;
}
void CDevImpl_TMZ::vInitLibFunc()
{
    ICReaderOpenUsbByFD = nullptr;        // 1. 打开USB设备
    ICReaderClose = nullptr;              // 2. 关闭设备
    ICReaderGetVer = nullptr;             // 3. 获取固件版本
    ICReaderBeep = nullptr;               // 4. 设置鸣响
    ICReaderDevStatus = nullptr;          // 5. 获取设备状态
    GetCardState = nullptr;               // 6. 取卡片状态
    CPUPowerOn = nullptr;                 // 7. 卡上电激活
    CPUPowerOff = nullptr;                // 8. 卡断电
    CPUCardAPDU = nullptr;                // 9. 发送APDU
    ICReaderLEDCtrl = nullptr;            // 10. LED灯控制
}

//------------------------------------------------------------------------------------------------------
// 打开指定设备
INT CDevImpl_TMZ::OpenDevice()
{
    return OpenDevice(nullptr);
}

// 打开指定设备(指定参数)
INT CDevImpl_TMZ::OpenDevice(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 如果已打开，则关闭
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
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                    ConvertCode_Impl2Str(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    // 打开设备
    m_lDevHandle = ICReaderOpenUsbByFD(0);
    if (m_lDevHandle < 0)
    {
        if (m_nRetErrOLD[1] != IMP_ERR_NOTOPEN)
        {
            Log(ThisModule, __LINE__, "打开设备: ->ICReaderOpenUsbByFD(%ld) Fail. DeviceId: %d, Return: %s",
                0, m_lDevHandle, ConvertCode_Impl2Str(IMP_ERR_NOTOPEN));
            m_nRetErrOLD[1] = IMP_ERR_NOTOPEN;
        }
        return IMP_ERR_NOTOPEN;
    } else
    {
        Log(ThisModule, __LINE__, "打开设备: ->ICReaderOpenUsbByFD(%ld) Succ. DeviceId: %d",
            0, m_lDevHandle);
    }

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }
    m_bReCon = FALSE; // 是否断线重连状态: 初始F

    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

INT CDevImpl_TMZ::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 关闭设备
    if (m_lDevHandle > 0)
    {
        ICReaderClose(m_lDevHandle);
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
        Log(ThisModule, __LINE__, "设备关闭,释放动态库: Close Device(%d), unLoad Library.", m_lDevHandle);
    }

    return IMP_SUCCESS;
}

BOOL CDevImpl_TMZ::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

//------封装动态库接口------------------------------------------------------------------------------------
// 3. 获取固件版本
INT CDevImpl_TMZ::GetFWVersion(LPSTR lpFwVer)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = ICReaderGetVer(m_lDevHandle, lpFwVer);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取固件版本: ICReaderGetVer(%ld, %s) fail. Return: %s",
            m_lDevHandle, lpFwVer, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 4. 设置鸣响
INT CDevImpl_TMZ::SetDeviceBeep(UCHAR ucMsec, UCHAR ucDelay, UCHAR ucNum)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = ICReaderBeep(m_lDevHandle, ucMsec, ucDelay, ucNum);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置鸣响: ICReaderBeep(%ld, %d, %d, %d) fail. Return: %s",
            m_lDevHandle, ucMsec, ucDelay, ucNum, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 5. 获取设备状态
INT CDevImpl_TMZ::GetDeviceStatus(UCHAR &ucStat)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = ICReaderDevStatus(m_lDevHandle, &ucStat);
    if (nRet != IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[2])
        {
            Log(ThisModule, __LINE__, "获取设备状态: ICReaderDevStatus(%ld, %d) fail. Return: %s",
                m_lDevHandle, ucStat, ConvertCode_Impl2Str(nRet));
            m_nRetErrOLD[2] = nRet;
        }
        return nRet;
    }

    return IMP_SUCCESS;
}

// 6. 取卡片状态
INT CDevImpl_TMZ::GetCardStatus(INT &nStat, UCHAR ucSlot)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = GetCardState(m_lDevHandle, ucSlot, &nStat);
    if (nRet != IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[3])
        {
            Log(ThisModule, __LINE__, "取卡片状态: GetCardState(%ld, %d) fail. Return: %s",
                m_lDevHandle, nStat, ConvertCode_Impl2Str(nRet));
            m_nRetErrOLD[3] = nRet;
        }
        return nRet;
    }

    return IMP_SUCCESS;
}

// 7. 卡上电激活
INT CDevImpl_TMZ::SetCPUPowerOn(UCHAR ucSlot, INT nTimeOut, UCHAR &ucType,
                                LPUCHAR lpSnrData, UCHAR &ucSnrLen,
                                LPUCHAR lpData, INT &nDataLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CPUPowerOn(m_lDevHandle, ucSlot, nTimeOut, &ucType, &ucSnrLen, lpSnrData, &nDataLen, lpData);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "卡上电激活: CPUPowerOn(%ld, %d, %d) fail. Return: %s",
            m_lDevHandle, ucSlot, nTimeOut, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 8. 卡断电
INT CDevImpl_TMZ::SetCPUPowerOff(UCHAR ucSlot)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CPUPowerOff(m_lDevHandle, ucSlot);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "卡断电: CPUPowerOff(%ld, %d) fail. Return: %s",
            m_lDevHandle, ucSlot, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 9. 发送APDU
INT CDevImpl_TMZ::SendCPUCardAPDU(UCHAR ucSlot, LPUCHAR lpSndData, INT nSndLen,
                                  LPUCHAR lpRcvData, INT &nRcvLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CPUCardAPDU(m_lDevHandle, ucSlot, nSndLen, lpSndData, &nRcvLen, lpRcvData);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "发送APDU: CPUCardAPDU(%ld, %d, %d, %s) fail. Return: %s",
            m_lDevHandle, ucSlot, nSndLen, lpSndData, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 10. LED灯控制
INT CDevImpl_TMZ::SetLEDCtrl(UCHAR ucCtrl, UCHAR ucDelay)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = ICReaderLEDCtrl(m_lDevHandle, ucCtrl, ucDelay);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "LED灯控制: ICReaderLEDCtrl(%ld, %d, %d) fail. Return: %s",
            m_lDevHandle, ucCtrl, ucDelay, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

//----------------------------------对外参数设置接口----------------------------------
// 设置断线重连标记
INT CDevImpl_TMZ::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

// 设置动态库路径(DeviceOpen前有效)
INT CDevImpl_TMZ::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, __LINE__, "入参[%s]无效,不设置动态库路径.");
        return IMP_SUCCESS;
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

    return IMP_SUCCESS;
}

LPSTR CDevImpl_TMZ::ConvertCode_Impl2Str(INT nErrCode)
{
/*#define Convert_code_str(RET, STR) \
    sprintf(m_szErrStr, "%d|%s", RET, STR); \
    return m_szErrStr;*/


    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nErrCode)
    {
        case IMP_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "成功");
            break;
        case IMP_ERR_LOAD_LIB:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "动态库加载失败");
            break;
        case IMP_ERR_PARAM_INVALID:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "参数无效");
            break;
        case IMP_ERR_UNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "未知错误");
            break;
        case IMP_ERR_NOTOPEN:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设备未Open");
            break;
        // <0: Device返回
        case IMP_DEV_ERR_F31:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "取消PIN输入");
            break;
        case IMP_DEV_ERR_F32:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "键盘超时");
            break;
        case IMP_DEV_ERR_F33:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "输入密码长度错误");
            break;
        case IMP_DEV_ERR_F85:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "键盘不支持");
            break;
        case IMP_DEV_ERR_F86:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "键盘返回数据格式错误");
            break;
        case IMP_DEV_ERR_F97:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "无效句柄");
            break;
        case IMP_DEV_ERR_F98:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设备异常断开");
            break;
        case IMP_DEV_ERR_F99:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "获取设备状态参数错误");
            break;
        case IMP_DEV_ERR_F100:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设置设备状态参数错误");
            break;
        case IMP_DEV_ERR_F101:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设置通讯事件错误");
            break;
        case IMP_DEV_ERR_F113:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "读数据错误");
            break;
        case IMP_DEV_ERR_F114:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "写数据错误");
            break;
        case IMP_DEV_ERR_F115:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "命令头错误");
            break;
        case IMP_DEV_ERR_F116:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "命令尾错误");
            break;
        case IMP_DEV_ERR_F117:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "数据错位");
            break;
        case IMP_DEV_ERR_F118:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "校验位错误");
            break;
        case IMP_DEV_ERR_F119:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "超时错误(上层软件超时返回，没有等待到硬件返回数据)");
            break;
        case IMP_DEV_ERR_F129:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "数据分配空间错误(内存错误)");
            break;
        case IMP_DEV_ERR_F130:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "长度错误");
            break;
        case IMP_DEV_ERR_F131:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "传入数据格式错误");
            break;
        case IMP_DEV_ERR_F144:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设备不支持该操作(动态库未加载)");
            break;
        case IMP_DEV_ERR_F145:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "二代证错误");
            break;
        case IMP_DEV_ERR_F146:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "无权限(一般为文件操作，可能是设置的路径问题)");
            break;
        case IMP_DEV_ERR_F147:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "解码库加载失败");
            break;
        case IMP_DEV_ERR_F148:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "身份证解码错误");
            break;
        case IMP_DEV_ERR_F149:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "其他错误");
            break;
        case IMP_DEV_ERR_F161:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "T57卡交互数据异常");
            break;
        case IMP_DEV_ERR_F162:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "无T57卡");
            break;
        case IMP_DEV_ERR_F163:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "T57卡操作作卡片数据出现错误无回应");
            break;
        case IMP_DEV_ERR_F164:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "T57卡参数设置失败");
            break;
        case IMP_DEV_ERR_F165:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "T57卡密码认证没通过");
            break;
        case IMP_DEV_ERR_F4097:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设备功能不支持或参数不支持");
            break;
        case IMP_DEV_ERR_F4098:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "命令执行错误");
            break;
        case IMP_DEV_ERR_F8193:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "写EEPROM失败");
            break;
        case IMP_DEV_ERR_F8194:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "读EEPROM失败");
            break;
        case IMP_DEV_ERR_F12289:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "不支持卡类型");
            break;
        case IMP_DEV_ERR_F12290:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "无卡");
            break;
        case IMP_DEV_ERR_F12291:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "有卡已上电");
            break;
        case IMP_DEV_ERR_F12292:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "有卡未上电(或非接有卡状态)");
            break;
        case IMP_DEV_ERR_F12293:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "卡上电失败");
            break;
        case IMP_DEV_ERR_F12294:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "操作卡片数据无回应,超时(接触式存储卡无响应)");
            break;
        case IMP_DEV_ERR_F12295:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "操作卡片数据出现错误");
            break;
        case IMP_DEV_ERR_F12296:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "非接卡Halt失败");
            break;
        case IMP_DEV_ERR_F12297:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "多张非接卡");
            break;
        case IMP_DEV_ERR_F16386:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "设备底层超时未响应返回");
            break;
        case IMP_DEV_ERR_F20481:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "磁头未开启");
            break;
        case IMP_DEV_ERR_F20482:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "刷卡失败");
            break;
        case IMP_DEV_ERR_F20483:
            sprintf(m_szErrStr, "%d|%s", nErrCode, "刷卡超时");
            break;
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}

