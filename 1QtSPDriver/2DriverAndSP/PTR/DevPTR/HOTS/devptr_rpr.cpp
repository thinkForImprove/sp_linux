/*****************************************************************************
** FileName     : DevPTR_RPR.CPP                                            **
** Describtion  : 打印处理                                                  **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      : 2019/7/8 RPR01000000 JiaoChong(CFES) Create               **
*****************************************************************************/

#include "devptr_rpr.h"
#include "rpr_usb.h"
#include <QTextCodec>
extern RPR_USB *pRPR_USB;

extern DevPTR_RPR *devPTR;

// 字体文件路径
//#define     FT_HEITI               "/usr/share/fonts/simhei/simhei.ttf"
//#define     FT_HEITI               "/usr/share/fonts/truetype/noto/NotoMono-Regular.ttf"
//#define     FT_HEITI               "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc"
char FT_HEITI[256] =  "/usr/share/fonts/simhei/simhei.ttf";

//IPrinterDevice g_iPrinterDevice;

// 毫米*10 转换为 打印机计算单位(0.125mm) 及 反转换
#define MM2HS(M)  ( M == 0 ? 0 : (WORD)(M / 1.25))
#define HS2MM(H)  ((WORD)(H * 1.25))

// 获得 截取 物理左/上边距后的值
// 缺省：物理左边距3.75mm，物理上边距110mm
// 算法：IF 入参 <= 3.75mm 则 ret 0; ELSE ret 入参 - 物理左边距
//       IF 入参 <= 110mm 则 ret 0; ELSE ret 入参 - 物理上边距
#define GETLEFT(L) (L <= DEF_LEFTDIST_MM ? 0 : L - DEF_LEFTDIST_MM)
#define GETTOP(T) (T <= DEF_TOPDIST_MM ? 0 : T - DEF_TOPDIST_MM)
#define GETTOP2(T) (T <= DEF_TOPDIST_MM ? 0 : T - DEF_TOPDIST_MM2)

// 版本号
BYTE  byVRTU_HOTS[17]  = "DevPTR01000000";

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
//long DEVPTR_EXPORT CreateIDevPTR(LPCSTR lpDevType, IDevPTR *&pDev)
//{
//    devPTR = new DevPTR_RPR();
//    pDev = &(devPTR->iPtrDev);
//    return (pDev != nullptr) ? 0 : -1;
//}

/************************************************************
** 功能：结束设备连接handle
** 输入：无
** 输出：无
** 返回：无
************************************************************/
void ExitIDevPTR()
{
    delete devPTR;
}

long IPrinterDevice::Release()
{
    devPTR->N_Close_Req();
    return 0;
}

/************************************************************
** 功能：打开与设备的连接
** 输入：pMode: 自定义OPEN参数字串,
    串口： 格式为："COMX: BaudRate,Parity,DataBits,StopBits"(例如："COM2:115200,N,8,1")
    USB:  格式自定义
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::Open(const char *pMode)
{
    BOOL bIsUSB = TRUE;

    if (pMode != nullptr && memcmp(pMode, "COM", 3) == 0)
        bIsUSB = FALSE;

    if (bIsUSB == TRUE)  // USB口
    {
        if (devPTR->N_Open_Req() != OK)
        {
            return ERR_PTR_OTHER;
        }
    }
    else
    {
        // COM口连接
        return ERR_PTR_NO_DEVICE;
    }

    return ERR_PTR_SUCCESS;
}

long IPrinterDevice::OpenDev(const unsigned short usMode)
{
    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：关闭与设备的连接
** 输入：无
** 输出：无
** 返回：无
************************************************************/
void IPrinterDevice::Close()
{
    devPTR->N_Close_Req();
    return;
}

/************************************************************
** 功能：设备初始化
** 输入: 无
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::Init()
{
    // 设备初始化
    if (devPTR->N_Reset_Req() != OK)
    {
        return ERR_PTR_OTHER;
    }

    // 参数初始化
    devPTR->N_Init();

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：设置当前打印格式
** 输入：stPrintFormat：定义当前打印的格式具体内容
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::SetPrintFormat(const STPRINTFORMAT &stPrintFormat)
{
    if (devPTR->N_SetPrintFormat_Req(stPrintFormat) != OK)
        return ERR_PTR_OTHER;

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：打印字串(无指定打印坐标)
         打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
** 输入：pStr：要打印的字串
         ulDataLen：数据长度，如果为-1，pStr以\0结束
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::PrintData(const char *pStr, unsigned long ulDataLen)
{
    // 数据为空或数据长度<1,返回参数错误
    if (pStr == nullptr || ulDataLen < 1)
    {
        return ERR_PTR_PARAM_ERR;
    }

    // 转码
    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextCodec *codec1 = QTextCodec::codecForName("gb18030");
    if (nullptr == codec1) codec1 = QTextCodec::codecForName("gb2312");
    if (nullptr == codec1) codec1 = QTextCodec::codecForName("gbk");
    if (nullptr == codec1)
    {
        return ERR_PTR_PARAM_ERR;
    }
    QTextCodec::setCodecForLocale(codec1);
    std::string szData = QString::fromLocal8Bit(PSZ(pStr), ulDataLen).toStdString().c_str();
    QTextCodec::setCodecForLocale(codec);

    //日立打印机使用ANSI打印,需要转换.
    if (devPTR->N_PrintData_Req((LPBYTE)szData.c_str(), szData.size()) != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：图片打印(无指定打印坐标)
** 输入：szImagePath：图片保存路径
         ulWidth：打印宽度
         ulHeight：打印高度
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::PrintImage(const char *szImagePath, ULONG ulWidth, ULONG ulHeight)
{
    // 图片路径为NULL或宽高<1,返回参数错误
    if (szImagePath == nullptr || ulWidth < 1 || ulHeight < 1)
    {
        return ERR_PTR_PARAM_ERR;
    }

    // 图片文件是否存在
    if (access(szImagePath, F_OK) != 0)
    {
        devPTR->N_SetErrCode(szErrPrt_FileNotFound);
        return ERR_PTR_PARAM_ERR;
    }

    // 打印图片
    if (devPTR->N_PrintImage_Req((LPBYTE)szImagePath) != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：指定坐标打印图片
** 输入：szImagePath：图片保存绝对路径
         ulOrgX: 打印图片的列坐标
         ulOrgY: 打印图片的行坐标
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::PrintImageOrg(const char* szImagePath, ULONG ulOrgX, ULONG ulOrgY)
{
    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：切纸
** 输入：bDetectBlackStripe：是否检测黑标
         ulFeedSize，切纸前走纸的长度，单位0.1毫米
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::CutPaper(bool bDetectBlackStripe, unsigned long ulFeedSize)
{
    if (devPTR->N_CutPaper_Req(bDetectBlackStripe, ulFeedSize) != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：主动查询一次设备状态
** 输入：无
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::QueryStatus()
{
    //return 0;
    if (devPTR->N_QueryStatus_Req() != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：得到打印机状态
** 输入：无
** 输出：pPrinterStatus，返回打印机状态
        pPaperStatus，返回纸状态
        pTonerStatus，返回TONER状态
** 返回：见返回错误码定义
************************************************************/
void IPrinterDevice::GetStatus(PaperStatus &PaperStatus, TonerStatus &TonerStatus, OutletStatus &OutletStatus)
{
    devPTR->N_GetStatus_Get(PaperStatus, TonerStatus, OutletStatus);

    return;
}

/************************************************************
** 功能：设置当前打印模式
** 输入：stPrintMode：定义当前打印模式的具体内容
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::SetPrintMode(const STPRINTMODE &stPrintMode)
{
    if (devPTR->N_SetPrintMode_Req(stPrintMode) != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：页模式下传打印字串
         打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
** 输入：pStr：要打印的字串
        ulDataLen：数据长度，如果为-1，pStr以\0结束
        unsigned long ulOrgX: 打印字串的列坐标，单位: 0.1mm
        unsigned long ulOrgY: 打印字串的行坐标，单位: 0.1mm
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::PrintPageTextOut(const char *pStr, unsigned long ulDataLen,
                                      unsigned long ulOrgX, unsigned long ulOrgY)
{
    // 数据为空或数据长度<1,返回参数错误
    if (pStr == nullptr || ulDataLen < 1)
    {
        return ERR_PTR_PARAM_ERR;
    }

    if (devPTR->N_PrintPageTextOut_Req((LPBYTE)pStr, ulDataLen, ulOrgX, ulOrgY) != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：页模式下传打印图片
** 输入：szImagePath：图片保存绝对路径
        unsigned long ulOrgX: 打印图片的列坐标，单位: 0.1mm
        unsigned long ulOrgY: 打印图片的行坐标，单位: 0.1mm
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::PrintPageImageOut(const char *szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    // 图片路径为NULL或宽高<1,返回参数错误
    if (szImagePath == nullptr)
    {
        return ERR_PTR_PARAM_ERR;
    }

    // 图片文件是否存在
    if (access(szImagePath, F_OK) != 0)
    {
        devPTR->N_SetErrCode(szErrPrt_FileNotFound);
        return ERR_PTR_PARAM_ERR;
    }

    // 打印图片
    if (devPTR->N_PrintPageImageOut_Req((LPBYTE)szImagePath, ulOrgX, ulOrgY) != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：执行页模式数据批量打印
** 输入：无
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::PrintPageData()
{
    if (devPTR->N_PrintPageData_Req() != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：行模式打印字串
         打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
** 输入：pStr：要打印的字串
        ulDataLen：数据长度，如果为-1，pStr以\0结束
        unsigned long ulOrgX: 打印字串的列坐标，单位: 0.1mm
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::PrintLineData(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX)
{
    // 数据为空或数据长度<1,返回参数错误
    if (pStr == nullptr || ulDataLen < 1)
        return ERR_PTR_PARAM_ERR;

    if (devPTR->N_PrintLineData_Req((LPBYTE)pStr, ulDataLen, ulOrgX) != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：获取处理错误码
** 输入：无
** 输出：无
** 返回：处理错误码(总计7位，错误码6位，1位结束符)
************************************************************/
char *IPrinterDevice::GetErrCode()
{
    return (char *)devPTR->N_GetErrCode_Req();
}

/************************************************************
** 功能：获取动态库版本
** 输入：无
** 输出：无
** 返回：动态库版本(总计17位，1位结束符)
************************************************************/
char *IPrinterDevice::GetVersion()
{
    //return (char *)devPTR->N_GetVersion_Req();
    return (char*)byVRTU_HOTS;
}

/************************************************************
** 功能：获取硬件设备指令处理返回码
** 输入：无
** 输出：无
** 返回：返回码
************************************************************/
unsigned long IPrinterDevice::GetDevErrCode()
{
    return devPTR->N_GetDevErrCode();
}

/************************************************************
** 功能：校正标记纸的起始位置
** 输入：byMakePos : 标记位置值,
** 输出：无
** 返回：见返回错误码定义
************************************************************/
long IPrinterDevice::ChkPaperMarkHeader(unsigned int uiMakePos)
{
    // 数据为空或数据长度<1,返回参数错误
    if (uiMakePos < 0 && uiMakePos > 318)
        return ERR_PTR_PARAM_ERR;

    if (devPTR->N_ChkPaperMarkHeader_Req(uiMakePos) != OK)
    {
        return ERR_PTR_OTHER;
    }

    return ERR_PTR_SUCCESS;
}

