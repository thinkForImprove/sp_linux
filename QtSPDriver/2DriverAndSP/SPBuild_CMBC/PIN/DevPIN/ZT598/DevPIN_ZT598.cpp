#include "DevPIN_ZT598.h"


static const char *ThisFile = "DevPIN_ZT598.cpp";
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static EPP_KEYVAL gszKeyVal[] = { \
    {0x30, "0"}, {0x31, "1"}, {0x32, "2"}, {0x33, "3"}, {0x34, "4"},
    {0x35, "5"}, {0x36, "6"}, {0x37, "7"}, {0x38, "8"}, {0x39, "9"},
    {0x1B, "CANCEL"}, {0x0D, "ENTER"}, {0x08, "CLEAR"},
    {0x22, "00"}, {0x2E, "."}, {0x2A, "*"},
    {0x41, "F1"}, {0x42, "F2"}, {0x43, "F3"}, {0x44, "F4"}, {0x45, "F5"},
    {0x46, "F6"}, {0x47, "F7"}, {0x48, "F8"}
};
static EPP_KEYVAL gszKeyVal_H6J[] = { \
    {0x30, "0"}, {0x31, "1"}, {0x32, "2"}, {0x33, "3"}, {0x34, "4"},
    {0x35, "5"}, {0x36, "6"}, {0x37, "7"}, {0x38, "8"}, {0x39, "9"},
    {0x1B, "返回"}, {0x0D, "确认"}, {0x08, "上翻"}, {0x25, "下翻"}, {0x23, "退格"}, {0x2A, "."},
    {0x41, "F1"}, {0x42, "F2"}, {0x43, "F3"}, {0x44, "F4"}, {0x45, "F5"},
    {0x46, "F6"}, {0x47, "F7"}, {0x48, "F8"}
};

static LPCSTR gIniUID = "00000000000000000000000000000000";
static LPCSTR gEppUID = "88888888888888888888888888888888";
//////////////////////////////////////////////////////////////////////////
//extern "C" DEVPIN_EXPORT long CreateIDevPIN(LPCSTR lpDevType, IDevPIN *&pDev)
//{
//    pDev = new CDevPIN_ZT598(lpDevType);
//    return (pDev != nullptr) ? 0 : -1;
//}
//////////////////////////////////////////////////////////////////////////
CDevPIN_ZT598::CDevPIN_ZT598(LPCSTR lpDevType)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    InitErrCode();
    m_bInitEpp = false;
    m_uKcvDecryptKeyNo = 0xFF;
}

CDevPIN_ZT598::~CDevPIN_ZT598()
{

}

void CDevPIN_ZT598::Release()
{

}

long CDevPIN_ZT598::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (m_pDev == nullptr)
    {
        if (0 != m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "PIN", "DevPIN_ZT598"))
        {
            UpdateStatus(DEVICE_OFFLINE, ERR_DEVPORT_LIBRARY);
            Log(ThisModule, __LINE__, "加载库(AllDevPort.dll) 失败");
            return ERR_DEVPORT_LIBRARY;
        }
    }

    CAllDevPortAutoLog _auto_log(m_pDev, ThisModule);
    long lRet = m_pDev->Open(lpMode);
    if (0 != lRet)
    {
        UpdateStatus(DEVICE_OFFLINE, ERR_DEVPORT_NOTOPEN);
        Log(ThisModule, __LINE__, "打开串口失败:%d[%s]", lRet, lpMode);
        return ERR_DEVPORT_NOTOPEN;
    }

    //读取配置文件
    ReadConfig();                       //30-00-00-00(FS#0010)

    //设置CLEAR按键功能
    SetClearKeyFunc();                  //30-00-00-00(FS#0010)

    //下载全0 SM4明文密钥
    char szKCV[64] = { 0 };
    memset(szKCV, 0, sizeof(szKCV));
    lRet = SM4ImportMKey(0, gIniUID, szKCV);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载国密全0铭文密钥失败");
        return lRet;
    }

    //下载全0 DES密钥
    m_bInitEpp = true;
    lRet = ImportMKey(0, 0, gIniUID, szKCV);
    if (lRet != ERR_SUCCESS)
    {
        //忽略EC错误
        if(lRet != -0xEC){
            Log(ThisModule, __LINE__, "下载DES全0密钥失败");
            return lRet;
        }
    }
    m_bInitEpp = false;


    return ERR_DEVPORT_SUCCESS;
}

long CDevPIN_ZT598::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if (m_pDev->IsOpened())
        m_pDev->Close();
    return ERR_DEVPORT_SUCCESS;
}

long CDevPIN_ZT598::InitEpp()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 删除密钥
    long lRet = DeleteKey();
    if (lRet != ERR_SUCCESS)
        return lRet;

    // 删除国密密钥
    //  lRet = SMDeleteKey();
    //  if (lRet != ERR_SUCCESS)
    //  {
    //      Log(ThisModule, __LINE__, "删除国密密钥失败");
    //      return lRet;
    //  }    

    // 下载UID的异或Key，全为0
    char szKCV[64] = { 0 };
    m_bInitEpp = true;
    lRet = ImportMKey(0, 0, gIniUID, szKCV);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载UID为Key失败");
        return lRet;
    }
    m_bInitEpp = false;    

    // 初始化UID
    lRet = AuthEpp(gEppUID, 0, 0);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "初始化UID失败");
        return lRet;
    }

    // 验证UID
    QByteArray vtResult;
    QByteArray vtUidData;
    lRet = AuthRandData(gEppUID, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取随机数失败：%d", lRet);
        return lRet;
    }

    lRet = AuthEpp(vtResult.constData(), 1, 0, &vtUidData);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "认证密钥失败：%d[uKeyNo=%d]", lRet, 0);
        return lRet;
    }

    // 加密UID返回数据
    char sz3DesAuth[256] = { 0 };
    m_cDES.gDes(vtUidData.data(), (char *)gEppUID, sz3DesAuth);
    lRet = AuthEpp(sz3DesAuth, 2, 0);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "验证UID失败");
        return lRet;
    }    

    // 设置密文输入超时时间
    lRet = SetControlMode(2, 0xFF);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置密文输入超时时间失败");
        return lRet;
    }

    // 设置按键超时时间
    lRet = SetControlMode(0x0A, 0x13);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置按键超时时间失败");
        return lRet;
    }

    return 0;
}

long CDevPIN_ZT598::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x35\x30\x33\x30");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "复位命令失败：%d", lRet);
        return lRet;
    }

    UpdateStatus(DEVICE_ONLINE, 0);
    Log(ThisModule, 1, "复位成功");
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::GetStatus(DEVPINSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);
//    LOGDEVACTION();

    CAutoCopyDevStatus<DEVPINSTATUS> _auto(&stStatus, &m_stStatus);
    QByteArray vtResult;
    //QByteArray vtCmd("\x4E\x31");
    QByteArray vtCmd("\x49");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
//        if (lRet > ERR_HW_A980_D0)// 小于此值的，在UnPack中已更新
//            UpdateStatus(DEVICE_HWERROR, lRet);
        if(lRet == ERR_DEVPORT_WRITE || lRet == ERR_DEVPORT_WTIMEOUT ||
           lRet == ERR_DEVPORT_READERR || lRet == ERR_DEVPORT_RTIMEOUT){
            UpdateStatus(DEVICE_OFFLINE, lRet);
        } else {
            UpdateStatus(DEVICE_HWERROR, lRet);
        }

        Log(ThisModule, __LINE__, "读取查询状态命令失败：%d", lRet);
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_ZT598::InstallStatus(bool bClear)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    if (bClear)
    {
        QByteArray vtCmd("\x53");
        long lRet = SendReadCmd(vtCmd, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "清除安装标志：%d", lRet);
            return lRet;
        }
        Log(ThisModule, 1, "清除安装标志成功");
    }
    else
    {
        QByteArray vtCmd("\x4E\x30");
        long lRet = SendReadCmd(vtCmd, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "读取查询状态命令失败：%d", lRet);
            return lRet;
        }

        Log(ThisModule, 0, "查询安装状态成功：%s", vtResult.constData());
        if (vtResult[2] == '1')
        {
            Log(ThisModule, __LINE__, "EPP未安装");
            return 1;
        }

        Log(ThisModule, 1, "EPP已安装");
    }
    return 0;
}

long CDevPIN_ZT598::GetFirmware(LPSTR lpFWVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x33");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取固件信息命令失败：%d", lRet);
        return lRet;
    }

    // 分析返回数据
    char szEppID[32] = { 0 };
    char szFunctionCode[32] = { 0 };
    char szModelNumber[32] = { 0 };
    char szHardwareVer[32] = { 0 };
    char szFirmwareVer[32] = { 0 };
    char szHardwardSN[32] = { 0 };
    memcpy(szFunctionCode, vtResult.constData() + 118, 8);
    memcpy(szModelNumber, vtResult.constData() + 126, 6);
    memcpy(szHardwareVer, vtResult.constData() + 132, 3);
    memcpy(szFirmwareVer, vtResult.constData() + 135, 3);
    memcpy(szHardwardSN, vtResult.constData() + 138, 8);
    // 清空格
    for (int i = strlen(szModelNumber) - 1; i >= 0; i++)
    {
        if (szModelNumber[i] != 0x20)
            break;

        szModelNumber[i] = 0x00;
    }
    // 读取键盘ID
    vtResult.clear();
    vtCmd = "\x49";
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取键盘ID命令失败：%d", lRet);
    }
    else
    {
        memcpy(szEppID, vtResult.constData(), 8);
    }

    sprintf(lpFWVer, "[FW:%s-%s-%s-%s][SN:%s][ID:%s]", szModelNumber, szHardwareVer, szFirmwareVer, szFunctionCode, szHardwardSN, szEppID);
    m_strFirmware = lpFWVer;
    Log(ThisModule, 1, "EPP固件信息：%s", lpFWVer);
    return 0;
}

long CDevPIN_ZT598::SetKeyValue(LPCSTR lpKeyVal)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    UINT uLen = strlen(lpKeyVal);
    if (uLen != 48)
    {
        Log(ThisModule, __LINE__, "无效按键数据长度：Len=%d[%s]", uLen, lpKeyVal);
        return ERR_PARAM;
    }

    if (m_strAllKeyVal == lpKeyVal)
    {
        Log(ThisModule, 1, "新旧按键值一致，不用重新设置：%s", lpKeyVal);
        return 0;
    }

    QByteArray vtResult;
    QByteArray vtCmd("\x34");
    vtCmd.append(lpKeyVal);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置新按键值失败：%d[%s]", lRet, lpKeyVal);
        return lRet;
    }

    m_strAllKeyVal = lpKeyVal;
    Log(ThisModule, 1, "设置新按键值成功：%s", lpKeyVal);
    return 0;
}

