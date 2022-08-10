#pragma once
/***************************************************************
* 文件名称: IDEVPTR.h
* 文件描述: 声明打印模块底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年4月25日
* 文件版本: 1.0.0.1
****************************************************************/

#include <string.h>
#include <QtCore/qglobal.h>
#include <XFSPTR.H>
#include <XFSAPI.H>
#include <stdio.h>
#include "QtTypeDef.h"

#if defined IDEVPTR_LIBRARY
#define DEVPTR_EXPORT     Q_DECL_EXPORT
#else
#define DEVPTR_EXPORT     Q_DECL_IMPORT
#endif


// ----------------------------------宏定义----------------------------------

// 功能入口函数返回值
//成功
#define PTR_SUCCESS                 (0)     // 操作成功
#define ERR_PTR_CANCEL              (1)     // 命令取消
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
#define ERR_PTR_CHRONIC             (-14)   // 软件故障
#define ERR_PTR_HWERR               (-15)   // 硬件故障
#define ERR_PTR_IMAGE_ERROR         (-16)   // 图片相关错误
#define ERR_PTR_NO_DEVICE           (-17)   // 指定名的设备不存在
#define ERR_PTR_UNSUP_CMD           (-18)   // 不支持的指令
#define ERR_PTR_DATA_ERR            (-19)   // 收发数据错误
#define ERR_PTR_TIMEOUT             (-20)   // 超时
#define ERR_PTR_DRVHND_ERR          (-21)   // 驱动错误
#define ERR_PTR_DRVHND_REMOVE       (-22)   // 驱动丢失
#define ERR_PTR_USB_ERR             (-23)   // USB/COM/连接错误
#define ERR_PTR_DEVBUSY             (-25)   // 设备忙
#define ERR_PTR_OTHER               (-26)   // 其它错误，如调用API错误等
#define ERR_PTR_DEVUNKNOWN          (-27)   // 设备未知
#define ERR_PTR_NOMEDIA             (-30)   // 指定位置无介质
#define ERR_PTR_HAVEMEDIA           (-31)   // 指定位置有介质
#define ERR_PTR_PAPER_ERR           (-32)   // 介质异常
#define ERR_PTR_JSON_ERR            (-40)   // JSON处理错误
#define ERR_PTR_SCAN_FAIL           (-41)   // 扫描失败
#define ERR_PTR_DATA_DISCERN        (-42)   // 数据识别失败
#define ERR_PTR_NO_MEDIA            (-43)   // 通道无纸
#define ERR_PTR_RETRACTFULL         (-44)   // 回收箱满
#define ERR_PTR_MS_BLANK            (-45)   // 空白磁条/磁条无效
#define ERR_PTR_OFFLINE             (-46)   // 设备断线
#define ERR_PTR_SHORTAGEMEMORY      (-47)   // 内存不足

/* 该部分内容加在 QtTypeDef.h 中, 可通过[#include "QtTypeDef.h"]方式引用
// 初始化
#define MSET_0(a) memset(a, 0x00, sizeof(a));

// 比较
#define MCMP_IS0(a, b) \
    (memcmp(a, b, strlen(b)) == 0 && memcmp(a, b, strlen(a)) == 0)

// 复制(不指定长度)
#define MCPY_NOLEN(d, s) \
    memset(d, 0x00, sizeof(d)); \
    memcpy(d, s, strlen(s) > sizeof(d) ? sizeof(d) - 1 : strlen(s));

// 复制(指定长度)
#define MCPY_LEN(d, s, l) \
    memset(d, 0x00, sizeof(d)); \
    memcpy(d, s, l);

// 转换: 0.1毫米单位转1毫米单位
#define M01M2MM(MM) (INT)((MM + 5) / 10)

// 毫米转像素数(入参: MM 0.1毫米单位, DPI)
#define MM2PX(MM, DPI) \
    (INT)((float)(MM * 1.0 / 10.0 / 25.4 * (float)(DPI * 1.0)) + 0.5 / 10.0)
*/

// -----------------------------------枚举定义-----------------------------------