/************************************************************
** 功能：获取固件版本
** 输入：无
** 输出：无
** 返回：固件版本
************************************************************/
bool IPrinterDevice::GetFWVersion(char *szFWVer, unsigned long *ulSize)
{
    return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// 打印控制处理类                                                           //
//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : DevPTR_RPR()                                              **
** Describtion  : 构造函数                                                  **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
DevPTR_RPR::DevPTR_RPR(): m_Log("RPR")
{
    N_Close_Req();
    pRPR_USB = new RPR_USB();   // 打印设备指令处理结构体
    Inji_Ed = new INJI_ED();
    bIsDevOpen = FALSE;         // 设备当前状态，T:开启/F:关闭
    N_Init();                   // 变量初始化
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : DevPTR_RPR()                                              **
** Describtion  : 析构函数                                                  **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
DevPTR_RPR::~DevPTR_RPR()
{
    delete pRPR_USB;
    delete Inji_Ed;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : void N_Init()                                             **
** Describtion  : 变量初始化                                                **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void DevPTR_RPR::N_Init()
{
    // 打印模式变量初始化
    bIsPrintMode = FALSE;               // 是否已设置打印模式，缺省F
    bPageMode = FALSE;                  // 是否行打印模式，缺省F

    // 打印规格变量初始化
    wLeft = DEF_LEFTDIST_MM;            // 左边距，缺省: 3.7mm，单位:0.1mm
    wTop = DEF_TOPDIST_MM;              // 上边距，缺省:111mm，单位:0.1mm
    wWidth = HS2MM(MAX_PAGEWIDE);       // 可打印宽，缺省：= 72mm，单位:0.1mm
    wHeight = 0;                        // 可打印高，缺省0，单位:0.1mm

    wRowHeight = 30;                    // 行高，缺省3mm，单位:0.1mm
    wRowDistance = 0;                   // 行间距，单位:0.1mm，缺省0
    wMojiDistance = 0;                  // 字符间距，单位:0.1mm，缺省0

    // 错误码，初始值0000000
    memset(byErrCode, 0x00, sizeof(byErrCode));
    memcpy(byErrCode, szErrInit_Succ, 7);

    m_ePaperStatus = PAPER_STATUS_UNKNOWN;
    m_eTonerStatus = TONER_STATUS_UNKNOWN;
    m_eOutletStatus = OUTLET_STATUS_UNKNOWN;
    // 打印数据类变量初始化
    Inji_Ed->N_Init();

    Inji_Ed->N_SetPrintMode(wLeft, wTop, wWidth, wHeight, wRowHeight);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : WORD HSSPR::N_SetErrCode()                                **
** Describtion  : 设置错误码                                                **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void DevPTR_RPR::N_SetErrCode(const BYTE *byErrCd)
{
    memset(byErrCode, 0x00, sizeof(byErrCode));
    memcpy(byErrCode, byErrCd, 6);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : BYTE N_GetErrCode_Req()                                   **
** Describtion  : 错误码返回处理入口                                         **
** Parameter    : 无                                                        **
** Return       : 6位错误码+1位结束符                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
BYTE *DevPTR_RPR::N_GetErrCode_Req()
{
    return byErrCode;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : BYTE* N_GetVersion_Req()                                  **
** Describtion  : 版本号返回处理入口                                         **
** Parameter    : 无                                                        **
** Return       : 11位版本号+1位结束符                                       **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
BYTE *DevPTR_RPR::N_GetVersion_Req()
{
    //return byVRTU;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : BYTE N_GetErrCode_Req()                                   **
** Describtion  : 获取硬件设备指令处理返回码入口                                 **
** Parameter    : 无                                                        **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
ULONG DevPTR_RPR::N_GetDevErrCode()
{
    return pRPR_USB->N_GetErrCode_Usb();
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_Open_Req()                                         **
** Describtion  : OPEN设备处理入口                                           **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_Open_Req()
{
    long lRet = OK;

    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备为非Open状态 || 设备handle为0，执行open
    if (bIsDevOpen == FALSE || pRPR_USB->N_GetDevHandle_Usb() == 0)
    {
        lRet = pRPR_USB->N_Open_Usb();
    }

    if (lRet == OK)
    {
        bIsDevOpen = TRUE;  // 设置设备为Open状态
    }
    else
    {
        bIsDevOpen = FALSE;  // 设置设备为非Open状态
    }

    return lRet;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_Reset_Req()                                        **
** Describtion  : 设备重置处理入口                                           **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_Reset_Req()
{
    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    return pRPR_USB->N_Reset_Usb(TRUE);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_Close_Req()                                        **
** Describtion  : 设备终止处理入口                                           **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_Close_Req()
{
    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    bIsDevOpen = FALSE;     // 设置设备状态为非Open
    return pRPR_USB->N_Close_Usb();
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_CutPaper_Req()                                     **
** Describtion  : 切纸处理入口                                              **
** Parameter    : bDetectBlackStripe ：Input：是否检测黑标                   **
**                ulFeedSize : Input : 切纸前走纸的长度，单位0.1毫米无        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_CutPaper_Req(BOOL bDetectBlackStripe, ULONG ulFeedSize)
{
    long    lCommandSize = 0;
    BYTE    byPrintData[SEND_BLOCK_MAX_SIZE];  // 下传指令数据

    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 设置切纸指令到打印数据处理buffer
    Inji_Ed->N_AddCutPaperCmd(bDetectBlackStripe, ulFeedSize);

    // 获取 打印数据处理buffer size
    lCommandSize = (long)Inji_Ed->N_GetHensyuPtr();

    // 下传指令数据Copy到byPrintData
    Inji_Ed->N_CopyHensyuToSendBuffer(byPrintData, 0, lCommandSize);

    N_Init();

    // 调用切纸函数
    return pRPR_USB->N_CutPaper_Usb((LPBYTE)(byPrintData), lCommandSize);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_SetPrintFormat_Req()                               **
** Describtion  : 文本规格设置入口                                          **
** Parameter    :                                                           **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_SetPrintFormat_Req(STPRINTFORMAT stPrintFormat)
{
    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 设置行高=字体高+行间距
    if (stPrintFormat.uLPI > 0)
    {
        wRowHeight = 30 + stPrintFormat.uLPI;
    }

    // 打印区域设置到打印数据类
    Inji_Ed->N_SetPrintMode(wLeft, wTop, wWidth, wHeight, wRowHeight);

    // 文本打印规格设置到打印数据处理类(Inji_Ed)中
    return Inji_Ed->N_SetPrintFormat(stPrintFormat);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_SetPrintMode_Req()                                 **
** Describtion  : 打印模式设置入口                                          **
** Parameter    :                                                           **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_SetPrintMode_Req(STPRINTMODE stPrintMode)
{
    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 定义打印模式(TRUE:页模式/FALSE:行模式)
    bPageMode = stPrintMode.bPageMode;

    // 定义介质打印左边距
    if (stPrintMode.wLeft > 0)
    {
        wLeft = stPrintMode.wLeft;
    }

    // 定义介质打印上边距
    if (stPrintMode.wTop > 0)
    {
        wTop = stPrintMode.wTop;
    }

    // 定义介质可打印宽度
    if (stPrintMode.wWidth > 0)
    {
        wWidth = stPrintMode.wWidth;
    }

    // 定义介质可打印高度
    if (stPrintMode.wHeight > 0)
    {
        wHeight = stPrintMode.wHeight;
    }

    // 打印区域设置到打印数据类
    Inji_Ed->N_SetPrintMode(wLeft, wTop, wWidth, wHeight, wRowHeight);

    // 设置模式打印
    if (bPageMode == TRUE)  // 页模式
    {
        Inji_Ed->N_AddPageModeStart();
    }
    else
    {
        // 行模式
        Inji_Ed->N_AddLineModeStart();
    }

    bIsPrintMode = TRUE;    // 打印模式设置标记为T

    return OK;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_PrintPageTextOut_Req()                             **
** Describtion  : 页模式下发文本数据处理入口                                **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_PrintPageTextOut_Req(LPBYTE lpData, ULONG ulDataSz,
                                        ULONG ulOrgX, ULONG ulOrgY)
{
    long    lRet = OK;

    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 未设置打印模式
    if (bIsPrintMode == FALSE)
    {
        // 设置模式打印
        if (bPageMode == TRUE)                  // 页模式
        {
            Inji_Ed->N_AddPageModeStart();       // 设置页模式
            bIsPrintMode = TRUE;                // 修改为已设置打印模式
        }
        else
        {
            // 行模式不支持该函数
            N_SetErrCode(szErrPrt_NoSetMode);   // 设置处理错误码
            N_Init();                           // 返回NG前初始化变量，释放动态申请空间
            return NG;
        }
    }
    else
    {
        // 判断打印模式是否设置正确
        if (bPageMode == FALSE)                 // 行模式不支持该函数
        {
            bIsPrintMode = FALSE;               // 修改为未设置打印模式
            N_SetErrCode(szErrPrt_NoSetMode);   // 设置处理错误码
            N_Init();                           // 返回NG前初始化变量，释放动态申请空间
            return NG;
        }
    }

    // 处理入参坐标：< 上边距/左边距，缺省为上边距/左边距
    // 打印坐标是从文字/图片底边算起，因此上边距需要加上行高
    ulOrgX = (ulOrgX <= wLeft ? wLeft : ulOrgX);
    ulOrgY = (ulOrgY <= wTop ? wTop + wRowHeight :
              (ulOrgY - wRowHeight <= wTop ? wTop + wRowHeight :  ulOrgY));

    // 设置行列绝对位置打印坐标
    Inji_Ed->N_SetAbsolutePos(ulOrgX, ulOrgY);
    Inji_Ed->wYPosition = ulOrgY;

    // 打印数据内容及长度赋值到数据处理类
    Inji_Ed->N_PushInjiData(lpData, ulDataSz);

    // 开始数据处理
    lRet = Inji_Ed->N_PrintData_Edit();
    if (lRet != OK)
    {
        // 数据处理失败后，返回NG前初始化变量，释放动态申请空间
        N_Init();
        return lRet;
    }

    // 传入行坐标 > 已设置最大行坐标时，更新已设置最大行坐标
    if (ulOrgY > Inji_Ed->wYPosCount)
    {
        Inji_Ed->wYPosCount = ulOrgY;
    }

    return lRet;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_PrintPageImageOut_Req()                            **
** Describtion  : 页模式下发图片数据处理入口                                **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_PrintPageImageOut_Req(LPBYTE lpszImgFName, ULONG ulOrgX, ULONG ulOrgY)
{
    long    lRet = OK;

    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 未设置打印模式
    if (bIsPrintMode == FALSE)
    {
        // 设置模式打印
        if (bPageMode == TRUE)                  // 页模式
        {
            Inji_Ed->N_AddPageModeStart();       // 设置页模式
            bIsPrintMode = TRUE;                // 修改为已设置打印模式
        }
        else
        {
            // 行模式不支持该函数
            N_SetErrCode(szErrPrt_NoSetMode);   // 设置处理错误码
            N_Init();                           // 返回NG前初始化变量，释放动态申请空间
            return NG;
        }
    }
    else
    {
        // 判断打印模式是否设置正确
        if (bPageMode == FALSE)                 // 行模式不支持该函数
        {
            bIsPrintMode = FALSE;               // 修改为未设置打印模式
            N_SetErrCode(szErrPrt_NoSetMode);   // 设置处理错误码
            N_Init();                           // 返回NG前初始化变量，释放动态申请空间
            return NG;
        }
    }

    // 处理入参坐标：< 上边距/左边距，缺省为上边距/左边距
    //ulOrgX = (ulOrgX < wLeft ? wLeft : ulOrgX);
    //ulOrgY = (ulOrgY < wTop? wTop : ulOrgY);

    // 设置行列绝对位置打印坐标
    //Inji_Ed->N_SetAbsolutePos(ulOrgX, ulOrgY);

    // 图片文件解析,位图数据写入打印Buffer
    lRet = Inji_Ed->N_PrintImage_Edit(lpszImgFName, ulOrgX, ulOrgY);
    if (lRet != OK)
    {
        N_Init();                               // 返回NG前初始化变量，释放动态申请空间
        return lRet;
    }

    return lRet;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_PrintPageData_Req()                                **
** Describtion  : 页模式批量打印数据处理入口                                **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_PrintPageData_Req()
{
    long    lRet = OK;
    long    lOffset = 0;
    long    lCommandSize = 0;

    BYTE byPrintData[SEND_BLOCK_MAX_SIZE];

    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 未设置打印模式,不打印，返回NG
    if (bIsPrintMode == FALSE)
    {
        N_SetErrCode(szErrPrt_NoSetMode);   // 设置处理错误码
        N_Init();                           // 返回NG前初始化变量，释放动态申请空间
        return NG;
    }

    // 根据已下传数据坐标确定重新设定打印区域(高度)
    Inji_Ed->N_AddHensyuChar(Inji_Ed->wPageModePtr + 6, BYTE(MM2HS(Inji_Ed->wYPosCount) % DIVREM_COMP));
    Inji_Ed->N_AddHensyuChar(Inji_Ed->wPageModePtr + 7, BYTE(MM2HS(Inji_Ed->wYPosCount) / DIVREM_COMP));

    // 数据打印并恢复行模式
    Inji_Ed->N_AddCmdHensyuArray(CMDF_PAGE_CHANGE_0C);

    // 数据处理完成后，文本打印格式指定
    Inji_Ed->N_AddCmdHensyuArray(CMDF_INJIMODE_1B21);

    // 数据处理完成后，设置打印结束标记设置
    Inji_Ed->N_AddCmdHensyuArray(CMDF_PARAMETER_1D6C);

    // -------------------------------------------------------------------------------------------
    // 获取处理后的打印数据集合长度
    lCommandSize = (long)Inji_Ed->N_GetHensyuPtr();

    // 分批下传打印数据到硬件设备，每次送信上限4096字节
    for (lOffset = 0; lOffset + SPR_SND_BUFF_SIZE < lCommandSize; lOffset += SPR_SND_BUFF_SIZE)
    {
        // 处理后buffers打印数据 Copy到 下传数据buffer
        Inji_Ed->N_CopyHensyuToSendBuffer(byPrintData, lOffset, SPR_SND_BUFF_SIZE);

        // 调用USB接口函数下发打印数据
        lRet = pRPR_USB->N_SendData_Usb((LPBYTE)(byPrintData), SPR_SND_BUFF_SIZE);
        if (lRet != OK)
        {
            N_Init();                    // 返回NG前初始化变量，释放动态申请空间
            return lRet;
        }
    }

    // 最后一排打印数据 Copy到 下传数据buffer
    Inji_Ed->N_CopyHensyuToSendBuffer(byPrintData, lOffset, lCommandSize - lOffset);

    // 调用USB接口函数下发打印数据
    lRet = pRPR_USB->N_SendData_Usb((LPBYTE)(byPrintData), lCommandSize - lOffset);
    if (lRet != OK)
    {
        N_Init();                       // 返回NG前初始化变量，释放动态申请空间
        return lRet;
    }

    N_Init();                           // 处理结束，初始化变量，释放动态申请空间

    bIsPrintMode = FALSE;               // 修改为未设置打印模式

    return lRet;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_PrintLineData_Req()                                **
** Describtion  : 行模式打印文本数据处理入口                                    **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                            **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_PrintLineData_Req(LPBYTE lpData, ULONG ulDataSz, ULONG ulOrgX)
{

    {
        m_Log.LogAction("SouData");
        //记录发送的数据
        char temp[30] = {0};
        sprintf(temp, "SEND[%3d]:", ulDataSz);
        m_Log.NewLine().Log(temp).LogHex((LPCSTR)lpData, ulDataSz).EndLine();
    }

    long    lRet = OK;
    long    lOffset = 0;
    long    lCommandSize = 0;
    BYTE    byPrintData[SEND_BLOCK_MAX_SIZE];

    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 处理入参坐标,设置左边距
    Inji_Ed->N_SetPrintLeft(ulOrgX);

    // 切换到行模式
    Inji_Ed->N_AddLineModeStart();

    // 设置行列绝对位置打印坐标
    Inji_Ed->N_SetAbsolutePos(ulOrgX, 10);

    /*
        // 打印数据内容及长度赋值到数据处理类
        Inji_Ed->N_PushInjiData(lpData, ulDataSz);

        // 开始数据处理
        lRet = Inji_Ed->N_PrintData_Edit();
        if (lRet != OK)
        {
            N_Init();                       // 返回NG前初始化变量，释放动态申请空间
            return NG;
        }
    */

    // 行模式数据无限制打印
    // 分次处理下发数据，每次上限3000字节
    ULONG ulCount = 0;      // 已处理数据计数
    ULONG ulPrtCount = 0;   // 一次下发到设备的数据量计数
    LPBYTE lpDataTmp = lpData;  //
    while (ulDataSz - ulCount > 0)
    {

        lpDataTmp = lpData + ulCount;
        while (1)
        {
            if (ulDataSz - ulCount >= 3)
            {
                BYTE    byK1 = *(lpDataTmp + ulPrtCount);
                BYTE    byK2 = *(lpDataTmp + ulPrtCount + 1);

                // 验证当前位置是字符还是汉字
                long lRetCode = lDGetCodeType(byK1, byK2);
                if (lRetCode == NG)
                {
                    ulCount = ulCount + 1;
                    ulPrtCount = ulPrtCount + 1;
                }
                else
                {
                    ulCount = ulCount + 3;
                    ulPrtCount = ulPrtCount + 3;
                }
            }
            else
            {
                ulCount = ulCount + 1;
                ulPrtCount = ulPrtCount + 1;
            }

            if (ulCount >= ulDataSz || ulPrtCount >= 3000)
                break;
        }

        // 打印数据内容及长度赋值到数据处理类
        Inji_Ed->N_PushInjiData(lpDataTmp, ulPrtCount);



        // 开始数据处理
        lRet = Inji_Ed->N_PrintData_Edit();
        if (lRet != OK)
        {
            N_Init();                       // 返回NG前初始化变量，释放动态申请空间
            return NG;
        }

        // 获取处理后的打印数据集合长度
        lCommandSize = (long)Inji_Ed->N_GetHensyuPtr();

        // 分批下传打印数据到硬件设备，每次送信上限4096字节
        for (lOffset = 0; lOffset + SPR_SND_BUFF_SIZE < lCommandSize; lOffset += SPR_SND_BUFF_SIZE)
        {
            // 处理后buffers打印数据 Copy到 下传数据buffer
            Inji_Ed->N_CopyHensyuToSendBuffer(byPrintData, lOffset, SPR_SND_BUFF_SIZE);

            // 调用USB接口函数下发打印数据
            lRet = pRPR_USB->N_SendData_Usb((LPBYTE)(byPrintData), SPR_SND_BUFF_SIZE);
            if (lRet != OK)
            {
                N_Init();                    // 返回NG前初始化变量，释放动态申请空间
                return lRet;
            }
        }

        // 最后一排打印数据 Copy到 下传数据buffer
        Inji_Ed->N_CopyHensyuToSendBuffer(byPrintData, lOffset, lCommandSize - lOffset);

        // 调用USB接口函数下发打印数据
        lRet = pRPR_USB->N_SendData_Usb((LPBYTE)(byPrintData), lCommandSize - lOffset);
        if (lRet != OK)
        {
            N_Init();                       // 返回NG前初始化变量，释放动态申请空间
            return lRet;
        }

        Inji_Ed->N_ClearPrintBuffer();

        ulPrtCount = 0;
    }

    // 数据打印并恢复行模式
    Inji_Ed->N_AddCmdHensyuArray(CMDF_PAGE_CHANGE_0C);

    // 数据处理完成后，文本打印格式指定
    Inji_Ed->N_AddCmdHensyuArray(CMDF_INJIMODE_1B21);

    // 数据处理完成后，打印结束标记设置
    Inji_Ed->N_AddCmdHensyuArray(CMDF_PARAMETER_1D6C);

    // -------------------------------------------------------------------------------------------
    // 获取处理后的打印数据集合长度
    lCommandSize = (long)Inji_Ed->N_GetHensyuPtr();

    // 分批下传打印数据到硬件设备，每次送信上限4096字节
    for (lOffset = 0; lOffset + SPR_SND_BUFF_SIZE < lCommandSize; lOffset += SPR_SND_BUFF_SIZE)
    {
        // 处理后buffers打印数据 Copy到 下传数据buffer
        Inji_Ed->N_CopyHensyuToSendBuffer(byPrintData, lOffset, SPR_SND_BUFF_SIZE);

        // 调用USB接口函数下发打印数据
        lRet = pRPR_USB->N_SendData_Usb((LPBYTE)(byPrintData), SPR_SND_BUFF_SIZE);
        if (lRet != OK)
        {
            N_Init();                    // 返回NG前初始化变量，释放动态申请空间
            return lRet;
        }
    }

    // 最后一排打印数据 Copy到 下传数据buffer
    Inji_Ed->N_CopyHensyuToSendBuffer(byPrintData, lOffset, lCommandSize - lOffset);

    // 调用USB接口函数下发打印数据
    lRet = pRPR_USB->N_SendData_Usb((LPBYTE)(byPrintData), lCommandSize - lOffset);
    if (lRet != OK)
    {
        N_Init();                       // 返回NG前初始化变量，释放动态申请空间
        return lRet;
    }

    N_Init();                           // 处理结束，初始化变量，释放动态申请空间

    bIsPrintMode = FALSE;               // 修改为未设置打印模式

    return lRet;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_PrintData_Req(LPBYTE lpData, WORD wDataSz)         **
** Describtion  : 打印数据处理入口(不指定打印坐标)                          **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_PrintData_Req(LPBYTE lpData, ULONG ulDataSz)
{
    return N_PrintLineData_Req(lpData, ulDataSz, 0);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_PrintImage_Req()                                   **
** Describtion  : 打印图片处理入口(不指定打印坐标)                          **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_PrintImage_Req(LPBYTE lpszImgFName)
{

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 页模式下发图片，行列坐标:0,0
    if (N_PrintPageImageOut_Req(lpszImgFName, 0, 0) != OK)
    {
        N_Init();       // 返回NG前初始化变量，释放动态申请空间
        return NG;
    }

    // 批量打印
    if (N_PrintPageData_Req() != OK)
    {
        N_Init();       // 返回NG前初始化变量，释放动态申请空间
        return NG;
    }

    return OK;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_QueryStatus_Req()                                  **
** Describtion  : 获取设备状态处理入口                                      **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_QueryStatus_Req()
{
    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备是否处于连线中(判断VID+PID不存在 & 错误码为无错误)
    /* if((bChkUsbDev(RPR_USBVID, RPR_USBPID) == FALSE))
     {
        BYTE *a = N_GetErrCode_Req();
         // 错误码非OK，bChkUsbDev()执行有错误
         if(memcmp(N_GetErrCode_Req(), szErrInit_Succ, 6) != 0)
         {
             return ERR_PTR_OTHER;
         }else
         {
             return ERR_PTR_NO_DEVICE;
         }
     }
     */

    // 设备Open状态
    if (bIsDevOpen == FALSE)
    {
        return NG;    // 设备没有打开
    }


    // 获取打印纸、出纸口状态
    long lRes = N_GetStatus_Req(m_ePaperStatus, m_eTonerStatus, m_eOutletStatus);
    if (lRes != OK)
    {
        return NG;
    }
    return OK;
}

long  DevPTR_RPR::N_GetStatus_Get(PaperStatus &PaperStatus, TonerStatus &TonerStatus,
                                  OutletStatus &OutletStatus)
{
    PaperStatus = m_ePaperStatus;
    TonerStatus = m_eTonerStatus;
    OutletStatus = m_eOutletStatus;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_GetStatus_Req()                                    **
** Describtion  : 获取设备部件状态处理入口                                  **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_GetStatus_Req(PaperStatus &PaperStatus, TonerStatus &TonerStatus, OutletStatus &OutletStatus)
{
    PaperStatus = PAPER_STATUS_UNKNOWN;     // 打印纸状态，初始未知
    TonerStatus = TONER_STATUS_UNSUPPORT;     // INK、TONER或色带状态状态，初始未知
    OutletStatus = OUTLET_STATUS_UNKNOWN;   // 出纸口状态，初始未知

    LPBYTE  lpStat = nullptr;

    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 获取sensor状态数据串
    lpStat = pRPR_USB->N_GetStatus_Usb();

    if (lpStat != nullptr)
    {
        // 打印纸状态
        if ((*(lpStat + SPR_YOUSHI)) & SPR_STATUS_NEAR_END)
        {
            PaperStatus = PAPER_STATUS_LOW;     // 纸少
        }
        else
        {
            PaperStatus = PAPER_STATUS_NORMAL;  // 正常
        }

        // 打印纸状态: 卡纸/堵塞
        if ((*(lpStat + SPR_ERROR2) & SPR_STATUS_JAM_ERR) ||         // 用纸搬送异常
            (*(lpStat + SPR_ERROR2) & SPR_STATUS_PAPER_ERR) ||        // 异常用纸检出
            (*(lpStat + SPR_ERROR2) & SPR_STATUS_PRINT_RETRACT) ||    // 印字中用纸回收异常
            (*(lpStat + SPR_ERROR2) & SPR_STATUS_PAPER_REMAIN))       // 搬送路用纸残留
        {
            PaperStatus = PAPER_STATUS_JAMMED;     // 卡纸
        }

        // 出纸口状态: 有纸/无纸
        if ((*(lpStat + SPR_HANSOURO)) & SPR_STATUS_SENSOR_A)
        {
            OutletStatus = OUTLET_STATUS_MEDIA;     // 有纸
        }
        else
        {
            OutletStatus = OUTLET_STATUS_NOMEDIA;   // 无纸
        }
    }

    /*
    // 打印纸状态
    if(pRPR_USB->N_GetStatus_Usb(SPR_YOUSHI) & SPR_STATUS_NEAR_END)
    {
        PaperStatus = PAPER_STATUS_LOW;     // 纸少
    }else
    {
        PaperStatus = PAPER_STATUS_NORMAL;  // 正常
    }

    // 打印纸状态: 卡纸/堵塞
    if((pRPR_USB->N_GetStatus_Usb(SPR_ERROR2) & SPR_STATUS_JAM_ERR) ||          // 用纸搬送异常
       (pRPR_USB->N_GetStatus_Usb(SPR_ERROR2) & SPR_STATUS_PAPER_ERR) ||        // 异常用纸检出
       (pRPR_USB->N_GetStatus_Usb(SPR_ERROR2) & SPR_STATUS_PRINT_RETRACT) ||    // 印字中用纸回收异常
       (pRPR_USB->N_GetStatus_Usb(SPR_ERROR2) & SPR_STATUS_PAPER_REMAIN))       // 搬送路用纸残留
    {
        PaperStatus = PAPER_STATUS_JAMMED;     // 卡纸
    }

    // 出纸口状态: 有纸/无纸
    if((pRPR_USB->N_GetStatus_Usb(SPR_HANSOURO) & 0x0F) == SPR_STATUS_SENSOR_A)
    {
        OutletStatus = OUTLET_STATUS_MEDIA;     // 有纸
    }else
    {
        OutletStatus = OUTLET_STATUS_NOMEDIA;   // 无纸
    }*/

    return OK;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : void N_ChkPaperMarkHeader_Req()                           **
** Describtion  : 校正标记纸的起始位置                                          **
** Parameter    : 无                                                        **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long DevPTR_RPR::N_ChkPaperMarkHeader_Req(UINT uiMarkPos)
{
    long    lCommandSize = 0;
    BYTE    byPrintData[SEND_BLOCK_MAX_SIZE];  // 下传指令数据

    // 初始化处理错误码
    N_SetErrCode(szErrInit_Succ);

    // 设备处于非Open状态
    if (bIsDevOpen == FALSE)
    {
        N_SetErrCode(szErrUSB_DevNotOpen);
        return NG;
    }

    // 设置指令到打印数据处理buffer
    Inji_Ed->N_AddChkPaperCmd(uiMarkPos);

    // 获取 打印数据处理buffer size
    lCommandSize = (long)Inji_Ed->N_GetHensyuPtr();

    // 下传指令数据Copy到byPrintData
    Inji_Ed->N_CopyHensyuToSendBuffer(byPrintData, 0, lCommandSize);

    N_Init();

    // 调用函数
    return pRPR_USB->N_SendData_Usb((LPBYTE)(byPrintData), lCommandSize);
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : BOOL bChkUsbDev()                                         **
** Describtion  : 枚举设备是否存在                                             **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
BOOL DevPTR_RPR::bChkUsbDev(ULONG ulVid, ULONG ulPid)
{
    long    lRet = 0;
    ssize_t stCnt = 0;
    libusb_context *lbCtx = nullptr;        // context上下文
    libusb_device **ldDevList = nullptr;    // 设备handle列表变量
    libusb_device *ldDev = nullptr;         // 指定设备handle变量
    BOOL    bIsDevHave = FALSE;             // 返回设备是否存在标记
    int     i = 0;

    // 初始化libusb
    lRet = libusb_init(&lbCtx);
    if (lRet < 0)
    {
        N_SetErrCode(szErrUSB_EnumDevInit);
        return FALSE;
    }

    // 获取设备列表
    stCnt = libusb_get_device_list(nullptr, &ldDevList);
    if (stCnt < 0)
    {
        N_SetErrCode(szErrUSB_EnumGetDevList);
        return FALSE;
    }

    // 循环验证是否存在对应VID+PID
    while ((ldDev = ldDevList[i++]) != nullptr)
    {
        struct libusb_device_descriptor ldDesc;     // 设备信息描述符

        lRet = libusb_get_device_descriptor(ldDev, &ldDesc);    // 获取 设备信息描述符
        if (lRet < 0)
        {
            N_SetErrCode(szErrUSB_EnumGetDevDesc);
            return FALSE;
        }

        // VID+PID存在=设备连线，设置存在标记为T,跳出循环
        if (ldDesc.idVendor == ulVid && ldDesc.idProduct == ulPid)
        {
            bIsDevHave = TRUE;
            break;
        }
    }

    // 关闭libusb
    libusb_exit(lbCtx);

    ldDevList = nullptr;
    ldDev = nullptr;

    return bIsDevHave;
}


//////////////////////////////////////////////////////////////////////////////
// 打印数据处理类                                                             //
//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::INJI_ED                                          **
** Function     : INJI_ED::INJI_ED()                                        **
** Describtion  : 构造函数                                                  **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
INJI_ED::INJI_ED()
{
    N_Init();
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::~INJI_ED                                         **
** Function     : INJI_ED::~INJI_ED()                                       **
** Describtion  : 析构函数                                                  **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
INJI_ED::~INJI_ED()
{
    N_FreeBuffer();
    N_FreeMojiBuffer();
}

/*****************************************************************************
** FunctionType : private                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void N_Init()                                             **
** Describtion  : 初始化函数                                                **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_Init()
{
    wLineWidthByte      = 0;                // 一行打印数据量(字节/mm)
    wPaperFeedSz        = 0;                // 正向送纸，单位：0.1mm

    wXPosition          = 0;                // 列当前坐标
    wYPosition          = 0;                // 行当前坐标
    wYPosCount          = 0;                // 行坐标计数

    wPageModePtr        = 0;                // 打印区域设置指令在处理后打印数据保存Buffer的位置

    byHensyuBuffer      = nullptr;          // 处理后打印数据保存Buffer
    dwHensyuPtr         = 0;                // 处理后已保存数据的Length
    dwHensyuBufferSize  = 0;                // 处理后打印数据保存Buffer Size 上限

    byMojiBuffer        = nullptr;          // 处理中数据保存buffer
    dwMojiPtr           = 0;                // 处理中以保存数据Length

    lpbySansyo          = nullptr;          // 下传初始数据指针(循环处理数据用)
    lpbyLastSansyo      = nullptr;          // 下传初始数据指针(循环处理数据用)
    wSansyoPtr          = 0;                // 下传初始数据指针位移量(循环处理数据用)
    wSansyoSz           = 0;                // 下传数据总Size

    bLineEnd            = FALSE;            // 换行标记/数据处理结束标记，缺省F

    // 文本规格
    memset(byFontType, 0x00, sizeof(byFontType));   // 字体文件路径
    memcpy(byFontType, FT_HEITI, strlen(FT_HEITI));
    wFontSize           = 0;                // 字体大小
    wFontStyle          = PTR_TEXT_NORMAL;  // 文本风格，缺省正常
    wRowDistance        = 0;                // 行间距，缺省0
    wMojiDistance       = 0;                // 字符间距

    // 打印规格
    wLeft               = 0;                // 左边距，单位:0.1mm
    wTop                = 0;                // 上边距，单位:0.1mm
    wWidth              = 0;                // 可打印宽，单位:0.1mm
    wHeight             = 0;                // 可打印高，单位:0.1mm
    wRowHeight          = 0;                // 行高，单位:0.1mm
}

/*****************************************************************************
** FunctionType : private                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_InitBuffer                                     **
** Function     : void INJI_ED::N_InitBuffer()                              **
** Describtion  : 处理后打印数据保存Buffer空间申请初始化                    **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_InitBuffer()
{
    // buffer已申请空间，先Free
    if (byHensyuBuffer != nullptr)
    {
        free(byHensyuBuffer);
    }

    // 申请空间，长度0x1FFFE = 131,070字节
    byHensyuBuffer = nullptr;
    byHensyuBuffer = (LPBYTE) malloc(HENSYU_BUFF_SIZE);
    if (byHensyuBuffer != nullptr)
    {
        memset(byHensyuBuffer, 0x00, HENSYU_BUFF_SIZE);
    }
    dwHensyuPtr = 0;
    dwHensyuBufferSize = HENSYU_BUFF_SIZE;
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void INJI_ED::N_InitMojiBuffer()                          **
** Describtion  : 处理中数据保存buffer空间申请初始化                        **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_InitMojiBuffer()
{
    // buffer已申请空间，先Free
    if (byMojiBuffer != nullptr)
    {
        free(byMojiBuffer);
    }

    // 申请空间，长度0x1FFFE = 131,070字节
    byMojiBuffer = nullptr;
    byMojiBuffer = (LPBYTE)malloc(HENSYU_BUFF_SIZE);
    if (byMojiBuffer != nullptr)
    {
        memset(byMojiBuffer, 0x00, HENSYU_BUFF_SIZE);
    }
    dwMojiPtr = 0;
}

/*****************************************************************************
** FunctionType : private                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_FreeBuffer                                     **
** Function     : void INJI_ED::N_FreeBuffer()                              **
** Describtion  : 处理后打印数据保存Buffer空间释放                          **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_FreeBuffer()
{
    if (byHensyuBuffer != nullptr)
    {
        free(byHensyuBuffer);
    }

    byHensyuBuffer = nullptr;
    dwHensyuBufferSize = 0;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void INJI_ED::N_FreeMojiBuffer()                          **
** Describtion  : 处理中数据保存buffer空间释放                              **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_FreeMojiBuffer()
{
    if (byMojiBuffer != nullptr)
    {
        free(byMojiBuffer);
    }

    byMojiBuffer = nullptr;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void INJI_ED::N_ClearPrintBuffer()                        **
** Describtion  : 清空打印buffer空间                                          **
** Parameter    : 无                                                        **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_ClearPrintBuffer()
{
    if (byHensyuBuffer != nullptr)
    {
        memset(byHensyuBuffer, 0x00, dwHensyuBufferSize);
    }

    dwHensyuPtr = 0;

    if (byMojiBuffer != nullptr)
    {
        memset(byMojiBuffer, 0x00, dwHensyuBufferSize);
    }

    dwMojiPtr = 0;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void INJI_ED::N_PushInjiData()                            **
** Describtion  : 下传初始打印数据空间指针赋值                              **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_PushInjiData(LPBYTE lpbyInji, long lSz)
{
    lpbyLastSansyo  = lpbyInji;     // 下传初始数据指针
    lpbySansyo      = lpbyInji;     // 下传初始数据指针
    wSansyoSz       = (WORD)lSz;    // 下传初始数据长度
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_AddHensyuArray                                 **
** Function     : void N_HensyuArray(LPBYTE lpbyBuffer, long lLen)          **
** Describtion  : 数据写入处理后打印数据保存buffer                          **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddHensyuArray(LPBYTE lpbyBuffer, long lLen)
{
    if ((dwHensyuPtr + lLen) > dwHensyuBufferSize)
    {
        return;
    }

    memcpy(byHensyuBuffer + dwHensyuPtr, lpbyBuffer, lLen);
    dwHensyuPtr += lLen;
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_AddMojiChar                                    **
** Function     : void N_AddMojiChar()                                      **
** Describtion  : 向数据处理中buffer写入数据                                **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddMojiChar(BYTE byMoji)
{
    byMojiBuffer[dwMojiPtr++] = byMoji;
}

/*****************************************************************************
** FunctionType : privage                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_AddImageTouroku                                **
** Function     : void N_AddImageTouroku()                                  **
** Describtion  : 文本位图数据(以位图图像模式打印)写入处理中Buffer          **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddImageTouroku(LPBYTE lpbyFontBuff, long lLen)
{
    N_AddMojiChar(0x1B);
    N_AddMojiChar(0x2A);
    N_AddMojiChar(0x21);
    N_AddMojiChar((BYTE)(lLen / 3)); // nl、nh表示要打印的比特图像的横向点数，成为[nh×256+nl]
    N_AddMojiChar(0x00);            // nl、nh表示要打印的比特图像的横向点数，成为[nh×256+nl]
    memcpy(&byMojiBuffer[dwMojiPtr], lpbyFontBuff, lLen);
    dwMojiPtr += lLen;
}

/*****************************************************************************
** FunctionType : Private                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_ClearLineInfo                                  **
** Function     : void N_ClearLineInfo()                                    **
** Describtion  : 清空打印数据处理中buffer                                     **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_ClearLineInfo()
{
    memset(byMojiBuffer, 0x00, sizeof(byMojiBuffer));
    bLineEnd    = FALSE;
    dwMojiPtr    = 0;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void N_CopyHensyuToSendBuffer()                           **
** Describtion  : 从处理后下发打印数据buffer中Copy指定数目的数据            **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_CopyHensyuToSendBuffer(LPBYTE lpData, long lOffset, WORD wBuffSize)
{
    memset(lpData, 0x00, sizeof(lpData));
    memcpy(lpData, byHensyuBuffer + lOffset, wBuffSize);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_AddHensyuChar                                  **
** Function     : void N_AddHensyuChar()                                    **
** Describtion  : 处理后可打印数据buffer末尾写入数据                        **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddHensyuChar(BYTE byChar)
{
    if ((dwHensyuPtr + 1) > dwHensyuBufferSize)
    {
        return;
    }

    *(byHensyuBuffer + dwHensyuPtr) = byChar;
    dwHensyuPtr++;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_AddHensyuChar                                  **
** Function     : void N_AddHensyuChar()                                    **
** Describtion  : 在处理后可打印数据buffer指定位置写入数据                  **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddHensyuChar(WORD wHensyPtr, BYTE byChar)
{
    if (wHensyPtr > dwHensyuBufferSize)
    {
        return;
    }

    *(byHensyuBuffer + wHensyPtr) = byChar;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_AddImageTouroku                                **
** Function     : void N_AddImageTouroku()                                  **
** Describtion  : 获取已处理的可打印buffer中数据长度                        **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
DWORD INJI_ED::N_GetHensyuPtr()
{
    return dwHensyuPtr;
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_SendDataLen                                    **
** Function     : void N_SendDataLen()                                      **
** Describtion  : 获取已处理的可下发打印的数据数量(处理中+处理后buffer)     **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
DWORD INJI_ED::N_SendDataLen()
{
    return (dwMojiPtr + dwHensyuPtr);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void N_CmdHensyuArray(BYTE byCMD)                         **
** Describtion  : 设置下发指令到处理后打印数据buffer                        **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddCmdHensyuArray(BYTE byCMD)
{
    switch (byCMD)
    {
    // 打印区域设置
    case CMDF_PAGEMODE_SET_1B57:
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        byHensyuBuffer[dwHensyuPtr - 8 + 0] = MM2HS(GETLEFT(wLeft)) % DIVREM_COMP;      // 左边距1
        byHensyuBuffer[dwHensyuPtr - 8 + 1] = MM2HS(GETLEFT(wLeft)) / DIVREM_COMP;      // 左边距2
        byHensyuBuffer[dwHensyuPtr - 8 + 2] = MM2HS(GETTOP(wTop)) % DIVREM_COMP;       // 上边距1
        byHensyuBuffer[dwHensyuPtr - 8 + 3] = MM2HS(GETTOP(wTop)) / DIVREM_COMP;       // 上边距2
        byHensyuBuffer[dwHensyuPtr - 8 + 4] = MM2HS(wWidth) % DIVREM_COMP;     // 可打印宽1
        byHensyuBuffer[dwHensyuPtr - 8 + 5] = MM2HS(wWidth) / DIVREM_COMP;     // 可打印宽2
        byHensyuBuffer[dwHensyuPtr - 8 + 6] = MM2HS(wHeight) % DIVREM_COMP;    // 可打印高1
        byHensyuBuffer[dwHensyuPtr - 8 + 7] = MM2HS(wHeight) / DIVREM_COMP;    // 可打印高2
        break;

    // 左边界设定
    case CMDF_LEFT_MARGIN_1D4C00 :
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        byHensyuBuffer[dwHensyuPtr - 2] = MM2HS(GETLEFT(wLeft)) % DIVREM_COMP;
        byHensyuBuffer[dwHensyuPtr - 1] = MM2HS(GETLEFT(wLeft)) / DIVREM_COMP;
        break;

    // 横绝对位置:00
    case CMDF_PAGE_JPOSX_INIT_1B24 :
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        break ;

    // 纵绝对位置:00
    case CMDF_PAGE_JPOSY_INIT_1D24 :
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        break ;

    // 横相对位置:00
    case CMDF_PAGE_XPOSX_INIT_1B5C :
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        break ;

    // 纵相对位置:00
    case CMDF_PAGE_XPOSY_INIT_1D5C :
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        break ;

    // 换行量设定
    case CMDF_GYOKANSPACE_1B3320 :
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        byHensyuBuffer[dwHensyuPtr - 1] = MM2HS(wRowHeight);
        break ;

    // 正方向送纸
    case CMDF_OKURI_SET_1B4A00 :
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        byHensyuBuffer[dwHensyuPtr - 1] = wPaperFeedSz;
        break ;

    default :
        dwHensyuPtr += GetCmdData_HOTS(byCMD,
                                       byHensyuBuffer + dwHensyuPtr, dwHensyuPtr, dwHensyuBufferSize);
        break ;
    }
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : WORD GetCmdData_HOTS()                                    **
** Describtion  : 设备底层指令分析匹配                                      **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
WORD INJI_ED::GetCmdData_HOTS(BYTE byID, LPBYTE lpbyCMD, LONG lBufferPtr, LONG lBufferSize)
{
    CMD_Data SndData[] =
    {
        /* 数据打印并恢复行模式 */      { CMDF_PAGE_CHANGE_0C,           1, 0x0C},
        /* 图像LSB/MSB选择 */           { CMDF_INJIPOSITION_123D01,      3, 0x12, 0x3D, 0x01},
        /* 设置文字右空格量:00 */       { CMDF_MOJIKANSPACE_1B2000,      3, 0x1B, 0x20, 0x00},
        /* 指定文本打印格式:00 */       { CMDF_INJIMODE_1B21,            3, 0x1B, 0x21, 0x00},
        /* 横绝对位置:00 */             { CMDF_PAGE_JPOSX_INIT_1B24,     4, 0x1B, 0x24, 0x00, 0x00},
        /* 设置换行量:H20=D32=4MM */    { CMDF_GYOKANSPACE_1B3320,       3, 0x1B, 0x33, 0x20},
        /* 设置换行量:H20=D64=8MM */    { CMDF_GYOKANSPACE_1B3340,       3, 0x1B, 0x33, 0x40},
        /* 正向送纸:00 */               { CMDF_OKURI_SET_1B4A00,         3, 0x1B, 0x4A, 0x00},
        /* 正向送纸:1C=D28=3.5MM */     { CMDF_OKURI_SET_1B4A1C,         3, 0x1B, 0x4A, 0x1C},
        /* 正向送纸:21=D33=4.125MM */   { CMDF_OKURI_SET_1B4A21,         3, 0x1B, 0x4A, 0x21},
        /* 正向送纸:22=D34=4.25MM */    { CMDF_OKURI_SET_1B4A22,         3, 0x1B, 0x4A, 0x22},
        /* 正向送纸:30=D48=6MM */       { CMDF_OKURI_SET_1B4A30,         3, 0x1B, 0x4A, 0x30},
        /* 正向送纸:4D=D77=9.625MM */   { CMDF_OKURI_SET_1B4A4D,         3, 0x1B, 0x4A, 0x4D},
        /* 正向送纸:59=D89=11.125MM */  { CMDF_OKURI_SET_1B4A59,         3, 0x1B, 0x4A, 0x59},
        /* 正向送纸:62=D98=12.25MM */   { CMDF_OKURI_SET_1B4A62,         3, 0x1B, 0x4A, 0x62},
        /* 正向送纸:81=D129=16.125MM */ { CMDF_OKURI_SET_1B4A81,         3, 0x1B, 0x4A, 0x81},
        /* 正向送纸:84=D132=16.5MM */   { CMDF_OKURI_SET_1B4A84,         3, 0x1B, 0x4A, 0x84},
        /* 正向送纸:FF=D255=31.875MM */ { CMDF_OKURI_SET_1B4AFF,         3, 0x1B, 0x4A, 0xFF},
        /* 设置为页模式 */              { CMDF_PAGEMODE_ON_1B4C,         2, 0x1B, 0x4C},
        /* 设置为行模式 */              { CMDF_LINEMODE_ON_1B53,         2, 0x1B, 0x53},
        /* 设置页模式打印区域 */        { CMDF_PAGEMODE_SET_1B57,       10, 0x1B, 0x57, 0x00, 0x00, 0x00, 0x00, 0x12, 0x02, 0x00, 0x00},
        /* 横相对位置:00 */             { CMDF_PAGE_XPOSX_INIT_1B5C,     4, 0x1B, 0x5C, 0x00, 0x00},
        /* 切纸 */                      { CMDF_CUT_1B69,                 2, 0x1B, 0x69},
        /* 纵绝对位置:00 */             { CMDF_PAGE_JPOSY_INIT_1D24,     4, 0x1D, 0x24, 0x00, 0x00},
        /* 页模式垂直绝对位置:18 */     { CMDF_PAGE_KIJUN_1D2418,        4, 0x1D, 0x24, 0x18, 0x00},
        /* 页模式垂直绝对位置:30 */     { CMDF_PAGE_KIJUN_1D2430,        4, 0x1D, 0x24, 0x30, 0x00},
        /* 标记起始位置开始移动 */      { CMDF_MARKCHK_1D3C,             2, 0x1D, 0x3C},
        /* 校正标记纸的起始位置 */      { CMDF_MARKHEADCHK_1D41,         4, 0x1D, 0x41, 0x00, 0x00},
        /* 左边界设定 */                { CMDF_LEFT_MARGIN_1D4C00,       4, 0x1D, 0x4C, 0x00, 0x00},
        /* 介质送出保留 */              { CMDF_HORYU_1D5832,             3, 0x1D, 0x58, 0x32},
        /* 介质送出:00 */               { CMDF_HAISYUTU_1D583300,        4, 0x1D, 0x58, 0x33, 0x00},
        /* 介质送出:01 */               { CMDF_HAISYUTU_1D583301,        4, 0x1D, 0x58, 0x33, 0x01},
        /* 介质回收:FF */               { CMDF_KAISYU_1D5834,            4, 0x1D, 0x58, 0x34, 0xFF},
        /* 纵相对位置:00 */             { CMDF_PAGE_XPOSY_INIT_1D5C,     4, 0x1D, 0x5C, 0x00, 0x00},
        /* 打印结束标志*/               { CMDF_PARAMETER_1D6C,           3, 0x1D, 0x6C, 0x00},
    };

    // １２ｈ ７８ｈ剪切位置校正(先指定下位字符)
    //             打印上端位置校正

    //１２ｈ ７７ｈ   ＳＷＤＩＰ４  ０４ｈ  ８４ｈ  ２バイト    マーク位置補正
    //              ＳＷＤＩＰ７    ０７ｈ  ８７ｈ  １バイト    カット位置補正設定
    long size = sizeof(SndData) / sizeof(SndData[0]);
    WORD wSz  = 0;

    for (long i = 0 ; i < size ; i++)
    {
        if (byID != SndData[i].byID) continue;
        wSz = SndData[i].lLen;
        if (byID == CMDF_PARAMETER_1D6C)
        {
            *(SndData[i].lpCMD + 2) = GetParameterID();
        }

        if (lBufferPtr + wSz > lBufferSize)
        {
            return 0;
        }

        memcpy(lpbyCMD, SndData[i].lpCMD, wSz);
        break;
    }

    return wSz;
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : WORD GetParameterID()                                     **
** Describtion  : 获取下发指令序列号                                        **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
BYTE INJI_ED::GetParameterID()
{
    static BYTE ParameterID = 1;

    ParameterID++;
    if (ParameterID > 63)
    {
        ParameterID = 1;
    }
    return ParameterID;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void INJI_ED::N_SetPrintMode()                            **
** Describtion  : 设置打印规格                                              **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_SetPrintMode(WORD wDLeft, WORD wDTop, WORD wDWidth, WORD wDHeight, WORD wDRowHeight)
{
    wLeft = wDLeft;
    wTop  = wDTop;
    wWidth = wDWidth;
    wHeight = wDHeight;
    wRowHeight = wDRowHeight;
    wLineWidthByte = wWidth / 10; // 一行打印数据量(字节/mm)
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void INJI_ED::N_SetPrintLeft()                            **
** Describtion  : 设置打印左边距                                              **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_SetPrintLeft(WORD wDLeft)
{
    wLeft = wDLeft;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void N_AddCutPaperCmd()                                   **
** Describtion  : 切纸指令处理                                              **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddCutPaperCmd(BOOL bDetectBlackStripe, ULONG ulFeedSize)
{
    // 初始化打印数据处理buffer
    N_InitBuffer();

    // 按黑标切纸
    if (bDetectBlackStripe == TRUE)
    {
        // 校正标记纸的起始位置
        N_AddCmdHensyuArray(CMDF_MARKCHK_1D3C);

        // 打印结束标记
        N_AddCmdHensyuArray(CMDF_PARAMETER_1D6C);
    }

    if (ulFeedSize > 0)
    {
        wPaperFeedSz = ulFeedSize;

        // 正向送纸
        N_AddCmdHensyuArray(CMDF_OKURI_SET_1B4A00);

        // 打印结束标记
        N_AddCmdHensyuArray(CMDF_PARAMETER_1D6C);
    }

    // 切纸指令

    N_AddCmdHensyuArray(CMDF_CUT_1B69);
    N_AddCmdHensyuArray(CMDF_INJIPOSITION_123D01);

    // 打印结束标记
    N_AddCmdHensyuArray(CMDF_PARAMETER_1D6C);

    // 纸送出指令
    N_AddCmdHensyuArray(CMDF_HAISYUTU_1D583301);

    // 打印结束标记
    N_AddCmdHensyuArray(CMDF_PARAMETER_1D6C);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void N_AddChkPaperCmd()                                   **
** Describtion  : 校正标记纸起始位置指令处理                                    **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddChkPaperCmd(UINT uiMarkPos)
{
    // 初始化打印数据处理buffer
    N_InitBuffer();

    // 校正标记纸起始位置指令
    N_AddCmdHensyuArray(CMDF_MARKHEADCHK_1D41);
    byHensyuBuffer[dwHensyuPtr - 1] = MM2HS(uiMarkPos);

    // 结束标记
    N_AddCmdHensyuArray(CMDF_PARAMETER_1D6C);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : DevPTR_RPR                                                **
** Symbol       : DevPTR_RPR::                                              **
** Function     : long N_SetPrintFormat()                                   **
** Describtion  : 打印文本格式设置入口                                      **
** Parameter    :                                                           **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long INJI_ED::N_SetPrintFormat(STPRINTFORMAT stPrintFormat)
{
    // 字体文件是否有值
    if (strlen(stPrintFormat.szFontType) < 1 ||
        memcmp(stPrintFormat.szFontType, "0", 1) == 0)
    {
        // 无值，使用缺省黑体字体文件
        // 缺省黑体字体文件是否存在
        if (access(FT_HEITI, F_OK) != 0)
        {
            devPTR->N_SetErrCode(szErrSet_FTypeMNotFound);
            //return NG;
        }
        memset(byFontType, 0x00, sizeof(byFontType));
        memcpy(byFontType, FT_HEITI, strlen(FT_HEITI));

    }
    else
    {
        // 有值，使用传入字体文件
        // 字体文件是否存在
        if (access(stPrintFormat.szFontType, F_OK) != 0)
        {
            devPTR->N_SetErrCode(szErrSet_FTypeNotFound);
            return NG;
        }
        memset(byFontType, 0x00, sizeof(byFontType));
        memcpy(byFontType, stPrintFormat.szFontType, strlen(stPrintFormat.szFontType));

        strcpy(FT_HEITI, stPrintFormat.szFontType);
    }

    // 字体大小
    if (stPrintFormat.uFontSize > 0)
    {
        wFontSize = stPrintFormat.uFontSize;
    }
    else
    {
        devPTR->N_SetErrCode(szErrSet_FSizeInvalid);
        //return NG;
    }

    // 文本风格，只支持分别设置常规/粗体
    if (stPrintFormat.ulStyle == PTR_TEXT_NORMAL ||
        stPrintFormat.ulStyle == PTR_TEXT_BOLD)
    {
        wFontStyle = stPrintFormat.ulStyle;
    }
    else
    {

        devPTR->N_SetErrCode(szErrSet_FStyleInvalid);
        //return NG;
    }

    // 行间距
    if (stPrintFormat.uLPI > 1)
    {
        wRowDistance = stPrintFormat.uLPI;
    }
    else
    {
        devPTR->N_SetErrCode(szErrSet_RDistanInvalid);
        //return NG;
    }

    // 字符间距
    if (stPrintFormat.uWPI > 1)
    {
        wMojiDistance = stPrintFormat.uWPI;
    }
    else
    {
        devPTR->N_SetErrCode(szErrSet_MDistanInvalid);
        //return NG;
    }

    return OK;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void N_PrintData_Edit()                                   **
** Describtion  : 打印数据处理开始                                          **
** Parameter    :                                                           **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long INJI_ED::N_PrintData_Edit()
{
    long    lRet = OK;
    wSansyoPtr  = 0;    // 当前初始数据处理位置

    N_ClearLineInfo();

    // 传入打印数据个数 != 0，开始处理循环
    if (wSansyoSz != 0)
    {
        // 开始循环处理，根据情况每次处理1位字符或2位/3位汉字
        // lpbySansyo + wSansyoSz：下传打印初始数据结尾
        // lpbyLastSansyo + wSansyoPtr: 正在处理中的 下传打印初始数据 位置
        while (lpbyLastSansyo + wSansyoPtr < lpbySansyo + wSansyoSz)
        {
            switch (*(lpbyLastSansyo + wSansyoPtr)) // 验证当前字符
            {
            case 0x1B:      // ESC 处理
                //N_ESCSet();
                break;
            case 0x1D:      // GS 处理
                //N_GSSet();
                break;
            default :       // 其他字符/汉字处理
                lRet = N_MojiSet_Usb();
                break;
            }

            // 一次循环结果处理
            if (lRet != OK)
            {
                return lRet;
            }

            // 处理成功
            // 处理后占用空间数目 >= 打印数据buffer上限
            if (N_SendDataLen() >= dwHensyuBufferSize)
            {
                // 下传打印数据量过大
                if (dwHensyuBufferSize >= HENSYU_BUFF_SIZE * 3)
                {
                    devPTR->N_SetErrCode(szErrPrt_DataTooLarge);
                    return NG;
                }
                else
                {
                    // 再申请更大内存空间，上限：HENSYU_BUFF_SIZE * 3
                    if ((byHensyuBuffer = (LPBYTE)realloc(byHensyuBuffer, dwHensyuBufferSize + HENSYU_BUFF_SIZE)) != nullptr)
                    {
                        // 更新处理后打印数据buffer上限
                        dwHensyuBufferSize = dwHensyuBufferSize + HENSYU_BUFF_SIZE;
                        if ((byMojiBuffer = (LPBYTE)realloc(byMojiBuffer, dwHensyuBufferSize)) == nullptr)
                        {
                            // 申请空间失败
                            devPTR->N_SetErrCode(szErrPrt_DataRmalloc);
                            return NG;
                        }
                    }
                    else
                    {
                        // 申请空间失败
                        devPTR->N_SetErrCode(szErrPrt_DataRmalloc);
                        return NG;
                    }
                }
            }

            // 结束标记为T,数据处理结束
            if (bLineEnd == TRUE)
            {
                break;
            }
        }

        // 保留12位写结束指令
        if (N_SendDataLen() < dwHensyuBufferSize - 12)
        {
            // 处理中buffer数据 写入 可打印数据buffer
            N_AddHensyuArray(byMojiBuffer, dwMojiPtr);
            N_ClearLineInfo();
        }
        else
        {
            // 下传打印数据量过大,返回NG
            devPTR->N_SetErrCode(szErrPrt_DataTooLarge);
            return NG;
        }
    }

    return OK;
}

/*****************************************************************************
** FunctionType : public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_PrintEd_Usb                                    **
** Function     : void N_PrintImage_Edit()                                  **
** Describtion  : 打印图片处理开始                                          **
** Parameter    :                                                           **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long INJI_ED::N_PrintImage_Edit(LPBYTE lpszImgFName, ULONG ulOrgX, ULONG ulOrgY)
{
    long lRet = OK;

    HWPRTSPRPRINTIMAGE lpSprPrintImage;         // 图片数据存储结构体

    BYTE    byImageData[MAX_IMAGEFILE_SIZE];    // 图像位图数据上限:140000
    WORD    wImageWidth = 0;                    // 图片宽
    WORD    wImageHeight = 0;                   // 图片高

    LPBYTE  lpbyPageBuf = NULL;                 // 图片位图置换为打印机认可数据保存Buffer
    DWORD   dwPageBuffSz = 0;                   // lpbyPageBuf长度

    BYTE    byTmp = 0xFF;         //
    BYTE    byToNext = 0;
    WORD    wStartByteZ = 0;     // 左边距占位多少(整数)
    WORD    wStartByteY = 0;     // 左边距占位多少(余数)
    WORD    wImageWidthByte = 0;
    int     i = 0, j = 0;


    // 图片文件解析，保存到lpSprPrintImage结构体
    memset(&lpSprPrintImage, 0x00, sizeof(lpSprPrintImage));
    lpSprPrintImage.lpbImageData = byImageData;
    lpSprPrintImage.ulImageSize  = MAX_IMAGEFILE_SIZE;
    lRet = N_ImageRead((LPSTR)lpszImgFName, &lpSprPrintImage);
    if (lRet != OK)
    {
        return NG;
    }

    wImageWidth = lpSprPrintImage.wImageWidth;      // 单位：像素点
    wImageHeight = lpSprPrintImage.wImageHeight;    // 单位：像素点
    wImageWidthByte = (wImageWidth + 7) / 8;        // 图片1行占多少字节，1字节8位，+7方便取(/8)整数

    // 图片文件中位图数据置换
    // 图片文件中位图信息是按行从下到上记录，置换为从上到下，从左到右的顺序。
    // 原理：BMP文件格式 = 文件头 + 信息头 + 调色板 + 位图数据。
    //   彩色表/调色板（color table）是单色、16色和256色图像文件所特有的，相对应的调色板大小是2、16和256。
    //       调色板以4字节为单位，每4个字节存放一个颜色值，图像的数据是指向调色板的索引。
    //   位图数据记录了位图的每一个像素值，记录顺序是：扫描行内是从左到右,扫描行之间是从下到上。
    //       位图的一个像素值所占的字节数: 当biBitCount=1时，8个像素占1个字节; 当biBitCount=4时，2个像素占1个字节;
    // 　　  当biBitCount=8时，1个像素占1个字节; 当biBitCount=24时,1个像素占3个字节; 当biBitCount=32时,1个像素占4个字节。
    //       Windows规定一个扫描行所占的字节数必须是4的倍数(即以long为单位),不足的以0填充，所以位图数据的大小计算公式应该为
    //        ：bmppitch = ((biWidth * bitCountPerPix + 31) >> 5) << 2 (>>5即÷32，>>2即÷8)
    //   打印机支持色深为单色=1，即8个像素占1个字节，4的倍数为32，按4字节递增。即32像素4个字节，33像素占8个字节。调色板为8字节（2位调色板）
    N_ImageDataAnalyse(&lpSprPrintImage);

    // 根据可打印宽度和图片高度申请内存空间
    dwPageBuffSz = wImageHeight * wLineWidthByte;
    lpbyPageBuf = (BYTE *)malloc(dwPageBuffSz);
    if (lpbyPageBuf == nullptr)
    {
        devPTR->N_SetErrCode(szErrPrt_ImgCrtBuff);
        return NG;
    }
    memset(lpbyPageBuf, 0x00, dwPageBuffSz);

    // 位图数据写入打印Buffer
    //wStartByteZ = MM2HS(ulOrgX <= wLeft ? 0 : ulOrgX - wLeft) / 8;
    //wStartByteY = MM2HS(ulOrgX <= wLeft ? 0 : ulOrgX - wLeft) % 8;

    wStartByteZ = 0;//MM2HS(wLeft) / 8;
    wStartByteY = 0;//MM2HS(wLeft) % 8;
    byToNext = 0;
    for (i = 0; i < wImageHeight; i++) // 行循环处理
    {
        for (j = 0; j < wImageWidthByte; j++)
        {
            lpbyPageBuf[i * wLineWidthByte + wStartByteZ + j] |= (lpSprPrintImage.lpbImageData[i * wImageWidthByte + j] >> wStartByteY);
            lpbyPageBuf[i * wLineWidthByte + wStartByteZ + j] |= byToNext;

            byToNext = (lpSprPrintImage.lpbImageData[i * wImageWidthByte + j] << (8 - wStartByteY));   // 图片一行占几个字节] << (8-wTmp);
        }

        if (j < wLineWidthByte)
            lpbyPageBuf[i * wLineWidthByte + wStartByteZ + j] |= byToNext;
    }

    // 处理入参坐标：< 上边距/左边距，缺省为上边距/左边距
    // 打印坐标是从图片底边算起，因此上边距需要加上图片高度
    ulOrgX = (ulOrgX <= wLeft ? wLeft : ulOrgX);
    ulOrgY = (ulOrgY <= wTop ? wTop + HS2MM(wImageHeight) : ulOrgY);

    // 设置行列绝对位置打印坐标
    N_SetAbsolutePos(ulOrgX, ulOrgY - HS2MM(wImageHeight));
    wYPosition = ulOrgY;

    // 位图数据写入处理后打印数据buffer前，先进行光栅位设置
    byHensyuBuffer[dwHensyuPtr++] = 0x1D;
    byHensyuBuffer[dwHensyuPtr++] = 0x76;
    byHensyuBuffer[dwHensyuPtr++] = 0x30;
    byHensyuBuffer[dwHensyuPtr++] = 0x30;
    byHensyuBuffer[dwHensyuPtr++] = wLineWidthByte % DIVREM_COMP;   // 可用打印宽(mm)
    byHensyuBuffer[dwHensyuPtr++] = wLineWidthByte / DIVREM_COMP;
    byHensyuBuffer[dwHensyuPtr++] = wImageHeight % DIVREM_COMP;     // 实际打印高(0.125mm)
    byHensyuBuffer[dwHensyuPtr++] = wImageHeight / DIVREM_COMP;

    // 位图数据写入处理后打印数据buffer
    N_AddHensyuArray(lpbyPageBuf, dwPageBuffSz);

    // 释放位图数据buffer空间
    if (lpbyPageBuf != NULL)
    {
        free(lpbyPageBuf);
        lpbyPageBuf = NULL;
    }

    // 行坐标 > 已设置最大行坐标时，更新已设置最大行坐标
    if (ulOrgY > wYPosCount)
    {
        wYPosCount = ulOrgY;
    }
    return OK;

}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void N_AddPageModeStart()                                 **
** Describtion  : 设置页打印模式                                            **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddPageModeStart()
{
    // 打印数据集合buffer数据清空
    N_FreeBuffer();
    N_FreeMojiBuffer();

    // 打印数据集合buffer数据初始化
    N_InitBuffer();
    N_InitMojiBuffer();

    // 设置打印区域
    N_AddCmdHensyuArray(CMDF_PAGEMODE_SET_1B57);
    wPageModePtr = dwHensyuPtr - 8;

    // 设置页打印模式
    N_AddCmdHensyuArray(CMDF_PAGEMODE_ON_1B4C);

    wXPosition = 0;     // 列指针
    wYPosition = 0;     // 行指针
    wYPosCount = 0;     // 行最大指针

    // 设置换行量
    N_AddCmdHensyuArray(CMDF_GYOKANSPACE_1B3320);

    // 设置文字右空格量
    N_AddCmdHensyuArray(CMDF_MOJIKANSPACE_1B2000);

    // 页模式垂直绝对位置: 18
    N_AddCmdHensyuArray(CMDF_PAGE_KIJUN_1D2418);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::                                                 **
** Function     : void N_AddLineModeStart()                                 **
** Describtion  : 设置行打印模式                                            **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_AddLineModeStart()
{
    // 打印数据集合buffer数据清空
    N_FreeBuffer();
    N_FreeMojiBuffer();

    // 打印数据集合buffer数据初始化
    N_InitBuffer();
    N_InitMojiBuffer();

    // 标准模式选择(1B 53)
    N_AddCmdHensyuArray(CMDF_LINEMODE_ON_1B53);

    // 设置左边距(1D 4C)
    N_AddCmdHensyuArray(CMDF_LEFT_MARGIN_1D4C00);

    // 设置换行量(1B 33 XX:20)
    N_AddCmdHensyuArray(CMDF_GYOKANSPACE_1B3320);

    // 设置文字右空格量(1B 20 00)
    N_AddCmdHensyuArray(CMDF_MOJIKANSPACE_1B2000);
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_SetAbsolutePos                                 **
** Function     : void N_SetAbsolutePos()                                   **
** Describtion  : 设置行列绝对位置                                          **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_SetAbsolutePos(ULONG ulOrgX, ULONG ulOrgY)
{
    if (ulOrgX > 0)
    {
        // 设置横绝对位置
        N_AddCmdHensyuArray(CMDF_PAGE_JPOSX_INIT_1B24);
        byHensyuBuffer[dwHensyuPtr - 2] = MM2HS(GETLEFT(ulOrgX)) % DIVREM_COMP;
        byHensyuBuffer[dwHensyuPtr - 1] = MM2HS(GETLEFT(ulOrgX)) / DIVREM_COMP;
    }

    if (ulOrgY > 0)
    {
        // 设置纵绝对位置
        N_AddCmdHensyuArray(CMDF_PAGE_JPOSY_INIT_1D24);
        byHensyuBuffer[dwHensyuPtr - 2] = (BYTE)(MM2HS(GETTOP2(ulOrgY)) % DIVREM_COMP);
        byHensyuBuffer[dwHensyuPtr - 1] = (BYTE)(MM2HS(GETTOP2(ulOrgY)) / DIVREM_COMP);
    }
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_SetRelativePos                                 **
** Function     : void N_SetRelativePos()                                   **
** Describtion  : 设置行列相对位置                                          **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_SetRelativePos(ULONG ulOrgX, ULONG ulOrgY)
{
    // 设置相对位置
    N_AddCmdHensyuArray(CMDF_PAGE_XPOSX_INIT_1B5C);
    byHensyuBuffer[dwHensyuPtr - 2] = MM2HS(ulOrgX) % DIVREM_COMP;
    byHensyuBuffer[dwHensyuPtr - 1] = MM2HS(ulOrgX) / DIVREM_COMP;

    // 设置相对位置
    N_AddCmdHensyuArray(CMDF_PAGE_XPOSY_INIT_1D5C);
    byHensyuBuffer[dwHensyuPtr - 2] = MM2HS(ulOrgY) % DIVREM_COMP;
    byHensyuBuffer[dwHensyuPtr - 1] = MM2HS(ulOrgY) / DIVREM_COMP;
}

/*****************************************************************************
** FunctionType : protected                                                 **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_MojiSet_Usb                                    **
** Function     : void N_MojiSet_Usb()                                      **
** Describtion  : 字符/汉字处理                                             **
** Parameter    :                                                           **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long INJI_ED::N_MojiSet_Usb()
{
    BYTE    byK1, byK2, byK3;               // 字符/汉字处理中保存
    WORD    wDotID;                         // 3位汉字合成
    WORD    wPixelRows = 24;
    WORD    wPixelCols = 24;
    BYTE    byFontBuff[128];                // 字符图像打印数据Buffer
    WORD    wFontBuffSz  = 0;               // FreeType处理后位图空间长度，字符36，汉字72,对应byFontBuff
    WORD    wFontMoveSz  = 0;               // 处理后跳过的个数，字符1，汉字3
    WORD    wFtBmpTop = 0;                  // FT生成图像有效底边与上边的距离(FT图像TOP)

    WORD    wBmpBelowblank = 0;  // 打印字符图像下留白按24*24标准的1/8行(字符图像准下留白)(像素行)
    WORD    wBmpTop2Belank = 0;             // 当前字符图像FT图像TOP与字符图像准下留白的距离(TOP距留白)
    long    lRtn = OK;                      // 返回值
    int     i, j;

    wFontBuffSz = wPixelRows * wPixelCols / 8;
    wBmpBelowblank = wPixelCols / 8;

    // 循环处理
    switch (*(lpbyLastSansyo + wSansyoPtr))
    {
    case 0x0A:  // 换行
    case 0x0D:  // 回车
        {
            // 如果是回车换行符，归为1位处理，处理指针后移2位
            if ((lpbyLastSansyo + wSansyoPtr + 1) != nullptr)
            {
                if ((*(lpbyLastSansyo + wSansyoPtr) == 0x0D && *(lpbyLastSansyo + wSansyoPtr + 1) == 0x0A) ||
                    (*(lpbyLastSansyo + wSansyoPtr) == 0x0A && *(lpbyLastSansyo + wSansyoPtr + 1) == 0x0D))
                {
                    wSansyoPtr++;
                    lpbyLastSansyo += wSansyoPtr;
                    wSansyoPtr = 0;
                    break;
                }
            }
            N_AddMojiChar(0x0A);
            wSansyoPtr++;       // 处理指针+1
            //bLineEnd = TRUE;    // 可换行标记/结束标记为T
            lpbyLastSansyo  += wSansyoPtr;
            wSansyoPtr = 0;
            break;
        }
    case 0x00:  //NUL
        bLineEnd = TRUE;    // 可换行标记/结束标记为T
        wSansyoPtr++;
        break;
    case 0xA0: // 半角
        wSansyoPtr++;
        break ;
    case 0x7F:// 半角(JIS8)文字(7F)
        wSansyoPtr++;
        break;
    default :
        {
            // 字符、汉字处理
            byK1 = *(lpbyLastSansyo + wSansyoPtr);
            byK2 = *(lpbyLastSansyo + wSansyoPtr + 1);
            byK3 = *(lpbyLastSansyo + wSansyoPtr + 2);

            // 验证当前位置是字符还是汉字
            long lRetCode = lDGetCodeType(byK1, byK2);
            if (lRetCode == NG)    // 字符
            {
                lRetCode = ctAnk;
                wFontBuffSz = wFontBuffSz / 2;
                wFontMoveSz = 1;
            }
            else
            {
                lRetCode = ctGaiji;
                //wFontBuffSz = 72;
                wFontMoveSz = 3;

                // 组合汉字
                unsigned int iH = (unsigned int)(byK1 & 0x0F) << 12;        // BIN: 0000,1111
                unsigned int iM = (unsigned int)(((byK2 & 0x3C) >> 2)) << 8; // BIN: 0011,1100
                unsigned int iL = ((byK2 & 0x03) << 6) + (byK3 & 0x03f);    // BIN: 0000,0011;00111111
                wDotID = iH + iM + iL;
            }




            // FreeType 处理
            FT_Library library;
            FT_Face ftFace;
            FT_Error ftError;
            int iRow = 0, iCol = 0;    // 行,列

            // 对FreeType库初始化，并读取矢量库文件
            ftError = FT_Init_FreeType(&library);
            if (ftError != 0)
            {
                devPTR->N_SetErrCode(szErrFT_Init);
                return NG;
            }

            // 创建外观对象，成功返回0
            ftError = FT_New_Face(library, (char *)byFontType, 0, &ftFace);
            if (ftError != 0)
            {
                devPTR->N_SetErrCode(szErrFT_New_Face);
                return NG;
            }

            // 设置当前字体属性，19*26像素(宽×高),装在字形前设置
            ftError = FT_Set_Pixel_Sizes(ftFace, wPixelCols, wPixelRows);
            if (ftError != 0)
            {
                devPTR->N_SetErrCode(szErrFT_Pixel_Sizes);
                return NG;
            }

            // 在face里倍选中的字符表中查找与给出的字符码对应的字形索引
            FT_UInt uiGlyphindex;
            if (lRetCode == ctAnk)
                uiGlyphindex = FT_Get_Char_Index(ftFace, byK1);
            else
                uiGlyphindex = FT_Get_Char_Index(ftFace, wDotID);

            // 自行索引读取到字形槽(glyph slo)中，装载对应字形图像
            ftError = FT_Load_Glyph(ftFace, uiGlyphindex, FT_LOAD_DEFAULT);
            if (ftError != 0)
            {
                devPTR->N_SetErrCode(szErrFT_Load_Glyph);
                return NG;
            }



            // 字形转换为位图
            ftError = FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_MONO);
            //ftError = FT_glyph_(ftFace->glyph, FT_RENDER_MODE_MONO);
            if (ftError != 0)
            {
                devPTR->N_SetErrCode(szErrFT_Render_Glyph);
                return NG;
            }

            // 把FT生成的位图导入Buffer。
            // 按[列][行]排列，图像数据规律：左旋90度，上为左，下为右，图像反向。
            // 每1个字节保存1位图像数据。
            BYTE byData[24][24] = {0};  // [列][行]
            for (iRow = 0; iRow < (ftFace->glyph->bitmap.rows); iRow ++)
            {
                for (iCol = 0; iCol < ftFace->glyph->bitmap.width; iCol ++)
                {
                    if ((ftFace->glyph->bitmap.buffer[iRow * ftFace->glyph->bitmap.pitch + iCol / 8] &
                         (0x80 >> (iCol % 8))) == 0)
                    {
                        byData[iCol][iRow] = 0x00;
                    }
                    else
                    {
                        byData[iCol][iRow] = 0x01;
                    }
                }
            }

            //wFtBmpTop = ftFace->glyph->bitmap_top;      // FT图像TOP
            wFtBmpTop = ftFace->glyph->bitmap.rows;
            wBmpTop2Belank = (wPixelCols - wBmpBelowblank) - wFtBmpTop;     // TOP距留白

            // 关闭及释放FT
            FT_Done_Face(ftFace);
            FT_Done_FreeType(library);

            // 粗体字设置，列右移一个像素单位与原位或
            if (wFontStyle == PTR_TEXT_BOLD)
            {
                BYTE byData2[24][24] = {0};
                for (j = 0; j < wPixelRows; j ++)
                {
                    for (i = 0; i < wPixelCols; i ++)
                    {
                        byData2[i][j] = byData[i][j];
                    }

                    /*for(i = 0; i < wPixelCols - 1; i ++)
                    {
                        byData2[i][j] = byData2[i][j] | byData[i + 1][j];
                    }*/

                    for (i = 1; i < wPixelCols - 1; i ++)
                    {
                        byData2[i][j] = byData2[i][j] | byData[i - 1][j];
                    }
                }

                for (j = 0; j < wPixelRows; j ++)
                {
                    for (i = 0; i < wPixelCols; i ++)
                    {
                        byData[j][i] = byData2[j][i];
                    }
                }
            }

            BOOL bNeedPreBlankHdl = FALSE;
            if((lRetCode == ctAnk) ||
               (wDotID == 0xFF0C) || (wDotID == 0x3002) ||      //，。
               (wDotID == 0xFF1A) || (wDotID == 0xFF01) ||      //: ！
               (wDotID == 0xFF1F) || (wDotID == 0xFF1B) ||      //? ;
               (wDotID == 0x3001) || (wDotID == 0x2014) ||      //、 破折号
               (wDotID == 0x2026) || (wDotID == 0x2013) ||      //省略号 连接号
               (wDotID == 0xFF0E)) {                            //间隔号
                bNeedPreBlankHdl = TRUE;
            }

            // 字符位图留白处理(汉字+字符)
            // FT生成的字符图像数据按上对齐，特殊字符如‘=’、‘.’等贴近上边界。
            // 导入打印buffer前需要消除下留白，修正特殊字符贴近下边界。
            // 算法：按行从下到上处理，起始行=总行数-下留白行数，
            //       起始行-TOP距留白的行按列 与 0x00 进行 同位列与(&) 操作，
            //       结果赋值对应起始行的列，赋值后，前者置为0x00。
            //       IF 起始行-TOP距留白的行按列 < 0,即已超过最顶行，break跳出
            //                if(lRetCode == ctAnk)        // 字符非汉字
            if(bNeedPreBlankHdl)
            {
                for (j = wPixelCols - wBmpBelowblank - 1; j >= 0; j --)   // 按列顺序处理
                {
                    if (j - wBmpTop2Belank < 0)
                        break;

                    for (i = 0; i < wPixelRows; i ++)
                    {
                        byData[i][j] = byData[i][j - wBmpTop2Belank] & 0xFF;
                        byData[i][j - wBmpTop2Belank] = 0x00;
                    }
                }
            }

            // 图像数据导入打印Buffer。
            // 规律：图像开始按列写入，第1列从上到下，顺序第2列、第3列...
            //       每8位图像数据组成1个字节写入1字节Buffer，即：
            //       图像数据保存Buffer中每8个字节从高位到低位存入一个字节，
            //       24＊24图像，字符占一半，汉字用全部，
            //       计算后: 字符生成36字节打印数据，汉字生成72字节打印数据。
            for (i = 0; i < wFontBuffSz; i ++)
            {
                BYTE byWorkData = 0;
                for (j = 0; j < 8; j ++)
                {
                    byWorkData += (0x01 & byData[i / 3][(i % 3) * 8 + j]) << (7 - j);
                }
                byFontBuff[i] = byWorkData;
            }

            // 外字登录(Moji/Gaiji Buffer)
            N_AddImageTouroku(byFontBuff, wFontBuffSz);

            wSansyoPtr += wFontMoveSz ;

            // 按文字左边距设置相对位置右移
            N_AddMojiChar(0x1B);
            N_AddMojiChar(0x5C);
            N_AddMojiChar(MM2HS(wMojiDistance));
            N_AddMojiChar(0x00);
        }
    }

    return OK;
}

/*****************************************************************************
** FunctionType : protected                                                 **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_ImageRead                                      **
** Function     : void INJI_ED::N_ImageRead()                               **
** Describtion  : 读取图片内容                                              **
** Parameter    :                                                           **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long INJI_ED::N_ImageRead(LPSTR lpszImgFName, LPHWPRTSPRPRINTIMAGE lpSprPrintImage)
{
    long    lRtn = OK;

    // 图片文件信息头
    BmpFileHeader    stBmpFileHeader;

    // 位图信息头
    BmpInfoHeader    stBmpInfoHeader;

    // 读文件信息头
    lRtn = N_FileRead_Bin(lpszImgFName, 0, sizeof(stBmpFileHeader), (LPSTR)&stBmpFileHeader);
    if (lRtn != sizeof(stBmpFileHeader)) // 文件信息头错误
    {
        devPTR->N_SetErrCode(szErrPrt_ImgReadNotH);
        return NG;
    }
    //if(memcmp((char*)(stBmpFileHeader.byBM), "BM", 2) != 0)  // 非BMP格式
    if (stBmpFileHeader.byBM != 0x4D42)
    {
        devPTR->N_SetErrCode(szErrPrt_ImgNotBmp);
        return NG;
    }
    if (stBmpFileHeader.ulOffset != 0x3E) // BMP文件无效
    {
        devPTR->N_SetErrCode(szErrPrt_ImgInvalid);
        return NG;
    }

    // 读位图信息头
    lRtn = N_FileRead_Bin(lpszImgFName, sizeof(stBmpFileHeader), sizeof(stBmpInfoHeader), (LPSTR)&stBmpInfoHeader);
    if (lRtn != sizeof(stBmpInfoHeader)) // 文件位图信息头错误
    {
        devPTR->N_SetErrCode(szErrPrt_ImgReadNotBH);
        return NG;
    }
    if (stBmpInfoHeader.ulHeaderSize != 0x28) // 位图信息头长度无效
    {
        devPTR->N_SetErrCode(szErrPrt_ImgBHeadLen);
        return NG;
    }
    if ((stBmpInfoHeader.ulBmpWidth  > MAX_PAGEWIDE || stBmpInfoHeader.ulBmpHeight > MAX_PAGEHEIGHT) ||
        (stBmpInfoHeader.ulBmpWidth  == 0 || stBmpInfoHeader.ulBmpHeight == 0))  // 文件过大/过小
    {
        devPTR->N_SetErrCode(szErrPrt_ImgBSzInvalid);
        return NG;
    }
    if (stBmpInfoHeader.wPlanes != 1 || stBmpInfoHeader.wColorBits != 1) // 不是单色BMP文件
    {
        devPTR->N_SetErrCode(szErrPrt_ImgBNotMono);
        return NG;
    }
    if (stBmpInfoHeader.ulCompression != 0) // 压缩格式的BMP
    {
        devPTR->N_SetErrCode(szErrPrt_ImgBIsCompre);
        return NG;
    }

    if (lpSprPrintImage->ulImageSize < stBmpInfoHeader.ulImageSize) // 图像大小>预定存储空间大小
    {
        devPTR->N_SetErrCode(szErrPrt_ImgBSzBig);
        return NG;
    }

    // 读图像数据
    lRtn = N_FileRead_Bin(lpszImgFName, 0, stBmpInfoHeader.ulImageSize, (LPSTR)(lpSprPrintImage->lpbImageData));
    if ((ULONG)lRtn != stBmpInfoHeader.ulImageSize) // 无效的图像数据
    {
        devPTR->N_SetErrCode(szErrPrt_ImgBDInvaild);
        return NG;
    }

    lpSprPrintImage->ulImageSize    = stBmpInfoHeader.ulImageSize;  // 图像大小
    lpSprPrintImage->wImageWidth    = stBmpInfoHeader.ulBmpWidth;  // 图像宽
    lpSprPrintImage->wImageHeight   = stBmpInfoHeader.ulBmpHeight; // 图像高

    return OK;
}

/*****************************************************************************
** FunctionType : protected                                                 **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_FileRead_Bin                                   **
** Function     : void INJI_ED::N_FileRead_Bin()                            **
** Describtion  : 读文件内容                                                **
** Parameter    :                                                           **
** Return       : OK:成功/NG:错误                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
long INJI_ED::N_FileRead_Bin(LPSTR lpszFileName, DWORD wOffset, DWORD wFileSize, LPSTR lpFileData)
{
    FILE    *fp = nullptr;
    UINT    nBytesRead = 0;

    fp = fopen(lpszFileName, "rb");
    if (fp == nullptr) // 文件Open错误
    {
        devPTR->N_SetErrCode(szErrPrt_FileNotOpen);
        return NG;
    }

    if (wOffset > 0)
    {
        if (fseek(fp, wOffset, SEEK_SET) != 0)
        {
            // 文件位移失败
            fclose(fp);
            devPTR->N_SetErrCode(szErrPrt_FileNotSeek);
            return NG;
        }
    }

    nBytesRead = fread(lpFileData, 1, wFileSize, fp);

    fclose(fp);

    return nBytesRead;
}

/*****************************************************************************
** FunctionType : protected                                                 **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    : INJI_ED                                                   **
** Symbol       : INJI_ED::N_ImageDataAnalyse                               **
** Function     : void INJI_ED::N_ImageDataAnalyse()                        **
** Describtion  : 位图数据置换为正常的行列模式排列                          **
** Parameter    :                                                           **
** Return       : 无                                                        **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
void INJI_ED::N_ImageDataAnalyse(HWPRTSPRPRINTIMAGE *lpSprPrintImage)
{
    long    lOffset = 0;        // lpbyAnalyseTemp偏移位
    WORD    wY, wX;
    BYTE    byImDt;             // 每位字节处理变量
    INT32   ulCLUT[2];          // 调色板
    LPBYTE  lpbyAnalyseTemp;    // 处理后临时存放Buffer

    LPBYTE lpbyImageData = lpSprPrintImage->lpbImageData;
    ULONG ulBmpHeight = lpSprPrintImage->wImageHeight;
    ULONG ulBmpWidth = lpSprPrintImage->wImageWidth ;

    //计算图片每行占多少字节(对于Image,必须按照每行32来计算，行间距可变的处理无法适用)
    WORD wIntervalY = WORD((ulBmpWidth + (IMAGE_LINE - 1)) / IMAGE_LINE) * IMAGE_LINE / 8;

    // 处理前赋值
    lpbyAnalyseTemp = new BYTE[lpSprPrintImage->ulImageSize];
    memcpy(ulCLUT, lpbyImageData + sizeof(BmpFileHeader) + sizeof(BmpInfoHeader), sizeof(ulCLUT));
    lpbyImageData = lpbyImageData + sizeof(BmpFileHeader) + sizeof(BmpInfoHeader) + sizeof(ulCLUT);
    lpSprPrintImage->ulImageSize = lpSprPrintImage->ulImageSize - sizeof(BmpFileHeader) - sizeof(BmpInfoHeader) - sizeof(ulCLUT);

    memset(lpbyAnalyseTemp, 0x00, sizeof(lpbyAnalyseTemp));

    // 循环置换位图
    for (wY = 0; wY < ulBmpHeight; wY++) // 行循环
    {
        for (wX = 0; wX < WORD(ulBmpWidth / 8); wX++) // 列循环，标准行占位处理(准确4×8字节)
        {
            if (ulCLUT[0] == 0x00000000)   // 根据调色板确定该位黑白(0:黑)
            {
                byImDt = lpbyImageData[wIntervalY * (ulBmpHeight - wY - 1) + wX] ^ 0xFF;
            }
            else
            {
                byImDt = lpbyImageData[wIntervalY * (ulBmpHeight - wY - 1) + wX];
            }
            lpbyAnalyseTemp[lOffset] = byImDt; lOffset++;
        }

        // 以下是非标准行占位处理(非准确4×8字节)
        if (ulBmpWidth % 8 == 0)
        {
            continue;
        }
        if (ulCLUT[0] == 0x00000000)
        {
            byImDt = lpbyImageData[wIntervalY * (ulBmpHeight - wY - 1) + WORD(ulBmpWidth / 8)] ^ 0xFF;
        }
        else
        {
            byImDt = lpbyImageData[wIntervalY * (ulBmpHeight - wY - 1) + WORD(ulBmpWidth / 8)];
        }
        switch (ulBmpWidth % 8)
        {
        case 1: byImDt &= 0x80; break;
        case 2: byImDt &= 0xC0; break;
        case 3: byImDt &= 0xE0; break;
        case 4: byImDt &= 0xF0; break;
        case 5: byImDt &= 0xF8; break;
        case 6: byImDt &= 0xFC; break;
        case 7: byImDt &= 0xFE; break;
        }
        lpbyAnalyseTemp[lOffset] = byImDt; lOffset++;
    }

    // 置换后的位图数据写回位图空间
    memcpy(lpSprPrintImage->lpbImageData, lpbyAnalyseTemp, lpSprPrintImage->ulImageSize);
    delete[] lpbyAnalyseTemp;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    :                                                           **
** Symbol       :                                                           **
** Function     : int WINAPI _mbbtype()                                     **
** Describtion  : 外部函数，用来判断入参字节类型(是否多字节)                **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
extern "C"
int WINAPI _mbbtype(unsigned char cChr, int iType)
{

    int iRtn = -1;
    unsigned char iCode = cChr >> 4;
    if (iCode == 0x0E)
    {
        iRtn = _MBC_LEAD;
    }
    else
    {
        iRtn = 0;
    }
    return iRtn;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : DevPTR_RPR.CPP                                            **
** ClassName    :                                                           **
** Symbol       :                                                           **
** Function     : int WINAPI lDGetCodeType()                                **
** Describtion  : 外部函数，按高低字节传入验证是否单字节或多字节前导字符    **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      :                                                           **
*****************************************************************************/
extern "C"
long WINAPI lDGetCodeType(BYTE byHigh, BYTE byLow)
{
    WORD wKCode;

    wKCode = (byHigh << 8) + byLow;
    if (wKCode >= 0xA1A1u && wKCode <= 0xFEFEu)
    {
        return GAIJI;
    }
    else
    {
        switch (_mbbtype(byHigh, 0))           // 检查第1字节(Byte)
        {
        case _MBC_LEAD:                                // 属于2字节文字的第1字节
            if (_mbbtype(byLow, 1) == _MBC_ILLEGAL)               // 2byte目异常？
                return GAIJI;                                       // 外字
            wKCode = (byHigh << 8) + byLow;
            if (wKCode >= 0x8140u && wKCode <= 0x81ACu ||            // JIS第１水准&
                wKCode >= 0x81B8u && wKCode <= 0x81BFu ||         //    第２水准？
                wKCode >= 0x81C8u && wKCode <= 0x81CEu ||         //
                wKCode >= 0x81DAu && wKCode <= 0x81E8u ||         //
                wKCode >= 0x81F0u && wKCode <= 0x81F7u ||         //
                wKCode >= 0x81FCu && wKCode <= 0x81FCu ||         //
                wKCode >= 0x824Fu && wKCode <= 0x8258u ||         //
                wKCode >= 0x8260u && wKCode <= 0x8279u ||         //
                wKCode >= 0x8281u && wKCode <= 0x829Au ||         //
                wKCode >= 0x829Fu && wKCode <= 0x82F1u ||         //
                wKCode >= 0x8340u && wKCode <= 0x8396u ||         //
                wKCode >= 0x839Fu && wKCode <= 0x83B6u ||         //
                wKCode >= 0x83BFu && wKCode <= 0x83D6u ||         //
                wKCode >= 0x8440u && wKCode <= 0x8460u ||         //
                wKCode >= 0x8470u && wKCode <= 0x847Eu ||         //
                wKCode >= 0x8480u && wKCode <= 0x8491u ||         //
                wKCode >= 0x849Fu && wKCode <= 0x84BEu ||         //
                wKCode >= 0x889fu && wKCode <= 0x9872u ||         //
                wKCode >= 0x989fu && wKCode <= 0x9ffcu ||         //
                wKCode >= 0xe040u && wKCode <= 0xeaa4u)           //
            {
                return OK;
            }
            else
            {
                if ((wKCode >= 0xfa40) && (wKCode <= 0xfc4b))
                {
                    return IBMKANJI;                            // IBM汉字
                }
                else
                {
                    return GAIJI;                               // 外字
                }
            }
        case _MBC_SINGLE:                                     // 单字节文字
        default:                                              // 无法验证
            return NG;
        }
    }
}
