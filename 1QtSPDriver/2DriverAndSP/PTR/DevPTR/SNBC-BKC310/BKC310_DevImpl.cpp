#include "BKC310_DevImpl.h"
#include "PossdkIntf.h"
#include "BKC310Def.h"

static const char *ThisFile = "BKC310_DevImpl.cpp";

CBKC310_DevImpl::CBKC310_DevImpl()
{
    SetLogFile(LOG_NAME_BKC310, ThisFile);  // 设置日志文件名和错误发生的文件

    //m_hBTNHDll = NULL;
    m_hPrinter = NULL;
    m_bDevOpenOk = FALSE;
    //ClearErrorCode(&m_errorInfo);

    memset(byFontType, 0x00, sizeof(byFontType));   // 字体
    wFontSize = 11;         // 五号字体(单位:磅)
    wLineHeight = 0;        // 行高，缺省: 30，单位:0.1mm
    wCharSpace = 0;         // 字符间距，缺省: 0，单位:0.1mm
    wRowDistance = 0;       // 行间距，缺省: 0，单位:0.1mm
    wLeft = 0;              // 左边距，缺省: ０，单位:0.1mm
    wTop = 0;               // 上边距，缺省: ０0，单位:0.1mm
    wWidth = 0;             // 可打印宽，缺省: 0，单位:0.1mm
    wHeight = 0;            // 可打印高，缺省: 0，单位:0.1mm

    memset(m_szAutoStatusOLD, 0x00, sizeof(m_szAutoStatusOLD));        // 上一次自动获取状态保留  // 30-00-00-00(FT#0052)
    memset(m_szRealTimeStatusOLD, 0x00, sizeof(m_szRealTimeStatusOLD));// 上一次获取实时状态保留  // 30-00-00-00(FT#0052)    
    m_nAutoStatusRetOLD = ERR_SUCCESS;    // 上一次获取自动状态结果保留 // 30-00-00-00(FT#0052)
    m_nRealTimeStatusRetOLD = ERR_SUCCESS;// 上一次获取实时状态结果保留 // 30-00-00-00(FT#0052)
}

CBKC310_DevImpl::~CBKC310_DevImpl()
{
    CloseDevice();
}

BOOL CBKC310_DevImpl::OpenDevice()
{
    return OpenDevice(DEV_SNBC_BKC310);
}
BOOL CBKC310_DevImpl::OpenDevice(WORD wType)
{
    THISMODULE(__FUNCTION__);

    wDevType = wType;                                                   // 30-00-00-00(FT#0052)

    // 设置LOG                                                           // 30-00-00-00(FT#0052)
    if (wDevType == DEV_SNBC_BKC310)                                    // 30-00-00-00(FT#0052)
    {                                                                   // 30-00-00-00(FT#0052)
        SetLogFile(LOG_NAME_BKC310, ThisFile);                          // 30-00-00-00(FT#0052)
    } else                                                              // 30-00-00-00(FT#0052)
    if (wDevType == DEV_SNBC_BTNH80)                                    // 30-00-00-00(FT#0052)
    {                                                                   // 30-00-00-00(FT#0052)
        SetLogFile(LOG_NAME_BTNH80, ThisFile);                          // 30-00-00-00(FT#0052)
    } else                                                              // 30-00-00-00(FT#0052)
    {                                                                   // 30-00-00-00(FT#0052)
        Log(ThisModule, 1, "打开设备: Input DeviceType[%d] NotFound, DeviceTypeList[%d|%s,%d|%s], Return: %d",
            wDevType, DEV_SNBC_BKC310, "BKC310", DEV_SNBC_BTNH80, "BTNH80", FALSE);// 30-00-00-00(FT#0052)
        return FALSE;                                                   // 30-00-00-00(FT#0052)
    }                                                                   // 30-00-00-00(FT#0052)

    //如果已打开，则关闭
    CloseDevice();

    //wDevType = wType;                                                 // 30-00-00-00(FT#0052)

    m_bDevOpenOk = FALSE;
    if (m_possdkIntf.LoadPossdkDll() != TRUE){
        //SetErrorCode(&m_errorInfo, ERROR_TYPE_SW, "00000001");
        Log(ThisFile, 1, "Load PossdkDll(INI->DriverDllName) fail.");
        return FALSE;
    }

    if (wDevType == DEV_SNBC_BKC310) {
        if (InitPtr(BK_SERIES_PRINTER_NAME) != TRUE){
            return FALSE;
        }
        //SetLogFile(BKC310_LOG_NAME, ThisFile);// 30-00-00-00(FT#0052)
    } else
    if (wDevType == DEV_SNBC_BTNH80) {
        if (InitPtr(BT_SERIES_PRINTER_NAME) != TRUE){
            return FALSE;
        }
        //SetLogFile(BTNH80_LOG_NAME, ThisFile);// 30-00-00-00(FT#0052)
    } else {
        Log(ThisFile, 1, "Input DeviceType[%d] NotFound, DeviceTypeList[%d|%s,%d|%s",
            wDevType, DEV_SNBC_BKC310, BKC310_DEV_MODEL_NAME, DEV_SNBC_BTNH80, BTNH80_DEV_MODEL_NAME);
        return FALSE;
    }

    if (Connect() != TRUE){
        return FALSE;
    }

    m_bDevOpenOk = TRUE;
    return TRUE;
}