long CDevPIN_ZT598::SetActiveKey(DWORD dwActiveFuncKey, DWORD dwActiveFDKKey)
{
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::ImportMKey(UINT uMKeyNo, UINT uKeyMode, LPCSTR lpKeyVal, LPSTR lpKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (!m_bInitEpp)// 在初始化键盘时，因还没有下载UID，所以不用认证
    {
        // 认证密钥
        long lRet = AuthKey(uMKeyNo);
        if (lRet != ERR_SUCCESS)
        {
            if (lRet != -0xEC && lRet != -0xE4)
            {
                Log(ThisModule, __LINE__, "认证密钥失败: lRet=%d", lRet);
                return lRet;
            }
            else
            {
                lRet = AuthUID();
                if (lRet != ERR_SUCCESS)
                {
                    Log(ThisModule, __LINE__, "认证UID密钥失败: lRet=%d", lRet);
                    return lRet;
                }
            }
        }
    }

    LOGDEVACTION();
    BYTE uKeyLen = 0;
    QByteArray vtResult;
    QByteArray vtCmd;

    UINT uValLen = strlen(lpKeyVal);
    switch (uValLen)
    {
    case 16: uKeyLen = 0x31;        break;
    case 32: uKeyLen = 0x32;        break;
    case 48: uKeyLen = 0x33;        break;
    default:                        break;
    }

    if (uKeyLen == 0 || uKeyMode > 4)
    {
        Log(ThisModule, __LINE__, "无效Key用途,长度或模式：uKeyLen=%d[%d], nMKeyMode=%d",
            uKeyLen, uValLen, uKeyMode);
        return ERR_PARAM;
    }

    if (!m_bInitEpp && uMKeyNo < 0x01 || uMKeyNo > 0x1F)
    {
        Log(ThisModule, __LINE__, "无效保存主密钥ID：uMKeyNo=%02X", uMKeyNo);
        return ERR_PARAM;
    }

    // 下载明文主密钥
    char szKeyNo[64] = { 0 };
    sprintf(szKeyNo, "%02X", uMKeyNo);

    vtCmd.append(0x31);
    vtCmd.append(szKeyNo);
    vtCmd.append(0x30 + uKeyMode);
    vtCmd.append(uKeyLen);
    vtCmd.append(lpKeyVal);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "主密钥导入失败：lRet=%d[uMKeyNo=%02X]", lRet, uMKeyNo);
        return lRet;
    }

    CAutoHex cHex((LPBYTE)vtResult.constData(), vtResult.size());
    strcpy(lpKCV, cHex.GetHex());
    Log(ThisModule, 1, "主密钥导入成功:uMKeyNo=%02X", uMKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode/* = 2*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 认证密钥
    long lRet;
    if(uMKeyNo == 0xFF){
        lRet = AuthKey(0);
    } else {
        lRet = AuthKey(uMKeyNo);
    }

    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "认证主密钥失败：uMKeyNo=%02X", uMKeyNo);
        return lRet;
    }

    LOGDEVACTION();
    BYTE byKeyLen = 0;
    BYTE byKeyUse = 0;
    QByteArray vtResult;
    QByteArray vtCmd;

    switch (uKeyUse)
    {
    case PKU_CRYPT:             byKeyUse = 0x36;        break;
    case PKU_FUNCTION:          byKeyUse = 0x33;        break;
    case PKU_MACING:            byKeyUse = 0x35;        break;
    case PKU_KEYENCKEY:         byKeyUse = 0x32;        break;
    case PKU_NODUPLICATE:       byKeyUse = 0x36;        break;
    case PKU_SVENCKEY:          byKeyUse = 0x36;        break;
    case PKU_CONSTRUCT:         byKeyUse = 0x36;        break;
    case PKU_SECURECONSTRUCT:   byKeyUse = 0x36;        break;
    default:                                            break;
    }

    UINT uValLen = strlen(lpKeyVal);
    switch (uValLen)
    {
    case 16: byKeyLen = 0x31;       break;
    case 32: byKeyLen = 0x32;       break;
    case 48: byKeyLen = 0x33;       break;
    default:                        break;
    }

    if (byKeyLen == 0 || byKeyUse == 0)
    {
        Log(ThisModule, __LINE__, "无效Key用途,长度或模式：byKeyUse=%d[%u], byKeyLen=%d[%d]",
            byKeyUse, uKeyUse, byKeyLen, uValLen);
        return ERR_PARAM;
    }

    if(uKeyUse == PKU_KEYENCKEY && (uKeyNo < 0x10 || uKeyNo > 0x2F))
    {
        Log(ThisModule, __LINE__, "无效保存主密钥ID：nKeyNo=%02X, nMKeyNo=%02X", uKeyNo, uMKeyNo);
        return ERR_PARAM;
    }
    else if(uKeyUse != PKU_KEYENCKEY && (uKeyNo < 0x30 ||  uKeyNo > 0x7F))
    {
        Log(ThisModule, __LINE__, "无效保存工作密钥ID：nKeyNo=%02X, nMKeyNo=%02X", uKeyNo, uMKeyNo);
        return ERR_PARAM;
    }

    //无加密Key时，先对数据进行加密处理
    char cKeyValueAsi[80] = {0};
    memcpy(cKeyValueAsi, lpKeyVal, strlen(lpKeyVal));
    if(uMKeyNo == 0xFF){
        byKeyLen = 0x33;                  //24bytes
        BYTE byKeyValBCD[24] = {0};
        UINT uLen = 0;
        //ASI to HEX
        CAutoHex::Hex2Bcd(lpKeyVal, byKeyValBCD, uLen);

        switch(uLen){
        case 8:
            memcpy(byKeyValBCD + 8, byKeyValBCD, 8);
            memcpy(byKeyValBCD + 16, byKeyValBCD, 8);
            break;
        case 16:
            memcpy(byKeyValBCD + 16, byKeyValBCD, 8);
            break;
        case 24:
            break;
        default:
            break;
        }
        BYTE byAllZeroKey[16] = {0};
        m_encrypt.DataEncrypt3DES(byKeyValBCD, byAllZeroKey, byKeyValBCD);
        m_encrypt.DataEncrypt3DES(byKeyValBCD + 8, byAllZeroKey, byKeyValBCD + 8);
        m_encrypt.DataEncrypt3DES(byKeyValBCD + 16, byAllZeroKey, byKeyValBCD + 16);

        //HEX to ASI
        memset(cKeyValueAsi, 0, sizeof(cKeyValueAsi));
        CAutoHex::Bcd2Hex(byKeyValBCD, sizeof(byKeyValBCD), cKeyValueAsi);
    }

    char szKeyNo[64] = { 0 };
    char szMKeyNo[64] = { 0 };
    sprintf(szKeyNo, "%02X", uKeyNo);
    if(uMKeyNo == 0xFF){
        strcpy(szMKeyNo, "00");
    } else {
        sprintf(szMKeyNo, "%02X", uMKeyNo);
    }


    vtCmd.append(0x32);
    vtCmd.append(szMKeyNo);
    vtCmd.append(szKeyNo);
    vtCmd.append(byKeyUse);
    vtCmd.append(byKeyLen);
    vtCmd.append(cKeyValueAsi);
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "工作密钥导入失败：lRet=%d[nKeyNo=%02X, nMKeyNo=%02X]", lRet, uKeyNo, uMKeyNo);
        return lRet;
    }

    //KCV数据解密
    if(lpKCV != nullptr){
        if(m_uKcvDecryptKeyNo == 0xFF){
            return ERR_PARAM;
        }

        BYTE byAllZero[8];
        memset(byAllZero, 0, sizeof(byAllZero));
        char szKcvDecryptKeyNo[64] = {0};
        sprintf(szKcvDecryptKeyNo, "%02x", m_uKcvDecryptKeyNo);

        vtCmd.clear();
        vtCmd.append(0x39);
        vtCmd.append(szKcvDecryptKeyNo);
        vtCmd.append(0x31);
        vtCmd.append("0000000000000000");
        lRet = SendReadCmd(vtCmd, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "KCV解密失败：lRet=%d[nKeyNo=%02X, nMKeyNo=%02X]", lRet, uKeyNo, uMKeyNo);
            return lRet;
        }
        strcpy(lpKCV, vtResult.constData());
        m_uKcvDecryptKeyNo = 0xFF;
    }

    Log(ThisModule, 1, "工作密钥导入成功:nKeyNo=%02X, nMKeyNo=%02X", uKeyNo, uMKeyNo);
    return ERR_SUCCESS;
}

void CDevPIN_ZT598::SetDesKcvDecryptKey(UINT uKeyNo)
{
    m_uKcvDecryptKeyNo = uKeyNo;
}


long CDevPIN_ZT598::DeleteKey(UINT uKeyNo /*= 0*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;
    if (uKeyNo == 0)
    {
        // Erasure all Key, and then reset. parameter will return to the
        // factory default value, including the user information area,
        // key value area, control code, delete 128 key
        vtCmd.append("\x35\x30\x33\x31");
    }
    else
    {

    }

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "删除密钥命令失败：%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    Log(ThisModule, 1, "删除密钥成功：uKeyNo=%d", uKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::TextInput()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_vtKeyVal.clear();
    long lRet = SetControlMode(0, 0x35);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打开按键声音命令失败：%d", lRet);
        return lRet;
    }

    // 使用这种方法，在设备日志是，有记录明文输入
    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd("\x35\x30\x33\x33");
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打开明文输入命令失败：%d", lRet);
        SetControlMode(0, 0x32);// 关闭
        return lRet;
    }

    // 测试时，关闭声音
    //    lRet = SetControlMode(0, 0x34);
    //    if (lRet != ERR_SUCCESS)
    //    {
    //        Log(ThisModule, __LINE__, "关闭按键声音失败：%d", lRet);
    //        return lRet;
    //    }

    Log(ThisModule, 1, "打开明文输入成功");
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::PinInput(UINT uMinLen/* = 4*/, UINT uMaxLen/* = 6*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    m_vtKeyVal.clear();
    if (uMinLen > uMaxLen || uMinLen > 12 || uMaxLen > 12)
    {
        Log(ThisModule, __LINE__, "参数错误：uMinLen=%d, uMaxLen=%d", uMinLen, uMaxLen);
        return ERR_PARAM;;
    }

    long lRet = 0;
    do
    {
//        lRet = SetControlMode(0, 0x34);
//        if (lRet != ERR_SUCCESS)
//        {
//            Log(ThisModule, __LINE__, "关闭按键声音失败：%d", lRet);
//            break;
//        }

//        lRet = SetControlMode(0, 0x32);
//        if (lRet != ERR_SUCCESS)
//        {
//            Log(ThisModule, __LINE__, "关闭键盘失败：%d", lRet);
//            break;
//        }

        lRet = SetPinInputLen(uMinLen, uMaxLen);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "设置密码长度失败：%d", lRet);
            break;
        }
          // 此步验证可以去掉 
//        lRet = AuthUID();
//        if (lRet != ERR_SUCCESS)
//        {
//            Log(ThisModule, __LINE__, "UID认证失败：%d", lRet);
//            break;
//        }

        // 打开按键声音（先打开按键声音，防止后打开时夹杂按键数据返回）
        lRet = SetControlMode(0, 0x35);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开按键声音命令失败：%d", lRet);
            return lRet;
        }

        QByteArray vtResult;
//30-00-00-00(FS#0010)        QByteArray vtCmd("\x36\x30\x30\x30\x31\x32");
        QByteArray vtCmd("\x36\x30\x30\x30\x31");       //30-00-00-00(FS#0010)
        if(m_bSupportBackspace){                        //30-00-00-00(FS#0010)
            vtCmd.push_back(0x31);                      //30-00-00-00(FS#0010)
        } else {                                        //30-00-00-00(FS#0010)
            vtCmd.push_back(0x32);                      //30-00-00-00(FS#0010)
        }                                               //30-00-00-00(FS#0010)
        lRet = SendReadCmd(vtCmd, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开密文输入命令失败：%d", lRet);
            break;
        }

        Log(ThisModule, 1, "打开密文输入成功");
        return 0;
    } while (false);

    SetControlMode(0, 0x32);// 关闭键盘
    return lRet;
}

long CDevPIN_ZT598::ReadKeyPress(EPP_KEYVAL &stKeyVal, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    memset(&stKeyVal, 0x00, sizeof(EPP_KEYVAL));
    if (!m_vtKeyVal.empty())
    {
        stKeyVal = m_vtKeyVal.front();
        m_vtKeyVal.pop_front();
        return 0;
    }

    LOGDEVACTION();
    BYTE szKeyBuff[64] = {0};
    DWORD dwWaitLen = sizeof(szKeyBuff) - 1;
    long lRet = m_pDev->Read((char *)szKeyBuff, dwWaitLen, dwTimeOut);
    if (lRet == ERR_SUCCESS)
    {
        for (UINT i = 0; i < dwWaitLen && i < sizeof(szKeyBuff); i++)
        {
            if (szKeyBuff[i] == 0x00)
                continue;

            if (ConvertKeyVal(szKeyBuff[i], stKeyVal))
            {
                m_vtKeyVal.push_back(stKeyVal);
            }
        }

        if (!m_vtKeyVal.empty())
        {
            stKeyVal = m_vtKeyVal.front();
            m_vtKeyVal.pop_front();
            return 0;
        }
    }
    else if (lRet == ERR_DEVPORT_RTIMEOUT)
    {
        m_pDev->CancelLog();
    }
    return lRet;
}

long CDevPIN_ZT598::CancelInput(bool bPinInput)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd("\x35\x30\x33\x32");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "取消[%s]输入命令失败：%d", bPinInput ? "密文" : "明文", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "取消[%s]输入成功", bPinInput ? "密文" : "明文");
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::GetPinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    UINT uFmType = 0;
    switch (uFormat)
    {
    case PIN_ASCII:         uFmType = 0x40;     break;
    case PIN_ISO9564_0:     uFmType = 0x20;     break;
    case PIN_ISO9564_1:     uFmType = 0x21;     break;
    case PIN_ISO9564_2:     uFmType = 0x22;     break;
    case PIN_ISO9564_3:     uFmType = 0x23;     break;
    case PIN_IBM3624:       uFmType = 0x30;     break;
    default:
        Log(ThisModule, __LINE__, "不支持模式：%02X", uFormat);
        return ERR_NOT_SUPPORT;
    }

    UINT uLen = strlen(lpCustomerData);
//    if (uLen != 12 && uFormat != PIN_ASCII && uFormat != PIN_IBM3624)
    if (uLen != 12 && uFormat != PIN_ISO9564_1)
    {
        Log(ThisModule, __LINE__, "无效长度用户数据：%d!=12", uLen);
        return ERR_PARAM;
    }

    // 设置填充
    long lRet = SetControlMode(3, bPadding);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置填充失败：%d", lRet);
        return lRet;
    }

    // 认证密钥
    lRet = AuthKey(uKeyNo);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "认证密钥失败：%d[nKeyNo=%02X]", lRet, uKeyNo);
        return lRet;
    }

    // 设置密钥和算法，并读取密码块
    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;
    vtCmd.resize(4);
    sprintf(vtCmd.data(), "\x37%02X%c", uKeyNo, uFmType);
    if (uFormat != PIN_ASCII && uFormat != PIN_IBM3624)
        vtCmd.append(lpCustomerData);
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        if(lRet == -0xEF){
            lRet = ERR_NO_PIN;
        }
        Log(ThisModule, __LINE__, "读取密码块失败：%d[nKeyNo=%02X,uFormat=%02X]", lRet, uKeyNo, uFmType);
        return lRet;
    }

    if (vtResult.length() != 20)
    {
        Log(ThisModule, __LINE__, "读取密码块数据异常：%s", vtResult.constData());
        return ERR_REDATA;
    }

    // 因前面还有四个其他值
    memcpy(lpPinBlock, vtResult.constData() + 4, 16);
    Log(ThisModule, 0, "读取密码块成功");
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::DataCrypt(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpCryptData == nullptr || lpIVData == nullptr || lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    UINT uType = 0;
    switch (uMode)
    {
    case ECB_EN:    uType = 0x31;   break;// ECB // Encryption
    case ECB_DE:    uType = 0x41;   break;// ECB // Decryption
    case CBC_EN:    uType = 0x32;   break;// CBC // Encryption
    case CBC_DE:    uType = 0x42;   break;// CBC // Decryption
    default:
        Log(ThisModule, __LINE__, "不支持Crypt模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    UINT uMaxLen = 1024;// 单次最大字符数
    CAutoHex cXor;
    QByteArray vtXorData;
    QByteArray vtResult;
    QByteArray vtResultTotal;
    QByteArray vtIVData(lpIVData);
    QByteArray vtCryptData(lpCryptData);
    Pading(vtCryptData, bPadding);// 填充
    do
    {
        if ((UINT)vtCryptData.size() <= uMaxLen)
        {
            vtXorData = vtCryptData;
            vtCryptData.clear();
        }
        else
        {
            vtXorData   = vtCryptData.mid(0, uMaxLen);
            vtCryptData = vtCryptData.mid(uMaxLen);
        }

        // 处理IV
        if (uMode == CBC_EN)
        {
            cXor.HexXOR(vtXorData, vtIVData);
        }

        long lRet = Crypt(uKeyNo, uType, vtXorData, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "加解密失败：%d[uMode=%d]", lRet, uMode);
            return lRet;
        }

        if (uMode == CBC_DE) // CBC 解密
        {
            cXor.HexXOR(vtResult, vtIVData);// 处理IV
            vtIVData = vtXorData.right(16);// 新IV
        }
        else if (uMode == CBC_EN)
        {
            vtIVData = vtResult.right(16);// 新IV
        }

        vtResultTotal += vtResult;
        if (vtCryptData.isEmpty())// 已全部计算
            break;
    } while (true);

    memcpy(lpData, vtResultTotal, vtResultTotal.size());
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::DataMAC(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpMacData == nullptr || lpIVData == nullptr || lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    UINT uMacType = 0;
    QByteArray vtLastPath;
    QByteArray vtMacData(lpMacData);
    switch (uMode)
    {
    case MAC_ANSIX99:
        uMacType = 0x32;
        break;
    case MAC_PBOC:
        vtMacData.append("80");
        uMacType = 0x32;
        break;
    case MAC_ANSIX919:
        uMacType = 0x32;
        break;
    case MAC_ISO16609:
    case MAC_CBC:
        uMacType = 0x41;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持MAC模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }
    Pading(vtMacData, 0x00);

    UINT uMaxLen = 1024;// 单次最大计算MAC字符数
    CAutoHex cXOR;
    QByteArray vtResult;
    QByteArray vtXorData;
    QByteArray vtIVData(lpIVData);
    do
    {
        if ((UINT)vtMacData.size() <= uMaxLen)
        {
            vtXorData = vtMacData;
            vtMacData.clear();
        }
        else
        {
            vtXorData = vtMacData.mid(0, uMaxLen);
            vtMacData = vtMacData.mid(uMaxLen);
        }

        if ((uMode == MAC_PBOC || uMode == MAC_ANSIX919))
        {
            if (vtXorData.size() > 16)
            {
                vtLastPath = vtXorData.right(16);
                vtXorData  = vtXorData.left(vtXorData.size() - 16);
            }
            else
            {
                uMacType = 0x41;// 只有16长度的，只计算最后一步就行了
            }
        }

        // 处理IV
        cXOR.HexXOR(vtXorData, vtIVData);
        long lRet = MAC(uKeyNo, uMacType, vtXorData, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "计算MAC失败：%d[uMode=%d，uMacType=%02X]", lRet, uMode, uMacType);
            return lRet;
        }

        if (uMode == MAC_PBOC || uMode == MAC_ANSIX919)
        {
            if (!vtLastPath.isEmpty())
            {
                // 最后一部分算MAC
                vtXorData = cXOR.HexXOR(vtResult.constData(), vtLastPath);
                long lRet = MAC(uKeyNo, 0x41, vtXorData, vtResult);
                if (lRet != ERR_SUCCESS)
                {
                    Log(ThisModule, __LINE__, "计算MAC失败：%d[uMode=%d, uMacType=%02X]", lRet, uMode, uMacType);
                    return lRet;
                }
            }
        }

        // 处理IV
        vtIVData = vtResult;
        if (vtMacData.isEmpty())// 已全部计算MAC
            break;
    } while (true);

    memcpy(lpData, vtResult.constData(), vtResult.size());
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::RandData(LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x3A\x41\x30");
    vtCmd.append(gEppUID);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取32位长度的随机数失败：%d", lRet);
        return lRet;
    }
    if (vtResult.size() != 32)
    {
        Log(ThisModule, __LINE__, "获取32位长度的随机数失败：%d", lRet);
        return ERR_REDATA;
    }

    memcpy(lpData, vtResult, vtResult.size());
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::ReadSymmKeyKCV(UINT uKeyNo, BYTE byKcvMode, LPSTR lpKcv)
{
    return ERR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
long CDevPIN_ZT598::SMGetFirmware(LPSTR lpSMVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x73");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取SM芯片固件信息命令失败：%d", lRet);
        return lRet;
    }

    // 分析返回数据
    char szInfo[256] = { 0 };
    for (UINT i = 0, k = 0; i < (UINT)vtResult.size();)
    {
        BYTE bH = vtResult[i++] & 0x0F;
        BYTE bL = vtResult[i++] & 0x0F;
        BYTE bCh = (bH << 4) | bL;
        if (bCh == 0x24)
            bCh = '-';
        szInfo[k++] = bCh;
    }
    strcpy(lpSMVer, szInfo);
    Log(ThisModule, 1, "EPP的SM固件信息：%s", szInfo);
    return ERR_SUCCESS;
}


long CDevPIN_ZT598::SM4ImportMKey(UINT uMKeyNo, LPCSTR lpKeyVal, LPSTR lpKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;
    UINT uValLen = strlen(lpKeyVal);
    if (uValLen == 0)
    {
        Log(ThisModule, __LINE__, "无效Key长度：uValLen=%d", uValLen);
        return ERR_PARAM;
    }

    if (/*uMKeyNo < 0x01 || */uMKeyNo > 0x1F)
    {
        Log(ThisModule, __LINE__, "无效保存主密钥ID：uMKeyNo=%02X", uMKeyNo);
        return ERR_PARAM;
    }

    // 下载明文主密钥
    char szKeyNo[64] = { 0 };
    sprintf(szKeyNo, "%02X", uMKeyNo);

    vtCmd.append(0x62);
    vtCmd.append(szKeyNo);
    vtCmd.append(lpKeyVal);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM主密钥导入失败：lRet=%d[uMKeyNo=%02X]", lRet, uMKeyNo);
        return lRet;
    }

    if(lpKCV != nullptr){
        memcpy(lpKCV, vtResult.constData(), vtResult.size());
    }
    Log(ThisModule, 1, "SM主密钥导入成功:uMKeyNo=%02X", uMKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::SM4ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode/* = 2*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    BYTE byKeyUse = 0;
    QByteArray vtResult;
    QByteArray vtCmd;

    switch (uKeyUse)
    {
    case PKU_CRYPT:             byKeyUse = 0x36;        break;
    case PKU_FUNCTION:          byKeyUse = 0x33;        break;
    case PKU_MACING:            byKeyUse = 0x35;        break;
    case PKU_KEYENCKEY:         byKeyUse = 0x32;        break;
    case PKU_NODUPLICATE:       byKeyUse = 0x36;        break;
    case PKU_SVENCKEY:          byKeyUse = 0x36;        break;
    case PKU_CONSTRUCT:         byKeyUse = 0x36;        break;
    case PKU_SECURECONSTRUCT:   byKeyUse = 0x36;        break;
    default:                                            break;
    }

    UINT uValLen = strlen(lpKeyVal);
    if (uValLen != 32 || byKeyUse == 0)
    {
        Log(ThisModule, __LINE__, "无效Key用途,长度：byKeyUse=%d[%u], uValLen=%d", byKeyUse, uKeyUse, uValLen);
        return ERR_PARAM;
    }

    if(uKeyUse == PKU_KEYENCKEY && (uKeyNo > 0x1F ))
    {
        Log(ThisModule, __LINE__, "无效保存主密钥ID：uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
        return ERR_PARAM;
    }
    else if(uKeyUse != PKU_KEYENCKEY && (uKeyNo < 0x20 || uKeyNo > 0x7F))
    {
        Log(ThisModule, __LINE__, "无效保存工作密钥ID：uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
        return ERR_PARAM;
    }

    //无加密Key时，先对数据进行加密处理
    BYTE byKeyValueAsi[80] = {0};
    memcpy(byKeyValueAsi, lpKeyVal, strlen(lpKeyVal));
    if(uMKeyNo == 0xFF){
        BYTE byKeyValBCD[16] = {0};
        BYTE byLoadKeyValBCD[24] = {0};
        UINT uLen = 0;
        //ASI to HEX
        CAutoHex::Hex2Bcd(lpKeyVal, byKeyValBCD, uLen);
        memcpy(byLoadKeyValBCD, byKeyValBCD, sizeof(byKeyValBCD));

        BYTE byAllZeroKey[16] = {0};
        m_encrypt.sm4_enc_ecb(byKeyValBCD, 16, byLoadKeyValBCD, 16, byAllZeroKey);

        //HEX to ASI
        memset(byKeyValueAsi, 0, sizeof(byKeyValueAsi));
        CAutoHex::Bcd2Hex(byLoadKeyValBCD, 16, (char *)byKeyValueAsi);
    }

    char szKeyNo[64] = { 0 };
    char szMKeyNo[64] = { 0 };
    sprintf(szKeyNo, "%02X", uKeyNo);
    if(uMKeyNo == 0xFF){
        strcpy(szMKeyNo, "00");
    } else {
        sprintf(szMKeyNo, "%02X", uMKeyNo);
    }

    vtCmd.append("\x63\x33\x30");
    vtCmd.append(szMKeyNo);
    vtCmd.append(szKeyNo);
    vtCmd.append((char *)byKeyValueAsi);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM工作密钥导入失败：lRet=%d[uKeyNo=%02X, uMKeyNo=%02X]", lRet, uKeyNo, uMKeyNo);
        return lRet;
    }

    if(lpKCV != nullptr){
       memcpy(lpKCV, vtResult.constData(), vtResult.size());
    }
    Log(ThisModule, 1, "SM工作密钥导入成功:uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
    return ERR_SUCCESS;
}


long CDevPIN_ZT598::SM4RemoteImportKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostSM2PKeyNo, UINT uKeyUse, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal, LPSTR lpKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_NOT_SUPPORT;
}

void SetTypeMode(BYTE uTypeVal, BYTE uModeVal, UINT &uType, UINT &uMode)
{
    uType = uTypeVal;
    uMode = uModeVal;
}
long CDevPIN_ZT598::SM4CryptData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpCryptData == nullptr || lpIVData == nullptr || lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    UINT uType = 0;
    UINT uSmMode = 0;
    switch (uMode)
    {
    case ECB_EN:    SetTypeMode(0x31, 0x41, uType, uSmMode);        break;// ECB // Encryption
    case ECB_DE:    SetTypeMode(0x32, 0x41, uType, uSmMode);        break;// ECB // Decryption
    case CBC_EN:    SetTypeMode(0x31, 0x42, uType, uSmMode);        break;// CBC // Encryption
    case CBC_DE:    SetTypeMode(0x32, 0x42, uType, uSmMode);        break;// CBC // Decryption
    default:
        Log(ThisModule, __LINE__, "不支持Crypt模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    UINT uMaxLen = 1024;// 单次最大字符数
    QByteArray vtData;
    QByteArray vtResult;
    QByteArray vtResultTotal;
    QByteArray vtIVData(lpIVData);
    QByteArray vtCryptData(lpCryptData);
    Pading(vtCryptData, bPadding, true, 32);// 填充
    if (uMode == ECB_EN || uMode == ECB_DE)
        vtIVData = "00000000000000000000000000000000";
    do
    {
        if ((UINT)vtCryptData.size() <= uMaxLen)
        {
            vtData = vtCryptData;
            vtCryptData.clear();
        }
        else
        {
            vtData = vtCryptData.mid(0, uMaxLen);
            vtCryptData = vtCryptData.mid(uMaxLen);
        }

        long lRet = SM4Crypt(uKeyNo, uType, uSmMode, vtData, vtIVData, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "SM4加解密失败：%d[uMode=%d]", lRet, uMode);
            return lRet;
        }

        // 处理IV
        if (uMode == CBC_EN)
            vtIVData = vtResult.right(32);
        else if (uMode == CBC_DE)
            vtIVData = vtData.right(32);

        // 处理结果
        vtResultTotal += vtResult;
        if (vtCryptData.isEmpty())// 已全部计算
            break;
    } while (true);

    memcpy(lpData, vtResultTotal.constData(), vtResultTotal.size());
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::SM4MACData(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpMacData == nullptr || lpIVData == nullptr || lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    UINT uMacType = 0;
    switch (uMode)
    {
    case MAC_PBOC:          uMacType = 0x43;    break;
    case MAC_BANKSYS:       uMacType = 0x40;    break;
    default:
        Log(ThisModule, __LINE__, "不支持MAC模式：uMode=%d", uMode);
        return ERR_NOT_SUPPORT;
    }

    UINT uMaxLen = 1024;// 单次最大计算MAC字符数
    CAutoHex cXor;
    QByteArray vtResult;
    QByteArray vtXorData;
    QByteArray vtIVData(lpIVData);
    QByteArray vtMacData(lpMacData);
    Pading(vtMacData, 0x00, true, 32);
    do
    {
        if ((UINT)vtMacData.size() <= uMaxLen)
        {
            vtXorData = vtMacData;
            vtMacData.clear();
        }
        else
        {
            vtXorData = vtMacData.mid(0, uMaxLen);
            vtMacData = vtMacData.mid(uMaxLen);
        }

        long lRet = SM4MAC(uKeyNo, uMacType, vtXorData, vtIVData, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "SM4计算MAC失败：%d[uMode=%d，uMacType=%02X]", lRet, uMode, uMacType);
            return lRet;
        }

        vtIVData = vtResult;// 新IV
        if (vtMacData.isEmpty())// 已全部计算MAC
            break;
    } while (true);

    memcpy(lpData, vtResult.constData(), vtResult.size());
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::SM4PinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    UINT uLen = strlen(lpCustomerData);
    if (uLen != 12)
    {
        Log(ThisModule, __LINE__, "无效长度用户数据：%d!=12", uLen);
        return ERR_PARAM;
    }

    UINT uType = 0;
    switch (uFormat)
    {
    case PIN_SM4:       uType = 0x30;       break;
    default:
        Log(ThisModule, __LINE__, "不支持模式：%02X", uFormat);
        return ERR_NOT_SUPPORT;
    }

    // 设置填充
    long lRet = SetControlMode(3, bPadding);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置填充失败：%d", lRet);
        return lRet;
    }

    // 设置密钥和算法，并读取密码块
    LOGDEVACTION();
    CAutoHex cHex;
    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.resize(4);
    sprintf(vtCmd.data(), "\x6F%02X%c", uKeyNo, uType);
    vtCmd.append(lpCustomerData);
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取密码块失败：%d[nKeyNo=%02X,uFormat=%02X]", lRet, uKeyNo, uFormat);
        return lRet;
    }

    if (vtResult.length() != 36)
    {
        Log(ThisModule, __LINE__, "读取密码块数据异常：%s", vtResult.constData());
        return ERR_REDATA;
    }

    // 因前面还有四个其他值
    memcpy(lpPinBlock, vtResult.constData() + 4, 32);
    Log(ThisModule, 1, "读取国密密码块成功");
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::SM2ImportKey(UINT uKeyNo, UINT uVendorPKeyNo, BOOL bPublicKey, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZT598::SM2EncryptData(LPCSTR lpSM2PKeyData, LPCSTR lpData, LPCSTR lpCryptData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZT598::SM2DecryptData(LPCSTR lpSM2SKeyData, LPCSTR lpCryptData, LPCSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZT598::SM2SignData(LPCSTR lpSM2SKeyData, LPCSTR lpZA, LPCSTR lpData, LPSTR lpSignData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZT598::SM2VerifySign(LPCSTR lpSM2PKeyData, LPCSTR lpZA, LPCSTR lpData, LPCSTR lpSignData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZT598::SM2ExportPKey(UINT uKeyNo, UINT uSignKeyNo, LPSTR lpZA, LPSTR lpSM2PKeyData, LPSTR lpSignData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZT598::SM2GenerateKey(UINT uSKeyNo, UINT uPKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZT598::SM3CaculateZAData(LPCSTR lpUserData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZT598::SMDeleteKey(UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x6E");
    if (uKeyNo == 0)//  删除全部
        vtCmd.append("\x33\x32");
    else
        vtCmd.append("\x33\x33");

    char szKeyNo[64] = { 0 };
    sprintf(szKeyNo, "%02X", uKeyNo);
    vtCmd.append(szKeyNo);

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "删除国密密钥命令失败：%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    //下载全0 SM4明文密钥
    char szKCV[64] = { 0 };
    memset(szKCV, 0, sizeof(szKCV));
    lRet = SM4ImportMKey(0, gIniUID, szKCV);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载国密全0密钥失败");
        return lRet;
    }

    Log(ThisModule, 1, "删除国密密钥成功：uKeyNo=%d", uKeyNo);
    return 0;
}

long CDevPIN_ZT598::SM4SaveKeyType(UINT uType)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    // uType：0->不相同，1->可相同
    UINT nCmd = (uType == 0) ? 0 : 1;
    QByteArray vtResult;
    QByteArray vtCmd;
    vtCmd.resize(4);
    sprintf(vtCmd.data(), "\x35\x44\x33%d", nCmd);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置国密密钥保存方式失败：uType=%d[0->不相同，1->可相同], lRet=%d", uType, lRet);
        return lRet;
    }
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::RSAInitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x49");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取RSA密钥信息失败：lRet=%d", lRet);
        return lRet;
    }
    if (vtResult.size() < 22)
    {
        Log(ThisModule, __LINE__, "RSA密钥信息长度异常：uLen=%d", vtResult.size());
        return ERR_REDATA;
    }

    // 分析返回数据
    QByteArray strStatus = vtResult.mid(16, 6).data();
    Log(ThisModule, 1, "RSA密钥信息：KeyStatus=%s，下载方法：%02X", strStatus.data(), (BYTE)strStatus[5]);

    // 厂商公钥
    if ((BYTE)strStatus[0] != 0x31)
    {
        Log(ThisModule, __LINE__, "没下载厂商公钥");
        return ERR_RSA_NOT_INIT;
    }
    // 键盘签名序号
    if ((BYTE)strStatus[1] != 0x31)
    {
        Log(ThisModule, __LINE__, "没下载键盘签名序号");
        return ERR_RSA_NOT_INIT;
    }
    // 没下载EPP私钥
    if ((BYTE)strStatus[2] != 0x31)
    {
        Log(ThisModule, __LINE__, "没下载EPP私钥");
        return ERR_RSA_NOT_INIT;
    }
    // 没下载EPP公钥
    if ((BYTE)strStatus[3] != 0x31)
    {
        Log(ThisModule, __LINE__, "没下载EPP公钥");
        return ERR_RSA_NOT_INIT;
    }
    // HOST公钥，不用判断，因此是在ATM运行时，自动下载的
    //    if ((BYTE)strStatus[4] != 0x31)
    //    {
    //        Log(ThisModule, __LINE__, "没下载HOST公钥");
    //    }

    return ERR_SUCCESS;
}

long CDevPIN_ZT598::RSAImportRootHostPK(QByteArray strHostPK, QByteArray strSignHostPK)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x4F\x31");

    // 公钥和签名
    QByteArray vtBCD;
    QByteArray strHexPK = FmImportPK(strHostPK.toStdString(), strSignHostPK.toStdString()).c_str();
    CAutoHex::Hex2Bcd(strHexPK, vtBCD);
    vtCmd.push_back(vtBCD);

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载RootHostPK命令失败：lRet=%d", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "下载RootHostPK成功");
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::RSAImportSignHostPK(QByteArray strHostPK, QByteArray strSignHostPK)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x50\x31");

    // 公钥和签名
    QByteArray vtBCD;
    QByteArray strHexPK = FmImportPK(strHostPK.data(), strSignHostPK.data()).c_str();
    CAutoHex::Hex2Bcd(strHexPK, vtBCD);
    vtCmd.push_back(vtBCD);

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载RootHostPK命令失败：lRet=%d", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "下载SignHostPK成功");
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::RSAExportEppEncryptPK(QByteArray &strPK, QByteArray &strSignPK)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x47");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导出RSA加密公钥信息失败：lRet=%d", lRet);
        return lRet;
    }

    // 分析返回数据
    Log(ThisModule, 1, "加密公钥数据[ %s ]签名", ((BYTE)vtResult[0] == 0x31) ? "已" : "未");
    if ((BYTE)vtResult[0] != 0x31)
    {
        Log(ThisModule, __LINE__, "加密公钥数据未签名失败");
        return ERR_REDATA;
    }

    vectorString vtPK;
    if (!GetPKByExportData(vtResult.data() + 1, vtResult.size() - 1, vtPK))
    {
        Log(ThisModule, __LINE__, "解析导出加密公钥数据失败");
        return ERR_REDATA;
    }
    if (vtPK.size() != 2)
    {
        Log(ThisModule, __LINE__, "解析数据返回个数不正确");
        return ERR_REDATA;
    }

    strPK = vtPK[0].c_str();
    strSignPK = vtPK[1].c_str();
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::RSAExportEppSN(QByteArray &strSN, QByteArray &strSignSN)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x46");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导出密码键盘SN失败：lRet=%d", lRet);
        return lRet;
    }

    // 分析返回数据
    Log(ThisModule, 1, "EPP序号[ %s ]签名", ((BYTE)vtResult[0] == 0x31) ? "已" : "未");
    vectorString vtData;
    if (!GetTLVData(vtResult.data() + 1, vtResult.size() - 1, vtData))
    {
        Log(ThisModule, __LINE__, "解析TLV数据失败");
        return ERR_REDATA;
    }
    if (vtData.size() != 2)
    {
        Log(ThisModule, __LINE__, "解析TLV数据返回个数不正确");
        return ERR_REDATA;
    }
    // 序号
    CAutoHex cHex((BYTE *)vtData[0].data(), vtData[0].length());
    strSN = cHex.GetHex();

    // 签名序号
    cHex.Bcd2Hex((BYTE *)vtData[1].data(), vtData[1].length());
    strSignSN = cHex.GetHex();
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::RSAExportEppRandData(QByteArray &strRandData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x4A\x20");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "生成EPP随机数失败：lRet=%d", lRet);
        return lRet;
    }

    // 随机数
    strRandData = vtResult;
    Log(ThisModule, 1, "生成EPP随机数成功：EppRandData=%s]", vtResult.data());
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::RSAImportDesKey(UINT uMKeyNo, QByteArray strMKVal, QByteArray strSignMKVal, QByteArray &strKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    char szIndex[8] = {0};
    QByteArray vtResult;
    QByteArray vtCmd("\x48\x30\x30\x30\x31\x30\x31");

    // 保存位置
    sprintf(szIndex, "%02d", uMKeyNo);
    vtCmd.push_back(szIndex);

    // 公钥和签名
    QByteArray vtBCD;
    std::string strHexMK = FmImportDesKey(strMKVal.data(), strSignMKVal.data());
    CAutoHex::Hex2Bcd(strHexMK.c_str(), vtBCD);
    vtCmd.push_back(vtBCD);

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载DESKEY命令失败：lRet=%d", lRet);
        return lRet;
    }

    if (vtResult.size() != 9)
    {
        Log(ThisModule, __LINE__, "返回数据长度不正确，要求长度[9] ！= %d", vtResult.size());
        return ERR_REDATA;
    }

    // DES key length
    // 0x30 = 8 bytes
    // 0x31 = 16 bytes
    // 0x32 = 24 bytes
    int nKeyLen = ((BYTE)vtResult[0] == 31) ? 16 : 24;
    CAutoHex::Bcd2Hex((BYTE *)(vtResult.data() + 1), 8, strKCV);

    Log(ThisModule, 1, "下载DESKEY成功[KeyLen=%d][KCV=%s]", nKeyLen, strKCV.constData());
    return ERR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
long CDevPIN_ZT598::PackCmd(LPCSTR lpCmd, DWORD dwCmdLen, LPSTR lpData, DWORD &dwSize)
{
    if (lpCmd == nullptr || lpData == nullptr || dwCmdLen >= dwSize)
        return ERR_DEVPORT_PARAM;

    // Command format: 02h+<Ln>+<CMD>+<DATA>+03h+<LRC>
    // if Ln > 999 with Ln=000 in command
    BYTE szCmd[8192] = {0};
    szCmd[0] = CMD_START;

    if (dwCmdLen < 999)
        sprintf((char *)szCmd + 1, "%03u", dwCmdLen);
    else
        sprintf((char *)szCmd + 1, "%s", "000");

    memcpy(szCmd + 4, lpCmd, dwCmdLen);
    szCmd[4 + dwCmdLen] = CMD_END;
    memcpy(szCmd + 5 + dwCmdLen, GetLRC(szCmd + 1, 4 + dwCmdLen).c_str(), 2);
    if (7 + dwCmdLen < dwSize)
        dwSize = 7 + dwCmdLen;

    memcpy(lpData, szCmd, dwSize);
    return 0;
}

long CDevPIN_ZT598::UnPackCmd(LPCSTR lpCmd, DWORD dwReadLen, LPSTR lpData, DWORD &dwSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    // 键盘支持7字节长度的LEN返回，即LEN可能为 "000" + 4 bytes  or  "XXx"+4 bytes (此项为OpenPort 时的默认设置)
    // Return format: 02h+<Ln>+<RTN>+<DATA>+03h+<LRC>
    CAutoHex cHex;
    DWORD dwCmdLen      = 0;    // 命令长度
    DWORD dwDataLen     = 0;    // 数据长度
    DWORD dwLenOffset   = 0;    // 长度偏移位
    DWORD dwCmdOffset   = 0;    // 命令头偏移位
    DWORD dwDataOffset  = 0;    // 数据偏移位
    DWORD dwCount       = 0;    // 开始和结束符计算出的命令长度
    bool bCmdOK         = false;

    // 查找开始和结束符
    for (DWORD i = 0; i < dwReadLen; i++)
    {
        if ((BYTE)lpCmd[i] == CMD_START && dwCmdOffset == 0)
        {
            dwCmdOffset = i;// 找到开始符
            dwLenOffset = LLEN_3;
            // LEN可能为 "000" + 4 bytes
            char szTmp[32] = {0};
            memcpy(szTmp, lpCmd + dwCmdOffset + HLEN, 3);
            dwDataLen = (DWORD)atol(szTmp);
//            if (dwDataLen == 0)
//            {
//                dwLenOffset = LLEN_7;
//                memset(szTmp, 0x00, sizeof(szTmp));
//                memcpy(szTmp, lpCmd + HLEN + LLEN_3, 4);
//                dwDataLen = (DWORD)atol(szTmp);
//            }
            // 有长度，直接向后位移，因数据中可能有0x03字符
            i += dwLenOffset + dwDataLen;
        }
        else if ((BYTE)lpCmd[i] == CMD_END)
        {
            if (i + 3 <= dwReadLen)
            {
                bCmdOK  = true;
                dwCount = i - dwCmdOffset + 3;
            }
            break;
        }
    }
    if (!bCmdOK)
    {
        Log(ThisModule, __LINE__, "返回数据结构分析失败[dwReadLen=%u]", dwReadLen);
        return ERR_REDATA;
    }
    // 长度判断
    if (dwDataLen > 0)//dwDataLen有可能为0
    {
        dwCmdLen = HLEN + dwLenOffset + dwDataLen + 3;
        if (dwCmdLen != dwCount)
        {
            Log(ThisModule, __LINE__, "返回数据长度不对：%u != %u[dwReadLen=%u]", dwCmdLen, dwCount, dwReadLen);
            return ERR_REDATA;
        }
    }
    else
    {
        dwDataLen = dwCount - HLEN - dwLenOffset - 3;
    }

    // 校验数据
    std::string strLRC  = GetLRC((BYTE*)lpCmd + dwCmdOffset + 1, dwCount - 3);
    std::string strCmdLRC;
    strCmdLRC.append(1, lpCmd[dwCmdOffset + dwCount - 2]);
    strCmdLRC.append(1, lpCmd[dwCmdOffset + dwCount - 1]);
    if (strCmdLRC != strLRC)
    {
        Log(ThisModule, __LINE__, "返回数据检验码LRC匹配失败：CmdLRC[%s] != LRC[%s]", strCmdLRC.c_str(), strLRC.c_str());
        return ERR_REDATA;
    }

    // 返回码：6Xh/7X/8X/9Xh Command is ok
    BYTE byRTN = lpCmd[dwCmdOffset + 1 + dwLenOffset];
    if (byRTN < 0x60 || (byRTN > 0x9F && byRTN != 0xA3))
    {
        UpdateStatus(DEVICE_ONLINE, byRTN);
        return ConvertErrCode(byRTN, ThisModule);
    }
    UpdateStatus(DEVICE_ONLINE, 0);

    // 返回数据
    if (dwDataLen < dwSize)
    {
        dwSize = dwDataLen - 1;
    }
    if (dwSize > 0)
    {
        dwDataOffset = dwCmdOffset + 1 + dwLenOffset + 1;
        memcpy(lpData, lpCmd + dwDataOffset, dwSize);
    }

    return 0;
}

string CDevPIN_ZT598::GetLRC(const BYTE *pszCmd, DWORD dwCmdLen)
{
    // LRC——2 byte HEX length, Exclusive OR all byte of between 02h and 03h, include 03h
    BYTE byLRC = 0;
    for (DWORD i = 0; i < dwCmdLen; i++)
    {
        byLRC ^= pszCmd[i];
    }

    char szLRC[8] = { 0 };
    sprintf(szLRC, "%02X", byLRC);
    return std::string(szLRC);
}

string CDevPIN_ZT598::GetRandData()
{
    char szTmp[256] = { 0 };
    std::string strRandData;

    srand((UINT)time(nullptr));
    for (int i = 0; ; i++)
    {
        sprintf(szTmp + strlen(szTmp), "%d", rand());
        if (strlen(szTmp) > 8)
            break;
    }

    CAutoHex cHex((BYTE *)szTmp, 8);
    strRandData = cHex.GetHex();
    return strRandData;
}

long CDevPIN_ZT598::SendReadCmd(const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 命令打包
    BYTE szPackCmd[8192] = { 0 };
    DWORD dwCmdSize = sizeof(szPackCmd);
    long lRet = PackCmd(vtCmd.constData(), vtCmd.size(), (char *)szPackCmd, dwCmdSize);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打包命令失败：%d", lRet);
        return lRet;
    }

    // 发送命令和接收返回数据
    QByteArray vtSendCmd;
    QByteArray vtReadData;
    vtSendCmd.append((char *)szPackCmd, dwCmdSize);
    lRet = SendReadData(vtSendCmd, vtReadData, dwTimeOut);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "调用SendAndRead发送或等待命令返回失败：lRet=%d", lRet);
        return lRet;
    }

    char szData[8192] = { 0 };
    DWORD dwSize = sizeof(szData);
    lRet = UnPackCmd(vtReadData, vtReadData.size(), szData, dwSize);
    if (lRet == ERR_SUCCESS)
    {
        vtResult.clear();
        vtResult.append(szData, dwSize);
    }
    return lRet;
}

long CDevPIN_ZT598::SendReadData(const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 发送命令和接收返回数据
    long lRet = m_pDev->Send(vtCmd, vtCmd.size(), dwTimeOut);
//    if (lRet != vtCmd.size())
    if(lRet != 0)
    {
        Log(ThisModule, __LINE__, "调用Send发送命令返回失败：lRet=%d", lRet);
        return ERR_DEVPORT_WRITE;
    }

    char szRead[8192] = { 0 };
    DWORD dwReadLen   = sizeof(szRead);
    ULONG dwStart = CQtTime::GetSysTick();
    do
    {
        if (CQtTime::GetSysTick() - dwStart > dwTimeOut)
            break;

        memset(szRead, 0x00, sizeof(szRead));
        dwReadLen   = sizeof(szRead);
        lRet = m_pDev->Read(szRead, dwReadLen, dwTimeOut);
        if (lRet == 0 && dwReadLen == 0)
            continue;

        if (lRet != 0)
        {
            Log(ThisModule, __LINE__, "调用Read返回失败：lRet=%d", lRet);
            return  lRet;
        }
        if (lRet == 0 && dwReadLen > 0)
        {
            // 合并并校验数据的完整性
            DWORD dwLen = 0;    // 数据长度
            bool bStart = false;
            bool bEnd   = false;
            vtResult.append(szRead, dwReadLen);
            if (vtResult.size() < 8)// 没有达到最小长度
                continue;
            for (int i = 0; i < vtResult.size(); i++)
            {
                if (vtResult.at(i) == CMD_START && !bStart)
                {
                    bStart = true;
                    // LEN可能为 "000" + 4 bytes
                    if (vtResult.size() < i + 4)
                        break;

                    char szTmp[32] = {0};
                    memcpy(szTmp, vtResult.constData() + i + 1, 3);
                    dwLen = (DWORD)atol(szTmp);
                    if (dwLen == 0)
                    {
                        if (vtResult.size() < i + 8)
                            break;
                        memcpy(szTmp, vtResult.constData() + i + 4, 4);
                        dwLen = (DWORD)atol(szTmp);
                    }
                }

                if (vtResult.at(i) == CMD_END)
                {
                    if (dwLen <= 999 && i >= dwLen + 4)
                        bEnd = true;
                    else if (dwLen > 999 && i >= dwLen + 8)
                        bEnd = true;
                    if (bEnd && vtResult.size() < i + 2)
                        bEnd = false;
                }
                if (bStart && bEnd && i + 3 <= vtResult.size())
                    return ERR_DEVPORT_SUCCESS;
            }
        }

    } while (true);

    Log(ThisModule, __LINE__, "读取返回数据超时失败");
    return ERR_DEVPORT_RTIMEOUT;
}

long CDevPIN_ZT598::ConvertErrCode(BYTE byRTN, LPCSTR ThisModule)
{
    auto it = m_mapErrCode.find(byRTN);
    if (it != m_mapErrCode.end())
    {
        Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][%s]", it->first, it->second.c_str());
        return (-byRTN);
    }
    else
    {
        Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][未知错误]", byRTN);
        return ERR_OTHER;
    }
}

void CDevPIN_ZT598::InitErrCode()
{
    m_mapErrCode[0xD0] = "SM2 Encrypt Failure";
    m_mapErrCode[0xD1] = "SM2 Decrypt Failure";
    m_mapErrCode[0xD2] = "SM2 Signature Failure";
    m_mapErrCode[0xD3] = "SM2 Verify Signature Failure";
    m_mapErrCode[0xD4] = "SM2 Key Exchange Failure";
    m_mapErrCode[0xD5] = "SM2 Verify Key Exchange Failure ";
    m_mapErrCode[0xD6] = "A980 return other abnormal error";
    m_mapErrCode[0xD7] = "A980 no response";
    m_mapErrCode[0xD8] = "Invalid SM4 Key Value";
    m_mapErrCode[0xDA] = "RSA Key has not init";
    m_mapErrCode[0xDB] = "RSA Key has been initialized";
    m_mapErrCode[0xDC] = "Command MAC check fail";
    m_mapErrCode[0xDD] = "The device has been tampered";
    m_mapErrCode[0xDE] = "A980 Chip don’t mounted";

    m_mapErrCode[0xE0] = "In low voltage, need to replace the battery";
    m_mapErrCode[0xE1] = "Not IMK which can’t do wnload the master key";
    m_mapErrCode[0xE2] = "Not TMK which can’t download the work key ";
    m_mapErrCode[0xE3] = "Key length Error";
    m_mapErrCode[0xE4] = "Key index doesn’t exist";
    m_mapErrCode[0xE5] = "no tag to match or no such key";
    m_mapErrCode[0xE6] = "Key parity check error";
    m_mapErrCode[0xE7] = "Key check value error";
    m_mapErrCode[0xE8] = "command length error";
    m_mapErrCode[0xE9] = "Illegal data ";
    m_mapErrCode[0xEA] = "Code Information doesn’t exist";
    m_mapErrCode[0xEB] = "Parameter type error";
    m_mapErrCode[0xEC] = "authentication fails";
    m_mapErrCode[0xED] = "authentication locks";
    m_mapErrCode[0xEE] = "Overtime input on EPP";
    m_mapErrCode[0xEF] = "Other abnormal error";

    m_mapErrCode[0xF1] = "The structure of RSA key is error";
    m_mapErrCode[0xF2] = "The signature of RSA length is error or not match with RSA key";
    m_mapErrCode[0xF3] = "The padding of RSA data is error or don’t meet PKCS requirement";
    m_mapErrCode[0xF4] = "Status Signatuer Verification error";
    m_mapErrCode[0xF5] = "Status Invalid Modulus Length";
    m_mapErrCode[0xF6] = "Status Invalid Exponent Length";
    m_mapErrCode[0xF7] = "Status Serial Verification error";
    m_mapErrCode[0xF8] = "Illegle input mode";
    m_mapErrCode[0xF9] = "Invalid password or pass word not initialization";
    m_mapErrCode[0xFA] = "UID or KBPK not initialize";
    m_mapErrCode[0xFB] = "Command locked";
    m_mapErrCode[0xFC] = "Password length error";
    m_mapErrCode[0xFD] = "RSA Key Read failure";
    m_mapErrCode[0xFE] = "External EEPORM hardware error";
    m_mapErrCode[0xFF] = "The anti-removal switch is not closed";
    return;
}

void CDevPIN_ZT598::UpdateStatus(WORD wDevice, long lErrCode)
{
    m_stStatus.clear();
    m_stStatus.wDevice = wDevice;
    sprintf(m_stStatus.szErrCode, "0%02X", qAbs(lErrCode));
}

bool CDevPIN_ZT598::ConvertKeyVal(BYTE bKey, EPP_KEYVAL &stKeyVal)
{
    EPP_KEYVAL *pKeyVal = gszKeyVal;
    UINT uKeyCount = sizeof(gszKeyVal) / sizeof(gszKeyVal[0]);
    //  if (std::string::npos != m_strFirmware.find("-H6J-"))
    //  {
    //      pKeyVal = gszKeyVal_H6J;
    //      uKeyCount = sizeof(gszKeyVal_H6J) / sizeof(gszKeyVal_H6J[0]);
    //  }

    for (UINT k = 0; k < uKeyCount; k++)
    {
        if (bKey == pKeyVal[k].byKeyVal)
        {
            memcpy(&stKeyVal, &pKeyVal[k], sizeof(EPP_KEYVAL));
            return true;
        }
    }
    return false;
}

long CDevPIN_ZT598::AuthRandData(LPCSTR lpUID, QByteArray &vtResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    UINT uUidLen = strlen(lpUID);
    if (uUidLen != 32)
    {
        Log(ThisModule, __LINE__, "无效UID数据长度：Len=%d",  uUidLen);
        return ERR_PARAM;
    }

    QByteArray vtCmd("\x3A\x41\x30");
    vtCmd.append(lpUID);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取32位长度的认证随机数失败：%d", lRet);
        return lRet;
    }
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::AuthEpp(LPCSTR lpUID, UINT uAuthMode, UINT uKeyNo, QByteArray *pvtResult/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x3A");
    switch (uAuthMode)
    {
    case 0:
        vtCmd.append("\x38\x30");
        break;
    case 1:
        {
            char szKeyNo[64] = { 0 };
            sprintf(szKeyNo, "%02X", uKeyNo);
            vtCmd.append(szKeyNo);
        }
        break;
    case 2:
        vtCmd.append("\x39\x30");
        break;
    default:
        Log(ThisModule, __LINE__, "无效认证模式: uAuthMode=%d", uAuthMode);
        return ERR_PARAM;
    }
    UINT uUidLen = strlen(lpUID);
    if (uUidLen != 32)
    {
        Log(ThisModule, __LINE__, "无效UID数据长度：uAuthMode=%d[Len=%d]", uAuthMode, uUidLen);
        return ERR_PARAM;
    }

    vtCmd.append(lpUID);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "认证命令失败：%d[uAuthMode=%d]", lRet, uAuthMode);
        return lRet;
    }
    if (pvtResult != nullptr)
        *pvtResult = vtResult;

    Log(ThisModule, 1, "认证成功: uAuthMode=%d", uAuthMode);
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::AuthKey(UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    QByteArray vtResult;
    QByteArray vtUidData;
    long lRet = AuthRandData(gEppUID, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取随机数失败：%d", lRet);
        return lRet;
    }

    lRet = AuthEpp(vtResult.constData(), 1, uKeyNo, &vtUidData);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "认证密钥失败：%d[uKeyNo=%02X]", lRet, uKeyNo);
        return lRet;
    }

    // 加密返回数据
    char sz3DesAuth[256] = {0};
    m_cDES.gDes(vtUidData.data(), (char *)gEppUID, sz3DesAuth);
    lRet = AuthEpp(sz3DesAuth, 2, 0);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "验证UID失败");
        return lRet;
    }
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::AuthUID()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    QByteArray vtResult;
    QByteArray vtUidData;
    long lRet = AuthRandData(gEppUID, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取随机数失败：%d", lRet);
        return lRet;
    }

    lRet = AuthEpp(vtResult.constData(), 1, 0, &vtUidData);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "认证UID失败：%d", lRet);
        return lRet;
    }

    // 加密返回数据
    char sz3DesAuth[256] = { 0 };
    m_cDES.gDes(vtUidData.data(), (char *)gEppUID, sz3DesAuth);
    lRet = AuthEpp(sz3DesAuth, 2, 0);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "验证UID失败");
        return lRet;
    }
    return ERR_SUCCESS;
}


long CDevPIN_ZT598::SetControlMode(BYTE nMode, BYTE nCmd)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;
    vtCmd.resize(4);
    sprintf(vtCmd.data(), "\x35%X%02X", nMode, nCmd);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置控制模式失败：%d[%X%02X]", lRet, nMode, nCmd);
        return lRet;
    }
    return ERR_SUCCESS;
}

QByteArray *CDevPIN_ZT598::Pading(QByteArray &vtResult, BYTE byPading, bool bEnd/* = true*/, UINT uPadLen/* = 16*/)
{
    char szPad[8] = {0};
    QByteArray vtPad;
    UINT uLen = vtResult.size() % uPadLen;
    if (uLen == 0)
        return &vtResult;

    sprintf(szPad, "%02X", byPading);
    uLen = (uPadLen - uLen) / 2;
    for (UINT i = 0; i < uLen; i++)
    {
        vtPad += szPad;
    }
    if (!vtPad.isEmpty())
    {
        if (bEnd)
            vtResult.append(vtPad);
        else
            vtResult.prepend(vtPad);
    }
    return &vtResult;
}

