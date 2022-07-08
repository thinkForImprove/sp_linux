/***************************************************************
* 文件名称：DevImpl_BKC310.h
* 文件描述：新北洋BK-C310/BT-NH80M打印模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月27日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_BKC310_H
#define DEVIMPL_BKC310_H

#include <string>
#include <dlfcn.h>
#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "IAllDevPort.h"
#include "POSSDK.h"
#include "../XFS_PTR/def.h"

using namespace std;

/*************************************************************
// 返回值/错误码　宏定义
// <0 : Impl处理返回
// 0~100: 硬件设备返回
*************************************************************/
// >0: Impl处理返回
#define IMP_SUCCESS                 0                       // 成功
#define IMP_ERR_LOAD_LIB            1                       // 动态库加载失败
#define IMP_ERR_PARAM_INVALID       2                       // 参数无效
#define IMP_ERR_UNKNOWN             3                       // 未知错误
// >0: Device返回
#define IMP_DEV_ERR_SUCCESS         ERR_SUCCESS             // 0
#define IMP_DEV_ERR_FIIL            ERR_FAIL                // -1:失败
#define IMP_DEV_ERR_HANDLE          ERR_HANDLE              // -2:无效句柄
#define IMP_DEV_ERR_PARAMATER       ERR_PARAMATER           // -3:无效参数
#define IMP_DEV_ERR_FILE            ERR_FILE                // -4:参数文件错误
#define IMP_DEV_ERR_READ            ERR_READ                // -5:读取文件或数据错误
#define IMP_DEV_ERR_WRITE           ERR_WRITE               // -6:下发文件或数据错误
#define IMP_DEV_ERR_NOT_SUPPORT     ERR_NOT_SUPPORT         // -7:此功能不支持
#define IMP_DEV_ERR_BITMAP_INVAILD  ERR_BITMAP_INVAILD      // -8:位图错误
#define IMP_DEV_ERR_LOADDLL_FAILURE ERR_LOADDLL_FAILURE     // -9:动态库加载失败
#define IMP_DEV_ERR_FIRNOSUPPORT    ERR_FIRNOSUPPORT        // -10:固件不支持
#define IMP_DEV_ERR_UNKOWN_ERROR    ERR_UNKOWN_ERROR        // -127:未知错误

/*************************************************************
// 无分类　宏定义
*************************************************************/

#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

#define LOAD_LIBINFO_FUNC2(LPFUNC, FUNC) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(#FUNC); \
    if(!FUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

// 加载动态库接口
#define LOAD_LIBINFO_FUNC_DL(HANDLE, LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)dlsym(HANDLE, FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        Log(ThisModule, __LINE__, "dlxxx加载动态库接口<%s> Fail. ", FUNC2); \
        return FALSE;   \
    }

#define BT_SERIES_PRINTER_NAME "TKIOSK" // BT系列单元类打印机使用
#define BK_SERIES_PRINTER_NAME "KIOSK"  // BK系列单元类打印机使用

#define DLL_DEVLIB_NAME  "POSSDK.so"    // 缺省动态库名
#define LOG_NAME         "DevImpl_BT8500M.log"   // 缺省日志名

#define LOG_NAME_SNBC       "DevRPR_SNBC.log"
#define LOG_NAME_BKC310     "DevRPR_BKC310.log"
#define LOG_NAME_BTNH80     "DevRPR_BTNH80.log"

#define MM2SPOT(n)      ((int)((float)(n * 1.00)/1.25))

#ifndef MIN
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#endif


/*
命令编辑、发送接收等处理。
*/


