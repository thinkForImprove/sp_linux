/***************************************************************
* 文件名称：XFS_IDC.cpp
* 文件描述：读卡器模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_IDC.h"

static const char *ThisFile = "XFS_IDC.cpp";
static const char *DEVTYPE = "IDC";
#define LOG_NAME     "XFS_IDC.log"

/*************************************************************************
// 主处理                                                                 *
*************************************************************************/
CXFS_IDC::CXFS_IDC() : m_clErrorDet((LPSTR)DEVTYPE)
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);        // 设置日志文件名和错误发生文件
    m_bChipPowerOff = FALSE;
    m_enWaitTaken = WTF_NONE;
    memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));
    memset(m_nRetErrOLD, 0, sizeof(INT) * 12);

    InitCRM();  // 变量初始化
    InitSIU();  // 变量初始化
}

CXFS_IDC::~CXFS_IDC()
{
    ;
}

// 开始运行
long CXFS_IDC::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseIDC
    if (m_pBase.Load("SPBaseIDC.dll", "CreateISPBaseIDC", DEVTYPE) != 0)
    {
        Log(ThisModule, __LINE__, "IDC: 加载SPBaseIDC失败");
        SetErrorDetail(0, (LPSTR)EC_XFS_SPBaseLoadFail);
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();

    return 0;
}

//-----------------------------基本接口----------------------------------
// Open命令入口
HRESULT CXFS_IDC::OnOpen(LPCSTR lpLogicalName)
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
    if (m_stConfig.wDeviceType != XFS_CRT350N)// &&
        //m_stConfig.wDeviceType != XFS_EC2G)
    {
        Log(ThisModule, __LINE__, "IDC: Open fail, INI指定了不支持的设备类型[%d], Return :%d.",
            m_stConfig.wDeviceType, WFS_ERR_DEV_NOT_READY);
        SetErrorDetail(0, (LPSTR)EC_XFS_DevNotSupp);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 加载DevXXX动态库
    if (LoadDevIDCDll(ThisModule) != TRUE)
    {
        return WFS_ERR_INTERNAL_ERROR;
    }

    // IDC Open前启动退卡模块,避免IDC启动错误造成退卡模块不执行Open
    if (m_stCRMConfig.bCRMDeviceSup == TRUE)     // 支持启用退卡模块
    {
        HRESULT hCRMRet = StartOpenCRM();
        if (hCRMRet != WFS_SUCCESS)
        {
            if (m_stCRMConfig.wOpenFailRet == 0)
            {
                Log(ThisModule, __LINE__,
                    "IDC: 退卡模块启动: ->StartOpenCRM() Fail, ErrCode: %d, Return: %d.",
                    hCRMRet, hCRMRet);
                return hCRMRet;
            } else
            {
                Log(ThisModule, __LINE__,
                    "IDC: 退卡模块启动: ->StartOpenCRM() Fail, ErrCode: %d, INI配置OpenFailRet=%d, Not Return.",
                    hCRMRet, m_stCRMConfig.wOpenFailRet);
            }
        }
    } else
    {
        Log(ThisModule, __LINE__, "IDC: INI指定不支持的启用CRM设备.",
            WFS_SUCCESS);
    }

    // IDC Open前启动异物检知SIU模块
    HRESULT hSIURet = StartOpenSIU();
    if (hSIURet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "IDC: 异物检知(SIU)启动: ->StartOpenSIU() Fail, ErrCode: %d, Not Return.",
            hSIURet);
    } else
    {
        Log(ThisModule, __LINE__,
            "IDC: 异物检知(SIU)启动: ->StartOpenSIU() Succ.");

        // 启动异物检知进程
        m_bThreadSkiExit = FALSE;               // 通知异物检知进程退出(不退出)
        if (!m_thRunSkimming.joinable())
        {
            m_thRunSkimming = std::thread(&CXFS_IDC::ThreadSkimming, this);
            if (m_thRunSkimming.joinable())
            {
                m_thRunSkimming.detach();   // 进程分离
            }
        }
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = InnerOpen();
    if (m_stConfig.wOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    // Open后复位
    if (hRet == WFS_SUCCESS && m_stConfig.wOpenResetCardAction > 0)   // 支持
    {
        Log(ThisModule, __LINE__, "IDC: 执行Open后复位...");
        InnerOpenReset();
    }

    return hRet;
}

HRESULT CXFS_IDC::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pIDCDev != nullptr)
    {
        m_pIDCDev->Close();
        m_pIDCDev = nullptr;
    }

    EndCloseCRM();  // CRM结束
    EndCloseSIU();  // SIU结束

    return WFS_SUCCESS;
}

