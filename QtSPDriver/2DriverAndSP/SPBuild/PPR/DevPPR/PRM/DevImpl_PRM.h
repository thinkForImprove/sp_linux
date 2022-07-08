/***************************************************************
* 文件名称：DevImpl_PRM.h
* 文件描述：PRM存折打印模块底层指令，提供控制接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_PRM_H
#define DEVIMPL_PRM_H

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
// >0: Device异常返回
#define IMP_ERR_USB_OPEN                -1              // USB打开异常
#define IMP_ERR_CONNECT                 -2              // 连接异常
#define IMP_ERR_SNDCMD                  -3              // 发送失败(查询指令发送异常)
#define IMP_ERR_RCVCMD                  -4              // 读取失败
#define IMP_ERR_ALREAY_OPEN             -5              // 端口已打开
#define IMP_ERR_NOT_OPEN                -6              // 端口未打开(打开失败)
#define IMP_ERR_FILE_NOFOUND            -10             // 文件不存在
#define IMP_ERR_FILE_OPEN               -11             // 未找到文件或打开失败
#define IMP_ERR_FILE_READ               -12             // 文件读取失败
#define IMP_ERR_FILE_LENGTH             -13             // 文件长度获取失败
#define IMP_ERR_GETMEDIA_STAT           -20             // 获取介质状态失败
#define ERR_ERR_GETMEDIA_SIZE           -21             // 介质尺寸获取失败
#define IMP_ERR_GETDEV_STAT             -22             // 获取机器状态失败
#define IMP_ERR_DEV_TIME                -30             // 等待超时
#define IMP_ERR_MEM_APPLY               -31             // 内存申请失败
#define IMP_ERR_PAR_INVALID             -32             // 输入参数无效
#define IMP_ERR_DATA_CODESET            -60             // 文本字符编码格式错误
#define IMP_ERR_IMAGE_NOT_SUPP          -70             // 不支持的图片格式

/*************************************************************************
// 设备返回设备状态值　宏定义
*************************************************************************/
#define STATUS_NOMAL                    1               // 状态正常
#define STATUS_MEDIA_NEAR_END           2               // 打印介质处于页尾行首位置
#define STATUS_COVEROPEN_OFFLINE        3               // 脱机或开盖
#define STATUS_MEDIA_PRESENT            4               // 打印介质被全部插入/打印介质已插入机内
#define STATUS_COMM_ERROR               5               // 传输错误（打印机在接收时发现错误）
#define STATUS_OFFLINE                  6               // 脱机
#define STATUS_COVER_OPEN               7               // 开盖
#define STATUS_COMMAND_WRONG            8               // 命令错误
#define STATUS_MEDIA_JAMMED             9               // 介质阻塞
#define STATUS_PAPER_NOHAVE             10              // 缺纸/无纸
#define STATUS_COVER_OPEN2              11              // 开盖
#define STATUS_COMMAND_WRONG3           12              // 命令错误
#define STATUS_MAGNETIC_UNREAD          13              // 读错或校验错
#define STATUS_MS_BLANK                 14              // 空白磁条
#define STATUS_MSR_SUCCESS              15              // 读存折成功,打印机状态正常

/*************************************************************************
// 设备返回介质状态值　宏定义
*************************************************************************/
#define STATUS_MEDIA_NONE               1               // 打印介质不存在
#define STATUS_MEDIA_PREENTERING        2               // 打印介质存在
#define STATUS_MEDIA_ENTERING           3               // 打印介质已对齐
#define STATUS_MEDIA_INSERTED_ALL       5               // 至页顶
#define STATUS_MEDIA_MAGNETIC_UNREAD    8               // 磁条不能读出
#define STATUS_ERROR_UNKNOW             12              // 未知状态

/*************************************************************************
// 无分类　宏定义
*************************************************************************/
#define LOG_NAME            "DevImpl_PRM.log"           // 缺省日志名
#define DLL_DEVLIB_NAME     "libpr2print.so"            // 缺省动态库名

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
// 打印机SDK接口使用 宏定义
*************************************************************************/
// Init控制
#define INIT_NOACTION                   1               // 无动作
#define INIT_EJECT                      2               // 退折
#define INIT_RETRACT                    0               // 吞折

// 下划线类型
#define UNDER_LINE                      0               // 下划线
#define ABOVE_LINE                      1               // 上划线
#define UPUNDER_LINE                    2               // 上下划线
#define NOT_UPUNDER_LINE                3               // 取消上下划线

// 坐标单位
#define UNIT_MM                         0               // 毫米
#define UNIT_INCH                       1               // 英寸

// CPI
#define CPI_10                          10
#define CPI_12                          12
#define CPI_15                          15
#define CPI_166                         16.6
#define CPI_171                         17.1

