/***************************************************************
* 文件名称：XFS_PTR.h
* 文件描述：
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1

* 版本历史信息2
* 变更说明：1. Rawdata打印增加自动换行;2.JAM时dev为HWErr
* 票   号：30-00-00-00(FT#0008)
* 变更日期：2019年8月1日
* 文件版本：1.0.0.2
****************************************************************/

#include "XFS_PTR.h"
#include "string.h"
#include "QtTypeInclude.h"
#include <QTextCodec>
#include <stdlib.h>
#include <unistd.h>

// PTR SP 版本号
BYTE    byVRTU[17] = {"HWSPRSTE00000100"};

// 事件日志
static const char *ThisFile = "XFX_PTR";

#define LOG_NAME     "XFS_PTR.log"          // 30-00-00-00(FT#0052)

#define IDS_INFO_CUTPAPER_SUCCESS   "凭条打印机切纸成功"
#define IDS_INFO_RESET_DEVICE       "开始修复打印机"
#define IDS_INFO_RESET_INFO         "打印机复位成功,无纸,返回值=[%d] 传入参数为：[%u]"
#define IDS_INFO_RESET_SUCCESS      "打印机复位成功,返回值=[%d] 传入参数为：[%u]"
#define IDS_INFO_STARTUP_INFO       "Printer StartUp, DeviceName=[%s], DeviceDLLName=[%s]"
#define IDS_INFO_PAPER_TAKEN        "纸被取走"

#define IDS_ERR_REESET_ERROR        "打印机复位错误, 返回值=[%d]"
#define IDS_ERR_RESET_AND_CUTPAPER  "打印机复位ControlMedia错误,返回值=[%d]"
#define IDS_ERR_JPR_UNSUPP_COMMAND  "流水打印机不支持此指令[%u]"
#define IDS_ERR_RPR_UNSUPP_COMMAND  "凭条打印机不支持此指令[%u]"
#define IDS_ERR_NO_PAPER_WHENCUT    "切纸时发现无纸"
#define IDS_ERR_PAPER_JAMMED        "打印机卡纸"
#define IDS_ERR_CUTPAPER_ERROR      "切纸错误(检测黑标:[%d],进纸:[%d])"
#define IDS_ERR_DEVIVE_STA          "当前设备状态不为ONLINE，当前设备状态为：[%d]"
#define IDS_ERR_PRINTSTRING_FAILD   "PrintString失败:[%d]"
#define IDS_ERR_PRINTIMAGE_FAILD    "打印图片错误:[%d]"
#define IDS_ERR_PRINT_FAILD         "打印错误:[%d]"
#define IDS_ERR_INIT_ERROR          "初始化设备错误:[%d]"
#define IDS_ERR_COMPORT_ERROR       "串口参数不正确"
#define IDS_ERR_LOADDLL_FAILD       "LoadLibrary[%s]失败:Error()=[%d]"
#define IDS_ERR_GetProcAdd_FAILD    "GetProcAddress[%s], CreatePrinterDevice)失败"
#define IDS_ERR_CreateDev_FAILD     "调用[%s]的CreatePrinterDevice[%s]失败"
#define IDS_ERR_Open_FAIlD          "调用[%s]的Open()失败:%[d]"
#define IDS_ERR_NO_CHARSET          "系统缺少中文字符集"



void GetDocFileName(char *pDocFileName, DWORD dwLen);

//const char *PrinterStatus2Desc(int nPrinterStatus)
//{
//    switch (nPrinterStatus)
//    {
//    case PTR_DEV_COMM_ERR:
//        return "ComError";
//    case PTR_DEV_CUTTER:
//        return "CutterError";
//    case PTR_DEV_HEADER:
//        return "HeaderError";
//    case PTR_DEV_ONLINE:
//        return "Online";
//    case PTR_DEV_OFFLINE:
//        return "Offline";
//    case PTR_DEV_POWEROFF:
//        return "PowerOff";
//    case PTR_DEV_NODEVICE:
//        return "NoDevice";
//    case PTR_DEV_HWERROR:
//        return "HWError";
//    case PTR_DEV_USERERROR:
//        return "UserError";
//    case PTR_DEV_BUSY:
//        return "Busy";
//    default:
//        return "OtherError";
//    }
//}


CXFS_PTR::CXFS_PTR()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件 // 30-00-00-00(FT#0052)
    m_WaitTaken = WTF_NONE;
    m_bNeedReset = true;
}

CXFS_PTR::~CXFS_PTR()
{

}


long CXFS_PTR::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseBCR
    if (0 != m_pBase.Load("SPBasePTR.dll", "CreateISPBasePTR", "PTR"))
    {
        Log(ThisModule, __LINE__, "加载SPBasePTR失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_PTR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    InitPTRData(lpLogicalName);
    InitConifig();
    InitStatus();
    //InitCaps();

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    return StartOpen();		// 30-00-00-00(FT#0032)
}

HRESULT CXFS_PTR::OnClose()
{
    THISMODULE(__FUNCTION__);
    Log(ThisModule, 1, "PRT SP Close ");
    if (m_pPrinter != nullptr)
    {
        Log(ThisModule, 1, "m_pPrinter->Close()");
        m_pPrinter->Close();
    }
    return 0;
}

HRESULT CXFS_PTR::OnStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    int iRet = ERR_PTR_SUCCESS;

    if (m_pPrinter == nullptr)
    {
        iRet = ERR_PTR_NOT_OPEN;
    } else
    {
        iRet = m_pPrinter->QueryStatus();
    }

    if (iRet == ERR_PTR_NOT_OPEN)
    {
        //iRet = m_pPrinter->Open(nullptr);					// 30-00-00-00(FT#0015)
        /*iRet = m_pPrinter->OpenDev((unsigned short)m_sConfig.nDriverType);// 30-00-00-00(FT#0015)
        if (iRet < 0)
        {
            Log(ThisModule, -1, "Auto OpenDev(%d) fail, ErrCode:%d", m_sConfig.nDriverType, iRet);// 30-00-00-00(FT#0015)
            //return 0;
        }*/
        StartOpen();	// 30-00-00-00(FT#0032)
        iRet = m_pPrinter->QueryStatus();					// 30-00-00-00(FT#0015)
    }

    UpdateDeviceStatus(iRet);
    return 0;
}

HRESULT CXFS_PTR::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    if (m_WaitTaken == WTF_NONE)	// 30-00-00-00(FT#0048)
    {								// 30-00-00-00(FT#0048)
        return WFS_ERR_CANCELED;	// 30-00-00-00(FT#0048)
    }								// 30-00-00-00(FT#0048)
	
    int iRet = m_pPrinter->QueryStatus();
    UpdateDeviceStatus(iRet);
    return 0;
}

HRESULT CXFS_PTR::OnCancelAsyncRequest()
{
    return 0;
}

HRESULT CXFS_PTR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// PTR类型接口
// INFOR
HRESULT CXFS_PTR::GetStatus(LPWFSPTRSTATUS &lpStatus)
{
    m_sStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_sStatus;
    return 0;
}

HRESULT CXFS_PTR::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_sCaps;
    return 0;
}

HRESULT CXFS_PTR::GetFormList(LPSTR &lpszFormList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    lpszFormList = (LPSTR)m_pData->GetNames();
    return 0;
}

HRESULT CXFS_PTR::GetMediaList(LPSTR &lpszMediaList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    lpszMediaList = (LPSTR)m_pData->GetNames(false);
    return 0;
}

HRESULT CXFS_PTR::GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader)
{
    THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    long hRes;
    {
        CSPPrinterForm *pForm = m_pData->FindForm((LPCSTR)lpFormName);
        if (pForm)
        {
            if (pForm->IsLoadSucc())
            {
                m_pData->m_LastForm.ExtractFromForm(pForm);

                SIZE sizeForm = pForm->GetOrigSize();
                for (DWORD iChild = 0; iChild < pForm->GetSubItemCount(); iChild++)
                {
                    ISPPrinterItem *pItem = pForm->GetSubItem(iChild);
                    if (pItem == NULL)
                        continue;
                    SIZE sizeItem = pItem->GetOrigSize();
                    SIZE3 sizePosItem = pItem->GetOrigPosition();
                    if (sizePosItem.cx + sizeItem.cx > sizeForm.cx ||
                        sizePosItem.cy + sizeItem.cy > sizeForm.cy)
                    {
                        Log(ThisModule, -1, IDS_ERR_FORM_INVALID, pForm->GetName());
                        return WFS_ERR_PTR_FORMINVALID;
                    }
                }
                hRes = WFS_SUCCESS;
            }
            else
            {
                hRes = WFS_ERR_PTR_FORMINVALID;
            }
        }
        else
        {
            hRes = WFS_ERR_PTR_FORMNOTFOUND;
        }
    }
    lpFrmHeader = &m_pData->m_LastForm;
    return hRes;
}

