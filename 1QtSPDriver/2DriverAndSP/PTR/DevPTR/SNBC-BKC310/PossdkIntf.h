#ifndef CPOSSDKINTF_H
#define CPOSSDKINTF_H

#define DLL_POSSDK_NAME "POSSDK.so"
//#include "wintypes.h"
#include "POSSDK.h"
#include "QtTypeDef.h"
#include <QLibrary>

#define FUNC_POINTER_ERROR_RETURN(LPFUNC) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE;    \
        return FALSE;   \
    }

class CPossdkIntf
{
public:
    CPossdkIntf();
    ~CPossdkIntf();
public:
    BOOL LoadPossdkDll();
    void UnloadPossdkDll();
private:
    BOOL LoadPossdkIntf();
private:
    QLibrary m_possdkLibrary;
    BOOL m_bLoadIntfFail;

    char m_szPossdkDllPath[MAX_PATH];
public:
    /*-----------------------------FUNCTION POINTER------------------------------*/
    //connect port
    pEnumDeviceInfo EnumDeviceInfo;
    pOpenUsbClassPort OpenUsbClassPort;
    pInit Init;
    pClosePort ClosePort;
    pSendPortData SendPortData;
    pReadPortData ReadPortData;
    pSetPortTimeout SetPortTimeout;
    pGetPortTimeout GetPortTimeout;
    pOpenUsbApiPort OpenUsbApiPort;
    pOpenNetPort OpenNetPort;
    pOpenCOMPort OpenCOMPort;
    pOpenLPTPort OpenLPTPort;
    pOpenNetPortByName OpenNetPortByName;
    pOpenBlueToothPortByName OpenBlueToothPortByName;
    pOpenBlueToothPort OpenBlueToothPort;
    pOpenDriverPort OpenDriverPort;

    //query
    pRealTimeQueryStatus RealTimeQueryStatus;
    pNonRealTimeQueryStatus NonRealTimeQueryStatus;
    pAutoQueryStatus AutoQueryStatus;
    pFirmwareVersion FirmwareVersion;
    pSoftwareVersion SoftwareVersion;
    pVendorInformation VendorInformation;
    pPrinterName PrinterName;
    pResolutionRatio ResolutionRatio;
    pHardwareSerialNumber HardwareSerialNumber;

    //text
    pFeedLine FeedLine;
    pPrintTextOut PrintTextOut;
    pUniversalTextOut UniversalTextOut;
    pSetTextLineHight SetTextLineHight;
    pSetTextBold SetTextBold;
    pSetTextDoubleWidthAndHeight SetTextDoubleWidthAndHeight;
    pSetAlignmentMode SetAlignmentMode;
    pSetTextCharacterSpace SetTextCharacterSpace;
    pSetTextMagnifyTimes SetTextMagnifyTimes;
    pSetTextFontType SetTextFontType;
    pSetTextUpsideDownMode SetTextUpsideDownMode;
    pSetTextOppositeColor SetTextOppositeColor;
    pSetTextColorEnable SetTextColorEnable;
    pSetTextFontColor SetTextFontColor;
    pSetTextUnderline SetTextUnderline;
    pSetTextRotate SetTextRotate;
    pSetTextCharsetAndCodepage SetTextCharsetAndCodepage;
    pSetTextUserDefinedCharacterEnable SetTextUserDefinedCharacterEnable;
    pSetTextDefineUserDefinedCharacter SetTextDefineUserDefinedCharacter;

    //bitmap
    pPrintBitmap PrintBitmap;
    pPrintBitmapByMode PrintBitmapByMode;
    pDownloadRAMBitmapByFile DownloadRAMBitmapByFile;
    pPrintRAMBitmap PrintRAMBitmap;
    pDownloadFlashBitmapByFile DownloadFlashBitmapByFile;
    pPrintFlashBitmap PrintFlashBitmap;
    pPrintTrueType PrintTrueType;

    //barcode
    pPrintBarcode PrintBarcode;
    pPrintBarcodeSimple PrintBarcodeSimple;
    pBarcodePrintQR BarcodePrintQR;
    pBarcodePrintPDF417 BarcodePrintPDF417;
    pBarcodePrintPDF417Simple mpBarcodePrintPDF417Simple;
    pBarcodePrintMaxicode BarcodePrintMaxicode;
    pBarcodePrintGS1DataBar BarcodePrintGS1DataBar;

    //basic set
    pDownloadFile DownloadFile;
    pPrintSetMode PrintSetMode;
    pPageModeSetArea PageModeSetArea;
    pPageModePrint PageModePrint;
    pPageModeClearBuffer PageModeClearBuffer;
    pFeedLineNumber FeedLineNumber;
    pCutPaper CutPaper;
    pReset Reset;
    pKickOutDrawer KickOutDrawer;
    pApplicationUnit ApplicationUnit;
    pPrintDensity PrintDensity;
    pMotionUnit MotionUnit;
    pSelectPaperType SelectPaperType;
    pSelectPaperTypeEEP SelectPaperTypeEEP;
};

#endif // CPOSSDKINTF_H
