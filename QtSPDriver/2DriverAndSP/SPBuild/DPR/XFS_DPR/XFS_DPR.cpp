/***************************************************************
* 文件名称：XFS_DPR.cpp
* 文件描述：文档打印模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月23日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_DPR.h"
#include "string.h"
#include "QtTypeInclude.h"
#include <QTextCodec>
#include <stdlib.h>
#include <unistd.h>

// PTR SP 版本号
BYTE    byVRTU[17] = {"HWDPRSTE00000100"};

// 事件日志
static const char *ThisFile = "XFS_DPR.cpp";
#define LOG_NAME     "XFS_DPR.log"
static const char *DEVTYPE = "DPR";

//-----------------------------------------------------------------------------------
//-------------------------------------构造/析构--------------------------------------
CXFS_DPR::CXFS_DPR()
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);  // 设置日志文件名和错误发生的文件
    m_WaitTaken = WTF_NONE;
    m_bNeedReset = true;
    m_bCancelFlag = FALSE;
    m_nReConRet = PTR_SUCCESS;
    m_qlRawDataFileList.clear();
    m_pszInData = nullptr;
}

CXFS_DPR::~CXFS_DPR()
{

}

//-----------------------------------------------------------------------------------
// 开始运行SP
long CXFS_DPR::StartRun()
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
HRESULT CXFS_DPR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    //InitPTRData(lpLogicalName);
    // TODO ini
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

    hRet = Reset(nullptr);
    if (hRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "Open时复位失败. ErrCode:%d, ret=WFS_ERR_HARDWARE_ERROR", ConvertErrCode(hRet));
        return WFS_ERR_HARDWARE_ERROR;
    }

    return hRet;
}

// 关闭设备
HRESULT CXFS_DPR::OnClose()
{
    THISMODULE(__FUNCTION__);

    if (m_pPrinter != nullptr)
    {
        Log(ThisModule, 1, "关闭设备连接成功.");
        m_pPrinter->Close();
    }

    m_qlRawDataFileList.clear();

    if (m_pszInData != nullptr)
    {
        delete[] m_pszInData;
        m_pszInData = nullptr;
    }

    return WFS_SUCCESS;
}

// 实时状态更新
HRESULT CXFS_DPR::OnStatus()
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
HRESULT CXFS_DPR::OnWaitTaken()
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
HRESULT CXFS_DPR::OnCancelAsyncRequest()
{
    m_bCancelFlag = TRUE;
    return WFS_SUCCESS;
}

// 固件升级
HRESULT CXFS_DPR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}


//-----------------------------------------------------------------------------------
//-------------------------------重载CSPBaseClass的方法--------------------------------
HRESULT CXFS_DPR::OnInit()
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

HRESULT CXFS_DPR::OnExit()
{
    if (nullptr != m_pPrinter)
    {
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//----------------------------------PTR类型接口(INFO)----------------------------------
HRESULT CXFS_DPR::GetStatus(LPWFSPTRSTATUS &lpStatus)
{
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_stStatus;

    // 回收箱统计: 状态 + 计数

    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_sCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::GetFormList(LPSTR &lpszFormList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    lpszFormList = (LPSTR)m_pData->GetNames();

    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::GetMediaList(LPSTR &lpszMediaList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    lpszMediaList = (LPSTR)m_pData->GetNames(false);

    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader)
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

HRESULT CXFS_DPR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
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

HRESULT CXFS_DPR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
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
HRESULT CXFS_DPR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_stStatus.fwDevice != WFS_PTR_DEVONLINE &&
        m_stStatus.fwDevice != WFS_PTR_DEVBUSY)
    {
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (m_stStatus.fwPaper[0] == WFS_PTR_PAPEROUT)
    {
        return WFS_ERR_PTR_PAPEROUT;
    }

    if (m_stStatus.fwInk == WFS_PTR_INKOUT)
    {
        return WFS_ERR_PTR_INKOUT;
    }

    if (m_stStatus.fwToner == WFS_PTR_TONEROUT ||
        m_stDPRStatus.wLife == LIFE_STAT_OUT  ||
        m_stDPRStatus.wLessToner == 1)
    {
        return WFS_ERR_PTR_TONEROUT;
    }

    return ControlMedia(*lpdwMeidaControl);
}

// 格式化打印
HRESULT CXFS_DPR::PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

// 格式化读
HRESULT CXFS_DPR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{    
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

// 无格式打印
HRESULT CXFS_DPR::RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    if (m_stStatus.fwDevice != WFS_PTR_DEVONLINE &&
        m_stStatus.fwDevice != WFS_PTR_DEVBUSY)
    {
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (m_stStatus.fwPaper[0] == WFS_PTR_PAPEROUT)
    {
        return WFS_ERR_PTR_PAPEROUT;
    }

    if (m_stStatus.fwInk == WFS_PTR_INKOUT)
    {
        return WFS_ERR_PTR_INKOUT;
    }

    if (m_stStatus.fwToner == WFS_PTR_TONEROUT ||
        m_stDPRStatus.wLife == LIFE_STAT_OUT  ||
        m_stDPRStatus.wLessToner == 1)
    {
        return WFS_ERR_PTR_TONEROUT;
    }

    LPWFSPTRRAWDATA pIn = (LPWFSPTRRAWDATA)lpRawData;
    QString qDevName = m_stConfig.szPrinterName;
    if (qDevName.isEmpty())
    {
        qDevName = "Pantum-P3018D-series";
    }

    long hRes = SendRawData(pIn->wInputData, pIn->ulSize, pIn->lpbData);
    if (hRes != WFS_SUCCESS)
    {
        Log(ThisModule, hRes, "->SendRawData() Fail, ErrCode = %d, Return: %d.",
            hRes, ConvertErrCode(hRes));
        return ConvertErrCode(hRes);
    }

    QString qCmdData = "";
    if (m_qlRawDataFileList.size() <= 0)
    {
        Log(ThisModule, hRes, "->RawData() Fail, 打印列表为空");
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 横向打印，设置-o
    if (m_stPrint.type == 0 || m_stPrint.type == 2)
    {
        qCmdData.append("lp -d ")
                .append(qDevName)
                .append(" -o media=a4 -o landscape -o orientation-requested=4 -p ")
                .append(QString::number(m_stConfig.wJobPriority))
                .append(" -n ")
                .append(QString::number(m_stPrint.num))
                .append(" ");

        for (int i = 0; i < m_qlRawDataFileList.size(); i++)
        {
            qCmdData.append(m_qlRawDataFileList.at(i)).append(" ");
        }
    } else if (m_stPrint.type == 4)
    {
        // 纵向打印
        qCmdData.append("lp -d ")
                .append(qDevName)
                .append(" -o media=a4 -p ")
                .append(QString::number(m_stConfig.wJobPriority))
                .append(" -n ")
                .append(QString::number(m_stPrint.num))
                .append(" ");

        for (int i = 0; i < m_qlRawDataFileList.size(); i++)
        {
            qCmdData.append(m_qlRawDataFileList.at(i)).append(" ");
        }
    } else {
        Log(ThisModule, hRes, "->RawData() Fail, 不支持的打印格式, type=%d", m_stPrint.type);
        return WFS_ERR_INVALID_DATA;
    }

    if (m_pszInData == nullptr)
    {
        for (int i = 0; i < 3; i++)
        {
            m_pszInData = new char[qCmdData.length() + 1];
            if (m_pszInData != nullptr)
                break;
        }
        if (m_pszInData == nullptr)
        {
            Log(ThisModule, hRes, "->malloc() Fail, 申请内存失败, m_pszInData == nullptr. ret=WFS_ERR_INTERNAL_ERROR");
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    setlocale(LC_ALL, "zh_CN UTF-8");
    sprintf(m_pszInData, "%s", (LPSTR)qCmdData.toStdString().c_str());
    //memcpy(m_pszInData, (LPSTR)qCmdData.toStdString().c_str(), qCmdData.length());

    lpRawDataIn = nullptr;
    return hRes;
}

// 获得插入物理设备中的媒介的长宽度
HRESULT CXFS_DPR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

// 将媒介回收计数由当前值归零
HRESULT CXFS_DPR::ResetCount(const LPUSHORT lpusBinNum)
{
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

// 获取图象数据
HRESULT CXFS_DPR::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

// 复位
HRESULT CXFS_DPR::Reset(const LPWFSPTRRESET lpReset)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    // Check: 状态
    UpdateDeviceStatus();   // 取当前最新状态
    // 设备Check
    CHECK_DEVICE();


    // 执行复位(调用IDevPTR接口)
    nRet = m_pPrinter->Reset();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, nRet, "->Reset() Fail, ErrCode = %d, Return: %d.",
            nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    // 预热成功

    return WFS_SUCCESS;
}

// 媒介回收
HRESULT CXFS_DPR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    THISMODULE(__FUNCTION__);
    return WFS_ERR_UNSUPP_COMMAND;
}

// 纸张移动
HRESULT CXFS_DPR::DispensePaper(const LPWORD lpPaperSource)
{
    THISMODULE(__FUNCTION__);

    return WFS_ERR_UNSUPP_COMMAND;
}

// 指示灯控制
HRESULT CXFS_DPR::SetGuidanceLight(const LPWFSPTRSETGUIDLIGHT lpSetGuidLight)
{
    return WFS_ERR_UNSUPP_COMMAND;
}


//-----------------------------------------------------------------------------------
//--------------------------------------事件消息---------------------------------------
// 上报Device HWERR事件
void CXFS_DPR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

// 上报状态变化事件
void CXFS_DPR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

// 上报无媒介事件
void CXFS_DPR::FireNoMedia(LPCSTR szPrompt)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}

// 上报媒介放入事件
void CXFS_DPR::FireMediaInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

// 上报Field错误事件
void CXFS_DPR::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

// 上报Field警告事件
void CXFS_DPR::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}

// 上报回收箱变化事件
void CXFS_DPR::FireRetractBinThreshold(USHORT BinNumber, WORD wStatus)
{
    WFSPTRBINTHRESHOLD BinThreshold;
    BinThreshold.usBinNumber     = BinNumber;
    BinThreshold.wRetractBin = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_RETRACTBINTHRESHOLD, &BinThreshold);
}

// 上报媒介取走事件
void CXFS_DPR::FireMediaTaken()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}

// 上报纸状态/票箱状态变化事件
void CXFS_DPR::FirePaperThreshold(WORD wSrc, WORD wStatus)
{
    WFSPTRPAPERTHRESHOLD PaperThreshold;
    PaperThreshold.wPaperSource     = wSrc;
    PaperThreshold.wPaperThreshold  = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_PAPERTHRESHOLD, &PaperThreshold);
}

// 上报碳带状态变化事件
void CXFS_DPR::FireTonerThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_TONERTHRESHOLD, &wStatus);
}

// 上报墨盒状态变化事件
void CXFS_DPR::FireInkThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}

// 上报灯状态变化事件
void CXFS_DPR::FireLampThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}

// 本事件指出在没有任何打印执行命令执行的情况下物理媒体已插入设备中。
// 本事件只有当媒介自动进入时才会生成。
void CXFS_DPR::FireSRVMediaInserted()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIAINSERTED, nullptr);
}

// 上报复位中检测到设备内有媒介事件
void CXFS_DPR::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}
