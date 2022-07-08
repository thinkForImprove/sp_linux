/***************************************************************
* 文件名称：XFS_SIU.cpp
* 文件描述：读卡器模块+SIU异物检知处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_IDC.h"

//-----------------------------基本接口----------------------------------
// OpenSIU命令入口
HRESULT CXFS_IDC::StartOpenSIU()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    if (m_stConfig.wSkimmingSupp == 0)     // 不支持启用异物检知
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定不支持的异物检知(SIU), Return: SUCCESS.",
            WFS_SUCCESS);
        return WFS_SUCCESS;
    }

    // 加载DevXXX动态库
    if (LoadSIUDevDll(ThisModule) != TRUE)
    {
        return WFS_ERR_INTERNAL_ERROR;
    }

    hRet = InnerOpenSIU();

    return hRet;
}

// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_IDC::InnerOpenSIU(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    INT nRet = IDC_SUCCESS;

    if (m_stConfig.wSkimmingSupp == 1)     // 支持启用退卡模块
    {
        // Open前下传初始参数(非断线重连)
        if (bReConn == FALSE)
        {
            ;
        }
    }

    // 打开SIU设备
    nRet = m_pSIUDev->Open("");
    if (nRet != ERR_IOMC_SUCCESS)
    {
        if (bReConn == FALSE)
        {
            Log(ThisModule, __LINE__, "SIU: Open[%s] fail, ErrCode = %d, Return: %d",
                DEVTYPE2STR(m_stConfig.wDeviceType), nRet, WFS_ERR_HARDWARE_ERROR);
        }
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (bReConn == TRUE)
    {
        Log(ThisModule, __LINE__, "SIU: 断线重连成功.");
    } else
    {
        Log(ThisModule, __LINE__, "SIU: 打开设备连接成功.");
    }

    m_bSIUIsOnLine = TRUE;                        // SIU模块是否连线

    return WFS_SUCCESS;
}

// CloseSIU命令入口
HRESULT CXFS_IDC::EndCloseSIU()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bThreadSkiExit = TRUE;    // 通知异物检知进程退出
   // m_thRunSkimming.join();

    if (m_pSIUDev != nullptr)
    {
        m_pSIUDev->Close();
        m_pSIUDev = nullptr;
    }

    return WFS_SUCCESS;
}

// 加载DevSIU动态库
BOOL CXFS_IDC::LoadSIUDevDll(LPCSTR ThisModule)
{
    if (m_pSIUDev == nullptr)
    {
        if (m_pSIUDev.Load(m_stConfig.szDevDllNameSIU,
                           "CreateIDevSIU", "IOMC") != 0)
        {
            Log(ThisModule, __LINE__,
                "SIU: 加载库失败: DriverDllName=%s, ReturnCode:%s",
                m_stConfig.szDevDllNameSIU, m_pSIUDev.LastError().toUtf8().constData());
            return FALSE;
        }
    }

    return (m_pSIUDev != nullptr);
}

// 变量初始化
INT CXFS_IDC::InitSIU()
{
    m_bSIUIsOnLine = FALSE;                 // SIU模块是否连线
    m_bThreadSkiExit = FALSE;               // 通知异物检知进程退出
    m_bIsHaveSkimming = FALSE;              // 是否检知有异物
    return WFS_SUCCESS;
}

// 异物检知处理进程
void CXFS_IDC::ThreadSkimming()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    QTime   qtTimeCurr;             // 记录当前时间
    BOOL    bIsHaveSki = FALSE;     // 当前状态是否有异物
    INT     nRet = 0;
    INT     nLastRet = 0;
    ULONG   ululTimeCount = 0;      // 间隔时间

    CHAR    szResultStr[][32] = { "0:检知无异物", "1:检知有异物", "检知错误" };

    Log(ThisModule, __LINE__,
        "SIU: 异物检知进程[%ld]启动...", std::this_thread::get_id());

    while(1)
    {
        if (m_bThreadSkiExit == TRUE)
        {
            Log(ThisModule, __LINE__,
                "SIU: 异物检知进程[%ld]退出.", std::this_thread::get_id());
            break;
        }

        if (m_pSIUDev == nullptr)
        {
            Log(ThisModule, __LINE__,
                "SIU: 设备句柄(m_pSIUDev)为空, 结束进程[%ld].", std::this_thread::get_id());
            break;
        }

        nLastRet = nRet;
        nRet = m_pSIUDev->GetData(GET_SKIMMING_CREADER, nullptr);
        if (nRet != nLastRet)
        {
            Log(ThisModule, __LINE__,
                "SIU: 异物检知进程[%ld]: 获取异物检知结果变化[%s]->[%s].",
                std::this_thread::get_id(),
                nLastRet == 0 || nLastRet == 1 ? szResultStr[nLastRet] : szResultStr[2],
                nRet == 0 || nRet == 1 ? szResultStr[nRet] : szResultStr[2]);
        }

        if (nRet < 0)
        {
            if (nRet != nLastRet)
            {
                Log(ThisModule, __LINE__,
                    "SIU: 异物检知进程[%ld]: 获取异常: ->GetData(%d, NULL) Fail, ErrCode: %d",
                    std::this_thread::get_id(), GET_SKIMMING_CREADER, nRet);
            }
        } else
        if (nRet == 1)  // 检知异物
        {
            if (bIsHaveSki == FALSE)    // 第一次检知异物
            {
                bIsHaveSki = TRUE;
                qtTimeCurr = QTime::currentTime();
            } else  // 非第一次检知异物,检查时间间隔
            {
                if (m_bIsHaveSkimming != TRUE)
                {
                    if ((qtTimeCurr.msecsTo(QTime::currentTime()) / 1000) >=
                            m_stConfig.wSkimmingErrorDuration)  // 超出故障时间
                    {
                         m_bIsHaveSkimming = TRUE;  // 检知有异物
                         SendSkimmingNoticeToDev(TRUE); // 发送检知有异物通知
                         Log(ThisModule, __LINE__,
                             "SIU: 异物检知进程[%ld]: 检知有异物 : %d 秒内未清除, 设备故障.",
                             std::this_thread::get_id(), m_stConfig.wSkimmingErrorDuration);
                    }
                }
            }
        } else
        if (nRet == 0)  // 未检知异物
        {
            if (bIsHaveSki == TRUE)    // 第一次检知无异物
            {
                bIsHaveSki = FALSE;
                qtTimeCurr = QTime::currentTime();
            } else  // 非第一次检知异物,检查时间间隔
            {
                if (m_bIsHaveSkimming != FALSE)
                {
                    if ((qtTimeCurr.msecsTo(QTime::currentTime()) / 1000) >=
                            m_stConfig.wSkimmingNormalDuration)  // 超出正常时间
                    {
                         m_bIsHaveSkimming = FALSE;  // 检知无异物
                         SendSkimmingNoticeToDev(FALSE); // 发送检知无异物通知
                         Log(ThisModule, __LINE__,
                             "SIU: 异物检知进程[%ld]: 异物清除, %d 秒内检知无异物, 设备恢复.",
                             std::this_thread::get_id(), m_stConfig.wSkimmingNormalDuration);
                    }
                }
            }
        } else
        {
            if (nRet != nLastRet)
            {
                Log(ThisModule, __LINE__,
                    "SIU: 异物检知进程[%ld]: 获取未知应答: ->GetData(%d, NULL) Fail, ErrCode: %d",
                    std::this_thread::get_id(), GET_SKIMMING_CREADER, nRet);
            }
        }

        usleep(1000 * 1000 * m_stConfig.wSkimmingMonitorInterval);
    }
}

// 发送异物检知通知到DevXXX
INT CXFS_IDC::SendSkimmingNoticeToDev(BOOL bIsHave)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    WORD wNotice = (bIsHave == TRUE ? 1 : 0);
    m_pIDCDev->SetData(SET_SND_NOTICE_2DEV, &wNotice);

    return WFS_SUCCESS;
}

