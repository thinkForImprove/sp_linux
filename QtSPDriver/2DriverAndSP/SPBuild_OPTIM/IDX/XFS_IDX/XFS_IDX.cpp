/***************************************************************
* 文件名称：XFS_IDX.cpp
* 文件描述：身份证读卡器模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月25日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_IDX.h"

#include <unistd.h>
#include <sys/stat.h>


static const char *DEVTYPE = "IDX";
static const char *ThisFile = "XFS_IDX.cpp";
#define LOG_NAME       "XFS_IDX.log"

/*************************************************************************
// 主处理                                                                 *
*************************************************************************/
CXFS_IDX::CXFS_IDX()
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);
    m_stConfig.clear();
    m_enWaitTaken = WTF_NONE;
    memset(&m_Caps, 0x00, sizeof(WFSIDCCAPS));
    m_CardDatas.Clear();
    m_cExtra.Clear();
    m_nReConErr = 0;
}

CXFS_IDX::~CXFS_IDX()
{
    //
}

long CXFS_IDX::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseIDC
    if (0 != m_pBase.Load("SPBaseIDC.dll", "CreateISPBaseIDC", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseIDX失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();

    return 0;
}

// ----------------------------基本接口----------------------------
// 打开设备
HRESULT CXFS_IDX::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // INI读取
    InitConfig();
    InitCaps();
    InitStatus();

    // 检查INI设置设备类型
    if (m_stConfig.wDeviceType != XFS_TYPE_BSID81 &&
        m_stConfig.wDeviceType != XFS_TYPE_DMTM1M)
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定了不支持的设备类型[%d], Return :%d.",
            m_stConfig.wDeviceType, WFS_ERR_DEV_NOT_READY);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 设备驱动动态库验证
    if (strlen(m_stConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 加载设备驱动动态库
    if (m_pDev == NULL)
    {
        hRet = m_pDev.Load(m_stConfig.szDevDllName,
                           "CreateIDevIDC", DEVTYPE_CHG(m_stConfig.wDeviceType));
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
HRESULT CXFS_IDX::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
    {
        m_pDev->Cancel();
        m_pDev->Close();
    }

    return WFS_SUCCESS;
}

// 状态更新
HRESULT CXFS_IDX::OnStatus()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;
    UpdateStatus();
    if (m_stStatus.fwDevice == WFS_IDC_DEVOFFLINE)
    {
        m_pDev->SetData(SET_DEV_RECON, nullptr);    // 设置为断线重连执行状态
        //m_pDev->Close();
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
            //UpdateStatus();
        }
    }

    return WFS_SUCCESS;
}

// Taken上报
HRESULT CXFS_IDX::OnWaitTaken()
{
    if (m_enWaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    UpdateStatus();
    return WFS_SUCCESS;
}

HRESULT CXFS_IDX::OnCancelAsyncRequest()
{
    if (m_pDev != nullptr)
    {
        m_pDev->Cancel();
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_IDX::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

//----------------------------查询命令----------------------------

// 状态查询
HRESULT CXFS_IDX::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stStatus.usCards = (m_stConfig.wRetainSupp == 1 ? m_stConfig.wRetainCardCount : 0);
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_stStatus;    

    return WFS_SUCCESS;
}

// 能力值查询
HRESULT CXFS_IDX::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Caps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_Caps;

    return WFS_SUCCESS;
}

//----------------------------执行命令----------------------------

// 退卡
HRESULT CXFS_IDX::EjectCard(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    // 更新当前设备介质状态
    UpdateStatus();

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 无卡状态检查
    CHK_MEDIA_ISHAVE(m_stStatus.fwMedia);

    // 卡JAM状态检查
    CHK_MEDIA_ISJAM(m_stStatus.fwMedia);

    if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)    // 卡在出口
    {
        Log(ThisModule, __LINE__, "退卡: 卡已在出口, 不执行退卡操作, Return: %d.", WFS_SUCCESS);
        m_enWaitTaken = WTF_TAKEN;
        return WFS_SUCCESS;
    }

    hRet = InnerEject();
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "退卡: ->InnerEject() fail, RetCode: %d, Return: %d.", hRet, hRet);
        return hRet;
    }

    return WFS_SUCCESS;
}

// 吞卡
long CXFS_IDX::RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    if (m_stConfig.wRetainSupp == 0)    // 不支持吞卡
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    // 更新当前设备介质状态
    UpdateStatus();

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 无卡状态检查
    CHK_MEDIA_ISHAVE(m_stStatus.fwMedia);

    // 回收盒检查
    if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainFull)     // 回收满
    {
        Log(ThisModule, __LINE__,
            "吞卡: 当前回收计数[%d] >= INI设置回收警阀值[%d], Return: %d.",
            m_stConfig.wRetainCardCount, m_stConfig.wRetainFull, WFS_ERR_IDC_RETRACTBINFULL);
        return WFS_ERR_IDC_RETRACTBINFULL;
    }

    // Taken标记归零
    m_enWaitTaken = WTF_NONE;

    // 回参: 回收前卡位置
    switch(m_stStatus.fwMedia)
    {
        case WFS_IDC_MEDIAPRESENT:      // 内部
            m_stWFSIdcRetainCard.fwPosition = WFS_IDC_MEDIAPRESENT;
            break;
        case WFS_IDC_MEDIAENTERING:     // 出口
            m_stWFSIdcRetainCard.fwPosition = WFS_IDC_MEDIAENTERING;
            break;
        default:                        // 未知
            m_stWFSIdcRetainCard.fwPosition = WFS_IDC_MEDIAUNKNOWN;
            break;
    }

    hRet = InnerRetainCard();
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "吞卡: ->InnerRetainCard() fail, RetCode: %d, Return: %d.", hRet, hRet);
        return hRet;
    }

    // 回参: 回收卡数目
    m_stWFSIdcRetainCard.usCount = m_stConfig.wRetainCardCount;

    lpRetainCardData = &m_stWFSIdcRetainCard;

    return WFS_SUCCESS;
}

