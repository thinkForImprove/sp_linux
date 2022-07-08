/***************************************************************
* 文件名称：XFS_FIDC.cpp
* 文件描述：非接读卡器模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_FIDC.h"

static const char *DEVTYPE = "FIDC";
static const char *ThisFile = "XFS_FIDC.cpp";
#define LOG_NAME     "XFS_FIDC.log"

/*************************************************************************
// 主处理                                                                 *
*************************************************************************/
CXFS_FIDC::CXFS_FIDC()
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);
    m_bChipPowerOff = FALSE;
    m_bMultiCard = FALSE;
    m_ulTakeMonitorStartTime = 0;
    m_nReConErr = IDC_SUCCESS;                  // 断线重连错误码记录
}

CXFS_FIDC::~CXFS_FIDC()
{
    OnClose();
}

long CXFS_FIDC::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseIDC
    if (0 != m_pBase.Load("SPBaseIDC.dll", "CreateISPBaseIDC", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseIDC失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

//-----------------------------基本接口----------------------------------
// Open命令入口
HRESULT CXFS_FIDC::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    InitConfig();
    InitCaps();
    InitStatus();

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 检查INI设置设备类型
    if (m_stConfig.wDeviceType != XFS_MT50 &&
        //m_stConfig.wDeviceType != XFS_CJ201 &&
        m_stConfig.wDeviceType != XFS_TMZ &&
        m_stConfig.wDeviceType != XFS_CRT603CZ7)
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定了不支持的设备类型[%d], Return :%d.",
            m_stConfig.wDeviceType, WFS_ERR_DEV_NOT_READY);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 加载DevXXX动态库
    if (!LoadDevDll(ThisModule))
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

HRESULT CXFS_FIDC::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    ControlLight(GLIGHTS_ACT_CLOSE);

    if (m_pDev != nullptr)
    {
        m_pDev->Cancel();
        m_pDev->Close();
    }
    return WFS_SUCCESS;
}

// 状态实时更新
HRESULT CXFS_FIDC::OnStatus()
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;
    UpdateDeviceStatus();
    if(m_stStatus.fwDevice == WFS_IDC_DEVOFFLINE)
    {
        // 断线自动重连
        m_pDev->SetData(SET_DEV_RECON, nullptr);    // 设置为断线重连执行状态
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
        }
    }
    return WFS_SUCCESS;
}

// Taken事件实时检查
HRESULT CXFS_FIDC::OnWaitTaken()
{
    if (m_WaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    UpdateDeviceStatus();
    return WFS_SUCCESS;
}

// 取消命令
HRESULT CXFS_FIDC::OnCancelAsyncRequest()
{
    if (m_pDev != nullptr)
    {
        m_pDev->Cancel();
    }
    return WFS_SUCCESS;
}

// 固件升级
HRESULT CXFS_FIDC::OnUpdateDevPDL()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return WFS_IDC_NOTSUPP;
}

//----------------------------IDC类型接口----------------------------
// 查询状态
HRESULT CXFS_FIDC::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_stStatus.lpszExtra = (LPSTR)m_cStaExtra.GetExtra();
    lpStatus = &m_stStatus;
    return WFS_SUCCESS;
}

// 能力值
HRESULT CXFS_FIDC::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    m_Caps.lpszExtra = (LPSTR)m_cCapExtra.GetExtra();
    lpCaps = &m_Caps;
    return WFS_SUCCESS;
}

HRESULT CXFS_FIDC::GetFormList(LPSTR &lpFormList)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_HARDWARE_ERROR;
    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
    {
        hRet = WFS_SUCCESS;
    }
    lpFormList = (LPSTR)(LPCSTR)m_FormNames;
    //LPSTR szTest = (LPSTR)lpFormList;
    return hRet;
}

HRESULT CXFS_FIDC::GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_ERR_HARDWARE_ERROR;
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

// 退卡
HRESULT CXFS_FIDC::EjectCard(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    UpdateDeviceStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 检查介质状态
    if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)  // 无卡
    {
        if (m_stConfig.bPostRemovedAftEjectFixed == TRUE)
        {
            FireMediaRemoved(); // 事件上报
        }
        Log(ThisModule, __LINE__, "退卡: 没有检测到卡, Return: %d", WFS_SUCCESS);
        return WFS_SUCCESS;
    } else
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING ||  // 卡在出口
        m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT)     // 卡在内部
    {
        nRet = m_pDev->MediaControl(MEDIA_EJECT);     // 执行退卡
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__, "退卡: ->MeidaControl(%d) Fail, ErrCode: %d, Return: %d",
                MEDIA_EJECT, nRet, ConvertDevErrCode2WFS(nRet));
            return ConvertDevErrCode2WFS(nRet);
        }

        ControlLight(GLIGHTS_ACT_FLASH);  // 灯状态变为闪烁
        m_stStatus.fwMedia = WFS_IDC_MEDIAENTERING;
        m_WaitTaken = WTF_TAKEN;
        m_ulTakeMonitorStartTime = CQtTime::GetSysTick();
        return WFS_SUCCESS;
    }

    return WFS_SUCCESS;
}

