#include "XFS_IDC.h"
static const char *DEVTYPE = "IDC";
static const char *ThisFile = "XFS_IDC.cpp";
//////////////////////////////////////////////////////////////////////////
#define TIMEID_UPDATE_STATUS                1789
#define RETAIN_CARD_COUNT                   "card_num"
#define RETAIN_CARD_MAXCOUNT                "bin_size"
#define FLUXABLE                            "flux_able"
#define WRACCFOLLEJECT                      "wr_acc_folleject"
#define SPIDCETCPATH                        "/usr/local/CFES/ETC/IDCConfig.ini"
//////////////////////////////////////////////////////////////////////////
CXFS_IDC::CXFS_IDC()
{
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    m_nResetFailedTimes = 0;
    m_bJamm = FALSE;
    m_bNeedRepair = FALSE;
    m_bICCActived = FALSE;
    m_WaitTaken = WTF_NONE;
    m_bIsSetCardData = FALSE;     // RetainCard前需要执行SetCardData命令
    memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));
    memset(&m_Caps, 0, sizeof(m_Caps));
}

CXFS_IDC::~CXFS_IDC()
{

}


long CXFS_IDC::StartRun()
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

// 基本接口
HRESULT CXFS_IDC::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //获取Manager配置文件中sp配置
    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();
    m_strPort = m_cXfsReg.GetValue("CONFIG", "Port", "");                   //30-00-00-00(FT#0019)
    int iDevType = m_cXfsReg.GetValue("DevType", (DWORD)0);
    switch(iDevType){
    case 0:
        m_strDevName = "CRT";
        break;
    case 1:
        m_strDevName = "EC2G";
        break;
    default:
        m_strDevName = "CRT";
        break;
    }

    //获取读卡器sp配置
    InitConfig();
    InitCaps();                                                             //30-00-00-00(FT#0019)
    InitStatus();                                                           //30-00-00-00(FT#0019)

    if (!LoadDevDll(ThisModule))
    {
        UpdateDevStatus(ERR_IDC_HWERR);                                     //30-00-00-00(FT#0019)
        return m_stConfig.usReturnOpenVal == 0 ? WFS_ERR_HARDWARE_ERROR : WFS_SUCCESS;         //30-00-00-00(FT#0019)
        //return WFS_ERR_HARDWARE_ERROR;                                    //30-00-00-00(FT#0019)
    }   