HRESULT CXFS_PTR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
{
    THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    long hRes;
    {
        CSPPrinterMedia *pMedia = m_pData->FindMedia((LPCSTR)lpMediaName);
        if (pMedia)
        {
            if (pMedia->IsLoadSucc())
            {
                m_pData->m_LastMedia.ExtractFromMedia(pMedia);

                if (m_pData->m_LastMedia.wPrintAreaX + m_pData->m_LastMedia.wPrintAreaHeight > m_pData->m_LastMedia.wPrintAreaHeight ||
                    m_pData->m_LastMedia.wPrintAreaY + m_pData->m_LastMedia.wPrintAreaWidth  > m_pData->m_LastMedia.wPrintAreaWidth)
                {
                    return WFS_ERR_PTR_MEDIAINVALID;
                }

                hRes = WFS_SUCCESS;
            }
            else
            {
                hRes = WFS_ERR_PTR_MEDIAINVALID;
            }
        }
        else
        {
            hRes = WFS_ERR_PTR_MEDIANOTFOUND;
        }
    }
    lpFrmMedia = &m_pData->m_LastMedia;
    return hRes;

}

HRESULT CXFS_PTR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
{
    THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    long hRes = WFS_SUCCESS;
    m_pData->LoadForms();
    {
        hRes = m_pData->FindField(lpQueryField->lpszFormName, lpQueryField->lpszFieldName);
        if (WFS_SUCCESS == hRes)
        {
            if (0 == m_pData->m_LastFields.GetCount())
            {
                hRes = WFS_ERR_PTR_FIELDNOTFOUND;
            }
            else
            {
                hRes = WFS_SUCCESS;

            }
        }
    }
    lpszMediaList = m_pData->m_LastFields;
    return 0;
}

// EXECUTE
HRESULT CXFS_PTR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    return ControlMedia(*lpdwMeidaControl);
}

HRESULT CXFS_PTR::PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut)
{    
    if (m_sStatus.fwDevice != WFS_PTR_DEVONLINE &&			//30-00-00-00（FT#0008）
        m_sStatus.fwDevice != WFS_PTR_DEVBUSY)				//30-00-00-00（FT#0008）
    {														//30-00-00-00（FT#0008）
        return WFS_ERR_HARDWARE_ERROR;						//30-00-00-00（FT#0008）
    }														//30-00-00-00（FT#0008）

    if (m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT)			//30-00-00-00（FT#0008）
    {														//30-00-00-00（FT#0008）
        return WFS_ERR_PTR_PAPEROUT;						//30-00-00-00（FT#0008）
    }														//30-00-00-00（FT#0008）

    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    m_pData->LoadMedias();
    LPWFSPTRPRINTFORM pIn = (LPWFSPTRPRINTFORM)lpPrintForm;
    long hRes = InnerPrintForm(pIn);
    return hRes;
}

HRESULT CXFS_PTR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    m_pData->LoadMedias();
    m_pData->m_ReadFormOut.Clear();
    LPWFSPTRREADFORM pIn = (LPWFSPTRREADFORM)lpReadForm;
    long hRes = InnerReadForm(pIn);
    if (WFS_SUCCESS == hRes)
    {
        lpReadFormOut = m_pData->m_ReadFormOut;
    }
    return hRes;
}

HRESULT CXFS_PTR::RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    LPWFSPTRRAWDATA pIn = (LPWFSPTRRAWDATA)lpRawData;

    if (m_sStatus.fwDevice != WFS_PTR_DEVONLINE &&				//30-00-00-00（FT#0008）
        m_sStatus.fwDevice != WFS_PTR_DEVBUSY)					//30-00-00-00（FT#0008）
    {															//30-00-00-00（FT#0008）
        return WFS_ERR_HARDWARE_ERROR;							//30-00-00-00（FT#0008）
    }															//30-00-00-00（FT#0008）

    if (m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT)				//30-00-00-00（FT#0008）
    {															//30-00-00-00（FT#0008）
        return WFS_ERR_PTR_PAPEROUT;							//30-00-00-00（FT#0008）
    }															//30-00-00-00（FT#0008）

    long hRes = SendRawData(WFS_PTR_INPUTDATA == pIn->wInputData, pIn->ulSize, pIn->lpbData);
    if (WFS_SUCCESS == hRes)
    {
        lpRawDataIn = (LPWFSPTRRAWDATAIN)m_pData->m_InputRawData;
    }
    return hRes;
}

HRESULT CXFS_PTR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    THISMODULE(__FUNCTION__);
    //    WFSPTRMEDIAUNIT *pIn = (WFSPTRMEDIAUNIT *)lpMediaUnit;
    //    m_pData->m_LastExtents.ulSizeX = pIn->wUnitX;
    //    m_pData->m_LastExtents.ulSizeY = pIn->wUnitY;
    //    long hRes = GetMediaExtents(pIn->wBase, &m_pData->m_LastExtents.ulSizeX, &m_pData->m_LastExtents.ulSizeY);
    //    if(WFS_SUCCESS == hRes)
    //    {
    //        lpMediaExt = &m_pData->m_LastExtents;
    //    }
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_PTR::ResetCount(const LPUSHORT lpusBinNum)
{
    THISMODULE(__FUNCTION__);
    //long hRes = ResetRetractBinCount(lpusBinNum != nullptr ? *(USHORT *)lpusBinNum : -1);
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_PTR::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //    m_pData->m_ReadImageOut.Clear();
    //    LPWFSPTRIMAGEREQUEST lpIn = (LPWFSPTRIMAGEREQUEST)lpImgRequest;
    //    long hRes = ReadImage(lpIn);
    //    if(WFS_SUCCESS == hRes)
    //    {
    //        lppImage = (LPWFSPTRIMAGE*)m_pData->m_ReadImageOut;
    //    }
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_PTR::Reset(const LPWFSPTRRESET lpReset)
{
    LPWFSPTRRESET pIn = (LPWFSPTRRESET)lpReset;
    long hRes = Reset(nullptr != pIn ? pIn->dwMediaControl : 0,
                      nullptr != pIn ? pIn->usRetractBinNumber : 0);
    return hRes;
}

HRESULT CXFS_PTR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_PTR::DispensePaper(const LPWORD lpPaperSource)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

// Fire消息
void CXFS_PTR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_PTR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_PTR::FireNoMedia(LPCSTR szPrompt)
{
    //FireEvent(MFT_EE, WFS_EXEE_PTR_NOMEDIA, WFS_SUCCESS, (LPVOID)szPrompt);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}
void CXFS_PTR::FireMediaInserted()
{
    //FireEvent(MFT_EE, WFS_EXEE_PTR_MEDIAINSERTED);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

void CXFS_PTR::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    //FireEvent(MFT_EE, WFS_EXEE_PTR_FIELDERROR, WFS_SUCCESS, &fail);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

void CXFS_PTR::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    //FireEvent(MFT_EE, WFS_EXEE_PTR_FIELDWARNING, WFS_SUCCESS, &fail);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}
void CXFS_PTR::FireRetractBinThreshold(USHORT BinNumber, WORD wStatus)
{
    WFSPTRBINTHRESHOLD BinThreshold;
    BinThreshold.usBinNumber     = BinNumber;
    BinThreshold.wRetractBin = wStatus;
    //FireEvent(MFT_UE, WFS_USRE_PTR_RETRACTBINTHRESHOLD, WFS_SUCCESS, &BinThreshold);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_RETRACTBINTHRESHOLD, &BinThreshold);
}
void CXFS_PTR::FireMediaTaken()
{
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIATAKEN);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}
void CXFS_PTR::FirePaperThreshold(WORD wSrc, WORD wStatus)
{
    WFSPTRPAPERTHRESHOLD PaperThreshold;
    PaperThreshold.wPaperSource     = wSrc;
    PaperThreshold.wPaperThreshold  = wStatus;
    //FireEvent(MFT_UE, WFS_USRE_PTR_PAPERTHRESHOLD, WFS_SUCCESS, &PaperThreshold);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_PAPERTHRESHOLD, &PaperThreshold);
}
void CXFS_PTR::FireTonerThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_TONERTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_TONERTHRESHOLD, &wStatus);
}
void CXFS_PTR::FireInkThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_INKTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}
void CXFS_PTR::FireLampThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_LAMPTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}
void CXFS_PTR::FireSRVMediaInserted()
{
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIAINSERTED);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIAINSERTED, nullptr);
}
void CXFS_PTR::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIADETECTED, WFS_SUCCESS, &md);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}


inline int MulDiv(int number, int numberator, int denominator)
{
    long long ret = number;
    ret *= numberator;
    if (0 == denominator)
    {
        ret = (-1);
    }
    else
    {
        ret /= denominator;
    }
    return (int) ret;
}

