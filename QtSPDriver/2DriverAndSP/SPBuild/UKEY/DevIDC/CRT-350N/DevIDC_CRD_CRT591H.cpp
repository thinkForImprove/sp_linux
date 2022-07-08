/***************************************************************
* 文件名称：DevIDC_CRD_CRT591H.cpp
* 文件描述：CRT-591H发卡模块底层命令处理
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年6月30日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevIDC_CRT.h"
#include "IDevCRD.h"
#include "../../DevCRD/CRT-591H/DevImpl_CRT591H.h"

//一个字节(char)按8位获取数据定义
#define     DATA_BIT0       (0x01)  //第一位
#define     DATA_BIT1       (0x02)
#define     DATA_BIT2       (0x04)
#define     DATA_BIT3       (0x08)
#define     DATA_BIT4       (0x10)
#define     DATA_BIT5       (0x20)
#define     DATA_BIT6       (0x40)
#define     DATA_BIT7       (0x80)

// 读取设备状态
int CDevIDC_CRT::GetDevStat(STCRDDEVSTATUS &stStat)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szSndCmdParam[64] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };
    CHAR    *szRcvDataTmp = nullptr;

    stStat.Clear();

    memcpy(szSndCmd, SND_CMD_STAT_SENSOR, STAND_CMD_LENGHT);

    // 下发命令
    nRet = ConvertErrCode_DevIDC2CRD(SendCmd(szSndCmd, szSndCmdParam, strlen(szSndCmdParam), ThisModule));
    if (nRet != CRD_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取设备状态: 命令下发: SendCmd(%s, %s, %d)失败, Return: %d.",
            szSndCmd, szSndCmdParam, strlen(szSndCmdParam), nRet);
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRcvDataLen = GetResponse_COM(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRcvDataLen < 0)
    {
        Log(ThisModule, __LINE__, "读取设备状态: 接收应答: 命令<%s>->GetResponse_COM(%s, %s, %d)失败, Return: %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, (nRcvDataLen));
        return ConvertErrCode_DevIDC2CRD(nRcvDataLen);
    }

    // 设备状态
    stStat.wDevice = DEVICE_STAT_ONLINE;    // 设备处于连接状态

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(szSndCmd, szRcvData, nRcvDataLen);
    if (nRet2 != CRD_SUCCESS)
    {
        return nRet2;
    }

    //
    if (nRcvDataLen < 21)
    {
        Log(ThisModule, __LINE__, "读取设备状态: 命令<%s> 返回Data=[%s], 有效Data位数[%d]<21,无效, Return: %d.",
            CmdToStr(szSndCmd), szRcvData, nRcvDataLen, TIMEOUT_WAIT_ACTION,
            ConvertErrCodeToStr(ERR_CRD_READERROR));
        return ERR_CRD_READERROR;
    }

    // 状态解析
    szRcvDataTmp = szRcvData + 8;
    if (szRcvDataTmp[1 + 0] == 0x31)    // 发卡箱1已放置
    {
        if ((szRcvDataTmp[0] & DATA_BIT4) == 0) // 发卡箱1无卡
        {
            stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_EMPTY; // 空
        } else
        {
            if ((szRcvDataTmp[0] & DATA_BIT5) == 0) // 发卡箱1卡满
            {
                stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_OK;
            } else
            {
                stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_LOW;   // 少
            }
        }
        stStat.stUnitInfo.wUnitCnt ++;
    } else
    {
        stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_MISSING;
    }

    if (szRcvDataTmp[1 + 3] == 0x31)    // 发卡箱2已放置
    {
        if ((szRcvDataTmp[0] & DATA_BIT1) == 0) // 发卡箱2无卡
        {
            stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_EMPTY; // 空
        } else
        {
            if ((szRcvDataTmp[0] & DATA_BIT2) == 0) // 发卡箱1卡满
            {
                stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_OK;
            } else
            {
                stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_LOW;   // 少
            }
        }
        stStat.stUnitInfo.wUnitCnt ++;
    } else
    {
        stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_MISSING;
    }

    // 单元状态
    if (stStat.stUnitInfo.wUnitCnt == 0) // 无发卡箱
    {
        stStat.wDispensr = DISP_STAT_STOP;
    } else
    if (stStat.stUnitInfo.wUnitCnt == 1) // 有1个发卡箱存在
    {
        stStat.wDispensr = DISP_STAT_STATE;
    } else
    {
        stStat.wDispensr = DISP_STAT_OK;
    }

    // 传输状态
    stStat.wTransport = TRANS_STAT_OK;

    // 介质状态
    if (szRcvDataTmp[1 + 14] == 0x31 || szRcvDataTmp[1 + 15] == 0x31)   // 发卡箱1/2出口有卡
    {
        stStat.wMedia = MEDIA_STAT_EXITING;
    } else
    {
        stStat.wMedia = MEDIA_STAT_NOTPRESENT;
    }

    // 门状态
    stStat.wShutter = SHUTTER_STAT_NOTSUPP;

    // 设备位置状态
    stStat.wDevicePos = DEVPOS_STAT_INPOS;

    // 反欺诈模块状态
    stStat.wAntiFraudMod = ANFRAUD_STAT_NOTSUPP;

    // 卡移动状态
    if (szRcvDataTmp[1 + 14] == 0x31)    // 发卡器出口有卡
    {
        stStat.wCardMoveStat = CAREMOVE_STAT_DISP_SIDE;
    } else
    {
        szRcvDataTmp = szRcvData + 6;
        if (memcmp(szRcvDataTmp, CRD_STAT_IDRWSIDE_HAVECARD, strlen(CRD_STAT_IDRWSIDE_HAVECARD)) == 0)
        {
            stStat.wCardMoveStat = CAREMOVE_STAT_IDRW_SIDE;     // 卡在读卡器内
        } else
        if (memcmp(szRcvDataTmp, CRD_STAT_IDRWENTR_HAVECARD, strlen(CRD_STAT_IDRWENTR_HAVECARD)) == 0)
        {
            stStat.wCardMoveStat = CAREMOVE_STAT_IDRW_ENTR;     // 卡在读卡器插卡口
        }
    }

    return CRD_SUCCESS;
}

// 发卡
int CDevIDC_CRT::DispenseCard(const int nUnitNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szSndCmdParam[64] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    memcpy(szSndCmd, SND_CMD_DISPENSE_CARD2, STAND_CMD_LENGHT);
    sprintf(szSndCmdParam, "%d", nUnitNo);

    // 下发命令
    nRet = ConvertErrCode_DevIDC2CRD(SendCmd(szSndCmd, szSndCmdParam, strlen(szSndCmdParam), ThisModule));
    if (nRet != CRD_SUCCESS)
    {
        Log(ThisModule, __LINE__, "发卡: 命令下发: SendCmd(%s, %s, %d)失败, Return: %d.",
            szSndCmd, szSndCmdParam, strlen(szSndCmdParam), nRet);
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRcvDataLen = GetResponse_COM(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRcvDataLen < 0)
    {
        Log(ThisModule, __LINE__, "发卡: 接收应答: 命令<%s>->GetResponse_COM(%s, %s, %d)失败, Return: %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, (nRcvDataLen));
        return ConvertErrCode_DevIDC2CRD(nRcvDataLen);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(szSndCmd, szRcvData, nRcvDataLen);
    if (nRet2 != CRD_SUCCESS)
    {
        return nRet2;
    }

    return CRD_SUCCESS;
}

// 获取版本号(1DevCRM版本/2固件版本/3设备软件版本/4其他)
void CDevIDC_CRT::GetVersion(char* szVer, long lSize, ushort usType)
{
    CHAR    szVersion[128] = { 0x00 };

    switch(usType)
    {
        case 1 :
            break;
        case 2: // 固件版本
            GetFWVersion(szVersion);
            break;
        case 4:
            GetSerialNumer(szVersion);
            break;
    }

    memcpy(szVer, szVersion, strlen((char*)szVersion) > lSize ? lSize : strlen((char*)szVersion));
}

// 取发卡机固件版本
INT CDevIDC_CRT::GetFWVersion(LPSTR lpVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    memcpy(szSndCmd, SND_CMD_INIT_NORMAL, STAND_CMD_LENGHT);
    szSndCmd[1] = 0xA4;

    // 下发命令
    nRet = ConvertErrCode_DevIDC2CRD(SendCmd(szSndCmd, nullptr, 0, ThisModule));
    if (nRet != CRD_SUCCESS)
    {
        Log(ThisModule, __LINE__, "取发卡机固件版本: 命令下发: SendCmd(%s, NULL, 0)失败, Return: %d.",
            szSndCmd, nRet);
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRcvDataLen = GetResponse_COM(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRcvDataLen < 0)
    {
        Log(ThisModule, __LINE__, "取发卡机固件版本: 接收应答: 命令<%s>->GetResponse_COM(%s, %s, %d)失败, Return: %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, (nRcvDataLen));
        return ConvertErrCode_DevIDC2CRD(nRcvDataLen);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(szSndCmd, szRcvData, nRcvDataLen);
    if (nRet2 != CRD_SUCCESS)
    {
        return nRet2;
    }

    if (nRcvDataLen < 10)
    {
        Log(ThisModule, __LINE__, "取发卡机固件版本: 命令<%s> 返回Data=[%s], 有效Data位数[%d]<10,无效, Return: %d.",
            CmdToStr(szSndCmd), szRcvData, nRcvDataLen, TIMEOUT_WAIT_ACTION,
            ConvertErrCodeToStr(ERR_CRD_READERROR));
        return ERR_CRD_READERROR;
    }

    memcpy(lpVer, szRcvData + 9, nRcvDataLen - 9);

    return 0;
}

// 取发卡机序列号
INT CDevIDC_CRT::GetSerialNumer(LPSTR lpVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);
    VERTIFYISOPEN();
    LOGDEVACTION();

    INT     nRet = 0;
    CHAR    szSndCmd[24] = { 0x00 };
    CHAR    szRcvData[2068] = { 0x00 };

    memcpy(szSndCmd, SND_CMD_INIT_NORMAL, STAND_CMD_LENGHT);
    szSndCmd[1] = 0xA2;

    // 下发命令
    nRet = ConvertErrCode_DevIDC2CRD(SendCmd(szSndCmd, nullptr, 0, ThisModule));
    if (nRet != CRD_SUCCESS)
    {
        Log(ThisModule, __LINE__, "取发卡机序列号: 命令下发: SendCmd(%s, NULL, 0)失败, Return: %d.",
            szSndCmd, nRet);
        return nRet;
    }

    // 接收应答
    INT nRcvDataLen = 0;
    nRcvDataLen = GetResponse_COM(szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, ThisModule);
    if (nRcvDataLen < 0)
    {
        Log(ThisModule, __LINE__, "取发卡机序列号: 接收应答: 命令<%s>->GetResponse_COM(%s, %s, %d)失败, Return: %d.",
            szSndCmd, szRcvData, sizeof(szRcvData), TIMEOUT_WAIT_ACTION, (nRcvDataLen));
        return ConvertErrCode_DevIDC2CRD(nRcvDataLen);
    }

    // 应答数据Check
    INT nRet2 =  RcvDataCheck(szSndCmd, szRcvData, nRcvDataLen);
    if (nRet2 != CRD_SUCCESS)
    {
        return nRet2;
    }

    if (nRcvDataLen < 10)
    {
        Log(ThisModule, __LINE__, "取发卡机序列号: 命令<%s> 返回Data=[%s], 有效Data位数[%d]<10,无效, Return: %d.",
            CmdToStr(szSndCmd), szRcvData, nRcvDataLen, TIMEOUT_WAIT_ACTION,
            ConvertErrCodeToStr(ERR_CRD_READERROR));
        return ERR_CRD_READERROR;
    }

    memcpy(lpVer, szRcvData + 9, nRcvDataLen - 9);

    return 0;
}
// 设置数据
int CDevIDC_CRT::SetData(void *vData, WORD wDataType)
{
    switch(wDataType)
    {
        case 99:
            m_IDCInterProtocal = *((USHORT*)(vData));
            break;
        default:
            break;
    }
}


// COM口连接方式
INT CDevIDC_CRT::SendSinglePacket_COM(const char* pszCmd, const char *lpData, int nLen, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    char buf[2068] = {0};
    int nRet = 0;
    BYTE dwReportID = {0};
    int nbufSize = 0;

    // one packet
    dwReportID = CRT_REPORT_ID_COM;
    buf[0] = (char)dwReportID;
    buf[1] = (char)((nLen + 3) / 256);
    buf[2] = (char)((nLen + 3) % 256);
    //TEXT命令
    memcpy(buf + 3, pszCmd, 3);

    if (lpData != nullptr)
    {
        memcpy(buf + 6, lpData, nLen);
    }

    nbufSize = nLen + 6;

    // 计算CRC
    USHORT usCRC = GetDataCRC((UCHAR*)buf, (USHORT)nbufSize);
    buf[nbufSize] = (usCRC >> 8) & 0xFF;    // 高位
    buf[nbufSize + 1] = usCRC & 0xFF;       // 低位
    nbufSize = nbufSize + 2;

    // 发送数据记录到Log
    /*CHAR szTmp[12] = { 0x00 };
    std::string stdSndData;
    for (int i = 0; i < nbufSize; i ++)
    {
        sprintf(szTmp, "%02X ", (int)buf[i]);
        stdSndData.append(szTmp);
    }
    Log(ThisModule, __LINE__, "SendData To Device: CMD<%s> %d|%s",
        pszCmd, nbufSize, stdSndData.c_str());*/

    for (int iIndex = 0; iIndex < 4; iIndex++)
    {
        nRet = m_pDev->Send(buf, nbufSize, TIMEOUT_WAIT_ACTION);

        if (nRet != m_nLastError && nRet != ERR_DEVPORT_SUCCESS && nRet != nbufSize)
        {
            if (nRet == ERR_DEVPORT_WRITE)
                Log(ThisModule, __LINE__, "%s: 读卡器没有响应", pszCallFunc);
            else
                Log(ThisModule, __LINE__, "%s: SendData()出错，返回%d", pszCallFunc, nRet);
        }
        m_nLastError = nRet;
        if (nRet < 0)
        {
            if (nRet == ERR_DEVPORT_NOTOPEN || nRet == ERR_DEVPORT_WRITE)
            {
                Close();
                int nRetTemp = Open(m_strMode.c_str());
                if ((nRetTemp == ERR_DEVPORT_SUCCESS) && (iIndex < 3))
                {
                    m_bReOpen = TRUE;
                    continue;
                }
            }
            m_bReOpen = FALSE; //重连失败
            return ConvertErrorCode(nRet);
        }
        else
            break;
    }

    return ERR_IDC_SUCCESS;
}

