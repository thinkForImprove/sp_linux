#include "DevRPR_SNBC.h"

static const char *ThisFile = "DevRPR_SNBC.cpp";

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

CDevPTR_SNBC::CDevPTR_SNBC(LPCSTR lpDevType)
{
     //SetLogFile(LOGFILE, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

     if (MCMP_IS0(lpDevType, DEV_SNBC_BKC310_STR))
     {
         wDevType = DEV_SNBC_BKC310;
         SetLogFile(LOG_NAME_BKC310, ThisFile);
         g_devBKC310Impl.SetLogFile(LOG_NAME_BKC310, ThisFile);
     } else
     if (MCMP_IS0(lpDevType, DEV_SNBC_BTNH80_STR))
     {
         wDevType = DEV_SNBC_BTNH80;
         SetLogFile(LOG_NAME_BTNH80, ThisFile);
         g_devBKC310Impl.SetLogFile(LOG_NAME_BTNH80, ThisFile);
     } else
     {
         SetLogFile(LOGFILE, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件
     }

     m_wFontType = 0;                           // 字体类型
     m_wDouWidth = 1;                           // 倍宽(1~6)
     m_wDouHeight = 1;                          // 倍高(1~6)
     m_wFontStyle = 0;                          // 字体风格(0~3)
}

CDevPTR_SNBC::~CDevPTR_SNBC()
{

}

void CDevPTR_SNBC::Release()
{
    return;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : Open()                                                    **
** Describtion  : 打开设备　　　　                                             **
** Parameter    : 输入: pMode: 自定义OPEN参数字串                              **
**                输出: 无                                                   **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::Open(const char *pMode)
{
    if (MCMP_IS0(pMode, DEV_SNBC_BKC310_STR))
    {
        wDevType = DEV_SNBC_BKC310;
        SetLogFile(LOG_NAME_BKC310, ThisFile);
    } else
    if (MCMP_IS0(pMode, DEV_SNBC_BTNH80_STR))
    {
        wDevType = DEV_SNBC_BTNH80;
        SetLogFile(LOG_NAME_BTNH80, ThisFile);
    } else
    {
        return ERR_PTR_PARAM_ERR;
    }

    INT nRet = PTR_SUCCESS;
    if ((nRet = g_devBKC310Impl.DeviceOpen(wDevType)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
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
int CDevPTR_SNBC::Close()
{
    g_devBKC310Impl.DeviceClose();
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : Init()                                                    **
** Describtion  : 设备初始化                                                 **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::Init()
{
    return PTR_SUCCESS;
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
int CDevPTR_SNBC::Reset()
{
    INT nRet = PTR_SUCCESS;
    if ((nRet = g_devBKC310Impl.Reset()) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : Init()                                                    **
** Describtion  : 设备复位(有参数)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    INT nRet = PTR_SUCCESS;
    if ((nRet = g_devBKC310Impl.Reset()) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : SetPrintFormat()                                          **
** Describtion  : 设置当前打印格式                                            **
** Parameter    : stPrintFormat: 定义当前打印的格式具体内容                     **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::GetStatus(DEVPTRSTATUS &stStatus)
{
    //BOOL bRet = TRUE;
    INT nRet = IMP_SUCCESS;
    char cRealtimeDevStatus = 0;
    char cNonRealtimeDevStatus = 0;
    char cAutoStatus[6] = {0x00};
    char cAutoStatus_KIOSK[8] = { 0x00};

    // 设备状态初始化
    stStatus.wDevice = DEV_STAT_ONLINE;
    stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
    for (INT i = 0; i < 16; i ++)
        stStatus.wPaper[i] = PAPER_STAT_NOTSUPP;
    stStatus.wInk = INK_STAT_NOTSUPP;
    stStatus.wToner = TONER_STAT_FULL;

    // 获取状态
    if (wDevType == DEV_SNBC_BKC310)
    {
        ULONG ulStaBufSz = sizeof(cAutoStatus);
        if((nRet = g_devBKC310Impl.GetAutoStatus(cAutoStatus, &ulStaBufSz)) == IMP_SUCCESS)
        {
            //g_devBKC310Impl.GetTraceObj()->TraceInBuffer(TRM_INT, TRM_LV_COMMN, (LPBYTE)cAutoStatus, ulStaBufSz, "AutoStatus:");
            //纸状态
            if((cAutoStatus[0] & DATA_BIT2) ||
               ((cAutoStatus[4] & DATA_BIT1) && (cAutoStatus[4] & DATA_BIT2))) //纸张用尽
            {
                stStatus.wPaper[0] = PAPER_STAT_OUT;
            } else
            if(cAutoStatus[0] & DATA_BIT0)       //纸少
            {
                stStatus.wPaper[0] = PAPER_STAT_LOW;
            } else
            {
                stStatus.wPaper[0] = PAPER_STAT_FULL;
            }

            // 根据纸状态设置Media状态
            if(stStatus.wPaper[0] == PAPER_STAT_OUT)
            {
                stStatus.wMedia = MEDIA_STAT_NOTPRESENT;;
            } else
            {
                stStatus.wMedia = MEDIA_STAT_PRESENT;
            }

            // 判断出纸口是否有纸
            if((cAutoStatus[1] & DATA_BIT0)){
                //stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_HAVE_PAPER;
                stStatus.wMedia = MEDIA_STAT_ENTERING;         //纸在出/入口处
            } else {
                //stPrintStatusOut.bPaperInPresenter = PRINT_PRESENTER_NOT_PAPER;
            }

            // 设备Error
            if((cAutoStatus[0] & DATA_BIT1) ||                    //打印头抬起
               (cAutoStatus[0] & DATA_BIT3) ||                    //打印头过热
               (cAutoStatus[0] & DATA_BIT4) ||                    //切刀错
               ((cAutoStatus[1] & DATA_BIT4) && (stStatus.wPaper[0] != PAPER_STAT_OUT))) //自动上纸错误
            {
                stStatus.wDevice = DEV_STAT_HWERROR;
            }

            // 卡纸
            if((cAutoStatus[1] & DATA_BIT1) ||                    //打印处或上纸通道塞纸
               (cAutoStatus[1] & DATA_BIT2) ||                    //PRST处塞纸
               (cAutoStatus[0] & DATA_BIT5)){                     //卡纸
                stStatus.wDevice = DEV_STAT_HWERROR;
                stStatus.wMedia = MEDIA_STAT_JAMMED;
                stStatus.wPaper[0] = PAPER_STAT_JAMMED;
            }
        }
    } else
    {
        ULONG ulStaBufSz = sizeof(cAutoStatus_KIOSK);
        ULONG ulStaBufSzReal = sizeof(cRealtimeDevStatus);

        // 实时状态获取
        if ((nRet = g_devBKC310Impl.GetRtStatus(&cRealtimeDevStatus, &ulStaBufSzReal)) == IMP_SUCCESS)
        {
            // 纸状态
            if(cRealtimeDevStatus & DATA_BIT1) //纸张用尽
            {
                stStatus.wPaper[0] = PAPER_STAT_OUT;
            } else
            if(cRealtimeDevStatus & DATA_BIT4) //纸少
            {
                stStatus.wPaper[0] = PAPER_STAT_LOW;
            } else
            {
                stStatus.wPaper[0] = PAPER_STAT_FULL;     //Not exactly full
            }

            if(stStatus.wPaper[0] == PAPER_STAT_OUT)
            {
                stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
            } else
            {
                stStatus.wMedia = MEDIA_STAT_PRESENT;
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
               (cRealtimeDevStatus & DATA_BIT3) ||   //打印头过热
               (cRealtimeDevStatus & DATA_BIT2) ||   //切刀错
               ((cRealtimeDevStatus & DATA_BIT7) && (stStatus.wPaper[0] != PAPER_STAT_OUT)))
            {                     //自动上纸错误
                stStatus.wDevice = DEV_STAT_HWERROR;
            }

            //卡纸
            if((cRealtimeDevStatus & DATA_BIT5) ||                    //打印处或上纸通道塞纸
               (cRealtimeDevStatus & DATA_BIT6)){                    //PRST处塞纸
                stStatus.wDevice = DEV_STAT_HWERROR;
                stStatus.wMedia = MEDIA_STAT_JAMMED;
                stStatus.wPaper[0] = PAPER_STAT_JAMMED;;
            }
        }
        // 自动状态获取,只检查卡纸
        if((nRet = g_devBKC310Impl.GetAutoStatus(cAutoStatus_KIOSK, &ulStaBufSz)) == IMP_SUCCESS)
        {
            //卡纸
            if((cAutoStatus_KIOSK[1] & DATA_BIT2) ||                 //防卡纸模块检测有卡纸
               (cAutoStatus_KIOSK[1] & DATA_BIT7)){                  //防拉纸模块检测有拉纸
                stStatus.wDevice = DEV_STAT_HWERROR;
                stStatus.wMedia = MEDIA_STAT_JAMMED;
                stStatus.wPaper[0] = PAPER_STAT_JAMMED;
            }
        }
    }

    //if(bRet != TRUE)
    if (nRet != IMP_SUCCESS)
    {
        stStatus.wDevice = DEV_STAT_OFFLINE;
    }

    if (stStatus.wDevice == DEV_STAT_OFFLINE ||
        stStatus.wDevice == DEV_STAT_POWEROFF)
    {
        return ERR_PTR_NOT_OPEN;
    } else
    if (stStatus.wDevice == DEV_STAT_HWERROR ||
        stStatus.wDevice == DEV_STAT_USERERROR)
    {
        return ERR_PTR_HWERR;
    }else
    if (stStatus.wDevice == DEV_STAT_BUSY) {
        return ERR_PTR_DEVBUSY;
    } else
    if (stStatus.wDevice == DEV_STAT_NODEVICE)
    {
        return ERR_PTR_DEVUNKNOWN;
    }

    return PTR_SUCCESS;
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
** Parameter    : 输入: pStr: 要打印的字串                                     **
**                     ulDataLen: 数据长度，如果为-1，pStr以\0结束             **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::PrintData(const char *pStr, unsigned long ulDataLen)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = PTR_SUCCESS;
    Log(ThisModule, __LINE__, "打印字串: X=%d|L=%d|D=%s|", 0, ulDataLen, pStr);

    if ((nRet = g_devBKC310Impl.LineModePrintText((LPSTR)pStr, ulDataLen, 0)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : PrintDataOrg()                                            **
** Describtion  : 打印字串(指定打印坐标)                                      **
**                打印格式使用上一次调用SetPrintFormat设置的格式，               **
**                如果没有设置则使用默认参数                                    **
** Parameter    : 输入: pStr: 要打印的字串                                     **
**                     ulDataLen: 数据长度，如果为-1，pStr以\0结束             **
**                     ulOrgX: 打印的列坐标                                   **
**                     ulOrgY: 打印的行坐标                                  **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::PrintDataOrg(const char *pStr, unsigned long ulDataLen,
                               unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    ULONG ulX = ulOrgX * 8 / 10;    // 入参单位:0.1MM,转换为点

    INT nRet = PTR_SUCCESS;
    //Log(ThisModule, __LINE__, "打印字串: X=%d(%d)|Y=%d|L=%d|D=%s|", ulOrgX, ulX, ulOrgY, ulDataLen, pStr);

    /*if ((nRet = g_devBKC310Impl.LineModePrintText((LPSTR)pStr, ulDataLen, ulX)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }*/
    // 下发打印文本
    if (m_stPrtFontPar.wFontMode == PRT_FONT_MODE_DEF)
    {
        Log(ThisModule, __LINE__,
            "打印字串(缺省点阵方式): X=%d(%d点)|FontType=%d|FontStyle=%d|L=%d|D=%s",
            ulX, ulX * 8 / 10, m_wFontType, m_wFontStyle, ulDataLen, pStr);

        if ((nRet = g_devBKC310Impl.LineModePrintTextUniversal((LPSTR)pStr, ulDataLen,
                                                               ulX * 8 / 10, m_wDouWidth, m_wDouHeight,
                                                               m_wFontType, m_wFontStyle)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印文本(缺省点阵方式): LineModePrintTextUniversal(%s, %d, %d, %d, %d, %d, %d) fail, ErrCode: %d, Return: %s.",
                pStr, ulDataLen, ulX * 8 / 10, m_wDouWidth, m_wDouHeight, m_wFontType, m_wFontStyle,
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        Log(ThisModule, __LINE__,
            "打印字串(TrueType方式): X=%d(%d点)|FontName=%s|H*W=%d*%d|L=%d|D=%s",
            ulX, ulX * 8 / 10, m_stPrtFontPar.szFontName,
            m_stPrtFontPar.dwMarkPar[1], m_stPrtFontPar.dwMarkPar[1], ulDataLen, pStr);

        if ((nRet = g_devBKC310Impl.LineModePrintTrueType(
                                (LPSTR)pStr, ulDataLen, ulX * 8 / 10,  m_stPrtFontPar.szFontName,
                                m_stPrtFontPar.dwMarkPar[1] * 8 / 10, m_stPrtFontPar.dwMarkPar[1] * 8 / 10,
                                m_wFontStyle > 0 ? TRUE : FALSE)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印文本(TrueType方式): LineModePrintTrueType(%s, %d, %d, %s, %d, %d, %d, %d) fail, "
                "ErrCode: %d, Return: %s.",
                (LPSTR)pStr, ulDataLen, ulX * 8 / 10, m_stPrtFontPar.szFontName,
                m_stPrtFontPar.dwMarkPar[1] * 8 / 10, m_stPrtFontPar.dwMarkPar[1] * 8 / 10,
                m_wFontStyle > 0 ? TRUE : FALSE, TRUE,
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : PrintImage()                                              **
** Describtion  : 图片打印(无指定打印坐标)                                     **
** Parameter    : 输入: szImagePath: 图片保存路径                              **
**                     ulWidth: 打印宽度                                     **
**                     ulHeight: 打印高度                                    **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = PTR_SUCCESS;
    Log(ThisModule, __LINE__, "打印图片: X=%d|W=%d|H=%d|D=%s|", 0, ulWidth, ulHeight, szImagePath);

    if (wDevType == DEV_SNBC_BKC310)
    {
        INT nRet = PTR_SUCCESS;
        if ((nRet = g_devBKC310Impl.LineModePrintImageRAM((LPSTR)szImagePath, strlen(szImagePath), 0)) != IMP_SUCCESS)
        {
            return ConvertErrorCode(nRet);
        }
    } else {
        INT nRet = PTR_SUCCESS;
        if ((nRet = g_devBKC310Impl.LineModePrintImage((LPSTR)szImagePath, strlen(szImagePath), 0)) != IMP_SUCCESS)
        {
            return ConvertErrorCode(nRet);
        }
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : PrintImageOrg()                                           **
** Describtion  : 图片打印(指定打印坐标)                                       **
** Parameter    : 输入: szImagePath: 图片保存路径                              **
**                     ulOrgX: 打印图片的列坐标                               **
**                     ulOrgY: 打印图片的行坐标                              **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = PTR_SUCCESS;
    Log(ThisModule, __LINE__, "打印图片: X=%d(%d)|Y=%d(%d)|D=%s|",
        ulOrgX, ulOrgX * 15 * 8 / 10, ulOrgX, ulOrgY * 15 * 8 / 10, szImagePath);

    if (wDevType == DEV_SNBC_BKC310)
    {
        INT nRet = PTR_SUCCESS;
        if ((nRet = g_devBKC310Impl.LineModePrintImageRAM((LPSTR)szImagePath, strlen(szImagePath),
                                                          ulOrgX * 15 * 8 / 10)) != IMP_SUCCESS)
        {
            return ConvertErrorCode(nRet);
        }
    } else
    {
        INT nRet = PTR_SUCCESS;
        if ((nRet = g_devBKC310Impl.LineModePrintImageByMode((LPSTR)szImagePath, strlen(szImagePath), ulOrgX * 15 * 8 / 10)) != IMP_SUCCESS)
        {
            return ConvertErrorCode(nRet);
        }
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                           **
** ClassName    : CDevPTR_SNBC                                               **
** Symbol       : CDevPTR_SNBC::                                             **
** Function     : PrintForm()                                               **
** Describtion  : PrintForm打印                                              **
** Parameter    : 输入: stPrintIn:                                           **
**                输出: stPrintOut:                                          **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::PrintForm(DEVPRINTFORMIN stPrintIn, DEVPRINTFORMOUT &stPrintOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 注: stPrintIn.ulX, stPrintIn.ulY 单位: 0.1毫米
    //

    if (stPrintIn.wInMode == PRINTFORM_TEXT)
    {
        // 下发打印文本
        if (m_stPrtFontPar.wFontMode == PRT_FONT_MODE_DEF)
        {
            Log(ThisModule, __LINE__,
                "打印字串(缺省点阵方式): X=%d(%d点)|Y=%d(%d点)|DouWidth=%d|DouHeight=%d|"
                "FontType=%d|FontStyle=%d|L=%d|D=%s",
                stPrintIn.ulX, stPrintIn.ulX * 8 / 10, stPrintIn.ulY, stPrintIn.ulY * 8 / 10,
                m_wDouWidth, m_wDouHeight, m_wFontType, m_wFontStyle,
                stPrintIn.dwDataSize, stPrintIn.szData);

            if ((nRet = g_devBKC310Impl.LineModePrintTextUniversal((LPSTR)stPrintIn.szData, stPrintIn.dwDataSize,
                                                                   stPrintIn.ulX * 8 / 10, m_wDouWidth, m_wDouHeight,
                                                                   m_wFontType, m_wFontStyle,
                                                                   (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE))) != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "打印文本(缺省点阵): LineModePrintTextUniversal(%s, %d, %d, %d, %d, %d, %d) fail, ErrCode: %d, Return: %s.",
                    stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10, m_wDouWidth, m_wDouHeight,
                    m_wFontType, m_wFontStyle, stPrintIn.lOtherParam[0], nRet,
                    ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        } else
        {
            Log(ThisModule, __LINE__,
                "打印字串(TrueType方式): X=%d(%d点)|Y=%d(%d点)|DouWidth=%d|DouHeight=%d|"
                "FontType=%d|FontStyle=%d|FontName=%s|H*W=%d*%d|L=%d|D=%s",
                stPrintIn.ulX, stPrintIn.ulX * 8 / 10, stPrintIn.ulY, stPrintIn.ulY * 8 / 10,
                m_wDouWidth, m_wDouHeight, m_wFontType, m_wFontStyle,
                m_stPrtFontPar.szFontName, m_stPrtFontPar.dwMarkPar[1], m_stPrtFontPar.dwMarkPar[2],
                stPrintIn.dwDataSize, stPrintIn.szData);
            if ((nRet = g_devBKC310Impl.LineModePrintTrueType(
                                    (LPSTR)stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10,
                                    m_stPrtFontPar.szFontName,
                                    m_stPrtFontPar.dwMarkPar[1] * 8 / 10, m_stPrtFontPar.dwMarkPar[1] * 8 / 10,
                                    m_wFontStyle > 0 ? TRUE : FALSE,
                                    (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE))) != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "打印文本(TrueType): LineModePrintTrueType(%s, %d, %d, %s, %d, %d, %d, %d) fail, "
                    "ErrCode: %d, Return: %s.",
                    stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10,
                    m_stPrtFontPar.szFontName,
                    m_stPrtFontPar.dwMarkPar[1] * 8 / 10, m_stPrtFontPar.dwMarkPar[1] * 8 / 10,
                    m_wFontStyle > 0 ? TRUE : FALSE, (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE),
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    } else
    if (stPrintIn.wInMode == PRINTFORM_PIC)
    {
        Log(ThisModule, __LINE__, "打印图片: X=%d(%d)|Y=%d(%d)|D=%s|",
            stPrintIn.ulX, stPrintIn.ulX * 8 / 10,
            stPrintIn.ulY, stPrintIn.ulY * 8 / 10, stPrintIn.szData);

        // 打印图片
        if (wDevType == DEV_SNBC_BKC310)
        {
            nRet = g_devBKC310Impl.LineModePrintImageRAM((LPSTR)stPrintIn.szData, stPrintIn.dwDataSize,
                                                         stPrintIn.ulX * 8 / 10,
                                                         (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE));
        } else
        {
            nRet = g_devBKC310Impl.LineModePrintImageByMode((LPSTR)stPrintIn.szData, stPrintIn.dwDataSize,
                                                            stPrintIn.ulX * 8 / 10,
                                                            (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE));
        }

        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印图片: LineModePrintImageRAM/LineModePrintImageByMode(%s, %d, %d, %d) fail, ErrCode: %d, Return: %s.",
                stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10, stPrintIn.lOtherParam[0],
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (stPrintIn.wInMode == PRINTFORM_BAR)  // 条形码
    {
        // 转换条码类型
        WORD wBarCode = BAR_UPCA;
        switch(stPrintIn.lOtherParam[1])
        {
            case BAR_UPCA:      wBarCode = 65; break;   // UPC-A
            case BAR_UPCC:      wBarCode = 66; break;   // UPC-C
            case BAR_JAN13:     wBarCode = 67; break;   // JAN13/EAN13
            case BAR_JAN8:      wBarCode = 68; break;   // JAN8/EAN8
            case BAR_CODE39:    wBarCode = 69; break;   // CODE 39
            case BAR_INTE:      wBarCode = 70; break;   // INTERLEAVED 2 OF 5
            case BAR_CODEBAR:   wBarCode = 71; break;   // CODEBAR
            case BAR_CODE93:    wBarCode = 72; break;   // CODE93
            case BAR_CODE128:   wBarCode = 73; break;   // CODE128
        }

        Log(ThisModule, __LINE__, "打印条形码: X=%d(%d)|Y=%d(%d)|Type=%d(%d)|"
                                  "BasicWidth=%d|Height=%d|HriFontType=%d|HriFontPos=%d|D=%s|",
            stPrintIn.ulX, stPrintIn.ulX * 8 / 10, stPrintIn.ulY, stPrintIn.ulY * 8 / 10,
            stPrintIn.lOtherParam[1], wBarCode, stPrintIn.lOtherParam[2],
            stPrintIn.lOtherParam[3], stPrintIn.lOtherParam[4],
            stPrintIn.lOtherParam[5], stPrintIn.szData);

        // 打印条形码
        nRet = g_devBKC310Impl.LineModePrintBarcode(stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10,
                                                    wBarCode, stPrintIn.lOtherParam[2],
                                                    stPrintIn.lOtherParam[3], stPrintIn.lOtherParam[4],
                                                    stPrintIn.lOtherParam[5],
                                                    (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE));
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印条形码: LineModePrintBarcode(%s, %d, %d, %d, %d, %d, %d, %d, %d, %d) fail, "
                "ErrCode: %d, Return: %s.",
                stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX, stPrintIn.lOtherParam[1],
                stPrintIn.lOtherParam[2], stPrintIn.lOtherParam[3], stPrintIn.lOtherParam[4],
                stPrintIn.lOtherParam[5], stPrintIn.lOtherParam[0],
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (stPrintIn.wInMode == PRINTFORM_QR)  // 二维码
    {
        Log(ThisModule, __LINE__, "打印二维码: X=%d(%d)|Y=%d(%d)|BasicWidth=%d|SymbolType=%d|"
                                  "LanguageMode=%d|ErrorCorrect=%d|D=%s|",
            stPrintIn.ulX, stPrintIn.ulX * 8 / 10, stPrintIn.ulY, stPrintIn.ulY * 8 / 10,
            stPrintIn.lOtherParam[1], stPrintIn.lOtherParam[2], stPrintIn.lOtherParam[3],
            stPrintIn.lOtherParam[4], stPrintIn.szData);

        // 打印二维码
        nRet = g_devBKC310Impl.LineModePrintQrcode(stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10,
                                                   stPrintIn.lOtherParam[1], stPrintIn.lOtherParam[2],
                                                   stPrintIn.lOtherParam[3], stPrintIn.lOtherParam[4],
                                                   (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE));
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印二维码: LineModePrintQrcode(%s, %d, %d, %d, %d, %d, %d, %d) fail, ErrCode: %d, Return: %s.",
                stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10, stPrintIn.lOtherParam[1],
                stPrintIn.lOtherParam[2], stPrintIn.lOtherParam[3], stPrintIn.lOtherParam[4],
                stPrintIn.lOtherParam[0], nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (stPrintIn.wInMode == PRINTFORM_PDF417)  // PDF417码
    {
        Log(ThisModule, __LINE__, "打印PDF417码: X=%d(%d)|Y=%d(%d)|BasicWidth=%d|"
                                  "Height=%d|Lines=%d|Columns=%d|ScaleH=%d|ScaleV=%d|"
                                  "CorrectGrade=%d|D=%s|",
            stPrintIn.ulX, stPrintIn.ulX * 8 / 10, stPrintIn.ulY, stPrintIn.ulY * 8 / 10,
            stPrintIn.lOtherParam[1], stPrintIn.lOtherParam[2], stPrintIn.lOtherParam[3],
            stPrintIn.lOtherParam[4], stPrintIn.lOtherParam[5], stPrintIn.lOtherParam[6],
            stPrintIn.lOtherParam[7], stPrintIn.szData);

        // 打印PDF417条码
        nRet = g_devBKC310Impl.LineModePrintPDF417(stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10,
                                                   stPrintIn.lOtherParam[1], stPrintIn.lOtherParam[2],
                                                   stPrintIn.lOtherParam[3], stPrintIn.lOtherParam[4],
                                                   stPrintIn.lOtherParam[5], stPrintIn.lOtherParam[6],
                                                   stPrintIn.lOtherParam[7],
                                                   (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE));
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印PDF417码: LineModePrintPDF417(%s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d) fail, ErrCode: %d, Return: %s.",
                stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX, stPrintIn.lOtherParam[1],
                stPrintIn.lOtherParam[2], stPrintIn.lOtherParam[3], stPrintIn.lOtherParam[4],
                stPrintIn.lOtherParam[5], stPrintIn.lOtherParam[6], stPrintIn.lOtherParam[7],
                stPrintIn.lOtherParam[0], nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (stPrintIn.wInMode == PRINTFORM_FEEDLINE)
    {
        Log(ThisModule, __LINE__, "打印换行: X=%d|Y=%d|LH=%d", stPrintIn.ulX, stPrintIn.ulY, stPrintIn.lOtherParam[0]);

        // 设置行高
        DWORD dwLineHeight = (stPrintIn.lOtherParam[0] > 30 ? stPrintIn.lOtherParam[0] : 30);
        if ((nRet = g_devBKC310Impl.BK_SetTextLineHight(dwLineHeight)) != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印文本前设置行高: BK_SetTextLineHight(%d) fail. Not Return.",
                dwLineHeight);
        }

        // 打印换行
        if ((nRet = g_devBKC310Impl.PrtFeedLine(stPrintIn.ulY)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印换行: PrtFeedLine(%d) fail, ErrCode: %d, Return: %s.",
                stPrintIn.ulX, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        }
    } else
    {
        ;   // 不支持的打印类型
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : ReadForm()                                                **
** Describtion  : ReadForm获取                                               **
** Parameter    : 输入: stReadIn:                                             **
**                输出: stReadOut:                                           **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut)
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : ReadForm()                                                **
** Describtion  : ReadImage获取                                              **
** Parameter    : 输入: stReadIn:                                             **
**                输出: stReadOut:                                           **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : MeidaControl()                                            **
** Describtion  : 介质控制                                                   **
** Parameter    : 输入: stReadIn:                                             **
**                输出: stReadOut:                                           **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    bool bPaperType = true; // 缺省黑标纸
    if ((enMediaAct & MEDIA_CTR_ATPFORWARD) == MEDIA_CTR_ATPFORWARD)    // 黑标纸
    {
        bPaperType = true;
    } else
    if ((enMediaAct & MEDIA_CTR_ATPBACKWARD) == MEDIA_CTR_ATPBACKWARD)  // 连续纸
    {
        bPaperType = false;
    }

    // 0全切, 1走纸一段距离并半切, 2走纸一段距离并全切, 3半切
    INT nCutMode = 0;
    if ((enMediaAct & MEDIA_CTR_TURNMEDIA) == MEDIA_CTR_TURNMEDIA)      // 1走纸一段距离并半切
    {
        nCutMode = 1;
    } else
    if ((enMediaAct & MEDIA_CTR_STAMP) == MEDIA_CTR_STAMP)              // 2走纸一段距离并全切
    {
        nCutMode = 2;
    } else
    if ((enMediaAct & MEDIA_CTR_PARK) == MEDIA_CTR_PARK)                // 3半切
    {
        nCutMode = 3;
    }

    INT nRet = PTR_SUCCESS;
    if ((nRet = g_devBKC310Impl.SetCutPaper(bPaperType, usParam, nCutMode)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : SetData()                                                 **
** Describtion  : 设置                                                       **
** Parameter    : 输入: vData: 设置数据                                       **
**                     wDataType: 设置类别                                   **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::SetData(void *vData, WORD wDataType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = PTR_SUCCESS;

    switch(wDataType)
    {
        case DTYPE_LIB_PATH:    // 设置Lib路径
        {
            g_devBKC310Impl.SetLibPath((LPCSTR)vData);
            break;
        }
        case SET_PRT_FORMAT:            // 设置打印格式
        {
            if (vData != nullptr)
            {
                g_devBKC310Impl.SetPrintFormat(*((LPSTPRINTFORMAT)vData));
            }
            break;
        }
        case SET_BLACK_MOVE:            // 设置黑标偏移
        {
            if (vData != nullptr)
            {
                return g_devBKC310Impl.BK_SetBlackMove(*((UINT*)vData));
            } else
            {
                return ERR_PTR_PARAM_ERR;
            }
            break;
        }
        case SET_DEV_RECON:             // 设置断线重连标记为T
        {
            return g_devBKC310Impl.SetReConFlag(TRUE);
        }        
        case SET_DEV_OPENMODE:          // 设置设备打开模式                            // 30-00-00-00(FT#0067)
        {                                                                           // 30-00-00-00(FT#0067)
            g_devBKC310Impl.SetOpenMode(((LPSTDEVOPENMODE)vData)->nOpenMode,        // 30-00-00-00(FT#0067)
                                        ((LPSTDEVOPENMODE)vData)->nOpenParam);      // 30-00-00-00(FT#0067)
            break;
        }                                                                           // 30-00-00-00(FT#0067)
        case DTYPE_SET_PRTFONT:         // 设置打印字体
        {
            SetPrintMode(*((LPDEVPTRFONTPAR)vData));
            break;
        }
        case SET_DEV_PRTMODE:           // 设置设备的打印模式
        {
            if (*(LPWORD)vData == PRT_MODE_LINE)    // 行模式
            {
                nRet = g_devBKC310Impl.BK_SetPrintMode(PRINT_LINE_MODE);
                Log(ThisModule, __LINE__, "设置打印模式: BK_SetPrintMode(%d|LineMode), ErrCode = %d.",
                    PRINT_LINE_MODE, nRet);
            } else
            {
                nRet = g_devBKC310Impl.BK_SetPrintMode(PRINT_PAGE_MODE);
                Log(ThisModule, __LINE__, "设置打印模式: BK_SetPrintMode(%d|PageMode), ErrCode = %d.",
                    PRINT_PAGE_MODE, nRet);
            }
            break;
        }
        case SET_DEV_LINEFEED:          // 打印换行
        {
            Log(ThisModule, __LINE__, "打印换行: LineFeed=%d", vData == nullptr ? 1 : *((INT*)vData));
            // 打印换行
            if ((nRet = g_devBKC310Impl.PrtFeedLine(vData == nullptr ? 1 : *((INT*)vData))) != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__, "打印换行: PrtFeedLine(%d) fail, ErrCode: %d, Return: %s.",
                    vData == nullptr ? 1 : *((INT*)vData), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
            break;
        }
        case SET_DEV_ROWHEIGHT:          // 设置行高
        {
            Log(ThisModule, __LINE__, "设置行高: RowHeight=%d", vData == nullptr ? 30 : *((INT*)vData));
            // 设置行高
            if ((nRet = g_devBKC310Impl.BK_SetTextLineHight(vData == nullptr ? 30 : *((INT*)vData))) != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "设置行高: BK_SetTextLineHight(%d) fail, ErrCode: %d, Return: %s.",
                    vData == nullptr ? 1 : *((INT*)vData), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
            break;
        }
        default:
            break;
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : SetData()                                                 **
** Describtion  : 获取                                                      **
** Parameter    : 输入: vData: 获取数据                                       **
**                     wDataType: 获取类别                                   **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
int CDevPTR_SNBC::GetData(void *vData, WORD wDataType)
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                          **
** ClassName    : CDevPTR_SNBC                                              **
** Symbol       : CDevPTR_SNBC::                                            **
** Function     : SetData()                                                 **
** Describtion  : 获取版本                                                   **
** Parameter    : 输入: lSize: 应答缓存size                                   **
**                     usType: 获取类别                                      **
**                输出: szVer: 应答缓存                                       **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/27                                                **
** V-R-T        : DEV-SNBC-00000010                                         **
** History      :                                                           **
*****************************************************************************/
void CDevPTR_SNBC::GetVersion(char* szVer, long lSize, ushort usType)
{
    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_DEVRPR:    // DevRPR版本号
                break;
            case GET_VER_FW:        // 固件版本号
                g_devBKC310Impl.GetFwVersion(szVer, (ULONG*)(&lSize));
                break;
            default:
                break;
        }
    }
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPTR_SNBC.CPP                                           **
** ClassName    : CDevPTR_SNBC                                               **
** Symbol       : CDevPTR_SNBC::                                             **
** Function     : SetData()                                                 **
** Describtion  : 设置打印字体                                                **
** Parameter    : 输入: stFontPar 打印属性                                    **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
INT CDevPTR_SNBC::SetPrintMode(DEVPTRFONTPAR stFontPar)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    // AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    memcpy(&m_stPrtFontPar, &stFontPar, sizeof(DEVPTRFONTPAR));

    // 打印前属性初始化
    g_devBKC310Impl.BK_SetTextLineHight(0);                             // 打印行高
    g_devBKC310Impl.BK_SetTextCharacterSpace(0);                        // 字符间距
    g_devBKC310Impl.BK_SetTextBold(0);                                  // 字体正常
    g_devBKC310Impl.BK_SetTextDoubleWH(0, 0);                           // 倍宽倍高


    // 打印前属性设置:扩展属性
    m_wFontType = 0;                                                    // 标准ASCII(设备内置)
    m_wDouWidth = 1;                                                    // 倍宽(1~6)
    m_wDouHeight = 1;                                                   // 倍高(1~6)
    m_wFontStyle = 0;                                                   // 字体类型(0~3)
    if ((stFontPar.dwFontType & FONT_BOLD) == FONT_BOLD)                // 黑体(粗体)
    {
        m_wFontStyle = (m_wFontStyle + 0x08);
    }
    if ((stFontPar.dwFontType & FONT_DOUBLE) == FONT_DOUBLE)            // 2倍宽
    {
        m_wDouWidth = 2;
    }
    if ((stFontPar.dwFontType & FONT_TRIPLE) == FONT_TRIPLE)            // 3倍宽
    {
        m_wDouWidth = 3;
    }
    if ((stFontPar.dwFontType & FONT_DOUBLEHIGH) == FONT_DOUBLEHIGH)    // 2倍高
    {
        m_wDouHeight = 2;
    }
    if ((stFontPar.dwFontType & FONT_TRIPLEHIGH) == FONT_TRIPLEHIGH)    // 3倍高
    {
        m_wDouHeight = 3;
    }
    if ((stFontPar.dwFontType & FONT_UNDER) == FONT_UNDER)              // 一条下划线
    {
        m_wFontStyle = (m_wFontStyle + 0x80);
    }
    if ((stFontPar.dwFontType & FONT_UPSIDEDOWN) == FONT_UPSIDEDOWN)    // 倒置
    {
        m_wFontStyle = (m_wFontStyle + 0x200);
    }
    if ((stFontPar.dwFontType & FONT_ROTATE90) == FONT_ROTATE90)        // 顺时针旋转90度
    {
        m_wFontStyle = (m_wFontStyle + 0x1000);
    }
    if ((stFontPar.dwFontType & FONT_CONDENSED) == FONT_CONDENSED)      // 压缩:对应压缩ASCII
    {
        m_wFontType = 1;                                                // 压缩ASCII(设备内置)
    }

    if (stFontPar.dwMarkPar[0] == 3)                                    // 标准宋体(设备内置)
    {
        m_wFontType = 3;
    }

    return PTR_SUCCESS;
}

// 转换为IDevPTR返回码/错误码
INT CDevPTR_SNBC::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        // >0: Impl处理返回
        case IMP_SUCCESS:               // 0:成功, ERR_SUCCESS/IMP_DEV_ERR_SUCCESS
            return PTR_SUCCESS;
        case IMP_ERR_LOAD_LIB:          // 1:动态库加载失败
            return ERR_PTR_OTHER;
        case IMP_ERR_PARAM_INVALID:     // 2:参数无效
            return ERR_PTR_PARAM_ERR;
        case IMP_ERR_UNKNOWN:           // 3:未知错误
            return ERR_PTR_OTHER;
        // >0: Device返回
        case IMP_DEV_ERR_FIIL:          // -1:失败
        case IMP_DEV_ERR_HANDLE:        // -2:无效句柄
        case IMP_DEV_ERR_PARAMATER:     // -3:无效参数
        case IMP_DEV_ERR_FILE:          // -4:参数文件错误
        case IMP_DEV_ERR_READ:          // -5:读取文件或数据错误
        case IMP_DEV_ERR_WRITE:         // -6:下发文件或数据错误
        case IMP_DEV_ERR_NOT_SUPPORT:   // -7:此功能不支持
        case IMP_DEV_ERR_BITMAP_INVAILD:// -8:位图错误
        case IMP_DEV_ERR_LOADDLL_FAILURE:// -9:动态库加载失败
        case IMP_DEV_ERR_FIRNOSUPPORT:  // -10:固件不支持
        case IMP_DEV_ERR_UNKOWN_ERROR:  // -127:未知错误
            return ERR_PTR_HWERR;
        default:
            return ERR_PTR_OTHER;
    }
}

