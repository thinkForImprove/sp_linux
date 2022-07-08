/***************************************************************
* 文件名称：DevPPR_PRM.cpp
* 文件描述：PRM存折打印模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevDSR_BSD216.h"
#include "data_convertor.h"
#include <QTextCodec>

#define ThisFile (basename(__FILE__))

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

CDevDSR_BSD216::CDevDSR_BSD216(LPCSTR lpDevType) : g_devImpl(LOG_NAME_DEV, (LPSTR)lpDevType)
{
     SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件
}

CDevDSR_BSD216::~CDevDSR_BSD216()
{

}

void CDevDSR_BSD216::Release()
{
    return;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
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
int CDevDSR_BSD216::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    if ((nRet = g_devImpl.DeviceOpen()) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
** Function     : Close()                                                   **
** Describtion  : 关闭设备　　　　                                             **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevDSR_BSD216::Close()
{
    g_devImpl.DeviceClose();
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备初始化                                                 **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevDSR_BSD216::Init()
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备复位(Reset)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevDSR_BSD216::Reset()
{
    THISMODULE(__FUNCTION__);

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
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
** Function     : Init()                                                    **
** Describtion  : 设备复位(有参数)                                            **
** Parameter    : 无                                                        **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevDSR_BSD216::ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
** Function     : SetPrintFormat()                                          **
** Describtion  : 设置当前打印格式                                            **
** Parameter    : stPrintFormat: 定义当前打印的格式具体内容                     **
** Return       : 见返回错误值定义                                            **
** Note         :                                                           **
** Date         : 2020/10/20                                                **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
int CDevDSR_BSD216::GetStatus(DEVPTRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    // 设备状态初始化
    stStatus.wDevice = DEV_STAT_OFFLINE;
    stStatus.wMedia = MEDIA_STAT_UNKNOWN;
    memset(stStatus.wPaper, PAPER_STAT_UNKNOWN, sizeof(stStatus.wPaper));
    stStatus.wInk = INK_STAT_NOTSUPP;
    stStatus.wToner = TONER_STAT_NOTSUPP;

    // 获取设备状态
    //usleep(500 * 1000);
    DEVICESTATUS ds;
    memset(&ds, 0, sizeof(DEVICESTATUS));
    nRet = g_devImpl.GetDevStatus(&ds);
    if (nRet != 0)
    {
        stStatus.wDevice = DEV_STAT_OFFLINE;
        return ERR_PTR_NOT_OPEN;
    }

    stStatus.wDevice = DEV_STAT_ONLINE; // init to online
    switch(nRet)
    {        
        case SNBC_MEMORY_ERROR:
        case SNBC_COVER_ERROR:
        case SNBC_HARDWARE_ERROR:
        case SNBC_COMMUNICATION_ERROR:
            stStatus.wDevice = DEV_STAT_HWERROR;
            break;
    }

    if (stStatus.wDevice == DEV_STAT_ONLINE)
    {        
        if (ds.bRearSensorHavePaper || ds.bFrontSensorHavePaper)
        {
            stStatus.wMedia = MEDIA_STAT_PRESENT;
        }
        else
        {
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
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

int CDevDSR_BSD216::PrintData(const char *pStr, unsigned long ulDataLen)
{
    THISMODULE(__FUNCTION__);
    return PTR_SUCCESS;
}

int CDevDSR_BSD216::PrintDataOrg(const char *pStr, unsigned long ulDataLen,
                              unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    return PTR_SUCCESS;
}

int CDevDSR_BSD216::PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    INT nRet = PTR_SUCCESS;
    return PTR_SUCCESS;
}

int CDevDSR_BSD216::PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    return PTR_SUCCESS;
}

int CDevDSR_BSD216::PrintForm(DEVPRINTFORMIN stPrintIn, DEVPRINTFORMOUT &stPrintOut)
{
    THISMODULE(__FUNCTION__);
    return PTR_SUCCESS;
}

int CDevDSR_BSD216::ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut)
{
    THISMODULE(__FUNCTION__);
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
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
int CDevDSR_BSD216::ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    if ((nRet = g_devImpl.ImplScanImage(stImageIn, stImageOut)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "文件扫描: ReadImage fail, ErrCode: %d, Return: %s",
           nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
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
int CDevDSR_BSD216::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    DWORD dwSupAction = MEDIA_CTR_EJECT;

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
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
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
int CDevDSR_BSD216::SetData(void *vData, WORD wDataType)
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
        case SET_DEV_PARAM:     // 设置参数
        {
            break;
        }
        default:
            break;
    }

    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
** Function     : SetData()                                                 **
** Describtion  : 获取                                                      **
** Parameter    : 输入: vData: 获取数据                                       **
**                     wDataType: 获取类别                                   **
**                输出: 无                                                  **
** Note       : DSR Device not support, just return sucess                 **
** Date         : 2022/2/17                                                **
*****************************************************************************/
int CDevDSR_BSD216::GetData(void *vData, WORD wDataType)
{
    return PTR_SUCCESS;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : CDevDSR_BSD216.CPP                                           **
** ClassName    : CDevDSR_BSD216                                               **
** Symbol       : CDevDSR_BSD216::                                             **
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
void CDevDSR_BSD216::GetVersion(char* szVer, long lSize, ushort usType)
{
    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_DEV:    // DevDSR版本号
                memcpy(szVer, byVRTU_DEV,
                       (strlen((char*)byVRTU_DEV) >= lSize ? lSize -1 : strlen((char*)byVRTU_DEV)));
                break;
            case GET_VER_FW:        // 固件版本号
                g_devImpl.GetFWVersion(szVer, lSize);
                break;
            default:
                break;
        }
    }
}