/*
  功能：读取读卡器的返回数据(COM方式)
  参数：pszReponse返回数据的缓冲区，nLen缓冲区长度，nTimeout超时(毫秒)，pWaitCancel是IWaitCancel指针
  返回：>0数据长度，=0错误
*/
INT CDevIDC_CRT::GetResponse_COM(char *pszResponse, int nLen, int nTimeout, const char *pszCallFunc)
{
    THISMODULE(__FUNCTION__);
    Q_UNUSED(nLen)

    char pszReply[COM_READ_LENTH];
    int nRet = 0;
    int nIndex = 0;
    int timeout = nTimeout;
    int nReadCnt = 0;

    if (m_pDev == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pDev == null, Device Not Open, Return: %d",
            ERR_IDC_NOT_OPEN);
        return ERR_IDC_NOT_OPEN;
    }

    while(TRUE)
    {
        DWORD ulInOutLen = COM_READ_LENTH;
        memset(pszReply, 0, sizeof(pszReply));
        nReadCnt = 1;
        nRet = m_pDev->Read(pszReply, ulInOutLen, timeout);
        if (nRet < 0 || ulInOutLen < 1)
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
            if (pszReply[0] == CRT_ACK_COM)
            {
                /*Log(ThisModule, __LINE__, "RecvData To Device: %d|%s\n"
                                          "RecvData Byte[0] == %02X|ACK(确认下发CMD有效), 不做处理.",
                    nRet, stdRcvData.c_str(), CRT_ACK_COM);*/
                if (ulInOutLen > 1)
                {
                    memcpy(pszResponse + nIndex, pszReply + 1, ulInOutLen -1);
                    nIndex += (ulInOutLen - 1);
                }
                continue;
            } else
            if (pszReply[0] == CRT_NAK_COM)
            {
                /*Log(ThisModule, __LINE__, "RecvData To Device: %d|%s\n"
                                          "RecvData Byte[0] == %02X|NAK(否认,下发命令无效), Return: %d.",
                    nRet, stdRcvData.c_str(), ERR_DEVPORT_NODEFINED);*/
                return ERR_IDC_READERROR; // 无效下发数据
            } /*else
            {
                Log(ThisModule, __LINE__, "RecvData To Device: %d|%s.",
                    nRet, stdRcvData.c_str());
            }*/

            memcpy(pszResponse + nIndex, pszReply, ulInOutLen);
            nIndex += ulInOutLen;

            if (nIndex < (MAKEWORD(pszResponse[2], pszResponse[1]) + 3 + 2))
            {
                continue;
            }

            break;
        }
    }

    char szSndACK[2] = { 0x06, 0x00 };
    m_pDev->Send(szSndACK, 1, TIMEOUT_WAIT_ACTION);

    DWORD nRecvLen = MAKEWORD(pszResponse[2], pszResponse[1]);
    if (nRecvLen != 0) {
        if(nIndex < nRecvLen + 3 + 2){
            Log(ThisModule, __LINE__, "%s: 读卡器返回数据接收不完整", pszCallFunc);
            return ERR_IDC_READERROR;
        }
        return nRecvLen + 3 + 2;
    } else {
        Log(ThisModule, __LINE__, "%s: 读卡器返回数据错误", pszCallFunc);
        return ERR_IDC_READERROR;
    }
}