// 状态实时获取
HRESULT CXFS_IDC::OnStatus()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;
    UpdateDeviceStatus();
    if(m_stStatus.fwDevice == WFS_IDC_DEVOFFLINE)
    {
        if (m_bCRMIsOnLine != TRUE)
        {
            SetErrorDetail(0, (LPSTR)EC_XFS_DevOffLine);
        } else
        {
            SetErrorDetail(0, (LPSTR)EC_IDC_XFS_CRMOffLine);
        }

        // 断线自动重连
        m_pIDCDev->SetData(SET_DEV_RECON, nullptr);    // 设置为断线重连执行状态
        hRet = InnerOpen(TRUE);                     // 执行断线重连
        if (hRet != WFS_SUCCESS)
        {
            if (m_nRetErrOLD[0] != hRet)
            {
                Log(ThisModule, __LINE__,
                    "IDC: 断线重连中: OnStatus->InnerOpen(%d) fail, ErrCode: %d.", TRUE, hRet);
                m_nRetErrOLD[0] = hRet;
            }
        } else
        {
            if (m_nRetErrOLD[0] != hRet)
            {
                Log(ThisModule, __LINE__,
                    "IDC: 断线重连完成: OnStatus->InnerOpen(%d) succ, RetCode:%d.", TRUE, hRet);
                m_nRetErrOLD[0] = hRet;
            }
        }
    }

    // CRM 状态及断线重连
    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        // 断线中, 自动重连
        if (m_bCRMIsOnLine == FALSE)
        {
            if (m_bCRMIsOnLine != TRUE)
            {
                SetErrorDetail(0, (LPSTR)EC_IDC_XFS_CRMOffLine);
            } else
            {
                SetErrorDetail(0, (LPSTR)EC_CRM_XFS_DevOffLine);
            }

            m_pCRMDev->SetData(SET_DEV_RECON, nullptr);    // 设置为断线重连执行状态
            hRet = InnerOpenCRM(TRUE);                     // 执行断线重连
            if (hRet != WFS_SUCCESS)
            {
                if (m_nRetErrOLD[1] != hRet)
                {
                    Log(ThisModule, __LINE__,
                        "CRM: 断线重连中: OnStatus->InnerOpenCRM(%d) fail, ErrCode: %d.", TRUE, hRet);
                    m_nRetErrOLD[1] = hRet;
                }
            } else
            {
                if (m_nRetErrOLD[1] != hRet)
                {
                    Log(ThisModule, __LINE__,
                        "CRM: 断线重连完成: OnStatus->InnerOpenCRM(%d) succ, ErrCode: %d.", TRUE, hRet);
                    m_nRetErrOLD[1] = hRet;
                }
            }
        }
    }

    return WFS_SUCCESS;
}

// Taken事件状态实时获取
HRESULT CXFS_IDC::OnWaitTaken()
{
    if (m_enWaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    UpdateDeviceStatus();
    return WFS_SUCCESS;
}

// 取消命令
HRESULT CXFS_IDC::OnCancelAsyncRequest()
{
    if (m_pIDCDev != nullptr)
    {
        m_pIDCDev->Cancel();
    }
    return WFS_SUCCESS;
}

// 固件升级
HRESULT CXFS_IDC::OnUpdateDevPDL()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return WFS_IDC_NOTSUPP;
}

//----------------------------IDC类型接口----------------------------
// 查询状态
HRESULT CXFS_IDC::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_stConfig.wRetainSupp == 1)
    {
        m_stStatus.usCards = m_stConfig.wRetainCardCount;   // 吞卡计数
    } else
    {
        m_stStatus.usCards = 0; // 不支持吞卡,计数为0
    }

    m_cStatExtra.AddExtra("ErrorDetail", m_clErrorDet.GetSPErrCode());
    m_cStatExtra.AddExtra("LastErrorDetail", m_clErrorDet.GetSPErrCodeLast());

    m_stStatus.lpszExtra = (LPSTR)m_cStatExtra.GetExtra();
    lpStatus = &m_stStatus;
    return WFS_SUCCESS;
}