// 初始Open/断线重连Open  // 30-00-00-00(FT#0032)
HRESULT CXFS_PTR::StartOpen()
{
    char *szDevPTRVer = NULL;
    char szFWVersion[64] = { 0x00 };
    long lFWVerSize = 0;

    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (!LoadDevDll(ThisModule))
    {
        return WFS_SUCCESS;//WFS_ERR_HARDWARE_ERROR;
    }

    //int nRet = m_pPrinter->Open(nullptr);
    int nRet = m_pPrinter->OpenDev((unsigned short)m_sConfig.nDriverType);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "Open fail , ErrCode:%d", nRet);
        return WFS_SUCCESS;//WFS_ERR_HARDWARE_ERROR;
    }
    m_bNeedReset = false;
    if (m_sConfig.dwMarkHeader > 0)
    {
        nRet = m_pPrinter->ChkPaperMarkHeader(m_sConfig.dwMarkHeader);
        if (nRet < 0)
        {
            Log(ThisModule, -1, "ChkPaperMarkHeader fail, ErrCode:%d", nRet);
            return WFS_SUCCESS;//WFS_ERR_HARDWARE_ERROR;
        }
        Log(ThisModule, 1, "ChkPaperMarkHeader success");
    }



    nRet = OnInit();
    //if(nRet < 0)
    //     return nRet;

    //    nRet = GetFWVersion();
    //    if(nRet < 0)
    //        return nRet;

    szDevPTRVer = m_pPrinter->GetVersion();
    lFWVerSize = sizeof(szFWVersion);
    if (m_pPrinter->GetFWVersion(szFWVersion, (unsigned long*)(&lFWVerSize)) != TRUE) {
        memset(szFWVersion, 0x00, sizeof(szFWVersion));
    }

    //m_cExtra.AddExtra("SPVer", "1.0.1");
    m_cExtra.Clear();
    m_cExtra.AddExtra("VRTCount", "3");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", szDevPTRVer);
    m_cExtra.AddExtra("VRTDetail[02]", szFWVersion);
    return WFS_SUCCESS;
}


long CXFS_PTR::InnerPrintForm(LPWFSPTRPRINTFORM pInData)
{
    const char *const ThisModule = "InnerPrintForm";
    AutoLogFuncBeginEnd();

    CSPPtrData *pData = (CSPPtrData *)m_pData;
    PrintContext pc;
    memset(&pc, 0, sizeof(pc));

    pc.pPrintData = pInData;

    pc.pForm = pData->FindForm(pInData->lpszFormName);
    if (!pc.pForm)
    {
        Log(ThisModule, -1, IDS_ERR_FORM_NOT_FOUND, pInData->lpszFormName);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!pc.pForm->IsLoadSucc())
    {
        Log(ThisModule, -1, IDS_ERR_FORM_INVALID, pInData->lpszFormName);
        return WFS_ERR_PTR_FORMINVALID;
    }
    pc.pMedia = pData->FindMedia(pInData->lpszMediaName);
    if (!pc.pMedia)
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_NOT_FOUND, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIANOTFOUND;
    }
    if (!pc.pMedia->IsLoadSucc())
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_INVALID, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIAINVALID;
    }

    // 检查是否Media可打印区域超过其自身大小
    do
    {
        RECT rectMedia = {0, 0, 0, 0};
        pc.pMedia->GetPrintArea(rectMedia);
        SIZE sizeMedia = pc.pMedia->GetSize();
        if (rectMedia.right > sizeMedia.cx ||
            rectMedia.bottom > sizeMedia.cy)
        {
            Log(ThisModule, -1, IDS_ERR_MEDIA_INVALID, pc.pMedia->GetName());
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    } while (0);

    RECT rcMD;
    ((CSPPrinterForm *)pc.pForm)->GetMulDiv(rcMD);

    if (WFS_PTR_OFFSETUSEFORMDEFN == pInData->wOffsetX)
    {
        pInData->wOffsetX = pc.pForm->GetPosition().cx;
    }
    if (WFS_PTR_OFFSETUSEFORMDEFN == pInData->wOffsetY)
    {
        pInData->wOffsetY = pc.pForm->GetPosition().cy;
    }
    SIZE offset;
    offset.cx = MulDiv(pInData->wOffsetX, rcMD.left, rcMD.right);
    offset.cy = MulDiv(pInData->wOffsetY, rcMD.top, rcMD.bottom);

    SIZE sizeForm = pc.pForm->GetSize();
    SIZE sizeMedia = pc.pMedia->GetSize();
    if ((sizeMedia.cx > 0 && offset.cx + sizeForm.cx > sizeMedia.cx) ||
        (sizeMedia.cy > 0 && offset.cy + sizeForm.cy > sizeMedia.cy))
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_OVERFLOW, pInData->lpszFormName, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    RECT rc;
    pc.pMedia->GetPrintArea(rc);
    if (offset.cx < rc.left ||
        (rc.right - rc.left > 0 && sizeForm.cx + offset.cx > rc.right - rc.left) ||
        offset.cy < rc.top ||
        (rc.bottom - rc.top > 0 && sizeForm.cy + offset.cy > rc.bottom - rc.top))
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_OVERFLOW_PRINTAREA, pInData->lpszFormName, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    FORMALIGN FormAlign = pc.pForm->GetAlign();
    if (WFS_PTR_ALNUSEFORMDEFN != pInData->wAlignment &&
        WFS_PTR_ALNTOPLEFT != pInData->wAlignment &&
        WFS_PTR_ALNTOPRIGHT != pInData->wAlignment &&
        WFS_PTR_ALNBOTTOMLEFT != pInData->wAlignment &&
        WFS_PTR_ALNBOTTOMRIGHT != pInData->wAlignment)
    {
        return WFS_ERR_INVALID_DATA;
    }
    if (WFS_PTR_ALNUSEFORMDEFN != pInData->wAlignment)
    {
        FormAlign = (FORMALIGN)(pInData->wAlignment - WFS_PTR_ALNTOPLEFT + TOPLEFT);
    }
    switch (FormAlign)
    {
    case TOPLEFT:
        break;  //(default)
    case TOPRIGHT:
        if (0 < sizeMedia.cx)
        {
            offset.cx = sizeMedia.cx - sizeForm.cx - offset.cx;
        }
        break;
    case BOTTOMLEFT:
        if (0 < sizeMedia.cy)
        {
            offset.cy = sizeMedia.cy - sizeForm.cy - offset.cy;
        }
        break;
    case BOTTOMRIGHT:
        if (0 <  sizeMedia.cx)
        {
            offset.cx = sizeMedia.cx - sizeForm.cx - offset.cx;
        }
        if (0 < sizeMedia.cy)
        {
            offset.cy = sizeMedia.cy - sizeForm.cy - offset.cy;
        }
        break;
    default:
        return WFS_ERR_INVALID_DATA;
    }

    CMultiString Fields = pInData->lpszFields;

    //功能：打印字段内容或FRAME
    if (m_sConfig.nVerifyField > 0)
    {
        do
        {
            for (int i = 0; i < Fields.GetCount(); i++)
            {
                LPCSTR lpField = Fields.GetAt(i);
                if (NULL == lpField)
                {
                    continue;
                }
                char szFieldName[1024] = {0};
                for (int j = 0; j < (int)strlen(lpField) && j < 1023; j++)
                {
                    if ('=' != lpField[j]  && '\0' != lpField[j])
                    {
                        szFieldName[j] = lpField[j];
                    }
                }
                if (0 == strcmp(szFieldName, ""))
                {
                    continue;
                }
                DWORD iChild = 0;
                for (; iChild < pc.pForm->GetSubItemCount(); iChild++)
                {
                    if (0 == strcmp(pc.pForm->GetSubItem(iChild)->GetName(), szFieldName))
                    {
                        break;
                    }
                }
                if (iChild == pc.pForm->GetSubItemCount())
                {
                    Log(ThisModule, -1, IDS_ERR_FILED_NOT_FOUND, szFieldName);
                    if (m_sConfig.nVerifyField == 1)
                    {
                        return WFS_ERR_PTR_FIELDNOTFOUND;
                    } else
                    {
                        FireFieldWarning(pInData->lpszFormName, szFieldName, WFS_PTR_FIELDNOTFOUND);
                    }
                }
            }

        } while (0);
    }

    HRESULT hRes = StartForm(&pc);
    if (WFS_SUCCESS != hRes)
    {
        Log(ThisModule, Result2ErrorCode(hRes), IDS_ERR_START_FORM, pInData->lpszFormName);
        return hRes;
    }

    for (DWORD iChild = 0; iChild < pc.pForm->GetSubItemCount() && WFS_SUCCESS == hRes; iChild++)
    {
        ISPPrinterItem *pItem = pc.pForm->GetSubItem(iChild);
        SIZE3 SubOffset;
        SubOffset.cx = SubOffset.cy = SubOffset.cz = 0;
        pc.pSubform = NULL;
        if (ITEM_SUBFORM == pItem->GetItemType())
        {
            pc.pSubform = (ISPPrinterSubform *)pItem;
            SubOffset = pc.pSubform->GetPosition();
        }
        for (DWORD iField = 0; (!pc.pSubform || iField < pc.pSubform->GetSubItemCount()) && WFS_SUCCESS == hRes; iField++)
        {
            if (pc.pSubform)
            {
                pItem = pc.pSubform->GetSubItem(iField);
            }
            SIZE OffsetAll = offset;
            OffsetAll.cx += SubOffset.cx;
            OffsetAll.cy += SubOffset.cy;
            hRes = PrintFieldOrFrame(pc, pItem, OffsetAll, Fields);
            if (!pc.pSubform)
            {
                break;
            }
        }
    }

    if (WFS_SUCCESS != hRes)
    {
        pc.bCancel = TRUE;
    }

    // HRESULT hResOld = hRes;                              // 30-00-00-00(FT#0052)
    hRes = EndForm(&pc);

    if (hRes != WFS_SUCCESS || pInData->dwMediaControl == 0)// 30-00-00-00(FT#0052)
    {                                                       // 30-00-00-00(FT#0052)
        UpdateDeviceStatus(ERR_PTR_SUCCESS);                // 30-00-00-00(FT#0052)
        if (m_sStatus.fwMedia == WFS_PTR_MEDIAJAMMED)       // 30-00-00-00(FT#0052)
        {                                                   // 30-00-00-00(FT#0052)
            m_bNeedKeepJammedStatus = TRUE;                 // 30-00-00-00(FT#0052)
            Log(ThisModule, -1, IDS_ERR_PAPER_JAMMED);      // 30-00-00-00(FT#0052)
            return WFS_ERR_PTR_PAPERJAMMED;                 // 30-00-00-00(FT#0052)
        }                                                   // 30-00-00-00(FT#0052)
    }                                                       // 30-00-00-00(FT#0052)

    //return  WFS_SUCCESS != hResOld ? hResOld : hRes;      // 30-00-00-00(FT#0052)
    return  hRes;                                           // 30-00-00-00(FT#0052)
}