/************************************************************
 * 功能: 应答数据Check
 * 参数: lpcSndCmd 入参 下发命令
 *      lpcRcvData 入参 应答数据
 *      nRcvDataLen 入参 应答数据长度
 *                  回参 去掉CRC(2Byte)后的应答数据长度
 * 返回值: 参考错误码
************************************************************/
INT CDevIDC_CRT::RcvDataCheck(LPCSTR lpcSndCmd, LPCSTR lpcRcvData, INT &nRcvDataLen)
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
            lpcSndCmd, nRcvDataLen, ConvertErrCodeToStr(ERR_CRD_DATA_INVALID));
        return ERR_CRD_DATA_INVALID;
    }

    // 检查应答命令与发送命令是否一致
    if (memcmp(lpcSndCmd + 1, lpcRcvData + RCVCMDPOS + 1, STAND_CMD_LENGHT - 1) != 0)
    {
        Log(ThisModule, __LINE__, "CMD<%s> 返回无效的命令应答数据[%02X,%02X,%02X], 命令不一致, Return: %s.",
            lpcSndCmd, (int)(lpcRcvData[RCVCMDPOS]), (int)(lpcRcvData[RCVCMDPOS + 1]),
            (int)(lpcRcvData[RCVCMDPOS + 2]), ConvertErrCodeToStr(ERR_CRD_DATA_INVALID));
        return ERR_CRD_DATA_INVALID;
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
            usCRC, ConvertErrCodeToStr(ERR_CRD_DATA_INVALID));
        return ERR_CRD_DATA_INVALID;
    }

    // 错误应答数据检查
    if (memcmp(lpcRcvData + RCVCMDPOS, "N", 1) == 0) // 错误应答
    {
        CHAR    szErrCode[2+1] = { 0x00 };
        memcpy(szErrCode, lpcRcvData + RCVSTATPOS, 2);
        Log(ThisModule, __LINE__, "CMD<%s> 返回错误应答数据[CMD: %02X,%02X,%02X; STAT: %02X,%02X,%02X], "
                                  "Return %s.",
            CmdToStr((LPSTR)lpcSndCmd), (int)(lpcRcvData[RCVCMDPOS]), (int)(lpcRcvData[RCVCMDPOS + 1]),
            (int)(lpcRcvData[RCVCMDPOS + 2]), (int)(lpcRcvData[RCVSTATPOS]), (int)(lpcRcvData[RCVSTATPOS + 1]),
            (int)(lpcRcvData[RCVSTATPOS + 2]), ConvertErrCodeToStr(ConvertErrCode_CRT2Dev(szErrCode)));
        return ConvertErrCode_CRT2Dev(szErrCode);
    }

    // 返回应答数据长度(STX+LEN+CMD+DATA)
    nRcvDataLen = MAKEWORD(lpcRcvData[2], lpcRcvData[1]) + 3;

    return CRD_SUCCESS;
}