// 查询能力值
HRESULT CXFS_IDC::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_stCaps.lpszExtra = (LPSTR)m_cCapsExtra.GetExtra();
    lpCaps = &m_stCaps;
    return WFS_SUCCESS;
}

// 取Form名列表
HRESULT CXFS_IDC::GetFormList(LPSTR &lpFormList)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
    {
        hRet = WFS_SUCCESS;
    }
    lpFormList = (LPSTR)(LPCSTR)m_FormNames;

    return hRet;
}

// 指定Form名取Form属性
HRESULT CXFS_IDC::GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_UNSUPP_COMMAND;
    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
    {
        if (lpFormName == nullptr)
        {
            hRet = WFS_ERR_INVALID_DATA;
        } else
        {
            SP_IDC_FORM *pForm = FLFind(m_FormList, lpFormName);
            if (!pForm)
            {
                hRet = WFS_ERR_IDC_FORMNOTFOUND;
            } else
            if (!pForm->bLoadSucc)
            {
                hRet = WFS_ERR_IDC_FORMINVALID;
            } else
            {
                hRet = CheackFormInvalid(m_FormList, (char *)pForm->FormName.c_str(), pForm->fwAction);
                if (hRet == WFS_SUCCESS)
                {
                    m_LastForm.lpszFormName             = (char *)pForm->FormName.c_str();
                    m_LastForm.cFieldSeparatorTrack1    = pForm->cFieldSeparatorTracks[0];
                    m_LastForm.cFieldSeparatorTrack2    = pForm->cFieldSeparatorTracks[1];
                    m_LastForm.cFieldSeparatorTrack3    = pForm->cFieldSeparatorTracks[2];
                    m_LastForm.fwAction                 = pForm->fwAction;
                    m_LastForm.lpszTracks                   = (char *)pForm->sTracks.c_str();
                    m_LastForm.bSecure                  = pForm->bSecures[0] | pForm->bSecures[1] | pForm->bSecures[2];
                    m_LastForm.lpszTrack1Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[0];
                    m_LastForm.lpszTrack2Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[1];
                    m_LastForm.lpszTrack3Fields         = (LPSTR)(LPCSTR)pForm->szTrackFields[2];
                    lpForm = &m_LastForm;
                }
            }
        }
    }
    return hRet;
}

// Form读卡
HRESULT CXFS_IDC::ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    LoadFormFile(m_strSPName.c_str(), m_FormList);

    HRESULT hRet = CheackFormInvalid(m_FormList, (char *)lpFormName, WFS_IDC_ACTIONREAD);
    if (hRet == WFS_SUCCESS)
    {
        SP_IDC_FORM *pForm = nullptr;
        const char *pFormName = (char *)lpFormName;
        hRet = SPGetIDCForm(m_FormList, pFormName, pForm, WFS_IDC_ACTIONREAD);
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "Form读卡: ->GetIDCForm(%s) Fail, Return: %d", pFormName);
            return hRet;
        } else
        {
            DWORD dwOption = TracksToDataSourceOption(pForm->szTracks);
            if (IsDoSecureCheck(pForm, dwOption))
            {
                dwOption |= WFS_IDC_SECURITY;
            }
            m_TrackData = CMultiMultiString();
            m_CardDatas.Clear();
            hRet = InnerAcceptAndReadTrack(dwOption, m_stConfig.dwInCardTimeOut);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "Form读卡: ->InnerAcceptAndReadTrack(%d, %d) Fail, Return: %d");
                return hRet;
            }
        }

        if (hRet == WFS_SUCCESS)
        {
            hRet = SPParseData(pForm);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "Form读卡: 处理应答数据: SPParseData() Fail, Return: %d");
                return hRet;
            }
        }

    }

    return WFS_SUCCESS;
}

