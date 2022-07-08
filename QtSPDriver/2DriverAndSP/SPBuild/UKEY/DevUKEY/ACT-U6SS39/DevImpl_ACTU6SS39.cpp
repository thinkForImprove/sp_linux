/***************************************************************
* 文件名称：DevImpl_CRT780B.cpp
* 文件描述：CRT780B退卡模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/
#include "DevImpl_ACTU6SS39.h"

static const char *ThisModule = "DevImpl_ACTU6SS39.cpp";

CDevImpl_ACTU6SS39::CDevImpl_ACTU6SS39()
{
    SetLogFile(LOG_NAME, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_ACTU6SS39::CDevImpl_ACTU6SS39(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_ACTU6SS39::Init()
{
    m_bDevOpenOk = FALSE;    
    m_uiSndTimeOut = TIMEOUT_SNDCMD;
    m_uiRcvTimeOut = TIMEOUT_RCVCMD;

    memset(m_nGetStatRetList, 0, sizeof(INT) * 8);
    memset(m_szGetStatRetList, 0x00, sizeof(CHAR) * (8 * 32));
}

CDevImpl_ACTU6SS39::~CDevImpl_ACTU6SS39()
{

}

/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参　格式: USB:VID,PID  VID/PID为4位16进制字符串
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    LOGDEVACTION();

    if (m_pDev == nullptr)
    {
        if (m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "CRD", "DevCRD_ACTU6SS39") != 0)
        {
            Log(ThisModule, __LINE__, "加载动态库(AllDevPort.dll) Fail, Return: %s",
                ConvertErrCodeToStr(IMP_ERR_LOAD_LIB));
            return IMP_ERR_LOAD_LIB;
        }
    }

    // 入参为null
    if (lpMode == nullptr || strlen(lpMode) < 1)
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

    // 区分连接标记
    if (m_strMode.compare(0, 3, "USB", 3) == 0)
    {
        m_wSignalCom = 0;               // 通讯方式(0USB)
    } else
    if (m_strMode.compare(0, 3, "COM", 3) == 0)
    {
        m_wSignalCom = 1;               // 通讯方式(1COM)
    }

    m_bDevOpenOk = TRUE;

    Log(ThisModule, 1, "Open Device [%s] Success. ", m_strMode.c_str());

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    LOGDEVACTION();

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
INT CDevImpl_ACTU6SS39::Release()
{
    return DeviceClose();
}

BOOL CDevImpl_ACTU6SS39::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/************************************************************
 * 功能: 设置收发报文超时时间(单位:毫秒)
 * 参数: uiSndTimeOut 入参 下发命令超时(0不设置,采用默认值)
 *      uiRcvTimeOut 入参 接收命令超时(0不设置,采用默认值)
 * 返回值: 无
************************************************************/
void CDevImpl_ACTU6SS39::SetTimeOut(UINT uiSndTimeOut, UINT uiRcvTimeOut)
{
    if (uiSndTimeOut > 0)
    {
        m_uiSndTimeOut = uiSndTimeOut;
    }

    if (uiRcvTimeOut > 0)
    {
        m_uiRcvTimeOut = uiRcvTimeOut;
    }
}