BOOL CBKC310_DevImpl::CloseDevice()
{
    if (m_hPrinter != NULL)
    {
        m_possdkIntf.ClosePort(m_hPrinter);
        m_hPrinter = NULL;
    }

    m_possdkIntf.UnloadPossdkDll();
    m_bDevOpenOk = FALSE;

    Log(ThisFile, 1, "Close Device, unLoad PossdkDll.");

    return TRUE;
}

/*BOOL CBKC310_DevImpl::SetPrintSetting(PrinterDevConfig stPrinterDevConfig)
{
    if (m_bDevOpenOk != TRUE){
        return FALSE;
    }
    //Set paper type
    int iRet = m_possdkIntf.SelectPaperType(m_hPrinter, stPrinterDevConfig.stPrinterSetting.wPaperType);
    if (iRet != ERR_SUCCESS){
        Log(ThisFile, 1, "SetPrintSetting()->SelectPaperType fail. ReturnCode:%d.", iRet);
        return FALSE;
    }

    //Set cut position for black paper
    if (stPrinterDevConfig.stPrinterSetting.wPaperType == 1){
        BYTE byHighByte = HIBYTE(stPrinterDevConfig.stPrinterSetting.wFeedLength);
        BYTE byLowerByte = LOBYTE(stPrinterDevConfig.stPrinterSetting.wFeedLength);
//        BYTE byCmdData[10] = {0x1B, 0x73, 0x42, 0x45, 0x92, 0x9A, byLowerByte, byHighByte, 0x2B, 0x0A};
        BYTE byCmdData[] = {0x1B, 0x63, 0x31, byLowerByte, byHighByte};
        int iRetLen = 0;
        iRet = m_possdkIntf.SendPortData(m_hPrinter, (char *)byCmdData, sizeof(byCmdData), &iRetLen);
        if (iRet != ERR_SUCCESS){
            Log(ThisFile, 1, "SetPrintSetting()->SendPortData fail. ReturnCode:%d.", iRet);
            return FALSE;
        }
    }

    return TRUE;
}*/

BOOL CBKC310_DevImpl::InitPtr(char *pszPrinterName)
{
    m_hPrinter = m_possdkIntf.Init(pszPrinterName);
    if (m_hPrinter == (DEVICEHANDLE)ERR_PARAMATER ||
        m_hPrinter == (DEVICEHANDLE)ERR_FAIL )
    {
        Log(ThisFile, 1, "InitPtr()->Init(%s) fail, ReturnCode:%d.", pszPrinterName, m_hPrinter);
        return FALSE;
    }

    return TRUE;
}