// 介质控制动作
// 值同WFS标准ControlMedia命令,支持多个命令组合, 根据Cen标准MediaControl定义.
// 也可作为另类含义使用,可根据需要在对应模块中不冲突的情形下赋予其他含义,例如:
//   票据发放(CPR)中 MEDIA_CTR_ATPFORWARD为介质入箱, MEDIA_CTR_ATPBACKWARD为介质出箱
//   票据受理(CSR)中 MEDIA_CTR_STAMP为指定Box
//   凭条(RPR)中MEDIA_CTR_ATPFORWARD为指定为黑标纸,MEDIA_CTR_ATPBACKWARD为指定为连续纸
enum MEDIA_ACTION
{
    MEDIA_CTR_NOTACTION             = 0x0000,   // 无动作
    MEDIA_CTR_EJECT                 = 0x0001,   // WFS:1:介质退出
    MEDIA_CTR_PERFORATE             = 0x0002,   // WFS:2:打印数据并在介质上打眼
    MEDIA_CTR_CUT                   = 0x0004,   // WFS:4:介质切割
    MEDIA_CTR_SKIP                  = 0x0008,   // WFS:8:打印数据并跳介质到黑标
    MEDIA_CTR_FLUSH                 = 0x0010,   // WFS:16:打印先前定义的数据
    MEDIA_CTR_RETRACT               = 0x0020,   // WFS:32:打印数据并回收
    MEDIA_CTR_STACK                 = 0x0040,   // WFS:64:打印数据并移动介质到堆放器
    MEDIA_CTR_PARTIALCUT            = 0x0080,   // WFS:128:打印数据并部分切割介质
    MEDIA_CTR_ALARM                 = 0x0100,   // WFS:256:发出警报声音
    MEDIA_CTR_ATPFORWARD            = 0x0200,   // WFS:512:打印数据并向前翻一页
    MEDIA_CTR_ATPBACKWARD           = 0x0400,   // WFS:1024:打印数据并向后翻一页
    MEDIA_CTR_TURNMEDIA             = 0x0800,   // WFS:2048:打印数据后插入介质
    MEDIA_CTR_STAMP                 = 0x1000,   // WFS:5096:打印数据并在介质上压印
    MEDIA_CTR_PARK                  = 0x2000,   // WFS:8192:介质放入纸盒
    MEDIA_CTR_EXPEL                 = 0x4000,   // WFS:16384:打印数据并从出口吐出
};

//　Status.Device返回状态
enum DEVPTR_DEVICE_STATUS
{
    DEV_STAT_ONLINE                  = 0,    // 设备正常
    DEV_STAT_OFFLINE                 = 1,    // 设备脱机
    DEV_STAT_POWEROFF                = 2,    // 设备断电
    DEV_STAT_NODEVICE                = 3,    // 设备不存在
    DEV_STAT_HWERROR                 = 4,    // 设备故障
    DEV_STAT_USERERROR               = 5,    //
    DEV_STAT_BUSY                    = 6,    // 设备读写中
};

//　staus.Media返回状态(介质状态)
enum DEVPTR_MEDIA_STATUS
{
    MEDIA_STAT_PRESENT               = 0,    // 通道内有介质
    MEDIA_STAT_NOTPRESENT            = 1,    // 通道内无介质
    MEDIA_STAT_JAMMED                = 2,    // 通道内有介质且被夹住
    MEDIA_STAT_NOTSUPP               = 3,    // 不支持检测通道内是否有介质
    MEDIA_STAT_UNKNOWN               = 4,    // 通道内介质状态未知
    MEDIA_STAT_ENTERING              = 5,    // 介质在出口
};

// status.Paper返回状态(票箱状态)
enum DEVPTR_PAPER_STATUS
{
    PAPER_STAT_FULL                  = 0,    // 满
    PAPER_STAT_LOW                   = 1,    // 少
    PAPER_STAT_OUT                   = 2,    // 无
    PAPER_STAT_NOTSUPP               = 3,    // 设备不支持该能力
    PAPER_STAT_UNKNOWN               = 4,    // 不能确定当前状态
    PAPER_STAT_JAMMED                = 5,    // 被卡住
};

// status.Toner返回状态(碳带状态)
enum DEVPTR_TONER_STATUS
{
    TONER_STAT_FULL                  = 0,    // 碳带状态满或正常
    TONER_STAT_LOW                   = 1,    // 碳带少
    TONER_STAT_OUT                   = 2,    // 碳带无
    TONER_STAT_NOTSUPP               = 3,    // 设备不支持该能力
    TONER_STAT_UNKNOWN               = 4,    // 不能确定当前状态

};

// status.Ink返回状态(墨盒状态)
enum DEVPTR_INK_STATUS
{
    INK_STAT_FULL                    = 0,    // 墨盒状态满或正常
    INK_STAT_LOW                     = 1,    // 墨盒少
    INK_STAT_OUT                     = 2,    // 墨盒无
    INK_STAT_NOTSUPP                 = 3,    // 设备不支持该能力
    INK_STAT_UNKNOWN                 = 4,    // 不能确定当前状态
};

// status.Life返回状态(硒鼓状态)
enum DEVPTR_LIFE_STATUS
{
    LIFE_STAT_FULL                    = 0,    // 硒鼓状态满或正常
    LIFE_STAT_LOW                     = 1,    // 硒鼓少
    LIFE_STAT_OUT                     = 2,    // 硒鼓无
    LIFE_STAT_NOTSUPP                 = 3,    // 设备不支持该能力
    LIFE_STAT_UNKNOWN                 = 4,    // 不能确定当前状态
};

// status.Retract返回状态(回收箱状态)
enum DEVPTR_RETRACT_STATUS
{
    RETRACT_STAT_OK                  = 0,    // 回收箱正常
    RETRACT_STAT_FULL                = 1,    // 回收箱满
    RETRACT_STAT_HIGH                = 2,    // 回收箱将满
    RETRACT_STAT_MISSING             = 3,    // 找不到设备
    RETRACT_STAT_UNKNOWN             = 4,    // 不能确定当前状态
};