long CDevPIN_ZT598::Crypt(UINT uKeyNo, UINT uCryptType, LPCSTR lpCryptData, QByteArray &vtResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 认证密钥
    long lRet = AuthKey(uKeyNo);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "认证密钥失败：%d[uKeyNo=%02X]", lRet, uKeyNo);
        return lRet;
    }

    // 加或解密
    LOGDEVACTION();
    QByteArray vtCmd;

    vtCmd.resize(4);
    sprintf(vtCmd.data(), "\x39%02X%c", uKeyNo, uCryptType);
    vtCmd.append(lpCryptData);
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "加或解密失败：%d[uKeyNo=%02X,uCryptType=%02X]", lRet, uKeyNo, uCryptType);
        return lRet;
    }

    if (vtResult.size() % 16 != 0)
    {
        Log(ThisModule, __LINE__, "加或解密返回数据长度异常：%s", vtResult.size());
        return ERR_REDATA;
    }

    return ERR_SUCCESS;
}

long CDevPIN_ZT598::MAC(UINT uKeyNo, UINT uMacType, LPCSTR lpMacData, QByteArray &vtResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 认证密钥
    long lRet = AuthKey(uKeyNo);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "认证密钥失败：%d[nKeyNo=%02X]", lRet, uKeyNo);
        return lRet;
    }

    // 计算MAC
    LOGDEVACTION();
    CAutoHex cHex;
    QByteArray vtBCD;
    QByteArray vtCmd;

    vtCmd.resize(4);
    sprintf(vtCmd.data(), "\x38%02X%c", uKeyNo, uMacType);
    cHex.Hex2Bcd(lpMacData, vtBCD);
    vtCmd.append(vtBCD);
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "计算MAC失败：%d[nKeyNo=%02X,uMacType=%02X]", lRet, uKeyNo, uMacType);
        return lRet;
    }

    if (vtResult.size() % 16 != 0)
    {
        Log(ThisModule, __LINE__, "计算MAC返回数据长度异常：%s", vtResult.size());
        return ERR_REDATA;
    }

    return ERR_SUCCESS;
}

