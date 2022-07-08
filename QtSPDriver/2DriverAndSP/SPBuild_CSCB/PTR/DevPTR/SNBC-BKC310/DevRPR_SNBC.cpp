#include "DevRPR_SNBC.h"

// 版本号
BYTE  byVRTU[17]  = "DevPRPR01000000";

static const char *ThisFile = "DevRPR_SNBC.cpp";

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

CDevPTR_SNBC::CDevPTR_SNBC(LPCSTR lpDevType)
{
     SetLogFile(LOGFILE, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件
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
    } else
    if (MCMP_IS0(pMode, DEV_SNBC_BTNH80_STR))
    {
        wDevType = DEV_SNBC_BTNH80;
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
    INT nRet = PTR_SUCCESS;
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
    if (wDevType == DEV_SNBC_BKC310)
    {
        INT nRet = PTR_SUCCESS;
        if ((nRet = g_devBKC310Impl.LineModePrintImageRAM((LPSTR)szImagePath, strlen(szImagePath), ulOrgX * 15 * 8 / 10)) != IMP_SUCCESS)
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

    INT nRet = PTR_SUCCESS;
    if ((nRet = g_devBKC310Impl.ControlMedia(bPaperType, usParam)) != IMP_SUCCESS)
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
    if (vData != nullptr)
    {
        switch(wDataType)
        {
            case SET_PRT_FORMAT:    // 设置打印格式
                g_devBKC310Impl.SetPrintFormat(*((LPSTPRINTFORMAT)vData));
                break;
            default:
                break;
        }
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
                memcpy(szVer, byVRTU,
                       (strlen((char*)byVRTU) >= lSize ? lSize -1 : strlen((char*)byVRTU)));
                break;
            case GET_VER_FW:        // 固件版本号
                g_devBKC310Impl.GetFwVersion(szVer, (ULONG*)(&lSize));
                break;
            default:
                break;
        }
    }
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

CHAR* CDevPTR_SNBC::ConvertErrCodeToStr(INT nRet)
{

}





