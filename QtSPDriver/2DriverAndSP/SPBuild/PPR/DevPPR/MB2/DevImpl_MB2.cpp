/***************************************************************
* 文件名称：DevImpl_MB2.cpp
* 文件描述：MB2存折打印模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_MB2.h"
#include "../XFS_PPR/def.h"

static const char *ThisFile = "DevImpl_MB2.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[7] != IMP_ERR_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertErrCodeToStr(IMP_ERR_NOTOPEN)); \
        } \
        m_nRetErrOLD[7] = IMP_ERR_NOTOPEN; \
        return IMP_ERR_NOTOPEN; \
    }

//----------------------------------构造/析构/初始化----------------------------------
CDevImpl_MB2::CDevImpl_MB2()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_MB2::CDevImpl_MB2(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_MB2::CDevImpl_MB2(LPSTR lpLog, LPSTR lpDevStr)
{
    SetLogFile(lpLog, ThisFile, lpDevStr);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_MB2::~CDevImpl_MB2()
{
    DeviceClose();
}

// 参数初始化
void CDevImpl_MB2::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    m_bDevOpenOk = FALSE;

    //m_hBTNHDll = NULL;
    m_hPrinter = NULL;
    m_bDevOpenOk = FALSE;
    //ClearErrorCode(&m_errorInfo);

    m_szDevStataOLD = 0x00;                                 // 上一次获取设备状态保留
    m_bReCon = FALSE;                                       // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));
    MCPY_NOLEN(m_szOpenMode, "USB");                        // 打开模式(缺省USB)
    m_nBaudRate = 9600;                                     // 波特率(缺省9600)

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("PPR/MB2/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_bLoadIntfFail = TRUE;

    // 动态库接口函数初始化
    PRxOpen = nullptr;                    // 1. 打开PR通信端口连接
    PRxClose = nullptr;                   // 2. 断开PR通信连接
    PRxResetInit = nullptr;               // 3. 打印机复位初始化
    PRxCleanError = nullptr;              // 4. 打印机清错
    PRxGetStatus = nullptr;               // 5. 获取打印机状态
    PRxGetMediaStatus = nullptr;          // 6. 获取设备中介质所处状态
    PRxInsertMedia = nullptr;             // 7. 插入介质控制
    PRxEjectMedia = nullptr;              // 8. 按指定方向退出打印机内部介质
    PRxGetMediaLength = nullptr;          // 9. 获取介质长
    PRxSwallowPassBook = nullptr;         // 10. 吞折
    PRxGetMediaWidth = nullptr;           // 11. 获取介质宽
    PRxGetMediaPosH = nullptr;            // 12. 获取介质距离打印机左边界的水平位置
    PRxBeep = nullptr;                    // 13. 鸣响
    PRxGetFirmwareVersion = nullptr;      // 14. 查询打印机固件版本信息on = nullptr;
    PRxSetPageLength = nullptr;           // 15. 设定页面长度
    PRxPrtWriteData = nullptr;            // 16. 向端口发送16进制数据
    PRxPrtChangeCodeDistance = nullptr;   // 17. 在字符右边加空列
    PRxPrtSelWestCode = nullptr;          // 18. 设定西文字符类型
    PRxPrtFlushPrt = nullptr;             // 19. 刷新打印机缓冲区
    PRxPrtMoveAbsPos = nullptr;           // 20. 移动打印头到指定的绝对位置
    PRxPrtMoveRelPos = nullptr;           // 21. 移动打印头到指定的相对位置
    PRxPrtSetPtrProperty = nullptr;       // 22. 执行指定设置属性
    PRxPrtPrintOCR = nullptr;             // 23. 打印OCR字符
    PRxPrtPrintBarCode = nullptr;         // 24. 打印条码类型
    PRxPrtPrintText = nullptr;            // 25. 打印输出数据,与PrtMoveAbsPos组合运用
    PRxPrtSetFontWidth = nullptr;         // 26. 设定字符宽度
    PRxPrtPrintBmp = nullptr;             // 27. 打印位图
    PRxSetPrintColor = nullptr;           // 28. 打印颜色设置
    PRxMsConfigHMS = nullptr;             // 29. 设置磁道读写的基本参数
    PRxMsWriteEx = nullptr;               // 30. 按预先在Set-UP中选定的标准写磁道
    PRxMsReadEx = nullptr;                // 31. 按预先在Set-UP中选定的标准读磁道
    PRxMsWrite = nullptr;                 // 32. 写磁道
    PRxMsRead = nullptr;                  // 33. 读磁道
    PRxScanSetCtrl = nullptr;             // 34. 设扫描基本控制参数
    PRxScanSetImageCtrl = nullptr;        // 35. 设置图像扫描控制参数
    PRxScan = nullptr;                    // 36. 按指定扫描参数扫描图像
}

//----------------------------------SDK接口加载----------------------------------
// 加载动态库
BOOL CDevImpl_MB2::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库<%s> Fail. ErrCode:%s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> Succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

// 释放动态库
void CDevImpl_MB2::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

// 加载动态库接口函数
BOOL CDevImpl_MB2::bLoadLibIntf()
{
    //1.打开PR通信端口连接
    LOAD_LIBINFO_FUNC(pPRxOpen, PRxOpen, "PRxOpen");
    //2.断开PR通信连接
    LOAD_LIBINFO_FUNC(pPRxClose, PRxClose, "PRxClose");
    //3.打印机复位初始化
    LOAD_LIBINFO_FUNC(pPRxResetInit, PRxResetInit, "PRxResetInit");
    //4.打印机清错
    LOAD_LIBINFO_FUNC(pPRxCleanError, PRxCleanError, "PRxCleanError");
    //5.获取打印机状态
    LOAD_LIBINFO_FUNC(pPRxGetStatus, PRxGetStatus, "PRxGetStatus");
    //6.获取设备中介质所处状态
    LOAD_LIBINFO_FUNC(pPRxGetMediaStatus, PRxGetMediaStatus, "PRxGetMediaStatus");
    //7.插入介质控制
    LOAD_LIBINFO_FUNC(pPRxInsertMedia, PRxInsertMedia, "PRxInsertMedia");
    //8.按指定方向退出打印机内部介质
    LOAD_LIBINFO_FUNC(pPRxEjectMedia, PRxEjectMedia, "PRxEjectMedia");
    //9.获取介质长
    LOAD_LIBINFO_FUNC(pPRxGetMediaLength, PRxGetMediaLength, "PRxGetMediaLength");
    //10.吞折
    LOAD_LIBINFO_FUNC(pPRxSwallowPassBook, PRxSwallowPassBook, "PRxSwallowPassBook");
    //11.获取介质宽
    LOAD_LIBINFO_FUNC(pPRxGetMediaWidth, PRxGetMediaWidth, "PRxGetMediaWidth");
    //12.获取介质距离打印机左边界的水平位置
    LOAD_LIBINFO_FUNC(pPRxGetMediaPosH, PRxGetMediaPosH, "PRxGetMediaPosH");
    //13.鸣响
    LOAD_LIBINFO_FUNC(pPRxBeep, PRxBeep, "PRxBeep");
    //14.查询打印机固件版本信息
    LOAD_LIBINFO_FUNC(pPRxGetFirmwareVersion, PRxGetFirmwareVersion, "PRxGetFirmwareVersion");
    //15.设定页面长度
    LOAD_LIBINFO_FUNC(pPRxSetPageLength, PRxSetPageLength, "PRxSetPageLength");
    //16.向端口发送16进制数据
    LOAD_LIBINFO_FUNC(pPRxPrtWriteData, PRxPrtWriteData, "PRxPrtWriteData");
    //17.在字符右边加空列
    LOAD_LIBINFO_FUNC(pPRxPrtChangeCodeDistance, PRxPrtChangeCodeDistance, "PRxPrtChangeCodeDistance");
    //18.设定西文字符类型
    LOAD_LIBINFO_FUNC(pPRxPrtSelWestCode, PRxPrtSelWestCode, "PRxPrtSelWestCode");
    //19.刷新打印机缓冲区
    LOAD_LIBINFO_FUNC(pPRxPrtFlushPrt, PRxPrtFlushPrt, "PRxPrtFlushPrt");
    //20.移动打印头到指定的绝对位置
    LOAD_LIBINFO_FUNC(pPRxPrtMoveAbsPos, PRxPrtMoveAbsPos, "PRxPrtMoveAbsPos");
    //21.移动打印头到指定的相对位置
    LOAD_LIBINFO_FUNC(pPRxPrtMoveRelPos, PRxPrtMoveRelPos, "PRxPrtMoveRelPos");
    //22.执行指定设置属性
    LOAD_LIBINFO_FUNC(pPRxPrtSetPtrProperty, PRxPrtSetPtrProperty, "PRxPrtSetPtrProperty");
    //23.打印OCR字符
    LOAD_LIBINFO_FUNC(pPRxPrtPrintOCR, PRxPrtPrintOCR, "PRxPrtPrintOCR");
    //24.打印条码类型
    LOAD_LIBINFO_FUNC(pPRxPrtPrintBarCode, PRxPrtPrintBarCode, "PRxPrtPrintBarCode");
    //25.打印输出数据, 与PrtMoveAbsPos组合运用
    LOAD_LIBINFO_FUNC(pPRxPrtPrintText, PRxPrtPrintText, "PRxPrtPrintText");
    //26.设定字符宽度
    LOAD_LIBINFO_FUNC(pPRxPrtSetFontWidth, PRxPrtSetFontWidth, "PRxPrtSetFontWidth");
    //27.打印位图
    LOAD_LIBINFO_FUNC(pPRxPrtPrintBmp, PRxPrtPrintBmp, "PRxPrtPrintBmp");
    //28.打印颜色设置
    //LOAD_LIBINFO_FUNC(pPRxSetPrintColor, PRxSetPrintColor, "PRxSetPrintColor");
    //29.设置磁道读写的基本参数
    LOAD_LIBINFO_FUNC(pPRxMsConfigHMS, PRxMsConfigHMS, "PRxMsConfigHMS");
    //30.按预先在Set-UP中选定的标准写磁道
    LOAD_LIBINFO_FUNC(pPRxMsWriteEx, PRxMsWriteEx, "PRxMsWriteEx");
    //31.按预先在Set-UP中选定的标准读磁道
    LOAD_LIBINFO_FUNC(pPRxMsReadEx, PRxMsReadEx, "PRxMsReadEx");
    //32.写磁道
    LOAD_LIBINFO_FUNC(pPRxMsWrite, PRxMsWrite, "PRxMsWrite");
    //33.读磁道
    LOAD_LIBINFO_FUNC(pPRxMsRead, PRxMsRead, "PRxMsRead");
    //34.设扫描基本控制参数
    LOAD_LIBINFO_FUNC(pPRxScanSetCtrl, PRxScanSetCtrl, "PRxScanSetCtrl");
    //35.设置图像扫描控制参数
    LOAD_LIBINFO_FUNC(pPRxScanSetImageCtrl, PRxScanSetImageCtrl, "PRxScanSetImageCtrl");
    //36.按指定扫描参数扫描图像
    LOAD_LIBINFO_FUNC(pPRxScan, PRxScan, "PRxScan");

    m_bLoadIntfFail = FALSE;

    return TRUE;
}

//----------------------------------SDK封装接口方法----------------------------------
// 打开设备
INT CDevImpl_MB2::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;

    //如果已打开，则关闭
    DeviceClose();

    m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                    ConvertErrCodeToStr(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    // 设备连接
    LONG lRetErrCode = 0;
    m_hPrinter = PRxOpen(m_szOpenMode, 0, 0, nullptr, &lRetErrCode, m_nBaudRate);
    if (m_hPrinter == 0 || lRetErrCode < 0)
    {
        if (lRetErrCode != m_nRetErrOLD[1])
        {
            Log(ThisModule, __LINE__,
                "打开设备: PrxOpen(%s, 0, 0, nullptr, %d, %d) = %d, Failed, ErrCode: %d, Return: %s.",
                m_szOpenMode, lRetErrCode, m_nBaudRate, m_hPrinter,
                lRetErrCode, ConvertErrCodeToStr(lRetErrCode));
            m_nRetErrOLD[1] = lRetErrCode;
        }
        return (INT)lRetErrCode;
    } else
    {
        Log(ThisModule, __LINE__,
            "打开设备: PrxOpen(%s, 0, 0, nullptr, %d, %d) = %d, Success.",
            m_szOpenMode, lRetErrCode, m_nBaudRate, m_hPrinter);
    }

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }

    m_bReCon = FALSE;   // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));
    m_bDevOpenOk = TRUE;

    return IMP_SUCCESS;
}

// 关闭设备
INT CDevImpl_MB2::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    if (m_hPrinter > 0)
    {
        if (PRxClose(m_hPrinter) != 0)
        {
            if (PRxClose(m_hPrinter) != 0)
            {
                PRxClose(m_hPrinter);
            }
        }
    }

    if (m_bReCon != TRUE)   // 断线重连状态时不释放动态库
    {
        vUnLoadLibrary();
    }

    m_bDevOpenOk = FALSE;

    if (m_bReCon != TRUE)   // 断线重连状态时不记录LOG
    {
        Log(ThisModule, __LINE__, "关闭设备: Close Device, unLoad PossdkDll.");
    }

    return IMP_SUCCESS;
}

// 复位设备
INT CDevImpl_MB2::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = PRxResetInit(m_hPrinter);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备复位: PRxResetInit(%d) fail. Return: %s.",
            m_hPrinter, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 4. 打印机清错
INT CDevImpl_MB2::CleanError()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = PRxCleanError(m_hPrinter);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印机清错: PRxCleanError(%d) fail. Return: %s.",
            m_hPrinter, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 获取设备状态
INT CDevImpl_MB2::GetDevStatus(LPSTR lpDevStat)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxGetStatus(m_hPrinter, (LPUCHAR)lpDevStat);
    if (nRet < IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[2])
        {
            Log(ThisModule, __LINE__, "获取设备状态: PRxGetStatus(%d) fail. Return: %s.",
                m_hPrinter, ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[2] = nRet;
        }
        return nRet;
    }

    // 比较两次状态记录LOG

    return nRet;
}

// 获取介质状态
INT CDevImpl_MB2::GetMediaStatus(LPSTR lpDevStat)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxGetMediaStatus(m_hPrinter, (LPUCHAR)lpDevStat);
    if (nRet < IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[3])
        {
            Log(ThisModule, __LINE__, "获取介质状态: PRxGetMediaStatus(%d) fail. Return: %s.",
                m_hPrinter, ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[3] = nRet;
        }
        return nRet;
    }

    // 比较两次状态记录LOG
    if (nRet != m_nRetErrOLD[3])
    {
        Log(ThisModule, __LINE__, "介质状态变化: %d->%d.", m_nRetErrOLD[3], nRet);
        m_nRetErrOLD[3] = nRet;
    }

    return nRet;
}

// 7. 介质吸入
INT CDevImpl_MB2::MediaInsert(DWORD dwInsLen, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxInsertMedia(m_hPrinter, dwInsLen, dwTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[4])
        {
            Log(ThisModule, __LINE__, "介质吸入: PRxInsertMedia(%d, %d, %d) fail. Return: %s.",
               m_hPrinter, dwInsLen, dwTimeOut, ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[4] = nRet;
        }
		CleanError(); // 清错
        return nRet;
    } else
    {
        m_nRetErrOLD[4] = nRet;
    }

    CleanError(); // 清错

    return IMP_SUCCESS;
}

// 8. 介质退出
INT CDevImpl_MB2::MediaEject(WORD dwEjectMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxEjectMedia(m_hPrinter, dwEjectMode);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "介质退出: PRxEjectMedia(%d, %d) fail. Return: %s.",
           m_hPrinter, dwEjectMode, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 13. 鸣响
INT CDevImpl_MB2::SetBeep(DWORD dwCount/* = 1*/, DWORD dwTime/* = 1000*/)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = PRxBeep(m_hPrinter, dwCount, dwTime);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置鸣响: PRxBeep(%d, %d, %d) fail. Return: %s.",
            m_hPrinter, dwCount, dwTime, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 14. 获取固件版本
