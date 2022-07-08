/***************************************************************
* 文件名称：DevCPR_BTT080AII.cpp
* 文件描述：新北洋流水打印模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevJPR_BTT080AII.h"

static const char *ThisFile = "DEVJPR_BTT080AII.cpp";

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

CDevJPR_BTT080AII::CDevJPR_BTT080AII(LPCSTR lpDevType)
    : g_devImpl(LOG_NAME_DEV)
{
     SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

     m_wFontType = 0;                               // 字体类型
     m_wDouWidth = 1;                               // 倍宽(1~6)
     m_wDouHeight = 1;                              // 倍高(1~6)
     m_wFontStyle = 0;                              // 字体风格(0~3)
}

CDevJPR_BTT080AII::~CDevJPR_BTT080AII()
{

}

void CDevJPR_BTT080AII::Release()
{
    return;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : Open()                                                    **
** Describtion  : 打开设备　　　　                                             **
** Parameter    : 输入: pMode: 自定义OPEN参数字串                              **
**                输出: 无                                                   **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    if ((nRet = g_devImpl.DeviceOpen()) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : Close()                                                   **
** Describtion  : 关闭设备　　　　                                             **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::Close()
{
    g_devImpl.DeviceClose();
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : Init()                                                    **
** Describtion  : 设备初始化                                                 **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::Init()
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : Init()                                                    **
** Describtion  : 设备复位(Reset)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::Reset()
{
    INT nRet = PTR_SUCCESS;
    if ((nRet = g_devImpl.Reset()) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : Init()                                                    **
** Describtion  : 设备复位(有参数)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    INT nRet = PTR_SUCCESS;
    if ((nRet = g_devImpl.Reset()) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : SetPrintFormat()                                          **
** Describtion  : 设置当前打印格式                                            **
** Parameter    : stPrintFormat: 定义当前打印的格式具体内容                     **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::GetStatus(DEVPTRSTATUS &stStatus)
{
    //BOOL bRet = TRUE;
    INT nRet = IMP_SUCCESS;
    char szAutoStatus[8] = { 0x00 };
    char szRealtimeDevStatus = 0;

    // 设备状态初始化
    stStatus.wDevice = DEV_STAT_OFFLINE;
    stStatus.wMedia = MEDIA_STAT_UNKNOWN;
    for (INT i = 0; i < 16; i ++)
    {
        stStatus.wPaper[i] = PAPER_STAT_UNKNOWN;
    }
    stStatus.wInk = INK_STAT_NOTSUPP;
    stStatus.wToner = TONER_STAT_NOTSUPP;

    // 实时查询打印机状态
    ULONG ulStaBufSzReal = sizeof(szRealtimeDevStatus);

    // 实时状态获取
    nRet = g_devImpl.GetRtStatus(&szRealtimeDevStatus, &ulStaBufSzReal);
    if (nRet == IMP_SUCCESS)
    {
        if ((szRealtimeDevStatus & DATA_BIT0) ||     // 上盖打开
            (szRealtimeDevStatus & DATA_BIT3) ||     // 打印头过热
            (szRealtimeDevStatus & DATA_BIT2) ||     // 切刀错
            ((szRealtimeDevStatus & DATA_BIT7) && (stStatus.wPaper[0] != PAPER_STAT_OUT)))   // 自动上纸错误
        {
            stStatus.wDevice = DEV_STAT_HWERROR;
        } else
        //卡纸
        /*if ((szRealtimeDevStatus & DATA_BIT5) ||     // 打印处或上纸通道塞纸
            (szRealtimeDevStatus & DATA_BIT6))       // PRST处塞纸
        {
            stStatus.wDevice = DEV_STAT_HWERROR;
            stStatus.wMedia = MEDIA_STAT_JAMMED;
            stStatus.wPaper[0] = PAPER_STAT_JAMMED;;
        } else*/
        {
            stStatus.wDevice = DEV_STAT_ONLINE;
            // 纸状态
            if (szRealtimeDevStatus & DATA_BIT1)     // 纸张用尽
            {
                stStatus.wPaper[0] = PAPER_STAT_OUT;
            } else
            if (szRealtimeDevStatus & DATA_BIT4)     // 纸少
            {
                stStatus.wPaper[0] = PAPER_STAT_LOW;
            } else
            {
                stStatus.wPaper[0] = PAPER_STAT_FULL;
            }

            // 根据纸状态设置Media状态
            if (stStatus.wPaper[0] == PAPER_STAT_OUT)
            {
                stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
            } else
            {
                stStatus.wMedia = MEDIA_STAT_PRESENT;
            }
        }
    } else
    {
        if (nRet != IMP_DEV_ERR_FIIL && nRet != IMP_ERR_NOTOPEN)   // -1/4: OFFLINE时返回
        {
            stStatus.wDevice = DEV_STAT_HWERROR;
        }
    }

    // 自动状态返回查询打印机状态(TKIOSK)
    /*ULONG ulStaBufSz = sizeof(szAutoStatus);
    if((nRet = g_devImpl.GetAutoStatus(szAutoStatus, &ulStaBufSz)) == IMP_SUCCESS)
    {
        // 非脱机状态
        if((szAutoStatus[0] & DATA_BIT3) == 0)
        {
            // 设备Error
            if((szAutoStatus[0] & DATA_BIT5) ||                      // 上盖打开
               (szAutoStatus[1] & DATA_BIT4))                        // 自动上纸错误
            {
                stStatus.wDevice = DEV_STAT_HWERROR;
            } else
            // 卡纸
            if((szAutoStatus[1] & DATA_BIT2) ||                      // 防卡纸模块报错
               (szAutoStatus[1] & DATA_BIT7))                        // 防拉纸模块报错
            {
                stStatus.wDevice = DEV_STAT_HWERROR;
                stStatus.wMedia = MEDIA_STAT_JAMMED;
                stStatus.wPaper[0] = PAPER_STAT_JAMMED;
            } else
            {
                // BUSY
                if(szAutoStatus[2] & DATA_BIT6)                          // 打印中
                {
                    stStatus.wDevice = DEV_STAT_BUSY;
                } else
                {
                    stStatus.wDevice = DEV_STAT_ONLINE;
                }

                // 纸状态
                if((szAutoStatus[2] & DATA_BIT2) && (szAutoStatus[2] & DATA_BIT3))    // 纸张用尽
                {
                    stStatus.wPaper[0] = PAPER_STAT_OUT;
                } else
                if((szAutoStatus[2] & DATA_BIT0) && (szAutoStatus[2] & DATA_BIT1))    // 纸少
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
            }
        }
    }*/

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
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : PrintData()                                              **
** Describtion  : 打印字串(无指定打印坐标)                                     **
**                打印格式使用上一次调用SetPrintFormat设置的格式，               **
**                如果没有设置则使用默认参数                                    **
** Parameter    : 输入: pStr: 要打印的字串                                     **
**                     ulDataLen: 数据长度，如果为-1，pStr以\0结束             **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                            **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::PrintData(const char *pStr, unsigned long ulDataLen)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = PTR_SUCCESS;
    Log(ThisModule, __LINE__, "打印字串: X=%d|L=%d|D=%s|", 0, ulDataLen, pStr);

    if ((nRet = g_devImpl.LineModePrintText((LPSTR)pStr, ulDataLen, 0)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
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
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::PrintDataOrg(const char *pStr, unsigned long ulDataLen,
                               unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    ULONG ulX = ulOrgX * 8 / 10;    // 入参单位:0.1MM,转换为点

    INT nRet = PTR_SUCCESS;
    Log(ThisModule, __LINE__, "打印字串: X=%d(%d)|Y=%d|L=%d|D=%s|", ulOrgX, ulX, ulOrgY, ulDataLen, pStr);

    if ((nRet = g_devImpl.LineModePrintText((LPSTR)pStr, ulDataLen, ulX)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : PrintImage()                                              **
** Describtion  : 图片打印(无指定打印坐标)                                     **
** Parameter    : 输入: szImagePath: 图片保存路径                              **
**                     ulWidth: 打印宽度                                     **
**                     ulHeight: 打印高度                                    **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = PTR_SUCCESS;
    Log(ThisModule, __LINE__, "打印图片: X=%d|W=%d|H=%d|D=%s|", 0, ulWidth, ulHeight, szImagePath);

    if ((nRet = g_devImpl.LineModePrintImageRAM((LPSTR)szImagePath, strlen(szImagePath), 0)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : PrintImageOrg()                                           **
** Describtion  : 图片打印(指定打印坐标)                                       **
** Parameter    : 输入: szImagePath: 图片保存路径                              **
**                     ulOrgX: 打印图片的列坐标                               **
**                     ulOrgY: 打印图片的行坐标                               **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = PTR_SUCCESS;
    Log(ThisModule, __LINE__, "打印图片: X=%d(%d)|Y=%d(%d)|D=%s|",
        ulOrgX, ulOrgX * 15 * 8 / 10, ulOrgX, ulOrgY * 15 * 8 / 10, szImagePath);

    if ((nRet = g_devImpl.LineModePrintImageRAM((LPSTR)szImagePath,
                                                strlen(szImagePath),
                                                ulOrgX * 15 * 8 / 10)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}


/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
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
int CDevJPR_BTT080AII::PrintForm(DEVPRINTFORMIN stPrintIn, DEVPRINTFORMOUT &stPrintOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 注: stPrintIn.ulX, stPrintIn.ulY 单位: 0.1毫米
    //

    if (stPrintIn.wInMode == PRINTFORM_TEXT)
    {
        Log(ThisModule, __LINE__, "打印字串: X=%d(%d)|Y=%d(%d)|DouWidth=%d|DouHeight=%d|"
                                  "FontType=%d|FontStyle=%d|L=%d|D=%s|",
            stPrintIn.ulX, stPrintIn.ulX * 8 / 10, stPrintIn.ulY, stPrintIn.ulY * 8 / 10,
            m_wDouWidth, m_wDouHeight, m_wFontType, m_wFontStyle,
            stPrintIn.dwDataSize, stPrintIn.szData);

        // 打印文本
        //if ((nRet = g_devBKC310Impl.LineModePrintText((LPSTR)stPrintIn.szData, stPrintIn.dwDataSize,
        //                                              stPrintIn.ulX * 8 / 10,
        //                                             (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE))) != IMP_SUCCESS)
        //{
        //    Log(ThisModule, __LINE__, "打印文本: LineModePrintText(%s, %d, %d, %d) fail, ErrCode: %d, Return: %s.",
        //        stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10, stPrintIn.lOtherParam[0],
        //        nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        //    return ConvertErrorCode(nRet);
        //}

        if ((nRet = g_devImpl.LineModePrintTextUniversal((LPSTR)stPrintIn.szData, stPrintIn.dwDataSize,
                                                         stPrintIn.ulX * 8 / 10, m_wDouWidth, m_wDouHeight,
                                                         m_wFontType, m_wFontStyle,
                                                         (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE))) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印文本: LineModePrintTextUniversal(%s, %d, %d, %d, %d, %d, %d) fail, ErrCode: %d, Return: %s.",
                stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10, m_wDouWidth, m_wDouHeight,
                m_wFontType, m_wFontStyle, stPrintIn.lOtherParam[0], nRet,
                ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (stPrintIn.wInMode == PRINTFORM_PIC)
    {
        Log(ThisModule, __LINE__, "打印图片: X=%d(%d)|Y=%d(%d)|D=%s|",
            stPrintIn.ulX, stPrintIn.ulX * 8 / 10,
            stPrintIn.ulY, stPrintIn.ulY * 8 / 10, stPrintIn.szData);

        // 打印图片
        //nRet = g_devImpl.LineModePrintImageRAM((LPSTR)stPrintIn.szData, stPrintIn.dwDataSize,
        //                                       stPrintIn.ulX * 8 / 10,
        //                                       (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE));
        nRet = g_devImpl.LineModePrintImageByMode((LPSTR)stPrintIn.szData, stPrintIn.dwDataSize,
                                                  stPrintIn.ulX * 8 / 10,
                                                  (stPrintIn.lOtherParam[0] == 0 ? FALSE : TRUE));
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
        nRet = g_devImpl.LineModePrintBarcode(stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10,
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
        nRet = g_devImpl.LineModePrintQrcode(stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10,
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
        nRet = g_devImpl.LineModePrintPDF417(stPrintIn.szData, stPrintIn.dwDataSize, stPrintIn.ulX * 8 / 10,
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
        if ((nRet = g_devImpl.BK_SetTextLineHight(dwLineHeight)) != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印文本前设置行高: BK_SetTextLineHight(%d) fail. Not Return.",
                dwLineHeight);
        }

        // 打印换行
        if ((nRet = g_devImpl.PrtFeedLine(stPrintIn.ulY)) != IMP_SUCCESS)
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
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : ReadForm()                                                **
** Describtion  : ReadForm获取                                               **
** Parameter    : 输入: stReadIn:                                            **
**                输出: stReadOut:                                           **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut)
{
    return ERR_PTR_UNSUP_CMD;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : ReadForm()                                                **
** Describtion  : ReadImage获取                                              **
** Parameter    : 输入: stReadIn:                                            **
**                输出: stReadOut:                                           **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)
{
    return ERR_PTR_UNSUP_CMD;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : MeidaControl()                                            **
** Describtion  : 介质控制                                                   **
** Parameter    : 输入: stReadIn:                                            **
**                输出: stReadOut:                                           **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
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

    INT nRet = PTR_SUCCESS;
    if ((nRet = g_devImpl.ControlMedia(bPaperType, usParam)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : SetData()                                                 **
** Describtion  : 设置                                                       **
** Parameter    : 输入: vData: 设置数据                                       **
**                     wDataType: 设置类别                                   **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::SetData(void *vData, WORD wDataType)
{    
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    switch(wDataType)
    {
        /*case SET_PRT_FORMAT:    // 设置打印格式
        {
            if (vData != nullptr)
            {
                g_devImpl.SetPrintFormat(*((LPSTPRINTFORMAT)vData));
            }
            break;
        }*/    
        case DTYPE_LIB_PATH:    // 设置Lib路径
        {
            g_devImpl.SetLibPath((LPCSTR)vData);
            break;
        }
        case SET_DEV_RECON:     // 设置断线重连标记为T
        {
            return g_devImpl.SetReConFlag(TRUE);
        }
        case SET_DEV_OPENMODE:  // 设置设备打开模式
        {
            g_devImpl.SetOpenMode(((LPSTDEVOPENMODE)vData)->nOpenMode,
                                  ((LPSTDEVOPENMODE)vData)->nOpenParam);
            break;
        }
        case DTYPE_SET_PRTFONT: // 设置打印字体
        {
            SetPrintMode(*((LPDEVPTRFONTPAR)vData));
            break;
        }
        case SET_DEV_LINEFEED:          // 打印换行
        {
            Log(ThisModule, __LINE__, "打印换行: LineFeed=%d", vData == nullptr ? 1 : *((INT*)vData));
            // 打印换行
            if ((nRet = g_devImpl.PrtFeedLine(vData == nullptr ? 1 : *((INT*)vData))) != IMP_SUCCESS)
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
            if ((nRet = g_devImpl.BK_SetTextLineHight(vData == nullptr ? 30 : *((INT*)vData))) != ERR_SUCCESS)
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
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : SetData()                                                 **
** Describtion  : 获取                                                      **
** Parameter    : 输入: vData: 获取数据                                       **
**                     wDataType: 获取类别                                   **
**                输出: 无                                                  **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevJPR_BTT080AII::GetData(void *vData, WORD wDataType)
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : SetData()                                                 **
** Describtion  : 获取版本                                                   **
** Parameter    : 输入: lSize: 应答缓存size                                   **
**                     usType: 获取类别                                      **
**                输出: szVer: 应答缓存                                       **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
void CDevJPR_BTT080AII::GetVersion(char* szVer, long lSize, ushort usType)
{
    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_DEV:   // DevXXX版本号
                break;
            case GET_VER_FW:    // 固件版本号
                g_devImpl.GetFwVersion(szVer, (ULONG*)(&lSize));
                break;
            default:
                break;
        }
    }
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevJPR_BTT080AII.CPP                                     **
** ClassName    : CDevJPR_BTT080AII                                         **
** Symbol       : CDevJPR_BTT080AII::                                       **
** Function     : SetData()                                                 **
** Describtion  : 设置打印字体                                                **
** Parameter    : 输入: stFontPar 打印属性                                    **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
INT CDevJPR_BTT080AII::SetPrintMode(DEVPTRFONTPAR stFontPar)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    // AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 打印前属性初始化
    g_devImpl.BK_SetTextLineHight(0);                                   // 打印行高
    g_devImpl.BK_SetTextCharacterSpace(0);                              // 字符间距
    g_devImpl.BK_SetTextBold(0);                                        // 字体正常
    g_devImpl.BK_SetTextDoubleWH(0, 0);                                 // 倍宽倍高


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
INT CDevJPR_BTT080AII::ConvertErrorCode(INT nRet)
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

