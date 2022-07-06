#pragma once


//#include "PrinterDef.h"
#include <string>
//#include "TraceManager.h"
#include "PossdkIntf.h"
#include "IDevPTR.h"
#include "ILogWrite.h"
//#include "ErrorCode.h"

using namespace std;

#define BT_SERIES_PRINTER_NAME "TKIOSK" // BT系列单元类打印机使用
#define BK_SERIES_PRINTER_NAME "KIOSK"  // BK系列单元类打印机使用

#define BKC310_LOG_NAME     "BKC310.log"
#define BTNH80_LOG_NAME     "BTNH80.log"

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


class CBKC310_DevImpl : public CLogManage
{
public:
    CBKC310_DevImpl();
    virtual ~CBKC310_DevImpl();

public:
    BOOL OpenDevice();
    BOOL OpenDevice(WORD wType);
    BOOL CloseDevice();
    //BOOL SetPrintSetting(PrinterDevConfig stPrinterDevConfig);
    BOOL Reset();
    BOOL GetStatus(char *pRtStatusBuf, ULONG *pulRtBufSz, char *pNonRtStatusBuf, ULONG *pulNonRtBufSz,
                   char *pAutoStatusBuf, ULONG *pulAutoStaBufSz);
    BOOL GetRtStatus(char *pStaBuf, ULONG *pulBufSz);
    BOOL GetNonRtStatus(char *pStaBuf, ULONG *pulBufSz);
    BOOL GetAutoStatus(char *pStaBuf, ULONG *pulBufSz);
    BOOL GetFwVersion(LPSTR pFwVerBuff, ULONG *ulFwVerSize);
    BOOL GetSoftwareVersion(LPSTR pVerBuff, ULONG *ulVerBuffSize);
    BOOL GetPrinterName(LPSTR pszName, WORD &wNameSize);
    BOOL ControlMedia(int nFeedLength);
    BOOL ControlMedia(BOOL bDetectBlackStripe, int nFeedLength);
    BOOL LineModePrintText(LPSTR lpPrintString, WORD wStringSize, WORD wStartX);
    BOOL LineModePrintImage(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX);
    BOOL LineModePrintImageRAM(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX);
    BOOL LineModePrintImageByMode(LPSTR lpImageStr, WORD lpImageStrSize, WORD wStartX);
    BOOL PrintFrontSetFormat();     // 打印前格式设置到打印机
    void SetPrintFormat(STPRINTFORMAT stPrintFormat);   // 打印格式记录保留

    //CTraceManager *GetTraceObj();

    //LPSPErrorInfo GetErrorCode();
private:
    BOOL InitPtr(char *pszPrinterName);
    BOOL Connect();

    //ErrorCode
    //void ClearErrorCode(LPSPErrorInfo lpErrorInfo);
    //void SetErrorCode(LPSPErrorInfo lpErrorInfo, char cErrorType[], char cErrorDetailCode[],
    //                  char *pDescription = NULL);
    ULONG SplitString(LPCSTR lpszStr, CHAR cSplit, CHAR lpszStrArray[][MAX_PATH], ULONG ulArrayCnt);

public:
    BOOL BK_SetPrintMode(WORD wPrintMode);
    BOOL BK_SetTextLineHight(WORD wLineHeight, BOOL IsSpot = FALSE);         // 设置打印行高
    BOOL BK_SetTextCharacterSpace(WORD nCharSpace, BOOL IsSpot = FALSE);     // 设置字符间距
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
    //BOOL        m_bBlackMark;
    //void        *m_hBTNHDll;
    DEVICEHANDLE    m_hPrinter;
    BOOL            m_bDevOpenOk;

    //CTraceManager m_traceMgr;
    CPossdkIntf     m_possdkIntf;
    //CErrorCode    m_errorCode;
    //SPErrorInfo   m_errorInfo;

    BYTE    byFontType[MAX_PATH];// 字体
    WORD    wFontSize;          // 字体大小
    WORD    wLeft;              // 左边距，缺省: 37，单位:0.1mm
    WORD    wTop;               // 上边距，缺省: 110，单位:0.1mm
    WORD    wWidth;             // 可打印宽，缺省: 720，单位:0.1mm
    WORD    wHeight;            // 可打印高，缺省: 0，单位:0.1mm

    WORD    wLineHeight;        // 行高，缺省: 0，单位:0.1mm
    WORD    wRowDistance;       // 行间距，缺省: 0，单位:0.1mm
    WORD    wCharSpace;         // 字符间距，缺省: 0，单位:0.1mm

    WORD    wDevType;
    CHAR    m_szAutoStatusOLD[12];  // 上一次自动获取状态保留       // 30-00-00-00(FT#0052)
    CHAR    m_szRealTimeStatusOLD[12];  // 上一次获取实时状态保留   // 30-00-00-00(FT#0052)
    INT     m_nAutoStatusRetOLD;    // 上一次获取自动状态结果保留    // 30-00-00-00(FT#0052)
    INT     m_nRealTimeStatusRetOLD;// 上一次获取实时状态结果保留    // 30-00-00-00(FT#0052)

};