BOOL CBKC310_DevImpl::Connect()
{
    char szDeviceNameListStr[MAX_PATH] = {0};
    int iDeviceNameNum = 0;
    bool bClassModeOpenRes = false;
    //尝试以class mode方式打开
    int iRet = m_possdkIntf.EnumDeviceInfo(PORT_USBDEVICE_CLASS, szDeviceNameListStr, sizeof(szDeviceNameListStr), &iDeviceNameNum);
    if (iRet != ERR_SUCCESS){
        Log(ThisFile, 1, "Connect()->EnumDeviceInfo() fail. ReturnCode:%d.", iRet);
    } else {
        //Different device name split by @
        char szDeviceNameList[1][MAX_PATH] = {0};
        iRet = SplitString(szDeviceNameListStr, '@', szDeviceNameList, 1);
        if (iRet == 0){
            Log(ThisFile, 1, "Device name list is empty(%d).", iRet);
        } else {    //class mode
            iRet = m_possdkIntf.OpenUsbClassPort(m_hPrinter, szDeviceNameList[0], 0);
            if (iRet != ERR_SUCCESS)
            {
                Log(ThisFile, 1, "Connect()->OpenUsbClassPort(%s) fail. ReturnCode:%d.", szDeviceNameList[0], iRet);
            } else {
                bClassModeOpenRes = true;
                Log(ThisFile, 1, "Connect()->OpenUsbClassPort(%s) succ. ReturnCode:%d.", szDeviceNameList[0], iRet);
            }
        }
    }

    if (bClassModeOpenRes == false){
        //尝试以API模式
        //API mode,innerDeviceId is 720 or 232
        iRet = m_possdkIntf.OpenUsbApiPort(m_hPrinter, 232);
        if (iRet != ERR_SUCCESS)
        {
            iRet = m_possdkIntf.OpenUsbApiPort(m_hPrinter, 720);
            if (iRet != ERR_SUCCESS){
                iRet = m_possdkIntf.OpenUsbApiPort(m_hPrinter, -1);
                if (iRet != ERR_SUCCESS){
                    Log(ThisFile, 1, "Connect()->OpenUsbApiPort(232,720, -1) fail.");
                    return FALSE;
                }else {
                    Log(ThisFile, 1, "Connect()->OpenUsbApiPort(-1) succ.");
                }
            } else {
                Log(ThisFile, 1, "Connect()->OpenUsbApiPort(720) succ.");
            }
        } else {
            Log(ThisFile, 1, "Connect()->OpenUsbApiPort(232) succ.");
        }
    }

    return TRUE;
}

BOOL CBKC310_DevImpl::Reset()
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "Reset()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int iRet = m_possdkIntf.Reset(m_hPrinter);
    if (iRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "Reset()->Reset fail. ReturnCode:%d.", iRet);
        return FALSE;
    }

    //ClearErrorCode(&m_errorInfo);
    return TRUE;
}

BOOL CBKC310_DevImpl::GetRtStatus(char *pStaBuf, ULONG *pulBufSz)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "getRtStatus()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    if ((pStaBuf != NULL) && (pulBufSz != NULL) && (*pulBufSz > 0)) {
        int iRet = m_possdkIntf.RealTimeQueryStatus(m_hPrinter, pStaBuf, *pulBufSz, pulBufSz);
        if (iRet != ERR_SUCCESS)
        {
            // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余                  // 30-00-00-00(FT#0052)
            if (iRet != m_nRealTimeStatusRetOLD)                                                        // 30-00-00-00(FT#0052)
            {                                                                                           // 30-00-00-00(FT#0052)
                Log(ThisFile, 1, "getRtStatus()->RealTimeQueryStatus() fail. ReturnCode:%d.", iRet);    // 30-00-00-00(FT#0052)
                m_nRealTimeStatusRetOLD = iRet;                                                         // 30-00-00-00(FT#0052)
            }                                                                                           // 30-00-00-00(FT#0052)
            return FALSE;
        }
    } else {
        Log(ThisFile, 1, "getRtStatus()->Input fail.");
        return FALSE;
    }

    // 30-00-00-00(FT#0052) 比较两次状态记录LOG START
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
    // 30-00-00-00(FT#0052) 比较两次状态记录LOG END

    return TRUE;
}