// 退卡
HRESULT CXFS_IDC::EjectCard(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    UpdateDeviceStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 检查介质状态
    if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)  // 无卡
    {
        if (m_stConfig.wPostRemovedAftEjectFixed == 1)
        {
            FireMediaRemoved(); // 事件上报
        }
        Log(ThisModule, __LINE__, "IDC: 退卡: 没有检测到卡, Return: %d", WFS_SUCCESS);
        SetErrorDetail(0, (LPSTR)EC_XFS_MedNotFound);
        return WFS_ERR_IDC_NOMEDIA;
    } else
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)    // 卡在出口
    {
        m_stStatus.fwMedia = WFS_IDC_MEDIAENTERING;
        m_enWaitTaken = WTF_TAKEN;
        Log(ThisModule, __LINE__, "IDC: 退卡: 卡在出口, Return: %d", WFS_SUCCESS);
    } else
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT)     // 卡在内部
    {
        hRet = InnerEject(); // 执行退卡
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__, "IDC: 退卡: ->InnerEject() Fail, ErrCode: %d, Return: %d",
                hRet, hRet);
            return hRet;
        }

        m_stStatus.fwMedia = WFS_IDC_MEDIAENTERING;
        m_enWaitTaken = WTF_TAKEN;
    }

    return WFS_SUCCESS;
}

// 吞卡
long CXFS_IDC::RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    if (m_stConfig.wRetainSupp == 0)    // 不支持吞卡
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    // 更新当前设备介质状态
    UpdateDeviceStatus();

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 无卡状态检查
    CHK_MEDIA_ISHAVE(m_stStatus.fwMedia);

    // 回收箱满状态检查
    CHK_RETAIN_ISFULL(m_stStatus.fwRetainBin);

    // 回收盒检查(INI)
    if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainFull)     // 回收满
    {
        Log(ThisModule, __LINE__,
            "吞卡: 当前回收计数[%d] >= INI设置回收警阀值[%d], Return: %d.",
            m_stConfig.wRetainCardCount, m_stConfig.wRetainFull, WFS_ERR_IDC_RETRACTBINFULL);
        SetErrorDetail(0, (LPSTR)EC_XFS_BoxFull);
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

// 回收计数清零
HRESULT CXFS_IDC::ResetCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_stConfig.wRetainSupp == 0)    // 不支持吞卡
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    // 设置清零及INI
    UpdateRetainCards(RETCNT_CLEAR);

    // 更新当前设备介质状态
    UpdateDeviceStatus();

    // 清除硬件回收计数
    m_pIDCDev->SetData(SET_DEV_RETAINCNT);

    return WFS_SUCCESS;
}

