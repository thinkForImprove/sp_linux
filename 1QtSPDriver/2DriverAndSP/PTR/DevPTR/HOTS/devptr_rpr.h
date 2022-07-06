/*****************************************************************************
** FileName     : DevPTR_RPR.CPP                                            **
** Describtion  : 打印处理 头文件                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      : 2019/7/8 RPR01000000 JiaoChong(CFES) Create               **
*****************************************************************************/
#ifndef DEVPTR_RPR_H
#define DEVPTR_RPR_H

#include <QtCore/qglobal.h>
#include "baseTypeChange.h"
#include "KS_PRM.H"

#include "IDevPTR.h"
#include "SprDLLDef.h"

#include <QLibrary>
#include <syslog.h>
#include <mcheck.h>
#include <ft2build.h>
#include <freetype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <libusb-1.0/libusb.h>

#include "DevPortLogFile.h"
//////////////////////////////////////////////////////////////////////////////
//  外部函数定义部分                                                        //
//////////////////////////////////////////////////////////////////////////////
extern "C" long WINAPI lDGetCodeType(BYTE byHigh, BYTE byLow);
extern "C" int WINAPI _mbbtype(unsigned char cChr, int iType);


//////////////////////////////////////////////////////////////////////////////
//  全局宏定义部分                                                            //
//////////////////////////////////////////////////////////////////////////////

#define     RPR_USBVID      0x04A4
#define     RPR_USBPID      0x00B7

// 打印计算/计量单位
#define     PRINT_METECOMP          0.125       // 打印计量单位:0.125mm
#define     PRINT_METECOMP2         1.25        // 打印计量单位2:0.125mm * 10,用于0.1mm时计算
#define     MAX_PAGEWIDE            576         // 缺省打印介质宽，计量单位:0.125mm,8个计量单位为1mm,576/8=72mm
#define     DEF_LEFTDIST_HOTS       INT32(3.75/PRINT_METECOMP)  // HOST 缺省左边界3.75mm
#define     DEF_LEFTDIST_MM         INT32(37)   // 单位:0.1mm, HOST 缺省左边界3.75mm
#define     DEF_TOPDIST_HOTS        INT32(11/PRINT_METECOMP)    // HOST 缺省上边界11mm
#define     DEF_TOPDIST_MM          INT32(110)  // 单位:0.1mm, HOST 缺省左上边界11mm
#define     DEF_TOPDIST_MM2         INT32(90)  // 单位:0.1mm, HOST 缺省左上边界11mm

// 存储空间Buffer Size
#define     HENSYU_BUFF_SIZE        (0x1FFFE)   // 数据处理bufferBuffer长度，131,070
#define     BUFFER_SIZE_128         128         // 128字节Buffer长度
#define     SEND_BLOCK_MAX_SIZE     65535       // 一批次下发到设备的打印数据量(字节)
#define     MAX_PATH                256         // 路径最大长度

// 图片信息相关
#define     IMAGE_LINE              32          // 图片行(垂直)点数
#define     MAX_PAGEWIDE            576         // 可打印图片宽上限(像素)
#define     MAX_PAGEHEIGHT          1816        // 可打印图片高上限(像素)
#define     MAX_IMAGEFILE_SIZE      140000      // 图片数据Buffer最大长度

// 字符验证相关
#define     _MBC_ILLEGAL            -1                  // 无效的字符(0x20–0x7E,0xA1–0xDF,0x81–0x9F,0xE0–0xFC以外的任何值)
//    对应_mbbtype(): Type=1以外的任何值,用来验证有效的单字节或前导字节
//    对应_mbbtype(): Type=1,用来验证有效的结尾字节
#define     _MBC_SINGLE             0                       // 单字节(0x20–0x7E,0xA1–0xDF)
//    对应_mbbtype(): Type=1以外的任何值,用来验证有效的单字节或前导字节
#define     _MBC_LEAD               1                       // 多字节字符的前导字节(0x81–0x9F,0xE0–0xFC)
//    对应_mbbtype(): Type=1 以外的任何值,用来验证有效的单字节或前导字节
#define     _MBC_TRAIL              2                       // 多字节字符的结尾字节(0x40–0x7E,0x80–0xFC)
//    对应_mbbtype(): Type=1=,用来验证有效的结尾字节
#define     GAIJI                   2                       // 外字
#define     IBMKANJI                3                       // IBM汉字