BOOL CBKC310_DevImpl::GetNonRtStatus(char *pStaBuf, ULONG *pulBufSz)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "GetNonRtStatus()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    if ((pStaBuf != NULL) && (pulBufSz != NULL) && (*pulBufSz > 0)) {
        int iRet = m_possdkIntf.NonRealTimeQueryStatus(m_hPrinter, pStaBuf, *pulBufSz, pulBufSz);
        if (iRet != ERR_SUCCESS)
        {
            Log(ThisFile, 1, "GetNonRtStatus()->NonRealTimeQueryStatus() fail. ReturnCode:%d.", iRet);
            return FALSE;
        }
    } else {
        Log(ThisFile, 1, "GetNonRtStatus()->Input fail.");
        return FALSE;
    }

    return TRUE;
}

BOOL CBKC310_DevImpl::GetAutoStatus(char *pStaBuf, ULONG *pulBufSz)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "GetAutoStatus()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    if ((pStaBuf != NULL) && (pulBufSz != NULL) && (*pulBufSz > 0)) {
        int iRet = m_possdkIntf.AutoQueryStatus(m_hPrinter, pStaBuf, *pulBufSz, 1, pulBufSz);
        if (iRet != ERR_SUCCESS)
        {
            // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余               // 30-00-00-00(FT#0052)
            if (iRet != m_nAutoStatusRetOLD)                                                         // 30-00-00-00(FT#0052)
            {                                                                                        // 30-00-00-00(FT#0052)
                Log(ThisFile, 1, "GetAutoStatus()->AutoQueryStatus() fail. ReturnCode:%d.", iRet);   // 30-00-00-00(FT#0052)
                m_nAutoStatusRetOLD = iRet;                                                          // 30-00-00-00(FT#0052)
            }                                                                                        // 30-00-00-00(FT#0052)
            return FALSE;
        }
    } else {
        Log(ThisFile, 1, "GetAutoStatus()->Input fail.");
        return FALSE;
    }

    // 30-00-00-00(FT#0052) 比较两次状态记录LOG START
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
    // 30-00-00-00(FT#0052) 比较两次状态记录LOG END

    return TRUE;
}

BOOL CBKC310_DevImpl::GetFwVersion(LPSTR pFwVerBuff, ULONG *ulFwVerSize)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "GetFwVersion()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int iRet = m_possdkIntf.FirmwareVersion(m_hPrinter, pFwVerBuff, *ulFwVerSize, ulFwVerSize);
    if (iRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "GetFwVersion()->FirmwareVersion() fail. ReturnCode:%d.", iRet);
        return FALSE;
    }

    return TRUE;
}

BOOL CBKC310_DevImpl::GetSoftwareVersion(LPSTR pVerBuff, ULONG *ulVerBuffSize)
{
    if (m_bDevOpenOk != TRUE) {
        Log(ThisFile, 1, "GetSoftwareVersion()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    if (m_bDevOpenOk != TRUE || pVerBuff == NULL || ulVerBuffSize == 0) {
        Log(ThisFile, 1, "GetSoftwareVersion()-> Input fail, return fail.");
        return FALSE;
    }

    int iRet = m_possdkIntf.SoftwareVersion(m_hPrinter, pVerBuff, *ulVerBuffSize, ulVerBuffSize);
    if (iRet != ERR_SUCCESS){
        Log(ThisFile, 1, "GetSoftwareVersion()->SoftwareVersion() fail. ReturnCode:%d.", iRet);
        return FALSE;
    }

    return TRUE;
}

/*BOOL CBKC310_DevImpl::GetPrinterName(LPSTR pszName, WORD &wNameSize)
{
    BOOL bRet = TRUE;

    if (!m_bOpenSuccess)
    {
        return FALSE;
    }

    BYTE usCmd[] = { 0x1d, 0x49, 0x99 };
    bRet = m_lpIDevComm->SendData(usCmd, sizeof(usCmd));
    if (!bRet)
    {
        return FALSE;
    }

    BYTE  usReadData[13] = {0};
    WORD wReadSize = 0;

    wReadSize = m_lpIDevComm->ReceiveData(usReadData, BK_COMMAND_READ_TIMEOUT);
    if (wReadSize > 12)
    {
        memcpy( pszName, (LPSTR)usReadData, wReadSize );
        wNameSize = wReadSize;
    }

    return bRet;
}*/

/**
 @功能：	指定走纸长度进行切纸
 @参数：	nFeedLength    　　　: 走纸长度,单位:0.1MM
 @返回：	TRUE/FALSE
 */
BOOL CBKC310_DevImpl::ControlMedia(int nFeedLength)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "ControlMedia()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int iRet = m_possdkIntf.CutPaper(m_hPrinter, 2, MM2SPOT(nFeedLength));
    if (iRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "ControlMedia()->CutPaper(2, %d|%d) fail. ReturnCode:%d.",
            nFeedLength, MM2SPOT(nFeedLength), iRet);
        return FALSE;
    }

    return TRUE;
}