//    InitCaps();                                                           //30-00-00-00(FT#0019)
//    InitStatus();                                                         //30-00-00-00(FT#0019)

    // IDC Open前启动退卡模块,避免IDC启动错误造成退卡模块不执行Open
    HRESULT hCRMRet = StartOpenCRM();
    if (hCRMRet != WFS_SUCCESS)
    {
        if (m_stConfig.usReturnOpenVal == 0)
        {
            Log(ThisModule, __LINE__, "退卡模块启动: StartOpenCRM() Fail, ErrCode: %d, Return: %d.",
                hCRMRet, hCRMRet);
            return hCRMRet;
        } else
        {
            Log(ThisModule, __LINE__, "退卡模块启动: StartOpenCRM() Fail, ErrCode: %d, "
                                      "INI配置ReturnOpenVal=%d, Not Return.",
                hCRMRet, m_stConfig.usReturnOpenVal);
            //return hCRMRet;
        }
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    m_cExtra.AddExtra("VERCount", "2");
    //m_cExtra.AddExtra("VRTDetail[00]", "0000000000000000");             // SP版本程序名称8位+版本8位
    m_cExtra.AddExtra("VERDetail[00]", (CHAR*)byXFSVRTU);
    m_cExtra.AddExtra("VERDetail[01]", (CHAR*)byDevIDCVRTU);
    int nRet = m_pDev->Open(m_strPort.c_str());                             //30-00-00-00(FT#0019)
    UpdateDevStatus(nRet);                                                  //30-00-00-00(FT#0019)
    if (nRet < 0)
    {
        Log(ThisModule, -1, "Open fail");
        OnStatus();                                                         //30-00-00-00(FT#0019)
        return m_stConfig.usReturnOpenVal == 0 ? WFS_ERR_HARDWARE_ERROR : WFS_SUCCESS;         //30-00-00-00(FT#0019)
        //return WFS_ERR_HARDWARE_ERROR;                                    //30-00-00-00(FT#0019)
    }

    nRet = Init();
    if (nRet < 0)
    {
        UpdateDevStatus(ERR_IDC_HWERR);
        return m_stConfig.usReturnOpenVal == 0 ? nRet : WFS_SUCCESS;        //30-00-00-00(FT#0019)
        //return nRet;                                                      //30-00-00-00(FT#0019)
    }

    nRet = GetFWVersion();
    if (nRet < 0)
    {
        return m_stConfig.usReturnOpenVal == 0 ? nRet : WFS_SUCCESS;        //30-00-00-00(FT#0019)
        //return nRet;                                                      //30-00-00-00(FT#0019)
    }


    UpdateRetainCards(MCM_NOACTION);

    // 清除回收计数器
    nRet = SetRecycleCount("00");
    if (nRet < 0)
    {
        return m_stConfig.usReturnOpenVal == 0 ? nRet : WFS_SUCCESS;        //30-00-00-00(FT#0019)
        //return nRet;                                                      //30-00-00-00(FT#0019)
    }

    //----------退卡模块处理----------
    /*if (m_stCRMINIConfig.bCRMDeviceSup == TRUE)     // 支持启用退卡模块
    {
        int nCRMRet = CRM_SUCCESS;

        // 加载退卡模块动态库
        if (!LoadCRMDevDll(ThisModule))
        {
            return WFS_ERR_HARDWARE_ERROR;
        }

        // Open退卡模块
        nCRMRet = m_pCRMDev->Open(m_stCRMINIConfig.szCRMDeviceConList);
        if (nCRMRet != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "Open CRM Device[%s] Fail, CRM RetCode: %d, Return: %d",
                m_stCRMINIConfig.szCRMDeviceConList, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        // 退卡模块初始化
        CRMInitAction emCRMAction;
        if (m_stCRMINIConfig.wDeviceInitAction == 0)
        {
            emCRMAction = CRMINIT_HOMING;   // 正常归位
        } else
        if (m_stCRMINIConfig.wDeviceInitAction == 1)
        {
            emCRMAction = CRMINIT_EJECT;    // 强制退卡
        } else
        if (m_stCRMINIConfig.wDeviceInitAction == 2)
        {
            emCRMAction = CRMINIT_STORAGE;  // 强制暂存
        } else
        if (m_stCRMINIConfig.wDeviceInitAction == 3)
        {
            emCRMAction = CRMINIT_NOACTION; // 无动作
        } else
        {
            emCRMAction = CRMINIT_NOACTION; // 无动作
        }
        nCRMRet = m_pCRMDev->Init(emCRMAction);
        if (nCRMRet != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CRM Device Init[%d] Fail, CRM RetCode: %d, Return: %d",
                emCRMAction, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }    

        // 更新扩展状态_版本参数
        CHAR szCRMDevVer[MAX_PATH] = { 0x00 };      // DevCRM动态库版本
        m_pCRMDev->GetVersion(szCRMDevVer, sizeof(szCRMDevVer) - 1, 1);
        CHAR szCRMSoftVer[MAX_PATH] = { 0x00 };     // CRM设备软件版本
        m_pCRMDev->GetVersion(szCRMSoftVer, sizeof(szCRMSoftVer) - 1, 3);
        CHAR szCRMSerialNumber[MAX_PATH] = { 0x00 };// CRM设备序列号
        m_pCRMDev->GetVersion(szCRMSerialNumber, sizeof(szCRMSerialNumber) - 1, 4);

        m_cExtra.AddExtra("CRM VRTCount", "3");
        m_cExtra.AddExtra("CRM VRT[00]-DecCRM", szCRMDevVer);
        m_cExtra.AddExtra("CRM VRT[01]-Soft", szCRMSoftVer);
        m_cExtra.AddExtra("CRM VRT[02]-SerialNo", szCRMSerialNumber);
    }*/

    // 更新一次状态
    OnStatus();                                                         //30-00-00-00(FT#0019)
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pDev != nullptr)
        m_pDev->Close();
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::OnStatus()
{
    long lRet = UpdateCardStatus();
    if (m_Status.fwDevice == WFS_IDC_DEVOFFLINE){       //30-00-00-00(FT#0019)
        m_pDev->Close();                                //30-00-00-00(FT#0019)
        if (m_pDev->Open(m_strPort.c_str()) == 0)       //30-00-00-00(FT#0019)
        {                                               //30-00-00-00(FT#0019)
            m_pDev->Init(CARDACTION_NOACTION, (WobbleAction)m_stConfig.usNeedWobble);
            lRet = UpdateCardStatus();                  //30-00-00-00(FT#0019)
            UpdateDevStatus(lRet);
        }                                               //30-00-00-00(FT#0019)
        // 退卡模块重连
        if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
        {
            if (m_pCRMDev != nullptr)
            {
                m_pCRMDev->Close();
                m_pCRMDev->Open(m_stCRMINIConfig.szCRMDeviceConList);
            }
        }
    }                                                   //30-00-00-00(FT#0019)    

    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::OnWaitTaken()
{
    if (m_WaitTaken == WTF_NONE)
    {
        return WFS_ERR_CANCELED;
    }
    WaitItemTaken();
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::OnCancelAsyncRequest()
{
    if (m_pDev != nullptr)
        m_pDev->CancelReadCard();
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// 查询命令
HRESULT CXFS_IDC::GetStatus(LPWFSIDCSTATUS &lpStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Status.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_Status;
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::GetCapabilities(LPWFSIDCCAPS &lpCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_Caps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_Caps;
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::GetFormList(LPSTR &lpFormList)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;
    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
    {
        hRes = WFS_SUCCESS;
    }
    lpFormList = (LPSTR)(LPCSTR)m_FormNames;
    //LPSTR szTest = (LPSTR)lpFormList;
    return hRes;
}

HRESULT CXFS_IDC::GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;
    if (LoadFormFile(m_strSPName.c_str(), m_FormList))
    {
        if (lpFormName == nullptr)
        {
            hRes = WFS_ERR_INVALID_DATA;
        }
        else
        {
            SP_IDC_FORM *pForm = FLFind(m_FormList, lpFormName);
            if (!pForm)
                hRes = WFS_ERR_IDC_FORMNOTFOUND;
            else if (!pForm->bLoadSucc)
                hRes = WFS_ERR_IDC_FORMINVALID;
            else
            {
                hRes = CheackFormInvalid(m_FormList, (char *)pForm->FormName.c_str(), pForm->fwAction);
                if (hRes == WFS_SUCCESS)
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
    return hRes;
}


// 执行命令
HRESULT CXFS_IDC::ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    LoadFormFile(m_strSPName.c_str(), m_FormList);

    int hRes = CheackFormInvalid(m_FormList, (char *)lpFormName, WFS_IDC_ACTIONREAD);
    if (hRes == WFS_SUCCESS)
    {
        SP_IDC_FORM *pForm = nullptr;
        const char *pFormName = (char *)lpFormName;
        hRes = SPGetIDCForm(m_FormList, pFormName, pForm, WFS_IDC_ACTIONREAD);
        if (hRes != WFS_SUCCESS)
        {
            Log(ThisModule, hRes, "GetIDCForm(%s)失败", pFormName);
        }
        else
        {
            DWORD dwOption = TracksToDataSourceOption(pForm->szTracks);
            if (IsDoSecureCheck(pForm, dwOption))
                dwOption |= WFS_IDC_SECURITY;
            m_TrackData = CMultiMultiString();
            m_CardDatas.Clear();
            hRes = AcceptAndReadTrack(dwOption, m_stConfig.dwTotalTimeOut);
            if (hRes != WFS_SUCCESS)
            {
                Log(ThisModule, hRes, "AcceptAndReadTrack失败");
            }
        }

        if (hRes == WFS_SUCCESS)
        {
            hRes = SPParseData(pForm);
            if (hRes != WFS_SUCCESS)
            {
                Log(ThisModule, hRes, "SPParseData失败");
            }
        }

    }
    return hRes;
}

//功能：形成磁道数据并写磁道
//输入：pData : 数据
//      pWrite : 参数
//返回：WFS_SUCCESS, 成功，否则，失败。
long CXFS_IDC::OnWriteTrack(WFSIDCWRITETRACK *pWrite)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRes = WFS_SUCCESS;
    SP_IDC_FORM *pForm = NULL;
    hRes = SPGetIDCForm(m_FormList, pWrite->lpstrFormName, pForm, WFS_IDC_ACTIONWRITE);
    if (hRes != WFS_SUCCESS)
    {
        LogWrite(ThisModule, -1, IDS_GET_FORM_FAILED, pWrite->lpstrFormName, hRes);
    }

    if (hRes == WFS_SUCCESS)
    {
        int Value[3];
        ZeroMemory(Value, sizeof(Value));
        DWORD dwOption = 0;
        CMultiString InputData(pWrite->lpstrTrackData);
        char head[10];
        for (int iTrack = 0; iTrack < 3; iTrack++)
        {
            sprintf(head, "TRACK%d", iTrack + 1);
            DWORD dwTracks = TracksToDataSourceOption(pForm->szTracks);
            DWORD dwTrack = TrackIndexToDataSourceOption(iTrack);
            if (!(dwTracks & dwTrack))
                continue;
            LPWFSIDCCARDDATA pCardData = m_CardDatas.Find(TrackIndexToDataSourceOption(iTrack));
            if (!pCardData)
            {
                if (NeedTrack(pForm->szTracks, iTrack))
                {
                    LogWrite(ThisModule, -1, IDS_NO_TRACK_DATA, pForm->FormName.c_str(), iTrack + 1);
                    FireInvalidTrackData(WFS_IDC_DATAMISSING, head, NULL);
                }
                continue;
            }
            pCardData->fwWriteMethod = pWrite->fwWriteMethod;

            dwOption |= dwTrack;

            vector<int> SepList;
            GenSepList((char *)pCardData->lpbData, pCardData->ulDataLength, pForm->cFieldSeparatorTracks[iTrack], SepList);

            FFLIT itField;
            for (itField = pForm->TracksFields[iTrack].begin(); itField != pForm->TracksFields[iTrack].end(); itField++)
            {
                if (!WriteFieldData(pCardData, *itField, SepList, pForm->sDefault.c_str() && pForm->sDefault == "?", pForm->cFieldSeparatorTracks[iTrack], InputData))
                {
                    LogWrite(ThisModule, -1, IDS_WRITE_FIELD_DATA_FAILED, pForm->FormName.c_str(), iTrack + 1, (*itField)->FieldName.c_str());
                    break;
                }
            }
            if (itField == pForm->TracksFields[iTrack].end())
            {
                Value[iTrack] = true;
            }
        }
        const char *p = pForm->szTracks;

        BOOL bSucc;
        if (!ComputeTracks(&p, Value, bSucc) || !bSucc)
        {
            LogWrite(ThisModule, -1, IDS_NO_ALL_TRACK, pForm->FormName.c_str(), pForm->sTracks.c_str(), Value[0], Value[1], Value[2]);
            hRes = WFS_ERR_IDC_INVALIDDATA;
        }

        if (hRes == WFS_SUCCESS)
        {
            hRes = WriteTrackData(dwOption);
        }
    }
    return hRes;
}

HRESULT CXFS_IDC::WriteTrack(const LPWFSIDCWRITETRACK lpWriteData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long hRes = CheackFormInvalid(m_FormList, (char *)(((WFSIDCWRITETRACK *)lpWriteData)->lpstrFormName), WFS_IDC_ACTIONWRITE);
    if (hRes == WFS_SUCCESS)
    {
        hRes = OnWriteTrack((WFSIDCWRITETRACK *)lpWriteData);
    }
    return hRes;
}

long CXFS_IDC::WriteTrackData(DWORD dwWriteOption)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_Status.fwRetainBin == WFS_IDC_RETAINBINFULL)
    {
        Log(ThisModule, -1, "写卡时回收盒满");
        return WFS_ERR_HARDWARE_ERROR;
    }

    int nRet = UpdateCardStatus();
    if (nRet < 0)
    {
        Log(ThisModule, -1, "吞卡前检测卡失败:%s", ProcessReturnCode(nRet));
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (m_Cardstatus != IDCSTAUTS_INTERNAL)//没有卡
    {
        nRet = m_pDev->AcceptCard(m_stConfig.dwTotalTimeOut, TRUE);
        UpdateDevStatus(nRet);
        if (nRet < 0)
        {
            Log(ThisModule, -1, "接收卡时出错:%s", ProcessReturnCode(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        }
        else if (nRet == ERR_IDC_INSERT_TIMEOUT)
        {
            Log(ThisModule, -1, "接收卡时超时");
            return WFS_ERR_TIMEOUT;
        }
        else if (nRet == ERR_IDC_USER_CANCEL)
        {
            Log(ThisModule, -1, "接收卡时用户取消");
            return WFS_ERR_CANCELED;
        }
        else
        {
            FireCardInserted();
        }
    }

    m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;

    STTRACK_INFO    trackInfo ;
    BYTE *pArrary[3];
    pArrary[0] = (BYTE *)trackInfo.TrackData[0].szTrack;
    pArrary[1] = (BYTE *)trackInfo.TrackData[1].szTrack;
    pArrary[2] = (BYTE *)trackInfo.TrackData[2].szTrack;

    ULONG ulTrackLen[3] = {0};
    ulTrackLen[0] = sizeof(trackInfo.TrackData[0].szTrack);
    ulTrackLen[1] = sizeof(trackInfo.TrackData[1].szTrack);
    ulTrackLen[2] = sizeof(trackInfo.TrackData[2].szTrack);

    WORD  wMethodTrack[3] = {0};

    for (int nr = 0; nr < 3; nr ++)
    {
        if (!GetTrackInfo(WFS_IDC_TRACK1 << nr, &ulTrackLen[nr], pArrary[nr], &wMethodTrack[nr]))
        {
            if (ulTrackLen[nr] != 0)
            {
                Log(ThisModule, -1, "GetTrackInfo()失败, 当前为第%d磁道", nr + 1);
                return WFS_ERR_INVALID_DATA;
            }
        }
    }

    nRet = m_pDev->WriteTracks(trackInfo);
    UpdateDevStatus(nRet);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "写卡失败:%s", ProcessReturnCode(nRet));
    }
    return Convert2XFSErrCode(nRet);
}

HRESULT CXFS_IDC::EjectCard(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    Q_UNUSED(dwTimeOut)

    int iRet = UpdateCardStatus();
    if (iRet < 0)
    {
        return WFS_ERR_HARDWARE_ERROR;
    }
    //ULONG dwStart = CQtTime::GetSysTick();

    IDC_IDCSTAUTS  LatestCardstatus = m_Cardstatus;
    if (m_Cardstatus == IDCSTAUTS_NOCARD)//没有检测到卡
    {
        Log(ThisModule, -1, "退卡时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }
    else if (m_Cardstatus == IDCSTAUTS_ENTERING ||
             m_Cardstatus == IDCSTAUTS_INTERNAL)//卡在读卡器口
    {
        int nRet = m_pDev->EjectCard();
        UpdateDevStatus(nRet);
        if (nRet >= 0)
        {   
            //解除jam状态
            m_bJamm = FALSE;

            if (m_Cardstatus != IDCSTAUTS_NOCARD)
            {
                m_WaitTaken = WTF_TAKEN;
            }

            nRet = UpdateCardStatus();
            if (nRet < 0)
            {
                Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(nRet));
                return WFS_ERR_HARDWARE_ERROR;
            }

            m_Status.fwMedia = WFS_IDC_MEDIAENTERING;
            if (LatestCardstatus != IDCSTAUTS_NOCARD)
                m_WaitTaken = WTF_TAKEN;

            return WFS_SUCCESS;
        }
        else if (nRet == ERR_IDC_JAMMED)
        {
            m_Status.fwMedia = WFS_IDC_MEDIAJAMMED;
            nRet = UpdateCardStatus();
            if (nRet < 0)
            {
                Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(nRet));
                return WFS_ERR_HARDWARE_ERROR;
            }
            Log(ThisModule, -1, "退卡时:%s", ProcessReturnCode(nRet));
            return WFS_ERR_IDC_MEDIAJAM;
        }
        else
        {
            Log(ThisModule, -1, "退卡时:%s", ProcessReturnCode(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        }
    }
    else if (m_Cardstatus == IDCSTAUTS_ICC_PRESS ||
             m_Cardstatus == IDCSTAUTS_ICC_ACTIVE ||
             m_Cardstatus == IDCSTAUTS_ICC_POWEROFF) // IC处于激活或压下状态
    {
        // 先去掉激活状态
        int nRet = 0;
        if (m_Cardstatus == IDCSTAUTS_ICC_PRESS ||
            m_Cardstatus == IDCSTAUTS_ICC_ACTIVE)
        {
            nRet = m_pDev->ICCDeActivation();
            UpdateDevStatus(nRet);
            if (0 != nRet)
            {
                Log(ThisModule, -1, "IC卡退卡去激活时:%s", ProcessReturnCode(nRet));
                return WFS_ERR_HARDWARE_ERROR;
            }
        }

        // 释放IC卡
        nRet = m_pDev->ICCRelease();
        UpdateDevStatus(nRet);
        if (0 != nRet)
        {
            Log(ThisModule, -1, "IC卡退卡释放卡时:%s", ProcessReturnCode(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        }

        // 释放IC后再退卡
        nRet = m_pDev->EjectCard();
        UpdateDevStatus(nRet);
        if (nRet >= 0)
        {
            //解除jam状态
            m_bJamm = FALSE;

            if (LatestCardstatus != IDCSTAUTS_NOCARD)
            {
                m_WaitTaken = WTF_TAKEN;
            }

            int nRet = UpdateCardStatus();
            if (nRet < 0)
            {
                Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(nRet));
                return WFS_ERR_HARDWARE_ERROR;
            }

            m_Status.fwMedia = WFS_IDC_MEDIAENTERING;
            if (LatestCardstatus != IDCSTAUTS_NOCARD)
                m_WaitTaken = WTF_TAKEN;

            return WFS_SUCCESS;
        }
        else if (nRet == ERR_IDC_JAMMED)
        {
            m_Status.fwMedia = WFS_IDC_MEDIAJAMMED;
            nRet = UpdateCardStatus();
            if (nRet < 0)
            {
                Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(nRet));
                return WFS_ERR_HARDWARE_ERROR;
            }
            Log(ThisModule, -1, "IC卡退卡时:%s", ProcessReturnCode(nRet));
            return WFS_ERR_IDC_MEDIAJAM;
        }
        else
        {
            Log(ThisModule, -1, "IC卡退卡时:%s", ProcessReturnCode(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        }
    }
    else
    {
        Log(ThisModule, -1, "退卡时卡状态异常:%s", m_Cardstatus);
    }

    return WFS_ERR_HARDWARE_ERROR;
}

int CXFS_IDC::SetRecycleCount(LPCSTR pszCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = m_pDev->SetRecycleCount(pszCount);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "设置回收计数器失败:(%d)%s", nRet, ProcessReturnCode(nRet));
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

int CXFS_IDC::MoveCardToMMPosition(BOOL bIsCheck)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = WFS_SUCCESS;
    if (bIsCheck == TRUE)
    {
        //检测是否有卡
        /*int */nRet = UpdateCardStatus();
        if(nRet < 0){
            return Convert2XFSErrCode(nRet);
        }

        if(m_Cardstatus == IDCSTAUTS_NOCARD){
            return WFS_SUCCESS;
        }

        //释放触点
        nRet = m_pDev->ICCRelease();
        if (0 != nRet) {
            Log(ThisModule, -1, "释放IC触点失败:%d", nRet);
            //该处错误不做返回
        }

        //移动卡完全到读卡器内部
        nRet = m_pDev->ICCMove();
        UpdateDevStatus(nRet);
        if (nRet < 0)
        {
            Log(ThisModule, -1, "移动卡完全到读卡器内部失败:(%d)%s", nRet, ProcessReturnCode(nRet));
            return Convert2XFSErrCode(nRet);
        }
    } else
    {
        // 移动卡完全到读卡器内部(只移动卡,不取状态,不主动上报事件)
        nRet = m_pDev->ICCMove();
        if (nRet < 0)
        {
            Log(ThisModule, -1, "移动卡完全到读卡器内部失败:(%d)%s", nRet, ProcessReturnCode(nRet));
            return Convert2XFSErrCode(nRet);
        }
    }

    return WFS_SUCCESS;
}

// Open退卡模块
HRESULT CXFS_IDC::StartOpenCRM()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //----------退卡模块处理----------
    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE)     // 支持启用退卡模块
    {
        int nCRMRet = CRM_SUCCESS;

        // 加载退卡模块动态库
        if (!LoadCRMDevDll(ThisModule))
        {
            return WFS_ERR_HARDWARE_ERROR;
        }

        // Open退卡模块
        nCRMRet = m_pCRMDev->Open(m_stCRMINIConfig.szCRMDeviceConList);
        if (nCRMRet != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "Open CRM Device[%s] Fail, CRM RetCode: %d, Return: %d",
                m_stCRMINIConfig.szCRMDeviceConList, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        // 退卡模块初始化
        CRMInitAction emCRMAction;
        if (m_stCRMINIConfig.wDeviceInitAction == 0)
        {
            emCRMAction = CRMINIT_HOMING;   // 正常归位
        } else
        if (m_stCRMINIConfig.wDeviceInitAction == 1)
        {
            emCRMAction = CRMINIT_EJECT;    // 强制退卡
        } else
        if (m_stCRMINIConfig.wDeviceInitAction == 2)
        {
            emCRMAction = CRMINIT_STORAGE;  // 强制暂存
        } else
        if (m_stCRMINIConfig.wDeviceInitAction == 3)
        {
            emCRMAction = CRMINIT_NOACTION; // 无动作
        } else
        {
            emCRMAction = CRMINIT_NOACTION; // 无动作
        }
        nCRMRet = m_pCRMDev->Init(emCRMAction);
        if (nCRMRet != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CRM Device Init[%d] Fail, CRM RetCode: %d, Return: %d",
                emCRMAction, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        // 更新扩展状态_版本参数
        //CHAR szCRMDevVer[MAX_PATH] = { 0x00 };      // DevCRM动态库版本
        //m_pCRMDev->GetVersion(szCRMDevVer, sizeof(szCRMDevVer) - 1, 1);
        CHAR szCRMSoftVer[MAX_PATH] = { 0x00 };     // CRM设备软件版本
        m_pCRMDev->GetVersion(szCRMSoftVer, sizeof(szCRMSoftVer) - 1, 3);
        CHAR szCRMSerialNumber[MAX_PATH] = { 0x00 };// CRM设备序列号
        m_pCRMDev->GetVersion(szCRMSerialNumber, sizeof(szCRMSerialNumber) - 1, 4);

        m_cExtra.AddExtra("CRM VRTCount", "3");
        m_cExtra.AddExtra("CRM VRT[00]-DecCRM", (char*)byDevCRMVRTU);
        m_cExtra.AddExtra("CRM VRT[01]-Soft", szCRMSoftVer);
        m_cExtra.AddExtra("CRM VRT[02]-SerialNo", szCRMSerialNumber);
    }

    return WFS_SUCCESS;
}

long CXFS_IDC::RetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_Status.fwRetainBin == WFS_IDC_RETAINBINFULL)
    {
        Log(ThisModule, -1, "回收箱满");
        FireRetainBinThreshold(WFS_IDC_RETAINBINFULL);
        return WFS_ERR_IDC_RETAINBINFULL;
    }

    IDC_IDCSTAUTS IDCstatus;
    int nRet = UpdateCardStatus();

    if (nRet < 0)
    {
        Log(ThisModule, -1, "获取卡状态异常");
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (m_Cardstatus == IDCSTAUTS_NOCARD)//没有检测到卡
    {
        Log(ThisModule, -1, "吞卡时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }
//    else if (m_Cardstatus == IDCSTAUTS_ENTERING || m_Cardstatus == IDCSTAUTS_INTERNAL) //卡在读卡器口
//    {
//        return WFS_SUCCESS;
//    }

    if (m_Cardstatus == IDCSTAUTS_ICC_ACTIVE) // IC卡处于激活状态
    {
        // 先去掉IC卡激活状态
        nRet = m_pDev->ICCDeActivation();
        UpdateDevStatus(nRet);
        UpdateCardStatus();
        if (nRet < 0)
        {
            Log(ThisModule, -1, "吞卡时去激活状态失败:(%d)%s", nRet, ProcessReturnCode(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    if (m_Cardstatus == IDCSTAUTS_ICC_PRESS) // IC卡处于压下状态
    {
        // 先释放IC卡
        nRet = m_pDev->ICCRelease();
        UpdateDevStatus(nRet);
        if (0 != nRet)
        {
            Log(ThisModule, -1, "吞卡时释放卡失败_1:(%d)%s", nRet, ProcessReturnCode(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    nRet = m_pDev->EatCard();
    UpdateDevStatus(nRet);    
    if (nRet < 0)
    {
        if (nRet == ERR_IDC_JAMMED)
        {
            Log(ThisModule, -1, "吞卡失败_1:(%d)%s", nRet, ProcessReturnCode(nRet));
            return WFS_ERR_IDC_MEDIAJAM;
        } else if (nRet == ERR_IDC_CARDPULLOUT)                                     //30-00-00-00(FT#0009)
        {                                                                           //30-00-00-00(FT#0009)
            Log(ThisModule, -1, "吞卡失败_2:(%d)%s", nRet, ProcessReturnCode(nRet)); //30-00-00-00(FT#0009)
            return WFS_ERR_IDC_NOMEDIA;                                             //30-00-00-00(FT#0009)
        }                                                                           //30-00-00-00(FT#0009)
//        if (m_Cardstatus == IDCSTAUTS_NOCARD)
//        {
//            FireMediaRemoved();
//            Log(ThisModule, -1, "吞卡时没有检测到卡");
//            return WFS_ERR_IDC_NOMEDIA;
//        }
        Log(ThisModule, -1, "吞卡失败_3:(%d)%s", nRet, ProcessReturnCode(nRet));      //30-00-00-00(FT#0009)
        return WFS_ERR_HARDWARE_ERROR;
    }

    //解除jam状态
    m_bJamm = FALSE;

    m_WaitTaken = WTF_NONE;
    FireMediaRetained();
    UpdateRetainCards(MCM_INC_ONE);
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData)
{
    HRESULT hRes = RetainCard();
    if (hRes != WFS_SUCCESS)
        return hRes;
    m_CardRetain.usCount = m_Status.usCards;
    m_CardRetain.fwPosition = WFS_IDC_MEDIANOTPRESENT;
    lpRetainCardData = &m_CardRetain;
    return WFS_SUCCESS;
}


HRESULT CXFS_IDC::ResetCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    UpdateRetainCards(m_stConfig.bSuppDeRetractCount ? MCM_DEC_ONE : MCM_CLEAR);

    //清除硬件计数
    SetRecycleCount("00");
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::SetKey(const LPWFSIDCSETKEY lpKeyData)
{
    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_bJamm){
        return WFS_ERR_HARDWARE_ERROR;
    }

    WORD wOption = *(WORD *)lpReadData;
    m_CardDatas.Clear();
    long hRes = AcceptAndReadTrack(wOption, dwTimeOut);
    if (hRes != WFS_SUCCESS)
    {
        Log(ThisModule, -1, "AcceptAndReadTrack failed");
    }
    lppCardData = (LPWFSIDCCARDDATA *)m_CardDatas;
    return hRes;
}

HRESULT CXFS_IDC::WriteRawData(const LPWFSIDCCARDDATA *lppCardData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //保存以前的卡数据
    CWFSIDCCardDataPtrArray CardDatas;
    CardDatas = m_CardDatas;

    //拷贝新的数据
    m_CardDatas = (LPWFSIDCCARDDATA *)lppCardData;
    WORD dwOption = m_CardDatas.GetOption();

    //调用写磁道
    long hRes = WriteTrackData(dwOption);
    if (hRes != WFS_SUCCESS)
    {
        Log(ThisModule, -1, "WriteTrack failed, Option:%d", dwOption);
        //失败恢复数据
        m_CardDatas = CardDatas;
    }
    else//成功，拷贝不存在的老数据
    {
        WORD wOption = m_CardDatas.GetOption();
        for (int i = 0; CardDatas[i]; i++)
        {
            if (!(CardDatas[i]->wDataSource & wOption))
                continue;
            m_CardDatas.SetAt(CardDatas[i]->wDataSource, *CardDatas[i]);
        }
    }
    return hRes;
}

HRESULT CXFS_IDC::ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if(m_bJamm){
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_ChipIO.wChipProtocol = lpChipIOIn->wChipProtocol;
    m_ChipIO.ulChipDataLength = MAX_CHIP_IO_LEN;

     ICCardProtocol eProFlag;
    if (lpChipIOIn->wChipProtocol != WFS_IDC_CHIPT0  && lpChipIOIn->wChipProtocol != WFS_IDC_CHIPT1)
    {
        Log(ThisModule, -1, "wChipProtocol=%d", lpChipIOIn->wChipProtocol);
        return WFS_ERR_IDC_PROTOCOLNOTSUPP;
        //return WFS_ERR_INVALID_DATA;
    }

    if (lpChipIOIn->wChipProtocol == WFS_IDC_CHIPT0)
    {
        eProFlag = ICCARD_PROTOCOL_T0;
    }
    else if (lpChipIOIn->wChipProtocol == WFS_IDC_CHIPT1)
    {
        eProFlag = ICCARD_PROTOCOL_T1;
    }

    if (m_Status.fwMedia == WFS_IDC_MEDIANOTPRESENT)//没有检测到卡
    {
        Log(ThisModule, -1, "ChipIO时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }

    if (WFS_IDC_CHIPPOWEREDOFF == m_Status.fwChipPower)
    {
        return WFS_ERR_IDC_ATRNOTOBTAINED;
    }

    if (m_Status.fwChipPower  != WFS_IDC_CHIPONLINE)
    {
        return WFS_ERR_IDC_INVALIDMEDIA;
    }

    char szDataInOut[512] = {0};
    memcpy(szDataInOut, lpChipIOIn->lpbChipData, lpChipIOIn->ulChipDataLength);
    unsigned int nInOutLen = lpChipIOIn->ulChipDataLength;
    int nRet = m_pDev->ICCChipIO(eProFlag, szDataInOut, nInOutLen, sizeof(szDataInOut));
    UpdateDevStatus(nRet);                                  //30-00-00-00(FT#0019)
    if (ERR_IDC_PROTOCOLNOTSUPP == nRet)
    {
        return WFS_ERR_IDC_INVALIDDATA;
    }
    if (ERR_IDC_INVALIDCARD == nRet)
    {
        return WFS_ERR_IDC_INVALIDMEDIA;
    }

    //UpdateDevStatus(nRet);                                //30-00-00-00(FT#0019)
    if (0 != nRet)
    {
        Log(ThisModule, -1, "ChipIO失败，返回：%d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_ChipIO.ulChipDataLength = nInOutLen;
    memcpy(m_ChipIO.lpbChipData, szDataInOut, nInOutLen);
    lpChipIOOut = &m_ChipIO;
    return 0;
}

HRESULT CXFS_IDC::Reset(LPWORD lpResetIn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 支持CRM时,先执行CRMReset
    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 执行设备复位
        INT nCRMRet = CRM_SUCCESS;
        if ((nCRMRet = m_pCRMDev->CMReset()) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "Reset: CRM Device Reset: ->CMReset() Fail, "
                                "CRM RetCode: %d, Return: %d.",
                nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        m_bIsSetCardData = FALSE;     // RetainCard前需要执行SetCardData命令
        memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));
    }

    WORD wAction = *lpResetIn;
    // 没有动作参数时由SP决定
    if (wAction == 0)
    {
        wAction = WFS_IDC_RETAIN;
    }

    if (wAction < WFS_IDC_NOACTION || wAction > WFS_IDC_RETAIN)
    {
        return WFS_ERR_INVALID_DATA;
    }

    CardAction ActFlag;
    if (wAction == WFS_IDC_RETAIN)
    {
        ActFlag = CARDACTION_RETRACT;
    }
    else if (wAction == WFS_IDC_NOACTION)
    {
        //ActFlag = CARDACTION_NOACTION;
        ActFlag = CARDACTION_NOMOVEMENT;
    }
    else if (wAction == WFS_IDC_EJECT)
    {
        ActFlag = CARDACTION_EJECT;
    }
    else
    {
        return WFS_ERR_INVALID_DATA;
    }

    long nRet = UpdateCardStatus();
    if (nRet < 0)
    {
        Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(nRet));
        return WFS_ERR_HARDWARE_ERROR;
    }

    IDC_IDCSTAUTS  LatestCardstatus = m_Cardstatus;
    if((LatestCardstatus != IDCSTAUTS_NOCARD) && (LatestCardstatus != IDCSTATUS_UNKNOWN))
    {
        if ((wAction == WFS_IDC_RETAIN) && (m_Status.fwRetainBin == WFS_IDC_RETAINBINFULL))
        {
            Log(ThisModule, -1, "卡回收盒已满");
            FireRetainBinThreshold(WFS_IDC_RETAINBINFULL);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    nRet = m_pDev->Init(ActFlag, (WobbleAction)m_stConfig.usNeedWobble);
    UpdateDevStatus(nRet);
    if (nRet >= 0)
    {
        if (wAction == WFS_IDC_EJECT && LatestCardstatus != IDCSTAUTS_NOCARD)
        {
            m_WaitTaken = WTF_TAKEN;
        }

        int nRet = UpdateCardStatus();
        if (nRet < 0)
        {
            Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(nRet));
            return WFS_ERR_HARDWARE_ERROR;
        } else {
            m_nResetFailedTimes = 0;

            //有卡时更新fwMedia状态并发送相关事件
            if((LatestCardstatus != IDCSTAUTS_NOCARD) && (LatestCardstatus != IDCSTATUS_UNKNOWN)){
                if (wAction == WFS_IDC_RETAIN)
                {
                    // 复位吞卡成功后, 要更新一下介质状态
                    m_Status.fwMedia = WFS_IDC_MEDIANOTPRESENT;
                    UpdateRetainCards(MCM_INC_ONE);
                    FireMediaDetected(WFS_IDC_CARDRETAINED);
                }
                else if (wAction == WFS_IDC_EJECT)
                {
                    m_Status.fwMedia = WFS_IDC_MEDIAENTERING;
                    m_WaitTaken = WTF_TAKEN;
                    FireMediaDetected(WFS_IDC_CARDEJECTED);
                } else {
                    FireMediaDetected(WFS_IDC_CARDREADPOSITION);
                }
                Log(ThisModule, -1, "复位时检测到卡");
            }

            //复位成功，解除jam状态
            m_bJamm = FALSE;
            //m_WaitTaken = WTF_NONE;                 //30-00-00-00(FT#0019)
            return WFS_SUCCESS;
        }       
    }
    else // 复位动作失败
    {
        if (wAction == WFS_IDC_EJECT)
        {
            m_nResetFailedTimes++;
            if (m_stConfig.bCaptureOnResetFailed && m_nResetFailedTimes >= m_stConfig.usResetRetryTimes)
            {
                nRet = m_pDev->Init(CARDACTION_RETRACT, (WobbleAction)m_stConfig.usNeedWobble);
                UpdateDevStatus(nRet);
                if (nRet >= 0)
                {
                    m_Status.fwMedia = WFS_IDC_MEDIANOTPRESENT;
                    if((LatestCardstatus != IDCSTAUTS_NOCARD) && (LatestCardstatus != IDCSTATUS_UNKNOWN))
                    {
                        UpdateRetainCards(MCM_INC_ONE);
                        FireMediaDetected(WFS_IDC_CARDRETAINED);
                    }
                    //m_WaitTaken = WTF_TAKEN;
                    //解除jam状态
                    m_bJamm = FALSE;
                    return WFS_SUCCESS;
                } else {
                    Log(ThisModule, -1, "复位退卡失败两次后吞卡失败");
                }
            }
        }        
    }

    // CRT310
    if (nRet == ERR_IDC_ICRW_ERROR) {
        Log(ThisModule, -1, "复位吞卡(卡被取走，返回码会被重置为设备故障)");
        nRet = ERR_IDC_HWERR;
    } else if (nRet == ERR_IDC_JAMMED) {
        nRet = WFS_ERR_IDC_MEDIAJAM;
    } else {
        nRet = WFS_ERR_HARDWARE_ERROR;
    }

    Log(ThisModule, -1, "复位失败:(%d)%s", nRet, ProcessReturnCode(nRet));
    return nRet;
}

HRESULT CXFS_IDC::ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_bJamm){
        return WFS_ERR_HARDWARE_ERROR;
    }
    m_ChipPowerRes.Clear();

    WORD wPower = * lpChipPower;
    if ((WFS_IDC_CHIPPOWERCOLD != wPower) &&
        (WFS_IDC_CHIPPOWERWARM != wPower) &&
        (WFS_IDC_CHIPPOWEROFF  != wPower))
    {
        return WFS_ERR_INVALID_DATA;
    }

    int iTmpRet = UpdateCardStatus();
    if (iTmpRet < 0)
    {
        Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(iTmpRet));
        return WFS_ERR_HARDWARE_ERROR;
    }
    else if (m_Cardstatus == IDCSTAUTS_NOCARD)//没有检测到卡
    {
        Log(ThisModule, -1, "ChipPower时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }

    //////////////////////////////////////////////////////////////////////////
    char szATRInfo[100] = {0};
    uint nATRLen = 0;
    int nRet = 0;

    if (m_Status.fwChipPower  == WFS_IDC_CHIPNODEVICE)
    {
        Log(ThisModule, -1, "ChipPower时卡没有芯片或芯片有问题");
        return WFS_ERR_IDC_INVALIDMEDIA;
    }
    else if (m_Status.fwChipPower  == WFS_IDC_CHIPNOCARD && m_Cardstatus == IDCSTAUTS_NOCARD)
    {
        Log(ThisModule, -1, "ChipPower时没有检测到卡");
        return WFS_ERR_IDC_NOMEDIA;
    }
    else if (m_Status.fwChipPower  == WFS_IDC_CHIPHWERROR ||
             m_Status.fwChipPower  == WFS_IDC_CHIPUNKNOWN)
    {
        return WFS_ERR_HARDWARE_ERROR;
    }
    else if (m_Status.fwChipPower  == WFS_IDC_CHIPNOTSUPP)
    {
        Log(ThisModule, -1, "ChipPower时设备不支持IC卡操作");
        return WFS_ERR_UNSUPP_COMMAND;
    }
    else if (m_Status.fwChipPower  == WFS_IDC_CHIPONLINE ||
             m_Status.fwChipPower  == WFS_IDC_CHIPBUSY)
    {
        if (WFS_IDC_CHIPPOWERCOLD == wPower)
        {
            nRet = m_pDev->ICCDeActivation();
            UpdateDevStatus(nRet);                                  //30-00-00-00(FT#0019)
            if (0 != nRet)
            {
                Log(ThisModule, -1, "IC冷复位（当前IC卡为已激活状态，现在正尝试反激活）失败，复位参数：%d", wPower);
                if (ERR_IDC_INVALIDCARD == nRet)
                {
                    return WFS_ERR_IDC_INVALIDMEDIA;
                }
                //SetFailState(nRet);
                if (ERR_IDC_UNSUP_CMD == nRet)
                {
                    return WFS_ERR_UNSUPP_COMMAND;
                }
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
    }
    else // WFS_IDC_CHIPPOWEREDOFF
    {
        if(wPower != WFS_IDC_CHIPPOWEROFF){
            nRet = m_pDev->ICCPress();
            if(nRet < 0){
                return WFS_ERR_HARDWARE_ERROR;
            }
        }

        if (WFS_IDC_CHIPPOWERWARM == wPower)
        {
            nRet = m_pDev->ICCReset(ICCARDRESET_COLD, szATRInfo, nATRLen);
            UpdateDevStatus(nRet);
            if (0 != nRet)
            {
                Log(ThisModule, -1, "IC复位（当前IC卡为未激活状态，现在正尝试激活）失败，复位参数：%d", wPower);
                if (ERR_IDC_INVALIDCARD == nRet)
                {
                    return WFS_ERR_IDC_INVALIDMEDIA;
                }
                if (ERR_IDC_UNSUP_CMD == nRet)
                {
                    return WFS_ERR_UNSUPP_COMMAND;
                }
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
        else if (WFS_IDC_CHIPPOWEROFF == wPower)
        {
            m_ChipPowerRes.SetData(szATRInfo, nATRLen);
            lpData = m_ChipPowerRes.GetData();
            return WFS_SUCCESS;
        }
    }

    memset(szATRInfo, 0, sizeof(szATRInfo));
    //////////////////////////////////////////////////////////////////////////

    if (wPower == WFS_IDC_CHIPPOWERCOLD)
    {
        nRet = m_pDev->ICCReset(ICCARDRESET_COLD, szATRInfo, nATRLen);
        if (nRet == 0)
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPONLINE;
        }
    }
    else if (wPower == WFS_IDC_CHIPPOWERWARM)
    {
        nRet = m_pDev->ICCReset(ICCARDRESET_WARM, szATRInfo, nATRLen);
        if (nRet == 0)
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPONLINE;
        }
    }
    else if (wPower == WFS_IDC_CHIPPOWEROFF)
    {
        nRet = m_pDev->ICCDeActivation();
        UpdateDevStatus(nRet);                                  //30-00-00-00(FT#0019)
        if (nRet == 0)
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPPOWEREDOFF;
        }
    }

    if (0 != nRet)
    {
        Log(ThisModule, -1, "IC复位失败，复位参数：%d", wPower);
        if (ERR_IDC_INVALIDCARD == nRet)
        {
            return WFS_ERR_IDC_INVALIDMEDIA;
        }
        if (ERR_IDC_UNSUP_CMD == nRet)
        {
            return WFS_ERR_UNSUPP_COMMAND;
        }
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_ChipPowerRes.SetData(szATRInfo, nATRLen);
    lpData = m_ChipPowerRes.GetData();

    return WFS_SUCCESS;
}

//功能：用指定FORM分析磁道数据
//输入：pData   : 数据
//      pForm   : 指定FORM
//返回：WFS_SUCCESS，成功，否则失败。
long CXFS_IDC::SPParseData(SP_IDC_FORM *pForm)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRes = WFS_SUCCESS;

    int iTrack;
    int Value[3];
    ZeroMemory(Value, sizeof(Value));
    char head[30] = {0};
    for (iTrack = 0; iTrack < 3; iTrack++)
    {
        BOOL bNeedTrack = (TracksToDataSourceOption(pForm->szTracks) & TrackIndexToDataSourceOption(iTrack)) &&
                          NeedTrack(pForm->szTracks, iTrack);
        sprintf(head, "TRACK%d", iTrack + 1);
        LPWFSIDCCARDDATA pCardData = m_CardDatas.Find(TrackIndexToDataSourceOption(iTrack));
        if (!pCardData)
        {
            if (bNeedTrack)
            {
                LogWrite(ThisModule, -1, IDS_NO_TRACK_DATA, pForm->FormName.c_str(), iTrack + 1);
                FireInvalidTrackData(WFS_IDC_DATAMISSING, head, NULL);
            }
            continue;
        }
        if (pCardData->wStatus == WFS_IDC_DATAOK)
            Value[iTrack] = true;
        else
        {
            if (bNeedTrack)
            {
                LogWrite(ThisModule, -1, IDS_TRACK_DATA_ERROR, iTrack + 1, pCardData->wStatus);
                FireInvalidTrackData(pCardData->wStatus, head, (char *)pCardData->lpbData);
            }
            continue;
        }

        vector<int> SepList;
        GenSepList((char *)pCardData->lpbData, pCardData->ulDataLength, pForm->cFieldSeparatorTracks[iTrack], SepList);

        CMultiString ms;
        char *pHead = head;
        FFLIT itField;
        for (itField = pForm->TracksFields[iTrack].begin(); itField != pForm->TracksFields[iTrack].end(); itField++)
        {
            //分析字段数据写入ms中
            LPBYTE lpByte = pCardData->lpbData;
            int nLen = pCardData->ulDataLength;
            long nFieldLen = 0;
            long nFieldOffset = 0;
            if (!ComputeFieldInfo(lpByte, nLen, (*itField)->FieldSepIndices, (*itField)->nOffsets, SepList, nFieldOffset, nFieldLen))
            {
                LogWrite(ThisModule, -1, IDS_COMPUTE_FIELD_FAILED, (*itField)->FieldName.c_str());
                break;
            }
            char buf[1024];
            strncpy(buf, (const char *)lpByte + nFieldOffset, nFieldLen);
            buf[nFieldLen] = 0;
            char temp[1024];
            if (pHead)
                sprintf(temp, "%s:%s=%s", pHead, (*itField)->FieldName.c_str(), buf);
            else
                sprintf(temp, "%s=%s", (*itField)->FieldName.c_str(), buf);
            ms.Add(temp);
            pHead = NULL;
        }
        if (itField != pForm->TracksFields[iTrack].end())
        {
            if (bNeedTrack)
                FireInvalidTrackData(pCardData->wStatus, head, (char *)pCardData->lpbData);
            continue;
        }
        m_TrackData.Add(ms);
    }
    const char *p = pForm->szTracks;
    BOOL bSucc;
    if (!ComputeTracks(&p, Value, bSucc) || !bSucc)
    {
        LogWrite(ThisModule, -1, IDS_NO_ALL_TRACK, pForm->FormName.c_str(), pForm->sTracks.c_str(), Value[0], Value[1], Value[2]);
        hRes = WFS_ERR_IDC_INVALIDDATA;
    }
    return hRes;
}

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
    return 0;
}

#ifdef CARD_REJECT_GD_MODE
//读卡器新增扩展部分
HRESULT CXFS_IDC::ReduceCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_IDC::SetCount(LPWORD lpwCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

HRESULT CXFS_IDC::IntakeCardBack()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //HRESULT hRes = WFS_ERR_HARDWARE_ERROR;
    HRESULT hRes = WFS_ERR_UNSUPP_COMMAND;

    return hRes;
}

// 退卡模块(CRM)-指定卡号退卡
HRESULT CXFS_IDC::CMEjectCard(LPCSTR lpszCardNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    INT nIDCRet = WFS_SUCCESS;

    //std::thread m_thRunEjectWait;

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CMEject: CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 卡号Check
        if (lpszCardNo == nullptr || strlen(lpszCardNo) < 1 ||
            strlen(lpszCardNo) > 20)
        {
            Log(ThisModule, -1, "CMEject: CardNo[%s] Is Invalid(==NULL||<1). Return: %d.",
                lpszCardNo, WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }

        // CRM退卡前检查读卡器内是否有卡(读卡器命令)
        IDC_IDCSTAUTS enCardstatus = IDCSTATUS_UNKNOWN;
        INT nStatRet =  m_pDev->DetectCard(enCardstatus);
        if (nStatRet >= 0)
        {
            if (enCardstatus != IDCSTAUTS_NOCARD)
            {
                Log(ThisModule, -1, "CMEject: Check CardReader Is Have Card: ->DetectCard(), "
                                    "CardStat = %d(0:无卡), Return: %d.",
                    enCardstatus, WFS_ERR_DEV_NOT_READY);
                return WFS_ERR_DEV_NOT_READY;
            }
        } else
        {
            Log(ThisModule, -1, "CMEject: Check CardReader Is Have Card: ->DetectCard() Fail,"
                                " ErrCode: %d|%s, Return: %d.",
                nStatRet, ProcessReturnCode(nStatRet), WFS_ERR_HARDWARE_ERROR);
            return WFS_ERR_HARDWARE_ERROR;
        }


        // 检查卡号是否存在
        if ((nCRMRet = m_pCRMDev->GetData((void*)lpszCardNo, CARDNO_ISEXIST)) == ERR_CRM_CARD_NOTFOUND)
        {
            Log(ThisModule, -1, "CMEject: CardNo(%s): ->GetData() Is Existenec,"
                                " ErrCode: %d, Return: %d.",
                lpszCardNo, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }
        // CRM退卡时要求与读卡器同时动作
        // 建立线程，循环下发读卡器退卡命令，等待退卡模块退卡动作
        m_CardReaderEjectFlag = WFS_ERR_HARDWARE_ERROR;
        m_bThreadEjectExit = FALSE; // 指示线程结束标记
        if (!m_thRunEjectWait.joinable())
        {
            m_thRunEjectWait = std::thread(&CXFS_IDC::ThreadEject_Wait, this);
            if (m_thRunEjectWait.joinable())
            {
                m_thRunEjectWait.detach();
            }
        }

        // 执行退卡
        nCRMRet = m_pCRMDev->CMEjectCard(lpszCardNo);
        if ((nCRMRet) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CMEject: CardNo[%s] Eject: ->CMEjectCard(%s) Fail. "
                                "CRM RetCode: %d, Return: %d.",
                lpszCardNo, lpszCardNo, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            m_bThreadEjectExit = TRUE;  // CRM ERR,结束线程(循环下发读卡器退卡)
            return CRMErr2XFSErrCode(nCRMRet);
        } else
        {
            WORD wCount = 0;
            while(wCount < (3 * 1000 / 20)) // 循环等待3秒,确保线程循环正确结束
            {
                if (m_bThreadEjectExit == TRUE)
                {
                    break;
                }

                usleep(1000 * 20);
                wCount ++;
            }

            if (m_bThreadEjectExit != TRUE)
            {
                Log(ThisModule, -1, "CMEject: CardNo[%s] CardReader Eject(内部保留): "
                                    "->MoveCardToMMPosition() TimeOut. Return: %d.",
                    lpszCardNo, WFS_ERR_TIMEOUT);
                return WFS_ERR_TIMEOUT;
            } else
            if (m_CardReaderEjectFlag != WFS_SUCCESS)
            {
                Log(ThisModule, -1, "CMEject: CardNo[%s] CardReader Eject(内部保留): "
                                    "->MoveCardToMMPosition() Fail. Return: %d.",
                    lpszCardNo, m_CardReaderEjectFlag);
                return m_CardReaderEjectFlag;
            }
        }

        // 退卡成功，结束线程(循环下发读卡器退卡)
        m_bThreadEjectExit = TRUE;

        // INI设置退卡到卡口(读卡器命令)
        // 3. 退卡后卡位置(0读卡器内部;1读卡器前入口,缺省0）
        if (m_stCRMINIConfig.wEjectCardPos == 1)
        {
            nIDCRet = EjectCard(0);
            if (nIDCRet != WFS_SUCCESS)
            {
                Log(ThisModule, -1, "CMEject: CardNo[%s] CardReader Eject(出口): ->EjectCard(0) Fail. "
                                    "Return: %d.",
                    lpszCardNo, Convert2XFSErrCode(nIDCRet));
                return Convert2XFSErrCode(nIDCRet);
            }

        }

        // 卡CMRetain进入回收时,读卡器回收计数+1; CMEject退卡成功时,读卡器回收计数-1
        UpdateRetainCards(MCM_DEC_ONE);

        // 移动暂存仓回到初始位置
        INT nSlotNo = 0;
        if ((nCRMRet = m_pCRMDev->CMCardSlotMove(0, nSlotNo)) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CMEject: Move Slot To InitPos: ->CMCardSlotMove(0, 0) Fail. "
                                "CRM RetCode: %d, Return: %d.",
                nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-执行CMRetainCard前设置收卡卡号
HRESULT CXFS_IDC::CMSetCardData(LPCSTR lpszCardNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;

    m_bIsSetCardData = FALSE;

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 收卡卡号Check
        if (lpszCardNo == nullptr || strlen(lpszCardNo) < 12 ||
            strlen(lpszCardNo) > 20)
        {
            Log(ThisModule, -1, "CMSetCardData: CardData(%s) Is Invalid(==NULL||<12||>20). Return: %d.",
                WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }

        // 检查卡号是否存在
        if ((nCRMRet = m_pCRMDev->GetData((void*)lpszCardNo, CARDNO_ISEXIST)) == ERR_CRM_CARD_ISEXIST)
        {
            Log(ThisModule, -1, "CMSetCardData: CardData(%s) Is Existenec. ErrCode: %d, Return: %d.",
                lpszCardNo, nCRMRet, WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }

        memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));
        memcpy(m_szStorageCardNo, lpszCardNo, strlen(lpszCardNo));
        m_bIsSetCardData = TRUE;        // 已执行SetCardData命令标记=T

    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-执行收卡/暂存
HRESULT CXFS_IDC::CMRetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    INT nIDCRet = WFS_SUCCESS;
    int nSlotNo = 0;

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 检查 已执行SetCardData命令标记
        if (m_bIsSetCardData != TRUE)
        {
            Log(ThisModule, -1, "CMRetain: Command(CMSetCardData) Not Run. Return: %d.",
                WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }
        m_bIsSetCardData = FALSE;

        // 收卡卡号Check
        if (m_szStorageCardNo == nullptr || strlen(m_szStorageCardNo) < 12 ||
            strlen(m_szStorageCardNo) > 54)
        {
            Log(ThisModule, -1, "CMRetain: CardNo(%s) Is Invalid(==NULL||<12||>54). Return: %d.",
                m_szStorageCardNo, WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }

        // ----------------执行收卡----------------
        // 1. 检查读卡器中是否有卡(读卡器命令)
        UpdateCardStatus();

        // INI设置卡在入口支持CMRetain
        /*if (m_stCRMINIConfig.wEnterCardRetainSup == 1)
        {
            if (m_Cardstatus == IDCSTAUTS_ENTERING) // 卡在入口
            {
                // 读卡器入口卡移动到内部(读卡器命令)
                nIDCRet = MoveCardToMMPosition();
                if (nIDCRet != WFS_SUCCESS)
                {
                    Log(ThisModule, -1,
                        "CMRetain: Move Card To CardReader Internal: "
                        "->MoveCardToMMPosition() Fail. Return: %d.",
                        nIDCRet);
                    return nIDCRet;
                }
            }
        }*/

        if (m_Status.fwMedia != WFS_IDC_MEDIAPRESENT)  // 读卡器中无卡
        {
            if (m_Status.fwMedia == WFS_IDC_MEDIAJAMMED)
            {
                Log(ThisModule, -1, "CMRetain: CardReader Is Have Card，MediaJAM. Return: %d.",
                    WFS_ERR_IDC_MEDIAJAM);
                return WFS_ERR_IDC_MEDIAJAM;
            }

            Log(ThisModule, -1, "CMRetain: CardReader Is No Have Card. Return: %d.",
                WFS_ERR_IDC_NOMEDIA);
            return WFS_ERR_IDC_NOMEDIA;
        }

        nIDCRet = MoveCardToMMPosition();
        if(nIDCRet != WFS_SUCCESS){
            return nIDCRet;
        }
        // 2. 移动空置暂存仓对准读卡器后出口
        if ((nCRMRet = m_pCRMDev->CMCardSlotMove(1, nSlotNo)) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CMRetain: Move SlotNo[%d] To CardReader AfterDoor: ->CMCardSlotMove(1, %d) Fail. "
                                "CRM RetCode: %d, Return: %d.",
                nSlotNo, nSlotNo, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        // 3. 读卡器中卡移动到后出口(读卡器命令),读卡器回收计数+1
        nIDCRet = RetainCard();
        if (nIDCRet != WFS_SUCCESS)
        {
            Log(ThisModule, -1, "CMRetain: Move Card Is CardReader To AfterDoor: "
                                "->RetainCard() Fail. Return: %d.",
                nIDCRet);
            return nIDCRet;
        }

        // 4. CRM执行收卡/吞卡
        if ((nCRMRet = m_pCRMDev->CMRetainCard(m_szStorageCardNo, nSlotNo)) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CMRetain: Retain Card[%s] To SlotNo[%d]: ->CMRetainCard(%s, %d) Fail. "
                                "CRM RetCode: %d, Return: %d.",
                m_szStorageCardNo, nSlotNo, m_szStorageCardNo, nSlotNo, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }
        memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-获取状态
HRESULT CXFS_IDC::CMStatus(BYTE lpucQuery[118], BYTE lpucStatus[118])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STCRMSTATUS stCRMStat;
    INT nCRMRet = CRM_SUCCESS;

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 获取CRM状态
        if ((nCRMRet = m_pCRMDev->GetStatus(stCRMStat)) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "Get CRM Status Fail. CRM RetCode: %d, Return: %d.",
                nCRMRet, CRMErr2XFSErrCode(nCRMRet));
//            return CRMErr2XFSErrCode(nCRMRet);
        }

        // 组织状态应答(118byte,分解如下)
        // 1-20:暂存仓1卡号; 21-40:暂存仓2卡号; 41-60:暂存仓3卡号; 61-90:暂存仓4卡号;
        // 81-100:暂存仓5卡号; 101-102:已暂存卡数目; 103-112:5个暂存仓senser状态;
        // 113-114:垂直电机状态; 115-116: 水平电机状态; 117-118:设备状态
        memset(lpucStatus, 0x00, 118);
        //memset(lpucStatus, 'F', 100);
        memset(lpucStatus, m_stCRMINIConfig.szPlaceHolder[0], 100);
        for (int i = 0; i < 5; i ++)
        {
            if (strlen(stCRMStat.szSlotCard[i]) > 0)
            {
                INT nLen = strlen(stCRMStat.szSlotCard[i]);
                memcpy(lpucStatus + i * 20, stCRMStat.szSlotCard[i],
                       nLen > 20 ? 20 : nLen);
            }
        }
        sprintf((CHAR*)(lpucStatus + 100), "%02d", stCRMStat.wStorageCount);
        for (int i = 0; i < 5; i ++)
        {
            //sprintf((CHAR*)(lpucStatus + 102 + i * 2), "%02d", stCRMStat.wSensorStat[i]);
            if (stCRMStat.wSensorStat[i] == 0)  // 无卡
            {
                memcpy((CHAR*)(lpucStatus + 102 + i * 2), m_stCRMINIConfig.szSlotNoHaveCard, 2);
            } else
            {
                memcpy((CHAR*)(lpucStatus + 102 + i * 2), m_stCRMINIConfig.szSlotHaveCard, 2);
            }
        }
        sprintf((CHAR*)(lpucStatus + 112), "%02d%02d%02d",
                stCRMStat.wVertPowerStat, stCRMStat.wHoriPowerStat, stCRMStat.wDeviceStat);
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-读卡器回收盒计数减1
HRESULT CXFS_IDC::CMReduceCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 读卡器回收盒计数减1(读卡器命令)
        UpdateRetainCards(MCM_DEC_ONE);

    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-设置读卡器回收盒最大计数
HRESULT CXFS_IDC::CMSetCount(LPWORD lpwCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 设置读卡器回收盒最大计数(读卡器命令)
        CINIFileReader ConfigFile;
        ConfigFile.LoadINIFile(SPIDCETCPATH);
        CINIWriter cINI = ConfigFile.GetWriterSection("DATA");
        cINI.SetValue(RETAIN_CARD_COUNT, (DWORD)*lpwCount);
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-吞卡到读卡器回收盒
HRESULT CXFS_IDC::CMEmptyCard(LPCSTR lpszCardBox)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    INT nIDCRet = WFS_SUCCESS;
    INT nSlotNo = 0;

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 暂存仓编号Check
        nSlotNo = atoi(lpszCardBox);

        if (nSlotNo == 0 && m_stCRMINIConfig.wEmptyAllCard0 != 0)   // 支持回收所有卡
        {
            CHAR szSlotNo[2];
            for (INT i = 1; i < 6; i ++)    // 循环回收所有卡
            {
                memset(szSlotNo, 0x00, sizeof(szSlotNo));
                sprintf(szSlotNo, "%d", i);
                // 检查暂存仓是否有卡
                if ((nCRMRet = m_pCRMDev->GetData((void*)szSlotNo, SLOTNO_ISEXIST)) == ERR_CRM_SLOT_NOTEXIST)
                {
                    Log(ThisModule, -1, "CMEmpty: SlotNo[%s] Is Not Have Card, continue.",szSlotNo);
                    continue;
                }

                // 回收卡
                if ((nIDCRet = InnerCMEmptyCard(i)) != WFS_SUCCESS)
                {
                    Log(ThisModule, -1, "CMEmpty: InnerCMEmptyCard(%d) fail , ErrCode: %d, Return: %d.",
                        i, nIDCRet, nIDCRet);
                    return nIDCRet;
                }
            }
        } else
        {
            if (nSlotNo < 1 || nSlotNo > 5)
            {
                Log(ThisModule, -1, "CMEmpty: SlotNo(%s|%d) Is Invalid(==NULL||<1||>5). Return: %d.",
                    lpszCardBox, nSlotNo, WFS_ERR_INVALID_DATA);
                return WFS_ERR_INVALID_DATA;
            }

            // 检查暂存仓是否有卡
            if ((nCRMRet = m_pCRMDev->GetData((void*)lpszCardBox, SLOTNO_ISEXIST)) == ERR_CRM_SLOT_NOTEXIST)
            {
                Log(ThisModule, -1, "CMEmpty: SlotNo[%s] Is Not Have Card: ->GetData(), ErrCode: %d, Return: %d.",
                    lpszCardBox, nCRMRet, WFS_ERR_IDC_CMNOMEDIA);
                return WFS_ERR_IDC_CMNOMEDIA;
            }

            // 回收卡
            if ((nIDCRet = InnerCMEmptyCard(nSlotNo)) != WFS_SUCCESS)
            {
                Log(ThisModule, -1, "CMEmpty: InnerCMEmptyCard(%d) fail , ErrCode: %d, Return: %d.",
                    nSlotNo, nIDCRet, nIDCRet);
                return nIDCRet;
            }
        }

        /* 该部分处理合在InnerCMEmptyCard()
        // CRM退卡前检查读卡器内是否有卡(读卡器命令)
        IDC_IDCSTAUTS enCardstatus = IDCSTATUS_UNKNOWN;
        nIDCRet =  m_pDev->DetectCard(enCardstatus);
        if (nIDCRet >= 0)
        {
            if (enCardstatus != IDCSTAUTS_NOCARD)
            {
                Log(ThisModule, -1, "CMEmpty: Check CardReader Is Have Card: ->DetectCard(), "
                                    "CardStat = %d(0:无卡), Return: %d.",
                    enCardstatus, WFS_ERR_DEV_NOT_READY);
                return WFS_ERR_DEV_NOT_READY;
            }
        } else
        {
            Log(ThisModule, -1, "CMEmpty: Check CardReader Is Have Card: ->DetectCard() Fail,"
                                " ErrCode: %d|%s, Return: %d.",
                nIDCRet, ProcessReturnCode(nIDCRet), WFS_ERR_HARDWARE_ERROR);
            return WFS_ERR_HARDWARE_ERROR;
        }

        // 检查读卡器回收盒是否FULL
        if (IsRetainCardFull() == TRUE)
        {
            Log(ThisModule, -1, "CMEmpty: CardReader Retain Is Full: ->IsRetainCardFull() Ret TRUE, "
                                "Return: %d.", WFS_ERR_IDC_RETAINBINFULL);
            return WFS_ERR_IDC_RETAINBINFULL;
        }

        // CRM退卡时要求与读卡器同时动作
        // 建立线程，循环下发读卡器退卡命令，等待退卡模块退卡动作
        m_CardReaderEjectFlag = WFS_ERR_HARDWARE_ERROR;
        m_bThreadEjectExit = FALSE; // 指示线程结束标记
        m_thRunEjectWait = std::thread(&CXFS_IDC::ThreadEject_Wait, this);
        if (m_thRunEjectWait.joinable())
        {
            m_thRunEjectWait.detach();
        }

        // 执行退卡(指定暂存仓编号)
        nCRMRet = m_pCRMDev->CMEjectCard(nSlotNo);
        if ((nCRMRet) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CMEmpty: SlotNo[%d] Eject: ->CMEjectCard(%d) Fail. "
                                "CRM RetCode: %d, Return: %d.",
                nSlotNo, nSlotNo, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            m_bThreadEjectExit = TRUE;  // CRM ERR,结束线程(循环下发读卡器退卡)
            return CRMErr2XFSErrCode(nCRMRet);
        } else
        {
            WORD wCount = 0;
            while(wCount < (3 * 1000 / 20)) // 循环等待3秒,确保线程循环正确结束
            {
                if (m_bThreadEjectExit == TRUE)
                {
                    break;
                }

                usleep(1000 * 20);
                wCount ++;
            }

            if (m_bThreadEjectExit != TRUE)
            {
                Log(ThisModule, -1, "CMEmpty: SlotNo[%d] CardReader Eject: ->MoveCardToMMPosition() TimeOut. "
                                    "Return: %d.",
                    nSlotNo, WFS_ERR_TIMEOUT);
                return WFS_ERR_TIMEOUT;
            } else
            if (m_CardReaderEjectFlag != WFS_SUCCESS)
            {
                Log(ThisModule, -1, "CMEmpty: SlotNo[%d] CardReader Eject: ->MoveCardToMMPosition() Fail. "
                                    "Return: %d.",
                    nSlotNo, m_CardReaderEjectFlag);
                return m_CardReaderEjectFlag;
            }
        }

        // 退卡成功，结束线程(循环下发读卡器退卡)
        m_bThreadEjectExit = TRUE;

        // 卡CMRetain进入回收时,读卡器回收计数+1; CMEject退卡成功时,读卡器回收计数-1
        UpdateRetainCards(MCM_DEC_ONE);

        // 移动暂存仓回到初始位置
        INT nSlotNo = 0;
        if ((nCRMRet = m_pCRMDev->CMCardSlotMove(0, nSlotNo)) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CMEject: Move Slot To InitPos: ->CMCardSlotMove(0, 0) Fail. "
                                "CRM RetCode: %d, Return: %d.",
                nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        // 卡收进回收盒+回收计数加1(读卡器命令)
        nIDCRet = RetainCard();
        if (nIDCRet != WFS_SUCCESS)
        {
            Log(ThisModule, -1, "CMRetain: Move Card Is CardReader To AfterDoor: "
                                "->RetainCard() Fail. Return: %d.",
                nIDCRet);
            return nIDCRet;
        }*/
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_IDC::InnerCMEmptyCard(INT nSlotNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    INT nIDCRet = WFS_SUCCESS;

    // CRM退卡前检查读卡器内是否有卡(读卡器命令)
    IDC_IDCSTAUTS enCardstatus = IDCSTATUS_UNKNOWN;
    nIDCRet =  m_pDev->DetectCard(enCardstatus);
    if (nIDCRet >= 0)
    {
        if (enCardstatus != IDCSTAUTS_NOCARD)
        {
            Log(ThisModule, -1, "InnerCMEmpty: Check CardReader Is Have Card: ->DetectCard(), "
                                "CardStat = %d(0:无卡), Return: %d.",
                enCardstatus, WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }
    } else
    {
        Log(ThisModule, -1, "InnerCMEmpty: Check CardReader Is Have Card: ->DetectCard() Fail,"
                            " ErrCode: %d|%s, Return: %d.",
            nIDCRet, ProcessReturnCode(nIDCRet), WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 检查读卡器回收盒是否FULL
    if (IsRetainCardFull() == TRUE)
    {
        Log(ThisModule, -1, "InnerCMEmpty: CardReader Retain Is Full: ->IsRetainCardFull() Ret TRUE, "
                            "Return: %d.", WFS_ERR_IDC_RETAINBINFULL);
        return WFS_ERR_IDC_RETAINBINFULL;
    }

    // CRM退卡时要求与读卡器同时动作
    // 建立线程，循环下发读卡器退卡命令，等待退卡模块退卡动作
    m_CardReaderEjectFlag = WFS_ERR_HARDWARE_ERROR;
    m_bThreadEjectExit = FALSE; // 指示线程结束标记
    m_thRunEjectWait = std::thread(&CXFS_IDC::ThreadEject_Wait, this);
    if (m_thRunEjectWait.joinable())
    {
        m_thRunEjectWait.detach();
    }

    // 执行退卡(指定暂存仓编号)
    nCRMRet = m_pCRMDev->CMEjectCard(nSlotNo);
    if ((nCRMRet) != CRM_SUCCESS)
    {
        Log(ThisModule, -1, "InnerCMEmpty: SlotNo[%d] Eject: ->CMEjectCard(%d) Fail. "
                            "CRM RetCode: %d, Return: %d.",
            nSlotNo, nSlotNo, nCRMRet, CRMErr2XFSErrCode(nCRMRet));
        m_bThreadEjectExit = TRUE;  // CRM ERR,结束线程(循环下发读卡器退卡)
        return CRMErr2XFSErrCode(nCRMRet);
    } else
    {
        WORD wCount = 0;
        while(wCount < (3 * 1000 / 20)) // 循环等待3秒,确保线程循环正确结束
        {
            if (m_bThreadEjectExit == TRUE)
            {
                break;
            }

            usleep(1000 * 20);
            wCount ++;
        }

        if (m_bThreadEjectExit != TRUE)
        {
            Log(ThisModule, -1, "InnerCMEmpty: SlotNo[%d] CardReader Eject: ->MoveCardToMMPosition() TimeOut. "
                                "Return: %d.",
                nSlotNo, WFS_ERR_TIMEOUT);
            return WFS_ERR_TIMEOUT;
        } else
        if (m_CardReaderEjectFlag != WFS_SUCCESS)
        {
            Log(ThisModule, -1, "InnerCMEmpty: SlotNo[%d] CardReader Eject: ->MoveCardToMMPosition() Fail. "
                                "Return: %d.",
                nSlotNo, m_CardReaderEjectFlag);
            return m_CardReaderEjectFlag;
        }
    }

    // 退卡成功，结束线程(循环下发读卡器退卡)
    m_bThreadEjectExit = TRUE;

    // 卡CMRetain进入回收时,读卡器回收计数+1; CMEject退卡成功时,读卡器回收计数-1
    UpdateRetainCards(MCM_DEC_ONE);

    // 移动暂存仓回到初始位置
    INT nInitSlotNo = 0;
    if ((nCRMRet = m_pCRMDev->CMCardSlotMove(0, nInitSlotNo)) != CRM_SUCCESS)
    {
        Log(ThisModule, -1, "InnerCMEmpty: Move Slot To InitPos: ->CMCardSlotMove(0, 0) Fail. "
                            "CRM RetCode: %d, Return: %d.",
            nCRMRet, CRMErr2XFSErrCode(nCRMRet));
        return CRMErr2XFSErrCode(nCRMRet);
    }

    // 卡收进回收盒+回收计数加1(读卡器命令)
    nIDCRet = RetainCard();
    if (nIDCRet != WFS_SUCCESS)
    {
        Log(ThisModule, -1, "InnerCMEmpty: Move Card Is CardReader To AfterDoor: "
                            "->RetainCard() Fail. Return: %d.",
            nIDCRet);
        return nIDCRet;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-获取吞卡时间
HRESULT CXFS_IDC::CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STCRMSLOTINFO stCRMInfo;
    INT nCRMRet = CRM_SUCCESS;

    CHAR szCardNo[20+1] = { 0x00 };
    CHAR szInTime[14+1] = { 0x00 };

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 获取CRM吞卡时间
        if ((nCRMRet = m_pCRMDev->GetCardSlotInfo(stCRMInfo)) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "Get CRM CardInfo Time Fail. CRM RetCode: %d, Return: %d.",
                nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        // 组织应答,每组数据38byte(DX-YYYYYYYYYYYYYYYYYYYY-YYYYMMDDHHmmss),解析如下:
        // X:暂存仓编号; YY...YY:20位卡号，不足右补F; YY...ss:14位时间
        for (int i = 0; i < 5; i ++)
        {
            //memset(szCardNo, 'F', sizeof(szCardNo) - 1);
            //memset(szInTime, 'F', sizeof(szInTime) - 1);
            memset(szCardNo, m_stCRMINIConfig.szPlaceHolder[0], sizeof(szCardNo) - 1);
            memset(szInTime, m_stCRMINIConfig.szPlaceHolder[0], sizeof(szInTime) - 1);

            if (stCRMInfo.bSlotHave[i] == TRUE)
            {
                INT nSize = strlen(stCRMInfo.szSlotCard[i]);
                memcpy(szCardNo, stCRMInfo.szSlotCard[i], nSize > 20 ? 20 : nSize);
                nSize = strlen(stCRMInfo.szStorageTime[i]);
                memcpy(szInTime, stCRMInfo.szStorageTime[i], nSize > 14 ? 14 : nSize);
            }
            sprintf(lpszCardInfo + i * 38, "D%d-%s-%s", i + 1, szCardNo, szInTime);
        }
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-设备复位
HRESULT CXFS_IDC::CMReset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, -1, "CMReset: CRM Device Handle Invalid(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            return WFS_ERR_DEV_NOT_READY;
        }

        long lRet = MoveCardToMMPosition();
        if(lRet != WFS_SUCCESS){
            return lRet;
        }

        // 执行设备复位
        if ((nCRMRet = m_pCRMDev->CMReset()) != CRM_SUCCESS)
        {
            Log(ThisModule, -1, "CMReset: CRM Device Reset: ->CMReset() Fail, "
                                "CRM RetCode: %d, Return: %d.",
                nCRMRet, CRMErr2XFSErrCode(nCRMRet));
            return CRMErr2XFSErrCode(nCRMRet);
        }

        m_bIsSetCardData = FALSE;     // RetainCard前需要执行SetCardData命令
        memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));

    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}


#endif

void CXFS_IDC::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_IDC::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_IDC::FireCardInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIAINSERTED, nullptr);
}

void CXFS_IDC::FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIAREMOVED, nullptr);
}

void CXFS_IDC::FireMediaRetained()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIARETAINED, nullptr);
}

void CXFS_IDC::FireRetainBinThreshold(WORD wReBin)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_IDC_RETAINBINTHRESHOLD, (LPVOID)&wReBin);
}

void CXFS_IDC::FireMediaDetected(WORD ResetOut)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIADETECTED, (LPVOID)&ResetOut);
}

void CXFS_IDC::FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData)
{
    WFSIDCTRACKEVENT data;
    data.fwStatus   = wStatus;
    data.lpstrTrack = pTrackName;
    data.lpstrData  = pTrackData;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDTRACKDATA, &data);
}


long CXFS_IDC::AcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //读卡时不选任何参数值返回DATAINVALID
    if ((dwReadOption & 0x803F) == 0)
    {
        Log(ThisModule, -1, "接收卡参数无效");
        return WFS_ERR_INVALID_DATA;
    }

//    if (m_Status.fwDevice == WFS_IDC_DEVHWERROR ||
//        m_Status.fwDevice == WFS_IDC_DEVOFFLINE ||
//        m_Status.fwDevice == WFS_IDC_DEVPOWEROFF)
//    {
//        Log(ThisModule, -1, "m_Status.fwDevice = %d",m_Status.fwDevice);
//        return WFS_ERR_HARDWARE_ERROR;
//    }

    if (m_Status.fwRetainBin == WFS_IDC_RETAINBINFULL && !m_stConfig.bAcceptWhenCardFull)
    {
        Log(ThisModule, -1, "接收卡时回收盒满");
        return WFS_ERR_IDC_RETAINBINFULL;
    }

    int nRet = UpdateCardStatus();
    if (nRet < 0)
    {
        Log(ThisModule, -1, "UpdateCardStatus failed %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (m_Cardstatus != IDCSTAUTS_INTERNAL)
    {
        //CMyCancelData cancelData(this);
        // 增加是否接受磁卡的判断
        BOOL bMagneticCard = TRUE;

        if ((dwReadOption & WFS_IDC_FLUXINACTIVE) || !m_stConfig.bFluxInActive)
        {
            bMagneticCard = FALSE;
        }
        Log(ThisModule, 1, "接收卡方式: dwReadOption & WFS_IDC_FLUXINACTIVE = %u, iFluxInActive = %d",
            dwReadOption & WFS_IDC_FLUXINACTIVE, m_stConfig.bFluxInActive);

        nRet = m_pDev->AcceptCard(dwTimeOut, bMagneticCard);
        UpdateDevStatus(nRet);
        if (nRet < 0)
        {
            //GetMediaPresentState(m_pDriver->DetectCard(IDCstatus));// 更新一次介质状态
            UpdateCardStatus();

            Log(ThisModule, 1, "进卡失败，返回码nRet = %d, %s", nRet, ProcessReturnCode(nRet));
            if (nRet == ERR_IDC_CARDTOOSHORT)
            {
                FireCardInserted();
                Log(ThisModule, -1, "CARDTOOSHORT");
                //return WFS_ERR_IDC_CARDTOOSHORT;              //30-00-00-00(FT#0043)
                return WFS_ERR_IDC_INVALIDMEDIA;                //30-00-00-00(FT#0043)
            }
            else if (nRet == ERR_IDC_CARDTOOLONG)
            {
                FireCardInserted();
                Log(ThisModule, -1, "CARDTOOLONG");
                //return WFS_ERR_IDC_CARDTOOLONG;               //30-00-00-00(FT#0043)
                return WFS_ERR_IDC_INVALIDMEDIA;                //30-00-00-00(FT#0043)
            } else if (nRet == ERR_IDC_CARDPULLOUT)
            {
                m_pDev->CancelReadCard();
                Log(ThisModule, -1, "CARDPULLOUT");
                return WFS_ERR_HARDWARE_ERROR;
            } else {
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
        else if ((nRet == ERR_IDC_INSERT_TIMEOUT) ||
                 (nRet == ERR_IDC_USER_CANCEL))
        {
            do
            {
                IDC_IDCSTAUTS IDCstatus;
                int iRetCard = m_pDev->DetectCard(IDCstatus);
                if (iRetCard < 0)
                {
                    break;
                }
                if (IDCstatus == IDCSTAUTS_INTERNAL)
                {
                    m_pDev->EjectCard();
                }
            } while (false);

            if (nRet == ERR_IDC_INSERT_TIMEOUT)
            {
                return WFS_ERR_TIMEOUT;
            }
            else
            {
                return WFS_ERR_CANCELED;
            }
        }
        else
        {
            int nIndex = 0;
            while((m_Cardstatus != IDCSTAUTS_INTERNAL) && (nIndex < 20))
            {
                nRet = UpdateCardStatus();
                if (nRet < 0)
                {
                    Log(ThisModule, -1, "检测卡失败:%s", ProcessReturnCode(nRet));
                    return WFS_ERR_HARDWARE_ERROR;
                }
                CQtTime::Sleep(100);
                nIndex++;
            }

            if (m_Cardstatus == IDCSTAUTS_INTERNAL)
            {
                Log(ThisModule, 1, "进卡成功");
                m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
                FireCardInserted();
            } else
            {
                m_Status.fwDevice = WFS_IDC_DEVHWERROR;
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
    } else {
        //创自读卡器在异常状态时，卡位置有可能不正确，移动到读卡位置
        if(m_strDevName == "CRT"){
            nRet = m_pDev->Init(CARDACTION_NOACTION, (WobbleAction)m_stConfig.usNeedWobble);
            UpdateDevStatus(nRet);
            if(nRet < 0){
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
    }

    m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
    m_Status.fwChipPower = WFS_IDC_CHIPNODEVICE;


    bool bTrack = ((dwReadOption & WFS_IDC_TRACK1) || (dwReadOption & WFS_IDC_TRACK2)  || (dwReadOption & WFS_IDC_TRACK3));
    bool bChip = (dwReadOption & WFS_IDC_CHIP);

    int nTrackRet = WFS_SUCCESS;
    int nChipRet  = WFS_SUCCESS;

    if (!bTrack && !bChip)
    {
        return WFS_ERR_UNSUPP_DATA;
    }

    if (bTrack)  //磁卡操作
    {
        nTrackRet = ReadTrackData(dwReadOption);        
    }

    CQtTime::Sleep(500);
    if (bChip) // IC卡操作
    {
        nChipRet = ReadChip();
        if (nChipRet != WFS_SUCCESS)
        {
            if(nChipRet == WFS_ERR_IDC_MEDIAJAM){
                UpdateDevStatus(ERR_IDC_JAMMED);
                m_Status.fwMedia = WFS_IDC_MEDIAJAMMED;
            } else {
                SetTrackInfo(WFS_IDC_CHIP, WFS_IDC_DATAMISSING, 0, NULL);
            }
        }
    }

//    if(m_stCRMINIConfig.bCRMDeviceSup){
//        INT nRet = MoveCardToMMPosition(); // 卡移动到读卡器内部(读卡器命令)
//        if (nRet != WFS_SUCCESS)
//        {
//            Log(ThisModule, 1, "卡移动到读卡器内部, Fail.");
//            return nRet;
//        }
//    }
    if ((bTrack && nTrackRet == WFS_SUCCESS) || (bChip && nChipRet == WFS_SUCCESS))
    {
        return WFS_SUCCESS;
    }
    else
    {        
        return nTrackRet != WFS_SUCCESS ? nTrackRet : nChipRet;
    }
}

long CXFS_IDC::Convert2XFSErrCode(long lIDCErrCode)
{
    switch (lIDCErrCode)
    {
    case ERR_IDC_SUCCESS:                 return WFS_SUCCESS;
    case ERR_IDC_INSERT_TIMEOUT:          return WFS_ERR_TIMEOUT;
    case ERR_IDC_USER_CANCEL:             return WFS_ERR_CANCELED;
    case ERR_IDC_COMM_ERR:                return WFS_ERR_CONNECTION_LOST;
    case ERR_IDC_JAMMED:                  return WFS_ERR_IDC_MEDIAJAM;
    case ERR_IDC_OFFLINE:                 return WFS_ERR_CONNECTION_LOST;
    case ERR_IDC_NOT_OPEN:                return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_RETAINBINFULL:           return WFS_ERR_IDC_RETAINBINFULL;
    case ERR_IDC_HWERR:                   return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_STATUS_ERR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_UNSUP_CMD:               return WFS_ERR_UNSUPP_COMMAND;
    case ERR_IDC_PARAM_ERR:               return WFS_ERR_INVALID_DATA;
    case ERR_IDC_READTIMEOUT:             return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_WRITETIMEOUT:            return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_READERROR:               return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_WRITEERROR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_INVALIDCARD:             return WFS_ERR_IDC_INVALIDMEDIA;
    case ERR_IDC_NOTRACK:                 return WFS_ERR_IDC_INVALIDMEDIA;
    case ERR_IDC_CARDPULLOUT:             return WFS_ERR_USER_ERROR;
    case ERR_IDC_CARDTOOSHORT:            return WFS_ERR_IDC_CARDTOOSHORT;
    case ERR_IDC_CARDTOOLONG:             return WFS_ERR_IDC_CARDTOOLONG;
    case ERR_IDC_WRITETRACKERROR:         return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_PROTOCOLNOTSUPP:         return WFS_ERR_UNSUPP_DATA;
    case ERR_IDC_ICRW_ERROR:              return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_NO_DEVICE:               return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_OTHER:                   return WFS_ERR_HARDWARE_ERROR;
    case ERR_IDC_USERERR:                 return WFS_ERR_USER_ERROR;
    case ERR_IDC_TAMPER:                  return WFS_ERR_USER_ERROR;
    default:                              return WFS_ERR_HARDWARE_ERROR;
    }
}

void CXFS_IDC::UpdateRetainCards(MODIFY_CARDS_MODE mode, bool fire_threshold/* = true*/)
{
    if (MCM_INC_ONE == mode)
    {
        m_Status.usCards++;
    }
    else if (MCM_DEC_ONE == mode)
    {
        if (m_Status.usCards > 0)
        {
            m_Status.usCards--;
        }
    }
    else if (MCM_CLEAR == mode)
    {
        m_Status.usCards = 0;
    }

    // 数据持久化
    CINIFileReader ConfigFile;
    ConfigFile.LoadINIFile(SPIDCETCPATH);
    CINIWriter cINI = ConfigFile.GetWriterSection("DATA");
    cINI.SetValue(RETAIN_CARD_COUNT, m_Status.usCards);
    //m_cXfsReg.SetValue("CONFIG", RETAIN_CARD_COUNT, m_Status.usCards);

    WORD fwRetainBin = 0;
    if (m_Status.usCards < (m_Caps.usCards * 3 / 4))
    {
        fwRetainBin = WFS_IDC_RETAINBINOK;
    }
    else if (m_Status.usCards < m_Caps.usCards)
    {
        fwRetainBin = WFS_IDC_RETAINBINHIGH;
    }
    else
    {
        fwRetainBin = WFS_IDC_RETAINBINFULL;
    }

    if (fwRetainBin != m_Status.fwRetainBin)
    {
        m_Status.fwRetainBin = fwRetainBin;

        if (fire_threshold) FireRetainBinThreshold(m_Status.fwRetainBin);
    }
}

const char *CXFS_IDC::ProcessReturnCode(int nCode)
{
    switch (nCode)
    {
    case ERR_IDC_SUCCESS:       return "操作成功";
    case ERR_IDC_INSERT_TIMEOUT: return "进卡超时";
    case ERR_IDC_USER_CANCEL:   return "用户取消";
    case ERR_IDC_PARAM_ERR:     return "参数错误";
    case ERR_IDC_COMM_ERR:      return "通讯错误";
    case ERR_IDC_STATUS_ERR:    return "读卡器状态出错";
    case ERR_IDC_JAMMED:        return "读卡器堵卡";
    case ERR_IDC_OFFLINE:       return "读卡器脱机";
    case ERR_IDC_NOT_OPEN:      return "没有调用Open";
    case ERR_IDC_RETAINBINFULL: return "回收箱满";
    case ERR_IDC_HWERR:         return "硬件故障";
    case ERR_IDC_READTIMEOUT:   return "读数据超时";
    case ERR_IDC_READERROR:     return "读数据错误";
    case ERR_IDC_INVALIDCARD:   return "无效芯片，或磁道数据无效";
    case ERR_IDC_NOTRACK:       return "非磁卡，未检测到磁道";
    case ERR_IDC_CARDPULLOUT:   return "接收卡时，卡被拖出";
    case ERR_IDC_CARDTOOSHORT:  return "卡太短";
    case ERR_IDC_CARDTOOLONG:   return "卡太长";
    case ERR_IDC_PROTOCOLNOTSUPP: return "不支持的IC通讯协议";
    case ERR_IDC_NO_DEVICE:     return "指定名的设备不存在";
    case ERR_IDC_OTHER:         return "其它错误，如调用API错误等";
    default:                    return "未定义错误码";
    }
}

long CXFS_IDC::WaitItemTaken()
{
    IDC_IDCSTAUTS lastCardStatus = m_Cardstatus;
    UpdateCardStatus();
    if ((lastCardStatus == IDCSTAUTS_ENTERING) &&
        (m_Cardstatus == IDCSTAUTS_NOCARD))
    {
        //FireMediaRemoved();
    }
    return 0;
}

long CXFS_IDC::UpdateCardStatus()
{
    THISMODULE(__FUNCTION__);

    WORD fwMedia = WFS_IDC_MEDIAUNKNOWN;
    WORD fwChipPower = WFS_IDC_CHIPUNKNOWN;

    m_Cardstatus = IDCSTATUS_UNKNOWN;
    int iRetStatus =  m_pDev->DetectCard(m_Cardstatus);
    if (iRetStatus >= 0)
    {
        m_bICCActived = FALSE;
        switch (m_Cardstatus)
        {
        case IDCSTAUTS_NOCARD:
            {
                fwChipPower = WFS_IDC_CHIPNOCARD;
                fwMedia = WFS_IDC_MEDIANOTPRESENT;
                break;
            }
        case IDCSTAUTS_ENTERING:
            {
                fwMedia = WFS_IDC_MEDIAENTERING;
                fwChipPower = WFS_IDC_CHIPNOCARD;
                break;
            }
        case IDCSTAUTS_INTERNAL:
            {
                fwMedia = WFS_IDC_MEDIAPRESENT;
                if (!m_bICCActived)
                    //fwChipPower = WFS_IDC_CHIPNOCARD;
                    fwChipPower = WFS_IDC_CHIPPOWEREDOFF;
                else
                    fwChipPower = WFS_IDC_CHIPONLINE;
                break;
            }
        case IDCSTAUTS_ICC_PRESS:
            {
                fwChipPower = WFS_IDC_CHIPPOWEREDOFF;
                fwMedia = WFS_IDC_MEDIAPRESENT;
                //fwChipPower = WFS_IDC_CHIPNOCARD;
                break;
            }
        case IDCSTAUTS_ICC_POWEROFF:
            {
                fwMedia = WFS_IDC_MEDIAPRESENT;
                if (m_Status.fwChipPower  == WFS_IDC_CHIPONLINE ||
                    m_Status.fwChipPower  == WFS_IDC_CHIPPOWEREDOFF ||
                    m_Status.fwChipPower  == WFS_IDC_CHIPBUSY )
                {
                    fwChipPower = WFS_IDC_CHIPPOWEREDOFF;
                }
            break;
            }
        case IDCSTAUTS_ICC_ACTIVE:
            {
                m_bICCActived = TRUE;
                fwMedia = WFS_IDC_MEDIAPRESENT;
                //fwChipPower  = WFS_IDC_CHIPBUSY;
                fwChipPower  = WFS_IDC_CHIPONLINE;
                break;
            }
        default:
            {
                fwMedia = WFS_IDC_MEDIAUNKNOWN;
                fwChipPower = WFS_IDC_CHIPUNKNOWN;
                break;
            }
        }

        //如果为jam状态，重置状态
        if(m_bJamm){
            fwMedia = WFS_IDC_MEDIAJAMMED;
            fwChipPower = WFS_IDC_CHIPUNKNOWN;
        }
    }
    else
    {        
        Log(ThisModule, -1, "检测卡出错:%s", ProcessReturnCode(iRetStatus));
        UpdateDevStatus(iRetStatus);
    }

    //检查skimming异常
    //30-00-00-00(FS#0014) add start
    if(m_Status.fwDevice == WFS_IDC_DEVONLINE){
        if(m_pDev->GetSkimmingCheckStatus()){
            WORD fwDevice = WFS_IDC_DEVUSERERROR;
            FireStatusChanged(fwDevice);
            FireHWEvent(WFS_ERR_ACT_NOACTION, nullptr);
            m_Status.fwDevice = fwDevice;
        }
    } else {
        if(m_Status.fwDevice == WFS_IDC_DEVUSERERROR &&
           !m_pDev->GetSkimmingCheckStatus() && !m_bNeedRepair){
            WORD fwDevice = WFS_IDC_DEVONLINE;
            FireStatusChanged(fwDevice);
            m_Status.fwDevice = fwDevice;
        }
    }
    //30-00-00-00(FS#0014) add end

    m_Status.fwChipPower = fwChipPower;
    m_Status.fwMedia = fwMedia;    

    if (fwMedia == WFS_IDC_MEDIANOTPRESENT && m_WaitTaken == WTF_TAKEN)
    {
        FireMediaRemoved();
        Log(ThisModule, 1, "用户取走卡");
        m_WaitTaken = WTF_NONE;
    }
    return iRetStatus;
}

void CXFS_IDC::UpdateDevStatus(int iRet)
{
    THISMODULE(__FUNCTION__);

    WORD fwDevice = WFS_IDC_DEVHWERROR;
    DWORD dwHWAct = WFS_ERR_ACT_NOACTION;

    if (ERR_IDC_UNSUP_CMD == iRet   ||
        ERR_IDC_USER_CANCEL == iRet ||
        ERR_IDC_INSERT_TIMEOUT == iRet)
    {
        iRet = ERR_IDC_SUCCESS;
    }

    switch (iRet)
    {
    case ERR_IDC_SUCCESS:
        fwDevice = WFS_IDC_DEVONLINE;
        break;
    case ERR_IDC_PARAM_ERR:
    case ERR_IDC_READERROR:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_NOACTION;
        break;
    //case ERR_IDC_COMM_ERR:                    //30-00-00-00(FT#0019)
    case ERR_IDC_OFFLINE:
    case ERR_IDC_NO_DEVICE:
    case ERR_IDC_READTIMEOUT:
        fwDevice = WFS_IDC_DEVOFFLINE;
        dwHWAct = WFS_ERR_ACT_NOACTION;
        break;
    case ERR_IDC_STATUS_ERR:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_RESET;
        break;
    case ERR_IDC_JAMMED:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_RESET;
        m_bJamm = TRUE;
        break;
    case ERR_IDC_COMM_ERR:                      //30-00-00-00(FT#0019)
    case ERR_IDC_NOT_OPEN:
    case ERR_IDC_HWERR:
    case ERR_IDC_OTHER:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_HWCLEAR;
        break;
    case ERR_IDC_RETAINBINFULL:
        fwDevice = WFS_IDC_DEVONLINE;
        break;
    case ERR_IDC_USERERR:
        fwDevice = WFS_IDC_DEVUSERERROR;
        break;
    default:
        fwDevice = WFS_IDC_DEVHWERROR;
        dwHWAct = WFS_ERR_ACT_RESET;
        break;
    }

    if (iRet < 0)
    {
        m_bNeedRepair = TRUE;
    }

    // 防盗嘴
    //***********************异型口感应器状态************************
    if (m_stConfig.bTamperSensorSupport)
    {
        //DWORD dwTamperSensorStatus = TANMPER_SENSOR_NOT_AVAILABLE;
    }

    //***********************设备状态*****************************
    /*if (m_nEjectedCard == EJECT_FAILED || m_nEatCard == EAT_FAILED)
    {
        fwDevice = WFS_IDC_DEVHWERROR;
    }*/

    if (m_Status.fwDevice != fwDevice)
    {
        FireStatusChanged(fwDevice);
        if (fwDevice != WFS_IDC_DEVONLINE && fwDevice != WFS_IDC_DEVBUSY)
        {
            FireHWEvent(dwHWAct, nullptr);
            m_bNeedRepair = TRUE;
        }
    }
    m_Status.fwDevice = fwDevice;
}
/*
// 更新扩展数据
void CXFS_IDC::UpdateExtra()
{
    CHAR szFWVersion[64] = { 0x00 };
    LONG lFWVerSize = 0;

    // 组织状态扩展数据
    //m_cStatExtra.Clear();
    m_cStatExtra.AddExtra("VRTCount", "2");
    m_cStatExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cStatExtra.AddExtra("VRTDetail[01]", (char*)byDevIDCVRTU);

    // 取固件版本写入扩展数据
    m_pDev->GetVersion(szFWVersion, 64, 0);
    if (strlen(szFWVersion) > 0)
    {
        m_cStatExtra.AddExtra("VRTCount", "3");
        m_cStatExtra.AddExtra("VRTDetail[02]", szFWVersion);
    }

    m_cCapsExtra.Clear();
    m_cCapsExtra.CopyFrom(m_cStatExtra);
}
*/
WORD CXFS_IDC::GetMediaPresentState(IDC_IDCSTAUTS CardStatus)
{
    switch (CardStatus)
    {
    case IDCSTAUTS_NOCARD:
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPNOCARD;
            m_Status.fwMedia = WFS_IDC_MEDIANOTPRESENT;
            break;
        }
    case IDCSTAUTS_ENTERING:    m_Status.fwMedia = WFS_IDC_MEDIAENTERING;   break;
    case IDCSTAUTS_INTERNAL:    m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;    break;

    default:
        {
            m_Status.fwChipPower  = WFS_IDC_CHIPUNKNOWN;
            m_Status.fwMedia = WFS_IDC_MEDIAUNKNOWN;
            break;
        }
    }
    return  m_Status.fwMedia;
}

long CXFS_IDC::ReadChip()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    int nRet = 0;
    int nIndex = 0;

    // 压下IC卡触点  
    nRet = m_pDev->ICCPress();
    if (ERR_IDC_HWERR == nRet)
    {
        UpdateDevStatus(nRet);
    }

    if (nRet != 0)
    {
        Log("ICCPress", -1, "ReadChip失败:%s", ProcessReturnCode(nRet));
        return Convert2XFSErrCode(nRet);
    }
    else
    {
        m_Status.fwChipPower  = WFS_IDC_CHIPNODEVICE;
        m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
    }

    // 激活IC卡
    char szATRInfo[MAX_LEN_ATRINFO] = {0};
    unsigned int nATRLen = sizeof(szATRInfo);
    m_pDev->ICCActive(szATRInfo, nATRLen);
    //if (ERR_IDC_HWERR == nRet)
    //{
        //UpdateDevStatus(nRet);
    //}
    //if (nRet != 0)
    //{
        //m_Status.fwChipPower  = WFS_IDC_CHIPNODEVICE;
        //Log("ICCActive", -1, "ReadChip失败:%s", ProcessReturnCode(nRet));
        //return Convert2XFSErrCode(nRet);
    //}
    //else

    m_Status.fwChipPower  = WFS_IDC_CHIPONLINE;
    m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;

    while (nIndex != nATRLen)
    {
        if (szATRInfo[nIndex] == 0)
        {
            nIndex++;
        }
        else
        {
            break;
        }
    }

    if (nIndex == nATRLen)
        SetTrackInfo(WFS_IDC_CHIP, WFS_IDC_DATAMISSING, 0, NULL);
    else
        SetTrackInfo(WFS_IDC_CHIP, WFS_IDC_DATAOK, nATRLen, (BYTE *)szATRInfo);

    return WFS_SUCCESS;
}


int CXFS_IDC::Init()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = m_pDev->Init(CARDACTION_NOACTION, (WobbleAction)m_stConfig.usNeedWobble);
    UpdateDevStatus(nRet);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "初始化读卡器出错1");
        return WFS_ERR_HARDWARE_ERROR;
    }
    else
    {
        Log(ThisModule, 0, "初始化读卡器成功");
    }

    UpdateCardStatus();
    if (m_Cardstatus != IDCSTAUTS_NOCARD &&
        m_Cardstatus != IDCSTATUS_UNKNOWN)
    {
        nRet = m_pDev->Init(m_stConfig.CardInitAction, (WobbleAction)m_stConfig.usNeedWobble);
        UpdateDevStatus(nRet);
        UpdateCardStatus();
        if (nRet < 0)
        {
            Log(ThisModule, -1, "初始化读卡器出错,Action:%d", m_stConfig.CardInitAction);
            return WFS_ERR_HARDWARE_ERROR;
        }
        else
        {
            if ((m_Cardstatus == IDCSTAUTS_NOCARD) &&
                (m_stConfig.CardInitAction == CARDACTION_RETRACT))
            {
                UpdateRetainCards(MCM_INC_ONE, false);
                FireMediaDetected(WFS_IDC_CARDRETAINED);
            }
            Log(ThisModule, 0, "初始化读卡器成功");
        }
    }

    return WFS_SUCCESS;
}

int CXFS_IDC::GetFWVersion()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    char szFWVersion[10][MAX_LEN_FWVERSION] = {0};
    unsigned int uLen = sizeof(szFWVersion);
    int nRet = m_pDev->GetFWVersion(szFWVersion[0], uLen);
    UpdateDevStatus(nRet);
    if (nRet < 0)
        return WFS_ERR_HARDWARE_ERROR;
    int count = 2, row = 0;
    char detail[255];
    for(row = 0; row < 10; row++)
    {
        if(strlen(szFWVersion[row]) != 0)
        {
            char ver[255] = "VERDetail[";
            memset(detail, 0, sizeof(detail));
            if(count < 10){
                sprintf(detail, "0%d", count);
            }
            else{
                sprintf(detail, "%d", count);
            }
            strcat(ver, detail);
            strcat(ver, "]");
            Log(ThisModule, 0, "设备固件版本信息%s", szFWVersion[row]);
            m_cExtra.AddExtra(ver, szFWVersion[row]);
            count++;
        }
    }
    sprintf(detail, "%d", count);
    m_cExtra.AddExtra("VERCount", detail);

   // Log(ThisModule, 0, "设备固件版本信息：IC:%s, EMV:%s, MT:%s", szFWVersion, szEMVVersion, szGIEVersion);
    return nRet;
}

void CXFS_IDC::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 新增IC卡操作
    m_Status.fwChipPower  = WFS_IDC_CHIPNODEVICE;
    m_Status.fwMedia = WFS_IDC_MEDIANOTPRESENT;
    m_Status.fwSecurity = WFS_IDC_SECNOTSUPP;

    m_Status.lpszExtra = NULL;

    if (m_Caps.usCards == 0)
    {
        m_Status.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    }
    else
    {
        if (m_Status.usCards < (m_Caps.usCards * 3 / 4))
        {
            m_Status.fwRetainBin = WFS_IDC_RETAINBINOK;
        }
        else if (m_Status.usCards < m_Caps.usCards)
        {
            m_Status.fwRetainBin = WFS_IDC_RETAINBINHIGH;
        }
        else
        {
            m_Status.fwRetainBin = WFS_IDC_RETAINBINFULL;
        }
    }
}

void CXFS_IDC::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Caps.wClass = WFS_SERVICE_CLASS_IDC;
    m_Caps.fwType = WFS_IDC_TYPEMOTOR;
    m_Caps.bCompound = FALSE;
    m_Caps.fwReadTracks = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
    m_Caps.fwWriteTracks = m_stConfig.bCanWriteTrack ? (WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3) : WFS_IDC_NOTSUPP;
    // 新增IC卡操作
    m_Caps.fwChipProtocols = WFS_IDC_CHIPT0 | WFS_IDC_CHIPT1;
    m_Caps.fwChipPower = WFS_IDC_CHIPPOWERCOLD | WFS_IDC_CHIPPOWERWARM | WFS_IDC_CHIPPOWEROFF;
    m_Caps.fwSecType = WFS_IDC_SECNOTSUPP;
    m_Caps.fwPowerOnOption = WFS_IDC_RETAIN;    // NoAction -> Retain
    m_Caps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_Caps.fwWriteMode = m_stConfig.bCanWriteTrack ? (WFS_IDC_LOCO | WFS_IDC_HICO | WFS_IDC_AUTO) : WFS_IDC_NOTSUPP;
    //m_Caps.usCards                        = m_cXfsReg.GetValue("CONFIG", RETAIN_CARD_MAXCOUNT, 20);
    m_Caps.bFluxSensorProgrammable        = m_cXfsReg.GetValue("CONFIG", FLUXABLE, 1) == 1;
    m_Caps.bReadWriteAccessFollowingEject = m_cXfsReg.GetValue("CONFIG", WRACCFOLLEJECT, 1) == 1;
    m_Caps.lpszExtra = NULL;
}

void CXFS_IDC::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
{
    WFSIDCCARDDATA data;
    data.wDataSource    = wSource;
    data.wStatus        = wStatus;
    data.ulDataLength   = uLen;
    data.lpbData        = pData;
    data.fwWriteMethod  = 0;

    m_CardDatas.SetAt(wSource, data);
    return;
}

bool CXFS_IDC::GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho)
{
    LPWFSIDCCARDDATA pCardData = m_CardDatas.GetAt(wSource);
    if (!pCardData)
    {
        *pLen = 0;
        return FALSE;
    }

    if (*pLen < pCardData->ulDataLength)
    {
        *pLen = pCardData->ulDataLength;
        return false;
    }

    *pLen           = pCardData->ulDataLength;
    *pWriteMetho    = pCardData->fwWriteMethod;
    memcpy(pData, pCardData->lpbData, *pLen);
    return true;
}

long CXFS_IDC::ReadTrackData(DWORD dwReadOption)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STTRACK_INFO    trackInfo;
    // ReadRawData前先将磁道初始化避免KAL出错崩溃
    if (dwReadOption & WFS_IDC_TRACK1)
    {
        SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAINVALID, 0, NULL);
    }
    if (dwReadOption & WFS_IDC_TRACK2)
    {
        SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAINVALID, 0, NULL);
    }
    if (dwReadOption & WFS_IDC_TRACK3)
    {
        SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAINVALID, 0, NULL);
    }

    int nRet = m_pDev->ReadTracks(trackInfo);
    if (nRet == 0)
    {
        Log("ReadTrack", __LINE__, "ReadTracks()调用成功");
        m_Status.fwMedia = WFS_IDC_MEDIAPRESENT;
        if (dwReadOption & WFS_IDC_TRACK1)
        {
            if (trackInfo.TrackData[0].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK1, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[0].szTrack), (BYTE *)trackInfo.TrackData[0].szTrack);
            }
        }

        if (dwReadOption & WFS_IDC_TRACK2)
        {
            if (trackInfo.TrackData[1].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK2, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[1].szTrack), (BYTE *)trackInfo.TrackData[1].szTrack);
            }
        }

        if (dwReadOption & WFS_IDC_TRACK3)
        {
            if (trackInfo.TrackData[2].bTrackOK)
            {
                SetTrackInfo(WFS_IDC_TRACK3, WFS_IDC_DATAOK, strlen(trackInfo.TrackData[2].szTrack), (BYTE *)trackInfo.TrackData[2].szTrack);
            }
        }
    }
    else if (nRet == ERR_IDC_NOTRACK)
    {
        return WFS_ERR_IDC_INVALIDMEDIA;
    }
    else if (nRet == ERR_IDC_CARDTOOSHORT)
    {
        return WFS_ERR_IDC_CARDTOOSHORT;
    }
    else if (nRet == ERR_IDC_CARDTOOLONG)
    {
        return WFS_ERR_IDC_CARDTOOLONG;
    }
    else if (nRet == ERR_IDC_READERROR)
    {
        return WFS_ERR_IDC_INVALIDDATA;
    }
    else if (nRet == ERR_IDC_INVALIDCARD)
    {
        return WFS_SUCCESS;
    }
    else if (nRet == ERR_IDC_JAMMED)
    {
        UpdateDevStatus(nRet);
        m_Status.fwMedia = WFS_IDC_MEDIAJAMMED;
        return WFS_ERR_IDC_MEDIAJAM;
    }
    else
    {
        UpdateDevStatus(nRet);
        Log("ReadTrack", -1, "ReadTrack时出错:%s", ProcessReturnCode(nRet));
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

bool CXFS_IDC::LoadDevDll(LPCSTR ThisModule)
{
    if (m_pDev == nullptr)
    {
        char szDevDllName[256] = { 0 };
        strcpy(szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));
        if (0 != m_pDev.Load(szDevDllName, "CreateIDevIDC", m_strDevName.c_str()))
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s", szDevDllName);
            return false;
        }
    }

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE)     // 支持启用退卡模块
    {
        // 加载退卡模块动态库
        if (!LoadCRMDevDll(ThisModule))
        {
            return false;
        }
    }

    return (m_pDev != nullptr);
}

int CXFS_IDC::InitConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    QByteArray strFile("/IDCConfig.ini");

#ifdef QT_WIN32
    strFile.prepend(SPETCPATH);
#else
    strFile.prepend(SPETCPATH);
#endif

     m_stConfig.Clear();

     CHAR szBuffer[1024] = { 0x00 };

    CINIFileReader configfile;
    if (!configfile.LoadINIFile(strFile.constData()))
    {
        Log(ThisModule, __LINE__, "CRT:加载配置文件失败：%s", strFile.constData());
        return -1;
    }

    CINIReader cINI = configfile.GetReaderSection("IDC");
    m_stConfig.CardInitAction       = (CardAction)(WORD)cINI.GetValue("InitAction", "4");
    m_stConfig.bAcceptWhenCardFull  = (WORD)cINI.GetValue("AcceptWhenFull", "0") == 1;
    m_stConfig.dwTotalTimeOut       = (WORD)cINI.GetValue("TotalTimeOut", "100") * 1000;
    m_stConfig.usNeedWobble         = (WORD)cINI.GetValue("NeedWobble", "1");
    m_stConfig.bCanWriteTrack       = (WORD)cINI.GetValue("CanWriteTrack", "1") == 1; //默认可以写磁道
    m_stConfig.bFireRmoveWhenRetain = (WORD)cINI.GetValue("FireRmoveWhenRetain", "0") == 1;
    m_stConfig.bAutoUpdateToOK      = (WORD)cINI.GetValue("AutoUpdateToOK", "1") == 1; // 默认允许状态自动恢复正常
    m_stConfig.usResetRetryTimes    = (WORD)cINI.GetValue("ResetRetryTimes", "0");
    m_stConfig.bFluxInActive        = (WORD)cINI.GetValue("FluxInActive", "1") == 1;   //没此配置项或0, 只进磁卡, 1进所有卡
    m_stConfig.bSuppDeRetractCount  = (WORD)cINI.GetValue("SuppDeRetractCount", "0") == 1;
    m_stConfig.bCaptureOnResetFailed = m_stConfig.usResetRetryTimes != 0;

    cINI = configfile.GetReaderSection("DATA");
    m_Status.usCards = cINI.GetValue(RETAIN_CARD_COUNT, 0);
    m_Caps.usCards = cINI.GetValue(RETAIN_CARD_MAXCOUNT, 20);

    cINI = configfile.GetReaderSection("OPEN");                                 //30-00-00-00(FT#0019)
    m_stConfig.usReturnOpenVal = (WORD)cINI.GetValue("ReturnOpenVal", "0");     //30-00-00-00(FT#0019)

    //--------------------退卡模块INI配置参数获取--------------------
    // 是否支持启用退卡模块: 0不支持/1支持,缺省0
    m_stCRMINIConfig.bCRMDeviceSup = ((WORD)m_cXfsReg.GetValue("CRM_DEVICE_CFG", "CRMDeviceSup", (DWORD)0) == 1);

    if (m_stCRMINIConfig.bCRMDeviceSup == TRUE)
    {
        // 底层设备控制动态库名(退卡模块)
        strcpy(m_stCRMINIConfig.szCRMDeviceDllName, m_cXfsReg.GetValue("CRMDriverDllName", ""));

        // 退卡模块设备类型(缺省0)
        m_stCRMINIConfig.wCRMDeviceType = (WORD)m_cXfsReg.GetValue("CRM_DEVICE_CFG", "DeviceType", (DWORD)0);

        // CMStatus/CMGetCardInfo命令数据占位符,缺省F
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue("CRM_DEVICE_CFG", "PlaceHolder", "F"));
        if (strlen(szBuffer) > 0)
        {
            memcpy(m_stCRMINIConfig.szPlaceHolder, szBuffer, 1);
        } else
        {
            memcpy(m_stCRMINIConfig.szPlaceHolder, "F", 1);
        }

        // 是否支持CMEmptyCard入参为0时回收所有卡(0:不支持, 1:支持, 缺省0)
        m_stCRMINIConfig.wEmptyAllCard0 = m_cXfsReg.GetValue("CRM_DEVICE_CFG", "CMEmptyAllCard0", (DWORD)0);

        // 卡槽有卡状态标记(仅限2位),缺省01
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue("CRM_DEVICE_CFG", "SlotHaveCard", "01"));
        if (strlen(szBuffer) > 1)
        {
            memcpy(m_stCRMINIConfig.szSlotHaveCard, szBuffer, 2);
        } else
        {
            memcpy(m_stCRMINIConfig.szSlotHaveCard, "01", 2);
        }

        // 卡槽无卡状态标记(仅限2位),缺省00
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue("CRM_DEVICE_CFG", "SlotNoHaveCard", "00"));
        if (strlen(szBuffer) > 1)
        {
            memcpy(m_stCRMINIConfig.szSlotNoHaveCard, szBuffer, 2);
        } else
        {
            memcpy(m_stCRMINIConfig.szSlotNoHaveCard, "00", 2);
        }

        CHAR szKeyName[32];
        sprintf(szKeyName, "CRM_DEVICE_SET_%d", m_stCRMINIConfig.wCRMDeviceType);

        if (m_stCRMINIConfig.wCRMDeviceType == CRM_DEV_CRT730B)
        {
            // 1. 设备连接串,缺省(USB:23D8,0730)
            strcpy(m_stCRMINIConfig.szCRMDeviceConList, m_cXfsReg.GetValue(szKeyName, "DeviceConList", "USB:23D8,0730"));

            // 2. 设备初始化动作(0正常归位;1强行退卡;2强行暂存;3无动作,缺省0)
            m_stCRMINIConfig.wDeviceInitAction = (WORD)m_cXfsReg.GetValue(szKeyName, "DeviceInitAction", (DWORD)0);
            if (m_stCRMINIConfig.wDeviceInitAction < 0 || m_stCRMINIConfig.wDeviceInitAction > 3)
            {
                m_stCRMINIConfig.wDeviceInitAction = 0;
            }

            // 3. 退卡后卡位置(0读卡器内部;1读卡器前入口,缺省0）
            m_stCRMINIConfig.wEjectCardPos = (WORD)m_cXfsReg.GetValue(szKeyName, "EjectCardPos", (DWORD)0);
            if (m_stCRMINIConfig.wEjectCardPos < 0 || m_stCRMINIConfig.wEjectCardPos > 1)
            {
                m_stCRMINIConfig.wEjectCardPos = 0;
            }

            // 4. 读卡器前入口有卡是否支持收入暂存仓（0不支持;1支持,缺省0
            m_stCRMINIConfig.wEnterCardRetainSup = (WORD)m_cXfsReg.GetValue(szKeyName, "EnterCardRetainSup", (DWORD)0);
            if (m_stCRMINIConfig.wEnterCardRetainSup < 0 || m_stCRMINIConfig.wEnterCardRetainSup > 1)
            {
                m_stCRMINIConfig.wEnterCardRetainSup = 0;
            }
        }
    }

    return 0;
}