// 设备/打印机状态
#define     SPR_STATUS_OFFLINE              0x08    // 离线

#define     SPR_STATUS_REGULAR_POS          0xD0    //
#define     SPR_STATUS_TAKE_WAIT            0x80
#define     SPR_STATUS_SENSOR_A             0x01    // 传感器A
#define     SPR_STATUS_SENSOR_B             0x02    // 传感器B
#define     SPR_STATUS_SENSOR_C             0x04    // 传感器C
#define     SPR_STATUS_SENSOR_D             0x08    // 传感器A
#define     SPR_STATUS_SENSOR_ABCD          0x0F    // 传感器ABCD

#define     SPR_STATUS_PAPER_REMAIN         0x10    // 搬送路径纸残留
#define     SPR_STATUS_ROLLA_EJECT          0x02    // A口排除
#define     SPR_STATUS_DIRECTION            0x40    // 

#define     SPR_STATUS_NEAR_END             0x01    // 纸少
#define     SPR_STATUS_NO_PAPER             0x04    // 无纸

#define     SPR_STATUS_JAM_ERR              0x01    // 用纸搬送异常
#define     SPR_STATUS_PAPER_ERR            0x02    // 异常用纸检出
#define     SPR_STATUS_RETRACT_ERR          0x04    // 回收异常
#define     SPR_STATUS_PRINT_RETRACT        0x08    // 印字中用纸回收异常
#define     SPR_STATUS_PAPER_REMAIN         0x10    // 搬送路用纸残留
#define     SPR_STATUS_SELFRESET            0x20    // 
#define     SPR_STATUS_DIRECTION            0x40    // 搬运动作中
#define     SPR_STATUS_FUSEOFF              0x80    // 

// 整除余计算标准
#define     DIVREM_COMP                     255


//////////////////////////////////////////////////////////////////////////////
//  全局变量定义部分                                                           //
//////////////////////////////////////////////////////////////////////////////
//  Command Send Data Format
const BYTE  CMDF_PAGE_CHANGE_0C          = 0x01; // 数据打印并恢复行模式
const BYTE  CMDF_INJIPOSITION_123D01     = 0x02; // 图像LSB/MSB选择
const BYTE  CMDF_MOJIKANSPACE_1B2000     = 0x03; // 设置文字右空格量:00
const BYTE  CMDF_INJIMODE_1B21           = 0x04; // 指定文本打印格式:00
const BYTE  CMDF_PAGE_JPOSX_INIT_1B24    = 0x05; // 横绝对位置:00
const BYTE  CMDF_GYOKANSPACE_1B3320      = 0x06; // 设置换行量:H20=D32=4MM
const BYTE  CMDF_GYOKANSPACE_1B3340      = 0x07; // 设置换行量:H20=D64=8MM
const BYTE  CMDF_OKURI_SET_1B4A00        = 0x08; // 正向送纸:00
const BYTE  CMDF_OKURI_SET_1B4A1C        = 0x09; // 正向送纸:1C=D28=3.5MM
const BYTE  CMDF_OKURI_SET_1B4A21        = 0x10; // 正向送纸:21=D33=4.125MM
const BYTE  CMDF_OKURI_SET_1B4A22        = 0x11; // 正向送纸:22=D34=4.25MM
const BYTE  CMDF_OKURI_SET_1B4A30        = 0x12; // 正向送纸:30=D48=6MM
const BYTE  CMDF_OKURI_SET_1B4A4D        = 0x13; // 正向送纸:4D=D77=9.625MM
const BYTE  CMDF_OKURI_SET_1B4A59        = 0x14; // 正向送纸:59=D89=11.125MM
const BYTE  CMDF_OKURI_SET_1B4A62        = 0x15; // 正向送纸:62=D98=12.25MM
const BYTE  CMDF_OKURI_SET_1B4A81        = 0x16; // 正向送纸:81=D129=16.125MM
const BYTE  CMDF_OKURI_SET_1B4A84        = 0x17; // 正向送纸:84=D132=16.5MM
const BYTE  CMDF_OKURI_SET_1B4AFF        = 0x18; // 正向送纸:FF=D255=31.875MM
const BYTE  CMDF_PAGEMODE_ON_1B4C        = 0x19; // 设置为页模式
const BYTE  CMDF_LINEMODE_ON_1B53        = 0x20; // 设置为行模式
const BYTE  CMDF_PAGEMODE_SET_1B57       = 0x21; // 设置页模式打印区域
const BYTE  CMDF_PAGE_XPOSX_INIT_1B5C    = 0x22; // 横相对位置:00
const BYTE  CMDF_CUT_1B69                = 0x23; // 切纸
const BYTE  CMDF_PAGE_JPOSY_INIT_1D24    = 0x24; // 纵绝对位置:00
const BYTE  CMDF_PAGE_KIJUN_1D2418       = 0x25; // 页模式垂直绝对位置:18
const BYTE  CMDF_PAGE_KIJUN_1D2430       = 0x26; // 页模式垂直绝对位置:30
const BYTE  CMDF_MARKCHK_1D3C            = 0x27; // 标记起始位置开始移动
const BYTE  CMDF_MARKHEADCHK_1D41        = 0x28; // 校正标记纸的起始位置
const BYTE  CMDF_LEFT_MARGIN_1D4C00      = 0x29; // 左边界设定
const BYTE  CMDF_HORYU_1D5832            = 0x30; // 介质送出保留
const BYTE  CMDF_HAISYUTU_1D583300       = 0x31; // 介质送出:00
const BYTE  CMDF_HAISYUTU_1D583301       = 0x32; // 介质送出:01
const BYTE  CMDF_KAISYU_1D5834           = 0x33; // 介质回收:FF
const BYTE  CMDF_PAGE_XPOSY_INIT_1D5C    = 0x34; // 纵相对位置:00
const BYTE  CMDF_PARAMETER_1D6C          = 0x35; // 打印结束标志





