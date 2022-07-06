#include "PossdkIntf.h"
#include "QtTypeDef.h"
#include "QtDLLLoader.h"
//#include "FileDir.h"
//#include "CommDef.h"

CPossdkIntf::CPossdkIntf()
{
    memset(m_szPossdkDllPath, 0, sizeof(m_szPossdkDllPath));

    //
    QString strDllName(QString::fromLocal8Bit(DLL_POSSDK_NAME));
#ifdef Q_OS_WIN
    strDllName.prepend(WINPATH);
#else
    //strDllName = strDllName.left(strDllName.lastIndexOf("."));
    strDllName.prepend(LINUXPATHLIB);
#endif

    m_possdkLibrary.setFileName(strDllName);
    m_bLoadIntfFail = TRUE;

    //connect port
    EnumDeviceInfo = NULL;
    OpenUsbClassPort = NULL;
    Init = NULL;
    ClosePort = NULL;
    SendPortData = NULL;
    ReadPortData = NULL;
    SetPortTimeout = NULL;
    GetPortTimeout = NULL;
    OpenUsbApiPort = NULL;
    OpenNetPort = NULL;
    OpenCOMPort = NULL;
    OpenLPTPort = NULL;
    OpenNetPortByName = NULL;
    OpenBlueToothPortByName = NULL;
    OpenBlueToothPort = NULL;
    OpenDriverPort = NULL;

    //query
    RealTimeQueryStatus = NULL;
    NonRealTimeQueryStatus = NULL;
    AutoQueryStatus = NULL;
    FirmwareVersion = NULL;
    SoftwareVersion = NULL;
    VendorInformation = NULL;
    PrinterName = NULL;
    ResolutionRatio = NULL;
    HardwareSerialNumber = NULL;

    //text
    FeedLine = NULL;
    PrintTextOut = NULL;
    UniversalTextOut = NULL;
    SetTextLineHight = NULL;
    SetTextBold = NULL;
    SetTextDoubleWidthAndHeight = NULL;
    SetAlignmentMode = NULL;
    SetTextCharacterSpace = NULL;
    SetTextMagnifyTimes = NULL;
    SetTextFontType = NULL;
    SetTextUpsideDownMode = NULL;
    SetTextOppositeColor = NULL;
    SetTextColorEnable = NULL;
    SetTextFontColor = NULL;
    SetTextUnderline = NULL;
    SetTextRotate = NULL;
    SetTextCharsetAndCodepage = NULL;
    SetTextUserDefinedCharacterEnable = NULL;
    SetTextDefineUserDefinedCharacter = NULL;

    //bitmap
    PrintBitmap = NULL;
    PrintBitmapByMode = NULL;
    DownloadRAMBitmapByFile = NULL;
    PrintRAMBitmap = NULL;
    DownloadFlashBitmapByFile = NULL;
    PrintFlashBitmap = NULL;
    PrintTrueType = NULL;

    //barcode
    PrintBarcode = NULL;
    PrintBarcodeSimple = NULL;
    BarcodePrintQR = NULL;
    BarcodePrintPDF417 = NULL;
    mpBarcodePrintPDF417Simple = NULL;
    BarcodePrintMaxicode = NULL;
    BarcodePrintGS1DataBar = NULL;

    //basic set
    DownloadFile = NULL;
    PrintSetMode = NULL;
    PageModeSetArea = NULL;
    PageModePrint = NULL;
    PageModeClearBuffer = NULL;
    FeedLineNumber = NULL;
    CutPaper = NULL;
    Reset = NULL;
    KickOutDrawer = NULL;
    ApplicationUnit = NULL;
    PrintDensity = NULL;
    MotionUnit = NULL;
    SelectPaperType = NULL;
    SelectPaperTypeEEP = NULL;
}

CPossdkIntf::~CPossdkIntf()
{
    UnloadPossdkDll();
}

BOOL CPossdkIntf::LoadPossdkDll()
{
    if(m_possdkLibrary.isLoaded() != true){
        if(m_possdkLibrary.load() != true){
            return FALSE;
        }
    }

    if(m_bLoadIntfFail){
        if(LoadPossdkIntf() != TRUE){
            return FALSE;
        }
    }
    return TRUE;
}