long CXFS_PTR::InnerReadForm(LPWFSPTRREADFORM pInData)
{
    return WFS_ERR_UNSUPP_COMMAND;
}


bool CXFS_PTR::LoadDevDll(LPCSTR ThisModule)
{
    if (m_pPrinter == nullptr)
    {
        //char szDevDllName[256] = { 0 };
        //int  nDriverType = 1;
        //strcpy(szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));
        //nDriverType = m_cXfsReg.GetValue("DriverType", 1);
        //if (0 != m_pPrinter.Load(szDevDllName, "CreateIDevPTR", "PTR"))
        if (0 != m_pPrinter.Load(m_sConfig.szDevDllName, "CreateIDevPTR", DEVTYPE2STR(m_sConfig.nDriverType)))
        {
            Log(ThisModule, __LINE__, "加载库失败: Driv"
                                      "erDllName=%s, DriverType=%d|%s, ERR:%s",
                m_sConfig.szDevDllName, m_sConfig.nDriverType,
                DEVTYPE2STR(m_sConfig.nDriverType), m_pPrinter.LastError().toUtf8().constData());
            return false;
        }
    }
    return (m_pPrinter != nullptr);
}

void CXFS_PTR::InitConifig()
{
    LPCSTR ThisModule = "InitConifig";

    strcpy(m_sConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));
    m_sConfig.nDriverType = m_cXfsReg.GetValue("DriverType", DEV_SNBC_BKC310);
    SetDeviceType(m_sConfig.nDriverType);

    m_sConfig.type  = (PTR_TYPE)m_cXfsReg.GetValue("CONFIG", "type", 1);
    m_sConfig.bDetectBlackStripe = m_cXfsReg.GetValue("CONFIG", "feedblcakdetect", 1) != 0;
    m_sConfig.nFeed = (int)m_cXfsReg.GetValue("CONFIG", "feedsize", (DWORD)0);

    m_sConfig.dwMarkHeader = m_cXfsReg.GetValue("CONFIG", "MarkHeader", 318);

    m_sConfig.nVerifyField = m_cXfsReg.GetValue("CONFIG", "verify_field", (DWORD)0);
    if (m_sConfig.nVerifyField < 0 || m_sConfig.nVerifyField > 2)
        m_sConfig.nVerifyField = 0;

    m_sConfig.nPageSize = m_cXfsReg.GetValue("CONFIG", "split_size", 2976);
    m_sConfig.nPageLine = m_cXfsReg.GetValue("CONFIG", "page_lines", 30);
    m_sConfig.nLineSize = m_cXfsReg.GetValue("CONFIG", "line_size", 50);    // 30-00-00-00(FT#0008)
    m_sConfig.bEnableSplit = m_cXfsReg.GetValue("CONFIG", "enabled_split", 1) != 0;


    LPCSTR lpFont = m_cXfsReg.GetValue("CONFIG", "FontType", "");
    strcpy(m_stPrintFormat.szFontType, lpFont);
    Log(ThisModule, 1, "加载FontType%s", m_stPrintFormat.szFontType);
    m_stPrintFormat.uFontSize = m_cXfsReg.GetValue("CONFIG", "FontSize", (DWORD)0);
    m_stPrintFormat.ulStyle = m_cXfsReg.GetValue("CONFIG", "Style", (DWORD)0);
    m_stPrintFormat.uWPI = m_cXfsReg.GetValue("CONFIG", "WPI", (DWORD)0);
    m_stPrintFormat.uLPI = m_cXfsReg.GetValue("CONFIG", "LPI", (DWORD)0);
    m_stPrintFormat.uLineHeight = m_cXfsReg.GetValue("CONFIG", "LineHeight", (DWORD)0);
    Log(ThisModule, 1, "Style%d", m_stPrintFormat.ulStyle);

    if (49 > m_sConfig.nPageSize)
    {
        m_sConfig.nPageSize = 50;
    }
    if (2976 < m_sConfig.nPageSize)
    {
        m_sConfig.nPageSize = 2976;
    }

    if (1 > m_sConfig.nPageLine)
    {
        m_sConfig.nPageLine = 1;
    }
    if (44 < m_sConfig.nPageLine)
    {
        m_sConfig.nPageLine = 44;
    }

    if (m_sConfig.nLineSize < 1 || m_sConfig.nLineSize > 50)    // 30-00-00-00(FT#0008)
    {                                                           // 30-00-00-00(FT#0008)
        m_sConfig.nLineSize = 50;                               // 30-00-00-00(FT#0008)
    }                                                           // 30-00-00-00(FT#0008)

    m_sConfig.nTakeSleep = m_cXfsReg.GetValue("TakeCfg", "TakeSleep", (DWORD)3);
}

long CXFS_PTR::UpdateStatus()
{
    return 0;
}

long CXFS_PTR::InitStatus()
{
    memset(&m_sStatus, 0x00, sizeof(WFSPTRSTATUS));
    m_sStatus.fwDevice      = WFS_PTR_DEVNODEVICE;
    m_sStatus.fwPaper[0]    = WFS_PTR_PAPERUNKNOWN;
    m_sStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
    m_sStatus.fwToner       = WFS_PTR_TONERFULL;
    m_sStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
    m_sStatus.fwInk         = WFS_PTR_INKNOTSUPP;
}

long CXFS_PTR::ConvertErrCode(long lRes)
{
    switch (lRes)
    {
    case ERR_PTR_SUCCESS:       return WFS_SUCCESS;
    case ERR_PTR_PARAM_ERR:     return WFS_ERR_UNSUPP_DATA;
    case ERR_PTR_COMM_ERR:      return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_NO_PAPER:      return WFS_ERR_PTR_PAPEROUT;
    case ERR_PTR_JAMMED:        return WFS_ERR_PTR_PAPERJAMMED;
    case ERR_PTR_NOT_OPEN:      return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_HEADER:        return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_CUTTER:        return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_TONER:         return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_STACKER_FULL:  return WFS_ERR_PTR_STACKERFULL;
    case ERR_PTR_NO_RESUME:     return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_CAN_RESUME:    return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_FORMAT_ERROR:  return WFS_ERR_UNSUPP_DATA;
    case ERR_PTR_CHRONIC:       return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_HWERR:         return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_IMAGE_ERROR:   return WFS_ERR_HARDWARE_ERROR;
    case ERR_PTR_UNSUP_CMD:     return WFS_ERR_UNSUPP_COMMAND;
    case ERR_PTR_NO_DEVICE:     return WFS_ERR_HARDWARE_ERROR;
    default:                    return WFS_ERR_HARDWARE_ERROR;
    }
}

HRESULT CXFS_PTR::OnInit()
{
    const char *ThisModule = "OnInit";

    if (m_sConfig.type == PTR_TYPE_RECEIPT)
    {
        //m_pPrinter = new CReceiptPrinter;
    }

    long nRet = m_pPrinter->Init();
    UpdateDeviceStatus(nRet);
    if (nRet < 0)
    {
        return ConvertErrCode(nRet);
    }
    m_pPrinter->SetPrintFormat(m_stPrintFormat);
    return ConvertErrCode(nRet);
}