//////////////////////////////////////////////////////////////////////////////
//  结构体定义部分                                                            //
//////////////////////////////////////////////////////////////////////////////

//文字种判别用定数
enum _code_type
{
    ctKanji,            // 0:汉字编码
    ctIbmKanji,         // 1:IBM汉字编码
    ctAnk,              // 2:半角字符编码
    ctGaiji,            // 3:外字编码
    ctIllegal           // 4:非法字符编码
};


// 设备底层指令集合
struct CMD_Data
{
    BYTE    byID;
    long    lLen;
    BYTE    lpCMD[16];
};

// 自定义打印图片信息结构体
typedef struct _spr_print_image
{
    ULONG   ulImageSize;        // 图像大小(字节)
    LPBYTE  lpbImageData;       // 指向图像数据的指针
    WORD    wOffsetX;           //Offset X
    WORD    wOffsetY;           //Offset Y (Fixed to 0)
    WORD    wImageWidth;        //Width of Image
    WORD    wImageHeight;       //Height of Image
    WORD    wImageDataType;     //Type of Image Data (Fixed to 0)
    WORD    wFunctionType;      //Function Type
    WORD    wImageID;           //Image ID (0 <= wImageID <= 254)
} HWPRTSPRPRINTIMAGE, *LPHWPRTSPRPRINTIMAGE;

// BMP文件头结构体
typedef struct _BmpFileHeader
{
    INT16   byBM;          // 2位,图片类型(BM/0x4D42)
    INT32   ulFileSize;    // 4位,位图文件的大小，用字节为单位
    INT16   byDummy1;      // 2位,保留，必须设置为0
    INT16   byDummy2;      // 2位,保留，必须设置为0
    INT32   ulOffset;      // 4位,从文件头开始到实际的图象数据之间的字节的偏移量
} __attribute__((packed))BmpFileHeader;

// BMP文件信息头结构体
typedef struct _BmpInfoHeader
{
    INT32    ulHeaderSize;      // 4位,BITMAPINFOHEADER结构所需要的字节数
    INT32    ulBmpWidth;        // 4位,图象的宽度，以象素为单位
    INT32    ulBmpHeight;       // 4位,图象的高度，以象素为单位(值是正数，图像是倒向，负数，图像是正向)
    INT16    wPlanes;           // 2位,为目标设备说明位面数，其值将总是被设为1
    INT16    wColorBits;        // 2位,BMP图像的色深，表示比特数/象素，其值为1、4、8、16、24、或32
    INT32    ulCompression;     // 4位,图象数据压缩的类型(没有压缩的类型：BI_RGB)
    INT32    ulImageSize;       // 4位,图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0(不压缩)
    INT32    ulXResolution;     // 4位,表示水平分辨率，用象素/米表示
    INT32    ulYResolution;     // 4位,表示垂直分辨率，用象素/米表示
    INT32    ulUsedColors;      // 4位,位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）
    INT32    ulImportantColors; // 4位,对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要
} __attribute__((packed))BmpInfoHeader;