void CPossdkIntf::UnloadPossdkDll()
{
    if(m_possdkLibrary.isLoaded()){
        m_possdkLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CPossdkIntf::LoadPossdkIntf()
{
    m_bLoadIntfFail = FALSE;
    EnumDeviceInfo = (pEnumDeviceInfo)m_possdkLibrary.resolve("EnumDeviceInfo");
    FUNC_POINTER_ERROR_RETURN(EnumDeviceInfo);

    Init = (pInit)m_possdkLibrary.resolve("Init");
    FUNC_POINTER_ERROR_RETURN(Init);

    OpenUsbClassPort = (pOpenUsbClassPort)m_possdkLibrary.resolve("OpenUsbClassPort");
    FUNC_POINTER_ERROR_RETURN(OpenUsbClassPort);

    ClosePort = (pClosePort)m_possdkLibrary.resolve("ClosePort");
    FUNC_POINTER_ERROR_RETURN(ClosePort);

    SendPortData = (pSendPortData)m_possdkLibrary.resolve("SendPortData");
    FUNC_POINTER_ERROR_RETURN(SendPortData);

    ReadPortData = (pReadPortData)m_possdkLibrary.resolve("ReadPortData");
    FUNC_POINTER_ERROR_RETURN(ReadPortData);

    SetPortTimeout = (pSetPortTimeout)m_possdkLibrary.resolve("SetPortTimeout");
    FUNC_POINTER_ERROR_RETURN(SetPortTimeout);

    GetPortTimeout = (pGetPortTimeout)m_possdkLibrary.resolve("GetPortTimeout");
    FUNC_POINTER_ERROR_RETURN(GetPortTimeout);

    OpenUsbApiPort = (pOpenUsbApiPort)m_possdkLibrary.resolve("OpenUsbApiPort");
    FUNC_POINTER_ERROR_RETURN(OpenUsbApiPort);

    OpenNetPort = (pOpenNetPort)m_possdkLibrary.resolve("OpenNetPort");
    FUNC_POINTER_ERROR_RETURN(OpenNetPort);

    OpenCOMPort = (pOpenCOMPort)m_possdkLibrary.resolve("OpenCOMPort");
    FUNC_POINTER_ERROR_RETURN(OpenCOMPort);

    OpenLPTPort = (pOpenLPTPort)m_possdkLibrary.resolve("OpenLPTPort");
    FUNC_POINTER_ERROR_RETURN(OpenLPTPort);

    OpenNetPortByName = (pOpenNetPortByName)m_possdkLibrary.resolve("OpenNetPortByName");
    FUNC_POINTER_ERROR_RETURN(OpenNetPortByName);

    OpenBlueToothPortByName = (pOpenBlueToothPortByName)m_possdkLibrary.resolve("OpenBlueToothPortByName");
    FUNC_POINTER_ERROR_RETURN(OpenBlueToothPortByName);

    OpenBlueToothPort = (pOpenBlueToothPort)m_possdkLibrary.resolve("OpenBlueToothPort");
    FUNC_POINTER_ERROR_RETURN(OpenBlueToothPort);

    OpenDriverPort = (pOpenDriverPort)m_possdkLibrary.resolve("OpenDriverPort");
    FUNC_POINTER_ERROR_RETURN(OpenDriverPort);

    //query
    RealTimeQueryStatus = (pRealTimeQueryStatus)m_possdkLibrary.resolve("RealTimeQueryStatus");
    FUNC_POINTER_ERROR_RETURN(RealTimeQueryStatus);

    NonRealTimeQueryStatus = (pNonRealTimeQueryStatus)m_possdkLibrary.resolve("NonRealTimeQueryStatus");
    FUNC_POINTER_ERROR_RETURN(NonRealTimeQueryStatus);

    AutoQueryStatus = (pAutoQueryStatus)m_possdkLibrary.resolve("AutoQueryStatus");
    FUNC_POINTER_ERROR_RETURN(AutoQueryStatus);

    FirmwareVersion = (pFirmwareVersion)m_possdkLibrary.resolve("FirmwareVersion");
    FUNC_POINTER_ERROR_RETURN(FirmwareVersion);

    SoftwareVersion = (pSoftwareVersion)m_possdkLibrary.resolve("SoftwareVersion");
    FUNC_POINTER_ERROR_RETURN(SoftwareVersion);

    VendorInformation = (pVendorInformation)m_possdkLibrary.resolve("VendorInformation");
    FUNC_POINTER_ERROR_RETURN(VendorInformation);

    PrinterName = (pPrinterName)m_possdkLibrary.resolve("PrinterName");
    FUNC_POINTER_ERROR_RETURN(PrinterName);

    ResolutionRatio = (pResolutionRatio)m_possdkLibrary.resolve("ResolutionRatio");
    FUNC_POINTER_ERROR_RETURN(ResolutionRatio);

    HardwareSerialNumber = (pHardwareSerialNumber)m_possdkLibrary.resolve("HardwareSerialNumber");
    FUNC_POINTER_ERROR_RETURN(HardwareSerialNumber);

    //text
    FeedLine = (pFeedLine)m_possdkLibrary.resolve("FeedLine");
    FUNC_POINTER_ERROR_RETURN(FeedLine);

    PrintTextOut = (pPrintTextOut)m_possdkLibrary.resolve("PrintTextOut");
    FUNC_POINTER_ERROR_RETURN(PrintTextOut);

    UniversalTextOut = (pUniversalTextOut)m_possdkLibrary.resolve("UniversalTextOut");
    FUNC_POINTER_ERROR_RETURN(UniversalTextOut);

    SetTextLineHight = (pSetTextLineHight)m_possdkLibrary.resolve("SetTextLineHight");
    FUNC_POINTER_ERROR_RETURN(SetTextLineHight);

    SetTextBold = (pSetTextBold)m_possdkLibrary.resolve("SetTextBold");
    FUNC_POINTER_ERROR_RETURN(SetTextBold);

    SetTextDoubleWidthAndHeight = (pSetTextDoubleWidthAndHeight)m_possdkLibrary.resolve("SetTextDoubleWidthAndHeight");
    FUNC_POINTER_ERROR_RETURN(SetTextDoubleWidthAndHeight);

    SetAlignmentMode = (pSetAlignmentMode)m_possdkLibrary.resolve("SetAlignmentMode");
    FUNC_POINTER_ERROR_RETURN(SetAlignmentMode);

    SetTextCharacterSpace = (pSetTextCharacterSpace)m_possdkLibrary.resolve("SetTextCharacterSpace");
    FUNC_POINTER_ERROR_RETURN(SetTextCharacterSpace);

    SetTextMagnifyTimes = (pSetTextMagnifyTimes)m_possdkLibrary.resolve("SetTextMagnifyTimes");
    FUNC_POINTER_ERROR_RETURN(SetTextMagnifyTimes);

    SetTextFontType = (pSetTextFontType)m_possdkLibrary.resolve("SetTextFontType");
    FUNC_POINTER_ERROR_RETURN(SetTextFontType);

    SetTextUpsideDownMode = (pSetTextUpsideDownMode)m_possdkLibrary.resolve("SetTextUpsideDownMode");
    FUNC_POINTER_ERROR_RETURN(SetTextUpsideDownMode);

    SetTextOppositeColor = (pSetTextOppositeColor)m_possdkLibrary.resolve("SetTextOppositeColor");
    FUNC_POINTER_ERROR_RETURN(SetTextOppositeColor);

    SetTextColorEnable = (pSetTextColorEnable)m_possdkLibrary.resolve("SetTextColorEnable");
    FUNC_POINTER_ERROR_RETURN(SetTextColorEnable);

    SetTextFontColor = (pSetTextFontColor)m_possdkLibrary.resolve("SetTextFontColor");
    FUNC_POINTER_ERROR_RETURN(SetTextFontColor);

    SetTextUnderline = (pSetTextUnderline)m_possdkLibrary.resolve("SetTextUnderline");
    FUNC_POINTER_ERROR_RETURN(SetTextUnderline);

    SetTextRotate = (pSetTextRotate)m_possdkLibrary.resolve("SetTextRotate");
    FUNC_POINTER_ERROR_RETURN(SetTextRotate);

    SetTextCharsetAndCodepage = (pSetTextCharsetAndCodepage)m_possdkLibrary.resolve("SetTextCharsetAndCodepage");
    FUNC_POINTER_ERROR_RETURN(SetTextCharsetAndCodepage);

    SetTextUserDefinedCharacterEnable = (pSetTextUserDefinedCharacterEnable)m_possdkLibrary.resolve("SetTextUserDefinedCharacterEnable");
    FUNC_POINTER_ERROR_RETURN(SetTextUserDefinedCharacterEnable);

    SetTextDefineUserDefinedCharacter = (pSetTextDefineUserDefinedCharacter)m_possdkLibrary.resolve("SetTextDefineUserDefinedCharacter");
    FUNC_POINTER_ERROR_RETURN(SetTextDefineUserDefinedCharacter);

    //bitmap
    PrintBitmap = (pPrintBitmap)m_possdkLibrary.resolve("PrintBitmap");
    FUNC_POINTER_ERROR_RETURN(PrintBitmap);

    PrintBitmapByMode = (pPrintBitmapByMode)m_possdkLibrary.resolve("PrintBitmapByMode");
    FUNC_POINTER_ERROR_RETURN(PrintBitmapByMode);

    PrintRAMBitmap = (pPrintRAMBitmap)m_possdkLibrary.resolve("PrintRAMBitmap");
    FUNC_POINTER_ERROR_RETURN(PrintRAMBitmap);

    DownloadFlashBitmapByFile = (pDownloadFlashBitmapByFile)m_possdkLibrary.resolve("DownloadFlashBitmapByFile");
    FUNC_POINTER_ERROR_RETURN(DownloadFlashBitmapByFile);

    DownloadRAMBitmapByFile = (pDownloadRAMBitmapByFile)m_possdkLibrary.resolve("DownloadRAMBitmapByFile");
    FUNC_POINTER_ERROR_RETURN(DownloadRAMBitmapByFile);

    PrintFlashBitmap = (pPrintFlashBitmap)m_possdkLibrary.resolve("PrintFlashBitmap");
    FUNC_POINTER_ERROR_RETURN(PrintFlashBitmap);

    PrintTrueType = (pPrintTrueType)m_possdkLibrary.resolve("PrintTrueType");
    FUNC_POINTER_ERROR_RETURN(PrintTrueType);

    //barcode
    PrintBarcode = (pPrintBarcode)m_possdkLibrary.resolve("PrintBarcode");
    FUNC_POINTER_ERROR_RETURN(PrintBarcode);

    PrintBarcodeSimple = (pPrintBarcodeSimple)m_possdkLibrary.resolve("PrintBarcodeSimple");
    FUNC_POINTER_ERROR_RETURN(PrintBarcodeSimple);

    BarcodePrintQR = (pBarcodePrintQR)m_possdkLibrary.resolve("BarcodePrintQR");
    FUNC_POINTER_ERROR_RETURN(BarcodePrintQR);

    BarcodePrintPDF417 = (pBarcodePrintPDF417)m_possdkLibrary.resolve("BarcodePrintPDF417");
    FUNC_POINTER_ERROR_RETURN(BarcodePrintPDF417);

    mpBarcodePrintPDF417Simple = (pBarcodePrintPDF417Simple)m_possdkLibrary.resolve("BarcodePrintPDF417Simple");
    FUNC_POINTER_ERROR_RETURN(mpBarcodePrintPDF417Simple);

    BarcodePrintMaxicode = (pBarcodePrintMaxicode)m_possdkLibrary.resolve("BarcodePrintMaxicode");
    FUNC_POINTER_ERROR_RETURN(BarcodePrintMaxicode);

    BarcodePrintGS1DataBar = (pBarcodePrintGS1DataBar)m_possdkLibrary.resolve("BarcodePrintGS1DataBar");
    FUNC_POINTER_ERROR_RETURN(BarcodePrintGS1DataBar);

    //basic set
    DownloadFile = (pDownloadFile)m_possdkLibrary.resolve("DownloadFile");
    FUNC_POINTER_ERROR_RETURN(DownloadFile);

    PrintSetMode = (pPrintSetMode)m_possdkLibrary.resolve("PrintSetMode");
    FUNC_POINTER_ERROR_RETURN(PrintSetMode);

    PageModeSetArea = (pPageModeSetArea)m_possdkLibrary.resolve("PageModeSetArea");
    FUNC_POINTER_ERROR_RETURN(PageModeSetArea);

    PageModePrint = (pPageModePrint)m_possdkLibrary.resolve("PageModePrint");
    FUNC_POINTER_ERROR_RETURN(PageModePrint);

    PageModeClearBuffer = (pPageModeClearBuffer)m_possdkLibrary.resolve("PageModeClearBuffer");
    FUNC_POINTER_ERROR_RETURN(PageModeClearBuffer);

    FeedLineNumber = (pFeedLineNumber)m_possdkLibrary.resolve("FeedLineNumber");
    FUNC_POINTER_ERROR_RETURN(FeedLineNumber);

    CutPaper = (pCutPaper)m_possdkLibrary.resolve("CutPaper");
    FUNC_POINTER_ERROR_RETURN(CutPaper);

    Reset = (pReset)m_possdkLibrary.resolve("Reset");
    FUNC_POINTER_ERROR_RETURN(Reset);

    KickOutDrawer = (pKickOutDrawer)m_possdkLibrary.resolve("KickOutDrawer");
    FUNC_POINTER_ERROR_RETURN(KickOutDrawer);

    ApplicationUnit = (pApplicationUnit)m_possdkLibrary.resolve("ApplicationUnit");
    FUNC_POINTER_ERROR_RETURN(ApplicationUnit);

    PrintDensity = (pPrintDensity)m_possdkLibrary.resolve("PrintDensity");
    FUNC_POINTER_ERROR_RETURN(PrintDensity);

    MotionUnit = (pMotionUnit)m_possdkLibrary.resolve("MotionUnit");
    FUNC_POINTER_ERROR_RETURN(MotionUnit);

    SelectPaperType = (pSelectPaperType)m_possdkLibrary.resolve("SelectPaperType");
    FUNC_POINTER_ERROR_RETURN(SelectPaperType);

    SelectPaperTypeEEP = (pSelectPaperTypeEEP)m_possdkLibrary.resolve("SelectPaperTypeEEP");
    FUNC_POINTER_ERROR_RETURN(SelectPaperTypeEEP);

    return TRUE;
}
