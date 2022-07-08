#ifndef PRX_H
#define PRX_H

#include <string.h>

#define SUCCESS 0

// 用户调用不正确，报错
#define ERR_UNKNOW             -1   //未知错误
#define ERR_PARAM_INVALID      -2   //参数无效
#define ERR_NO_OPEN            -3   //未调用过OPEN函数/调用不成功
#define ERR_NOT_OLI_EMULATION  -4   //未调用过OPEN函数/调用不成功

// 设备创建和不支持，报错
#define ERR_DEV_CREATE_FAILURE -11        //创建设备失败
#define ERR_DEV_NO_SUPPORT     -12        //设备不支持

// 系统通信，报错
#define ERR_COMM_OPEN          -21        //通信打开出错
#define ERR_COMM_WRITE         -22        //通信写出错
#define ERR_COMM_READ          -23        //通信读出错
#define ERR_COMM_PARITY        -24        //通信校验错
#define ERR_TIMEOUT            -25        //通信超时
#define ERR_COMM_OFF           -26        //设备离线


// 硬件报错
#define ERR_MEDIA_NO_FOUND     -31       //无纸
#define ERR_MS_BLANK           -32        //空白磁条
#define ERR_MS_READ_OR_PARITY  -33        //磁条读或校验出错

//
#define  ERR_BMP                      -40
#define  ERR_BMP_NOT_BMP              ERR_BMP -1
#define  ERR_BMP_NOT_SUPPORT_COLOR    ERR_BMP -2
#define  ERR_BMP_NOT_UNCOPRESSED      ERR_BMP -3
#define  ERR_BMP_SIZE                 ERR_BMP -4

//状态码：
#define STATUS_NOMAL					0   //状态正常
#define STATUS_MEDIA_NONE			    1   //无介质
#define STATUS_MEDIA_PREENTERING        2   //介质存在、位于入口
#define STATUS_MEDIA_ENTERING           3   //介质已对齐
#define STATUS_MEDIA_PRESENT			4   //介质已插入
#define STATUS_MEDIA_INSERTED_ALL       5   //至页顶
#define STATUS_MEDIA_NEAR_END           6   //至页尾
#define STATUS_MEDIA_JAMMED             7   //介质阻塞
#define STATUS_MEDIA_MAGNETIC_UNREAD    8   //磁条不能读出
#define STATUS_COVER_OPEN               9   //处于开盖状态
#define STATUS_OFFLINE                  9   //处于离线状态
#define STATUS_COMMAND_WRONG           10   //命令错
#define STATUS_COMM_ERROR              11    //通信错
#define STATUS_ERROR_UNKNOW            12    //未知状态

#define DECIMILLIMETER_PER_INCH        254
#define BASIC_PACE                     216


#define MIN_PAGE_LENGTH                700   // 700 *0.1mm
#define MAX_PAGE_LENGTH                26694  // 4800 *0.1mm

#pragma pack(push,1)
typedef struct tagPOINT
{
    int  x;
    int  y;
}POINT,*PPOINT;
#pragma pack(pop)

/******************\
扫描控制参数
\******************/
#pragma pack(push,1)
typedef struct tagScanCtrl {

    long dpi;

    long ejectMode; // Eject Mode （0：不退；1：前退；2：后退）

    tagScanCtrl()
    {
        dpi = 300;

        ejectMode = 1;
    }

} ScanCtrl;
#pragma pack(pop)

/******************\
扫描控制参数
\******************/
#pragma pack(push,1)
typedef struct tagScanImageCtrl {

    long cisColorMode; //扫描光调色模式（0：Red  1：Green  2：red+green+blue 3：blue 4：真彩色RGB）

    long grayMode;  //灰度（0--黑白；1--16色；2--256色/24位真彩色； 3--不支持/不做扫描）

    long brightness; // 图片亮度(1--255)

    long thresholdLevel; //黑白扫描包容度

    char *saveImagePath; //保存扫描结果的文件路径

    POINT origin;

    POINT end;

    long scanDirection; //获取扫描图像的方向（0：镜像--正向扫描； 1：正像---反向扫描）

    tagScanImageCtrl()
    {
        cisColorMode = 2;

        grayMode = 1;

        brightness = 100;

        thresholdLevel = 100;

        memset(&origin, 0, sizeof(POINT));

        memset(&end, 0, sizeof(POINT));

        scanDirection = 1;

    }
} ScanImageCtrl;
#pragma pack(pop)