// 设备状态获取输入参数
typedef enum
{
    SPR_PRINTER,
    SPR_ERROR,
    SPR_YOUSHI,             // 打印纸状态
    SPR_HANSOURO,
    SPR_PARAMETER,
    SPR_ERROR2,
    SPR_2ROLL,
    SPR_ROLL_SENSOR,
    SPR_ROLL_A,
    SPR_ROLL_B
} enSPRSTATUS;

//////////////////////////////////////////////////////////////////////////////
// 打印数据处理类                                                             //
//////////////////////////////////////////////////////////////////////////////
class INJI_ED
{
    //******* 函数声明 ******************
public:     // 构造函数+析构函数
    INJI_ED();
    ~INJI_ED();

public:     // 打印处理主调函数
    long    N_SetPrintFormat(STPRINTFORMAT stPrintFormat);      // 设置文本格式
    void    N_SetPrintMode(WORD wDLeft, WORD wDTop, WORD wDWidth, WORD wDHeight, WORD wDRowHeight);     // 设置打印模式
    void    N_SetPrintLeft(WORD wDLeft);     // 设置打印模式左边距
    long    N_PrintData_Edit();     // 文本数据处理
    long    N_PrintImage_Edit(LPBYTE lpszImgFName, ULONG ulOrgX = 0, ULONG ulOrgY = 0);     // 图片数据处理
    void    N_AddCutPaperCmd(BOOL bDetectBlackStripe, ULONG ulFeedSize);    // 切纸指令处理
    void    N_AddPageModeStart();   // 设置页打印模式
    void    N_AddLineModeStart();   // 设置行打印模式
    void    N_SetAbsolutePos(ULONG ulOrgX, ULONG ulOrgY);   // 设置行列绝对位置
    void    N_SetRelativePos(ULONG ulOrgX, ULONG ulOrgY);   // 设置行列相对位置
    void    N_AddChkPaperCmd(UINT uiMarkPos);   // 校正标记纸起始位置指令处理

public:
    // 类处理函数
    void    N_Init();   // 初始化
    void    N_PushInjiData(LPBYTE, LONG);   // 下传初始打印数据空间指针赋值
    void    N_AddCmdHensyuArray(BYTE byCMD);    // 设置下发指令到处理后打印数据buffer
    void    N_CopyHensyuToSendBuffer(LPBYTE lpData, LONG lOffset, WORD wBuffSize);  // 从处理后下发打印数据buffer中Copy指定数目的数据
    void    N_AddHensyuChar(BYTE byChar);   // 处理后可打印数据buffer末尾写入数据
    void    N_AddHensyuChar(WORD wHensyPtr, BYTE byChar);  // 在处理后可打印数据buffer指定位置写入数据
    DWORD   N_GetHensyuPtr();   // 获取已处理的可打印buffer中数据长度
    void    N_ClearLineInfo();  // 清空临时处理打印数据空间
    void    N_ClearPrintBuffer();   // 清空打印buffer空间

private:
    // 类处理函数
    void    N_InitBuffer();         // 数据处理buffer初始化
    void    N_FreeBuffer();         // 数据处理buffer空间释放
    void    N_InitMojiBuffer();     // 数据处理buffer初始化
    void    N_FreeMojiBuffer();     // 数据处理buffer空间释放
    void    N_AddHensyuArray(LPBYTE lpbyBuffer, LONG lLen);     // 数据写入处理后打印数据保存buffer
    void    N_AddMojiChar(BYTE byMoji);     // 向数据处理中buffer写入数据
    void    N_AddImageTouroku(LPBYTE lpbyFontBuff, long lLen);  // 文本位图数据(以位图图像模式打印)写入处理中Buffer
    DWORD   N_SendDataLen();    // 获取已处理的可下发打印的数据数量(处理中+处理后buffer)
    WORD    GetCmdData_HOTS(BYTE byID, LPBYTE lpbyCMD, LONG lBufferPtr, LONG lBufferSize);  // 设备底层指令分析匹配
    BYTE    GetParameterID();   // 获取下发指令序列号


protected:
    // 数据处理函数
    long    N_MojiSet_Usb();    // 字符/汉字处理
    long    N_ImageRead(LPSTR lpszImgFName, LPHWPRTSPRPRINTIMAGE lpSprPrintImage);  // 读取图片内容
    long    N_FileRead_Bin(LPSTR lpszFileName, DWORD wOffset, DWORD wFileSize, LPSTR lpFileData);   // 文件内容读取
    void    N_ImageDataAnalyse(HWPRTSPRPRINTIMAGE *lpSprPrintImage);    // 位图数据置换为正常的行列模式排列