// 打印字体(主要用于 打印参数结构体[Dev_PTR_PrtFont_Param])
// 对应Form.Style设置,参数值对应TextPrinter.h中FS_XXXX宏定义
// 支持多个组合(如倍宽+倍高);
enum FONT_TYPE
{
    FONT_NORMAL                     = 0x00000001,   // 不指定,缺省,正常
    FONT_BOLD                       = 0x00000002,   // 粗体
    FONT_ITALICS                    = 0x00000004,   // 斜体
    FONT_UNDER                      = 0x00000008,   // 一条下划线
    FONT_DOUUNDER                   = 0x00000010,   // 两条下划线
    FONT_DOUBLE                     = 0x00000020,   // 2倍宽
    FONT_TRIPLE                     = 0x00000040,   // 3倍宽
    FONT_QUADRUPLE                  = 0x00000080,   // 4倍宽
    FONT_STRIKETHROUGH              = 0x00000100,   // 删除线
    FONT_ROTATE90                   = 0x00000200,   // 顺时针旋转90度
    FONT_ROTATE270                  = 0x00000400,   // 逆时针旋转270度
    FONT_UPSIDEDOWN                 = 0x00000800,   // 倒置
    FONT_PROPORTIONAL               = 0x00001000,   // 比例间距
    FONT_DOUBLEHIGH                 = 0x00002000,   // 2倍高
    FONT_TRIPLEHIGH                 = 0x00004000,   // 3倍高
    FONT_QUADRUPLEHIGH              = 0x00008000,   // 4倍高
    FONT_CONDENSED                  = 0x00010000,   // 压缩
    FONT_SUPERSCRIPT                = 0x00020000,   // 上标
    FONT_SUBSCRIPT                  = 0x00040000,   // 下标
    FONT_OVERSCORE                  = 0x00080000,   // 上划线
    FONT_LETTERQUALITY              = 0x00100000,   // 字母打印质量
    FONT_NEARLETTERQUALITY          = 0x00200000,   // 近似字母质量
    FONT_DOUBLESTRIKE               = 0x00400000    // 双线
};

// SetData()/GetData()使用执行类别(通用0～50,50以上为各模块自行定义)
#define DTYPE_INIT          0       // 初始化值
#define DTYPE_LIB_PATH      1       // 设置/获取动态库路径
#define DTYPE_SET_PRTFONT   2       // 设置打印字体


// -----------------------------------结构体定义-----------------------------------
// 文档打印机特有状态
typedef struct Dev_DPR_Status
{
    WORD wLife;                 // 硒鼓状态(参考enum DEVPTR_LIFE_STATUS)
    DWORD dwTotal;              // 已打印总张数
    DWORD dwA4Print;            // 已打印 A4 张数
    DWORD dwA5Print;            // 已打印 A5 张数
    WORD wLessToner;            // 硒鼓或粉盒即将耗尽,1表示即将耗尽，0表示硒鼓或粉盒正常

    Dev_DPR_Status()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_DPR_Status));
        wLife = LIFE_STAT_UNKNOWN;
    }
} DEVDPRSTATUS, *LPDEVDPRSTATUS;

typedef struct Dev_PTR_Status   // 处理后的设备状态
{
    WORD wDevice;               // 设备状态(参考enum DEVPTR_DEVICE_STATUS)
    WORD wMedia;                // Media状态(参考enum DEVPTR_MEDIA_STATUS)
    WORD wPaper[16];            // 票箱状态(参考enum DEVPTR_PAPER_STATUS)
    WORD wToner;                // 碳带状态(参考enum DEVPTR_TONER_STATUS)
    WORD wInk;                  // 墨盒状态(参考enum DEVPTR_INK_STATUS)
    struct
    {
        WORD wBin;              // 回收箱状态(参考enum DEVPTR_RETRACT_STATUS)
        USHORT usCount;         // 回收箱张数
    }stRetract[16];
    char szErrCode[8];          // 8位的错误码
    WORD wOtherCode[16];        // 其他状态值,用于非标准WFS/未定义值的返回
    DEVDPRSTATUS stDPRStatus;   // 文档打印机状态
    Dev_PTR_Status()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_PTR_Status));
        wDevice = DEV_STAT_OFFLINE;
        wMedia = MEDIA_STAT_UNKNOWN;
        for (INT i = 0; i < 16; i ++)
        {
            wPaper[i] = PAPER_STAT_UNKNOWN;
            stRetract[i].wBin = RETRACT_STAT_UNKNOWN;
            wOtherCode[i] = 0;
        }
        wToner = TONER_STAT_UNKNOWN;
        wInk = INK_STAT_UNKNOWN;
    }
} DEVPTRSTATUS, *LPDEVPTRSTATUS;

//--------------------PrintForm相关结构体定义--------------------
// PrintForm类别
#define PRINTFORM_TEXT          0x01    // 文本
#define PRINTFORM_PIC           0x02    // 图片
#define PRINTFORM_BAR           0x03    // 条码
#define PRINTFORM_PDF417        0x04    // PDF417码
#define PRINTFORM_QR            0x05    // 二维码
#define PRINTFORM_FEEDLINE      0x06    // 打印换行

