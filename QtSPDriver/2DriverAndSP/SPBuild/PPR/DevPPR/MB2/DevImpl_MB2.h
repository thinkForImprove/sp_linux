/***************************************************************
* 文件名称：DevImpl_MB2.h
* 文件描述：MB2存折打印模块底层指令，提供控制接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_MB2_H
#define DEVIMPL_MB2_H

#include <string>
#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "IDevPTR.h"
#include "../XFS_PPR/def.h"

/*************************************************************************
// 返回值/错误码　宏定义
// <0 : Impl处理返回
// 0~100: 硬件设备返回
*************************************************************************/
// >100: Impl处理返回
#define IMP_SUCCESS                     0               // 成功
#define IMP_ERR_LOAD_LIB                101             // 动态库加载失败
#define IMP_ERR_PARAM_INVALID           102             // 参数无效
#define IMP_ERR_UNKNOWN                 103             // 未知错误
#define IMP_ERR_NOTOPEN                 104             // 设备未Open
// >0: Device返回
// 用户调用不正确，报错
#define IMP_ERR_DEV_UNKNOW              -1              // 未知错误
#define IMP_ERR_DEV_PARAM_INVALID       -2              // 参数无效
#define IMP_ERR_DEV_NO_OPEN             -3              // 未调用过OPEN函数/调用不成功
#define IMP_ERR_DEV_NOT_OLI_EMULATION   -4              // 未调用过OPEN函数/调用不成功
// 设备创建和不支持，报错
#define IMP_ERR_DEV_CREATE_FAILURE      -11             // 创建设备失败
#define IMP_ERR_DEV_NO_SUPPORT          -12             // 设备不支持
// 系统通信，报错
#define IMP_ERR_COMM_OPEN               -21             // 通信打开出错
#define IMP_ERR_COMM_WRITE              -22             // 通信写出错
#define IMP_ERR_DEV_COMM_READ           -23             // 通信读出错
#define IMP_ERR_DEV_COMM_PARITY         -24             // 通信校验错
#define IMP_ERR_DEV_TIMEOUT             -25             // 通信超时
#define IMP_ERR_DEV_COMM_OFF            -26             // 设备离线
// 硬件报错
#define IMP_ERR_DEV_MEDIA_NOFOUND       -31             // 无纸
#define IMP_ERR_DEV_MS_BLANK            -32             // 空白磁条
#define IMP_ERR_DEV_MS_READORPARITY     -33             // 磁条读或校验出错

#define IMP_ERR_DEV_OFFLINE             -106            // 设备断电/断线
#define IMP_ERR_DEV_OFFLINE2            -107            // 设备断电/断线

/*************************************************************************
// 设备返回状态值　宏定义
*************************************************************************/
#define STATUS_NOMAL                    0               // 状态正常
#define STATUS_MEDIA_NONE               1               // 无介质
#define STATUS_MEDIA_PREENTERING        2               // 介质存在、位于入口
#define STATUS_MEDIA_ENTERING           3               // 介质已对齐
#define STATUS_MEDIA_PRESENT            4               // 介质已插入
#define STATUS_MEDIA_INSERTED_ALL       5               // 至页顶
#define STATUS_MEDIA_NEAR_END           6               // 至页尾
#define STATUS_MEDIA_JAMMED             7               // 介质阻塞
#define STATUS_MEDIA_MAGNETIC_UNREAD    8               // 磁条不能读出
#define STATUS_COVER_OPEN               9               // 处于开盖状态
#define STATUS_OFFLINE                  9               // 处于离线状态
#define STATUS_COMMAND_WRONG            10              // 命令错
#define STATUS_COMM_ERROR               11              // 通信错
#define STATUS_ERROR_UNKNOW             12              // 未知状态

/*************************************************************************
// 无分类　宏定义
*************************************************************************/
#define LOG_NAME            "DevImpl_MB2.log"         // 缺省日志名
#define DLL_DEVLIB_NAME     "libPRx.so"               // 缺省动态库名

// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

// 0.1毫米转换为点值
#define MM2SPOT(n)      ((int)((float)(n * 1.00)/1.25))

#ifndef MIN
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#endif

/*************************************************************************
// 打印机SDK使用结构体 宏定义
*************************************************************************/
#pragma pack(push,1)
typedef struct tagPOINT
{
    int  x;
    int  y;
}POINT,*PPOINT;
#pragma pack(pop)