CHAR* CDevIDC_CRT::CmdToStr(LPSTR lpCmd)
{
    memset(m_szCRDCmdStr, 0x00, sizeof(m_szCRDCmdStr));

    if (strcmp(lpCmd, SND_CMD_DISPENSE_CARD2) == 0)
    {
        sprintf(m_szCRDCmdStr, "%s", "C22|发卡:选择卡箱发卡到ICRW");
        return m_szCRDCmdStr;
    } else
    if (strcmp(lpCmd, SND_CMD_STAT_SENSOR) == 0)
    {
        sprintf(m_szCRDCmdStr, "%s", "C1A|状态查询:传感器状态");
        return m_szCRDCmdStr;
    } else
    {
        if (lpCmd[0] == 'C' && lpCmd[1] == 0xA4 && lpCmd[2] == '0')
        {
            sprintf(m_szCRDCmdStr, "%s", "C[0xA4]0|取版本:固件版本");
            return m_szCRDCmdStr;
        } else
        if (lpCmd[0] == 'C' && lpCmd[1] == 0xA2 && lpCmd[2] == '0')
        {
            sprintf(m_szCRDCmdStr, "%s", "C[0xA2]0|取版本:序列号");
            return m_szCRDCmdStr;
        } else
        {
            sprintf(m_szCRDCmdStr, "%s|未知", lpCmd);
            return m_szCRDCmdStr;
        }
    }
}