// 条码类别
enum BARMODE
{
    BAR_UPCA            = 0,        // UPC-A
    BAR_UPCC            = 1,        // UPC-C
    BAR_JAN13           = 2,        // JAN13/EAN13
    BAR_JAN8            = 3,        // JAN8/EAN8
    BAR_CODE39          = 4,        // CODE39
    BAR_INTE            = 5,        // INTERLEAVED 2 OF 5
    BAR_CODEBAR         = 6,        // CODEBAR
    BAR_CODE93          = 7,        // CODE93
    BAR_CODE128         = 8,        // CODE128
    BAR_PDF417          = 9,        // PDF417
    BAR_QRCODE          = 10,       // QRCODE
};

// PrintForm入参结构体
typedef struct Dev_PTR_PrintForm_InParam
{
    WORD wInMode;           // 类别(参考宏定义 ReadForm类别)
    ULONG ulX, ulY;         // 打印坐标
    LPSTR lpData;           // 入参数据指针
    CHAR szData[65536];     // 入参数据
    DWORD dwDataSize;       // 数据长度
    DWORD dwTimeOut;        // 超时时间
    LONG  lOtherParam[24];  // 其他参数

    Dev_PTR_PrintForm_InParam()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_PTR_PrintForm_InParam));
        lpData = nullptr;
    }
} DEVPRINTFORMIN, *LPDEVPRINTFROMIN,
  PRINTFORMIN, LPPRINTFORMIN;

// PrintForm回参结构体
typedef struct Dev_PTR_PrintForm_OutParam
{
    WORD wInMode;           // 类别(参考宏定义 ReadForm类别)
    WORD wResult;           // 0成功/1失败/2不支持
    LPSTR lpData;           // 返回结果(需显式申请空间)
    CHAR szRetData[65536];  // 返回数据
    DWORD dwRetDataSize;    // 返回数据长度

    Dev_PTR_PrintForm_OutParam()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_PTR_PrintForm_OutParam));
        lpData = nullptr;
    }
} DEVPRINTFORMOUT, *LPDEVPRINTFORMOUT,
  PRINTFORMOUT, LPPRINTFORMOUT;

//--------------------ReadForm相关结构体定义--------------------
// ReadForm类别
#define READFORM_TRACK1             0x01    // 磁道1
#define READFORM_TRACK2             0x02    // 磁道2
#define READFORM_TRACK3             0x04    // 磁道3

// ReadForm入参结构体
typedef struct Dev_PTR_ReadForm_InParam
{
    WORD wInMode;           // 类别(参考宏定义 ReadForm类别)
    LPSTR lpData;
    CHAR szData[65536];     // 入参数据
    DWORD dwDataSize;       // 数据长度
    DWORD dwTimeOut;        // 超时时间
    LONG  lOtherParam[12];  // 其他参数

    Dev_PTR_ReadForm_InParam()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_PTR_ReadForm_InParam));
        lpData = nullptr;
    }
} DEVPTRREADFORMIN, *LPDEVPTRRREADFROMIN,
  READFORMIN, LPREADFORMIN;

// ReadForm回参结构体
typedef struct Dev_PTR_ReadForm_OutParam
{
    WORD wInMode;           // 类别(参考宏定义 ReadForm类别)
    WORD wResult;           // 0成功/1失败/2不支持
    LPSTR lpData;           // 返回结果(需显式申请空间)
    CHAR szRetData[65536];  // 返回数据
    DWORD dwRetDataSize;    // 返回数据长度

    Dev_PTR_ReadForm_OutParam()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_PTR_ReadForm_OutParam));
        lpData = nullptr;
    }
} DEVPTRREADFORMOUT, *LPDEVPTRREADFORMOUT,
  READFORMOUT, LPREADFORMOUT;

//--------------------ReadImage相关结构体定义--------------------
// ImageSourec类别
#define IMAGE_MODE_FRONT            0x01    // 扫描正面图像
#define IMAGE_MODE_BACK             0x02    // 扫描背面图像
#define IMAGE_MODE_CODELINE         0x04    // 获取票据号并鉴伪
#define IMAGE_MODE_UPPER            0x08    // 从1号票箱获取票号等信息
#define IMAGE_MODE_LOWER            0x10    // 从2号票箱获取票号等信息
#define IMAGE_MODE_EXTERNAL         0x20    // 从3号票箱获取票号等信息
#define IMAGE_MODE_AUX              0x40    // 从4号票箱获取票号等信息
#define IMAGE_MODE_RFID             0x80    // 获取RFID数据

// ImageType类别
#define IMAGE_TYPE_TIF              0x01    // TIF6.0
#define IMAGE_TYPE_WMF              0x02    // WMF(windows Metafile)
#define IMAGE_TYPE_BMP              0x03    // BMP
#define IMAGE_TYPE_JPG              0x04    // JPG

// ImageColor类别
#define IMAGE_COLOR_BIN             0x01    // 二进位
#define IMAGE_COLOR_GARY            0x02    // 灰度图
#define IMAGE_COLOR_FULL            0x03    // 全色

// ImageCodeLine类别
#define IMAGE_CODE_CMC7             0x01    // CMC7
#define IMAGE_CODE_E13B             0x02    // E13B
#define IMAGE_CODE_OCR              0x03    // OCR

