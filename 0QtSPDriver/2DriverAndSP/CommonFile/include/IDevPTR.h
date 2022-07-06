#pragma once
/***************************************************************
* 文件名称：IDevPTR.h
* 文件描述：声明打印机底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1

* 版本历史信息2
* 变更说明：修改文件，增加新函数，结构体，全局变量声明
* 变更日期：2019年8月1日
* 文件版本：1.0.0.2
****************************************************************/


//#include "memory.h"
//#include <QColor>

#include <string.h>
#include <QtCore/qglobal.h>

#if defined IDEVPTR_LIBRARY
#define DEVPTR_EXPORT     Q_DECL_EXPORT
#else
#define DEVPTR_EXPORT     Q_DECL_IMPORT
#endif


// -----------------功能入口函数返回值-------------------------------------------------------
//成功
#define ERR_PTR_SUCCESS             (0)     // 操作成功
//状态错误
#define ERR_PTR_PARAM_ERR           (-1)    // 参数错误
#define ERR_PTR_COMM_ERR            (-2)    // 通讯错误
#define ERR_PTR_NO_PAPER            (-3)    // 打印机缺纸
#define ERR_PTR_JAMMED              (-4)    // 堵纸等机械故障
#define ERR_PTR_NOT_OPEN            (-6)    // 设备没有打开
#define ERR_PTR_HEADER              (-7)    // 打印头故障
#define ERR_PTR_CUTTER              (-8)    // 切刀故障
#define ERR_PTR_TONER               (-9)    // INK或色带故障
#define ERR_PTR_STACKER_FULL        (-10)   // 用户没有取走
#define ERR_PTR_NO_RESUME           (-11)   // 不可恢复的错误
#define ERR_PTR_CAN_RESUME          (-12)   // 可恢复的错误
#define ERR_PTR_FORMAT_ERROR        (-13)   // 打印字串格式错误
#define ERR_PTR_CHRONIC             (-14)   // 慢性故障
#define ERR_PTR_HWERR               (-15)   // 硬件故障
#define ERR_PTR_IMAGE_ERROR         (-16)   // 打印图片相关错误
#define ERR_PTR_NO_DEVICE           (-17)   // 如果指定名的设备不存在，CreatePrinterDevice返回
#define ERR_PTR_UNSUP_CMD           (-18)   // 不支持的指令
#define ERR_PTR_DATA_ERR            (-19)   // 收发数据错误
#define ERR_PTR_TIMEOUT             (-20)   // 超时
#define ERR_PTR_DRVHND_ERR          (-21)   // 驱动错误
#define ERR_PTR_DRVHND_REMOVE       (-22)   // 驱动丢失
#define ERR_PTR_USB_ERR             (-23)   // USB错误
#define ERR_PTR_TIMEOUT             (-25)   // 超时
#define ERR_PTR_OTHER               (-26)   // 其它错误，如调用API错误等