// 扫描控制参数
#pragma pack(push,1)
typedef struct tagScanCtrl
{
    long dpi;
    long ejectMode;     // Eject Mode （0：不退；1：前退；2：后退）
    tagScanCtrl()
    {
        dpi = 300;
        ejectMode = 1;
    }
} ScanCtrl, SCANCTRL, *LPSCANCTRL;
#pragma pack(pop)

// 扫描控制参数
#pragma pack(push,1)
typedef struct tagScanImageCtrl
{
    long cisColorMode;      // 扫描光调色模式（0：Red  1：Green  2：red+green+blue 3：blue 4：真彩色RGB）
    long grayMode;          // 灰度（0--黑白；1--16色；2--256色/24位真彩色； 3--不支持/不做扫描）
    long brightness;        // 图片亮度(1--255)
    long thresholdLevel;    // 黑白扫描包容度
    char *saveImagePath;    // 保存扫描结果的文件路径
    POINT origin;
    POINT end;
    long scanDirection;     // 获取扫描图像的方向（0：镜像--正向扫描； 1：正像---反向扫描）
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
} ScanImageCtrl, SCANIMAGECTRL, *LPSCANIMAGECTRL;
#pragma pack(pop)

/*************************************************************************
// 打印机SDK接口(PRxPrtSetPtrProperty)使用 宏定义
*************************************************************************/
// 设置类型(iType参数)
enum basicType
{
    printType   = 0,    // 打印质量
    fontType,           // 打印字距
    Ext_type            // 扩展属性
};

// 打印质量(iProperty参数)
enum proptype
{
    Draft       = 0,    // 草稿
    HighSpeedDraft,     // 高速草稿
    NLQ1,               // NLQ1
    NLQ2,               // NLQ2
    LQ2                 // LQ2
};

// 打印字距(iProperty参数)
enum fontTypeDetail
{
    CPI10       = 0,    // 10CPI(字符+间距=2.5MM,高度=3.0MM)
    CPI12       = 1,    // 12CPI(字符+间距=2.2MM,高度=3.0MM)
    CPI13point3 = 2,    // 13.3CPI(字符+间距=2.0MM,高度=3.0MM)
    CPI15       = 3,    // 15CPI(字符+间距=1.7MM,高度=3.0MM)
    CPI16point6 = 4,    // 16.6CPI(字符+间距=1.6MM,高度=3.0MM)
    CPI17point1 = 5,    // 17.1CPI(字符+间距=1.5MM,高度=3.0MM)
    CPI18       = 6     // 18CPI(字符+间距=1.4MM,高度=3.0MM)
};

// 扩展属性(iProperty参数)
enum extentionType
{
    DH                      = 0,    // 倍高
    CancelDH                = 1,    // 取消倍高
    DW                      = 2,    // 倍宽
    CancelDW                = 3,    // 取消倍宽
    TriH                    = 4,    // 3倍高
    CancelTriH              = 5,    // 取消3倍高
    TriW                    = 6,    // 3倍宽
    CancelTriW              = 7,    // 取消3倍宽
    Black                   = 8,    // 黑体
    CancelBlack             = 9,    // 取消黑体
    up_underline            = 10,   // 上下划线
    Cancel_up_underline     = 11,   // 取消上下划线
    underline               = 12,   // 下划线
    CancelUnderline         = 13,   // 取消下划线
    up_sign                 = 14,   // 上标
    under_sign              = 15,   // 下标
    Cancel_upuder_sign      = 16    // 取消上下标
};

/*************************************************************************
// 打印机SDK接口使用 宏定义
*************************************************************************/
// Eject 控制
#define MEDIA_EJECT_FRONT       0       // 向前退出
#define MEDIA_EJECT_AFTER       1       // 向后退出

// 磁条类型
#define MAG_IBM3604             0       // IBM 3604,读磁道2
#define MAG_DIN_ISO             1       // DIN/ISO,读磁道3
#define MAG_ISO7811             2       // ISO7811,读磁道2
#define MAG_IBM4746             3       // IBM 4746,读磁道2
#define MAG_ANSI                4       // ANSI,读磁道3
#define MAG_HT                  5       // 兼容日立HT系列,读磁道未知
#define MAG_ISO8484             6       // ISO8484,读磁道3