// ReadImage入参结构体
typedef struct Dev_PTR_ReadImage_InParam
{
    WORD wInMode;               // 类别(参考宏定义 ImageSource类别)
    WORD wNoteType;             // 票据类别(参考宏定义 票据类别)
    WORD wTimeOut;
    WORD wImageType[2];         // 图像格式(参考宏定义 ImageType类别)
    WORD wColorFormat[2];       // 颜色格式(参考宏定义 ImageColor类别)
    WORD wCodeLine;             // 代码行格式(参考宏定义 ImageCodeLine类别)
    LONG lOtherParam[12];       // 其他参数
    CHAR szImageFrontPath[1024];// 正面图像路径
    CHAR szImageBackPath[1024]; // 背面图像路径
    LPSTR lpData;
    ULONG ulDataSize;

    Dev_PTR_ReadImage_InParam()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_PTR_ReadImage_InParam));
        if (lpData != nullptr)
        {
            delete[] lpData;
            lpData = nullptr;
            ulDataSize = 0;
        }
    }
} DEVPTRREADIMAGEIN, *LPDEVPTRREADIMAGEIN,
  READIMAGEIN, LPREADIMAGEIN;

// ReadImage回参结构体
#define READIMAGE_RET_OK        0   // 数据获取正常/鉴别结果为真
#define READIMAGE_RET_NOTSUPP   1   // 不支持获取该
#define READIMAGE_RET_MISSING   2   // 数据获取失败/鉴别结果为假
typedef struct Dev_PTR_ReadImage_OutParam
{
    WORD wInMode;     // 类别
    WORD wResult;     // 参考READIMAGE_RET_XXXX
    LPSTR lpData;
    ULONG ulDataSize;

    Dev_PTR_ReadImage_OutParam()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_PTR_ReadImage_OutParam));
        if (lpData != nullptr)
        {
            delete[] lpData;
            lpData = nullptr;
            ulDataSize = 0;
        }
    }
} DEVPTRREADIMAGEOUT, *LPDEVPTRREADIMAGEOUT,
  READIMAGEOUT, LPREADIMAGEOUT;

// 打印参数结构体
typedef struct Dev_PTR_PrtFont_Param
{
    WORD wFontMode;         // 打印字体格式(点阵/TrueType等,各模块自行定义值)
    CHAR szFontName[256];   // 打印字体名(例如:黑体,宋体,sinhei等)
    CHAR szFontPath[256];   // 打印字体路径(例如:ttf字体路径的加载等)
    WORD wFontSize[4];      // 字号
    DWORD dwFontType;       // 字体类型(粗体,斜体等)(参考enum FONT_TYPE)
    DWORD dwMarkPar[12];    // 辅助参数(无具体定义,根据实际需要定义)

    Dev_PTR_PrtFont_Param()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(Dev_PTR_PrtFont_Param));
    }
} DEVPTRFONTPAR, *LPDEVPTRFONTPAR;


// ---------------------------------设备处理结构体定义---------------------------------
struct IDevPTR
{
    /************************************************************
    ** 功能：释放接口
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual void Release() = 0;

    /************************************************************
    ** 功能: 打开与设备的连接
    ** 输入: pMode: 自定义OPEN参数字串
    **       串口: 格式为: "COMX: BaudRate,Parity,DataBits,StopBits"
    **                   (例如: "COM2:115200,N,8,1")
    **       USB: 格式自定义
    **       其他: 格式自定义
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Open(const char *pMode) = 0;

    /************************************************************
    ** 功能: 关闭与设备的连接
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Close() = 0;

    /************************************************************
    ** 功能: 设备初始化
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Init() = 0;

    /************************************************************
    ** 功能: 设备复位
    ** 输入: 无
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int Reset() = 0;

    /************************************************************
    ** 功能: 设备复位
    ** 输入: enMediaAct 介质动作
    **      usParam    介质参数
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam = 0) = 0;

    /************************************************************
    ** 功能: 取设备状态
    ** 输入: 无
    ** 输出: stStatus 设备状态结构体
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int GetStatus(DEVPTRSTATUS &stStatus) = 0;

    /************************************************************
    ** 功能: 打印字串(无指定打印坐标)
    ** 输入: pStr        要打印的字串
    **      ulDataLen   数据长度，如果为-1，pStr以\0结束
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int PrintData(const char *pStr, unsigned long ulDataLen) = 0;

    /************************************************************
    ** 功能: 打印字串(指定打印坐标)
    ** 输入: pStr        要打印的字串
    **      ulDataLen   数据长度，如果为-1，pStr以\0结束
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int PrintDataOrg(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY) = 0;

    /************************************************************
    ** 功能: 图片打印(无指定打印坐标)
    ** 输入: szImagePath      图片保存路径
    **      ulWidth          打印宽度
    **      ulHeigh          打印高度
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight) = 0;

    /************************************************************
    ** 功能：指定坐标打印图片
    ** 输入：szImagePath：图片保存绝对路径
            unsigned long ulOrgX: 打印图片的列坐标，单位: 0.1mm
            unsigned long ulOrgY: 打印图片的行坐标，单位: 0.1mm
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY) = 0;

    /************************************************************
    ** 功能: PrintForm打印
    ** 输入: stPrintIn      入参
    ** 输出: stPrintOut     回参
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int PrintForm(DEVPRINTFORMIN stPrintIn, DEVPRINTFORMOUT &stPrintOut)
    {
        return PTR_SUCCESS;
    }

    /************************************************************
    ** 功能: ReadForm获取
    ** 输入: stReadIn      入参
    ** 输出: stPrintOut     回参
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut) = 0;

    /************************************************************
    ** 功能: ReadImage获取
    ** 输入: stScanIn      获取类别
    ** 输出: stScanOut     获取到的数据
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut) = 0;

    /************************************************************
    ** 功能: 介质控制
    ** 输入: enMediaAct    介质动作
    **      usParam       介质参数,缺省0
    ** 输出: 无
    ** 返回: 见返回错误码定义
    ************************************************************/
    virtual int MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam = 0) = 0;

    /************************************************************
    ** 功能：设置数据
    ** 输入：vData        传入数据
    **      wDataType    传入数据类型
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int SetData(void *vData, WORD wDataType = 0) = 0;

    /************************************************************
     ** 功能：获取数据
     ** 输入：wDataType 　获取数据类型
     ** 输出：vData       获取数据
     ** 返回：见返回错误码定义
     ************************************************************/
    virtual int GetData(void *vData, WORD wDataType = 0) = 0;

    /************************************************************
     ** 功能：获取版本
     ** 输入：lSize       数据buffer size
     **      usType      获取类型(1DEVPTR版本/2固件版本/3其他)
     ** 输出：szVer       版本数据
     ** 返回：见返回错误码定义
     ************************************************************/
    virtual void GetVersion(char* szVer, long lSize, ushort usType) = 0;

};


