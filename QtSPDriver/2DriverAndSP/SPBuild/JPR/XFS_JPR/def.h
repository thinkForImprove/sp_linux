#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义

//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"JPR00010100"};      // XFS_JPR 版本号
static const BYTE byDevVRTU[17] = {"Dev010100"};        // DevPTR 版本号
// 设备类型
#define DEV_BTT080AII           0               // 新北洋BT-T080AII打印机

#define IDEV_BTT080AII          "BT-T080AII"    // 新北洋BT-T080AII打印机

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_PRT_FORMAT          51              // 设置打印格式
#define SET_DEV_RECON           52              // 设置断线重连标记
#define SET_DEV_OPENMODE        53              // 设备打开模式
#define SET_DEV_LINEFEED        54              // 打印换行
#define SET_DEV_ROWHEIGHT       55              // 设置行高

// GetVersion()使用
#define GET_VER_DEV             1               // DevJPR版本号
#define GET_VER_FW              2               // 固件版本号

// 缺省行高
#define LINE_HEIGHT_DEF         30              // 30(单位0.1MM)


// 新增打印模式结构体
typedef struct _print_mode
{
    BOOL bPageMode = TRUE;      // 定义打印模式(TRUE:页模式/FALSE:行模式)
    WORD wLeft = 0;             // 定义介质打印左边距，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
    WORD wTop = 0;              // 定义介质打印上边距，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
    WORD wWidth = 0;            // 定义介质可打印宽度，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
    WORD wHeight = 0;           // 定义介质可打印高度，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
    WORD wRowHeight = 0;        // 定义介质可打印行高，单位:0.1毫米(值为0则采用缺省值或根据打印内容自行设定)
} STPRINTMODE, *LPSTPRINTMODE;

// 一个字节(char)按8位获取数据定义
#define     DATA_BIT0       (0x01)  // 第一位
#define     DATA_BIT1       (0x02)
#define     DATA_BIT2       (0x04)
#define     DATA_BIT3       (0x08)
#define     DATA_BIT4       (0x10)
#define     DATA_BIT5       (0x20)
#define     DATA_BIT6       (0x40)
#define     DATA_BIT7       (0x80)

// 设备打开模式
typedef struct st_DevOpen_mode
{
    INT nOpenMode;          // 打开模式
    INT nOpenParam;         // 模式参数
    CHAR szOpenParam[64];   // 模式参数
    st_DevOpen_mode()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_DevOpen_mode));
    }
} STDEVOPENMODE, *LPSTDEVOPENMODE;

#endif // DEF_H
