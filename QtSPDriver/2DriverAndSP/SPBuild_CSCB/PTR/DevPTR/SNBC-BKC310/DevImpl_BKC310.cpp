#include "DevImpl_BKC310.h"
#include "BKC310Def.h"
#include "../XFS_PTR/def.h"

static const char *ThisFile = "BKC310_DevImpl.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        Log(ThisModule, 1, "检查设备OPEN标记: #OFLAG == FALSE, Device Not Open, return fail.Return: %s.", \
        ConvertErrCodeToStr(IMP_ERR_UNKNOWN)); \
        return IMP_ERR_UNKNOWN; \
    }

CDevImpl_BKC310::CDevImpl_BKC310()
{
    SetLogFile(LOG_NAME_SNBC, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_BKC310::~CDevImpl_BKC310()
{
    DeviceClose();
}

// 参数初始化
void CDevImpl_BKC310::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_bDevOpenOk = FALSE;

    //m_hBTNHDll = NULL;
    m_hPrinter = NULL;
    m_bDevOpenOk = FALSE;
    //ClearErrorCode(&m_errorInfo);

    memset(byFontType, 0x00, sizeof(byFontType));   // 字体
    wFontSize = 11;         // 五号字体(单位:磅)
    wLineHeight = 0;        // 行高，缺省: 30，单位:0.1mm
    wCharSpace = 0;         // 字符间距，缺省: 0，单位:0.1mm
    wRowDistance = 0;       // 行间距，缺省: 0，单位:0.1mm
    wLeft = 0;              // 左边距，缺省: 0，单位:0.1mm
    wTop = 0;               // 上边距，缺省: 0，单位:0.1mm
    wWidth = 0;             // 可打印宽，缺省: 0，单位:0.1mm
    wHeight = 0;            // 可打印高，缺省: 0，单位:0.1mm

    memset(m_szAutoStatusOLD, 0x00, sizeof(m_szAutoStatusOLD));  // 上一次自动获取状态保留
    memset(m_szRealTimeStatusOLD, 0x00, sizeof(m_szRealTimeStatusOLD));// 上一次获取实时状态保留
    m_nAutoStatusRetOLD = IMP_DEV_ERR_SUCCESS;    // 上一次获取自动状态结果保留
    m_nRealTimeStatusRetOLD = IMP_DEV_ERR_SUCCESS;// 上一次获取实时状态结果保留

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    //strDllName.prepend("RPR/SNBC/");
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
    //connect port
    EnumDeviceInfo = NULL;
    OpenUsbClassPort = NULL;
    Init_ = NULL;
    ClosePort = NULL;
    SendPortData = NULL;
    ReadPortData = NULL;
    SetPortTimeout = NULL;
    GetPortTimeout = NULL;
    OpenUsbApiPort = NULL;
    OpenNetPort = NULL;
    OpenCOMPort = NULL;
    OpenLPTPort = NULL;
    OpenNetPortByName = NULL;
    OpenBlueToothPortByName = NULL;
    OpenBlueToothPort = NULL;
    OpenDriverPort = NULL;

    //query
    RealTimeQueryStatus = NULL;
    NonRealTimeQueryStatus = NULL;
    AutoQueryStatus = NULL;
    FirmwareVersion = NULL;
    SoftwareVersion = NULL;
    VendorInformation = NULL;
    PrinterName = NULL;
    ResolutionRatio = NULL;
    HardwareSerialNumber = NULL;

    //text
    FeedLine = NULL;
    PrintTextOut = NULL;
    UniversalTextOut = NULL;
    SetTextLineHight = NULL;
    SetTextBold = NULL;
    SetTextDoubleWidthAndHeight = NULL;
    SetAlignmentMode = NULL;
    SetTextCharacterSpace = NULL;
    SetTextMagnifyTimes = NULL;
    SetTextFontType = NULL;
    SetTextUpsideDownMode = NULL;
    SetTextOppositeColor = NULL;
    SetTextColorEnable = NULL;
    SetTextFontColor = NULL;
    SetTextUnderline = NULL;
    SetTextRotate = NULL;
    SetTextCharsetAndCodepage = NULL;
    SetTextUserDefinedCharacterEnable = NULL;
    SetTextDefineUserDefinedCharacter = NULL;

    //bitmap
    PrintBitmap = NULL;
    PrintBitmapByMode = NULL;
    DownloadRAMBitmapByFile = NULL;
    PrintRAMBitmap = NULL;
    DownloadFlashBitmapByFile = NULL;
    PrintFlashBitmap = NULL;
    PrintTrueType = NULL;

    //barcode
    PrintBarcode = NULL;
    PrintBarcodeSimple = NULL;
    BarcodePrintQR = NULL;
    BarcodePrintPDF417 = NULL;
    mpBarcodePrintPDF417Simple = NULL;
    BarcodePrintMaxicode = NULL;
    BarcodePrintGS1DataBar = NULL;

    //basic set
    DownloadFile = NULL;
    PrintSetMode = NULL;
    PageModeSetArea = NULL;
    PageModePrint = NULL;
    PageModeClearBuffer = NULL;
    FeedLineNumber = NULL;
    CutPaper = NULL;
    Reset_ = NULL;
    KickOutDrawer = NULL;
    ApplicationUnit = NULL;
    PrintDensity = NULL;
    MotionUnit = NULL;
    SelectPaperType = NULL;
    SelectPaperTypeEEP = NULL;
}

INT CDevImpl_BKC310::DeviceOpen()
{
    return DeviceOpen(DEV_SNBC_BKC310);
}

INT CDevImpl_BKC310::DeviceOpen(WORD wType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;

    wDevType = wType;

    // 设置LOG
    if (wDevType == DEV_SNBC_BKC310)
    {
        SetLogFile(LOG_NAME_BKC310, ThisFile);
    } else
    if (wDevType == DEV_SNBC_BTNH80)
    {
        SetLogFile(LOG_NAME_BTNH80, ThisFile);
    } else
    {
        Log(ThisModule, 1, "打开设备: Input DeviceType[%d] NotFound, DeviceTypeList[%d|%s,%d|%s], Return: %s",
            wDevType, DEV_SNBC_BKC310, DEV_SNBC_BKC310_STR, DEV_SNBC_BTNH80, DEV_SNBC_BTNH80_STR,
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //如果已打开，则关闭
    DeviceClose();

    m_bDevOpenOk = FALSE;

    // 加载动态库
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            Log(ThisModule, __LINE__, "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                ConvertErrCodeToStr(IMP_ERR_LOAD_LIB));
            return IMP_ERR_LOAD_LIB;
        }
    }

    // 设备初始化
    if (wDevType == DEV_SNBC_BKC310)
    {
        if ((nRet = InitPtr(BK_SERIES_PRINTER_NAME)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开设备: 设备初始化: InitPtr(%s) Failed, Return: %s.",
                BK_SERIES_PRINTER_NAME, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    } else
    if (wDevType == DEV_SNBC_BTNH80)
    {
        if ((nRet = InitPtr(BT_SERIES_PRINTER_NAME)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开设备: 设备初始化: InitPtr(%s) Failed, Return: %s.",
                BT_SERIES_PRINTER_NAME, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    //
    if ((nRet = Connect()) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打开设备: 连接设备: Connect() Failed, Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    m_bDevOpenOk = TRUE;

    return IMP_SUCCESS;
}

BOOL CDevImpl_BKC310::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_hPrinter != NULL)
    {
        ClosePort(m_hPrinter);
        m_hPrinter = NULL;
    }

    vUnLoadLibrary();
    m_bDevOpenOk = FALSE;

    Log(ThisModule, 1, "关闭设备: Close Device, unLoad PossdkDll.");

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::InitPtr(char *pszPrinterName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_hPrinter = Init_(pszPrinterName);
    if (m_hPrinter == (DEVICEHANDLE)ERR_PARAMATER ||
        m_hPrinter == (DEVICEHANDLE)ERR_FAIL )
    {
        Log(ThisModule, 1, "初始化设备: Init_(%s) fail, Return: %s.",
            pszPrinterName, m_hPrinter);
        return ERR_FAIL;
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::Connect()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    char szDeviceNameListStr[MAX_PATH] = {0};
    int nDeviceNameNum = 0;
    bool bClassModeOpenRes = false;

    // 尝试以class mode方式打开
    int nRet = EnumDeviceInfo(PORT_USBDEVICE_CLASS, szDeviceNameListStr, sizeof(szDeviceNameListStr),
                              &nDeviceNameNum);
    if (nRet != IMP_DEV_ERR_SUCCESS)
    {
        Log(ThisModule, 1, "连接设备: 枚举设备名称: EnumDeviceInfo(%d) fail. ReturnCode: %s.",
            PORT_USBDEVICE_CLASS, ConvertErrCodeToStr(nRet));
    } else
    {
        //Different device name split by @
        char szDeviceNameList[1][MAX_PATH] = {0};
        nRet = SplitString(szDeviceNameListStr, '@', szDeviceNameList, 1);
        if (nRet == 0)
        {
            Log(ThisModule, 1, "连接设备: 解析枚举的设备名称: Device name list is empty(%d).", nRet);
        } else
        {    //class mode
            nRet = OpenUsbClassPort(m_hPrinter, szDeviceNameList[0], 0);
            if (nRet != IMP_DEV_ERR_SUCCESS)
            {
                Log(ThisModule, 1, "连接设备: 打开USB类模式设备: OpenUsbClassPort(%s) fail. ReturnCode: %s.",
                    szDeviceNameList[0], ConvertErrCodeToStr(nRet));
            } else
            {
                bClassModeOpenRes = true;
                Log(ThisModule, 1, "连接设备: 打开USB类模式设备: OpenUsbClassPort(%s) succ. ReturnCode: %s.",
                    szDeviceNameList[0], ConvertErrCodeToStr(nRet));
            }
        }
    }

    if (bClassModeOpenRes == false)
    {
        //尝试以API模式
        //API mode,innerDeviceId is 720 or 232
        nRet = OpenUsbApiPort(m_hPrinter, 232);
        if (nRet != ERR_SUCCESS)
        {
            nRet = OpenUsbApiPort(m_hPrinter, 720);
            if (nRet != ERR_SUCCESS)
            {
                nRet = OpenUsbApiPort(m_hPrinter, -1);
                if (nRet != ERR_SUCCESS)
                {
                    Log(ThisModule, 1, "连接设备: 打开USB API模式设备: OpenUsbApiPort(232,720, -1) fail. Return: %s.",
                    ConvertErrCodeToStr(nRet));
                    return nRet;
                } else {
                    Log(ThisModule, 1, "连接设备: 打开USB API模式设备: OpenUsbApiPort(-1) succ.");
                }
            } else {
                Log(ThisModule, 1, "连接设备: 打开USB API模式设备: OpenUsbApiPort(720) succ.");
            }
        } else {
            Log(ThisModule, 1, "连接设备: 打开USB API模式设备: OpenUsbApiPort(232) succ.");
        }
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = Reset_(m_hPrinter);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "设备复位: Reset fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    //ClearErrorCode(&m_errorInfo);
    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::GetRtStatus(char *pStaBuf, ULONG *pulBufSz)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if ((pStaBuf != NULL) && (pulBufSz != NULL) && (*pulBufSz > 0)) {
        int nRet = RealTimeQueryStatus(m_hPrinter, pStaBuf, *pulBufSz, pulBufSz);
        if (nRet != ERR_SUCCESS)
        {
            // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
            if (nRet != m_nRealTimeStatusRetOLD)
            {
                Log(ThisModule, 1, "实时查询设备状态: RealTimeQueryStatus() fail. Return: %s.", ConvertErrCodeToStr(nRet));
                m_nRealTimeStatusRetOLD = nRet;
            }
            return nRet;
        }
    } else {
        Log(ThisModule, 1, "实时查询设备状态: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 比较两次状态记录LOG
    if (memcmp(m_szRealTimeStatusOLD, pStaBuf, *pulBufSz) != 0)
    {
        CHAR szTmp[1024] = { 0x00 };
        for (INT i = 0; i < *pulBufSz; i ++)
        {
            sprintf(szTmp + strlen(szTmp), "BYTE%d:0x%02X:%d%d%d%d%d%d%d%d->0x%02X:%d%d%d%d%d%d%d%d%s|",
                    i, (BYTE)m_szRealTimeStatusOLD[i],
                       (m_szRealTimeStatusOLD[i] >> 7) & 0x01, (m_szRealTimeStatusOLD[i] >> 6) & 0x01,
                       (m_szRealTimeStatusOLD[i] >> 5) & 0x01, (m_szRealTimeStatusOLD[i] >> 4) & 0x01,
                       (m_szRealTimeStatusOLD[i] >> 3) & 0x01, (m_szRealTimeStatusOLD[i] >> 2) & 0x01,
                       (m_szRealTimeStatusOLD[i] >> 1) & 0x01, (m_szRealTimeStatusOLD[i] >> 0) & 0x01,
                       (BYTE)pStaBuf[i],
                       (pStaBuf[i] >> 7) & 0x01, (pStaBuf[i] >> 6) & 0x01,
                       (pStaBuf[i] >> 5) & 0x01, (pStaBuf[i] >> 4) & 0x01,
                       (pStaBuf[i] >> 3) & 0x01, (pStaBuf[i] >> 2) & 0x01,
                       (pStaBuf[i] >> 1) & 0x01, (pStaBuf[i] >> 0) & 0x01,
                       m_szRealTimeStatusOLD[i] != pStaBuf[i] ? " *" : ""
                    );
        }
        Log(ThisFile, 1, "getRtStatus()->RealTimeQueryStatus(): 状态结果比较: %s.", szTmp);
        memcpy(m_szRealTimeStatusOLD, pStaBuf, *pulBufSz);
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::GetNonRtStatus(char *pStaBuf, ULONG *pulBufSz)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if ((pStaBuf != NULL) && (pulBufSz != NULL) && (*pulBufSz > 0)) {
        int nRet = NonRealTimeQueryStatus(m_hPrinter, pStaBuf, *pulBufSz, pulBufSz);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "NonRealTimeQueryStatus() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    } else {
        Log(ThisModule, 1, "非实时查询设备状态: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::GetAutoStatus(char *pStaBuf, ULONG *pulBufSz)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if ((pStaBuf != NULL) && (pulBufSz != NULL) && (*pulBufSz > 0)) {
        int nRet = AutoQueryStatus(m_hPrinter, pStaBuf, *pulBufSz, 1, pulBufSz);
        if (nRet != ERR_SUCCESS)
        {
            // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
            if (nRet != m_nAutoStatusRetOLD)
            {
                Log(ThisModule, 1, "自动状态返回查询设备状态: AutoQueryStatus() fail. Return: %s.", ConvertErrCodeToStr(nRet));
                m_nAutoStatusRetOLD = nRet;
            }
            return nRet;
        }
    } else {
        Log(ThisModule, 1, "Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 比较两次状态记录LOG
    if (memcmp(m_szAutoStatusOLD, pStaBuf, *pulBufSz) != 0)
    {
        CHAR szTmp[1024] = { 0x00 };
        for (INT i = 0; i < *pulBufSz; i ++)
        {
            sprintf(szTmp + strlen(szTmp), "BYTE%d:0x%02X:%d%d%d%d%d%d%d%d->0x%02X:%d%d%d%d%d%d%d%d%s|",
                    i, (BYTE)m_szAutoStatusOLD[i],
                       (m_szAutoStatusOLD[i] >> 7) & 0x01, (m_szAutoStatusOLD[i] >> 6) & 0x01,
                       (m_szAutoStatusOLD[i] >> 5) & 0x01, (m_szAutoStatusOLD[i] >> 4) & 0x01,
                       (m_szAutoStatusOLD[i] >> 3) & 0x01, (m_szAutoStatusOLD[i] >> 2) & 0x01,
                       (m_szAutoStatusOLD[i] >> 1) & 0x01, (m_szAutoStatusOLD[i] >> 0) & 0x01,
                       (BYTE)pStaBuf[i],
                       (pStaBuf[i] >> 7) & 0x01, (pStaBuf[i] >> 6) & 0x01,
                       (pStaBuf[i] >> 5) & 0x01, (pStaBuf[i] >> 4) & 0x01,
                       (pStaBuf[i] >> 3) & 0x01, (pStaBuf[i] >> 2) & 0x01,
                       (pStaBuf[i] >> 1) & 0x01, (pStaBuf[i] >> 0) & 0x01,
                       m_szAutoStatusOLD[i] != pStaBuf[i] ? " *" : ""
                    );
        }
        Log(ThisFile, 1, "GetAutoStatus()->AutoQueryStatus(): 状态结果比较: %s.", szTmp);
        memcpy(m_szAutoStatusOLD, pStaBuf, *pulBufSz);
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::GetFwVersion(LPSTR pFwVerBuff, ULONG *ulFwVerSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = FirmwareVersion(m_hPrinter, pFwVerBuff, *ulFwVerSize, ulFwVerSize);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "获取设备固件版本号: FirmwareVersion() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::GetSoftwareVersion(LPSTR pVerBuff, ULONG *ulVerBuffSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (m_bDevOpenOk != TRUE || pVerBuff == NULL || ulVerBuffSize == 0)
    {
        Log(ThisModule, 1, "获取软件版本号: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = SoftwareVersion(m_hPrinter, pVerBuff, *ulVerBuffSize, ulVerBuffSize);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "获取软件版本号: SoftwareVersion() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	指定走纸长度进行切纸
 @参数：	nFeedLength    　　　: 走纸长度,单位:0.1MM
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::ControlMedia(int nFeedLength)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = CutPaper(m_hPrinter, 2, MM2SPOT(nFeedLength));
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "切纸+走纸: CutPaper(2, %d|%d) fail. Return: %s.",
            nFeedLength, MM2SPOT(nFeedLength), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	指定打印纸类型及走纸长度进行切纸
 @参数：	bDetectBlackStripe  : TRUE,黑标纸; FALSE,连续纸
 @      nFeedLength    　　　: 走纸长度,单位:0.1MM
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::ControlMedia(BOOL bDetectBlackStripe, int nFeedLength)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (bDetectBlackStripe == TRUE) // 指定为黑标纸
    {
        int nRet = SelectPaperType(m_hPrinter, 1);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "设定纸类型为黑标纸: SelectPaperType(1) fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    } else {  // 指定为连续纸
        int nRet = SelectPaperType(m_hPrinter, 0);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "设定纸类型为连续纸: SelectPaperType(0) fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return ControlMedia(nFeedLength);
}

/**
 @功能：	行模式打印字符串
 @参数：	lpPrintString  : 待打印字符串
 @      wStringSize    : 待打印字符串size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintText(LPSTR lpPrintString, WORD wStringSize, WORD wStartX)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpPrintString, wStringSize);
    Log(ThisModule, 1, "行模式打印字符串: StartX:%d, DataSize:%d, PrintData:%s.", wStartX, wStringSize, strResult.c_str());

    if (wStringSize < 1)
    {
        Log(ThisModule, 1, "行模式打印字符串: 打印数据长度[wStringSize = %d] < 1, NotPrint, Return: %s",
            wStringSize, ConvertErrCodeToStr(IMP_SUCCESS));
        return IMP_SUCCESS;
    }

    if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印字符串: 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if ((nRet = PrintFrontSetFormat()) != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印字符串: 设置打印参数: PrintFrontSetFormat() fail. Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (strlen((char*)byFontType) > 0 && wFontSize > 0)
    {
        nRet = PrintTrueType(m_hPrinter, (char *)strResult.c_str(), wStartX, -1,
                                          (char*)byFontType, wFontSize, wFontSize, 0);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "行模式打印字符串: 打印TrueType字体: PrintTrueType(FontType=%s, FontSize=%d) fail. Return: %s.",
                byFontType, wFontSize, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    } else {
        nRet = PrintTextOut(m_hPrinter, (char *)strResult.c_str(), wStartX, -1);
        if ( nRet != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "行模式打印字符串: 打印数据送入打印缓冲区: PrintTextOut() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    nRet = FeedLine(m_hPrinter);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印字符串: 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	行模式打印图片
 @参数：	lpImageStr     : 图片路径
 @      lpImageStrSize : 图片路径size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintImage(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    Log(ThisModule, 1, "行模式打印图片: StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());

    if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片: 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    nRet = PrintBitmap(m_hPrinter, (char *)strResult.c_str(), wStartX, -1, 1);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片: 打印位图: PrintBitmap() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    nRet = FeedLine(m_hPrinter);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片: 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	行模式打印图片(预下载RAM方式)
 @参数：	lpImageStr     : 图片路径
 @      lpImageStrSize : 图片路径size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintImageRAM(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    Log(ThisModule, 1, "行模式打印图片(预下载RAM方式): StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());

    if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片(预下载RAM方式): 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    nRet = DownloadRAMBitmapByFile(m_hPrinter, (char *)strResult.c_str(), 6);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片(预下载RAM方式): 预下载位图到设备RAM中: DownloadRAMBitmapByFile() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return FALSE;
    }

    nRet = PrintRAMBitmap(m_hPrinter, 6, wStartX, -1, 0);
    if ( nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片(预下载RAM方式): 打印RAM中位图: PrintRAMBitmap() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return FALSE;

    }

    nRet = FeedLine(m_hPrinter);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片(预下载RAM方式): 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	行模式打印图片(ByMode方式)
 @参数：	lpImageStr     : 图片路径
 @      lpImageStrSize : 图片路径size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintImageByMode(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    Log(ThisModule, 1, "行模式打印图片(ByMode方式): StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());

    if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片(ByMode方式): 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    nRet = PrintBitmapByMode(m_hPrinter, (char *)strResult.c_str(), wStartX, -1, BITMAP_MODE_24DOUBLE_DENSITY);
    if ( nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片(ByMode方式): 打印单色位图: PrintBitmapByMode() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    nRet = FeedLine(m_hPrinter);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "行模式打印图片(ByMode方式): 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::PrintFrontSetFormat()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = IMP_SUCCESS;
    if (wLineHeight > 0) // 设置打印行高
    {
        if ((nRet = BK_SetTextLineHight(wLineHeight)) != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "打印参数设置: 设置打印行高: BK_SetTextLineHight(%d) fail. Return: %s.",
                wLineHeight, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    if (wCharSpace > 0) // 设置字符间距
    {
        if ((nRet = BK_SetTextCharacterSpace(wCharSpace)) != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "打印参数设置: 设置字符间距: BK_SetTextCharacterSpace(%d) fail. Return: %s.",
                wCharSpace, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/*CTraceManager *CDevImpl_BKC310::GetTraceObj()
{
    return &//m_traceMgr;
}

LPSPErrorInfo CDevImpl_BKC310::GetErrorCode()
{
    return &m_errorInfo;
}*/

/////////////////////////////////////////////////////////////////////////
/**
 @功能：	设置打印模式
 @参数：	wPrintMode: 0,行模式(标准模式) / 1,页模式
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::BK_SetPrintMode(WORD wPrintMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = PrintSetMode(m_hPrinter, wPrintMode);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "设置打印模式: PrintSetMode(%d:0行/2页) fail. Return: %s.", wPrintMode, ConvertErrCodeToStr(nRet));
        return FALSE;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	设置打印行高
 @参数：	wLineHeight: 行高，单位参考IsSpot
 @           IsSpot: TRUE,wLineHeight单位为0.1mm; FALSE,wLineHeight单位为点
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::BK_SetTextLineHight(WORD wLineHeight, BOOL IsSpot /* = FALSE */)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = SetTextLineHight(m_hPrinter, (IsSpot == TRUE ? wLineHeight : MM2SPOT(wLineHeight)));
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "SetTextLineHight(%d|%d) fail. Return: %s.",
            wLineHeight, MM2SPOT(wLineHeight), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	设置字符间距
 @参数：	nCharSpace: 字符间距，单位参考IsSpot
 @          IsSpot: TRUE,nCharSpace单位为0.1mm; FALSE,nCharSpace单位为点
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::BK_SetTextCharacterSpace(WORD nCharSpace, BOOL IsSpot /* = FALSE */)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = SetTextCharacterSpace(m_hPrinter, 0,
                                     (IsSpot == TRUE ? nCharSpace : MM2SPOT(nCharSpace)), 0);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "SetTextCharacterSpace(0, %d|%d, 0) fail. Return: %s.",
            nCharSpace, MM2SPOT(nCharSpace), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    nRet = SetTextCharacterSpace(m_hPrinter, 0,
                                 (IsSpot == TRUE ? nCharSpace : MM2SPOT(nCharSpace)), 1);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "SetTextCharacterSpace(0, %d|%d, 1) fail. Return: %s.",
            nCharSpace, MM2SPOT(nCharSpace), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/*BOOL CDevImpl_BKC310::BK_PrintInPapgeMode()
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usCmd[] = { 0x0c };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetCharRightInternal(BYTE bRightInternal)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usCmd[] = { 0x1b, 0x20, bRightInternal };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetPresenterWaitTime(BYTE bWaitTime)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usCmd[] = { 0x1b, 0x63, 0x39, bWaitTime };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetPresenterMode(int nPresentMode)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usCmd[] = { 0x1b, 0x63, 0x38, (BYTE)nPresentMode };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetPaperType(BOOL bIsBlackMark)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usBM = bIsBlackMark ? 0x02 : 0x00 ;
    BYTE usCmd[] = { 0x1b, 0x63, 0x30, usBM };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    m_bBlackMark = bIsBlackMark;

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetBlodMode(BOOL bIsBlod)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usBlod = bIsBlod ? 0x01 : 0x00;
    BYTE usCmd[] = { 0x1b, 0x45, usBlod };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetPageModeArea(WORD wLineCount, int iMinLines, int iRowInterval)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    if ( !m_bBlackMark )         //连续纸切纸
    {
        if ( wLineCount < 5 )    //默认最小行数为5行
        {
            wLineCount = 5;
        }
    }

    if ( iRowInterval < 30 )     //根据实际打印效果具体调整,否则打印机硬件会少打一行
    {
        wLineCount++;
    }

    BYTE usdyL = (BYTE)( iRowInterval * wLineCount % 256 );
    BYTE usdyH = (BYTE)( iRowInterval * wLineCount / 256 );

    if ( m_bBlackMark )          //设置黑标标记切纸
    {
        //当在一个黑标打印区域内，凭条切纸一个黑标长度，否则按实际打印长度来
        if ( iRowInterval * wLineCount < 24 * ( iMinLines + 1 ) )
        {
            usdyL = (BYTE)( 24 * ( iMinLines + 1) % 256 );  //根据测试，当行高设为24时，凭条纸一个黑标距离为30行
            usdyH = (BYTE)( 24 * ( iMinLines + 1) / 256 );
        }
    }

    BYTE usCmd[] = { 0x1b, 0x57, 0x01, 0x00, 0x00, 0x00, 0x60, 0x02, usdyL, usdyH };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetRowInterval(BYTE usRowInterval)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usCmd[] = { 0x1b, 0x33, usRowInterval };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetInternalChar(BYTE usInterChar)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usCmd[] = { 0x1b, 0x52, usInterChar };

    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetCodePage(int iCodePage)
{
    BOOL bRet = TRUE;
    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    switch (iCodePage)
    {
    case PTR_CHARSET_CHN:  //默认中文
        {
            BYTE usCmd[] = { 0x1c, 0x26 };
            bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
            break;
        }
    case PTR_CHARSET_ENGUS: //英文(美国)
        {
            BYTE usCmd[] = { 0x1b, 0x74, 0x00 };
            bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
            break;
        }
    case PTR_CHARSET_FRA:   //法文
        {
            BYTE usCmd[] = { 0x1c, 0x2e, 0x1b, 0x74, 0x10 };
            bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
            break;
        }
    case PTR_CHARSET_FARSI: //波斯语
    case PTR_CHARSET_ARB:   //阿拉伯文
        {
            BYTE usCmd[] = { 0x1b, 0x74, 0x15 };
            bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
            break;
        }
    case PTR_CHARSET_PORT:  //葡萄牙语
        {
            BYTE usCmd[] = { 0x1c, 0x2e, 0x1b, 0x74, 0x03 };
            bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
            break;
        }
    default:               //其他则为中文
        {
            BYTE usCmd[] = { 0x1c, 0x26 };
            bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
            break;
        }
    }

    return bRet;
}

BOOL CDevImpl_BKC310::BK_SetAllowPrint()
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usCmd[] = { 0x1b, 0x73, 0x42, 0x45, 0x92, 0x9a, 0x01, 0x00, 0x3f };

    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
}

 BOOL CDevImpl_BKC310::BK_SendDataToBuf(LPSTR lpPrintString, WORD wStringSize)
 {
    BOOL bRet = TRUE;

    bRet = m_lpIDevComm->SendData((unsigned char*)lpPrintString, wStringSize);
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
 }*/

/*void CDevImpl_BKC310::SetErrorCode(LPSPErrorInfo lpErrorInfo, char cErrorType[], char cErrorDetailCode[], char *pDescription)
{
    m_errorCode.SetErrorCode(lpErrorInfo, MODULE_SPR_SNBC, cErrorType, cErrorDetailCode, pDescription);
}

void CDevImpl_BKC310::ClearErrorCode(LPSPErrorInfo lpErrorInfo)
{
    if (lpErrorInfo != NULL){
        memset(lpErrorInfo->sDescription, 0, sizeof(lpErrorInfo->sDescription));
        memset(lpErrorInfo->errCode.sErrorCode, '0', sizeof(lpErrorInfo->errCode.sErrorCode));
    }
}*/

void CDevImpl_BKC310::SetPrintFormat(STPRINTFORMAT stPrintFormat)
{
    //wRowDistance = (stPrintFormat.uLPI > 0 ? stPrintFormat.uLPI : wRowDistance);  // 行间距
    if (strlen(stPrintFormat.szFontType) > 0 && stPrintFormat.uFontSize > 0) {
        memcpy(byFontType, stPrintFormat.szFontType, strlen(stPrintFormat.szFontType));
        wFontSize = stPrintFormat.uFontSize;
    }
    wLineHeight = (stPrintFormat.uLPI > 0 ? stPrintFormat.uLineHeight : wLineHeight);  // 行间距
    wCharSpace = (stPrintFormat.uWPI > 0 ? stPrintFormat.uWPI : wCharSpace);      // 字符间距
}

ULONG CDevImpl_BKC310::SplitString(LPCSTR lpszStr, CHAR cSplit, CHAR lpszStrArray[][MAX_PATH], ULONG ulArrayCnt)
{
    ULONG ulCnt = 0;
    string str = "", s = "";
    size_t pos = string::npos, pre = 0;

    if (lpszStr == NULL)
    {
        return 0;
    }
    if ((strlen(lpszStr) == 0) || (cSplit == '\0'))
    {
        if ((lpszStrArray != NULL) && (ulArrayCnt > 0))
        {
            strcpy(lpszStrArray[0], lpszStr);
        }
        return 1;
    }

    str = string(lpszStr);
    pos = str.find(cSplit, pre);
    while (pos != string::npos)
    {
        s = str.substr(pre, pos - pre);
        pre = pos + 1;
        pos = str.find(cSplit, pre);
        if ((lpszStrArray != NULL) && (ulCnt < ulArrayCnt))
        {
            strcpy(lpszStrArray[ulCnt], s.c_str());
        }
        ulCnt++;
    }
    s = str.substr(pre);
    if ((lpszStrArray != NULL) && (ulCnt < ulArrayCnt))
    {
        strcpy(lpszStrArray[ulCnt], s.c_str());
    }
    ulCnt++;

    if ((lpszStrArray != NULL) && (ulArrayCnt > 0))
    {
        ulCnt = MIN(ulCnt, ulArrayCnt);
    }

    return ulCnt;
}

BOOL CDevImpl_BKC310::bLoadLibrary()
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

void CDevImpl_BKC310::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_BKC310::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    //connect, port
    LOAD_LIBINFO_FUNC(pEnumDeviceInfo, EnumDeviceInfo, "EnumDeviceInfo");
    LOAD_LIBINFO_FUNC(pOpenUsbClassPort, OpenUsbClassPort, "OpenUsbClassPort");
    LOAD_LIBINFO_FUNC(pInit, Init_, "Init");
    LOAD_LIBINFO_FUNC(pClosePort, ClosePort, "ClosePort");
    LOAD_LIBINFO_FUNC(pSendPortData, SendPortData, "SendPortData");
    LOAD_LIBINFO_FUNC(pReadPortData, ReadPortData, "ReadPortData");
    LOAD_LIBINFO_FUNC(pSetPortTimeout, SetPortTimeout, "SetPortTimeout");
    LOAD_LIBINFO_FUNC(pGetPortTimeout, GetPortTimeout, "GetPortTimeout");
    LOAD_LIBINFO_FUNC(pOpenUsbApiPort, OpenUsbApiPort, "OpenUsbApiPort");
    LOAD_LIBINFO_FUNC(pOpenNetPort, OpenNetPort, "OpenNetPort");
    LOAD_LIBINFO_FUNC(pOpenCOMPort, OpenCOMPort, "OpenCOMPort");
    LOAD_LIBINFO_FUNC(pOpenLPTPort, OpenLPTPort, "OpenLPTPort");
    LOAD_LIBINFO_FUNC(pOpenNetPortByName, OpenNetPortByName, "OpenNetPortByName");
    LOAD_LIBINFO_FUNC(pOpenBlueToothPortByName, OpenBlueToothPortByName, "OpenBlueToothPortByName");
    LOAD_LIBINFO_FUNC(pOpenBlueToothPort, OpenBlueToothPort, "OpenBlueToothPort");
    LOAD_LIBINFO_FUNC(pOpenDriverPort, OpenDriverPort, "OpenDriverPort");

    //query
    LOAD_LIBINFO_FUNC(pRealTimeQueryStatus, RealTimeQueryStatus, "RealTimeQueryStatus");
    LOAD_LIBINFO_FUNC(pNonRealTimeQueryStatus, NonRealTimeQueryStatus, "NonRealTimeQueryStatus");
    LOAD_LIBINFO_FUNC(pAutoQueryStatus, AutoQueryStatus, "AutoQueryStatus");
    LOAD_LIBINFO_FUNC(pFirmwareVersion, FirmwareVersion, "FirmwareVersion");
    LOAD_LIBINFO_FUNC(pSoftwareVersion, SoftwareVersion, "SoftwareVersion");
    LOAD_LIBINFO_FUNC(pVendorInformation, VendorInformation, "VendorInformation");
    LOAD_LIBINFO_FUNC(pPrinterName, PrinterName, "PrinterName");
    LOAD_LIBINFO_FUNC(pResolutionRatio, ResolutionRatio, "ResolutionRatio");
    LOAD_LIBINFO_FUNC(pHardwareSerialNumber, HardwareSerialNumber, "HardwareSerialNumber");

    //text
    LOAD_LIBINFO_FUNC(pFeedLine, FeedLine, "FeedLine");
    LOAD_LIBINFO_FUNC(pPrintTextOut, PrintTextOut, "PrintTextOut");
    LOAD_LIBINFO_FUNC(pUniversalTextOut, UniversalTextOut, "UniversalTextOut");
    LOAD_LIBINFO_FUNC(pSetTextLineHight, SetTextLineHight, "SetTextLineHight");
    LOAD_LIBINFO_FUNC(pSetTextBold, SetTextBold, "SetTextBold");
    LOAD_LIBINFO_FUNC(pSetTextDoubleWidthAndHeight, SetTextDoubleWidthAndHeight, "SetTextDoubleWidthAndHeight");
    LOAD_LIBINFO_FUNC(pSetAlignmentMode, SetAlignmentMode, "SetAlignmentMode");
    LOAD_LIBINFO_FUNC(pSetTextCharacterSpace, SetTextCharacterSpace, "SetTextCharacterSpace");
    LOAD_LIBINFO_FUNC(pSetTextMagnifyTimes, SetTextMagnifyTimes, "SetTextMagnifyTimes");
    LOAD_LIBINFO_FUNC(pSetTextFontType, SetTextFontType, "SetTextFontType");
    LOAD_LIBINFO_FUNC(pSetTextUpsideDownMode, SetTextUpsideDownMode, "SetTextUpsideDownMode");
    LOAD_LIBINFO_FUNC(pSetTextOppositeColor, SetTextOppositeColor, "SetTextOppositeColor");
    LOAD_LIBINFO_FUNC(pSetTextColorEnable, SetTextColorEnable, "SetTextColorEnable");
    LOAD_LIBINFO_FUNC(pSetTextFontColor, SetTextFontColor, "SetTextFontColor");
    LOAD_LIBINFO_FUNC(pSetTextUnderline, SetTextUnderline, "SetTextUnderline");
    LOAD_LIBINFO_FUNC(pSetTextRotate, SetTextRotate, "SetTextRotate");
    LOAD_LIBINFO_FUNC(pSetTextCharsetAndCodepage, SetTextCharsetAndCodepage, "SetTextCharsetAndCodepage");
    LOAD_LIBINFO_FUNC(pSetTextUserDefinedCharacterEnable, SetTextUserDefinedCharacterEnable, "SetTextUserDefinedCharacterEnable");
    LOAD_LIBINFO_FUNC(pSetTextDefineUserDefinedCharacter, SetTextDefineUserDefinedCharacter, "SetTextDefineUserDefinedCharacter");

    //bitmap
    LOAD_LIBINFO_FUNC(pPrintBitmap, PrintBitmap, "PrintBitmap");
    LOAD_LIBINFO_FUNC(pPrintBitmapByMode, PrintBitmapByMode, "PrintBitmapByMode");
    LOAD_LIBINFO_FUNC(pDownloadRAMBitmapByFile, DownloadRAMBitmapByFile, "DownloadRAMBitmapByFile");
    LOAD_LIBINFO_FUNC(pPrintRAMBitmap, PrintRAMBitmap, "PrintRAMBitmap");
    LOAD_LIBINFO_FUNC(pDownloadFlashBitmapByFile, DownloadFlashBitmapByFile, "DownloadFlashBitmapByFile");
    LOAD_LIBINFO_FUNC(pPrintFlashBitmap, PrintFlashBitmap, "PrintFlashBitmap");
    LOAD_LIBINFO_FUNC(pPrintTrueType, PrintTrueType, "PrintTrueType");

    //barcode
    LOAD_LIBINFO_FUNC(pPrintBarcode, PrintBarcode, "PrintBarcode");
    LOAD_LIBINFO_FUNC(pPrintBarcodeSimple, PrintBarcodeSimple, "PrintBarcodeSimple");
    LOAD_LIBINFO_FUNC(pBarcodePrintQR, BarcodePrintQR, "BarcodePrintQR");
    LOAD_LIBINFO_FUNC(pBarcodePrintPDF417, BarcodePrintPDF417, "BarcodePrintPDF417");
    LOAD_LIBINFO_FUNC(pBarcodePrintPDF417Simple, mpBarcodePrintPDF417Simple, "BarcodePrintPDF417Simple");
    LOAD_LIBINFO_FUNC(pBarcodePrintMaxicode, BarcodePrintMaxicode, "BarcodePrintMaxicode");
    LOAD_LIBINFO_FUNC(pBarcodePrintGS1DataBar, BarcodePrintGS1DataBar, "BarcodePrintGS1DataBar");

    //basic set
    LOAD_LIBINFO_FUNC(pDownloadFile, DownloadFile, "DownloadFile");
    LOAD_LIBINFO_FUNC(pPrintSetMode, PrintSetMode, "PrintSetMode");
    LOAD_LIBINFO_FUNC(pPageModeSetArea, PageModeSetArea, "PageModeSetArea");
    LOAD_LIBINFO_FUNC(pPageModePrint, PageModePrint, "PageModePrint");
    LOAD_LIBINFO_FUNC(pPageModeClearBuffer, PageModeClearBuffer, "PageModeClearBuffer");
    LOAD_LIBINFO_FUNC(pFeedLineNumber, FeedLineNumber, "FeedLineNumber");
    LOAD_LIBINFO_FUNC(pCutPaper, CutPaper, "CutPaper");
    LOAD_LIBINFO_FUNC(pReset, Reset_, "Reset");
    LOAD_LIBINFO_FUNC(pKickOutDrawer, KickOutDrawer, "KickOutDrawer");
    LOAD_LIBINFO_FUNC(pApplicationUnit, ApplicationUnit, "ApplicationUnit");
    LOAD_LIBINFO_FUNC(pPrintDensity, PrintDensity, "PrintDensity");
    LOAD_LIBINFO_FUNC(pMotionUnit, MotionUnit, "MotionUnit");
    LOAD_LIBINFO_FUNC(pSelectPaperType, SelectPaperType, "SelectPaperType");
    LOAD_LIBINFO_FUNC(pSelectPaperTypeEEP, SelectPaperTypeEEP, "SelectPaperTypeEEP");

    return TRUE;
}

INT CDevImpl_BKC310::ConvertErrorCode(INT nRet)
{
    return nRet;
}

CHAR* CDevImpl_BKC310::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case IMP_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "成功");
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
        case IMP_DEV_ERR_FIIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "失败");
            return m_szErrStr;
        case IMP_DEV_ERR_HANDLE:
            sprintf(m_szErrStr, "%d|%s", nRet, "无效句柄");
            return m_szErrStr;
        case IMP_DEV_ERR_PARAMATER:
            sprintf(m_szErrStr, "%d|%s", nRet, "无效参数");
            return m_szErrStr;
        case IMP_DEV_ERR_FILE:
            sprintf(m_szErrStr, "%d|%s", nRet, "参数文件错误");
            return m_szErrStr;
        case IMP_DEV_ERR_READ:
            sprintf(m_szErrStr, "%d|%s", nRet, "读取文件或数据错误");
            return m_szErrStr;
        case IMP_DEV_ERR_WRITE:
            sprintf(m_szErrStr, "%d|%s", nRet, "下发文件或数据错误");
            return m_szErrStr;
        case IMP_DEV_ERR_NOT_SUPPORT:
            sprintf(m_szErrStr, "%d|%s", nRet, "此功能不支持");
            return m_szErrStr;
        case IMP_DEV_ERR_BITMAP_INVAILD:
            sprintf(m_szErrStr, "%d|%s", nRet, "位图错误");
            return m_szErrStr;
        case IMP_DEV_ERR_LOADDLL_FAILURE:
            sprintf(m_szErrStr, "%d|%s", nRet, "动态库加载失败");
            return m_szErrStr;
        case IMP_DEV_ERR_FIRNOSUPPORT:
            sprintf(m_szErrStr, "%d|%s", nRet, "固件不支持");
            return m_szErrStr;
        case IMP_DEV_ERR_UNKOWN_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "未知错误");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
            return m_szErrStr;
    }
}

