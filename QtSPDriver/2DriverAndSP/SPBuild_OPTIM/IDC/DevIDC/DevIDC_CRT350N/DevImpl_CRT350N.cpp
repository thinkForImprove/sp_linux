/***************************************************************
* 文件名称：DevImpl_CRT350N.cpp
* 文件描述：封装读卡器模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/
#include "DevImpl_CRT350N.h"

static const char *ThisFile = "DevImpl_CRT350N.cpp";

CDevImpl_CRT350N::CDevImpl_CRT350N()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CRT350N::CDevImpl_CRT350N(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CRT350N::CDevImpl_CRT350N(LPSTR lpLog, LPCSTR lpDevType)
{
    SetLogFile(lpLog, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件
    MSET_0(m_szDevType);
    MCPY_NOLEN(m_szDevType, strlen(lpDevType) > 0 ? lpDevType : "CRT-350N");
    Init();
}

// 参数初始化
void CDevImpl_CRT350N::Init()
{
    m_strMode.clear();                              // 连接USB串
    m_bDevOpenOk = FALSE;                           // 设备是否Open
    m_bReCon = FALSE;                               // 是否断线重连状态
    m_wPredictIC = 0;                               // 进卡检查模式
    MSET_0(m_szDevType);                            // 设备类型
    MSET_0(m_szErrStr);                             // IMPL错误解释
    MSET_0(m_szCmdStr);                             // 命令解释
    m_dwSndTimeOut = TIMEOUT_WAIT_ACTION;           // 命令下发超时时间(毫秒)
    m_dwRcvTimeOut = TIMEOUT_WAIT_ACTION;           // 命令接收超时时间(毫秒)
    memset(m_nRetErrOLD, 0, sizeof(INT) * 8);
}

CDevImpl_CRT350N::~CDevImpl_CRT350N()
{
    //vUnLoadLibrary();
}


/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参　格式: USB:VID,PID  VID/PID为4位16进制字符串
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::OpenDevice(LPSTR lpMode, LPCSTR lpcInit, INT nInitLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT     nRet = IMP_SUCCESS;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    if (m_pDev == nullptr)
    {
        if (m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "IDC", m_szDevType) != 0)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: 加载USB动态库: ->Load(AllDevPort.dll) Fail, Return: %s.",
                    ConvertCode_Impl2Str(IMP_ERR_LOAD_LIB));
                m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            }

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
            Log(ThisModule, __LINE__, "打开设备: 入参为null,使用缺省VID/PID[%s].",
                m_strMode.c_str());
        }
    } else
    {
        m_strMode = lpMode;
        //Log(ThisModule, __LINE__, "打开设备: 入参为[%s].", m_strMode.c_str());
    }

    // 打开设备
    LOGDEVACTION();
    long lRet = m_pDev->Open(m_strMode.c_str());
    if (lRet < 0)
    {
        if (m_nRetErrOLD[1] != lRet)
        {
            Log(ThisModule, __LINE__,
                "打开设备: ->Open(%s) Fail, ErrCode: %d, Return: %d.",
                m_strMode.c_str(), lRet, ConvertCode_Impl2Str(ConvertCode_USB2Impl(lRet)));
            m_nRetErrOLD[1] = lRet;
        }

        return ConvertCode_USB2Impl(lRet);
    }

    // 初始化: 命令收发检查
    MCPY_LEN(m_szResetParam, lpcInit, nInitLen);
    memcpy(szSndCmd, SND_CMD_INIT_C07, STAND_CMD_LENGHT);
    nRet = SndRcvToChk(szSndCmd, lpcInit, nInitLen, szRcvData, nRcvSize, "打开设备: 模块初始化", !m_bReCon);
    if (nRet != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[2] != nRet)
        {
            Log(ThisModule, __LINE__,
                "打开设备: 模块初始化: 命令收发: ->SndRcvToChk(%s, %s, %d)失败, ErrCode: %d, Return %s.",
                szSndCmd, lpcInit, nInitLen, nRet, ConvertCode_Impl2Str(nRet));
            m_nRetErrOLD[2] = nRet;
        }
        return nRet;
    }


    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "断线重连: 打开设备(%s)+初始化: Succ.",
            m_strMode.c_str());
        m_bReCon = FALSE; // 是否断线重连状态: 初始F
    } else
    {
        Log(ThisModule, __LINE__,  "打开设备(%s)+初始化 Succ.",
            m_strMode.c_str());
    }

    //memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::CloseDevice()
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
INT CDevImpl_CRT350N::Release()
{
    return CloseDevice();
}

BOOL CDevImpl_CRT350N::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/************************************************************
 * 功能: 1. 模块初始化
 * 参数: wInitMode 初始化方式(0/1/2/4)　参考宏定义
 *      bIsRetainCnt 是否启用回收计数器
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::DeviceInit(EN_DEV_INIT enInit, LPSTR lpParam, WORD wParLen,
                                 BOOL bIsRetainEna)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    // 初始化方式
    if (enInit == INIT_EJECT)       // 初始化:有卡时弹到前端
    {
        if (bIsRetainEna == TRUE)
        {
            memcpy(szSndCmd, SND_CMD_INIT_C04, STAND_CMD_LENGHT);
        } else
        {
            memcpy(szSndCmd, SND_CMD_INIT_C00, STAND_CMD_LENGHT);
        }

    } else
    if (enInit == INIT_RETAIN)      // 初始化:有卡时吞卡
    {
        if (bIsRetainEna == TRUE)
        {
            memcpy(szSndCmd, SND_CMD_INIT_C05, STAND_CMD_LENGHT);
        } else
        {
            memcpy(szSndCmd, SND_CMD_INIT_C01, STAND_CMD_LENGHT);
        }
    } else
    if (enInit == INIT_REACCEPT)    // 初始化:有卡时重进卡
    {
        if (bIsRetainEna == TRUE)
        {
            memcpy(szSndCmd, SND_CMD_INIT_C06, STAND_CMD_LENGHT);
        } else
        {
            memcpy(szSndCmd, SND_CMD_INIT_C02, STAND_CMD_LENGHT);
        }
    } else
    if (enInit == INIT_NOACTION)    // 初始化:有卡时不移动卡
    {
        if (bIsRetainEna == TRUE)
        {
            memcpy(szSndCmd, SND_CMD_INIT_C07, STAND_CMD_LENGHT);
        } else
        {
            memcpy(szSndCmd, SND_CMD_INIT_C03, STAND_CMD_LENGHT);
        }
    } else
    {
        Log(ThisModule, __LINE__, "初始化: 入参wMode[%d]无效, Return %s.",
            enInit, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, lpParam, wParLen, szRcvData, nRcvSize, "初始化");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "初始化: 命令收发: ->SndRcvToChk(%s, %s, %d)失败, ErrCode: %d, Return %s.",
            szSndCmd, (lpParam == nullptr ? "NULL" : lpParam), wParLen, nRet,
            ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 2. 获取设备状态
 * 参数: wMode 状态类别(1/2/4)　参考宏定义
 * 　　　nStat　返回状态数组,如下: 各状态参考宏定义
 *             [0]:设备状态; [1]:卡状态; [2]:触点状态;
 *             [3]:闸门状态; [4]:后卡口; 5~11位保留
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::GetDeviceStat(WORD wMode, INT nStat[12])
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    if (wMode == GET_STAT_ALL ||   // 设备+卡
        wMode == GET_STAT_DEV ||   // 设备
        wMode == GET_STAT_CARD)    // 卡
    {
        memcpy(szSndCmd, SND_CMD_STAT_C13, STAND_CMD_LENGHT);
    } else
    {
        //Log(ThisModule, __LINE__, "获取状态: 入参wMode[%d]无效, Return %s.",
        //    wMode, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, nullptr, 0, szRcvData, nRcvSize, "获取状态", FALSE);
    if (nRet != IMP_SUCCESS)
    {
        return nRet;
    }

    // szRcvData: 3byte+P/N,1,PM,st1,st0,Se1,Se0,Se2,Se3
    // st1+st0: 00读卡器内无卡, 01卡在卡口(前), 02卡在读卡器内
    // Se1: B7:0, B6:1, B5:0, B4:0, B3:0, B2:ICD:0IC触点接触/1IC触点释放
    //      B1:PDI:0无卡/1有卡, B0:PHD:0无磁信号(进卡后保持0)/1有磁信号
    // Se0: B7:0, B6:1, B5:0, B4:SW1:0无卡/1有卡, B3:SW2:0闸门关/1闸门开,
    //      B2:PD1:0无卡/1有卡, B1:PD2:0无卡/1有卡, B0:PD3:0无卡/1有卡
    // Check 应答状态码(st1,st0)
    CHAR szStat[2+1] = { 0x00 };
    memcpy(szStat, szRcvData + 6, 2);
    if (MCMP_IS0(szStat, CARDER_STAT_NOCARD))            // 0x30,0x30:读卡器内无卡
    {
        nStat[1] = IMPL_STAT_CARD_NOTHAVE;
    } else
    if (MCMP_IS0(szStat, CARDER_STAT_CARD_IS_EXPORT))    // 0x30,0x31:卡在出口
    {
        nStat[1] = IMPL_STAT_CARD_ISEXPORT;
    } else
    if (MCMP_IS0(szStat, CARDER_STAT_CARD_IS_INSIDE))    // 0x30,0x32:卡在内部
    {
        nStat[1] = IMPL_STAT_CARD_ISINSIDE;
    } else                                               // 卡状态未知
    {
        nStat[1] = IMPL_STAT_CARD_INVALID;
    }

    // 检查闸门状态(Se0 Bit3)
    nStat[3] = (BIT3(szRcvData[9]) == 1 ? IMPL_STAT_SHUT_OPEN : IMPL_STAT_SHUT_CLOSE);

    // 检查触点状态(Se1 Bit2)
    nStat[2] = (BIT2(szRcvData[8]) == 1 ? IMPL_STAT_MEET_DOWN : IMPL_STAT_MEET_UP);

    // 检查后卡口(SE0 Bit0)
    nStat[4] = (BIT0(szRcvData[9]) == 1 ? IMPL_STAT_CARD_ISAFTER_EXPORT : IMPL_STAT_CARD_NOTHAVE);

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 3. 设置回收计数
 * 参数: wSize 更新值
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::SetDevRetainCnt(WORD wSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szSndPar[24] = { 0x00 };
    INT     nSndParLen = 0;
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    if (wSize < 0 || wSize > 99)
    {
        Log(ThisModule, __LINE__, "设置回收计数: 入参wMode[%d]无效, 必须 0～99, Return %s.",
            wSize, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    memcpy(szSndCmd, SND_CMD_SET_RETAINCNT, STAND_CMD_LENGHT);
    sprintf(szSndPar, "%02d", wSize);
    nSndParLen = 2;

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, szSndPar, nSndParLen, szRcvData, nRcvSize, "设置回收计数");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设置回收计数: 命令收发: ->SndRcvToChk(%s, %s, %d)失败, ErrCode: %d, Return %s.",
            szSndCmd, szSndPar, nSndParLen, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 4. 取版本号
 * 参数: wType 类别(0:固件)
 *      lpVerStr 返回版本内容
 *      wVerSize lpVerStr传入空间Size
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::GetVersion(WORD wType, LPSTR lpVerStr, WORD wVerSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    switch(wType)
    {
        case 0:     // 取固件版本
            memcpy(szSndCmd, SND_CMD_READ_VER_CA6, STAND_CMD_LENGHT);
            break;
        case 1:     // 取软件版本
            memcpy(szSndCmd, SND_CMD_READ_VER_CA1, STAND_CMD_LENGHT);
            break;
        default:
            Log(ThisModule, __LINE__, "取版本号: 入参wType[%d]无效, 必须 0～1, Return %s.",
                wType, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
            return IMP_ERR_PARAM_INVALID;
    }

    if (lpVerStr == nullptr || wVerSize == 0)
    {
        Log(ThisModule, __LINE__, "取版本号: 入参lpVerStr[%s]/wVerSize[%s]无效, Return %s.",
            lpVerStr == nullptr ? "NULL" : lpVerStr, wVerSize,
            ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, nullptr, 0, szRcvData, nRcvSize, "设置回收计数");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设置回收计数: 命令收发: ->SndRcvToChk(%s, NULL, 0)失败, ErrCode: %d, Return %s.",
            szSndCmd, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    WORD wSize = nRcvSize - 5;
    memcpy(lpVerStr, szRcvData + 5, (wSize > wVerSize ? wVerSize - 1 : wSize));

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 5. 异步进卡
 * 参数: wType 类别
 *      nParam: 辅助参数
 *       抖动进卡时: 0进卡时不抖动, 1进卡时抖动
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::CardAcceptASync(EN_INCARD_PAR enInCard, INT nParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);
    CHAR    szSndParam[16] = { 0x00 };
    INT     nSndParamSize = 0;

    switch(enInCard)
    {
        case INCARD_NOT:        // 禁止进卡
            memcpy(szSndCmd, SND_CMD_ASYNC_INCARD_CH3A1, STAND_CMD_LENGHT);
            break;
        case INCARD_ALL:        // 进卡(所有卡不检查)
            memcpy(szSndCmd, SND_CMD_ASYNC_INCARD_CH3A0, STAND_CMD_LENGHT);
            break;
        case INCARD_MAG:        // 只允许磁卡
            memcpy(szSndCmd, SND_CMD_ASYNC_INCARD_CH3A2, STAND_CMD_LENGHT);
            break;
        case INCARD_IC:         // 只允许芯片卡
            memcpy(szSndCmd, SND_CMD_ASYNC_INCARD_CH3A3, STAND_CMD_LENGHT);
            break;
        case INCARD_MAG2IC:     // 允许进磁卡和芯片卡
            memcpy(szSndCmd, SND_CMD_ASYNC_INCARD_CH3A4, STAND_CMD_LENGHT);
            break;
        case INCARD_MAG3IC:     // 允许进磁卡或芯片卡
            memcpy(szSndCmd, SND_CMD_ASYNC_INCARD_CH3AD, STAND_CMD_LENGHT);
            break;
        case INCARD_SHAKE:      // 抖动进卡
        {
            memcpy(szSndCmd, SND_CMD_ASYNC_INCARD_CH3AX, STAND_CMD_LENGHT);
            if (nParam == 0)
            {
                szSndParam[0] = 0x30;
            } else
            {
                szSndParam[0] = 0x31;
            }
            nSndParamSize = 1;
            break;
        }
        default:
            Log(ThisModule, __LINE__, "异步进卡: 入参wType[%d]无效, 必须 0～1, Return %s.",
                enInCard, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
            return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, szSndParam, nSndParamSize, szRcvData, nRcvSize, "异步进卡");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "异步进卡: 命令收发: ->SndRcvToChk(%s, %s, %d, 0)失败, ErrCode: %d, Return %s.",
            szSndCmd, szSndParam, nSndParamSize, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 6. 读磁处理
 * 参数: wType 类别
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::MagneticRead(DWORD dwMag, LPSTR lpData, INT nSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    // 存在读两个磁道及以上,使用读所有磁道方式
    if ((AND_IS1(dwMag, MAG_READ_TRACK1) && AND_IS1(dwMag, MAG_READ_TRACK2)) ||
        (AND_IS1(dwMag, MAG_READ_TRACK2) && AND_IS1(dwMag, MAG_READ_TRACK3)) ||
        (AND_IS1(dwMag, MAG_READ_TRACK1) && AND_IS1(dwMag, MAG_READ_TRACK3)))
    {
        memcpy(szSndCmd, SND_CMD_READ_CARD_C65, STAND_CMD_LENGHT);
    } else
    {
        if (AND_IS1(dwMag, MAG_READ_TRACK1))
        {
            memcpy(szSndCmd, SND_CMD_READ_CARD_C61, STAND_CMD_LENGHT);
        } else
        if (AND_IS1(dwMag, MAG_READ_TRACK2))
        {
            memcpy(szSndCmd, SND_CMD_READ_CARD_C62, STAND_CMD_LENGHT);
        } else
        if (AND_IS1(dwMag, MAG_READ_TRACK3))
        {
            memcpy(szSndCmd, SND_CMD_READ_CARD_C63, STAND_CMD_LENGHT);
        }
    }

    if (lpData == nullptr || nSize == 0)
    {
        Log(ThisModule, __LINE__, "读磁处理: 入参lpData[%s]/Size[%s]无效, Return %s.",
            lpData == nullptr ? "NULL" : lpData, nSize,
            ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, nullptr, 0, szRcvData, nRcvSize, "读磁处理");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "读磁处理: 命令收发: ->SndRcvToChk(%s, NULL, 0)失败, ErrCode: %d, Return %s.",
            szSndCmd, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    INT nLen = nRcvSize - RCV_DATA_POS;
    memcpy(lpData, szRcvData + RCV_DATA_POS, (nSize > nSize ? nSize - 1 : nLen));

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 8. 芯片操作
 * 参数: wType 类别
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::ChipOperation(EN_CHIP_OPATION enAtion, LPSTR lpRcvData, INT *nRcvDataSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);
    BOOL    bHaveRetData = FALSE;

    switch(enAtion)
    {
        case CHIP_PRESS:        // 芯片接触/压下
            memcpy(szSndCmd, SND_CMD_CHIP_PRESS, STAND_CMD_LENGHT);
            break;
        case CHIP_RELEASE:      // 芯片释放
            memcpy(szSndCmd, SND_CMD_CHIP_RELEASE, STAND_CMD_LENGHT);
            break;
        case CHIP_ACTIVE:       // 芯片激活(冷复位)
            memcpy(szSndCmd, SND_CMD_CHIP_ACTION_CI0, STAND_CMD_LENGHT);
            bHaveRetData = TRUE;    // 命令应答有返回数据
            break;
        case CHIP_WARM:         // 热复位
            memcpy(szSndCmd, SND_CMD_CHIP_RESETWARM_CI8, STAND_CMD_LENGHT);
            bHaveRetData = TRUE;    // 命令应答有返回数据
            break;
        case CHIP_DEACTIVE:     // 芯片关闭
            memcpy(szSndCmd, SND_CMD_CHIP_DEACTION_CI1, STAND_CMD_LENGHT);
            break;
        default:
            Log(ThisModule, __LINE__, "芯片操作: 入参enAtion[%d]无效, 必须 0～1, Return %s.",
                enAtion, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
            return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, nullptr, 0, szRcvData, nRcvSize, "芯片操作");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "芯片操作: 命令收发: ->SndRcvToChk(%s, NULL, 0)失败, ErrCode: %d, Return %s.",
            szSndCmd, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    // 命令应答有返回数据
    if (bHaveRetData == TRUE)
    {
        INT nLen = nRcvSize - RCV_DATA_POS;
        *nRcvDataSize = (nLen > *nRcvDataSize ? *nRcvDataSize - 1 : nLen);
        memcpy(lpRcvData, szRcvData + RCV_DATA_POS, *nRcvDataSize);
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 9. 芯片通信
 * 参数: wType 类别
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::ChipProtocol(EN_CHIP_OPATION enAtion, LPSTR lpSnd, WORD wSndSize,
                                   LPSTR lpRcv, DWORD &dwRcvSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);
    BOOL    bHaveRetData = FALSE;
    //DWORD   dwRcvDataSize = dwRcvSize;

    switch(enAtion)
    {
        case CHIP_COMMT0:       // 芯片T0通信
            memcpy(szSndCmd, SND_CMD_CHIP_COMMT0_CI3, STAND_CMD_LENGHT);
            break;
        case CHIP_COMMT1:       // 芯片T1通信
            memcpy(szSndCmd, SND_CMD_CHIP_COMMT1_CI4, STAND_CMD_LENGHT);
            break;
        case CHIP_COMMAUTO:     // 芯片通信(自动选择)
            memcpy(szSndCmd, SND_CMD_CHIP_COMMAUTO_CI9, STAND_CMD_LENGHT);
            break;
        default:
            Log(ThisModule, __LINE__, "芯片通信: 入参enAtion[%d]无效, 必须 0～1, Return %s.",
                enAtion, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
            return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, lpSnd, wSndSize, szRcvData, nRcvSize, "芯片通信");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "芯片通信: 命令收发: ->SndRcvToChk(%s, %s, %d)失败, ErrCode: %d, Return %s.",
            szSndCmd, lpSnd == nullptr ? "NULL" : lpSnd, wSndSize, nRet,
            ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    // 返回61XX的特殊处理
    if((szRcvData[RCV_DATA_POS] == 0x61) && ((MAKEWORD(szRcvData[2], szRcvData[1])) - 5 == 2))
    {
        BYTE byGetChipResCmd[5] = { 0x00, 0xC0, 0x00, 0x00, 0x00 };
        byGetChipResCmd[4] = szRcvData[RCV_DATA_POS + 1];

        // 命令收发检查
        MSET_0(szRcvData);
        nRcvSize = sizeof(szRcvData);
        nRet = SndRcvToChk(szSndCmd, (LPSTR)byGetChipResCmd, 5, szRcvData, nRcvSize,
                           "芯片操作: 返回61XX的特殊处理");
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片通信: 返回61XX的特殊处理: 命令收发: ->SndRcvToChk(%s, %s, %d)失败, "
                "ErrCode: %d, Return %s.",
                szSndCmd, (LPSTR)byGetChipResCmd, 5, nRet, ConvertCode_Impl2Str(nRet));
            return nRet;
        }
    }

    // 应答超出1000Byte的处理(PI5)
    DWORD dwRcvLen = 0;     // 每次返回应答数据长度
    DWORD dwOffset = 0;     // 已获取应答数据长度
    if (szRcvData[RCV_CMD_START_POS + 2] == '5')
    {
        do
        {
            // 命令收发检查
            MSET_0(szSndCmd);
            memcpy(szSndCmd, SND_CMD_CHIP_COMMKZ3_CI7, STAND_CMD_LENGHT);
            MSET_0(szRcvData);
            nRcvSize = sizeof(szRcvData);
            nRet = SndRcvToChk(szSndCmd, nullptr, 0, szRcvData, nRcvSize, "芯片操作: 返回61XX的特殊处理");
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "芯片通信: 应答超出1000Byte的处理: 命令收发: ->SndRcvToChk(%s, NULL, 0)失败, "
                    "ErrCode: %d, Return %s.",
                    szSndCmd, nRet, ConvertCode_Impl2Str(nRet));
                return nRet;
            }

            dwRcvLen = (MAKEWORD(szRcvData[2], szRcvData[1])) - 5;
            if (dwRcvLen + dwOffset > dwRcvSize)
            {
                dwRcvLen = dwRcvSize - dwOffset;
                memcpy(lpRcv + dwOffset, szRcvData + RCV_DATA_POS, dwRcvLen);
                Log(ThisModule, __LINE__,
                    "芯片通信: 应答超出1000Byte的处理: Chipio实际接收数据大于入参buffer[%d]",
                    dwRcvSize);
                break;
            }
            memcpy(lpRcv + dwOffset, szRcvData + RCV_DATA_POS, dwRcvLen);
            dwOffset += dwRcvLen;

        } while (szRcvData[RCV_CMD_START_POS + 2] != '6');
        dwRcvSize = dwOffset;
    } else  // 不超过1000字节
    {
        // 命令应答有返回数据
        INT nLen = nRcvSize - RCV_DATA_POS;
        dwRcvSize = (nLen > dwRcvSize ? dwRcvSize - 1 : nLen);
        memcpy(lpRcv, szRcvData + RCV_DATA_POS, dwRcvSize);
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 10. 介质控制
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::MediaOperation(EN_CARD_OPATION enAtion, WORD wParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szParam[124] = { 0x00 };
    CHAR    szSndCmd2[124] = { 0x00 };      // 缺省命令失败后的执行命令
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);
    CHAR    szLogBuff[256] = { 0x00 };

    sprintf(szLogBuff, "介质控制");

    switch(enAtion)
    {
        case CARD_EJECT:       // 介质退出
            memcpy(szSndCmd, SND_CMD_INIT_C00, STAND_CMD_LENGHT);   // 弹卡到前端
            memcpy(szSndCmd2, SND_CMD_CARD_MOVE_C30, STAND_CMD_LENGHT);
            MCPY_NOLEN(szParam, m_szResetParam);
            sprintf(szLogBuff + strlen(szLogBuff), ": 退卡");
            break;
        case CARD_RETAIN:      // 介质回收
            memcpy(szSndCmd, SND_CMD_INIT_C01, STAND_CMD_LENGHT);
            memcpy(szSndCmd2, SND_CMD_CARD_MOVE_C31, STAND_CMD_LENGHT);
            MCPY_NOLEN(szParam, m_szResetParam);
            sprintf(szLogBuff + strlen(szLogBuff), ": 回收卡");
            break;
        case CARD_MOVE:        // 介质移动
            memcpy(szSndCmd, SND_CMD_CARD_MOVE_C32, STAND_CMD_LENGHT);
            sprintf(szLogBuff + strlen(szLogBuff), ": 卡移动MM位");
            break;
        default:
            Log(ThisModule, __LINE__, "%s: 入参enAtion[%d]无效, Return %s.",
                szLogBuff, enAtion, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
            return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, szParam, strlen(szParam), szRcvData, nRcvSize, szLogBuff);
    if (nRet != IMP_SUCCESS)
    {
        if ((enAtion == CARD_EJECT || enAtion == CARD_RETAIN) &&
            (szRcvData + RCV_CMD_START_POS)[0] == 'N')
        {
            Log(ThisModule, __LINE__, "%s: 命令<%s>应答: 执行失败, 切换执行命令<%s>: ...",
                szLogBuff, szSndCmd, szSndCmd2);

            nRet = SndRcvToChk(szSndCmd2, nullptr, 0, szRcvData, nRcvSize, szLogBuff);
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "%s: 切换执行命令<%s>: 命令收发: ->SndRcvToChk(%s, NULL, 0)失败, "
                    "ErrCode: %d, Return %s.",
                    szLogBuff, szSndCmd2, szSndCmd2, nRet, ConvertCode_Impl2Str(nRet));
                return nRet;
            }
        } else
        {
            Log(ThisModule, __LINE__,
                "%s: 命令收发: ->SndRcvToChk(%s, NULL, 0)失败, ErrCode: %d, Return %s.",
                szLogBuff, szSndCmd, nRet, ConvertCode_Impl2Str(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}


/************************************************************
 * 功能: 11. 获取防盗钩状态
 * 参数:
 * 返回值: 参考错误码/防盗钩状态宏定义
************************************************************/
INT CDevImpl_CRT350N::GetTamperStat(INT &nStat)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    nStat = IMPL_TAMPER_UNKNOWN;
    memcpy(szSndCmd, SND_CMD_TAMPER_GETSTAT_Cc8, STAND_CMD_LENGHT);

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, nullptr, 0, szRcvData, nRcvSize, "获取防盗钩状态");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取防盗钩状态: 命令收发: ->SndRcvToChk(%s, NULL, 0) Fail, ErrCode: %d, Return %s.",
            szSndCmd, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    if (BIT0(szRcvData[RCV_DATA_POS]) == 1)
    {
        nStat = IMPL_TAMPER_PRESS;      // 防盗钩压下
    } else
    {
        nStat = IMPL_TAMPER_RELEASE;    // 防盗钩释放
    }

    if (BIT1(szRcvData[RCV_DATA_POS]) == 1)
    {
        nStat += IMPL_TAMPER_PRESS_NORUN;      // 防盗钩压下未执行
    } else
    {
        nStat += IMPL_TAMPER_RELEASE_NORUN;    // 防盗钩释放未执行
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 12. 设置防盗钩动作
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::SetTamperOperAtion(EN_TAMPER_OPATION enAtion)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szSndParam[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);
    CHAR    szLogBuff[256] = { 0x00 };

    sprintf(szLogBuff, "设置防盗钩动作");

    switch(enAtion)
    {
        case TAM_SETSTAND:       // 设置为标准模式
            memcpy(szSndCmd, SND_CMD_TAMPER_SETWORK_Cc9, STAND_CMD_LENGHT);
            szSndParam[0] = 0x30;
            sprintf(szLogBuff + strlen(szLogBuff), ": 设置为标准模式");
            break;
        case TAM_SETAUTO:       // 设置为自动模式
            memcpy(szSndCmd, SND_CMD_TAMPER_SETWORK_Cc9, STAND_CMD_LENGHT);
            szSndParam[0] = 0x31;
            sprintf(szLogBuff + strlen(szLogBuff), ": 设置为自动模式");
            break;
        case TAM_SETPRESS:      // 设置压下
            memcpy(szSndCmd, SND_CMD_TAMPER_CONT_CcH3A, STAND_CMD_LENGHT);
            szSndParam[0] = 0x30;
            sprintf(szLogBuff + strlen(szLogBuff), ": 设置压下");
            break;
        case TAM_SETRELEASE:    // 设置释放
            memcpy(szSndCmd, SND_CMD_TAMPER_CONT_CcH3A, STAND_CMD_LENGHT);
            szSndParam[0] = 0x31;
            sprintf(szLogBuff + strlen(szLogBuff), ": 设置释放");
            break;
        default:
            Log(ThisModule, __LINE__, "%s: 入参enAtion[%d]无效, Return %s.",
                szLogBuff, enAtion, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
            return IMP_ERR_PARAM_INVALID;
    }

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, nullptr, 0, szRcvData, nRcvSize, szLogBuff);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "%s: 命令收发: ->SndRcvToChk(%s, NULL, 0) Fail, ErrCode: %d, Return %s.",
            szLogBuff, szSndCmd, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 13. 重进卡
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::CardReAccept()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    memcpy(szSndCmd, SND_CMD_RE_INCARD_C40, STAND_CMD_LENGHT);

    // 命令收发检查
    nRet = SndRcvToChk(szSndCmd, nullptr, 0, szRcvData, nRcvSize, "重进卡");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "重进卡: 命令收发: ->SndRcvToChk(%s, NULL, 0) Fail, ErrCode: %d, Return %s.",
            szSndCmd, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 下发数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 *      lpFuncData 日志输出字串
 *      bIsPrtLog 是否打印日志
 * 返回值: 参考IMPL错误码
************************************************************/
INT CDevImpl_CRT350N::SendCmd(LPCSTR lpcCmd, LPCSTR lpcData, INT nLen, LPCSTR lpFuncData,
                              BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (m_pDev == nullptr)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__, "%s: m_pDev == NULL, USB动态库句柄无效, Return: %ｓ",
                lpFuncData, ConvertCode_Impl2Str(IMP_ERR_DEVPORT_NOTOPEN));
        }
        return ERR_DEVPORT_NOTOPEN;
    }

    if (nLen > 0 && lpcData == nullptr)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "%s: 下发命令数据长度[%d] > 0, 数据Buff == NULL, 不执行下发, Return: %s",
                lpFuncData, nLen, ConvertCode_Impl2Str(IMP_ERR_PARAM_INVALID));
            return IMP_ERR_PARAM_INVALID;
        }
    }

    INT nRet = IMP_SUCCESS;

    // 满足一帧下发
    if (nLen >= 0 && nLen <= CRT_PACK_MAX_CMP_LEN)
    {
        /*if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__, "%s: 数据长度[%d] <= %d, 满足一帧下发...",
                lpFuncData, nLen, CRT_PACK_MAX_CMP_LEN);
        }*/
        nRet = SendSinglePacket(lpcCmd, lpcData, nLen, lpFuncData, bIsPrtLog);
        if (nRet < 0)
        {
            return nRet;
        }

    } else
    // 分多帧下发
    if ((nLen > CRT_PACK_MAX_CMP_LEN) && (lpcData != nullptr))
    {
        /*if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__, "%s: 数据长度[%d] >= %d, 满足多帧下发...",
                lpFuncData, nLen, CRT_PACK_MAX_CMP_LEN);
        }*/
        nRet = SendMultiPacket(lpcCmd, lpcData, nLen, lpFuncData, bIsPrtLog);
        if (nRet < 0)
        {
            return nRet;
        }
    }

    return nRet;
}

