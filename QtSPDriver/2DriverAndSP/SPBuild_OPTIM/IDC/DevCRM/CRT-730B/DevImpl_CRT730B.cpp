/***************************************************************
* 文件名称：DevImpl_CRT780B.cpp
* 文件描述：CRT780B退卡模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/
#include "DevImpl_CRT730B.h"

static const char *ThisModule = "DevImpl_CRT730B.cpp";

CDevImpl_CRT730B::CDevImpl_CRT730B()
{
    SetLogFile(LOG_NAME, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CRT730B::CDevImpl_CRT730B(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_CRT730B::Init()
{
    m_strMode.clear();
    m_bDevOpenOk = FALSE;
    m_dwSndTimeOut = TIMEOUT_WAIT_ACTION;
    m_dwRcvTimeOut = TIMEOUT_WAIT_ACTION;
}

CDevImpl_CRT730B::~CDevImpl_CRT730B()
{

}


/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参　格式: USB:VID,PID  VID/PID为4位16进制字符串
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    if (m_pDev == nullptr)
    {
        if (m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "CRM", "CRT730B") != 0)
        {
            Log(ThisModule, __LINE__, "加载动态库(AllDevPort.dll) Fail, Return: %s",
                ConvertErrCodeToStr(IMP_ERR_LOAD_LIB));
            return IMP_ERR_LOAD_LIB;
        }
    }

    // 入参为null,使用缺省VID/PID
    if (lpMode == nullptr || strlen(lpMode) < 13)
    {
        Log(ThisModule, __LINE__, "入参为无效[%s], Return: %s.", lpMode,
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }
    m_strMode = lpMode;

    // 打开设备
    INT nRet = m_pDev->Open(m_strMode.c_str());
    if (nRet != ERR_DEVPORT_SUCCESS)
    {
        Log(ThisModule, __LINE__, "Open Device[%s] failed, ErrCode:%d, Return: %s.",
            m_strMode.c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    m_bDevOpenOk = TRUE;

    Log(ThisModule, __LINE__, "Open Device [%s] Success. ", m_strMode.c_str());

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    //LOGDEVACTION();

    if (m_bDevOpenOk == TRUE)
    {
        if (m_pDev == nullptr)
        {
            return IMP_SUCCESS;
        }
        m_pDev->Close();
        m_pDev.Release();
    }

    m_bDevOpenOk = FALSE;

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 释放动态库
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::Release()
{
    return DeviceClose();
}

BOOL CDevImpl_CRT730B::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/************************************************************
 * 功能: 1. 模块初始化
 * 参数: wInitMode 初始化方式(0/1/2/4)　参考宏定义
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::DeviceInit(WORD wMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // Check初始化参数
    if (wMode == MODE_INIT_NORMAL)      // 初始化:正常归位
    {
        memcpy(szSndCmd, SND_CMD_INIT_NORMAL, STAND_CMD_LENGHT);
    } else
    if (wMode == MODE_INIT_EJECT)       // 初始化:强行退卡
    {
        memcpy(szSndCmd, SND_CMD_INIT_EJECT, STAND_CMD_LENGHT);
    } else
    if (wMode == MODE_INIT_STORAGE)     // 初始化:强行暂存
    {
        memcpy(szSndCmd, SND_CMD_INIT_STORAGE, STAND_CMD_LENGHT);
    } else
    if (wMode == MODE_INIT_NOACTION)    // 初始化:无动作
    {
        memcpy(szSndCmd, SND_CMD_INIT_NOACTION, STAND_CMD_LENGHT);
    } else
    {
        Log(ThisModule, __LINE__, "入参为无效[%d], != 0/1/2/4, Return: %s.", wMode,
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;       // 无效入参
    }

    // 下发命令
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "模块初始化: SendCmd(%s, NULL, 0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "模块初始化: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(szSndCmd, szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 2. 获取设备状态
 * 参数: wMode 状态类别(1/2/4)　参考宏定义
 * 　　　nStat　返回状态数组,如下: 各状态参考宏定义
 *             [0]:设备状态; [1]:暂存仓1状态; [2]:暂存仓2状态;
 *             [3]:暂存仓3状态; [4]:暂存仓4状态; [5]:暂存仓5状态;
 *             [6]:维护门状态;7~11位保留 *
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::GetDeviceStat(WORD wMode, INT nStat[12])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    memset(nStat, -1, sizeof(nStat));

    if (wMode == MODE_STAT_ALL || wMode == MODE_STAT_DOOR)      // 设备+暂存仓+维护门|维护门
    {
        memcpy(szSndCmd, SND_CMD_STAT_DEVSLOTDOOR, STAND_CMD_LENGHT);
    } else
    if (wMode == MODE_STAT_SLOT)       // 暂存仓
    {
        memcpy(szSndCmd, SND_CMD_STAT_DEVSLOT, STAND_CMD_LENGHT);
    } else
    if (wMode == MODE_STAT_DEV)     // 设备
    {
        memcpy(szSndCmd, SND_CMD_STAT_DEVICE, STAND_CMD_LENGHT);
    } else
    {
        return IMP_ERR_PARAM_INVALID;       // 无效入参
    }

    // 下发命令
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "获取设备状态: SendCmd(%s, NULL, 0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "获取设备状态: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(szSndCmd, szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    // Check 应答状态码
    if (wMode == MODE_STAT_DOOR)      // 设备+暂存仓+维护门|维护门
    {
        nStat[6] = RcvDoorStatChg(szRcvData + 6);
    } else
    if (wMode == MODE_STAT_SLOT)       // 暂存仓
    {
        nStat[1] = RcvCardSlotStatChg(szRcvData + 6, 1);
        nStat[2] = RcvCardSlotStatChg(szRcvData + 6, 2);
        nStat[3] = RcvCardSlotStatChg(szRcvData + 6, 3);
        nStat[4] = RcvCardSlotStatChg(szRcvData + 6, 4);
        nStat[5] = RcvCardSlotStatChg(szRcvData + 6, 5);
    } else
    if (wMode == MODE_STAT_DEV)     // 设备
    {
        nStat[0] = RcvDeviceStatChg(szRcvData + 6);
    } else
    {
        nStat[0] = RcvDeviceStatChg(szRcvData + 6);
        nStat[1] = RcvCardSlotStatChg(szRcvData + 6, 1);
        nStat[2] = RcvCardSlotStatChg(szRcvData + 6, 2);
        nStat[3] = RcvCardSlotStatChg(szRcvData + 6, 3);
        nStat[4] = RcvCardSlotStatChg(szRcvData + 6, 4);
        nStat[5] = RcvCardSlotStatChg(szRcvData + 6, 5);
        nStat[6] = RcvDoorStatChg(szRcvData + 6);
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 3. 暂存仓移动
 * 参数: wMode 状态类别(0/1)　参考宏定义
 *      wSlotNo 暂存仓编号(1~5)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::CardSlotMove(WORD wMode, WORD wSlotNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szSndData[1024] = { 0x00 };
    INT     nSndDataLen = 0;
    CHAR    szRcvData[2068] = { 0x00 };

    if (wMode == MODE_SLOT_MOVEINIT)      // 回到初始位置
    {
        memcpy(szSndCmd, SND_CMD_SLOT_MOVEINIT, STAND_CMD_LENGHT);
    } else
    if (wMode == MODE_SLOT_MOVEPOS)       // 指定暂存仓对准卡口
    {
        if (wSlotNo < 1 || wSlotNo > 5)
        {
            Log(ThisModule, __LINE__, "暂存仓移动: Input SlotNo<%d> 无效, Return: %s.",
                wSlotNo, ConvertErrCodeToStr(IMP_ERR_SLOTNO_INVALID));
            return IMP_ERR_SLOTNO_INVALID;
        }
        memcpy(szSndCmd, SND_CMD_SLOT_MOVEPOS, STAND_CMD_LENGHT);
        sprintf(szSndData , "%d", wSlotNo);
        nSndDataLen = 1;
    } else
    {
        return IMP_ERR_PARAM_INVALID;       // 无效入参
    }

    // 下发命令
    nRet = SendCmd(szSndCmd, szSndData, nSndDataLen);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "暂存仓移动: SendCmd(%s, %s, %d)失败, Return: %s.",
            szSndCmd, szSndData, nSndDataLen, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "暂存仓移动: 命令<%s%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szSndData, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 4. 卡存入暂存仓
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::CardStorage()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_CARD_STORAGE, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "卡存入暂存仓: SendCmd(%s, NULL, 0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "卡存入暂存仓: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 5. 卡退出暂存仓
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::CardEject()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_CARD_EJECT, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "卡退出暂存仓: SendCmd(%s, NULL, 0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "卡退出暂存仓: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 6. 写指定暂存仓信息
 * 参数: wSlotNo　暂存仓编号(1~5)
 *      lpInfo 写入数据，最大68位
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::WriteCardSlotInfo(WORD wSlotNo, LPSTR lpInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szSndData[1024] = { 0x00 };
    INT     nSndDataLen = 0;
    CHAR    szRcvData[2068] = { 0x00 };
    CHAR    szInfoData[68+1] = { 0x00 };

    if (wSlotNo < 1 || wSlotNo > 5)
    {
        Log(ThisModule, __LINE__, "写指定暂存仓信息: Input SlotNo<%d> 无效, Return: %s.",
            wSlotNo, ConvertErrCodeToStr(IMP_ERR_SLOTNO_INVALID));
        return IMP_ERR_SLOTNO_INVALID;
    }

    if (lpInfo != nullptr)
    {
        WORD wLen = strlen(lpInfo);
        wLen = (wLen > 68 ? 68 : wLen);
        memcpy(szInfoData, lpInfo, wLen);
    }

    // 下发命令
    memcpy(szSndCmd, SND_CMD_SLOT_WRITEINFO, STAND_CMD_LENGHT);
    sprintf(szSndData, "%d%s", wSlotNo, szInfoData);
    nSndDataLen = strlen(szSndData);
    nRet = SendCmd(szSndCmd, szSndData, nSndDataLen);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "写指定暂存仓信息: SendCmd(%s, %s, %d)失败, Return: %s.",
            szSndCmd, szSndData, nSndDataLen, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "写指定暂存仓信息: 命令<%s%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szSndData, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 7. 读指定暂存仓信息
 * 参数: wSlotNo　暂存仓编号(1~5)
 *      lpInfo 返回数据，最大68位
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::ReadCardSlotInfo(WORD wSlotNo, LPSTR lpInfo, WORD wInfoSize)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szSndData[1024] = { 0x00 };
    INT     nSndDataLen = 0;
    CHAR    szRcvData[2068] = { 0x00 };

    if (wSlotNo < 1 || wSlotNo > 5)
    {
        Log(ThisModule, __LINE__, "读指定暂存仓信息: Input SlotNo<%d> 无效, Return: %s.",
            wSlotNo, ConvertErrCodeToStr(IMP_ERR_SLOTNO_INVALID));
        return IMP_ERR_SLOTNO_INVALID;
    }

    // 下发命令
    memcpy(szSndCmd, SND_CMD_SLOT_READINFO, STAND_CMD_LENGHT);
    sprintf(szSndData, "%d", wSlotNo);
    nSndDataLen = strlen(szSndData);
    nRet = SendCmd(szSndCmd, szSndData, nSndDataLen);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "读指定暂存仓信息: SendCmd(%s, %s, %d)失败, Return: %s.",
            szSndCmd, szSndData, nSndDataLen,  ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__,
            "读指定暂存仓信息: 命令<%s%s>->GetResponse(%s, %d, %d, %d)失败, Return: %d.",
            szSndCmd, szSndData, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    GETBUFF(lpInfo, wInfoSize, szRcvData, nRcvDataLen);

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 8. 清除暂存仓信息
 * 参数: wSlotNo　暂存仓编号(1~5)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::ClearCardSoltInfo(WORD wSlotNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szSndData[1024] = { 0x00 };
    INT     nSndDataLen = 0;
    CHAR    szRcvData[2068] = { 0x00 };

    if (wSlotNo < 0 || wSlotNo > 5)
    {
        Log(ThisModule, __LINE__, "清除暂存仓信息: Input SlotNo<%d> 无效, Return: %s.",
            wSlotNo, ConvertErrCodeToStr(IMP_ERR_SLOTNO_INVALID));
        return IMP_ERR_SLOTNO_INVALID;
    }

    // 下发命令
    if (wSlotNo == 0)
    {
        memcpy(szSndCmd, SND_CMD_SLOT_CLEARINFOALL, STAND_CMD_LENGHT);
    } else
    {
        memcpy(szSndCmd, SND_CMD_SLOT_CLEARINFO, STAND_CMD_LENGHT);
        sprintf(szSndData, "%d", wSlotNo);
        nSndDataLen = strlen(szSndData);
    }
    nRet = SendCmd(szSndCmd, szSndData, nSndDataLen);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "清除暂存仓信息: SendCmd(%s, %s, %d)失败, Return: %s.",
            szSndCmd, szSndData, nSndDataLen, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "清除暂存仓信息: 命令<%s%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szSndData, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 9. 获取软件版本信息
 * 参数: szVersion 返回数据，最大8位
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B:: GetSoftVersion(LPSTR lpVersion)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_SYSE_SOFTVER, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "获取软件版本信息: SendCmd(%s, NULL, %0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "获取软件版本信息: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    GETBUFF(lpVersion, 8, szRcvData, nRcvDataLen);

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 10. 获取设备能力信息
 * 参数: nDevCap 返回数据组，最大9位(0不支持/已禁用;1支持/未禁用;)
 *              [0]:暂存仓数; [1]:后部吞卡支持; [2]:保留; [3]:直接吞卡支持;
 * 　　　　　　　　[4]:外部按键清空支持;　[5]:暂存仓指示灯支持; [6]:错误显示支持;
 * 　　　　　　　　[7]:后部吞卡是否禁用; [8]:外部按键清空是否禁用;
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::GetDeviceCapab(INT nDevCap[9])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_SYSE_DEVCAPAB, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "获取设备能力信息: SendCmd(%s, NULL, 0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "获取设备能力信息: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    memset(nDevCap, 0, sizeof(nDevCap));
    nDevCap[0] = szRcvData[5];
    for(int i = 0; i < 8; i ++)
    {
        if((szRcvData[6] & (0x01 << i)) == 0x01)
            nDevCap[i + 1] = 1;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 11. 获取存卡计数
 * 参数: nCount 返回数据
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::GetCardStorageCount(INT &nCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    CHAR    szTmp[7+1] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_COUNT_STORAGE, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "获取存卡计数: SendCmd(%s, NULL, %0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "获取存卡计数: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    // 计数
    memcpy(szTmp, szRcvData + 5, 7);
    nCount = atoi(szTmp);

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 12. 获取退卡计数
 * 参数: nCount 返回数据
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::GetCardEjectCount(INT &nCount)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    CHAR    szTmp[7+1] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_COUNT_EJECT, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "获取退卡计数: SendCmd(%s, NULL, %0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "获取退卡计数: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisModule,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    // 计数
    memcpy(szTmp, szRcvData + 5, 7);
    nCount = atoi(szTmp);

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 13. 清除所有计数
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::ClearCardSoltCount()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_COUNT_CLEAR, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "清除所有计数: SendCmd(%s, NULL, %0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "清除所有计数: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 14. 获取设备序列号
 * 参数: szSerialNum　返回序列号(最大18位)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::GetDeviceSerialNumber(LPSTR lpSerialNum)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_GET_SERIALNUM, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "获取设备序列号: SendCmd(%s, NULL, %0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "获取设备序列号: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    GETBUFF(lpSerialNum, 18, szRcvData, nRcvDataLen);

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 15. 设备复位
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::DeviceReset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_DEV_RESET, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "设备复位: SendCmd(%s, NULL, %0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet < 0)
    {
        Log(ThisModule, __LINE__, "设备复位: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 16. 进入固件升级模式
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::DeviceUpdate()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_DEV_UPDATA, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "进入固件升级模式: SendCmd(%s, NULL, 0)失败, Return: %s.",
            szSndCmd, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "进入固件升级模式: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, Return: %s.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, nRcvDataLen,
            ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRcvDataLen);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 应答数据Check
 * 参数: lpcSndCmd 入参 下发命令
 *      lpcRcvData 入参 应答数据
 *      nRcvDataLen 入参 应答数据长度
 *                  回参 去掉CRC(2Byte)后的应答数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT &nRcvDataLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT     nErrCode = 0;
    CHAR    szErrCode[2+1] = { 0x00 };

    // 检查应答数据长度
    if (nRcvDataLen < 8)
    {
        Log(ThisModule, __LINE__, "CMD<%s> 返回命令应答数据长度[%d]<8, Return: %s.",
            lpcSndCmd, nRcvDataLen,  ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
        return IMP_ERR_RCVDATA_INVALID;
    }

    // 检查应答命令与发送命令是否一致
    if (memcmp(lpcSndCmd + 1, lpcRcvData + RCVCMDPOS + 1, STAND_CMD_LENGHT - 1) != 0)
    {
        Log(ThisModule, __LINE__, "CMD<%s> 返回无效的命令应答数据[%02X,%02X,%02X], 命令不一致, Return: %s.",
            lpcSndCmd, (int)(lpcRcvData[RCVCMDPOS]), (int)(lpcRcvData[RCVCMDPOS + 1]),
            (int)(lpcRcvData[RCVCMDPOS + 2]), ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
        return IMP_ERR_RCVDATA_INVALID;
    }

    // 检查CRC
    USHORT usCRC = 0x0000;
    usCRC = GetDataCRC((UCHAR*)(lpcRcvData), (USHORT)(nRcvDataLen - 2));// 计算CRC
    if (lpcRcvData[nRcvDataLen - 2] != ((usCRC >> 8) & 0xFF) ||
        lpcRcvData[nRcvDataLen - 1] != (usCRC & 0xFF))
    {
        Log(ThisModule, __LINE__, "CMD<%s> 返回无效的命令应答数据, CRC不一致, "
                                  "RcvData CRC = %02X,%02X, 计算CRC = %04X, Return: %s.",
            lpcSndCmd, (int)(lpcRcvData[nRcvDataLen - 2]), (int)(lpcRcvData[nRcvDataLen - 1]),
            usCRC, ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
        return IMP_ERR_RCVDATA_INVALID;
    }

    // 错误应答数据检查
    if (memcmp(lpcRcvData + RCVCMDPOS, "N", 1) == 0) // 错误应答
    {
        //CHAR    szErrCode[2+1] = { 0x00 };
        memcpy(szErrCode, lpcRcvData + RCVSTATPOS, 2);
        Log(ThisModule, __LINE__, "CMD<%s> 返回错误应答数据[CMD: %02X,%02X,%02X; STAT: %02X,%02X], "
                                  "Return %s.",
            CmdToStr((LPSTR)lpcSndCmd), (int)(lpcRcvData[RCVCMDPOS]), (int)(lpcRcvData[RCVCMDPOS + 1]),
            (int)(lpcRcvData[RCVCMDPOS + 2]), (int)(lpcRcvData[RCVSTATPOS]), (int)(lpcRcvData[RCVSTATPOS + 1]),
            ConvertErrCodeToStr(ConvertErrorCode(szErrCode)));
        return ConvertErrorCode(szErrCode);
    }

    // 返回应答数据长度(STX+LEN+CMD+DATA)
    nRcvDataLen = MAKEWORD(lpcRcvData[2], lpcRcvData[1]) + 3;

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 应答数据设备状态转换
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::RcvDeviceStatChg(LPCSTR lpStat)
{
    if (memcmp(lpStat, CRDSLOT_STAT_INIT, 2) == 0)
        return IMPL_STAT_CRDSLOT_INIT;
    else
    if (memcmp(lpStat, CRDSLOT_STAT_MOVE_TO_DOOR1, 2) == 0)
        return IMPL_STAT_CRDSLOT_MOVE_TO_DOOR1;
    else
    if (memcmp(lpStat, CRDSLOT_STAT_MOVE_TO_DOOR2, 2) == 0)
        return IMPL_STAT_CRDSLOT_MOVE_TO_DOOR2;
    else
    if (memcmp(lpStat, CRDSLOT_STAT_MOVE_TO_DOOR3, 2) == 0)
        return IMPL_STAT_CRDSLOT_MOVE_TO_DOOR3;
    else
    if (memcmp(lpStat, CRDSLOT_STAT_MOVE_TO_DOOR4, 2) == 0)
        return IMPL_STAT_CRDSLOT_MOVE_TO_DOOR4;
    else
    if (memcmp(lpStat, CRDSLOT_STAT_MOVE_TO_DOOR5, 2) == 0)
        return IMPL_STAT_CRDSLOT_MOVE_TO_DOOR5;
    else
    if (memcmp(lpStat, CRDSLOT_STAT_POSITION_FAIL, 2) == 0)
        return IMPL_STAT_CRDSLOT_POSITION_FAIL;
    else
        return IMPL_STAT_CRDSLOT_INVALID;
}

/************************************************************
 * 功能: 应答数据暂存仓状态转换
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::RcvCardSlotStatChg(LPCSTR lpStat, INT nSlotNo)
{
    if (lpStat[2 + nSlotNo - 1] == CRDSLOT_STAT_CARD_NOTHAVE) // 无卡
    {
        return IMPL_STAT_CRDSLOT_NOTHAVE;
    } else
    if (lpStat[2 + nSlotNo - 1] == CRDSLOT_STAT_CARD_ISHAVE) // 有卡
    {
        return IMPL_STAT_CRDSLOT_ISHAVE;
    } else
    {
        return IMPL_STAT_CRDSLOT_INVALID;
    }
}

/************************************************************
 * 功能: 应答数据维护门状态转换
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::RcvDoorStatChg(LPCSTR lpStat)
{
    if (lpStat[2 + 5] == SAFEDOOR_STAT_CLOSE) // 维护门关闭
    {
        return IMPL_STAT_SAFEDOOR_CLOSE;
    } else
    if (lpStat[2 + 5] == SAFEDOOR_STAT_OPEN) // 维护门开启
    {
        return IMPL_STAT_SAFEDOOR_OPEN;
    } else
    {
        return IMPL_STAT_SAFEDOOR_INVALID;
    }
}

/************************************************************
 * 功能: 下发数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::SendCmd(const char *pszCmd, const char *lpData, int nLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null, Device Not Open, Return: %s",
            ConvertErrCodeToStr(ERR_DEVPORT_NOTOPEN));
        return ERR_DEVPORT_NOTOPEN;
    }

    int nRet = 0;

    // 满足一帧下发
    if (nLen >= 0 && nLen <= CRT_PACK_MAX_CMP_LEN)
    {
        // one packet
        Log(ThisModule, __LINE__, "Send One Packet,Data Length is :%d", nLen);
        nRet = SendSinglePacket(pszCmd, lpData, nLen);
        if (nRet < 0)
        {
            return nRet;
        }

    } else
    // 分多帧下发
    if ((nLen > CRT_PACK_MAX_CMP_LEN) && (lpData != nullptr))
    {
        // Multi packet
        Log(ThisModule, __LINE__, "Send Multi Packet, Data Length is :%d", nLen);
        nRet = SendMultiPacket(pszCmd, lpData, nLen);
        if (nRet < 0)
        {
            return nRet;
        }
    }

    return nRet;
}

/************************************************************
 * 功能: 下发一帧数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::SendSinglePacket(const char* pszCmd, const char *lpData, int nLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    char buf[2068] = {0};
    int nRet = 0;
    BYTE dwReportID = {0};
    int nbufSize = 0;
    USHORT usCRC = 0x00;

    // 一帧数据 packet(无包头)
    // STX（1Byte)+DataLen(2Byte)+CMD(3Byte)+Data(<=56Byte)+CRC(2Byte)
    buf[0] = (char)STX;
    buf[1] = (char)((nLen + 3) / 256);  // 长度位计算(CMD+Data),高位
    buf[2] = (char)((nLen + 3) % 256);  // 长度位计算(CMD+Data),低位

    // + 命令
    memcpy(buf + 3, pszCmd, 3);

    // + Data
    if (lpData != nullptr)
    {
        memcpy(buf + 6, lpData, nLen);
    }

    nbufSize = nLen + 6;    // 帧数据总长度

    // 计算CRC
    usCRC = GetDataCRC((UCHAR*)(buf), (USHORT)(nbufSize));
    buf[nbufSize] = (usCRC >> 8) & 0xFF;    // 高位
    buf[nbufSize + 1] = usCRC & 0xFF;       // 低位
    nbufSize = nbufSize + 2;

    // 发送数据记录到Log
    CHAR szTmp[12] = { 0x00 };
    std::string stdSndData;
    for (int i = 0; i < nbufSize; i ++)
    {
        sprintf(szTmp, "%02X ", (int)buf[i]);
        stdSndData.append(szTmp);
    }
    Log(ThisModule, __LINE__, "命令下发: CMD<%s> %d|%s",
        CmdToStr((LPSTR)pszCmd), nbufSize, stdSndData.c_str());

    // 发送命令到设备，总计3次(如果设备未连接则先执行连接操作)
    for (int iIndex = 0; iIndex < 4; iIndex++)
    {
        nRet = m_pDev->Send(buf, nbufSize, m_dwSndTimeOut);

        if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
        {
            if (nRet == ERR_DEVPORT_WRITE)
            {
                Log(ThisModule, __LINE__, "Device没有响应, ErrCode=%s, Not Return.",
                    ConvertErrCodeToStr(nRet));
            } else
            {
                Log(ThisModule, __LINE__, "Send()出错，ErrCode=%s, Not Return.",
                    ConvertErrCodeToStr(nRet));
            }
        }
        m_nLastError = nRet;
        if (nRet < 0)
        {
            if (nRet == ERR_DEVPORT_NOTOPEN)
            {
                m_pDev->Close();
                int nRetTemp = m_pDev->Open(m_strMode.c_str());
                if ((nRetTemp == ERR_DEVPORT_SUCCESS) && (iIndex < 3))
                {
                    continue;
                }
            }
            Log(ThisModule, __LINE__, "Send()出错，ErrCode=%d, Return: %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        } else
        {
            break;
        }
    }

    return ConvertErrorCode(nRet);
}

/************************************************************
 * 功能: 下发多帧数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::SendMultiPacket(const char *pszCmd, const char *lpData, int nLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    char buf[2068] = {0};
    int nPackCount = 0;
    int nBufOffset = 0;     // 下发Buffer指针
    int nlpDataOffset = 0;  // 数据位置指针
    int nRet = 0;
    USHORT usCRC = 0x00;
    INT nPackSize = 0;

    // (无包头)Send总长度: STX（1Byte)+DataLen(2Byte)+CMD(3Byte)+DataLen+
    nPackSize = (6 + 2 + nLen);

    // 分包 发送的数据格式(无包头)
    // 第一帧: STX（1Byte)+DataLen(2Byte)+CMD(3Byte)+Data(<=58Byte)
    // 中间帧: Data(<=64Byte)
    // 最后帧: Data(<=64Byte)+CRC(2Byte)

    // 分帧数目: nPackSize + 2(CRC)
    nPackCount = ((nPackSize + 2) / (CRT_PACK_MAX_LEN - 1)) +
            (((nPackSize + 2) / (CRT_PACK_MAX_LEN - 1)) > 0 ? 1 : 0);

    // Multi packet
    buf[0] = (char)STX;
    buf[1] = (char)((nLen + 3) / 256);  // 长度位计算(CMD+Data),高位
    buf[2] = (char)((nLen + 3) % 256);  // 长度位计算(CMD+Data),低位位

    // + 命令
    memcpy(buf + 3, pszCmd, 3);

    // 根据分帧数据顺序下发
    for (int nWriteIdx = 0; nWriteIdx < nPackCount; nWriteIdx++)
    {
        // 发送命令到设备，总计3次(如果设备未连接则先执行连接操作)
        for (int iIndex = 0; iIndex < 4; iIndex++)
        {
            // 第一组帧数据
            if (nWriteIdx == 0)
            {
                // CRT_PACK_MAX_CMP_LEN = 56
                memcpy(buf + 6, lpData, CRT_PACK_MAX_CMP_LEN + 2);
                nBufOffset = 6 + CRT_PACK_MAX_CMP_LEN + 2;  // = 64
                nlpDataOffset = CRT_PACK_MAX_CMP_LEN + 2;   // = 58

                // 发送数据记录到Log
                CHAR szTmp[12] = { 0x00 };
                std::string stdSndData;
                for (int i = 0; i < nBufOffset - 1; i ++)
                {
                    sprintf(szTmp, "%02X ", (int)buf[i]);
                    stdSndData.append(szTmp);
                }
                Log(ThisModule, __LINE__, "命令下发: 第%d(%s)帧: CMD<%s>: 下发数据<%d|%s>",
                    nWriteIdx + 1, nPackCount, CmdToStr((LPSTR)pszCmd),
                    nBufOffset, stdSndData.c_str());

                // 计算CRC(保存值,最后一帧写入),CRC只算CMD+Data
                usCRC = GetDataCRC((UCHAR*)(buf), nBufOffset);

                // 下发到Device
                nRet = m_pDev->Send(buf, nBufOffset, m_dwSndTimeOut);
            } else
            if (nWriteIdx == nPackCount - 1)// 最后一组帧数据
            {
                INT nOverAgeLen = nLen - nlpDataOffset; // 剩余数据
                memcpy(buf + nBufOffset, lpData + nlpDataOffset, nOverAgeLen);

                // 计算CRC,写入最后两位
                usCRC = GetDataCRC((UCHAR*)(buf + nBufOffset), (USHORT)(nOverAgeLen), usCRC);
                buf[nBufOffset + nOverAgeLen] = (usCRC >> 8) & 0xFF;
                buf[nBufOffset + nOverAgeLen] = usCRC & 0xFF;

                // 发送数据记录到Log
                CHAR szTmp[12] = { 0x00 };
                std::string stdSndData;
                for (int i = 0; i < (nOverAgeLen + 2); i ++)
                {
                    sprintf(szTmp, "%02X ", (int)buf[nBufOffset + i]);
                    stdSndData.append(szTmp);
                }
                Log(ThisModule, __LINE__, "命令下发: 第%d(%s)帧: CMD<%s>: 数据<%d|%s>",
                    nWriteIdx + 1, nPackCount, CmdToStr((LPSTR)pszCmd),
                    nOverAgeLen + 2, stdSndData.c_str());

                // 下发到Device
                nRet = m_pDev->Send(buf + nBufOffset, nOverAgeLen + 2, m_dwSndTimeOut);
            } else  // 中间分组帧数据
            {
                // CRT_PACK_MAX_LEN = 65
                memcpy(buf + nBufOffset, lpData + nlpDataOffset, CRT_PACK_MAX_LEN - 1); // 每帧64Byte

                // 发送数据记录到Log
                CHAR szTmp[12] = { 0x00 };
                std::string stdSndData;
                for (int i = 0; i < CRT_PACK_MAX_LEN - 1; i ++)
                {
                    sprintf(szTmp, "%02X ", (int)buf[nBufOffset + i]);
                    stdSndData.append(szTmp);
                }
                Log(ThisModule, __LINE__, "命令下发: 第%d(%s)帧: CMD<%s>: 下发数据<%d|%s>",
                    nWriteIdx + 1, nPackCount, CmdToStr((LPSTR)pszCmd),
                    CRT_PACK_MAX_LEN - 1, stdSndData.c_str());

                // 计算CRC(保存值,最后一帧写入),CRC只算CMD+Data
                usCRC = GetDataCRC((UCHAR*)(buf + nBufOffset), CRT_PACK_MAX_LEN - 1, usCRC);

                nBufOffset = nBufOffset + CRT_PACK_MAX_LEN - 1;
                nlpDataOffset = nlpDataOffset + CRT_PACK_MAX_LEN - 1;

                // 下发到Device
                nRet = m_pDev->Send(buf + nBufOffset, CRT_PACK_MAX_LEN - 1, m_dwSndTimeOut);
            }

            if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
            {
                if (nRet == ERR_DEVPORT_WRITE)
                {
                    Log(ThisModule, __LINE__, "Device没有响应, ErrCode=%s, Not Return.",
                        ConvertErrCodeToStr(nRet));
                } else
                {
                    Log(ThisModule, __LINE__, "Send()出错，ErrCode=%s, Not Return.",
                        ConvertErrCodeToStr(nRet));
                }
            }
            m_nLastError = nRet;
            if (nRet < 0)
            {
                if (nRet == ERR_DEVPORT_NOTOPEN)
                {
                    m_pDev->Close();
                    int nRetTemp = m_pDev->Open(m_strMode.c_str());
                    if ((nRetTemp == ERR_DEVPORT_SUCCESS) && (iIndex < 3))
                    {
                        nWriteIdx = 0;
                        continue;
                    }
                }
                Log(ThisModule, __LINE__, "Send()出错，ErrCode=%d, Return: %s.",
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
            else
                break;
        }
    }

    return ConvertErrorCode(nRet);
}

/************************************************************
 * 功能: 下发一帧数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::SendSinglePacket0(const char* pszCmd, const char *lpData, int nLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    char buf[2068] = {0};
    int nRet = 0;
    BYTE dwReportID = {0};
    int nbufSize = 0;
    USHORT usCRC = 0x00;

    // 一帧数据 packet
    // 包头(1Byte)+STX（1Byte)+DataLen(2Byte)+CMD(3Byte)+Data(<=56Byte)+CRC(2Byte)
    dwReportID = REPORTID;
    buf[0] = (char)dwReportID;
    buf[1] = (char)STX;
    buf[2] = (char)((nLen + 3) / 256);  // 长度位计算(CMD+Data),高位
    buf[3] = (char)((nLen + 3) % 256);  // 长度位计算(CMD+Data),低位

    // + 命令
    memcpy(buf + 4, pszCmd, 3);

    // + Data
    if (lpData != nullptr)
    {
        memcpy(buf + 7, lpData, nLen);
    }

    nbufSize = nLen + 7;    // 帧数据总长度

    // 计算CRC
    usCRC = GetDataCRC((UCHAR*)(buf + 1), (USHORT)(nbufSize - 1));
    buf[nbufSize] = (usCRC >> 8) & 0xFF;    // 高位
    buf[nbufSize + 1] = usCRC & 0xFF;       // 低位
    nbufSize = nbufSize + 2;

    // 发送数据记录到Log
    CHAR szTmp[12] = { 0x00 };
    std::string stdSndData;
    for (int i = 0; i < nbufSize; i ++)
    {
        sprintf(szTmp, "%02X ", (int)buf[i]);
        stdSndData.append(szTmp);
    }
    Log(ThisModule, __LINE__, "SendData To Device: CMD<%s> %d|%s",
        CmdToStr((LPSTR)pszCmd), nbufSize, stdSndData.c_str());

    // 发送命令到设备，总计3次(如果设备未连接则先执行连接操作)
    for (int iIndex = 0; iIndex < 4; iIndex++)
    {
        nRet = m_pDev->Send(buf, nbufSize, TIMEOUT_WAIT_ACTION);

        if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
        {
            if (nRet == ERR_DEVPORT_WRITE)
            {
                Log(ThisModule, __LINE__, "Device没有响应, ErrCode=%s, Not Return.",
                    ConvertErrCodeToStr(nRet));
            } else
            {
                Log(ThisModule, __LINE__, "Send()出错，ErrCode=%s, Not Return.",
                    ConvertErrCodeToStr(nRet));
            }
        }
        m_nLastError = nRet;
        if (nRet < 0)
        {
            if (nRet == ERR_DEVPORT_NOTOPEN)
            {
                m_pDev->Close();
                int nRetTemp = m_pDev->Open(m_strMode.c_str());
                if ((nRetTemp == ERR_DEVPORT_SUCCESS) && (iIndex < 3))
                {
                    continue;
                }
            }
            Log(ThisModule, __LINE__, "Send()出错，ErrCode=%d, Return: %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        } else
        {
            break;
        }
    }

    return ConvertErrorCode(nRet);
}

/************************************************************
 * 功能: 下发多帧数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::SendMultiPacket0(const char *pszCmd, const char *lpData, int nLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    char buf[2068] = {0};
    int nPackCount = 0;
    int nbufOffset = 0;     // 下发Buffer指针
    int nlpDataOffset = 0;  // 数据位置指针
    BYTE dwReportID = {0};
    int nRet = 0;
    USHORT usCRC = 0x00;
    INT nPackSize = 0;

    // (无包头)Send总长度: STX（1Byte)+DataLen(2Byte)+CMD(3Byte)+DataLen
    nPackSize = (6 + 2 + nLen);

    // 分包 发送的数据格式
    // 第一帧: 包头(1Byte)+STX（1Byte)+DataLen(2Byte)+CMD(3Byte)+Data(<=58Byte)
    // 中间帧: 包头(1Byte)+Data(<=64Byte)
    // 最后帧: 包头(1Byte)+Data(<=64Byte)+CRC(2Byte)

    // 分帧数目
    nPackCount = (nPackSize / CRT_PACK_MAX_LEN) + ((nPackSize / CRT_PACK_MAX_LEN) > 0 ? 1 : 0);

    // Multi packet
    dwReportID = REPORTID;
    buf[0] = (char)dwReportID;          // 包头
    buf[1] = (char)STX;
    buf[2] = (char)((nLen + 3) / 256);  // 长度位计算(CMD+Data),高位
    buf[3] = (char)((nLen + 3) % 256);  // 长度位计算(CMD+Data),低位位

    // + 命令
    memcpy(buf + 4, pszCmd, 3);

    // 根据分帧数据顺序下发
    for (int nWriteIdx = 0; nWriteIdx < nPackCount; nWriteIdx++)
    {
        // 发送命令到设备，总计3次(如果设备未连接则先执行连接操作)
        for (int iIndex = 0; iIndex < 4; iIndex++)
        {
            // 第一组帧数据
            if (nWriteIdx == 0)
            {
                memcpy(buf + 7, lpData, CRT_PACK_MAX_CMP_LEN + 2);
                nbufOffset = CRT_PACK_MAX_LEN;
                nlpDataOffset = CRT_PACK_MAX_CMP_LEN + 2;

                // 发送数据记录到Log
                CHAR szTmp[12] = { 0x00 };
                std::string stdSndData;
                for (int i = 0; i < CRT_PACK_MAX_LEN; i ++)
                {
                    sprintf(szTmp, "%02X ", (int)buf[i]);
                    stdSndData.append(szTmp);
                }
                Log(ThisModule, __LINE__, "SendData <%d> To Device: CMD<%s> %d|%s",
                    nWriteIdx + 1, CmdToStr((LPSTR)pszCmd), CRT_PACK_MAX_LEN, stdSndData.c_str());

                // 计算CRC(保存值,最后一帧写入),CRC只算CMD+Data
                usCRC = GetDataCRC((UCHAR*)(buf + 1), CRT_PACK_MAX_LEN - 1);

                // 下发到Device
                nRet = m_pDev->Send(buf, CRT_PACK_MAX_LEN, TIMEOUT_WAIT_ACTION);
            } else
            if (nWriteIdx == nPackCount - 1)// 最后一组帧数据
            {
                int overageLen = nLen - nlpDataOffset;
                buf[nbufOffset] = (char)dwReportID;
                memcpy(buf + nbufOffset + 1, lpData + nlpDataOffset, overageLen);

                // 计算CRC,写入最后两位
                usCRC = GetDataCRC((UCHAR*)(buf + nbufOffset + 1), (USHORT)(overageLen), usCRC);
                buf[nbufOffset + 1 + overageLen] = (usCRC >> 8) & 0xFF;
                buf[nbufOffset + 1 + overageLen + 1] = usCRC & 0xFF;

                // 发送数据记录到Log
                CHAR szTmp[12] = { 0x00 };
                std::string stdSndData;
                for (int i = 0; i < (1 + overageLen + 2); i ++)
                {
                    sprintf(szTmp, "%02X ", (int)buf[i + nbufOffset]);
                    stdSndData.append(szTmp);
                }
                Log(ThisModule, __LINE__, "SendData <%d> To Device: CMD<%s> %d|%s",
                    nWriteIdx + 1, CmdToStr((LPSTR)pszCmd), 1 + overageLen + 2, stdSndData.c_str());

                // 下发到Device
                nRet = m_pDev->Send(buf + nbufOffset, 1 + overageLen + 2, TIMEOUT_WAIT_ACTION);
            } else  // 中间分组帧数据
            {
                buf[nbufOffset] = (char)dwReportID;     // 写包头
                memcpy(buf + nbufOffset + 1, lpData + nlpDataOffset, CRT_PACK_MAX_LEN);

                // 发送数据记录到Log
                CHAR szTmp[12] = { 0x00 };
                std::string stdSndData;
                for (int i = 0; i < CRT_PACK_MAX_LEN; i ++)
                {
                    sprintf(szTmp, "%02X ", (int)buf[nbufOffset + i]);
                    stdSndData.append(szTmp);
                }
                Log(ThisModule, __LINE__, "SendData <%d> To Device: CMD<%s> %d|%s",
                    nWriteIdx + 1, CmdToStr((LPSTR)pszCmd), CRT_PACK_MAX_LEN, stdSndData.c_str());

                // 计算CRC(保存值,最后一帧写入),CRC只算CMD+Data
                usCRC = GetDataCRC((UCHAR*)(buf + nbufOffset + 1), CRT_PACK_MAX_LEN - 1, usCRC);

                nbufOffset = nbufOffset + CRT_PACK_MAX_LEN;
                nlpDataOffset = nbufOffset + CRT_PACK_MAX_LEN - 1;

                // 下发到Device
                nRet = m_pDev->Send(buf + nbufOffset, CRT_PACK_MAX_LEN, TIMEOUT_WAIT_ACTION);
            }

            if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
            {
                if (nRet == ERR_DEVPORT_WRITE)
                {
                    Log(ThisModule, __LINE__, "Device没有响应, ErrCode=%s, Not Return.",
                        ConvertErrCodeToStr(nRet));
                } else
                {
                    Log(ThisModule, __LINE__, "Send()出错，ErrCode=%s, Not Return.",
                        ConvertErrCodeToStr(nRet));
                }
            }
            m_nLastError = nRet;
            if (nRet < 0)
            {
                if (nRet == ERR_DEVPORT_NOTOPEN)
                {
                    m_pDev->Close();
                    int nRetTemp = m_pDev->Open(m_strMode.c_str());
                    if ((nRetTemp == ERR_DEVPORT_SUCCESS) && (iIndex < 3))
                    {
                        nWriteIdx = 0;
                        continue;
                    }
                }
                Log(ThisModule, __LINE__, "Send()出错，ErrCode=%d, Return: %s.",
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
            else
                break;
        }
    }

    return ConvertErrorCode(nRet);
}

/************************************************************
 * 功能：读取读卡器的返回数据
 * 参数：pszReponse返回数据的缓冲区
 *      nLen缓冲区长度
 *      nTimeout超时(毫秒)
 *      nOutLen 返回的赢大数据长度
 * 返回：参考错误码
************************************************************/
INT CDevImpl_CRT730B::GetResponse(char *pszResponse, int nLen, int nTimeout, INT &nOutLen)
{
    Q_UNUSED(nLen)

    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    char pszReply[64];
    int nRet = 0;
    int nIndex = 0;
    INT nTimeOut = m_dwRcvTimeOut;

    if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null, Device Not Open, Return: %s",
            ConvertErrCodeToStr(ERR_DEVPORT_NOTOPEN));
        return ERR_DEVPORT_NOTOPEN;
    }

    while(TRUE)
    {
        DWORD ulInOutLen = USB_READ_LENTH;
        memset(pszReply, 0, sizeof(pszReply));
        nRet = m_pDev->Read(pszReply, ulInOutLen, nTimeOut); // 返回值为数据长度
        if (nRet != ERR_DEVPORT_SUCCESS || ulInOutLen < 1)
        {
            break;
        } else
        {
            // 接收数据记录到Log
            CHAR szTmp[12] = { 0x00 };
            std::string stdRcvData;
            for (int i = 0; i < ulInOutLen; i ++)
            {
                sprintf(szTmp, "%02X ", (int)pszReply[i]);
                stdRcvData.append(szTmp);
            }
            // 单一 确认/否认 命令丢弃
            if (pszReply[0] == ACK)
            {
                Log(ThisModule, __LINE__, "RecvData To Device: %d|%s\n"
                                          "RecvData Byte[0] == %02X|ACK(确认下发CMD有效), 不做处理.",
                    ulInOutLen, stdRcvData.c_str(), ACK);
                continue;
            } else
            if (pszReply[0] == NAK)
            {
                Log(ThisModule, __LINE__, "RecvData To Device: %d|%s\n"
                                          "RecvData Byte[0] == %02X|NAK(否认,下发命令无效), Return: %s.",
                    ulInOutLen, stdRcvData.c_str(), ConvertErrCodeToStr(IMP_ERR_SNDDATA_INVALID));
                return IMP_ERR_SNDDATA_INVALID; // 无效下发数据
            } else
            {
                Log(ThisModule, __LINE__, "RecvData To Device: %d|%s.",
                    ulInOutLen, stdRcvData.c_str());
            }

            if (nIndex == 0)
            {
               // 第一个包
               nIndex = ulInOutLen;
               memcpy(pszResponse, pszReply, nIndex);
               nTimeOut = 50;
            } else
            {
                // 多个包
                memcpy(pszResponse + nIndex, pszReply, ulInOutLen);
                nIndex += ulInOutLen; // 接收应答计数
            }
            continue;
        }
    }

    // Check接收的应答数据完整性
    // 应答数据: STX（1Byte)+LEN(2Byte)+"P/N"(1Byte)+CMD(2Byte)+Stat(2Byte)+Data(XXByte)+CRC(2Byte)
    // LEN = "P/N"(1Byte)+CMD(2Byte)+Stat(2Byte)+Data(XXByte) Is Length
    DWORD nRecvLen = MAKEWORD(pszResponse[2], pszResponse[1]);  // [1]高位+[2]低位=数据长度
    if (nRecvLen > 0)
    {
        // LEN + 3(STX+LEN) + 2(CRC) = 接收有效应答
        nOutLen = (nRecvLen + 3 + 2);
    } else
    {
        Log(ThisModule, __LINE__, "Device返回数据错误, RcvDataLen = %02X|%02X = %d, Return: %s.",
            pszResponse[1], pszResponse[2], nRecvLen, ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
        return IMP_ERR_RCVDATA_INVALID; // 应答数据无效
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能：计算数据CRC
 * 参数：ucData 数据
 *      usDataSize 数据长度
 *      usInitCrc 初始CRC,缺省0x00
 * 返回：CRC
************************************************************/
USHORT CDevImpl_CRT730B::GetDataCRC(UCHAR *ucData, USHORT usDataSize, USHORT usInitCrc)
{
    UCHAR   ucCh;
    USHORT  i;
    USHORT  usCrc = usInitCrc;

    for (i = 0; i < usDataSize; i ++)
    {
        ucCh = *ucData++;
        usCrc = Calc_CRC(usCrc, (USHORT)ucCh);
    }

    return usCrc;
}

USHORT CDevImpl_CRT730B::Calc_CRC(USHORT usCrc, USHORT usCh)
{
    USHORT  i;
    usCh <<= 8;
    for (i = 8; i > 0; i--)
    {
        if ((usCh ^ usCrc) & 0x8000)
        {
            usCrc = (usCrc << 1) ^ POLINOMIAL;
        } else
        {
            usCrc <<= 1;
        }
        usCh <<= 1;
    }

    return usCrc;
}

INT CDevImpl_CRT730B::ConvertErrorCode(long lRet)
{
    switch (lRet)
    {
        case ERR_DEVPORT_SUCCESS    : return IMP_SUCCESS;
        case ERR_DEVPORT_NOTOPEN    : return IMP_ERR_DEVPORT_NOTOPEN;        // (-1) 没打开
        case ERR_DEVPORT_FAIL       : return IMP_ERR_DEVPORT_FAIL;           // (-2) 通讯错误
        case ERR_DEVPORT_PARAM      : return IMP_ERR_DEVPORT_PARAM;          // (-3) 参数错误
        case ERR_DEVPORT_CANCELED   : return IMP_ERR_DEVPORT_CANCELED;       // (-4) 操作取消
        case ERR_DEVPORT_READERR    : return IMP_ERR_DEVPORT_READERR;        // (-5) 读取错误
        case ERR_DEVPORT_WRITE      : return IMP_ERR_DEVPORT_WRITE;          // (-6) 发送错误
        case ERR_DEVPORT_RTIMEOUT   : return IMP_ERR_DEVPORT_RTIMEOUT;       // (-7) 操作超时
        case ERR_DEVPORT_WTIMEOUT   : return IMP_ERR_DEVPORT_WTIMEOUT;       // (-8) 操作超时
        case ERR_DEVPORT_LIBRARY    : return IMP_ERR_DEVPORT_LIBRARY;        // (-98) 加载通讯库失败
        case ERR_DEVPORT_NODEFINED  : return IMP_ERR_DEVPORT_NODEFINED;      // (-99) 未知错误
    }
}

INT CDevImpl_CRT730B::ConvertErrorCode(CHAR szDeviceErr[3])
{
    if (memcmp(szDeviceErr, DEV_ERR_CMD_INVALID, 2) == 0) // 命令编码未定义
    {
        return IMP_ERR_DEVRET_CMD_INVALID;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_CMD_PARAM, 2) == 0) // 命令参数错误
    {
        return IMP_ERR_DEVRET_CMD_PARAM;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_CMD_DATA, 2) == 0) // 命令数据错误
    {
        return IMP_ERR_DEVRET_CMD_DATA;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_CMD_UNABLE, 2) == 0) // 命令无法执行
    {
        return IMP_ERR_DEVRET_CMD_UNABLE;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_DOOR_ISOPEN, 2) == 0)// 维护门已经开启（此时不能发送动作指令!）
    {
        return IMP_ERR_DEVRET_DOOR_ISOPEN;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_POWER_VOLLOW, 2) == 0) // 电源电压过低(低于20V)
    {
        return IMP_ERR_DEVRET_POWER_VOLLOW;
    }
    if (memcmp(szDeviceErr, DEV_ERR_POWER_VOLHIGH, 2) == 0)// 电源电压过高(高于28V)
    {
        return IMP_ERR_DEVRET_POWER_VOLHIGH;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_POWER_NEEDINIT, 2) == 0)// 电源恢复正常但没有重新初始化
    {
        return IMP_ERR_DEVRET_POWER_NEEDINIT;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_POWER_FRAM, 2) == 0)// 模块内部FRAM错误
    {
        return IMP_ERR_DEVRET_POWER_FRAM;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_SOLT_SENSER, 2) == 0)// 暂存仓传感器错误
    {
        return IMP_ERR_DEVRET_SOLT_SENSER;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_SLOT_JAM, 2) == 0)// 升降暂存仓阻塞
    {
        return IMP_ERR_DEVRET_SLOT_JAM;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_INCARD_JAM, 2) == 0)// 进卡阻塞
    {
        return IMP_ERR_DEVRET_INCARD_JAM;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_INCARD_TIMEOUT, 2) == 0)// 等待进卡超时
    {
        return IMP_ERR_DEVRET_INCARD_TIMEOUT;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_INCARD_TAKE, 2) == 0)// 进卡时卡被拔走
    {
        return IMP_ERR_DEVRET_INCARD_TAKE;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_EJECT_JAM, 2) == 0)// 退卡堵塞
    {
        return IMP_ERR_DEVRET_EJECT_JAM;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_POWER_LOCAT_JAM, 2) == 0)// 电机定位堵塞
    {
        return IMP_ERR_DEVRET_POWER_LOCAT_JAM;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_SLOT_NOTAIM_CK, 2) == 0)// 暂存仓未对准进卡口
    {
        return IMP_ERR_DEVRET_SLOT_NOTAIM_CK;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_SLOT_HAVECARD, 2) == 0)// 目标暂存仓有卡
    {
        return IMP_ERR_DEVRET_SLOT_HAVECARD;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_SLOT_NOTHAVE, 2) == 0)// 目标暂存仓无卡
    {
        return IMP_ERR_DEVRET_SLOT_NOTHAVE;
    } else
    if (memcmp(szDeviceErr, DEV_ERR_CINFO_NOSAVE, 2) == 0)// 卡片信息未保存
    {
        return IMP_ERR_DEVRET_CINFO_NOSAVE;
    } else
    {
        return IMP_ERR_UNKNOWN;     // 未知错误
    }

    return IMP_SUCCESS;
}