/************************************************************
 * 功能: 1. 设备复位
 * 参数: usMode 初始化方式(0/1/2/4)　参考宏定义
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::DeviceReset(USHORT usMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvDataLen = 2068;

    UINT uiRcvTimeOut = TIMEOUT_RCVCMD;

    // Check 复位参数
    if (usMode == MODE_RESET_NOACTIVE)        // 复位无动作
    {
        memcpy(szSndCmd, S_CMD_RESET_NOACTIVE, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_RESET_RETAIN)          // 复位回收
    {
        memcpy(szSndCmd, S_CMD_RESET_RETAIN, STAND_CMD_LENGHT);
        uiRcvTimeOut = (m_uiRcvTimeOut  > TIMEOUT_RESET ? m_uiRcvTimeOut : TIMEOUT_RESET);
    } else
    if (usMode == MODE_RESET_EJECT)           // 复位弹出
    {
        memcpy(szSndCmd, S_CMD_RESET_EJECT, STAND_CMD_LENGHT);
        uiRcvTimeOut = (m_uiRcvTimeOut  > TIMEOUT_RESET ? m_uiRcvTimeOut : TIMEOUT_RESET);
    } else
    if (usMode == MODE_RESET_TRANS)           // 复位传动机构
    {
        memcpy(szSndCmd, S_CMD_RESET_TRANS, STAND_CMD_LENGHT);
        uiRcvTimeOut = (m_uiRcvTimeOut  > TIMEOUT_RESET ? m_uiRcvTimeOut : TIMEOUT_RESET);
    } else
    {
        Log(ThisModule, __LINE__, "入参为无效[%d], != %d/%d/%d/%d, Return: %s.", usMode,
            MODE_RESET_NOACTIVE, MODE_RESET_RETAIN, MODE_RESET_EJECT, MODE_RESET_TRANS,
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;       // 无效入参
    }

    // 下发命令
    nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "设备复位",
                       uiRcvTimeOut, m_uiSndTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "介质移动: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
            CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 2. 获取设备状态
 * 参数: usMode 状态类别(1/2/4)　参考宏定义
 * 　　　nStat　返回状态数组,如下: 各状态参考宏定义
 *             [0]:设备状态; [1]:箱1状态; [2]:箱2状态; [3]:回收箱状态;
 *             [4]:前端位置有无UK; [5]:扫描位置有无UK; 6~11位保留 *
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::GetDeviceStat(USHORT usMode, INT nStat[12])
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    // LOGDEVACTION();

    CHAR szTmp[12] = { 0x00 };
    std::string stdRcvDataOLD;
    std::string stdRcvDataNEW;

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvDataLen = 2068;
    CHAR    szGetData[64] = { 0x00 };

#define CHK_DEVSTAT(RET) \
    if (RET == ERR_DEVPORT_FAIL || RET == ERR_DEVPORT_RTIMEOUT || RET == ERR_DEVPORT_WTIMEOUT) \
    { \
        nStat[0] = DEV_STAT_OFFLINE; \
    } else \
    { \
        nStat[0] = DEV_STAT_HWERR; \
    }

#define MCMP_RCVDATA(IDX) \
    stdRcvDataNEW.clear(); stdRcvDataOLD.clear(); \
    for (int i = 0; i < nRcvDataLen; i ++) \
    { \
        sprintf(szTmp, "%02X ", (INT)szRcvData[i]); \
        stdRcvDataNEW.append(szTmp); \
    } \
    for (int i = 0; i < nRcvDataLen; i ++) \
    { \
        sprintf(szTmp, "%02X ", (INT)m_szGetStatRetList[IDX][i]); \
        stdRcvDataOLD.append(szTmp); \
    }\
    memcpy(m_szGetStatRetList[IDX], szRcvData, nRcvDataLen)

    // 设备状态(初始ONLINE)
    nStat[0] = DEV_STAT_ONLINE;
    nStat[1] = STAT_UNKNOWN;     // 箱1未知
    nStat[2] = STAT_UNKNOWN;     // 箱2未知
    nStat[3] = STAT_UNKNOWN;     // 箱未知
    nStat[4] = STAT_UNKNOWN;     // 未知
    nStat[5] = STAT_UNKNOWN;     // 未知

    // --------获取UKEY箱状态--------
    // 下发命令
    memset(szRcvData, 0x00, sizeof(szRcvData));
    nRcvDataLen = 2068;
    memcpy(szSndCmd, S_CMD_STAT_UKEY_BOX, STAND_CMD_LENGHT);
    nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "获取UKEY箱状态",
                       m_uiRcvTimeOut, m_uiSndTimeOut, FALSE);
    if (nRet != IMP_SUCCESS)
    {        
        CHK_DEVSTAT(nRet);
        if (nRet != m_nGetStatRetList[0])
        {
            Log(ThisModule, __LINE__, "获取设备状态: UKEY箱状态: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
                CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
            m_nGetStatRetList[0] = nRet;
        }
        return nRet;
    } else
    {
        // Check: 有效应答数据RE_DATA长度
        if ((nRcvDataLen - RCVINFOPOS -1 ) < 2)
        {
            if (nRet != m_nGetStatRetList[0])
            {
                Log(ThisModule, __LINE__, "获取设备状态: UKEY箱状态: 命令<%s>->GetResponse()返回有效数据位数 Fail, < 2, Return: %s.",
                    szSndCmd, ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
                m_nGetStatRetList[0] = nRet;
            }
            return IMP_ERR_RCVDATA_INVALID;
        }

        memset(szGetData, 0x00, sizeof(szGetData));
        GETBUFF(szGetData, sizeof(szGetData), szRcvData, nRcvDataLen);
        if (szGetData[0] == '0')    // 箱1无卡
        {
            nStat[1] = STAT_EMPTY;
        } else
        if (szGetData[0] == '1')    // 箱1少卡
        {
            nStat[1] = STAT_LOW;
        } else
        if (szGetData[0] == '2')    // 箱1足卡
        {
            nStat[1] = STAT_FULL;
        } else
        if (szGetData[0] == '3')    // 箱1不存在
        {
            nStat[1] = STAT_NOHAVE;
        }
        if (szGetData[1] == '0')    // 箱2无卡
        {
            nStat[2] = STAT_EMPTY;
        } else
        if (szGetData[1] == '1')    // 箱2少卡
        {
            nStat[2] = STAT_LOW;
        } else
        if (szGetData[1] == '2')    // 箱2足卡
        {
            nStat[2] = STAT_FULL;
        } else
        if (szGetData[1] == '3')    // 箱2不存在
        {
            nStat[2] = STAT_NOHAVE;
        }
    }
    m_nGetStatRetList[0] = nRet;

    // --------获取回收箱状态--------
    // 下发命令
    memset(szRcvData, 0x00, sizeof(szRcvData));
    nRcvDataLen = 2068;
    memcpy(szSndCmd, S_CMD_STAT_RETAIN_BOX, STAND_CMD_LENGHT);
    nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "回收箱状态",
                       m_uiRcvTimeOut, m_uiSndTimeOut, FALSE);
    if (nRet != IMP_SUCCESS)
    {
        CHK_DEVSTAT(nRet);
        if (nRet != m_nGetStatRetList[1])
        {
            Log(ThisModule, __LINE__, "获取设备状态: 回收箱状态: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
                CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
            m_nGetStatRetList[1] = nRet;
        }
        return nRet;
    } else
    {
        // Check: 有效应答数据RE_DATA长度
        if ((nRcvDataLen - RCVINFOPOS -1 ) < 1)
        {

            if (nRet != m_nGetStatRetList[1])
            {
                Log(ThisModule, __LINE__, "获取设备状态: 回收箱状态: 命令<%s>->GetResponse()返回有效数据位数 Fail, < 1, Return: %s.",
                    szSndCmd, ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
                m_nGetStatRetList[1] = nRet;
            }
            return IMP_ERR_RCVDATA_INVALID;
        }

        memset(szGetData, 0x00, sizeof(szGetData));
        GETBUFF(szGetData, sizeof(szGetData), szRcvData, nRcvDataLen);
        if (szGetData[0] == 'N')    // 未满
        {
            nStat[3] = STAT_LOW;
        } else
        if (szGetData[0] == 'Y')    // 满
        {
            nStat[3] = STAT_FULL;
        } else
        if (szGetData[0] == 'P')    // 空
        {
            nStat[3] = STAT_EMPTY;
        } else
        if (szGetData[0] == 'E')    // 不存在
        {
            nStat[3] = STAT_NOHAVE;
        }
    }
    m_nGetStatRetList[1] = nRet;
    if (memcmp(m_szGetStatRetList[1], szRcvData, nRcvDataLen) != 0)
    {
        MCMP_RCVDATA(1);
        Log(ThisModule, __LINE__, "获取设备状态: 回收箱状态: 命令<%s>->GetResponse()返回数据变化: %s->%s.",
            szSndCmd, stdRcvDataOLD.c_str(), stdRcvDataNEW.c_str());
    }

    // --------获取前端位置状态--------
    // 下发命令
    memset(szRcvData, 0x00, sizeof(szRcvData));
    nRcvDataLen = 2068;
    memcpy(szSndCmd, S_CMD_STAT_FRONT_HAVE, STAND_CMD_LENGHT);
    nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "前端位置UK有无",
                       m_uiRcvTimeOut, m_uiSndTimeOut, FALSE);
    if (nRet != IMP_SUCCESS)
    {
        CHK_DEVSTAT(nRet);
        if (nRet != m_nGetStatRetList[2])
        {
            Log(ThisModule, __LINE__, "获取设备状态: 前端位置UK有无: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
                CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
            m_nGetStatRetList[2] = nRet;
        }

        return nRet;
    } else
    {
        // Check: 有效应答数据RE_DATA长度
        if ((nRcvDataLen - RCVINFOPOS -1 ) < 1)
        {
            if (nRet != m_nGetStatRetList[2])
            {
                Log(ThisModule, __LINE__, "获取设备状态: 前端位置UK有无: 命令<%s>->GetResponse()返回有效数据位数 Fail, < 1, Return: %s.",
                    szSndCmd, ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
                m_nGetStatRetList[2] = nRet;
            }
            return IMP_ERR_RCVDATA_INVALID;
        }

        memset(szGetData, 0x00, sizeof(szGetData));
        GETBUFF(szGetData, sizeof(szGetData), szRcvData, nRcvDataLen);
        if (szGetData[0] == 'Y')    // 有
        {
            nStat[4] = STAT_ISHAVE;
        } else
        if (szGetData[0] == 'N')    // 无
        {
            nStat[4] = STAT_NOHAVE;
        }
    }
    m_nGetStatRetList[2] = nRet;
    if (memcmp(m_szGetStatRetList[2], szRcvData, nRcvDataLen) != 0)
    {
        MCMP_RCVDATA(2);
        Log(ThisModule, __LINE__, "获取设备状态: 前端位置UK有无: 命令<%s>->GetResponse()返回数据变化: %s->%s.",
            szSndCmd, stdRcvDataOLD.c_str(), stdRcvDataNEW.c_str());
    }


    // --------获取扫描位置状态--------
    // 下发命令
    memset(szRcvData, 0x00, sizeof(szRcvData));
    nRcvDataLen = 2068;
    memcpy(szSndCmd, S_CMD_STAT_SCAN_HAVE, STAND_CMD_LENGHT);
    nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "扫描位置UK有无",
                       m_uiRcvTimeOut, m_uiSndTimeOut, FALSE);
    if (nRet != IMP_SUCCESS)
    {
        CHK_DEVSTAT(nRet);
        if (nRet != m_nGetStatRetList[3])
        {
            Log(ThisModule, __LINE__, "获取设备状态: 扫描位置UK有无: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
                        CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
            m_nGetStatRetList[3] = nRet;
        }
        return nRet;
    } else
    {
        // Check: 有效应答数据RE_DATA长度
        if ((nRcvDataLen - RCVINFOPOS -1 ) < 1)
        {
            if (nRet != m_nGetStatRetList[3])
            {
                Log(ThisModule, __LINE__, "获取设备状态: 扫描位置UK有无: 命令<%s>->GetResponse()返回有效数据位数 Fail, < 1, Return: %s.",
                    szSndCmd, ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
                m_nGetStatRetList[3] = nRet;
            }
            return IMP_ERR_RCVDATA_INVALID;
        }

        memset(szGetData, 0x00, sizeof(szGetData));
        GETBUFF(szGetData, sizeof(szGetData), szRcvData, nRcvDataLen);
        if (szGetData[0] == 'Y')    // 有
        {
            nStat[5] = STAT_ISHAVE;
        } else
        if (szGetData[0] == 'N')    // 无
        {
            nStat[5] = STAT_NOHAVE;
        }
    }
    m_nGetStatRetList[3] = nRet;
    if (memcmp(m_szGetStatRetList[3], szRcvData, nRcvDataLen) != 0)
    {
        MCMP_RCVDATA(3);
        Log(ThisModule, __LINE__, "获取设备状态: 扫描位置UK有无: 命令<%s>->GetResponse()返回数据变化: %s->%s.",
            szSndCmd, stdRcvDataOLD.c_str(), stdRcvDataNEW.c_str());
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 3. 介质移动
 * 参数: usMode 状态类别(1/2/4)　参考宏定义
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::MoveMedia(USHORT usMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvDataLen = 2068;

    // Check 参数
    if (usMode == MODE_MOVE_BOX1_SCAN1)          // 移动操作:箱1UKEY到扫描位1
    {
        memcpy(szSndCmd, S_CMD_MOVE_BOX1_SCAN1, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_BOX2_SCAN2)          // 移动操作:箱2UKEY到扫描位1
    {
        memcpy(szSndCmd, S_CMD_MOVE_BOX2_SCAN2, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_UKEY1_FSCAN)          // 移动操作:箱1UKEY移动到前端扫描位置
    {
        memcpy(szSndCmd, S_CMD_MOVE_UKEY1_FSCAN, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_UKEY2_FSCAN)          // 移动操作:箱2UKEY移动到前端扫描位置
    {
        memcpy(szSndCmd, S_CMD_MOVE_UKEY2_FSCAN, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_SCAN2DOOR)           // 移动操作:UKEY从扫描位置发送到门口
    {
        memcpy(szSndCmd, S_CMD_MOVE_SCAN2DOOR, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_UKEY_RETAIN)         // 移动操作:回收UKEY
    {
        memcpy(szSndCmd, S_CMD_MOVE_UKEY_RETAIN, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_UKEY1_DOOR)          // 移动操作:箱1UKEY移动到门口
    {
        memcpy(szSndCmd, S_CMD_MOVE_UKEY1_DOOR, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_UKEY2_DOOR)          // 移动操作:卡箱2UKEY移动到门口
    {
        memcpy(szSndCmd, S_CMD_MOVE_UKEY2_DOOR, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_DOOR2SCAN)           // 移动操作:UKEY从出口到扫描位置
    {
        memcpy(szSndCmd, S_CMD_MOVE_DOOR2SCAN, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_BOX2_SCAN2_N)        // 移动操作:箱2UKEY到扫描位2,不扫描
    {
        memcpy(szSndCmd, S_CMD_MOVE_BOX2_SCAN2_N, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_BOX1_SCAN1_N)        // 移动操作:箱1UKEY到扫描位1,不扫描
    {
        memcpy(szSndCmd, S_CMD_MOVE_BOX1_SCAN1_N, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_BOX2_SCAN1_N)        // 移动操作:箱2UKEY到扫描位1,不扫描
    {
        memcpy(szSndCmd, S_CMD_MOVE_BOX2_SCAN1_N, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_MOVE_BOX1_SCAN2_N)        // 移动操作卡箱2UKEY到扫描位2,不扫描
    {
        memcpy(szSndCmd, S_CMD_MOVE_BOX1_SCAN2_N, STAND_CMD_LENGHT);
    } else
    {
        Log(ThisModule, __LINE__, "入参为无效[%d], Return: %s.", usMode,
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;       // 无效入参
    }

    // 下发命令
    UINT uiRcvTimeOut = (m_uiRcvTimeOut > TIMEOUT_DISPENSECARD ? m_uiRcvTimeOut : TIMEOUT_DISPENSECARD);
    nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "介质移动",
                       uiRcvTimeOut, m_uiSndTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "介质移动: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
            CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 4. 介质扫描
 * 参数: usMode 状态类别(1/2/4)　参考宏定义
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::ScanMedia(USHORT usMode, LPSTR lpRcvData, LPINT lpRcvDataLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvDataLen = 2068;

    // Check 参数
    if (usMode == MODE_SCAN_GET_INFO)            // 扫描信息:获取扫描信息
    {
        memcpy(szSndCmd, S_CMD_SCAN_GET_INFO, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_SCAN_CLR_INFO)            // 扫描信息:清除扫描信息
    {
        memcpy(szSndCmd, S_CMD_SCAN_CLR_INFO, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_SCAN_START)               // 扫描信息:开启一次扫码
    {
        memcpy(szSndCmd, S_CMD_SCAN_START, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_SCAN_QR_OPEN)             // 扫描信息:打开二维码功能
    {
        memcpy(szSndCmd, S_CMD_SCAN_QR_OPEN, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_SCAN_QR_CLOSE)            // 扫描信息:关闭二维码功能
    {
        memcpy(szSndCmd, S_CMD_SCAN_QR_CLOSE, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_SCAN_BAR_OPEN)            // 扫描信息:打开条码功能
    {
        memcpy(szSndCmd, S_CMD_SCAN_BAR_OPEN, STAND_CMD_LENGHT);
    } else
    if (usMode == MODE_SCAN_BAR_CLOSE)           // 扫描信息:关闭条码功能
    {
        memcpy(szSndCmd, S_CMD_SCAN_BAR_CLOSE, STAND_CMD_LENGHT);
    } else
    {
        Log(ThisModule, __LINE__, "入参为无效[%d], Return: %s.", usMode,
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;       // 无效入参
    }

    // 下发命令
    UINT uiRcvTimeOut = (m_uiRcvTimeOut > TIMEOUT_READSCAN ? m_uiRcvTimeOut : TIMEOUT_READSCAN);
    nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "扫描信息",
                       uiRcvTimeOut, m_uiSndTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "扫描信息: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
            CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 有应答数据
    if (usMode == MODE_SCAN_GET_INFO || usMode == MODE_SCAN_START)            // 扫描信息:获取扫描信息|开启一次扫码
    {
        GETBUFF(lpRcvData, *lpRcvDataLen, szRcvData, nRcvDataLen);
        *lpRcvDataLen = (nRcvDataLen - RCVINFOPOS - 1);
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 5. 设置设备参数
 * 参数: usMode 状态类别(1/2/4)　参考宏定义
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::SetDeviceParam(USHORT usMode, INT nParam[12])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvDataLen = 2068;
    CHAR    szSndData[6+1] = { 0x00 };

    if (usMode == MODE_SCAN_PARAM)          // 扫描时托盘移动参数
    {
        memcpy(szSndCmd, S_CMD_STPAM_WRITE, STAND_CMD_LENGHT);   // "B1"

        // 扫描失败后上升第一步距离: Byte0 * 255 + Byte1
        // 扫描失败后上升第一步后每步上升距离: Byte2
        // 扫描失败后上升重复次数: Byte3
        // 移出卡口超时时间: Byte4 * 255 + Byte5
        szSndData[0] = nParam[0] / 255;
        szSndData[1] = nParam[0] % 255;
        szSndData[2] = nParam[1];
        szSndData[3] = nParam[2];
        szSndData[4] = nParam[3] / 255;
        szSndData[5] = nParam[3] % 255;

        // 下发命令
        nRet = CmdSendRecv(szSndCmd, szSndData, 6, szRcvData, &nRcvDataLen, "设置机器参数",
                           m_uiRcvTimeOut, m_uiSndTimeOut);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "扫描信息: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
                CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
            return nRet;
        }
    } else
    {
        if (usMode == MODE_ST_POWOFF_NOACT)     // 掉电后处理:无动作
        {
            memcpy(szSndCmd, S_CMD_STFUN_POWFAL_NOR, STAND_CMD_LENGHT);
        } else
        if (usMode == MODE_ST_POWOFF_RETAIN)    // 掉电后处理:回收
        {
            memcpy(szSndCmd, S_CMD_STFUN_POWFAL_RETAIN, STAND_CMD_LENGHT);
        } else
        if (usMode == MODE_ST_POWOFF_EJECT)     // 掉电后处理:弹出
        {
            memcpy(szSndCmd, S_CMD_STFUN_POWFAL_EJECT, STAND_CMD_LENGHT);
        } else
        {
            return IMP_SUCCESS;
        }

        // 下发命令
        nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "设置设备参数",
                           m_uiRcvTimeOut, m_uiSndTimeOut);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "设置设备参数: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
                CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 6. 获取设备参数
 * 参数: usMode 状态类别(1/2/4)　参考宏定义
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::GetDeviceParam(USHORT usMode, INT nParam[12])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvDataLen = 2068;
    CHAR    *szRcvDataPos = nullptr;

    if (usMode == MODE_SCAN_PARAM)  // 获取扫描时托盘移动参数
    {
        memcpy(szSndCmd, S_CMD_STPAM_READ, STAND_CMD_LENGHT);   // "B0" 读取机器参数

        // 下发命令
        nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "获取机器参数",
                           m_uiRcvTimeOut, m_uiSndTimeOut);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "扫描信息: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
                CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
            return nRet;
        }

        szRcvDataPos = szRcvData + RCVINFOPOS;
        // 扫描失败后上升第一步距离: Byte0 * 255 + Byte1
        nParam[0] = szRcvDataPos[0] * 255 + szRcvDataPos[1];
        // 扫描失败后上升第一步后每步上升距离: Byte2
        nParam[1] = szRcvDataPos[2] * 255;
        // 扫描失败后上升重复次数: Byte3
        nParam[2] = szRcvDataPos[3];
        // 移出卡口超时时间: Byte4 * 255 + Byte5
        nParam[3] = szRcvDataPos[4] * 255 + szRcvDataPos[5];
    }

    return nRet;
}

/************************************************************
 * 功能: 7. 获取设备固件版本
 * 参数: usMode 状态类别(1/2/4)　参考宏定义
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::GetDeviceFW(LPSTR lpDevFW, USHORT usLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvDataLen = 2068;

    memcpy(szSndCmd, S_CMD_RESET_NOACTIVE, STAND_CMD_LENGHT);   // 无动作复位

    // 下发命令
    nRet = CmdSendRecv(szSndCmd, nullptr, 0, szRcvData, &nRcvDataLen, "扫描信息",
                       m_uiRcvTimeOut, m_uiSndTimeOut);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "扫描信息: CmdSendRecv: 命令<%s>收发/解析有错误, Return: %s.",
            CmdToStr(szSndCmd), ConvertErrCodeToStr(nRet));
        return nRet;
    }

    // 有应答数据
    GETBUFF(lpDevFW, usLen, szRcvData, nRcvDataLen);

    return nRet;
}

/************************************************************
 * 功能: 命令收发
 * 参数: lpcCmd     命令
 *      lpcData    命令参数
 *      nLen       命令参数长度
 *      lpRcvData   命令应答数据
 *      nRcvDataLen 命令应答数据长度(不包含BCC)
 * 返回值: 参考错误码(IMP_XXX)
************************************************************/
INT CDevImpl_ACTU6SS39::CmdSendRecv(LPCSTR lpcSndCmd, LPCSTR lpcSndData, INT nDataLen,
                                    LPSTR lpRcvData, LPINT lpRcvDataLen, LPSTR lpDesc,
                                    UINT uiRcvTimeOut, UINT uiSndTimeOut, BOOL bLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;
    INT nRcvDataLen = *lpRcvDataLen;

    if (m_pDev == nullptr && bLog == TRUE)
    {
        Log(ThisModule, __LINE__, "m_pDev == null, Device Not Open, Return: %s",
            ConvertErrCodeToStr(IMP_ERR_DEVPORT_NOTOPEN));
        return IMP_ERR_DEVPORT_NOTOPEN;
    }

    // 下发命令(返回值需转换为IMP)
    nRet = SendCmd(lpcSndCmd, lpcSndData, nDataLen, uiSndTimeOut);
    if (nRet != 0)
    {
        if (bLog == TRUE)
        {
            Log(ThisModule, __LINE__, "%s: SendCmd(%s, %s, %d, %d)失败, RetCode: %d, Return: %s.",
                (lpDesc == nullptr ? " " : lpDesc),
                CmdToStr((LPSTR)lpcSndCmd), (lpcSndData == nullptr ? "(null)" : lpcSndData), nDataLen,
                uiSndTimeOut, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        }
        return ConvertErrorCode(nRet);
    }

    // 接收应答(返回值需转换为IMP)
    nRet = GetResponse(lpRcvData, nRcvDataLen, uiRcvTimeOut, nRcvDataLen);
    if (nRet != IMP_SUCCESS)
    {
        if (bLog == TRUE)
        {
            Log(ThisModule, __LINE__, "%s: 命令<%s>->GetResponse(%s, %d, %d, %d)失败, RetCode: %d, Return: %s.",
                (lpDesc == nullptr ? " " : lpDesc),
                CmdToStr((LPSTR)lpcSndCmd), lpRcvData, nRcvDataLen, uiRcvTimeOut, nRcvDataLen,
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        }
        return ConvertErrorCode(nRet);
    }
    if (bLog == TRUE)   // 应答数据输出
    {
        CHAR szTmp[12] = { 0x00 };
        std::string stdSndData;
        for (int i = 0; i < nRcvDataLen; i ++)
        {
            sprintf(szTmp, "%02X ", (INT)lpRcvData[i]);
            stdSndData.append(szTmp);
        }
        Log(ThisModule, __LINE__, "%s: 命令<%s>->GetResponse()成功, RecvData: %s.",
            (lpDesc == nullptr ? " " : lpDesc), CmdToStr((LPSTR)lpcSndCmd), stdSndData.c_str());
    }

    // 应答数据Check(返回值为IMP,不需要转换)
    nRet =  RcvDataCheck(lpcSndCmd, lpRcvData, nRcvDataLen, bLog);
    if (nRet != IMP_SUCCESS)
    {
        if (bLog == TRUE)
        {
            Log(ThisModule, __LINE__, "%s: 命令<%s>应答数据解析有错误, Return: %s.",
                (lpDesc == nullptr ? "(null)" : lpDesc),
                lpcSndCmd, ConvertErrCodeToStr(nRet));
        }
        return nRet;
    }
    *lpRcvDataLen = nRcvDataLen;

    return nRet;
}

/************************************************************
 * 功能: 下发数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令参数
 *      nLen   命令参数长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::SendCmd(LPCSTR lpcCmd, LPCSTR lpcData, INT nLen, UINT uiTimeout, BOOL bSingle)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);
    //LOGDEVACTION();
    CHAR szCmdBuff[2068] = { 0x00 };        // 命令buff
    USHORT usCmdBuffSize = 0;               // 命令buff长度
    BYTE byBCC = 0x00;
    int nRet = 0;

    if (bSingle == FALSE)
    {
        // 一帧数据 packet
        // STX(1Bytes)+SELEN(2Bytes)+CM(1Bytes)+PM(1Bytes)+SE_DATA(XBytes)+ETX(1Bytes)+BCC(1Bytes)
        szCmdBuff[usCmdBuffSize ++] = (CHAR)STX;
        szCmdBuff[usCmdBuffSize ++] = (CHAR)((nLen + STAND_CMD_LENGHT) / 256);  // 长度位计算(CMD+Data),高位
        szCmdBuff[usCmdBuffSize ++] = (CHAR)((nLen + STAND_CMD_LENGHT) % 256);  // 长度位计算(CMD+Data),低位

        // + 命令
        memcpy(szCmdBuff + (usCmdBuffSize), lpcCmd, STAND_CMD_LENGHT);
        usCmdBuffSize += STAND_CMD_LENGHT;

        // + Data
        if (lpcData != nullptr)
        {
            memcpy(szCmdBuff + (usCmdBuffSize), lpcData, nLen);
            usCmdBuffSize += nLen;
        }

        szCmdBuff[usCmdBuffSize ++] = (CHAR)ETX;

        // 计算CRC
        byBCC = GetDataBCC(szCmdBuff, usCmdBuffSize);
        szCmdBuff[usCmdBuffSize ++] = byBCC;
    } else
    {
        memcpy(szCmdBuff + usCmdBuffSize, lpcData, nLen);
        usCmdBuffSize += nLen;
    }

    /*CHAR szTmp[12] = { 0x00 };
    std::string stdSndData;
    for (int i = 0; i < usCmdBuffSize; i ++)
    {
        sprintf(szTmp, "%02X ", (INT)szCmdBuff[i]);
        stdSndData.append(szTmp);
    }
    if (bSingle == FALSE)
    {
        Log(ThisModule, __LINE__, "SendData To Device: CMD<%s> %d|%s",
            CmdToStr((LPSTR)lpcCmd), usCmdBuffSize, stdSndData.c_str());
    } else
    {
        Log(ThisModule, __LINE__, "SendData To Device: 控制标记 %d|%s",
            usCmdBuffSize, stdSndData.c_str());
    }*/

    // 发送命令到设备，总计3次(如果设备未连接则先执行连接操作)
    for (INT iIndex = 0; iIndex < 4; iIndex++)
    {
        nRet = m_pDev->Send(szCmdBuff, usCmdBuffSize, uiTimeout);   // 返回下发命令长度为成功

        //if (nRet == usCmdBuffSize)
        if (nRet == ERR_DEVPORT_SUCCESS)
        {
            //nRet = ERR_DEVPORT_SUCCESS;
            break;
        }

        /*if (nRet != m_nLastError && nRet < ERR_DEVPORT_SUCCESS)
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
        }*/
    }

    return nRet;//ConvertErrorCode(nRet);
}

/************************************************************
 * 功能：读取读卡器的返回数据
 * 参数：lpResponse返回数据的缓冲区
 *      nLen缓冲区长度
 *      nTimeout超时(毫秒)
 *      nOutLen 返回的赢大数据长度
 * 返回：参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::GetResponse(LPSTR lpResponse, INT nLen, UINT uiTimeout, INT &nOutLen)
{
    Q_UNUSED(nLen)

    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    CHAR pszReply[RESP_BUFF_SIZE];
    CHAR szSndCMD[64];
    int nRet = 0;
    int nIndex = 0;
    int timeout = uiTimeout;

    /*if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null, Device Not Open, Return: %s",
            ConvertErrCodeToStr(ERR_DEVPORT_NOTOPEN));
        return ERR_DEVPORT_NOTOPEN;
    }*/

    // 应答报文:
    //   STX(1Bytes)+RELEN(2Bytes)+"P/N"(1Bytes)+CM(1Bytes)+PM(1Bytes)+RE_DATA(XBytes)+ETX(1Bytes)+BCC(1Bytes)
    //   RELEN = "P/N"(1Bytes)+CM(1Bytes)+PM(1Bytes)+RE_DATA(XBytes)
    //   总长度 = RELEN + 3(STX+RELEN) + 2(ETX+BCC)
    // 报文接收方式: 循环获取
    //   1. 接收到ACK,同步发送ENQ,不计入接收长度
    //   2. 接收长度>3时,验证是否接收完成:完成则退出接收循环

    while(TRUE)
    {
        DWORD ulInOutLen = RESP_BUFF_SIZE;
        memset(pszReply, 0x00, sizeof(pszReply));
        nRet = m_pDev->Read(pszReply, ulInOutLen, uiTimeout); // 返回值<0为失败, ulInOutLen返回数据长度
        if (nRet < 0)  // 失败则跳出
        {
            break;
        } else
        {
            // 接收数据记录到Log
            /*CHAR szTmp[12] = { 0x00 };
            std::string stdRcvData;
            for (int i = 0; i < ulInOutLen; i ++)
            {
                sprintf(szTmp, "%02X ", (int)pszReply[i]);
                stdRcvData.append(szTmp);
            }*/

            // 返回命令肯定结果,下发ENQ(命令执行请求)
            if (pszReply[0] == ACK)
            {
                /*Log(ThisModule, __LINE__, "RecvData To Device: %d|%s\n"
                                          "RecvData Byte[0] == %02X|ACK(确认下发CMD有效), 下发ENQ(命令执行请求).",
                    ulInOutLen, stdRcvData.c_str(), ACK);*/

                // 下发命令
                memset(szSndCMD, 0x00, sizeof(szSndCMD));
                szSndCMD[0] = ENQ;
                nRet = SendCmd(nullptr, (LPCSTR)szSndCMD, 1, m_uiSndTimeOut, TRUE);
                if (nRet != 0)
                {
                    /*Log(ThisModule, __LINE__, "下发ENQ(命令执行请求): SendCmd(%02X, NULL, 0)失败, Return: %s.",
                        (INT)ENQ, ConvertErrCodeToStr(nRet));*/
                    return nRet;
                }
                continue;   // 进入下一次读循环
            } else
            if (pszReply[0] == NAK)
            {
                /*Log(ThisModule, __LINE__, "RecvData To Device: %d|%s\n"
                                          "RecvData Byte[0] == %02X|NAK(否认,下发命令无效), Return: %s.",
                    nRet, stdRcvData.c_str(), ConvertErrCodeToStr(IMP_ERR_SNDDATA_INVALID));*/
                return IMP_ERR_SNDDATA_INVALID; // 无效下发数据
            } /*else
            {
                Log(ThisModule, __LINE__, "RecvData To Device: %d|%s.",
                    nRet, stdRcvData.c_str());
            }*/

            // 接收的应答数据计数
            memcpy(lpResponse + nIndex, pszReply, ulInOutLen);
            nIndex = nIndex + ulInOutLen;

            if (nIndex > 3) // 接收到的应答数据检查
            {
                if (nIndex >= (MAKEWORD(lpResponse[2], lpResponse[1]) + 3 + 2))
                {
                    break;      // 接收数据完整,跳出
                } else
                {
                    continue;   // 继续接收
                }
            }
        }
    }

    // 读操作超时+写操作超时+通讯错误:->读数据超时
    if (nRet == ERR_DEVPORT_RTIMEOUT || nRet == ERR_DEVPORT_WTIMEOUT || nRet == ERR_DEVPORT_FAIL)
    {
        return nRet;//IMP_ERR_DEVPORT_RTIMEOUT;
    }

    // Check接收的应答数据完整性
    // 应答数据: STX（1Byte)+LEN(2Byte)+"P/N"(1Byte)+CMD(2Byte)+Data(XXByte)+BCC(1Byte)
    // LEN = "P/N"(1Byte)+CMD(2Byte)+Data(XXByte) Is Length
    DWORD nRecvLen = MAKEWORD(lpResponse[2], lpResponse[1]);  // [1]高位+[2]低位=数据长度
    if (nRecvLen > 0)
    {
        nOutLen = (nRecvLen + 3 + 2);   // LEN + 3(STX+LEN) + 1(BCC) = 接收有效应答
    } else
    {
        /*Log(ThisModule, __LINE__, "Device返回数据错误, RcvDataLen = %02X|%02X = %d, Return: %s.",
            lpResponse[1], lpResponse[2], nRecvLen, ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));*/
        return ERR_DEVPORT_READERR;// IMP_ERR_RCVDATA_INVALID; // 应答数据无效
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能：计算数据BCC
 * 参数：lpData 数据
 *      usDataSize 数据长度
 * 返回：BCC
************************************************************/
BYTE CDevImpl_ACTU6SS39::GetDataBCC(LPSTR lpData, USHORT usDataSize)
{
    if (lpData == nullptr)
    {
        return 0;
    }

    INT i;
    BYTE byBCC = lpData[0];
    for (i = 1; i < usDataSize; i ++)
    {
        byBCC ^= lpData[i];
    }
    return byBCC;
}

/************************************************************
 * 功能: 应答数据Check
 * 参数: lpcSndCmd 入参 下发命令
 *      lpcRcvData 入参 应答数据
 *      nRcvDataLen 入参 应答数据长度
 *                  回参 去掉BCC(1Byte)后的应答数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_ACTU6SS39::RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT &nRcvDataLen, BOOL bLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    INT     nErrCode = 0;
    CHAR    szErrCode = 0x00;

    // 检查应答数据长度(下限)
    // STX(1Bytes)+SELEN(2Bytes)+"P/N"(1Bytes)+CM(1Bytes)+PM(1Bytes)+ETX(1Bytes)+BCC(1Bytes) = 8
    if (nRcvDataLen < 8)
    {
        if (bLog == TRUE)
        {
            Log(ThisModule, __LINE__, "CMD<%s> 返回命令应答数据长度[%d]<8, Return: %s.",
                lpcSndCmd, nRcvDataLen,  ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
        }
        return IMP_ERR_RCVDATA_INVALID;
    }

    // 检查应答命令与发送命令是否一致
    if (memcmp(lpcSndCmd, lpcRcvData + RCVCMDPOS, STAND_CMD_LENGHT) != 0)
    {
        if (bLog == TRUE)
        {
            Log(ThisModule, __LINE__, "CMD<%s> 返回无效的命令应答数据[%02X|%c,%02X|%c], 命令不一致, Return: %s.",
                lpcSndCmd, (INT)(lpcRcvData[RCVCMDPOS]), lpcRcvData[RCVCMDPOS],
                (INT)(lpcRcvData[RCVCMDPOS + 1]), lpcRcvData[RCVCMDPOS + 1],
                ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
        }
        return IMP_ERR_RCVDATA_INVALID;
    }

    // 检查BCC
    BYTE byBCC = 0x00;
    byBCC = GetDataBCC((LPSTR)(lpcRcvData), (USHORT)(nRcvDataLen - 1));// 计算BCC
    if (lpcRcvData[nRcvDataLen -1] != byBCC)
    {
        if (bLog == TRUE)
        {
            Log(ThisModule, __LINE__, "CMD<%s> 返回无效的命令应答数据, BCC不一致, "
                "RcvData BCC = %02X, 计算CRC = %02X, Return: %s.",
                lpcSndCmd, (INT)(lpcRcvData[nRcvDataLen - 1]), (INT)byBCC,
                ConvertErrCodeToStr(IMP_ERR_RCVDATA_INVALID));
        }
        return IMP_ERR_RCVDATA_INVALID;
    }

    // 错误应答数据检查
    if (memcmp(lpcRcvData + RCVRETPOS, CMD_FAIL, 1) == 0) // 错误应答
    {
        szErrCode = lpcRcvData[RCVERRPOS];
        if (bLog == TRUE)
        {
            Log(ThisModule, __LINE__, "CMD<%s> 返回错误应答数据[CMD: %02X,%02X,%02X; ERR_CD: %02X], "
                                      "Return %s.",
                CmdToStr((LPSTR)lpcSndCmd), (INT)(lpcRcvData[RCVCMDPOS]),
                (INT)(lpcRcvData[RCVCMDPOS + 1]), (INT)(lpcRcvData[RCVCMDPOS + 2]),
                szErrCode, ConvertErrCodeToStr(ConvertErrorCode(szErrCode)));
        }
        return ConvertErrorCode(szErrCode);
    }

    // 返回应答数据长度(STX+LEN+CMD+DATA+ETX)
    nRcvDataLen = (nRcvDataLen - 1);

    return IMP_SUCCESS;
}

// DEVICE/USB/COM错误码转换为Impl错误码
INT CDevImpl_ACTU6SS39::ConvertErrorCode(INT nRet)
{
    switch (nRet)
    {
        case ERR_DEVPORT_SUCCESS    : return IMP_SUCCESS;
        case ERR_DEVPORT_NOTOPEN    : return IMP_ERR_DEVPORT_NOTOPEN;           // (-1) 没打开
        case ERR_DEVPORT_FAIL       : return IMP_ERR_DEVPORT_FAIL;              // (-2) 通讯错误
        case ERR_DEVPORT_PARAM      : return IMP_ERR_DEVPORT_PARAM;             // (-3) 参数错误
        case ERR_DEVPORT_CANCELED   : return IMP_ERR_DEVPORT_CANCELED;          // (-4) 操作取消
        case ERR_DEVPORT_READERR    : return IMP_ERR_DEVPORT_READERR;           // (-5) 读取错误
        case ERR_DEVPORT_WRITE      : return IMP_ERR_DEVPORT_WRITE;             // (-6) 发送错误
        case ERR_DEVPORT_RTIMEOUT   : return IMP_ERR_DEVPORT_RTIMEOUT;          // (-7) 操作超时
        case ERR_DEVPORT_WTIMEOUT   : return IMP_ERR_DEVPORT_WTIMEOUT;          // (-8) 操作超时
        case ERR_DEVPORT_LIBRARY    : return IMP_ERR_DEVPORT_LIBRARY;           // (-98) 加载通讯库失败
        case ERR_DEVPORT_NODEFINED  : return IMP_ERR_DEVPORT_NODEFINED;         // (-99) 未知错误
        case ERR_CD_INVLID          : return IMP_ERR_DEVRET_CMD_INVLID;         // 0xFF:未定义的命令
        case ERR_CD_PARAM           : return IMP_ERR_DEVRET_CMD_PARAM;          // 0x01:命令参数错误
        case ERR_CD_UKEY_NOHAVE     : return IMP_ERR_DEVRET_CMD_UKEY_NOHAVE;    // 0x02:卡箱无卡
        case ERR_CD_PASS_BLOCK      : return IMP_ERR_DEVRET_CMD_PASS_BLOCK;     // 0x03:通道堵塞
        case ERR_CD_SENSOR_FAIL     : return IMP_ERR_DEVRET_CMD_SENSOR_FAIL;    // 0x04:传感器坏
        case ERR_CD_ELEV_NOBOT      : return IMP_ERR_DEVRET_CMD_ELEV_NOBOT;     // 0x05:升降梯不在底部
        case ERR_CD_CMD_NORUN       : return IMP_ERR_DEVRET_CMD_CMD_NORUN;      // 0x06:命令不能执行
        case ERR_CD_SCAN_FAIL       : return IMP_ERR_DEVRET_CMD_SCAN_FAIL;      // 0x07:获取扫描信息失败
        case ERR_CD_RETAIN_FULL     : return IMP_ERR_DEVRET_CMD_RETAIN_FULL;    // 0x08:回收箱满
        case ERR_CD_REPASS_BLOCK    : return IMP_ERR_DEVRET_CMD_REPASS_BLOCK;   // 0x09:回收通道堵塞
        case ERR_CD_VAL_OVER        : return IMP_ERR_DEVRET_CMD_VAL_OVER;       // 0x0A:设置值超出范围
        case ERR_CD_REBOX_NOHAVE    : return IMP_ERR_DEVRET_CMD_REBOX_NOHAVE;   // 0x0B:无回收箱
        case ERR_CD_ELEV_NOSCAN     : return IMP_ERR_DEVRET_CMD_ELEV_NOSCAN;    // 0x0C:升降梯不在扫描位
        case ERR_CD_POWER_FAIL      : return IMP_ERR_DEVRET_CMD_POWER_FAIL;     // 0x10:电压异常,过低或过高
        case ERR_CD_ELEC_FAIL       : return IMP_ERR_DEVRET_CMD_ELEC_FAIL;      // 0x12:升降电机模块故障
        case ERR_CD_SELF_FAIL       : return IMP_ERR_DEVRET_CMD_SELF_FAIL;      // 0x13:自适应出错
        case ERR_CD_ELEVM_FAIL      : return IMP_ERR_DEVRET_CMD_ELEVM_FAIL;     // 0x14:电梯模块故障
        case ERR_CD_CAM_FAIL        : return IMP_ERR_DEVRET_CMD_CAM_FAIL;       // 0x15:凸轮模块故障
        case ERR_CD_BOX_NOHAVE      : return IMP_ERR_DEVRET_CMD_BOX_NOHAVE;     // 0x17:没有卡箱或卡箱没有到位
        default: return nRet;
    }
}

// IMPL错误码解释
CHAR* CDevImpl_ACTU6SS39::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case IMP_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "成功");
            return m_szErrStr;
        case IMP_ERR_LOAD_LIB:
            sprintf(m_szErrStr, "%d|%s", nRet, "IMPL:动态库加载失败");
            return m_szErrStr;
        case IMP_ERR_PARAM_INVALID:
            sprintf(m_szErrStr, "%d|%s", nRet, "IMPL:参数无效");
            return m_szErrStr;
        case IMP_ERR_READERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "IMPL:读数据错误");
            return m_szErrStr;
        case IMP_ERR_WRITEERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "IMPL:写数据错误");
            return m_szErrStr;
        case IMP_ERR_RCVDATA_INVALID :
            sprintf(m_szErrStr, "%d|%s", nRet, "IMPL:无效的应答数据");
            return m_szErrStr;
        case IMP_ERR_UNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "IMPL:未知错误");
            return m_szErrStr;
        case IMP_ERR_SNDDATA_INVALID:
            sprintf(m_szErrStr, "%d|%s", nRet, "IMPL:无效的下发数据");
            return m_szErrStr;

        // <0 : USB/COM接口处理返回
        case IMP_ERR_DEVPORT_NOTOPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:没打开");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:通讯错误");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_PARAM:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:参数错误");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_CANCELED:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:操作取消");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_READERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:读取错误");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_WRITE:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:发送错误");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_RTIMEOUT:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:操作超时");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_WTIMEOUT:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:操作超时");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_LIBRARY:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:加载通讯库失败");
            return m_szErrStr;
        case IMP_ERR_DEVPORT_NODEFINED:
            sprintf(m_szErrStr, "%d|%s", nRet, "DevPort:未知错误");
            return m_szErrStr;

        // 0~0xFF: 硬件设备返回
        case IMP_ERR_DEVRET_CMD_INVLID:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义的命令");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_PARAM:
            sprintf(m_szErrStr, "%d|%s", nRet, "命令参数错误");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_UKEY_NOHAVE:
            sprintf(m_szErrStr, "%d|%s", nRet, "卡箱无卡");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_PASS_BLOCK:
            sprintf(m_szErrStr, "%d|%s", nRet, "通道堵塞");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_SENSOR_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "传感器坏");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_ELEV_NOBOT:
            sprintf(m_szErrStr, "%d|%s", nRet, "升降梯不在底部");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_CMD_NORUN:
            sprintf(m_szErrStr, "%d|%s", nRet, "命令不能执行");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_SCAN_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "获取扫描信息失败");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_RETAIN_FULL:
            sprintf(m_szErrStr, "%d|%s", nRet, "回收箱满");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_REPASS_BLOCK:
            sprintf(m_szErrStr, "%d|%s", nRet, "回收通道堵塞");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_VAL_OVER:
            sprintf(m_szErrStr, "%d|%s", nRet, "设置值超出范围");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_REBOX_NOHAVE:
            sprintf(m_szErrStr, "%d|%s", nRet, "无回收箱");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_ELEV_NOSCAN:
            sprintf(m_szErrStr, "%d|%s", nRet, "升降梯不在扫描位");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_POWER_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "电压异常,过低或过高");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_ELEC_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "升降电机模块故障");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_SELF_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "自适应出错");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_ELEVM_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "电梯模块故障");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_CAM_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "凸轮模块故障");
            return m_szErrStr;
        case IMP_ERR_DEVRET_CMD_BOX_NOHAVE:
            sprintf(m_szErrStr, "%d|%s", nRet, "没有卡箱或卡箱没有到位");
            return m_szErrStr;
    }
}

// 下发命令码解释
CHAR* CDevImpl_ACTU6SS39::CmdToStr(LPSTR lpCmd)
{
    #define RET_CMDSTR(CMD1, CMD2, STR) \
        if (memcmp(CMD1, CMD2, strlen(CMD1)) == 0 && memcmp(CMD1, CMD2, strlen(CMD2)) == 0) \
        { \
            sprintf(m_szCmdStr, "%s:%s", CMD1, STR); \
            return m_szCmdStr; \
        }

    memset(m_szCmdStr, 0x00, sizeof(m_szCmdStr));
    RET_CMDSTR(lpCmd, S_CMD_RESET_RETAIN, "复位:回收");
    RET_CMDSTR(lpCmd, S_CMD_RESET_NOACTIVE, "复位:无动作,上传软件版本");
    RET_CMDSTR(lpCmd, S_CMD_RESET_TRANS, "复位:传动机构");
    RET_CMDSTR(lpCmd, S_CMD_RESET_EJECT, "复位:弹卡");
    RET_CMDSTR(lpCmd, S_CMD_STAT_SENSOR, "状态查询:读取每个传感器状态");
    RET_CMDSTR(lpCmd, S_CMD_STAT_FRONT_HAVE, "状态查询:前段是否有UKEY");
    RET_CMDSTR(lpCmd, S_CMD_STAT_SCAN_HAVE, "状态查询:扫描位置是否有UKEY");
    RET_CMDSTR(lpCmd, S_CMD_STAT_UKEY_BOX, "状态查询:查询UKEY箱状态");
    RET_CMDSTR(lpCmd, S_CMD_STAT_RETAIN_BOX, "状态查询:查询回收箱状态");
    RET_CMDSTR(lpCmd, S_CMD_STAT_UKBOX_HAVE, "状态查询:查询UKEY箱静止状态,只返回有无UK");
    RET_CMDSTR(lpCmd, S_CMD_STPAM_READ, "参数设置:读取");
    RET_CMDSTR(lpCmd, S_CMD_STPAM_WRITE, "参数设置:写入/设置");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_BOX1_SCAN1, "移动操作:卡箱1到扫描位1");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_BOX2_SCAN2, "移动操作:卡箱2到扫描位1");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_UKEY1_FSCAN, "移动操作:卡箱1UKEY移动到前端扫描位置");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_UKEY2_FSCAN, "移动操作:卡箱2UKEY移动到前端扫描位置");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_SCAN2DOOR, "移动操作:UKEY从扫描未知发送到门口");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_UKEY_RETAIN, "移动操作:回收UKEY");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_UKEY1_DOOR, "移动操作:卡箱1UKEY移动到门口");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_UKEY2_DOOR, "移动操作:卡箱2UKEY移动到门口");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_DOOR2SCAN, "移动操作:UKEY从出口到扫描位置");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_BOX2_SCAN2_N, "移动操作:卡箱2到扫描位2,不扫描");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_BOX1_SCAN1_N, "移动操作:卡箱1到扫描位1,不扫描");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_BOX2_SCAN1_N, "移动操作:卡箱2到扫描位1,不扫描");
    RET_CMDSTR(lpCmd, S_CMD_MOVE_BOX1_SCAN2_N, " 移动操作:卡箱1到扫描位2,不扫描");
    RET_CMDSTR(lpCmd, S_CMD_SCAN_GET_INFO, "扫描信息:获取扫描信息");
    RET_CMDSTR(lpCmd, S_CMD_SCAN_CLR_INFO, "扫描信息:清除扫描信息");
    RET_CMDSTR(lpCmd, S_CMD_SCAN_START, "扫描信息:开启一次扫码");
    RET_CMDSTR(lpCmd, S_CMD_SCAN_DEF, "扫描信息:未知");
    RET_CMDSTR(lpCmd, S_CMD_SCAN_QR_OPEN, "扫描信息:打开二维码功能");
    RET_CMDSTR(lpCmd, S_CMD_SCAN_QR_CLOSE, "扫描信息:关闭二维码功能");
    RET_CMDSTR(lpCmd, S_CMD_SCAN_BAR_OPEN, "扫描信息:打开条码功能");
    RET_CMDSTR(lpCmd, S_CMD_SCAN_BAR_CLOSE, "扫描信息:关闭条码功能");
    RET_CMDSTR(lpCmd, S_CMD_STFUN_POWFAL_NOR, "功能设置:掉电无动作");
    RET_CMDSTR(lpCmd, S_CMD_STFUN_POWFAL_RETAIN, "功能设置:掉电回收");
    RET_CMDSTR(lpCmd, S_CMD_STFUN_POWFAL_EJECT, "功能设置:掉电弹出");
    RET_CMDSTR(lpCmd, S_CMD_FUNSTA_POWFAL_MODE, "功能状态查询:掉电工作模式");
}

//---------------------------------------------
