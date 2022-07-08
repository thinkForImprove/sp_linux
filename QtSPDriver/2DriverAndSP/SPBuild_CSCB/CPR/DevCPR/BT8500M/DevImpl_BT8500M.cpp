/***************************************************************
* 文件名称：DevImpl_BT8500M.cpp
* 文件描述：BT-8500M票据发放模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_BT8500M.h"
#include "data_convertor.h"

static const char *ThisModule = "DevImpl_BT8500M.cpp";

CDevImpl_BT8500M::CDevImpl_BT8500M()
{
    SetLogFile(LOG_NAME, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_BT8500M::CDevImpl_BT8500M(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_BT8500M::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    //strDllName.prepend("CPR/BT8500M/");
    strDllName.prepend(DLL_DEVLIB_PATH);
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());

    // 动态库接口函数初始化
    OpenDevice = nullptr;             // 1. 打开设备
    CloseDevice = nullptr;            // 2. 关闭设备
    FeedCheck = nullptr;              // 3. 启动分纸
    FeedCheckEx = nullptr;            // 4. 启动定位分纸
    PrintAndScan = nullptr;           // 5. 启动打印和扫描
    OutPaperDoorControl = nullptr;    // 6. 闸门开关控制
    GetDevStatus = nullptr;           // 7. 查询设备状态
    TakeCheck = nullptr;              // 8. 打开闸门取支票
    SetCheckOut = nullptr;            // 9. 出票或退票，退票时可选择是否盖章
    ResetDev = nullptr;               // 10. 设备复位
    GetFirmwareVersion = nullptr;     // 11. 获取固件版本号
    GetLastErrorCode = nullptr;       // 12. 获取最后一个错误码
    GetLastErrorCode = nullptr;       // 13. 可将通道中的票据走出,回收时可选择是否盖章
    RetractCheck = nullptr;           // 14. 遗忘回收
    SetCheckNumOCRArea = nullptr;     // 15. 设置票号OCR识别区域
    GetCheckNumFromArea = nullptr;    // 16. 获取票号OCR识别结果
    GetCheckImage = nullptr;          // 17. 获取票据的正反面图像
    SetCheckOCRArea = nullptr;        // 18. 设置票面OCR识别区域
    GetCheckInfo = nullptr;           // 19. 获取票据鉴伪及OCR信息
    ResetDevEx = nullptr;             // 20. 设备复位Ex
    SetPaperCount = nullptr;          // 21. 设置票箱张数
    GetPaperCount = nullptr;          // 22. 获取票箱张数
    BatchPrintAndScan = nullptr;      // 23. 批量打印
    RegistFun = nullptr;              // 25. 回调函数注册
    GetIdentifyInfo = nullptr;        // 26. 鉴伪/保存图像
    GetCheckNum = nullptr;            // 27. 获取票号
    UpdateFirmware = nullptr;         // 28. 固件升级
    GetCheckNumImage = nullptr;       // 29.
    GetRfidInfo = nullptr;            // 30.
    ReadCommand = nullptr;            // 31.
    SendCommand = nullptr;            // 32.

    m_nGetStatErrOLD = PRINTER_SUCCESS;
    m_nGetOpenErrOLD = PRINTER_SUCCESS;

    // 加载设备动态库
    //if (bLoadLibrary() != TRUE)
    //{
    //    Log(ThisModule, 1, "加载动态库: bLoadLibrary() fail. ");
    //}

    m_bLoadIntfFail = TRUE;
}

CDevImpl_BT8500M::~CDevImpl_BT8500M()
{

}


/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            Log(ThisModule, __LINE__, "打开设备: 重新加载动态库: bLoadLibrary() Failed, Return: %s.",
                ConvertErrCodeToStr(IMP_ERR_LOAD_LIB));
            return IMP_ERR_LOAD_LIB;
        }
    }

    // 打开设备
    nRet = OpenDevice();
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        if (m_nGetOpenErrOLD != nRetErrCode)
        {
            Log(ThisModule, __LINE__, "打开设备: OpenDevice() Failed, ErrCode:%d, Return: %s.",
                        nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
            m_nGetOpenErrOLD = nRetErrCode;
        }
        return nRetErrCode;
    }
    m_nGetOpenErrOLD = nRetErrCode;

    m_bDevOpenOk = TRUE;

    Log(ThisModule, 1, "打开设备: OpenDevice() Success.");

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::DeviceClose(BOOL bUnLoad)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bDevOpenOk == TRUE)
    {
        if (CloseDevice() != 0)
        {
            if (CloseDevice() != 0)
            {
                CloseDevice();
            }
        }
    }

    m_bDevOpenOk = FALSE;

    if (bUnLoad == TRUE)
    {
        vUnLoadLibrary();
    }

    return IMP_SUCCESS;
}

BOOL CDevImpl_BT8500M::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}


/************************************************************
 * 功能: 3. 启动分纸
 * 参数: nCheckBox 入参 票箱号(1~4)
 *      nPaperType 入参 票据类型(0支票/1存单/2国债)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nFeedCheck(INT nCheckBox, INT nPaperType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nCheckBox < 0 || nCheckBox > 4)
    {
        Log(ThisModule, __LINE__, "启动分纸: 票箱号[%d] Failed, Return: %s.",
            nCheckBox, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 分纸
    nRet = FeedCheck(nCheckBox, nPaperType);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "启动分纸: FeedCheck(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nCheckBox, nPaperType, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 4. 启动定位分纸
 * 参数: lpStr 入参 分纸字符串
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nFeedCheckEx(LPSTR lpStr)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpStr == nullptr || strlen(lpStr) < 1)
    {
        Log(ThisModule, __LINE__, "启动定位分纸: Input Param [%s] Is NULL|<1, Return: %s.",
            lpStr, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 分纸
    nRet = FeedCheckEx(lpStr);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "启动定位分纸: FeedCheckEx(%ds) Failed, ErrCode:%d, Return: %s.",
            lpStr, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 5. 启动打印和扫描
 * 参数: lpPrintContent 入参　打印内容字符串
 *      bScanEnabled 入参　0不扫描 非零扫描
 *      nPrintNext 入参　是否启动下一张打印 0否 1 是
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nPrintAndScan(LPSTR lpPrintContent, INT nScanEnabled, INT nPrintNext)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpPrintContent == nullptr)
    {
        Log(ThisModule, __LINE__, "启动打印和扫描: 打印内容字符串[%s] Failed, Return: %s.",
            lpPrintContent, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    Log(ThisModule, __LINE__, "启动打印和扫描JSON: %s.", lpPrintContent);
    nRet = PrintAndScan(lpPrintContent, nScanEnabled, nPrintNext);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "启动打印和扫描: PrintAndScan(, %d, %d) Failed, ErrCode:%d, Return: %s.",
            nScanEnabled, nPrintNext, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 6. 闸门开关控制
 * 参数: nCmd 入参 0开门，1关门
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nOutPaperDoorControl(INT nCmd)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nCmd < 0 || nCmd > 1)
    {
        Log(ThisModule, __LINE__, "闸门开关控制: 标记入参[%d] Failed, Return: %s.",
            nCmd, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = OutPaperDoorControl(nCmd);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "闸门开关控制: OutPaperDoorControl(%d) Failed, ErrCode:%d, Return: %s.",
            nCmd, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 7. 查询设备状态
 * 参数: stDevicestatus 回参设备状态结构体
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetDevStatus(DEVICESTATUS &stDevicestatus)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetDevStatus(&stDevicestatus);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRetErrCode != m_nGetStatErrOLD)
        {
            m_nGetStatErrOLD = nRetErrCode;
            Log(ThisModule, __LINE__, "查询设备状态: GetDevStatus() Failed, ErrCode:%d, Return: %s.",
                nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        }

        return nRetErrCode;
    }
    m_nGetStatErrOLD = nRetErrCode;

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 8. 打开闸门取支票
 * 参数: nTimeout 入参 闸门关闭超时时间
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nTakeCheck(INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = TakeCheck(nTimeOut);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "打开闸门取支票: TakeCheck(%d) Failed, ErrCode:%d, Return: %s.",
            nTimeOut, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 9. 出票或退票，退票时可选择是否盖章
 * 参数: nMode 入参 1出票/0退票；
 *      bStamp 入参 1盖章/0不盖章
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nSetCheckOut(INT nMode, INT nStamp)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nMode < 0 || nMode > 1)
    {
        Log(ThisModule, __LINE__, "出票或退票: 动作入参[%d] Failed, Return: %s.",
            nMode, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }
    if (nStamp < 0 || nStamp > 1)
    {
        Log(ThisModule, __LINE__, "出票或退票: 盖章入参[%d] Failed, Return: %s.",
            nStamp, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = SetCheckOut(nMode, nStamp);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "出票或退票: SetCheckOut(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nMode, nStamp, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 10. 设备复位
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nResetDev()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = ResetDev();
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设备复位: ResetDev() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 11. 获取固件版本号
 * 参数: nBufSize 入参 缓存数据长度
 *      lpVersion 回参 固件版本信息
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetFirmwareVersion(LPSTR lpVersion, INT nBufSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpVersion == nullptr)
    {
        Log(ThisModule, __LINE__, "获取固件版本号: 缓存变量入参[%s] Failed, Return: %s.",
            lpVersion, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    //nRet = GetFirmwareVersion(lpVersion, nBufSize);
    GetFirmwareVersion(lpVersion, nBufSize);
    for (INT i = 0; i < strlen(lpVersion); i ++)
    {
        if (lpVersion[i] == '\r' || lpVersion[i] == '\n')
        {
            lpVersion[i] = 0x00;
        }
    }
    /*if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取固件版本号: GetFirmwareVersion(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpVersion, nBufSize, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }*/

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 12. 获取最后一个错误码
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetLastErrorCode()
{
    THISMODULE(__FUNCTION__);
    //// AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    return GetLastErrorCode();
}