CHAR* CDevImpl_CRT730B::ConvertErrCodeToStr(long lRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(lRet)
    {
    case IMP_SUCCESS:
        sprintf(m_szErrStr, "%d|%s", lRet, "成功");
        return m_szErrStr;
    case IMP_ERR_LOAD_LIB:
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:动态库加载失败");
        return m_szErrStr;
    case IMP_ERR_PARAM_INVALID:
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:参数无效");
        return m_szErrStr;
    case IMP_ERR_READERROR:
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:读数据错误");
        return m_szErrStr;
    case IMP_ERR_WRITEERROR:
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:写数据错误");
        return m_szErrStr;
    case IMP_ERR_RCVDATA_INVALID :
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:无效的应答数据");
        return m_szErrStr;
    case IMP_ERR_STAT_POS_FAIL   :
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:状态返回暂存仓位置异常");
        return m_szErrStr;
    case IMP_ERR_STAT_INVALID:
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:状态返回无效值");
        return m_szErrStr;
    case IMP_ERR_SLOTNO_INVALID:
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:指定暂存仓编号无效");
        return m_szErrStr;
    case IMP_ERR_UNKNOWN:
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:未知错误");
        return m_szErrStr;
    case IMP_ERR_SNDDATA_INVALID:
        sprintf(m_szErrStr, "%d|%s", lRet, "IMPL:无效的下发数据");
        return m_szErrStr;

    // <0 : USB/COM接口处理返回
    case IMP_ERR_DEVPORT_NOTOPEN:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:没打开");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_FAIL:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:通讯错误");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_PARAM:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:参数错误");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_CANCELED:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:操作取消");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_READERR:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:读取错误");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_WRITE:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:发送错误");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_RTIMEOUT:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:操作超时");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_WTIMEOUT:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:操作超时");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_LIBRARY:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:加载通讯库失败");
        return m_szErrStr;
    case IMP_ERR_DEVPORT_NODEFINED:
        sprintf(m_szErrStr, "%d|%s", lRet, "DevPort:未知错误");
        return m_szErrStr;

    // 0~100: 硬件设备返回
    case IMP_ERR_DEVRET_CMD_INVALID:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:命令编码未定义");
        return m_szErrStr;
    case IMP_ERR_DEVRET_CMD_PARAM:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:命令参数错误");
        return m_szErrStr;
    case IMP_ERR_DEVRET_CMD_DATA:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:命令数据错误");
        return m_szErrStr;
    case IMP_ERR_DEVRET_CMD_UNABLE:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:命令无法执行");
        return m_szErrStr;
    case IMP_ERR_DEVRET_DOOR_ISOPEN:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:维护门已经开启（此时不能发送动作指令!）");
        return m_szErrStr;
    case IMP_ERR_DEVRET_POWER_VOLLOW:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:电源电压过低(低于20V)");
        return m_szErrStr;
    case IMP_ERR_DEVRET_POWER_VOLHIGH:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:电源电压过高(高于28V)");
        return m_szErrStr;
    case IMP_ERR_DEVRET_POWER_NEEDINIT:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:电源恢复正常但没有重新初始化");
        return m_szErrStr;
    case IMP_ERR_DEVRET_POWER_FRAM:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:模块内部FRAM错误");
        return m_szErrStr;
    case IMP_ERR_DEVRET_SOLT_SENSER:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:暂存仓传感器错误");
        return m_szErrStr;
    case IMP_ERR_DEVRET_SLOT_JAM:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:升降暂存仓阻塞");
        return m_szErrStr;
    case IMP_ERR_DEVRET_INCARD_JAM:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:进卡阻塞");
        return m_szErrStr;
    case IMP_ERR_DEVRET_INCARD_TIMEOUT:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:等待进卡超时");
        return m_szErrStr;
    case IMP_ERR_DEVRET_INCARD_TAKE:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:进卡时卡被拔走");
        return m_szErrStr;
    case IMP_ERR_DEVRET_EJECT_JAM:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:退卡堵塞");
        return m_szErrStr;
    case IMP_ERR_DEVRET_POWER_LOCAT_JAM:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:电机定位堵塞");
        return m_szErrStr;
    case IMP_ERR_DEVRET_SLOT_NOTAIM_CK:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:暂存仓未对准进卡口");
        return m_szErrStr;
    case IMP_ERR_DEVRET_SLOT_HAVECARD:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:目标暂存仓有卡");
        return m_szErrStr;
    case IMP_ERR_DEVRET_SLOT_NOTHAVE:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:目标暂存仓无卡");
        return m_szErrStr;
    case IMP_ERR_DEVRET_CINFO_NOSAVE:
        sprintf(m_szErrStr, "%d|%s", lRet, "DEV:卡片信息未保存");
        return m_szErrStr;
    }
}