// -----------------程序内部处理错误码-------------------------------------------------------
// 标示程序内部处理错误位置
const unsigned char  szErrInit_Succ[7]            = "000000"; // 初始化为6个0(无错误)
// USB处理错误码
const unsigned char  szErrUSB_DLLLoad[7]          = "A00001"; // USB驱动链接库加载失败
const unsigned char  szErrUSB_OpenCall[7]         = "A00002"; // USB下发Open指令失败
const unsigned char  szErrUSB_OpenBack[7]         = "A00003"; // USB下发Open指令回调函数处理失败
const unsigned char  szErrUSB_HadInval[7]         = "A00004"; // USB设备句柄无效
const unsigned char  szErrUSB_DevStop[7]          = "A00005"; // USB设备处于停止状态
const unsigned char  szErrUSB_OReset[7]           = "A00006"; // USBOpen后Reset失败
const unsigned char  szErrUSB_ResetCall[7]        = "A00007"; // USB下发Reset指令失败
const unsigned char  szErrUSB_ResetBack[7]        = "A00008"; // USB下发Open指令回调函数处理失败
const unsigned char  szErrUSB_CutCall[7]          = "A00009"; // USB下发CutPaper指令失败
const unsigned char  szErrUSB_CutBack[7]          = "A00010"; // USB下发CutPaper指令回调函数处理失败
const unsigned char  szErrUSB_SndCall[7]          = "A00011"; // USB下发SndData指令失败
const unsigned char  szErrUSB_SndBack[7]          = "A00012"; // USB下发SndData指令回调函数处理失败
const unsigned char  szErrUSB_StatusCall[7]       = "A00013"; // USB下发status指令失败
const unsigned char  szErrUSB_SnsCall[7]          = "A00014"; // USB下发Sns指令失败
const unsigned char  szErrUSB_SnsBack[7]          = "A00015"; // USB下发Sns指令回调函数处理失败
const unsigned char  szErrUSB_GetSnsStatus[7]     = "A00016"; // 获取Sonser状态失败
const unsigned char  szErrUSB_SnsTimeOut[7]       = "A00017"; // USB下发Sns指令获取状态超时
const unsigned char  szErrUSB_DLLFunInval[7]      = "A00018"; // USB驱动链接库函数无效(NULL)
const unsigned char  szErrUSB_EnumDevInit[7]      = "A00019"; // 枚举USB设备初始化失败(libusb_init)
const unsigned char  szErrUSB_EnumGetDevList[7]   = "A00020"; // 枚举USB设备获取列表失败(libusb_get_device_list)
const unsigned char  szErrUSB_EnumGetDevDesc[7]   = "A00021"; // 枚举USB设备获取设备信息描述符失败(libusb_get_device_descriptor)
const unsigned char  szErrUSB_DevNotOpen[7]       = "A00022"; // 设备未Open
// 文本图片处理错误码
const unsigned char  szErrPrt_NoSetMode[7]        = "B00001"; // 未设置打印模式
const unsigned char  szErrPrt_ImgReadNotH[7]      = "B00002"; // 未读到BMP图片文件信息头
const unsigned char  szErrPrt_ImgNotBmp[7]        = "B00003"; // BMP图片非Bmp格式
const unsigned char  szErrPrt_ImgInvalid[7]       = "B00004"; // BMP图片文件信息头位移量无效
const unsigned char  szErrPrt_ImgReadNotBH[7]     = "B00005"; // 未读到BMP图片位图信息头
const unsigned char  szErrPrt_ImgBHeadLen[7]      = "B00006"; // 读到的BMP图片信息头长度错误
const unsigned char  szErrPrt_ImgBSzInvalid[7]    = "B00007"; // 读到的BMP图片信息头记录图片过大/过小
const unsigned char  szErrPrt_ImgBNotMono[7]      = "B00008"; // BMP图片非单色位图
const unsigned char  szErrPrt_ImgBIsCompre[7]     = "B00008"; // BMP图片不能是压缩格式
const unsigned char  szErrPrt_ImgBSzBig[7]        = "B00009"; // BMP图片过大,请保持在140000字节以内
const unsigned char  szErrPrt_ImgBDInvaild[7]     = "B00010"; // BMP图片位图数据无效
const unsigned char  szErrPrt_ImgCrtBuff[7]       = "B00011"; // BMP图片处理空间申请失败
const unsigned char  szErrPrt_FileNotOpen[7]      = "B00012"; // 文件Open失败,无法获取数据
const unsigned char  szErrPrt_FileNotSeek[7]      = "B00013"; // 文件指针位移失败,无法获取数据
const unsigned char  szErrPrt_FileNotFound[7]     = "B00014"; // 文件不存在
const unsigned char  szErrPrt_DataTooLarge[7]     = "B00015"; // 下传打印数据量过大
const unsigned char  szErrPrt_DataRmalloc[7]      = "B00016"; // 处理后打印数据buffer再申请内存失败
// FreeType处理错误码
const unsigned char  szErrFT_Init[7]              = "C00001"; // FreeType库初始化失败(FT_Init_FreeType)
const unsigned char  szErrFT_New_Face[7]          = "C00002"; // 创建外观对象失败(FT_New_Face)
const unsigned char  szErrFT_Pixel_Sizes[7]       = "C00003"; // 设置当前字体属性失败(FT_Set_Pixel_Sizes)
const unsigned char  szErrFT_Load_Glyph[7]        = "C00004"; // 索引读取到字形槽及装载对应字形图像失败(FT_Load_Glyph)
const unsigned char  szErrFT_Render_Glyph[7]      = "C00005"; // 字形转换为位图失败(FT_Render_Glyph)
// 参数设置
const unsigned char  szErrSet_FTypeInvalid[7]     = "D00001"; // 传入的字体文件无效
const unsigned char  szErrSet_FSizeInvalid[7]     = "D00002"; // 传入的字体大小无效
const unsigned char  szErrSet_FStyleInvalid[7]    = "D00003"; // 传入的文本风格无效/不支持的文本风格
const unsigned char  szErrSet_RDistanInvalid[7]   = "D00004"; // 传入的行间距无效
const unsigned char  szErrSet_MDistanInvalid[7]   = "D00005"; // 传入的列间距无效
const unsigned char  szErrSet_FTypeNotFound[7]    = "D00006"; // 传入的字体文件不存在
const unsigned char  szErrSet_FTypeMNotFound[7]   = "D00007"; // 缺省字体文件不存在