/**
 @功能：	指定打印纸类型及走纸长度进行切纸
 @参数：	bDetectBlackStripe  : TRUE,黑标纸; FALSE,连续纸
 @      nFeedLength    　　　: 走纸长度,单位:0.1MM
 @返回：	TRUE/FALSE
 */
BOOL CBKC310_DevImpl::ControlMedia(BOOL bDetectBlackStripe, int nFeedLength)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "ControlMedia()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    if (bDetectBlackStripe == TRUE) {   // 指定为黑标纸
        int iRet = m_possdkIntf.SelectPaperType(m_hPrinter, 1);
        if (iRet != ERR_SUCCESS)
        {
            Log(ThisFile, 1, "ControlMedia()->SelectPaperType(1) fail. ReturnCode:%d.", iRet);
            return FALSE;
        }
    } else {  // 指定为连续纸
        int iRet = m_possdkIntf.SelectPaperType(m_hPrinter, 0);
        if (iRet != ERR_SUCCESS)
        {
            Log(ThisFile, 1, "ControlMedia()->SelectPaperType(0) fail. ReturnCode:%d.", iRet);
            return FALSE;
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
BOOL CBKC310_DevImpl::LineModePrintText(LPSTR lpPrintString, WORD wStringSize, WORD wStartX)
{
    if (m_bDevOpenOk != TRUE) {
        Log(ThisFile, 1, "LineModePrintText()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int nRet = 0;
    string strResult;
    strResult.append(lpPrintString, wStringSize);
    Log(ThisFile, 1, "LineModePrintText()->StartX:%d, DataSize:%d, PrintData:%s.", wStartX, wStringSize, strResult.c_str());

    if (wStringSize < 1)
        return TRUE;

    if (BK_SetPrintMode(PRINT_LINE_MODE) != TRUE) {
        Log(ThisFile, 1, "LineModePrintText()->BK_SetPrintMode(%d|LineMode) fail. ReturnCode:%d.", PRINT_LINE_MODE, nRet);
        return FALSE;
    }

    if (PrintFrontSetFormat() != TRUE)
    {
        Log(ThisFile, 1, "LineModePrintText()->PrintFrontSetFormat() fail. ");
        return FALSE;
    }

    if (strlen((char*)byFontType) > 0 && wFontSize > 0) {
        nRet = m_possdkIntf.PrintTrueType(m_hPrinter, (char *)strResult.c_str(), wStartX, -1,
                                          (char*)byFontType, wFontSize, wFontSize, 0);
        if (nRet != ERR_SUCCESS)
        {
            Log(ThisFile, 1, "LineModePrintText()->PrintTrueType(FontType=%s, FontSize=%d) fail. ReturnCode:%d.",
                byFontType, wFontSize, nRet);
            return FALSE;
        }
    } else {
        nRet = m_possdkIntf.PrintTextOut(m_hPrinter, (char *)strResult.c_str(), wStartX, -1);
        if ( nRet != ERR_SUCCESS)
        {
            Log(ThisFile, 1, "LineModePrintText()->PrintTextOut() fail. ReturnCode:%d.", nRet);
            return FALSE;
        }
    }

    nRet = m_possdkIntf.FeedLine(m_hPrinter);
    if (nRet != ERR_SUCCESS){
        Log(ThisFile, 1, "LineModePrintText()->FeedLine() fail. ReturnCode:%d.", nRet);
        return FALSE;
    }
    return TRUE;
}

/**
 @功能：	行模式打印图片
 @参数：	lpImageStr     : 图片路径
 @      lpImageStrSize : 图片路径size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
BOOL CBKC310_DevImpl::LineModePrintImage(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX)
{
    if (m_bDevOpenOk != TRUE){
        Log(ThisFile, 1, "LineModePrintImage()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    Log(ThisFile, 1, "LineModePrintImage()->StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());
    if (BK_SetPrintMode(PRINT_LINE_MODE) != TRUE)
    {
        Log(ThisFile, 1, "LineModePrintImage()->BK_SetPrintMode(%d|LineMode) fail. ReturnCode:%d.", PRINT_LINE_MODE, nRet);
        return FALSE;
    }

    nRet = m_possdkIntf.PrintBitmap(m_hPrinter, (char *)strResult.c_str(), wStartX, -1, 1);
    if ( nRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "LineModePrintImage()->PrintBitmap() fail. ReturnCode:%d.", nRet);
        return FALSE;
    }

    nRet = m_possdkIntf.FeedLine(m_hPrinter);
    if (nRet != ERR_SUCCESS){
        Log(ThisFile, 1, "LineModePrintImage()->FeedLine() fail. ReturnCode:%d.", nRet);
        return FALSE;
    }
    return TRUE;
}

/**
 @功能：	行模式打印图片(预下载RAM方式)
 @参数：	lpImageStr     : 图片路径
 @      lpImageStrSize : 图片路径size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
BOOL CBKC310_DevImpl::LineModePrintImageRAM(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX)
{
    if (m_bDevOpenOk != TRUE){
        Log(ThisFile, 1, "LineModePrintImageRAM()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    Log(ThisFile, 1, "LineModePrintImageRAM()->StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());
    if (BK_SetPrintMode(PRINT_LINE_MODE) != TRUE)
    {
        Log(ThisFile, 1, "LineModePrintImageRAM()->BK_SetPrintMode(%d|LineMode) fail. ReturnCode:%d.", PRINT_LINE_MODE, nRet);
        return FALSE;
    }

    nRet = m_possdkIntf.DownloadRAMBitmapByFile(m_hPrinter, (char *)strResult.c_str(), 6);
    if ( nRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "LineModePrintImageRAM()->DownloadRAMBitmapByFile() fail. ReturnCode:%d.", nRet);
        return FALSE;
    }

    nRet = m_possdkIntf.PrintRAMBitmap(m_hPrinter, 6, wStartX, -1, 0);
    if ( nRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "LineModePrintImageRAM()->PrintRAMBitmap() fail. ReturnCode:%d.", nRet);
        return FALSE;

    }

    nRet = m_possdkIntf.FeedLine(m_hPrinter);
    if (nRet != ERR_SUCCESS){
        Log(ThisFile, 1, "LineModePrintImageRAM()->FeedLine() fail. ReturnCode:%d.", nRet);
        return FALSE;
    }
    return TRUE;
}

/**
 @功能：	行模式打印图片(ByMode方式)
 @参数：	lpImageStr     : 图片路径
 @      lpImageStrSize : 图片路径size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
BOOL CBKC310_DevImpl::LineModePrintImageByMode(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX)
{
    if (m_bDevOpenOk != TRUE){
        Log(ThisFile, 1, "LineModePrintImageByMode()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int nRet = 0;
    string strResult;
    strResult.append(lpImageStr, lpImageStrSize);
    Log(ThisFile, 1, "LineModePrintImageByMode()->StartX:%d, DataSize:%d, PrintImage:%s.", wStartX, lpImageStrSize, strResult.c_str());
    if (BK_SetPrintMode(PRINT_LINE_MODE) != TRUE)
    {
        Log(ThisFile, 1, "LineModePrintImageByMode()->BK_SetPrintMode(%d|LineMode) fail. ReturnCode:%d.", PRINT_LINE_MODE, nRet);
        return FALSE;
    }

    nRet = m_possdkIntf.PrintBitmapByMode(m_hPrinter, (char *)strResult.c_str(), wStartX, -1, BITMAP_MODE_24DOUBLE_DENSITY);
    if ( nRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "LineModePrintImageByMode()->PrintBitmapByMode() fail. ReturnCode:%d.", nRet);
        return FALSE;
    }

    nRet = m_possdkIntf.FeedLine(m_hPrinter);
    if (nRet != ERR_SUCCESS){
        Log(ThisFile, 1, "LineModePrintImageByMode()->FeedLine() fail. ReturnCode:%d.", nRet);
        return FALSE;
    }
    return TRUE;
}

BOOL CBKC310_DevImpl::PrintFrontSetFormat()
{
    if (m_bDevOpenOk != TRUE){
        Log(ThisFile, 1, "PrintFrontSetFormat()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    if (wLineHeight > 0) {  // 设置打印行高
        if (BK_SetTextLineHight(wLineHeight) != TRUE) {
            Log(ThisFile, 1, "PrintFrontSetFormat()->BK_SetTextLineHight(%d) fail. ", wLineHeight);
            return FALSE;
        }
    }

    if (wCharSpace > 0) {   // 设置字符间距
        if (BK_SetTextCharacterSpace(wCharSpace) != TRUE) {
            Log(ThisFile, 1, "PrintFrontSetFormat()->BK_SetTextCharacterSpace(%d) fail. ", wCharSpace);
            return FALSE;
        }
    }

    return TRUE;
}

/*CTraceManager *CBKC310_DevImpl::GetTraceObj()
{
    return &//m_traceMgr;
}

LPSPErrorInfo CBKC310_DevImpl::GetErrorCode()
{
    return &m_errorInfo;
}*/

/////////////////////////////////////////////////////////////////////////
/**
 @功能：	设置打印模式
 @参数：	wPrintMode: 0,行模式(标准模式) / 1,页模式
 @返回：	TRUE/FALSE
 */
BOOL CBKC310_DevImpl::BK_SetPrintMode(WORD wPrintMode)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "BK_SetPrintMode()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int nRet = m_possdkIntf.PrintSetMode(m_hPrinter, wPrintMode);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "BK_SetPrintMode()->PrintSetMode(%d) fail. ReturnCode:%d.", wPrintMode, nRet);
        return FALSE;
    }

    return TRUE;
}

/**
 @功能：	设置打印行高
 @参数：	wLineHeight: 行高，单位参考IsSpot
 @           IsSpot: TRUE,wLineHeight单位为0.1mm; FALSE,wLineHeight单位为点
 @返回：	TRUE/FALSE
 */
BOOL CBKC310_DevImpl::BK_SetTextLineHight(WORD wLineHeight, BOOL IsSpot /* = FALSE */)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "BK_SetTextLineHight()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int nRet = m_possdkIntf.SetTextLineHight(m_hPrinter, (IsSpot == TRUE ? wLineHeight : MM2SPOT(wLineHeight)));
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "BK_SetTextLineHight()->SetTextLineHight(%d|%d) fail. ReturnCode:%d.",
            wLineHeight, MM2SPOT(wLineHeight), nRet);
        return FALSE;
    }

    return TRUE;
}

