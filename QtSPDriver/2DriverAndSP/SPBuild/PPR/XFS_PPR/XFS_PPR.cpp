/***************************************************************
* 文件名称：XFS_PPR.cpp
* 文件描述：存折打印模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_PPR.h"


CXFS_PPR::CXFS_PPR()
{
    SetLogFile(LOG_NAME, ThisFile);     // 设置日志文件名和错误发生的文件
    m_nReConErr = PTR_SUCCESS;
    m_bCmdRuning = FALSE;               // 是否命令执行中:F
    m_bCmdCanceled = FALSE;             // 是否取消吸折
    m_WaitTaken = WTF_NONE;
    m_dwRetractCntEvent = 0;
    memset(m_wNeedFireEvent, 0, sizeof(m_wNeedFireEvent));  // 需要特殊上报的事件
}

CXFS_PPR::~CXFS_PPR()
{

}


long CXFS_PPR::StartRun()
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

HRESULT CXFS_PPR::OnOpen(LPCSTR lpLogicalName)
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

HRESULT CXFS_PPR::OnClose()
{
    THISMODULE(__FUNCTION__);

    Log(ThisModule, __LINE__, "PPR SP Close ");
    m_bCmdCanceled = TRUE;
    usleep(1000 * 500);     // 休眠500毫秒,确保吸折动作取消
    if (m_pPrinter != nullptr)
    {
        Log(ThisModule, __LINE__, "m_pPrinter->Close()");
        m_pPrinter->Close();
    }
    return WFS_SUCCESS;
}

// 实时状态(循环调用)
HRESULT CXFS_PPR::OnStatus()
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
HRESULT CXFS_PPR::OnWaitTaken()
{
    if (m_WaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }

    if (m_WaitTaken == WTF_TAKEN)
    {
        UpdateDeviceStatus();
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::OnCancelAsyncRequest()
{
    m_bCmdCanceled = TRUE;
    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// PTR类型接口
// INFOR
HRESULT CXFS_PPR::GetStatus(LPWFSPTRSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);

    // 回收设置
    if (m_stRetractCfg.nRetractSup == 1)    // 不支持回收
    {
        if (m_stStatus.lppRetractBins == nullptr)    // 重新申请回收箱计数buffer
        {
            m_stStatus.lppRetractBins = new LPWFSPTRRETRACTBINS[m_stRetractCfg.wRetBoxCount + 1];
            memset(m_stStatus.lppRetractBins, 0x00, sizeof(LPWFSPTRRETRACTBINS) * (m_stRetractCfg.wRetBoxCount + 1));
            m_stStatus.lppRetractBins[0] = new WFSPTRRETRACTBINS;
            m_stStatus.lppRetractBins[1] = nullptr;
        }

        m_stStatus.lppRetractBins[0]->usRetractCount = m_stRetractCfg.dwRetractCnt;
        if (m_stRetractCfg.dwRetractCnt <= m_stRetractCfg.dwFullThreshold)  // 将满
        {
            m_stStatus.lppRetractBins[0]->wRetractBin = WFS_PTR_RETRACTBINHIGH;
        }
        if (m_stRetractCfg.dwRetractCnt >= m_stRetractCfg.dwRetractVol)     // 满
        {
            m_stStatus.lppRetractBins[0]->wRetractBin = WFS_PTR_RETRACTBINFULL;
        } else
        {
            m_stStatus.lppRetractBins[0]->wRetractBin = WFS_PTR_RETRACTBINOK;// 正常
        }
    }

    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_stStatus;

    if (m_bCmdRuning == TRUE)   // 有命令执行中,Device==BUSY
    {
        lpStatus->fwDevice = WFS_PTR_DEVBUSY;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_sCaps;
    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetFormList(LPSTR &lpszFormList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    lpszFormList = (LPSTR)m_pData->GetNames();
    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetMediaList(LPSTR &lpszMediaList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    lpszMediaList = (LPSTR)m_pData->GetNames(false);
    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader)
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
                if (pItem == nullptr)
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

HRESULT CXFS_PPR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
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

HRESULT CXFS_PPR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
{
    THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    long hRes = WFS_SUCCESS;
    m_pData->LoadForms();

    hRes = m_pData->FindField(lpQueryField->lpszFormName, lpQueryField->lpszFieldName);
    if (hRes == WFS_SUCCESS)
    {
        if (m_pData->m_LastFields.GetCount() == 0)
        {
            hRes = WFS_ERR_PTR_FIELDNOTFOUND;
        } else
        {
            hRes = WFS_SUCCESS;

        }
    }

    lpszMediaList = m_pData->m_LastFields;

    return WFS_SUCCESS;
}

// EXECUTE
HRESULT CXFS_PPR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    THISMODULE(__FUNCTION__);
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);
    return InnerMediaControl(*lpdwMeidaControl);
}

HRESULT CXFS_PPR::PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut)
{    
    THISMODULE(__FUNCTION__);

    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);
    //PAPER_OUT_RET(m_stStatus.fwPaper[0]);

    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    m_pData->LoadMedias();
    LPWFSPTRPRINTFORM pIn = (LPWFSPTRPRINTFORM)lpPrintForm;
    m_bCmdRuning = TRUE;                // 命令执行中
    HRESULT hRet = InnerPrintForm(pIn, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        RunAutoReset(hRet); // 执行自动复位
    }
    m_bCmdRuning = FALSE;               // 非命令执行中
    return hRet;
}

HRESULT CXFS_PPR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);
    //PAPER_OUT_RET(m_stStatus.fwPaper[0]);

    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    m_pData->LoadMedias();
    LPWFSPTRREADFORM pIn = (LPWFSPTRREADFORM)lpReadForm;
    m_bCmdRuning = TRUE;                // 命令执行中
    HRESULT hRet = InnerReadForm(pIn, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        RunAutoReset(hRet); // 执行自动复位
    }
    lpReadFormOut = m_pData->m_ReadFormOut;
    m_bCmdRuning = FALSE;               // 非命令执行中
    return hRet;
}

HRESULT CXFS_PPR::RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);
    PAPER_OUT_RET(m_stStatus.fwPaper[0]);

    LPWFSPTRRAWDATA pIn = (LPWFSPTRRAWDATA)lpRawData;
    m_bCmdRuning = TRUE;                // 命令执行中
    HRESULT hRet = InnerReadRawData(pIn->wInputData == WFS_PTR_INPUTDATA, pIn->ulSize, pIn->lpbData, dwTimeOut);
    if (hRet == WFS_SUCCESS)
    {
        lpRawDataIn = (LPWFSPTRRAWDATAIN)m_pData->m_InputRawData;
    } else
    {
        RunAutoReset(hRet); // 执行自动复位
    }
    m_bCmdRuning = FALSE;               // 非命令执行中
    return hRet;
}

HRESULT CXFS_PPR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    //THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_PPR::ResetCount(const LPUSHORT lpusBinNum)
{
    THISMODULE(__FUNCTION__);

    // 回收计数清零
    if (m_stRetractCfg.nRetractSup == 1)    // 支持回收
    {
        m_stRetractCfg.dwRetractCnt = 0;
        m_cXfsReg.SetValue("RETRACT_CFG", "RetractCount", "0");
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    HRESULT hRet = WFS_SUCCESS;

    if (m_stConfig.nDeviceType == DEV_MB2)
    {
        m_bCmdRuning = TRUE;                // 命令执行中
        hRet = InnerReadImage(lpImgRequest, lppImage, dwTimeOut);
        m_bCmdRuning = FALSE;               // 非命令执行中
    } else
    {
        Log(ThisModule, __LINE__, "扫描: 设备[%d]不支持. Return: %d.",
            m_stConfig.nDeviceType, WFS_ERR_UNSUPP_COMMAND);
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return hRet;
}

HRESULT CXFS_PPR::Reset(const LPWFSPTRRESET lpReset)
{
    LPWFSPTRRESET pIn = (LPWFSPTRRESET)lpReset;
    HRESULT hRet = InnerReset(pIn != nullptr ? pIn->dwMediaControl : 0,
                              pIn != nullptr ? pIn->usRetractBinNumber : 0);
    return hRet;
}

// 介质回收
HRESULT CXFS_PPR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;

    if (m_stRetractCfg.nRetractSup == 1)    // 支持回收
    {
        hRet = InnerMediaControl(WFS_PTR_CTRLRETRACT);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "介质回收: ->InnerMediaControl(%d) Fail, Return: %d.",
               WFS_PTR_CTRLRETRACT, hRet);
            return hRet;
        }
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
}

HRESULT CXFS_PPR::DispensePaper(const LPWORD lpPaperSource)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

