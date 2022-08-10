/***************************************************************
* 文件名称：XFS_IDX_DEC.cpp
* 文件描述：身份证读卡器模块子命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月25日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_IDX.h"

//----------------------------命令子处理----------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_IDX::InnerOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    // Open前下传初始参数(非断线重连)
    if (bReConn == FALSE)
    {
        // 设置证件图像参数
        m_pDev->SetData(SET_IMAGE_PAR, &(m_stConfig.stImageParam));

        // 设置SDK路径
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pDev->SetData(SET_LIB_PATH, m_stConfig.szSDKPath);
        }

        // 设置SDK版本
        m_pDev->SetData(SET_LIB_VER, &(m_stConfig.wSDKVersion));

        // 设置设备打开模式
        //m_pDev->SetData(SET_DEV_OPENMODE, &(m_stConfig.stDevOpenMode));
    }

    // 打开设备
    nRet = m_pDev->Open(nullptr);
    if (nRet != IDC_SUCCESS)
    {
        if (bReConn == FALSE)
        {
            Log(ThisModule, __LINE__, "打开设备连接失败, ErrCode:%d, Return: %d",
                nRet, ConvertDevErrCode2WFS(nRet));
        }
        return ConvertDevErrCode2WFS(nRet);
    }

    // Reset
    if (m_stConfig.wOpenResetSupp == 1)
    {
        WORD wResetAction = (m_stConfig.wResetCardAction == 0 ? WFS_IDC_NOACTION :
                    (m_stConfig.wResetCardAction == 1 ? WFS_IDC_EJECT : WFS_IDC_RETAIN));

        HRESULT hRet = Reset(&wResetAction);
        if (hRet != WFS_SUCCESS)
        {
            if (m_stConfig.wResetFailReturn == 0)
            {
                Log(ThisModule, __LINE__, "打开设备连接后Reset失败．Return Error. ");
                return hRet;
            } else
            {
                Log(ThisModule, __LINE__, "打开设备连接后Reset失败．Not Return Error. ");
            }
        }
    }

    // 更新扩展状态
    UpdateExtra();

    // 更新一次状态
    //OnStatus();
    UpdateStatus();

    if (bReConn == TRUE)
    {
        Log(ThisModule, __LINE__, "断线重连成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}

// 读INI
int CXFS_IDX::InitConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD    wTmp = 0;
    CHAR    szTmp[256];
    CHAR    szIniAppName[MAX_PATH];

    // -----[default]下参数------------
    // 底层设备控制动态库名
    memset(&m_stConfig, 0x00, sizeof(STINICONFIG));
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));


    // -----[DEVICE_CONFIG]下参数------------
    // 设备类型(0/新北洋BS-ID81)
    m_stConfig.wDeviceType = (WORD)m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (DWORD)0);


    // -----[DEVICE_SET_0]下参数------------
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.wDeviceType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    // SDK版本,缺省0
    m_stConfig.wSDKVersion = (WORD)m_cXfsReg.GetValue(szIniAppName, "SDK_Version", (DWORD)0);

    // -----[OPEN_CONFIG]下参数------------
    // Open时是否执行Reset动作(0不执行/1执行,缺省0)
    m_stConfig.wOpenResetSupp = (WORD)m_cXfsReg.GetValue("OPEN_CONFIG", "OpenResetSupp", (DWORD)0);
    if (m_stConfig.wOpenResetSupp != 0 &&
        m_stConfig.wOpenResetSupp != 1)
    {
        m_stConfig.wOpenResetSupp = 0;
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.nOpenFailRet != 0 && m_stConfig.nOpenFailRet != 1)
    {
        m_stConfig.nOpenFailRet = 0;
    }


    // -----[RESET_CONFIG]下参数------------
    // Reset时卡动作(0无动作/1退卡/2吞卡,缺省0)
    m_stConfig.wResetCardAction = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetCardAction", (DWORD)0);
    if (m_stConfig.wResetCardAction != 0 &&
        m_stConfig.wResetCardAction != 1 &&
        m_stConfig.wResetCardAction != 2)
    {
        m_stConfig.wResetCardAction = 0;
    }

    // Reset失败时返回标准(0原样返回/1忽略失败和错误返回成功,缺省0)
    m_stConfig.wResetFailReturn = (WORD)m_cXfsReg.GetValue("RESET_CONFIG", "ResetFailReturn", (DWORD)0);
    if (m_stConfig.wResetFailReturn != 0 &&
        m_stConfig.wResetFailReturn != 1)
    {
        m_stConfig.wResetFailReturn = 0;
    }


    // -----[REJECT_CONFIG]下参数------------
    // 退卡时完全弹出/保持在门口(0保持在门口/1完全弹出,缺省0)
    m_stConfig.wEjectMode = (WORD)m_cXfsReg.GetValue("EJECT_CONFIG", "EjectMode", (DWORD)0);
    if (m_stConfig.wEjectMode != 0 &&
        m_stConfig.wEjectMode != 1)
    {
        m_stConfig.wEjectMode = 0;
    }

    // -----[RETAIN_CONFIG]下参数------------
    // 是否支持回收功能(0不支持/1支持,缺省0)
    m_stConfig.wRetainSupp = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG", "RetainSupp", (DWORD)0);
    if (m_stConfig.wRetainSupp != 0 &&
        m_stConfig.wRetainSupp != 1)
    {
        m_stConfig.wRetainSupp = 0;
    }

    // 吞卡计数
    m_stConfig.wRetainCardCount = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG", "RetainCardCount", (DWORD)0);

    // 回收将满报警阀值(缺省15)
    m_stConfig.wRetainThreshold = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG", "RetainThreshold", (DWORD)15);

    // 回收满阀值(缺省20)
    m_stConfig.wRetainFull = (WORD)m_cXfsReg.GetValue("RETAIN_CONFIG", "RetainFull", (DWORD)20);


    // -----[READRAWDATA_CONFIG]下参数------------
    // Readrawdata命令执行完成后是否自动退卡(0不支持/1支持,缺省0)
    m_stConfig.wReadEndRunEject = (WORD)m_cXfsReg.GetValue("READRAWDATA_CONFIG", "ReadEndRunEject", (DWORD)0);
    if (m_stConfig.wReadEndRunEject != 0 &&
        m_stConfig.wReadEndRunEject != 1)
    {
        m_stConfig.wReadEndRunEject = 0;
    }

    // 入参模式(0/1, 缺省0)
    m_stConfig.wReadRawDataInParamMode = (WORD)m_cXfsReg.GetValue("READRAWDATA_CONFIG", "InParamMode", (DWORD)0);
    if (m_stConfig.wReadRawDataInParamMode != 0 &&
        m_stConfig.wReadRawDataInParamMode != 1)
    {
        m_stConfig.wReadRawDataInParamMode = 0;
    }

    m_stConfig.wDebugMode = 1 == (WORD)m_cXfsReg.GetValue("READRAWDATA_CONFIG", "DebugMode", (DWORD)0);

    // -----[IMAGE_CONFIG]下参数------------
    // 证件头像保存路径
    strcpy(m_stConfig.stImageParam.szHeadImgSavePath, m_cXfsReg.GetValue("IMAGE_CONFIG", "HeadImageSavePath", HEADIMAGE_SAVE_DEF));
    if (strlen(m_stConfig.stImageParam.szHeadImgSavePath) < 1)
    {
        memset(m_stConfig.stImageParam.szHeadImgSavePath, 0x00, 256);
        sprintf(m_stConfig.stImageParam.szHeadImgSavePath,
                "%s/image", getenv("HOME"));
    }

    if (FileAccess::create_directory_by_path(m_stConfig.stImageParam.szHeadImgSavePath, FALSE) != TRUE)
    {
        Log(ThisModule, -1, "FileAccess::create_directory_by_path(FALSE) 创建目录[%s]失败. Not Return Error. ",
            m_stConfig.stImageParam.szHeadImgSavePath);
    }

    // 证件扫描保存路
    strcpy(m_stConfig.stImageParam.szScanImgSavePath, m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageSavePath", SCANIMAGE_SAVE_DEF));
    if (strlen(m_stConfig.stImageParam.szScanImgSavePath) < 1)
    {
        memset(m_stConfig.stImageParam.szScanImgSavePath, 0x00, 256);
        sprintf(m_stConfig.stImageParam.szScanImgSavePath,
                "%s/image", getenv("HOME"));
    }
    if (FileAccess::create_directory_by_path(m_stConfig.stImageParam.szScanImgSavePath, FALSE) != TRUE)
    {
        Log(ThisModule, -1, "FileAccess::create_directory_by_path(FALSE) 创建目录[%s]失败. Not Return Error. ",
            m_stConfig.stImageParam.szScanImgSavePath);
    }

    // 保存头像名(包含扩展名,不包含路径,空为不指定,用缺省值)
    strcpy(m_stConfig.stImageParam.szHeadImgName, m_cXfsReg.GetValue("IMAGE_CONFIG", "HeadImageSaveName", ""));

    // 证件扫描正面图保存名(包含扩展名,不包含路径,空为不指定,用缺省值)
    strcpy(m_stConfig.stImageParam.szScanImgFrontName, m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageFrontSaveName", ""));

    // 证件扫描背面图保存名(包含扩展名,不包含路径,空为不指定,用缺省值)
    strcpy(m_stConfig.stImageParam.szScanImgBackName, m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageBackSaveName", ""));

    // 证件扫描图片保存格式(0BMP/1JPG,缺省1)
    m_stConfig.stImageParam.wScanImgSaveType = (WORD)m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageSaveType", 1);

    // 证件扫描图片保存缩放放比例(0.1~3.0,缺省2.0)
    memset(szTmp, 0x00, sizeof(szTmp));
    strcpy(szTmp, m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageSavaZoomScale", "2.0"));
    m_stConfig.stImageParam.fScanImgSaveZoomSc = QString(szTmp).toFloat();

    // 证件扫描图像是否以人像信息面为正面(0否/1是,缺省1)
    m_stConfig.stImageParam.wScanImageFrontIsInfor =
            (WORD)m_cXfsReg.GetValue("IMAGE_CONFIG", "ScanImageFrontIsInfor", 1);

    // 银行编号
    /* // 40-00-00-00(FT#0010) DELETE END
    m_stConfig.stImageParam.wBankNo = (WORD)m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (INT)0);
    if (m_stConfig.stImageParam.wBankNo < 0 && m_stConfig.stImageParam.wBankNo > 1)
    {
        m_stConfig.stImageParam.wBankNo = 0;
    }*/ // 40-00-00-00(FT#0010) DELETE END
    m_stConfig.wBankNo = (WORD)m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (DWORD)0);   // 40-00-00-00(FT#0010)
    if (m_stConfig.wBankNo < 0 && m_stConfig.wBankNo > 1)                       // 40-00-00-00(FT#0010)
    {                                                                                       // 40-00-00-00(FT#0010)
        m_stConfig.wBankNo = 0;                                                       // 40-00-00-00(FT#0010)
    }                                                                                       // 40-00-00-00(FT#0010)
    m_stConfig.stImageParam.wBankNo = m_stConfig.wBankNo;                 // 40-00-00-00(FT#0010)

    return WFS_SUCCESS;
}

