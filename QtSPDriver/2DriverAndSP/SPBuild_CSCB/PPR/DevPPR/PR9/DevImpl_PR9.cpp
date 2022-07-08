/***************************************************************
* 文件名称：DevImpl_CRT780B.cpp
* 文件描述：CRT780B退卡模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/
#include "DevImpl_PR9.h"

static const char *ThisModule = "DevImpl_PR9.cpp";

CDevImpl_PR9::CDevImpl_PR9()
{
    SetLogFile(LOG_NAME, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_PR9::CDevImpl_PR9(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_PR9::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("CPR/PR9/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    // 动态库接口函数初始化
    OpenDevice = nullptr;             // 1. 打开设备
    CloseDevice = nullptr;            // 2. 关闭设备
    FeedCheck = nullptr;              // 3. 启动分纸
    GetCheckNum = nullptr;            // 4. 获取票号
    PrintAndScan = nullptr;           // 5. 启动打印和扫描
    OutPaperDoorControl = nullptr;    // 6. 闸门开关控制
    GetIdentifyInfo = nullptr;        // 7. 鉴伪/保存图像
    GetDevStatus = nullptr;           // 8. 查询设备状态
    TakeCheck = nullptr;              // 9. 打开闸门取支票
    SetCheckOut = nullptr;            // 10. 出票或退票，退票时可选择是否盖章
    ResetDev = nullptr;               // 11. 设备复位
    GetFirmwareVersion = nullptr;     // 12. 获取固件版本号
    UpdateFirmware = nullptr;         // 13. 固件升级
    GetLastErrorCode = nullptr;       // 14. 获取最后一个错误码
    GetLastErrorCode = nullptr;       // 15. 可将通道中的票据走出,回收时可选择是否盖章
    RetractCheck = nullptr;           // 16. 遗忘回收
    SetCheckNumOCRArea = nullptr;     // 17. 设置票号OCR识别区域
    GetCheckNumFromArea = nullptr;    // 18. 获取票号OCR识别结果
    GetCheckImage = nullptr;          // 19. 获取票据的正反面图像
    SetCheckOCRArea = nullptr;        // 20. 设置票面OCR识别区域
    GetCheckInfo = nullptr;           // 21. 获取票据鉴伪及OCR信息
    GetCheckNumImage = nullptr;       // 22.
    GetRfidInfo = nullptr;            // 23.
    ReadCommand = nullptr;            // 24.
    SendCommand = nullptr;            // 25.

    // 加载设备动态库
    if (bLoadLibrary() != TRUE)
    {
        Log(ThisModule, 1, "加载动态库: bLoadLibrary() fail. ");
    }
}

CDevImpl_PR9::~CDevImpl_PR9()
{

}


/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 打开设备
    nRet = OpenDevice();
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "打开设备: OpenDevice() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    m_bDevOpenOk = TRUE;

    Log(ThisModule, 1, "打开设备: OpenDevice() Success.");

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::DeviceClose()
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

    vUnLoadLibrary();

    return IMP_SUCCESS;
}

BOOL CDevImpl_PR9::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}


/************************************************************
 * 功能: 3. 启动分纸
 * 参数: nCheckBox 入参 票箱号(1~4)
 *      nPaperType 入参 票据类型(0支票/1存单/2国债)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nFeedCheck(INT nCheckBox, INT nPaperType)
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
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "启动分纸: FeedCheck(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nCheckBox, nPaperType, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 4. 获取票号
 * 参数: lpCheckNo 票号识别信息，调用前需申请缓冲区，传入NULL不识别票号
 *      lpFilename 要保存的票号的文件路径包含文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetCheckNum(LPSTR lpCheckNo, LPSTR lpFilename)
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
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票号: GetCheckNum(%s, %s) Failed, ErrCode:%d, Return: %s.",
            lpCheckNo, lpFilename, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
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
INT CDevImpl_PR9::nPrintAndScan(LPSTR lpPrintContent, INT nScanEnabled, INT nPrintNext)
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
    nRet = PrintAndScan(lpPrintContent, nScanEnabled, nPrintNext);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "启动打印和扫描: PrintAndScan(%s, %d, %d) Failed, ErrCode:%d, Return: %s.",
            lpPrintContent, nScanEnabled, nPrintNext, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 6. 闸门开关控制
 * 参数: nCmd 入参 0开门，1关门
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nOutPaperDoorControl(INT nCmd)
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
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "闸门开关控制: OutPaperDoorControl(%d) Failed, ErrCode:%d, Return: %s.",
            nCmd, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 7. 鉴伪/保存图像
 * 参数: iIdentifyResult 回参 鉴伪结果 1：真票 0：假票
 *      lpIdentifyInfo 入参 需申请缓冲区，鉴伪结果待定，如果为NULL，则不进行鉴伪
 *      nBufferlen 入参 缓冲区长度
 *      lpFrontImageFileName 入参 要保存的正面的文件路径包含文件名
 *      lpRearImageFileName 入参 要保存的反面的文件路径包含文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetIdentifyInfo(INT *iIdentifyResult, LPSTR lpIdentifyInfo, INT nBufferlen,
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
    if (nRet != 0)
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
 * 功能: 8. 查询设备状态
 * 参数: stDevicestatus 回参设备状态结构体
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetDevStatus(DEVICESTATUS &stDevicestatus)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetDevStatus(&stDevicestatus);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "查询设备状态: GetDevStatus() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 9. 打开闸门取支票
 * 参数: nTimeout 入参 闸门关闭超时时间
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nTakeCheck(INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = TakeCheck(nTimeOut);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "打开闸门取支票: TakeCheck(%d) Failed, ErrCode:%d, Return: %s.",
            nTimeOut, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 10. 出票或退票，退票时可选择是否盖章
 * 参数: nMode 入参 1出票/0退票；
 *      bStamp 入参 1盖章/0不盖章
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nSetCheckOut(INT nMode, INT nStamp)
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
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "出票或退票: SetCheckOut(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nMode, nStamp, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 11. 设备复位
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nResetDev()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = ResetDev();
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设备复位: ResetDev() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 12. 获取固件版本号
 * 参数: nBufSize 入参 缓存数据长度
 *      lpVersion 回参 固件版本信息
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetFirmwareVersion(LPSTR lpVersion, INT nBufSize)
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
    nRet = GetFirmwareVersion(lpVersion, nBufSize);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取固件版本号: GetFirmwareVersion(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpVersion, nBufSize, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 13. 固件升级
 * 参数: lpFileName 入参 固件程序文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nUpdateFirmware(LPSTR lpFileName)
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
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "固件升级: UpdateFirmware(%s) Failed, ErrCode:%d, Return: %s.",
            lpFileName, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 14. 获取最后一个错误码
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetLastErrorCode()
{
    THISMODULE(__FUNCTION__);
    //// AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    return GetLastErrorCode();
}

/************************************************************
 * 功能: 15. 可将通道中的票据走出,回收时可选择是否盖章
 * 参数: nMode 入参 1出票/0退票；
 *      bStamp 入参 1盖章/0不盖章
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nMoveCheck(INT nMode, INT nStamp)
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
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "将通道中的票据走出: MoveChec(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nMode, nStamp, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 16. 遗忘回收
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nRetractCheck()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = RetractCheck();
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "遗忘回收: RetractCheck() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 17. 设置票号OCR识别区域
 * 参数: lpArea 入参 票号OCR识别区域，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nSetCheckNumOCRArea(LPSTR lpArea)
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
    nRet = SetCheckNumOCRArea(lpArea);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设置票号OCR识别区域: SetCheckNumOCRArea(%s) Failed, ErrCode:%d, Return: %s.",
            lpArea, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 18. 获取票号OCR识别结果
 * 参数: nSize 入参 pchOCR缓冲区大小（字节数）。
 *      lpFilename 入参 要保存的票号的文件路径包含文件名，如果要bmp图像文件后缀为“.bmp”，如果要jpg图像，后缀为“.jpg”，传入空不保存图像。
 *      lpOCR 回参　票号OCR识别结果，使用JSON格式。
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetCheckNumFromArea(LPSTR lpOCR, INT nSize, LPSTR lpFilename)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetCheckNumFromArea(lpOCR, nSize, lpFilename);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票号OCR识别结果: GetCheckNumFromArea(%s, %d, %s) Failed, ErrCode:%d, Return: %s.",
            lpOCR, nSize, lpFilename, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 19. 获取票据的正反面图像
 * 参数: lpFrontImageFile 入参 票据正面图像保存路径（含文件名），如果为NULL则不保存正面图像。
 *      lpBackImageFile 入参 票据反面图像保存路径（含文件名），如果为NULL则不保存反面图像
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetCheckImage(LPSTR lpFrontImageFile, LPSTR lpBackImageFile)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetCheckImage(lpFrontImageFile, lpBackImageFile);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票据的正反面图像: GetCheckImage(%s, %s) Failed, ErrCode:%d, Return: %s.",
            lpFrontImageFile, lpBackImageFile, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 20. 设置票面OCR识别区域
 * 参数: lpArea 入参 票面OCR识别区域，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nSetCheckOCRArea(LPSTR  lpArea)
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
    nRet = SetCheckOCRArea(lpArea);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设置票面OCR识别区域: SetCheckOCRArea(%s) Failed, ErrCode:%d, Return: %s.",
            lpArea, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 21. 获取票据鉴伪及OCR信息
 * 参数: nSize 入参 lpInfo缓冲区大小（字节数）
 *      lpInfo 回参 票据鉴伪及OCR信息，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetCheckInfo(LPSTR lpInfo, INT nSize)
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
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票据鉴伪及OCR信息: GetCheckInfo(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpInfo, nSize, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 22.
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetCheckNumImage(LPSTR lpFileName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetCheckNumImage(lpFileName);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": GetCheckNumImage(%s) Failed, ErrCode:%d, Return: %s.",
            lpFileName, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 23.
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nGetRfidInfo(LPSTR lpRfidInfo)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = GetRfidInfo(lpRfidInfo);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": GetRfidInfo(%s) Failed, ErrCode:%d, Return: %s.",
            lpRfidInfo, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 24.
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nReadCommand(INT nRelength, LPBYTE lpBuffer, INT* nLength)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = ReadCommand(nRelength, lpBuffer, nLength);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": ReadCommand(%d, %s, %d) Failed, ErrCode:%d, Return: %s.",
            nRelength, lpBuffer, *nLength, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 25.
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_PR9::nSendCommand(LPSTR lpBuffer, INT nLength){
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    nRet = SendCommand(lpBuffer, nLength);
    if (nRet != 0)
    {
        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": SendCommand(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpBuffer, nLength, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}


INT CDevImpl_PR9::ConvertErrorCode(INT nRet)
{
    return IMP_SUCCESS;
}

CHAR* CDevImpl_PR9::ConvertErrCodeToStr(long lRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(lRet)
    {
    case IMP_SUCCESS:
        sprintf(m_szErrStr, "%d|%s", lRet, "成功");
        return m_szErrStr;
    }
}

BOOL CDevImpl_PR9::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);


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

void CDevImpl_PR9::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_PR9::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1. 打开设备
    OpenDevice = (mOpenDevice)m_LoadLibrary.resolve("OpenDevice");
    FUNC_POINTER_ERROR_RETURN(OpenDevice, "OpenDevice");

    // 2. 关闭设备
    CloseDevice = (mCloseDevice)m_LoadLibrary.resolve("CloseDevice");
    FUNC_POINTER_ERROR_RETURN(CloseDevice, "CloseDevice");

    // 3. 启动分纸
    FeedCheck = (mFeedCheck)m_LoadLibrary.resolve("FeedCheck");
    FUNC_POINTER_ERROR_RETURN(FeedCheck, "FeedCheck");

    // 4. 获取票号
    GetCheckNum = (mGetCheckNum)m_LoadLibrary.resolve("GetCheckNum");
    FUNC_POINTER_ERROR_RETURN(GetCheckNum, "GetCheckNum");

    // 5. 启动打印和扫描
    PrintAndScan = (mPrintAndScan)m_LoadLibrary.resolve("PrintAndScan");
    FUNC_POINTER_ERROR_RETURN(PrintAndScan, "PrintAndScan");

    // 6. 闸门开关控制
    OutPaperDoorControl = (mOutPaperDoorControl)m_LoadLibrary.resolve("OutPaperDoorControl");
    FUNC_POINTER_ERROR_RETURN(OutPaperDoorControl, "OutPaperDoorControl");

    // 7. 鉴伪/保存图像
    GetIdentifyInfo = (mGetIdentifyInfo)m_LoadLibrary.resolve("GetIdentifyInfo");
    FUNC_POINTER_ERROR_RETURN(GetIdentifyInfo, "GetIdentifyInfo");

    // 8. 查询设备状态
    GetDevStatus = (mGetDevStatus)m_LoadLibrary.resolve("GetDevStatus");
    FUNC_POINTER_ERROR_RETURN(GetDevStatus, "GetDevStatus");

    // 9. 打开闸门取支票
    TakeCheck = (mTakeCheck)m_LoadLibrary.resolve("TakeCheck");
    FUNC_POINTER_ERROR_RETURN(TakeCheck, "TakeCheck");

    // 10. 出票或退票，退票时可选择是否盖章
    SetCheckOut = (mSetCheckOut)m_LoadLibrary.resolve("SetCheckOut");
    FUNC_POINTER_ERROR_RETURN(SetCheckOut, "SetCheckOut");

    // 11. 设备复位
    ResetDev = (mResetDev)m_LoadLibrary.resolve("ResetDev");
    FUNC_POINTER_ERROR_RETURN(ResetDev, "ResetDev");

    // 12. 获取固件版本号
    GetFirmwareVersion = (mGetFirmwareVersion)m_LoadLibrary.resolve("GetFirmwareVersion");
    FUNC_POINTER_ERROR_RETURN(GetFirmwareVersion, "GetFirmwareVersion");

    // 13. 固件升级
    UpdateFirmware = (mUpdateFirmware)m_LoadLibrary.resolve("UpdateFirmware");
    FUNC_POINTER_ERROR_RETURN(UpdateFirmware, "UpdateFirmware");

    // 14. 获取最后一个错误码
    GetLastErrorCode = (mGetLastErrorCode)m_LoadLibrary.resolve("GetLastErrorCode");
    FUNC_POINTER_ERROR_RETURN(GetLastErrorCode, "GetLastErrorCode");

    // 15. 可将通道中的票据走出,回收时可选择是否盖章
    GetLastErrorCode = (mGetLastErrorCode)m_LoadLibrary.resolve("GetLastErrorCode");
    FUNC_POINTER_ERROR_RETURN(GetLastErrorCode, "GetLastErrorCode");

    // 16. 遗忘回收
    RetractCheck = (mRetractCheck)m_LoadLibrary.resolve("RetractCheck");
    FUNC_POINTER_ERROR_RETURN(RetractCheck, "RetractCheck");

    // 17. 设置票号OCR识别区域
    SetCheckNumOCRArea = (mSetCheckNumOCRArea)m_LoadLibrary.resolve("SetCheckNumOCRArea");
    FUNC_POINTER_ERROR_RETURN(SetCheckNumOCRArea, "SetCheckNumOCRArea");

    // 18. 获取票号OCR识别结果
    GetCheckNumFromArea = (mGetCheckNumFromArea)m_LoadLibrary.resolve("GetCheckNumFromArea");
    FUNC_POINTER_ERROR_RETURN(GetCheckNumFromArea, "GetCheckNumFromArea");

    // 19. 获取票据的正反面图像
    GetCheckImage = (mGetCheckImage)m_LoadLibrary.resolve("GetCheckImage");
    FUNC_POINTER_ERROR_RETURN(GetCheckImage, "GetCheckImage");

    // 20. 设置票面OCR识别区域
    SetCheckOCRArea = (mSetCheckOCRArea)m_LoadLibrary.resolve("SetCheckOCRArea");
    FUNC_POINTER_ERROR_RETURN(SetCheckOCRArea, "SetCheckOCRArea");

    // 21. 获取票据鉴伪及OCR信息
    GetCheckInfo = (mGetCheckInfo)m_LoadLibrary.resolve("GetCheckInfo");
    FUNC_POINTER_ERROR_RETURN(GetCheckInfo, "GetCheckInfo");

    // 22.
    GetCheckNumImage = (mGetCheckNumImage)m_LoadLibrary.resolve("GetCheckNumImage");
    FUNC_POINTER_ERROR_RETURN(GetCheckNumImage, "GetCheckNumImage");

    // 23.
    GetRfidInfo = (mGetRfidInfo)m_LoadLibrary.resolve("GetRfidInfo");
    FUNC_POINTER_ERROR_RETURN(GetRfidInfo, "GetRfidInfo");

    // 24.
    ReadCommand = (mReadCommand)m_LoadLibrary.resolve("ReadCommand");
    FUNC_POINTER_ERROR_RETURN(ReadCommand, "ReadCommand");

    // 25.
    SendCommand = (mSendCommand)m_LoadLibrary.resolve("SendCommand");
    FUNC_POINTER_ERROR_RETURN(SendCommand, "SendCommand");
    return TRUE;
}

//---------------------------------------------