/*************************************************************************
// 动态库输出函数接口　定义
*************************************************************************/
// 1. 打开PR通信端口连接
typedef long (*pPRxOpen)(const char *pszCommPort, long lCurEmulation, long lDestEmulation,
                         char *pszCommCfgFilePath, long *plErrorCode, long lBaudrate);
// 2. 断开PR通信连接
typedef long (*pPRxClose)(long hComm);
// 3. 打印机复位初始化
typedef long (*pPRxResetInit)(long hComm);
// 4. 打印机清错
typedef long (*pPRxCleanError)(long hComm);
// 5. 获取打印机状态
typedef long (*pPRxGetStatus)(long hComm, unsigned char *devCode);
// 6. 获取设备中介质所处状态
typedef long (*pPRxGetMediaStatus)(long hComm, unsigned char *devCode);
// 7. 插入介质控制
typedef long (*pPRxInsertMedia)(long hComm, long lInPosV, long lTimeout);
// 8. 按指定方向退出打印机内部介质
typedef long (*pPRxEjectMedia)(long hComm, long lDirection);
// 9. 获取介质长
typedef long (*pPRxGetMediaLength)(long hComm);
// 10. 吞折
typedef long (*pPRxSwallowPassBook)(long hComm);
// 11. 获取介质宽
typedef long (*pPRxGetMediaWidth)(long hComm);
// 12. 获取介质距离打印机左边界的水平位置
typedef long (*pPRxGetMediaPosH)(long hComm);
// 13. 鸣响
typedef long (*pPRxBeep)(long hComm, long lBeepCounts, long lIntervalTime);
// 14. 查询打印机固件版本信息
typedef long (*pPRxGetFirmwareVersion)(long hComm, char *szFirmwareVersion);
// 15. 设定页面长度
typedef long (*pPRxSetPageLength)(long hComm, long PageLength);
// 16. 向端口发送16进制数据
typedef long (*pPRxPrtWriteData)(long hComm, unsigned char *command, long dataLength, long writeTimeout);
// 17. 在字符右边加空列
typedef long (*pPRxPrtChangeCodeDistance)(long hComm, float fDistance);
// 18. 设定西文字符类型
typedef long (*pPRxPrtSelWestCode)(long hComm, int iSel);
// 19. 刷新打印机缓冲区
typedef long (*pPRxPrtFlushPrt)(long hComm);
// 20. 移动打印头到指定的绝对位置
typedef long (*pPRxPrtMoveAbsPos)(long hComm, long lX, long lY);
// 21. 移动打印头到指定的相对位置
typedef long (*pPRxPrtMoveRelPos)(long hComm, long lY);
// 22. 执行指定设置属性
typedef long (*pPRxPrtSetPtrProperty)(long hComm, int iType, int iProperty);
// 23. 打印OCR字符
typedef long (*pPRxPrtPrintOCR)(long hComm, int iProperty,char* strData);
// 24. 打印条码类型
typedef long (*pPRxPrtPrintBarCode)(long hComm, int iProperty, bool bNeedTest, bool bNeedRead, char* strData);
// 25. 打印输出数据,与PrtMoveAbsPos组合运用
typedef long (*pPRxPrtPrintText)(long hComm, char* strData);
// 26. 设定字符宽度
typedef long (*pPRxPrtSetFontWidth)(long hComm,float fWidth);
// 27. 打印位图
typedef long (*pPRxPrtPrintBmp)(long hComm, long lX, long lY, char *szBmpFile, char *szPrtDataFile,
                                char *szPrintType);
// 28. 打印颜色设置
typedef long (*pPRxSetPrintColor)(long hComm, int iColor);
// 29. 设置磁道读写的基本参数
typedef long (*pPRxMsConfigHMS)(long hComm, long lMagneticType, long lMagnPos, unsigned char charOfEnd,
                                bool bDuplicData, long lRetryCounts);