// 状态初始化
void CXFS_IDX::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stStatus.fwDevice    = WFS_IDC_DEVNODEVICE;
    m_stStatus.fwMedia     = WFS_IDC_MEDIAUNKNOWN;
    m_stStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    m_stStatus.fwSecurity  = WFS_IDC_SECNOTSUPP;
    m_stStatus.usCards     = 0;
    m_stStatus.fwChipPower = WFS_IDC_CHIPNOTSUPP;
    m_stStatus.lpszExtra   = nullptr;

    m_stStatusOLD.fwDevice    = m_stStatus.fwDevice;
    m_stStatusOLD.fwMedia     = m_stStatus.fwMedia;
    m_stStatusOLD.fwRetainBin = m_stStatus.fwRetainBin;
    m_stStatusOLD.fwSecurity  = m_stStatus.fwSecurity;
    m_stStatusOLD.usCards     = m_stStatus.usCards;
    m_stStatusOLD.fwChipPower = m_stStatus.fwChipPower;
    m_stStatusOLD.lpszExtra   = m_stStatus.lpszExtra;
}

// 能力值初始化
void CXFS_IDX::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_Caps.wClass = WFS_SERVICE_CLASS_IDC;
    m_Caps.fwType = WFS_IDC_TYPESWIPE;
    m_Caps.bCompound = FALSE;
    m_Caps.fwReadTracks = WFS_IDC_NOTSUPP;
    m_Caps.fwWriteTracks = WFS_IDC_NOTSUPP;
    m_Caps.usCards = (m_stConfig.wRetainSupp == 1 ? m_stConfig.wRetainFull : 0);  // 指定回收盒可以容纳的最大卡张数(如果不可用则为0)
    m_Caps.fwChipProtocols = WFS_IDC_NOTSUPP;
    m_Caps.fwSecType = WFS_IDC_SECNOTSUPP;
    m_Caps.fwPowerOnOption = WFS_IDC_NOACTION;
    m_Caps.fwPowerOffOption = WFS_IDC_NOACTION;
    m_Caps.bFluxSensorProgrammable        = FALSE;
    m_Caps.bReadWriteAccessFollowingEject = FALSE;
    m_Caps.fwWriteMode = WFS_IDC_NOTSUPP;
    m_Caps.fwChipPower = WFS_IDC_NOTSUPP;
    m_Caps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
}