// 退卡模块(CRM)-加载DevCRM动态库
bool CXFS_IDC::LoadCRMDevDll(LPCSTR ThisModule)
{
    if (m_pCRMDev == nullptr)
    {
        if (0 != m_pCRMDev.Load(m_stCRMINIConfig.szCRMDeviceDllName,
                                "CreateIDevCRM",
                                CRMDEVTYPE_CHG(m_stCRMINIConfig.wCRMDeviceType)))
        {
            Log(ThisModule, __LINE__,
                "加载退卡模块库失败: DriverDllName=%s, CRMDEVTYPE=%d. ReturnCode:%s",
                m_stCRMINIConfig.szCRMDeviceDllName, m_stCRMINIConfig.wCRMDeviceType,
                m_pCRMDev.LastError().toUtf8().constData());
            return false;
        }
    }

    return (m_pCRMDev != nullptr);
}

// 退卡模块(CRM)-退卡时要求与读卡器同时动作，该接口为读卡器后出口吸卡动作循环执行
void CXFS_IDC::ThreadEject_Wait()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD    wCount = 0;

    while(wCount < (WORD)(1000 * 3 / 20))
    {
        if (m_bThreadEjectExit == TRUE)
        {
            break;
        }
        wCount ++;
        m_CardReaderEjectFlag = MoveCardToMMPosition(FALSE); // 卡移动到读卡器内部(读卡器命令)
        if (m_CardReaderEjectFlag == WFS_SUCCESS)
        {
            break;
        }
        usleep(1000 * 20);
    }
    m_bThreadEjectExit = TRUE;
}