/******************************************************************\
方法名：	long PRxOpen(char *pszCommPort,
                      long lCurEmulation,
                      long lDestEmulation,
                      char *pszCommCfgFilePath,
                      long *plErrorCode,
                      long lBaudrate)

功能说明：	指定PR仿真模式，打开PR通信端口连接。

            只有该方法调用并且成功后，其他方法才有效。

输入参数：	pszCommPort --- 通信端口（如: "COM1"、"USB"）

            lCurEmulation --- 当前PR仿真模式（ 0 - - OLIVETTI,
                                               1 - - OKI,
                                               2 - - IBM
                                               3 - - LQ）

            lDestEmulation --- 目标仿真模式（ 0 - - OLIVETTI,
                                              1 - - OKI,
                                              2 - - IBM
                                              3 - - LQ）

            pszCommCfgFilePath --- 配置文件完整路径

            lBaudrate --- 波特率（如: 9600,19200）


输出参数：	lErrorCode: 错误码（0：无错；其他：有错（包括参数错等）

返回：	非零(设备控制句柄) --- 成功；

        0 --- 失败
\******************************************************************/
long PRxOpen(const char *pszCommPort,
          long lCurEmulation,
          long lDestEmulation,
          char *pszCommCfgFilePathr,
          long *plErrorCode,
          long lBaudrate);


/***************************************************************************\
方法原型：long PRxClose(long hComm)

功能说明：断开PR通信连接, 释放关联资源。

输入参数：hComm：由Open所得到的通讯句柄

输出参数： 无

返回：成功：0；其他－－失败

注释：在串口通讯使用完毕后，必须调用此接口，以释放相关系统资源

\***************************************************************************/
long PRxClose(long hComm);


/******************************************************************\
方法名：	long PRxCleanError(long hComm)

功能说明：	打印机清错。

            只有该方法调用并且成功后，其他方法才有效。

输入参数：	hComm: 设备控制句柄（来自方法Open）

输出参数：	无

返回：	0 －－成功；其他（包括参数错等）－－失败

\******************************************************************/
long PRxCleanError(long hComm);

/******************************************************************\
方法名：	long PRxResetInit(long hComm)

功能说明：	打印机复位初始化。

            只有该方法调用并且成功后，其他方法才有效。

输入参数：	hComm: 设备控制句柄（来自方法Open）

输出参数：	无

返回：	0 －－成功；其他（包括参数错等）－－失败

\******************************************************************/
long PRxResetInit(long hComm);

/******************************************************************\
方法名：	long PRxGetStatus( long hComm, unsigned char *devCode)

功能说明：	获取打印机状态。

输入参数：	hComm: 设备控制句柄（来自方法Open）

输出参数：	devCode：设备返回状态信号

返回： 0  －－ 正常（STATUS_NOMAL）；

       1  －－ 缺纸；（STATUS_MEDIA_NONE）

       5  －－ 介质完全进入打印机（STATUS_MEDIA_INSERTED_ALL）；

       6  －－ 至页尾行首（STATUS_MEDIA_NEAR_END）；

       7  －－ 卡纸（STATUS_MEDIA_JAMMED）

       9  －－ 开盖或脱机（STATUS_COVER_OPEN / STATUS_OFFLINE）；

      10  －－ 命令错（STATUS_COMMAND_WRONG）

      11  －－ 打印机报通信错（STATUS_COMM_ERROR）

      12  －－ 打印机报未知错（STATUS_ERROR_UNKNOW）

      其他（包括参数错等） < 0
\******************************************************************/
long PRxGetStatus( long hComm, unsigned char *devCode);


/******************************************************************\
方法名：	long PRxGetMediaStatus( long hComm, unsigned char *devCode )

功能说明：	获取打印机状态。

输入参数：	hComm: 设备控制句柄（来自方法Open）

输出参数：	devCode：设备返回状态信号

返回：	1 －－ 没有介质/缺纸；（STATUS_MEDIA_NONE）

        2 －－ 入纸口检测到有介质（STATUS_MEDIA_PREENTERING）

        3 －－ 打印介质已对齐，介质准备插入 （STATUS_MEDIA_ENTERING）

        4 －－ 打印介质已插入打印机内部（STATUS_MEDIA_PRESENT）

        5  －－ 介质完全进入打印机（STATUS_MEDIA_INSERTED_ALL）

        6  －－ 至页尾行首（STATUS_MEDIA_NEAR_END）；

        7  －－ 卡纸（STATUS_MEDIA_JAMMED）

        12 －－ 未知错误（STATUS_ERROR_UNKNOW）；

    < 0: 其他（包括参数错等）

\******************************************************************/
long PRxGetMediaStatus( long hComm, unsigned char *devCode );