// 更新扩展数据
void CXFS_IDX::UpdateExtra()
{
    CHAR szFWVer[64] = { 0x00 };
    CHAR szSWVer[64] = { 0x00 };

    // 组织状态扩展数据
    m_cExtra.Clear();
    m_cExtra.AddExtra("VRTCount", "2");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", (char*)byDevVRTU);

    // 取固件版本写入扩展数据
    m_pDev->GetVersion(GET_VER_FW, szFWVer, 64);
    if (strlen(szFWVer) > 0)
    {
        m_cExtra.AddExtra("VRTCount", "3");
        m_cExtra.AddExtra("VRTDetail[02]_FW", szFWVer);
    }

    // 取软件版本写入扩展数据
    m_pDev->GetVersion(GET_VER_LIB, szSWVer, sizeof(szSWVer) - 1);
    if (strlen(szSWVer) > 0)
    {
        if (strlen(szFWVer) > 0)
        {
            m_cExtra.AddExtra("VRTCount", "4");
            m_cExtra.AddExtra("VRTDetail[03]_SOFT", szSWVer);
        } else
        {
            m_cExtra.AddExtra("VRTCount", "3");
            m_cExtra.AddExtra("VRTDetail[02]_SOFT", szSWVer);
        }
    }
}

// 读卡子处理1
HRESULT CXFS_IDX::InnerReadRawData(DWORD dwReadOption, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();


    INT nRet = IDC_SUCCESS;

    // 检查内部是否有卡, 无卡则执行进卡
    if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT ||    // 内部无卡
        m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)        // 卡在出口
    {
        // 执行进卡
        nRet = m_pDev->MediaControl(MEDIA_ACCEPT_IC, dwTimeOut);    // 进卡(使用IC卡进卡标记)
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__, "进卡: ->MeidaControl(%d, %d) Fail, ErrCode: %d, Return: %d",
                MEDIA_ACCEPT_IC, dwTimeOut, nRet, ConvertDevErrCode2WFS(nRet));

            // 检查失败原因
            if (nRet == ERR_IDC_INSERT_TIMEOUT ||   // 放卡超时
                nRet == ERR_IDC_USER_CANCEL)        // 用户取消
            {
                UpdateStatus();     // 更新当前状态
                if (m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT) // 内部有卡, 执行退卡
                {
                    m_pDev->MediaControl(MEDIA_EJECT);
                }
            }

            return ConvertDevErrCode2WFS(nRet);
        } else
        {
            Log(ThisModule, __LINE__, "进卡: ->MeidaControl(%d, %d) Succ");
            m_stStatus.fwMedia = WFS_IDC_MEDIAPRESENT;
            FireCardInserted();
            m_enWaitTaken = WTF_TAKEN;
        }
    }

    // 进卡成功, 执行读数据
    // 读芯片数据(激活IC卡)
    STMEDIARW stMediaRead;
    stMediaRead.dwRWType = ConvertWFS2MediaRW(dwReadOption);
    stMediaRead.dwTimeOut = dwTimeOut;
    stMediaRead.stData[0].wSize = sizeof(stMediaRead.stData[0].szData);
    m_pDev->SetData(SET_DEBUG_MODE, &m_stConfig.wDebugMode);
    nRet = m_pDev->MediaReadWrite(MEDIA_READ, stMediaRead);
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡: ->MediaReadWrite(%d) Fail, ErrCode: %d, Return: %d",
            MEDIA_READ, nRet, ConvertDevErrCode2WFS(nRet));
        return ConvertDevErrCode2WFS(nRet);
    }

    // 组织应答数据
    if ((dwReadOption & WFS_IDC_TRACK1) == WFS_IDC_TRACK1)
    {
        SetTrackInfo(WFS_IDC_TRACK1, ConvertMediaRWResult2WFS(stMediaRead.stData[0].wResult),
                     stMediaRead.stData[0].wSize, (BYTE*)stMediaRead.stData[0].szData);
    }
    if ((dwReadOption & WFS_IDC_CHIP) == WFS_IDC_CHIP)
    {
        SetTrackInfo(WFS_IDC_CHIP, ConvertMediaRWResult2WFS(stMediaRead.stData[0].wResult),
                     stMediaRead.stData[0].wSize, (BYTE*)stMediaRead.stData[0].szData);
    }

    if ((dwReadOption & WFS_IDC_TRACK2) == WFS_IDC_TRACK2)
    {
        SetTrackInfo(WFS_IDC_TRACK2, ConvertMediaRWResult2WFS(stMediaRead.stData[1].wResult),
                     stMediaRead.stData[1].wSize, (BYTE*)stMediaRead.stData[1].szData);
    }
    if ((dwReadOption & WFS_IDC_FRONTIMAGE) == WFS_IDC_FRONTIMAGE)
    {
        SetTrackInfo(WFS_IDC_FRONTIMAGE, ConvertMediaRWResult2WFS(stMediaRead.stData[1].wResult),
                     stMediaRead.stData[1].wSize, (BYTE*)stMediaRead.stData[1].szData);
    }

    if ((dwReadOption & WFS_IDC_TRACK3) == WFS_IDC_TRACK3)
    {
        SetTrackInfo(WFS_IDC_TRACK3, ConvertMediaRWResult2WFS(stMediaRead.stData[2].wResult),
                     stMediaRead.stData[2].wSize, (BYTE*)stMediaRead.stData[2].szData);
    }
    if ((dwReadOption & WFS_IDC_BACKIMAGE) == WFS_IDC_BACKIMAGE)
    {
        SetTrackInfo(WFS_IDC_BACKIMAGE, ConvertMediaRWResult2WFS(stMediaRead.stData[2].wResult),
                     stMediaRead.stData[2].wSize, (BYTE*)stMediaRead.stData[2].szData);
    }

    return WFS_SUCCESS;
}

