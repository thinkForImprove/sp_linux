/***************************************************************************
* 文件名称：XFS_CRM.cpp
* 文件描述：退卡模块命令处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
***************************************************************************/

#include "XFS_IDC.h"

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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 卡号Check
        if (lpszCardNo == nullptr || strlen(lpszCardNo) < 1 ||
            strlen(lpszCardNo) > 20)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定卡号退卡: 入参卡号[%s] 无效(==NULL||<1). Return: %d.",
                lpszCardNo, WFS_ERR_INVALID_DATA);
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_CardNoInv);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_IDCHaveMed);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_CardNotFound);
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
        SetErrorDetail(1, (LPSTR)EC_CRM_ERR_ININotSupp);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
            return WFS_ERR_DEV_NOT_READY;
        }

        // 吞卡卡号Check
        if (lpszCardNo == nullptr || strlen(lpszCardNo) < 1||
            strlen(lpszCardNo) > 54)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定吞卡卡号: 入参卡号[%s]无效(==NULL||<1||>54). Return: %d.",
                WFS_ERR_INVALID_DATA);
            SetErrorDetail(1, EC_CRM_ERR_CardNoInv);
            return WFS_ERR_INVALID_DATA;
        }

        // 检查卡号是否存在
        nCRMRet = m_pCRMDev->GetData(CARDNO_ISEXIST, (void*)lpszCardNo);
        if (nCRMRet == ERR_CRM_CARD_ISEXIST)
        {
            Log(ThisModule, __LINE__,
                "CRM: 指定吞卡卡号: 入参卡号[%s]已存在, Return: %d.",
                lpszCardNo, WFS_ERR_INVALID_DATA);
            SetErrorDetail(1, EC_CRM_ERR_CardNoIsHave);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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
            SetErrorDetail(1, (LPSTR)EC_CRM_ERR_DevNotOpen);
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

// -------------------------------------- END --------------------------------------
