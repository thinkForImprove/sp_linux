/***************************************************************
* 文件名称：DevImpl_CRT780B.cpp
* 文件描述：CRT780B退卡模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/
#include "DevImpl_MB2.h"
#include "MB2Def.h"
#include "../XFS_PPR/def.h"

static const char *ThisModule = "DevImpl_MB2.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        Log(ThisModule, 1, "检查设备OPEN标记: #OFLAG == FALSE, Device Not Open, return fail.Return: %s.", \
        ConvertErrCodeToStr(IMP_ERR_UNKNOWN)); \
        return IMP_ERR_UNKNOWN; \
    }

CDevImpl_MB2::CDevImpl_MB2()
{
    SetLogFile(LOG_NAME, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_MB2::CDevImpl_MB2(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_MB2::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    //ClearErrorCode(&m_errorInfo);

    memset(byFontType, 0x00, sizeof(byFontType));   // 字体
    wFontSize = 11;         // 五号字体(单位:磅)
    wLineHeight = 0;        // 行高，缺省: 30，单位:0.1mm
    wCharSpace = 0;         // 字符间距，缺省: 0，单位:0.1mm
    wRowDistance = 0;       // 行间距，缺省: 0，单位:0.1mm
    wLeft = 0;              // 左边距，缺省: 0，单位:0.1mm
    wTop = 0;               // 上边距，缺省: 0，单位:0.1mm
    wWidth = 0;             // 可打印宽，缺省: 0，单位:0.1mm
    wHeight = 0;            // 可打印高，缺省: 0，单位:0.1mm

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    // 动态库接口函数初始化
    Open = nullptr;
    Close = nullptr;
    InsertMedia = nullptr;
    EjectMedia = nullptr;
    SwallowPassBook = nullptr;
    Beep = nullptr;
    GetMeidaPosH = nullptr;
    SetPageLength = nullptr;
    PrtWriteData = nullptr;
    PrtChangeCodeDistance = nullptr;
    PrtSelWestCode = nullptr;
    PrtFlushPrt = nullptr;
    PrtMoveAbsRelPos = nullptr;
    PrtSetFontWidth = nullptr;
    Scan = nullptr;
    ScanSetImageCtrl = nullptr;

    GetMediaWidth = nullptr;
    GetMediaLenth = nullptr;
    PrtMoveAbsPos = nullptr;
    PrtPrintText = nullptr;
    Reset = nullptr;
    CleanError = nullptr;
    PrtSetPtrProperty = nullptr;
    GetMediaStatus = nullptr;
    GetStatus = nullptr;

    MsRead = nullptr;
//    TurnOnLight = nullptr;
//    TurnOffLight = nullptr;
    PrxPrtPrintBmp = nullptr;
//   PrxSetFontType = nullptr;
//    PrxPrtFont = nullptr;

    bCancelFlg = FALSE;             //TEST#30

    // 加载设备动态库
    if (bLoadLibrary() != TRUE)
    {
        Log(ThisModule, 1, "加载动态库: bLoadLibrary() fail. ");
    }
}

CDevImpl_MB2::~CDevImpl_MB2()
{

}


/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
************************************************************/
LONG CDevImpl_MB2::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    char cConnectType[4] = {0};
    memcpy(cConnectType, "USB",3);

    LONG lBaudRate = 9600;
    LONG lRetErrCode = 0;
    // 打开设备
    m_HComm = Open(cConnectType, 0, 0, nullptr, &lRetErrCode, lBaudRate);
    if (m_HComm == 0)
    {
//      iRetErrCode = GetLastErrorCode();
      Log(ThisModule, __LINE__, "OpenDevice() Failed, ErrCode:%d, Return: %s.",
          lRetErrCode, ConvertErrCodeToStr(lRetErrCode));
      return lRetErrCode;
    }

    m_bDevOpenOk = TRUE;

    Log(ThisModule, 1, "打开设备: OpenDevice() Success.");

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bDevOpenOk == TRUE)
    {
        if (Close(m_HComm) != 0)
        {
            if (Close(m_HComm) != 0)
            {
                Close(m_HComm);
            }
        }
    }

    m_bDevOpenOk = FALSE;

    vUnLoadLibrary();

    return IMP_SUCCESS;
}