/************************************************************
 * 功能: 下发一帧数据到Device
 * 参数: lpcCmd 命令
 *      lpcCmdPar 命令数据
 *      nParLen   命令数据长度
 *      lpFuncData 日志输出字串
 *      bIsPrtLog 是否打印日志
 * 返回值: 参考IMPL错误码
************************************************************/
INT CDevImpl_CRT350N::SendSinglePacket(LPCSTR lpcCmd, LPCSTR lpcCmdPar, INT nParLen,
                                       LPCSTR lpFuncData, BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR szCmdBuffer[2068] = { 0x00 };              // 下发命令
    INT nCmdBuffSize = 0;                           // 下发命令长度
    INT nRet = ERR_DEVPORT_SUCCESS;
    BYTE dwReportID = { 0x00 };

    // 一帧包头
    dwReportID = REPORTID;
    szCmdBuffer[0] = (CHAR)dwReportID;              // 包头标识
    szCmdBuffer[1] = (CHAR)((nParLen + 3) / 256);   // 包长度高位字节
    szCmdBuffer[2] = (CHAR)((nParLen + 3) % 256);   // 包长度低位字节

    // TEXT命令
    memcpy(szCmdBuffer + 3, lpcCmd, 3);             // 追加命令

    if (lpcCmdPar != nullptr)
    {
        memcpy(szCmdBuffer + 6, lpcCmdPar, nParLen);      // 追加命令参数
    }

    nCmdBuffSize = nParLen + 6;

    for (INT nIndex = 0; nIndex < 4; nIndex++)
    {
        nRet = m_pDev->Send(szCmdBuffer, nCmdBuffSize, m_dwSndTimeOut);
        if (nRet != ERR_DEVPORT_SUCCESS)
        {
            if (nRet == ERR_DEVPORT_NOTOPEN || nRet == ERR_DEVPORT_WRITE)
            {
                if (bIsPrtLog == TRUE)
                {
                    if (nIndex < 3)
                    {
                        Log(ThisModule, __LINE__,
                            "%s: 单帧命令下发第[%d]次: ->Send(%s, %d, %d) Fail, ErrCode: %s, "
                            "重新下发...",
                            lpFuncData, nIndex + 1, szCmdBuffer, nCmdBuffSize, m_dwSndTimeOut,
                            ConvertCode_Impl2Str(nRet));
                        continue;
                    } else
                    {
                        Log(ThisModule, __LINE__,
                            "%s: 单帧命令下发第[%d]次: ->Send(%s, %d, %d) Fail, ErrCode: %s, "
                            "Return: %s",
                            lpFuncData, nIndex + 1, szCmdBuffer, nCmdBuffSize, m_dwSndTimeOut,
                            ConvertCode_Impl2Str(nRet), ConvertCode_Impl2Str(nRet));
                        return nRet;
                    }
                }
                /*if (bIsPrtLog == TRUE)
                {
                    Log(ThisModule, __LINE__,
                        "%s: 单帧命令下发第[%d]次: ->Send(%s, %d, %d) Fail, ErrCode: %s, "
                        "设定为设备问题, 重新Open...",
                        lpFuncData, nIndex + 1, szCmdBuffer, nCmdBuffSize, m_dwSndTimeOut,
                        ConvertCode_Impl2Str(nRet));
                }

                m_pDev->Close();
                INT nRetTemp = m_pDev->Open(m_strMode.c_str());
                if ((nRetTemp == ERR_DEVPORT_SUCCESS) && (nIndex < 3))
                {
                    if (bIsPrtLog == TRUE)
                    {
                        Log(ThisModule, __LINE__,
                            "%s: 单帧命令下发第[%d]次失败: 重新Open：->Open(%s) Succ, 重新下发命令... ",
                            lpFuncData, nIndex + 1, m_strMode.c_str());
                    }
                    continue;
                }
                if (bIsPrtLog == TRUE)
                {
                    Log(ThisModule, __LINE__,
                        "%s: 单帧命令下发第[%d]次失败: 重新Open：->Open(%s) Fail, 超出重试次数上限[%d], Return: %s ",
                        lpFuncData, nIndex + 1, m_strMode.c_str(), 4, ConvertCode_Impl2Str(nRet));
                }
                return nRet;*/
            }
        } else
        {
            break;
        }
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 下发多帧数据到Device
 * 参数: lpcCmd 命令
 *      lpcCmdPar 命令数据
 *      nParLen   命令数据长度
 *      lpFuncData 日志输出字串
 *      bIsPrtLog 是否打印日志
 * 返回值: 参IMPL考错误码
************************************************************/
INT CDevImpl_CRT350N::SendMultiPacket(LPCSTR lpcCmd, LPCSTR lpcCmdPar, INT nParLen,
                                      LPCSTR lpFuncData, BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR szCmdBuffer[2068] = { 0x00 };              // 下发命令
    INT nRet = ERR_DEVPORT_SUCCESS;
    INT nPackCount = 0;                             // 分包计数
    BYTE dwReportID = { 0x00 };
    INT nSndSize = 0;                               // 下发包长度

    // 发送的数据格式
    // ReportID(1 byte) + LEN(2 byte) + CMD(2 byte) + TEXT(58 byte)

    // 一帧包头
    dwReportID = REPORTID;
    szCmdBuffer[0] = (CHAR)dwReportID;              // 包头标识
    szCmdBuffer[1] = (CHAR)((nParLen + 3) / 256);   // 包长度高位字节
    szCmdBuffer[2] = (CHAR)((nParLen + 3) % 256);   // 包长度低位字节

    // TEXT命令
    memcpy(szCmdBuffer + 3, lpcCmd, 3);             // 追加命令

    // 分包
    nPackCount = 1 + nParLen / CRT_PACK_MAX_CMP_LEN;

    for (INT nWriteIdx = 0; nWriteIdx < nPackCount; nWriteIdx++)
    {
        int nbufOffset = nWriteIdx * CRT_PACK_MAX_LEN; //64
        int nlpDataOffset = nWriteIdx * CRT_PACK_MAX_CMP_LEN; //59

        for (INT nIndex = 0; nIndex < 4; nIndex++)
        {
            if (nWriteIdx == 0)                     // 第一个包处理
            {
                nSndSize = CRT_PACK_MAX_LEN;
                memcpy(szCmdBuffer + 6, lpcCmdPar, CRT_PACK_MAX_CMP_LEN);
                nRet = m_pDev->Send(szCmdBuffer, CRT_PACK_MAX_LEN, m_dwSndTimeOut);
            } else
            if (nWriteIdx == nPackCount - 1)        // 最后一个包处理
            {
                nSndSize = CRT_PACK_MAX_LEN;
                INT overageLen = nParLen - nWriteIdx * CRT_PACK_MAX_CMP_LEN;
                memcpy(szCmdBuffer + nbufOffset, szCmdBuffer, 6);
                memcpy(szCmdBuffer + nbufOffset + 6, lpcCmdPar + nlpDataOffset, overageLen);
                nSndSize = overageLen + 6;
                nRet = m_pDev->Send(szCmdBuffer, overageLen + 6, m_dwSndTimeOut);
            } else                                  // 中间发送包处理
            {
                memcpy(szCmdBuffer + nbufOffset, szCmdBuffer, 6);
                memcpy(szCmdBuffer + nbufOffset + 6, lpcCmdPar + nlpDataOffset, CRT_PACK_MAX_CMP_LEN);
                nSndSize = CRT_PACK_MAX_LEN;
                nRet = m_pDev->Send(szCmdBuffer, CRT_PACK_MAX_LEN, m_dwSndTimeOut);
            }

            if (nRet != ERR_DEVPORT_SUCCESS)
            {
                if (nRet == ERR_DEVPORT_NOTOPEN || nRet == ERR_DEVPORT_WRITE)
                {
                    if (bIsPrtLog == TRUE)
                    {
                        if (nIndex < 3)
                        {
                            Log(ThisModule, __LINE__,
                                "%s: 多帧命令下发第[%d]帧第[%d]次: ->Send(%s, %d, %d) Fail, ErrCode: %s, "
                                "重新下发...",
                                lpFuncData, nWriteIdx + 1, nIndex + 1, szCmdBuffer, nSndSize, m_dwSndTimeOut,
                                ConvertCode_Impl2Str(nRet));
                            continue;
                        } else
                        {
                            Log(ThisModule, __LINE__,
                                "%s: 多帧命令下发第[%d]帧第[%d]次: ->Send(%s, %d, %d) Fail, ErrCode: %s, "
                                "Return: %s.",
                                lpFuncData, nWriteIdx + 1, nIndex + 1, szCmdBuffer, nSndSize, m_dwSndTimeOut,
                                ConvertCode_Impl2Str(nRet), ConvertCode_Impl2Str(nRet));
                            return nRet;
                        }

                    }
                    /*if (bIsPrtLog == TRUE)
                    {
                        Log(ThisModule, __LINE__,
                            "%s: 多帧命令下发第[%d]次: ->Send(%s, %d, %d) Fail, ErrCode: %s, "
                            "设定为设备问题, 重新Open...",
                            lpFuncData, nIndex + 1, szCmdBuffer, nSndSize, m_dwSndTimeOut,
                            ConvertCode_Impl2Str(nRet));
                    }

                    m_pDev->Close();
                    INT nRetTemp = m_pDev->Open(m_strMode.c_str());
                    if ((nRetTemp == ERR_DEVPORT_SUCCESS) && (nIndex < 3))
                    {
                        if (bIsPrtLog == TRUE)
                        {
                            Log(ThisModule, __LINE__,
                                "%s: 多帧命令下发第[%d]次失败: 重新Open：->Open(%s) Succ, 重新执行设备初始化下发命令... ",
                                lpFuncData, nIndex + 1, m_strMode.c_str());
                        }
                        nWriteIdx = 0;
                        continue;
                    }
                    if (bIsPrtLog == TRUE)
                    {
                        Log(ThisModule, __LINE__,
                            "%s: 多帧命令下发第[%d]次失败: 重新Open：->Open(%s) Fail, 超出重试次数上限[%d], Return: %s ",
                            lpFuncData, nIndex + 1, m_strMode.c_str(), 4, ConvertCode_Impl2Str(nRet));
                    }*/
                    if (bIsPrtLog == TRUE)
                    {
                        Log(ThisModule, __LINE__,
                            "%s: 多帧命令下发第[%d]次: ->Send(%s, %d, %d) Fail, ErrCode: %s, "
                            "设定为设备问题, 重新Open...",
                            lpFuncData, nIndex + 1, szCmdBuffer, nSndSize, m_dwSndTimeOut,
                            ConvertCode_Impl2Str(nRet));
                    }

                    return nRet;
                }
            } else
            {
                break;
            }

            /*if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS)
            {
                if (bIsPrtLog == TRUE)
                {
                    Log(ThisModule, __LINE__,
                        "%s: 多帧命令下发:  ->Send(%s, %d, %d) Fail, ErrCode: %s, Not Return.",
                        lpFuncData, szCmdBuffer, nSndSize, m_dwSndTimeOut,
                        ConvertCode_Impl2Str(nRet));
                }
            }
            m_nLastError = nRet;
            if (nRet < 0)
            {
                if (nRet == ERR_DEVPORT_NOTOPEN || nRet == ERR_DEVPORT_WRITE)
                {
                    m_pDev->Close();
                    int nRetTemp = m_pDev->Open(m_strMode.c_str());
                    if ((nRetTemp == ERR_DEVPORT_SUCCESS) && (iIndex < 3))
                    {
                        m_bReCon = TRUE;
                        nWriteIdx = 0;
                        continue;
                    }
                }
                m_bReCon = FALSE; // 重连失败
                return ConvertCode_USB2Impl(nRet);
            } else
            {
                break;
            }*/
        }
    }

    return ConvertCode_USB2Impl(nRet);
}

/************************************************************
 * 功能：读取读卡器的返回数据
 * 参数：pszReponse返回数据的缓冲区
 *      nLen缓冲区长度
 *      lpFuncData 日志输出字串
 *      bIsPrtLog 是否打印日志
 * 返回：>0数据长度，<0错误
************************************************************/
INT CDevImpl_CRT350N::GetResponse(LPSTR lpResponse, INT nLen, LPCSTR lpFuncData, BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    Q_UNUSED(nLen)

    CHAR szReply[64];
    INT nRet = 0;
    INT nIndex = 0;
    INT nTimeOut = m_dwRcvTimeOut;

    if (m_pDev == nullptr)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__, "%s: m_pDev == NULL, USB动态库句柄无效, Return: %d",
                ConvertCode_Impl2Str(IMP_ERR_DEVPORT_NOTOPEN));
        }
        return ERR_DEVPORT_NOTOPEN;
    }

    while (TRUE)
    {
        DWORD ulInOutLen = USB_READ_LENTH;
        MSET_0(szReply);
        nRet = m_pDev->Read(szReply, ulInOutLen, nTimeOut);
        if (nRet <= ERR_DEVPORT_SUCCESS)
        {
            break;
        } else
        {
            if (nIndex == 0)
            {
               // 第一个包
               nIndex = nRet;
               memcpy(lpResponse, szReply, nIndex);
               nTimeOut = 50;
            } else
            {
                // 多个包
                memcpy(lpResponse + nIndex, szReply + 1, nRet - 1);
                nIndex += nRet - 1;
            }
            if (szReply[0] == 0x04)
            {
                break;
            }
            continue;
        }
    }

    DWORD nRecvLen = MAKEWORD(lpResponse[2], lpResponse[1]);
    if (nRecvLen != 0)
    {
        if (nIndex < nRecvLen + 3)
        {
            if (bIsPrtLog == TRUE)
            {
                Log(ThisModule, __LINE__,
                    "%s : 数据接收: 检查完整性: ->Read() 返回长度[%d], 解析数据记录长度[%d+3], 不一致, Return: %s",
                    lpFuncData, nIndex, nRecvLen, ConvertCode_Impl2Str(IMP_ERR_RCVDATA_NOTCOMP));
            }
            return IMP_ERR_RCVDATA_NOTCOMP;
        }
        return nRecvLen + 3;
    } else
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "%s : 数据接收: ->Read() 返回数据解析长度=[0], 问题数据, Return: %s",
                lpFuncData, ConvertCode_Impl2Str(IMP_ERR_READERROR));
        }
        return IMP_ERR_READERROR;
    }
}

