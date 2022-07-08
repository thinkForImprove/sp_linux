/***************************************************************
* 文件名称: DevImpl_DMTM1M.cpp
* 文件描述: 身份证模块底层指令, 提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年3月25日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevImpl_DMTM1M.h"
#include "data_convertor.h"
#include "libwlt.h"

static const char *ThisFile = "DevImpl_DMTM1M.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[4] != IMP_ERR_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertCode_IMPL2Str(IMP_ERR_NOTOPEN)); \
        } \
        m_nRetErrOLD[4] = IMP_ERR_NOTOPEN; \
        return IMP_ERR_NOTOPEN; \
    }

CDevImpl_DMTM1M::CDevImpl_DMTM1M()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_DMTM1M::CDevImpl_DMTM1M(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_DMTM1M::Init()
{
    m_bDevOpenOk = FALSE;                           // 设备Open标记,初始化:F

    // 设置SDK缺省路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("IDX/DMT-M1-M/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_wLoadDllVer = 0;
    m_bLoadIntfFail = TRUE;

    vInitLibFunc();
}

CDevImpl_DMTM1M::~CDevImpl_DMTM1M()
{
    vUnLoadLibrary();
}


BOOL CDevImpl_DMTM1M::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.load() != true)
    {
        if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> fail, ReturnCode: %s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
        }
        return FALSE;
    } else
    {
        Log(ThisModule, __LINE__, "加载动态库<%s> succ. ", m_szLoadDllPath);
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail, ReturnCode: %s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_DMTM1M::vUnLoadLibrary()
{
    THISMODULE(__FUNCTION__);

    if (m_LoadLibrary.isLoaded())
    {
        if (m_LoadLibrary.unload() != TRUE)
        {
            Log(ThisModule, __LINE__, "卸载动态库<%s> fail, ReturnCode: %s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().data());
        }
        m_bLoadIntfFail = TRUE;
        vInitLibFunc();
    }
}

BOOL CDevImpl_DMTM1M::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1. 连接读卡器
    LOAD_LIBINFO_FUNC(pDMT_connectReader, DMT_connectReader, "connectReader");

    // 2. 断开读卡器连接
    LOAD_LIBINFO_FUNC(pDMT_disconnect, DMT_disconnect, "disconnect");

    // 3. 获取卡片ATR(找卡)
    LOAD_LIBINFO_FUNC(pDMT_getAtr, DMT_getAtr, "getAtr");

    // 4. 获取读卡器固件版本
    LOAD_LIBINFO_FUNC(pDMT_getVersion, DMT_getVersion, "getVersion");

    // 5. APDU指令传输(实现读卡器和芯片APDU指令传输)
    LOAD_LIBINFO_FUNC(pDMT_transceive, DMT_transceive, "transceive");

    // 6.
    LOAD_LIBINFO_FUNC(pDMT_firmwareUpdate, DMT_firmwareUpdate, "firmwareUpdate");

    // 7. 获取SAMID
    LOAD_LIBINFO_FUNC(pDMT_getSAMID, DMT_getSAMID, "getSAMID");

    // 8. 读取身份证数据
    LOAD_LIBINFO_FUNC(pDMT_readIDCardMsg, DMT_readIDCardMsg, "readIDCardMsg");

    // 9.
    LOAD_LIBINFO_FUNC(pDMT_buzzer, DMT_buzzer, "buzzer");

    // 10.
    LOAD_LIBINFO_FUNC(pDMT_getChipSN, DMT_getChipSN, "getChipSN");

    // 11.
    LOAD_LIBINFO_FUNC(pDMT_getBoardSN, DMT_getBoardSN, "getBoardSN");

    // 12.
    LOAD_LIBINFO_FUNC(pDMT_mutualAuth, DMT_mutualAuth, "mutualAuth");

    // 13.
    LOAD_LIBINFO_FUNC(pDMT_samCommand, DMT_samCommand, "samCommand");

    // 14.
    LOAD_LIBINFO_FUNC(pDMT_samCardCommand, DMT_samCardCommand, "samCardCommand");

    // 15.
    LOAD_LIBINFO_FUNC(pDMT_reset, DMT_reset, "reset");

    return TRUE;
}

void CDevImpl_DMTM1M::vInitLibFunc()
{
    DMT_connectReader = nullptr;        // 1. 连接读卡器
    DMT_disconnect = nullptr;           // 2. 断开读卡器连接
    DMT_getAtr = nullptr;               // 3. 获取卡片ATR(找卡)
    DMT_getVersion = nullptr;           // 4. 获取读卡器固件版本
    DMT_transceive = nullptr;           // 5. APDU指令传输(实现读卡器和芯片APDU指令传输)
    DMT_firmwareUpdate = nullptr;       // 6.
    DMT_getSAMID = nullptr;             // 7. 获取SAMID
    DMT_readIDCardMsg = nullptr;        // 8. 读取身份证数据
    DMT_buzzer = nullptr;               // 9.
    DMT_getChipSN = nullptr;            // 10.
    DMT_getBoardSN = nullptr;           // 11.
    DMT_mutualAuth = nullptr;           // 12.
    DMT_samCommand = nullptr;           // 13.
    DMT_samCardCommand = nullptr;       // 14.
    DMT_reset = nullptr;                // 15.
}

//------------------------------------------------------------------------------------------------------
INT CDevImpl_DMTM1M::OpenDevice()
{
    return OpenDevice(0);
}

INT CDevImpl_DMTM1M::OpenDevice(WORD wType)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    // 如果已打开，则关闭
    if (m_bDevOpenOk == TRUE)
    {
        CloseDevice();
    }

    m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                    ConvertCode_IMPL2Str(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    // 打开设备
    nRet = DMT_connectReader((CHAR*)"usb");
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[1])
        {
            Log(ThisModule, __LINE__,
                "打开设备: ->DMT_connectReader(%s) fail, ErrCode: %d, Return: %s.",
                "usb", nRet, ConvertCode_IMPL2Str(nRet));
            m_nRetErrOLD[1] = nRet;
        }
        return nRet;
    }

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }
    m_bReCon = FALSE; // 是否断线重连状态: 初始F

    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

INT CDevImpl_DMTM1M::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 关闭设备
    if (m_bDevOpenOk == TRUE)
    {
        DMT_disconnect();
    }

    if (m_bReCon == FALSE)  // 非断线重连时关闭需释放动态库
    {
        // 释放动态库
        vUnLoadLibrary();
    }

    // 设备Open标记=F
    m_bDevOpenOk = FALSE;

    if (m_bReCon == FALSE)
    {
        Log(ThisModule, __LINE__, "设备关闭,释放动态库: Close Device(), unLoad Library.");
    }

    return IMP_SUCCESS;
}

BOOL CDevImpl_DMTM1M::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 1. 设备复位
INT CDevImpl_DMTM1M::nResetDevice(INT nMode)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    nRet = DMT_reset();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设备复位: ->DMT_reset() fail, ErrCode: %d, Return: %s.",
             nRet, ConvertCode_IMPL2Str(nRet));
    }

    return nRet;
}

// 2. 取设备状态
INT CDevImpl_DMTM1M::nGetDevStat()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    UCHAR szSamID[16] = { 0x00 };

    nRet = DMT_getSAMID(szSamID);
    if (m_nRetErrOLD[2] != nRet)
    {
        Log(ThisModule, __LINE__,
            "查设备(卡)状态: ->DMT_getSAMID() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        m_nRetErrOLD[2] = nRet;
    }

    return nRet;
}

// 3. 取卡状态
INT CDevImpl_DMTM1M::nGetCardStat()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    UCHAR szAtr[64] = { 0x00 };
    INT nSize = sizeof(szAtr);

    nRet = DMT_getAtr(szAtr, &nSize);
    if (m_nRetErrOLD[3] != nRet)
    {
        Log(ThisModule, __LINE__,
            "查设备(卡)状态: ->DMT_getAtr() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        m_nRetErrOLD[3] = nRet;
    }

    return nRet;
}

// 4. 退卡
// nMode: 0退卡到出口; 1完全弹出
INT CDevImpl_DMTM1M::nEjectIdCard(INT nMode)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    // 硬件接口调用: nRet =
    /*if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "退卡: ->StartScanIdCard(%d) fail, ErrCode: %d, Return: %s.",
             nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }*/

    return IMP_SUCCESS;
}