    //******* 变量声明 ******************
public:
    WORD    wLineWidthByte;                 // 一行打印数据量(字节/mm)
    WORD    wPaperFeedSz;                   // 正向送纸，单位：0.1mm

    WORD    wXPosition;                     // 列当前坐标
    WORD    wYPosition;                     // 行当前坐标
    WORD    wYPosCount;                     // 行坐标计数

    WORD    wPageModePtr;                   // 打印区域设置指令在处理后打印数据保存Buffer的位置

protected:
    LPBYTE  byHensyuBuffer;                 // 处理后打印数据保存Buffer
    DWORD   dwHensyuPtr;                    // 处理后已保存数据的Length
    DWORD   dwHensyuBufferSize;             // 处理后打印数据保存Buffer Size 上限

    LPBYTE  byMojiBuffer;                   // 处理中数据保存buffer
    DWORD   dwMojiPtr;                       // 处理中以保存数据Length

    LPBYTE  lpbySansyo;                     // 下传初始数据指针(循环处理数据用)
    LPBYTE  lpbyLastSansyo;                 // 下传初始数据指针(循环处理数据用)
    WORD    wSansyoPtr;                     // 下传初始数据指针位移量(循环处理数据用)
    WORD    wSansyoSz;                      // 下传数据总Size

    BOOL    bLineEnd;                       // 换行标记/数据处理结束标记，缺省F

    // 打印规格
    WORD    wLeft;                          // 左边距，单位:0.1mm
    WORD    wTop;                           // 上边距，单位:0.1mm
    WORD    wWidth;                         // 可打印宽，单位:0.1mm
    WORD    wHeight;                        // 可打印高，单位:0.1mm
    WORD    wRowHeight;                     // 行高，单位:0.1mm

    // 文本规格
    BYTE    byFontType[MAX_PATH];           // 字体
    WORD    wFontSize;                      // 字体大小
    WORD    wFontStyle;                     // 文本风格
    WORD    wRowDistance;                   // 行间距，缺省0
    WORD    wMojiDistance;                  // 字符间距
};



class IPrinterDevice : public IDevPTR
{
public:
    IPrinterDevice() {}
    virtual ~IPrinterDevice() {}
public:
    virtual long Release();
    // 打开与设备的连接
    virtual long Open(const char *pMode);
    virtual long OpenDev(const unsigned short usMode);
    // 关闭与设备的连接
    virtual void Close();
    // 设备初始化
    virtual long Init();
    // 设置当前打印格式
    virtual long SetPrintFormat(const STPRINTFORMAT &stPrintFormat);
    // 打印字串(无指定打印坐标), 打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
    virtual long PrintData(const char *pStr, unsigned long ulDataLen);
    // 图片打印(无指定打印坐标)
    virtual long PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight);
    // 指定坐标打印图片
    virtual long PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY);
    // 切纸, bDetectBlackStripe：是否检测黑标, ulFeedSize，切纸前走纸的长度，单位0.1毫米
    virtual long CutPaper(bool bDetectBlackStripe, unsigned long ulFeedSize);
    // 查询一次设备状态
    virtual long QueryStatus();
    // 得到打印机状态, pPrinterStatus，返回打印机状态, pPaperStatus，返回纸状态, pTonerStatus，返回TONER状态
    virtual void GetStatus(PaperStatus &PrinterStatus, TonerStatus &PaperStatus, OutletStatus &TonerStatus);
    // 设置当前打印模式, stPrintMode：定义当前打印模式的具体内容
    virtual long SetPrintMode(const STPRINTMODE &stPrintMode);
    // 页模式下传打印字串, 打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
    virtual long PrintPageTextOut(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY);
    // 页模式下传打印图片
    virtual long PrintPageImageOut(const char *szImagePath, unsigned long ulOrgX, unsigned long ulOrgY);
    // 执行页模式数据批量打印
    virtual long PrintPageData();
    // 行模式打印字串, 打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
    virtual long PrintLineData(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX = 0);
    // 获取处理错误码
    virtual char *GetErrCode();
    // 获取动态库版本
    virtual char *GetVersion();
    // 获取固件版本
    virtual bool GetFWVersion(char *szFWVer, unsigned long *ulSize);
    // 获取硬件设备指令处理返回码
    virtual unsigned long GetDevErrCode();
    // 校正标记纸的起始位置, byMakePos : 标记位置值，单位: 0.1mm，上限31.8mm
    virtual long ChkPaperMarkHeader(unsigned int uiMakePos);
};
//////////////////////////////////////////////////////////////////////////////
// 打印机处理入口类                                                            //
//////////////////////////////////////////////////////////////////////////////
class DevPTR_RPR
{
    //******* 函数声明 ******************
public:
    // 构造/析构函数
    DevPTR_RPR();
    ~DevPTR_RPR();
public:
    // 类处理函数
    void    N_Init();   // 变量初始化
    void    N_SetErrCode(const BYTE *byErrCd);  // 设置处理错误码

public: // 对外调用设备处理函数
    long    N_Open_Req();   // 打开设备
    long    N_Reset_Req();  // 设备初始化
    long    N_Close_Req();  // 关闭设备
    long    N_CutPaper_Req(BOOL bDetectBlackStripe, ULONG ulFeedSize);  // 切纸
    long    N_PrintData_Req(LPBYTE lpData, ULONG ulDataSz);     // 打印数据(测试用)
    long    N_PrintImage_Req(LPBYTE lpImage);       // 打印图片(测试用)
    long    N_GetStatus_Req(PaperStatus &PaperStatus, TonerStatus &TonerStatus,
                            OutletStatus &OutletStatus);    // 获取设备部件状态
    long    N_QueryStatus_Req();    // 获取设备连接状态
    long    N_SetPrintFormat_Req(STPRINTFORMAT stPrintFormat);  // 设置文本打印格式
    long    N_SetPrintMode_Req(STPRINTMODE stPrintMode);    // 设置打印模式