long CDevPIN_ZT598::SetPinInputLen(UINT uMinLen /*= 0*/, UINT uMaxLen /*= 12*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;
    vtCmd.resize(4);
    sprintf(vtCmd.data(), "\x35\x34%02d", uMinLen);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置密码最小长度失败：%d[uMinLen=%d]", lRet, uMinLen);
        return lRet;
    }

    sprintf(vtCmd.data(), "\x35\x35%02d", uMaxLen);
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置密码最大长度失败：%d[uMaxLen=%d]", lRet, uMaxLen);
        return lRet;
    }
    return ERR_SUCCESS;
}

long CDevPIN_ZT598::SM4Crypt(UINT uKeyNo, UINT uType, UINT uMode, LPCSTR lpCryptData, LPCSTR lpIVData, QByteArray &vtResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    UINT uIVLen = strlen(lpIVData);
    if (uIVLen != 32)
    {
        Log(ThisModule, __LINE__, "无效IV长度：%d != 32", uIVLen);
        return ERR_PARAM;
    }

    // 加或解密
    LOGDEVACTION();
    QByteArray vtCmd;

    vtCmd.resize(7);
    sprintf(vtCmd.data(), "\x64%02X%02X%02X", uKeyNo, uType, uMode);
    vtCmd.append(lpIVData);
    vtCmd.append(lpCryptData);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "加或解密失败：%d[uKeyNo=%02X,uType=%02X]", lRet, uKeyNo, uType);
        return lRet;
    }

    if (vtResult.size() % 32 != 0)
    {
        Log(ThisModule, __LINE__, "加或解密返回数据长度异常：%s", vtResult.size());
        return ERR_REDATA;
    }

    return ERR_SUCCESS;
}

long CDevPIN_ZT598::SM4MAC(UINT uKeyNo, UINT uMacType, LPCSTR lpMacData, LPCSTR lpIVData, QByteArray &vtResult)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    UINT uIVLen = strlen(lpIVData);
    if (uIVLen != 32)
    {
        Log(ThisModule, __LINE__, "无效IV长度：%d != 32", uIVLen);
        return ERR_PARAM;
    }

    // 加或解密
    LOGDEVACTION();
    QByteArray vtCmd;

    vtCmd.resize(37);
    sprintf(vtCmd.data(), "\x65%02X%s%02X", uKeyNo, lpIVData, uMacType);
    vtCmd.append(lpMacData);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "计算MAC失败：%d[uKeyNo=%02X,uType=%02X]", lRet, uKeyNo, uMacType);
        return lRet;
    }

    if (vtResult.size() % 32 != 0)
    {
        Log(ThisModule, __LINE__, "计算MAC返回数据长度异常：%s", vtResult.size());
        return ERR_REDATA;
    }

    return ERR_SUCCESS;
}

string CDevPIN_ZT598::GetTAG(string strHexData, bool bInteger)
{
    char szHead[8] = {0};
    char szTAG[32] = {0};
    char szLen[32] = {0};
    BYTE byBit     = 0;
    long lLen      = strHexData.length() / 2;

    bool bLenStart = false;
    for (int i = 3; i >= 0; i--)
    {
        BYTE byLen = (lLen >> (i * 8)) & 0xFF;
        if (byLen > 0)
            bLenStart = true;
        if (bLenStart)
            sprintf(szLen + strlen(szLen), "%02X", byLen);
    }
    if (strlen(szLen) == 0)
        sprintf(szLen, "%02X", lLen);

    sprintf(szHead, "%s", bInteger ? "02" : "30");
    if (lLen < 0x80)
    {
        sprintf(szTAG, "%s%s", szHead, szLen);
    }
    else
    {
        byBit = strlen(szLen) / 2;
        sprintf(szTAG, "%s8%d%s", szHead, byBit, szLen);
    }
    return std::string(szTAG);
}

string CDevPIN_ZT598::AddTAG(LPCSTR lpHexData, bool bInteger)
{
    std::string strHexData;
    strHexData = lpHexData;
    return strHexData.insert(0, GetTAG(strHexData, bInteger));
}

bool CDevPIN_ZT598::GetTLVData(LPCSTR lpData, DWORD dwDataLen, vectorString &vtData)
{
    //长度的表示方法有以下2种情况：
    //  1. 数据长度<0x80的时候，Length即为数据的长度；
    //  2. 数据长度>=0x80的时候，Length为0x8?，表示后面跟的？表示的Length长度。
    //     例如82 01 20，82表示82后面2个字节为长度的字节长度，数据长度为0x0120。
    // 开头：0x30 0x82 0x02 0x0A
    // 分段：0x02 0x82 0x01 0x01
    long lOffset   = 0;
    long lLen      = 0;
    char szTmp[64] = {0};
    if ((BYTE)lpData[0] != 0x30 && (BYTE)lpData[0] != 0x02)
    {
        return false;
    }

    if ((BYTE)lpData[1] < 0x80)
    {
        lLen = lpData[1];
        lOffset = 2;
    }
    else
    {
        BYTE byBit = lpData[1] & 0x0F;
        for (BYTE i = 0; i < byBit; i++)
        {
            sprintf(szTmp + i * 2, "%02X", lpData[2 + i]);
        }

        lLen = CAutoHex::HexToLen(szTmp);
        lOffset = 2 + byBit;
    }

    // 开头
    if ((BYTE)lpData[0] == 0x30)
    {
        return GetTLVData(lpData + lOffset, dwDataLen - lOffset, vtData);
    }

    std::string strData(lpData + lOffset, lLen);
    vtData.push_back(strData);
    if (lOffset + lLen < (long)dwDataLen)
    {
        return GetTLVData(lpData + lOffset + lLen, dwDataLen - lOffset - lLen, vtData);
    }
    return true;
}

string CDevPIN_ZT598::GetHexPK(string strModulus, string strExponent)
{
    // 公钥格式
    //  RSAPublicKey ::= SEQUENCE {
    //      modulus INTEGER;
    //      publicExponent INTEGER;
    //  }
    std::string strHexPK;

    strModulus.insert(0, GetTAG(strModulus).c_str());
    strExponent.insert(0, GetTAG(strExponent).c_str());
    strHexPK = strModulus + strExponent;
    strHexPK.insert(0, GetTAG(strHexPK, false).c_str());
    return strHexPK;
}

bool CDevPIN_ZT598::GetPKByExportData(LPCSTR lpData, DWORD dwDataLen, vectorString &vtPK)
{
    THISMODULE(__FUNCTION__);
    vectorString vtData;
    if (!GetTLVData(lpData, dwDataLen, vtData))
    {
        Log(ThisModule, __LINE__, "解析TLV数据失败");
        return false;
    }
    if (vtData.size() != 3)
    {
        Log(ThisModule, __LINE__, "解析TLV数据返回个数不正确");
        return false;
    }

    // 明文和签名的公钥数据
    char szModulus[2048]   = {0};
    char szExponent[32]    = {0};
    char szSignature[2048] = {0};

    CAutoHex cHex((BYTE *)vtData[0].data(), vtData[0].length());
    strcpy(szModulus, cHex.GetHex());

    cHex.Bcd2Hex((BYTE *)vtData[1].data(), vtData[1].length());
    strcpy(szExponent, cHex.GetHex());

    cHex.Bcd2Hex((BYTE *)vtData[2].data(), vtData[2].length());
    strcpy(szSignature, cHex.GetHex());

    vtPK.push_back(GetHexPK(szModulus, szExponent));
    vtPK.push_back(szSignature);

    // 签名数据不用加TAG头，因签名前的数据已有TAG头了
    //vtPK.push_back(m_cRSA.AddTAG(szSignature));
    return true;
}

string CDevPIN_ZT598::FmImportPK(string strHexPK, string strHexSignPK)
{
    /* 格式：
    EPPPublicKey ::= SEQUENCE {
        RSAPublicKey ::= SEQUENCE {
            modulus INTEGER;
            publicExponent INTEGER;
        }
        signature INTEGER;
    }
    */
    std::string strHexImportPK;
    strHexSignPK = AddTAG(strHexSignPK.c_str());
    strHexImportPK = strHexPK + strHexSignPK;
    strHexImportPK = AddTAG(strHexImportPK.c_str(), false);
    return strHexImportPK;
}