/************************************************************
 * 功能: 命令收发及检查
 * 参数:
 * 返回值: IMPL错误码
************************************************************/
INT CDevImpl_CRT350N::SndRcvToChk(LPCSTR lpcSndCmd, LPCSTR lpcSndPar, INT nParSize,
                                  LPSTR lpRcvData, INT &nRcvSize, LPCSTR lpPrtData,
                                  BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //LOGDEVACTION();

    INT nRet = IMP_SUCCESS;

    // 命令下发
    nRet = SendCmd(lpcSndCmd, lpcSndPar, nParSize, lpPrtData, bIsPrtLog);
    if (nRet != IMP_SUCCESS)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__, "%s: ->SendCmd(%s, %s, %d, %s)失败, ErrCode: %d, Return %s.",
                lpPrtData, CmdToStr(LPSTR(lpcSndCmd)), lpcSndPar == nullptr ? "NULL" : lpcSndPar,
                nParSize, lpPrtData, nRet, ConvertCode_Impl2Str(nRet));
        }

        return nRet;
    }

    // 接收应答
    nRet = GetResponse(lpRcvData, nRcvSize, lpPrtData, bIsPrtLog);
    if (nRet < 0)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "%s: 命令<%s>应答: ->GetResponse(%s, %d, %s)失败, ErrCode: %d, Return %s.",
                lpPrtData, CmdToStr(LPSTR(lpcSndCmd)), lpRcvData, nRcvSize, lpPrtData,
                nRet, ConvertCode_Impl2Str(nRet));
        }
        return nRet;
    }
    nRcvSize = nRet;

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(lpcSndCmd, lpRcvData, nRcvSize, lpPrtData, bIsPrtLog);
    if (nRet2 != IMP_SUCCESS)
    {
        return nRet2;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 应答数据Check
 * 参数:
 * 返回值: IMPL错误码
************************************************************/
INT CDevImpl_CRT350N::RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT nRcvDataLen,
                                   LPCSTR lpPrtData, BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // RcvData: ReqId(1byte) + Length(2Byte) + P/N + CMD(2Byte) + Stat(2Byte)

    LPCSTR lpcStr = nullptr;

    if (nRcvDataLen < RCV_DATA_POS)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "%s: 应答数据Check: CMD<%s> 返回命令应答数据长度[%d]<%d, Return %s.",
                lpPrtData, CmdToStr(LPSTR(lpcSndCmd)), nRcvDataLen, RCV_DATA_POS,
                ConvertCode_Impl2Str(IMP_ERR_RCVDATA_INVALID));
        }

        return IMP_ERR_RCVDATA_INVALID;
    }

    lpcStr = lpcRcvData + RCV_CMD_START_POS;
