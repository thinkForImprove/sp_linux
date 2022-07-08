#pragma once

#pragma pack(push,1)

// Device State
#define	DEV_OFFLINE				(0)
#define	DEV_NORMAL				(1)
#define	DEV_HWERR				(2)
#define	DEV_JAM					(3)
#define	DEV_BUSY				(4)
#define	DEV_UNKNOWN				(128)

//WPaper
#define     PRINT_PAPER_FULL                      (0)
#define     PRINT_PAPER_LOW                       (1)
#define     PRINT_PAPER_OUT                       (2)
#define     PRINT_PAPER_NOTSUPP                   (3)
#define     PRINT_PAPER_UNKNOWN                   (4)
#define     PRINT_PAPER_JAMMED                    (5)

//wMeida
#define     PRINT_MEDIA_PRESENT                   (0)
#define     PRINT_MEDIA_NOTPRESENT                (1)
#define     PRINT_MEDIA_JAMMED                    (2)
#define     PRINT_MEDIA_UNKNOWN                   (4)
#define     PRINT_MEDIA_ENTERING                  (5)

//WPresenter
#define     PRINT_PRESENTER_NOT_PAPER             (0)
#define     PRINT_PRESENTER_HAVE_PAPER            (1)

// mode of bitmap
#define     BITMAP_MODE_8SINGLE_DENSITY           (0)   // 8单密度
#define     BITMAP_MODE_8DOUBLE_DENSITY           (1)	// 2双密度
#define     BITMAP_MODE_24SINGLE_DENSITY          (2)	// 24单密度
#define     BITMAP_MODE_24DOUBLE_DENSITY          (3)	// 24双密度

/*
硬件命令相关的数据结构和宏定义。
*/
#define     BKC310_DEV_MODEL_NAME                  "BK-C310"
#define     BTNH80_DEV_MODEL_NAME                  "BT-NH80"
#define     BK_COMMAND_READ_TIMEOUT                (2000)


//BK-TO80II ERRORCODE
#define     PTR_PAPER_JAMMED_PRINT_POSITION         "Paper Jammed at Print Position"	        // 打印处卡纸错误
#define     PTR_PAPER_JAMMED_PRESENTER          	"Paper Jammed at Presenter"                 // 打印头Presenter卡纸错误
#define     PTR_PRINTERHEAD_UNCOVER             	"Print head uncover"                        // 打印头抬起(上盖打开)
#define     PTR_PRINTERHEAD_TMPERATURE_HIGH         "Print head tmpererature high"              // 打印头温度高
#define     PTR_CUTTER_ERROR                        "Print Cutter Error"                        // 切刀错误
#define     PTR_MARK_NOT_FIND                       "Print Mark not find"                       // 找不到标记
#define     PTR_AUTO_PRESENT_ERROR                  "Print auto present error"                  // 自动上纸错误

//一个字节(char)按8位获取数据定义
#define     DATA_BIT0       (0x01)  //第一位
#define     DATA_BIT1       (0x02)
#define     DATA_BIT2       (0x04)
#define     DATA_BIT3       (0x08)
#define     DATA_BIT4       (0x10)
#define     DATA_BIT5       (0x20)
#define     DATA_BIT6       (0x40)
#define     DATA_BIT7       (0x80)

//代码页语言名称
#define		PTR_CHARSET_CHN					(0x0000)    //简体中文
#define		PTR_CHARSET_ENGUS				(0x0001)	//英语(美国)
#define		PTR_CHARSET_FRA					(0x0002)	//法语
#define		PTR_CHARSET_ARB					(0x0003)	//阿拉伯语
#define		PTR_CHARSET_PORT				(0x0004)	//葡萄牙语
#define		PTR_CHARSET_FARSI				(0x0005)	//波斯语

#define     PRINT_LINE_MODE                   0
#define     PRINT_PAGE_MODE                   1

//设备实时状态结构
//typedef struct REALTIME_DEVSTATUS_BT_NH80M
//{
//    char szPrinterState1;               //打印机状态1，对应(0x10,0x04,0x05)
//    char szPrinterState2;               //打印机状态1，对应(0x10,0x04,0x06)
//    char szPresenterState;              //Presenter状态，对应(0x10,0x04,0x09)
//}RealtimeDevStatus_BKC310, *LPRealtimeDevStatus_BKC310;

typedef struct tag_PrinterStatus
{
    WORD wDevState;
    WORD wMedia;
    WORD wPaper;
    BOOL bPaperInPresenter;
    //CHAR sErrorCode[MAX_ERRCODE_LEN];
    //CHAR sDescription[MAX_DESC_LEN];
} PrintStatus, *LPPrintStatus;

// Page Option
typedef struct tag_BKRprPageOption
{
    unsigned int nCodePage;             //代码页
    unsigned int nHeadLines;            //打印顶端空出行数
    unsigned int nRowInterval;          //设置打印区域的行间距
    unsigned int nMinLines;             //最小页面的行数
    unsigned int nCharRightInternal;    //字符右间距
} BKRprPageOption, *LPBKRprPageOption;

// PTR Option
typedef struct tag_BKRprPtrOption
{
    unsigned int  nPresenterMode;       //Presenter动作模式
    unsigned char cPresenterWaitTime;   //Presenter动作等待时间
    bool          bIsBlackMark;         //标记纸黑标
    unsigned int  nPaperFeedBeforeCut;   //切纸前走纸距离
    BKRprPageOption    pageOption;
} BKRprOption, *LPBKRprOption;

typedef struct _FONTFORMAT{
    unsigned int nFontType;
    bool bBold;
    unsigned int nCharHorMagnifyTimes;        //1~6 is vaild
    unsigned int nCharVerMagnifyTimes;       //1~6 is vaild
    bool bDoubleWidthAndHeight;
    bool bUpsideDown;
    bool bOppositeColor;
    bool bUnderLine;
    bool bRotate90;
}FONTFORMAT;



#pragma pack(pop)
