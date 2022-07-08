/***************************************************************
* 文件名称：XFS_CRM.cpp
* 文件描述：退卡模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_IDC.h"

//-----------------------------基本接口----------------------------------
// OpenCRM命令入口
HRESULT CXFS_IDC::StartOpenCRM()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    //InitConfigCRM();       // 读INI

    if (m_stCRMConfig.bCRMDeviceSup != TRUE)     // 不支持启用退卡模块
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定不支持的启用CRM设备, Return: SUCCESS.",
            WFS_SUCCESS);
        SetErrorDetail(1, (LPSTR)EC_CRM_XFS_ININotSupp);
        return WFS_SUCCESS;
    }

    // 检查INI设置设备类型
    if (m_stCRMConfig.wCRMDeviceType != DEV_CRM_CRT730B)
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定了不支持的CRM设备类型[%d], Return :%d.",
            m_stConfig.wDeviceType, WFS_ERR_DEV_NOT_READY);
        SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotSupp);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 加载DevXXX动态库
    if (LoadCRMDevDll(ThisModule) != TRUE)
    {
        return WFS_ERR_INTERNAL_ERROR;
    }

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    hRet = InnerOpenCRM();
    if (m_stConfig.wOpenFailRet == 1)
    {
        hRet = WFS_SUCCESS;
    }

    return hRet;
}

// CloseCRM命令入口
HRESULT CXFS_IDC::EndCloseCRM()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (m_pCRMDev != nullptr)
    {
        m_pCRMDev->Close();
        m_pCRMDev = nullptr;
    }

    return WFS_SUCCESS;
}

// Open退卡设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_IDC::InnerOpenCRM(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    // //AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    CHAR szBuff[32] = { 0x00 };

    if (m_stCRMConfig.bCRMDeviceSup == TRUE)     // 支持启用退卡模块
    {
        // Open前下传初始参数(非断线重连)
        //if (bReConn == FALSE)
        //{
        //    ;
        //}

        // Open退卡模块
        nCRMRet = m_pCRMDev->Open(m_stCRMConfig.szCRMDeviceConList);
        if (nCRMRet != CRM_SUCCESS)
        {
            if (bReConn == FALSE)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 打开设备: ->Open(%s) Fail, ErrCode: %s, Return: %d",
                    m_stCRMConfig.szCRMDeviceConList, CRM_ConvertDevErrCodeToStr(nCRMRet),
                    CRM_ConvertDevErrCode2WFS(nCRMRet));
            }
            SetErrorDetail(1);
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }

        // 退卡模块初始化
        CRMInitAction emCRMAction;
        if (m_stCRMConfig.wDeviceInitAction == 0)
        {
            emCRMAction = CRMINIT_HOMING;   // 正常归位
            sprintf(szBuff, "正常归位");
        } else
        if (m_stCRMConfig.wDeviceInitAction == 1)
        {
            emCRMAction = CRMINIT_EJECT;    // 强制退卡
            sprintf(szBuff, "强制退卡");
        } else
        if (m_stCRMConfig.wDeviceInitAction == 2)
        {
            emCRMAction = CRMINIT_STORAGE;  // 强制暂存
            sprintf(szBuff, "强制暂存");
        } else
        if (m_stCRMConfig.wDeviceInitAction == 3)
        {
            emCRMAction = CRMINIT_NOACTION; // 无动作
            sprintf(szBuff, "无动作");
        } else
        {
            emCRMAction = CRMINIT_NOACTION; // 无动作
            sprintf(szBuff, "无动作");
        }
        nCRMRet = m_pCRMDev->Init(emCRMAction);
        if (nCRMRet != CRM_SUCCESS)
        {
            if (bReConn == FALSE)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 打开设备: 初始化(%s): ->Init(%d) Fail, ErrCode: %s, Return: %d",
                    szBuff, emCRMAction, CRM_ConvertDevErrCodeToStr(nCRMRet),
                    CRM_ConvertDevErrCode2WFS(nCRMRet));
            }
            SetErrorDetail(1);
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }

        // 更新扩展状态_版本参数
        m_cStatExtra.AddExtra("CRM_VRTCount", "1");
        m_cStatExtra.AddExtra("CRM_VRT[00]-DevCRM", (LPSTR)byDevCRMVRTU);


        CHAR szCRMSoftVer[MAX_PATH] = { 0x00 };     // CRM设备软件版本
        m_pCRMDev->GetVersion(3, szCRMSoftVer, sizeof(szCRMSoftVer) - 1);
        if (strlen(szCRMSoftVer) > 0)
        {
            m_cStatExtra.AddExtra("CRM_VRTCount", "2");
            m_cStatExtra.AddExtra("CRM_VRT[01]-Soft", szCRMSoftVer);
        }



        CHAR szCRMSerialNumber[MAX_PATH] = { 0x00 };// CRM设备序列号
        m_pCRMDev->GetVersion(4, szCRMSerialNumber, sizeof(szCRMSerialNumber) - 1);
        if (strlen(szCRMSerialNumber) > 0)
        {
            if (strlen(szCRMSoftVer) > 0)
            {
                m_cStatExtra.AddExtra("CRM_VRTCount", "3");
                m_cStatExtra.AddExtra("CRM_VRT[02]-SerialNo", szCRMSerialNumber);
            } else
            {
                m_cStatExtra.AddExtra("CRM_VRTCount", "2");
                m_cStatExtra.AddExtra("CRM_VRT[01]-SerialNo", szCRMSerialNumber);
            }
        }

        if (bReConn == TRUE)
        {
            Log(ThisModule, __LINE__, "CRM: 设备断线重连成功, Extra=%s.",
                m_cStatExtra.GetExtraInfo().c_str());
        } else
        {
            Log(ThisModule, __LINE__, "CRM: 打开C设备连接成功, Extra=%s.",
                m_cStatExtra.GetExtraInfo().c_str());
        }

        m_bCRMIsOnLine = TRUE;
        m_clErrorDet.SetErrCodeInit();
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-加载DevCRM动态库
BOOL CXFS_IDC::LoadCRMDevDll(LPCSTR ThisModule)
{
    if (m_pCRMDev == nullptr)
    {
        if (m_pCRMDev.Load(m_stCRMConfig.szDevDllNameCRM,
                           "CreateIDevCRM",
                           CRM_DEVTYPE2STR(m_stCRMConfig.wCRMDeviceType)) != 0)
        {
            Log(ThisModule, __LINE__,
                "CRM: 加载库失败: DriverDllName=%s, CRMDEVTYPE=%d. ReturnCode:%s",
                m_stCRMConfig.szDevDllNameCRM, m_stCRMConfig.wCRMDeviceType,
                m_pCRMDev.LastError().toUtf8().constData());
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevCRMLoadFail);
            return FALSE;
        }
    }

    return (m_pCRMDev != nullptr);
}

// 加载INI设置(CRM)
INT CXFS_IDC::InitConfigCRM()
{
    THISMODULE(__FUNCTION__);
    ////AutoLogFuncBeginEnd();

    CHAR szBuffer[1024] = { 0x00 };

    //--------------------退卡模块INI配置参数获取--------------------
    // 是否支持启用退卡模块: 0不支持/1支持,缺省0
    m_stCRMConfig.bCRMDeviceSup = ((WORD)m_cXfsReg.GetValue("CRM_DEVICE_CFG", "CRMDeviceSup", (DWORD)0) == 1);

    if (m_stCRMConfig.bCRMDeviceSup == TRUE)
    {
        // 底层设备控制动态库名(退卡模块)
        strcpy(m_stCRMConfig.szDevDllNameCRM, m_cXfsReg.GetValue("CRMDriverDllName", ""));

        // 退卡模块设备类型(缺省0)
        m_stCRMConfig.wCRMDeviceType = (WORD)m_cXfsReg.GetValue("CRM_DEVICE_CFG", "DeviceType", (DWORD)0);

        // CMStatus/CMGetCardInfo命令数据占位符,缺省F
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue("CRM_DEVICE_CFG", "PlaceHolder", "F"));
        if (strlen(szBuffer) > 0)
        {
            memcpy(m_stCRMConfig.szPlaceHolder, szBuffer, 1);
        } else
        {
            memcpy(m_stCRMConfig.szPlaceHolder, "F", 1);
        }

        // 是否支持CMEmptyCard入参为0时回收所有卡(0:不支持, 1:支持, 缺省0)
        m_stCRMConfig.wEmptyAllCard0 = m_cXfsReg.GetValue("CRM_DEVICE_CFG", "CMEmptyAllCard0", (DWORD)0);

        // 卡槽有卡状态标记(仅限2位),缺省01
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue("CRM_DEVICE_CFG", "SlotHaveCard", "01"));
        if (strlen(szBuffer) > 1)
        {
            memcpy(m_stCRMConfig.szSlotHaveCard, szBuffer, 2);
        } else
        {
            memcpy(m_stCRMConfig.szSlotHaveCard, "01", 2);
        }

        // 卡槽无卡状态标记(仅限2位),缺省00
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue("CRM_DEVICE_CFG", "SlotNoHaveCard", "00"));
        if (strlen(szBuffer) > 1)
        {
            memcpy(m_stCRMConfig.szSlotNoHaveCard, szBuffer, 2);
        } else
        {
            memcpy(m_stCRMConfig.szSlotNoHaveCard, "00", 2);
        }

        // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
        m_stCRMConfig.wOpenFailRet = m_cXfsReg.GetValue("CRM_DEVICE_CFG", "OpenFailRet", (DWORD)0);
        if (m_stCRMConfig.wOpenFailRet != 0 && m_stConfig.wOpenFailRet != 1)
        {
            m_stCRMConfig.wOpenFailRet = 0;
        }

        // 退卡模式(CMEject命令), 0:指定卡号退卡, 1:指定卡槽号退卡, 缺省0
        m_stCRMConfig.wCMEjectMode = m_cXfsReg.GetValue("CRM_DEVICE_CFG", "CMEjectMode", (DWORD)0);
        if (m_stCRMConfig.wCMEjectMode != 0 && m_stCRMConfig.wCMEjectMode != 1)
        {
            m_stCRMConfig.wCMEjectMode = 0;
        }

        CHAR szKeyName[32];
        sprintf(szKeyName, "CRM_DEVICE_SET_%d", m_stCRMConfig.wCRMDeviceType);

        if (m_stCRMConfig.wCRMDeviceType == DEV_CRM_CRT730B)
        {
            // 1. 设备连接串,缺省(USB:23D8,0730)
            strcpy(m_stCRMConfig.szCRMDeviceConList, m_cXfsReg.GetValue(szKeyName, "DeviceConList", "USB:23D8,0730"));

            // 2. 设备初始化动作(0正常归位;1强行退卡;2强行暂存;3无动作,缺省0)
            m_stCRMConfig.wDeviceInitAction = (WORD)m_cXfsReg.GetValue(szKeyName, "DeviceInitAction", (DWORD)0);
            if (m_stCRMConfig.wDeviceInitAction < 0 || m_stCRMConfig.wDeviceInitAction > 3)
            {
                m_stCRMConfig.wDeviceInitAction = 0;
            }

            // 3. 退卡后卡位置(0读卡器内部;1读卡器前入口,缺省0）
            m_stCRMConfig.wEjectCardPos = (WORD)m_cXfsReg.GetValue(szKeyName, "EjectCardPos", (DWORD)0);
            if (m_stCRMConfig.wEjectCardPos < 0 || m_stCRMConfig.wEjectCardPos > 1)
            {
                m_stCRMConfig.wEjectCardPos = 0;
            }

            // 4. 读卡器前入口有卡是否支持收入卡槽（0不支持;1支持,缺省0)
            m_stCRMConfig.wEnterCardRetainSup = (WORD)m_cXfsReg.GetValue(szKeyName, "EnterCardRetainSup", (DWORD)0);
            if (m_stCRMConfig.wEnterCardRetainSup < 0 || m_stCRMConfig.wEnterCardRetainSup > 1)
            {
                m_stCRMConfig.wEnterCardRetainSup = 0;
            }
        }
    }

    PrintIniCRM();  // INI配置输出

    return WFS_SUCCESS;
}

// 变量初始化
INT CXFS_IDC::InitCRM()
{
    m_bCRMIsOnLine = FALSE;                 // 是否连线
    m_bIsSetCardData = FALSE;               // RetainCard前需要执行SetCardData命令
    memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));         // 吞卡卡卡号
    m_CardReaderEjectFlag = 0;              // 读卡器退卡执行标记
    return WFS_SUCCESS;
}

// INI配置输出
INT CXFS_IDC::PrintIniCRM()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    QString qsIniPrt = "";
    CHAR szBuff[256] = { 0x00 };

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 是否支持启用退卡模块(0不支持;1支持): CRM_DEVICE_CFG->CRMDeviceSup = %d",
            m_stCRMConfig.bCRMDeviceSup);
    qsIniPrt.append(szBuff);

    if (m_stCRMConfig.bCRMDeviceSup != TRUE)
    {
        return WFS_SUCCESS;
    }

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 退卡模块动态库名: default->CRMDriverDllName = %s",
            m_stCRMConfig.szDevDllNameCRM);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 退卡模块设备类型: CRM_DEVICE_CFG->CRMDeviceType = %d",
            m_stCRMConfig.wCRMDeviceType);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 退卡模块连接字符串: CRM_DEVICE_%d->DeviceConList = %d",
            m_stCRMConfig.wCRMDeviceType, m_stCRMConfig.szCRMDeviceConList);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 设备初始化动作(0正常归位;1强行退卡;2强行暂存;3无动作): CRM_DEVICE_%d->DeviceInitAction = %d",
            m_stCRMConfig.wCRMDeviceType, m_stCRMConfig.wDeviceInitAction);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 退卡后卡位置(0读卡器内部;1读卡器前入口）: CRM_DEVICE_%d->EjectCardPos = %d",
            m_stCRMConfig.wEjectCardPos, m_stCRMConfig.wDeviceInitAction);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 卡在入口是否支持CMRetain(0不支持;1支持): CRM_DEVICE_%d->EnterCardRetainSup = %d",
            m_stCRMConfig.wCRMDeviceType, m_stCRMConfig.wEnterCardRetainSup);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\\nt\t\t\t 是否支持CMEmptyCard入参为0时回收所有卡: CRM_DEVICE_CFG->PlaceHolder = %s",
            m_stCRMConfig.szPlaceHolder);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t CMStatus/CMGetCardInfo命令数据占位符: CRM_DEVICE_CFG->CMEmptyAllCard0 = %d",
            m_stCRMConfig.wEmptyAllCard0);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 卡槽有卡状态标记: CRM_DEVICE_CFG->SlotHaveCard = %s",
            m_stCRMConfig.szSlotHaveCard);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 卡槽无卡状态标记: CRM_DEVICE_CFG->SlotNoHaveCard = %s",
            m_stCRMConfig.szSlotNoHaveCard);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t 退卡模式(CMEject命令)(0:指定卡号退卡,1:指定卡槽号退卡): CRM_DEVICE_CFG->CMEjectMode = %s",
            m_stCRMConfig.wCMEjectMode);
    qsIniPrt.append(szBuff);

    MSET_0(szBuff);
    sprintf(szBuff, "\n\t\t\t\t Open失败时返回值(0原样返回/1返回SUCCESS): CRM_DEVICE_CFG->OpenFailRet = %d",
            m_stCRMConfig.wOpenFailRet);
    qsIniPrt.append(szBuff);

    Log(ThisModule, __LINE__, "CRM INI配置取得如下:%s", qsIniPrt.toStdString().c_str());

    return WFS_SUCCESS;
}

//-----------------------------CEN标准命令接口----------------------------------
// 退卡模块(CRM)-指定卡号退卡
HRESULT CXFS_IDC::CMEjectCard(LPCSTR lpszCardNo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    INT nIDCRet = WFS_SUCCESS;

    //std::thread m_thRunEjectWait;

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定卡号退卡: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d.",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 卡号Check
        if (lpszCardNo == nullptr || strlen(lpszCardNo) < 1 ||
            strlen(lpszCardNo) > 20)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定卡号退卡: 入参卡号[%s] 无效(==NULL||<1). Return: %d.",
                lpszCardNo, WFS_ERR_INVALID_DATA);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_CardNoInv);
            return WFS_ERR_INVALID_DATA;
        }

        // CRM退卡前检查读卡器内是否有卡(读卡器命令)
        UpdateDeviceStatus(); // 更新读卡器当前状态
        if (m_stStatus.fwMedia == WFS_IDC_MEDIALATCHED ||   // 内部有卡且锁定
            m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING ||  // 出口有卡
            m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT)     // 内部有卡
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定卡号退卡: 检查: 读卡器出口/内部有卡, CRM无法退卡, Return: %d.",
                 WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_IDCHaveMed);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 检查卡号是否存在
        nCRMRet = m_pCRMDev->GetData(CARDNO_ISEXIST, (void*)lpszCardNo);
        if (nCRMRet == ERR_CRM_CARD_NOTFOUND)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定卡号退卡: 卡号[%s]: 不存在, ErrCode: %s, Return: %d.",
                lpszCardNo, CRM_ConvertDevErrCodeToStr(nCRMRet),
                CRM_ConvertDevErrCode2WFS(nCRMRet));
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_CardNotFound);
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
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

        // 执行CRM退卡
        nCRMRet = m_pCRMDev->CMEjectCard(lpszCardNo);
        if ((nCRMRet) != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定卡号退卡: ->CMEjectCard(%s) Fail, ErrCode: %s, Return: %d.",
                lpszCardNo, lpszCardNo, CRM_ConvertDevErrCodeToStr(nCRMRet),
                CRM_ConvertDevErrCode2WFS(nCRMRet));
            m_bThreadEjectExit = TRUE;  // CRM ERR,结束线程(循环下发读卡器退卡)
            SetErrorDetail(1);
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
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
                Log(ThisModule, __LINE__, "CRM: 指定卡号退卡: CardNo[%s] CardReader Eject(内部保留): "
                                    "->MoveCardToMMPosition() TimeOut. Return: %d.",
                    lpszCardNo, WFS_ERR_TIMEOUT);
                return WFS_ERR_TIMEOUT;
            } else
            if (m_CardReaderEjectFlag != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__, "CMEject: CardNo[%s] CardReader Eject(内部保留): "
                                    "->MoveCardToMMPosition() Fail. Return: %d.",
                    lpszCardNo, m_CardReaderEjectFlag);
                return m_CardReaderEjectFlag;
            }
        }

        // 退卡成功，结束线程(循环下发读卡器退卡)
        m_bThreadEjectExit = TRUE;

        // INI设置退卡到卡口(读卡器命令)
        // 3. 退卡后卡位置(0读卡器内部;1读卡器前入口,缺省0）
        if (m_stCRMConfig.wEjectCardPos == 1)
        {
            nIDCRet = InnerEject();
            if (nIDCRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 指定卡号退卡: 卡号[%s]退卡到读卡器出口: ->InnerEject() Fail, ErrCode: %s, Return: %d.",
                    lpszCardNo, ConvertDevErrCodeToStr(nIDCRet), nIDCRet);
                SetErrorDetail(1);
                return nIDCRet;
            }
        }

        // 卡CMRetain进入回收时,读卡器回收计数+1; CMEject退卡成功时,读卡器回收计数-1
        UpdateRetainCards(RETCNT_RED_ONE);

        // 移动卡槽回到初始位置
        INT nSlotNo = 0;
        if ((nCRMRet = m_pCRMDev->CMCardSlotMove(0, nSlotNo)) != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定卡号退卡: 移动卡槽回到初始位置: ->CMCardSlotMove(0, 0) Fail. "
                "ErrCode: %s, Return: %d.",
                CRM_ConvertDevErrCodeToStr(nCRMRet), CRM_ConvertDevErrCode2WFS(nCRMRet));
            SetErrorDetail(1);
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }
    } else
    {
        SetErrorDetail(1, (LPSTR)EC_CRM_XFS_ININotSupp);
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-执行CMRetainCard前设置吞卡卡号
HRESULT CXFS_IDC::CMSetCardData(LPCSTR lpszCardNo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;

    m_bIsSetCardData = FALSE;

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定吞卡卡号: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 吞卡卡号Check
        if (lpszCardNo == nullptr || strlen(lpszCardNo) < 12 ||
            strlen(lpszCardNo) > 54)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定吞卡卡号: 入参卡号[%s]无效(==NULL||<12||>54). Return: %d.",
                WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }

        // 检查卡号是否存在
        nCRMRet = m_pCRMDev->GetData(CARDNO_ISEXIST, (void*)lpszCardNo);
        if (nCRMRet == ERR_CRM_CARD_ISEXIST)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定吞卡卡号: 入参卡号[%s]已存在, Return: %d.",
                lpszCardNo, WFS_ERR_INVALID_DATA);
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

// 退卡模块(CRM)-执行吞卡到卡槽
HRESULT CXFS_IDC::CMRetainCard()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    INT nCRMRet = CRM_SUCCESS;
    INT nIDCRet = IDC_SUCCESS;
    int nSlotNo = 0;

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 吞卡到卡槽: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 检查 已执行SetCardData命令标记
        if (m_bIsSetCardData != TRUE)
        {
            Log(ThisModule, __LINE__,
                "CRM: 吞卡到卡槽: 未执行CMSetCardData()指定吞卡卡号. Return: %d.",
                WFS_ERR_INVALID_DATA);
            return WFS_ERR_INVALID_DATA;
        }
        m_bIsSetCardData = FALSE;


        // 吞卡前检查
        UpdateDeviceStatus();

        // 1. 读卡器检查
        // 读卡器无卡
        if (m_stStatus.fwMedia != WFS_IDC_MEDIAPRESENT)  // 读卡器中无卡
        {
            Log(ThisModule, __LINE__,  "CRM: 吞卡到卡槽: 读卡器无卡, Return: %d.",
                WFS_ERR_IDC_NOMEDIA);
            return WFS_ERR_IDC_NOMEDIA;
        }

        // 读卡器卡JAM
        if (m_stStatus.fwMedia == WFS_IDC_MEDIAJAMMED)
        {
            Log(ThisModule, __LINE__, "CRM: 吞卡到卡槽: 读卡器内卡处于JAM状态, Return: %d.",
                WFS_ERR_IDC_MEDIAJAM);
            return WFS_ERR_IDC_MEDIAJAM;
        }

        // INI设置卡在入口支持CMRetain
        if (m_stCRMConfig.wEnterCardRetainSup == 1)
        {
            if (m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING) // 卡在出口, 移动到内部(读卡器命令)
            {
                nIDCRet = m_pIDCDev->MediaControl(MEDIA_MOVE);
                if (nIDCRet != IDC_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "CRM: 吞卡到卡槽: 卡在读卡器出口(移动到内部): ->MediaControl(%d) Fail, ErrCode: %s, Return: %d.",
                        MEDIA_MOVE, ConvertDevErrCodeToStr(nIDCRet), ConvertDevErrCodeToStr(nIDCRet));
                    return hRet;
                }
            }
        }

        // 后出口有卡,移动到内部
        if (m_bAfteIsHaveCard == TRUE)
        {
            nIDCRet = m_pIDCDev->MediaControl(MEDIA_MOVE);
            if (nIDCRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 吞卡到卡槽: 卡在读卡器后出口(移动到内部): ->MediaControl(%d) Fail, ErrCode: %s, Return: %d.",
                    MEDIA_MOVE, ConvertDevErrCodeToStr(nIDCRet), ConvertDevErrCodeToStr(nIDCRet));
                return hRet;
            }
        }

        // 2. 移动空置卡槽对准读卡器后出口
        if ((nCRMRet = m_pCRMDev->CMCardSlotMove(1, nSlotNo)) != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 吞卡到卡槽: 移动第一个空卡槽对准读卡器后出口: ->CMCardSlotMove(1, %d) Fail, ErrCode: %s, Return: %d.",
                nSlotNo, CRM_ConvertDevErrCodeToStr(nCRMRet), CRM_ConvertDevErrCode2WFS(nCRMRet));
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }
        Log(ThisModule, __LINE__, "CRM: 吞卡到卡槽: 移动第一个空卡槽[%d]已对准读卡器后出口: Succ", nSlotNo);

        // 3. 读卡器中卡移动到后出口(读卡器命令),读卡器回收计数+1
        hRet = InnerRetainCard();
        if (hRet != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 吞卡到卡槽: 读卡器中卡移动到后出口: ->InnerRetainCard() Fail, ErrCode: %d, Return: %d.",
                hRet, hRet);
            return hRet;
        }

        // 4. CRM执行吞卡
        if ((nCRMRet = m_pCRMDev->CMRetainCard(m_szStorageCardNo, nSlotNo)) != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 吞卡到卡槽: ->CMRetainCard(%s, %d) Fail, ErrCode: %s, Return: %d.",
                m_szStorageCardNo, nSlotNo, m_szStorageCardNo, nSlotNo, CRM_ConvertDevErrCodeToStr(nCRMRet),
                CRM_ConvertDevErrCode2WFS(nCRMRet));
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }
        memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));
        m_bIsSetCardData = FALSE;
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
    //AutoLogFuncBeginEnd();

    STCRMSTATUS stCRMStat;
    INT nCRMRet = CRM_SUCCESS;

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            m_bCRMIsOnLine = FALSE;
            Log(ThisModule, __LINE__,
                "CRM: 获取状态: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 获取CRM状态
        if ((nCRMRet = m_pCRMDev->GetStatus(stCRMStat)) != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 获取状态: ->GetStatus() Fail. ErrCode: %s, Return: %d.",
                CRM_ConvertDevErrCodeToStr(nCRMRet), CRM_ConvertDevErrCode2WFS(nCRMRet));
            //return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }

        // 组织状态应答(118byte,分解如下)
        // 1-20:卡槽1卡号; 21-40:卡槽2卡号; 41-60:卡槽3卡号; 61-90:卡槽4卡号;
        // 81-100:卡槽5卡号; 101-102:已暂存卡数目; 103-112:5个卡槽senser状态;
        // 113-114:垂直电机状态; 115-116: 水平电机状态; 117-118:设备状态
        memset(lpucStatus, 0x00, 118);
        memset(lpucStatus, m_stCRMConfig.szPlaceHolder[0], 100);
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
                memcpy((CHAR*)(lpucStatus + 102 + i * 2), m_stCRMConfig.szSlotNoHaveCard, 2);
            } else
            {
                memcpy((CHAR*)(lpucStatus + 102 + i * 2), m_stCRMConfig.szSlotHaveCard, 2);
            }
        }
        sprintf((CHAR*)(lpucStatus + 112), "%02d%02d%02d",
                stCRMStat.wVertPowerStat, stCRMStat.wHoriPowerStat, stCRMStat.wDeviceStat);

        if (stCRMStat.wDeviceStat == 2)
        {
            m_bCRMIsOnLine = FALSE;
        } else
        {
            m_bCRMIsOnLine = TRUE;
        }
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
    //AutoLogFuncBeginEnd();

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 读卡器回收盒计数减1: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 读卡器回收盒计数减1(读卡器命令)
        if (UpdateRetainCards(RETCNT_RED_ONE) == 0)
        {
            Log(ThisModule, __LINE__, "CRM: 读卡器回收盒计数减1: ->UpdateRetainCards() Fail");
        } else
        {
            Log(ThisModule, __LINE__, "CRM: 读卡器回收盒计数减1: ->UpdateRetainCards() Succ");
        }

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
    //AutoLogFuncBeginEnd();

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 设置读卡器回收盒最大计数: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 设置读卡器回收盒最大计数(读卡器命令)
        if (UpdateRetainCards(RETCNT_SETSUM, *lpwCount) == 0)
        {
            Log(ThisModule, __LINE__,
                "CRM: 设置读卡器回收盒最大计数为[%d]: ->UpdateRetainCards() Fail",
                *lpwCount);
        } else
        {
            Log(ThisModule, __LINE__,
                "CRM: 设置读卡器回收盒最大计数为[%d]: ->UpdateRetainCards() Succ",
                *lpwCount);
        }
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
    //AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;
    INT nSlotNo = 0;

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 吞卡到读卡器回收盒: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 卡槽编号Check
        nSlotNo = atoi(lpszCardBox);
        Log(ThisModule, __LINE__, "CRM: 吞卡到读卡器回收盒(所有): 入参卡槽号[Str:%s]转换为[int:%d]",
            lpszCardBox, nSlotNo);

        if (nSlotNo == 0 && m_stCRMConfig.wEmptyAllCard0 != 0)   // 支持回收所有卡
        {
            CHAR szSlotNo[2];
            for (INT i = 1; i < 6; i ++)    // 循环回收所有卡
            {
                memset(szSlotNo, 0x00, sizeof(szSlotNo));
                sprintf(szSlotNo, "%d", i);

                // 检查卡槽是否有卡
                nCRMRet = m_pCRMDev->GetData(SLOTNO_ISEXIST, (void*)szSlotNo);
                if (nCRMRet == ERR_CRM_SLOT_NOTEXIST)
                {
                    Log(ThisModule, __LINE__,
                        "CRM: 吞卡到读卡器回收盒(所有): 指定卡槽[%s]无卡, 检查下一个卡槽.",szSlotNo);
                    continue;
                }

                // 吞卡到读卡器回收盒
                hRet = InnerCMEmptyCard(i);
                if (hRet != WFS_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "CRM: 吞卡到读卡器回收盒(所有): ->InnerCMEmptyCard(%d) fail, ErrCode: %d, Return: %d.",
                        i, hRet, hRet);
                    return hRet;
                }
            }
        } else
        {
            if (nSlotNo < 1 || nSlotNo > 5)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 吞卡到读卡器回收盒(指定): 指定卡槽[%s|%d]无效(==NULL||<1||>5), Return: %d.",
                    lpszCardBox, nSlotNo, WFS_ERR_INVALID_DATA);
                return WFS_ERR_INVALID_DATA;
            }

            // 检查卡槽是否有卡
            nCRMRet = m_pCRMDev->GetData(SLOTNO_ISEXIST, (void*)lpszCardBox);
            if (nCRMRet == ERR_CRM_SLOT_NOTEXIST)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 吞卡到读卡器回收盒(指定): 指定卡槽[%d]无卡: Return: %d.",
                    nSlotNo, WFS_ERR_IDC_CMNOMEDIA);
                return WFS_ERR_IDC_CMNOMEDIA;
            }

            // 吞卡到读卡器回收盒
            hRet = InnerCMEmptyCard(nSlotNo);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 吞卡到读卡器回收盒(指定): ->InnerCMEmptyCard(%d) fail, ErrCode: %d, Return: %d.",
                    nSlotNo, hRet, hRet);
                return hRet;
            }
        }
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-获取卡槽信息
HRESULT CXFS_IDC::CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024])
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    STCRMSLOTINFO stCRMInfo;
    INT nCRMRet = CRM_SUCCESS;

    CHAR szCardNo[20+1] = { 0x00 };
    CHAR szInTime[14+1] = { 0x00 };

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 获取卡槽信息: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 获取CRM吞卡时间
        if ((nCRMRet = m_pCRMDev->GetCardSlotInfo(stCRMInfo)) != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 获取卡槽信息: ->GetCardSlotInfo() Fail. ErrCode: %s, Return: %d.",
                CRM_ConvertDevErrCodeToStr(nCRMRet), CRM_ConvertDevErrCode2WFS(nCRMRet));
            //return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }

        // 组织应答,每组数据38byte(DX-YYYYYYYYYYYYYYYYYYYY-YYYYMMDDHHmmss),解析如下:
        // X:卡槽编号; YY...YY:20位卡号，不足右补F; YY...ss:14位时间
        for (int i = 0; i < 5; i ++)
        {
            //memset(szCardNo, 'F', sizeof(szCardNo) - 1);
            //memset(szInTime, 'F', sizeof(szInTime) - 1);
            memset(szCardNo, m_stCRMConfig.szPlaceHolder[0], sizeof(szCardNo) - 1);
            memset(szInTime, m_stCRMConfig.szPlaceHolder[0], sizeof(szInTime) - 1);

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
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;
    INT nCRMRet = CRM_SUCCESS;

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 设备复位: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 检查读卡器后出口是否有卡, 有卡则将卡移到内部, 避免影响CRM复位
        UpdateDeviceStatus(); // 更新当前设备介质状态
        if (m_bAfteIsHaveCard == TRUE)
        {
            hRet = m_pIDCDev->MediaControl(MEDIA_MOVE);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 设备复位: 读卡器后出口有卡(移到内部): ->MediaControl(%d) Fail. Return: %d",
                    MEDIA_MOVE, hRet);
                return hRet;
            }
        }

        // 执行设备复位
        nCRMRet = m_pCRMDev->CMReset();
        if (nCRMRet != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 设备复位: ->CMReset() Fail, ErrCode: %s, Return: %d.",
                CRM_ConvertDevErrCodeToStr(nCRMRet), CRM_ConvertDevErrCode2WFS(nCRMRet));
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }

        m_bIsSetCardData = FALSE;     // RetainCard前需要执行SetCardData命令
        memset(m_szStorageCardNo, 0x00, sizeof(m_szStorageCardNo));
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-所有卡吞到读卡器回收盒
HRESULT CXFS_IDC::CMEmpytAllCard()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;
    INT nSlotNo = 0;

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 所有卡吞到读卡器回收盒: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 卡槽编号Check
        CHAR szSlotNo[2];
        for (INT i = 1; i < 6; i ++)    // 循环回收所有卡
        {
            memset(szSlotNo, 0x00, sizeof(szSlotNo));
            sprintf(szSlotNo, "%d", i);

            // 检查卡槽是否有卡
            nCRMRet = m_pCRMDev->GetData(SLOTNO_ISEXIST, (void*)szSlotNo);
            if (nCRMRet == ERR_CRM_SLOT_NOTEXIST)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 所有卡吞到读卡器回收盒: 指定卡槽[%s]无卡, 检查下一个卡槽.",szSlotNo);
                continue;
            }

            // 吞卡到读卡器回收盒
            hRet = InnerCMEmptyCard(i);
            if (hRet != WFS_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "CRM: 所有卡吞到读卡器回收盒: ->InnerCMEmptyCard(%d) fail, ErrCode: %d, Return: %d.",
                    i, hRet, hRet);
                return hRet;
            }
        }
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-清除指定卡槽信息
HRESULT CXFS_IDC::CMClearSlot(LPCSTR lpszSlotNo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;

    if (m_stCRMConfig.bCRMDeviceSup == TRUE) // 支持启用CRM
    {
        if (m_pCRMDev == nullptr)   // 无效/设备未Open
        {
            Log(ThisModule, __LINE__,
                "CRM: 清除指定卡槽信息: 设备未Open/设备句柄无效(m_pCRMDev Is NULL) Fail. Return: %d",
                WFS_ERR_DEV_NOT_READY);
            SetErrorDetail(1, (LPSTR)EC_CRM_XFS_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 卡槽编号
        WORD wSlotNo = 0;
        wSlotNo = atoi(lpszSlotNo);
        Log(ThisModule, __LINE__, "CRM: 清除指定卡槽信息: 入参卡槽号[Str:%s]转换为[int:%d]",
            lpszSlotNo, wSlotNo);

        nCRMRet = m_pCRMDev->SetData(SET_CLEAR_SLOT, &wSlotNo);
        if (nCRMRet != CRM_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 清除指定卡槽信息: ->SetData(%d, %d) Fail, ErrCode: %d, Return: %d.",
                SET_CLEAR_SLOT, wSlotNo, CRM_ConvertDevErrCodeToStr(nCRMRet), CRM_ConvertDevErrCode2WFS(nCRMRet));
            return CRM_ConvertDevErrCode2WFS(nCRMRet);
        }
    } else
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

// 指定卡槽号退卡回收
HRESULT CXFS_IDC::InnerCMEmptyCard(INT nSlotNo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nCRMRet = CRM_SUCCESS;
    INT nIDCRet = WFS_SUCCESS;

    // CRM退卡前检查读卡器内是否有卡(读卡器命令)
    UpdateDeviceStatus(); // 更新当前设备介质状态

    if (m_stStatus.fwMedia == WFS_IDC_MEDIALATCHED ||   // 内部有卡且锁定
        m_stStatus.fwMedia == WFS_IDC_MEDIAPRESENT ||   // 内部有卡
        m_stStatus.fwMedia == WFS_IDC_MEDIAENTERING)    // 出口有卡
    {
        Log(ThisModule, __LINE__,
            "CRM: 子处理: 指定卡槽号退卡回收: 检查读卡器: MediaStat = %d|内部或出口有卡,  Return: %d.",
            m_stStatus.fwMedia, WFS_ERR_DEV_NOT_READY);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 检查读卡器回收盒是否FULL
    if (m_stStatus.fwRetainBin == WFS_IDC_RETAINBINFULL)
    {
        Log(ThisModule, __LINE__,
            "CRM: 子处理: 指定卡槽号退卡回收: 读卡器回收盒满, Return: %d.",
            WFS_ERR_IDC_RETAINBINFULL);
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

    // 执行退卡(指定卡槽编号)
    nCRMRet = m_pCRMDev->CMEjectCard(nSlotNo);
    if ((nCRMRet) != CRM_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "CRM: 子处理: 指定卡槽号退卡回收: ->CMEjectCard(%d) Fail, ErrCode: %s, Return: %d.",
            nSlotNo, CRM_ConvertDevErrCodeToStr(nCRMRet), CRM_ConvertDevErrCode2WFS(nCRMRet));
        m_bThreadEjectExit = TRUE;  // CRM ERR,结束线程(循环下发读卡器退卡)
        return CRM_ConvertDevErrCode2WFS(nCRMRet);
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
            Log(ThisModule, __LINE__,
                "CRM: 子处理: 指定卡槽号退卡回收: 卡槽[%d]卡后出口移到读卡器内部超时, Return: %d.",
                nSlotNo, WFS_ERR_TIMEOUT);
            return WFS_ERR_TIMEOUT;
        } else
        if (m_CardReaderEjectFlag != WFS_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "CRM: 子处理: 指定卡槽号退卡回收: 卡槽[%d]卡后出口移到读卡器内部失败, Return: %d.",
                nSlotNo, m_CardReaderEjectFlag);
            return m_CardReaderEjectFlag;
        }
    }

    // 退卡成功，结束线程(循环下发读卡器退卡)
    m_bThreadEjectExit = TRUE;

    // 卡CMRetain进入回收时,读卡器回收计数+1; CMEject退卡成功时,读卡器回收计数-1
    UpdateRetainCards(RETCNT_RED_ONE);

    // 移动卡槽回到初始位置
    INT nInitSlotNo = 0;
    if ((nCRMRet = m_pCRMDev->CMCardSlotMove(0, nInitSlotNo)) != CRM_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "CRM: 子处理: 指定卡槽号退卡回收: 移动卡槽到初始位置: ->CMCardSlotMove(0, 0) Fail. "
            "ErrCode: %s, Return: %d.",
            CRM_ConvertDevErrCodeToStr(nCRMRet), CRM_ConvertDevErrCode2WFS(nCRMRet));
        return CRM_ConvertDevErrCode2WFS(nCRMRet);
    }

    // 卡收进回收盒+回收计数加1(读卡器命令)
    nIDCRet = InnerRetainCard();
    if (nIDCRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "CRM: 子处理: 指定卡槽号退卡回收: 读卡器内卡收进回收盒: ->InnerRetainCard() Fail. Return: %d.",
            nIDCRet);
        return nIDCRet;
    }

    return WFS_SUCCESS;
}

// 退卡模块(CRM)-退卡时要求与读卡器同时动作，该接口为读卡器后出口吸卡动作循环执行
void CXFS_IDC::ThreadEject_Wait()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    WORD    wCount = 0;

    while(wCount < (WORD)(1000 * 3 / 20))
    {
        if (m_bThreadEjectExit == TRUE)
        {
            break;
        }
        wCount ++;
        m_CardReaderEjectFlag = m_pIDCDev->MediaControl(MEDIA_MOVE); // 卡移动到读卡器内部(读卡器命令)
        if (m_CardReaderEjectFlag == WFS_SUCCESS)
        {
            break;
        }
        usleep(1000 * 20);
    }
    m_bThreadEjectExit = TRUE;
}
