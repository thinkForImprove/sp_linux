/***************************************************************
* 文件名称：XFS_DSR.cpp
* 文件描述：文档扫描模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/
#include "file_access.h"
#include "XFS_DSR.h"
#include <qthread.h>

CXFS_DSR::CXFS_DSR()
{
    SetLogFile(LOG_NAME, ThisFile);     // 设置日志文件名和错误发生的文件
    m_nReConErr = PTR_SUCCESS;
    m_bCmdRuning = FALSE;               // 是否命令执行中:F
    m_bCmdCanceled = FALSE;             // 命令是否取消
    m_WaitTaken = WTF_NONE;
    m_dwRetractCntEvent = 0;
    memset(m_wNeedFireEvent, 0, sizeof(m_wNeedFireEvent));  // 需要特殊上报的事件
}

CXFS_DSR::~CXFS_DSR()
{
}


long CXFS_DSR::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载 BasePTR
    if (0 != m_pBase.Load("SPBasePTR.dll", "CreateISPBasePTR", "PTR"))
    {
        Log(ThisModule, __LINE__, "加载 SPBasePTR 失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();

    return 0;
}

HRESULT CXFS_DSR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    InitConifig();
    InitStatus();
    InitCaps();

    // 获取 SPBase 的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // Open 失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = StartOpen();
    if (m_stConfig.nOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

HRESULT CXFS_DSR::OnClose()
{
    THISMODULE(__FUNCTION__);

    Log(ThisModule, __LINE__, "DSR SP Close ");

    m_bCmdCanceled = TRUE;
    usleep(1000 * 500);     // 休眠500毫秒,确保吸折动作取消
    if (m_DSRinter != nullptr)
    {
        Log(ThisModule, __LINE__, "m_DSRinter->Close()");
        m_DSRinter->Close();
    }
    return WFS_SUCCESS;
}

// 实时状态(循环调用)
HRESULT CXFS_DSR::OnStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);

    DEVPTRSTATUS stDevStatus;
    //Log(ThisModule, __LINE__, "OnStatus thread id = %d", QThread::currentThread());
    int nRet = m_DSRinter->GetStatus(stDevStatus);
    if (nRet == ERR_PTR_NOT_OPEN)
    {
        m_DSRinter->SetData(nullptr, SET_DEV_RECON);        // 设置为断线重连执行状态
        nRet = m_DSRinter->Open(DEVTYPE2STR(m_stConfig.nDeviceType));
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
HRESULT CXFS_DSR::OnWaitTaken()
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

HRESULT CXFS_DSR::OnCancelAsyncRequest()
{
    m_bCmdCanceled = TRUE;
    return WFS_SUCCESS;
}

HRESULT CXFS_DSR::OnUpdateDevPDL()
{
    return WFS_ERR_UNSUPP_COMMAND;
}

// PTR类型接口
// INFOR
HRESULT CXFS_DSR::GetStatus(LPWFSPTRSTATUS &lpStatus)
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

HRESULT CXFS_DSR::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_sCaps;
    return WFS_SUCCESS;
}

HRESULT CXFS_DSR::GetFormList(LPSTR &lpszFormList)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::GetMediaList(LPSTR &lpszMediaList)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::PrintForm(const LPWFSPTRPRINTFORM lppRintForm, DWORD dwTimeOut)
{    
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::ResetCount(const LPUSHORT lpusBinNum)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);
    return InnerMediaControl(*lpdwMeidaControl);
}

HRESULT CXFS_DSR::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    HRESULT hRet = WFS_SUCCESS;

    if (m_stConfig.nDeviceType == DEV_BSD216)
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

HRESULT CXFS_DSR::Reset(const LPWFSPTRRESET lpReset)
{
    LPWFSPTRRESET pIn = (LPWFSPTRRESET)lpReset;
    long hRes = InnerReset(pIn != nullptr ? pIn->dwMediaControl : 0,
                           pIn != nullptr ? pIn->usRetractBinNumber : 0);
    return hRes;
}

HRESULT CXFS_DSR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

HRESULT CXFS_DSR::DispensePaper(const LPWORD lpPaperSource)
{
    return WFS_ERR_UNSUPP_COMMAND;
}