/***************************************************************************\
方法原型：long PRxInsertMedia(long hComm, long lInPosV, long lTimeout)

功能说明：进纸

输入参数：hComm：由Open所得到的通讯句柄

          lTimeOut: 超时时间，以秒为单位（>=0）

          lInPosV：指定介质进入打印机内部的位置（指定介质进入打印机时所处的打印位置:0.1mm）

输出参数： 无

返回：0 －－成功；其他（包括参数错等）－－失败

注释: lTimeOut <= 0 时，无限等待；

      lInPosV：=0 时，进纸，默认至介质0行

               非零，43 < lInPosV < 26694；
\***************************************************************************/
long PRxInsertMedia( long hComm, long lInPosV, long lTimeout);


/***************************************************************************\
方法原型：long PRxEjectMedia( long hComm, int iDirection)

功能说明：按指定方向退出打印机内部介质

输入参数：hComm：由Open所得到的通讯句柄

          lDirection: 介质退出方向（0：前端；1：后端）


输出参数： 无

返回：0 －－成功；其他（包括参数错等）－－失败
\***************************************************************************/
long PRxEjectMedia( long hComm, int iDirection);

/***************************************************************************\
方法原型：long PRxGetMediaLength(long hComm)

功能说明：获取介质长度

输入参数：hComm：由Open所得到的通讯句柄

输出参数： 无

返回：>= 0 －－ 介质宽度（单位0.1mm）；

      其他 －－失败

\***************************************************************************/
long PRxGetMediaLength(long hComm);


/***************************************************************************\
方法原型：long PRxGetMediaWidth(long hComm)

功能说明：获取介质宽度

输入参数：hComm：由Open所得到的通讯句柄

输出参数： 无

返回：>= 0 －－ 介质宽度（单位0.1mm）；

      其他 －－失败

\***************************************************************************/
long PRxGetMediaWidth(long hComm);


/***************************************************************************\
方法原型：long PRxGetMediaPosH(long hComm)

功能说明：获取介质距离打印机左边界的水平位置

输入参数：hComm：由Open所得到的通讯句柄

输出参数： 无

返回：>= 0 －－ 介质距离打印机左边界的水平位置（单位0.1mm）；

      其他 －－失败

\***************************************************************************/
long PRxGetMediaPosH(long hComm);


/********************************************************************************\
方法原型：long PRxBeep(long hComm, long lBeepCounts, long lIntervalTime)

功能说明：PRxBeep

输入参数：hComm: 设备控制句柄（来自方法Open）

          lIntervalTime: 鸣响间隔时长（单位：ms）（>=0）

          lBeepCounts: 鸣响次数；（>=0）

输出参数：无

返回：0 --- 成功

      其他失败

\********************************************************************************/
long PRxBeep(long hComm, long lBeepCounts, long lIntervalTime);

/***************************************************************************\
方法原型：long  PRxMsConfigHMS(long hComm,
                            long lMagneticType,
                            long lMagnPos,
                            unsigned char charOfEnd,
                            bool bDuplicData,
                            long lRetryCounts
                           )

功能说明：设置磁道读写的基本参数

输入参数：hComm: 由Open所得到的通讯句柄

          lMagneticType: 磁道类型：( 0：IBM 3604
                                     1：DIN/ISO
                                     2：ISO7811(菜单PNS 2040K = 否)
                                     3: IBM4746
                                     4: ANSI
                                     5: 兼容日立HT系列
                                     6: ISO8484 )；

         lMagnPos：磁道偏移位置（0：磁道位置－标准
                                 1：磁道位置－+10mm
                                 2：磁道位置－+20mm）

         charOfEnd: 结束标志：(0x0F: 仅IBM 3604、兼容IBM4746和兼容日立HT系列具有此值;

                                0x0C: 标准 )；

         bDuplicData：数据是双遍记录吗：（true:写；false:不写）

         lRetryCounts: 读写不成功时，可重复尝试次数（1：可重复1次；3：可重复3次; 其他无效）

输出参数： 无

返回：0 --- 成功

      其他 －－失败
\***************************************************************************/
long PRxMsConfigHMS(long hComm,long lMagneticType, long lMagnPos,unsigned char charOfEnd,bool bDuplicData,long lRetryCounts);

