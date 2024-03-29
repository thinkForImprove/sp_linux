/***************************************************************
* 文件名称：DevIDC_CRT730B.h
* 文件描述：退卡模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/
#include "DevCRM_CRT730B.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include "../XFS_IDC/def.h"

// CRM 版本号
//BYTE    byDevVRTU[17] = {"DevCRM00000100"};

static const char *ThisModule = "DevCRM_CRT730B.cpp";

//////////////////////////////////////////////////////////////////////////

CDevCRM_CRT730B::CDevCRM_CRT730B() : m_devCRT730B(LOG_NAME_DEVCRM)
{
    SetLogFile(LOG_NAME_DEVCRM, ThisModule);  // 设置日志文件名和错误发生的文件

    m_clErrorDet.SetDevName(EC_DEV_CRM);
    m_clErrorDet.SetHWName(EC_HW_DEF);

    m_wDeviceType = 1;                  // 设备类型
}

CDevCRM_CRT730B::~CDevCRM_CRT730B()
{
    Close();
}

int CDevCRM_CRT730B::Release()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CRM_SUCCESS;
}

// 建立CRM连接
int CDevCRM_CRT730B::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 建立CRM连接
    if (m_devCRT730B.IsDeviceOpen() != TRUE)
    {
        if ((nRet = m_devCRT730B.DeviceOpen((LPSTR)lpMode)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "建立CRM连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
                lpMode, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }

    return CRM_SUCCESS;
}

// CRM初始化
int CDevCRM_CRT730B::Init(CRMInitAction eActFlag)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // CRM初始化
    if ((nRet = m_devCRT730B.DeviceInit(eActFlag)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "CRM初始化: ->DeviceInit(%d) Fail, ErrCode=%d, Return %s.",
            eActFlag, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    // 更新暂存仓结构体信息
    if ((nRet = UpdataSTSlotData()) != CRM_SUCCESS)
    {
        Log(ThisModule, __LINE__, "CRM初始化: 更新暂存仓结构体信息: ->UpdataSTSlotData() Fail, ErrCode=%d, Return %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return CRM_SUCCESS;
}

// 关闭CRM连接
int CDevCRM_CRT730B::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_devCRT730B.DeviceClose();
    Log(ThisModule, __LINE__, "关闭CRM连接: ->DeviceClose() Succ, Return %s.",
        ConvertErrCodeToStr(CRM_SUCCESS));

    return CRM_SUCCESS;
}

// 设备复位
int CDevCRM_CRT730B::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return CRM_SUCCESS;
}

int CDevCRM_CRT730B::GetDevInfo(char *pInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return CRM_SUCCESS;
}

// 读取暂存仓信息
int CDevCRM_CRT730B::GetCardSlotInfo(STCRMSLOTINFO &stInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    stInfo.Clear();

    if (m_stCardSlotInfo.SlotHaveCount() > 0)
    {
        for (int i = 0; i < 5; i ++)
        {
            if (m_stCardSlotInfo.stSlots[i].bIsHave == TRUE)
            {
                stInfo.bSlotHave[i] = TRUE;
                memcpy(stInfo.szSlotCard[i], m_stCardSlotInfo.stSlots[i].szCardNo,
                       strlen(m_stCardSlotInfo.stSlots[i].szCardNo));
                memcpy(stInfo.szStorageTime[i], m_stCardSlotInfo.stSlots[i].szInTime,
                       strlen(m_stCardSlotInfo.stSlots[i].szInTime));
            }
        }
    }

    return CRM_SUCCESS;
}

// 读取设备状态
int CDevCRM_CRT730B::GetStatus(STCRMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    INT    nDevStat[12];

    stStatus.Clear();

    if (m_devCRT730B.IsDeviceOpen() != TRUE)
    {
        Log(ThisModule, __LINE__, "读取设备状态: ->IsDeviceOpen() Is FALSE, Device Not Open, Return %s.",
            ConvertErrCodeToStr(ERR_CRM_NOT_OPEN));
        stStatus.wDeviceStat = 2;   // 设备不在线
        return ERR_CRM_NOT_OPEN;
    }

    // 取设备状态
    // nStat　返回状态数组,如下: 各状态参考宏定义
    //             [0]:设备状态; [1]:暂存仓1状态; [2]:暂存仓2状态;
    //             [3]:暂存仓3状态; [4]:暂存仓4状态; [5]:暂存仓5状态;
    //             [6]:维护门状态;7~11位保留
    memset(nDevStat, 0, sizeof(nDevStat));
    if ((nRet = m_devCRT730B.GetDeviceStat(MODE_STAT_ALL, nDevStat)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取设备状态: ->GetDeviceStat(%d) Fail, ErrCode=%d, Return %s.",
            MODE_STAT_ALL, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        if (nRet == IMP_ERR_DEVPORT_NOTOPEN)
        {
            stStatus.wDeviceStat = 2;   // 未连接/未打开
        } else
        {
            stStatus.wDeviceStat = 3;   // 硬件故障
        }
        return ConvertErrorCode(nRet);
    }    

    // 组织应答状态
    // 设备状态(0/繁忙;1正常;2不在线;3硬件故障;4卡槽满;
    //         5卡槽空;6读卡器故障;7读卡器和模块都不正常;8可复位)

    if (nDevStat[0] == IMPL_STAT_CRDSLOT_POSITION_FAIL ||      // 暂存仓位置异常
        nDevStat[6] == IMPL_STAT_SAFEDOOR_OPEN)                // 维护门开启
    {
        stStatus.wDeviceStat = 3;   // 硬件故障
        return ERR_CRM_HWERR;
    }

    if (nDevStat[1] == IMPL_STAT_CRDSLOT_ISHAVE)         // 暂存仓1有卡
    {
        stStatus.wSensorStat[0] = 1;
    } else
    {
        m_stCardSlotInfo.Clear(1);
        stStatus.wSensorStat[0] = 0;
    }
    if (nDevStat[2] == IMPL_STAT_CRDSLOT_ISHAVE)        // 暂存仓2有卡
    {
        stStatus.wSensorStat[1] = 1;
    } else
    {
        m_stCardSlotInfo.Clear(2);
        stStatus.wSensorStat[1] = 0;
    }
    if (nDevStat[3] == IMPL_STAT_CRDSLOT_ISHAVE)        // 暂存仓3有卡
    {
        stStatus.wSensorStat[2] = 1;
    } else
    {
        m_stCardSlotInfo.Clear(3);
        stStatus.wSensorStat[2] = 0;
    }
    if (nDevStat[4] == IMPL_STAT_CRDSLOT_ISHAVE)        // 暂存仓4有卡
    {
        stStatus.wSensorStat[3] = 1;
    } else
    {
        m_stCardSlotInfo.Clear(4);
        stStatus.wSensorStat[3] = 0;
    }
    if (nDevStat[5] == IMPL_STAT_CRDSLOT_ISHAVE)        // 暂存仓5有卡
    {
        stStatus.wSensorStat[4] = 1;
    } else
    {
        m_stCardSlotInfo.Clear(5);
        stStatus.wSensorStat[4] = 0;
    }

    if (m_stCardSlotInfo.SlotHaveCount() == 0)
    {
        stStatus.wDeviceStat = 5;   // 卡槽空
    } else
    if (m_stCardSlotInfo.SlotHaveCount() == 5)
    {
        stStatus.wDeviceStat = 4;   // 卡槽满
    }

    // 卡号 + 以暂存卡数目
    stStatus.wStorageCount = 0;
    if (m_stCardSlotInfo.SlotHaveCount() > 0)
    {
        for (int i = 0; i < 5; i ++)
        {
            if (m_stCardSlotInfo.stSlots[i].bIsHave == TRUE)
            {
                memcpy(stStatus.szSlotCard[i], m_stCardSlotInfo.stSlots[i].szCardNo,
                       sizeof(stStatus.szSlotCard[i]) - 1);
                stStatus.wStorageCount ++;
            }
        }
    }

    return CRM_SUCCESS;
}

// 指定卡号退卡
int CDevCRM_CRT730B::CMEjectCard(const char *szCardNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    WORD   wSlotNo = 0;

    for (int i = 0; i < 5; i ++)
    {
        if (m_stCardSlotInfo.stSlots[i].bIsHave == TRUE)
        {
            // 支持 指定非完整卡号退卡
            int nLen = strlen(m_stCardSlotInfo.stSlots[i].szCardNo);
            if (strlen(szCardNo) > nLen)
            {
                continue;
            }
            if (memcmp(m_stCardSlotInfo.stSlots[i].szCardNo + (nLen - strlen(szCardNo)),
                       szCardNo, strlen(szCardNo)) == 0)
            {
                wSlotNo = i + 1;
                break;
            }
        }
    }

    if (wSlotNo == 0)
    {
        Log(ThisModule, __LINE__, "指定卡号退卡: CardNo[%s] Not Found, Return %s.",
            szCardNo, ConvertErrCodeToStr(ERR_CRM_CARD_NOTFOUND));
        return ERR_CRM_CARD_NOTFOUND;
    }

    Log(ThisModule, __LINE__, "指定卡号退卡: CardNo[%s]==SlotNo[%d]: ->CMEjectCard(%d).",
        szCardNo, wSlotNo, wSlotNo);
    return CMEjectCard(wSlotNo);

}

// 指定暂存仓编号退卡
int CDevCRM_CRT730B::CMEjectCard(const int nSlotNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;

    if (nSlotNo < 6 && nSlotNo > 0)
    {
        // 指定暂存仓对准卡口
        if ((nRet = m_devCRT730B.CardSlotMove(MODE_SLOT_MOVEPOS, nSlotNo)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "指定暂存仓编号退卡: 指定暂存仓[%d]对准卡口: ->CardSlotMove(%d, %d) Fail, "
                                      "ErrCode=%d, Return %s.",
                nSlotNo, MODE_SLOT_MOVEPOS, nSlotNo, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }

        // 执行退卡
        if ((nRet = m_devCRT730B.CardEject()) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "指定暂存仓编号退卡: 执行退卡: ->CardEject() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }

        // 指定暂存仓回到初始位置
        /*if ((nRet = m_devCRT730B.CardSlotMove(MODE_SLOT_MOVEINIT, nSlotNo)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "指定暂存仓编号退卡: 暂存仓回到初始位置: ->CardSlotMove(%d, %d) Fail,
                                      "ErrCode=%d, Return %d.",
                MODE_SLOT_MOVEINIT, nSlotNo, nRet, ConvertErrorCode(nRet));
            return ConvertErrorCode(nRet);
        }*/

        // 清除暂存仓信息
        if ((nRet = m_devCRT730B.ClearCardSoltInfo(nSlotNo)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "指定暂存仓编号退卡: 清除指定暂存仓[%d]信息: ->ClearCardSoltInfo(%d) Fail, "
                                      "ErrCode=%d, Return %s.",
                nSlotNo, nSlotNo, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }

        // 清除暂存仓结构体变量信息
        m_stCardSlotInfo.Clear(nSlotNo);
    } else
    {
        Log(ThisModule, __LINE__, "指定暂存仓编号退卡: SlotNo[%d] Not Found, Return %d.",
            nSlotNo, ConvertErrCodeToStr(ERR_CRM_CARD_NOTFOUND));
        return ERR_CRM_CARD_NOTFOUND;
    }

    return CRM_SUCCESS;
}

