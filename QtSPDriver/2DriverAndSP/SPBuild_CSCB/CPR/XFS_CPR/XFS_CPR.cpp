/***************************************************************
* 文件名称：XFS_CPR.cpp
* 文件描述：票据发放模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_CPR.h"

// PTR SP 版本号
BYTE    byVRTU[17] = {"HWCPRSTE00000100"};

// 事件日志
static const char *ThisFile = "XFS_CPR.cpp";

#define LOG_NAME     "XFS_CPR.log"

//-----------------------------------------------------------------------------------
//-------------------------------------构造/析构--------------------------------------
CXFS_CPR::CXFS_CPR()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    m_WaitTaken = WTF_NONE;
    m_bNeedReset = true;
    m_nReConRet = PTR_SUCCESS;
    memset(m_stWFSRetractBin, 0x00, sizeof(WFSPTRRETRACTBINS) * 16);
    memset(m_stWFSRetractBinOLD, 0x00, sizeof(WFSPTRRETRACTBINS) * 16);
}

CXFS_CPR::~CXFS_CPR()
{

}

//-----------------------------------------------------------------------------------
// 开始运行SP
long CXFS_CPR::StartRun()
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
HRESULT CXFS_CPR::OnOpen(LPCSTR lpLogicalName)
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
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DEVTYPE=%d|%s. ReturnCode:%d.",
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
HRESULT CXFS_CPR::OnClose()
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
HRESULT CXFS_CPR::OnStatus()
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
HRESULT CXFS_CPR::OnWaitTaken()
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
HRESULT CXFS_CPR::OnCancelAsyncRequest()
{
    return WFS_SUCCESS;
}

// 固件升级
HRESULT CXFS_CPR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}


//-----------------------------------------------------------------------------------
//-------------------------------重载CSPBaseClass的方法--------------------------------
HRESULT CXFS_CPR::OnInit()
{
    THISMODULE(__FUNCTION__);

    INT nRet = m_pPrinter->Init();
    //UpdateDeviceStatus(nRet);
    if (nRet < 0)
    {
        return ConvertErrCode(nRet);
    }
    //m_pPrinter->SetPrintFormat(m_stPrintFormat);
    return ConvertErrCode(nRet);
}

HRESULT CXFS_CPR::OnExit()
{
    if (nullptr != m_pPrinter)
    {
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//----------------------------------PTR类型接口(INFO)----------------------------------
HRESULT CXFS_CPR::GetStatus(LPWFSPTRSTATUS &lpStatus)
{
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_stStatus;

    // 回收箱统计: 状态 + 计数
    if (m_stStatus.fwDevice == WFS_PTR_DEVBUSY || m_stStatus.fwDevice == WFS_PTR_DEVONLINE)
    {
        INT nRetractBins = 0;
        if ((nRetractBins = m_stNoteBoxList.nIsRetractBox()) > 0)
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
            INT nBoxSum = 0;
            for (INT i = 0; i < BOX_COUNT; i ++)
            {
                if (m_stNoteBoxList.stNoteBox[i].bIsHave == TRUE && m_stNoteBoxList.stNoteBox[i].usBoxType == BOX_RETRACT)
                {
                    if (m_stNoteBoxList.stNoteBox[i].usNoteCount >= m_stNoteBoxList.stNoteBox[i].usFullThreshold) // FULL
                    {
                        m_stStatus.lppRetractBins[nBoxSum]->wRetractBin = WFS_PTR_RETRACTBINFULL;
                    } else
                    if (m_stNoteBoxList.stNoteBox[i].usNoteCount >= m_stNoteBoxList.stNoteBox[i].usThreshold)     // HIGH
                    {
                        m_stStatus.lppRetractBins[nBoxSum]->wRetractBin = WFS_PTR_RETRACTBINHIGH;
                    } else
                    {
                        m_stStatus.lppRetractBins[nBoxSum]->wRetractBin = WFS_PTR_RETRACTBINOK;
                    }
                    m_stStatus.lppRetractBins[nBoxSum]->usRetractCount = m_stNoteBoxList.stNoteBox[i].usNoteCount;
                    nBoxSum ++;
                }
            }
        }
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_CPR::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_sCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_CPR::GetFormList(LPSTR &lpszFormList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    lpszFormList = (LPSTR)m_pData->GetNames();

    return WFS_SUCCESS;
}

HRESULT CXFS_CPR::GetMediaList(LPSTR &lpszMediaList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    lpszMediaList = (LPSTR)m_pData->GetNames(false);

    return WFS_SUCCESS;
}

HRESULT CXFS_CPR::GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader)
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

HRESULT CXFS_CPR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

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

HRESULT CXFS_CPR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
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
HRESULT CXFS_CPR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    THISMODULE(__FUNCTION__);

    // Check: 状态
    UpdateDeviceStatus();   // 取当前最新状态

    // 设备Check
    CHECK_DEVICE();

    return ControlMedia(*lpdwMeidaControl);
}

// 格式化打印
HRESULT CXFS_CPR::PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    // Check: 状态
    UpdateDeviceStatus();   // 取当前最新状态

    // 设备Check
    CHECK_DEVICE();

    // Media Check: 无
    if (m_stStatus.fwMedia != WFS_PTR_MEDIAPRESENT)
    {
        Log(ThisModule, -1, "Media != PRESENT,通道内无票, Return: %d.", WFS_ERR_PTR_NOMEDIAPRESENT);
        return WFS_ERR_PTR_NOMEDIAPRESENT;
    }

    // 设置边界
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);

    // 加载Form文件
    m_pData->LoadForms();

    // 加载Media文件
    m_pData->LoadMedias();

    LPWFSPTRPRINTFORM pIn = (LPWFSPTRPRINTFORM)lpPrintForm;
    HRESULT hRet = InnerPrintForm(pIn);

    return hRet;
}

// 格式化读
HRESULT CXFS_CPR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{    
    return WFS_ERR_UNSUPP_COMMAND;

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

// 无格式打印
HRESULT CXFS_CPR::RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    LPWFSPTRRAWDATA pIn = (LPWFSPTRRAWDATA)lpRawData;
    long hRes = SendRawData(WFS_PTR_INPUTDATA == pIn->wInputData, pIn->ulSize, pIn->lpbData);
    if (WFS_SUCCESS == hRes)
    {
        lpRawDataIn = (LPWFSPTRRAWDATAIN)m_pData->m_InputRawData;
    }
    return hRes;
}

// 获得插入物理设备中的媒介的长宽度
HRESULT CXFS_CPR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    THISMODULE(__FUNCTION__);

    return WFS_ERR_UNSUPP_COMMAND;
}

// 将媒介回收计数由当前值归零
HRESULT CXFS_CPR::ResetCount(const LPUSHORT lpusBinNum)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = PTR_SUCCESS;
    WORD wRetractBox = *lpusBinNum;

    // Check: 是否有可用的回收箱
    if (m_stNoteBoxList.nIsRetractBox() < 1)
    {
        Log(ThisModule, -1, "INI没有设置可用的回收箱, Return: %d.", WFS_ERR_UNSUPP_COMMAND);
        return WFS_ERR_UNSUPP_COMMAND;
    }

    // Check: 指定回收箱是否存在
    if (wRetractBox > 0)
    {
        if (m_stNoteBoxList.nBoxIsHave(wRetractBox, BOX_RETRACT) == BOX_NOHAVE)
        {
            Log(ThisModule, -1, "指定回收箱索引[%d]不存在, Return: %d.", wRetractBox, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    // 统计: 票据计数: 0,写入INI记录
    SetRetractBoxCount(wRetractBox, 0, TRUE);

    return WFS_SUCCESS;
}

// 获取图象数据
HRESULT CXFS_CPR::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    // Check: 状态
    UpdateDeviceStatus();   // 取当前最新状态
    // 设备Check
    CHECK_DEVICE();

    // Media 无
    if (m_stStatus.fwMedia != WFS_PTR_MEDIAPRESENT)
    {
        Log(ThisModule, -1, "Media != PRESENT,通道内无票, Return: %d.", WFS_ERR_PTR_NOMEDIAPRESENT);
        return WFS_ERR_PTR_NOMEDIAPRESENT;
    }

    HRESULT hRet = InnerReadImage(lpImgRequest, lppImage, dwTimeOut);

    return hRet;
}

// 复位
HRESULT CXFS_CPR::Reset(const LPWFSPTRRESET lpReset)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LPWFSPTRRESET pIn = (LPWFSPTRRESET)lpReset;
    INT nRet = PTR_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;

    // CHECK
    // WFS_PTR_CTRLEJECT 退出媒介
    // WFS_PTR_CTRLRETRACT 根据usRetractBinNumber 规定将媒介回收入回收盒
    // WFS_PTR_CTRLEXPEL 从出口抛出媒介
    WORD wCheckCmd = WFS_PTR_CTRLEJECT | WFS_PTR_CTRLRETRACT | WFS_PTR_CTRLEXPEL;

    if ((pIn->dwMediaControl & wCheckCmd) == 0)
    {
        Log(ThisModule, -1, "Input dwMediaControl[%d] Is Invalid, Return: %d.", pIn->dwMediaControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if ((pIn->dwMediaControl & WFS_PTR_CTRLEXPEL) == WFS_PTR_CTRLEXPEL)
    {
        Log(ThisModule, -1, "Input dwMediaControl[%d] Is NoSupp, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if ((pIn->dwMediaControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT &&
        (pIn->dwMediaControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)
    {
        Log(ThisModule, -1, "入参dwMediaControl[%d] Is Invalid, 参数冲突, Return: %d.",
            pIn->dwMediaControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (pIn->dwMediaControl == WFS_PTR_CTRLEJECT)
    {
        hRet = ControlMedia(pIn->dwMediaControl);
        if (hRet != WFS_SUCCESS && hRet != WFS_ERR_PTR_NOMEDIAPRESENT)
        {
            Log(ThisModule, -1, "Input dwMediaControl[%d] Is EjectMedia: ->ControlMedia(%d) Is Fail, Return: %d.",
                pIn->dwMediaControl, pIn->dwMediaControl, hRet);
            return hRet;
        }
    } else
    {
        hRet = ControlMedia(WFS_PTR_CTRLRETRACT);
        if (hRet != WFS_SUCCESS && hRet != WFS_ERR_PTR_NOMEDIAPRESENT)
        {
            Log(ThisModule, -1, "Input dwMediaControl[%d] Is ControlMedia: ->ControlMedia(%d) Is Fail, Return: %d.",
                pIn->dwMediaControl, pIn->dwMediaControl,  hRet);
            return hRet;
        }
    }

    // 执行复位
    nRet = m_pPrinter->ResetEx(m_stConfig.usResetMode == 0 ? MEDIA_CTR_ALARM: MEDIA_CTR_STACK);
    UpdateDeviceStatus();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, -1, "->ResetEx(%d) Fail, ErrCode = %d, Return: %d.",
            (m_stConfig.usResetMode == 0 ? MEDIA_CTR_ALARM: MEDIA_CTR_STACK), nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    return WFS_SUCCESS;
}

// 媒介回收
HRESULT CXFS_CPR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = PTR_SUCCESS;
    WORD wRetractBox = *lpusBinNum;
    MEDIA_ACTION enMediaAct = MEDIA_CTR_RETRACT;

    // 票据回收前CHECK
    UpdateDeviceStatus();   // 取最新状态

    // 设备Ckeck
    CHECK_DEVICE();

    // Check: 通道内无票
    // ERR_MEDIA_NOTPRESENT();


    if (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT)
    {
        enMediaAct = MEDIA_CTR_PERFORATE; // 通道内回收
    } else
    {
        enMediaAct = MEDIA_CTR_RETRACT;   // 出口回收
    }


    // Check: 是否有可用的回收箱
    if (m_stNoteBoxList.nIsRetractBox() < 1)
    {
        Log(ThisModule, -1, "INI没有设置可用的回收箱, Return: %d.", WFS_ERR_UNSUPP_COMMAND);
        return WFS_ERR_UNSUPP_COMMAND;
    }

    // Check: 指定回收箱是否存在
    if (wRetractBox > 0)
    {
        if (m_stNoteBoxList.nBoxIsHave(wRetractBox, BOX_RETRACT) == BOX_NOHAVE)
        {
            Log(ThisModule, -1, "指定回收箱索引[%d]不存在, Return: %d.", wRetractBox, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    // Check: 回收箱是否已满
    if (m_stNoteBoxList.nGetBoxStat(wRetractBox, BOX_RETRACT) == BOX_FULL)
    {
        Log(ThisModule, -1, "指定回收箱[%d]已满, Return: %d.", wRetractBox, WFS_PTR_RETRACTBINFULL);
        return WFS_PTR_RETRACTBINFULL;
    }

    // 执行: 票据回收
    nRet = m_pPrinter->MeidaControl(enMediaAct, wRetractBox);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, -1, "票据回收->MeidaControl(%d, %d) Fail, ErrCode = %d, Return: %d.",
            enMediaAct, wRetractBox, nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    // 统计: 票据计数: +1,写入INI记录
    SetRetractBoxCount(wRetractBox, 1, TRUE);

    // 组织返回值
    lpusBinNumOut = new USHORT();
    *lpusBinNumOut = wRetractBox;

    return WFS_SUCCESS;
}

// 纸张移动
HRESULT CXFS_CPR::DispensePaper(const LPWORD lpPaperSource)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = PTR_SUCCESS;
    WORD wStorageBox = 0;       // 票箱索引
    WORD wPaperStatNo = 0;      // 纸状态结构体索引

    // 票据出箱前CHECK
    UpdateDeviceStatus();   // 取最新状态

    // 设备Ckeck
    CHECK_DEVICE();

    // Check: 通道内有票
    ERR_MEDIA_PRESENT();

    // Check: 是否有可用的票箱
    if ((wStorageBox = m_stNoteBoxList.nIsStorageBox()) < 1)
    {
        Log(ThisModule, -1, "INI没有设置可用的票箱, Return: %d.", WFS_ERR_UNSUPP_COMMAND);
        return WFS_ERR_UNSUPP_COMMAND;
    }

    // Check: 入参无效
    if (*lpPaperSource != WFS_PTR_PAPERUPPER && *lpPaperSource != WFS_PTR_PAPERLOWER &&
        *lpPaperSource != WFS_PTR_PAPEREXTERNAL && *lpPaperSource != WFS_PTR_PAPERAUX)
    {
        Log(ThisModule, -1, "Input[%d] Is Invalid, Return: %d.", *lpPaperSource, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 转换票箱索引号
    switch(*lpPaperSource)
    {
        case WFS_PTR_PAPERUPPER :
            wStorageBox = 1;
            wPaperStatNo = WFS_PTR_SUPPLYUPPER;
            break;
        case WFS_PTR_PAPERLOWER :
            wStorageBox = 2;
            wPaperStatNo = WFS_PTR_SUPPLYLOWER;
            break;
        case WFS_PTR_PAPEREXTERNAL :
            wStorageBox = 3;
            wPaperStatNo = WFS_PTR_SUPPLYEXTERNAL;
            break;
        case WFS_PTR_PAPERAUX :
            wStorageBox = 4;
            wPaperStatNo = WFS_PTR_SUPPLYAUX;
            break;
    }

    // Check: 指定索引号是否有效
    nRet = m_stNoteBoxList.nBoxIsHave(wStorageBox);
    if (nRet == BOX_NOHAVE)
    {
        Log(ThisModule, -1, "指定票箱索引[%d]不存在, Return: %d.",
            wStorageBox, WFS_ERR_PTR_SOURCEINVALID);
        return WFS_ERR_PTR_SOURCEINVALID;
    }

    // Check: 票据箱是否有票
    nRet = m_stNoteBoxList.nGetBoxCount(wStorageBox);   // 取指定票据箱张数
    if (nRet == BOX_NOHAVE) // 未找到票据存储箱
    {
        Log(ThisModule, -1, "指定票箱索引[%d]不存在, Return: %d.",
            wStorageBox, WFS_ERR_PTR_SOURCEINVALID);
        return WFS_ERR_PTR_SOURCEINVALID;
    }
    if (nRet < 1 || m_stStatus.fwPaper[wPaperStatNo] == WFS_PTR_PAPEROUT)   // 票据箱空: 计数||Paper状态
    {
        Log(ThisModule, -1, "指定票箱索引[%d]为空, INI计数[%d] < 1 || Status.Paper[%d] = PAPEROUT, Return: %d.",
            wStorageBox, nRet, wPaperStatNo, WFS_ERR_PTR_PAPEROUT);
        return WFS_ERR_PTR_PAPEROUT;
    }

    // 执行: 票箱出票
    nRet = m_pPrinter->MeidaControl(MEDIA_CTR_ATPBACKWARD, wStorageBox);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, -1, "票箱出票->MeidaControl(%d, %d) Fail, ErrCode = %d, Return: %d.",
            MEDIA_CTR_EXPEL, wStorageBox, nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    // 统计: 票据计数: -1,写入INI记录
    SetStorageBoxCount(wStorageBox, 1, FALSE);

    return WFS_SUCCESS;
}

// 指示灯控制
HRESULT CXFS_CPR::SetGuidanceLight(const LPWFSPTRSETGUIDLIGHT lpSetGuidLight)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

//-----------------------------------------------------------------------------------
//--------------------------------------事件消息---------------------------------------
// 上报Device HWERR事件
void CXFS_CPR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

// 上报状态变化事件
void CXFS_CPR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

// 上报无媒介事件
void CXFS_CPR::FireNoMedia(LPCSTR szPrompt)
{
    //FireEvent(MFT_EE, WFS_EXEE_PTR_NOMEDIA, WFS_SUCCESS, (LPVOID)szPrompt);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}

// 上报媒介放入事件
void CXFS_CPR::FireMediaInserted()
{
    //FireEvent(MFT_EE, WFS_EXEE_PTR_MEDIAINSERTED);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

// 上报Field错误事件
void CXFS_CPR::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    //FireEvent(MFT_EE, WFS_EXEE_PTR_FIELDERROR, WFS_SUCCESS, &fail);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

// 上报Field警告事件
void CXFS_CPR::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    //FireEvent(MFT_EE, WFS_EXEE_PTR_FIELDWARNING, WFS_SUCCESS, &fail);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}

// 上报回收箱变化事件
void CXFS_CPR::FireRetractBinThreshold(USHORT BinNumber, WORD wStatus)
{
    WFSPTRBINTHRESHOLD BinThreshold;
    BinThreshold.usBinNumber     = BinNumber;
    BinThreshold.wRetractBin = wStatus;
    //FireEvent(MFT_UE, WFS_USRE_PTR_RETRACTBINTHRESHOLD, WFS_SUCCESS, &BinThreshold);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_RETRACTBINTHRESHOLD, &BinThreshold);
}

// 上报媒介取走事件
void CXFS_CPR::FireMediaTaken()
{
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIATAKEN);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}

// 上报纸状态/票箱状态变化事件
void CXFS_CPR::FirePaperThreshold(WORD wSrc, WORD wStatus)
{
    WFSPTRPAPERTHRESHOLD PaperThreshold;
    PaperThreshold.wPaperSource     = wSrc;
    PaperThreshold.wPaperThreshold  = wStatus;
    //FireEvent(MFT_UE, WFS_USRE_PTR_PAPERTHRESHOLD, WFS_SUCCESS, &PaperThreshold);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_PAPERTHRESHOLD, &PaperThreshold);
}

// 上报碳带状态变化事件
void CXFS_CPR::FireTonerThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_TONERTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_TONERTHRESHOLD, &wStatus);
}

// 上报墨盒状态变化事件
void CXFS_CPR::FireInkThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_INKTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}

// 上报灯状态变化事件
void CXFS_CPR::FireLampThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_LAMPTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}

// 本事件指出在没有任何打印执行命令执行的情况下物理媒体已插入设备中。
// 本事件只有当媒介自动进入时才会生成。
void CXFS_CPR::FireSRVMediaInserted()
{
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIAINSERTED);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIAINSERTED, nullptr);
}

// 上报复位中检测到设备内有媒介事件
void CXFS_CPR::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIADETECTED, WFS_SUCCESS, &md);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}