/**
 @功能：	设置字符间距
 @参数：	nCharSpace: 字符间距，单位参考IsSpot
 @          IsSpot: TRUE,nCharSpace单位为0.1mm; FALSE,nCharSpace单位为点
 @返回：	TRUE/FALSE
 */
BOOL CBKC310_DevImpl::BK_SetTextCharacterSpace(WORD nCharSpace, BOOL IsSpot /* = FALSE */)
{
    if (m_bDevOpenOk != TRUE)
    {
        Log(ThisFile, 1, "BK_SetTextCharacterSpace()-> m_bDevOpenOk == FALSE, Device Not Open, return fail.");
        return FALSE;
    }

    int nRet = m_possdkIntf.SetTextCharacterSpace(m_hPrinter, 0,
                                                     (IsSpot == TRUE ? nCharSpace : MM2SPOT(nCharSpace)),
                                                     0);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "BK_SetTextCharacterSpace()->SetTextCharacterSpace(0, %d|%d, 0) fail. ReturnCode:%d.",
            nCharSpace, MM2SPOT(nCharSpace), nRet);
        return FALSE;
    }

    nRet = m_possdkIntf.SetTextCharacterSpace(m_hPrinter, 0,
                                                     (IsSpot == TRUE ? nCharSpace : MM2SPOT(nCharSpace)),
                                                     1);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisFile, 1, "BK_SetTextCharacterSpace()->SetTextCharacterSpace(0, %d|%d, 1) fail. ReturnCode:%d.",
            nCharSpace, MM2SPOT(nCharSpace), nRet);
        return FALSE;
    }

    return TRUE;
}