HRESULT CXFS_PTR::OnExit()
{
    if (nullptr != m_pPrinter)
    {
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}


//HRESULT CXFS_PTR::StartForm(PrintContext *pContext)
//{
//    const char *ThisModule = "StartForm";
////    if (m_pPrinter->IsJournalPrinter())
////    {
////        DWORD dwControl = pContext->pPrintData->media_control;
////        if (WFS_PTR_CTRLFLUSH == dwControl || 0 == dwControl)
////        {
////            return CXFS_PTR::StartForm(pContext);
////        }
////        else
////        {
////            Log(ThisModule, -1, IDS_ERR_JPR_UNSUPP_COMMAND, dwControl);
////            return WFS_ERR_UNSUPP_DATA;
////        }
////    }

//    return StartForm(pContext);
//}


void CXFS_PTR::SetStatus(int nPrinter, int nPaper, int nTone)
{
    /*
    // modified by huanghc 20091123, 驱动中设备状态为门口有纸则赋值SP时为ONLINE
    m_sStatus.fwDevice = (PTR_DEV_PAPERENTER == nPrinter ? PTR_DEV_ONLINE : nPrinter);

    m_sStatus.fwPaper[0] = nPaper;
    for(int nr = 1; nr < WFS_PTR_SUPPLYSIZE; nr ++)
    {
        m_sStatus.paper[nr] = WFS_PTR_PAPERNOTSUPP;
    }

    m_sStatus.fwToner = nTone;
    m_sStatus.fwInk   = WFS_PTR_INKNOTSUPP;
    m_sStatus.fwLamp  = WFS_PTR_LAMPNOTSUPP;
    if (PTR_DEV_PAPERENTER == nPrinter)
    {
        m_sStatus.fwMedia = WFS_PTR_MEDIAENTERING;
    }
    else if(WFS_PTR_PAPEROUT == nPaper)
    {
        m_sStatus.fwMedia = WFS_PTR_MEDIANOTPRESENT;
    }
    else if (WFS_PTR_PAPERUNKNOWN == nPaper)
    {
        m_sStatus.fwMedia = WFS_PTR_PAPERUNKNOWN;
    }
    else
    {
        m_sStatus.fwMedia = WFS_PTR_MEDIAPRESENT;
    }
    */
}


HRESULT CXFS_PTR::ControlMedia(DWORD dwControl)
{
    const char *ThisModule = "ControlMedia";

    if (m_sStatus.fwDevice != WFS_PTR_DEVONLINE &&			//30-00-00-00（FT#0008）
        m_sStatus.fwDevice != WFS_PTR_DEVBUSY)				//30-00-00-00（FT#0008）
    {														//30-00-00-00（FT#0008）
        return WFS_ERR_HARDWARE_ERROR;						//30-00-00-00（FT#0008）
    }														//30-00-00-00（FT#0008）

    if (m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT)			//30-00-00-00（FT#0008）
    {														//30-00-00-00（FT#0008）
        return WFS_ERR_PTR_PAPEROUT;						//30-00-00-00（FT#0008）
    }														//30-00-00-00（FT#0008）

    //    if (FALSE != m_pPrinter->IsJournalPrinter())
    //    {
    //        if (WFS_PTR_CTRLFLUSH == dwControl || 0 == dwControl)
    //        {
    //            return WFS_SUCCESS;
    //        }
    //        else
    //        {
    //            Log(ThisModule, -1, IDS_ERR_JPR_UNSUPP_COMMAND, dwControl);
    //            return WFS_ERR_UNSUPP_DATA;
    //        }
    //    }
    //    else
    //    {
    if ((WFS_PTR_CTRLCUT & dwControl) || (WFS_PTR_CTRLEJECT &  dwControl)) // 切纸
    {
        //m_nFeed                = 0;//todo (int)atol(GetSPIniValue(GetSPName(), "default", "feed_paper_after_black_stripe_detected", "0"));
        //m_bDetectBlackStripe = 0;//todo (int)atol(GetSPIniValue(GetSPName(), "default", "detect_black_stripe", "0"));
        //CloseTimer(TIMEID_UPDATE_STATUS);
        //int nRet = m_pPrinter->CutPaper(m_bDetectBlackStripe, m_nFeed);
        m_sConfig.bDetectBlackStripe = m_cXfsReg.GetValue("CONFIG", "feedblcakdetect", 1) != 0;
        m_sConfig.nFeed = (int)m_cXfsReg.GetValue("CONFIG", "feedsize", (DWORD)0);
        Log(ThisModule, 1, "bDetectBlackStripe%d,m_sConfig.nFeed :%d"
            , m_sConfig.bDetectBlackStripe, m_sConfig.nFeed);
        int nRet = m_pPrinter->CutPaper(m_sConfig.bDetectBlackStripe, m_sConfig.nFeed);
        usleep(500 * 1000); // 休止0.5秒，确保获取状态准确
        UpdateDeviceStatus(nRet);
        if (ERR_PTR_SUCCESS != nRet)
        {
            if (ERR_PTR_NO_PAPER == nRet)
            {
                Log(ThisModule, -1, IDS_ERR_NO_PAPER_WHENCUT);
                return WFS_ERR_PTR_PAPEROUT;
            }
            else if (ERR_PTR_JAMMED == nRet)
            {
                m_bNeedKeepJammedStatus = TRUE;
                Log(ThisModule, -1, IDS_ERR_PAPER_JAMMED);
                return WFS_ERR_PTR_PAPERJAMMED;
            }
            else
            {
                Log(ThisModule, -1, IDS_ERR_CUTPAPER_ERROR, \
                    m_sConfig.bDetectBlackStripe, m_sConfig.nFeed);
                return WFS_ERR_HARDWARE_ERROR;
            }
        } else
        {
            UpdateDeviceStatus(nRet);       // 30-00-00-00(FT#0052)
            if (m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT)
            {
                Log(ThisModule, -1, IDS_ERR_NO_PAPER_WHENCUT);
                return WFS_ERR_PTR_NOMEDIAPRESENT;
            } else
            if (m_sStatus.fwPaper[0] == WFS_PTR_PAPERJAMMED)
            {
                Log(ThisModule, -1, IDS_ERR_PAPER_JAMMED);
                return WFS_ERR_PTR_PAPERJAMMED;
            }
        }

        Log(ThisModule, 0, IDS_INFO_CUTPAPER_SUCCESS);
        m_WaitTaken = WTF_TAKEN;

        if (m_sConfig.nDriverType == DEV_SNBC_BTNH80 && m_sConfig.nTakeSleep > 0 &&
             m_WaitTaken == WTF_TAKEN) {
            dwTakeTimeSize = time(NULL);
        }
        return WFS_SUCCESS;
    }
    else if (WFS_PTR_CTRLFLUSH == dwControl)
    {
        if (WFS_PTR_DEVONLINE != m_sStatus.fwDevice)
        {
            Log(ThisModule, -1, IDS_ERR_DEVIVE_STA,  m_sStatus.fwDevice);
            return WFS_ERR_HARDWARE_ERROR;
        }
        else if (WFS_PTR_MEDIANOTPRESENT == m_sStatus.fwMedia)
        {
            return WFS_ERR_PTR_PAPEROUT;
        }
        else
        {
            return WFS_SUCCESS;
        }
    }
    else
    {
        Log(ThisModule, -1, IDS_ERR_RPR_UNSUPP_COMMAND, dwControl);
        return WFS_ERR_UNSUPP_DATA;
    }
    //}
}

HRESULT CXFS_PTR::SendRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData)
{
    const char *ThisModule = "SendRawData";

    if (0 == nSize)
    {
        return WFS_SUCCESS;
    }

    // 转码
    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextCodec *codec1 = QTextCodec::codecForName("gb18030");
    if (nullptr == codec1) codec1 = QTextCodec::codecForName("gb2312");
    if (nullptr == codec1) codec1 = QTextCodec::codecForName("gbk");
    if (nullptr == codec1)
    {
        Log(ThisModule, -1, IDS_ERR_PRINTSTRING_FAILD);
        return WFS_ERR_PTR_CHARSETDATA;
    }
    QString strText = QString::fromUtf8((char *)pData, nSize);
    QTextCodec::setCodecForLocale(codec1);
    QByteArray tmpData = strText.toLocal8Bit();
    char *pTempCode = tmpData.data();
    int nTempSize = tmpData.size();
    ULONG ulDataSize = 0;
    int nBufferSize = nTempSize + 2 + (nTempSize / m_sConfig.nLineSize) * 2;    // 30-00-00-00(FT#0008)
    BYTE *pBuf = new BYTE[/*nTempSize + 2*/nBufferSize];                        // 30-00-00-00(FT#0008)
    memset(pBuf, 0, /*nTempSize + 2*/nBufferSize);                              // 30-00-00-00(FT#0008)
    RemoveUnPrintableChar(nTempSize, (LPBYTE)pTempCode, ulDataSize, pBuf);
    QTextCodec::setCodecForLocale(codec);

    // 去除不可打印字符
    if ('\n' != pBuf[ulDataSize - 1])
    {
        pBuf[ulDataSize++] = '\n';
    }
    pBuf[ulDataSize] = 0;

    HRESULT hRes = PrintString((char *)pBuf, ulDataSize, FALSE);
    delete [] pBuf;
    pBuf = nullptr;

    if (WFS_SUCCESS != hRes)
    {
        Log(ThisModule, -1, IDS_ERR_PRINTSTRING_FAILD, hRes);
        return hRes;
    }

    if (bExpectResp)
    {
        //SetInputRawData(0, nullptr);
        m_pData->m_InputRawData.SetData(0, nullptr);
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PTR::Reset(DWORD dwMediaControl, USHORT usBinIndex)
{
    Q_UNUSED(usBinIndex);
    const char *const ThisModule = "Reset";
    Log(ThisModule, 1, IDS_INFO_RESET_DEVICE);

    int nRet = m_pPrinter->Init();
    nRet = m_pPrinter->QueryStatus();	// 30-00-00-00(FT#0032)
    UpdateDeviceStatus(nRet);
    if (ERR_PTR_SUCCESS != nRet)
    {
        //if (m_sStatus.fwPaper[0] == WFS_ERR_PTR_PAPEROUT)   // 无纸
        if (ERR_PTR_NO_PAPER == nRet)
        {
            m_bNeedKeepJammedStatus = FALSE;
            Log(ThisModule, 1, IDS_INFO_RESET_INFO, nRet, dwMediaControl);
            return WFS_SUCCESS;
        }

        Log(ThisModule, -1, IDS_ERR_REESET_ERROR, nRet);
        return WFS_ERR_HARDWARE_ERROR;

    }
    m_bReset = TRUE;

    m_bNeedKeepJammedStatus = FALSE;
    Log(ThisModule, 1, IDS_INFO_RESET_SUCCESS, nRet, dwMediaControl);
    return WFS_SUCCESS;
}

HRESULT CXFS_PTR::PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight)
{
    const char *ThisModule = "PrintImage";

    int nRet = m_pPrinter->PrintImage(szImagePath, nDstWidth, nDstHeight);
    UpdateDeviceStatus(nRet);
    if (ERR_PTR_SUCCESS != nRet)
    {
        Log(ThisModule, nRet, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PTR::PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY)
{
    const char *ThisModule = "PrintImageOrg";

    int nRet = m_pPrinter->PrintImageOrg(szImagePath, ulOrgX, ulOrgY);
    UpdateDeviceStatus(nRet);
    if (ERR_PTR_SUCCESS != nRet)
    {
        Log(ThisModule, nRet, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PTR::AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut)
{
    const char *const ThisModule = "AddPrintString";
    if (0 == dwSize)
    {
        return WFS_SUCCESS;
    }

    int iCurPos = 0, iLenToEnter = 0, iEnterNum = 0, iLastSize = 0;
    const char *pp = pBuffer;
    int PAGESIZE = m_sConfig.nPageSize;
    int PAGELINE = m_sConfig.nPageLine;
    // FORM打印时根据数据量判断是否需要换行
    bool bNeedCR = true;
    if (dwSize < (DWORD)PAGESIZE && FALSE != bIsFromPrint)
    {
        bNeedCR = false;
    }

    int nRet = ERR_PTR_SUCCESS;
    if (m_sConfig.bEnableSplit)
    {
        while (0 < dwSize)
        {
            iLastSize = dwSize > (DWORD)PAGESIZE ? PAGESIZE : dwSize;
            while (iCurPos < iLastSize)
            {
                if (pp[ iCurPos ] & 0x80)
                {
                    iCurPos++;
                }
                else
                {
                    if (pp[ iCurPos ] == '\n')
                    {
                        if (iEnterNum < PAGELINE)
                        {
                            ++iEnterNum;
                            iLenToEnter = iCurPos + 1;
                        }
                        else
                        {
                            iCurPos++;
                            break;
                        }
                    }
                }
                iCurPos++;
            }

            if ((iCurPos >= PAGESIZE) && ('\n' != pp[ iCurPos - 1 ]) && (0 != iLenToEnter))
            {
                iCurPos = iLenToEnter;
            }

            char cLastChar = ((char *)pp)[ iCurPos - 1 ];
            char cBackup;
            bool bChanged = false;
            // FORM打印时根据数据量判断是否需要换行
            if ('\n' != cLastChar  && bNeedCR)
            {
                cBackup = ((char *)pp)[ iCurPos ];
                ((char *)pp)[ iCurPos ] = '\n';
                iCurPos++;
                bChanged = true;
            }

            if ((dwSizeOut + iCurPos) < MAX_PRINTDATA_LEN)
            {
                memcpy(pBuffOut + dwSizeOut, pp, iCurPos);
                dwSizeOut += iCurPos;
            }
            else
            {
                Log(ThisModule, -1, "PrintData(%u) is more than MAXLEN", dwSizeOut + iCurPos);
            }
            //nRet = m_pPrinter->PrintData(const_cast<char *>( pp ), iCurPos );

            if (bChanged)
            {
                ((char *)pp)[ iCurPos ] = cBackup;
                --iCurPos;
            }
            bChanged = false;

            if (ERR_PTR_SUCCESS != nRet)
            {
                Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
                //缺纸时打印应返回WFS_ERR_PTR_PAPEROUT
                if (ERR_PTR_NO_PAPER == nRet)
                {
                    return WFS_ERR_PTR_PAPEROUT;
                }
                else
                {
                    return WFS_ERR_HARDWARE_ERROR;
                }
            }
            pp = pp + iCurPos;
            dwSize -= iCurPos;
            iCurPos = 0;
            iEnterNum = 0;
            iLenToEnter = 0;
        }
    }
    else
    {
        //nRet = m_pPrinter->PrintData(const_cast<char *>(pBuffer), dwSize);
        memcpy(pBuffOut + dwSizeOut, pp, iCurPos);
        dwSizeOut += iCurPos;
        if (ERR_PTR_SUCCESS != nRet)
        {
            Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
            // 缺纸时打印应返回WFS_ERR_PTR_PAPEROUT
            if (ERR_PTR_NO_PAPER == nRet)
            {
                return WFS_ERR_PTR_PAPEROUT;
            }
            else
            {
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
    }

    return WFS_SUCCESS;
}

long CXFS_PTR::PrintData(const char *pBuffer, DWORD dwSize)
{
    const char *ThisModule = "PrintData";
    int iRet = m_pPrinter->PrintData(pBuffer, dwSize);
    UpdateDeviceStatus(iRet);
    if (iRet)
    {
        Log(ThisModule, -1, "PrintData fail, ErrCode:%d", iRet);
    }
    return ConvertErrCode(iRet);
}

HRESULT CXFS_PTR::PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint)
{
    const char *const ThisModule = "PrintString";
    if (0 == dwSize)
    {
        return WFS_SUCCESS;
    }

    int iCurPos = 0, iLenToEnter = 0, iEnterNum = 0, iLastSize = 0;
    const char *pp = pBuffer;
    int PAGESIZE = m_sConfig.nPageSize;
    int PAGELINE = m_sConfig.nPageLine;
    // FORM打印时根据数据量判断是否需要换行
    bool bNeedCR = true;
    if (dwSize < (DWORD)PAGESIZE && FALSE != bIsFromPrint)
    {
        bNeedCR = false;
    }

    int nRet = ERR_PTR_SUCCESS;
    if (m_sConfig.bEnableSplit)
    {
        while (0 < dwSize)
        {
            iLastSize = dwSize > (DWORD)PAGESIZE ? PAGESIZE : dwSize;
            while (iCurPos < iLastSize)
            {
                if (pp[ iCurPos ] & 0x80)
                {
                    iCurPos++;
                }
                else
                {
                    if (pp[ iCurPos ] == '\n')
                    {
                        if (iEnterNum < PAGELINE)
                        {
                            ++iEnterNum;
                            iLenToEnter = iCurPos + 1;
                        }
                        else
                        {
                            iCurPos++;
                            break;
                        }
                    }
                }
                iCurPos++;
            }

            if ((iCurPos >= PAGESIZE) && ('\n' != pp[ iCurPos - 1 ]) && (0 != iLenToEnter))
            {
                iCurPos = iLenToEnter;
            }

            char cLastChar = ((char *)pp)[ iCurPos - 1 ];
            char cBackup;
            bool bChanged = false;
            // FORM打印时根据数据量判断是否需要换行
            if ('\n' != cLastChar  && bNeedCR)
            {
                cBackup = ((char *)pp)[ iCurPos ];
                ((char *)pp)[ iCurPos ] = '\n';
                iCurPos++;
                bChanged = true;
            }

            if (bIsFromPrint == false)                  // 30-00-00-00(FT#00008)
            {                                           // 30-00-00-00(FT#00008)
                if (dwSize > 0 && cLastChar == '\n')    // 30-00-00-00(FT#00008)
                {                                       // 30-00-00-00(FT#00008)
                    ((char *)pp)[ iCurPos - 1 ] = '\0'; // 30-00-00-00(FT#00008)
                }                                       // 30-00-00-00(FT#00008)
            }                                           // 30-00-00-00(FT#00008)

            nRet = m_pPrinter->PrintData(const_cast<char *>(pp), iCurPos);

            if (bChanged)
            {
                ((char *)pp)[ iCurPos ] = cBackup;
                --iCurPos;
            }
            bChanged = false;

            if (ERR_PTR_SUCCESS != nRet)
            {
                break;

                Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
            }
            pp = pp + iCurPos;
            dwSize -= iCurPos;
            iCurPos = 0;
            iEnterNum = 0;
            iLenToEnter = 0;
        }
    }
    else
    {
        nRet = m_pPrinter->PrintData(const_cast<char *>(pBuffer), dwSize);
    }
    UpdateDeviceStatus(nRet);
    return ConvertErrCode(nRet);
}

BOOL CXFS_PTR::NeedFormatString() const
{
    //    if (nullptr == m_pPrinter || m_pPrinter->IsJournalPrinter())
    //    {
    //        return FALSE;
    //    }

    if (m_sConfig.nDriverType == DEV_SNBC_BKC310 || m_sConfig.nDriverType == DEV_SNBC_BTNH80) // SNBC
    {
        return FALSE;
    }

    return TRUE;
}


void CXFS_PTR::RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData)
{
    ulOutSize = 0;
    int PAGELINE = m_sConfig.nLineSize;                                 // 一行字符数　30-00-00-00(FT#0008)
    int nLineCnt = 0;                                                   // 一行字符数计数　30-00-00-00(FT#0008)
    /*
     * 去除字符串中非ASCII码字符(0x00~0x7F)，去除非汉字字符(低字节A1~FE,高字节B0~F7)
     * 增加支持中文全角字符打印（低字节A0~FF,高字节A1~A3）
     */
    for (ULONG i = 0; i < ulInSize; i++)
    {
        if ((pInData[i] >= 0xB0) && (pInData[i] <= 0xF7)
            && ((pInData[i + 1] >= 0xA1) && (pInData[i + 1] <= 0xFE))
           )
        {
            if ((nLineCnt % PAGELINE == 0 && nLineCnt != 0) ||  // 30-00-00-00(FT#0008)
               (nLineCnt % PAGELINE + 1) == PAGELINE)           // 30-00-00-00(FT#0008)
            {                                                   // 30-00-00-00(FT#0008)
                pOutData[ulOutSize++] = '\n';                   // 30-00-00-00(FT#0008)
                nLineCnt = 0;                                   // 30-00-00-00(FT#0008)
            }                                                   // 30-00-00-00(FT#0008)
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
            nLineCnt = nLineCnt + 2;                            // 30-00-00-00(FT#0008)
        }
        else if ((pInData[i] >= 0xA1) && (pInData[i] <= 0xA3)
                 && ((pInData[i + 1] >= 0xA1) && (pInData[i + 1] <= 0xFF))
                )
        {
            if (0xA1 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xA1 == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            else if (0xA2 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xAB == pInData[i + 1] || 0xAC == pInData[i + 1] ||
                    0xAD == pInData[i + 1] || 0xAE == pInData[i + 1] || 0xAF == pInData[i + 1] ||
                    0xB0 == pInData[i + 1] || 0xEF == pInData[i + 1] || 0xF0 == pInData[i + 1] ||
                    0xFD == pInData[i + 1] || 0xFE == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            else if (0xA3 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            if ((nLineCnt % PAGELINE == 0 && nLineCnt != 0) ||          // 30-00-00-00(FT#0008)
               (nLineCnt % PAGELINE + 1) == PAGELINE)                   // 30-00-00-00(FT#0008)
            {                                                           // 30-00-00-00(FT#0008)
                pOutData[ulOutSize++] = '\n';                           // 30-00-00-00(FT#0008)
                nLineCnt = 0;                                           // 30-00-00-00(FT#0008)
            }                                                           // 30-00-00-00(FT#0008)
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
            nLineCnt = nLineCnt + 2;                                    // 30-00-00-00(FT#0008)
        }
        else if (pInData[i] >= 0x80)
        {
            i++;
        }
        else if ((pInData[i] == 0x0A)
                 || ((pInData[i] > 0x1F) && (pInData[i] < 0x7F)))
        {
            if (nLineCnt % PAGELINE == 0 && nLineCnt != 0)              // 30-00-00-00(FT#0008)
            {                                                           // 30-00-00-00(FT#0008)
                pOutData[ulOutSize++] = '\n';                           // 30-00-00-00(FT#0008)
                nLineCnt = 0;                                           // 30-00-00-00(FT#0008)
            }                                                           // 30-00-00-00(FT#0008)
            if (pInData[i] == '\n')                                     // 30-00-00-00(FT#0008)
            {                                                           // 30-00-00-00(FT#0008)
                nLineCnt = 0;                                           // 30-00-00-00(FT#0008)
            }                                                           // 30-00-00-00(FT#0008)
            pOutData[ulOutSize++] = pInData[i];
            nLineCnt = nLineCnt + 1;                                    // 30-00-00-00(FT#0008)
        }
    }
}

WORD CXFS_PTR::ConvertMediaStatus(OutletStatus eOutletStatus)
{
    switch (eOutletStatus)
    {
    case OUTLET_STATUS_NOMEDIA:    return WFS_PTR_MEDIANOTPRESENT;
    case OUTLET_STATUS_MEDIA:      return WFS_PTR_MEDIAENTERING;
    case OUTLET_STATUS_UNKNOWN:    return WFS_PTR_MEDIAUNKNOWN;
    default:                       return WFS_PTR_MEDIANOTSUPP;
    }
}

WORD CXFS_PTR::ConvertMediaStatus2(PaperStatus ePaperStatus)
{
    switch (ePaperStatus)
    {
        case PAPER_STATUS_NORMAL:
        case PAPER_STATUS_LOW:     return WFS_PTR_MEDIAPRESENT;
        case PAPER_STATUS_JAMMED:  return WFS_PTR_MEDIAJAMMED;
        case PAPER_STATUS_EMPTY:   return WFS_PTR_MEDIANOTPRESENT;
        case PAPER_STATUS_UNKNOWN: return WFS_PTR_MEDIAUNKNOWN;
        default:                   return WFS_PTR_MEDIANOTSUPP;
    }
}

WORD CXFS_PTR::ConvertTonerStatus(TonerStatus eTonerStatus)
{
    switch (eTonerStatus)
    {
    case TONER_STATUS_NORMAL:
    case TONER_STATUS_FULL:    return WFS_PTR_TONERFULL;
    case TONER_STATUS_LOW:     return WFS_PTR_TONERLOW;
    case TONER_STATUS_EMPTY:   return WFS_PTR_TONEROUT;
    case TONER_STATUS_UNKNOWN: return WFS_PTR_TONERUNKNOWN;
    default:                   return WFS_PTR_TONERNOTSUPP;
    }
}

WORD CXFS_PTR::ConvertPaperStatus(PaperStatus ePaperStatus)
{
    switch (ePaperStatus)
    {
    case PAPER_STATUS_NORMAL:  return WFS_PTR_PAPERFULL;
    case PAPER_STATUS_LOW:     return WFS_PTR_PAPERLOW;
    case PAPER_STATUS_JAMMED:  return WFS_PTR_PAPERJAMMED;
    case PAPER_STATUS_EMPTY:   return WFS_PTR_PAPEROUT;
    case PAPER_STATUS_UNKNOWN: return WFS_PTR_PAPERUNKNOWN;
    default:                   return WFS_PTR_PAPERNOTSUPP;
    }
}

void CXFS_PTR::UpdateDeviceStatus(int iRet)
{
    const char* ThisModule = "UpdateDeviceStatus";
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题
    //int     nPrinterStatus  = WFS_PTR_DEVHWERROR;             // 30-00-00-00(FT#0052)

    //BOOL    bNeedFirePrinterStatus  = FALSE;                  // 30-00-00-00(FT#0052)
    BOOL    bNeedFireStatusChanged  = FALSE;                    // 30-00-00-00(FT#0052)
    BOOL    bNeedFirePaperStatus    = FALSE;
    BOOL    bNeedFireTonerStatus    = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFirePaperTaken     = FALSE;

    // 事件上报记录Log
    CHAR szFireBuffer[1024] = { 0x00 };

    PaperStatus ePaperStatus = PAPER_STATUS_UNKNOWN;
    TonerStatus eTonerStatus = TONER_STATUS_UNKNOWN;
    OutletStatus eOutletStatus = OUTLET_STATUS_UNKNOWN;

    m_pPrinter->GetStatus(ePaperStatus, eTonerStatus, eOutletStatus);


    m_sStatus.fwInk                  = WFS_PTR_INKNOTSUPP;
    m_sStatus.fwLamp                 = WFS_PTR_LAMPNOTSUPP;
    m_sStatus.lppRetractBins         = nullptr;
    m_sStatus.usMediaOnStacker       = 0;
    m_sStatus.lpszExtra              = nullptr;


    WFSPTRSTATUS sLastStatus = m_sStatus;
    m_bNeedReset = false;// 30-00-00-00(FT#0032)
    if (!m_bNeedReset)
    {
        // 翻译设备状态
        switch (iRet)
        {
        // 打印机出纸口有纸设备状态为ONLINE
        case ERR_PTR_SUCCESS:
        case ERR_PTR_PARAM_ERR:
        case ERR_PTR_UNSUP_CMD:
            m_sStatus.fwDevice = WFS_PTR_DEVONLINE;
            //nPrinterStatus = WFS_PTR_DEVONLINE;               // 30-00-00-00(FT#0052)
            break;
        case ERR_PTR_COMM_ERR:
        case ERR_PTR_NOT_OPEN:
        case ERR_PTR_NO_DEVICE:
            m_sStatus.fwDevice = WFS_PTR_DEVOFFLINE;
            break;
        default:
            m_sStatus.fwDevice = WFS_PTR_DEVHWERROR;
            break;
        }

        //sStatus.device      = (WORD)nTmpPrinterStatus;
        //bNeedFireHWError       = (PTR_DEV_ONLINE != nPrinterStatus && PTR_DEV_PAPERENTER != nPrinterStatus);
    }
    if (m_sStatus.fwDevice == WFS_PTR_DEVOFFLINE)
    {
        m_sStatus.fwMedia = WFS_PTR_MEDIAUNKNOWN;
        m_sStatus.fwPaper[0] = WFS_PTR_PAPERUNKNOWN;
    }

    if (m_sStatus.fwDevice != sLastStatus.fwDevice)
    {
        //bNeedFirePrinterStatus = TRUE;                        // 30-00-00-00(FT#0052)
        bNeedFireStatusChanged = TRUE;                          // 30-00-00-00(FT#0052)
        //if (nPrinterStatus == WFS_PTR_DEVHWERROR)             // 30-00-00-00(FT#0052)
        if (m_sStatus.fwDevice == WFS_PTR_DEVHWERROR)           // 30-00-00-00(FT#0052)
        {
            bNeedFireHWError = TRUE;
        }
    }

    // **********************介质状态****************************
    if (m_sConfig.nDriverType == DEV_SNBC_BTNH80 && m_sConfig.nTakeSleep > 0 &&
         m_WaitTaken == WTF_TAKEN) {
        DWORD dwSize = time(NULL);
        if (dwSize - dwTakeTimeSize > m_sConfig.nTakeSleep) {
            bNeedFirePaperTaken = TRUE;
            m_WaitTaken = WTF_NONE;
        }
    } else {
        int nMediaStatus = ConvertMediaStatus(eOutletStatus);
        if (m_sStatus.fwMedia != nMediaStatus)
        {
            if ((m_sStatus.fwMedia == WFS_PTR_MEDIAENTERING) &&
                (nMediaStatus == WFS_PTR_MEDIANOTPRESENT))
            {
                bNeedFirePaperTaken = TRUE;
                m_WaitTaken = WTF_NONE;
            }
            m_sStatus.fwMedia = nMediaStatus;
        }
    }

    if (m_sStatus.fwMedia != WFS_PTR_MEDIAENTERING)
    {
        m_sStatus.fwMedia = ConvertMediaStatus2(ePaperStatus);
    }

    if (m_sStatus.fwMedia == WFS_PTR_MEDIAJAMMED)   // 30-00-00-00(FT#0008)
    {                                               // 30-00-00-00(FT#0008)
        m_sStatus.fwDevice = WFS_PTR_DEVHWERROR;    // 30-00-00-00(FT#0008)
        if (m_sStatus.fwDevice != sLastStatus.fwDevice)// 30-00-00-00(FT#0052)
        {                                           // 30-00-00-00(FT#0052)
            bNeedFireStatusChanged = TRUE;          // 30-00-00-00(FT#0052)
            if (m_sStatus.fwDevice == WFS_PTR_DEVHWERROR)// 30-00-00-00(FT#0052)
            {                                       // 30-00-00-00(FT#0052)
                bNeedFireHWError = TRUE;            // 30-00-00-00(FT#0052)
            }                                       // 30-00-00-00(FT#0052)
        }                                           // 30-00-00-00(FT#0052)
    }                                               // 30-00-00-00(FT#0008)

    // **********************纸状态****************************
    int nPaperStatus = ConvertPaperStatus(ePaperStatus);

    if (m_sStatus.fwPaper[0] != (WORD)nPaperStatus)
    {
        m_sStatus.fwPaper[0] = (WORD)nPaperStatus;
        //只有当纸状态变为少或空时才Fire状态
        if (WFS_PTR_PAPERLOW == nPaperStatus || WFS_PTR_PAPEROUT == nPaperStatus || nPaperStatus == WFS_PTR_PAPERFULL)// 30-00-00-00(FT#0052)
        {
            bNeedFirePaperStatus = TRUE;
        }
    }
    for (int i = 1; i < WFS_PTR_SUPPLYSIZE; i++)
    {
        m_sStatus.fwPaper[i] = WFS_PTR_PAPERNOTSUPP;
    }

    // **********************色带状态****************************
    WORD nTonerStatus = ConvertTonerStatus(eTonerStatus);
    if (m_sStatus.fwToner != (WORD)nTonerStatus)
    {
        //只有当Toner状态变为少或空时才Fire状态
        if (nTonerStatus == WFS_PTR_TONERLOW || nTonerStatus == WFS_PTR_TONEROUT)
        {
            bNeedFireTonerStatus = TRUE;
        }
        m_sStatus.fwToner = (WORD)nTonerStatus;
    }


    if (IsJournalPrinter())
    {
        //        static char buf[100];
        //        COfflineJournalPrinter *pPrinter = (COfflineJournalPrinter *)m_pPrinter;
        //        pPrinter->GetActualStatus(&nPrinterStatus, &nPaperStatus, &nTonerStatus);
        //        sprintf(buf, "DEV=%d%cPAPER=%d%c%c", nPrinterStatus, '\0',nPaperStatus,'\0','\0');
        //        m_sStatus.extra = buf;
    }


    if (bNeedFireHWError)
    {
        FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%d|",                  // 30-00-00-00(FT#0052)
                WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);                         // 30-00-00-00(FT#0052)
    }

    //if (bNeedFirePrinterStatus)                                                       // 30-00-00-00(FT#0052)
    if (bNeedFireStatusChanged)                                                         // 30-00-00-00(FT#0052)
    {
        FireStatusChanged(m_sStatus.fwDevice);
        sprintf(szFireBuffer + strlen(szFireBuffer), "StatusChange:%d|",  m_sStatus.fwDevice);// 30-00-00-00(FT#0052)
    }

    if (bNeedFirePaperStatus)
    {
        FirePaperThreshold(WFS_PTR_PAPERUPPER, m_sStatus.fwPaper[0]);
        sprintf(szFireBuffer + strlen(szFireBuffer), "PaperThreshold:%d|",  m_sStatus.fwPaper[0]);// 30-00-00-00(FT#0052)
    }

    if (bNeedFireTonerStatus)
    {
        FireTonerThreshold(m_sStatus.fwToner);
        sprintf(szFireBuffer + strlen(szFireBuffer), "TonerThreshold:%d|",  m_sStatus.fwToner);// 30-00-00-00(FT#0052)
    }

    if (bNeedFirePaperTaken)
    {
        FireMediaTaken();
        Log(ThisModule, 1, IDS_INFO_PAPER_TAKEN);
        sprintf(szFireBuffer + strlen(szFireBuffer), "PaperTaken:|");                   // 30-00-00-00(FT#0052)
    }

    // 比较两次状态记录LOG                                                                          // 30-00-00-00(FT#0052)
    if (memcmp(&sLastStatus, &m_sStatus, sizeof(WFSPTRSTATUS)) != 0)                             // 30-00-00-00(FT#0052)
    {                                                                                            // 30-00-00-00(FT#0052)
        Log(ThisModule, -1, "状态结果比较: Device:%d->%d%s|Media:%d->%d%s|Paper[0]:%d->%d%s|"      // 30-00-00-00(FT#0052)
                            "Ink:%d->%d%s|Toner:%d->%d%s|Lamp:%d->%d%s|; 事件上报记录: %s.",                        // 30-00-00-00(FT#0052)
            sLastStatus.fwDevice, m_sStatus.fwDevice, (sLastStatus.fwDevice != m_sStatus.fwDevice ? " *" : ""), // 30-00-00-00(FT#0052)
            sLastStatus.fwMedia, m_sStatus.fwMedia, (sLastStatus.fwMedia != m_sStatus.fwMedia ? " *" : ""),     // 30-00-00-00(FT#0052)
            sLastStatus.fwPaper[0], m_sStatus.fwPaper[0], (sLastStatus.fwPaper[0] != m_sStatus.fwPaper[0] ? " *" : ""),// 30-00-00-00(FT#0052)
            sLastStatus.fwInk, m_sStatus.fwInk, (sLastStatus.fwInk != m_sStatus.fwInk ? " *" : ""),             // 30-00-00-00(FT#0052)
            sLastStatus.fwToner, m_sStatus.fwToner, (sLastStatus.fwToner != m_sStatus.fwToner ? " *" : ""),     // 30-00-00-00(FT#0052)
            sLastStatus.fwLamp, m_sStatus.fwLamp, (sLastStatus.fwLamp != m_sStatus.fwLamp ? " *" : ""),         // 30-00-00-00(FT#0052)
            szFireBuffer);                                                                                      // 30-00-00-00(FT#0052)
    }                                                                                                           // 30-00-00-00(FT#0052)

    return;
}

bool CXFS_PTR::IsJournalPrinter()
{
    return false;
}