string CDevPIN_ZT598::FmImportDesKey(string strHexDES, string strHexSignDES)
{
    /* 格式：
        EncryptedSymmetricKey ::= SEQUENCE {
        encryptedKey INTEGER;
        signature INTEGER;
        }
        */
    std::string strHexImportDES;
    strHexDES       = AddTAG(strHexDES.c_str());
    strHexSignDES   = AddTAG(strHexSignDES.c_str());
    strHexImportDES = strHexDES + strHexSignDES;
    strHexImportDES = AddTAG(strHexImportDES.c_str(), false);
    return strHexImportDES;
}

//30-00-00-00(FS#0010)
void CDevPIN_ZT598::ReadConfig()
{
    THISMODULE(__FUNCTION__);

    QByteArray strFile("/PINConfig.ini");
#ifdef QT_WIN32
    strFile.prepend(SPETCPATH);
#else
    strFile.prepend(SPETCPATH);
#endif
    CINIFileReader cINIFileReader;
    if (!cINIFileReader.LoadINIFile(strFile.constData()))
    {
        Log(ThisModule, __LINE__, "加载配置文件失败：%s", strFile.constData());
        return;
    }

    CINIReader cIR = cINIFileReader.GetReaderSection("PINInfo");
    m_bSupportBackspace = (int)cIR.GetValue("SupportBackspace", 0) == 1;

    return;
}

//30-00-00-00(FS#0010)
bool CDevPIN_ZT598::SetClearKeyFunc()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd("\x35\x41\x32");

    if(m_bSupportBackspace){
        vtCmd.push_back(0x31);
    } else {
        vtCmd.push_back(0x32);
    }

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置CLEAR键功能[%s]失败", m_bSupportBackspace ? "BACKSPACE" : "CLEAR");
        return false;
    }

    return true;
}