// 复位子处理
HRESULT CXFS_IDX::InnerReset(WORD wAction, BOOL bIsHaveCard)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    MEDIA_ACTION enMediaAct;
    if (wAction == WFS_IDC_RETAIN)
    {
        enMediaAct = MEDIA_RETRACT;     // 吞卡
    } else
    if (wAction == WFS_IDC_NOACTION)
    {
        enMediaAct = MEDIA_NOTACTION;   // 无动作
    } else
    if (wAction == WFS_IDC_EJECT)
    {
        enMediaAct = MEDIA_EJECT;       // 退卡
    }

    // 执行复位
    nRet = m_pDev->Reset(enMediaAct, m_stConfig.wEjectMode);
    if (nRet != IDC_SUCCESS)    // 复位失败
    {
        if (nRet == ERR_IDC_MED_JAMMED) // JAM
        {
            FireMediaDetected(WFS_IDC_CARDJAMMED);
        }

        Log(ThisModule, __LINE__, "复位: ->Reset(%d, %d) Fail, ErrCode: %s, Return: %d",
            enMediaAct, m_stConfig.wEjectMode, ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        if (wAction == WFS_IDC_EJECT && bIsHaveCard == TRUE)   // 入参指定退卡
        {
            UpdateStatus();     // 更新当前状态
            if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING ||  // 卡在出口
                (bIsHaveCard == TRUE && m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT) ||
                (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT  // 内部无卡&&完全弹出
                 && m_stConfig.wEjectMode == 1))
            {
                m_enWaitTaken = WTF_TAKEN;    // 设置Taken标记
            }
        } else
        if (wAction == WFS_IDC_RETAIN && bIsHaveCard == TRUE)   // 入参指定吞卡
        {
            UpdateStatus();     // 更新当前状态
            if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)      // 内部无卡
            {
                FireMediaRetained();                                // 上报吞卡事件
                m_stConfig.wRetainCardCount ++;
                SetRetainCardCount(m_stConfig.wRetainCardCount);    // 吞卡计数+1
            }
        }
    }

    return WFS_SUCCESS;
}

