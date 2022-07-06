#include "DevRPR_SNBC.h"

// 版本号
BYTE  byVRTU_SBNC[17]  = "DevPTR01000000";

CBKC310_DevImpl  g_devBKC310Impl;

static const char *ThisFile = "DevRPR_SNBC.cpp";
//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

CDevPTR_SNBC::CDevPTR_SNBC(LPCSTR lpDevType)
{
     //SetLogFile(LOGFILE, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件// 30-00-00-00(FT#0052)
     //m_hPrinter = NULL;
     //m_bInitOk = FALSE;                         // 设备初始化结果为F

     // 错误码，初始值0000000
     //memset(byErrCode, 0x00, sizeof(byErrCode));
     //memcpy(byErrCode, szErrInit_Succ, 7);

     //m_ePaperStatus = PAPER_STATUS_UNKNOWN;     // 打印纸状态，初始未知// 30-00-00-00(FT#0052)
     //m_eTonerStatus = TONER_STATUS_UNSUPPORT;     // INK、TONER或色带状态状态，初始未知// 30-00-00-00(FT#0052)
     //m_eOutletStatus = OUTLET_STATUS_UNKNOWN;   // 出纸口状态，初始未知// 30-00-00-00(FT#0052)

    WORD wType = DEV_SNBC_BKC310;                       // 30-00-00-00(FT#0052)
    if (!strcmp(lpDevType, BTNH80_DEV_MODEL_NAME))      // 30-00-00-00(FT#0052)
    {                                                   // 30-00-00-00(FT#0052)
        wType = DEV_SNBC_BTNH80;                        // 30-00-00-00(FT#0052)
    }                                                   // 30-00-00-00(FT#0052)
    CDevPTR_SNBC(wType, lpDevType);                     // 30-00-00-00(FT#0052)
}

CDevPTR_SNBC::CDevPTR_SNBC(WORD wType)
{
     CDevPTR_SNBC(wType,
                  wType == DEV_SNBC_BKC310 ? BKC310_DEV_MODEL_NAME :
                    (wType == DEV_SNBC_BTNH80 ? BTNH80_DEV_MODEL_NAME : BKC310_DEV_MODEL_NAME));
}

CDevPTR_SNBC::CDevPTR_SNBC(WORD wType, LPCSTR lpDevType)
{
     //SetLogFile(LOGFILE, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件// 30-00-00-00(FT#0052)

     // 设置LOG                                                           // 30-00-00-00(FT#0052)
     if (wDevType == DEV_SNBC_BKC310)                                    // 30-00-00-00(FT#0052)
     {                                                                   // 30-00-00-00(FT#0052)
         SetLogFile(LOG_NAME_BKC310, ThisFile);                          // 30-00-00-00(FT#0052)
     } else                                                              // 30-00-00-00(FT#0052)
     if (wDevType == DEV_SNBC_BTNH80)                                    // 30-00-00-00(FT#0052)
     {                                                                   // 30-00-00-00(FT#0052)
         SetLogFile(LOG_NAME_BTNH80, ThisFile);                          // 30-00-00-00(FT#0052)
     }                                                                   // 30-00-00-00(FT#0052)

     //m_hPrinter = NULL;
     //m_bInitOk = FALSE;                         // 设备初始化结果为F

     // 错误码，初始值0000000
     //memset(byErrCode, 0x00, sizeof(byErrCode));
     //memcpy(byErrCode, szErrInit_Succ, 7);

     wDevType = wType;

     m_ePaperStatus = PAPER_STATUS_UNKNOWN;     // 打印纸状态，初始未知
     m_eTonerStatus = TONER_STATUS_UNSUPPORT;     // INK、TONER或色带状态状态，初始未知
     m_eOutletStatus = OUTLET_STATUS_UNKNOWN;   // 出纸口状态，初始未知
}

CDevPTR_SNBC::~CDevPTR_SNBC()
{

}