// 读卡
HRESULT CXFS_IDC::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wReadDataChk = WFS_IDC_TRACK1 |        // 0x0001
                        WFS_IDC_TRACK2 |        // 0x0002
                        WFS_IDC_TRACK3 |        // 0x0004
                        WFS_IDC_CHIP   |        // 0x0008
                        WFS_IDC_SECURITY |      // 0x0010(安全校验)
                        WFS_IDC_FLUXINACTIVE |  // 0x0020(无磁芯片卡不报错)
                        //WFS_IDC_TRACK_WM |      // 0x8000
                        //WFS_IDC_MEMORY_CHIP |   // 0x0040
                        WFS_IDC_FRONTIMAGE |    // 0x0100
                        WFS_IDC_BACKIMAGE;      // 0x0200

    WORD wOption = *(WORD *)lpReadData;

    UpdateDeviceStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 卡JAM状态检查
    CHK_MEDIA_ISJAM(m_stStatus.fwMedia);

    // 入参Check
    if ((wOption & wReadDataChk) == 0)
    {
        Log(ThisModule, __LINE__, "IDC: 读卡: 入参lpReadData[%d] 无效, Return: %d",
            wOption, WFS_ERR_INVALID_DATA);
        SetErrorDetail(0, (LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    m_CardDatas.Clear();
    lppCardData = nullptr;
    HRESULT hRet = InnerAcceptAndReadTrack(wOption, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "IDC: 读卡: ->InnerAcceptAndReadTrack(%d, %d) Fail, Return: %d",
            wOption, dwTimeOut, hRet);
        return hRet;
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_CardDatas;

    return WFS_SUCCESS;
}

// 芯片读写
HRESULT CXFS_IDC::ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_ChipIO.wChipProtocol = lpChipIOIn->wChipProtocol;
    m_ChipIO.ulChipDataLength = MAX_CHIP_IO_LEN;

    // 入参: 通信协议Check
    if (lpChipIOIn->wChipProtocol != WFS_IDC_CHIPT0  && lpChipIOIn->wChipProtocol != WFS_IDC_CHIPT1)
    {
        Log(ThisModule, __LINE__, "IDC: 芯片读写: 入参wChipProtocol[%d]无效, Return: %d",
            lpChipIOIn->wChipProtocol, WFS_ERR_IDC_PROTOCOLNOTSUPP);
        SetErrorDetail(0, (LPSTR)EC_IDC_XFS_PortNotSupp);
        return WFS_ERR_IDC_PROTOCOLNOTSUPP;
    }

    UpdateDeviceStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 检查介质状态
    if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)  // 无卡
    {
        Log(ThisModule, __LINE__,
            "芯片读写: Media Status = %d|MEDIANOTPRESENT, 没有检测到卡, Return: %d",
            m_stStatus.fwMedia, WFS_ERR_IDC_NOMEDIA);
        SetErrorDetail(0, (LPSTR)EC_XFS_MedNotFound);
        return WFS_ERR_IDC_NOMEDIA;
    }
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)
    {
        Log(ThisModule, __LINE__,
            "芯片读写: Media Status = %d|MEDIAENTERING, 卡在出口, Return: %d",
            m_stStatus.fwMedia, WFS_ERR_IDC_ATRNOTOBTAINED);
        SetErrorDetail(0, (LPSTR)EC_XFS_MedIsExport);
        return WFS_ERR_IDC_ATRNOTOBTAINED;
    }
    if (m_stStatus.fwChipPower != WFS_IDC_CHIPONLINE)
    {
        Log(ThisModule, __LINE__,
            "芯片读写: ChipPower Status[%d] != %d|CHIPONLINE, 芯片非通电状态, Return: %d",
            m_stStatus.fwChipPower, WFS_IDC_CHIPONLINE, WFS_ERR_IDC_INVALIDMEDIA);
        SetErrorDetail(0, (LPSTR)EC_IDC_XFS_ChipPowerOff);
        return WFS_ERR_IDC_INVALIDMEDIA;
    }

    // 指定芯片通信协议
    CHIP_RW_MODE enChipMode = CHIP_IO_T0;   // 缺省T0协议
    if (lpChipIOIn->wChipProtocol == WFS_IDC_CHIPT0)
    {
        enChipMode = CHIP_IO_T0;
    } else
    if (lpChipIOIn->wChipProtocol == WFS_IDC_CHIPT1)
    {
        enChipMode = CHIP_IO_T1;
    }

    // 开始芯片通信
    STCHIPRW stChipRead;
    memcpy(stChipRead.stData[0].szData, lpChipIOIn->lpbChipData, lpChipIOIn->ulChipDataLength);
    stChipRead.stData[0].dwSize = lpChipIOIn->ulChipDataLength;
    stChipRead.stData[1].dwSize = sizeof(stChipRead.stData[1].szData);
    INT nRet = m_pIDCDev->ChipReadWrite(enChipMode, stChipRead);
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "芯片读写: ->ChipReadWrite(%d) Fail, ErrCode: %d, Return: %d",
            enChipMode, nRet, ConvertDevErrCode2WFS(nRet));
        SetErrorDetail();
        return ConvertDevErrCode2WFS(nRet);
    }

    // 组织应答数据
    m_ChipIO.ulChipDataLength = stChipRead.stData[1].dwSize;
    memcpy(m_ChipIO.lpbChipData, stChipRead.stData[1].szData, stChipRead.stData[1].dwSize);
    lpChipIOOut = &m_ChipIO;

    return WFS_SUCCESS;
}

