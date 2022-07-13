/***************************************************************************
* 文件名称: DevImpl_NT0861.cpp
* 文件描述: 封装NT0861模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年7月13日
* 文件版本: 1.0.0.1
***************************************************************************/
#include "DevImpl_NT0861.h"
#include "data_convertor.h"
#include "device_port.h"

static const char *ThisFile = "DevImpl_NT0861.cpp";

CDevImpl_NT0861::CDevImpl_NT0861()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_NT0861::CDevImpl_NT0861(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_NT0861::CDevImpl_NT0861(LPSTR lpLog, LPCSTR lpDevType)
{
    SetLogFile(lpLog, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件
    MSET_0(m_szDevType);
    MCPY_NOLEN(m_szDevType, strlen(lpDevType) > 0 ? lpDevType : "NT0861");
    Init();
}

// 参数初始化
void CDevImpl_NT0861::Init()
{
    m_strMode.clear();                              // 连接串
    m_bDevOpenOk = FALSE;                           // 设备是否Open
    m_bReCon = FALSE;                               // 是否断线重连状态
    MSET_0(m_szDevType);                            // 设备类型
    MSET_0(m_szErrStr);                             // IMPL错误解释
    MSET_0(m_szCmdStr);                             // 命令解释
    m_dwSndTimeOut = TIMEOUT_WAIT_ACTION;           // 命令下发超时时间(毫秒)
    m_dwRcvTimeOut = TIMEOUT_WAIT_ACTION;           // 命令接收超时时间(毫秒)
    memset(m_nRetErrOLD, 0, sizeof(INT) * 8);       // 处理错误值保存
    memset(m_szVersion, 0x00, sizeof(CHAR) * 4 * 128);// 版本保存数组
    MSET_0(m_szDevName);                            // 设备名
}

CDevImpl_NT0861::~CDevImpl_NT0861()
{
    //vUnLoadLibrary();
}


/***************************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参[串口格式]
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_NT0861::OpenDevice(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT Ret = IMP_SUCCESS;

    if (m_pDev == nullptr)
    {
        if (m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "BCR", m_szDevType) != 0)
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

    // 入参为null,使用缺省
    if (lpMode == nullptr || strlen(lpMode) < 1)
    {
        if (m_strMode.empty())
        {
            char szDevID[MAX_PATH] = {0};
            sprintf(szDevID, "%s", SERIAL_DEF);
            m_strMode = szDevID;
            Log(ThisModule, __LINE__, "打开设备: 入参为null,使用缺省串口[%s].",
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

    // 取设备名(用于查询设备是否断线)
    CDevicePort::GetComDevName(m_strMode.c_str(), m_szDevName, sizeof(m_szDevName));

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    // 根据Open或断线重连Open记录不同Log
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

    // 取所有版本保存
    if (GetVersion(0, nullptr, 0) == IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "取版本: FW:%s, P/N:%s, S/N:%s, C/N:%s",
            m_szVersion[0], m_szVersion[1], m_szVersion[2], m_szVersion[3]);
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_NT0861::CloseDevice()
{
    if (m_bDevOpenOk == TRUE)
    {
        if (m_pDev == nullptr)
        {
            return IMP_SUCCESS;
        }
        m_pDev->Close();
        //m_pDev.Release();
    }

    m_bDevOpenOk = FALSE;

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 释放动态库
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_NT0861::Release()
{
    return CloseDevice();
}

/***************************************************************************
 * 功能: 设备是否Open
 * 参数: 无
 * 返回值: TRUT:Open, FALSE:未Open
***************************************************************************/
BOOL CDevImpl_NT0861::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/***************************************************************************
 * 功能: 模块初始化
 * 参数: nParam 参数
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_NT0861::DeviceInit(INT nParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //LOGDEVACTION();

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 获取设备状态
 * 参数: 无
 * 返回值: 参考[enum EN_DEVSTAT]
***************************************************************************/
INT CDevImpl_NT0861::GetDeviceStat()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //LOGDEVACTION();

    // Utilities工程下device_port, Pro需加载libudev库
    // 设备不存在, Open状态下无设备为断线, 否则为无设备
    if (CDevicePort::SearchDeviceNameIsHave(m_szDevName) == DP_RET_NOTHAVE)
    {
        if (m_bDevOpenOk == TRUE)   //
        {
            return DEV_OFFLINE;
        }
        return DEV_NOTFOUND;
    }

    // 设备存在, Open状态下返回成功, 否则为未打开
    if (m_bDevOpenOk == TRUE)
    {
        return DEV_OK;
    } else
    {
        return DEV_NOTOPEN;
    }
}

/***************************************************************************
 * 功能: 取版本号
 * 参数: wType 类别(0:固件/1PN/2SN/3CN)
 *      lpVerStr 返回版本内容
 *      wVerSize lpVerStr传入空间Size
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_NT0861::GetVersion(WORD wType, LPSTR lpVerData, WORD wVerSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT nRet = IMP_SUCCESS;
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    // USB/COM动态库句柄检查
    CHK_DEVHANDLE(m_pDev);

    // 版本未提前获取时立即获取并保存
    if (strlen(m_szVersion[0]) == 0 || strlen(m_szVersion[1]) == 0 ||
        strlen(m_szVersion[2]) == 0 || strlen(m_szVersion[3]) == 0)
    {
        // 命令收发检查
        nRet = SndRcvToChk(SND_CMD_GETVER, szRcvData, nRcvSize, TRUE, "取版本号");
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "取版本号: 命令收发: ->SndRcvToChk(%d, TRUE)失败, ErrCode: %d, Return %s.",
                SND_CMD_GETVER, nRet, ConvertCode_Impl2Str(nRet));
            return nRet;
        }

        // 解析数据
        ULONG ulArraySize = DataConvertor::split_string(szRcvData, '\n', nullptr, 0);
        char szFieldNameArray[ulArraySize][CONST_VALUE_260];
        DataConvertor::split_string(szRcvData, '\n', szFieldNameArray, ulArraySize);

        for (INT i = 0; i < ulArraySize; i ++)
        {
            DataConvertor::trim_string_by_end_char(szFieldNameArray[i], '\r', szFieldNameArray[i], CONST_VALUE_260);
            if (memcmp(szFieldNameArray[i], "Firmware Version:", strlen("Firmware Version:")) == 0)  // FW
            {
                MSET_0(m_szVersion[0]);
                DataConvertor::trim_string(szFieldNameArray[i] + strlen("Firmware Version:"),
                                           m_szVersion[0], sizeof(m_szVersion[0]));
            } else
            if (memcmp(szFieldNameArray[i], "P/N:", strlen("P/N:")) == 0)  // PN
            {
                MSET_0(m_szVersion[1]);
                DataConvertor::trim_string(szFieldNameArray[i] + strlen("P/N:"),
                                           m_szVersion[1], sizeof(m_szVersion[1]));
            } else
            if (memcmp(szFieldNameArray[i], "S/N:", strlen("S/N:")) == 0)  // SN
            {
                MSET_0(m_szVersion[2]);
                DataConvertor::trim_string(szFieldNameArray[i] + strlen("S/N:"),
                                           m_szVersion[2], sizeof(m_szVersion[2]));
            } else
            if (memcmp(szFieldNameArray[i], "C/N:", strlen("C/N:")) == 0)  // CN
            {
                MSET_0(m_szVersion[3]);
                DataConvertor::trim_string(szFieldNameArray[i] + strlen("C/N:"),
                                           m_szVersion[3], sizeof(m_szVersion[3]));
            }
        }
    }

    if (lpVerData == nullptr || wVerSize == 0)
    {
        return IMP_SUCCESS;
    }

    INT nVerLenTmp = 0;
    if (wType == 0)
    {
        nVerLenTmp = (wVerSize > strlen(m_szVersion[0]) ? strlen(m_szVersion[0]) : wVerSize -1);
        MCPY_LEN(lpVerData, m_szVersion[0], nVerLenTmp);
    } else
    if (wType == 1)
    {
        nVerLenTmp = (wVerSize > strlen(m_szVersion[1]) ? strlen(m_szVersion[1]) : wVerSize -1);
        MCPY_LEN(lpVerData, m_szVersion[1], nVerLenTmp);
    } else
    if (wType == 2)
    {
        nVerLenTmp = (wVerSize > strlen(m_szVersion[2]) ? strlen(m_szVersion[2]) : wVerSize -1);
        MCPY_LEN(lpVerData, m_szVersion[2], nVerLenTmp);
    } else
    if (wType == 3)
    {
        nVerLenTmp = (wVerSize > strlen(m_szVersion[3]) ? strlen(m_szVersion[3]) : wVerSize -1);
        MCPY_LEN(lpVerData, m_szVersion[3], nVerLenTmp);
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 开始扫码
 * 参数: nMode
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_NT0861::ScanCodeStart(INT nMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT nRet = IMP_SUCCESS;
    CHAR szRcvData[2068] = { 0x00 };
    INT nRcvSize = sizeof(szRcvData);

    // USB/COM动态库句柄检查
    CHK_DEVHANDLE(m_pDev);

    // 命令收发检查
    nRet = SndRcvToChk(SND_CMD_SCAN_S, szRcvData, nRcvSize, TRUE, "开始扫码");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "开始扫码: 命令收发: ->SndRcvToChk(%d, TRUE)失败, ErrCode: %d, Return %s.",
            SND_CMD_SCAN_S, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 停止扫码
 * 参数: nMode
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_NT0861::ScanCodeEnd(INT nMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT nRet = IMP_SUCCESS;
    CHAR    szRcvData[2068] = { 0x00 };
    INT     nRcvSize = sizeof(szRcvData);

    // USB/COM动态库句柄检查
    CHK_DEVHANDLE(m_pDev);

    // 命令收发检查
    nRet = SndRcvToChk(SND_CMD_SCAN_E, szRcvData, nRcvSize, TRUE, "停止扫码");
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "停止扫码: 命令收发: ->SndRcvToChk(%d, TRUE)失败, ErrCode: %d, Return %s.",
            SND_CMD_SCAN_E, nRet, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 获取扫码数据
 * 参数: lpCodeData  回参 数据buffer
 *      nCodeDataSize 入参 数据Buffer大小, 回参 数据有效长度
 *      nCodeType   回参 条码类型
 *      dwTimeOut   入参 超时时间(毫秒)
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_NT0861::GetScanCode(LPSTR lpCodeData, INT &nCodeDataSize, INT &nCodeType, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    CHAR szRcvData[2068] = { 0x00 };
    INT nRcvSize = 0;
    INT nCodeDataLenTmp = nCodeDataSize;
    nCodeType = EN_CODE_UNKNOWN;         // 条码类型初始未知

    // USB/COM动态库句柄检查
    CHK_DEVHANDLE(m_pDev);

    // 接收扫描应答数据
    MSET_0(szRcvData);
    nRcvSize = sizeof(szRcvData);
    nRcvSize = GetResponse(szRcvData, sizeof(szRcvData), dwTimeOut, "获取扫码数据", FALSE);
    if (nRcvSize < 0)
    {
        if (m_nRetErrOLD[3] != nRcvSize)
        {
            Log(ThisModule, __LINE__,
                "获取扫码数据: ->GetResponse(%s, %d, %d, %s)失败, ErrCode: %d, Return %s.",
                szRcvData, nRcvSize, dwTimeOut, "获取扫码数据",
                nRcvSize, ConvertCode_Impl2Str(nRcvSize));
            m_nRetErrOLD[3] = nRcvSize;
        }
        return nRcvSize;
    } else
    {
        if (m_nRetErrOLD[3] != nRcvSize)
        {
            Log(ThisModule, __LINE__,
                "获取扫码数据: ->GetResponse(%s, %d, %d, %s)成功, ErrCode: %d, Return %s.",
                szRcvData, nRcvSize, dwTimeOut, "获取扫码数据",
                nRcvSize, ConvertCode_Impl2Str(nRcvSize));
            m_nRetErrOLD[3] = nRcvSize;
        }
    }

    // 解析数据
    if (nRcvSize > 0)
    {
        nCodeType = ConvertSymMode(szRcvData);      // 条码类型
        // 条码数据处理
        nCodeDataSize = (nCodeDataLenTmp > (nRcvSize - 1) ? (nRcvSize - 1) : nCodeDataLenTmp - 1);
        MCPY_LEN(lpCodeData, szRcvData + 1, nCodeDataSize);
    } else
    {
        nCodeDataSize = 0;
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 下发数据到Device
 * 参数: lpcCmd     入参 命令数据
 *      nCmdLen     入参 命令数据长度
 *      dwTimeOut   入参 超时时间(毫秒)
 *      lpFuncData  入参 日志输出字串
 *      bIsPrtLog   入参 是否打印日志
 * 返回值: 参考IMPL错误码
***************************************************************************/
INT CDevImpl_NT0861::SendCmd(LPCSTR lpcCmd, INT nCmdLen, DWORD dwTimeOut,
                             LPCSTR lpFuncData, BOOL bIsPrtLog/* = TRUE*/)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 组织命令数据串
    CHAR szCmdBuffer[2068] = { 0x00 };              // 下发命令
    INT nCmdBuffSize = 0;                           // 下发命令长度
    INT nRet = ERR_DEVPORT_SUCCESS;

    // 包头
    szCmdBuffer[0] = (CHAR)REPORTID;                // 特征位
    szCmdBuffer[1] = (CHAR)RESERVED;                // 保留位
    szCmdBuffer[2] = (CHAR)(nCmdLen / 256);         // 包长度高位字节
    szCmdBuffer[3] = (CHAR)(nCmdLen % 256);         // 包长度低位字节
    nCmdBuffSize += 4;

    // 命令数据
    memcpy(szCmdBuffer + nCmdBuffSize, lpcCmd, nCmdLen);    // 追加命令
    nCmdBuffSize += nCmdLen;

    // 取校验和
    /*DWORD dwCheckSum = 0;
    dwCheckSum = GetCheckSum(szCmdBuffer, nCmdBuffSize);
    szCmdBuffer[nCmdBuffSize ++] = ((dwCheckSum & 0xFF000000) > 0 ? (dwCheckSum & 0xFF000000) >> 24 : 0x00);
    szCmdBuffer[nCmdBuffSize ++] = ((dwCheckSum & 0x00FF0000) > 0 ? (dwCheckSum & 0x00FF0000) >> 16 : 0x00);
    szCmdBuffer[nCmdBuffSize ++] = ((dwCheckSum & 0x0000FF00) > 0 ? (dwCheckSum & 0x0000FF00) >> 8 : 0x00);
    szCmdBuffer[nCmdBuffSize ++] = ((dwCheckSum & 0x000000FF) > 0 ? (dwCheckSum & 0x000000FF) >> 0 : 0x00);*/

    // 命令下发
    nRet = m_pDev->Send(szCmdBuffer, nCmdBuffSize, dwTimeOut);
    if (nRet != ERR_DEVPORT_SUCCESS)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "%s: 命令下发: ->COM::Send(%s, %d, %d) Fail, ErrCode: %d, Return: %s",
                lpFuncData, szCmdBuffer, nCmdBuffSize, dwTimeOut,
                nRet, ConvertCode_Impl2Str(nRet));
            return nRet;
        }
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 读取设备的返回数据
 * 参数: lpRespData  回参 返回数据的缓冲区
 *      nRespDataLen 入参 缓冲区长度
 *      dwTimeOut   入参 超时时间(毫秒)
 *      lpFuncData  入参 日志输出字串
 *      bIsPrtLog   入参 是否打印日志
 * 返回: >=0数据长度，<0错误