// 倍宽
#define WIDTH_DOUBLE                    0               // 2倍宽
#define WIDTH_TRIPLE                    1               // 3倍宽
#define WIDTH_CANCEL                    2               // 取消倍宽

// 倍高
#define HEIGHT_DOUBLE                   1               // 2倍高
#define HEIGHT_TRIPLE                   3               // 3倍高
#define HEIGHT_CANCEL                   0               // 取消倍高

// 磁条
#define MSG_T2                  0       // 磁条2
#define MSG_T3                  1       // 磁条3



/*************************************************************************
// 动态库输出函数接口　定义
*************************************************************************/
// 1. 打开打印机端口
typedef int (*pCITIC_PR2_OpenPort)(char *port);
// 2. 关闭打印机端口
typedef int (*pCITIC_PR2_ClosePort)();
// 3. 初始化打印机
typedef int (*pCITIC_PR2_InitPrinter)(int flag, int timeout);
// 4. 打印文本内容
typedef int (*pCITIC_PR2_PrintData)(int unit, int xPosition, int yPosition, char* data, int len);
// 5. 设置打印格式
typedef int (*pCITIC_PR2_SetFormat)(double cpi, int bold, int width, int height);
// 6. 设置打印行间距
typedef int (*pCITIC_PR2_SetLineSpace)(int unit, double lpi);
// 7. 设置下划线类型
typedef int (*pCITIC_PR2_SetUnderLine)(int type);
// 8. 下发换行命令
typedef int (*pCITIC_PR2_LR)();
// 9. 进纸
typedef int (*pCITIC_PR2_InsertPaper)();
// 10. 退纸
typedef int (*pCITIC_PR2_EjectPaper)();
// 11. 获取打印机状态
typedef int (*pCITIC_PR2_GetPrinterStatus)();
// 12. 获取介质状态
typedef int (*pCITIC_PR2_GetPaperStatus)();
// 13. 获取固件版本信息
typedef int (*pCITIC_PR2_GetFWID)(char* fwid);
// 14. 清除打印机错误状态
typedef int (*pCITIC_PR2_ClearError)();
// 15. 打印机总清
typedef int (*pCITIC_PR2_ClearAll)();
// 16. 发送数据
typedef int (*pCITIC_PR2_Command)(char* cmd,int len);
// 17. 读取磁条信息
typedef int (*pCITIC_PR2_MSR)(int flag, char* data, int len);
// 18. 写入磁条信息
typedef int (*pCITIC_PR2_MSW)(int flag, char* data, int len);
// 19. 读取所有磁条信息
typedef int (*pCITIC_PR2_MSRALL)(char* t2data, int t2len, char* t3data, int t3len);
// 20. 读磁或打印完成后正常退折(若长时间不取则吞折)
typedef int (*pCITIC_PR2_SPB)(int timeout);
// 21. OLI下打印图片bmp图片
typedef int (*pCITIC_PR2_PrintBmpOLI)(char *file, bool ejectPaper, int x, int y);
// 22. OLI下打印图片bmp图片(可缩放)
typedef int (*pCITIC_PR2_PrintBmpOLI_Scale)(char *file, bool ejectPaper, int x, int y, float scale);