CHAR* CDevIDC_CRT::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szCRDErrStr, 0x00, sizeof(m_szCRDErrStr));

    switch(nRet)
    {
        case CRD_SUCCESS:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "操作成功");
            return m_szCRDErrStr;
        case ERR_CRD_INSERT_TIMEOUT:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "进卡超时");
            return m_szCRDErrStr;
        case ERR_CRD_USER_CANCEL:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "用户取消");
            return m_szCRDErrStr;
        case ERR_CRD_COMM_ERR:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "通讯错误");
            return m_szCRDErrStr;
        case ERR_CRD_JAMMED:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "堵卡");
            return m_szCRDErrStr;
        case ERR_CRD_OFFLINE:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "脱机");
            return m_szCRDErrStr;
        case ERR_CRD_NOT_OPEN:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "没有打开");
            return m_szCRDErrStr;
        case ERR_CRD_UNIT_FULL:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "卡箱满");
            return m_szCRDErrStr;
        case ERR_CRD_UNIT_EMPTY:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "卡箱空");
            return m_szCRDErrStr;
        case ERR_CRD_UNIT_NOTFOUND:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "卡箱不存在");
            return m_szCRDErrStr;
        case ERR_CRD_HWERR:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "硬件故障");
            return m_szCRDErrStr;
        case ERR_CRD_STATUS_ERR:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "状态出错");
            return m_szCRDErrStr;
        case ERR_CRD_UNSUP_CMD:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "不支持的指令");
            return m_szCRDErrStr;
        case ERR_CRD_PARAM_ERR:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "参数错误");
            return m_szCRDErrStr;
        case ERR_CRD_READTIMEOUT:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "读数据超时");
            return m_szCRDErrStr;
        case ERR_CRD_WRITETIMEOUT:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "写数据超时");
            return m_szCRDErrStr;
        case ERR_CRD_READERROR:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "读数据错");
            return m_szCRDErrStr;
        case ERR_CRD_WRITEERROR:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "写数据错");
            return m_szCRDErrStr;
        case ERR_CRD_DATA_INVALID:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "无效数据");
            return m_szCRDErrStr;
        case ERR_CRD_LOAD_LIB:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "动态库错误");
            return m_szCRDErrStr;
        //case ERR_CRD_OTHER:
        default:
            sprintf(m_szCRDErrStr, "%d|%s", nRet, "其他错误/未知错误");
            return m_szCRDErrStr;
    }
}

