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

static const char *ThisFile = "DevImpl_CRT730B.cpp";

CDevImpl_CRT730B::CDevImpl_CRT730B()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CRT730B::CDevImpl_CRT730B(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_CRT730B::Init()
{
    m_bDevOpenOk = FALSE;
    //memset(ScannerInfo, 0x00, sizeof(ScannerInfo));
    //memset(&m_stOldDevStatus, 0x00, sizeof(DEVSTATUS));
    //memset(&m_stNewDevStatus, 0x00, sizeof(DEVSTATUS));

}

CDevImpl_CRT730B::~CDevImpl_CRT730B()
{
    //vUnLoadLibrary();
}


/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参　格式: USB:VID,PID  VID/PID为4位16进制字符串
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::OpenDevice(LPSTR lpMode)
{
    if (m_pDev == nullptr)
    {
        if (m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "CRM", "DevIDC") != 0)
        {
            Log(ThisFile, __LINE__, "Load(AllDevPort.dll) failed.");
            return IMP_ERR_LOAD_LIB;
        }
    }

    // 入参为null,使用缺省VID/PID
    if (lpMode == nullptr || strlen(lpMode) < 13)
    {
        if (m_strMode.empty())
        {
            char szDevID[MAX_EXT] = {0};
            sprintf(szDevID, "USB:%d,%d", DEV_VID_DEF, DEV_PID_DEF);
            m_strMode = szDevID;
            Log(ThisFile, __LINE__, "入参为null,使用缺省VID/PID[%s]", m_strMode.c_str());
        }
    }
    else
    {
        m_strMode = lpMode;
        Log(ThisFile, __LINE__, "入参为[%s]", m_strMode.c_str());
    }

    // 打开设备
    long iRet = m_pDev->Open(m_strMode.c_str());
    if (iRet < 0)
    {
        Log(ThisFile, __LINE__, "Open dev failed, ReturnCode:%d, ChangeCode:%d ",
            iRet, ConvertErrorCode(iRet));
        return ConvertErrorCode(iRet);
    }

    Log(ThisFile, 1, "Open dev [%s] success ", m_strMode.c_str());

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::CloseDevice()
{
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
    return CloseDevice();
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
    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

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
        return IMP_ERR_PARAM_INVALID;       // 无效入参
    }

    // 下发命令
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "模块初始化: SendCmd(%s, NULL, 0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "模块初始化: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(szSndCmd, szRcvData, nRet);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    // Check 应答状态码
    switch(RcvDeviceStatChg(szRcvData + 3))
    {
        case IMPL_STAT_CRDSLOT_POSITION_FAIL:
            Log(ThisFile, __LINE__, "模块初始化: 命令<%s>->GetResponse()状态为[%02X,%02X], Return %d.",
                szSndCmd, (INT)(szRcvData[3]), (INT)(szRcvData[4]), IMP_ERR_STAT_POS_FAIL);
            return IMP_ERR_STAT_POS_FAIL;
        case IMPL_STAT_CRDSLOT_INVALID :
            Log(ThisFile, __LINE__, "模块初始化: 命令<%s>->GetResponse()状态为[%02X,%02X], Return %d.",
                szSndCmd, (INT)(szRcvData[3]), (INT)(szRcvData[4]), IMP_ERR_STAT_INVALID);
            return IMP_ERR_STAT_INVALID;
        delault: return IMP_SUCCESS;
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
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "获取设备状态: SendCmd(%s, NULL, 0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "获取设备状态: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(szSndCmd, szRcvData, nRet);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    // Check 应答状态码
    if (wMode == MODE_STAT_DOOR)      // 设备+暂存仓+维护门|维护门
    {
        nStat[6] = RcvDoorStatChg(szRcvData + 3);
    } else
    if (wMode == MODE_STAT_SLOT)       // 暂存仓
    {
        nStat[1] = RcvCardSlotStatChg(szRcvData + 3, 1);
        nStat[2] = RcvCardSlotStatChg(szRcvData + 3, 2);
        nStat[3] = RcvCardSlotStatChg(szRcvData + 3, 3);
        nStat[4] = RcvCardSlotStatChg(szRcvData + 3, 4);
        nStat[5] = RcvCardSlotStatChg(szRcvData + 3, 5);
    } else
    if (wMode == MODE_STAT_DEV)     // 设备
    {
        nStat[0] = RcvDeviceStatChg(szRcvData + 3);
    } else
    {
        nStat[0] = RcvDeviceStatChg(szRcvData + 3);
        nStat[1] = RcvCardSlotStatChg(szRcvData + 3, 1);
        nStat[2] = RcvCardSlotStatChg(szRcvData + 3, 2);
        nStat[3] = RcvCardSlotStatChg(szRcvData + 3, 3);
        nStat[4] = RcvCardSlotStatChg(szRcvData + 3, 4);
        nStat[5] = RcvCardSlotStatChg(szRcvData + 3, 5);
        nStat[6] = RcvDoorStatChg(szRcvData + 3);
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
            Log(ThisFile, __LINE__, "暂存仓移动: Input SlotNo<%d> 无效, Return %d.",
                wSlotNo, IMP_ERR_SLOTNO_INVALID);
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
    nRet = SendCmd(szSndCmd, szSndData, nSndDataLen, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "暂存仓移动: SendCmd(%s, %s, %d, %s)失败, Return %d.",
            szSndCmd, szSndData, nSndDataLen, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "暂存仓移动: 命令<%s%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szSndData, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
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
    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_CARD_STORAGE, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "卡存入暂存仓: SendCmd(%s, NULL, 0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "卡存入暂存仓: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
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
    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_CARD_EJECT, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "卡退出暂存仓: SendCmd(%s, NULL, 0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "卡退出暂存仓: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 6. 写指定暂存仓信息
 * 参数: wSlotNo　暂存仓编号(1~5)
 *      szInfo 写入数据，最大68位
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::WriteCardSlotInfo(WORD wSlotNo, CHAR szInfo[68+1])
{
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szSndData[1024] = { 0x00 };
    INT     nSndDataLen = 0;
    CHAR    szRcvData[2068] = { 0x00 };

    if (wSlotNo < 1 || wSlotNo > 5)
    {
        Log(ThisFile, __LINE__, "写指定暂存仓信息: Input SlotNo<%d> 无效, Return %d.",
            wSlotNo, IMP_ERR_SLOTNO_INVALID);
        return IMP_ERR_SLOTNO_INVALID;
    }

    // 下发命令
    memcpy(szSndCmd, SND_CMD_SLOT_WRITEINFO, STAND_CMD_LENGHT);
    sprintf(szSndData, "%d%s", wSlotNo, szInfo);
    nSndDataLen = strlen(szSndData);
    nRet = SendCmd(szSndCmd, szSndData, nSndDataLen, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "写指定暂存仓信息: SendCmd(%s, %s, %d, %s)失败, Return %d.",
            szSndCmd, szSndData, nSndDataLen, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "写指定暂存仓信息: 命令<%s%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szSndData, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 7. 读指定暂存仓信息
 * 参数: wSlotNo　暂存仓编号(1~5)
 *      szInfo 返回数据，最大68位
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::ReadCardSlotInfo(WORD wSlotNo, CHAR szInfo[68+1])
{
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szSndData[1024] = { 0x00 };
    INT     nSndDataLen = 0;
    CHAR    szRcvData[2068] = { 0x00 };

    if (wSlotNo < 1 || wSlotNo > 5)
    {
        Log(ThisFile, __LINE__, "读指定暂存仓信息: Input SlotNo<%d> 无效, Return %d.",
            wSlotNo, IMP_ERR_SLOTNO_INVALID);
        return IMP_ERR_SLOTNO_INVALID;
    }

    // 下发命令
    memcpy(szSndCmd, SND_CMD_SLOT_READINFO, STAND_CMD_LENGHT);
    sprintf(szSndData, "%d", wSlotNo);
    nSndDataLen = strlen(szSndData);
    nRet = SendCmd(szSndCmd, szSndData, nSndDataLen, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "读指定暂存仓信息: SendCmd(%s, %s, %d, %s)失败, Return %d.",
            szSndCmd, szSndData, nSndDataLen, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "读指定暂存仓信息: 命令<%s%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szSndData, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    memset(szInfo, 0x00, sizeof(szInfo));
    memcpy(szInfo, szRcvData + 5, strlen(szRcvData) - 5);

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 8. 清除暂存仓信息
 * 参数: wSlotNo　暂存仓编号(1~5)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::ClearCardSoltInfo(WORD wSlotNo)
{
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szSndData[1024] = { 0x00 };
    INT     nSndDataLen = 0;
    CHAR    szRcvData[2068] = { 0x00 };

    if (wSlotNo < 0 || wSlotNo > 5)
    {
        Log(ThisFile, __LINE__, "清除暂存仓信息: Input SlotNo<%d> 无效, Return %d.",
            wSlotNo, IMP_ERR_SLOTNO_INVALID);
        return IMP_ERR_SLOTNO_INVALID;
    }

    // 下发命令
    if (wSlotNo == 0)
    {
        memcpy(szSndCmd, SND_CMD_SLOT_CLEARINFOALL, STAND_CMD_LENGHT);
    } else
    {
        memcpy(szSndCmd, SND_CMD_SLOT_READINFO, STAND_CMD_LENGHT);
        sprintf(szSndData, "%d", wSlotNo);
        nSndDataLen = strlen(szSndData);
    }
    nRet = SendCmd(szSndCmd, szSndData, nSndDataLen, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "清除暂存仓信息: SendCmd(%s, %s, %d, %s)失败, Return %d.",
            szSndCmd, szSndData, nSndDataLen, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "清除暂存仓信息: 命令<%s%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szSndData, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
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
INT CDevImpl_CRT730B:: GetSoftVersion(CHAR szVersion[8+1])
{
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_SYSE_SOFTVER, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "获取软件版本信息: SendCmd(%s, NULL, %0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "获取软件版本信息: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    memset(szVersion, 0x00, sizeof(szVersion));
    memcpy(szVersion, szRcvData + 5, strlen(szRcvData) - 5);

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
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_SYSE_DEVCAPAB, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "获取设备能力信息: SendCmd(%s, NULL, %0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "获取设备能力信息: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
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
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    CHAR    szTmp[7+1] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_COUNT_STORAGE, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "获取存卡计数: SendCmd(%s, NULL, %0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "获取存卡计数: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
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
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    CHAR    szTmp[7+1] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_COUNT_EJECT, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "获取退卡计数: SendCmd(%s, NULL, %0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "获取退卡计数: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
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
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_COUNT_CLEAR, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "清除所有计数: SendCmd(%s, NULL, %0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "清除所有计数: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
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
INT CDevImpl_CRT730B::GetDeviceSerialNumber(CHAR szSerialNum[18+1])
{
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_GET_SERIALNUM, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "获取设备序列号: SendCmd(%s, NULL, %0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "获取设备序列号: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    memcpy(szSerialNum, szRcvData + 5, strlen(szRcvData) - 5);

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 15. 设备复位
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::DeviceReset()
{
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_DEV_RESET, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "设备复位: SendCmd(%s, NULL, %0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "设备复位: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
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
    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    // 下发命令
    memcpy(szSndCmd, SND_CMD_DEV_UPDATA, STAND_CMD_LENGHT);
    nRet = SendCmd(szSndCmd, nullptr, 0, ThisFile);
    if (nRet != 0)
    {
        Log(ThisFile, __LINE__, "进入固件升级模式: SendCmd(%s, NULL, %0, %s)失败, Return %d.",
            szSndCmd, ThisFile, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 接收应答
    nRet = GetResponse(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile);
    if (nRet < 0)
    {
        Log(ThisFile, __LINE__, "进入固件升级模式: 命令<%s>->GetResponse(%s, %d, %d, %s)失败, Return %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisFile,
            ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck((LPCSTR)szSndCmd, (LPCSTR)szRcvData, nRet);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 应答数据Check
 * 参数:
 * 返回值:
************************************************************/
INT CDevImpl_CRT730B::RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT nRcvDataLen)
{
    if (nRcvDataLen < 5)
    {
        Log(ThisFile, __LINE__, "CMD<%s> 返回命令应答数据长度[%d]<5, Return %d.",
            lpcSndCmd, nRcvDataLen,  IMP_ERR_RCVDATA_INVALID);
        return IMP_ERR_RCVDATA_INVALID;
    }

    if (memcmp(lpcSndCmd + 1, lpcSndCmd + 1, STAND_CMD_LENGHT - 1) != 0)
    {
        Log(ThisFile, __LINE__, "CMD<%s> 返回无效的命令应答数据[%02X,%02X,%02X], Return %d.",
            lpcSndCmd, (int)(lpcRcvData[0]), (int)(lpcRcvData[1]), (int)(lpcRcvData[2]),
                IMP_ERR_RCVDATA_INVALID);
        return IMP_ERR_RCVDATA_INVALID;
    }

    if (memcmp(lpcRcvData, "N", 1) == 0) // 错误应答
    {
        CHAR    szErrCode[2+1] = { 0x00 };
        memcpy(szErrCode, lpcRcvData + STAND_CMD_LENGHT, 2);
        Log(ThisFile, __LINE__, "CMD<%s> 返回错误应答数据[%02X,%02X,%02X,%02X,%02X], Return %d.",
            lpcSndCmd, (int)(lpcRcvData[0]), (int)(lpcRcvData[1]), (int)(lpcRcvData[2]),
            (int)(lpcRcvData[3]), (int)(lpcRcvData[4]), ConvertErrorCode(szErrCode));
        return ConvertErrorCode(szErrCode);
    }

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
 *      pszCallFunc
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::SendCmd(const char *pszCmd, const char *lpData, int nLen, const char *pszCallFunc)
{
    if (m_pDev == nullptr)
    {
        Log(ThisFile, __LINE__, "m_pDev == null");
        return ERR_DEVPORT_NOTOPEN;
    }

    int nRet = 0;

    // 满足一帧下发
    if (nLen >= 0 && nLen <= CRT_PACK_MAX_CMP_LEN)
    {
        // one packet
        Log(ThisFile, __LINE__, "Send one packet, Data Length is :%d", nLen);
        nRet = SendSinglePacket(pszCmd, lpData, nLen, ThisFile);
        if (nRet < 0)
        {
            return nRet;
        }

    } else
    // 分多帧下发
    if ((nLen > CRT_PACK_MAX_CMP_LEN) && (lpData != nullptr))
    {
        // Multi packet
        Log(ThisFile, __LINE__, "Send Multi packet, Data Length is :%d", nLen);
        nRet = SendMultiPacket(pszCmd, lpData, nLen, ThisFile);
        if (nRet < 0)
        {
            return nRet;
        }
    }

    return ConvertErrorCode(nRet);
}

/************************************************************
 * 功能: 下发一帧数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 *      pszCallFunc
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::SendSinglePacket(const char* pszCmd, const char *lpData, int nLen, const char *pszCallFunc)
{
    //ThisFile(__FUNCTION__);
    char buf[2068] = {0};
    int nRet = 0;
    BYTE dwReportID = {0};
    int nbufSize = 0;

    // one packet
    dwReportID = REPORTID;
    buf[0] = (char)dwReportID;
    buf[1] = (char)((nLen + 3) / 256);
    buf[2] = (char)((nLen + 3) % 256);
    // TEXT命令
    memcpy(buf + 3, pszCmd, 3);

    if (lpData != nullptr)
    {
        memcpy(buf + 6, lpData, nLen);
    }

    nbufSize = nLen + 5;

    for (int iIndex = 0; iIndex < 4; iIndex++)
    {
        nRet = m_pDev->Send(buf, nbufSize, TIMEOUT_WAIT_ACTION);

        if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
        {
            if (nRet == ERR_DEVPORT_WRITE)
                Log(ThisFile, __LINE__, "%s: 退卡模块没有响应", pszCallFunc);
            else
                Log(ThisFile, __LINE__, "%s: SendData()出错，返回%d", pszCallFunc, nRet);
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
            return ConvertErrorCode(nRet);
        }
        else
            break;
    }
}

/************************************************************
 * 功能: 下发多帧数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 *      pszCallFunc
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT730B::SendMultiPacket(const char *pszCmd, const char *lpData, int nLen, const char *pszCallFunc)
{
    char buf[2068] = {0};
    int nPackCount = 0;
    BYTE dwReportID = {0};
    // 发送的数据格式
    // ReportID(1 byte) + LEN(2 byte) + CMD(2 byte) + TEXT(58 byte)
    int nRet = 0;

    // Multi packet
    dwReportID = REPORTID;
    buf[0] = (char)dwReportID;
    buf[1] = (char)((nLen + 3) / 256);
    buf[2] = (char)((nLen + 3) % 256);
    memcpy(buf + 3, pszCmd, 3);

    // 分包
    nPackCount = 1 + nLen / CRT_PACK_MAX_CMP_LEN;

    for (int nWriteIdx = 0; nWriteIdx < nPackCount; nWriteIdx++)
    {
        int nbufOffset = nWriteIdx * CRT_PACK_MAX_LEN; //64
        int nlpDataOffset = nWriteIdx * CRT_PACK_MAX_CMP_LEN; //59

        for (int iIndex = 0; iIndex < 4; iIndex++)
        {
            // frist
            if (nWriteIdx == 0)
            {
                memcpy(buf + 6, lpData, CRT_PACK_MAX_CMP_LEN);
                nRet = m_pDev->Send(buf, CRT_PACK_MAX_LEN, TIMEOUT_WAIT_ACTION);

            // last
            } else
            if (nWriteIdx == nPackCount - 1)
            {
                int overageLen = nLen - nWriteIdx * CRT_PACK_MAX_CMP_LEN;
                memcpy(buf + nbufOffset, buf, 6);
                memcpy(buf + nbufOffset + 6, lpData + nlpDataOffset, overageLen);
                nRet = m_pDev->Send(buf, overageLen + 6, TIMEOUT_WAIT_ACTION);
            // middle
            } else
            {
                memcpy(buf + nbufOffset, buf, 6);
                memcpy(buf + nbufOffset + 6, lpData + nlpDataOffset, CRT_PACK_MAX_CMP_LEN);
                nRet = m_pDev->Send(buf, CRT_PACK_MAX_LEN, TIMEOUT_WAIT_ACTION);
            }

            if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
            {
                if (nRet == ERR_DEVPORT_WRITE)
                    Log(ThisFile, __LINE__, "%s: 退卡模块没有响应", pszCallFunc);
                else
                    Log(ThisFile, __LINE__, "%s: SendData()出错，返回%d", pszCallFunc, nRet);
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
                return ConvertErrorCode(nRet);
            }
            else
                break;
        }
    }
    return ConvertErrorCode(nRet);
}

/*
  功能：读取读卡器的返回数据
  参数：pszReponse返回数据的缓冲区
       nLen缓冲区长度
       nTimeout超时(毫秒)
       pWaitCancel是IWaitCancel指针
  返回：>0数据长度，=0错误
*/
INT CDevImpl_CRT730B::GetResponse(char *pszResponse, int nLen, int nTimeout, const char *pszCallFunc)
{
    Q_UNUSED(nLen)

    char pszReply[64];
    int nRet = 0;
    int nIndex = 0;
    int timeout = nTimeout;

    if (m_pDev == nullptr)
        return ERR_DEVPORT_NOTOPEN;

    while(TRUE)
    {
        DWORD ulInOutLen = USB_READ_LENTH;
        memset(pszReply, 0, sizeof(pszReply));
        nRet = m_pDev->Read(pszReply, ulInOutLen, timeout);
        if (nRet <= 0)
        {
            break;
        } else {
            if (nIndex == 0)
            {
               // 第一个包
               nIndex = nRet;
               memcpy(pszResponse, pszReply, nIndex);
               timeout = 50;
            } else
            {
                // 多个包
                memcpy(pszResponse + nIndex, pszReply + 1, nRet - 1);
                nIndex += nRet - 1;
            }
            if (pszReply[0] == 0x04)
            {
                break;
            }
            continue;
        }
    }

    DWORD nRecvLen = MAKEWORD(pszResponse[2], pszResponse[1]);
    if (nRecvLen != 0)
    {
        return nRecvLen + 3;
    }
    else
    {
        Log(ThisFile, __LINE__, "退卡模块返回数据错误");
        return IMP_ERR_READERROR;
    }
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

//---------------------------------------------