// 清空回收计数
HRESULT CXFS_IDX::ResetCount()
{
    THISMODULE(__FUNCTION__);

    if (m_stConfig.wRetainSupp == 0)    // 不支持吞卡
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    if (SetRetainCardCount(0) != 0)
    {
        Log(ThisModule, __LINE__,
            "清空回收计数: ->SetRetainCardCount(0) fail, Return: %d.", WFS_ERR_INTERNAL_ERROR);
        return WFS_ERR_INTERNAL_ERROR;
    }

    m_stConfig.wRetainCardCount = 0;

    // 更新当前设备介质状态
    UpdateStatus();

    return WFS_SUCCESS;
}

// 读卡
HRESULT CXFS_IDX::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    DWORD wOption = *(WORD *)lpReadData;

    UpdateStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 读卡时不选任何参数值返回 DATAINVALID
    if (m_stConfig.wReadRawDataInParamMode == 1)  // 入参模式1
    {
        DWORD nTrack = WFS_IDC_CHIP | WFS_IDC_FRONTIMAGE | WFS_IDC_BACKIMAGE;
        if ((wOption & nTrack) == 0)
        {
            Log(ThisModule, __LINE__,
                "读卡: 入参模式1: 接收ReadRawData参数[%d]无效, Return: %d.", wOption, WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }
    } else  // 通用参数
    {
        DWORD nTrack = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
        if ((wOption & nTrack) == 0)
        {
            Log(ThisModule, __LINE__,
                "读卡: 入参模式0: 接收ReadRawData参数[%d]无效, Return: %d.", wOption, WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }
    }                                                                           // 40-00-00-00(FT#0010)

    m_CardDatas.Clear();
    HRESULT hRet = InnerReadRawData(wOption, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "读卡: 调用子处理: ->InnerReadRawData() fail, RetCode: %d, Return: %d.", hRet, hRet);
        return hRet;
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_CardDatas;

    // 支持读证后退卡
    if (m_stConfig.wReadEndRunEject == 1 && hRet == WFS_SUCCESS)
    {
        return InnerEject();
    }

    return hRet;
}

// 复位
HRESULT CXFS_IDX::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wAction = 0;
    BOOL bHaveCard = FALSE;     // 是否有卡存在标记

    // 没有动作参数时由SP->INI决定
    if (*lpResetIn == 0)
    {
        if (m_stConfig.wResetCardAction == 0)   // 无动作
        {
            wAction = WFS_IDC_NOACTION;
        } else
        if (m_stConfig.wResetCardAction == 1)   // 退卡
        {
            wAction = WFS_IDC_EJECT;
        } else
        if (m_stConfig.wResetCardAction == 2)   // 吞卡
        {
            wAction = WFS_IDC_RETAIN;
        }
    } else
    {
        wAction = *lpResetIn;
    }

    // 无效参数
    if (wAction < WFS_IDC_NOACTION || wAction > WFS_IDC_RETAIN)
    {
        Log(ThisModule, __LINE__,
            "复位: 接收参数[%d]无效, Return: %d.", wAction, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    UpdateStatus(); // 更新当前设备介质状态

    // 复位前卡状态检查
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT ||   // 卡在内部
        m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING ||  // 卡在出口
        m_stStatus.fwMedia == WFS_IDC_MEDIAJAMMED)      // 卡JAM
    {
        FireMediaDetected(WFS_IDC_CARDREADPOSITION);
        bHaveCard = TRUE;
    }

    // 指定回收同时有卡存在时: 回收盒检查
    if (wAction == WFS_IDC_RETAIN && bHaveCard == TRUE)
    {
        if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainFull)     // 回收满
        {
            Log(ThisModule, __LINE__,
                "吞卡: 当前回收计数[%d] > INI设置回收警阀值[%d], Return: %d.", WFS_ERR_IDC_RETRACTBINFULL);
            return WFS_ERR_IDC_RETRACTBINFULL;
        }

        // Taken标记归零
        m_enWaitTaken = WTF_NONE;
    }

    HRESULT hRet = InnerReset(wAction, bHaveCard);
    if (m_stConfig.wResetFailReturn == 0)   // Reset结果原样返回
    {
        return hRet;
    } else
    {
        return WFS_SUCCESS;
    }
}