***************************************************************************/
INT CDevImpl_NT0861::GetResponse(LPSTR lpRespData, INT nRespDataLen, DWORD dwTimeOut,
                                 LPCSTR lpFuncData, BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    Q_UNUSED(nRespDataLen)

    INT nRet = IMP_SUCCESS;
    CHAR szReplyData[65536];        // 接收数据缓存
    DWORD dwReplySize = 0;          // 接收数据长度
    INT nRetDataSize = 0;           // 返回的数据长度    

    while (TRUE)
    {
        MSET_0(szReplyData);
        dwReplySize = sizeof(szReplyData);
        nRet = (INT)m_pDev->Read(szReplyData, dwReplySize, dwTimeOut);
        if (nRet < ERR_DEVPORT_SUCCESS)
        {
            if (bIsPrtLog == TRUE)
            {
                Log(ThisModule, __LINE__,
                    "%s : 数据接收: ->COM::Read() Fail, ErrCode = %d, Return: %s",
                    lpFuncData, nRet, ConvertCode_Impl2Str(nRet));
            }
            return nRet;
        } else  // 数据接收成功
        {
            nRetDataSize = (dwReplySize > nRespDataLen ? nRespDataLen - 1 : dwReplySize);
            MCPY_LEN(lpRespData, szReplyData, nRetDataSize);
            return nRetDataSize; // 结束循环返回
        }
    }
}