// 5. 取ATR
INT CDevImpl_DMTM1M::nGetAtr(LPSTR lpAtr, INT& nSize)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = DMT_getAtr((UCHAR*)lpAtr, &nSize);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "取ATR: ->DMT_getAtr() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 6. 取证件类型
// 1:身份证, 2:外国人, 3:港澳台
INT CDevImpl_DMTM1M::nGetIDCardType(INT &nType)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    UCHAR ucIDCardInfo[256] = { 0x00 };
    UCHAR ucPhoto[1024] = { 0x00 };
    UCHAR ucHeadImg[1024] = { 0x00 };
    INT nIDCardSize = sizeof(ucIDCardInfo);
    INT nPhotoSize = sizeof(ucPhoto);
    INT nHeadImgSize = sizeof(ucHeadImg);

    nRet = DMT_readIDCardMsg(ucIDCardInfo, &nIDCardSize, ucPhoto, &nPhotoSize, ucHeadImg, &nHeadImgSize);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "取证件类型: 取证件数据: ->DMT_readIDCardMsg() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    nType = m_clDmt.dmt_sm_getcardtype((LPSTR)ucIDCardInfo);
    if (nType < 0)
    {
        Log(ThisModule, __LINE__,
            "取证件类型: 分析证件类型: ->dmt_sm_getcardtype(%s) fail, ErrCode: %d, Return: %s.",
            ucIDCardInfo, nType, ConvertCode_IMPL2Str(IMP_ERR_OTHER));
        return IMP_ERR_OTHER;
    }

    return IMP_SUCCESS;
}