INT CDevIDC_CRT::ConvertErrCode_CRT2Dev(CHAR szDeviceErr[3])
{
    if (memcmp(szDeviceErr, CRT_ERR_CMD_INVALID, 2) == 0) // 命令编码未定义
    {
        return ERR_CRD_WRITEERROR;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_CMD_PARAM, 2) == 0) // 命令参数错误
    {
        return ERR_CRD_WRITEERROR;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_CMD_DATA, 2) == 0) // 命令数据错误
    {
        return ERR_CRD_WRITEERROR;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_CMD_UNABLE, 2) == 0) // 命令无法执行
    {
        return ERR_CRD_WRITEERROR;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_POWER_VOLLOW, 2) == 0) // 电源电压过低(低于20V)
    {
        return ERR_CRD_HWERR;
    }
    if (memcmp(szDeviceErr, CRT_ERR_POWER_VOLHIGH, 2) == 0)// 电源电压过高(高于28V)
    {
        return ERR_CRD_HWERR;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_POWER_NEEDINIT, 2) == 0)// 电源恢复正常但没有重新初始化
    {
        return ERR_CRD_HWERR;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_POWER_FRAM, 2) == 0)// 模块内部FRAM错误
    {
        return ERR_CRD_HWERR;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_INCARD_JAM, 2) == 0)// 进卡阻塞
    {
        return ERR_CRD_JAMMED;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_EJECT_JAM, 2) == 0)// 退卡堵塞
    {
        return ERR_CRD_JAMMED;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_POWER_LOCAT_JAM, 2) == 0)// 电机定位堵塞
    {
        return ERR_CRD_JAMMED;
    } else
    if (memcmp(szDeviceErr, CRT_ERR_UNIT_EMPTY, 2) == 0)// 卡箱空
    {
        return ERR_CRD_UNIT_EMPTY;
    } else
    {
        return IMP_ERR_UNKNOWN;     // 未知错误
    }

    return CRD_SUCCESS;
}