/************************************************************
** 功能: 获取设备连接handle
** 输入: lpDevType: 设备类型
** 输出: iPrtDevHandle
** 返回: 见返回错误码定义
************************************************************/
extern "C" DEVPTR_EXPORT long CreateIDevPTR(const char *lpDevType, IDevPTR *&pDev);


// ---------------------------------设备相关变量转换---------------------------------
class ConvertVar
{
private:
    CHAR m_szErrStr[1024];
public:
    // 设备状态转换为WFS格式
    WORD ConvertDeviceStatus(WORD wDevStat)
    {
        switch (wDevStat)
        {
            case DEV_STAT_ONLINE     /* 设备正常 */     : return WFS_PTR_DEVONLINE;
            case DEV_STAT_OFFLINE    /* 设备脱机 */     : return WFS_PTR_DEVOFFLINE;
            case DEV_STAT_POWEROFF   /* 设备断电 */     : return WFS_PTR_DEVPOWEROFF;
            case DEV_STAT_NODEVICE   /* 设备不存在 */    : return WFS_PTR_DEVNODEVICE;
            case DEV_STAT_HWERROR    /* 设备故障 */     : return WFS_PTR_DEVHWERROR;
            case DEV_STAT_USERERROR  /*  */             : return WFS_PTR_DEVUSERERROR;
            case DEV_STAT_BUSY       /* 设备读写中 */    : return WFS_PTR_DEVBUSY;
            defaule: return WFS_PTR_DEVOFFLINE;
        }
    }

    // Media状态转换为WFS格式
    WORD ConvertMediaStatus(WORD wMediaStat)
    {
        switch (wMediaStat)
        {
            case MEDIA_STAT_PRESENT   /* 通道内有 */               : return WFS_PTR_MEDIAPRESENT;
            case MEDIA_STAT_NOTPRESENT/* 通道内无 */               : return WFS_PTR_MEDIANOTPRESENT;
            case MEDIA_STAT_JAMMED    /* 通道内有且票被夹住 */       : return WFS_PTR_MEDIAJAMMED;
            case MEDIA_STAT_NOTSUPP   /* 不支持检测通道内是否有 */    : return WFS_PTR_MEDIANOTPRESENT;
            case MEDIA_STAT_UNKNOWN   /* 通道内状态未知 */           : return WFS_PTR_MEDIAUNKNOWN;
            case MEDIA_STAT_ENTERING  /* 在出票口 */                : return WFS_PTR_MEDIAENTERING;
            default: return WFS_PTR_MEDIAUNKNOWN;
        }
    }

    WORD ConvertPaper2MediaStatus(WORD wPaperStat)
    {
        switch (wPaperStat)
        {
            case PAPER_STAT_FULL      /* 满 */             :
            case PAPER_STAT_LOW       /* 少 */             : return WFS_PTR_MEDIAPRESENT;
            case PAPER_STAT_OUT       /* 无 */             : return WFS_PTR_MEDIANOTPRESENT;
            case PAPER_STAT_NOTSUPP   /* 设备不支持该能力 */  : return WFS_PTR_MEDIANOTSUPP;
            case PAPER_STAT_UNKNOWN   /* 不能确定当前状态 */  : return WFS_PTR_MEDIAUNKNOWN;
            case PAPER_STAT_JAMMED    /* 被卡住 */          : return WFS_PTR_MEDIAJAMMED;
            defaule: return WFS_PTR_MEDIAUNKNOWN;
        }
    }