// 指定卡号存卡
int CDevCRM_CRT730B::CMRetainCard(const char *szCardNo, const int nSlotNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    //WORD   wSlotNo = 0;

    /*for (int i = 0; i < 5; i ++)
    {
        if (m_stCardSlotInfo.stSlots[i].bIsHave == TRUE)
        {
            if (memcmp(m_stCardSlotInfo.stSlots[i].szCardNo,
                       szCardNo, strlen(szCardNo)) == 0 &&
                memcmp(m_stCardSlotInfo.stSlots[i].szCardNo,
                       szCardNo, strlen(m_stCardSlotInfo.stSlots[i].szCardNo)) == 0)
            {
                Log(ThisModule, __LINE__, "指定卡号收卡: CardNo[%s] Is Exist, Return %d.",
                    szCardNo, ERR_CRM_CARD_ISEXIST);
                return ERR_CRM_CARD_ISEXIST;
            }
        } else
        {
            if (wSlotNo < 1 || wSlotNo > 5)
            {
                wSlotNo = i + 1;    // 找到空置暂存仓
            }
        }
    }*/

    // 指定暂存仓对准卡口
    /*if ((nRet = m_devCRT730B.CardSlotMove(MODE_SLOT_MOVEPOS, wSlotNo)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "指定卡号收卡: 指定暂存仓对准卡口: ->CardSlotMove(%d, %d) Fail, ErrCode=%d, Return %d.",
            MODE_SLOT_MOVEPOS, wSlotNo, nRet, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }*/

    // 执行存卡
    if ((nRet = m_devCRT730B.CardStorage()) != IMP_SUCCESS)
    {
        if (nRet == IMP_ERR_DEVRET_31)
        {
            Log(ThisModule, __LINE__, "指定卡号收卡: 执行收卡: ->CardStorage() Fail, ErrCode=%d, 作为成功处理, 收卡成功.",
                ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        } else
        {
            Log(ThisModule, __LINE__, "指定卡号收卡: 执行收卡: ->CardStorage() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }

    // 移动暂存仓回到初始位置
    if ((nRet = m_devCRT730B.CardSlotMove(MODE_SLOT_MOVEINIT, nSlotNo)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "指定卡号收卡: 移动暂存仓回到初始位置: ->CardSlotMove(%d, %d) Fail, ErrCode=%d, Return %s.",
            MODE_SLOT_MOVEINIT, nSlotNo, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    // 修正暂存仓结构体变量信息
    m_stCardSlotInfo.Clear(nSlotNo);
    m_stCardSlotInfo.stSlots[nSlotNo - 1].bIsHave = TRUE;
    memcpy(m_stCardSlotInfo.stSlots[nSlotNo - 1].szCardNo, szCardNo, strlen(szCardNo));
    time_t tTimeCount = time(nullptr);
    struct tm *stLocalTime = localtime(&tTimeCount);
    sprintf(m_stCardSlotInfo.stSlots[nSlotNo - 1].szInTime, "%04d%02d%02d%02d%02d%02d",
            stLocalTime->tm_year + 1900, stLocalTime->tm_mon + 1, stLocalTime->tm_mday,
            stLocalTime->tm_hour, stLocalTime->tm_min, stLocalTime->tm_sec);

    // 写入暂存仓信息
    CHAR szBuffer[1024] = { 0x00 };
    sprintf(szBuffer, "%014s%s", m_stCardSlotInfo.stSlots[nSlotNo - 1].szInTime,
            m_stCardSlotInfo.stSlots[nSlotNo - 1].szCardNo);
    if ((nRet = m_devCRT730B.WriteCardSlotInfo(nSlotNo, (LPSTR)szBuffer)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "指定卡号退卡: 写入暂存仓[%d]信息: ->WriteCardSlotInfo(%d, %s) Fail, "
                                  "ErrCode=%d, Return %s.",
            nSlotNo, nSlotNo, szBuffer, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        m_stCardSlotInfo.Clear(nSlotNo);
        return ConvertErrorCode(nRet);
    }

    return CRM_SUCCESS;
}

// 移动暂存仓
int CDevCRM_CRT730B::CMCardSlotMove(const int nMode, int &nSlotNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    WORD   wCardSlotNo = 0;

    if (nMode == 0)
    {
        // 移动暂存仓回到初始位置
        if ((nRet = m_devCRT730B.CardSlotMove(MODE_SLOT_MOVEINIT, wCardSlotNo)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "移动暂存仓: 移动暂存仓回到初始位置: ->CardSlotMove(%d, %d) Fail, "
                                      "ErrCode=%d, Return %s.",
                MODE_SLOT_MOVEINIT, wCardSlotNo, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (nMode == 1) // 移动第1个空置的暂存仓对准卡口
    {
        if (m_stCardSlotInfo.SlotHaveCount() >= 5)
        {
            Log(ThisModule, __LINE__, "移动暂存仓: 无空置暂存仓/暂存仓满, Return %s.",
                wCardSlotNo, MODE_SLOT_MOVEPOS, wCardSlotNo, nRet,
                ConvertErrCodeToStr(ERR_CRM_SLOT_FULL));
            return ERR_CRM_SLOT_FULL;
        }

        for (int i = 0; i < 5; i ++)
        {
            if (m_stCardSlotInfo.stSlots[i].bIsHave == FALSE)
            {
                wCardSlotNo = i + 1;    // 找到空置暂存仓
                break;
            }
        }

        // 指定空置暂存仓对准卡口
        if ((nRet = m_devCRT730B.CardSlotMove(MODE_SLOT_MOVEPOS, wCardSlotNo)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "移动暂存仓: 指定空置暂存仓[%d]对准卡口: ->CardSlotMove(%d, %d) Fail, "
                                    "ErrCode=%d, Return %s.",
                wCardSlotNo, MODE_SLOT_MOVEPOS, wCardSlotNo, nRet,
                ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
        nSlotNo = wCardSlotNo;
    } else
    if (nMode == 2) // 指定暂存仓对准卡口
    {
        if (nSlotNo < 1 || nSlotNo > 5)
        {
            Log(ThisModule, __LINE__, "移动暂存仓: 指定暂存仓[%d]无效, Return %s.",
                nSlotNo, ConvertErrCodeToStr(ERR_CRM_PARAM_ERR));
            return ERR_CRM_PARAM_ERR;
        }

        // 指定暂存仓对准卡口
        if ((nRet = m_devCRT730B.CardSlotMove(MODE_SLOT_MOVEPOS, nSlotNo)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "移动暂存仓: 指定暂存仓[%d]对准卡口: ->CardSlotMove(%d, %d) Fail, "
                                    "ErrCode=%d, Return %s.",
                nSlotNo, MODE_SLOT_MOVEPOS, nSlotNo, nRet,
                ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        Log(ThisModule, __LINE__, "移动暂存仓: 移动方式[%d]无效, Return %s.",
            nMode, ConvertErrCodeToStr(ERR_CRM_PARAM_ERR));
        return ERR_CRM_PARAM_ERR;
    }

    return CRM_SUCCESS;
}

// 设备复位
int CDevCRM_CRT730B::CMReset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;

    // 执行设备复位
    /*if ((nRet = m_devCRT730B.DeviceReset()) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备复位: ->DeviceReset() Fail, ErrCode=%d, Return %s.",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/
    if ((nRet = m_devCRT730B.DeviceInit(CRMINIT_HOMING)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备复位: ->DeviceInit(%d) Fail, ErrCode=%d, Return %s.",
            CRMINIT_HOMING, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    // 更新暂存仓结构体信息
    if ((nRet = UpdataSTSlotData()) != CRM_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备复位: 更新暂存仓结构体信息: ->UpdataSTSlotData() Fail, "
                                  "ErrCode=%d, Return %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return CRM_SUCCESS;
}

// 更新暂存仓结构体信息
int CDevCRM_CRT730B::UpdataSTSlotData()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;

    // 取设备状态
    INT nDevStat[12] = { 0x00 };
    if ((nRet = m_devCRT730B.GetDeviceStat(MODE_STAT_ALL, nDevStat)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "更新暂存仓结构体信息: 取设备状态: ->GetDeviceStat(%d) Fail, "
                                "ErrCode=%d, 不更新信息, Return %s.",
            MODE_STAT_ALL, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    } else
    {
        Log(ThisModule, __LINE__, "更新暂存仓结构体信息: 取状态信息=%d%d%d%d%d%d%d%d%d%d%d%d",
            nDevStat[0], nDevStat[1], nDevStat[2], nDevStat[3], nDevStat[4], nDevStat[5],
            nDevStat[6], nDevStat[7], nDevStat[8], nDevStat[9], nDevStat[10], nDevStat[11]);

        if (nDevStat[0] == IMPL_STAT_CRDSLOT_POSITION_FAIL ||      // 暂存仓位置异常
            nDevStat[6] == IMPL_STAT_SAFEDOOR_OPEN)                // 维护门开启
        {
            Log(ThisModule, __LINE__,
                "更新暂存仓结构体信息: 状态信息: 暂存仓位置异常||维护门异常, 不更新信息, Return %s.",
                ConvertErrCodeToStr(ERR_CRM_HWERR));
            return ERR_CRM_HWERR;
        } else
        {
            // 更新暂存仓结构体信息
            CHAR    szSlotInfo[512] = { 0x00 };
            m_stCardSlotInfo.Clear();
            for (WORD i = 0; i < 5; i ++)
            {
                if (nDevStat[i + 1] == IMPL_STAT_CRDSLOT_NOTHAVE)       // 暂存仓无卡
                {
                    if ((nRet = m_devCRT730B.ClearCardSoltInfo(i + 1)) != IMP_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "更新暂存仓结构体信息: 暂存仓[%d]无卡，清除信息: ->ClearCardSoltInfo(%d) Fail, "
                            "ErrCode=%d, Return %s.",
                            i + 1, i + 1, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                        return ConvertErrorCode(nRet);
                    }
                } else
                if (nDevStat[i + 1] == IMPL_STAT_CRDSLOT_ISHAVE)       // 暂存仓有卡
                {
                    if (m_stCardSlotInfo.stSlots[i].bIsHave == FALSE)
                    {
                        CHAR szTimeCardNo[68+1] = { 0x00 };
                        if ((nRet = m_devCRT730B.ReadCardSlotInfo(i + 1, szTimeCardNo, 68)) != IMP_SUCCESS)
                        {
                            Log(ThisModule, __LINE__, "更新暂存仓结构体信息: 读暂存仓[%d]信息: ->ReadCardSlotInfo() Fail, "
                                                      "ErrCode=%d, Return %s.",
                                i + 1, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                            return ConvertErrorCode(nRet);
                        }
                        m_stCardSlotInfo.stSlots[i].bIsHave = TRUE;
                        memcpy(m_stCardSlotInfo.stSlots[i].szInTime, szTimeCardNo, 14);
                        memcpy(m_stCardSlotInfo.stSlots[i].szCardNo, szTimeCardNo + 14,
                               strlen(szTimeCardNo + 14));
                    }
                }

                sprintf(szSlotInfo + strlen(szSlotInfo), "%d:%d:%s:%s|",
                        i + 1, m_stCardSlotInfo.stSlots[i].bIsHave,
                        m_stCardSlotInfo.stSlots[i].szInTime,
                        m_stCardSlotInfo.stSlots[i].szCardNo);
            }
            Log(ThisModule, __LINE__, "更新暂存仓结构体信息: 更新信息=[%d|%s].",
                m_stCardSlotInfo.SlotHaveCount(), szSlotInfo);
        }
    }

    return CRM_SUCCESS;
}

// 设置数据
int CDevCRM_CRT730B::SetData(unsigned short usType, void *vData)
{
    switch(usType)
    {
        case SET_CLEAR_SLOT:    // 清除卡槽信息
        {
            if (vData != nullptr)
            {
                return m_devCRT730B.ClearCardSoltInfo(*((WORD*)vData));
            } else
            {
                return ERR_CRM_PARAM_ERR;
            }
        }
        case SET_DEV_RECON:    // 设置为断线重连执行状态
        {
            break;
        }
        case SET_DEV_OPENMODE:      // 设置设备打开模式
        {
            if (vData != nullptr)
            {
                memcpy(&(m_stOpenMode), ((LPSTDEVICEOPENMODE)vData), sizeof(STDEVICEOPENMODE));
                m_wDeviceType = (m_stOpenMode.nOtherParam[0] <= 0 ? m_stOpenMode.nOtherParam[0] : m_wDeviceType);
            }
            break;
        }
        default:
            break;
    }

    return CRM_SUCCESS;
}

// 获取数据
int CDevCRM_CRT730B::GetData(unsigned short usType, void *vData)
{
    switch(usType)
    {
        case CARDNO_ISEXIST:    // 检查卡号是否已存在
            {
                std::string stdCardNo = (CHAR*)vData;
                for (int i = 0; i < 5; i ++)
                {
                    if (m_stCardSlotInfo.stSlots[i].bIsHave == TRUE)
                    {
                        //if (stdCardNo.compare(m_stCardSlotInfo.stSlots[i].szCardNo) == 0)
                        // 支持 指定非完整卡号查询 START
                        int nLen = strlen(m_stCardSlotInfo.stSlots[i].szCardNo);
                        if (stdCardNo.length() > nLen)
                        {
                            continue;
                        }

                        if (stdCardNo.compare(m_stCardSlotInfo.stSlots[i].szCardNo +
                                              (nLen - stdCardNo.length())) == 0)
                        // 支持 指定非完整卡号查询 END
                        {
                            return ERR_CRM_CARD_ISEXIST;
                        }
                    }
                }
                return ERR_CRM_CARD_NOTFOUND;
            }
        case SLOTNO_ISEXIST:    // 检查暂存仓是否有卡/被占用
            {
                INT nSlotNo = atoi((CHAR*)vData);
                if (m_stCardSlotInfo.stSlots[nSlotNo - 1].bIsHave == TRUE)
                {
                    return ERR_CRM_SLOT_ISEXIST;
                } else
                {
                    return ERR_CRM_SLOT_NOTEXIST;
                }
            }
        default:
            break;
    }

    return CRM_SUCCESS;
}

// 获取版本号(1DevCRM版本/2固件版本/3设备软件版本/4其他)
int CDevCRM_CRT730B::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    CHAR    szVersion[128] = { 0x00 };

    if (usType == 2)
    {
        return ERR_CRM_OTHER;
    } else
    if (usType == 3)
    {
        if (m_devCRT730B.IsDeviceOpen() == TRUE)
        {
            if (m_devCRT730B.GetSoftVersion(szVersion) != IMP_SUCCESS)
            {
                return ERR_CRM_OTHER;
            }
        }
    } else
    if (usType == 4)
    {
        if (m_devCRT730B.IsDeviceOpen() == TRUE)
        {
            if (m_devCRT730B.GetDeviceSerialNumber(szVersion) != IMP_SUCCESS)
            {
                return ERR_CRM_OTHER;
            }
        }
    }

    memcpy(szVer, szVersion, strlen((char*)szVersion) > nSize ? nSize : strlen((char*)szVersion));
}

INT CDevCRM_CRT730B::ConvertErrorCode(long lRet)
{    
    ConvertImplErrCode2ErrDetail(lRet);

    switch(lRet)
    {
        // > 100: Impl处理返回
        case IMP_SUCCESS:                   // 成功->操作成功
            return CRM_SUCCESS;
        case IMP_ERR_LOAD_LIB:              // 动态库加载失败->动态库错误
            return ERR_CRM_LOAD_LIB;
        case IMP_ERR_PARAM_INVALID:         // 参数无效->参数错误
            return ERR_CRM_PARAM_ERR;
        case IMP_ERR_READERROR:             // 读数据错误->读数据错
            return ERR_CRM_READERROR;
        case IMP_ERR_WRITEERROR:            // 写数据错误->写数据错
            return ERR_CRM_WRITEERROR;
        case IMP_ERR_RCVDATA_INVALID:       // 无效的应答数据->读数据错
            return ERR_CRM_READERROR;
        case IMP_ERR_SNDDATA_INVALID:       // 无效的下发数据->写数据错
            return ERR_CRM_WRITEERROR;
        case IMP_ERR_STAT_POS_FAIL:         // 状态返回暂存仓位置异常->状态出错
        case IMP_ERR_STAT_INVALID:          // 状态返回无效值->状态出错
            return ERR_CRM_STATUS_ERR;
        case IMP_ERR_SLOTNO_INVALID:        // 指定暂存仓编号无效->参数错误
            return ERR_CRM_PARAM_ERR;
        case IMP_ERR_UNKNOWN:               // 未知错误->其他错误/未知错误
            return ERR_CRM_OTHER;


        // <0 : USB/COM接口处理返回
        case IMP_ERR_DEVPORT_NOTOPEN:       // (-1) 没打开->没有打开
            return ERR_CRM_NOT_OPEN;
        case IMP_ERR_DEVPORT_FAIL:          // (-2) 通讯错误->通讯错误
            return ERR_CRM_COMM_ERR;
        case IMP_ERR_DEVPORT_PARAM:         // (-3) 参数错误->参数错误
            return ERR_CRM_PARAM_ERR;
        case IMP_ERR_DEVPORT_CANCELED:      // (-4) 操作取消->用户取消
            return ERR_CRM_USER_CANCEL;
        case IMP_ERR_DEVPORT_READERR:       // (-5) 读取错误->读数据错
            return ERR_CRM_READERROR;
        case IMP_ERR_DEVPORT_WRITE:         // (-6) 发送错误->写数据错
            return ERR_CRM_WRITEERROR;
        case IMP_ERR_DEVPORT_RTIMEOUT:      // (-7) 操作超时->读数据错
            return ERR_CRM_READERROR;
        case IMP_ERR_DEVPORT_WTIMEOUT:      // (-8) 操作超时->写数据错
            return ERR_CRM_WRITEERROR;
        case IMP_ERR_DEVPORT_LIBRARY:       // (-98) 加载通讯库失败->动态库错误
            return ERR_CRM_LOAD_LIB;
        case IMP_ERR_DEVPORT_NODEFINED:     // (-99) 未知错误->其他错误/未知错误
            return ERR_CRM_OTHER;
        // 0~100: 硬件设备返回
        case IMP_ERR_DEVRET_00:             // 命令编码未定义->参数错误
        case IMP_ERR_DEVRET_01:             // 命令参数错误->参数错误
        case IMP_ERR_DEVRET_02:             // 命令数据错误->参数错误
            return ERR_CRM_PARAM_ERR;
        case IMP_ERR_DEVRET_03:             // 命令无法执行->通讯错误
            return ERR_CRM_COMM_ERR;
        case IMP_ERR_DEVRET_06:             // 维护门已经开启（此时不能发送动作指令!）->硬件故障
        case IMP_ERR_DEVRET_11:             // 电源电压过低(低于20V)->硬件故障
        case IMP_ERR_DEVRET_12:             // 电源电压过高(高于28V)->硬件故障
            return ERR_CRM_HWERR;
        case IMP_ERR_DEVRET_13:             // 电源恢复正常但没有重新初始化
        case IMP_ERR_DEVRET_16:             // 模块内部FRAM错误->硬件故障
            return ERR_CRM_HWERR;
        case IMP_ERR_DEVRET_19:             // 暂存仓传感器错误->硬件故障
        case IMP_ERR_DEVRET_20:             // 升降暂存仓阻塞->硬件故障
        case IMP_ERR_DEVRET_22:             // 进卡阻塞->硬件故障
            return ERR_CRM_HWERR;
        case IMP_ERR_DEVRET_23:             // 等待进卡超时->进卡超时
            return ERR_CRM_INSERT_TIMEOUT;
        case IMP_ERR_DEVRET_24:             // 进卡时卡被拔走->其他错误/未知错误
            return ERR_CRM_OTHER;
        case IMP_ERR_DEVRET_28:             // 退卡堵塞->堵卡
            return ERR_CRM_JAMMED;
        case IMP_ERR_DEVRET_29:             // 电机定位堵塞->硬件故障
        case IMP_ERR_DEVRET_30:             // 暂存仓未对准进卡口->硬件故障
            return ERR_CRM_HWERR;
        case IMP_ERR_DEVRET_31:             // 目标暂存仓有卡->指定卡已存在
            return ERR_CRM_CARD_ISEXIST;
        case IMP_ERR_DEVRET_32:             // 目标暂存仓无卡->指定卡不存在
            return ERR_CRM_CARD_NOTFOUND ;
        case IMP_ERR_DEVRET_35:             // 卡片信息未保存->硬件故障
            return ERR_CRM_HWERR;
        default:
            return ERR_CRM_OTHER;
    }
}

CHAR* CDevCRM_CRT730B::ConvertErrCodeToStr(long lRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(lRet)
    {
        case CRM_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", lRet, "操作成功");
            return m_szErrStr;
        case ERR_CRM_INSERT_TIMEOUT:
            sprintf(m_szErrStr, "%d|%s", lRet, "进卡超时");
            return m_szErrStr;
        case ERR_CRM_USER_CANCEL:
            sprintf(m_szErrStr, "%d|%s", lRet, "用户取消");
            return m_szErrStr;
        case ERR_CRM_COMM_ERR:
            sprintf(m_szErrStr, "%d|%s", lRet, "通讯错误");
            return m_szErrStr;
        case ERR_CRM_JAMMED:
            sprintf(m_szErrStr, "%d|%s", lRet, "堵卡");
            return m_szErrStr;
        case ERR_CRM_OFFLINE:
            sprintf(m_szErrStr, "%d|%s", lRet, "脱机");
            return m_szErrStr;
        case ERR_CRM_NOT_OPEN:
            sprintf(m_szErrStr, "%d|%s", lRet, "没有打开");
            return m_szErrStr;
        case ERR_CRM_SLOT_FULL:
            sprintf(m_szErrStr, "%d|%s", lRet, "卡箱满");
            return m_szErrStr;
        case ERR_CRM_HWERR:
            sprintf(m_szErrStr, "%d|%s", lRet, "硬件故障");
            return m_szErrStr;
        case ERR_CRM_STATUS_ERR:
            sprintf(m_szErrStr, "%d|%s", lRet, "状态出错");
            return m_szErrStr;
        case ERR_CRM_SLOT_ISEXIST:
            sprintf(m_szErrStr, "%d|%s", lRet, "指定卡箱被占用");
            return m_szErrStr;
        case ERR_CRM_SLOT_NOTEXIST:
            sprintf(m_szErrStr, "%d|%s", lRet, "指定卡箱没有被占用");
            return m_szErrStr;
        case ERR_CRM_UNSUP_CMD:
            sprintf(m_szErrStr, "%d|%s", lRet, "不支持的指令");
            return m_szErrStr;
        case ERR_CRM_PARAM_ERR:
            sprintf(m_szErrStr, "%d|%s", lRet, "参数错误");
            return m_szErrStr;
        case ERR_CRM_READTIMEOUT:
            sprintf(m_szErrStr, "%d|%s", lRet, "读数据超时");
            return m_szErrStr;
        case ERR_CRM_WRITETIMEOUT:
            sprintf(m_szErrStr, "%d|%s", lRet, "写数据超时");
            return m_szErrStr;
        case ERR_CRM_READERROR:
            sprintf(m_szErrStr, "%d|%s", lRet, "读数据错");
            return m_szErrStr;
        case ERR_CRM_WRITEERROR:
            sprintf(m_szErrStr, "%d|%s", lRet, "写数据错");
            return m_szErrStr;
        case ERR_CRM_CARD_NOTFOUND:
            sprintf(m_szErrStr, "%d|%s", lRet, "指定卡不存在");
            return m_szErrStr;
        case ERR_CRM_CARD_ISEXIST:
            sprintf(m_szErrStr, "%d|%s", lRet, "指定卡已存在");
            return m_szErrStr;
        case ERR_CRM_LOAD_LIB:
            sprintf(m_szErrStr, "%d|%s", lRet, "动态库错误");
            return m_szErrStr;
        //case ERR_CRM_OTHER:
        default:
            sprintf(m_szErrStr, "%d|%s", lRet, "其他错误/未知错误");
            return m_szErrStr;
    }
}

// 根据Impl错误码设置错误错误码字符串
INT CDevCRM_CRT730B::ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

#define CASE_SET_HW_DETAIL(IMP, DEVID) \
        case IMP: \
            m_clErrorDet.SetHWErrCodeInt(IMP, DEVID); break;

    switch(nRet)
    {
        // > 100: Impl处理返回
        //CASE_RET_DEVCODE(IMP_SUCCESS, IDC_SUCCESS)                        // 成功
        CASE_SET_DEV_DETAIL(IMP_ERR_LOAD_LIB, EC_ERR_LibraryLoadFail)       // 动态库加载失败
        CASE_SET_DEV_DETAIL(IMP_ERR_PARAM_INVALID, EC_ERR_ParInvalid)       // 参数无效
        CASE_SET_DEV_DETAIL(IMP_ERR_READERROR, EC_ERR_DataRWErr)            // 读数据错误
        CASE_SET_DEV_DETAIL(IMP_ERR_WRITEERROR, EC_ERR_DataRWErr)           // 写数据错误
        CASE_SET_DEV_DETAIL(IMP_ERR_RCVDATA_INVALID, EC_ERR_RecvData_inv)   // 无效的应答数据
        CASE_SET_DEV_DETAIL(IMP_ERR_STAT_POS_FAIL, EC_CRM_DEV_SlotPosAbnormal)// 暂存仓暂存仓位置异常
        CASE_SET_DEV_DETAIL(IMP_ERR_STAT_INVALID, EC_CRM_DEV_StatInvalid)   // 状态无效
        CASE_SET_DEV_DETAIL(IMP_ERR_SLOTNO_INVALID, EC_CRM_DEV_SlotNoInvalid)// 指定暂存仓编号无效
        CASE_SET_DEV_DETAIL(IMP_ERR_UNKNOWN, EC_ERR_UnKnownErr)             // 未知错误
        // <0 : USB/COM接口处理返回
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_NOTOPEN, EC_ERR_DevOpenFail)    // (-1) 没打开
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_FAIL, EC_ERR_ConnFail)          // (-2) 通讯错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_PARAM, EC_ERR_ParInvalid)       // (-3) 参数错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_READERR, EC_ERR_DataRWErr)      // (-5) 读取错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_WRITE, EC_ERR_DataRWErr)        // (-6) 发送错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_RTIMEOUT, EC_ERR_CommTimeOut)   // (-7) 操作超时
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_WTIMEOUT, EC_ERR_CommTimeOut)   // (-8) 操作超时
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_LIBRARY, EC_ERR_LibraryLoadFail)// (-98) 加载通讯库失败
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_NODEFINED, EC_ERR_UnKnownErr)   // (-99) 未知错误
        // 0~100: 硬件设备返回
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_00 - 1, m_wDeviceType)            // 命令编码未定义
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_01 - 1, m_wDeviceType)            // 命令参数错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_02 - 1, m_wDeviceType)            // 命令数据错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_03 - 1, m_wDeviceType)            // 命令无法执行
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_06 - 1, m_wDeviceType)            // 维护门已经开启（此时不能发送动作指令!）
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_11 - 1, m_wDeviceType)            // 电源电压过低(低于20V)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_12 - 1, m_wDeviceType)            // 电源电压过高(高于28V)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_13 - 1, m_wDeviceType)            // 电源恢复正常但没有重新初始化
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_16 - 1, m_wDeviceType)            // 模块内部FRAM错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_19 - 1, m_wDeviceType)            // 暂存仓传感器错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_20 - 1, m_wDeviceType)            // 升降暂存仓阻塞
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_22 - 1, m_wDeviceType)            // 进卡阻塞
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_23 - 1, m_wDeviceType)            // 等待进卡超时
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_24 - 1, m_wDeviceType)            // 进卡时卡被拔走
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_28 - 1, m_wDeviceType)            // 退卡堵塞
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_29 - 1, m_wDeviceType)            // 电机定位堵塞
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_30 - 1, m_wDeviceType)            // 暂存仓未对准进卡口
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_31 - 1, m_wDeviceType)            // 目标暂存仓有卡->指定卡已存在
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_32 - 1, m_wDeviceType)            // 目标暂存仓无卡->指定卡不存在
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_35 - 1, m_wDeviceType)            // 卡片信息未保存->硬件故障
    }

    return IDC_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////