// 30. 按预先在Set-UP中选定的标准写磁道
typedef long (*pPRxMsWriteEx)(long hComm, char* szData);
// 31. 按预先在Set-UP中选定的标准读磁道
typedef long (*pPRxMsReadEx)(long hComm, char* szData);
// 32. 写磁道
typedef long (*pPRxMsWrite)(long hComm, char* szData, long lMagneticType);
// 33. 读磁道
typedef long (*pPRxMsRead)(long hComm, char* szData, long lMagneticType);
// 34. 设扫描基本控制参数
typedef long (*pPRxScanSetCtrl)(ScanCtrl &scanCtrl, long lDpi, long lEjectMode);
// 35. 设置图像扫描控制参数
typedef long (*pPRxScanSetImageCtrl)(ScanImageCtrl &scanImageCtrl, char *szSaveImagePath,
                                     long lCisColorMode, long lGrayMode, long lBrightness,
                                     long lThresholdLevel, long lScanDirection, long lOriginX,
                                     long lOriginY, long lEndX, long lEndY) ;
// 36. 按指定扫描参数扫描图像
typedef long (*pPRxScan)(long hComm, ScanCtrl scanCtrl, ScanImageCtrl *frontCtrl, ScanImageCtrl *rearCtrl);


/*************************************************************************
// 命令编辑、发送接收等处理
*************************************************************************/
class CDevImpl_MB2 : public CLogManage
{
public:
    CDevImpl_MB2();
    CDevImpl_MB2(LPSTR lpLog);
    CDevImpl_MB2(LPSTR lpLog, LPSTR lpDevStr);
    virtual ~CDevImpl_MB2();

public: // 动态库接口封装
    INT DeviceOpen(LPSTR lpMode);                               // 1. 打开设备
    INT DeviceClose();                                          // 2. 关闭设备
    INT Reset();                                                // 3. 复位设备
    INT CleanError();                                           // 4. 打印机清错
    INT GetDevStatus(LPSTR lpDevStat);                          // 5. 获取设备状态
    INT GetMediaStatus(LPSTR lpDevStat);                        // 6. 获取介质状态
    INT MediaInsert(DWORD dwInsLen = 0, DWORD dwTimeOut = 3000);// 7. 介质吸入
    INT MediaEject(WORD dwEjectMode = 0);                       // 8. 介质退出    
    INT SetBeep(DWORD dwCount = 1, DWORD dwTime = 1000);        // 13. 鸣响
    INT GetFWVersion(LPSTR pVerBuff, ULONG *ulVerBuffSize);     // 14. 获取固件版本
    INT PrtWriteData(LPSTR lpCMD, DWORD dwSize, DWORD dwTimeOut);// 16. 向端口发送16进制数据;
    INT FlushBuffer();                                          // 19. 刷新打印机缓冲区
    INT MoveAbsPos(DWORD dwX, DWORD dwY);                       // 20. 移动打印头到指定的绝对位置
    INT MoveRelPos(DWORD dwY);                                  // 21. 移动打印头到指定的相对位置
    INT SetPtrProperty(INT nType, INT nProperty);               // 22. 设置打印属性
    INT PrintText(LPSTR lpData, DWORD dwDataLen);               // 25. 打印文本
    INT PrintBmp(DWORD dwX, DWORD dwY, LPSTR lpBmpFile,
                 LPSTR lpPrtType = "0");                        // 27.1. 打印位图
    INT PrintBmp(DWORD dwX, DWORD dwY, LPSTR lpBmpFile,
                 LPSTR lpDataFile, LPSTR lpPrtType);            // 27.2. 打印位图
    INT MsRead(DWORD dwMagType, LPSTR lpRetData, DWORD &dwSize);// 33. 读磁道
    INT Scan(SCANCTRL stScanCtrl, LPSCANIMAGECTRL lpStFrontCtrl,
             LPSCANIMAGECTRL lpStBackCtrl);                     // 36. 按指定扫描参数扫描图像

private:
    INT     ConvertErrorCode(INT nRet);                         // 转换为Impl返回码/错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);

public: // 对外参数设置接口
    INT SetReConFlag(BOOL bFlag);
    void SetLibPath(LPCSTR lpPath);                             // 设置动态库路径(DeviceOpen前有效)
    void SetOpenMode(LPSTR lpMode, INT nBandRate);              // 设置打开模式(DeviceOpen前有效)

private:
    CSimpleMutex    m_MutexAction;                              // 执行互斥
    LONG            m_hPrinter;                                 // 设备控制句柄
    BOOL            m_bDevOpenOk;                               // 设备Open标记
    CHAR            m_szErrStr[1024];
    CHAR            m_szDevStataOLD;                            // 上一次获取设备状态保留
    INT             m_nRetErrOLD[12];                           // 处理错误值保存(0:库加载/1:设备连接/2:设备状态/3介质状态/4介质吸入)
    BOOL            m_bReCon;                                   // 是否断线重连状态
    CHAR            m_szOpenMode[64];                           // 打开模式(缺省USB)
    INT             m_nBaudRate;                                // 波特率(缺省9600)

private:
    void Init();

private: // 接口加载
    BOOL    bLoadLibrary();
    void    vUnLoadLibrary();
    BOOL    bLoadLibIntf();

private: // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    INT         m_nDevErrCode;

private: // 动态库接口定义
    pPRxOpen                    PRxOpen;                    // 1. 打开PR通信端口连接
    pPRxClose                   PRxClose;                   // 2. 断开PR通信连接
    pPRxResetInit               PRxResetInit;               // 3. 打印机复位初始化
    pPRxCleanError              PRxCleanError;              // 4. 打印机清错
    pPRxGetStatus               PRxGetStatus;               // 5. 获取打印机状态
    pPRxGetMediaStatus          PRxGetMediaStatus;          // 6. 获取设备中介质所处状态
    pPRxInsertMedia             PRxInsertMedia;             // 7. 插入介质控制
    pPRxEjectMedia              PRxEjectMedia;              // 8. 按指定方向退出打印机内部介质
    pPRxGetMediaLength          PRxGetMediaLength;          // 9. 获取介质长
    pPRxSwallowPassBook         PRxSwallowPassBook;         // 10. 吞折
    pPRxGetMediaWidth           PRxGetMediaWidth;           // 11. 获取介质宽
    pPRxGetMediaPosH            PRxGetMediaPosH;            // 12. 获取介质距离打印机左边界的水平位置
    pPRxBeep                    PRxBeep;                    // 13. 鸣响
    pPRxGetFirmwareVersion      PRxGetFirmwareVersion;      // 14. 查询打印机固件版本信息on;
    pPRxSetPageLength           PRxSetPageLength;           // 15. 设定页面长度
    pPRxPrtWriteData            PRxPrtWriteData;            // 16. 向端口发送16进制数据
    pPRxPrtChangeCodeDistance   PRxPrtChangeCodeDistance;   // 17. 在字符右边加空列
    pPRxPrtSelWestCode          PRxPrtSelWestCode;          // 18. 设定西文字符类型
    pPRxPrtFlushPrt             PRxPrtFlushPrt;             // 19. 刷新打印机缓冲区
    pPRxPrtMoveAbsPos           PRxPrtMoveAbsPos;           // 20. 移动打印头到指定的绝对位置
    pPRxPrtMoveRelPos           PRxPrtMoveRelPos;           // 21. 移动打印头到指定的相对位置
    pPRxPrtSetPtrProperty       PRxPrtSetPtrProperty;       // 22. 执行指定设置属性
    pPRxPrtPrintOCR             PRxPrtPrintOCR;             // 23. 打印OCR字符
    pPRxPrtPrintBarCode         PRxPrtPrintBarCode;         // 24. 打印条码类型
    pPRxPrtPrintText            PRxPrtPrintText;            // 25. 打印输出数据,与PrtMoveAbsPos组合运用
    pPRxPrtSetFontWidth         PRxPrtSetFontWidth;         // 26. 设定字符宽度
    pPRxPrtPrintBmp             PRxPrtPrintBmp;             // 27. 打印位图
    pPRxSetPrintColor           PRxSetPrintColor;           // 28. 打印颜色设置
    pPRxMsConfigHMS             PRxMsConfigHMS;             // 29. 设置磁道读写的基本参数
    pPRxMsWriteEx               PRxMsWriteEx;               // 30. 按预先在Set-UP中选定的标准写磁道
    pPRxMsReadEx                PRxMsReadEx;                // 31. 按预先在Set-UP中选定的标准读磁道
    pPRxMsWrite                 PRxMsWrite;                 // 32. 写磁道
    pPRxMsRead                  PRxMsRead;                  // 33. 读磁道
    pPRxScanSetCtrl             PRxScanSetCtrl;             // 34. 设扫描基本控制参数
    pPRxScanSetImageCtrl        PRxScanSetImageCtrl;        // 35. 设置图像扫描控制参数
    pPRxScan                    PRxScan;                    // 36. 按指定扫描参数扫描图像
};

#endif // DEVIMPL_MB2_H