/***************************************************************************
 * 功能: 取校验和
 * 参数:
 * 返回值: 校验和
***************************************************************************/
DWORD CDevImpl_NT0861::GetCheckSum(LPCSTR lpcData, INT nDataLen, DWORD dwChkInit/* = CHECKVAL*/)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    for (INT i = 0; i < nDataLen; i ++)
    {
        dwChkInit -= lpcData[i];
    }

    return dwChkInit;
}

/***************************************************************************
 * 功能: 命令收发及检查
 * 参数:
 * 返回值: IMPL错误码
***************************************************************************/
INT CDevImpl_NT0861::SndRcvToChk(INT nCmdListNo, LPSTR lpRcvData, INT &nRcvSize,
                                 BOOL bIsCheck/* = TRUE*/, LPCSTR lpPrtData/* = nullptr*/,
                                 BOOL bIsPrtLog/* = TRUE*/)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    LOGDEVACTION();

    INT     nRet = IMP_SUCCESS;
    CHAR    szSndCmd[124] = { 0x00 };
    INT     nSndCmdSize = 0;

    MCPY_LEN(szSndCmd, stSndCmdList[nCmdListNo].byCMD, stSndCmdList[nCmdListNo].nCmdLen);
    nSndCmdSize = stSndCmdList[nCmdListNo].nCmdLen;

    // 命令下发
    nRet = SendCmd((LPCSTR)szSndCmd, nSndCmdSize, m_dwSndTimeOut, lpPrtData, bIsPrtLog);
    if (nRet != IMP_SUCCESS)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "%s: 命令<%s>下发: ->SendCmd(%s, %d, %d, %s)失败, ErrCode: %d, Return %s.",
                lpPrtData, CmdToStr(nCmdListNo), lpPrtData, CmdToStr(nCmdListNo),
                m_dwSndTimeOut, lpPrtData, nRet, ConvertCode_Impl2Str(nRet));
        }
        return nRet;
    }

    // 接收应答
    nRet = GetResponse(lpRcvData, nRcvSize, m_dwRcvTimeOut, lpPrtData, bIsPrtLog);
    if (nRet < 0)
    {
        if (bIsPrtLog == TRUE)
        {
            Log(ThisModule, __LINE__,
                "%s: 命令<%s>应答: ->GetResponse(%s, %d, %d, %d, %s)失败, ErrCode: %d, Return %s.",
                lpPrtData, CmdToStr(nCmdListNo), lpRcvData, nRcvSize,
                stSndCmdList[nCmdListNo].wRespCode, m_dwRcvTimeOut, lpPrtData,
                nRet, ConvertCode_Impl2Str(nRet));
        }
        return nRet;
    }
    nRcvSize = nRet;

    // 应答数据Check
    if (bIsCheck == TRUE)
    {
        INT nRet2 =  RcvDataCheck(nCmdListNo, lpRcvData, nRcvSize, lpPrtData, bIsPrtLog);
        if (nRet2 != IMP_SUCCESS)
        {
            return nRet2;
        }
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 应答数据Check
 * 参数:
 * 返回值: IMPL错误码
***************************************************************************/
INT CDevImpl_NT0861::RcvDataCheck(INT nCmdListNo, LPSTR lpRcvData, INT &nRcvDataLen,
                                  LPCSTR lpPrtData, BOOL bIsPrtLog)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 检查应答标记
    if (lpRcvData[nRcvDataLen - 1] == NAK)    // 命令参数无效
    {
        return IMP_ERR_PARAM_INVALID;
    } else
    if (lpRcvData[nRcvDataLen - 1] == ENQ)    // 命令无效
    {
        return IMP_ERR_SNDCMD_INVALID;
    } else
    if (lpRcvData[nRcvDataLen - 1] == ACK)    // 命令已处理
    {
        lpRcvData[nRcvDataLen - 1] = 0x00;
        nRcvDataLen --;
        return IMP_SUCCESS;
    }

    return IMP_SUCCESS;
}