class CDevImpl_BKC310 : public CLogManage
{
public:
    CDevImpl_BKC310();
    CDevImpl_BKC310(LPSTR lpLog);
    virtual ~CDevImpl_BKC310();

public:
    INT DeviceOpen();
    INT DeviceOpen(WORD wType);
    INT DeviceClose();
    //BOOL SetPrintSetting(PrinterDevConfig stPrinterDevConfig);
    INT Reset();
    INT GetStatus(char *pRtStatusBuf, ULONG *pulRtBufSz, char *pNonRtStatusBuf, ULONG *pulNonRtBufSz,
                   char *pAutoStatusBuf, ULONG *pulAutoStaBufSz);
    INT GetRtStatus(char *pStaBuf, ULONG *pulBufSz);
    INT GetNonRtStatus(char *pStaBuf, ULONG *pulBufSz);
    INT GetAutoStatus(char *pStaBuf, ULONG *pulBufSz);
    INT GetFwVersion(LPSTR pFwVerBuff, ULONG *ulFwVerSize);
    INT GetSoftwareVersion(LPSTR pVerBuff, ULONG *ulVerBuffSize);
    INT GetHardSerialNo(LPSTR pBuff, ULONG *ulVerBuffSize);
    INT GetPrinterName(LPSTR pBuff, ULONG *ulVerBuffSize);
    INT GetVendorInfo(LPSTR pBuff, ULONG *ulVerBuffSize);
    INT SetCutPaper(BOOL bDetectBlackStripe, int nFeedLength, INT nCutMode);
    INT LineModePrintText(LPSTR lpPrintString, WORD wStringSize, WORD wStartX, BOOL bFeedLine = TRUE);
    INT LineModePrintTextUniversal(LPSTR lpPrintString, WORD wStringSize, WORD wStartX, WORD wDouW, WORD wDouH,
                                   WORD nFontType, WORD nFontStyle, BOOL bFeedLine = TRUE);
    INT LineModePrintTrueType(LPSTR lpPrtData, INT nPrtDataSize, INT nStartX,
                              LPSTR lpFontName, INT nFontHeight, INT nFontWidth,
                              BOOL bIsBold = FALSE, BOOL bFeedLine = TRUE);         // 行模式TureType模式指定属性打印字符串
    INT LineModePrintImage(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX, BOOL bFeedLine = TRUE);
    INT LineModePrintImageRAM(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX, BOOL bFeedLine = TRUE);
    INT LineModePrintImageByMode(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX, BOOL bFeedLine = TRUE);
    INT LineModePrintBarcode(LPSTR lpData, WORD wDataSize, WORD wStartX, WORD wType,
                             WORD wBasicWidth, WORD wHeight, WORD wHriFontType,
                             WORD wHriFontPos, BOOL bFeedLine = TRUE);
    INT LineModePrintPDF417(LPSTR lpData, WORD wDataSize, WORD wStartX, WORD wBasicWidth,
                            WORD wHeight, WORD wLines, WORD wCols, WORD wScaleH,
                            WORD wScaleV, WORD wErrCor, BOOL bFeedLine = TRUE);
    INT LineModePrintQrcode(LPSTR lpData, WORD wDataSize, WORD wStartX, WORD wBasicWidth,
                            WORD wSymbolType, WORD wLangMode, WORD wErrCor,
                            BOOL bFeedLine = TRUE);
    INT PrtFeedLine(INT nLine = 1);     // 下发换行命令
    INT PrintFrontSetFormat();     // 打印前格式设置到打印机
    void SetPrintFormat(STPRINTFORMAT stPrintFormat);   // 打印格式记录保留

private:
    INT InitPtr(char *pszPrinterName);
    INT Connect();