/************************************************************
 * 功能: 13. 可将通道中的票据走出,回收时可选择是否盖章
 * 参数: nMode 入参 1出票/0退票；
 *      bStamp 入参 1盖章/0不盖章
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nMoveCheck(INT nMode, INT nStamp)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nMode < 0 || nMode > 1)
    {
        Log(ThisModule, __LINE__, "将通道中的票据走出: 动作入参[%d] Failed, Return: %s.",
            nMode, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }
    if (nStamp < 0 || nStamp > 1)
    {
        Log(ThisModule, __LINE__, "将通道中的票据走出: 盖章入参[%d] Failed, Return: %s.",
            nStamp, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = MoveCheck(nMode, nStamp);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "将通道中的票据走出: MoveChec(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nMode, nStamp, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 14. 遗忘回收
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nRetractCheck()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = RetractCheck();
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "遗忘回收: RetractCheck() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 15. 设置票号OCR识别区域
 * 参数: lpArea 入参 票号OCR识别区域，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nSetCheckNumOCRArea(LPSTR lpArea)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpArea == nullptr)
    {
        Log(ThisModule, __LINE__, "设置票号OCR识别区域: 入参 OCR识别区域[%s] Failed, Return: %s.",
            lpArea, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    Log(ThisModule, __LINE__, "设置票号OCR识别区域: %s.", lpArea);
    nRet = SetCheckNumOCRArea(lpArea);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设置票号OCR识别区域: SetCheckNumOCRArea() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 16. 获取票号OCR识别结果
 * 参数: nSize 入参 pchOCR缓冲区大小（字节数）。
 *      lpFilename 入参 要保存的票号的文件路径包含文件名，如果要bmp图像文件后缀为“.bmp”，如果要jpg图像，后缀为“.jpg”，传入空不保存图像。
 *      lpOCR 回参　票号OCR识别结果，使用JSON格式。
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetCheckNumFromArea(LPSTR lpOCR, INT nSize, LPSTR lpFilename)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetCheckNumFromArea(lpOCR, nSize, lpFilename);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票号OCR识别结果: GetCheckNumFromArea(%s, %d, %s) Failed, ErrCode:%d, Return: %s.",
            lpOCR, nSize, lpFilename, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 17. 获取票据的正反面图像
 * 参数: lpFrontImageFile 入参 票据正面图像保存路径（含文件名），如果为NULL则不保存正面图像。
 *      lpBackImageFile 入参 票据反面图像保存路径（含文件名），如果为NULL则不保存反面图像
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetCheckImage(LPSTR lpFrontImageFile, LPSTR lpBackImageFile)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetCheckImage(lpFrontImageFile, lpBackImageFile);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票据的正反面图像: GetCheckImage(%s, %s) Failed, ErrCode:%d, Return: %s.",
            lpFrontImageFile, lpBackImageFile, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 18. 设置票面OCR识别区域
 * 参数: lpArea 入参 票面OCR识别区域，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nSetCheckOCRArea(LPSTR  lpArea)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpArea == nullptr)
    {
        Log(ThisModule, __LINE__, "设置票面OCR识别区域: 入参 OCR识别区域[%s] Failed, Return: %s.",
            lpArea, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    Log(ThisModule, __LINE__, "设置票面OCR识别区域JSON: %s.", lpArea);
    nRet = SetCheckOCRArea(lpArea);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设置票面OCR识别区域: SetCheckOCRArea() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 19. 获取票据鉴伪及OCR信息
 * 参数: nSize 入参 lpInfo缓冲区大小（字节数）
 *      lpInfo 回参 票据鉴伪及OCR信息，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetCheckInfo(LPSTR lpInfo, INT nSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpInfo == nullptr)
    {
        Log(ThisModule, __LINE__, "获取票据鉴伪及OCR信息: 回参　票据鉴伪及OCR信息[%s] Failed, Return: %s.",
            lpInfo, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = GetCheckInfo(lpInfo, nSize);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票据鉴伪及OCR信息: GetCheckInfo(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpInfo, nSize, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 20. 设备复位Ex
 * 参数: bSoftReset 入参 0硬复位/1软复位
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nResetDevEx(BOOL bSoftReset)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    nRet = ResetDevEx(bSoftReset);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设备复位Ex: nResetDevEx(%d) Failed, ErrCode:%d, Return: %s.",
            bSoftReset, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 21. 设置票箱张数
 * 参数: nBoxType 入参 票箱类型(0回收/1发票箱)
 *      nBoxIndex 入参 票箱索引号(1~N)
 *      nPaperNum 入参 票张数(回收0～50/发票箱0～250)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nSetPaperCount(INT nBoxType, INT nBoxIndex, INT nPaperNum)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nBoxType != 0 && nBoxType != 1)
    {
        Log(ThisModule, __LINE__, "设置票箱张数: 入参票箱类型[%d]<0回收/1发票箱> Failed, Return: %s.",
            nBoxType, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }
    if (nBoxIndex < 1)
    {
        Log(ThisModule, __LINE__, "设置票箱张数: 入参票箱索引号[%d]<1 Failed, Return: %s.",
            nBoxIndex, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }
    if ((nBoxType == 0 && (nPaperNum > 50 || nPaperNum < 1)) ||
        (nBoxType == 1 && (nPaperNum > 250 || nPaperNum < 1)) )
    {
        Log(ThisModule, __LINE__, "设置票箱张数: 入参票张数[%d] Failed, (回收0～50/发票箱0～250), Return: %s.",
            nPaperNum, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = SetPaperCount(nBoxType, nBoxIndex, nPaperNum);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设置票箱张数: SetPaperCount(%d, %d, %d) Failed, ErrCode:%d, Return: %s.",
            nBoxType, nBoxIndex, nPaperNum, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 22. 获取票箱张数
 * 参数: nBoxType 入参 票箱类型(0回收/1发票箱)
 *      nBoxIndex 入参 票箱索引号(1~N)
 *      nPaperNum 回参 票张数
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetPaperCount(INT nBoxType, INT nBoxIndex, INT& nPaperNum)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nBoxType != 0 && nBoxType != 1)
    {
        Log(ThisModule, __LINE__, "获取票箱张数: 入参票箱类型[%d]<0回收/1发票箱> Failed, Return: %s.",
            nBoxType, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }
    if (nBoxIndex < 1)
    {
        Log(ThisModule, __LINE__, "获取票箱张数: 入参票箱索引号[%d]<1 Failed, Return: %s.",
            nBoxIndex, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = GetPaperCount(nBoxType, nBoxIndex, nPaperNum);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票箱张数: GetPaperCount(%d, %d, %d) Failed, ErrCode:%d, Return: %s.",
            nBoxType, nBoxIndex, nPaperNum, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 23. 批量打印
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nBatchPrintAndScan(LPSTR lpPrintParam, LPSTR lpScanParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = nBatchPrintAndScan(lpPrintParam, lpScanParam);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "批量打印: nBatchPrintAndScan(%s, %s) Failed, ErrCode:%d, Return: %s.",
            lpPrintParam, lpScanParam, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 25. 回调函数注册
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nRegistFun(mBatchPrintCallBack pCallBack, void* pContext)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查


    //
    nRet = RegistFun(pCallBack, pContext);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "回调函数注册: RegistFun() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 26. 鉴伪/保存图像
 * 参数: iIdentifyResult 回参 鉴伪结果 1：真票 0：假票
 *      lpIdentifyInfo 入参 需申请缓冲区，鉴伪结果待定，如果为NULL，则不进行鉴伪
 *      nBufferlen 入参 缓冲区长度
 *      lpFrontImageFileName 入参 要保存的正面的文件路径包含文件名
 *      lpRearImageFileName 入参 要保存的反面的文件路径包含文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetIdentifyInfo(INT *iIdentifyResult, LPSTR lpIdentifyInfo, INT nBufferlen,
                                        LPSTR lpFrontImageFileName, LPSTR lpRearImageFileName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetIdentifyInfo(iIdentifyResult, lpIdentifyInfo, nBufferlen, lpFrontImageFileName, lpRearImageFileName);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "鉴伪/保存图像: GetIdentifyInfo(%s, %s, %d, %s, %s) Failed, ErrCode:%d, Return: %s.",
            iIdentifyResult, lpIdentifyInfo, nBufferlen, lpFrontImageFileName, lpRearImageFileName,
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 27. 获取票号
 * 参数: lpCheckNo 票号识别信息，调用前需申请缓冲区，传入NULL不识别票号
 *      lpFilename 要保存的票号的文件路径包含文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetCheckNum(LPSTR lpCheckNo, LPSTR lpFilename)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpFilename == nullptr)
    {
        Log(ThisModule, __LINE__, "获取票号: 保存的票号的文件路径[%s] Failed, Return: %s.",
            lpFilename, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = GetCheckNum(lpCheckNo, lpFilename);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票号: GetCheckNum(%s, %s) Failed, ErrCode:%d, Return: %s.",
            lpCheckNo, lpFilename, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 28. 固件升级
 * 参数: lpFileName 入参 固件程序文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nUpdateFirmware(LPSTR lpFileName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpFileName == nullptr)
    {
        Log(ThisModule, __LINE__, "固件升级: 固件程序文件名[%s] Failed, Return: %s.",
            lpFileName, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = UpdateFirmware(lpFileName);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "固件升级: UpdateFirmware(%s) Failed, ErrCode:%d, Return: %s.",
            lpFileName, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 29.
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetCheckNumImage(LPSTR lpFileName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetCheckNumImage(lpFileName);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": GetCheckNumImage(%s) Failed, ErrCode:%d, Return: %s.",
            lpFileName, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 30.
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nGetRfidInfo(LPSTR lpRfidInfo)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetRfidInfo(lpRfidInfo);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": GetRfidInfo(%s) Failed, ErrCode:%d, Return: %s.",
            lpRfidInfo, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 31.
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nReadCommand(INT nRelength, LPBYTE lpBuffer, INT* nLength)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = ReadCommand(nRelength, lpBuffer, nLength);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": ReadCommand(%d, %s, %d) Failed, ErrCode:%d, Return: %s.",
            nRelength, lpBuffer, *nLength, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 32.
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_BT8500M::nSendCommand(LPSTR lpBuffer, INT nLength){
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = SendCommand(lpBuffer, nLength);
    if (nRet != BT8500M_RET_SUCC)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": SendCommand(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpBuffer, nLength, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}


INT CDevImpl_BT8500M::ConvertErrorCode(INT nRet)
{
    return nRet;
}

CHAR* CDevImpl_BT8500M::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case IMP_ERR_LOAD_LIB:
            sprintf(m_szErrStr, "%d|%s", nRet, "动态库加载失败");
            return m_szErrStr;
        case IMP_ERR_PARAM_INVALID:
            sprintf(m_szErrStr, "%d|%s", nRet, "参数无效");
            return m_szErrStr;
        case IMP_ERR_UNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "正常");
            return m_szErrStr;
        case PRINTER_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "");
            return m_szErrStr;
        case PRINTER_ERROR_OPEN_PORT:
            sprintf(m_szErrStr, "%d|%s", nRet, "打开端口失败");
            return m_szErrStr;
        case PRINTER_ERROR_COMMUNICATION:
            sprintf(m_szErrStr, "%d|%s", nRet, "端口通讯失败");
            return m_szErrStr;
        case PRINTER_ERROR_PAPERLACK:
            sprintf(m_szErrStr, "%d|%s", nRet, "缺纸");
            return m_szErrStr;
        case PRINTER_ERROR_BELTLACK:
            sprintf(m_szErrStr, "%d|%s", nRet, "缺色带");
            return m_szErrStr;
        case PRINTER_ERROR_FEEDER_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "分纸模块微动开关错");
            return m_szErrStr;
        case PRINTER_ERROR_DOUBLE_DOC:
            sprintf(m_szErrStr, "%d|%s", nRet, "重张");
            return m_szErrStr;
        case PRINTER_ERROR_BELTOVER:
            sprintf(m_szErrStr, "%d|%s", nRet, "碳带将近");
            return m_szErrStr;
        case PRINTER_ERROR_TEMPVOL:
            sprintf(m_szErrStr, "%d|%s", nRet, "温度电压异常");
            return m_szErrStr;
        case PRINTER_ERROR_PAPERINCOMPLETE:
            sprintf(m_szErrStr, "%d|%s", nRet, "票据缺角");
            return m_szErrStr;
        case PRINTER_ERROR_PRINTMICROSWITCH:
            sprintf(m_szErrStr, "%d|%s", nRet, "打印模块微动开关错");
            return m_szErrStr;
        case PRINTER_ERROR_PAPERSIZE:
            sprintf(m_szErrStr, "%d|%s", nRet, "票据长度错误");
            return m_szErrStr;
        case PRINTER_ERROR_HEADDOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "打印头压下错误");
            return m_szErrStr;
        case PRINTER_ERROR_TIME_OUT:
            sprintf(m_szErrStr, "%d|%s", nRet, "取票超时");
            return m_szErrStr;
        case PRINTER_ERROR_PAPERJAM:
            sprintf(m_szErrStr, "%d|%s", nRet, "卡纸错");
            return m_szErrStr;
        case PRINTER_ERROR_GATE:
            sprintf(m_szErrStr, "%d|%s", nRet, "闸门错");
            return m_szErrStr;
        case PRINTER_ERROR_BELTRECYCLE:
            sprintf(m_szErrStr, "%d|%s", nRet, "碳带回收错");
            return m_szErrStr;
        case PRINTER_ERROR_OUTPUTBOX_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "出票箱上盖抬起");
            return m_szErrStr;
        case PRINTER_ERROR_CALIBRATE:
            sprintf(m_szErrStr, "%d|%s", nRet, "扫描模块校正失败");
            return m_szErrStr;
        case PRINTER_ERROR_SCANVOLHIGH:
            sprintf(m_szErrStr, "%d|%s", nRet, "扫描模块电压偏高");
            return m_szErrStr;
        case PRINTER_ERROR_SCANVOLLOW:
            sprintf(m_szErrStr, "%d|%s", nRet, "扫描模块电压偏低");
            return m_szErrStr;
        case PRINTER_ERROR_FEEDER:
            sprintf(m_szErrStr, "%d|%s", nRet, "分纸错");
            return m_szErrStr;
        case PRINTER_ERROR_DOWNLOADIMAGE:
            sprintf(m_szErrStr, "%d|%s", nRet, "下载位图失败");
            return m_szErrStr;
        case PRINTER_ERROR_STOPPING:
            sprintf(m_szErrStr, "%d|%s", nRet, "打印机暂停中");
            return m_szErrStr;
        case PRINTER_ERROR_RFID:
            sprintf(m_szErrStr, "%d|%s", nRet, "获取射频信息失败");
            return m_szErrStr;
        case PRINTER_ERROR_NOT_IDLE:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备非空闲状态");
            return m_szErrStr;
        case PRINTER_ERROR_NOT_WAIT_PAPEROUT:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备不在出纸等待状态");
            return m_szErrStr;
        case PRINTER_ERROR_PAPER_CORRECTION:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备纠偏错");
            return m_szErrStr;
        case PRINTER_ERROR_CHANNEL_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "通道抬起压下错误");
            return m_szErrStr;
        case PRINTER_ERROR_SCAN_UNIT_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "扫描鉴伪模块上盖打开");
            return m_szErrStr;
        case PRINTER_ERROR_OCR_COVER_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "票号识别模块上盖打开");
            return m_szErrStr;
        case PRINTER_ERROR_SENSOR_CALIBRATE:
            sprintf(m_szErrStr, "%d|%s", nRet, "传感器校验失败");
            return m_szErrStr;
        case PRINTER_ERROR_OUTPUTBOX_PRESSURE:
            sprintf(m_szErrStr, "%d|%s", nRet, "发票箱压板错误");
            return m_szErrStr;
        case PRINTER_ERROR_STAMP:
            sprintf(m_szErrStr, "%d|%s", nRet, "印章错误");
            return m_szErrStr;
        case PRINTER_ERROR_SCAN_UNIT:
            sprintf(m_szErrStr, "%d|%s", nRet, "扫描模块错误");
            return m_szErrStr;
        case PRINTER_ERROR_UNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "未知错误");
            return m_szErrStr;
        case PRINTER_ERROR_FILE_R_W:
            sprintf(m_szErrStr, "%d|%s", nRet, "文件读写错误");
            return m_szErrStr;
        case PRINTER_ERROR_INIFILE:
            sprintf(m_szErrStr, "%d|%s", nRet, "读取配置文件错误");
            return m_szErrStr;
        case PRINTER_ERROR_OUTOFMEMORY:
            sprintf(m_szErrStr, "%d|%s", nRet, "内存不足");
            return m_szErrStr;
        case PRINTER_ERROR_PARAM:
            sprintf(m_szErrStr, "%d|%s", nRet, "接口传入参数错误");
            return m_szErrStr;
        case PRINTER_ERROR_JPEG_COMPRESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "JPEG压缩错误");
            return m_szErrStr;
        case PRINTER_ERROR_IMAGE_DESKEW:
            sprintf(m_szErrStr, "%d|%s", nRet, "裁剪纠偏错误");
            return m_szErrStr;
        case PRINTER_ERROR_GET_CHECK_NO:
            sprintf(m_szErrStr, "%d|%s", nRet, "票号识别错误");
            return m_szErrStr;
        case PRINTER_ERROR_IDENTIFY:
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪失败");
            return m_szErrStr;
        case PRINTER_ERROR_LOADLIB:
            sprintf(m_szErrStr, "%d|%s", nRet, "加载动态库失败");
            return m_szErrStr;
        case PRINTER_ERROR_FLOW:
            sprintf(m_szErrStr, "%d|%s", nRet, "调用流程错");
            return m_szErrStr;
        case PRINTER_ERROR_NOT_SET_PAPERCOUNT:
            sprintf(m_szErrStr, "%d|%s", nRet, "票箱张数事先未设置");
            return m_szErrStr;
        case PRINTER_ERROR_INIFILE_W:
            sprintf(m_szErrStr, "%d|%s", nRet, "写配置文件失败");
            return m_szErrStr;
        case PRINTER_ERROR_GBKTOUTF8:
            sprintf(m_szErrStr, "%d|%s", nRet, "打印条码时转码错误");
            return m_szErrStr;
        case PRINTER_ERROR_NOT_EMPTY_RETRACTBOX_COUNT:
            sprintf(m_szErrStr, "%d|%s", nRet, "未清空回收箱计数");
            return m_szErrStr;
        case PRINTER_ERROR_NOT_SUPPORT:
            sprintf(m_szErrStr, "%d|%s", nRet, "不支持的操作");
            return m_szErrStr;
        case PRINTER_ERROR_CHECKNO_COMPARE:
            sprintf(m_szErrStr, "%d|%s", nRet, "票号比对失败");
            return m_szErrStr;
        case PRINTER_ERROR_BATCHFAKE:
            sprintf(m_szErrStr, "%d|%s", nRet, "批量打印中有假票，该错误码只会通过RegistFun回调函数参数返回");
            return m_szErrStr;
        case PRINTER_ERROR_IMAGEDATA:
            sprintf(m_szErrStr, "%d|%s", nRet, "图像数据错误");
            return m_szErrStr;
        case PRINTER_ERROR_CHANNELHAVEPAPER:
            sprintf(m_szErrStr, "%d|%s", nRet, "通道有纸");
            return m_szErrStr;
        case PRINTER_ERROR_GET_QR_CODE:
            sprintf(m_szErrStr, "%d|%s", nRet, "二维码读取失败");
            return m_szErrStr;
        case PRINTER_ERROR_GET_IMAGE:
            sprintf(m_szErrStr, "%d|%s", nRet, "图像获取失败");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
            return m_szErrStr;
    }
}

// 设置动态库路径(DeviceOpen前有效)
void CDevImpl_BT8500M::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, 1, "入参[%s]无效,不设置动态库路径.");
        return;
    }

    if ((char*)lpPath[0] == "/")   // 绝对路径
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s", lpPath);
    } else
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s%s", LINUXPATHLIB, lpPath);
    }

    Log(ThisModule, 1, "设置动态库路径=<%s>.", m_szLoadDllPath);
}

BOOL CDevImpl_BT8500M::bLoadLibrary()
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
            Log(ThisModule, 1, "加载动态库<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        } else
        {
            Log(ThisModule, 1, "加载动态库<%s> Succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            Log(ThisModule, 1, "加载动态库函数接口<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
        {
            Log(ThisModule, 1, "加载动态库函数接口<%s> Succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_BT8500M::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_BT8500M::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

#define FUNC_LOADLIBRARY(FTYPE, LPFUNC, LPFUNC2) \
    LPFUNC = (FTYPE)m_LoadLibrary.resolve(LPFUNC2); \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

    // 1. 打开设备
    FUNC_LOADLIBRARY(mOpenDevice, OpenDevice, "OpenDevice");

    // 2. 关闭设备
    FUNC_LOADLIBRARY(mCloseDevice, CloseDevice, "CloseDevice");

    // 3. 启动分纸
    FUNC_LOADLIBRARY(mFeedCheck, FeedCheck, "FeedCheck");

    // 4. 启动定位分纸
    FUNC_LOADLIBRARY(mFeedCheckEx, FeedCheckEx, "FeedCheckEx");

    // 5. 启动打印和扫描
    FUNC_LOADLIBRARY(mPrintAndScan, PrintAndScan, "PrintAndScan");

    // 6. 闸门开关控制
    FUNC_LOADLIBRARY(mOutPaperDoorControl, OutPaperDoorControl, "OutPaperDoorControl");

    // 7. 查询设备状态
    FUNC_LOADLIBRARY(mGetDevStatus, GetDevStatus, "GetDevStatus");

    // 8. 打开闸门取支票
    FUNC_LOADLIBRARY(mTakeCheck, TakeCheck, "TakeCheck");

    // 9. 出票或退票，退票时可选择是否盖章
    FUNC_LOADLIBRARY(mSetCheckOut, SetCheckOut, "SetCheckOut");

    // 10. 设备复位
    FUNC_LOADLIBRARY(mResetDev, ResetDev, "ResetDev");

    // 11. 获取固件版本号
    FUNC_LOADLIBRARY(mGetFirmwareVersion, GetFirmwareVersion, "GetFirmwareVersion");

    // 12. 获取最后一个错误码
    FUNC_LOADLIBRARY(mGetLastErrorCode, GetLastErrorCode, "GetLastErrorCode");

    // 13. 可将通道中的票据走出,回收时可选择是否盖章
    FUNC_LOADLIBRARY(mMoveCheck, MoveCheck, "MoveCheck");

    // 14. 遗忘回收
    FUNC_LOADLIBRARY(mRetractCheck, RetractCheck, "RetractCheck");

    // 15. 设置票号OCR识别区域
    FUNC_LOADLIBRARY(mSetCheckNumOCRArea, SetCheckNumOCRArea, "SetCheckNumOCRArea");

    // 16. 获取票号OCR识别结果
    FUNC_LOADLIBRARY(mGetCheckNumFromArea, GetCheckNumFromArea, "GetCheckNumFromArea");

    // 17. 获取票据的正反面图像
    FUNC_LOADLIBRARY(mGetCheckImage, GetCheckImage, "GetCheckImage");

    // 18. 设置票面OCR识别区域
    FUNC_LOADLIBRARY(mSetCheckOCRArea, SetCheckOCRArea, "SetCheckOCRArea");

    // 19. 获取票据鉴伪及OCR信息
    FUNC_LOADLIBRARY(mGetCheckInfo, GetCheckInfo, "GetCheckInfo");

    // 20. 设备复位Ex
    FUNC_LOADLIBRARY(mResetDevEx, ResetDevEx, "ResetDevEx");

    // 21. 设置票箱张数
    FUNC_LOADLIBRARY(mSetPaperCount, SetPaperCount, "SetPaperCount");

    // 22. 获取票箱张数
    FUNC_LOADLIBRARY(mGetPaperCount, GetPaperCount, "GetCheckInfo");

    // 23. 批量打印
    FUNC_LOADLIBRARY(mBatchPrintAndScan, BatchPrintAndScan, "GetCheckInfo");

    // 25. 回调函数注册
    FUNC_LOADLIBRARY(mRegistFun, RegistFun, "RegistFun");

    // 26. 鉴伪/保存图像
    FUNC_LOADLIBRARY(mGetIdentifyInfo, GetIdentifyInfo, "GetIdentifyInfo");
    // 27. 获取票号
    FUNC_LOADLIBRARY(mGetCheckNum, GetCheckNum, "GetCheckNum");

    // 28. 固件升级
    FUNC_LOADLIBRARY(mUpdateFirmware, UpdateFirmware, "UpdateFirmware");

    // 29.
    FUNC_LOADLIBRARY(mGetCheckNumImage, GetCheckNumImage, "GetCheckNumImage");

    // 30.
    FUNC_LOADLIBRARY(mGetRfidInfo, GetRfidInfo, "GetRfidInfo");

    // 31.
    FUNC_LOADLIBRARY(mReadCommand, ReadCommand, "ReadCommand");

    // 32.
    FUNC_LOADLIBRARY(mSendCommand, SendCommand, "SendCommand");

    return TRUE;
}

//---------------------------------------------