//----------------------------------对外参数设置接口----------------------------------
// 设置断线重连标记
INT CDevImpl_NT0861::SetReConFlag(BOOL bFlag)
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
INT CDevImpl_NT0861::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径

    return IMP_SUCCESS;
}

// 设置命令收发超时时间
INT CDevImpl_NT0861::SetSndRcvTimeOut(DWORD dwSnd, DWORD dwRcv)
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
CHAR* CDevImpl_NT0861::CmdToStr(INT nCmdListNo)
{
    memset(m_szCmdStr, 0x00, sizeof(m_szCmdStr));    
    sprintf(m_szCmdStr, "<SYN>%c<CR>%s|%s",
            stSndCmdList[nCmdListNo].byCMD[CMD_POS_MODE],
            stSndCmdList[nCmdListNo].byCMD + CMD_POS_PARAM,
            stSndCmdList[nCmdListNo].szCmdStr
            );
    return m_szErrStr;
}

// USB处理错误值转换为Impl返回码/错误码
INT CDevImpl_NT0861::ConvertCode_USB2Impl(long lRet)
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

// Impl错误码转换解释字符串
LPSTR CDevImpl_NT0861::ConvertCode_Impl2Str(INT nErrCode)
{
#define NT0861_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nErrCode)
    {
        // > 100: Impl处理返回
        NT0861_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功")
        NT0861_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载失败")
        NT0861_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效")
        NT0861_CASE_CODE_STR(IMP_ERR_READERROR, nErrCode, "读数据错误")
        NT0861_CASE_CODE_STR(IMP_ERR_WRITEERROR, nErrCode, "写数据错误")
        NT0861_CASE_CODE_STR(IMP_ERR_RCVDATA_INVALID, nErrCode, "无效的应答数据")
        NT0861_CASE_CODE_STR(IMP_ERR_RCVDATA_NOTCOMP, nErrCode, "返回数据不完整")
        NT0861_CASE_CODE_STR(IMP_ERR_SNDCMD_INVALID, nErrCode, "下发命令无效")
        NT0861_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误")
        // <0 : USB/COM接口处理返回
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_NOTOPEN, nErrCode, "USB/COM: 没打开")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_FAIL, nErrCode, "USB/COM: 通讯错误")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_PARAM, nErrCode, "USB/COM: 参数错误")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_CANCELED, nErrCode, "USB/COM: 操作取消")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_READERR, nErrCode, "USB/COM: 读取错误")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_WRITE, nErrCode, "USB/COM: 发送错误")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_RTIMEOUT, nErrCode, "USB/COM: 读操作超时")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_WTIMEOUT, nErrCode, "USB/COM: 写操作超时")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_LIBRARY, nErrCode, "USB/COM: 加载通讯库失败")
        NT0861_CASE_CODE_STR(IMP_ERR_DEVPORT_NODEFINED, nErrCode, "USB/COM: 未知错误")
        // 0~100: 硬件设备返回
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}

