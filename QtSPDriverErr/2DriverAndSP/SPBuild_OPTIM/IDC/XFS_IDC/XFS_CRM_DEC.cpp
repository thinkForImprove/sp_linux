/***************************************************************************
* 文件名称：XFS_CRM_DEC.cpp
* 文件描述：退卡模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
***************************************************************************/

#include "XFS_IDC.h"

//-----------------------------基本接口----------------------------------
// OpenCRM命令入口
HRESULT CXFS_IDC::StartOpenCRM()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = WFS_SUCCESS;

    if (m_stCRMConfig.bCRMDeviceSup != TRUE)     // 不支持启用退卡模块
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定不支持的启用CRM设备, Return: SUCCESS.",
            WFS_SUCCESS);
        SetErrorDetail(1, (LPSTR)EC_CRM_ERR_ININotSupp);
        return WFS_SUCCESS;
    }

    // 检查INI设置设备类型
    if (m_stCRMConfig.wCRMDeviceType != XFS_CRM_CRT730B)
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定了不支持的CRM设备类型[%d], Return :%d.",
            m_stConfig.wDeviceType, WFS_ERR_DEV_NOT_READY);
        SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotSupp);
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

        // Open前下传初始参数(非断线重连)
        if (bReConn == FALSE)
        {
            // 设置SDK路径
            /*if (strlen(m_stCRMConfig.szSDKPath) > 0)
            {
                m_pIDCDev->SetData(SET_LIB_PATH, m_stConfig.szSDKPath);
            }*/

            // 设置设备打开模式
            m_pCRMDev->SetData(SET_DEV_OPENMODE, &(m_stCRMConfig.stDevOpenMode));
        }

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
        /*m_cStatExtra.AddExtra("CRM_VRTCount", "1");
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
        }*/

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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevCRMLoadFail);
            return FALSE;
        }
    }

    return (m_pCRMDev != nullptr);
}

// 加载INI设置(CRM)
INT CXFS_IDC::InitConfigCRM()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR szBuffer[1024] = { 0x00 };

    //--------------------退卡模块INI配置参数获取--------------------
    // 是否支持启用退卡模块: 0不支持/1支持,缺省0
    m_stCRMConfig.bCRMDeviceSup =
            ((WORD)m_cXfsReg.GetValue("CRM_DEVICE_CFG", "CRMDeviceSup", (DWORD)0) == 1);

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

        // 设备初始化动作(0正常归位;1强行退卡;2强行暂存;3无动作,缺省0)
        m_stCRMConfig.wDeviceInitAction = (WORD)m_cXfsReg.GetValue("CRM_DEVICE_CFG", "DeviceInitAction", (DWORD)0);
        if (m_stCRMConfig.wDeviceInitAction < 0 || m_stCRMConfig.wDeviceInitAction > 3)
        {
            m_stCRMConfig.wDeviceInitAction = 0;
        }

        // 退卡后卡位置(0读卡器内部;1读卡器前入口,缺省0）
        m_stCRMConfig.wEjectCardPos = (WORD)m_cXfsReg.GetValue("CRM_DEVICE_CFG", "EjectCardPos", (DWORD)0);
        if (m_stCRMConfig.wEjectCardPos < 0 || m_stCRMConfig.wEjectCardPos > 1)
        {
            m_stCRMConfig.wEjectCardPos = 0;
        }

        // 读卡器前入口有卡是否支持收入卡槽（0不支持;1支持,缺省0)
        m_stCRMConfig.wEnterCardRetainSup = (WORD)m_cXfsReg.GetValue("CRM_DEVICE_CFG", "EnterCardRetainSup", (DWORD)0);
        if (m_stCRMConfig.wEnterCardRetainSup < 0 || m_stCRMConfig.wEnterCardRetainSup > 1)
        {
            m_stCRMConfig.wEnterCardRetainSup = 0;
        }

        CHAR szKeyName[32];
        sprintf(szKeyName, "CRM_DEVICE_SET_%d", m_stCRMConfig.wCRMDeviceType);

        if (m_stCRMConfig.wCRMDeviceType == XFS_CRM_CRT730B)
        {
            m_stCRMConfig.stDevOpenMode.nOtherParam[0] = XFS_CRM_CRT730B;

            // 1. 设备连接串,缺省(USB:23D8,0730)
            strcpy(m_stCRMConfig.szCRMDeviceConList, m_cXfsReg.GetValue(szKeyName, "DeviceConList", "USB:23D8,0730"));
            strcpy(m_stCRMConfig.stDevOpenMode.szDevPath[0],
                   m_cXfsReg.GetValue(szKeyName, "DeviceConList", "USB:23D8,0730"));


        }

        if (m_stCRMConfig.wCRMDeviceType == XFS_cRM_ZDCRM01)
        {

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

// -------------------------------------- END --------------------------------------