    long    N_PrintPageTextOut_Req(LPBYTE lpData, ULONG ulDataSz, ULONG ulOrgX, ULONG ulOrgY);  // 页模式文本下传
    long    N_PrintPageImageOut_Req(LPBYTE lpszImgFName, ULONG ulOrgX, ULONG ulOrgY);   // 页模式图片下传
    long    N_PrintPageData_Req();  // 页模式批量打印
    long    N_PrintLineData_Req(LPBYTE lpData, ULONG ulDataSz, ULONG ulOrgX);   // 行模式文本打印
    BYTE   *N_GetErrCode_Req();     // 获取处理错误码
    BYTE   *N_GetVersion_Req();     // 获取Lib版本号
    ULONG   N_GetDevErrCode();      // 获取硬件设备指令处理返回码
    long    N_ChkPaperMarkHeader_Req(UINT uiMarkPos);   // 校正标记纸的起始位置

    long     N_GetStatus_Get(PaperStatus &PaperStatus, TonerStatus &TonerStatus,
                             OutletStatus &OutletStatus);
private:
    BOOL    bChkUsbDev(ULONG ulVid, ULONG ulPid);


    //******* 变量声明 ******************
public:
    IPrinterDevice iPtrDev;     // 设备处理函数调用handle

private:
    BOOL    bIsDevOpen;         // 设备当前状态，T:开启/F:关闭

    WORD    wLeft;              // 左边距，缺省: 37，单位:0.1mm
    WORD    wTop;               // 上边距，缺省: 110，单位:0.1mm
    WORD    wWidth;             // 可打印宽，缺省: 720，单位:0.1mm
    WORD    wHeight;            // 可打印高，缺省: 0，单位:0.1mm

    WORD    wRowHeight;         // 行高，缺省: 30，单位:0.1mm
    WORD    wRowDistance;       // 行间距，缺省: 0，单位:0.1mm
    WORD    wMojiDistance;      // 字符间距，缺省: 0，单位:0.1mm

    INJI_ED *Inji_Ed;           // 打印数据处理类变量
    BYTE    byErrCode[7];       // 错误码

    BOOL    bIsPrintMode;       // 是否已设置打印模式，缺省F
    BOOL    bPageMode;          // 是否行打印模式，缺省F

    PaperStatus m_ePaperStatus;
    TonerStatus m_eTonerStatus;
    OutletStatus m_eOutletStatus;

    CDevPortLogFile      m_Log;
};

#endif // DEVPTR_RPR_H