/****************************************************************************\
方法原型： long  PRxMsWriteEx(long hComm, char* szData) ;

功能说明：	按预先在Set-UP中选定的标准(或MsConfigHMS方法的设置)写磁道；

输入参数：	hComm: 由Open所得到的通讯句柄;

            szData: 写入数据；

输出参数：	无

返回：	0 ---- 成功 ； 其他----失败

注释：
\****************************************************************************/
long PRxMsWriteEx( long hComm, char* szData);


/****************************************************************************\
方法原型： long  PRxMsReadEx(long hComm, char* szData) ;

功能说明： 按预先在Set-UP中选定的标准(或MsConfigHMS方法的设置)读磁道；

输入参数：	hComm: 由Open所得到的通讯句柄;

            szData: 读出的数据；

输出参数：	无

返回：	> =0: 成功: 表示读出的数据长度 ；

        其他----失败
\****************************************************************************/
long PRxMsReadEx( long hComm, char* szData);


/****************************************************************************\
方法原型： long PRxMsWrite( long hComm, char* szData, long lMagneticType)

功能说明： 写磁道；

输入参数： hComm: 由Open所得到的通讯句柄;

           szData: 写入的数据；

           lMagneticType: 磁道类型：( 0：IBM 3604
                                      1：DIN/ISO
                                      2：ISO7811(菜单PNS 2040K = 否)
                                      3: IBM4746
                                      4: ANSI
                                      5: 兼容日立HT系列
                                      6: ISO8484 )；

输出参数：	无

返回：	0 ---- 成功 ； 其他----失败

注释：默认lMagnPos=0
          charOfEnd=0x0C
          bDuplicData=false
          lRetryCounts = 3
\****************************************************************************/
long PRxMsWrite( long hComm, char* szData, long lMagneticType);



/****************************************************************************\
方法原型： long PRxMsRead(long hComm, char* szData, long lMagneticType)

功能说明：读磁道；

输入参数：	hComm: 由Open所得到的通讯句柄;

            lMagneticType: 磁道类型：( 0：IBM 3604
                                       1：DIN/ISO
                                       2：ISO7811(菜单PNS 2040K = 否)
                                       3: IBM4746
                                       4: ANSI
                                       5: 兼容日立HT系列
                                       6: ISO8484 )；

输出参数：	szData: 读出的数据；

返回：	> =0: 成功: 表示读出的数据长度 ；

        其他----失败

注释：默认lMagnPos=0

          charOfEnd=0x0C

          bDuplicData=false

          lRetryCounts = 3
\****************************************************************************/
long  PRxMsRead(long hComm, char* szData, long lMagneticType);


/****************************************************************************\
方法名：	long PRxSetPageLength(long  hComm,  long PageLength);
功能说明：	设置页长
ESC Q nnn ESC Z----设定页面长度（70mm<=页长<=500mm, nnn<=255）
输入参数：	hComm：设备控制句柄（来自方法Open）
PageLenth: 设置的纸张长度
输出参数：	无
返回：	0 ---- 成功 ； 其他----失败

\****************************************************************************/
long PRxSetPageLength(long  hComm,  long PageLength);

/****************************************************************************\
方法名：	long PRxPrtWriteData(long  hComm, unsigned char*  command, long dataLength, long writeTimeout);
功能说明：	向端口发送16进制数据
输入参数：	hComm：设备控制句柄（来自方法Open）
Command: 发送数据字节序列起始地址
lDataLength: 发送数据长度
lWriteTimeout：超时时间
输出参数：	无
返回：	实际成功发送字节数；< 0 : 发送失败
\****************************************************************************/
long PRxPrtWriteData(long  hComm, unsigned char*  command, long dataLength, long writeTimeout);