// 打印纸状态
enum PaperStatus
{
    PAPER_STATUS_NORMAL        = 1, //正常
    PAPER_STATUS_LOW           = 2, //少纸
    PAPER_STATUS_JAMMED        = 3, //堵塞
    PAPER_STATUS_UNKNOWN       = 4, //未知
    PAPER_STATUS_EMPTY         = 5,
    PAPER_STATUS_UNSUPPORT     = 6,
};

// INK、TONER或色带状态
enum TonerStatus
{
    TONER_STATUS_NORMAL        = 1, //OK
    TONER_STATUS_FULL          = 2, //满
    TONER_STATUS_LOW           = 3, //少
    TONER_STATUS_EMPTY         = 4, //空
    TONER_STATUS_UNKNOWN       = 5, //未知
    TONER_STATUS_UNSUPPORT     = 6,
};

// 出纸口状态
enum OutletStatus
{
    OUTLET_STATUS_NOMEDIA       = 1,  //无纸
    OUTLET_STATUS_MEDIA         = 2,  //有纸
    OUTLET_STATUS_UNKNOWN       = 3,  //未知
    OUTLET_STATUS_UNSUPPORT     = 4,
};

// 文本风格
#define PTR_TEXT_NORMAL                0x00000001   //normal常规
#define PTR_TEXT_BOLD                  0x00000002   //bold粗体
#define PTR_TEXT_ITALIC                0x00000004//italic
#define PTR_TEXT_UNDER                 0x00000008//single underline
#define PTR_TEXT_DOUBLEUNDER           0x00000010//double underline
#define PTR_TEXT_DOUBLEWIDE            0x00000020//double width
#define PTR_TEXT_TRIPLEWIDE            0x00000040//triple width
#define PTR_TEXT_QUADRUPLEWIDE         0x00000080//quadruple width
#define PTR_TEXT_STRIKETHROUGH         0x00000100//strikethrough line
#define PTR_TEXT_ROTATE90              0x00000200
#define PTR_TEXT_ROTATE270             0x00000400
#define PTR_TEXT_UPSIDEDOWN            0x00000800//upside down
#define PTR_TEXT_PROPORTIONAL          0x00001000//proportional spacing
#define PTR_TEXT_DOUBLEHIGH            0x00002000//double high
#define PTR_TEXT_TRIPLEHIGH            0x00004000//triple high
#define PTR_TEXT_QUADRUPLEHIGH         0x00008000//quadruple high
#define PTR_TEXT_CONDENSED             0x00010000
#define PTR_TEXT_SUPERSCRIPT           0x00020000//superscript
#define PTR_TEXT_SUBSCRIPT             0x00040000//subscript
#define PTR_TEXT_OVERSCORE             0x00080000
#define PTR_TEXT_LETTERQUALITY         0x00100000
#define PTR_TEXT_NEARLETTERQUALITY     0x00200000
#define PTR_TEXT_DOUBLESTRIKE          0x00400000
#define PTR_TEXT_OPAQUE_STYLE          0x00800000

//#define DVEPRT_NO_VTABLE  __declspec(novtable)

typedef struct _print_format
{
    char szFontType[255];            //定义字体，0为缺省字体,该位置传入字体文件(ttf/ttc)绝对路径
    unsigned char uFontSize;         //定义字体大小，0为缺省值
    unsigned long ulStyle;           //定义文本风格，值为上述宏定义内容，可以为组合选项，0为缺省值
    unsigned char uWPI;              //定义字间距，0为缺省值
    unsigned char uLPI;              //定义行间距，0为缺省值
    //QColor Color;                  //定义字体颜色，RGB格式
    _print_format()
    {
        memset(this, 0, sizeof(_print_format));
    }
} STPRINTFORMAT, *LPSTPRINTFORMAT;

// 新增打印模式结构体
typedef struct _print_mode
{
    bool              bPageMode = true;       // 定义打印模式(TRUE:页模式/FALSE:行模式)
    unsigned short    wLeft = 0;              // 定义介质打印左边距，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
    unsigned short    wTop = 0;               // 定义介质打印上边距，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
    unsigned short    wWidth = 0;             // 定义介质可打印宽度，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
    unsigned short    wHeight = 0;            // 定义介质可打印高度，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
} STPRINTMODE, *LPSTPRINTMODE;