/*BOOL CBKC310_DevImpl::BK_PrintInPapgeMode()
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

BOOL CBKC310_DevImpl::BK_SetCharRightInternal(BYTE bRightInternal)
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

BOOL CBKC310_DevImpl::BK_SetPresenterWaitTime(BYTE bWaitTime)
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

BOOL CBKC310_DevImpl::BK_SetPresenterMode(int nPresentMode)
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

BOOL CBKC310_DevImpl::BK_SetPaperType(BOOL bIsBlackMark)
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

BOOL CBKC310_DevImpl::BK_SetBlodMode(BOOL bIsBlod)
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

BOOL CBKC310_DevImpl::BK_SetPageModeArea(WORD wLineCount, int iMinLines, int iRowInterval)
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

BOOL CBKC310_DevImpl::BK_SetRowInterval(BYTE usRowInterval)
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

BOOL CBKC310_DevImpl::BK_SetInternalChar(BYTE usInterChar)
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

BOOL CBKC310_DevImpl::BK_SetCodePage(int iCodePage)
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

BOOL CBKC310_DevImpl::BK_SetAllowPrint()
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

 BOOL CBKC310_DevImpl::BK_SendDataToBuf(LPSTR lpPrintString, WORD wStringSize)
 {
    BOOL bRet = TRUE;

    bRet = m_lpIDevComm->SendData((unsigned char*)lpPrintString, wStringSize);
    if (!bRet)
    {
        return FALSE;
    }

    return bRet;
 }*/