    // Paper状态转换为WFS格式
    WORD ConvertPaperStatus(WORD wPaperStat)
    {
        switch (wPaperStat)
        {
            case PAPER_STAT_FULL      /* 满 */             : return WFS_PTR_PAPERFULL;
            case PAPER_STAT_LOW       /* 少 */             : return WFS_PTR_PAPERLOW;
            case PAPER_STAT_OUT       /* 无 */             : return WFS_PTR_PAPEROUT;
            case PAPER_STAT_NOTSUPP   /* 设备不支持该能力 */  : return WFS_PTR_PAPERNOTSUPP;
            case PAPER_STAT_UNKNOWN   /* 不能确定当前状态 */  : return WFS_PTR_PAPERUNKNOWN;
            case PAPER_STAT_JAMMED    /* 被卡住 */          : return WFS_PTR_PAPERJAMMED;
            default: return WFS_PTR_PAPERUNKNOWN;
        }
    }

    // Toner状态转换为WFS格式
    WORD ConvertTonerStatus(WORD wTonerStat)
    {
        switch (wTonerStat)
        {
            case TONER_STAT_FULL      /* 碳带状态满或正常 */    : return WFS_PTR_TONERFULL;
            case TONER_STAT_LOW       /* 碳带少 */            : return WFS_PTR_TONERLOW;
            case TONER_STAT_OUT       /* 碳带无 */            : return WFS_PTR_TONEROUT;
            case TONER_STAT_NOTSUPP   /* 设备不支持该能力 */    : return WFS_PTR_TONERNOTSUPP;
            case TONER_STAT_UNKNOWN   /* 不能确定当前状态 */    : return WFS_PTR_TONERUNKNOWN;
            default: return WFS_PTR_TONERUNKNOWN;
        }
    }

    // Ink状态转换为WFS格式
    WORD ConvertInkStatus(WORD wInkStat)
    {
        switch (wInkStat)
        {
            case INK_STAT_FULL    /* 墨盒状态满或正常 */  : return WFS_PTR_INKFULL;
            case INK_STAT_LOW     /* 墨盒少 */           : return WFS_PTR_INKLOW;
            case INK_STAT_OUT     /* 墨盒无 */           : return WFS_PTR_INKOUT;
            case INK_STAT_NOTSUPP /* 设备不支持该能力 */  : return WFS_PTR_INKNOTSUPP;
            case INK_STAT_UNKNOWN /* 不能确定当前状态 */  : return WFS_PTR_INKUNKNOWN;
            default: return WFS_PTR_INKUNKNOWN;
        }
    }

    // Retract状态转换为WFS格式
    WORD ConvertRetractStatus(WORD wRetractStat)
    {
        switch (wRetractStat)
        {
            case RETRACT_STAT_OK/* 回收箱正常 */   : return WFS_PTR_RETRACTBINOK;
            case RETRACT_STAT_FULL/* 回收箱满 */   : return WFS_PTR_RETRACTBINFULL;
            case RETRACT_STAT_HIGH/* 回收箱将满 */   : return WFS_PTR_RETRACTBINHIGH;
            default: return WFS_PTR_RETRACTBINOK;
        }
    }