CHAR* CDevImpl_CRT730B::CmdToStr(LPSTR lpCmd)
{
    memset(m_szCmdStr, 0x00, sizeof(m_szCmdStr));

    if (strcmp(lpCmd, SND_CMD_INIT_NORMAL) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C00|初始化:正常归位");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_INIT_EJECT) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C01|初始化:强行退卡");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_INIT_STORAGE) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C02|初始化:强行暂存");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_INIT_NOACTION) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C04|初始化:无动作");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_STAT_DEVICE) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C10|状态查询:模块");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_STAT_DEVSLOT) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C11|状态查询:模块+暂存仓");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_STAT_DEVSLOTDOOR) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C14|状态查询:模块+暂存仓+维护门");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_SLOT_MOVEINIT) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C20|暂存仓移动:回到初始位置");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_SLOT_MOVEPOS) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C21|暂存仓移动:指定暂存仓对准卡口");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_CARD_STORAGE) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C30|卡进入暂存仓");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_CARD_EJECT) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C31|卡退出暂存仓");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_SLOT_WRITEINFO) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C50|写暂存仓信息");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_SLOT_READINFO) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C51|读暂存仓信息");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_SLOT_CLEARINFO) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C52|清除指定暂存仓信息");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_SLOT_CLEARINFOALL) == 0)
    {
        sprintf(m_szCmdStr, "%s", "C53|清除所有暂存仓信息");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_SYSE_SOFTVER) == 0)
    {
        sprintf(m_szCmdStr, "%s", "CR0|获取软件版本");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_SYSE_DEVCAPAB) == 0)
    {
        sprintf(m_szCmdStr, "%s", "CR1|获取设备能力值");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_COUNT_STORAGE) == 0)
    {
        sprintf(m_szCmdStr, "%s", "CC0|获取存入计数");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_COUNT_EJECT) == 0)
    {
        sprintf(m_szCmdStr, "%s", "CC1|获取退卡计数");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_COUNT_CLEAR) == 0)
    {
        sprintf(m_szCmdStr, "%s", "CC9|清除所有计数");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_GET_SERIALNUM) == 0)
    {
        sprintf(m_szCmdStr, "%s", "CS0|获取序列号");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_DEV_RESET) == 0)
    {
        sprintf(m_szCmdStr, "%s", "CK0|设备复位");
        return m_szCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_DEV_UPDATA) == 0)
    {
        sprintf(m_szCmdStr, "%s", "CK1|进入固件升级模式");
        return m_szCmdStr;
    } else
    {
        sprintf(m_szCmdStr, "%s|未知", lpCmd);
        return m_szCmdStr;
    }
}

//---------------------------------------------