//    if (memcmp(lpcSndCmd + 1, lpcStr + 1 , STAND_CMD_LENGHT - 1) != 0)
//    {
//        if (bIsPrtLog == TRUE)
//        {
//            Log(ThisModule, __LINE__,
//                "%s: 应答数据Check: CMD<%s> 返回无效的命令应答数据[%02X,%02X,%02X], Return %s.",
//                lpPrtData, CmdToStr(LPSTR(lpcSndCmd)), (int)(lpcStr[0]), (int)(lpcStr[1]),
//                (int)(lpcStr[2]), ConvertCode_Impl2Str(IMP_ERR_RCVDATA_INVALID));
//        }

//        return IMP_ERR_RCVDATA_INVALID;
//    }

    if (memcmp(lpcStr, "N", 1) == 0) // 错误应答
    {
        CHAR szErrCode[2+1] = { 0x00 };
        memcpy(szErrCode, lpcStr + STAND_CMD_LENGHT, 2);
        Log(ThisModule, __LINE__,
            "%s: 应答数据Check: CMD<%s> 返回错误应答数据[%02X,%02X,%02X,%02X,%02X], Return %s.",
            lpPrtData, CmdToStr(LPSTR(lpcSndCmd)), (int)(lpcStr[0]), (int)(lpcStr[1]),
            (int)(lpcStr[2]), (int)(lpcStr[3]), (int)(lpcStr[4]),
            ConvertCode_Impl2Str(ConvertCode_Dev2Impl(szErrCode)));
        return ConvertCode_Dev2Impl(szErrCode);
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 应答数据状态转换
 * 参数:
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT350N::RcvCardStatChg(LPCSTR lpStat)
{
    if (MCMP_IS0(lpStat, CARDER_STAT_NOCARD))           // 无卡
    {
        return IMPL_STAT_CARD_NOTHAVE;
    } else
    if (MCMP_IS0(lpStat, CARDER_STAT_CARD_IS_EXPORT))   // 卡在出口
    {
        return IMPL_STAT_CARD_ISEXPORT;
    } else
    if (MCMP_IS0(lpStat, CARDER_STAT_CARD_IS_INSIDE))   // 卡在内部
    {
        return IMPL_STAT_CARD_ISINSIDE;
    } else                                              // 无效值
    {
        return IMPL_STAT_CARD_INVALID;
    }
}

//----------------------------------对外参数设置接口----------------------------------
// 设置断线重连标记
INT CDevImpl_CRT350N::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

// 设置动态库路径(DeviceOpen前有效)
INT CDevImpl_CRT350N::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径

    return IMP_SUCCESS;
}