    ULONG SplitString(LPCSTR lpszStr, CHAR cSplit, CHAR lpszStrArray[][MAX_PATH], ULONG ulArrayCnt);
    INT   ConvertErrorCode(INT nRet);                     // 转换为Impl返回码/错误码
    CHAR* ConvertErrCodeToStr(INT nRet);
    void  SetLogName(LPSTR lpLog);

public:
    INT BK_SetPrintMode(WORD wPrintMode);
    INT BK_SetTextLineHight(WORD wLineHeight, BOOL IsSpot = FALSE);         // 设置打印行高
    INT BK_SetTextCharacterSpace(WORD nCharSpace, BOOL IsSpot = FALSE);     // 设置字符间距
    INT BK_SetBlackMove(UINT uMoveSize);                                    // 设置物理黑标位移
    INT BK_SetTextBold(WORD wBold = 0);                                     // 设置字体加粗
    INT BK_SetTextDoubleWH(WORD wWidth = 0, WORD wHeight = 0);              // 设置倍宽倍高
    INT  SetReConFlag(BOOL bFlag = TRUE);
    void SetLibPath(LPCSTR lpPath);                                         // 设置动态库路径(DeviceOpen前有效)
    void SetOpenMode(INT nMode, INT nDevID);                                // 设置打开模式(DeviceOpen前有效) // 30-00-00-00(FT#0067)
    /*
    BOOL BK_PrintInPapgeMode();
    BOOL BK_SetCharRightInternal(BYTE bRightInternal);
    BOOL BK_SetPresenterWaitTime(BYTE bWaitTime);
    BOOL BK_SetPresenterMode(int nPresentMode);
    BOOL BK_SetPaperType(BOOL bIsBlackMark);
    BOOL BK_SetBlodMode(BOOL bIsBlod);
    BOOL BK_SetPageModeArea(WORD wLineCount, int iMinLines, int iRowInterval);
    BOOL BK_SetRowInterval(BYTE usRowInterval);
    BOOL BK_SetInternalChar(BYTE usInterChar);
    BOOL BK_SetCodePage(int iCodePage);
    BOOL BK_SetAllowPrint(void);
    BOOL BK_SendDataToBuf(LPSTR lpPrintString, WORD wStringSize);*/

private:
    CSimpleMutex                    m_MutexAction;
    DEVICEHANDLE                    m_hPrinter;
    BOOL                            m_bDevOpenOk;
    CHAR                            m_szErrStr[1024];

    //CPossdkIntf     m_possdkIntf;

    BYTE    byFontType[MAX_PATH];                   // 字体
    WORD    wFontSize;                              // 字体大小
    WORD    wLeft;                                  // 左边距，缺省: 37，单位:0.1mm
    WORD    wTop;                                   // 上边距，缺省: 110，单位:0.1mm
    WORD    wWidth;                                 // 可打印宽，缺省: 720，单位:0.1mm
    WORD    wHeight;                                // 可打印高，缺省: 0，单位:0.1mm

    WORD    wLineHeight;                            // 行高，缺省: 0，单位:0.1mm
    WORD    wRowDistance;                           // 行间距，缺省: 0，单位:0.1mm
    WORD    wCharSpace;                             // 字符间距，缺省: 0，单位:0.1mm

    WORD    wDevType;
    CHAR    m_szAutoStatusOLD[12];                  // 上一次获取自动状态保留
    CHAR    m_szRealTimeStatusOLD[12];              // 上一次获取实时状态保留
    INT     m_nAutoStatusRetOLD;                    // 上一次获取自动状态结果保留
    INT     m_nRealTimeStatusRetOLD;                // 上一次获取实时状态结果保留
    INT     m_nRetErrOLD[8];                        // 处理错误值保存
    BOOL    m_bReCon;                               // 是否断线重连状态
    INT     m_nOpenMode;                            // 打开模式(缺省0)    // 30-00-00-00(FT#0067)
    INT     m_nDeviceID;                            // 内部ID(缺省-1)    // 30-00-00-00(FT#0067)

private:
    void Init();

private: // 接口加载(QLibrary方式)
    BOOL    bLoadLibrary();
    void    vUnLoadLibrary();
    BOOL    bLoadLibIntf();
    void    vInitLibFunc();

private: // 接口加载(QLibrary方式)
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    INT         m_nDevErrCode;



private: // 接口加载(dlxxx方式)
    BOOL    bDlLoadLibrary();
    void    vDlUnLoadLibrary();
    BOOL    bDlLoadLibIntf();

private: // 接口加载(dlxxx方式)
    void*   m_vDlHandle;


private: // 动态库接口定义
    //connect port
    pEnumDeviceInfo                     EnumDeviceInfo;
    pOpenUsbClassPort                   OpenUsbClassPort;
    pInit                               Init_;
    pClosePort                          ClosePort;
    pSendPortData                       SendPortData;
    pReadPortData                       ReadPortData;
    pSetPortTimeout                     SetPortTimeout;
    pGetPortTimeout                     GetPortTimeout;
    pOpenUsbApiPort                     OpenUsbApiPort;
    pOpenNetPort                        OpenNetPort;
    pOpenCOMPort                        OpenCOMPort;
    pOpenLPTPort                        OpenLPTPort;
    pOpenNetPortByName                  OpenNetPortByName;
    pOpenBlueToothPortByName            OpenBlueToothPortByName;
    pOpenBlueToothPort                  OpenBlueToothPort;
    pOpenDriverPort                     OpenDriverPort;

