#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义

// 设备类型
#define DEV_HOTS                0               // HOTS打印机
#define DEV_SNBC_BKC310         1               // 新北洋BK-C310打印机
#define DEV_SNBC_BTNH80         2               // 新北洋BT-NH80打印机

#define DEV_HOTS_STR            "HOTS"          // HOTS打印机
#define DEV_SNBC_BKC310_STR     "BK-C310"       // 新北洋BK-C310打印机
#define DEV_SNBC_BTNH80_STR     "BT-NH80"       // 新北洋BT-NH80打印机

// SetData()使用
#define SET_PRT_FORMAT          1               // 设置打印格式

// GetVersion()使用
#define GET_VER_DEVRPR          1               // DevRPR版本号
#define GET_VER_FW              2               // 固件版本号

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


// -----------------程序内部处理错误码(HOST处理使用)-------------------------------------------------
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


typedef struct _print_format
{
    char szFontType[255];            //定义字体，0为缺省字体,该位置传入字体文件(ttf/ttc)绝对路径
    unsigned char uFontSize;         //定义字体大小，0为缺省值
    unsigned long ulStyle;           //定义文本风格，值为上述宏定义内容，可以为组合选项，0为缺省值
    unsigned char uWPI;              //定义字间距，0为缺省值
    unsigned char uLPI;              //定义行间距，0为缺省值
    //QColor Color;                  //定义字体颜色，RGB格式
    unsigned char uLineHeight;       //定义行高,0为缺省值
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

#endif // DEF_H