// 7. 取身份数据
INT CDevImpl_DMTM1M::nGetIDCardInfo(INT nIDType, LPVOID lpVoid, LPCSTR lpcSaveImage, BOOL headImgNameIncluded, BOOL debugMode)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    UCHAR ucIDCardInfo[256] = { 0x00 };
    UCHAR ucPhoto[1024] = { 0x00 };
    UCHAR ucFigImg[1024] = { 0x00 };
    INT nIDCardSize = sizeof(ucIDCardInfo);
    INT nPhotoSize = sizeof(ucPhoto);
    INT nFigImgSize = sizeof(ucFigImg);

    nRet = DMT_readIDCardMsg(ucIDCardInfo, &nIDCardSize, ucPhoto, &nPhotoSize, ucFigImg, &nFigImgSize);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "取证件类型: 取证件数据: ->DMT_readIDCardMsg() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    if (nIDType == 1)   // 国内二代身份证
    {
        LPSTIDCARDINFO lpInfo = (LPSTIDCARDINFO)lpVoid;

        // 取基本信息
        m_clDmt.basemsg_name((LPSTR)ucIDCardInfo, lpInfo->szName);              // 姓名
        m_clDmt.basemsg_gender((LPSTR)ucIDCardInfo, lpInfo->szSex);             // 性别
        m_clDmt.basemsg_nation((LPSTR)ucIDCardInfo, lpInfo->szNation);          // 民族
        m_clDmt.basemsg_birth((LPSTR)ucIDCardInfo, lpInfo->szBirthday);         // 出生日期
        m_clDmt.basemsg_address((LPSTR)ucIDCardInfo, lpInfo->szAddress);        // 住址信息
        m_clDmt.basemsg_idnum((LPSTR)ucIDCardInfo, lpInfo->szNumber);           // 身份证号码
        m_clDmt.basemsg_issue((LPSTR)ucIDCardInfo, lpInfo->szDepartment);       // 签发机关
        m_clDmt.basemsg_startdate((LPSTR)ucIDCardInfo, lpInfo->szSTimeLimit);   // 起始有效日期
        m_clDmt.basemsg_enddate((LPSTR)ucIDCardInfo, lpInfo->szETimeLimit);     // 结束有效日期

        // 头像数据处理
        if (nPhotoSize > 0)
        {
            CHAR szSrcPhoto[102*126*3] = { 0x00 };
            UCHAR szDetPhoto[104*126*3] = { 0x00 };
            CHAR headImgFullName[sizeof(lpInfo->szImage)] = { 0 };

            INT nRet1 = unpack((LPSTR)ucPhoto, szSrcPhoto,106);  //106
            if (nRet1 == 1)
            {
                if (!headImgNameIncluded)
                    sprintf(headImgFullName, "%s/%s.bmp", lpcSaveImage, lpInfo->szNumber);
                else
                    strcpy(headImgFullName, lpcSaveImage);

                if (debugMode && (0 == access(headImgFullName, F_OK)))
                {
                    Log(ThisModule, __LINE__, "skip save img on debug mode: %s", headImgFullName);
                } else
                {
                    m_clDmt.restoreBmp((UCHAR*)szSrcPhoto, szDetPhoto);
                    m_clDmt.SavBMP(headImgFullName, szDetPhoto, sizeof(szDetPhoto));Log(ThisModule, __LINE__, "save img ok: %s", headImgFullName);
                    Log(ThisModule, __LINE__, "save img ok: %s", headImgFullName);
                }

                memcpy(lpInfo->szImage, headImgFullName, strlen(headImgFullName));
            } else
            {
                Log(ThisModule, __LINE__,
                    "取证件类型: 头像数据处理: 解码: ->libwlt->unpack() fail, ErrCode: %d, Return: %s.",
                    nRet1, ConvertCode_IMPL2Str(IMP_ERR_OTHER));
                return IMP_ERR_OTHER;
            }
        }

        // 指纹信息处理
        if (nFigImgSize > 0)
        {
            for (INT i = 0; i < nFigImgSize / 2; i ++)
            {
                sprintf(lpInfo->szFingerData1 + strlen(lpInfo->szFingerData1),
                        "%02x", (UCHAR)ucFigImg[i]);
                lpInfo->nFinger1DataLen += 2;
            }

            for (INT i = nFigImgSize / 2; i < nFigImgSize; i ++)
            {
                sprintf(lpInfo->szFingerData2 + strlen(lpInfo->szFingerData2),
                        "%02x", (UCHAR)ucFigImg[i]);
                lpInfo->nFinger2DataLen += 2;
            }
        }
    } else
    if (nIDType == 2)   // 国外身份证
    {
        LPSTIDFOREIGNINFO lpInfo = (LPSTIDFOREIGNINFO)lpVoid;

        // 取基本信息
        m_clDmt.code_convert("ucs-2le", "utf-8", (LPSTR)ucIDCardInfo, 120, lpInfo->szNameENG, 120);     // 英文姓名
        m_clDmt.code_convert("ucs-2le", "utf-8", (LPSTR)ucIDCardInfo + 120, 2, lpInfo->szSex, 2);       // 性别
        m_clDmt.basemsg_Permit_Number((LPSTR)ucIDCardInfo, lpInfo->szIDCardNO);                         // 证件号码
        m_clDmt.basemsg_Code((LPSTR)ucIDCardInfo, lpInfo->szNation);                                    // 国籍
        m_clDmt.code_convert("ucs-2le","utf-8",(LPSTR)ucIDCardInfo + 158, 30, lpInfo->szNameCHN, 120);  // 中文姓名
        m_clDmt.basemsg_startdate((LPSTR)ucIDCardInfo, lpInfo->szSTimeLimit);                           // 证件签发日期开始
        m_clDmt.basemsg_enddate((LPSTR)ucIDCardInfo, lpInfo->szETimeLimit);                             // 证件签发日期结束
        m_clDmt.basemsg_Version((LPSTR)ucIDCardInfo, lpInfo->szIDVersion);                              // 证件版本
        m_clDmt.basemsg_IssueID((LPSTR)ucIDCardInfo, lpInfo->szDepartment);                             // 签发机关
    } else
    if (nIDType == 3)   // 港澳台通信证
    {
        LPSTIDGATINFO lpInfo = (LPSTIDGATINFO)lpVoid;

        // 取基本信息
        m_clDmt.basemsg_name((LPSTR)ucIDCardInfo, lpInfo->szName);              // 姓名
        m_clDmt.basemsg_gender((LPSTR)ucIDCardInfo, lpInfo->szSex);             // 性别
        m_clDmt.basemsg_birth((LPSTR)ucIDCardInfo, lpInfo->szBirthday);         // 出生日期
        m_clDmt.basemsg_address((LPSTR)ucIDCardInfo, lpInfo->szAddress);        // 住址信息
        m_clDmt.basemsg_idnum((LPSTR)ucIDCardInfo, lpInfo->szNumber);           // 身份证号码
        m_clDmt.basemsg_issue((LPSTR)ucIDCardInfo, lpInfo->szDepartment);       // 签发机关
        m_clDmt.basemsg_startdate((LPSTR)ucIDCardInfo, lpInfo->szSTimeLimit);   // 起始有效日期
        m_clDmt.basemsg_enddate((LPSTR)ucIDCardInfo, lpInfo->szETimeLimit);     // 结束有效日期
        m_clDmt.basemsg_permit((LPSTR)ucIDCardInfo, lpInfo->szPassport);        // 通行证号码
        m_clDmt.basemsg_times((LPSTR)ucIDCardInfo, lpInfo->szIssue);            // 签发次数
    }

    return IMP_SUCCESS;
}


