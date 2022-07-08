/***************************************************************
* 文件名称：DevImpl_PRM.cpp
* 文件描述：PRM存折打印模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_PRM.h"
#include "../XFS_PPR/def.h"

static const char *ThisFile = "DevImpl_PRM.cpp";

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
CDevImpl_PRM::CDevImpl_PRM()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_PRM::CDevImpl_PRM(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_PRM::CDevImpl_PRM(LPSTR lpLog, LPSTR lpDevStr)
{
    SetLogFile(lpLog, ThisFile, lpDevStr);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_PRM::~CDevImpl_PRM()
{
    DeviceClose();
}

// 参数初始化
void CDevImpl_PRM::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    m_bDevOpenOk = FALSE;

    //m_hBTNHDll = NULL;
    m_bDevOpenOk = FALSE;
    //ClearErrorCode(&m_errorInfo);

    m_szDevStataOLD = 0x00;                                 // 上一次获取设备状态保留
    m_bReCon = FALSE;                                       // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));
    MCPY_NOLEN(m_szOpenMode, "USB");                        // 打开模式(缺省USB)
    m_nBaudRate = 9600;                                     // 波特率(缺省9600)

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("PPR/PRM/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_bLoadIntfFail = TRUE;

    // 动态库接口函数初始化
    PR2_OpenPort = nullptr;           // 1. 打开打印机端口
    PR2_ClosePort = nullptr;          // 2. 关闭打印机端口
    PR2_InitPrinter = nullptr;        // 3. 初始化打印机
    PR2_PrintData = nullptr;          // 4. 打印文本内容
    PR2_SetFormat = nullptr;          // 5. 设置打印格式
    PR2_SetLineSpace = nullptr;       // 6. 设置打印行间距
    PR2_SetUnderLine = nullptr;       // 7. 设置下划线类型
    PR2_LR = nullptr;                 // 8. 下发换行命令
    PR2_InsertPaper = nullptr;        // 9. 进纸
    PR2_EjectPaper = nullptr;         // 10. 退纸
    PR2_GetPrinterStatus = nullptr;   // 11. 获取打印机状态
    PR2_GetPaperStatus = nullptr;     // 12. 获取介质状态
    PR2_GetFWID = nullptr;            // 13. 获取固件版本信息
    PR2_ClearError = nullptr;         // 14. 清除打印机错误状态
    PR2_ClearAll = nullptr;           // 15. 打印机总清
    PR2_Command = nullptr;            // 16. 发送数据
    PR2_MSR = nullptr;                // 17. 读取磁条信息
    PR2_MSW = nullptr;                // 18. 写入磁条信息
    PR2_MSRALL = nullptr;             // 19. 读取所有磁条信息
    PR2_SPB = nullptr;                // 20. 读磁或打印完成后正常退折(若长时间不取卡则吞折)
    PR2_PrintBmpOLI = nullptr;        // 21. OLI下打印图片bmp图片
    PR2_PrintBmpOLI_Scale = nullptr;  // 22. OLI下打印图片bmp图片(可缩放)
}

//----------------------------------SDK接口加载----------------------------------
// 加载动态库
BOOL CDevImpl_PRM::bLoadLibrary()
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
void CDevImpl_PRM::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

// 加载动态库接口函数
BOOL CDevImpl_PRM::bLoadLibIntf()
{
    // 1. 打开打印机端口
    LOAD_LIBINFO_FUNC(pCITIC_PR2_OpenPort, PR2_OpenPort, "CITIC_PR2_OpenPort");
    // 2. 关闭打印机端口
    LOAD_LIBINFO_FUNC(pCITIC_PR2_ClosePort, PR2_ClosePort, "CITIC_PR2_ClosePort");
    // 3. 初始化打印机
    LOAD_LIBINFO_FUNC(pCITIC_PR2_InitPrinter, PR2_InitPrinter, "CITIC_PR2_InitPrinter");
    // 4. 打印文本内容
    LOAD_LIBINFO_FUNC(pCITIC_PR2_PrintData, PR2_PrintData, "CITIC_PR2_PrintData");
    // 5. 设置打印格式
    LOAD_LIBINFO_FUNC(pCITIC_PR2_SetFormat, PR2_SetFormat, "CITIC_PR2_SetFormat");
    // 6. 设置打印行间距
    LOAD_LIBINFO_FUNC(pCITIC_PR2_SetLineSpace, PR2_SetLineSpace, "CITIC_PR2_SetLineSpace");
    // 7. 设置下划线类型
    LOAD_LIBINFO_FUNC(pCITIC_PR2_SetUnderLine, PR2_SetUnderLine, "CITIC_PR2_SetUnderLine");
    // 8. 下发换行命令
    LOAD_LIBINFO_FUNC(pCITIC_PR2_LR, PR2_LR, "CITIC_PR2_LR");
    // 9. 进纸
    LOAD_LIBINFO_FUNC(pCITIC_PR2_InsertPaper, PR2_InsertPaper, "CITIC_PR2_InsertPaper");
    // 10. 退纸
    LOAD_LIBINFO_FUNC(pCITIC_PR2_EjectPaper, PR2_EjectPaper, "CITIC_PR2_EjectPaper");
    // 11. 获取打印机状态
    LOAD_LIBINFO_FUNC(pCITIC_PR2_GetPrinterStatus, PR2_GetPrinterStatus, "CITIC_PR2_GetPrinterStatus");
    // 12. 获取介质状态
    LOAD_LIBINFO_FUNC(pCITIC_PR2_GetPaperStatus, PR2_GetPaperStatus, "CITIC_PR2_GetPaperStatus");
    // 13. 获取固件版本信息
    LOAD_LIBINFO_FUNC(pCITIC_PR2_GetFWID, PR2_GetFWID, "CITIC_PR2_GetFWID");
    // 14. 清除打印机错误状态
    LOAD_LIBINFO_FUNC(pCITIC_PR2_ClearError, PR2_ClearError, "CITIC_PR2_ClearError");
    // 15. 打印机总清
    LOAD_LIBINFO_FUNC(pCITIC_PR2_ClearAll, PR2_ClearAll, "CITIC_PR2_CLearAll");
    // 16. 发送数据
    LOAD_LIBINFO_FUNC(pCITIC_PR2_Command, PR2_Command, "CITIC_PR2_Command");
    // 17. 读取磁条信息
    LOAD_LIBINFO_FUNC(pCITIC_PR2_MSR, PR2_MSR, "CITIC_PR2_MSR");
    // 18. 写入磁条信息
    LOAD_LIBINFO_FUNC(pCITIC_PR2_MSW, PR2_MSW, "CITIC_PR2_MSW");
    // 19. 读取所有磁条信息
    LOAD_LIBINFO_FUNC(pCITIC_PR2_MSRALL, PR2_MSRALL, "CITIC_PR2_MSRALL");
    // 20. 读磁或打印完成后正常退折(若长时间不取卡则吞折)
    LOAD_LIBINFO_FUNC(pCITIC_PR2_SPB, PR2_SPB, "CITIC_PR2_SPB");
    // 21. OLI下打印图片bmp图片
    LOAD_LIBINFO_FUNC(pCITIC_PR2_PrintBmpOLI, PR2_PrintBmpOLI, "CITIC_PrintBmpOLI");
    // 22. OLI下打印图片bmp图片(可缩放)
    LOAD_LIBINFO_FUNC(pCITIC_PR2_PrintBmpOLI_Scale, PR2_PrintBmpOLI_Scale, "CITIC_PrintBmpOLI_Scale");

    m_bLoadIntfFail = FALSE;

    return TRUE;
}

//----------------------------------SDK封装接口方法----------------------------------
// 1. 打开设备
INT CDevImpl_PRM::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;

    // 如果已打开，则关闭
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
    nRet = PR2_OpenPort(m_szOpenMode);
    if (nRet != IMP_SUCCESS && nRet != IMP_ERR_ALREAY_OPEN)
    {
        if (nRet != m_nRetErrOLD[1])
        {
            Log(ThisModule, __LINE__,
                "打开设备: PR2_OpenPort(%s) Failed, ErrCode: %d, Return: %s.",
                m_szOpenMode, nRet, ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[1] = nRet;
        }
        return nRet;
    } else
    {
        if (nRet != IMP_ERR_ALREAY_OPEN)
        {
            Log(ThisModule, __LINE__,
                "打开设备: PR2_OpenPort(%s) Success.", m_szOpenMode);
        }
    }

    // 设备Open后必须初始化才能正常使用
    nRet = Reset(INIT_NOACTION);
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[5])
        {
            m_nRetErrOLD[5] = nRet;
            if (nRet == IMP_ERR_DEV_TIME)   // 等待超时,即Open失败
            {
                Log(ThisModule, __LINE__,
                    "打开设备: Reset: Reset(%d) Failed, ErrCode: %d|TimeOut, Return: %s.",
                    INIT_NOACTION, nRet, ConvertErrCodeToStr(IMP_ERR_NOTOPEN));
                return IMP_ERR_NOTOPEN;
            } else
            {
                Log(ThisModule, __LINE__,
                    "打开设备: Reset: Reset(%d) Failed, ErrCode: %d, Return: %s.",
                    INIT_NOACTION, nRet, ConvertErrCodeToStr(nRet));
            }
        }
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__,
            "打开设备: Reset: Reset(%d) Success.", INIT_NOACTION);
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

// 2. 关闭设备
INT CDevImpl_PRM::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    if (m_bDevOpenOk == TRUE)
    {
        if (PR2_ClosePort != nullptr)
        {
            if (PR2_ClosePort() != IMP_SUCCESS)
            {
                if (PR2_ClosePort() != IMP_SUCCESS)
                {
                    PR2_ClosePort();
                }
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

// 3. 复位设备
INT CDevImpl_PRM::Reset(INT nAction, INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    //CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = PR2_InitPrinter(nAction, nTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[5])
        {
            Log(ThisModule, __LINE__, "设备复位: PR2_InitPrinter(%d, %d) fail. Return: %s.",
                nAction, nTimeOut, ConvertErrCodeToStr(nRet));
        }

        return nRet;
    }

    return IMP_SUCCESS;
}

// 4. 打印文本
INT CDevImpl_PRM::PrintText(DWORD dwX, DWORD dwY, LPSTR lpData,
                            DWORD dwDataLen, WORD wUnit/* = 0MM*/)
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

    INT nRet = PR2_PrintData(wUnit, dwX, dwY, lpData, dwDataLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印文本: PR2_PrintData(%d, %d, %d, %s, %d) fail. Return: %s.",
            wUnit, dwX, dwY, lpData, dwDataLen, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 5. 设置打印属性
INT CDevImpl_PRM::SetPtrProperty(INT nCPI, INT nBold, INT nWidth, INT nHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_SetFormat(nCPI, nBold, nWidth, nHeight);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置打印属性: PR2_SetFormat(%d, %d, %d, %d) fail. Return: %s.",
            nCPI, nBold, nWidth, nHeight, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 6. 设置打印行间距
INT CDevImpl_PRM::SetLineSpace(INT nLPI, WORD wUnit/* = 0MM*/)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_SetLineSpace(nLPI, wUnit);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置打印行间距: PR2_SetLineSpace(%d, %d) fail. Return: %s.",
            nLPI, wUnit, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 7. 设置下划线类型
INT CDevImpl_PRM::SetUnderLine(INT nType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_SetUnderLine(nType);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置下划线类型: PR2_SetUnderLine(%d) fail. Return: %s.",
            nType, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 8. 下发换行命令
INT CDevImpl_PRM::PrtFeedLine()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_LR();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下发换行命令: PR2_LR() fail. Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 9. 进纸(介质吸入)
INT CDevImpl_PRM::MediaInsert()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_InsertPaper();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "进纸: PR2_InsertPaper() fail. Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 10. 退纸(介质退出)
INT CDevImpl_PRM::MediaEject()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_EjectPaper();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "退纸: PR2_EjectPaper() fail. Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 11. 获取设备状态
INT CDevImpl_PRM::GetDevStatus()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_GetPrinterStatus();

    if (nRet != m_nRetErrOLD[2])    // 比较两次状态记录LOG
    {
        Log(ThisModule, __LINE__, "设备状态变化: %d->%d.", m_nRetErrOLD[2], nRet);
        m_nRetErrOLD[2] = nRet;
    }

    if (nRet < IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[2])
        {
            Log(ThisModule, __LINE__, "获取设备状态: PR2_GetPrinterStatus() fail. Return: %s.",
                ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[2] = nRet;
        }
        return nRet;
    }

    // 比较两次状态记录LOG

    return nRet;
}

// 12. 获取介质状态
INT CDevImpl_PRM::GetMediaStatus()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_GetPaperStatus();

    if (nRet != m_nRetErrOLD[3])    // 比较两次状态记录LOG
    {
        Log(ThisModule, __LINE__, "介质状态变化: %d->%d.", m_nRetErrOLD[3], nRet);
        m_nRetErrOLD[3] = nRet;
    }

    if (nRet < IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[3])
        {
            Log(ThisModule, __LINE__, "获取介质状态: PR2_GetPaperStatus() fail. Return: %s.",
                ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[3] = nRet;
        }
        return nRet;
    }

    return nRet;
}

// 13. 获取固件版本
INT CDevImpl_PRM::GetFWVersion(LPSTR pBuff, DWORD dwSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    CHAR szVersion[256] = { 0x00 };

    if (pBuff == NULL || dwSize == 0)
    {
        Log(ThisModule, __LINE__, "获取固件版本: Input Buffer fail. Return: %s.",
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = PR2_GetFWID(szVersion);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取固件版本: PR2_GetFWID() fail. Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    memcpy(pBuff, szVersion, (strlen(szVersion) > dwSize ? dwSize : strlen(szVersion)));

    return IMP_SUCCESS;
}

// 14. 清除打印机错误状态
INT CDevImpl_PRM::CleanError()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = PR2_ClearError();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "清除打印机错误状态: PR2_ClearError() fail. Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 15. 打印机总清
INT CDevImpl_PRM::CleanAll()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = PR2_ClearAll();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "清除打印机错误状态: PR2_ClearAll() fail. Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 16. 向端口发送数据
INT CDevImpl_PRM::PrtWriteData(LPSTR lpCMD, DWORD dwSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (lpCMD == NULL || strlen(lpCMD) == 0)
    {
        Log(ThisModule, __LINE__, "向端口发送数据: Input Buffer fail. Return: %s.",
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = PR2_Command(lpCMD, dwSize);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "向端口发送数据: PR2_Command(%s, %d) fail. Return: %s.",
            lpCMD, dwSize, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 17. 读取磁条信息
INT CDevImpl_PRM::MsRead(DWORD dwMagType, LPSTR lpRetData, DWORD dwSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_MSR(dwMagType, lpRetData, dwSize);
    if (nRet < IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取磁条信息: PR2_MSR(%d, %s, %d) fail. Return: %s.",
            dwMagType, lpRetData, dwSize, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 18. 写入磁条信息
INT CDevImpl_PRM::MsWrite(DWORD dwMagType, LPSTR lpRetData, DWORD dwSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_MSW(dwMagType, lpRetData, dwSize);
    if (nRet < IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "写入磁条信息: PR2_MSW(%d, %s, %d) fail. Return: %s.",
            dwMagType, lpRetData, dwSize, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 19. 读取所有磁条信息
INT CDevImpl_PRM::MsReadAll(LPSTR lpT2Data, DWORD dwT2Size, LPSTR lpT3Data, DWORD dwT3Size)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_MSRALL(lpT2Data, dwT2Size, lpT3Data, dwT3Size);
    if (nRet < IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取所有磁条信息: PR2_MSRALL(%s, %d, %s, %d) fail. Return: %s.",
            lpT2Data, dwT2Size, lpT3Data, dwT3Size, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 20. 读磁或打印完成后正常退折(若长时间不取则吞折)
INT CDevImpl_PRM::EjectAndRetract(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_SPB(dwTimeOut);
    if (nRet < IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "正常退折超时吞折: PR2_SPB(%d) fail. Return: %s.",
            dwTimeOut, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 21.1. 打印位图
INT CDevImpl_PRM::PrintBmp(DWORD dwX, DWORD dwY, LPSTR lpBmpFile, BOOL bEject)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_PrintBmpOLI(lpBmpFile, bEject, dwX, dwY);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印位图: PR2_PrintBmpOLI(%s, %d, %d, %d) fail. Return: %s.",
            lpBmpFile, bEject, dwX, dwY, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 21.2. 打印位图(可缩放)
INT CDevImpl_PRM::PrintBmp(DWORD dwX, DWORD dwY, LPSTR lpBmpFile, BOOL bEject, FLOAT fScale)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = PR2_PrintBmpOLI_Scale(lpBmpFile, bEject, dwX, dwY, fScale);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印位图(可缩放): PR2_PrintBmpOLI_Scale(%s, %d, %d, %d, %f) fail. Return: %s.",
            lpBmpFile, bEject, dwX, dwY, fScale, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 启动一次鸣响
INT CDevImpl_PRM::StartOnceBeep()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    CHAR szBeep[] = { 0x07, 0x07 };

    INT nRet = PR2_Command(szBeep, 2);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "启动一次鸣响: PR2_Command(%02X|%02X, %d) fail. Return: %s.",
            szBeep[0], szBeep[1], 2, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

//----------------------------------对外参数设置接口----------------------------------
INT CDevImpl_PRM::SetReConFlag(BOOL bFlag)
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
void CDevImpl_PRM::SetLibPath(LPCSTR lpPath)
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
void CDevImpl_PRM::SetOpenMode(LPSTR lpMode, INT nBandRate)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    MCPY_NOLEN(m_szOpenMode, lpMode);
    m_nBaudRate = nBandRate;

    Log(ThisModule, __LINE__, "设置打开模式: Mode = %s, BandRate = %d.",
        m_szOpenMode, m_nBaudRate);
}

//----------------------------------内部使用方法----------------------------------
INT CDevImpl_PRM::ConvertErrorCode(INT nRet)
{
    return nRet;
}

CHAR* CDevImpl_PRM::ConvertErrCodeToStr(INT nRet)
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
        case IMP_ERR_USB_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "USB打开异常");
            return m_szErrStr;
        case IMP_ERR_CONNECT:
            sprintf(m_szErrStr, "%d|%s", nRet, "连接异常");
            return m_szErrStr;
        case IMP_ERR_SNDCMD:
            sprintf(m_szErrStr, "%d|%s", nRet, "发送失败(查询指令发送异常)");
            return m_szErrStr;
        case IMP_ERR_RCVCMD:
            sprintf(m_szErrStr, "%d|%s", nRet, "读取失败");
            return m_szErrStr;
        case IMP_ERR_ALREAY_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "端口已打开");
            return m_szErrStr;
        case IMP_ERR_NOT_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "端口未打开(打开失败)");
            return m_szErrStr;
        case IMP_ERR_FILE_NOFOUND:
            sprintf(m_szErrStr, "%d|%s", nRet, "文件不存在");
            return m_szErrStr;
        case IMP_ERR_FILE_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "未找到文件或打开失败");
            return m_szErrStr;
        case IMP_ERR_FILE_READ:
            sprintf(m_szErrStr, "%d|%s", nRet, "文件读取失败");
            return m_szErrStr;
        case IMP_ERR_FILE_LENGTH:
            sprintf(m_szErrStr, "%d|%s", nRet, "文件长度获取失败");
            return m_szErrStr;
        case IMP_ERR_GETMEDIA_STAT:
            sprintf(m_szErrStr, "%d|%s", nRet, "获取介质状态失败");
            return m_szErrStr;
        case ERR_ERR_GETMEDIA_SIZE:
            sprintf(m_szErrStr, "%d|%s", nRet, "介质尺寸获取失败");
            return m_szErrStr;
        case IMP_ERR_GETDEV_STAT:
            sprintf(m_szErrStr, "%d|%s", nRet, "获取机器状态失败");
            return m_szErrStr;
        case IMP_ERR_DEV_TIME:
            sprintf(m_szErrStr, "%d|%s", nRet, "等待超时");
            return m_szErrStr;
        case IMP_ERR_MEM_APPLY:
            sprintf(m_szErrStr, "%d|%s", nRet, "内存申请失败");
            return m_szErrStr;
        case IMP_ERR_PAR_INVALID:
            sprintf(m_szErrStr, "%d|%s", nRet, "输入参数无效");
            return m_szErrStr;
        case IMP_ERR_DATA_CODESET:
            sprintf(m_szErrStr, "%d|%s", nRet, "文本字符编码格式错误");
            return m_szErrStr;
        case IMP_ERR_IMAGE_NOT_SUPP:
            sprintf(m_szErrStr, "%d|%s", nRet, "不支持的图片格式");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
            return m_szErrStr;
    }
}

