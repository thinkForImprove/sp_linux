/***************************************************************
* 文件名称：DevPPR_PRM.cpp
* 文件描述：PRM存折打印模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevPPR_PRM.h"
#include "data_convertor.h"
#include <QTextCodec>

// 版本号
//BYTE  byVRTU[17]  = "DevPPR01000000";

static const char *ThisFile = "DevPPR_PRM.cpp";

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

CDevPPR_PRM::CDevPPR_PRM(LPCSTR lpDevType)
    : g_devImpl(LOG_NAME_DEV, (LPSTR)lpDevType)
{
     SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件
}

CDevPPR_PRM::~CDevPPR_PRM()
{

}

void CDevPPR_PRM::Release()
{
    return;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;
    if ((nRet = g_devImpl.DeviceOpen((LPSTR)pMode)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
** Function     : Close()                                                   **
** Describtion  : 关闭设备　　　　                                             **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_PRM::Close()
{
    g_devImpl.DeviceClose();
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备初始化                                                 **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_PRM::Init()
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备复位(Reset)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_PRM::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    if ((nRet = g_devImpl.Reset(INIT_NOACTION)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "复位: 执行无动作复位: Reset(%d) fail, ErrCode: %d, Return: %s",
           INIT_NOACTION, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备复位(有参数)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_PRM::ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)
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
    if (enMediaAct == MEDIA_CTR_EJECT)  // 退折
    {
        if ((nRet = g_devImpl.Reset(INIT_EJECT)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "复位Ex: 执行退折处理: Reset(%s) fail, ErrCode: %d, Return: %s",
                INIT_EJECT, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (enMediaAct == MEDIA_CTR_RETRACT)// 回收
    {
        if ((nRet = g_devImpl.Reset(INIT_EJECT)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "复位Ex: 执行会输处理: Reset(%s) fail, ErrCode: %d, Return: %s",
                INIT_RETRACT, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        if ((nRet = g_devImpl.Reset(INIT_NOACTION)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "复位Ex: 执行无动作复位: Reset(%d) fail, ErrCode: %d, Return: %s",
               INIT_NOACTION, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
** Function     : SetPrintFormat()                                          **
** Describtion  : 设置当前打印格式                                            **
** Parameter    : stPrintFormat: 定义当前打印的格式具体内容                     **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevPPR_PRM::GetStatus(DEVPTRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);

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
    nRet = g_devImpl.GetDevStatus();
    if (nRet < 0)
    {
        if (nRet == IMP_ERR_NOT_OPEN || nRet == IMP_ERR_USB_OPEN ||
            nRet == IMP_ERR_CONNECT)              // 端口打开失败
        {
            stStatus.wDevice = DEV_STAT_OFFLINE;
            return ERR_PTR_NOT_OPEN;
        } else        
        {
            if (nRet == IMP_ERR_GETDEV_STAT)
            {
                // PRM 断电/断线时返回-22,计数3次之后为断线
                if (m_wGetStatErrCount >= 3)
                {
                    stStatus.wDevice = DEV_STAT_OFFLINE;
                    m_wGetStatErrCount = 0;
                } else
                {
                    m_wGetStatErrCount ++;
                    stStatus.wDevice = DEV_STAT_HWERROR;
                    return ERR_PTR_HWERR;
                }
            } else
            {
                stStatus.wDevice = DEV_STAT_HWERROR;
                return ERR_PTR_HWERR;
            }

        }
    }

    switch(nRet)
    {
        case STATUS_NOMAL:                      // 1:状态正常
        case STATUS_MEDIA_PREENTERING:          // 2:介质存在、位于入口
        case STATUS_MEDIA_PRESENT:              // 4:介质已插入
        case STATUS_MAGNETIC_UNREAD:            // 13:磁条不能读出
        case STATUS_MS_BLANK:                   // 14:空白磁条
        case STATUS_MSR_SUCCESS:                // 15:读存折成功,打印机状态正常
        case STATUS_PAPER_NOHAVE:               // 10:缺纸/无纸
        case STATUS_MEDIA_JAMMED:               // 9:介质阻塞(会返回该值,与实际不准)
            stStatus.wDevice = DEV_STAT_ONLINE;
            break;
        case STATUS_COVEROPEN_OFFLINE:          // 3:脱机或开盖
        case STATUS_OFFLINE:                    // 6:脱机
        case STATUS_COVER_OPEN:                 // 7:开盖
        case STATUS_COMMAND_WRONG:              // 8:命令错
        case STATUS_COVER_OPEN2:                // 11:开盖
        case STATUS_COMMAND_WRONG3:             // 12:命令错
        case STATUS_COMM_ERROR:                 // 5:通信错
            stStatus.wDevice = DEV_STAT_HWERROR;
            break;
    }

    if (stStatus.wDevice == DEV_STAT_ONLINE)
    {
        // 获取介质状态
        nRet = g_devImpl.GetMediaStatus();
        if (nRet < 0)
        {
            if (nRet == IMP_ERR_NOT_OPEN || nRet == IMP_ERR_USB_OPEN ||
                nRet == IMP_ERR_CONNECT)              // 端口打开失败
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
            case STATUS_MEDIA_NONE:                 // 无介质
                stStatus.wDevice = DEV_STAT_ONLINE;
                stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
                break;
            case STATUS_MEDIA_PREENTERING:          // 打印介质存在
            case STATUS_MEDIA_ENTERING:             // 介质已对齐
                stStatus.wDevice = DEV_STAT_ONLINE;
                stStatus.wMedia = MEDIA_STAT_ENTERING;
                //stStatus.wPaper[0] = PAPER_STAT_OUT;
                break;
            case STATUS_MEDIA_PRESENT:              // 打印介质被全部插入/打印介质已插入机内
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
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::PrintData(const char *pStr, unsigned long ulDataLen)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 注: 入参坐标单位为0.1MM,PRM使用单位为MM

    Log(ThisModule, __LINE__, "打印文本: X=%dMM|Y=%dMM|L=%d|D=%s|",
        0, 50, ulDataLen, pStr);

    if (ulDataLen < 1)
    {
        return PTR_SUCCESS;
    }

    // 入参数据为GBK，PRM使用UTF8编码, 需做转换

    // 字符集转换(GBK->UTF8)
    std::string stdUData = "";
    QTextCodec *codec = QTextCodec::codecForLocale();       // 取当前字符集
    QTextCodec *qtUTF8 = QTextCodec::codecForName("UTF-8"); // UTF8字符集
    QTextCodec *qtGBK = QTextCodec::codecForName("GB18030");// GBK字符集
    QTextCodec::setCodecForLocale(qtGBK);                   // 设置当前为GBK字符集
    QString qsText = QString::fromLocal8Bit((LPSTR)pStr);   // 导入GBK数据
    QTextCodec::setCodecForLocale(qtUTF8);                  // 设置当前为UTF8字符集
    QByteArray tmpData = qsText.toUtf8();                   // 转换为UTF8格式数据
    QTextCodec::setCodecForLocale(codec);                   // 还原当前字符集
    stdUData.append(tmpData.data());

    // 打印文本
    nRet = g_devImpl.PrintText(0, 5, (LPSTR)stdUData.c_str(), stdUData.length());
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印文本: PrintText(%d, %dMM, %s, %d) fail, ErrCode: %d, Return: %s.",
            0, 5, (LPSTR)stdUData.c_str(), stdUData.length(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::PrintDataOrg(const char *pStr, unsigned long ulDataLen,
                              unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 注: 入参坐标单位为0.1MM,PRM使用单位为MM

    Log(ThisModule, __LINE__, "打印文本: X=%d(%d)|Y=%d(%d)|L=%d|D=%s|",
        ulOrgX, ulOrgX / 10, ulOrgY, ulOrgY / 10, ulDataLen, pStr);

    if (ulDataLen < 1)
    {
        return PTR_SUCCESS;
    }

    // 入参数据为GBK，PRM使用UTF8编码, 需做转换

    // 字符集转换(GBK->UTF8)
    std::string stdUData = "";
    QTextCodec *codec = QTextCodec::codecForLocale();       // 取当前字符集
    QTextCodec *qtUTF8 = QTextCodec::codecForName("UTF-8"); // UTF8字符集
    QTextCodec *qtGBK = QTextCodec::codecForName("GB18030");// GBK字符集
    QTextCodec::setCodecForLocale(qtGBK);                   // 设置当前为GBK字符集
    QString qsText = QString::fromLocal8Bit((LPSTR)pStr);   // 导入GBK数据
    QTextCodec::setCodecForLocale(qtUTF8);                  // 设置当前为UTF8字符集
    QByteArray tmpData = qsText.toUtf8();                   // 转换为UTF8格式数据
    QTextCodec::setCodecForLocale(codec);                   // 还原当前字符集
    stdUData.append(tmpData.data());

    // 打印文本
    nRet = g_devImpl.PrintText(ulOrgX / 10, ulOrgY / 10, (LPSTR)stdUData.c_str(), stdUData.length());
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印文本: PrintText(%d, %d, %s, %d) fail, ErrCode: %d, Return: %s.",
            ulOrgX / 10, ulOrgY / 10, (LPSTR)stdUData.c_str(), stdUData.length(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    INT nRet = PTR_SUCCESS;

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 注: 入参坐标单位为0.1MM,PRM使用单位为MM

    Log(ThisModule, __LINE__, "打印图片: X=%d(%d)|Y=%d(%d)|D=%s|",
        ulOrgX, ulOrgX / 10, ulOrgY, ulOrgY / 10, szImagePath);

    // 打印图片
    nRet = g_devImpl.PrintBmp(ulOrgX / 10, ulOrgY / 10, (LPSTR)szImagePath, FALSE);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印图片: PrintBmp(%d, %d, %s) fail, ErrCode: %d, Return: %s.",
            ulOrgX / 10, ulOrgY / 10, szImagePath, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::PrintForm(DEVPRINTFORMIN stPrintIn, DEVPRINTFORMOUT &stPrintOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 注: 入参坐标单位为0.1MM,PRM使用单位为MM
    DWORD dwX = stPrintIn.ulX / 10;
    DWORD dwY = stPrintIn.ulY / 10;

    if (stPrintIn.wInMode == PRINTFORM_TEXT)
    {

        Log(ThisModule, __LINE__, "打印文本: X=%d(%d)|Y=%d(%d)|L=%d|D=%s|",
            stPrintIn.ulX, dwX, stPrintIn.ulX, dwY,
            stPrintIn.dwDataSize, stPrintIn.lpData);

        if (strlen(stPrintIn.lpData) < 1)
        {
            return PTR_SUCCESS;
        }

        // 入参数据为GBK，PRM使用UTF8编码, 需做转换

        // 字符集转换(GBK->UTF8)
        std::string stdUData = "";
        QTextCodec *codec = QTextCodec::codecForLocale();       // 取当前字符集
        QTextCodec *qtUTF8 = QTextCodec::codecForName("UTF-8"); // UTF8字符集
        QTextCodec *qtGBK = QTextCodec::codecForName("GB18030");// GBK字符集
        QTextCodec::setCodecForLocale(qtGBK);                   // 设置当前为GBK字符集
        QString qsText = QString::fromLocal8Bit((LPSTR)stPrintIn.lpData); // 导入GBK数据
        QTextCodec::setCodecForLocale(qtUTF8);                  // 设置当前为UTF8字符集
        QByteArray tmpData = qsText.toUtf8();                   // 转换为UTF8格式数据
        QTextCodec::setCodecForLocale(codec);                   // 还原当前字符集
        stdUData.append(tmpData.data());

        // 打印文本
        nRet = g_devImpl.PrintText(dwX, dwY, (LPSTR)stdUData.c_str(), stdUData.length());
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印文本: PrintText(%d, %d, %s, %d) fail, ErrCode: %d, Return: %s.",
                dwX, dwY, stPrintIn.lpData, stPrintIn.dwDataSize, nRet,
                ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (stPrintIn.wInMode == PRINTFORM_PIC)
    {
        Log(ThisModule, __LINE__, "打印图片: X=%d(%d)|Y=%d(%d)|D=%s|",
            stPrintIn.ulX, dwX, stPrintIn.ulY, dwY, stPrintIn.lpData);

        // 打印图片
        nRet = g_devImpl.PrintBmp(dwX, dwY, stPrintIn.lpData, FALSE);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打印图片: PrintBmp(%d, %d, %s) fail, ErrCode: %d, Return: %s.",
                dwX, dwY, stPrintIn.lpData, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        // 不支持的打印类型
        Log(ThisModule, __LINE__,
            "打印: 不支持的打印类型: stPrintIn.wInMode = %d, Return: %s.",
            stPrintIn.wInMode, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    // 打印接口下发打印命令后,不等待打印完成即返回,需通过检查设备状态来判断
    if (m_stParam.wPrintDataMode == 1)
    {
        DWORD dwTimeOut = stPrintIn.dwTimeOut;  // 超时时间
        DWORD dwTimeCount = 0;                  // 时间计数
        while (1)
        {
            nRet = g_devImpl.GetDevStatus();    // 获取设备状态
            if (nRet == STATUS_NOMAL)
            {
                break;
            } else
            if (nRet < 0 ||
                nRet ==  STATUS_COVEROPEN_OFFLINE ||    // 脱机或开盖
                nRet ==  STATUS_COMM_ERROR        ||    // 传输错误（打印机在接收时发现错误）
                nRet ==  STATUS_OFFLINE           ||    // 脱机
                nRet ==  STATUS_COVER_OPEN        ||    // 开盖
                nRet ==  STATUS_COMMAND_WRONG     ||    // 命令错
                nRet ==  STATUS_MEDIA_JAMMED      ||    // 介质阻塞
                nRet ==  STATUS_PAPER_NOHAVE      ||    // 缺纸/无纸
                nRet ==  STATUS_COVER_OPEN2       ||    // 开盖
                nRet ==  STATUS_COMMAND_WRONG3    )     // 命令错
            {
                Log(ThisModule, __LINE__, "打印文本/图片后去状态: GetDevStatus() fail, ErrCode: %d, Return: %s.",
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            } else
            {
                dwTimeOut += 200;
                continue;
            }

            if (dwTimeOut > 0 && dwTimeCount >= dwTimeOut)
            {
                Log(ThisModule, __LINE__,
                    "打印: 循环获取设备状态超时: Return: %s.",
                    ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    //AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;

    // 读磁道1
    if (stReadIn.wInMode == READFORM_TRACK1)
    {
        stReadOut.wInMode = READFORM_TRACK1;
    }

    // 读磁道2
    if (stReadIn.wInMode == READFORM_TRACK2)
    {
        nRet = g_devImpl.MsRead(MSG_T2, stReadOut.szRetData, sizeof(stReadOut.szRetData));
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "信息读取: 读磁道2: MsRead(%d) fail, ErrCode: %d, Return: %s.",
                MSG_T2, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            g_devImpl.CleanError(); // 清错
            return ConvertErrorCode(nRet);
        }
        stReadOut.wInMode = READFORM_TRACK2;
        stReadOut.dwRetDataSize = strlen(stReadOut.szRetData);
    }

    // 读磁道3
    if (stReadIn.wInMode == READFORM_TRACK3)
    {
        nRet = g_devImpl.MsRead(MSG_T3, stReadOut.szRetData, sizeof(stReadOut.szRetData));
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "信息读取: 读磁道3: MsRead(%d) fail, ErrCode: %d, Return: %s.",
                MSG_T3, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            g_devImpl.CleanError(); // 清错
            return ConvertErrorCode(nRet);
        }
        stReadOut.wInMode = READFORM_TRACK3;
        stReadOut.dwRetDataSize = strlen(stReadOut.szRetData);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
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
            nRet = g_devImpl.MediaEject();
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__, "退出介质: MediaEject() fail, ErrCode: %d, Return: %s.",
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
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

            nRet = g_devImpl.Reset(INIT_RETRACT, m_stParam.wFuncWaitTime);
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__, "回收步骤2_介质向后排出: Reset(%d) fail, ErrCode: %d, Return: %s.",
                    INIT_RETRACT, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }

        if ((enMediaAct & MEDIA_CTR_FLUSH) == MEDIA_CTR_FLUSH)  // 刷新缓存
        {

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
            return ConvertErrorCode(g_devImpl.MediaInsert());
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
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::SetData(void *vData, WORD wDataType)
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
        case SET_DEV_PARAM:     // 设置参数
        {
            memcpy(&m_stParam, (LPSTINICONFIG_PRM)vData, sizeof(STINICONFIG_PRM));
            break;
        }
        case SET_DEV_BEEP:      // 设置鸣响
        {
            if (((LPSTCONFIGBEEP)vData)->wSupp == 1)
            {
                g_devImpl.StartOnceBeep();
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
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
int CDevPPR_PRM::GetData(void *vData, WORD wDataType)
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
void CDevPPR_PRM::GetVersion(char* szVer, long lSize, ushort usType)
{
    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_DEV:    // DevRPR版本号
                break;
            case GET_VER_FW:        // 固件版本号
                g_devImpl.GetFWVersion(szVer, lSize);
                break;
            default:
                break;
        }
    }
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevPPR_PRM.CPP                                           **
** ClassName    : CDevPPR_PRM                                               **
** Symbol       : CDevPPR_PRM::                                             **
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
INT CDevPPR_PRM::SetPrintFont(DEVPTRFONTPAR stFontPar)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_Mutex);

    INT nRet = IMP_SUCCESS;
    WORD wCPI = CPI_12, wBlod = 0;
    WORD wDouWidth = WIDTH_CANCEL, wDouHeight = HEIGHT_CANCEL;
    WORD wUnderLine = NOT_UPUNDER_LINE;                                 // 无上下划线

    // 打印前属性初始化
    g_devImpl.SetLineSpace(0);                                          // 打印行间距0

    // 打印前属性设置:打印字距
    switch(stFontPar.dwMarkPar[0])
    {
        case 10: wCPI = CPI_10; break;
        case 12: wCPI = CPI_12; break;
        case 15: wCPI = CPI_15; break;
        case 16: wCPI = CPI_166; break;
        case 17: wCPI = CPI_171; break;
    }

    // 打印前属性设置:扩展属性
    if ((stFontPar.dwMarkPar[2] & FONT_BOLD) == FONT_BOLD)               // 黑体(粗体)
    {
        wBlod = 1;
    }
    if ((stFontPar.dwMarkPar[2] & FONT_DOUBLE) == FONT_DOUBLE)           // 2倍宽
    {
        wDouWidth = WIDTH_DOUBLE;
    }
    if ((stFontPar.dwMarkPar[2] & FONT_TRIPLE) == FONT_TRIPLE)           // 3倍宽
    {
        wDouWidth = WIDTH_TRIPLE;
    }
    if ((stFontPar.dwMarkPar[2] & FONT_DOUBLEHIGH) == FONT_DOUBLEHIGH)   // 2倍高
    {
        wDouHeight = HEIGHT_DOUBLE;
    }
    if ((stFontPar.dwMarkPar[2] & FONT_TRIPLEHIGH) == FONT_TRIPLEHIGH)   // 3倍高
    {
        //wDouHeight = HEIGHT_TRIPLE;
    }
    if ((stFontPar.dwMarkPar[2] & FONT_UNDER) == FONT_UNDER)             // 一条下划线
    {
        wUnderLine = UNDER_LINE;
    }
    if ((stFontPar.dwMarkPar[2] & FONT_OVERSCORE) == FONT_OVERSCORE)    // 上划线
    {
        wUnderLine = ABOVE_LINE;
    }
    if ((stFontPar.dwMarkPar[2] & FONT_UNDER) == FONT_UNDER &&
        (stFontPar.dwMarkPar[2] & FONT_OVERSCORE) == FONT_OVERSCORE)    // 上下划线
    {
        wUnderLine = UPUNDER_LINE;
    }

    g_devImpl.SetPtrProperty(wCPI, wBlod, wDouWidth, wDouHeight);       // 设置打印格式(字间距/是否加粗/是否倍宽倍高)
    g_devImpl.SetUnderLine(wUnderLine);                                 // 设置有无上下划线

    return PTR_SUCCESS;
}

// 转换为IDevPTR返回码/错误码
INT CDevPPR_PRM::ConvertErrorCode(INT nRet)
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
        case IMP_ERR_USB_OPEN:              // -1:USB打开异常
            return ERR_PTR_USB_ERR;
        case IMP_ERR_CONNECT:               // -2:连接异常
            return ERR_PTR_COMM_ERR;
        case IMP_ERR_SNDCMD:                // -3:发送失败(查询指令发送异常)
            return ERR_PTR_DATA_ERR;
        case IMP_ERR_RCVCMD:                // -4:读取失败
            return ERR_PTR_DATA_ERR;
        case IMP_ERR_ALREAY_OPEN:           // -5:端口已打开
            return ERR_PTR_HWERR;
        case IMP_ERR_NOT_OPEN:              // -6:端口未打开(打开失败)
            return ERR_PTR_NOT_OPEN;
        case IMP_ERR_FILE_NOFOUND:          // -10:文件不存在
            return ERR_PTR_OTHER;
        case IMP_ERR_FILE_OPEN:             // -11:未找到文件或打开失败
            return ERR_PTR_OTHER;
        case IMP_ERR_FILE_READ:             // -12:文件读取失败
            return ERR_PTR_OTHER;
        case IMP_ERR_FILE_LENGTH:           // -13:文件长度获取失败
            return ERR_PTR_OTHER;
        case IMP_ERR_GETMEDIA_STAT:         // -20:获取介质状态失败
            return ERR_PTR_HWERR;
        case ERR_ERR_GETMEDIA_SIZE:         // -21:介质尺寸获取失败
            return ERR_PTR_HWERR;
        case IMP_ERR_GETDEV_STAT:           // -22:获取机器状态失败
            return ERR_PTR_HWERR;
        case IMP_ERR_DEV_TIME:              // -30:等待超时
            return ERR_PTR_TIMEOUT;
        case IMP_ERR_MEM_APPLY:             // -31:内存申请失败
            return ERR_PTR_HWERR;
        case IMP_ERR_PAR_INVALID:           // -32:输入参数无效
            return ERR_PTR_PARAM_ERR;
        case IMP_ERR_DATA_CODESET:          // -60:文本字符编码格式错误
            return ERR_PTR_FORMAT_ERROR;
        case IMP_ERR_IMAGE_NOT_SUPP:        // -70:不支持的图片格式
            return ERR_PTR_IMAGE_ERROR;
        default:
            return ERR_PTR_OTHER;
    }
}