/****************************************************************************\
方法名：	long PRxPrtSelWestCode (long  hComm，int iSel)
功能说明：	设定西文字符类型
输入参数：	hComm：设备控制句柄（来自方法Open）
iSel: 选择打印要用到的字符集
enum westChar {internationalOne, internationalTwo,German,Portugal,Spain,Denmark_Norway,France,Italy,
Sweden_Finland,Switzerland,England, USA,Greece,Isreal,SpainTwo,Yugoslavia,STD31,
BOM_Canada,SDC,Turkey,PC_Denmark_Norway,PC_Denmark_OPE,PC_210_Greece,
PC_220_SpainTwo,PC_437_international, PC_850_LatinOne,PC_860_Portugal,PC_862_Isreal,
OLIVITTI_UNIX,ISO_8859_1_LatinOne};
输出参数：	无
返回：	0: 成功； 其它: 失败
\****************************************************************************/
long PRxPrtSelWestCode(long  hComm, int iSel);

/****************************************************************************\
方法名：	long PRxPrtFlushPrt(long  hComm);
功能说明：	刷新打印机缓冲区
输入参数：	hComm：设备控制句柄（来自方法Open）

输出参数：	无
返回：	0: 成功； 非0: 失败
\****************************************************************************/
long PRxPrtFlushPrt(long  hComm);

/****************************************************************************\
方法名：	long PRxPrtMoveAbsPos(long  hComm ，long lX,  long lY)
功能说明：	移动打印头到指定的绝对位置, 然后用其它接口打印文本、条码等
水平位置移动---ESC | A nnn
垂直位置移动--ESC # 0 (物理页顶)+ ESC & nn (行距)+ESC L nnn (垂直移动N行)
输入参数：	hComm：设备控制句柄（来自方法Open）
lX: 横坐标，以 0.1毫米为单位（最小值：4.23 毫米–左界4.23毫米，最大值：243.0 毫米–左界+9.4英寸）
lY: 纵坐标，以 0.1毫米为单位（最小值: 4.23 毫米–顶界4.23 毫米， 最大值： PrtInit接口中设定的页长, 70mm<=页长<=500mm, nnn<=255）

输出参数：	无
返回：	0: 成功； 其他-待查
\****************************************************************************/
long PRxPrtMoveAbsPos(long  hComm ,long lX,  long lY);

/****************************************************************************\
方法名：	long PRxPrtMoveRelPos(long  hComm ， long lY)
功能说明：	移动打印头到指定的相对位置, 然后用其它接口打印文本、条码等
垂直位置移动-- ESC & nn (行距)+ESC I nnn (垂直走纸N行)
输入参数：	hComm：设备控制句柄（来自方法Open）

lY: 纵坐标，以 0.1毫米为单位（最小值: 0.1毫米–1/240英寸，最大值： PrtInit接口中设定的页长, 70mm<=页长<=500mm, nnn<=255）

输出参数：	无
返回：	0: 成功； 其他-待查
\****************************************************************************/
long PRxPrtMoveRelPos(long  hComm , long lY);

/****************************************************************************\
方法名：	long PRxPrtSetPtrProperty(long hComm, int iType, int iProperty)；
功能说明：	执行指定设置属性
输入参数：	hComm：设备控制句柄（来自方法Open）
strProperty:  属性值串
iTYPE---
1）	PrintQuality_TYPE打印质量—iProperty(草稿、高速草稿、NLQ1、NLQ2、LQ2)
2）	（取消PrtSetFontWiDTH-设为0）CPI_TYPE 打印字距—iProperty(10CPI、  12CPI 、13.3CPI、15CPI 、16.6CPI 、17.1CPI、 18CPI)
3）	Ext_TYPE 扩展属性—iProperty(倍高、取消倍高、倍宽、取消倍宽、3倍高、取消3倍高、3倍宽、取消3倍宽、黑体、上下划线、上下标)

输出参数：	无
返回：	0: 成功； 1: 不支持所输入的属性值

\****************************************************************************/
/*
enum basicType {printType, fontType, Ext_type};
enum  proptype {Draft, HighSpeedDraft, NLQ1,NLQ2,LQ2};
enum fontTypeDetail {CPI10, CPI12, CPI13point3,CPI15, CPI16point6, CPI17point1, CPI18};
enum extentionType {DH, CancelDH, DW,CancelDW, TriH, CancelTriH, TriW, CancelTriW, Black,CancelBlack,
up_underline, Cancel_up_underline,underline,CancelUnderline};
*/
long PRxPrtSetPtrProperty(long hComm, int iType, int iProperty);

