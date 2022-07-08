/***************************************************************
* 文件名称：XFS_CSR.cpp
* 文件描述：票据受理模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_CSR.h"
#include "string.h"
#include "QtTypeInclude.h"
#include <QTextCodec>
#include <stdlib.h>
#include <unistd.h>

// PTR SP 版本号
BYTE    byVRTU[17] = {"HWCSRSTE00000100"};

// 事件日志
static const char *ThisFile = "XFS_CSR.cpp";

#define LOG_NAME     "XFS_CSR.log"


//-----------------------------------------------------------------------------------
//-------------------------------------构造/析构--------------------------------------
CXFS_CSR::CXFS_CSR()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    m_WaitTaken = WTF_NONE;
    m_bNeedReset = true;
    m_bCancelFlag = FALSE;
    m_nReConRet = PTR_SUCCESS;
    memset(m_stWFSRetractBin, 0x00, sizeof(WFSPTRRETRACTBINS) * 16);
    memset(m_stWFSRetractBinOLD, 0x00, sizeof(WFSPTRRETRACTBINS) * 16);
    m_bInsertEventRep = FALSE;
}

CXFS_CSR::~CXFS_CSR()
{

}

//-----------------------------------------------------------------------------------
// 开始运行SP
long CXFS_CSR::StartRun()
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

//-----------------------------------------------------------------------------------
//--------------------------------------基本接口---------------------------------------
// Open设备及初始化相关
HRESULT CXFS_CSR::OnOpen(LPCSTR lpLogicalName)
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

    // 设备驱动动态库验证
    if (strlen(m_stConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败.", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 加载设备驱动动态库
    if (m_pPrinter == nullptr)
    {
        if (m_pPrinter.Load(m_stConfig.szDevDllName, "CreateIDevPTR", DEVTYPE_CHG(m_stConfig.nDriverType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DEVTYPE=%d|%s. ReturnCode:%s.",
                m_stConfig.szDevDllName, m_stConfig.nDriverType, DEVTYPE_CHG(m_stConfig.nDriverType),
                m_pPrinter.LastError().toUtf8().constData());
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = StartOpen();
    if (m_stConfig.nOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

// 关闭设备
HRESULT CXFS_CSR::OnClose()
{
    THISMODULE(__FUNCTION__);

    if (m_pPrinter != nullptr)
    {
        Log(ThisModule, 1, "关闭设备连接成功.");
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

// 实时状态更新
HRESULT CXFS_CSR::OnStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    UpdateDeviceStatus();

    // 断线后进行重连
    if (m_stStatus.fwDevice == WFS_PTR_DEVOFFLINE)
    {
        StartOpen(TRUE);
        UpdateDeviceStatus();
    }

    return WFS_SUCCESS;
}

// Taken事件处理
HRESULT CXFS_CSR::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (m_WaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }

    UpdateDeviceStatus();

    return WFS_SUCCESS;
}

// 命令取消
HRESULT CXFS_CSR::OnCancelAsyncRequest()
{
    m_bCancelFlag = TRUE;
    return WFS_SUCCESS;
}

// 固件升级
HRESULT CXFS_CSR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}


//-----------------------------------------------------------------------------------
//-------------------------------重载CSPBaseClass的方法--------------------------------
HRESULT CXFS_CSR::OnInit()
{
    THISMODULE(__FUNCTION__);

    INT nRet = m_pPrinter->Init();
    //UpdateDeviceStatus(nRet);
    if (nRet < 0)
    {
        return ConvertErrCode(nRet);
    }
    return ConvertErrCode(nRet);
}

HRESULT CXFS_CSR::OnExit()
{
    if (nullptr != m_pPrinter)
    {
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//----------------------------------PTR类型接口(INFO)----------------------------------
HRESULT CXFS_CSR::GetStatus(LPWFSPTRSTATUS &lpStatus)
{
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_stStatus;

    // 回收箱统计: 状态 + 计数
    if (m_stStatus.fwDevice == WFS_PTR_DEVBUSY || m_stStatus.fwDevice == WFS_PTR_DEVONLINE)
    {
        INT nRetractBins = 0;
        if ((nRetractBins = m_stNoteBoxList.GetBoxCount()) > 0) // 根据INI获取回收箱列表去的箱数
        {
            // status保存箱信息是否已申请空间并获取已申请数
            USHORT usBinCount = 0;
            if (m_stStatus.lppRetractBins != nullptr)
            {
                while (true)
                {
                    if (m_stStatus.lppRetractBins[usBinCount] == nullptr)
                        break;
                    usBinCount++;
                }
            }

            // status保存箱信息是否已申请空间数与箱数不一致,则释放重新申请空间
            if (nRetractBins != usBinCount)
            {
                delete [] m_stStatus.lppRetractBins;
                m_stStatus.lppRetractBins = nullptr;

                m_stStatus.lppRetractBins = new LPWFSPTRRETRACTBINS[nRetractBins + 1];
                memset(m_stStatus.lppRetractBins, 0x00, sizeof(LPWFSPTRRETRACTBINS) * (nRetractBins + 1));
                for (INT i = 0; i < nRetractBins; i ++)
                {
                    m_stStatus.lppRetractBins[i] = new WFSPTRRETRACTBINS;
                }
                m_stStatus.lppRetractBins[nRetractBins] = nullptr;
            }

            // 填充回收箱信息
            if (m_stStatus.lppRetractBins != nullptr)
            {
                for (INT i = 0; i < nRetractBins; i ++)
                {
                    m_stStatus.lppRetractBins[i]->wRetractBin = m_stWFSRetractBin[i].wRetractBin;
                    m_stStatus.lppRetractBins[i]->usRetractCount = m_stWFSRetractBin[i].usRetractCount;
                }
            }
        }
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_sCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::GetFormList(LPSTR &lpszFormList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    lpszFormList = (LPSTR)m_pData->GetNames();

    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::GetMediaList(LPSTR &lpszMediaList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    lpszMediaList = (LPSTR)m_pData->GetNames(false);

    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader)
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

HRESULT CXFS_CSR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
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

HRESULT CXFS_CSR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
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

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//---------------------------------PTR类型接口(EXECUTE)-------------------------------
// 介质控制
HRESULT CXFS_CSR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    THISMODULE(__FUNCTION__);

    // Check: 状态
    UpdateDeviceStatus();   // 取当前最新状态

    // 设备Check
    CHECK_DEVICE();

    // 通道内无票
    if (m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT) \
    {
        Log(ThisModule, -1, "Media Status = %d[MEDIANOTPRESENT)], 通道内没有票据存在, Return: %d.", \
            WFS_PTR_MEDIAPRESENT, WFS_ERR_PTR_NOMEDIAPRESENT); \
        return WFS_ERR_PTR_NOMEDIAPRESENT;
    }

    return ControlMedia(*lpdwMeidaControl);
}

// 格式化打印
HRESULT CXFS_CSR::PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;

    m_bCancelFlag = FALSE;

    // Check: 状态
    UpdateDeviceStatus();   // 取当前最新状态
    // 设备Check
    CHECK_DEVICE();

    // Check: 入参
    if (lpPrintForm == nullptr)
    {
     Log(ThisModule, -1, "入参 Is NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
     return WFS_ERR_UNSUPP_DATA;
    }

    // 下发允许入票命令等待
    m_bInsertEventRep = TRUE;
    hRet = MediaInsertWait(dwTimeOut);
    if (hRet == WFS_SUCCESS)
    {
        // 设置边界
        SIZE tmpSize = GetTwipsPerRowCol();
        m_pData->SetTwipsPerRowCol(&tmpSize);
        // 加载Form文件
        m_pData->LoadForms();
        // 加载Media文件
        m_pData->LoadMedias();
        LPWFSPTRPRINTFORM pIn = (LPWFSPTRPRINTFORM)lpPrintForm;

        hRet = InnerPrintForm(pIn);
    }
    m_bInsertEventRep = FALSE;

    return hRet;
}

// 格式化读
HRESULT CXFS_CSR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{    
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

// 无格式打印
HRESULT CXFS_CSR::RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    /*LPWFSPTRRAWDATA pIn = (LPWFSPTRRAWDATA)lpRawData;
    long hRes = SendRawData(WFS_PTR_INPUTDATA == pIn->wInputData, pIn->ulSize, pIn->lpbData);
    if (WFS_SUCCESS == hRes)
    {
        lpRawDataIn = (LPWFSPTRRAWDATAIN)m_pData->m_InputRawData;
    }
    return hRes;*/
    return WFS_ERR_UNSUPP_COMMAND;
}

