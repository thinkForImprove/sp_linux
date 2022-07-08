/***************************************************************
* 文件名称：XFS_JPR.cpp
* 文件描述：流水打印模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月14日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_JPR.h"


CXFS_JPR::CXFS_JPR()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    m_nReConErr = PTR_SUCCESS;
    m_bCmdRuning = FALSE;            // 是否命令执行中:F
}

CXFS_JPR::~CXFS_JPR()
{

}


long CXFS_JPR::StartRun()
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

HRESULT CXFS_JPR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    InitPTRData(lpLogicalName);
    InitConifig();
    InitStatus();
    InitCaps();

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = StartOpen();
    if (m_stConfig.nOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

HRESULT CXFS_JPR::OnClose()
{
    THISMODULE(__FUNCTION__);

    Log(ThisModule, __LINE__, "JPR SP Close ");
    if (m_pPrinter != nullptr)
    {
        Log(ThisModule, __LINE__, "m_pPrinter->Close()");
        m_pPrinter->Close();
    }
    return WFS_SUCCESS;
}

// 实时状态(循环调用)
HRESULT CXFS_JPR::OnStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    DEVPTRSTATUS stDevStatus;
    int nRet = m_pPrinter->GetStatus(stDevStatus);

    if (nRet == ERR_PTR_NOT_OPEN)
    {
        m_pPrinter->SetData(nullptr, SET_DEV_RECON);        // 设置为断线重连执行状态
        nRet = m_pPrinter->Open(DEVTYPE2STR(m_stConfig.nDeviceType));
        if (nRet < 0)
        {
            if (m_nReConErr != nRet)
            {
                Log(ThisModule, __LINE__, "断线重连中: OpenDev(%d) fail, ErrCode:%d.", m_stConfig.nDeviceType, nRet);
                m_nReConErr = nRet;
            }
        } else
        {
            if (m_nReConErr != nRet)
            {
                Log(ThisModule, __LINE__, "断线重连完成: OpenDev(%d) succ, RetCode:%d.", m_stConfig.nDeviceType, nRet);
                m_nReConErr = nRet;
            }
        }
    }

    UpdateDeviceStatus();

    return WFS_SUCCESS;
}

// Taken事件等待
HRESULT CXFS_JPR::OnWaitTaken()
{
    return WFS_SUCCESS;
}

HRESULT CXFS_JPR::OnCancelAsyncRequest()
{
    return WFS_SUCCESS;
}

HRESULT CXFS_JPR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// PTR类型接口
// INFOR
HRESULT CXFS_JPR::GetStatus(LPWFSPTRSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);

    m_sStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_sStatus;

    if (m_bCmdRuning == TRUE)   // 有命令执行中,Device==BUSY
    {
        lpStatus->fwDevice = WFS_PTR_DEVBUSY;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_JPR::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_sCaps;
    return WFS_SUCCESS;
}

HRESULT CXFS_JPR::GetFormList(LPSTR &lpszFormList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    lpszFormList = (LPSTR)m_pData->GetNames();
    return WFS_SUCCESS;
}

HRESULT CXFS_JPR::GetMediaList(LPSTR &lpszMediaList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    lpszMediaList = (LPSTR)m_pData->GetNames(false);
    return WFS_SUCCESS;
}

HRESULT CXFS_JPR::GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader)
{
    THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    HRESULT hRet;

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
                    Log(ThisModule, __LINE__, "指定FORM名<%s>无效, Return: %d",
                        pForm->GetName(), WFS_ERR_PTR_FORMINVALID);
                    return WFS_ERR_PTR_FORMINVALID;
                }
            }
            hRet = WFS_SUCCESS;
        } else
        {
            hRet = WFS_ERR_PTR_FORMINVALID;
        }
    } else
    {
        hRet = WFS_ERR_PTR_FORMNOTFOUND;
    }

    lpFrmHeader = &m_pData->m_LastForm;
    return hRet;
}

HRESULT CXFS_JPR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
{
    //THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    HRESULT hRet;

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

            hRet = WFS_SUCCESS;
        } else
        {
            hRet = WFS_ERR_PTR_MEDIAINVALID;
        }
    }
    else
    {
        hRet = WFS_ERR_PTR_MEDIANOTFOUND;
    }

    lpFrmMedia = &m_pData->m_LastMedia;
    return hRet;

}

HRESULT CXFS_JPR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
{
    THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    HRESULT hRet = WFS_SUCCESS;
    m_pData->LoadForms();

    hRet = m_pData->FindField(lpQueryField->lpszFormName, lpQueryField->lpszFieldName);
    if (hRet == WFS_SUCCESS)
    {
        if (m_pData->m_LastFields.GetCount() == 0)
        {
            hRet = WFS_ERR_PTR_FIELDNOTFOUND;
        } else
        {
            hRet = WFS_SUCCESS;

        }
    }

    lpszMediaList = m_pData->m_LastFields;

    return WFS_SUCCESS;
}

// EXECUTE
HRESULT CXFS_JPR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_JPR::PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut)
{    
    THISMODULE(__FUNCTION__);

    DEV_STAT_RET_HWERR(m_sStatus.fwDevice);
    PAPER_OUT_RET(m_sStatus.fwPaper[0]);

    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    m_pData->LoadMedias();
    LPWFSPTRPRINTFORM pIn = (LPWFSPTRPRINTFORM)lpPrintForm;
    m_bCmdRuning = TRUE;                // 命令执行中
    HRESULT hRet = InnerPrintForm(pIn);
    m_bCmdRuning = FALSE;               // 非命令执行中
    return hRet;
}

HRESULT CXFS_JPR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{
    //THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_JPR::RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    DEV_STAT_RET_HWERR(m_sStatus.fwDevice);
    PAPER_OUT_RET(m_sStatus.fwPaper[0]);

    LPWFSPTRRAWDATA pIn = (LPWFSPTRRAWDATA)lpRawData;
    m_bCmdRuning = TRUE;                // 命令执行中
    HRESULT hRet = InnerRawData(WFS_PTR_INPUTDATA == pIn->wInputData, pIn->ulSize, pIn->lpbData);
    if (hRet == WFS_SUCCESS)
    {
        lpRawDataIn = (LPWFSPTRRAWDATAIN)m_pData->m_InputRawData;
    }
    m_bCmdRuning = FALSE;               // 非命令执行中
    return hRet;
}

HRESULT CXFS_JPR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    //THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_JPR::ResetCount(const LPUSHORT lpusBinNum)
{
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_JPR::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_JPR::Reset(const LPWFSPTRRESET lpReset)
{
    LPWFSPTRRESET pIn = (LPWFSPTRRESET)lpReset;
    long hRes = InnerReset(nullptr != pIn ? pIn->dwMediaControl : 0,
                           nullptr != pIn ? pIn->usRetractBinNumber : 0);
    return hRes;
}

HRESULT CXFS_JPR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_JPR::DispensePaper(const LPWORD lpPaperSource)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

// Fire消息
void CXFS_JPR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_JPR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_JPR::FireNoMedia(LPCSTR szPrompt)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}
void CXFS_JPR::FireMediaInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

void CXFS_JPR::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

void CXFS_JPR::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}

void CXFS_JPR::FireRetractBinThreshold(USHORT BinNumber, WORD wStatus)
{
    WFSPTRBINTHRESHOLD BinThreshold;
    BinThreshold.usBinNumber     = BinNumber;
    BinThreshold.wRetractBin = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_RETRACTBINTHRESHOLD, &BinThreshold);
}

void CXFS_JPR::FireMediaTaken()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}

void CXFS_JPR::FirePaperThreshold(WORD wSrc, WORD wStatus)
{
    WFSPTRPAPERTHRESHOLD PaperThreshold;
    PaperThreshold.wPaperSource     = wSrc;
    PaperThreshold.wPaperThreshold  = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_PAPERTHRESHOLD, &PaperThreshold);
}

void CXFS_JPR::FireTonerThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_TONERTHRESHOLD, &wStatus);
}

void CXFS_JPR::FireInkThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}

void CXFS_JPR::FireLampThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}

void CXFS_JPR::FireSRVMediaInserted()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIAINSERTED, nullptr);
}

void CXFS_JPR::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}