// 设置进卡检查模式
INT CDevImpl_CRT350N::SetPredictIC(WORD wPreIC)
{
    THISMODULE(__FUNCTION__);

    if (wPreIC >= 0 && wPreIC < 3)
    {
        m_wPredictIC = wPreIC;
        Log(ThisModule, __LINE__, "设置进卡检查模式: %d.", wPreIC);
    } else
    {
        m_wPredictIC = 0;
        Log(ThisModule, __LINE__, "设置进卡检查模式: 入参[%d] 不在 0~2 范围, 使用默认值: 0.",
            m_wPredictIC);
    }

    return IMP_SUCCESS;
}

// 设置命令收发超时时间
INT CDevImpl_CRT350N::SetSndRcvTimeOut(DWORD dwSnd, DWORD dwRcv)
{
    THISMODULE(__FUNCTION__);

    if (dwSnd < TIMEOUT_WAIT_ACTION)
    {
        Log(ThisModule, __LINE__, "设置命令收发超时时间: 命令下发超时[%d] < 缺省超时[%d], 使用缺省值.",
            dwSnd, TIMEOUT_WAIT_ACTION);
        m_dwSndTimeOut = TIMEOUT_WAIT_ACTION;
    } else
    {
        m_dwSndTimeOut = dwSnd;
        Log(ThisModule, __LINE__, "设置命令收发超时时间: 命令下发超时 = %d 毫秒.", dwSnd);
    }

    if (dwRcv < TIMEOUT_WAIT_ACTION)
    {
        Log(ThisModule, __LINE__, "设置命令收发超时时间: 命令接收超时[%d] < 缺省超时[%d], 使用缺省值.",
            dwRcv, TIMEOUT_WAIT_ACTION);
        m_dwRcvTimeOut = TIMEOUT_WAIT_ACTION;
    } else
    {
        m_dwRcvTimeOut = dwRcv;
        Log(ThisModule, __LINE__, "设置命令收发超时时间: 命令接收超时 = %d 毫秒.", m_dwRcvTimeOut);
    }

    return IMP_SUCCESS;
}

