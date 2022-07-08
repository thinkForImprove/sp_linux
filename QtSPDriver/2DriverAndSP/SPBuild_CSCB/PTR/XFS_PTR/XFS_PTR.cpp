/***************************************************************
* 文件名称：XFS_PTR.cpp
* 文件描述：凭条打印模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_PTR.h"


CXFS_PTR::CXFS_PTR()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
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
    char szDevRPRVer[64] = { 0x00 };
    char szFWVersion[64] = { 0x00 };

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

    return StartOpen();
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

    DEVPTRSTATUS stDevStatus;
    int nRet = m_pPrinter->GetStatus(stDevStatus);

    if (nRet == ERR_PTR_NOT_OPEN)
    {
        //iRet = m_pPrinter->Open(nullptr);					// 30-00-00-00(FT#0015)
        nRet = m_pPrinter->Open(DEVTYPE2STR(m_sConfig.nDriverType));// 30-00-00-00(FT#0015)
        if (nRet < 0)
        {
            Log(ThisModule, -1, "Auto OpenDev(%d) fail, ErrCode:%d", m_sConfig.nDriverType, nRet);// 30-00-00-00(FT#0015)
        }
        stDevStatus.Clear();
        nRet = m_pPrinter->GetStatus(stDevStatus);					// 30-00-00-00(FT#0015)
    }

    UpdateDeviceStatus(nRet);

    return 0;
}

HRESULT CXFS_PTR::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (m_WaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }

    DEVPTRSTATUS stDevStatus;
    int nRet = m_pPrinter->GetStatus(stDevStatus);
    UpdateDeviceStatus(nRet);
    return WFS_SUCCESS;
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
    long hRes = ResetDevice(nullptr != pIn ? pIn->dwMediaControl : 0,
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