INT CDevImpl_MB2::GetFWVersion(LPSTR pVerBuff, ULONG *ulVerBuffSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (pVerBuff == NULL || ulVerBuffSize == 0)
    {
        Log(ThisModule, __LINE__, "获取固件版本: Input Buffer fail. Return: %s.",
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = PRxGetFirmwareVersion(m_hPrinter, pVerBuff);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取固件版本: PRxGetFirmwareVersion(%d) fail. Return: %s.",
            m_hPrinter, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 16. 向端口发送16进制数据
INT CDevImpl_MB2::PrtWriteData(LPSTR lpCMD, DWORD dwSize, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (lpCMD == NULL || strlen(lpCMD) == 0)
    {
        Log(ThisModule, __LINE__, "向端口发送16进制数据: Input Buffer fail. Return: %s.",
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = PRxPrtWriteData(m_hPrinter, (LPBYTE)lpCMD, dwSize, dwTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "向端口发送16进制数据: PRxPrtWriteData(%d, %s, %d, %d) fail. Return: %s.",
            m_hPrinter, lpCMD, dwSize, dwTimeOut, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 19. 刷新打印机缓冲区
INT CDevImpl_MB2::FlushBuffer()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxPrtFlushPrt(m_hPrinter);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "刷新打印机缓冲区: PRxPrtFlushPrt(%d) fail. Return: %s.",
            m_hPrinter, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 20. 移动打印头到指定的绝对位置
INT CDevImpl_MB2::MoveAbsPos(DWORD dwX, DWORD dwY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxPrtMoveAbsPos(m_hPrinter, dwX, dwY);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "移动打印头到指定的绝对位置: PRxPrtMoveAbsPos(%d, %d, %d) fail. Return: %s.",
            m_hPrinter, dwX, dwY, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 21. 移动打印头到指定的相对位置
INT CDevImpl_MB2::MoveRelPos(DWORD dwY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxPrtMoveRelPos(m_hPrinter, dwY);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "移动打印头到指定的相对位置: PRxPrtMoveRelPos(%d, %d) fail. Return: %s.",
            m_hPrinter, dwY, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 22. 设置打印属性
INT CDevImpl_MB2::SetPtrProperty(INT nType, INT nProperty)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxPrtSetPtrProperty(m_hPrinter, nType, nProperty);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置打印属性: PRxPrtSetPtrProperty(%d, %d, %d) fail. Return: %s.",
            m_hPrinter, nType, nProperty, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 25. 打印文本
INT CDevImpl_MB2::PrintText(LPSTR lpData, DWORD dwDataLen)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (lpData == nullptr || strlen(lpData) == 0)
    {
        Log(ThisModule, __LINE__, "打印文本: PrintData Is NULL|<1, Not Print. Return: %s.",
            ConvertErrCodeToStr(IMP_SUCCESS));
        return IMP_SUCCESS;
    }

    INT nRet = PRxPrtPrintText(m_hPrinter, lpData);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印文本: PRxPrtPrintText(%d, %s) fail. Return: %s.",
            m_hPrinter, lpData, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 27.1. 打印位图
INT CDevImpl_MB2::PrintBmp(DWORD dwX, DWORD dwY, LPSTR lpBmpFile, LPSTR lpPrtType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    std::string stdDataFile = std::string(lpBmpFile).substr(0, std::string(lpBmpFile).find("."));
    stdDataFile.append(".bin");
    Log(ThisModule, __LINE__, "打印位图: PRxPrtPrintBmp(%d, %d, %d, %s, %s, %s)...",
        m_hPrinter, dwX, dwY, lpBmpFile, stdDataFile.c_str(), lpPrtType);
    INT nRet = PRxPrtPrintBmp(m_hPrinter, dwX, dwY, lpBmpFile, (LPSTR)stdDataFile.c_str(), lpPrtType);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印位图: PRxPrtPrintBmp(%d, %d, %d, %s, %s, %s) fail. Return: %s.",
            m_hPrinter, dwX, dwY, lpBmpFile, stdDataFile.c_str(), lpPrtType, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 27.2. 打印位图
INT CDevImpl_MB2::PrintBmp(DWORD dwX, DWORD dwY, LPSTR lpBmpFile, LPSTR lpDataFile, LPSTR lpPrtType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxPrtPrintBmp(m_hPrinter, dwX, dwY, lpBmpFile, lpDataFile, lpPrtType);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印位图: PRxPrtPrintBmp(%d, %d, %d, %s, %s, %s) fail. Return: %s.",
            m_hPrinter, dwX, dwY, lpBmpFile, lpDataFile, lpPrtType, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 33. 读磁道
INT CDevImpl_MB2::MsRead(DWORD dwMagType, LPSTR lpRetData, DWORD &dwSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxMsRead(m_hPrinter, lpRetData, dwMagType);
    if (nRet < IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读磁道: MsRead(%d, %d) fail. Return: %s.",
            m_hPrinter, dwMagType, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    dwSize = nRet;

    return IMP_SUCCESS;
}

// 36. 按指定扫描参数扫描图像
INT CDevImpl_MB2::Scan(SCANCTRL stScanCtrl, LPSCANIMAGECTRL lpStFrontCtrl,
                       LPSCANIMAGECTRL lpStBackCtrl)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PRxScan(m_hPrinter, stScanCtrl, lpStFrontCtrl, lpStBackCtrl);
    if (nRet < IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "按指定扫描参数扫描图像: PRxScan(%d) fail. Return: %s.",
            m_hPrinter, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

//----------------------------------对外参数设置接口----------------------------------
INT CDevImpl_MB2::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

// 设置动态库路径(DeviceOpen前有效)
void CDevImpl_MB2::SetLibPath(LPCSTR lpPath)
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

// 设置打开模式(DeviceOpen前有效)
void CDevImpl_MB2::SetOpenMode(LPSTR lpMode, INT nBandRate)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    MCPY_NOLEN(m_szOpenMode, lpMode);
    m_nBaudRate = nBandRate;

    Log(ThisModule, __LINE__, "设置打开模式: Mode = %s, BandRate = %d.",
        m_szOpenMode, m_nBaudRate);
}

//----------------------------------内部使用方法----------------------------------
INT CDevImpl_MB2::ConvertErrorCode(INT nRet)
{
    return nRet;
}

CHAR* CDevImpl_MB2::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case IMP_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "成功/状态正常");
            return m_szErrStr;
        case IMP_ERR_LOAD_LIB:
            sprintf(m_szErrStr, "%d|%s", nRet, "动态库加载失败");
            return m_szErrStr;
        case IMP_ERR_PARAM_INVALID:
            sprintf(m_szErrStr, "%d|%s", nRet, "参数无效");
            return m_szErrStr;
        case IMP_ERR_UNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "未知错误");
            return m_szErrStr;
        case IMP_ERR_NOTOPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备未Open");
            return m_szErrStr;
        // <0 : Device返回
        case IMP_ERR_DEV_UNKNOW:
            sprintf(m_szErrStr, "%d|%s", nRet, "未知错误");
            return m_szErrStr;
        case IMP_ERR_DEV_PARAM_INVALID:
            sprintf(m_szErrStr, "%d|%s", nRet, "参数无效");
            return m_szErrStr;
        case IMP_ERR_DEV_NO_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "未调用过OPEN函数/调用不成功");
            return m_szErrStr;
        case IMP_ERR_DEV_NOT_OLI_EMULATION:
            sprintf(m_szErrStr, "%d|%s", nRet, "未调用过OPEN函数/调用不成功");
            return m_szErrStr;
        case IMP_ERR_DEV_CREATE_FAILURE:
            sprintf(m_szErrStr, "%d|%s", nRet, "创建设备失败");
            return m_szErrStr;
        case IMP_ERR_DEV_NO_SUPPORT:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备不支持");
            return m_szErrStr;
        case IMP_ERR_COMM_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "通信打开出错");
            return m_szErrStr;
        case IMP_ERR_COMM_WRITE:
            sprintf(m_szErrStr, "%d|%s", nRet, "通信写出错");
            return m_szErrStr;
        case IMP_ERR_DEV_COMM_READ:
            sprintf(m_szErrStr, "%d|%s", nRet, "通信读出错");
            return m_szErrStr;
        case IMP_ERR_DEV_COMM_PARITY:
            sprintf(m_szErrStr, "%d|%s", nRet, "通信校验错");
            return m_szErrStr;
        case IMP_ERR_DEV_TIMEOUT:
            sprintf(m_szErrStr, "%d|%s", nRet, "通信超时");
            return m_szErrStr;
        case IMP_ERR_DEV_COMM_OFF:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备离线");
            return m_szErrStr;
        case IMP_ERR_DEV_MEDIA_NOFOUND:
            sprintf(m_szErrStr, "%d|%s", nRet, "无纸");
            return m_szErrStr;
        case IMP_ERR_DEV_MS_BLANK:
            sprintf(m_szErrStr, "%d|%s", nRet, "空白磁条");
            return m_szErrStr;
        case IMP_ERR_DEV_MS_READORPARITY:
            sprintf(m_szErrStr, "%d|%s", nRet, "磁条读或校验出错");
            return m_szErrStr;
        case IMP_ERR_DEV_OFFLINE:
        case IMP_ERR_DEV_OFFLINE2:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备断电/断线");
            return m_szErrStr;
        // 0~100 : 返回状态码
        //case STATUS_NOMAL:
        //    sprintf(m_szErrStr, "%d|%s", nRet, "状态正常");
        //    return m_szErrStr;
        case STATUS_MEDIA_NONE:
            sprintf(m_szErrStr, "%d|%s", nRet, "无介质");
            return m_szErrStr;
        case STATUS_MEDIA_PREENTERING:
            sprintf(m_szErrStr, "%d|%s", nRet, "介质存在/位于入口");
            return m_szErrStr;
        case STATUS_MEDIA_ENTERING:
            sprintf(m_szErrStr, "%d|%s", nRet, "介质已对齐");
            return m_szErrStr;
        case STATUS_MEDIA_PRESENT:
            sprintf(m_szErrStr, "%d|%s", nRet, "介质已插入");
            return m_szErrStr;
        case STATUS_MEDIA_INSERTED_ALL:
            sprintf(m_szErrStr, "%d|%s", nRet, "至页顶");
            return m_szErrStr;
        case STATUS_MEDIA_NEAR_END:
            sprintf(m_szErrStr, "%d|%s", nRet, "至页尾");
            return m_szErrStr;
        case STATUS_MEDIA_JAMMED:
            sprintf(m_szErrStr, "%d|%s", nRet, "介质阻塞");
            return m_szErrStr;
        case STATUS_MEDIA_MAGNETIC_UNREAD:
            sprintf(m_szErrStr, "%d|%s", nRet, "磁条不能读出");
            return m_szErrStr;
        case STATUS_COVER_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "处于开盖/离线状态");
            return m_szErrStr;
        /*case STATUS_OFFLINE:
            sprintf(m_szErrStr, "%d|%s", nRet, "处于离线状态");*/
            return m_szErrStr;
        case STATUS_COMMAND_WRONG:
            sprintf(m_szErrStr, "%d|%s", nRet, "命令错");
            return m_szErrStr;
        case STATUS_COMM_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "通信错");
            return m_szErrStr;
        case STATUS_ERROR_UNKNOW:
            sprintf(m_szErrStr, "%d|%s", nRet, "未知状态");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
            return m_szErrStr;
    }
}