// 命令转换为解释字符串
CHAR* CDevImpl_CRT350N::CmdToStr(LPSTR lpCmd)
{
#define CRT350N_CMD2STR(c1, c2, s) \
    if (MCMP_IS0(c1, c2)) \
    { \
        return s; \
    } else

    memset(m_szCmdStr, 0x00, sizeof(m_szCmdStr));

    // 初始化
    CRT350N_CMD2STR(lpCmd, SND_CMD_INIT_C00, "C00|有卡时弹到前端")
    CRT350N_CMD2STR(lpCmd, SND_CMD_INIT_C01, "C01|有卡时吞卡")
    CRT350N_CMD2STR(lpCmd, SND_CMD_INIT_C02, "C02|有卡时重进卡")
    CRT350N_CMD2STR(lpCmd, SND_CMD_INIT_C03, "C03|有卡时不移动卡")
    CRT350N_CMD2STR(lpCmd, SND_CMD_INIT_C04, "C04|同C00，但回收计数器工作")
    CRT350N_CMD2STR(lpCmd, SND_CMD_INIT_C05, "C05|同C01，但回收计数器工作")
    CRT350N_CMD2STR(lpCmd, SND_CMD_INIT_C06, "C06|同C02，但回收计数器工作")
    CRT350N_CMD2STR(lpCmd, SND_CMD_INIT_C07, "C07|同C03，但回收计数器工作")
    // 状态查询
    CRT350N_CMD2STR(lpCmd, SND_CMD_STAT_C10, "C10|报告是否有卡和卡的位置")
    CRT350N_CMD2STR(lpCmd, SND_CMD_STAT_C11, "C11|报告传感器状态")
    CRT350N_CMD2STR(lpCmd, SND_CMD_STAT_C12, "C12|报告传感器状态(扩展)")
    CRT350N_CMD2STR(lpCmd, SND_CMD_STAT_C13, "C13|报告传感器状态(扩展)")
    CRT350N_CMD2STR(lpCmd, SND_CMD_STAT_C14, "C14|报告传感器状态(扩展)")
    CRT350N_CMD2STR(lpCmd, SND_CMD_STAT_C1H40, "C1@|报告传感器状态(扩展)")
    // 同步进卡
    CRT350N_CMD2STR(lpCmd, SND_CMD_SYNC_INCARD_C20, "C20|前端同步进卡:不检测磁信号")
    CRT350N_CMD2STR(lpCmd, SND_CMD_SYNC_INCARD_C21, "C21|前端同步进卡:检测ISO磁道2,磁道3磁信号")
    CRT350N_CMD2STR(lpCmd, SND_CMD_SYNC_INCARD_C22, "C22|后端进卡:后端同步等待卡插入")
    CRT350N_CMD2STR(lpCmd, SND_CMD_SYNC_INCARD_C23, "C23|前端同步进IC卡:等待进IC卡")
    CRT350N_CMD2STR(lpCmd, SND_CMD_SYNC_INCARD_C24, "C24|前端同步进IC卡&磁卡:等待进IC卡&磁卡")
    CRT350N_CMD2STR(lpCmd, SND_CMD_SYNC_INCARD_C2H3D, "C2=|前端同步进IC卡或磁卡:等待进IC卡或磁卡")
    // 卡移动
    CRT350N_CMD2STR(lpCmd, SND_CMD_CARD_MOVE_C30, "C30|弹卡:将卡移到前端持卡位")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CARD_MOVE_C31, "C31|吞卡:将卡从读卡器后部回收")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CARD_MOVE_C32, "C32|设置MM:将卡移到MM起始位")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CARD_MOVE_C37, "C37|退卡:将卡移到前端不持卡位")
    // 重进卡
    CRT350N_CMD2STR(lpCmd, SND_CMD_RE_INCARD_C40, "C40|将卡从前端移动到卡内部")
    // 读磁卡
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C60, "C60|移动卡:移动读卡")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C61, "C61|读ISO磁道#1:传输ISO磁道#1数据")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C62, "C62|读ISO磁道#2:传输ISO磁道#2数据")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C63, "C63|读ISO磁道#3:传输ISO磁道#3数据")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C65, "C65|数据读所有磁道:传输所有磁道数据")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C66, "C66|清除磁卡缓存:清除缓存的读写磁卡数据")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C67, "C67|读磁道状态:磁道数据缓冲区状态")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C69, "C69|ISO磁道#1其他方式读:传输ISO磁道#1数据")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C6H3A, "C6:|ISO磁道#2其他方式读:传输ISO磁道#2数据")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_CARD_C6H3B, "C6;|ISO磁道#3其他方式读:传输ISO磁道#3数据")
    // 异步进卡控制
    CRT350N_CMD2STR(lpCmd, SND_CMD_ASYNC_INCARD_CH3A0 ,"C:0|使能进卡,不检测磁信号(所有卡)")
    CRT350N_CMD2STR(lpCmd, SND_CMD_ASYNC_INCARD_CH3A1 ,"C:1|禁止进卡")
    CRT350N_CMD2STR(lpCmd, SND_CMD_ASYNC_INCARD_CH3A2 ,"C:2|使能进卡,检测磁信号(只允许磁卡)")
    CRT350N_CMD2STR(lpCmd, SND_CMD_ASYNC_INCARD_CH3A3 ,"C:3|使能进卡,检测IC芯片")
    CRT350N_CMD2STR(lpCmd, SND_CMD_ASYNC_INCARD_CH3A4 ,"C:4|使能进卡,检查IC芯片和磁信号")
    CRT350N_CMD2STR(lpCmd, SND_CMD_ASYNC_INCARD_CH3AD ,"C:=|使能进卡,检查IC芯片或磁信号")
    CRT350N_CMD2STR(lpCmd, SND_CMD_ASYNC_INCARD_CH3AX ,"C:X|抖动进卡设置")
    // IC卡/RF卡走位
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_PRESS, "C@0|走位/触点接触")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_RELEASE, "C@2|触点释放")
    // 版本号
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_VER_CA1, "CA1|读版本号:读取应用程序版本号")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_VER_CA2, "CA2|读版本号:读取EMV2000版本")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_VER_CA3, "CA3|读版本号:读取EMV证书编号")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_VER_CA4, "CA4|读版本号:读取GIE-CB 证书编号")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_VER_CA5, "CA5|读版本号:读取IFM编号")
    CRT350N_CMD2STR(lpCmd, SND_CMD_READ_VER_CA6, "CA6|读版本号:读取ICC控制器版本")
    // 回收计数
    CRT350N_CMD2STR(lpCmd, SND_CMD_GET_RETAINCNT, "CC0|读取回收计数")
    CRT350N_CMD2STR(lpCmd, SND_CMD_SET_RETAINCNT, "CC1|设置回收计数")
    // IC卡/SAM卡控制
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_ACTION_CI0, "CI0|芯片卡激活")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_DEACTION_CI1, "CI1|芯片卡关闭")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_GETSTAT_CI2, "CI2|芯片卡状态查询")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_COMMT0_CI3, "CI3|芯片卡T0通讯")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_COMMT1_CI4, "CI4|芯片卡T1通讯")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_COMMKZ1_CI5, "CI5|芯片卡通讯扩展1(发送命令1000Byte及以下)")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_COMMKZ2_CI6, "CI6|芯片卡通讯扩展2(接收应答1000Byte及以下)")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_COMMKZ3_CI7, "CI7|芯片卡通讯扩展3(接收应答1000Byte以上)")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_RESETWARM_CI8, "CI8|芯片卡热复位")
    CRT350N_CMD2STR(lpCmd, SND_CMD_CHIP_COMMAUTO_CI9, "CI9|芯片卡自动通讯")
    // 切换
    // 存储卡控制
    // 监控取卡
    // I2C接口存储卡控制
    // 多磁道读
    // 日志
    // AT88SC102卡控制
    // ESU(磁干扰)
    // 防盗钩
    CRT350N_CMD2STR(lpCmd, SND_CMD_TAMPER_GETSTAT_Cc8  ,"Cc8|获取防盗钩状态")
    CRT350N_CMD2STR(lpCmd, SND_CMD_TAMPER_SETWORK_Cc9  ,"Cc9|设置防盗钩工作模式")
    CRT350N_CMD2STR(lpCmd, SND_CMD_TAMPER_CONT_CcH3A   ,"Cc:|控制防盗钩动作")
    {
        sprintf(m_szErrStr, "%s|%s", lpCmd, "未知命令");
        return m_szErrStr;
    }
}