// 退卡模块(CRM)-读卡器回收盒是否FULL
BOOL CXFS_IDC::IsRetainCardFull()
{
    // 数据持久化
    CINIFileReader ConfigFile;
    ConfigFile.LoadINIFile(SPIDCETCPATH);
    CINIWriter cINI = ConfigFile.GetWriterSection("DATA");
    cINI.SetValue(RETAIN_CARD_COUNT, m_Status.usCards);

    WORD fwRetainBin = 0;
    // 卡CMRetain进入暂存仓也算入读卡器回收计数,所以当前读卡器回收计数>最大回收计数时算FULL
    if (m_Status.usCards > m_Caps.usCards)
    {
        return TRUE;
    }

    return FALSE;
}

// 退卡模块(CRM)-CRM ErrCode 转换为 XFS ErrCode
long CXFS_IDC::CRMErr2XFSErrCode(long lCRMErrCode)
{
    switch (lCRMErrCode)
    {
        case CRM_SUCCESS:                   return WFS_SUCCESS;
        case ERR_CRM_INSERT_TIMEOUT:        return WFS_ERR_TIMEOUT;             // CRM:进卡超时
        case ERR_CRM_USER_CANCEL:           return WFS_ERR_CANCELED;            // CRM:用户取消
        case ERR_CRM_COMM_ERR:              return WFS_ERR_CONNECTION_LOST;     // CRM:通讯错误
        case ERR_CRM_JAMMED:                return WFS_ERR_IDC_MEDIAJAM;        // CRM:堵卡
        case ERR_CRM_OFFLINE:               return WFS_ERR_CONNECTION_LOST;     // CRM:脱机
        case ERR_CRM_NOT_OPEN:              return WFS_ERR_HARDWARE_ERROR;      // CRM:没有打开
        case ERR_CRM_SLOT_FULL:             return WFS_ERR_IDC_RETAINBINFULL;   // CRM:卡箱满
        case ERR_CRM_HWERR:                 return WFS_ERR_HARDWARE_ERROR;      // CRM:硬件故障
        case ERR_CRM_STATUS_ERR:            return WFS_ERR_HARDWARE_ERROR;      // CRM:状态出错
        case ERR_CRM_SLOT_ISEXIST:          return WFS_ERR_CM_MEDIANOTEXIST;    // CRM:指定卡箱被占用
        case ERR_CRM_SLOT_NOTEXIST:         return WFS_ERR_IDC_CMNOMEDIA;       // CRM:指定卡箱没有被占用
        case ERR_CRM_UNSUP_CMD:             return WFS_ERR_UNSUPP_COMMAND;      // CRM:不支持的指令
        case ERR_CRM_PARAM_ERR:             return WFS_ERR_INVALID_DATA;        // CRM:参数错误
        case ERR_CRM_READTIMEOUT:           return WFS_ERR_HARDWARE_ERROR;      // CRM:读数据超时
        case ERR_CRM_WRITETIMEOUT:          return WFS_ERR_HARDWARE_ERROR;      // CRM:写数据超时
        case ERR_CRM_READERROR:             return WFS_ERR_HARDWARE_ERROR;      // CRM:读数据错
        case ERR_CRM_WRITEERROR:            return WFS_ERR_HARDWARE_ERROR;      // CRM:写数据错
        case ERR_CRM_CARD_NOTFOUND:         return WFS_ERR_IDC_CMNOMEDIA;       // CRM:指定卡不存在
        case ERR_CRM_CARD_ISEXIST:          return WFS_ERR_CM_MEDIANOTEXIST;    // CRM:指定卡已存在
        case ERR_CRM_LOAD_LIB:              return WFS_ERR_INTERNAL_ERROR;      // CRM:动态库错误
        case ERR_CRM_OTHER:                 return WFS_ERR_HARDWARE_ERROR;      // CRM:其他错误/未知错误
        default:                            return WFS_ERR_HARDWARE_ERROR;
    }
}