// 8. 取固件版本
INT CDevImpl_DMTM1M::nGetFWVersion(LPSTR lpFwVer)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = DMT_getVersion(lpFwVer);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "取固件版本: ->DMT_getVersion() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}
// 设置动态库路径(DeviceOpen前有效)
void CDevImpl_DMTM1M::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, __LINE__, "入参[%s]无效,不设置动态库路径.", lpPath);
        return;
    }

    if (lpPath[0] == '/')   // 绝对路径
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s", lpPath);
    } else
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s%s", LINUXPATHLIB, lpPath);
    }

    Log(ThisModule, __LINE__, "设置动态库路径=<%s>.", m_szLoadDllPath);
}

// 设置动态库版本(DeviceOpen前有效)
void CDevImpl_DMTM1M::SetLibVer(WORD wVer)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
}

// 设置断线重连标记
INT CDevImpl_DMTM1M::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

LPSTR CDevImpl_DMTM1M::ConvertCode_IMPL2Str(INT nErrCode)
{
#define DMTM1M_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(nErrCode)
    {
        // Impl处理返回
        DMTM1M_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功");
        DMTM1M_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载失败");
        DMTM1M_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效");
        DMTM1M_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_NOTOPEN, nErrCode, "设备未Open");
        DMTM1M_CASE_CODE_STR(IMP_ERR_OTHER    , nErrCode, "其他错误");
        // Device返回
        //DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_0000H, nErrCode, "成功");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8101H, nErrCode, "设备未打开");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8102H, nErrCode, "传输错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8103H, nErrCode, "句柄错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8201H, nErrCode, "XOR错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8202H, nErrCode, "SUM错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8203H, nErrCode, "指令号错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8204H, nErrCode, "参数错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8205H, nErrCode, "包头错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8206H, nErrCode, "包长错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8D01H, nErrCode, "无卡");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8D02H, nErrCode, "apdu指令交互失败");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8E01H, nErrCode, "获取SAMID失败");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8E02H, nErrCode, "读取身份证信息失败");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8F01H, nErrCode, "XOR错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8F02H, nErrCode, "SUM错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8F03H, nErrCode, "指令号错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8F04H, nErrCode, "参数错误");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8F05H, nErrCode, "传输超时");
        DMTM1M_CASE_CODE_STR(IMP_ERR_DEV_8F06H, nErrCode, "扩展域参数错误");
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}


//---------------------------------------------