// 获得插入物理设备中的媒介的长宽度
HRESULT CXFS_CSR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

// 将媒介回收计数由当前值归零
HRESULT CXFS_CSR::ResetCount(const LPUSHORT lpusBinNum)
{
    THISMODULE(__FUNCTION__);
    WORD wRetractBox = 0;

    // 获取RSC-D400M设备INI设置(0右票箱/1左票箱)
    if (m_stConfig.nDriverType == IXFSCSR_TYPE_RSCD400M)
    {
        if (*lpusBinNum == m_stConfig.stCfg_RSCD400M.usLeftNoteBoxNo)      // 左票箱
        {
            wRetractBox = 1;
        } else
        if (*lpusBinNum == m_stConfig.stCfg_RSCD400M.usRightNoteBoxNo)     // 右票箱
        {
            wRetractBox = 0;
        } else
        {
            Log(ThisModule, __LINE__, "->入参(lpusBinNum = %d) 无效, Return: %d.",
                MEDIA_CTR_RETRACT, wRetractBox, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    } else
    {
        Log(ThisModule, __LINE__, "->INI指定设备类型[%d]不支持, Return: %d.",
            m_stConfig.nDriverType, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 统计: 票据计数: 0,写入INI记录
    SetRetractBoxCount(*lpusBinNum, 0, TRUE);

    return WFS_SUCCESS;
}

// 获取图象数据
HRESULT CXFS_CSR::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;

    m_bCancelFlag = FALSE;

    // Check: 状态
    UpdateDeviceStatus();   // 取当前最新状态
    // 设备Check
    //CHECK_DEVICE();
    if (m_stConfig.usBank == BANK_NO_CSBC) // 长沙银行
    {
        if (m_stStatus.fwDevice != WFS_PTR_DEVONLINE && m_stStatus.fwDevice != WFS_PTR_DEVBUSY)
        {
            Log(ThisModule, -1, "Device != ONLINE|BUSY, Return: %d.", WFS_ERR_PTR_MEDIAINVALID);
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    } else
    {
        CHECK_DEVICE();
    }

    // Check: 入参
    if (lpImgRequest == nullptr)
    {
        Log(ThisModule, -1, "入参 Is NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (lpImgRequest->fwImageSource == 0)
    {
        Log(ThisModule, -1, "入参 lpImgRequest->fwImageSource == 0, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 下发允许入票命令等待
    m_bInsertEventRep = TRUE;
    hRet = MediaInsertWait(dwTimeOut);
    if (hRet == WFS_SUCCESS)
    {
        hRet = InnerReadImage(lpImgRequest, lppImage, dwTimeOut);
    }
    m_bInsertEventRep = FALSE;

    if (m_stConfig.usBank == BANK_NO_CSBC) // 长沙银行
    {
        if (hRet == WFS_ERR_HARDWARE_ERROR) // HWERR转为MediaInvalid
        {
            hRet = WFS_ERR_PTR_MEDIAINVALID;
        }
    }

    return hRet;
}

// 复位
HRESULT CXFS_CSR::Reset(const LPWFSPTRRESET lpReset)
{
    THISMODULE(__FUNCTION__);

    LPWFSPTRRESET pIn = (LPWFSPTRRESET)lpReset;
    INT nRet = PTR_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;


    // WFS_PTR_CTRLEJECT 退出媒介
    // WFS_PTR_CTRLRETRACT 根据usRetractBinNumber 规定将媒介回收入回收盒
    // WFS_PTR_CTRLEXPEL 从出口抛出媒介
    WORD wCheckCmd = WFS_PTR_CTRLEJECT | WFS_PTR_CTRLRETRACT | WFS_PTR_CTRLEXPEL;

    // Check: 状态
    UpdateDeviceStatus();   // 取当前最新状态
    // 设备Check
    CHECK_DEVICE();

    // 出口|通道内有票,执行Eject/Retract
    if (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT || m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING)
    {
        if ((pIn->dwMediaControl & WFS_PTR_CTRLEXPEL) == WFS_PTR_CTRLEXPEL)
        {
            Log(ThisModule, nRet, "Input dwMediaControl[%d] Is NoSupp, Return: %d.", WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        if ((pIn->dwMediaControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT &&
            (pIn->dwMediaControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)
        {
            Log(ThisModule, nRet, "Input dwMediaControl[%d] Is Invalid, 参数冲突, Return: %d.", pIn->dwMediaControl, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        if (pIn->dwMediaControl == WFS_PTR_CTRLEJECT)
        {
            if ((hRet = ControlMedia(pIn->dwMediaControl)) != WFS_SUCCESS)
            {
                Log(ThisModule, nRet, "Input dwMediaControl[%d] Is EjectMedia: ->ControlMedia(%d) Is Fail, Return: %d.",
                    pIn->dwMediaControl, pIn->dwMediaControl, hRet);
                return hRet;
            }
        } else
        if (pIn->dwMediaControl == WFS_PTR_CTRLRETRACT)
        {
            LPUSHORT usOut = 0;
            hRet = RetractMedia((const LPUSHORT)(pIn->usRetractBinNumber), usOut);
            if (hRet != WFS_SUCCESS && hRet != WFS_ERR_PTR_NOMEDIAPRESENT)
            {
                Log(ThisModule, nRet, "Input dwMediaControl[%d] Is RetractMedia: ->RetractMedia(%d, %d) Is Fail, Return: %d.",
                    pIn->dwMediaControl, pIn->usRetractBinNumber, usOut, hRet);
                return hRet;
            }
        }
    }

    // 执行复位
    nRet = m_pPrinter->Reset();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, nRet, "->Reset() Fail, ErrCode = %d, Return: %d.",
            nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    return WFS_SUCCESS;
}

// 媒介回收
HRESULT CXFS_CSR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    THISMODULE(__FUNCTION__);

    // 票据回收前CHECK
    UpdateDeviceStatus();   // 取最新状态

    // 设备Ckeck
    CHECK_DEVICE();

    // 通道内无票
    if (m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT) \
    {
        Log(ThisModule, -1, "Media Status = %d[MEDIANOTPRESENT)], 通道内没有票据存在, Return: %d.", \
            WFS_PTR_MEDIAPRESENT, WFS_ERR_PTR_NOMEDIAPRESENT); \
        return WFS_ERR_PTR_NOMEDIAPRESENT;
    }

    // Full状态检查
    if (m_stNoteBoxList.GetBoxStat(*lpusBinNum) == BOX_FULL ||
        m_stNoteBoxList.GetRetraceFull(*lpusBinNum) == BOX_FULL)
    {
        Log(ThisModule, -1, "票据数已达INI设置Full阀值或物理箱满, 无法执行回收, Return: %d.",
            WFS_PTR_MEDIAPRESENT, WFS_ERR_PTR_RETRACTBINFULL);
        FireRetractBinThreshold(*lpusBinNum, WFS_PTR_RETRACTBINFULL);    // 上报回收箱满事件
        return WFS_ERR_PTR_RETRACTBINFULL;
    }

    return MediaRetract(lpusBinNum, lpusBinNumOut);
}

// 纸张移动
HRESULT CXFS_CSR::DispensePaper(const LPWORD lpPaperSource)
{
    THISMODULE(__FUNCTION__);

    return WFS_ERR_UNSUPP_COMMAND;
}

// 指示灯控制
HRESULT CXFS_CSR::SetGuidanceLight(const LPWFSPTRSETGUIDLIGHT lpSetGuidLight)
{
    return WFS_ERR_UNSUPP_COMMAND;
}


//-----------------------------------------------------------------------------------
//--------------------------------------事件消息---------------------------------------
// 上报Device HWERR事件
void CXFS_CSR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

// 上报状态变化事件
void CXFS_CSR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

// 上报无媒介事件
void CXFS_CSR::FireNoMedia(LPCSTR szPrompt)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}

// 上报媒介放入事件
void CXFS_CSR::FireMediaInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

// 上报Field错误事件
void CXFS_CSR::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

// 上报Field警告事件
void CXFS_CSR::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}

// 上报回收箱变化事件
void CXFS_CSR::FireRetractBinThreshold(USHORT BinNumber, WORD wStatus)
{
    WFSPTRBINTHRESHOLD BinThreshold;
    BinThreshold.usBinNumber     = BinNumber;
    BinThreshold.wRetractBin = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_RETRACTBINTHRESHOLD, &BinThreshold);
}

// 上报媒介取走事件
void CXFS_CSR::FireMediaTaken()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}

// 上报纸状态/票箱状态变化事件
void CXFS_CSR::FirePaperThreshold(WORD wSrc, WORD wStatus)
{
    WFSPTRPAPERTHRESHOLD PaperThreshold;
    PaperThreshold.wPaperSource     = wSrc;
    PaperThreshold.wPaperThreshold  = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_PAPERTHRESHOLD, &PaperThreshold);
}

// 上报碳带状态变化事件
void CXFS_CSR::FireTonerThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_TONERTHRESHOLD, &wStatus);
}

// 上报墨盒状态变化事件
void CXFS_CSR::FireInkThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}

// 上报灯状态变化事件
void CXFS_CSR::FireLampThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}

// 本事件指出在没有任何打印执行命令执行的情况下物理媒体已插入设备中。
// 本事件只有当媒介自动进入时才会生成。
void CXFS_CSR::FireSRVMediaInserted()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIAINSERTED, nullptr);
}

// 上报复位中检测到设备内有媒介事件
void CXFS_CSR::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}