INT CDevIDC_CRT::ConvertErrCode_DevIDC2CRD(INT nRet)
{
    switch(nRet)
    {
        case ERR_IDC_SUCCESS:
            return CRD_SUCCESS;
        case ERR_IDC_INSERT_TIMEOUT:    // 进卡超时
            return ERR_CRD_INSERT_TIMEOUT;
        case ERR_IDC_USER_CANCEL:       // 用户取消进卡
            return ERR_CRD_USER_CANCEL;
        case ERR_IDC_COMM_ERR:          // 通讯错误
            return ERR_CRD_COMM_ERR;
        case ERR_IDC_JAMMED:            // 读卡器堵卡
            return ERR_CRD_JAMMED;
        case ERR_IDC_OFFLINE:           // 读卡器脱机
            return ERR_CRD_OFFLINE;
        case ERR_IDC_NOT_OPEN:          // 读卡器没有打开
            return ERR_CRD_NOT_OPEN;
        case ERR_IDC_RETAINBINFULL:     // 回收箱满
            return ERR_CRD_UNIT_FULL;
        case ERR_IDC_HWERR:             // 硬件故障
            return ERR_CRD_HWERR;
        case ERR_IDC_STATUS_ERR:        // 读卡器其他状态出错
            return ERR_CRD_STATUS_ERR;
        case ERR_IDC_UNSUP_CMD:         // 不支持的指令
            return ERR_CRD_UNSUP_CMD;
        case ERR_IDC_PARAM_ERR:         // 参数错误
            return ERR_CRD_PARAM_ERR;
        case ERR_IDC_READTIMEOUT:       // 读数据超时
            return ERR_CRD_READTIMEOUT;
        case ERR_IDC_WRITETIMEOUT:      // 写数据超时
            return ERR_CRD_WRITETIMEOUT;
        case ERR_IDC_READERROR:         // 读卡错
            return ERR_CRD_READERROR;
        case ERR_IDC_WRITEERROR:        // 写卡错
            return ERR_CRD_WRITEERROR;
        case ERR_IDC_NO_DEVICE:         // 指定的设备不存在
            return ERR_CRD_UNIT_NOTFOUND;
        case ERR_IDC_OTHER:             // 其它错误
            return ERR_CRD_OTHER;
        default:
            return ERR_CRD_OTHER;
    }
}

/************************************************************
 * 功能：计算数据CRC
 * 参数：ucData 数据
 *      usDataSize 数据长度
 *      usInitCrc 初始CRC,缺省0x00
 * 返回：CRC
************************************************************/
USHORT CDevIDC_CRT::GetDataCRC(UCHAR *ucData, USHORT usDataSize, USHORT usInitCrc)
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

USHORT CDevIDC_CRT::Calc_CRC(USHORT usCrc, USHORT usCh)
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