long CDevPTR_SNBC::Release()
{

}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : Open()                                                    **
** Describtion  : 打开设备　　　　                                             **
** Parameter    : 输入：pMode: 自定义OPEN参数字串                              **
**                输出：无                                                   **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
long CDevPTR_SNBC::Open(const char *pMode)
{
    if (g_devBKC310Impl.OpenDevice() != TRUE) {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

long CDevPTR_SNBC::OpenDev(const unsigned short usMode)
{
    wDevType = usMode;
    if (g_devBKC310Impl.OpenDevice(usMode) != TRUE) {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : Close()                                                   **
** Describtion  : 关闭设备　　　　                                             **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
void CDevPTR_SNBC::Close()
{
    g_devBKC310Impl.CloseDevice();
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : Init()                                                    **
** Describtion  : 设备复位(Reset)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
long CDevPTR_SNBC::Init()
{
    if(g_devBKC310Impl.Reset() != TRUE) {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : SetPrintFormat()                                          **
** Describtion  : 设置当前打印格式                                            **
** Parameter    : stPrintFormat：定义当前打印的格式具体内容                     **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
long CDevPTR_SNBC::SetPrintFormat(const STPRINTFORMAT &stPrintFormat)
{
    g_devBKC310Impl.SetPrintFormat(stPrintFormat);
    return ERR_PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : PrintData()                                              **
** Describtion  : 打印字串(无指定打印坐标)                                     **
**                打印格式使用上一次调用SetPrintFormat设置的格式，               **
**                如果没有设置则使用默认参数                                    **
** Parameter    : 输入：pStr：要打印的字串                                     **
**                     ulDataLen：数据长度，如果为-1，pStr以\0结束             **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
long CDevPTR_SNBC::PrintData(const char *pStr, unsigned long ulDataLen)
{
    if (g_devBKC310Impl.LineModePrintText((LPSTR)pStr, ulDataLen, 0) != TRUE) {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : PrintImage()                                              **
** Describtion  : 图片打印(无指定打印坐标)                                     **
** Parameter    : 输入：szImagePath：图片保存路径                              **
**                     ulWidth：打印宽度                                     **
**                     ulHeight：打印高度                                    **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
long CDevPTR_SNBC::PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    if (wDevType == DEV_SNBC_BKC310) {
        if (g_devBKC310Impl.LineModePrintImageRAM((LPSTR)szImagePath, strlen(szImagePath), 0) != TRUE) {
            return ERR_PTR_OTHER;
        }
    } else {
        if (g_devBKC310Impl.LineModePrintImage((LPSTR)szImagePath, strlen(szImagePath), 0) != TRUE) {
            return ERR_PTR_OTHER;
        }
    }

    return ERR_PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : PrintImageOrg()                                           **
** Describtion  : 图片打印(指定打印坐标)                                       **
** Parameter    : 输入：szImagePath：图片保存路径                              **
**                     ulOrgX: 打印图片的列坐标                               **
**                     ulOrgY: 打印图片的行坐标                              **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
long CDevPTR_SNBC::PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    if (wDevType == DEV_SNBC_BKC310) {
        if (g_devBKC310Impl.LineModePrintImageRAM((LPSTR)szImagePath, strlen(szImagePath), ulOrgX * 15 * 8 / 10) != TRUE) {
            return ERR_PTR_OTHER;
        }
    } else {
        if (g_devBKC310Impl.LineModePrintImageByMode((LPSTR)szImagePath, strlen(szImagePath), ulOrgX * 15 * 8 / 10) != TRUE) {
            return ERR_PTR_OTHER;
        }
    }

    return ERR_PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : CutPaper()                                                **
** Describtion  : 切纸　　　　                                             　　**
** Parameter    : 输入：bDetectBlackStripe：是否检测黑标                    　　**
**                     ulFeedSize，切纸前走纸的长度，单位0.1毫米                 **
**                输出：无                                                   **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
long CDevPTR_SNBC::CutPaper(bool bDetectBlackStripe, unsigned long ulFeedSize)
{
    if(g_devBKC310Impl.ControlMedia(bDetectBlackStripe, ulFeedSize) != TRUE) {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : QueryStatus()                                             **
** Describtion  : 查询设备状态                                                **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
long CDevPTR_SNBC::QueryStatus()
{
    PrintStatus sPTRStatus;
    /*if (GetDevStatus(&sPTRStatus) != TRUE) {
        return ERR_PTR_OTHER;
    }*/

    GetDevStatus(&sPTRStatus);

    if (sPTRStatus.wDevState == DEV_OFFLINE) {
        return ERR_PTR_NOT_OPEN;
    } else
    if (sPTRStatus.wDevState == DEV_HWERR) {
        return ERR_PTR_HWERR;
    } else
    if (sPTRStatus.wDevState == DEV_JAM) {
        //return ERR_PTR_JAMMED;            // 30-00-00-00(FT#0052)
        return ERR_PTR_HWERR;               // 30-00-00-00(FT#0052)
    } else
    if (sPTRStatus.wDevState == DEV_BUSY) {
        return ERR_PTR_DEVBUSY;
    } else
    if (sPTRStatus.wDevState == DEV_UNKNOWN) {
        return ERR_PTR_DEVUNKNOWN;
    }

    return ERR_PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : GetStatus()                                               **
** Describtion  : 查询设备状态                                               　**
** Parameter    : 输入无                                                    　**
** 　　　　　　　　　输出　pPaperStatus，返回纸状态                        　　　　**
**　　　　　             pTonerStatus，返回TONER状态                         　**
**             　　　　　pOutletStatus，返回出纸口状态                       　　**
** Return       : 无                                                         **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
void CDevPTR_SNBC::GetStatus(PaperStatus &pPaperStatus, TonerStatus &pTonerStatus, OutletStatus &pOutletStatus)
{
    PrintStatus sPTRStatus;

    pPaperStatus = PAPER_STATUS_UNKNOWN;     // 打印纸状态，初始未知
    pTonerStatus = TONER_STATUS_UNSUPPORT;     // INK、TONER或色带状态状态，初始未知
    pOutletStatus = OUTLET_STATUS_UNKNOWN;   // 出纸口状态，初始未知

    if (GetDevStatus(&sPTRStatus) != TRUE) {
        return;
    }

    // 纸状态
    switch(sPTRStatus.wPaper) {
        case PRINT_PAPER_FULL :     // 0
            pPaperStatus = PAPER_STATUS_NORMAL;      // 1:正常
            break;
        case PRINT_PAPER_LOW :      // 1
            pPaperStatus = PAPER_STATUS_LOW;         // 2:少纸
            break;
        case PRINT_PAPER_OUT :      // 2
            pPaperStatus = PAPER_STATUS_EMPTY;       // 5:空
            break;
        case PRINT_PAPER_NOTSUPP :  // 3
            pPaperStatus = PAPER_STATUS_UNSUPPORT;   // 6:不支持
            break;
        case PRINT_PAPER_UNKNOWN :  // 4
            pPaperStatus = PAPER_STATUS_UNKNOWN;     // 1:未知
            break;
        case PRINT_PAPER_JAMMED :   // 5
            pPaperStatus = PAPER_STATUS_JAMMED;      // 3:堵塞
            break;
    }

    // Toner状态不设置

    // 出纸口状态
    switch(sPTRStatus.bPaperInPresenter) {
        case PRINT_PRESENTER_NOT_PAPER :    // 0
            pOutletStatus = OUTLET_STATUS_NOMEDIA;   // 1:无纸
            break;
        case PRINT_PRESENTER_HAVE_PAPER :   // 1
            pOutletStatus = OUTLET_STATUS_MEDIA;     // 2:有纸
            break;
        default:
            pOutletStatus = OUTLET_STATUS_UNKNOWN;
    }
}

long CDevPTR_SNBC::SetPrintMode(const STPRINTMODE &stPrintMode)
{
    return ERR_PTR_UNSUP_CMD;
}

long CDevPTR_SNBC::PrintPageTextOut(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY)
{
    return ERR_PTR_UNSUP_CMD;
}

long CDevPTR_SNBC::PrintPageImageOut(const char *szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    return ERR_PTR_UNSUP_CMD;
}

long CDevPTR_SNBC::PrintPageData()
{
    return ERR_PTR_UNSUP_CMD;
}

long CDevPTR_SNBC::PrintLineData(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX)
{
    return ERR_PTR_UNSUP_CMD;
}

char *CDevPTR_SNBC::GetErrCode()
{

}

char *CDevPTR_SNBC::GetVersion()
{
    return (char*)byVRTU_SBNC;
}

bool CDevPTR_SNBC::GetFWVersion(char *szFWVer, unsigned long *ulSize)
{
    return g_devBKC310Impl.GetFwVersion(szFWVer, ulSize);
}

unsigned long CDevPTR_SNBC::GetDevErrCode()
{

}

long CDevPTR_SNBC::ChkPaperMarkHeader(unsigned int uiMakePos)
{

}

BOOL CDevPTR_SNBC::GetDevStatus(LPPrintStatus lpPTRStatus)
{
    if(lpPTRStatus == NULL)
    {
        return FALSE;
    }

    BOOL bRet = TRUE;
    PrintStatus stPrintStatusOut;
    memset(&stPrintStatusOut, 0, sizeof(stPrintStatusOut));
    stPrintStatusOut.wDevState = DEV_NORMAL;
    stPrintStatusOut.wPaper = PRINT_PAPER_UNKNOWN; //未知
    stPrintStatusOut.wMedia = PRINT_MEDIA_UNKNOWN;
    stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_NOT_PAPER;

    char cRealtimeDevStatus = 0;
    char cNonRealtimeDevStatus = 0;
    char cAutoStatus[6] = {0};
    //char cAutoStatus_BT[4] = {0};                     // 30-00-00-00(FT#0052)
    char cAutoStatus_TKIOSK[8] = { 0x00 };              // 30-00-00-00(FT#0052)


    if (wDevType == DEV_SNBC_BKC310) {
        ULONG ulStaBufSz = sizeof(cAutoStatus);
        if((bRet = g_devBKC310Impl.GetAutoStatus(cAutoStatus, &ulStaBufSz)) != FALSE){
            //g_devBKC310Impl.GetTraceObj()->TraceInBuffer(TRM_INT, TRM_LV_COMMN, (LPBYTE)cAutoStatus, ulStaBufSz, "AutoStatus:");
            //纸状态
            if((cAutoStatus[0] & DATA_BIT2) ||
               ((cAutoStatus[4] & DATA_BIT1) && (cAutoStatus[4] & DATA_BIT2))) {       //纸张用尽
                stPrintStatusOut.wPaper = PRINT_PAPER_OUT;
            } else if(cAutoStatus[0] & DATA_BIT0)       //纸少
            {
                stPrintStatusOut.wPaper = PRINT_PAPER_LOW;
            } else {
                stPrintStatusOut.wPaper = PRINT_PAPER_FULL;     //Not exactly full
            }

            if(stPrintStatusOut.wPaper == PRINT_PAPER_OUT){
                stPrintStatusOut.wMedia = PRINT_MEDIA_NOTPRESENT;
            } else {
                stPrintStatusOut.wMedia = PRINT_MEDIA_PRESENT;
            }

            //判断出纸口是否有纸
            if((cAutoStatus[1] & DATA_BIT0)){
                stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_HAVE_PAPER;
                stPrintStatusOut.wMedia = PRINT_MEDIA_ENTERING;         //纸在出/入口处
            } else {
                stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_NOT_PAPER;
            }

            if((cAutoStatus[0] & DATA_BIT1) ||                    //打印头抬起
               (cAutoStatus[0] & DATA_BIT3) ||                    //打印头过热
               (cAutoStatus[0] & DATA_BIT4) ||                    //切刀错
               ((cAutoStatus[1] & DATA_BIT4) && (stPrintStatusOut.wPaper != PRINT_PAPER_OUT))){                     //自动上纸错误
                stPrintStatusOut.wDevState = DEV_HWERR;
            }

            //卡纸
            if((cAutoStatus[1] & DATA_BIT1) ||                    //打印处或上纸通道塞纸
               (cAutoStatus[1] & DATA_BIT2) ||                    //PRST处塞纸
               (cAutoStatus[0] & DATA_BIT5)){                     //卡纸
                stPrintStatusOut.wDevState = DEV_JAM;
                stPrintStatusOut.wMedia = PRINT_MEDIA_JAMMED;
                stPrintStatusOut.wPaper = PRINT_PAPER_JAMMED;;
            }
        } else {
            bRet = FALSE;
        }
    } else {
        //ULONG ulStaBufSz = sizeof(cAutoStatus_BT);                // 30-00-00-00(FT#0052)
        ULONG usStaBufSz = sizeof(cAutoStatus_TKIOSK);              // 30-00-00-00(FT#0052)
        ULONG ulStaBufSzReal = sizeof(cRealtimeDevStatus);
        /*if((bRet = g_devBKC310Impl.GetAutoStatus(cAutoStatus_BT, &ulStaBufSz)) != FALSE){
            //g_devBKC310Impl.GetTraceObj()->TraceInBuffer(TRM_INT, TRM_LV_COMMN, (LPBYTE)cAutoStatus, ulStaBufSz, "AutoStatus:");
            //纸状态
            if((cAutoStatus_BT[2] & DATA_BIT2) && (cAutoStatus_BT[2] & DATA_BIT3)) {       //纸张用尽
                stPrintStatusOut.wPaper = PRINT_PAPER_OUT;
            } else if((cAutoStatus_BT[2] & DATA_BIT0) && (cAutoStatus_BT[2] & DATA_BIT1))       //纸少
            {
                stPrintStatusOut.wPaper = PRINT_PAPER_LOW;
            } else {
                stPrintStatusOut.wPaper = PRINT_PAPER_FULL;     //Not exactly full
            }

            if(stPrintStatusOut.wPaper == PRINT_PAPER_OUT){
                stPrintStatusOut.wMedia = PRINT_MEDIA_NOTPRESENT;
            } else {
                stPrintStatusOut.wMedia = PRINT_MEDIA_PRESENT;
            }

            //判断出纸口是否有纸
            if((cAutoScAutoStatus_BTtatus[1] & DATA_BIT0)){
                stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_HAVE_PAPER;
                stPrintStatusOut.wMedia = PRINT_MEDIA_ENTERING;         //纸在出/入口处
            } else {
                stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_NOT_PAPER;
            }

            if((cAutoStatus_BT[0] & DATA_BIT5) ||                    //打印头抬起
               //(cAutoStatus_BT[0] & DATA_BIT3) ||                    //打印头过热
               (cAutoStatus_BT[1] & DATA_BIT3) ||                    //切刀错
               ((cAutoStatus_BT[1] & DATA_BIT4) && (stPrintStatusOut.wPaper != PRINT_PAPER_OUT))){                     //自动上纸错误
                stPrintStatusOut.wDevState = DEV_HWERR;
            }

            //卡纸
            if((cAutoStatus_BT[1] & DATA_BIT2) ||                 //防卡纸模块检测有卡纸
               (cAutoStatus_BT[1] & DATA_BIT7)){                  //防拉纸模块检测有拉纸
                stPrintStatusOut.wDevState = DEV_JAM;
                stPrintStatusOut.wMedia = PRINT_MEDIA_JAMMED;
                stPrintStatusOut.wPaper = PRINT_PAPER_JAMMED;;
            }
        } else*/
        if ((bRet = g_devBKC310Impl.GetRtStatus(&cRealtimeDevStatus, &ulStaBufSzReal)) != FALSE) {
            //纸状态
            if(cRealtimeDevStatus & DATA_BIT1) {       //纸张用尽
                stPrintStatusOut.wPaper = PRINT_PAPER_OUT;
            } else
            if(cRealtimeDevStatus & DATA_BIT4)       //纸少
            {
                stPrintStatusOut.wPaper = PRINT_PAPER_LOW;
            } else {
                stPrintStatusOut.wPaper = PRINT_PAPER_FULL;     //Not exactly full
            }

            if(stPrintStatusOut.wPaper == PRINT_PAPER_OUT){
                stPrintStatusOut.wMedia = PRINT_MEDIA_NOTPRESENT;
            } else {
                stPrintStatusOut.wMedia = PRINT_MEDIA_PRESENT;
            }
/*
            //判断出纸口是否有纸
            if((cAutoScAutoStatus_BTtatus[1] & DATA_BIT0)){
                stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_HAVE_PAPER;
                stPrintStatusOut.wMedia = PRINT_MEDIA_ENTERING;         //纸在出/入口处
            } else {
                stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_NOT_PAPER;
            }*/

            if((cRealtimeDevStatus & DATA_BIT0) ||   //打印头抬起
               (cRealtimeDevStatus & DATA_BIT3) ||       //打印头过热
               (cRealtimeDevStatus & DATA_BIT2) ||       //切刀错
               ((cRealtimeDevStatus & DATA_BIT7) && (stPrintStatusOut.wPaper != PRINT_PAPER_OUT))){                     //自动上纸错误
                stPrintStatusOut.wDevState = DEV_HWERR;
            }

            //卡纸
            if((cRealtimeDevStatus & DATA_BIT5) ||                    //打印处或上纸通道塞纸
               (cRealtimeDevStatus & DATA_BIT6)){                    //PRST处塞纸
                stPrintStatusOut.wDevState = DEV_JAM;
                stPrintStatusOut.wMedia = PRINT_MEDIA_JAMMED;
                stPrintStatusOut.wPaper = PRINT_PAPER_JAMMED;;
            }
        }
        else {
            bRet = FALSE;
        }
        // 自动获取状态,只判断卡纸                                                                     // 30-00-00-00(FT#0052)
        BOOL bRet_TKISOK = FALSE;                                                                   // 30-00-00-00(FT#0052)
        if((bRet_TKISOK = g_devBKC310Impl.GetAutoStatus(cAutoStatus_TKIOSK, &usStaBufSz)) != FALSE) // 30-00-00-00(FT#0052)
        {                                                                                           // 30-00-00-00(FT#0052)
            if((cAutoStatus_TKIOSK[1] & DATA_BIT2) ||                 //防卡纸模块检测有卡纸           // 30-00-00-00(FT#0052)
               (cAutoStatus_TKIOSK[1] & DATA_BIT7))                   //防拉纸模块检测有拉纸           // 30-00-00-00(FT#0052)
            {                                                                                       // 30-00-00-00(FT#0052)
                stPrintStatusOut.wDevState = DEV_JAM;                                               // 30-00-00-00(FT#0052)
                stPrintStatusOut.wMedia = PRINT_MEDIA_JAMMED;                                       // 30-00-00-00(FT#0052)
                stPrintStatusOut.wPaper = PRINT_PAPER_JAMMED;                                       // 30-00-00-00(FT#0052)
                bRet = TRUE;                                                                        // 30-00-00-00(FT#0052)
            }                                                                                       // 30-00-00-00(FT#0052)
        }                                                                                           // 30-00-00-00(FT#0052)
    }

    if(bRet != TRUE){
        stPrintStatusOut.wDevState = DEV_OFFLINE;
    }
    //LPSPErrorInfo lpSPErrorInfo = NULL;
    //lpSPErrorInfo = g_devBKC310Impl.GetErrorCode();
    //memcpy(stPrintStatusOut.sErrorCode, lpSPErrorInfo->errCode.sErrorCode, sizeof(lpSPErrorInfo->errCode.sErrorCode));
    //strcpy(stPrintStatusOut.sDescription, lpSPErrorInfo->sDescription);

    memcpy(lpPTRStatus, &stPrintStatusOut, sizeof(stPrintStatusOut));
    return bRet;
}

void CDevPTR_SNBC::SetDevErrCode(const BYTE *byErrCd)
{
    return;
}




