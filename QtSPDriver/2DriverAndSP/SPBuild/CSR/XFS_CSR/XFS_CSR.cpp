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
    m_bCmdRunning = FALSE;                              // 当前是否有命令执行中
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

    // Device状态ONLINE 同时 有当前命令执行中, 设置Device状态为BUSY
    if (m_stStatus.fwDevice == WFS_PTR_DEVONLINE && m_bCmdRunning == TRUE)
    {
        m_stStatus.fwDevice = WFS_PTR_DEVBUSY;
    }

    // 回收箱统计: 状态 + 计数
    if (m_stStatus.fwDevice == WFS_PTR_DEVBUSY || m_stStatus.fwDevice == WFS_PTR_DEVONLINE)
    {
        INT nRetractBins = 0;
        if ((nRetractBins = m_stNoteBoxList.GetBoxCount()) > 0) // 根据INI获取回收箱列表的箱数
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

HRESULT CXFS_CSR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
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

HRESULT CXFS_CSR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
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
        Log(ThisModule, __LINE__, "Media Status = %d[MEDIANOTPRESENT)], 通道内没有票据存在, Return: %d.", \
            WFS_PTR_MEDIAPRESENT, WFS_ERR_PTR_NOMEDIAPRESENT); \
        return WFS_ERR_PTR_NOMEDIAPRESENT;
    }

    return InnerControlMedia(*lpdwMeidaControl);
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
        Log(ThisModule, __LINE__, "入参 Is NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 下发允许入票命令等待
    m_bInsertEventRep = TRUE;
    m_bCmdRunning = TRUE;                              // 命令执行中
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
    m_bCmdRunning = FALSE;                              // 命令结束

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
    INT nCount = 0;                 // 回收箱内张数

    // 获取RSC-D400M设备INI设置
    if (m_stConfig.nDriverType == IXFSCSR_TYPE_RSCD400M)
    {
        if (*lpusBinNum != m_stConfig.stCfg_RSCD400M.usLeftNoteBoxNo && // 左票箱
            *lpusBinNum != m_stConfig.stCfg_RSCD400M.usRightNoteBoxNo)  // 右票箱
        {
            Log(ThisModule, __LINE__, "->入参(lpusBinNum = %d) 无效, Return: %d.",
                MEDIA_CTR_RETRACT, *lpusBinNum, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    } else
    {
        Log(ThisModule, __LINE__, "->INI指定设备类型[%d]不支持, Return: %d.",
            m_stConfig.nDriverType, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 取回收箱号对应的箱内张数
    nCount = m_stNoteBoxList.GetNoteCount(*lpusBinNum);

    // 统计: 票据计数: 0,写入INI记录
    SetRetractBoxCount(*lpusBinNum, nCount, FALSE);

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
            Log(ThisModule, __LINE__, "Device != ONLINE|BUSY, Return: %d.", WFS_ERR_PTR_MEDIAINVALID);
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    } else
    {
        CHECK_DEVICE();
    }

    // Check: 入参
    if (lpImgRequest == nullptr)
    {
        Log(ThisModule, __LINE__, "入参 Is NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (lpImgRequest->fwImageSource == 0)
    {
        Log(ThisModule, __LINE__, "入参 lpImgRequest->fwImageSource == 0, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 下发允许入票命令等待
    m_bInsertEventRep = TRUE;
    m_bCmdRunning = TRUE;                              // 命令执行中
    hRet = MediaInsertWait(dwTimeOut);
    if (hRet == WFS_SUCCESS)
    {
        hRet = InnerReadImage(lpImgRequest, lppImage, dwTimeOut);
    }
    m_bInsertEventRep = FALSE;
    m_bCmdRunning = FALSE;                              // 命令结束

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
            if ((hRet = InnerControlMedia(pIn->dwMediaControl)) != WFS_SUCCESS)
            {
                Log(ThisModule, nRet, "Input dwMediaControl[%d] Is EjectMedia: ->InnerControlMedia(%d) Is Fail, Return: %d.",
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
        Log(ThisModule, __LINE__, "Media Status = %d[MEDIANOTPRESENT)], 通道内没有票据存在, Return: %d.", \
            WFS_PTR_MEDIAPRESENT, WFS_ERR_PTR_NOMEDIAPRESENT); \
        return WFS_ERR_PTR_NOMEDIAPRESENT;
    }

    // Full状态检查
    if (m_stNoteBoxList.GetBoxStat(*lpusBinNum) == BOX_FULL ||
        m_stNoteBoxList.GetRetraceFull(*lpusBinNum) == BOX_FULL)
    {
        Log(ThisModule, __LINE__, "票据数已达INI设置Full阀值或物理箱满, 无法执行回收, Return: %d.",
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