// USB处理错误值转换为Impl返回码/错误码
INT CDevImpl_CRT350N::ConvertCode_USB2Impl(long lRet)
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

// 硬件错误值转换为Impl返回码/错误码
INT CDevImpl_CRT350N::ConvertCode_Dev2Impl(CHAR szDeviceErr[3])
{
#define CRT350N_CONV_DEVCODE2IMPL(s, d, r) \
    if (MCMP_IS0(s, d)) \
        return r; \
    else

    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "00", IMP_ERR_DEVRET_00)    // 命令编码未定义
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "01", IMP_ERR_DEVRET_01)    // 命令参数错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "02", IMP_ERR_DEVRET_02)    // 命令无法执行
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "03", IMP_ERR_DEVRET_03)    // 硬件不支持
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "04", IMP_ERR_DEVRET_04)    // 命令数据错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "05", IMP_ERR_DEVRET_05)    // IC触点未释放
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "06", IMP_ERR_DEVRET_06)    // 密钥不存在
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "07", IMP_ERR_DEVRET_07)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "08", IMP_ERR_DEVRET_08)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "09", IMP_ERR_DEVRET_09)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "10", IMP_ERR_DEVRET_10)    // 卡堵塞
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "11", IMP_ERR_DEVRET_11)    // Shutter错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "12", IMP_ERR_DEVRET_12)    // 传感器错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "13", IMP_ERR_DEVRET_13)    // 不规则卡长度(过长)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "14", IMP_ERR_DEVRET_14)    // 不规则卡长度(过短)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "15", IMP_ERR_DEVRET_15)    // FRAM错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "16", IMP_ERR_DEVRET_16)    // 卡位置移动
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "17", IMP_ERR_DEVRET_17)    // 重进卡时卡堵塞
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "18", IMP_ERR_DEVRET_18)    // SW1,SW2错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "19", IMP_ERR_DEVRET_19)    // 卡没有从后端插入
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "20", IMP_ERR_DEVRET_20)    // 读磁卡错误(奇偶校验错)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "21", IMP_ERR_DEVRET_21)    // 读磁卡错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "22", IMP_ERR_DEVRET_22)    // 写磁卡错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "23", IMP_ERR_DEVRET_23)    // 读磁卡错误(没有数据内容,只有STX起始符,ETX结束符和LRC)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "24", IMP_ERR_DEVRET_24)    // 读磁卡错误(没有磁条或没有编码-空白轨道)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "25", IMP_ERR_DEVRET_25)    // 写磁卡校验错误(品质错误)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "26", IMP_ERR_DEVRET_26)    // 读磁卡错误(没有SS)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "27", IMP_ERR_DEVRET_27)    // 读磁卡错误(没有ES)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "28", IMP_ERR_DEVRET_28)    // 读磁卡错误(LRC错误)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "29", IMP_ERR_DEVRET_29)    // 写磁卡校验错误(数据不一致)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "30", IMP_ERR_DEVRET_30)    // 电源掉电
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "31", IMP_ERR_DEVRET_31)    // DSR信号为OFF
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "32", IMP_ERR_DEVRET_32)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "40", IMP_ERR_DEVRET_40)    // 吞卡时卡拔走
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "41", IMP_ERR_DEVRET_41)    // IC触点或触点传感器错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "42", IMP_ERR_DEVRET_42)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "43", IMP_ERR_DEVRET_43)    // 无法走到IC卡位
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "44", IMP_ERR_DEVRET_44)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "45", IMP_ERR_DEVRET_45)    // 卡机强制弹卡
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "46", IMP_ERR_DEVRET_46)    // 前端卡未在指定时间内取走
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "47", IMP_ERR_DEVRET_47)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "48", IMP_ERR_DEVRET_48)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "49", IMP_ERR_DEVRET_49)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "50", IMP_ERR_DEVRET_50)    // 回收卡计数溢出
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "51", IMP_ERR_DEVRET_51)    // 马达错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "52", IMP_ERR_DEVRET_52)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "53", IMP_ERR_DEVRET_53)    // 数字解码读错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "54", IMP_ERR_DEVRET_54)    // 防盗钩移动错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "55", IMP_ERR_DEVRET_55)    // 防盗钩已经设置,命令不能执行
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "56", IMP_ERR_DEVRET_56)    // 芯片检测传感器错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "57", IMP_ERR_DEVRET_57)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "58", IMP_ERR_DEVRET_58)    // 防盗钩正在移动
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "59", IMP_ERR_DEVRET_59)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "60", IMP_ERR_DEVRET_60)    // IC卡或SAM卡Vcc条件异常
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "61", IMP_ERR_DEVRET_61)    // IC卡或SAM卡ATR通讯错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "62", IMP_ERR_DEVRET_62)    // IC卡或SAM卡在当前激活条件下ATR无效
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "63", IMP_ERR_DEVRET_63)    // IC卡或SAM卡通讯过程中无响应
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "63", IMP_ERR_DEVRET_64)    // IC卡或SAM卡通讯错误(除无响应外)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "65", IMP_ERR_DEVRET_65)    // IC卡或SAM卡未激活
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "66", IMP_ERR_DEVRET_66)    // IC卡或SAM卡不支持(仅对于非EMV激活)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "67", IMP_ERR_DEVRET_67)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "68", IMP_ERR_DEVRET_68)    //
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "69", IMP_ERR_DEVRET_69)    // IC卡或SAM卡不支持(仅对于EMV激活)
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "76", IMP_ERR_DEVRET_76)    // ESU模块和卡机通讯错误
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "95", IMP_ERR_DEVRET_95)    // ESU模块损坏或无连接
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "99", IMP_ERR_DEVRET_99)    // ESU模块过流
    CRT350N_CONV_DEVCODE2IMPL(szDeviceErr, "B0", IMP_ERR_DEVRET_B0)    // 未接收到初始化命令
    return IMP_SUCCESS;
}