// 读卡
HRESULT CXFS_FIDC::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wReadDataChk = WFS_IDC_TRACK1 |        // 0x0001
                        WFS_IDC_TRACK2 |        // 0x0002
                        WFS_IDC_TRACK3 |        // 0x0004
                        WFS_IDC_CHIP   |        // 0x0008
                        //WFS_IDC_SECURITY |      // 0x0010
                        //WFS_IDC_FLUXINACTIVE |  // 0x0020
                        //WFS_IDC_TRACK_WM |      // 0x8000
                        //WFS_IDC_MEMORY_CHIP |   // 0x0040
                        WFS_IDC_FRONTIMAGE |    // 0x0100
                        WFS_IDC_BACKIMAGE;      // 0x0200

    WORD wOption = *(WORD *)lpReadData;

    UpdateDeviceStatus(); // 更新当前设备介质状态

    // 检查设备状态
    DEV_STAT_RET_HWERR(m_stStatus.fwDevice);

    // 入参Check
    if ((wOption & wReadDataChk) == 0)
    {
        Log(ThisModule, __LINE__, "读卡: 入参lpReadData[%d] 无效, Return: %d",
            wOption, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    m_ulTakeMonitorStartTime = 0;

    m_cCardData.Clear();
    lppCardData = nullptr;
    HRESULT hRet = InnerReadRawData(wOption, dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡: ->InnerReadRawData(%d, %d) Fail, Return: %d",
            wOption, dwTimeOut, hRet);
        return hRet;
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_cCardData;

    return WFS_SUCCESS;
}

// 芯片读写
HRESULT CXFS_FIDC::ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_ChipIO.wChipProtocol = lpChipIOIn->wChipProtocol;
    m_ChipIO.ulChipDataLength = MAX_CHIP_IO_LEN;

    // 入参: 通信协议Check
    if (lpChipIOIn->wChipProtocol != WFS_IDC_CHIPT0  && lpChipIOIn->wChipProtocol != WFS_IDC_CHIPT1)
    {
        Log(ThisModule, __LINE__, "芯片读写: 入参wChipProtocol[%d]无效, Return: %d",
            lpChipIOIn->wChipProtocol, WFS_ERR_IDC_PROTOCOLNOTSUPP);
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
        return WFS_ERR_IDC_NOMEDIA;
    }
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)
    {
        Log(ThisModule, __LINE__,
            "芯片读写: Media Status = %d|MEDIAENTERING, 卡在出口, Return: %d",
            m_stStatus.fwMedia, WFS_ERR_IDC_ATRNOTOBTAINED);
        return WFS_ERR_IDC_ATRNOTOBTAINED;
    }
    if (m_stStatus.fwChipPower != WFS_IDC_CHIPONLINE)
    {
        Log(ThisModule, __LINE__,
            "芯片读写: ChipPower Status[%d] != %d|CHIPONLINE, 芯片非通电状态, Return: %d",
            m_stStatus.fwChipPower, WFS_IDC_CHIPONLINE, WFS_ERR_IDC_INVALIDMEDIA);
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
    INT nRet = m_pDev->ChipReadWrite(enChipMode, stChipRead);
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "芯片读写: ->ChipReadWrite(%d) Fail, ErrCode: %d, Return: %d",
            enChipMode, nRet, ConvertDevErrCode2WFS(nRet));
        return ConvertDevErrCode2WFS(nRet);
    }

    // 组织应答数据
    m_ChipIO.ulChipDataLength = stChipRead.stData[1].dwSize;
    memcpy(m_ChipIO.lpbChipData, stChipRead.stData[1].szData, stChipRead.stData[1].dwSize);
    lpChipIOOut = &m_ChipIO;

    return WFS_SUCCESS;
}

// 设备复位
HRESULT CXFS_FIDC::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    ControlLight(GLIGHTS_ACT_CLOSE);  // 非接灯复原

    WORD wAction = *lpResetIn;
    INT nRet = IDC_SUCCESS;

    // 没有动作参数时由SP决定
    if (wAction == 0)
    {
        Log(ThisModule, __LINE__, "复位: 入参ResetIn = 0, 设置默认值为 %d|RETAIN",
            WFS_IDC_RETAIN);
        wAction = WFS_IDC_RETAIN;
    }

    if (wAction != WFS_IDC_NOACTION &&
        wAction != WFS_IDC_RETAIN &&
        wAction != WFS_IDC_EJECT)
    {
        Log(ThisModule, __LINE__, "复位: 入参ResetIn[%d]无效, Return: %d",
            WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    MEDIA_ACTION enActFlag;
    if (wAction == WFS_IDC_RETAIN)
    {
        enActFlag = MEDIA_RETRACT;
    } else
    if (wAction == WFS_IDC_NOACTION)
    {
        enActFlag = MEDIA_NOTACTION;
    } else
    if (wAction == WFS_IDC_EJECT)
    {
        enActFlag = MEDIA_EJECT;
    }

    // 设备初始化
    nRet = m_pDev->Reset(enActFlag);       // 初始化介质无动作
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__, "复位: ->Init(%d) Fail, ErrCode: %d, Return: %d",
            enActFlag, nRet, ConvertDevErrCode2WFS(nRet));
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        Log(ThisModule, __LINE__, "复位: ->Init(%d) Succ",
            enActFlag);
    }

    return WFS_SUCCESS;
}