    // 错误码转换为WFS格式
    LONG ConvertErrCode(INT nRet)
    {
        switch (nRet)
        {
        case PTR_SUCCESS:           return WFS_SUCCESS;
        case ERR_PTR_PARAM_ERR:     return WFS_ERR_UNSUPP_DATA;
        case ERR_PTR_COMM_ERR:      return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_NO_PAPER:      return WFS_ERR_PTR_PAPEROUT;
        case ERR_PTR_JAMMED:        return WFS_ERR_PTR_PAPERJAMMED;
        case ERR_PTR_NOT_OPEN:      return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_HEADER:        return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_CUTTER:        return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_TONER:         return WFS_ERR_HARDWARE_ERROR;      // INK或色带故障
        case ERR_PTR_STACKER_FULL:  return WFS_ERR_PTR_STACKERFULL;     // 用户没有取走
        case ERR_PTR_NO_RESUME:     return WFS_ERR_HARDWARE_ERROR;      // 不可恢复的错误
        case ERR_PTR_CAN_RESUME:    return WFS_ERR_HARDWARE_ERROR;      // 可恢复的错误
        case ERR_PTR_FORMAT_ERROR:  return WFS_ERR_UNSUPP_DATA;         // 打印字串格式错误
        case ERR_PTR_CHRONIC:       return WFS_ERR_HARDWARE_ERROR;      // 软件故障
        case ERR_PTR_HWERR:         return WFS_ERR_HARDWARE_ERROR;      // 硬件故障
        case ERR_PTR_IMAGE_ERROR:   return WFS_ERR_HARDWARE_ERROR;      // 图片相关错误
        case ERR_PTR_NO_DEVICE:     return WFS_ERR_HARDWARE_ERROR;      // 指定名的设备不存在
        case ERR_PTR_UNSUP_CMD:     return WFS_ERR_UNSUPP_COMMAND;      // 不支持的指令
        case ERR_PTR_DATA_ERR:      return WFS_ERR_HARDWARE_ERROR;      // 收发数据错误
        case ERR_PTR_TIMEOUT:       return WFS_ERR_TIMEOUT;             // 超时
        case ERR_PTR_DRVHND_ERR:    return WFS_ERR_HARDWARE_ERROR;      // 驱动错误
        case ERR_PTR_DRVHND_REMOVE: return WFS_ERR_HARDWARE_ERROR;      // 驱动丢失
        case ERR_PTR_USB_ERR:       return WFS_ERR_HARDWARE_ERROR;      // USB/COM/连接错误
        case ERR_PTR_DEVBUSY:       return WFS_ERR_HARDWARE_ERROR;      // 设备忙
        case ERR_PTR_OTHER:         return WFS_ERR_HARDWARE_ERROR;      // 其它错误，如调用API错误等
        case ERR_PTR_DEVUNKNOWN:    return WFS_ERR_HARDWARE_ERROR;      // 设备未知
        case ERR_PTR_NOMEDIA:       return WFS_ERR_PTR_NOMEDIAPRESENT;  // 指定位置无介质
        case ERR_PTR_HAVEMEDIA:     return WFS_ERR_HARDWARE_ERROR;      // 指定位置有介质
        case ERR_PTR_PAPER_ERR:     return WFS_ERR_PTR_MEDIAINVALID;    // 介质异常
        case ERR_PTR_JSON_ERR:      return WFS_ERR_HARDWARE_ERROR;      // JSON处理错误
        case ERR_PTR_SCAN_FAIL:     return WFS_ERR_HARDWARE_ERROR;      // 扫描失败
        case ERR_PTR_DATA_DISCERN:  return WFS_ERR_HARDWARE_ERROR;      // 数据识别失败
        case ERR_PTR_NO_MEDIA:      return WFS_ERR_PTR_NOMEDIAPRESENT;  // 通道无纸
        case ERR_PTR_RETRACTFULL:   return WFS_ERR_PTR_RETRACTBINFULL;  // 回收箱满
        case ERR_PTR_MS_BLANK:      return WFS_ERR_PTR_MEDIAINVALID;    // 无效数据 // 空白磁条/磁条无效
        case ERR_PTR_OFFLINE:       return WFS_ERR_HARDWARE_ERROR;      // 设备断线
        default:                    return WFS_ERR_HARDWARE_ERROR;
        }
    }

    CHAR* ConvertErrCodeToStr(INT nRet)
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
            case ERR_PTR_NO_PAPER:
                sprintf(m_szErrStr, "%d|%s", nRet, "打印机缺纸");
                return m_szErrStr;
            case ERR_PTR_JAMMED:
                sprintf(m_szErrStr, "%d|%s", nRet, "堵纸等机械故障");
                return m_szErrStr;
            case ERR_PTR_NOT_OPEN:
                sprintf(m_szErrStr, "%d|%s", nRet, "设备没有打开");
                return m_szErrStr;
            case ERR_PTR_HEADER:
                sprintf(m_szErrStr, "%d|%s", nRet, "打印头故障");
                return m_szErrStr;
            case ERR_PTR_CUTTER:
                sprintf(m_szErrStr, "%d|%s", nRet, "切刀故障");
                return m_szErrStr;
            case ERR_PTR_TONER:
                sprintf(m_szErrStr, "%d|%s", nRet, "INK或色带故障");
                return m_szErrStr;
            case ERR_PTR_STACKER_FULL:
                sprintf(m_szErrStr, "%d|%s", nRet, "用户没有取走");
                return m_szErrStr;
            case ERR_PTR_NO_RESUME:
                sprintf(m_szErrStr, "%d|%s", nRet, "不可恢复的错误");
                return m_szErrStr;
            case ERR_PTR_CAN_RESUME:
                sprintf(m_szErrStr, "%d|%s", nRet, "可恢复的错误");
                return m_szErrStr;
            case ERR_PTR_FORMAT_ERROR:
                sprintf(m_szErrStr, "%d|%s", nRet, "打印字串格式错误");
                return m_szErrStr;
            case ERR_PTR_CHRONIC:
                sprintf(m_szErrStr, "%d|%s", nRet, "软件故障");
                return m_szErrStr;
            case ERR_PTR_HWERR:
                sprintf(m_szErrStr, "%d|%s", nRet, "硬件故障");
                return m_szErrStr;
            case ERR_PTR_IMAGE_ERROR:
                sprintf(m_szErrStr, "%d|%s", nRet, "图片相关错误");
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
                sprintf(m_szErrStr, "%d|%s", nRet, "其它错误或未知错误");
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
            case ERR_PTR_JSON_ERR:
                sprintf(m_szErrStr, "%d|%s", nRet, "JSON处理错误");
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
            case ERR_PTR_RETRACTFULL:
                sprintf(m_szErrStr, "%d|%s", nRet, "回收箱满");
                return m_szErrStr;
            case ERR_PTR_MS_BLANK:
                sprintf(m_szErrStr, "%d|%s", nRet, "空白磁条/磁条无效");
                return m_szErrStr;
            case ERR_PTR_OFFLINE:
                sprintf(m_szErrStr, "%d|%s", nRet, "设备断线");
                return m_szErrStr;
            default:
                sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
                return m_szErrStr;
        }

        return m_szErrStr;
    }

};