// Impl错误码转换解释字符串
LPSTR CDevImpl_CRT350N::ConvertCode_Impl2Str(INT nErrCode)
{
#define CRT350_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nErrCode)
    {
        // > 100: Impl处理返回
        CRT350_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功")
        CRT350_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载失败")
        CRT350_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效")
        CRT350_CASE_CODE_STR(IMP_ERR_READERROR, nErrCode, "读数据错误")
        CRT350_CASE_CODE_STR(IMP_ERR_WRITEERROR, nErrCode, "写数据错误")
        CRT350_CASE_CODE_STR(IMP_ERR_RCVDATA_INVALID, nErrCode, "无效的应答数据")
        CRT350_CASE_CODE_STR(IMP_ERR_RCVDATA_NOTCOMP, nErrCode, "返回数据不完整")
        CRT350_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误")
        // <0 : USB/COM接口处理返回
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_NOTOPEN, nErrCode, "USB/COM: 没打开")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_FAIL, nErrCode, "USB/COM: 通讯错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_PARAM, nErrCode, "USB/COM: 参数错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_CANCELED, nErrCode, "USB/COM: 操作取消")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_READERR, nErrCode, "USB/COM: 读取错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_WRITE, nErrCode, "USB/COM: 发送错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_RTIMEOUT, nErrCode, "USB/COM: 读操作超时")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_WTIMEOUT, nErrCode, "USB/COM: 写操作超时")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_LIBRARY, nErrCode, "USB/COM: 加载通讯库失败")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVPORT_NODEFINED, nErrCode, "USB/COM: 未知错误")
        // 0~100: 硬件设备返回
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_00, nErrCode, "硬件返回: 命令编码未定义")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_01, nErrCode, "硬件返回: 命令参数错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_02, nErrCode, "硬件返回: 命令无法执行")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_03, nErrCode, "硬件返回: 硬件不支持")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_04, nErrCode, "硬件返回: 命令数据错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_05, nErrCode, "硬件返回: IC触点未释放")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_06, nErrCode, "硬件返回: 密钥不存在")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_07, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_08, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_09, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_10, nErrCode, "硬件返回: 卡堵塞")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_11, nErrCode, "硬件返回: Shutter错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_12, nErrCode, "硬件返回: 传感器错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_13, nErrCode, "硬件返回: 不规则卡长度(过长)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_14, nErrCode, "硬件返回: 不规则卡长度(过短)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_15, nErrCode, "硬件返回: FRAM错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_16, nErrCode, "硬件返回: 卡位置移动")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_17, nErrCode, "硬件返回: 重进卡时卡堵塞")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_18, nErrCode, "硬件返回: SW1,SW2错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_19, nErrCode, "硬件返回: 卡没有从后端插入")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_20, nErrCode, "硬件返回: 读磁卡错误(奇偶校验错)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_21, nErrCode, "硬件返回: 读磁卡错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_22, nErrCode, "硬件返回: 写磁卡错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_23, nErrCode, "硬件返回: 读磁卡错误(没有数据内容,只有STX起始符,ETX结束符和LRC)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_24, nErrCode, "硬件返回: 读磁卡错误(没有磁条或没有编码-空白轨道)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_25, nErrCode, "硬件返回: 写磁卡校验错误(品质错误)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_26, nErrCode, "硬件返回: 读磁卡错误(没有SS)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_27, nErrCode, "硬件返回: 读磁卡错误(没有ES)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_28, nErrCode, "硬件返回: 读磁卡错误(LRC错误)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_29, nErrCode, "硬件返回: 写磁卡校验错误(数据不一致)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_30, nErrCode, "硬件返回: 电源掉电")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_31, nErrCode, "硬件返回: DSR信号为OFF")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_32, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_40, nErrCode, "硬件返回: 吞卡时卡拔走")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_41, nErrCode, "硬件返回: IC触点或触点传感器错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_42, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_43, nErrCode, "硬件返回: 无法走到IC卡位")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_44, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_45, nErrCode, "硬件返回: 卡机强制弹卡")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_46, nErrCode, "硬件返回: 前端卡未在指定时间内取走")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_47, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_48, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_49, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_50, nErrCode, "硬件返回: 回收卡计数溢出")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_51, nErrCode, "硬件返回: 马达错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_52, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_53, nErrCode, "硬件返回: 数字解码读错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_54, nErrCode, "硬件返回: 防盗钩移动错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_55, nErrCode, "硬件返回: 防盗钩已经设置,命令不能执行")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_56, nErrCode, "硬件返回: 芯片检测传感器错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_57, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_58, nErrCode, "硬件返回: 防盗钩正在移动")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_59, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_60, nErrCode, "硬件返回: IC卡或SAM卡Vcc条件异常")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_61, nErrCode, "硬件返回: IC卡或SAM卡ATR通讯错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_62, nErrCode, "硬件返回: IC卡或SAM卡在当前激活条件下ATR无效")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_63, nErrCode, "硬件返回: IC卡或SAM卡通讯过程中无响应")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_64, nErrCode, "硬件返回: IC卡或SAM卡通讯错误(除无响应外)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_65, nErrCode, "硬件返回: IC卡或SAM卡未激活")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_66, nErrCode, "硬件返回: IC卡或SAM卡不支持(仅对于非EMV激活)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_67, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_68, nErrCode, "硬件返回: 未知")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_69, nErrCode, "硬件返回: IC卡或SAM卡不支持(仅对于EMV激活)")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_76, nErrCode, "硬件返回: ESU模块和卡机通讯错误")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_95, nErrCode, "硬件返回: ESU模块损坏或无连接")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_99, nErrCode, "硬件返回: ESU模块过流")
        CRT350_CASE_CODE_STR(IMP_ERR_DEVRET_B0, nErrCode, "硬件返回: 未接收到初始化命令")
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}

//---------------------------------------------