BOOL CDevImpl_MB2::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/************************************************************
 * 功能: 3. 启动分纸
 * 参数: nCheckBox 入参 票箱号(1~4)
 *      nPaperType 入参 票据类型(0支票/1存单/2国债)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nFeedCheck(INT nCheckBox, INT nPaperType)
{
 /*   THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nCheckBox < 0 || nCheckBox > 4)
    {
        Log(ThisModule, __LINE__, "启动分纸: 票箱号[%d] Failed, Return: %s.",
            nCheckBox, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 分纸
    nRet = FeedCheck(nCheckBox, nPaperType);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "启动分纸: FeedCheck(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nCheckBox, nPaperType, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }*/

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 4. 获取票号
 * 参数: lpCheckNo 票号识别信息，调用前需申请缓冲区，传入NULL不识别票号
 *      lpFilename 要保存的票号的文件路径包含文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nGetCheckNum(LPSTR lpCheckNo, LPSTR lpFilename)
{
  /*  THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpFilename == nullptr)
    {
        Log(ThisModule, __LINE__, "获取票号: 保存的票号的文件路径[%s] Failed, Return: %s.",
            lpFilename, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = GetCheckNum(lpCheckNo, lpFilename);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票号: GetCheckNum(%s, %s) Failed, ErrCode:%d, Return: %s.",
            lpCheckNo, lpFilename, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }*/

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 5. 启动打印和扫描
 * 参数: lpPrintContent 入参　打印内容字符串
 *      bScanEnabled 入参　0不扫描 非零扫描
 *      nPrintNext 入参　是否启动下一张打印 0否 1 是
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nPrintAndScan(LPSTR lpPrintContent, INT nScanEnabled, INT nPrintNext)
{
/*    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpPrintContent == nullptr)
    {
        Log(ThisModule, __LINE__, "启动打印和扫描: 打印内容字符串[%s] Failed, Return: %s.",
            lpPrintContent, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    nRet = PrintAndScan(lpPrintContent, nScanEnabled, nPrintNext);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "启动打印和扫描: PrintAndScan(%s, %d, %d) Failed, ErrCode:%d, Return: %s.",
            lpPrintContent, nScanEnabled, nPrintNext, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }
*/
    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 6. 闸门开关控制
 * 参数: nCmd 入参 0开门，1关门
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nOutPaperDoorControl(INT nCmd)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nCmd < 0 || nCmd > 1)
    {
        Log(ThisModule, __LINE__, "闸门开关控制: 标记入参[%d] Failed, Return: %s.",
            nCmd, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
//    nRet = OutPaperDoorControl(nCmd);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "闸门开关控制: OutPaperDoorControl(%d) Failed, ErrCode:%d, Return: %s.",
            nCmd, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 7. 鉴伪/保存图像
 * 参数: iIdentifyResult 回参 鉴伪结果 1：真票 0：假票
 *      lpIdentifyInfo 入参 需申请缓冲区，鉴伪结果待定，如果为NULL，则不进行鉴伪
 *      nBufferlen 入参 缓冲区长度
 *      lpFrontImageFileName 入参 要保存的正面的文件路径包含文件名
 *      lpRearImageFileName 入参 要保存的反面的文件路径包含文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nGetIdentifyInfo(INT *iIdentifyResult, LPSTR lpIdentifyInfo, INT nBufferlen,
                                        LPSTR lpFrontImageFileName, LPSTR lpRearImageFileName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = GetIdentifyInfo(iIdentifyResult, lpIdentifyInfo, nBufferlen, lpFrontImageFileName, lpRearImageFileName);
    if (nRet != 0)
    {
//        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "鉴伪/保存图像: GetIdentifyInfo(%s, %s, %d, %s, %s) Failed, ErrCode:%d, Return: %s.",
            iIdentifyResult, lpIdentifyInfo, nBufferlen, lpFrontImageFileName, lpRearImageFileName,
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 8. 查询设备状态
 * 参数: stDevicestatus 回参设备状态结构体
 * 返回值: 参考错误码
************************************************************/
LONG CDevImpl_MB2::nGetDevStatus(unsigned char *DevCode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    long lRet = 0;

    lRet = GetStatus(m_HComm, nullptr);
    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "查询设备状态: nGetDevStatus() Failed, Return Code:%d", lRet);
     }
    return lRet;
}

/************************************************************
 * 功能: 8. 查询设备状态
 * 参数: stDevicestatus 回参设备状态结构体
 * 返回值: 参考错误码
************************************************************/
LONG CDevImpl_MB2::nGetMediaStatus(unsigned char *DevCode){
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    long lRet = 0;

    lRet = GetMediaStatus(m_HComm, nullptr);
    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "查询媒体状态: nGetMediaStatus() Failed, Return Code:%d", lRet);
     }
    return lRet;
}

/************************************************************
 * 功能: 9. 打开闸门取支票
 * 参数: nTimeout 入参 闸门关闭超时时间
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nTakeCheck(INT nTimeOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = TakeCheck(nTimeOut);
    if (nRet != 0)
    {
  //      nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "打开闸门取支票: TakeCheck(%d) Failed, ErrCode:%d, Return: %s.",
            nTimeOut, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 10. 出票或退票，退票时可选择是否盖章
 * 参数: nMode 入参 1出票/0退票；
 *      bStamp 入参 1盖章/0不盖章
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nSetCheckOut(INT nMode, INT nStamp)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nMode < 0 || nMode > 1)
    {
        Log(ThisModule, __LINE__, "出票或退票: 动作入参[%d] Failed, Return: %s.",
            nMode, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }
    if (nStamp < 0 || nStamp > 1)
    {
        Log(ThisModule, __LINE__, "出票或退票: 盖章入参[%d] Failed, Return: %s.",
            nStamp, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    //nRet = SetCheckOut(nMode, nStamp);
    if (nRet != 0)
    {
//        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "出票或退票: SetCheckOut(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nMode, nStamp, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 11. 设备复位
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nResetDev()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    nRet = Reset(m_HComm);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设备复位: ResetDev() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 12. 获取固件版本号
 * 参数: nBufSize 入参 缓存数据长度
 *      lpVersion 回参 固件版本信息
 * 返回值: 参考错误码
************************************************************/
LONG CDevImpl_MB2::nGetFirmwareVersion(LPSTR lpVersion)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LONG lRet = 0;
    // 入参检查
    if (lpVersion == nullptr)
    {
        Log(ThisModule, __LINE__, "获取固件版本号: 缓存变量入参[%s] Failed, Return: %s.",
            lpVersion, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    char cVersionFw[128] = {0};
    lRet = GetFirmwareVersion(m_HComm,cVersionFw);
    if (lRet != 0)
    {
        CQtTime::Sleep(10000);
        lRet = GetFirmwareVersion(m_HComm,cVersionFw);
        if(strlen(cVersionFw) != 0){
            memcpy(lpVersion,cVersionFw,strlen(cVersionFw));
        }
        Log(ThisModule, __LINE__, "获取固件版本号: GetFirmwareVersion Failed, Return:%d HComm=%d ",
            lRet, m_HComm);
        return lRet;
    }
    if(strlen(cVersionFw) != 0){
        memcpy(lpVersion,cVersionFw,strlen(cVersionFw));
    }
    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 13. 固件升级
 * 参数: lpFileName 入参 固件程序文件名
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nUpdateFirmware(LPSTR lpFileName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpFileName == nullptr)
    {
        Log(ThisModule, __LINE__, "固件升级: 固件程序文件名[%s] Failed, Return: %s.",
            lpFileName, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    //nRet = UpdateFirmware(lpFileName);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "固件升级: UpdateFirmware(%s) Failed, ErrCode:%d, Return: %s.",
            lpFileName, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 14. 获取最后一个错误码
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nGetLastErrorCode()
{
    THISMODULE(__FUNCTION__);
    //// AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

//    return GetLastErrorCode();
    return 1;
}

/************************************************************
 * 功能: 15. 可将通道中的票据走出,回收时可选择是否盖章
 * 参数: nMode 入参 1出票/0退票；
 *      bStamp 入参 1盖章/0不盖章
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nMoveCheck(INT nMode, INT nStamp)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (nMode < 0 || nMode > 1)
    {
        Log(ThisModule, __LINE__, "将通道中的票据走出: 动作入参[%d] Failed, Return: %s.",
            nMode, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }
    if (nStamp < 0 || nStamp > 1)
    {
        Log(ThisModule, __LINE__, "将通道中的票据走出: 盖章入参[%d] Failed, Return: %s.",
            nStamp, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    //nRet = MoveCheck(nMode, nStamp);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "将通道中的票据走出: MoveChec(%d, %d) Failed, ErrCode:%d, Return: %s.",
            nMode, nStamp, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 16. 遗忘回收
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nRetractCheck()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = RetractCheck();
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "遗忘回收: RetractCheck() Failed, ErrCode:%d, Return: %s.",
            nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 17. 设置票号OCR识别区域
 * 参数: lpArea 入参 票号OCR识别区域，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nSetCheckNumOCRArea(LPSTR lpArea)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpArea == nullptr)
    {
        Log(ThisModule, __LINE__, "设置票号OCR识别区域: 入参 OCR识别区域[%s] Failed, Return: %s.",
            lpArea, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    //nRet = SetCheckNumOCRArea(lpArea);
    if (nRet != 0)
    {
//        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设置票号OCR识别区域: SetCheckNumOCRArea(%s) Failed, ErrCode:%d, Return: %s.",
            lpArea, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 18. 获取票号OCR识别结果
 * 参数: nSize 入参 pchOCR缓冲区大小（字节数）。
 *      lpFilename 入参 要保存的票号的文件路径包含文件名，如果要bmp图像文件后缀为“.bmp”，如果要jpg图像，后缀为“.jpg”，传入空不保存图像。
 *      lpOCR 回参　票号OCR识别结果，使用JSON格式。
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nGetCheckNumFromArea(LPSTR lpOCR, INT nSize, LPSTR lpFilename)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = GetCheckNumFromArea(lpOCR, nSize, lpFilename);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票号OCR识别结果: GetCheckNumFromArea(%s, %d, %s) Failed, ErrCode:%d, Return: %s.",
            lpOCR, nSize, lpFilename, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 19. 获取票据的正反面图像
 * 参数: lpFrontImageFile 入参 票据正面图像保存路径（含文件名），如果为NULL则不保存正面图像。
 *      lpBackImageFile 入参 票据反面图像保存路径（含文件名），如果为NULL则不保存反面图像
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nGetCheckImage(LPSTR lpFrontImageFile, LPSTR lpBackImageFile)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = GetCheckImage(lpFrontImageFile, lpBackImageFile);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票据的正反面图像: GetCheckImage(%s, %s) Failed, ErrCode:%d, Return: %s.",
            lpFrontImageFile, lpBackImageFile, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 20. 设置票面OCR识别区域
 * 参数: lpArea 入参 票面OCR识别区域，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nSetCheckOCRArea(LPSTR  lpArea)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpArea == nullptr)
    {
        Log(ThisModule, __LINE__, "设置票面OCR识别区域: 入参 OCR识别区域[%s] Failed, Return: %s.",
            lpArea, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    //nRet = SetCheckOCRArea(lpArea);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "设置票面OCR识别区域: SetCheckOCRArea(%s) Failed, ErrCode:%d, Return: %s.",
            lpArea, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 21. 获取票据鉴伪及OCR信息
 * 参数: nSize 入参 lpInfo缓冲区大小（字节数）
 *      lpInfo 回参 票据鉴伪及OCR信息，使用JSON格式
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nGetCheckInfo(LPSTR lpInfo, INT nSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查
    if (lpInfo == nullptr)
    {
        Log(ThisModule, __LINE__, "获取票据鉴伪及OCR信息: 回参　票据鉴伪及OCR信息[%s] Failed, Return: %s.",
            lpInfo, ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    //
    //nRet = GetCheckInfo(lpInfo, nSize);
    if (nRet != 0)
    {
//        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, "获取票据鉴伪及OCR信息: GetCheckInfo(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpInfo, nSize, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 22.
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nGetCheckNumImage(LPSTR lpFileName)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = GetCheckNumImage(lpFileName);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": GetCheckNumImage(%s) Failed, ErrCode:%d, Return: %s.",
            lpFileName, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 23.
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nGetRfidInfo(LPSTR lpRfidInfo)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = GetRfidInfo(lpRfidInfo);
    if (nRet != 0)
    {
//        nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": GetRfidInfo(%s) Failed, ErrCode:%d, Return: %s.",
            lpRfidInfo, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 24.
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nReadCommand(INT nRelength, LPBYTE lpBuffer, INT* nLength)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = ReadCommand(nRelength, lpBuffer, nLength);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": ReadCommand(%d, %s, %d) Failed, ErrCode:%d, Return: %s.",
            nRelength, lpBuffer, *nLength, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 25.
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_MB2::nSendCommand(LPSTR lpBuffer, INT nLength){
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    INT nRetErrCode = 0;

    // 入参检查

    //
    //nRet = SendCommand(lpBuffer, nLength);
    if (nRet != 0)
    {
 //       nRetErrCode = GetLastErrorCode();
        Log(ThisModule, __LINE__, ": SendCommand(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpBuffer, nLength, nRetErrCode, ConvertErrCodeToStr(nRetErrCode));
        return nRetErrCode;
    }

    return IMP_SUCCESS;
}


INT CDevImpl_MB2::ConvertErrorCode(INT nRet)
{
    return IMP_SUCCESS;
}

CHAR* CDevImpl_MB2::ConvertErrCodeToStr(long lRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(lRet)
    {
    case IMP_SUCCESS:
        sprintf(m_szErrStr, "%d|%s", lRet, "成功");
        return m_szErrStr;
    }
}

BOOL CDevImpl_MB2::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);


    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            Log(ThisModule, 1, "加载动态库<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        } else
        {
            Log(ThisModule, 1, "加载动态库<%s> Succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            Log(ThisModule, 1, "加载动态库函数接口<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
        {
            Log(ThisModule, 1, "加载动态库函数接口<%s> Succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_MB2::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_MB2::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

     //打开设备
//     LOAD_LIBINFO_FUNC(OPEN, Open, "PRxOpen");
     LOAD_LIBINFO_FUNC(OPEN, Open, "_Z7PRxOpenPKcllPcPll");

//     LOAD_LIBINFO_FUNC(OPEN, Open, "_Z7OpenComPKcl");

     //关闭设备
     LOAD_LIBINFO_FUNC(CLOSE, Close, "CloseDev");

     //设备复位初始化
     LOAD_LIBINFO_FUNC(RESET, Reset, "_Z12PRxResetInitl");

     //打印机清错
     LOAD_LIBINFO_FUNC(CLEANERROR, CleanError, "_Z13PRxCleanErrorl");

     //取设备状态
     LOAD_LIBINFO_FUNC(GETSTATUS, GetStatus, "_Z12PRxGetStatuslPh");

     //获取设备中媒体状态
     LOAD_LIBINFO_FUNC(GETMEDIASTATUS, GetMediaStatus, "_Z17PRxGetMediaStatuslPh");

     //插入介质控制
     LOAD_LIBINFO_FUNC(INSERTMEDIA, InsertMedia, "_Z14PRxInsertMedialll");

     //按指定方向退出打印机内部介质
     LOAD_LIBINFO_FUNC(EJECTMEDIA, EjectMedia, "_Z13PRxEjectMediali");

     //获取介质长度
     LOAD_LIBINFO_FUNC(GETMEDIALENGTH, GetMediaLenth, "_Z17PRxGetMediaLengthl");

     //把存折从打印机后部退出
     LOAD_LIBINFO_FUNC(SWALLOWPASSBOOK, SwallowPassBook, "_Z18PRxSwallowPassBookl");

     //获取介质宽度
     LOAD_LIBINFO_FUNC(GETMEDIAWIDTH, GetMediaWidth, "_Z16PRxGetMediaWidthl");

     //获取存折距离左边界的距离
     LOAD_LIBINFO_FUNC(GETMEDIAPOSH, GetMeidaPosH, "_Z15PRxGetMediaPosHl");

     //鸣响
     LOAD_LIBINFO_FUNC(BEEP, Beep, "_Z7PRxBeeplll");

     //查询打印机固件版本信息
     LOAD_LIBINFO_FUNC(GETFIRMWAREVERSION, GetFirmwareVersion, "_Z21PRxGetFirmwareVersionlPc");

     //打印功能函数
     //设定页面长度
     LOAD_LIBINFO_FUNC(SETPAGELENGTH, SetPageLength, "_Z16PRxSetPageLengthll");
     //向端口发送16进制数据
     LOAD_LIBINFO_FUNC(PRTWRITEDATA, PrtWriteData, "_Z15PRxPrtWriteDatalPhll");
     //在字符右边加空列
     LOAD_LIBINFO_FUNC(PRTCHANGECODEDISTANCE, PrtChangeCodeDistance, "_Z24PRxPrtChangeCodeDistancelf");
     //设置西文字符类型
     LOAD_LIBINFO_FUNC(PRTSELWESTCODE, PrtSelWestCode, "_Z17PRxPrtSelWestCodeli");
     //刷新打印机缓冲区
     LOAD_LIBINFO_FUNC(PRTFLUSHPRT, PrtFlushPrt, "_Z14PRxPrtFlushPrtl");
     //移动打印头到指定绝对位置
     LOAD_LIBINFO_FUNC(PRTMOVEABSPOS, PrtMoveAbsPos, "_Z16PRxPrtMoveAbsPoslll");
     //移动打印头到相对位置
     LOAD_LIBINFO_FUNC(PRTMOVEABSRELPOS, PrtMoveAbsRelPos, "_Z16PRxPrtMoveRelPosll");
     //打印输出数据
     LOAD_LIBINFO_FUNC(PRTPRINTTEXT, PrtPrintText, "_Z15PRxPrtPrintTextlPc");
     //设置字符宽度
     LOAD_LIBINFO_FUNC(PRTSETFONTWIDTH, PrtSetFontWidth, "_Z18PRxPrtSetFontWidthlf");
     //读磁道
     LOAD_LIBINFO_FUNC(MSREAD, MsRead, "_Z9PRxMsReadlPcl");
     //打印位图
     LOAD_LIBINFO_FUNC(PRXPRTPRINTBMP, PrxPrtPrintBmp, "_Z14PRxPrtPrintBmplllPcS_S_");
     //插入介质控制
     LOAD_LIBINFO_FUNC(PRTSETPTRPROPERTY, PrtSetPtrProperty, "_Z20PRxPrtSetPtrPropertylii");

     //扫描功能函数
     //设扫描基本控制参数
     LOAD_LIBINFO_FUNC(SCANSETCTRL, sCanSetCtrl, "_Z14PRxScanSetCtrlP11tagScanCtrlll");
     //设图像扫描控制参数
     LOAD_LIBINFO_FUNC(SCANSETIMAGECTRL, ScanSetImageCtrl, "_Z19PRxScanSetImageCtrlP16tagScanImageCtrlPclllllllll");
     //按指定扫描参数扫描图像
     LOAD_LIBINFO_FUNC(SCAN, Scan, "_Z7PRxScanl11tagScanCtrlP16tagScanImageCtrlS1_");
     LOAD_LIBINFO_FUNC(PRXMSCONFIGHMS, PrxMsconfigHMS,"_Z14PRxMsConfigHMSlllhbl");
     LOAD_LIBINFO_FUNC(PRXMSREADEX, MsReadEx,"_Z11PRxMsReadExlPc");


     return TRUE;
}
/**
 @功能：	行模式打印字符串
 @参数：	lpPrintString  : 待打印字符串
 @      wStringSize    : 待打印字符串size
 @      wStartX        : 列坐标
 @返回：	TRUE/FALSE
 */
INT CDevImpl_MB2::LineModePrintText(LPSTR lpPrintString, unsigned long wStringSize, LONG wStartX, LONG wStartY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    LONG lRet = 0;
 //   string strResult;
 //   strResult.append(lpPrintString, wStringSize);
    char  cStrResult[1024] = {0};
    memcpy(cStrResult, lpPrintString,wStringSize);

//    Log(ThisModule, 1, "行模式打印字符串: StartX:%d, DataSize:%d, PrintData:%s.", wStartX, wStringSize, cStrResult);

    if (wStringSize < 1)
    {
        Log(ThisModule, 1, "行模式打印字符串: 打印数据长度[wStringSize = %d] < 1, NotPrint, Return: %s",
            wStringSize, ConvertErrCodeToStr(IMP_SUCCESS));
        return IMP_SUCCESS;
    }

    lRet = InsertMedia(m_HComm,0,0);
    lRet = PrtSetPtrProperty(m_HComm,1,6);            //CPI18:打印字距//
    //  lRet = PrtSetPtrProperty(m_HComm,1,0);            //CPI10:打印字距
    lRet = PrtSetPtrProperty(m_HComm,2,9);
    lRet = PrtSetPtrProperty(m_HComm,2,1);
    lRet = PrtSetPtrProperty(m_HComm,2,2);
    // lRet = PrtSetPtrProperty(m_HComm,2,3);
    lRet = PrtSetPtrProperty(m_HComm,2,8);
    lRet = PrtSetPtrProperty(m_HComm,2,6);
    lRet = PrtSetPtrProperty(m_HComm,2,7);
    lRet = PrtSetPtrProperty(m_HComm,2,11);
    lRet = PrtSetPtrProperty(m_HComm,2,13);
    lRet = PrtMoveAbsPos(m_HComm, wStartX,wStartY);
    lRet = PrtPrintText(m_HComm, cStrResult);
    if(lRet != 0){
       Log(ThisModule, 1, "LineModePrintText:  fail. Return: %s.",
           ConvertErrCodeToStr(lRet));
       return lRet;
    }
    return IMP_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////
/**
 @功能：	设置打印模式
 @参数：	wPrintMode: 0,行模式(标准模式) / 1,页模式
 @返回：	TRUE/FALSE
 */
INT CDevImpl_MB2::BK_SetPrintMode(WORD wPrintMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    ////int nRet = PrintSetMode(m_hPrinter, wPrintMode);
    //if (nRet != ERR_SUCCESS)
    {
        //Log(ThisModule, 1, "设置打印模式: PrintSetMode(%d:0行/2页) fail. Return: %s.", wPrintMode, ConvertErrCodeToStr(nRet));
        return FALSE;
    }

    return IMP_SUCCESS;
}

/**
 @功能：	设置打印行高
 @参数：	wLineHeight: 行高，单位参考IsSpot
 @           IsSpot: TRUE,wLineHeight单位为0.1mm; FALSE,wLineHeight单位为点
 @返回：	TRUE/FALSE
 */
INT CDevImpl_MB2::BK_SetTextLineHight(WORD wLineHeight, BOOL IsSpot /* = FALSE */)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    //int nRet = SetTextLineHight(m_hPrinter, (IsSpot == TRUE ? wLineHeight : MM2SPOT(wLineHeight)));
/*    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "SetTextLineHight(%d|%d) fail. Return: %s.",
            wLineHeight, MM2SPOT(wLineHeight), ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    return IMP_SUCCESS;
}

/**
 @功能：	设置字符间距
 @参数：	nCharSpace: 字符间距，单位参考IsSpot
 @          IsSpot: TRUE,nCharSpace单位为0.1mm; FALSE,nCharSpace单位为点
 @返回：	TRUE/FALSE
 */
INT CDevImpl_MB2::BK_SetTextCharacterSpace(WORD nCharSpace, BOOL IsSpot /* = FALSE */)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

 /*   CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    int nRet = SetTextCharacterSpace(m_hPrinter, 0,
                                     (IsSpot == TRUE ? nCharSpace : MM2SPOT(nCharSpace)), 0);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "SetTextCharacterSpace(0, %d|%d, 0) fail. Return: %s.",
            nCharSpace, MM2SPOT(nCharSpace), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    nRet = SetTextCharacterSpace(m_hPrinter, 0,
                                 (IsSpot == TRUE ? nCharSpace : MM2SPOT(nCharSpace)), 1);
    if (nRet != ERR_SUCCESS)
    {
        Log(ThisModule, 1, "SetTextCharacterSpace(0, %d|%d, 1) fail. Return: %s.",
            nCharSpace, MM2SPOT(nCharSpace), ConvertErrCodeToStr(nRet));
        return nRet;
    }*/

    return IMP_SUCCESS;
}
INT CDevImpl_MB2::PrintFrontSetFormat()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

 /*   INT nRet = IMP_SUCCESS;
    if (wLineHeight > 0) // 设置打印行高
    {
        if ((nRet = BK_SetTextLineHight(wLineHeight)) != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "打印参数设置: 设置打印行高: BK_SetTextLineHight(%d) fail. Return: %s.",
                wLineHeight, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    if (wCharSpace > 0) // 设置字符间距
    {
        if ((nRet = BK_SetTextCharacterSpace(wCharSpace)) != ERR_SUCCESS)
        {
            Log(ThisModule, 1, "打印参数设置: 设置字符间距: BK_SetTextCharacterSpace(%d) fail. Return: %s.",
                wCharSpace, ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }*/

    return IMP_SUCCESS;
}

/*void CDevImpl_MB2::SetPrintFormat(STPRINTFORMAT stPrintFormat)
{
    //wRowDistance = (stPrintFormat.uLPI > 0 ? stPrintFormat.uLPI : wRowDistance);  // 行间距
    if (strlen(stPrintFormat.szFontType) > 0 && stPrintFormat.uFontSize > 0) {
        memcpy(byFontType, stPrintFormat.szFontType, strlen(stPrintFormat.szFontType));
        wFontSize = stPrintFormat.uFontSize;
    }
    wLineHeight = (stPrintFormat.uLPI > 0 ? stPrintFormat.uLineHeight : wLineHeight);  // 行间距
    wCharSpace = (stPrintFormat.uWPI > 0 ? stPrintFormat.uWPI : wCharSpace);      // 字符间距
}*/

INT  CDevImpl_MB2::nRetractMediaToBox()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT iRet = 0;
    iRet = SwallowPassBook(m_HComm);
    if(iRet != 0){
        Log(ThisModule, 1, "Retarct pass book fail. Return: %d.",
            ConvertErrCodeToStr(iRet));
        return iRet;
    }
    return IMP_SUCCESS;

}

INT  CDevImpl_MB2::nControlMedia(MEDIA_ACTION2 enMediaAct)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记
    INT iRet = 0;
    iRet = EjectMedia(m_HComm, enMediaAct);
    if(iRet != 0){
        Log(ThisModule, 1, "eject passbook fail. Return: %d.",
            ConvertErrCodeToStr(iRet));
        return iRet;
    }
    return IMP_SUCCESS;
}

//test#30
LONG CDevImpl_MB2::lMediaInsertCheck(LONG lHcomm)
{
    char cMoveToo[16] = {0x1b,0x6c,0x1b,0x6e,0x1b,0x51,0x32,
                       0x35,0x30,0x1b,0x5a,0x1b,0x4c,0x30,0x30,0x30};
    LONG lResult = 0;
    while (lResult == ERR_UNKNOW || lResult >= 0)
    {
        lResult = GetMediaStatus(lHcomm, nullptr);

        if(bCancelFlg == TRUE){
            lResult = ERR_PTR_CANCEL;
            bCancelFlg = FALSE;
            break;
        }
        switch (lResult)
        {
            case STATUS_MEDIA_PRESENT:
            {
                lResult = SUCCESS;
                break;
            }
            case ERR_COMM_READ:
            {
                Beep(lHcomm, 1, 1500);
                lResult = ERR_UNKNOW;
                break;
            }
            case STATUS_MEDIA_NONE:
            {
                Beep(lHcomm, 1, 1500);
                break;
            }

            case STATUS_MEDIA_PREENTERING:
            case STATUS_MEDIA_ENTERING:
            case ERR_UNKNOW:
            {
                 PrtWriteData(lHcomm, (unsigned char *)cMoveToo,sizeof(cMoveToo), 0);//这里用PRxWriteData( )接口发个"1b 6c 1b 6e 1b 51 32 35 30 1b 5a 1b 4c 30 30 30 ”定位到第0行来代替。
            }
                break;
            default:
                return lResult;
        }

        if (lResult == SUCCESS)
        {
            break;
        }
    }
    return lResult;
 }

LONG  CDevImpl_MB2::nMsRead(LPSTR szData, LONG lmagneticType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记
    char cMicrData1[130]={0};

    LONG lRet = 0;
    lRet = lMediaInsertCheck(m_HComm);                          //test#30
    if(lRet != 0){
        Log(ThisModule, 1, "Insert Media cancel. Return: %d.",
            ConvertErrCodeToStr(lRet));
        return lRet;
    }
    lRet = InsertMedia(m_HComm,0,0);
    if(lRet != 0){
        Log(ThisModule, 1, "Insert Media fail. Return: %d.",
            ConvertErrCodeToStr(lRet));
        return lRet;
    }

/*    if(lmagneticType == 0){                             //读取磁道２
      lRet = PrxMsconfigHMS(m_HComm, 0, 0,0x0C,false,1);
    }else{                                              //读取磁道３
      lRet = PrxMsconfigHMS(m_HComm, 1, 0,0x0C,false,1);
    }
    if(lRet != 0){
        Log(ThisModule, 1, "PrxMsconfigHMS setting fail. Return: %d.",
            ConvertErrCodeToStr(lRet));
        return lRet;
    }*/
    lRet = MsRead(m_HComm, cMicrData1, lmagneticType);
//   lRet = MsReadEx(m_HComm, cMicrData1);
    if(lRet < 0){
        Log(ThisModule, 1, "Read form fail. Return: %d.",
            ConvertErrCodeToStr(lRet));
        return lRet;
    }

    memcpy(szData, cMicrData1, sizeof(cMicrData1));

    return lRet;
}

//Print image
LONG CDevImpl_MB2::nPrintImage(char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    LONG lRet = 0;
    string csStrTemp = "";
    string csStrTemp2 = szImagePath;
    int iStrCnt = csStrTemp2.find(".");
    csStrTemp = csStrTemp2.substr(0,iStrCnt);
    csStrTemp += ".dat";
    char cImageName[128] = {0};
    memcpy(cImageName,csStrTemp.c_str(),csStrTemp.length());

    lRet = PrxPrtPrintBmp(m_HComm,ulWidth,ulHeight,szImagePath, cImageName,"0");

    return WFS_SUCCESS;

}
//test#30
LONG CDevImpl_MB2::lSetCancelFlg(BOOL bFlgCancel)
{
    bCancelFlg = bFlgCancel;
}
//---------------------------------------------
