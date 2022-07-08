#include "DevImpl_BKC310.h"
#include "BKC310Def.h"
#include "../XFS_PTR/def.h"

#include <QTextCodec>
#include <unistd.h>

static const char *ThisFile = "BKC310_DevImpl.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[7] != IMP_ERR_UNKNOWN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertErrCodeToStr(IMP_ERR_UNKNOWN)); \
        } \
        m_nRetErrOLD[7] = IMP_ERR_UNKNOWN; \
        return IMP_ERR_UNKNOWN; \
    }

CDevImpl_BKC310::CDevImpl_BKC310()
{
    SetLogFile(LOG_NAME_SNBC, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_BKC310::CDevImpl_BKC310(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

void CDevImpl_BKC310::SetLogName(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
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
    memset(m_szAutoStatusOLD, 0x00, sizeof(m_szAutoStatusOLD));         // 上一次自动获取状态保留
    memset(m_szRealTimeStatusOLD, 0x00, sizeof(m_szRealTimeStatusOLD)); // 上一次获取实时状态保留
    m_nAutoStatusRetOLD = IMP_DEV_ERR_SUCCESS;                          // 上一次获取自动状态结果保留
    m_nRealTimeStatusRetOLD = IMP_DEV_ERR_SUCCESS;                      // 上一次获取实时状态结果保留
    m_bReCon = FALSE;                                                   // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, 8);
    m_nOpenMode = 0;                                                    // 打开模式(缺省0)    // 30-00-00-00(FT#0067)
    m_nDeviceID = -1;                                                   // 内部ID(缺省-1)    // 30-00-00-00(FT#0067)

    m_vDlHandle = nullptr;

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
    //m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    // 动态库接口函数初始化
    vInitLibFunc();

    // 动态库接口函数初始化
    //connect port
    /*EnumDeviceInfo = NULL;
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
    BarcodePrintPDF417Simple = NULL;
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
    SelectPaperTypeEEP = NULL;*/
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
        Log(ThisModule, __LINE__, "打开设备: Input DeviceType[%d] NotFound, DeviceTypeList[%d|%s,%d|%s], Return: %s",
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

    // 设备初始化
    if (wDevType == DEV_SNBC_BKC310)
    {
        nRet = InitPtr(BK_SERIES_PRINTER_NAME);
    } else
    if (wDevType == DEV_SNBC_BTNH80)
    {
        nRet = InitPtr(BT_SERIES_PRINTER_NAME);
    }    
    if (nRet != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[1] != nRet)
        {
            Log(ThisModule, __LINE__, "打开设备: 设备初始化: InitPtr(%s) Failed, Return: %s.",
                BK_SERIES_PRINTER_NAME, ConvertErrCodeToStr(nRet));
        }
        m_nRetErrOLD[1] = nRet;
        return nRet;
    }
    m_nRetErrOLD[1] = nRet;

    // 设备连接
    if ((nRet = Connect()) != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[2] != nRet)
        {
            Log(ThisModule, __LINE__, "打开设备: 连接设备: Connect() Failed, Return: %s.",
                ConvertErrCodeToStr(nRet));
        }
        m_nRetErrOLD[2] = nRet;
        return nRet;
    }
    m_nRetErrOLD[2] = nRet;

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }

    m_bReCon = FALSE;                                                   // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, 8);
    m_bDevOpenOk = TRUE;

    // Open成功后,获取设备信息记录到LOG
    CHAR szFWVersion[64] = { 0x00 };
    CHAR szSoftVersion[64] = { 0x00 };
    CHAR szHardSerialNo[64] = { 0x00 };
    CHAR szPrintName[64] = { 0x00 };
    CHAR szVendorInfo[64] = { 0x00 };
    ULONG ulRetDataSize = 64;

    GetFwVersion(szFWVersion, &ulRetDataSize);
    ulRetDataSize = 64;
    GetSoftwareVersion(szSoftVersion, &ulRetDataSize);
    ulRetDataSize = 64;
    GetHardSerialNo(szHardSerialNo, &ulRetDataSize);
    ulRetDataSize = 64;
    GetPrinterName(szPrintName, &ulRetDataSize);
    ulRetDataSize = 64;
    GetVendorInfo(szVendorInfo, &ulRetDataSize);

    Log(ThisModule, __LINE__, "设备信息: 固件版本=%s | 软件版本=%s | 硬件序列号=%s | "
                              "设备名=%s | 厂商信息=%s |.",
        szFWVersion, szSoftVersion, szHardSerialNo, szPrintName, szVendorInfo);

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_hPrinter != NULL)
    {
        ClosePort(m_hPrinter);
        m_hPrinter = NULL;
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

INT CDevImpl_BKC310::InitPtr(char *pszPrinterName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_hPrinter = Init_(pszPrinterName);
    if (m_hPrinter == (DEVICEHANDLE)ERR_PARAMATER ||
        m_hPrinter == (DEVICEHANDLE)ERR_FAIL )
    {
        Log(ThisModule, __LINE__, "初始化设备: Init_(%s) fail, Return: %s.",
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
    INT nRet = IMP_SUCCESS;                                                             // 30-00-00-00(FT#0067)

    // Open模式: 0 >> 首先以class mode方式打开,失败则尝试以API模式打开                        // 30-00-00-00(FT#0067)
    //          1 >> 指定内部ID以以API模式打开                                             // 30-00-00-00(FT#0067)
    if (m_nOpenMode == 0)                                                               // 30-00-00-00(FT#0067)
    {                                                                                   // 30-00-00-00(FT#0067)
        // 尝试以class mode方式打开
        /*int*/ nRet = EnumDeviceInfo(PORT_USBDEVICE_CLASS, szDeviceNameListStr, sizeof(szDeviceNameListStr), // 30-00-00-00(FT#0067)
                                  &nDeviceNameNum);
        if (nRet != IMP_DEV_ERR_SUCCESS)
        {
            if (m_nRetErrOLD[3] != nRet)
            {
                Log(ThisModule, __LINE__, "class mode方式连接设备: 枚举设备名称: EnumDeviceInfo(%d) fail. ErrCode: %s, Not Return.",
                    PORT_USBDEVICE_CLASS, ConvertErrCodeToStr(nRet));
                m_nRetErrOLD[3] = nRet;
            }
        } else
        {
            //Different device name split by @
            char szDeviceNameList[1][MAX_PATH] = {0};
            nRet = SplitString(szDeviceNameListStr, '@', szDeviceNameList, 1);
            if (nRet == 0)
            {
                if (m_nRetErrOLD[4] != nRet)
                {
                    Log(ThisModule, __LINE__,
                        "class mode方式连接设备: 解析枚举的设备名称: Device name list is empty(%d), Not Return.", nRet);
                    m_nRetErrOLD[4] = nRet;
                }
            } else
            {    //class mode
                nRet = OpenUsbClassPort(m_hPrinter, szDeviceNameList[0], 0);
                if (nRet != IMP_DEV_ERR_SUCCESS)
                {
                    if (m_nRetErrOLD[5] != nRet)
                    {
                        Log(ThisModule, __LINE__,
                            "class mode方式连接设备: 打开USB类模式设备: OpenUsbClassPort(%s) fail. ErrCode: %s, Not Return.",
                            szDeviceNameList[0], ConvertErrCodeToStr(nRet));
                        m_nRetErrOLD[5] = nRet;
                    }
                } else
                {
                    bClassModeOpenRes = true;
                    if (m_nRetErrOLD[5] != nRet)
                    {
                        Log(ThisModule, __LINE__,
                            "class mode方式连接设备: 打开USB类模式设备: OpenUsbClassPort(%s) succ. Code: %s.",
                        szDeviceNameList[0], ConvertErrCodeToStr(nRet));
                        m_nRetErrOLD[5] = nRet;
                    }
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
                        if (m_nRetErrOLD[6] != nRet)
                        {
                            Log(ThisModule, __LINE__,
                                "API模式连接设备: 打开USB API模式设备: OpenUsbApiPort(232, 720, -1) fail. Return: %s.",
                            ConvertErrCodeToStr(nRet));
                            m_nRetErrOLD[6] = nRet;
                        }
                        return nRet;
                    } else {
                        Log(ThisModule, __LINE__, "API模式连接设备: 打开USB API模式设备: OpenUsbApiPort(-1) succ.");
                    }
                } else {
                    Log(ThisModule, __LINE__, "API模式连接设备: 打开USB API模式设备: OpenUsbApiPort(720) succ.");
                }
            } else {
                Log(ThisModule, __LINE__, "API模式连接设备: 打开USB API模式设备: OpenUsbApiPort(232) succ.");
            }
        }
    } else                                                                              // 30-00-00-00(FT#0067)
    if (m_nOpenMode == 1)                                                               // 30-00-00-00(FT#0067)
    {                                                                                   // 30-00-00-00(FT#0067)
        nRet = OpenUsbApiPort(m_hPrinter, m_nDeviceID);                                 // 30-00-00-00(FT#0067)
        if (nRet != ERR_SUCCESS)                                                        // 30-00-00-00(FT#0067)
        {                                                                               // 30-00-00-00(FT#0067)
            if (m_nRetErrOLD[6] != nRet)                                                // 30-00-00-00(FT#0067)
            {                                                                           // 30-00-00-00(FT#0067)
                Log(ThisModule, __LINE__,                                               // 30-00-00-00(FT#0067)
                    "API模式连接设备: 指定内部ID打开设备: OpenUsbApiPort(%d) fail. Return: %s.",// 30-00-00-00(FT#0067)
                    m_nDeviceID, ConvertErrCodeToStr(nRet));                            // 30-00-00-00(FT#0067)
                m_nRetErrOLD[6] = nRet;                                                 // 30-00-00-00(FT#0067)
            }                                                                           // 30-00-00-00(FT#0067)
            return nRet;                                                                // 30-00-00-00(FT#0067)
        } else                                                                          // 30-00-00-00(FT#0067)
        {                                                                               // 30-00-00-00(FT#0067)
            Log(ThisModule, __LINE__,                                                   // 30-00-00-00(FT#0067)
                "API模式连接设备: 指定内部ID打开设备: OpenUsbApiPort(%d) succ.", m_nDeviceID);// 30-00-00-00(FT#0067)
        }                                                                               // 30-00-00-00(FT#0067)
    } else                                                                              // 30-00-00-00(FT#0067)
    {                                                                                   // 30-00-00-00(FT#0067)
        return IMP_ERR_PARAM_INVALID;                                                   // 30-00-00-00(FT#0067)
    }                                                                                   // 30-00-00-00(FT#0067)

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
        Log(ThisModule, __LINE__, "设备复位: Reset fail. Return: %s.", ConvertErrCodeToStr(nRet));
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
                Log(ThisModule, __LINE__, "实时查询设备状态: RealTimeQueryStatus() fail. Return: %s.", ConvertErrCodeToStr(nRet));
                m_nRealTimeStatusRetOLD = nRet;
            }
            return nRet;
        }
    } else {
        Log(ThisModule, __LINE__, "实时查询设备状态: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
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
        Log(ThisModule, __LINE__, "getRtStatus()->RealTimeQueryStatus(): 状态结果比较: %s.", szTmp);
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
            Log(ThisModule, __LINE__, "NonRealTimeQueryStatus() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    } else {
        Log(ThisModule, __LINE__, "非实时查询设备状态: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
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
                Log(ThisModule, __LINE__, "自动状态返回查询设备状态: AutoQueryStatus() fail. Return: %s.", ConvertErrCodeToStr(nRet));
                m_nAutoStatusRetOLD = nRet;
            }
            return nRet;
        }
    } else {
        Log(ThisModule, __LINE__, "Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
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
        Log(ThisModule, __LINE__, "GetAutoStatus()->AutoQueryStatus(): 状态结果比较: %s.", szTmp);
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
        Log(ThisModule, __LINE__, "获取设备固件版本号: FirmwareVersion() fail. Return: %s.", ConvertErrCodeToStr(nRet));
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
        Log(ThisModule, __LINE__, "获取软件版本号: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = SoftwareVersion(m_hPrinter, pVerBuff, *ulVerBuffSize, ulVerBuffSize);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取软件版本号: SoftwareVersion() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::GetHardSerialNo(LPSTR pBuff, ULONG *ulVerBuffSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (m_bDevOpenOk != TRUE || pBuff == NULL || ulVerBuffSize == 0)
    {
        Log(ThisModule, __LINE__, "获取硬件序列号: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = HardwareSerialNumber(m_hPrinter, pBuff, *ulVerBuffSize, ulVerBuffSize);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取硬件序列号: HardwareSerialNumber() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::GetPrinterName(LPSTR pBuff, ULONG *ulVerBuffSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (m_bDevOpenOk != TRUE || pBuff == NULL || ulVerBuffSize == 0)
    {
        Log(ThisModule, __LINE__, "获取打印机名: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = PrinterName(m_hPrinter, pBuff, *ulVerBuffSize, ulVerBuffSize);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取打印机名: PrinterName() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::GetVendorInfo(LPSTR pBuff, ULONG *ulVerBuffSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    if (m_bDevOpenOk != TRUE || pBuff == NULL || ulVerBuffSize == 0)
    {
        Log(ThisModule, __LINE__, "获取厂商信息: Input fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = VendorInformation(m_hPrinter, pBuff, *ulVerBuffSize, ulVerBuffSize);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取厂商信息: VendorInformation() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	指定打印纸类型及走纸长度进行切纸
 @参数：	bDetectBlackStripe  : TRUE,黑标纸; FALSE,连续纸
 @      nFeedLength    　　　: 走纸长度,单位:0.1MM
 @      nCutMode: 0:全切, 1:走纸一段距离并半切, 2：走纸一段距离并全切,3:半切
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::SetCutPaper(BOOL bDetectBlackStripe, int nFeedLength, INT nCutMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = IMP_SUCCESS;
    //BYTE bySndData[5] = { 0x1B, 0x63, 0x30, 0x00, 0x00 };
    //INT nRetSndLen = 0;

    if (bDetectBlackStripe == TRUE) // 指定为黑标纸
    {
        nRet = SelectPaperType(m_hPrinter, 1);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "设定纸类型为黑标纸: SelectPaperType(1) fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
        //bySndData[3] = 1;
        //Log(ThisModule, __LINE__, "设定纸类型为黑标纸硬件命令: %02X, %02X, %02X, %02X.", bySndData[0], bySndData[1], bySndData[2], bySndData[3]);
    } else {  // 指定为连续纸
        nRet = SelectPaperType(m_hPrinter, 0);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "设定纸类型为连续纸: SelectPaperType(0) fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
        //bySndData[3] = 0;
        //Log(ThisModule, __LINE__, "设定纸类型为连续纸硬件命令: %02X, %02X, %02X, %02X.", bySndData[0], bySndData[1], bySndData[2], bySndData[3]);
    }

    /*nRet = SendPortData(m_hPrinter, (LPSTR)bySndData, 4, &nRetSndLen);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "下发设定纸类型硬件命令: SendPortData(%d, CMD, 4, %d) Fail, ErrCode: %d, Return: %s",
            m_hPrinter, 4, nRetSndLen, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    // 切纸
    nRet = CutPaper(m_hPrinter, nCutMode, MM2SPOT(nFeedLength));
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "切纸+走纸: CutPaper(%d, %d|点:%d) fail. Return: %s.",
            nCutMode, nFeedLength, MM2SPOT(nFeedLength), ConvertErrCodeToStr(nRet));
        return nRet;
    }
    Log(ThisModule, __LINE__, "切纸+走纸: CutPaper(%d, %d|点:%d) Succ.",
        nCutMode, nFeedLength, MM2SPOT(nFeedLength));

    return nRet;
}

/**
 @功能：	行模式打印字符串
 @参数：	lpPrintString  : 待打印字符串
 @      wStringSize    : 待打印字符串size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintText(LPSTR lpPrintString, WORD wStringSize, WORD wStartX, BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpPrintString, wStringSize);
    //Log(ThisModule, __LINE__, "行模式打印字符串: StartX:%d, DataSize:%d, PrintData:%s.", wStartX, wStringSize, strResult.c_str());

    if (wStringSize < 1)
    {
        Log(ThisModule, __LINE__, "行模式打印字符串: 打印数据长度[wStringSize = %d] < 1, NotPrint, Return: %s",
            wStringSize, ConvertErrCodeToStr(IMP_SUCCESS));
        return IMP_SUCCESS;
    }

    /*if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印字符串: 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    if ((nRet = PrintFrontSetFormat()) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印字符串: 设置打印参数: PrintFrontSetFormat() fail. Return: %s.",
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (strlen((char*)byFontType) > 0 && wFontSize > 0)
    {
        nRet = PrintTrueType(m_hPrinter, (char *)strResult.c_str(), wStartX, -1,
                                          (char*)byFontType, wFontSize, wFontSize, 0);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印字符串: 打印TrueType字体: PrintTrueType(FontType=%s, FontSize=%d) fail. Return: %s.",
                byFontType, wFontSize, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    } else {
        nRet = PrintTextOut(m_hPrinter, (char *)strResult.c_str(), wStartX, -1);
        if ( nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印字符串: 打印数据送入打印缓冲区: PrintTextOut() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印字符串: 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/**
 @功能：	行模式指定属性打印字符串
 @参数：	lpPrintString  : 待打印字符串
 @      wStringSize    : 待打印字符串size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintTextUniversal(LPSTR lpPrintString, WORD wStringSize, WORD wStartX,
                                                WORD wDouW, WORD wDouH, WORD nFontType, WORD nFontStyle,
                                                BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpPrintString, wStringSize);
    //Log(ThisModule, __LINE__, "行模式指定属性打印字符串: StartX:%d, DataSize:%d, PrintData:%s.", wStartX, wStringSize, strResult.c_str());

    if (wStringSize < 1)
    {
        Log(ThisModule, __LINE__, "行模式指定属性打印字符串: 打印数据长度[wStringSize = %d] < 1, NotPrint, Return: %s",
            wStringSize, ConvertErrCodeToStr(IMP_SUCCESS));
        return IMP_SUCCESS;
    }

    /*if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式指定属性打印字符串: 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    nRet = UniversalTextOut(m_hPrinter, (char *)strResult.c_str(), wStartX, -1,
                            wDouW, wDouH, nFontType, nFontStyle);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式指定属性打印字符串: 打印数据送入打印缓冲区: UniversalTextOut() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式指定属性打印字符串: 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/**
 @功能：	行模式TureType模式指定属性打印字符串
 @参数：	lpPrintString  : 待打印字符串
 @      wStringSize    : 待打印字符串size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintTrueType(LPSTR lpPrtData, INT nPrtDataSize, INT  nStartX,
                                           LPSTR lpFontName, INT nFontHeight, INT nFontWidth,
                                           BOOL bIsBold, BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = IMP_SUCCESS;
    string strResult;
    strResult.append(lpPrtData, nPrtDataSize);

    if (nPrtDataSize < 1)
    {
        Log(ThisModule, __LINE__,
            "行模式TureType模式指定属性打印字符串: 打印数据长度[nPrtDataSize = %d] < 1, NotPrint, Return: %s",
            nPrtDataSize, ConvertErrCodeToStr(IMP_SUCCESS));
        return IMP_SUCCESS;
    }

    // 字符集转换(GBK->UTF8)
    QTextCodec *codec = QTextCodec::codecForLocale();       // 取当前字符集
    QTextCodec *qtUTF8 = QTextCodec::codecForName("UTF-8"); // UTF8字符集
    QTextCodec *qtGBK = QTextCodec::codecForName("GB18030");// GBK字符集
    QTextCodec::setCodecForLocale(qtGBK);                   // 设置当前为GBK字符集
    QString qsText = QString::fromLocal8Bit(lpPrtData);     // 导入GBK数据
    QTextCodec::setCodecForLocale(qtUTF8);                  // 设置当前为UTF8字符集
    QByteArray tmpData = qsText.toUtf8();                   // 转换为UTF8格式数据
    QTextCodec::setCodecForLocale(codec);                   // 还原当前字符集
    std::string stdUTF8Data;
    stdUTF8Data.append(tmpData.data());

    // 打印
    nRet = PrintTrueType(m_hPrinter, /*lpPrtData, nStartX, */
                         (LPSTR)stdUTF8Data.c_str(), stdUTF8Data.length(), -1,
                         lpFontName, nFontHeight, nFontWidth, (INT)bIsBold);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "行模式TureType模式指定属性打印字符串: 打印数据送入打印缓冲区: "
            "PrintTrueType(%d, %s, %d, %d, %s, %d, %d, %d) fail, ErrCode: %d, Return: %s.",
            m_hPrinter, /*lpPrtData, nStartX*/stdUTF8Data.c_str(), stdUTF8Data.length(), -1,
            lpFontName, nFontHeight, nFontWidth,
            bIsBold, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "行模式TureType模式指定属性打印字符串: 换行: FeedLine() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertErrCodeToStr(nRet));
            return nRet;
        }
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
INT CDevImpl_BKC310::LineModePrintImage(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX, BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    Log(ThisModule, __LINE__, "行模式打印图片: StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());

    /*if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印图片: 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    nRet = PrintBitmap(m_hPrinter, (char *)strResult.c_str(), wStartX, -1, 1);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印图片: 打印位图: PrintBitmap() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印图片: 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
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
INT CDevImpl_BKC310::LineModePrintImageRAM(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX, BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    // Log(ThisModule, __LINE__, "行模式打印图片(预下载RAM方式): StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());

    /*if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印图片(预下载RAM方式): 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    nRet = DownloadRAMBitmapByFile(m_hPrinter, (char *)strResult.c_str(), 6);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印图片(预下载RAM方式): 预下载位图到设备RAM中: DownloadRAMBitmapByFile() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return FALSE;
    }

    nRet = PrintRAMBitmap(m_hPrinter, 6, wStartX, -1, 0);
    if ( nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印图片(预下载RAM方式): 打印RAM中位图: PrintRAMBitmap() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return FALSE;

    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印图片(预下载RAM方式): 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
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
INT CDevImpl_BKC310::LineModePrintImageByMode(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX, BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    // Log(ThisModule, __LINE__, "行模式打印图片(ByMode方式): StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());

    /*if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印图片(ByMode方式): 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    nRet = PrintBitmapByMode(m_hPrinter, (char *)strResult.c_str(), wStartX, -1, BITMAP_MODE_24DOUBLE_DENSITY);
    if ( nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印图片(ByMode方式): 打印单色位图: PrintBitmapByMode() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印图片(ByMode方式): 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/**
 @功能：	行模式打印条形码
 @参数：	lpData: 打印数据        wDataSize: 数据size        StartX: 列坐标
 @      wType: 条码类型         wBasicWidth: 基本元素宽度点数 wHeight: 元素高度点数
 @      wHriFontType: HRI字符类型 wHriFontPos: HRI字符位置   bFeedLine: 是否执行换行
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintBarcode(LPSTR lpData, WORD wDataSize, WORD wStartX, WORD wType,
                                          WORD wBasicWidth, WORD wHeight, WORD wHriFontType,
                                          WORD wHriFontPos, BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    //Log(ThisModule, __LINE__, "行模式打印条形码: StartX:%d, DataSize:%d, Data:%s, wType:%d, BasicW:%d, Height:%d,"
    //                          "HriFontType:%d, HriFontPos:%d.",
    //    wStartX, wDataSize, lpData, wType, wBasicWidth, wHeight, wHriFontType, wHriFontPos);

    /*if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印条形码: 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    nRet = PrintBarcode(m_hPrinter, lpData, wStartX, -1, wType, wBasicWidth, wHeight,
                        wHriFontType, wHriFontPos, wDataSize);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印条形码: PrintBarcode(%d, %s, %d, %d, %d, "
                                  "%d, %d, %d, %d, %d) fail. Return: %s.",
            m_hPrinter, lpData, wStartX, -1, wType, wBasicWidth, wHeight,
            wHriFontType, wHriFontPos, wDataSize, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印条形码: 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/**
 @功能：	行模式打印PDF417码
 @参数：	lpData: 打印数据        wDataSize: 数据size        StartX: 列坐标
 @      wBasicWidth: 基本元素宽度点数   wHeight: 元素高度点数  wLines: 行数
 @      wCols: 列数            wScaleH: 外观比高度           wScaleV: 外观比宽度
 @      wErrCor: 纠错级别       bFeedLine: 是否换行
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintPDF417(LPSTR lpData, WORD wDataSize, WORD wStartX, WORD wBasicWidth,
                                         WORD wHeight, WORD wLines, WORD wCols, WORD wScaleH, WORD wScaleV,
                                         WORD wErrCor, BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    //Log(ThisModule, __LINE__, "行模式打印PDF417码: StartX:%d, DataSize:%d, Data:%s, BasicW:%d, Height:%d,"
    //                          "Lines:%d, Cols:%d, ScaleH:%d, ScaleV:%d, wErrCor:%d.",
    //    wStartX, wDataSize, lpData, wBasicWidth, wHeight, wLines, wCols, wScaleH, wScaleV, wErrCor);

    /*if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印PDF417码: 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    nRet = BarcodePrintPDF417(m_hPrinter, lpData, wStartX, -1, wBasicWidth, wHeight, wLines,
                              wCols, wScaleH, wScaleV, wErrCor, wDataSize);
    //nRet = BarcodePrintPDF417Simple(m_hPrinter, lpData, wStartX, -1, 80, 25);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印PDF417码: BarcodePrintPDF417(%d, %s, %d, %d, %d, "
                                  "%d, %d, %d, %d, %d, %d, %d) fail. Return: %s.",
            m_hPrinter, lpData, wStartX, -1, wBasicWidth, wHeight, wLines, wCols, wScaleH,
            wScaleV, wErrCor, wDataSize, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印PDF417码: 换行: FeedLine() fail. Return: %s.",
                ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/**
 @功能：	行模式打印二维码
 @参数：	lpData: 打印数据        wDataSize: 数据size        StartX: 列坐标
 @      wBasicWidth: 基本元素宽度点数   wSymbolType: 符号类型  wLangMode: 语言模式
 @      wErrCor: 纠错级别       bFeedLine: 是否换行
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::LineModePrintQrcode(LPSTR lpData, WORD wDataSize, WORD wStartX, WORD wBasicWidth,
                                         WORD wSymbolType, WORD wLangMode, WORD wErrCor, BOOL bFeedLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = 0;
    string strResult;
    //Log(ThisModule, __LINE__, "行模式打印二维码: StartX:%d, DataSize:%d, Data:%s, BasicW:%d, SymbolType:%d,"
    //                          "LangMode:%d, wErrCor:%d.",
    //   wStartX, wDataSize, lpData, wBasicWidth, wSymbolType, wLangMode, wErrCor);

    /*if ((nRet = BK_SetPrintMode(PRINT_LINE_MODE)) != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印二维码: 设置打印模式: BK_SetPrintMode(%d|LineMode) fail. Return: %s.",
            PRINT_LINE_MODE, ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    nRet = BarcodePrintQR(m_hPrinter, lpData, wStartX, -1, wBasicWidth, wSymbolType,
                          wLangMode, wErrCor, wDataSize);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "行模式打印二维码: BarcodePrintQR(%d, %s, %d, %d, %d, "
                                  "%d, %d, %d, %d) fail. Return: %s.",
            m_hPrinter, lpData, wStartX, -1, wBasicWidth, wSymbolType,
            wLangMode, wErrCor, wDataSize, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    if (bFeedLine == TRUE)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "行模式打印二维码: 换行: FeedLine() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/**
 @功能：	打印换行
 @参数：	nLine 换行数
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::PrtFeedLine(INT nLine)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = IMP_SUCCESS;

    for (INT i = 0; i < nLine; i ++)
    {
        nRet = FeedLine(m_hPrinter);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印换行: 换行%d次: FeedLine() fail. Return: %s.", i+ 1, ConvertErrCodeToStr(nRet));
            return nRet;
        }
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
            Log(ThisModule, __LINE__, "打印参数设置: 设置打印行高: BK_SetTextLineHight(%d) fail. Return: %s.",
                wLineHeight, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    if (wCharSpace > 0) // 设置字符间距
    {
        if ((nRet = BK_SetTextCharacterSpace(wCharSpace)) != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印参数设置: 设置字符间距: BK_SetTextCharacterSpace(%d) fail. Return: %s.",
                wCharSpace, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

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
        Log(ThisModule, __LINE__, "设置打印模式: PrintSetMode(%d:0行/2页) fail. Return: %s.", wPrintMode, ConvertErrCodeToStr(nRet));
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
        Log(ThisModule, __LINE__, "SetTextLineHight(%d|%d) fail. Return: %s.",
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
        Log(ThisModule, __LINE__, "SetTextCharacterSpace(0, %d|%d, 0) fail. Return: %s.",
            nCharSpace, MM2SPOT(nCharSpace), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    nRet = SetTextCharacterSpace(m_hPrinter, 0,
                                 (IsSpot == TRUE ? nCharSpace : MM2SPOT(nCharSpace)), 1);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SetTextCharacterSpace(0, %d|%d, 1) fail. Return: %s.",
            nCharSpace, MM2SPOT(nCharSpace), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 设置物理黑标位移
INT CDevImpl_BKC310::BK_SetBlackMove(UINT uMoveSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRetSndLen = 0;
    CHAR szSndData[10] = { 0x1B, 0x73, 0x42, 0x45, 0x92,
                           0x9A, 0x00, 0x00, 0x55, 0x0A };

    if (uMoveSize > 0)
    {
        szSndData[6] = (uMoveSize * 8 % 256);
        szSndData[7] = (uMoveSize * 8 / 256);
    } else
    {
        szSndData[7] = 0x01;    // 初始位置
    }

    int nRet = SendPortData(m_hPrinter, szSndData, 10, &nRetSndLen);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设置物理黑标位移: BK_SetBlackMove(data<%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X>, "
            "sndLen<10>, rcvLen<%d>) fail. Return: %s.",
            szSndData[0], szSndData[1], szSndData[2], szSndData[3], szSndData[4],
            szSndData[5], szSndData[6], szSndData[7], szSndData[8], szSndData[9],
            nRetSndLen, ConvertErrCodeToStr(nRet));
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__,
            "设置物理黑标位移: BK_SetBlackMove(data<%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X>, "
            "sndLen<10>, rcvLen<%d>) succ.",
            szSndData[0], szSndData[1], szSndData[2], szSndData[3], szSndData[4],
            szSndData[5], szSndData[6], szSndData[7], szSndData[8], szSndData[9],
            nRetSndLen);
    }

    return IMP_SUCCESS;
}

/**
 @功能：	设置字体加粗
 @参数：	wBold 0不加粗/1加粗
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::BK_SetTextBold(WORD wBold /* = 0 */)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = SetTextBold(m_hPrinter, (wBold == 0 ? wBold : 1));
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SetTextBold(%d) fail. Return: %s.",
            wBold == 0 ? wBold : 1, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	设置倍宽倍高
 @参数：	wWidth  倍宽   wHeight 倍高
 @返回：	TRUE/FALSE
 */
INT CDevImpl_BKC310::BK_SetTextDoubleWH(WORD wWidth, WORD wHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = SetTextDoubleWidthAndHeight(m_hPrinter, wWidth, wHeight);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SetTextDoubleWidthAndHeight(%d, %d) fail. Return: %s.",
            wWidth, wHeight);
        return nRet;
    }

    return IMP_SUCCESS;
}

INT CDevImpl_BKC310::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

// 设置动态库路径(DeviceOpen前有效)
void CDevImpl_BKC310::SetLibPath(LPCSTR lpPath)
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

// 设置打开模式(DeviceOpen前有效)    // 30-00-00-00(FT#0067) ADD START
void CDevImpl_BKC310::SetOpenMode(INT nMode, INT nDevID)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    m_nOpenMode = nMode;
    m_nDeviceID = nDevID;

    Log(ThisModule, __LINE__, "设置打开模式: Mode = %d, DeviceID = %d.",
        m_nOpenMode, m_nDeviceID);
}
// 30-00-00-00(FT#0067) ADD END

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

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
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
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Succ. ", m_szLoadDllPath);
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
        vInitLibFunc();
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
    LOAD_LIBINFO_FUNC(pBarcodePrintPDF417Simple, BarcodePrintPDF417Simple, "BarcodePrintPDF417Simple");
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

void CDevImpl_BKC310::vInitLibFunc()
{
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
    BarcodePrintPDF417Simple = NULL;
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

BOOL CDevImpl_BKC310::bDlLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = TRUE;

    if (m_vDlHandle == nullptr)
    {
        m_vDlHandle = dlopen(m_szLoadDllPath, RTLD_LAZY);//RTLD_NOW | RTLD_DEEPBIND);
        if (m_vDlHandle == nullptr)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "dlxxx方式加载动态库<%s> fail.", m_szLoadDllPath);
            }
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "dlxxx方式加载动态库<%s> succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bDlLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail. ", m_szLoadDllPath);
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_BKC310::vDlUnLoadLibrary()
{
    if (m_vDlHandle != nullptr)
    {
        dlclose(m_vDlHandle);
        m_vDlHandle = nullptr;
        m_bLoadIntfFail = TRUE;
        vInitLibFunc();
    }
}

BOOL CDevImpl_BKC310::bDlLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    //connect, port
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pEnumDeviceInfo, EnumDeviceInfo, "EnumDeviceInfo");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenUsbClassPort, OpenUsbClassPort, "OpenUsbClassPort");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pInit, Init_, "Init");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pClosePort, ClosePort, "ClosePort");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSendPortData, SendPortData, "SendPortData");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pReadPortData, ReadPortData, "ReadPortData");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetPortTimeout, SetPortTimeout, "SetPortTimeout");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pGetPortTimeout, GetPortTimeout, "GetPortTimeout");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenUsbApiPort, OpenUsbApiPort, "OpenUsbApiPort");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenNetPort, OpenNetPort, "OpenNetPort");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenCOMPort, OpenCOMPort, "OpenCOMPort");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenLPTPort, OpenLPTPort, "OpenLPTPort");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenNetPortByName, OpenNetPortByName, "OpenNetPortByName");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenBlueToothPortByName, OpenBlueToothPortByName, "OpenBlueToothPortByName");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenBlueToothPort, OpenBlueToothPort, "OpenBlueToothPort");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenDriverPort, OpenDriverPort, "OpenDriverPort");

    //query
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRealTimeQueryStatus, RealTimeQueryStatus, "RealTimeQueryStatus");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pNonRealTimeQueryStatus, NonRealTimeQueryStatus, "NonRealTimeQueryStatus");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pAutoQueryStatus, AutoQueryStatus, "AutoQueryStatus");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pFirmwareVersion, FirmwareVersion, "FirmwareVersion");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSoftwareVersion, SoftwareVersion, "SoftwareVersion");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pVendorInformation, VendorInformation, "VendorInformation");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrinterName, PrinterName, "PrinterName");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pResolutionRatio, ResolutionRatio, "ResolutionRatio");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pHardwareSerialNumber, HardwareSerialNumber, "HardwareSerialNumber");

    //text
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pFeedLine, FeedLine, "FeedLine");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintTextOut, PrintTextOut, "PrintTextOut");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pUniversalTextOut, UniversalTextOut, "UniversalTextOut");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextLineHight, SetTextLineHight, "SetTextLineHight");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextBold, SetTextBold, "SetTextBold");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextDoubleWidthAndHeight, SetTextDoubleWidthAndHeight, "SetTextDoubleWidthAndHeight");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetAlignmentMode, SetAlignmentMode, "SetAlignmentMode");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextCharacterSpace, SetTextCharacterSpace, "SetTextCharacterSpace");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextMagnifyTimes, SetTextMagnifyTimes, "SetTextMagnifyTimes");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextFontType, SetTextFontType, "SetTextFontType");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextUpsideDownMode, SetTextUpsideDownMode, "SetTextUpsideDownMode");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextOppositeColor, SetTextOppositeColor, "SetTextOppositeColor");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextColorEnable, SetTextColorEnable, "SetTextColorEnable");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextFontColor, SetTextFontColor, "SetTextFontColor");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextUnderline, SetTextUnderline, "SetTextUnderline");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextRotate, SetTextRotate, "SetTextRotate");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextCharsetAndCodepage, SetTextCharsetAndCodepage, "SetTextCharsetAndCodepage");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextUserDefinedCharacterEnable, SetTextUserDefinedCharacterEnable, "SetTextUserDefinedCharacterEnable");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetTextDefineUserDefinedCharacter, SetTextDefineUserDefinedCharacter, "SetTextDefineUserDefinedCharacter");

    //bitmap
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintBitmap, PrintBitmap, "PrintBitmap");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintBitmapByMode, PrintBitmapByMode, "PrintBitmapByMode");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pDownloadRAMBitmapByFile, DownloadRAMBitmapByFile, "DownloadRAMBitmapByFile");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintRAMBitmap, PrintRAMBitmap, "PrintRAMBitmap");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pDownloadFlashBitmapByFile, DownloadFlashBitmapByFile, "DownloadFlashBitmapByFile");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintFlashBitmap, PrintFlashBitmap, "PrintFlashBitmap");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintTrueType, PrintTrueType, "PrintTrueType");

    //barcode
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintBarcode, PrintBarcode, "PrintBarcode");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintBarcodeSimple, PrintBarcodeSimple, "PrintBarcodeSimple");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pBarcodePrintQR, BarcodePrintQR, "BarcodePrintQR");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pBarcodePrintPDF417, BarcodePrintPDF417, "BarcodePrintPDF417");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pBarcodePrintPDF417Simple, BarcodePrintPDF417Simple, "BarcodePrintPDF417Simple");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pBarcodePrintMaxicode, BarcodePrintMaxicode, "BarcodePrintMaxicode");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pBarcodePrintGS1DataBar, BarcodePrintGS1DataBar, "BarcodePrintGS1DataBar");

    //basic set
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pDownloadFile, DownloadFile, "DownloadFile");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintSetMode, PrintSetMode, "PrintSetMode");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPageModeSetArea, PageModeSetArea, "PageModeSetArea");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPageModePrint, PageModePrint, "PageModePrint");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPageModeClearBuffer, PageModeClearBuffer, "PageModeClearBuffer");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pFeedLineNumber, FeedLineNumber, "FeedLineNumber");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCutPaper, CutPaper, "CutPaper");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pReset, Reset_, "Reset");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pKickOutDrawer, KickOutDrawer, "KickOutDrawer");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pApplicationUnit, ApplicationUnit, "ApplicationUnit");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPrintDensity, PrintDensity, "PrintDensity");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pMotionUnit, MotionUnit, "MotionUnit");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSelectPaperType, SelectPaperType, "SelectPaperType");
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSelectPaperTypeEEP, SelectPaperTypeEEP, "SelectPaperTypeEEP");

    return TRUE;
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

    return m_szErrStr;
}