/****************************************************************************\
方法名：	long PRxPrtPrintText (long hComm, char* strData)；
功能说明：	打印输出数据，与PrtMoveAbsPos组合运用
输入参数：	hComm：设备控制句柄（来自方法Open）
strData: 要打印输出的数据

输出参数：	无
返回：	0: 成功； 1: 数据发送失败；
\****************************************************************************/
long PRxPrtPrintText(long hComm, char* strData);

/****************************************************************************\
方法名：	long PRxPrtPrintOCR (long hComm, int iProperty,char* strData)；
功能说明：	打印OCR字符
输入参数：	hComm：设备控制句柄（来自方法Open）
strProperty:  属性值串
strData: 打印数据 （检查是否OCR字符）
OCR字体（强制10CPI，原来属性改变注意说明）--iProperty(OCRA,OCRB)
输出参数：	无
返回：	0: 成功； 1: 数据非法
\****************************************************************************/
//enum OCRTYPE {OCRA,OCRB};
long PRxPrtPrintOCR(long hComm, int iProperty,char* strData);

/****************************************************************************\
方法名：long PRxPrtPrintBarCode(long hComm, int iProperty,Bool bNeedTest, Bool bNeedRead，char* strData)
功能说明：	打印条码类型
输入参数：	hComm：设备控制句柄（来自方法Open）
strProperty:  属性值串
iProperty(UPC-A、UPC-E、EAN-8、EAN-13、CODE 2/5 INTERLEAVED、CODE 39、CODABAR CODE、CODE 128)
Bool bNeedTest: 需要校验
Bool bNeedRead：需要阅读
strData: 打印内容 （检查是否合法）
输出参数：	无
返回：	0: 成功； 1: 数据非法
\****************************************************************************/
/* enum barcodeType {UPC_A,UPC_E,EAN_8,EAN_13,CODE_2_5_INTERLEAVED,CODE_2_5_INDUSTRIAL,CODE_39,CODABAR_CODE,CODE_128}; */
long PRxPrtPrintBarCode(long hComm, int iProperty,bool bNeedTest, bool bNeedRead,char* strData);

/****************************************************************************\
方法名：	long PRxPrtSetFontWidth(long hComm,float fWidth)；
功能说明：	设定字符宽度
输入参数：	hComm：设备控制句柄（来自方法Open）
fWidth：所需字符宽度 （mm）
输出参数：	无
返回：	0: 成功； 1:失败；
\****************************************************************************/
long PRxPrtSetFontWidth(long hComm,float fWidth);

/****************************************************************************\
方法名：	 PRX_API long PRxPrtPrintBmp( long hComm, long lX, long lY, char *szBmpFile, char *szPrtDataFile, char *szPrintType);
功能说明：	打印180DPI图片
输入参数：	hComm：由Open所得到的通讯句柄
            lX: 打印位图的横坐标（位图最左边，单位0.1mm）
                  lY: 打印位图的纵坐标（位图最上边，单位0.1mm)
                  szBmpFile: 要打印的位图路径；（如："C:\\bmpFile.bmp"）
                  szPrtDataFile：位图转换的中间文件路径（如："C:\\bmpData.bin"）
输出参数：	无
返回：	    0：成功
            其他(包括参数错误等)－－失败
\****************************************************************************/
long PRxPrtPrintBmp( long hComm, long lX, long lY, char *szBmpFile, char *szPrtDataFile, char *szPrintType);

/****************************************************************************\
方法原型：long PRxScan(long hComm, ScanCtrl scanCtrl, ScanImageCtrl *frontCtrl, ScanImageCtrl *rearCtrl)

功能说明：按指定参数扫描图像；

输入参数：scanCtrl --- 扫描设备基本控制参数

          frontCtrl --- 正面图像的扫描控制参数

          rearCtrl ---- 反面图像的扫描控制参数

输出参数：无;

返回：	> =0: 成功 ；

        其他----失败；

注释：
\****************************************************************************/
long PRxScan(long hComm, ScanCtrl scanCtrl, ScanImageCtrl *frontCtrl, ScanImageCtrl *rearCtrl);