// 复位
HRESULT CXFS_IDC::Reset(LPWORD lpResetIn)
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
        SetErrorDetail(0, (LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    HRESULT hRet = InnerReset(wAction);
    if (hRet == WFS_SUCCESS)
    {
        m_clErrorDet.SetErrCodeInit();
    }
    if (m_stConfig.wResetFailReturn == 0)   // Reset结果原样返回
    {
        return hRet;
    } else
    {
        return WFS_SUCCESS;
    }


    // 支持CRM时,先执行CRMReset
    /*if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__, "IDC: CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 执行设备复位
        INT nCRMRet = CRM_SUCCESS;
        if ((nCRMRet = m_pCRMDev->CMReset()) != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__, "IDC: Reset: CRM Device Reset: ->CMReset() Fail, "
                                "CRM RetCode: %d, Return: %d.",
                nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        m_bIsSetCardData = FALSE;     // RetainCard前需要执行SetCardData命令
        memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));
    }*/



    // CRT310
    /*if (nRet == ERR_IDC_ICRW_ERROR) {
        Log(ThisModule, __LINE__, "IDC: 复位吞卡(卡被取走，返回码会被重置为设备故障)");
        nRet = ERR_IDC_HWERR;
    } else if (nRet == ERR_IDC_JAMMED) {
        nRet = WFS_ERR_IDC_MEDIAJAM;
    } else {
        nRet = WFS_ERR_HARDWARE_ERROR;
    }

    Log(ThisModule, __LINE__, "IDC: 复位失败:(%d)%s", nRet, ProcessReturnCode(nRet));
    return nRet;*/
}

// 芯片上电
HRESULT CXFS_IDC::ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_bChipPowerOff = FALSE;
    m_ChipPower.Clear();
    WORD wPower = * lpChipPower;
    STCHIPRW stChipData;
    INT nRet = IDC_SUCCESS;

    // 入参: 上电参数Check
    if (wPower != WFS_IDC_CHIPPOWERCOLD && wPower != WFS_IDC_CHIPPOWERWARM &&
        wPower != WFS_IDC_CHIPPOWEROFF)
    {
        Log(ThisModule, __LINE__, "IDC: 芯片上电: 入参ChipPower[%d]无效, Return: %d",
            wPower, WFS_ERR_INVALID_DATA);
        SetErrorDetail(0, (LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    UpdateDeviceStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 无卡状态检查
    CHK_MEDIA_ISHAVE(m_stStatus.fwMedia);

    // 检查芯片状态
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPNODEVICE)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPNODEVICE, 卡没有芯片或芯片有问题, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_IDC_INVALIDMEDIA);
        SetErrorDetail(0, (LPSTR)EC_XFS_MedNotFound);
        return WFS_ERR_IDC_INVALIDMEDIA;
    } else
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPNOCARD)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPNOCARD, 芯片卡不存在, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_IDC_NOMEDIA);
        SetErrorDetail(0, (LPSTR)EC_XFS_MedNotFound);
        return WFS_ERR_IDC_NOMEDIA;
    } else
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPHWERROR)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPHWERROR, 芯片卡故障, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_HARDWARE_ERROR);
        SetErrorDetail(0, (LPSTR)EC_IDC_XFS_ChipPowerOff);
        return WFS_ERR_HARDWARE_ERROR;
    } else
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPUNKNOWN)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPUNKNOWN, 芯片卡状态未知, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_HARDWARE_ERROR);
        SetErrorDetail(0, (LPSTR)EC_IDC_XFS_ChipUnknown);
        return WFS_ERR_HARDWARE_ERROR;
    } else
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPNOTSUPP)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPNOTSUPP, 设备不支持芯片卡操作, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_UNSUPP_COMMAND);
        SetErrorDetail(0, (LPSTR)EC_IDC_XFS_ChipNotSupp);
        return WFS_ERR_UNSUPP_COMMAND;
    } else
    if (m_stStatus.fwChipPower  == WFS_IDC_CHIPONLINE ||    // 设备上电
        m_stStatus.fwChipPower  == WFS_IDC_CHIPBUSY)        // 运行中
    {
        if (wPower == WFS_IDC_CHIPPOWERCOLD)    // 入参冷复位: 已上电状态需要先断电
        {
            stChipData.Clear();
            nRet = m_pIDCDev->ChipReadWrite(CHIP_POW_OFF, stChipData);
            if (nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "芯片上电: 冷复位: 当前处于已激活状态, 执行断电: ->ChipReadWrite(%d|OFF) Fail, ErrCode: %d, Return: %d",
                    CHIP_POW_OFF, nRet, ConvertDevErrCode2WFS(nRet));
                SetErrorDetail();
                return ConvertDevErrCode2WFS(nRet);
            }
        }
    }
    else // WFS_IDC_CHIPPOWEREDOFF,断电状态
    {
        if (wPower == WFS_IDC_CHIPPOWERWARM)    // 热复位,执行冷复位方式(先上电后复位)
        {

            stChipData.Clear();
            nRet = m_pIDCDev->ChipReadWrite(CHIP_POW_COLD, stChipData);
            if (nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "芯片上电: 热复位: 当前处于未激活状态, 执行上电: ->ChipReadWrite(%d|COLD) Fail, ErrCode: %d, Return: %d",
                    CHIP_POW_COLD, nRet, ConvertDevErrCode2WFS(nRet));
                SetErrorDetail();
                return ConvertDevErrCode2WFS(nRet);
            }
        } else
        if (wPower == WFS_IDC_CHIPPOWEROFF)
        {
            m_ChipPower.Clear();
            lpData = m_ChipPower.GetData();
            return WFS_SUCCESS;
        }
    }

    if (wPower == WFS_IDC_CHIPPOWERCOLD)
    {
        stChipData.Clear();
        nRet = m_pIDCDev->ChipReadWrite(CHIP_POW_COLD, stChipData);
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片上电: 冷复位: ->ChipReadWrite(%d|COLD) Fail, ErrCode: %d, Return: %d",
                CHIP_POW_COLD, nRet, ConvertDevErrCode2WFS(nRet));
            SetErrorDetail();
            return ConvertDevErrCode2WFS(nRet);
        }
        m_ChipPower.SetData(stChipData.stData[1].szData, stChipData.stData[1].dwSize);
    } else
    if (wPower == WFS_IDC_CHIPPOWERWARM)
    {
        stChipData.Clear();
        nRet = m_pIDCDev->ChipReadWrite(CHIP_POW_WARM, stChipData);
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片上电: 热复位: ->ChipReadWrite(%d|WARM) Fail, ErrCode: %d, Return: %d",
                CHIP_POW_WARM, nRet, ConvertDevErrCode2WFS(nRet));
            SetErrorDetail();
            return ConvertDevErrCode2WFS(nRet);
        }
        m_ChipPower.SetData(stChipData.stData[1].szData, stChipData.stData[1].dwSize);
    } else
    if (wPower == WFS_IDC_CHIPPOWEROFF)
    {
        stChipData.Clear();
        nRet = m_pIDCDev->ChipReadWrite(CHIP_POW_OFF, stChipData);
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片断电: ->ChipReadWrite(%d|OFF) Fail, ErrCode: %d, Return: %d",
                CHIP_POW_OFF, nRet, ConvertDevErrCode2WFS(nRet));
            SetErrorDetail();
            return ConvertDevErrCode2WFS(nRet);
        }
        m_ChipPower.Clear();
    }

    lpData = m_ChipPower.GetData();

    return WFS_SUCCESS;
}

// 接收表单明和ReadRawData命令输出,并返回已解析的字符串
HRESULT CXFS_IDC::ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    SP_IDC_FORM *pForm = nullptr;
    LPWFSIDCPARSEDATA pParse = (LPWFSIDCPARSEDATA)lpDataIn;
    long hRes = SPGetIDCForm(m_FormList, pParse->lpstrFormName, pForm, WFS_IDC_ACTIONREAD);
    if (hRes != WFS_SUCCESS)
    {
        LogWrite(ThisModule, -1, IDS_GET_FORM_FAILED, pParse->lpstrFormName, hRes);
    }
    else
    {
        //保存以前的卡数据
        CWFSIDCCardDataPtrArray CardDatas;
        CardDatas = m_CardDatas;

        //拷贝新的数据
        m_CardDatas = pParse->lppCardData;

        //分析数据
        hRes = SPParseData(pForm);
        if (hRes != WFS_SUCCESS)
        {
            LogWrite(ThisModule, hRes, IDS_PARSE_DATA_FAILED, pParse->lpstrFormName, "");
        }
        //恢复老的数据
        m_CardDatas = CardDatas;
    }
    lpTrackData = (LPSTR)(LPCSTR)m_TrackData;
    return WFS_SUCCESS;
}