// 退卡子处理
HRESULT CXFS_IDX::InnerEject()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    if (m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT)    // 卡在内部
    {
        nRet = m_pDev->MediaControl(MEDIA_EJECT, m_stConfig.wEjectMode);
        if (nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__, "退卡: ->MediaControl(%d, %d) Fail, ErrCode: %s, Return: %d",
                MEDIA_EJECT, m_stConfig.wEjectMode, ConvertDevErrCodeToStr(nRet),
                ConvertDevErrCode2WFS(nRet));
            return ConvertDevErrCode2WFS(nRet);
        } else
        {
            usleep(500 * 1000); // Eject命令下发后等待500毫秒,确保取到有效的卡状态
            UpdateStatus();     // 更新当前状态
            if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING ||  // 卡在出口
                (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT  // 内部无卡&&完全弹出
                 && m_stConfig.wEjectMode == 1))
            {
                m_enWaitTaken = WTF_TAKEN;
            }
        }
    }

    return WFS_SUCCESS;
}

// 吞卡子处理
HRESULT CXFS_IDX::InnerRetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IDC_SUCCESS;

    nRet = m_pDev->MediaControl(MEDIA_RETRACT);
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__, "吞卡: ->MediaControl(%d) Fail, ErrCode: %s, Return: %d",
            MEDIA_EJECT, ConvertDevErrCodeToStr(nRet),
            ConvertDevErrCode2WFS(nRet));
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        usleep(500 * 1000); // 吞卡命令下发后等待500毫秒,确保取到有效的卡状态
        UpdateStatus();     // 更新当前状态
        if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)      // 内部无卡
        {
            FireMediaRetained();                                // 上报吞卡事件
            m_stConfig.wRetainCardCount ++;
            SetRetainCardCount(m_stConfig.wRetainCardCount);    // 吞卡计数+1
        }
    }

    return WFS_SUCCESS;
}