/****************************************************************************\
方法原型：long PRxScanSetImageCtrl(ScanImageCtrl *scanImageCtrl,
                                char *szSaveImagePath,
                                long lCisColorMode,
                                long lGrayMode=2,
                                long lBrightness=100,
                                long lThresholdLevel=100,
                                long lScanDirection=1,
                                long lOriginX=0,
                                long lOriginY=0,
                                long lEndX=0,
                                long lEndY=0)

功能说明：设置图像扫描的控制参数；

输入参数：szSaveImagePath --- 图像扫描结果的文件存放路径

          lCisColorMode --- CIS的光调色模式（0：Red  1：Green  2：red+green+blue 3：blue 4：真彩色RGB）

          lGrayMode --- 灰度（0--黑白；1--16色；2--256色/24位真彩色； 3--不支持/不做扫描）

          lBrightness --- 亮度（1-255）

          lThresholdLevel --- 黑白包容度（1-255）

          lScanDirection --- 扫描方向（0：由底至顶扫描； 1：由顶至底扫描）

          lOriginX ----  扫描区域起始点横坐标

          lOriginY ----  扫描区域起始点纵坐标

          lEndX ----  扫描区域终止点横坐标

          lEndY ----  扫描区域终止点纵坐标

输出参数：scanImageCtrl ---- 图像扫描控制参数;

返回：	> =0: 成功 ；

        其他----失败

注释：默认  lCisColorMode = 2 （red+green+blue）

            lGrayMode = 2 （256色/24位真彩色）

            lBrightness = 100

            lThresholdLevel = 100

            lScanDirection = 1

            lOriginX = 0

            lOriginY = 0

            lEndX = 0

            lEndY = 0

 lOriginX、lOriginY、lEndX、lEndY 用于定义扫描区域

 扫描区域 <= （扫描介质本身且或不超出物理扫描范围（物理扫描范围为：A4纸宽*A4纸2倍长））

 当 lOriginX = lOriginY = lEndX = lEndY = 0 时，设备在可扫描区域内对整张介质扫描；

\****************************************************************************/
long PRxScanSetImageCtrl(ScanImageCtrl *scanImageCtrl,
                         char *szSaveImagePath,
                         long lCisColorMode,
                         long lGrayMode,
                         long lBrightness,
                         long lThresholdLevel,
                         long lScanDirection,
                         long lOriginX,
                         long lOriginY,
                         long lEndX,
                         long lEndY);

/****************************************************************************\
方法原型：long PRxScanSetCtrl(ScanCtrl *scanCtrl, long lDpi=300, long lEjectMode = 1)

功能说明：设置扫描设备基本控制参数；

输入参数：lDpi --- 扫描控制DPI

          lEjectMode --- 扫描完成后，退出介质控制方式（0：不退、1：前退、2：后退）

输出参数：scanCtrl: 扫描设备基本控制参数;

返回：	> =0: 成功 ；

        其他----失败

注释：默认lDpi = 300

          lEjectMode = 1
\****************************************************************************/
long PRxScanSetCtrl(ScanCtrl *scanCtrl, long lDpi, long lEjectMode);

/***************************************************************************\
方法原型：long PRxSwallowPassBook( long hComm)

功能说明：吞折,把存折本从打印机后部退出，收到自助设备内部,该接口仅适用于在自助设备上的打印机固件版本

输入参数：hComm：由Open所得到的通讯句柄

          hComm: 设备控制句柄（来自方法Open）

输出参数： 无

返回：0 －－成功；1 - -  没有纸张，没有执行吞折操作   其他－－失败
\***************************************************************************/
long PRxSwallowPassBook(long hComm);

/****************************************************************************\
方法名：	long PRxGetFirmwareVer(long hComm,char *szData);
功能说明：	获取设备ID
输入参数：	hComm：设备控制句柄（来自方PRxPrtSelWestCode法Open）
输出参数：	szData: 获取的固件版本号
返回：	0: 成功；其他:失败;
\****************************************************************************/
long PRxGetFirmwareVersion(long hComm, char *szFirmwareVersion);

/****************************************************************************\
方法名：	long PRxPrtChangeCodeDistance(long  hComm,  float fDistance);
功能说明：	在字符右边加空列 ESC ! sp nnn
olivetti专有
输入参数：	hComm：设备控制句柄（来自方法Open）
fDistance: 希望增加的距离 （毫米）0~53.9mm
增加 nnn/120 英寸
输出参数：	无
返回：	0 －－成功；其他(包括参数错误等)－－失败
\****************************************************************************/
long PRxPrtChangeCodeDistance(long hComm, float fDistance);

#endif // TESTCPLUSDLL_H