/*************************************************************************
// 命令编辑、发送接收等处理
*************************************************************************/
class CDevImpl_PRM : public CLogManage
{
public:
    CDevImpl_PRM();
    CDevImpl_PRM(LPSTR lpLog);
    CDevImpl_PRM(LPSTR lpLog, LPSTR lpDevStr);
    virtual ~CDevImpl_PRM();

public: // 动态库接口封装
    INT DeviceOpen(LPSTR lpMode);                               // 1. 打开设备
    INT DeviceClose();                                          // 2. 关闭设备
    INT Reset(INT nAction, INT nTimeOut = 5/*秒*/);             // 3. 复位设备
    INT PrintText(DWORD dwX, DWORD dwY, LPSTR lpData,
                  DWORD dwDataLen, WORD wUnit = 0/*MM*/);       // 4. 打印文本
    INT SetPtrProperty(INT nCPI = CPI_12, INT nBold = 0,
                       INT nWidth = WIDTH_CANCEL,
                       INT nHeight = HEIGHT_CANCEL);            // 5. 设置打印属性
    INT SetLineSpace(INT nLPI, WORD wUnit = 0/*MM*/);           // 6. 设置打印行间距
    INT SetUnderLine(INT nType);                                // 7. 设置下划线类型
    INT PrtFeedLine();                                          // 8. 下发换行命令
    INT MediaInsert();                                          // 9. 进纸(介质吸入)
    INT MediaEject();                                           // 10. 退纸(介质退出)
    INT GetDevStatus();                                         // 11. 获取设备状态
    INT GetMediaStatus();                                       // 12. 获取介质状态
    INT GetFWVersion(LPSTR pBuff, DWORD dwSize);                // 13. 获取固件版本
    INT CleanError();                                           // 14. 清除打印机错误状态
    INT CleanAll();                                             // 15. 打印机总清
    INT PrtWriteData(LPSTR lpCMD, DWORD dwSize);                // 16. 向端口发送数据
    INT MsRead(DWORD dwMagType, LPSTR lpRetData, DWORD dwSize); // 17. 读取磁条信息
    INT MsWrite(DWORD dwMagType, LPSTR lpRetData, DWORD dwSize);// 18. 写入磁条信息
    INT MsReadAll(LPSTR lpT2Data, DWORD dwT2Size,
                  LPSTR lpT3Data, DWORD dwT3Size);              // 19. 读取所有磁条信息
    INT EjectAndRetract(DWORD dwTimeOut);                       // 20. 读磁或打印完成后正常退折(若长时间不取则吞折)
    INT PrintBmp(DWORD dwX, DWORD dwY, LPSTR lpBmpFile,
                 BOOL bEject);                                  // 21.1. 打印位图
    INT PrintBmp(DWORD dwX, DWORD dwY, LPSTR lpBmpFile,
                 BOOL bEject, FLOAT fScale);                    // 21.2. 打印位图(可缩放)
    INT StartOnceBeep();                                        // 启动一次鸣响


private:
    INT     ConvertErrorCode(INT nRet);                         // 转换为Impl返回码/错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);

public: // 对外参数设置接口
    INT SetReConFlag(BOOL bFlag);
    void SetLibPath(LPCSTR lpPath);                             // 设置动态库路径(DeviceOpen前有效)
    void SetOpenMode(LPSTR lpMode, INT nBandRate);              // 设置打开模式(DeviceOpen前有效)

private:
    CSimpleMutex    m_MutexAction;                              // 执行互斥
    BOOL            m_bDevOpenOk;                               // 设备Open标记
    CHAR            m_szErrStr[1024];
    CHAR            m_szDevStataOLD;                            // 上一次获取设备状态保留
    INT             m_nRetErrOLD[12];                           // 处理错误值保存(0:库加载/1:设备连接/2:设备状态/3介质状态/4介质吸入/5Open后Reset)
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
    pCITIC_PR2_OpenPort             PR2_OpenPort;           // 1. 打开打印机端口
    pCITIC_PR2_ClosePort            PR2_ClosePort;          // 2. 关闭打印机端口
    pCITIC_PR2_InitPrinter          PR2_InitPrinter;        // 3. 初始化打印机
    pCITIC_PR2_PrintData            PR2_PrintData;          // 4. 打印文本内容
    pCITIC_PR2_SetFormat            PR2_SetFormat;          // 5. 设置打印格式
    pCITIC_PR2_SetLineSpace         PR2_SetLineSpace;       // 6. 设置打印行间距
    pCITIC_PR2_SetUnderLine         PR2_SetUnderLine;       // 7. 设置下划线类型
    pCITIC_PR2_LR                   PR2_LR;                 // 8. 下发换行命令
    pCITIC_PR2_InsertPaper          PR2_InsertPaper;        // 9. 进纸
    pCITIC_PR2_EjectPaper           PR2_EjectPaper;         // 10. 退纸
    pCITIC_PR2_GetPrinterStatus     PR2_GetPrinterStatus;   // 11. 获取打印机状态
    pCITIC_PR2_GetPaperStatus       PR2_GetPaperStatus;     // 12. 获取介质状态
    pCITIC_PR2_GetFWID              PR2_GetFWID;            // 13. 获取固件版本信息
    pCITIC_PR2_ClearError           PR2_ClearError;         // 14. 清除打印机错误状态
    pCITIC_PR2_ClearAll             PR2_ClearAll;           // 15. 打印机总清
    pCITIC_PR2_Command              PR2_Command;            // 16. 发送数据
    pCITIC_PR2_MSR                  PR2_MSR;                // 17. 读取磁条信息
    pCITIC_PR2_MSW                  PR2_MSW;                // 18. 写入磁条信息
    pCITIC_PR2_MSRALL               PR2_MSRALL;             // 19. 读取所有磁条信息
    pCITIC_PR2_SPB                  PR2_SPB;                // 20. 读磁或打印完成后正常退折(若长时间不取卡则吞折)
    pCITIC_PR2_PrintBmpOLI          PR2_PrintBmpOLI;        // 21. OLI下打印图片bmp图片
    pCITIC_PR2_PrintBmpOLI_Scale    PR2_PrintBmpOLI_Scale;  // 22. OLI下打印图片bmp图片(可缩放)
};

#endif // DEVIMPL_PRM_H
