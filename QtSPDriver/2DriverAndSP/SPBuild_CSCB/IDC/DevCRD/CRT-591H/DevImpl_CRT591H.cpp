/***************************************************************
* 文件名称：DevImpl_CRT780B.cpp
* 文件描述：CRT780B退卡模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/
#include "DevImpl_CRT591H.h"

static const char *ThisModule = "DevImpl_CRT591H.cpp";

CDevImpl_CRT591H::CDevImpl_CRT591H()
{
    SetLogFile(LOG_NAME, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CRT591H::CDevImpl_CRT591H(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_CRT591H::Init()
{
    m_bDevOpenOk = FALSE;
}

CDevImpl_CRT591H::~CDevImpl_CRT591H()
{

}


/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参　格式: USB:VID,PID  VID/PID为4位16进制字符串
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT591H::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    if (m_pDev == nullptr)
    {
        if (m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "CRD", "DevCRD_CRT591H") != 0)
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
INT CDevImpl_CRT591H::DeviceClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
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
INT CDevImpl_CRT591H::Release()
{
    return DeviceClose();
}

BOOL CDevImpl_CRT591H::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/************************************************************
 * 功能: 1. 模块初始化
 * 参数: wInitMode 初始化方式(0/1/2/4)　参考宏定义
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT591H::DeviceInit(WORD wMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);



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
INT CDevImpl_CRT591H::GetDeviceStat(WORD wMode, INT nStat[12])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();



    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 14. 获取设备序列号
 * 参数: szSerialNum　返回序列号(最大18位)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT591H::GetDeviceSerialNumber(LPSTR lpSerialNum)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[124] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };



    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 15. 设备复位
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT591H::DeviceReset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    LOGDEVACTION();



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
INT CDevImpl_CRT591H::RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT &nRcvDataLen)
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
 * 功能: 下发数据到Device
 * 参数: pszCmd 命令
 *      lpData 命令数据
 *      nLen   命令数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_CRT591H::SendCmd(const char *pszCmd, const char *lpData, int nLen)
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
INT CDevImpl_CRT591H::SendSinglePacket(const char* pszCmd, const char *lpData, int nLen)
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
INT CDevImpl_CRT591H::SendMultiPacket(const char *pszCmd, const char *lpData, int nLen)
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
INT CDevImpl_CRT591H::GetResponse(char *pszResponse, int nLen, int nTimeout, INT &nOutLen)
{
    Q_UNUSED(nLen)

    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_MutexAction);

    char pszReply[64];
    int nRet = 0;
    int nIndex = 0;
    int timeout = nTimeout;

    if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null, Device Not Open, Return: %s",
            ConvertErrCodeToStr(ERR_DEVPORT_NOTOPEN));
        return ERR_DEVPORT_NOTOPEN;
    }

    while(TRUE)
    {
        DWORD ulInOutLen = 64;//USB_READ_LENTH;
        memset(pszReply, 0, sizeof(pszReply));
        nRet = m_pDev->Read(pszReply, ulInOutLen, timeout); // 返回值为数据长度
        if (nRet <= 0)
        {
            break;
        } else
        {
            // 接收数据记录到Log
            CHAR szTmp[12] = { 0x00 };
            std::string stdRcvData;
            for (int i = 0; i < nRet; i ++)
            {
                sprintf(szTmp, "%02X ", (int)pszReply[i]);
                stdRcvData.append(szTmp);
            }
            // 单一 确认/否认 命令丢弃
            if (pszReply[0] == ACK)
            {
                Log(ThisModule, __LINE__, "RecvData To Device: %d|%s\n"
                                          "RecvData Byte[0] == %02X|ACK(确认下发CMD有效), 不做处理.",
                    nRet, stdRcvData.c_str(), ACK);
                continue;
            } else
            if (pszReply[0] == NAK)
            {
                Log(ThisModule, __LINE__, "RecvData To Device: %d|%s\n"
                                          "RecvData Byte[0] == %02X|NAK(否认,下发命令无效), Return: %s.",
                    nRet, stdRcvData.c_str(), ConvertErrCodeToStr(IMP_ERR_SNDDATA_INVALID));
                return IMP_ERR_SNDDATA_INVALID; // 无效下发数据
            } else
            {
                Log(ThisModule, __LINE__, "RecvData To Device: %d|%s.",
                    nRet, stdRcvData.c_str());
            }

            if (nIndex == 0)
            {
               // 第一个包
               nIndex = nRet;
               memcpy(pszResponse, pszReply, nIndex);
               timeout = 50;
            } else
            {
                // 多个包
                memcpy(pszResponse + nIndex, pszReply, nRet);
                nIndex += nRet; // 接收应答计数
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
USHORT CDevImpl_CRT591H::GetDataCRC(UCHAR *ucData, USHORT usDataSize, USHORT usInitCrc)
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

USHORT CDevImpl_CRT591H::Calc_CRC(USHORT usCrc, USHORT usCh)
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

INT CDevImpl_CRT591H::ConvertErrorCode(long lRet)
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

INT CDevImpl_CRT591H::ConvertErrorCode(CHAR szDeviceErr[3])
{


    return IMP_SUCCESS;
}

CHAR* CDevImpl_CRT591H::ConvertErrCodeToStr(long lRet)
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
    }
}

CHAR* CDevImpl_CRT591H::CmdToStr(LPSTR lpCmd)
{
    memset(m_szCmdStr, 0x00, sizeof(m_szCmdStr));


}

//---------------------------------------------