/*void CBKC310_DevImpl::SetErrorCode(LPSPErrorInfo lpErrorInfo, char cErrorType[], char cErrorDetailCode[], char *pDescription)
{
    m_errorCode.SetErrorCode(lpErrorInfo, MODULE_SPR_SNBC, cErrorType, cErrorDetailCode, pDescription);
}

void CBKC310_DevImpl::ClearErrorCode(LPSPErrorInfo lpErrorInfo)
{
    if (lpErrorInfo != NULL){
        memset(lpErrorInfo->sDescription, 0, sizeof(lpErrorInfo->sDescription));
        memset(lpErrorInfo->errCode.sErrorCode, '0', sizeof(lpErrorInfo->errCode.sErrorCode));
    }
}*/

void CBKC310_DevImpl::SetPrintFormat(STPRINTFORMAT stPrintFormat)
{
    //wRowDistance = (stPrintFormat.uLPI > 0 ? stPrintFormat.uLPI : wRowDistance);  // 行间距
    if (strlen(stPrintFormat.szFontType) > 0 && stPrintFormat.uFontSize > 0) {
        memcpy(byFontType, stPrintFormat.szFontType, strlen(stPrintFormat.szFontType));
        wFontSize = stPrintFormat.uFontSize;
    }
    wLineHeight = (stPrintFormat.uLPI > 0 ? stPrintFormat.uLineHeight : wLineHeight);  // 行间距
    wCharSpace = (stPrintFormat.uWPI > 0 ? stPrintFormat.uWPI : wCharSpace);      // 字符间距
}

ULONG CBKC310_DevImpl::SplitString(LPCSTR lpszStr, CHAR cSplit, CHAR lpszStrArray[][MAX_PATH], ULONG ulArrayCnt)
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

