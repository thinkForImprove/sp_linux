/***************************************************************
* 文件名称：DevPPR_MB2.cpp
* 文件描述：MB2存折打印模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevPPR_MB2.h"

static const char *ThisFile = "DevPPR_MB2.cpp";

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

CDevPPR_MB2::CDevPPR_MB2(LPCSTR lpDevType)
    : g_devImpl(LOG_NAME_DEV, (LPSTR)lpDevType)
{
     SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件
}

CDevPPR_MB2::~CDevPPR_MB2()
{

}

void CDevPPR_MB2::Release()
{
    return;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;
    if ((nRet = g_devImpl.DeviceOpen((LPSTR)pMode)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    //g_devImpl.SetBeep(0, 0);

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
** Function     : Close()                                                   **
** Describtion  : 关闭设备　　　　                                             **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_MB2::Close()
{
    g_devImpl.DeviceClose();
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备初始化                                                 **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_MB2::Init()
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备复位(Reset)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_MB2::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    if ((nRet = g_devImpl.Reset()) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "复位: Reset() fail, ErrCode: %d, Return: %s",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备复位(有参数)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_MB2::ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    if (enMediaAct == MEDIA_CTR_CUT)    // 清错
    {
        if ((nRet = g_devImpl.CleanError()) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "复位Ex: 执行清错处理: CleanError() fail, ErrCode: %d, Return: %s",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        if ((nRet = g_devImpl.Reset()) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "复位Ex: 执行复位: Reset() fail, ErrCode: %d, Return: %s",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
** Function     : SetPrintFormat()                                          **
** Describtion  : 设置当前打印格式                                            **
** Parameter    : stPrintFormat: 定义当前打印的格式具体内容                     **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_MB2::GetStatus(DEVPTRSTATUS &stStatus)
{
    //BOOL bRet = TRUE;
    INT nRet = IMP_SUCCESS;
    CHAR szDevStatus[2] = { 0x00 };
    CHAR szMediaStatus[2] = { 0x00 };

    // 设备状态初始化
    stStatus.wDevice = DEV_STAT_OFFLINE;
    stStatus.wMedia = MEDIA_STAT_UNKNOWN;
    for (INT i = 0; i < 16; i ++)
    {
        stStatus.wPaper[i] = PAPER_STAT_UNKNOWN;
    }
    stStatus.wInk = INK_STAT_NOTSUPP;
    stStatus.wToner = TONER_STAT_NOTSUPP;

    // 获取设备状态
    nRet = g_devImpl.GetDevStatus(szDevStatus);
    if (nRet < 0)
    {
        if (nRet == IMP_ERR_DEV_NO_OPEN ||              // 未调用过OPEN函数/调用不成功
            nRet == IMP_ERR_DEV_NOT_OLI_EMULATION ||    // 未调用过OPEN函数/调用不成功
            nRet == IMP_ERR_DEV_COMM_OFF ||             // 设备离线
            nRet == IMP_ERR_DEV_OFFLINE ||
            nRet == IMP_ERR_DEV_OFFLINE2)                // 设备断电/断线
        {
            stStatus.wDevice = DEV_STAT_OFFLINE;
            return ERR_PTR_NOT_OPEN;
        } else
        {
            stStatus.wDevice = DEV_STAT_HWERROR;
            return ERR_PTR_HWERR;
        }
    }
    switch(nRet)
    {
        case STATUS_NOMAL:                      // 状态正常
        case STATUS_MEDIA_NONE:                 // 无介质
        case STATUS_MEDIA_PREENTERING:          // 介质存在、位于入口
        case STATUS_MEDIA_ENTERING:             // 介质已对齐
        case STATUS_MEDIA_PRESENT:              // 介质已插入
        case STATUS_MEDIA_INSERTED_ALL:         // 至页顶
        case STATUS_MEDIA_NEAR_END:             // 至页尾
        case STATUS_MEDIA_MAGNETIC_UNREAD:      // 磁条不能读出
            stStatus.wDevice = DEV_STAT_ONLINE;
            break;
        case STATUS_MEDIA_JAMMED:               // 介质阻塞
        case STATUS_COVER_OPEN:                 // 处于开盖状态/处于离线状态
        case STATUS_COMMAND_WRONG:              // 命令错
        case STATUS_COMM_ERROR:                 // 通信错
        case STATUS_ERROR_UNKNOW:               // 未知状态
            stStatus.wDevice = DEV_STAT_HWERROR;
            break;
    }

    // 获取介质状态
    nRet = g_devImpl.GetMediaStatus(szMediaStatus);
    if (nRet < 0)
    {
        if (nRet == IMP_ERR_DEV_NO_OPEN ||              // 未调用过OPEN函数/调用不成功
            nRet == IMP_ERR_DEV_NOT_OLI_EMULATION ||    // 未调用过OPEN函数/调用不成功
            nRet == IMP_ERR_DEV_COMM_OFF ||             // 设备离线
            nRet == IMP_ERR_DEV_OFFLINE ||
            nRet == IMP_ERR_DEV_OFFLINE2)                // 设备断电/断线
        {
            stStatus.wDevice = DEV_STAT_OFFLINE;
            return ERR_PTR_NOT_OPEN;
        } else
        {
            stStatus.wDevice = DEV_STAT_HWERROR;
            return ERR_PTR_HWERR;
        }
    }
    switch(nRet)
    {
        case STATUS_NOMAL:                      // 状态正常
            stStatus.wDevice = DEV_STAT_ONLINE;
            break;
        case STATUS_MEDIA_NONE:                 // 无介质
            stStatus.wDevice = DEV_STAT_ONLINE;
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
            break;
        case STATUS_MEDIA_PREENTERING:          // 介质存在、位于入口
        case STATUS_MEDIA_ENTERING:             // 介质已对齐
            stStatus.wDevice = DEV_STAT_ONLINE;
            stStatus.wMedia = MEDIA_STAT_ENTERING;
            //stStatus.wPaper[0] = PAPER_STAT_OUT;
            break;
        case STATUS_MEDIA_PRESENT:              // 介质已插入
        case STATUS_MEDIA_INSERTED_ALL:         // 至页顶
        case STATUS_MEDIA_NEAR_END:             // 至页尾
        case STATUS_MEDIA_MAGNETIC_UNREAD:      // 磁条不能读出
            stStatus.wDevice = DEV_STAT_ONLINE;
            stStatus.wMedia = MEDIA_STAT_PRESENT;
            //stStatus.wPaper[0] = PAPER_STAT_FULL;
            break;
        case STATUS_MEDIA_JAMMED:               // 介质阻塞
            stStatus.wDevice = DEV_STAT_HWERROR;
            stStatus.wMedia = MEDIA_STAT_JAMMED;
            //stStatus.wPaper[0] = PAPER_STAT_JAMMED;
            break;
        case STATUS_COVER_OPEN:                 // 处于开盖状态/处于离线状态
        case STATUS_COMMAND_WRONG:              // 命令错
        case STATUS_COMM_ERROR:                 // 通信错
        case STATUS_ERROR_UNKNOW:               // 未知状态
            stStatus.wDevice = DEV_STAT_HWERROR;
            break;
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
    if (stStatus.wDevice == DEV_STAT_BUSY)
    {
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
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::PrintData(const char *pStr, unsigned long ulDataLen)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    Log(ThisModule, __LINE__, "打印字串: X=%d|Y=%d|L=%d|D=%s|", 0, 0, ulDataLen, pStr);

    // 移动介质到指定位置
    nRet = g_devImpl.MoveAbsPos(0, 0);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印字串: 步骤1_移动介质到指定位置: MoveAbsPos(%d, %d) fail, ErrCode: %d, Return: %s.",
            0, 0, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    // 打印文本
    nRet = g_devImpl.PrintText((LPSTR)pStr, ulDataLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印字串: 步骤2_打印文本: PrintText(%s, %d) fail, ErrCode: %d, Return: %s.",
            pStr, ulDataLen, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::PrintDataOrg(const char *pStr, unsigned long ulDataLen,
                                    unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    Log(ThisModule, __LINE__, "打印字串: X=%d|Y=%d|L=%d|D=%s|", ulOrgX, ulOrgY, ulDataLen, pStr);

    // 介质吸入
    /*nRet = g_devImpl.MediaInsert(0, 500);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印字串: 步骤1_介质吸入: MediaInsert(0, 500) fail, ErrCode: %d, Return: %s.",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/

    // 移动介质到指定位置
    nRet = g_devImpl.MoveAbsPos(ulOrgX, ulOrgY);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印字串: 步骤1_移动介质到指定位置: MoveAbsPos(%d, %d) fail, ErrCode: %d, Return: %s.",
            ulOrgX, ulOrgY, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    // 打印文本
    nRet = g_devImpl.PrintText((LPSTR)pStr, ulDataLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印字串: 步骤2_打印文本: PrintText(%s, %d) fail, ErrCode: %d, Return: %s.",
            pStr, ulDataLen, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    return ERR_PTR_UNSUP_CMD;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    Log(ThisModule, __LINE__, "打印图片: X=%d|Y=%d|D=%s|", ulOrgX, ulOrgY, szImagePath);

    // 介质吸入
    /*nRet = g_devImpl.MediaInsert(0, 500);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印图片: 步骤1_介质吸入: MediaInsert(0, 500)) fail, ErrCode: %d, Return: %s.",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/

    // 打印图片
    nRet = g_devImpl.PrintBmp(ulOrgX, ulOrgY, (LPSTR)szImagePath);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印图片: PrintBmp(%d, %d, %s, %s) fail, ErrCode: %d, Return: %s.",
            ulOrgX, ulOrgY, szImagePath, "0", nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::PrintForm(DEVPRINTFORMIN stPrintIn, DEVPRINTFORMOUT &stPrintOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 介质吸入
    /*nRet = g_devImpl.MediaInsert(0, 500);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印: 步骤1_介质吸入: MediaInsert(0, %d) fail, ErrCode: %d, Return: %s.",
            500, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/

    if (stPrintIn.wInMode == PRINTFORM_TEXT)
    {
        Log(ThisModule, __LINE__, "打印字串: X=%d|Y=%d|L=%d|D=%s|",
            stPrintIn.ulX, stPrintIn.ulY, stPrintIn.dwDataSize, stPrintIn.lpData);

        // 移动介质到指定位置
        nRet = g_devImpl.MoveAbsPos(stPrintIn.ulX, stPrintIn.ulY);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印字串: 步骤1_移动介质到指定位置: MoveAbsPos(%d, %d) fail, ErrCode: %d, Return: %s.",
                stPrintIn.ulX, stPrintIn.ulY, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }

        // 打印文本
        nRet = g_devImpl.PrintText((LPSTR)stPrintIn.lpData, stPrintIn.dwDataSize);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印字串: 步骤2_打印文本: PrintText(%s, %d) fail, ErrCode: %d, Return: %s.",
                stPrintIn.lpData, stPrintIn.dwDataSize, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (stPrintIn.wInMode == PRINTFORM_PIC)
    {
        Log(ThisModule, __LINE__, "打印图片: X=%d|Y=%d|D=%s|", stPrintIn.ulX, stPrintIn.ulY, stPrintIn.lpData);

        // 打印图片
        nRet = g_devImpl.PrintBmp(stPrintIn.ulX, stPrintIn.ulY, stPrintIn.lpData);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印图片: PrintBmp(%d, %d, %s, %s) fail, ErrCode: %d, Return: %s.",
                stPrintIn.ulX, stPrintIn.ulY, stPrintIn.lpData, "0",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        ;   // 不支持的打印类型
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 介质吸入
    /*nRet = g_devImpl.MediaInsert(0, 500);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "信息读取: 步骤1_介质吸入: MediaInsert(0, %d) fail, ErrCode: %d, Return: %s.",
            500, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/

    // 读磁道1
    if (stReadIn.wInMode == READFORM_TRACK1)
    {
        stReadOut.wInMode = READFORM_TRACK1;
    }

    // 读磁道2
    if (stReadIn.wInMode == READFORM_TRACK2)
    {
        nRet = g_devImpl.MsRead(/*MAG_IBM3604*/stReadIn.lOtherParam[0],
                                stReadOut.szRetData, stReadOut.dwRetDataSize);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "信息读取: 步骤2_读磁道2: MsRead(%d) fail, ErrCode: %d, Return: %s.",
                stReadIn.lOtherParam[0], nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            g_devImpl.CleanError(); // 清错
            return ConvertErrorCode(nRet);
        }
        stReadOut.wInMode = READFORM_TRACK2;
    }

    // 读磁道3
    if (stReadIn.wInMode == READFORM_TRACK3)
    {
        nRet = g_devImpl.MsRead(/*MAG_DIN_ISO*/stReadIn.lOtherParam[1],
                                stReadOut.szRetData, stReadOut.dwRetDataSize);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "信息读取: 步骤2_读磁道3: MsRead(%d) fail, ErrCode: %d, Return: %s.",
                stReadIn.lOtherParam[1], nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            g_devImpl.CleanError(); // 清错
            return ConvertErrorCode(nRet);
        }
        stReadOut.wInMode = READFORM_TRACK3;
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    SCANCTRL stScanCtrl;
    SCANIMAGECTRL stFCtrl, stBCtrl;
    LPSCANIMAGECTRL stFrontCtrl, stBackCtrl;


    stBackCtrl = &stBCtrl;

    stScanCtrl.dpi = stImageIn.lOtherParam[0];      // 扫描分辨率控制
    stScanCtrl.ejectMode = 0;

    // 扫描正面图
    if ((stImageIn.wInMode & IMAGE_MODE_FRONT) == IMAGE_MODE_FRONT)
    {
        stFrontCtrl = &stFCtrl;
        stFCtrl.cisColorMode = stImageIn.lOtherParam[1];            // 扫描光调色模式
        stFCtrl.grayMode = stImageIn.lOtherParam[2];                // 扫描灰度模式
        stFCtrl.brightness = stImageIn.lOtherParam[3];              // 扫描亮度
        stFCtrl.thresholdLevel = stImageIn.lOtherParam[4];          // 扫描黑白包容度
        stFCtrl.scanDirection = stImageIn.lOtherParam[5];           // 正像
        stFCtrl.origin.x = 0;                                       // 扫描范围全为0
        stFCtrl.origin.y = 0;
        stFCtrl.end.x = 0;
        stFCtrl.end.y = 0;
        stFCtrl.saveImagePath = stImageIn.szImageFrontPath;         // 图像路径

        // 根据入参重设扫描灰度模式
        /*switch(stImageIn.wColorFormat[0])
        {
            case IMAGE_COLOR_BIN: stFCtrl.grayMode = 2; break;
            case IMAGE_COLOR_GARY: stFCtrl.grayMode = 0; break;
            case IMAGE_COLOR_FULL: stFCtrl.grayMode = 2; break;
            default: stFCtrl.grayMode = 2; break;
        }*/
    }

    // 扫描背面图
    if ((stImageIn.wInMode & IMAGE_MODE_BACK) == IMAGE_MODE_BACK)
    {
        stBackCtrl = &stBCtrl;
        stBCtrl.cisColorMode = stImageIn.lOtherParam[1];            // 扫描光调色模式
        stBCtrl.grayMode = stImageIn.lOtherParam[2];                // 扫描灰度模式
        stBCtrl.brightness = stImageIn.lOtherParam[3];              // 扫描亮度
        stBCtrl.thresholdLevel = stImageIn.lOtherParam[4];          // 扫描黑白包容度
        stBCtrl.scanDirection = stImageIn.lOtherParam[5];           // 正像
        stBCtrl.origin.x = 0;                                       // 扫描范围全为0
        stBCtrl.origin.y = 0;
        stBCtrl.end.x = 0;
        stBCtrl.end.y = 0;
        stBCtrl.saveImagePath = stImageIn.szImageBackPath;         // 图像路径


        // 根据入参重设扫描灰度模式
        /*switch(stImageIn.wColorFormat[1])
        {
            case IMAGE_COLOR_BIN: stBCtrl.grayMode = 2; break;
            case IMAGE_COLOR_GARY: stBCtrl.grayMode = 0; break;
            case IMAGE_COLOR_FULL: stBCtrl.grayMode = 2; break;
            default: stBCtrl.grayMode = 2; break;
        }*/
    }

    // 扫描图像
    nRet = g_devImpl.Scan(stScanCtrl, stFrontCtrl, stBackCtrl);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "扫描图像: Scan() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        g_devImpl.CleanError(); // 清错
        return ConvertErrorCode(nRet);
    }
    g_devImpl.CleanError(); // 清错

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    DWORD dwSupAction = (MEDIA_CTR_EJECT | MEDIA_CTR_RETRACT | MEDIA_CTR_FLUSH |
                         MEDIA_CTR_PARTIALCUT);

    if ((enMediaAct & dwSupAction) != 0)
    {
        if ((enMediaAct & MEDIA_CTR_EJECT) == MEDIA_CTR_EJECT)    // 退出介质
        {
            nRet = g_devImpl.MediaEject(MEDIA_EJECT_FRONT);
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__, "退出介质: MediaEject(%d) fail, ErrCode: %d, Return: %s.",
                    MEDIA_EJECT_FRONT, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
        if ((enMediaAct & MEDIA_CTR_RETRACT) == MEDIA_CTR_RETRACT)  // 回收介质
        {
            // 介质吸入
            nRet = g_devImpl.MediaInsert();
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__, "回收步骤1_介质吸入内部: MediaInsert() fail, ErrCode: %d, Return: %s.",
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }

            nRet = g_devImpl.MediaEject(MEDIA_EJECT_AFTER);
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__, "回收步骤2_介质向后排出: MediaEject(%d) fail, ErrCode: %d, Return: %s.",
                    MEDIA_EJECT_AFTER, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }

        if ((enMediaAct & MEDIA_CTR_FLUSH) == MEDIA_CTR_FLUSH)  // 刷新缓存
        {
            nRet = g_devImpl.FlushBuffer();
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__, "刷新缓存: FlushBuffer() fail, ErrCode: %d, Return: %s.",
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }

        if ((enMediaAct & MEDIA_CTR_PARTIALCUT) == MEDIA_CTR_PARTIALCUT)  // 介质吸入
        {
            /*nRet = g_devImpl.MediaInsert(0, usParam);
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__, "介质吸入: MediaInsert(0, %d) fail, ErrCode: %d, Return: %s.",
                    usParam, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }*/

            return ConvertErrorCode(g_devImpl.MediaInsert(0, usParam));
        }
    } else
    {
        Log(ThisModule, __LINE__, "介质控制: 入参[%d]不支持, 不执行, Return: %s.",
            dwSupAction, ConvertErrCodeToStr(PTR_SUCCESS));
        return ConvertErrorCode(PTR_SUCCESS);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::SetData(void *vData, WORD wDataType)
{    
    switch(wDataType)
    {
        case DTYPE_LIB_PATH:    // 设置Lib路径
        {
            g_devImpl.SetLibPath((LPCSTR)vData);
            break;
        }
        case SET_DEV_RECON:     // 设置断线重连标记为T
        {
            return g_devImpl.SetReConFlag(TRUE);
            break;
        }
        case SET_DEV_OPENMODE:  // 设置设备打开模式
        {
            g_devImpl.SetOpenMode(((LPSTDEVOPENMODE)vData)->szOpenParam,
                                  ((LPSTDEVOPENMODE)vData)->nOpenParam);
            break;
        }
        case DTYPE_SET_PRTFONT: // 设置打印字体
        {
            SetPrintFont(*((LPDEVPTRFONTPAR)vData));
            break;
        }
        case SET_DEV_BEEP:      // 设置鸣响
        {
            if (((LPSTCONFIGBEEP)vData)->wSupp == 1)
            {
                g_devImpl.SetBeep(((LPSTCONFIGBEEP)vData)->wCount, ((LPSTCONFIGBEEP)vData)->wInterval);
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
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
int CDevPPR_MB2::GetData(void *vData, WORD wDataType)
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
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
void CDevPPR_MB2::GetVersion(char* szVer, long lSize, ushort usType)
{
    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_DEV:    // DevRPR版本号
                break;
            case GET_VER_FW:        // 固件版本号
                g_devImpl.GetFWVersion(szVer, (ULONG*)(&lSize));
                break;
            default:
                break;
        }
    }
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_MB2.CPP                                           **
** ClassName    : CDevPPR_MB2                                               **
** Symbol       : CDevPPR_MB2::                                             **
** Function     : SetData()                                                 **
** Describtion  : 设置打印字体                                                **
** Parameter    : 输入: lSize: 应答缓存size                                   **
**                     usType: 获取类别                                      **
**                输出: szVer: 应答缓存                                       **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
INT CDevPPR_MB2::SetPrintFont(DEVPTRFONTPAR stFontPar)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 打印前属性初始化
    g_devImpl.SetPtrProperty(printType, Draft);                 // 打印质量:草稿
    g_devImpl.SetPtrProperty(fontType, CPI17point1);            // 打印字距:17.1CPI
    g_devImpl.SetPtrProperty(Ext_type, CancelDH);               // 扩展属性:取消倍高
    g_devImpl.SetPtrProperty(Ext_type, CancelDW);               // 扩展属性:取消倍宽
    g_devImpl.SetPtrProperty(Ext_type, CancelTriH);             // 扩展属性:取消3倍高
    g_devImpl.SetPtrProperty(Ext_type, CancelTriW);             // 扩展属性:取消3倍宽
    g_devImpl.SetPtrProperty(Ext_type, CancelBlack);            // 扩展属性:取消黑体
    g_devImpl.SetPtrProperty(Ext_type, Cancel_up_underline);    // 扩展属性:取消上下划线
    g_devImpl.SetPtrProperty(Ext_type, CancelUnderline);        // 扩展属性:取消下划线
    g_devImpl.SetPtrProperty(Ext_type, Cancel_upuder_sign);     // 扩展属性:取消上下标

    // 打印前属性设置:打印质量
    if (stFontPar.dwMarkPar[0] < 5 && stFontPar.dwMarkPar[0] != Draft)
    {
        g_devImpl.SetPtrProperty(printType, stFontPar.dwMarkPar[0]);
    }

    // 打印前属性设置:打印字距
    switch(stFontPar.dwMarkPar[1])
    {
        case 10: g_devImpl.SetPtrProperty(fontType, CPI10); break;
        case 12: g_devImpl.SetPtrProperty(fontType, CPI12); break;
        case 13: g_devImpl.SetPtrProperty(fontType, CPI13point3); break;
        case 15: g_devImpl.SetPtrProperty(fontType, CPI15); break;
        case 16: g_devImpl.SetPtrProperty(fontType, CPI16point6); break;
        case 18: g_devImpl.SetPtrProperty(fontType, CPI18); break;
    }

    // 打印前属性设置:扩展属性
    if ((stFontPar.dwMarkPar[2] & FONT_BOLD) == FONT_BOLD)               // 黑体(粗体)
    {
        g_devImpl.SetPtrProperty(Ext_type, Black);
    }
    if ((stFontPar.dwMarkPar[2] & FONT_DOUBLE) == FONT_DOUBLE)           // 2倍宽
    {
        g_devImpl.SetPtrProperty(Ext_type, DW);
    }
    if ((stFontPar.dwMarkPar[2] & FONT_TRIPLE) == FONT_TRIPLE)           // 3倍宽
    {
        g_devImpl.SetPtrProperty(Ext_type, TriW);
    }
    if ((stFontPar.dwMarkPar[2] & FONT_DOUBLEHIGH) == FONT_DOUBLEHIGH)   // 2倍高
    {
        g_devImpl.SetPtrProperty(Ext_type, DH);
    }
    if ((stFontPar.dwMarkPar[2] & FONT_TRIPLEHIGH) == FONT_TRIPLEHIGH)   // 3倍高
    {
        g_devImpl.SetPtrProperty(Ext_type, TriH);
    }
    if ((stFontPar.dwMarkPar[2] & FONT_UNDER) == FONT_UNDER)             // 一条下划线
    {
        g_devImpl.SetPtrProperty(Ext_type, underline);
    }
    if ((stFontPar.dwMarkPar[2] & FONT_DOUBLESTRIKE) == FONT_DOUBLESTRIKE)// 双线(上下划线)
    {
        g_devImpl.SetPtrProperty(Ext_type, up_underline);
    }
    if ((stFontPar.dwMarkPar[2] & FONT_SUPERSCRIPT) == FONT_SUPERSCRIPT)// 上标
    {
        g_devImpl.SetPtrProperty(Ext_type, up_sign);
    }
    if ((stFontPar.dwMarkPar[2] & FONT_SUBSCRIPT) == FONT_SUBSCRIPT)    // 下标
    {
        g_devImpl.SetPtrProperty(Ext_type, under_sign);
    }

    return PTR_SUCCESS;
}

// 转换为IDevPTR返回码/错误码
INT CDevPPR_MB2::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        // >0: Impl处理返回
        case IMP_SUCCESS:                   // 0:成功, ERR_SUCCESS/IMP_DEV_ERR_SUCCESS
            return PTR_SUCCESS;
        case IMP_ERR_LOAD_LIB:              // 1:动态库加载失败
            return ERR_PTR_OTHER;
        case IMP_ERR_PARAM_INVALID:         // 2:参数无效
            return ERR_PTR_PARAM_ERR;
        case IMP_ERR_UNKNOWN:               // 3:未知错误
            return ERR_PTR_OTHER;
        case IMP_ERR_NOTOPEN:               // 4:设备未Open
            return ERR_PTR_NOT_OPEN;
        // < 0: Device返回
        // 用户调用不正确，报错
        case IMP_ERR_DEV_UNKNOW:            // -1:未知错误
            return ERR_PTR_OTHER;
        case IMP_ERR_DEV_PARAM_INVALID:     // -2:参数无效
            return ERR_PTR_PARAM_ERR;
        case IMP_ERR_DEV_NO_OPEN:           // -3:未调用过OPEN函数/调用不成功
            return ERR_PTR_NOT_OPEN;
        case IMP_ERR_DEV_NOT_OLI_EMULATION: // -4:未调用过OPEN函数/调用不成功
            return ERR_PTR_NOT_OPEN;
        // 设备创建和不支持，报错
        case IMP_ERR_DEV_CREATE_FAILURE:    // -11:创建设备失败
            return ERR_PTR_NOT_OPEN;
        case IMP_ERR_DEV_NO_SUPPORT:        // -12:设备不支持
            return ERR_PTR_NOT_OPEN;
        // 系统通信，报错
        case IMP_ERR_COMM_OPEN:             // -21:通信打开出错
            return ERR_PTR_COMM_ERR;
        case IMP_ERR_COMM_WRITE:            // -22:通信写出错
            return ERR_PTR_DATA_ERR;
        case IMP_ERR_DEV_COMM_READ:         // -23:通信读出错
            return ERR_PTR_DATA_ERR;
        case IMP_ERR_DEV_COMM_PARITY:       // -24:通信校验错
            return ERR_PTR_DATA_ERR;
        case IMP_ERR_DEV_TIMEOUT:           // -25:通信超时
            return ERR_PTR_TIMEOUT;
        case IMP_ERR_DEV_COMM_OFF:          // -26:设备离线
            return ERR_PTR_OFFLINE;
        // 硬件报错
        case IMP_ERR_DEV_MEDIA_NOFOUND:     // -31:无纸
            return ERR_PTR_NO_MEDIA;
        case IMP_ERR_DEV_MS_BLANK:          // -32:空白磁条
            return ERR_PTR_MS_BLANK;
        case IMP_ERR_DEV_MS_READORPARITY:   // -33:磁条读或校验出错
            return ERR_PTR_DATA_ERR;
        case IMP_ERR_DEV_OFFLINE:           // 设备断电/断线
        case IMP_ERR_DEV_OFFLINE2:          // 设备断电/断线
            return ERR_PTR_NOT_OPEN;
        default:
            return ERR_PTR_OTHER;
    }
}

