/***************************************************************
* 文件名称: XFS_BCR.cpp
* 文件描述: 二维码模块命令处理接口(BCR命令系)
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2019年7月6日
* 文件版本: 1.0.0.1
****************************************************************/

#include "XFS_BCR.h"

static const char *ThisFile = "XFS_BCR.cpp";
static const char *DEVTYPE = "BCR_BCR";
#define LOG_NAME     "XFS_BCR.log"

/*************************************************************************
// 主处理                                                                 *
*************************************************************************/
CXFS_BCR::CXFS_BCR() : m_clErrorDet((LPSTR)DEVTYPE)
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);        // 设置日志文件名和错误发生文件
    memset(m_nRetErrOLD, 0, sizeof(INT) * 12);
}

CXFS_BCR::~CXFS_BCR()
{
    ;
}

// 开始运行
long CXFS_BCR::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseBCR
    if (m_pBase.Load("SPBaseBCR.dll", "CreateISPBaseBCR", DEVTYPE) != 0)
    {
        Log(ThisModule, __LINE__, "加载SPBaseBCR失败");
        SetErrorDetail((LPSTR)EC_XFS_SPBaseLoadFail);
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();

    return 0;
}

//-----------------------------基本接口----------------------------------
// Open命令入口
HRESULT CXFS_BCR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    // 获取Manager配置文件中sp配置
    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    InitConfig();       // 读INI
    InitCaps();         // 能力值初始化
    InitStatus();       // 状态值初始化

    m_cStatExtra.Clear();

    // 检查INI设置设备类型
    if (m_stConfig.wDeviceType != XFS_NT0861)// &&
        //m_stConfig.wDeviceType != XFS_EC2G)
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定了不支持的设备类型[%d], Return :%d.",
            m_stConfig.wDeviceType, WFS_ERR_DEV_NOT_READY);
        SetErrorDetail((LPSTR)EC_XFS_DevNotSupp);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 加载DevXXX动态库
    if (LoadDevBCRDll(ThisModule) != TRUE)
    {
        return WFS_ERR_INTERNAL_ERROR;
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = InnerOpen();
    if (m_stConfig.wOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

HRESULT CXFS_BCR::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pBCRDev != nullptr)
    {
        m_pBCRDev->Close();
        m_pBCRDev = nullptr;
    }

    return WFS_SUCCESS;
}

// 状态实时获取
HRESULT CXFS_BCR::OnStatus()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;
    UpdateDeviceStatus();
    if(m_stStatus.fwDevice == WFS_BCR_DEVOFFLINE)
    {
        // 断线自动重连
        m_pBCRDev->SetData(SET_DEV_RECON, nullptr);    // 设置为断线重连执行状态
        hRet = InnerOpen(TRUE);                     // 执行断线重连
        if (hRet != WFS_SUCCESS)
        {
            if (m_nRetErrOLD[0] != hRet)
            {
                Log(ThisModule, __LINE__,
                    "BCR(BCR): 断线重连中: OnStatus->InnerOpen(%d) fail, ErrCode: %d.", TRUE, hRet);
                m_nRetErrOLD[0] = hRet;
            }
        } else
        {
            if (m_nRetErrOLD[0] != hRet)
            {
                Log(ThisModule, __LINE__,
                    "BCR(BCR): 断线重连完成: OnStatus->InnerOpen(%d) succ, RetCode:%d.", TRUE, hRet);
                m_nRetErrOLD[0] = hRet;
            }
        }
    }

    return WFS_SUCCESS;
}

// Taken事件状态实时获取
HRESULT CXFS_BCR::OnWaitTaken()
{
    if (m_enWaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    UpdateDeviceStatus();
    return WFS_SUCCESS;
}

// 取消命令
HRESULT CXFS_BCR::OnCancelAsyncRequest()
{
    if (m_pBCRDev != nullptr)
    {
        m_pBCRDev->Cancel();
    }
    return WFS_SUCCESS;
}

// 固件升级
HRESULT CXFS_BCR::OnUpdateDevPDL()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return WFS_ERR_UNSUPP_COMMAND;
}

//----------------------------BCR类型接口----------------------------
// 查询状态
HRESULT CXFS_BCR::GetStatus(LPWFSBCRSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cStatExtra.AddExtra("ErrorDetail", m_clErrorDet.GetSPErrCode());
    m_cStatExtra.AddExtra("LastErrorDetail", m_clErrorDet.GetSPErrCodeLast());

    m_stStatus.lpszExtra = (LPSTR)m_cStatExtra.GetExtra();
    lpStatus = &m_stStatus;
    return WFS_SUCCESS;
}

// 查询能力值
HRESULT CXFS_BCR::GetCapabilities(LPWFSBCRCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_stCaps.lpszExtra = (LPSTR)m_cCapsExtra.GetExtra();
    lpCaps = &m_stCaps;
    return WFS_SUCCESS;
}

// 读卡
HRESULT CXFS_BCR::ReadBCR(const WFSBCRREADINPUT &stReadInput, LPWFSBCRREADOUTPUT *&lppReadOutput,
                          DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    UpdateDeviceStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 入参Check


    m_CardDatas.Clear();
    lppCardData = nullptr;
    HRESULT hRet = InnerReadBcr(wOption, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡(扫码): ->InnerAcceptAndReadTrack(%d, %d) Fail, Return: %d",
            wOption, dwTimeOut, hRet);
        return hRet;
    }
    lppCardData = (LPWFSBCRCARDDATA *)m_CardDatas;

    return WFS_SUCCESS;
}

// 复位
HRESULT CXFS_BCR::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = InnerReset(*lpResetIn);
    if (hRet == WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "复位: ->InnerReset(%d) Fail, ErrCode: %d",
            *lpResetIn, hRet);
        m_clErrorDet.SetErrCodeInit();
    }
    if (m_stConfig.wResetFailReturn == 0)   // Reset结果原样返回
    {
        return hRet;
    }

    return WFS_SUCCESS;
}

// -------------------------------------- END --------------------------------------