// 芯片上电
HRESULT CXFS_FIDC::ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData)
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
        Log(ThisModule, __LINE__, "芯片上电: 入参ChipPower[%d]无效, Return: %d",
            wPower, WFS_ERR_INVALID_DATA);
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
        return WFS_ERR_IDC_INVALIDMEDIA;
    } else
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPNOCARD)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPNOCARD, 芯片卡不存在, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_IDC_NOMEDIA);
        return WFS_ERR_IDC_NOMEDIA;
    } else
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPHWERROR)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPHWERROR, 芯片卡故障, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    } else
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPUNKNOWN)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPUNKNOWN, 芯片卡状态未知, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    } else
    if (m_stStatus.fwChipPower == WFS_IDC_CHIPNOTSUPP)
    {
        Log(ThisModule, __LINE__,
            "芯片上电: ChipPower Status = %d|CHIPNOTSUPP, 设备不支持芯片卡操作, Return: %d",
            m_stStatus.fwChipPower, WFS_ERR_UNSUPP_COMMAND);
        return WFS_ERR_UNSUPP_COMMAND;
    } else
    if (m_stStatus.fwChipPower  == WFS_IDC_CHIPONLINE ||    // 设备上电
        m_stStatus.fwChipPower  == WFS_IDC_CHIPBUSY)        // 运行中
    {
        if (wPower == WFS_IDC_CHIPPOWERCOLD)    // 入参冷复位: 已上电状态需要先断电
        {
            stChipData.Clear();
            nRet = m_pDev->ChipReadWrite(CHIP_POW_OFF, stChipData);
            if (nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "芯片上电: 冷复位: 当前处于已激活状态, 执行断电: ->ChipReadWrite(%d|OFF) Fail, ErrCode: %d, Return: %d",
                    CHIP_POW_OFF, nRet, ConvertDevErrCode2WFS(nRet));
                return ConvertDevErrCode2WFS(nRet);
            }
        }
    }
    else // WFS_IDC_CHIPPOWEREDOFF,断电状态
    {
        if (wPower == WFS_IDC_CHIPPOWERWARM)    // 热复位,执行冷复位方式(先上电后复位)
        {

            stChipData.Clear();
            nRet = m_pDev->ChipReadWrite(CHIP_POW_COLD, stChipData);
            if (nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "芯片上电: 热复位: 当前处于未激活状态, 执行上电: ->ChipReadWrite(%d|COLD) Fail, ErrCode: %d, Return: %d",
                    CHIP_POW_COLD, nRet, ConvertDevErrCode2WFS(nRet));
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
        nRet = m_pDev->ChipReadWrite(CHIP_POW_COLD, stChipData);
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片上电: 冷复位: ->ChipReadWrite(%d|COLD) Fail, ErrCode: %d, Return: %d",
                CHIP_POW_COLD, nRet, ConvertDevErrCode2WFS(nRet));
            return ConvertDevErrCode2WFS(nRet);
        }
        m_ChipPower.SetData(stChipData.stData[1].szData, stChipData.stData[1].dwSize);
    } else
    if (wPower == WFS_IDC_CHIPPOWERWARM)
    {
        stChipData.Clear();
        nRet = m_pDev->ChipReadWrite(CHIP_POW_WARM, stChipData);
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片上电: 热复位: ->ChipReadWrite(%d|WARM) Fail, ErrCode: %d, Return: %d",
                CHIP_POW_WARM, nRet, ConvertDevErrCode2WFS(nRet));
            return ConvertDevErrCode2WFS(nRet);
        }
        m_ChipPower.SetData(stChipData.stData[1].szData, stChipData.stData[1].dwSize);
    } else
    if (wPower == WFS_IDC_CHIPPOWEROFF)
    {
        stChipData.Clear();
        nRet = m_pDev->ChipReadWrite(CHIP_POW_OFF, stChipData);
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片断电: ->ChipReadWrite(%d|OFF) Fail, ErrCode: %d, Return: %d",
                CHIP_POW_OFF, nRet, ConvertDevErrCode2WFS(nRet));
            return ConvertDevErrCode2WFS(nRet);
        }
        m_ChipPower.Clear();
    }

    lpData = m_ChipPower.GetData();

    return WFS_SUCCESS;
}
