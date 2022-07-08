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
    m_nReConErr = PTR_SUCCESS;
    m_bBusy = FALSE;    // 30-00-00-00(FT#0066
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

    CHAR szFWVersion[64] = { 0x00 };

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

    Log(ThisModule, __LINE__, "PRT SP Close ");
    if (m_pPrinter != nullptr)
    {
        Log(ThisModule, __LINE__, "m_pPrinter->Close()");
        m_pPrinter->Close();
    }
    return WFS_SUCCESS;
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
        m_pPrinter->SetData(nullptr, SET_DEV_RECON);        // 设置为断线重连执行状态
        nRet = m_pPrinter->Open(DEVTYPE2STR(m_sConfig.nDriverType));// 30-00-00-00(FT#0015)
        if (nRet < 0)
        {
            if (m_nReConErr != nRet)
            {
                Log(ThisModule, __LINE__, "断线重连中: OpenDev(%d) fail, ErrCode:%d.", m_sConfig.nDriverType, nRet);// 30-00-00-00(FT#0015)
                m_nReConErr = nRet;
            }
        } else
        {
            if (m_nReConErr != nRet)
            {
                Log(ThisModule, __LINE__, "断线重连完成: OpenDev(%d) succ, RetCode:%d.", m_sConfig.nDriverType, nRet);// 30-00-00-00(FT#0015)
                m_nReConErr = nRet;
            }
        }
        stDevStatus.Clear();
        nRet = m_pPrinter->GetStatus(stDevStatus);					// 30-00-00-00(FT#0015)
    }

    UpdateDeviceStatus(nRet);

    return WFS_SUCCESS;
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
    INT nRet = m_pPrinter->GetStatus(stDevStatus);
    UpdateDeviceStatus(nRet);
    return WFS_SUCCESS;
}

HRESULT CXFS_PTR::OnCancelAsyncRequest()
{
    return WFS_SUCCESS;
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
    if(m_bBusy && m_sStatus.fwDevice == DEV_STAT_ONLINE)    // 30-00-00-00(FT#0066)
    {                                                       // 30-00-00-00(FT#0066)
        m_sStatus.fwDevice = DEV_STAT_BUSY;                 // 30-00-00-00(FT#0066)
    }                                                       // 30-00-00-00(FT#0066)
    lpStatus = &m_sStatus;
    return WFS_SUCCESS;
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
        }
        else
        {
            hRet = WFS_ERR_PTR_FORMINVALID;
        }
    }
    else
    {
        hRet = WFS_ERR_PTR_FORMNOTFOUND;
    }

    lpFrmHeader = &m_pData->m_LastForm;
    return hRet;
}

HRESULT CXFS_PTR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
{
    THISMODULE(__FUNCTION__);
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
        }
        else
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

HRESULT CXFS_PTR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
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
        }
        else
        {
            hRet = WFS_SUCCESS;

        }
    }

    lpszMediaList = m_pData->m_LastFields;
    return WFS_SUCCESS;
}

// EXECUTE
HRESULT CXFS_PTR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    return InnerMediaControl(*lpdwMeidaControl);
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
    m_bBusy = TRUE;         // 30-00-00-00(FT#0066)
    LPWFSPTRPRINTFORM pIn = (LPWFSPTRPRINTFORM)lpPrintForm;
    HRESULT hRet = InnerPrintForm(pIn);
    m_bBusy = FALSE;        // 30-00-00-00(FT#0066
    return hRet;
}

HRESULT CXFS_PTR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{
    //THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
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

    m_bBusy = TRUE;     // 30-00-00-00(FT#0066)
    HRESULT hRet = InnerRawData(WFS_PTR_INPUTDATA == pIn->wInputData, pIn->ulSize, pIn->lpbData);
    if (hRet == WFS_SUCCESS)
    {
        lpRawDataIn = (LPWFSPTRRAWDATAIN)m_pData->m_InputRawData;
    }
    m_bBusy = FALSE;        // 30-00-00-00(FT#0066
    return hRet;
}

HRESULT CXFS_PTR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    THISMODULE(__FUNCTION__);
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
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_PTR::Reset(const LPWFSPTRRESET lpReset)
{
    LPWFSPTRRESET pIn = (LPWFSPTRRESET)lpReset;
    HRESULT hRet = InnerReset(pIn != nullptr ? pIn->dwMediaControl : 0,
                              pIn != nullptr ? pIn->usRetractBinNumber : 0);
    return hRet;
}

HRESULT CXFS_PTR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_PTR::DispensePaper(const LPWORD lpPaperSource)
{
    return WFS_ERR_UNSUPP_COMMAND;
}