// 状态更新子处理
long CXFS_IDX::UpdateStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题

    INT     nRet = IDC_SUCCESS;
    DWORD   dwHWAct = WFS_ERR_ACT_NOACTION;
    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log
    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFireTaken          = FALSE;
    BOOL    bNeedFireMediaInserted  = FALSE;
    BOOL    bNeedFireRetainThreshold= FALSE;

    //----------------------设备状态处理----------------------
    STDEVIDCSTATUS stDevStatus;
    nRet = m_pDev->GetStatus(stDevStatus);
    if (nRet != IDC_SUCCESS)    // 返回值处理
    {
        switch (nRet)
        {
            case ERR_IDC_PARAM_ERR:
            case ERR_IDC_READ_ERR:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_NOACTION;
                break;
            case ERR_IDC_COMM_ERR:
            case ERR_IDC_DEV_OFFLINE:
            case ERR_IDC_DEV_NOTFOUND:
            case ERR_IDC_READ_TIMEOUT:
            case ERR_IDC_DEV_NOTOPEN:
                m_stStatus.fwDevice = WFS_IDC_DEVOFFLINE;
                dwHWAct = WFS_ERR_ACT_NOACTION;
                break;
            case ERR_IDC_MED_STAT_ERR:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
            case ERR_IDC_MED_JAMMED:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
            case ERR_IDC_DEV_HWERR:
            case ERR_IDC_OTHER:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_HWCLEAR;
                break;
            case ERR_IDC_RETAIN_FULL:
                m_stStatus.fwDevice = WFS_IDC_DEVONLINE;
                break;
            case ERR_IDC_USER_ERR:
                m_stStatus.fwDevice = WFS_IDC_DEVUSERERROR;
                break;
            default:
                m_stStatus.fwDevice = WFS_IDC_DEVHWERROR;
                dwHWAct = WFS_ERR_ACT_RESET;
                break;
        }
    } else
    {
        m_stStatus.fwDevice = ConvertDeviceStatus2WFS(stDevStatus.wDevice);
        m_stStatus.fwMedia = ConvertMediaStatus2WFS(stDevStatus.wMedia);
    }

    //----------------------状态检查----------------------
    // Device状态变化检查
    if (m_stStatus.fwDevice != m_stStatusOLD.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        if (m_stStatus.fwDevice == WFS_IDC_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }

    //----------------------Media检查----------------------
    if (m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT)     // 卡在内部
    {
        m_enWaitTaken = WTF_NONE;
    } else
    /*if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)    // 卡在出口
    {
        m_enWaitTaken = WTF_TAKEN;
    } else*/
    if (m_stStatus.fwMedia == WFS_IDC_MEDIANOTPRESENT)  // 内部无卡
    {
        if (m_enWaitTaken == WTF_TAKEN)
        {
            bNeedFireTaken = TRUE;
        } else
        {
            m_enWaitTaken = WTF_NONE;
        }
    }

    //----------------------回收盒检查----------------------
    if (m_stConfig.wRetainSupp == 1)    // 支持回收
    {
        if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainFull)      // 回收已满
        {
            if (m_wRetainThresholdFire != WFS_IDC_RETAINBINFULL)
            {
                m_stStatus.fwRetainBin = WFS_IDC_RETAINBINFULL;
                m_wRetainThresholdFire = WFS_IDC_RETAINBINFULL;
                bNeedFireRetainThreshold = TRUE;
            }
        } else
        if (m_stConfig.wRetainCardCount >= m_stConfig.wRetainThreshold) // 回收将满
        {
            if (m_wRetainThresholdFire != WFS_IDC_RETAINBINHIGH)
            {
                m_stStatus.fwRetainBin = WFS_IDC_RETAINBINHIGH;
                m_wRetainThresholdFire = WFS_IDC_RETAINBINHIGH;
                bNeedFireRetainThreshold = TRUE;
            }
        } else                                                          // 回收盒空
        {
            if (m_wRetainThresholdFire != WFS_IDC_RETAINBINOK)
            {
                m_stStatus.fwRetainBin = WFS_IDC_RETAINBINOK;
                m_wRetainThresholdFire = WFS_IDC_RETAINBINOK;
                bNeedFireRetainThreshold = TRUE;
            }
        }
    } else
    {
        m_stStatus.fwRetainBin = WFS_IDC_RETAINNOTSUPP;
    }

    //----------------------事件上报----------------------
    // 硬件故障事件
    if (bNeedFireHWError)
    {
        FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%d|",
                    WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
    }

    // 设备状态变化事件
    if (bNeedFireStatusChanged)
    {
        FireStatusChanged(m_stStatus.fwDevice);
        sprintf(szFireBuffer + strlen(szFireBuffer), "StatusChange:%d|",  m_stStatus.fwDevice);
    }

    // Taken事件
    if (bNeedFireTaken)
    {
        FireMediaRemoved();
        sprintf(szFireBuffer + strlen(szFireBuffer), "Taken|",  m_stStatus.fwDevice);
        m_enWaitTaken = WTF_NONE;
    }

    // RetainThreshold事件
    if (bNeedFireRetainThreshold)
    {
        FireRetainBinThreshold(m_wRetainThresholdFire);
        sprintf(szFireBuffer + strlen(szFireBuffer), "RetainThreshold:%d|", m_wRetainThresholdFire);
    }

    // 比较两次状态记录LOG
    if (m_stStatus.Diff(m_stStatusOLD) == true)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|Media:%d->%d%s|Paper[0]:%d->%d%s|"
                            "Ink:%d->%d%s|Toner:%d->%d%s|Lamp:%d->%d%s|; 事件上报记录: %s.",
            m_stStatusOLD.fwDevice, m_stStatus.fwDevice, (m_stStatusOLD.fwDevice != m_stStatus.fwDevice ? " *" : ""),
            m_stStatusOLD.fwMedia, m_stStatus.fwMedia, (m_stStatusOLD.fwMedia != m_stStatus.fwMedia ? " *" : ""),
            m_stStatusOLD.fwRetainBin, m_stStatus.fwRetainBin, (m_stStatusOLD.fwRetainBin != m_stStatus.fwRetainBin ? " *" : ""),
            m_stStatusOLD.fwSecurity, m_stStatus.fwSecurity, (m_stStatusOLD.fwSecurity != m_stStatus.fwSecurity ? " *" : ""),
            m_stStatusOLD.usCards, m_stStatus.usCards, (m_stStatusOLD.usCards != m_stStatus.usCards ? " *" : ""),
            m_stStatusOLD.fwChipPower, m_stStatus.fwChipPower, (m_stStatusOLD.fwChipPower != m_stStatus.fwChipPower ? " *" : ""),
            szFireBuffer);
        m_stStatusOLD.Copy(m_stStatus);
    }

    return WFS_SUCCESS;
}

// ReadRawData回参处理
void CXFS_IDX::SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData)
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

// 设置回收箱计数并记录INI
INT CXFS_IDX::SetRetainCardCount(WORD wCount)
{
    THISMODULE(__FUNCTION__);

    // 设置INI中回收计数
    BOOL bRet = m_cXfsReg.SetValue("RETAIN_CONFIG", "RetainCardCount", std::to_string(wCount).c_str());
    if (bRet != TRUE)
    {
        Log(ThisModule, __LINE__, "设置INI中回收计数为[%d]失败", wCount);
        return 1;
    }
    Log(ThisModule, __LINE__, "设置INI中回收计数为[%d]完成", wCount);
    return 0;
}