struct IDevPTR
{
    virtual long Release() = 0;
    /************************************************************
    ** 功能：打开与设备的连接
    ** 输入：pMode: 自定义OPEN参数字串,
        串口： 格式为："COMX: BaudRate,Parity,DataBits,StopBits"(例如："COM2:115200,N,8,1")
        USB:  格式自定义
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual long Open(const char *pMode) = 0;

    /************************************************************
    ** 功能：关闭与设备的连接
    ** 输入：无
    ** 输出：无
    ** 返回：无
    ************************************************************/
    virtual void Close() = 0;

    /************************************************************
    ** 功能：设备初始化
    ** 输入: 无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual long Init() = 0;

    /************************************************************
    ** 功能：设置当前打印格式
    ** 输入：stPrintFormat：定义当前打印的格式具体内容
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual long SetPrintFormat(const STPRINTFORMAT &stPrintFormat) = 0;

    /************************************************************
    ** 功能：打印字串(无指定打印坐标)
             打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
    ** 输入：pStr：要打印的字串
             ulDataLen：数据长度，如果为-1，pStr以\0结束
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual long PrintData(const char *pStr, unsigned long ulDataLen) = 0;

    /************************************************************
    ** 功能：图片打印(无指定打印坐标)
    ** 输入：szImagePath：图片保存路径
             ulWidth：打印宽度
             ulHeight：打印高度
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual long PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight) = 0;

    /************************************************************
    ** 功能：切纸
    ** 输入：bDetectBlackStripe：是否检测黑标
             ulFeedSize，切纸前走纸的长度，单位0.1毫米
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual long CutPaper(bool bDetectBlackStripe, unsigned long ulFeedSize) = 0;

    /************************************************************
    ** 功能：主动查询一次设备状态
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual long QueryStatus() = 0;

    /************************************************************
    ** 功能：得到打印机状态
    ** 输入：无
    ** 输出：pPrinterStatus，返回打印机状态
             pPaperStatus，返回纸状态
             pTonerStatus，返回TONER状态
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual void GetStatus(PaperStatus &PaperStatus, TonerStatus &TonerStatus, OutletStatus &OutletStatus) = 0;


    // 新增加函数

    /************************************************************
    ** 功能：设置当前打印模式
    ** 输入：stPrintMode：定义当前打印模式的具体内容
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    //virtual long SetPrintMode(const STPRINTMODE& stPrintMode) = 0;

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
    //virtual long PrintPageTextOut(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY) = 0;

    /************************************************************
    ** 功能：页模式下传打印图片
    ** 输入：szImagePath：图片保存绝对路径
            unsigned long ulOrgX: 打印图片的列坐标，单位: 0.1mm
            unsigned long ulOrgY: 打印图片的行坐标，单位: 0.1mm
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    //virtual long PrintPageImageOut(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY) = 0;

    /************************************************************
    ** 功能：执行页模式数据批量打印
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    //virtual long PrintPageData() = 0;

    /************************************************************
    ** 功能：行模式打印字串
             打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
    ** 输入：pStr：要打印的字串
            ulDataLen：数据长度，如果为-1，pStr以\0结束
            unsigned long ulOrgX: 打印字串的列坐标，单位: 0.1mm
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    //virtual long PrintLineData(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX = 0) = 0;

    /************************************************************
    ** 功能：获取处理错误码
    ** 输入：无
    ** 输出：无
    ** 返回：处理错误码(总计7位，错误码6位，1位结束符)
    ************************************************************/
    //virtual char* GetErrCode() = 0;

    /************************************************************
    ** 功能：获取链接库版本
    ** 输入：无
    ** 输出：无
    ** 返回：链接库版本(总计17位，1位结束符)
    ************************************************************/
    //virtual char* GetVersion() = 0;

    /************************************************************
    ** 功能：获取硬件设备指令处理返回码
    ** 输入：无
    ** 输出：无
    ** 返回：返回码
    ************************************************************/
    //virtual unsigned long GetDevErrCode() = 0;

    /************************************************************
    ** 功能：校正标记纸的起始位置
    ** 输入：byMakePos : 标记位置值，单位: 0.1mm，上限31.8mm
    ** 输出：无
    ** 返回：返回码
    ************************************************************/
    virtual long ChkPaperMarkHeader(unsigned int uiMakePos) = 0;
};


/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVPTR_EXPORT long  CreateIDevPTR(const char *lpDevType, IDevPTR *&pDev);

/************************************************************
** 功能：结束设备连接handle
** 输入：无
** 输出：无
** 返回：无
************************************************************/
//extern "C" DEVPTR_EXPORT void  ExitIDevPTR();