// 应答条码类型转换为Impl格式标准
INT CDevImpl_NT0861::ConvertSymMode(LPSTR lpSymData)
{
#define SYM_NT0861_TO_IMPL(D, NT, IMP) \
    if (memcmp(D, NT, 1) == 0) \
    { \
        return IMP;\
    } else

    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_Codabar, EN_CODE_Codabar)        // Codabar
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_Code128, EN_CODE_Code128)        // Code 128
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_Code3932, EN_CODE_Code3932)      // Code 39/Code3 2
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_Code93, EN_CODE_Code93)          // Code 93
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_DataMatrix, EN_CODE_DataMatrix)  // Data Matrix
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_InterL2OF5, EN_CODE_InterL2OF5)  // InterLeaved 2 of 5
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_PDF417, EN_CODE_PDF417)          // PDF417
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_QR, EN_CODE_QR)                  // QR
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_UPCA, EN_CODE_UPCA)              // UPC-A
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_UPCE, EN_CODE_UPCE)              // UPC-E
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_UPCE1, EN_CODE_UPCE1)            // UPC-E1
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_UPCE8, EN_CODE_UPCE8)            // UPC-E8
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_UPCE13, EN_CODE_UPCE13)          // UPC-E13
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_Matrix2OF5, EN_CODE_Matrix2OF5)  // Matrix 2 of 5
    SYM_NT0861_TO_IMPL(lpSymData, SYM_FLAG_Indust2OF5, EN_CODE_Indust2OF5)  // Industrial 2 of 5
    return EN_CODE_UNKNOWN;
}

// -------------------------------- END -----------------------------------