    //query
    pRealTimeQueryStatus                RealTimeQueryStatus;
    pNonRealTimeQueryStatus             NonRealTimeQueryStatus;
    pAutoQueryStatus                    AutoQueryStatus;
    pFirmwareVersion                    FirmwareVersion;
    pSoftwareVersion                    SoftwareVersion;
    pVendorInformation                  VendorInformation;
    pPrinterName                        PrinterName;
    pResolutionRatio                    ResolutionRatio;
    pHardwareSerialNumber               HardwareSerialNumber;

    //text
    pFeedLine                           FeedLine;
    pPrintTextOut                       PrintTextOut;
    pUniversalTextOut                   UniversalTextOut;
    pSetTextLineHight                   SetTextLineHight;
    pSetTextBold                        SetTextBold;
    pSetTextDoubleWidthAndHeight        SetTextDoubleWidthAndHeight;
    pSetAlignmentMode                   SetAlignmentMode;
    pSetTextCharacterSpace              SetTextCharacterSpace;
    pSetTextMagnifyTimes                SetTextMagnifyTimes;
    pSetTextFontType                    SetTextFontType;
    pSetTextUpsideDownMode              SetTextUpsideDownMode;
    pSetTextOppositeColor               SetTextOppositeColor;
    pSetTextColorEnable                 SetTextColorEnable;
    pSetTextFontColor                   SetTextFontColor;
    pSetTextUnderline                   SetTextUnderline;
    pSetTextRotate                      SetTextRotate;
    pSetTextCharsetAndCodepage          SetTextCharsetAndCodepage;
    pSetTextUserDefinedCharacterEnable  SetTextUserDefinedCharacterEnable;
    pSetTextDefineUserDefinedCharacter  SetTextDefineUserDefinedCharacter;

    //bitmap
    pPrintBitmap                        PrintBitmap;
    pPrintBitmapByMode                  PrintBitmapByMode;
    pDownloadRAMBitmapByFile            DownloadRAMBitmapByFile;
    pPrintRAMBitmap                     PrintRAMBitmap;
    pDownloadFlashBitmapByFile          DownloadFlashBitmapByFile;
    pPrintFlashBitmap                   PrintFlashBitmap;
    pPrintTrueType                      PrintTrueType;

    //barcode
    pPrintBarcode                       PrintBarcode;
    pPrintBarcodeSimple                 PrintBarcodeSimple;
    pBarcodePrintQR                     BarcodePrintQR;
    pBarcodePrintPDF417                 BarcodePrintPDF417;
    pBarcodePrintPDF417Simple           BarcodePrintPDF417Simple;
    pBarcodePrintMaxicode               BarcodePrintMaxicode;
    pBarcodePrintGS1DataBar             BarcodePrintGS1DataBar;

    //basic set
    pDownloadFile                       DownloadFile;
    pPrintSetMode                       PrintSetMode;
    pPageModeSetArea                    PageModeSetArea;
    pPageModePrint                      PageModePrint;
    pPageModeClearBuffer                PageModeClearBuffer;
    pFeedLineNumber                     FeedLineNumber;
    pCutPaper                           CutPaper;
    pReset                              Reset_;
    pKickOutDrawer                      KickOutDrawer;
    pApplicationUnit                    ApplicationUnit;
    pPrintDensity                       PrintDensity;
    pMotionUnit                         MotionUnit;
    pSelectPaperType                    SelectPaperType;
    pSelectPaperTypeEEP                 SelectPaperTypeEEP;

};

#endif // DEVIMPL_BKC310_H