// 转换为IDevPTR返回码/错误码
INT CDevDSR_BSD216::ConvertErrorCode(INT nRet)
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


        case SNBC_ERROR_UNKNOWN:
            return ERR_PTR_HWERR;
        case SNBC_MEMORY_ERROR:
            return ERR_PTR_HWERR;
        case SNBC_PARA_ERROR:
            return ERR_PTR_PARAM_ERR;
        case SNBC_COMMUNICATION_ERROR:
            return ERR_PTR_COMM_ERR;
        default:
            return ERR_PTR_OTHER;
    }
}

CHAR* CDevDSR_BSD216::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case PTR_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "操作成功");
            return m_szErrStr;
        case ERR_PTR_CANCEL:
            sprintf(m_szErrStr, "%d|%s", nRet, "命令取消");
            return m_szErrStr;
        case ERR_PTR_PARAM_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "参数错误");
            return m_szErrStr;
        case ERR_PTR_COMM_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "通讯错误");
            return m_szErrStr;
        case ERR_PTR_JAMMED:
            sprintf(m_szErrStr, "%d|%s", nRet, "堵纸等机械故障");
            return m_szErrStr;
        case ERR_PTR_NOT_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备没有打开");
            return m_szErrStr;
        case ERR_PTR_NO_RESUME:
            sprintf(m_szErrStr, "%d|%s", nRet, "不可恢复的错误");
            return m_szErrStr;
        case ERR_PTR_CAN_RESUME:
            sprintf(m_szErrStr, "%d|%s", nRet, "可恢复的错误");
            return m_szErrStr;
        case ERR_PTR_HWERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "硬件故障");
            return m_szErrStr;
        case ERR_PTR_NO_DEVICE:
            sprintf(m_szErrStr, "%d|%s", nRet, "指定名的设备不存在");
            return m_szErrStr;
        case ERR_PTR_UNSUP_CMD:
            sprintf(m_szErrStr, "%d|%s", nRet, "不支持的指令");
            return m_szErrStr;
        case ERR_PTR_DATA_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "收发数据错误");
            return m_szErrStr;
        case ERR_PTR_TIMEOUT:
            sprintf(m_szErrStr, "%d|%s", nRet, "超时");
            return m_szErrStr;
        case ERR_PTR_DRVHND_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "驱动错误");
            return m_szErrStr;
        case ERR_PTR_DRVHND_REMOVE:
            sprintf(m_szErrStr, "%d|%s", nRet, "驱动丢失");
            return m_szErrStr;
        case ERR_PTR_USB_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "USB/COM/连接错误");
            return m_szErrStr;
        case ERR_PTR_DEVBUSY:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备忙");
            return m_szErrStr;
        case ERR_PTR_OTHER:
            sprintf(m_szErrStr, "%d|%s", nRet, "其它错误，如调用API错误等");
            return m_szErrStr;
        case ERR_PTR_DEVUNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备未知");
            return m_szErrStr;
        case ERR_PTR_NOMEDIA:
            sprintf(m_szErrStr, "%d|%s", nRet, "指定位置无介质");
            return m_szErrStr;
        case ERR_PTR_HAVEMEDIA:
            sprintf(m_szErrStr, "%d|%s", nRet, "指定位置有介质");
            return m_szErrStr;
        case ERR_PTR_PAPER_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "介质异常");
            return m_szErrStr;
        case ERR_PTR_SCAN_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "扫描失败");
            return m_szErrStr;
        case ERR_PTR_DATA_DISCERN:
            sprintf(m_szErrStr, "%d|%s", nRet, "数据识别失败");
            return m_szErrStr;
        case ERR_PTR_NO_MEDIA:
            sprintf(m_szErrStr, "%d|%s", nRet, "通道无纸");
            return m_szErrStr;
        case ERR_PTR_OFFLINE:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备断线");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
            return m_szErrStr;
    }
}





