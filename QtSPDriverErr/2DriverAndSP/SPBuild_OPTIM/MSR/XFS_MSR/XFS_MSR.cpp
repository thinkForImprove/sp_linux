/***************************************************************
* 文件名称：XFS_MSR.cpp
* 文件描述：刷折器模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_MSR.h"


static const char *DEVTYPE = "MSR";
static const char *ThisFile = "XFS_MSR.cpp";
#define LOG_NAME     "XFS_MSR.log"

/*************************************************************************
// 主处理                                                                 *
*************************************************************************/
CXFS_MSR::CXFS_MSR()
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);
    m_stConfig.clear();
    m_stStatus.Clear();
    m_stStatusOLD.Clear();
    m_CardDatas.Clear();
    m_cExtra.Clear();
    m_nReConErr = 0;
}

CXFS_MSR::~CXFS_MSR()
{
    //
}

long CXFS_MSR::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseMSR
    if (0 != m_pBase.Load("SPBaseIDC.dll", "CreateISPBaseIDC", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseMSR失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();

    return 0;
}

//------------------------------------基本接口------------------------------------
// 打开设备
HRESULT CXFS_MSR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // INI读取
    InitConfig();

    // Capabilities Status　初始化
    InitCaps();
    InitStatus();
    
    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    if (m_stConfig.wDeviceType != XFS_TYPE_WBCS10 &&
        m_stConfig.wDeviceType != XFS_TYPE_WBT2172)
    {
        Log(ThisModule, __LINE__, "INI指定设备类型[%d]不支持", m_stConfig.wDeviceType);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 设备驱动动态库验证
    if (strlen(m_stConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 加载设备驱动动态库
    if (m_pDev == NULL)
    {
        hRet = m_pDev.Load(m_stConfig.szDevDllName,
                           "CreateIDevMSR", DEVTYPE_CHG(m_stConfig.wDeviceType));
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__,
                "加载库失败: DriverDllName=%s, DEVTYPE=%d|%s. ReturnCode:%s",
                m_stConfig.szDevDllName, m_stConfig.wDeviceType,
                DEVTYPE_CHG(m_stConfig.wDeviceType),
                m_pDev.LastError().toUtf8().constData());
            return WFS_ERR_INTERNAL_ERROR;
        }
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = InnerOpen();
    if (m_stConfig.nOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

// 关闭设备
HRESULT CXFS_MSR::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
    {
        m_pDev->Close();
    }

    return WFS_SUCCESS;
}

// 状态更新
HRESULT CXFS_MSR::OnStatus()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;
    UpdateStatus();
    if (m_stStatus.fwDevice == WFS_IDC_DEVOFFLINE)
    {
        m_pDev->SetData(SET_DEV_RECON, nullptr);    // 设置为断线重连执行状态
        m_pDev->Close();
        CQtTime::Sleep(500);
        hRet = InnerOpen(TRUE);                     // 执行断线重连
        if (hRet != WFS_SUCCESS)
        {
            if (m_nReConErr != hRet)
            {
                Log(ThisModule, __LINE__, "断线重连中: OnStatus->InnerOpen(%d) fail, ErrCode:%d.", TRUE, hRet);
                m_nReConErr = hRet;
            }
        } else
        {
            if (m_nReConErr != hRet)
            {
                Log(ThisModule, __LINE__, "断线重连完成: OnStatus->InnerOpen(%d) succ, RetCode:%d.", TRUE, hRet);
                m_nReConErr = hRet;
            }
            UpdateStatus();
        }
    }

    return WFS_SUCCESS;
}

// Taken上报
HRESULT CXFS_MSR::OnWaitTaken()
{
    return WFS_SUCCESS;
}

HRESULT CXFS_MSR::OnCancelAsyncRequest()
{
    if (m_pDev != nullptr)
    {
        m_pDev->Cancel();
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_MSR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

//------------------------------------查询命令------------------------------------

// 状态查询
HRESULT CXFS_MSR::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_stStatus;

    return WFS_SUCCESS;
}

// 能力值查询
HRESULT CXFS_MSR::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Caps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_Caps;

    return WFS_SUCCESS;
}

//------------------------------------执行命令------------------------------------
// 读卡
HRESULT CXFS_MSR::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wOption = *(WORD *)lpReadData;

    // 读卡时不选任何参数值返回DATAINVALID
    //if ((wOption & 0x803F) == 0)
    WORD nTrack = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
    if (wOption & nTrack == 0)
    {
        Log(ThisModule, __LINE__,
            "读卡: 接收ReadRawData参数[%d]无效, Return: %d.", wOption, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    m_CardDatas.Clear();
    HRESULT hRet = InnerReadRawData(wOption, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "读卡: 调用子处理: ->InnerReadRawData() fail, RetCode: %d, Return: %d.", hRet, hRet);
        return hRet;
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_CardDatas;
    return hRet;
}

// 复位
HRESULT CXFS_MSR::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = InnerReset();
    if (m_stConfig.wResetFailReturn == 0)
    {
        return hRet;
    } else
    {
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "复位: 调用子处理: ->InnerReset() fail, RetCode: %d, INI.ResetFailReturn==1, Return: %d.",
                hRet, WFS_SUCCESS);
        }
        return WFS_SUCCESS;
    }
}
