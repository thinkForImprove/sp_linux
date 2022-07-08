#include "DevPIN_ZTC90.h"

//ANS1
#define ASN1_INTEGER		0x02
#define ASN1_SEQUENCE		0x30
#define RVSDWORD(X)  (DWORD)((EXWORDHL(X & 0xffff) << 16) | (EXWORDHL((X & 0xffff0000) >> 16)))
static const char *ThisFile = "DevPIN_ZTC90.cpp";
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

static LPCSTR gZeroKeyData = "00000000000000000000000000000000";
static LPCSTR gRandomData = "8888888888888888";
static LPCSTR gEppUID = "8888888888888888";
static LPCSTR gAuthKey = "0000000000000000";
//////////////////////////////////////////////////////////////////////////
CDevPIN_ZTC90::CDevPIN_ZTC90(LPCSTR lpDevType)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);
    InitErrCode();
    m_bInitDesZeroKey = false;
    m_uKcvDecryptKeyNo = 0xFF;
    for(int i = 0; i < sizeof(m_iAlgoParamFlag)/sizeof(m_iAlgoParamFlag[0]);i++) {
        m_iAlgoParamFlag[i] = -1;
    }
}

CDevPIN_ZTC90::~CDevPIN_ZTC90()
{

}

void CDevPIN_ZTC90::Release()
{
    return;
}

long CDevPIN_ZTC90::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    //读取配置文件
    ReadConfig();

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

    //双向认证
    if(m_bUseEppAuth){
        lRet = MutualAuth(gAuthKey, gEppUID, 0x00);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }
    }

    //设置键盘参数
    lRet = SetEppCommConfigParam();
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //下载全0 DES密钥
    lRet = ImportAllZeroMKey();
    if (lRet != ERR_SUCCESS)
    {
        return lRet;
    }
    m_bInitDesZeroKey = true;

    //下载全0 SM4明文密钥
    lRet = ImportAllZeroMKey(false);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (m_pDev->IsOpened())
        m_pDev->Close();
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::InitEpp()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 删除密钥
    long lRet = DeleteKey();
    if (lRet != ERR_SUCCESS)
        return lRet;   

    //下载全0 DES密钥
    lRet = ImportAllZeroMKey();
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //下载全0国密密钥
    lRet = ImportAllZeroMKey(false);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    if(m_bUseEppAuth){
        lRet = MutualAuth(gAuthKey, gEppUID, 0x00);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }
    }

    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x31");
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

long CDevPIN_ZTC90::GetStatus(DEVPINSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);

    CAutoCopyDevStatus<DEVPINSTATUS> _auto(&stStatus, &m_stStatus);
    QByteArray vtResult;
    QByteArray vtCmd("\x44\x31");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        UpdateStatus(DEVICE_OFFLINE, lRet);
        Log(ThisModule, __LINE__, "测试键盘连接命令失败：%d", lRet);
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::InstallStatus(bool bClear)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();
/*
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
*/
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::GetFirmware(LPSTR lpFWVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    char szVerNo[17] = {0};
    char szSerialNumber[17] = {0};
    char szMFwVerNo[17] = {0};
    char szChipVerNo[4][22] = {0};

    QByteArray vtResult;
    QByteArray vtCmd("\x30");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取固件信息命令失败：%d", lRet);
    }
    memcpy(szVerNo, vtResult.constData(), 16);
    CAutoHex::Bcd2Hex((LPBYTE)(vtResult.constData() + 16), 8, szSerialNumber);

    // 读取主固件版本号
    vtResult.clear();
    vtCmd = "\x7F";
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取主固件版本号失败：%d", lRet);
    }
    memcpy(szMFwVerNo, vtResult.constData(), 16);

    // 读取芯片版本信息    
    for(int i = 0; i < 4; i++){
        vtCmd = "\x9F";
        BYTE byChipInfoType = i < 2 ? (0x31 + i) : (0x41 + i - 2);        
        vtCmd.push_back(byChipInfoType);
        vtResult.clear();
        lRet = SendReadCmd(vtCmd, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "读取芯片版本信息失败：%d[%d]", lRet, i);
        }
        memcpy(szChipVerNo[i], vtResult.constData(), vtResult.size());
    }

    sprintf(lpFWVer, "[Ver:%s][SN:%s][MainFirmware:%s][MainChip:%s|%s][SMChip:%s|%s]",
            szVerNo, szSerialNumber, szMFwVerNo, szChipVerNo[0], szChipVerNo[1],
            szChipVerNo[2], szChipVerNo[3]);
    m_strFirmware = lpFWVer;
    Log(ThisModule, 1, "EPP固件信息：%s", lpFWVer);
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::SetKeyValue(LPCSTR lpKeyVal)
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

    //键值转换16主键+８侧边键+8备用按键(固定为0x00)
    QByteArray vtKeyValBcd;
    CAutoHex::Hex2Bcd(lpKeyVal, vtKeyValBcd);
    vtKeyValBcd.append(8, 0x00);

    QByteArray vtResult;
    QByteArray vtCmd("\x7E");
    vtCmd.append(vtKeyValBcd);
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

long CDevPIN_ZTC90::SetActiveKey(DWORD dwActiveFuncKey, DWORD dwActiveFDKKey)
{
    return ERR_SUCCESS;
}

/*
 *@param :  uKeyMode:0xFF,明文模式;其他,密文模式解密密钥id
 */
long CDevPIN_ZTC90::ImportMKey(UINT uMKeyNo, UINT uKeyMode, LPCSTR lpKeyVal, LPSTR lpKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    LOGDEVACTION();
    long lRet = ERR_SUCCESS;
    QByteArray vtResult;
    QByteArray vtCmd;

    if (uMKeyNo > 0x1F)
    {
        Log(ThisModule, __LINE__, "无效保存主密钥ID：uMKeyNo=%02X", uMKeyNo);
        return ERR_PARAM;
    }

    // 下载主密钥
    UINT uEncKeyNo = uKeyMode;
    if(uEncKeyNo != 0xFF){
        //激活解密密钥
        lRet = ActiveKey(uEncKeyNo);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }

        //密文密钥号重新计算
        uMKeyNo += 0x40;
    }

    QByteArray vtKeyValBcd;
    CAutoHex::Hex2Bcd(lpKeyVal, vtKeyValBcd);
    vtCmd.push_back(0x32);
    vtCmd.push_back((char)uMKeyNo);
    vtCmd.append(vtKeyValBcd);
    lRet = SendReadCmd(vtCmd, vtResult);
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

long CDevPIN_ZTC90::ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode/* = 2*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    long lRet = ERR_SUCCESS;
    UINT uValLen = strlen(lpKeyVal);
    if(uValLen <= 0 || uValLen % 16 != 0){
        Log(ThisModule, __LINE__, "无效Key长度：uValLen=%d", uValLen);
        return ERR_PARAM;
    }

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;
    char cKeyValueAsi[80] = {0};
    bool bWKIsTDESKey =strlen(lpKeyVal) > 16;
    bool bMKIsTDESKey = true;
    if(uMKeyNo != 0xFF){
        bMKIsTDESKey = IsTDESKey(uMKeyNo);
    } else {
        //如果无主密钥，使用全0 DES密钥作为主密钥
        uMKeyNo = 0;
    }

    //如果主密钥为0，先对数据加密
    strcpy(cKeyValueAsi, lpKeyVal);
    if(uMKeyNo == 0){
        //使用全0 DES密钥加密数据
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

    //下载主密钥
    if(uKeyUse == PKU_KEYENCKEY){
        lRet = ImportMKey(uKeyNo, uMKeyNo, cKeyValueAsi, lpKCV);
        return lRet;
    }

    //下载工作密钥
    //根据主密钥和下载密钥长度设置下载参数
    lRet = SetLoadWKDESMode(bMKIsTDESKey);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }
    lRet = SetKCVDesMode(bWKIsTDESKey);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    QByteArray vtKeyValBcd;
    CAutoHex::Hex2Bcd(cKeyValueAsi, vtKeyValBcd);
    vtCmd.push_back(0x33);
    vtCmd.push_back(uMKeyNo);
    vtCmd.push_back(uKeyNo);
    vtCmd.append(vtKeyValBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "工作密钥导入失败:%d[KeyNo=%02x, MKeyNo=%02x,]", lRet, uKeyNo, uMKeyNo);
        return lRet;
    }

    if(lpKCV != nullptr){
        CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
        strcpy(lpKCV, autoHex.GetHex());
    }

    Log(ThisModule, 1, "工作密钥导入成功:KeyNo=%02X, MKeyNo=%02X", uKeyNo, uMKeyNo);
    return ERR_SUCCESS;
}


void CDevPIN_ZTC90::SetDesKcvDecryptKey(UINT uKeyNo)
{
    m_uKcvDecryptKeyNo = uKeyNo;
    return;
}

/*
 * @note : 0表示删除全部密钥
 */
long CDevPIN_ZTC90::DeleteKey(UINT uKeyNo /*= 0*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;
    if (uKeyNo == 0)
    {
        //删除所有DES密钥
        vtCmd.append("\x31\x38");
    }
    else
    {
        //删除指定密钥号密钥
        vtCmd.push_back(0x62);
        vtCmd.push_back((char)uKeyNo);
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

long CDevPIN_ZTC90::TextInput()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_vtKeyVal.clear();
    long lRet = SetCommConfigParam(0x03);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打开明文输入和按键声音命令失败：%d", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "打开明文输入成功");
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::PinInput(UINT uMinLen/* = 4*/, UINT uMaxLen/* = 6*/, bool bClearKeyAsBackspace/* = false*/)
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
        //设置最小长度
        lRet = SetAlgoParam(0x08, uMinLen);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "设置密文输入最小长度[%d]失败", uMinLen);
            return lRet;
        }

        // 打开按键声音（先打开按键声音，防止后打开时夹杂按键数据返回）
        lRet = SetCommConfigParam(0x02);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开按键声音命令失败：%d", lRet);
            return lRet;
        }

        QByteArray vtResult;
        QByteArray vtCmd("\x35");

        vtCmd.push_back((char)uMaxLen);
        vtCmd.push_back(0x01);
        vtCmd.append(3, 0x00);
        lRet = SendReadCmd(vtCmd, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打开密文输入命令失败：%d", lRet);
            break;
        }

        Log(ThisModule, 1, "打开密文输入成功");
        return 0;
    } while (false);

    //关闭键盘和按键声音
    SetCommConfigParam(0x00);
    return lRet;
}

long CDevPIN_ZTC90::ReadKeyPress(EPP_KEYVAL &stKeyVal, DWORD dwTimeOut)
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

long CDevPIN_ZTC90::CancelInput(bool bPinInput)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    LOGDEVACTION();
    long lRet = SetCommConfigParam(0x00);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "取消[%s]输入命令失败：%d", bPinInput ? "密文" : "明文", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "取消[%s]输入成功", bPinInput ? "密文" : "明文");
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::GetPinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    long lRet = ERR_SUCCESS;
    UINT uLen = strlen(lpCustomerData);
    if ((uFormat != PIN_ISO9564_1 && uLen != 12) ||
        (uFormat == PIN_ISO9564_1 && uLen != 10))
    {
        Log(ThisModule, __LINE__, "无效长度用户数据：%d!=12", uLen);
        return ERR_PARAM;
    }

    //设置参数
    //激活密钥
    lRet = ActiveKey(uKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //1.设置算法参数
    BYTE byItemOptId = 0x00;
    switch (uFormat)
    {
    case PIN_ASCII:
        byItemOptId = 0x00;
        break;
    case PIN_ISO9564_0:
        byItemOptId = 0x10;
        break;
    case PIN_ISO9564_1:
        byItemOptId = 0x11;
        break;
    case PIN_ISO9564_2:
        byItemOptId = 0x12;
        break;
    case PIN_ISO9564_3:
        byItemOptId = 0x13;
        break;
    case PIN_IBM3624:
        byItemOptId = 0x20;
        break;
    case PIN_VISA:
        byItemOptId = 0x23;
        break;
    case PIN_VISA3:
        byItemOptId = 0x24;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持PinBlock模式：%02X", uFormat);
        return ERR_NOT_SUPPORT;
    }
    lRet = SetAlgoParam(0x04, byItemOptId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }   

    //设置卡号或传输码
    lRet = SetPinBlockData(lpCustomerData);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    QByteArray vtCmd;
    QByteArray vtResult;

    vtCmd.push_back(0x42);

    lRet = SendReadCmd(vtCmd, vtResult);
    if(lRet != 0){
        if(lRet == -0x15){
            lRet = ERR_NO_PIN;
        }
        Log(ThisModule, __LINE__, "读取密码块失败：%d[nKeyNo=%02X,uFormat=%02X]", lRet, uKeyNo, uFormat);
        return lRet;
    }

    //HEX to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), 8);
    strcpy(lpPinBlock, autoHex.GetHex());
/*
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
 */
    Log(ThisModule, 0, "读取密码块成功");
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::DataCrypt(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, QByteArray &vtResultData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpCryptData == nullptr || lpIVData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    if(strlen(lpCryptData) % 16 != 0){
        Log(ThisModule, __LINE__, "加密数据长度非法，非８的整数倍");
        return ERR_PARAM;
    }

    long lRet = ERR_SUCCESS;
    BYTE byCmdCode = 0x36;
    //激活密钥
    lRet = ActiveKey(uKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设定参数
    //1.DES/TDES模式
    bool bTDESKey = IsTDESKey(uKeyNo);
    lRet = SetKCVDesMode(bTDESKey);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //2.设置加解密算法
    BYTE byItemOptId = 0x10;
    switch (uMode)
    {
    case ECB_EN:
    case ECB_DE:        
        byItemOptId = 0x10;
        byCmdCode = (uMode == ECB_EN) ? 0x36 : 0x37;
        break;
    case CBC_EN:
    case CBC_DE:
        byItemOptId = 0x11;
        byCmdCode = (uMode == CBC_EN) ? 0x36 : 0x37;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持Crypt模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }
    lRet = SetAlgoParam(0x07, byItemOptId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置初始向量
    if((strlen(lpIVData) > 0) && (uMode == CBC_EN || uMode == CBC_DE)){
        QByteArray vtIVDataBcd;
        CAutoHex::Hex2Bcd(lpIVData, vtIVDataBcd);
        lRet = SetIVData(vtIVDataBcd);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }
    }

    QByteArray vtCmd;
    QByteArray vtResult;
    QByteArray vtCryptDataBcd;

    CAutoHex::Hex2Bcd(lpCryptData, vtCryptDataBcd);
    vtCmd.push_back(byCmdCode);
    vtCmd.push_back(vtCryptDataBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "DES密钥加解密失败：%d[uMode=%d]", lRet, uMode);
        return lRet;
    }

    //HEX to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    vtResultData.append(autoHex.GetHex());

/*
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
*/
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::DataMAC(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, QByteArray &vtResultData, BYTE byPad/* = 0xFF*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpMacData == nullptr || lpIVData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    long lRet = ERR_SUCCESS;
    //激活密钥
    lRet = ActiveKey(uKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置参数    
    //1.设置DES/TDES模式
    lRet = SetAlgoParam(0x01, IsTDESKey(uKeyNo) ? 0x30 : 0x20);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }
    //2.设置mac算法
    BYTE byItemOptId = 0x01;
    switch (uMode)
    {
    case MAC_ANSIX99:
    case MAC_ANSIX919:
        byItemOptId = 0x01;
        break;
    case MAC_PBOC:
        byItemOptId = 0x02;
        break;
    case MAC_ISO16609:
    case MAC_CBC:
        byItemOptId = 0x04;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持MAC模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }
    lRet = SetAlgoParam(0x06, byItemOptId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置初始向量
    if(strlen(lpIVData) > 0){
        QByteArray vtIVDataBcd;
        CAutoHex::Hex2Bcd(lpIVData, vtIVDataBcd);
        lRet = SetIVData(vtIVDataBcd);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }
    }

    QByteArray vtCmd;
    QByteArray vtResult;
    QByteArray vtMacDataBcd;

    CAutoHex::Hex2Bcd(lpMacData, vtMacDataBcd);
    vtCmd.push_back(0x41);
    vtCmd.append(vtMacDataBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "计算Mac加密数据失败:%d[%d]", lRet, uMode);
        return lRet;
    }

    //HEX to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    vtResultData.append(autoHex.GetHex());
/*
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
*/
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::RandData(LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x33");
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取16位长度的随机数失败：%d", lRet);
        return lRet;
    }   

    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    strcpy(lpData, autoHex.GetHex());
    return ERR_SUCCESS;
}

/*
 *@function:读取对称密钥KCV
 *@note:只实现了DES密钥
 */
long CDevPIN_ZTC90::ReadSymmKeyKCV(UINT uKeyNo, BYTE byKcvMode, LPSTR lpKcv, BYTE bySymmKeyType/* = 0*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x65");

    vtCmd.push_back((char)uKeyNo);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取对称密钥校验值失败：%d，KeyNo：%d", lRet, uKeyNo);
        return lRet;
    }

    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    strcpy(lpKcv, autoHex.GetHex());
    return ERR_SUCCESS;
}

//30-00-00-00(FS#0003)
long CDevPIN_ZTC90::SM2ExportPKeyForGD(UINT uKeyNo, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    long lRet = SM2ExportPKey(uKeyNo, 0xFF, nullptr, lpData, nullptr);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    return ERR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
long CDevPIN_ZTC90::SMGetFirmware(LPSTR lpSMVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    return ERR_SUCCESS;
}


long CDevPIN_ZTC90::SM4ImportMKey(UINT uMKeyNo, LPCSTR lpKeyVal, LPSTR lpKCV, UINT uEncKeyNo/* = 0xFF*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if(lpKeyVal == nullptr){
        return ERR_PARAM;
    }

    QByteArray vtKeyValBcd;
    CAutoHex::Hex2Bcd(lpKeyVal, vtKeyValBcd);
    if (vtKeyValBcd.size() != 16)
    {
        Log(ThisModule, __LINE__, "无效Key长度：uValLen=%d", vtKeyValBcd.size());
        return ERR_PARAM;
    }

    if (uMKeyNo > 0x1F)
    {
        Log(ThisModule, __LINE__, "无效保存主密钥ID：uMKeyNo=%02X", uMKeyNo);
        return ERR_PARAM;
    }

    LOGDEVACTION();
    long lRet = ERR_SUCCESS;
    QByteArray vtResult;
    QByteArray vtCmd;
    if(uEncKeyNo != 0xFF){
        //密文下载主密钥
        uMKeyNo += 0x40;

        //激活密钥
        lRet = ActiveSMKey(uEncKeyNo);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }

        //参数设设置
        lRet = SetSMAlgoParam(0x00, 0x30);
        if(lRet != ERR_SUCCESS){
            return ERR_SUCCESS;
        }
    } else {
        //明文下载主密钥
    }

    vtCmd.push_back(0x82);
    vtCmd.push_back((char)uMKeyNo);
    vtCmd.append(vtKeyValBcd);
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM主密钥导入失败：lRet=%d[uMKeyNo=%02X]", lRet, uMKeyNo);
        return lRet;
    }

    if(lpKCV != nullptr){
        //Hex to Asi
        CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
        strcpy(lpKCV, autoHex.GetHex());
    }
    Log(ThisModule, 1, "SM主密钥导入成功:uMKeyNo=%02X", uMKeyNo);
    return ERR_SUCCESS;
}

//30-00-00-00(FS#0003)
/*
 *@param
 *@note    暂时不支持验签，忽略验签参数
 */
long CDevPIN_ZTC90::SM4ImportMKeyForGD(UINT uMKeyNo, UINT uDKeyNo, UINT uKeyUse, LPCSTR lpKeyVal,
                                       LPSTR lpKCV, UINT uVerifSignKeyNo/* = 0xFF*/, LPSTR lpSignData/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if(lpKeyVal == nullptr || strlen(lpKeyVal) == 0){
        return ERR_PARAM;
    }

    LOGDEVACTION();
    long lRet = ERR_SUCCESS;
    QByteArray vtCmd;
    QByteArray vtResult;
    QByteArray vtKeyValBcd;
    QByteArray vtZABcd;
    QByteArray vtSignDataBcd;

    char cKeyValHex[32] = {0};
    if(true){
        //使用私钥数据解密
        lRet = SM2DecryptData(uDKeyNo, lpKeyVal, cKeyValHex);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }

        //合成SM4密钥
        UINT uEncKeyNo = 0;             //无加密密钥时，默认使用全0密钥为主密钥
        BYTE byLoadKeyValBCD[24] = {0}; //全0密钥加密后的密钥数据
        BYTE byAllZeroKey[16] = {0};    //全0密钥
        QByteArray vtKeyValBcd;         //原始密钥数据
        CAutoHex::Hex2Bcd(cKeyValHex, vtKeyValBcd);
        m_encrypt.sm4_enc_ecb((LPBYTE)vtKeyValBcd.constData(), 16, byLoadKeyValBCD, 16, byAllZeroKey);

        CAutoHex autoHex(byLoadKeyValBCD, 16);
        lRet = SM4ImportMKey(uMKeyNo, autoHex.GetHex(), lpKCV, uEncKeyNo);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }
    } else {
        //必须有验签数据
        if(uVerifSignKeyNo == 0xFF || lpSignData == nullptr || strlen(lpSignData) == 0){
            return ERR_PARAM;
        }
        //使用私钥对数据进行签名
        //Todo:计算ZA值,需要有user data

        CAutoHex::Hex2Bcd(lpKeyVal, vtKeyValBcd);
        CAutoHex::Hex2Bcd(lpSignData, vtSignDataBcd);

        vtCmd.push_back(0x9B);
        vtCmd.push_back((char)uMKeyNo);
        vtCmd.push_back((char)uDKeyNo);
        vtCmd.push_back((char)uVerifSignKeyNo);
        vtCmd.push_back(vtZABcd);
        vtCmd.push_back(vtKeyValBcd);
        vtCmd.push_back(vtSignDataBcd);
        lRet = SendReadCmd(vtCmd, vtResult);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "导入SM2加密的SM4密钥失败：%d[uMKeyNo=%d,uDKeyNo=%d,uVerifSignKeyNo=%d]",
                lRet, uMKeyNo, uDKeyNo, uVerifSignKeyNo);
            return lRet;
        }

        if(lpKCV != nullptr){
            CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
            strcpy(lpKCV, autoHex.GetHex());
        }
    }

    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::SM4ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode/* = 2*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if(lpKeyVal == nullptr){
        return ERR_PARAM;
    }

    QByteArray vtKeyValBcd;
    CAutoHex::Hex2Bcd(lpKeyVal, vtKeyValBcd);
    if (vtKeyValBcd.size() != 16 || uKeyUse == 0)
    {
        Log(ThisModule, __LINE__, "无效Key用途,长度：KeyUse=%d, ValLen=%d", uKeyUse, vtKeyValBcd.size());
        return ERR_PARAM;
    }

    if(uKeyUse == PKU_KEYENCKEY && (uKeyNo > 0x1F ))
    {
        Log(ThisModule, __LINE__, "无效保存主密钥ID：uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
        return ERR_PARAM;
    }
    else if(uKeyUse != PKU_KEYENCKEY && (uKeyNo < 0x40 || uKeyNo > 0xBF))
    {
        Log(ThisModule, __LINE__, "无效保存工作密钥ID：uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
        return ERR_PARAM;
    }

    long lRet = ERR_SUCCESS;
    QByteArray vtResult;
    QByteArray vtCmd;

    QByteArray vtLoadKeyValBcd = vtKeyValBcd;
    if(uMKeyNo == 0xFF){
        uMKeyNo = 0;                    //无加密密钥时，默认使用全0密钥为主密钥
        BYTE byLoadKeyValBCD[24] = {0};
        BYTE byAllZeroKey[16] = {0};
        m_encrypt.sm4_enc_ecb((LPBYTE)vtKeyValBcd.constData(), 16, byLoadKeyValBCD, 16, byAllZeroKey);

        vtLoadKeyValBcd.clear();
        vtLoadKeyValBcd.append((LPCSTR)byLoadKeyValBCD, 16);
    }

    if(uKeyUse == PKU_KEYENCKEY){
        QByteArray vtLoadKeyValHex;
        CAutoHex::Bcd2Hex((LPBYTE)vtLoadKeyValBcd.constData(), vtLoadKeyValBcd.size(), vtLoadKeyValHex);
        lRet = SM4ImportMKey(uKeyNo, vtLoadKeyValHex.constData(), lpKCV, uMKeyNo);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }
    } else {
        //激活主密钥
        lRet = ActiveSMKey(uMKeyNo);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }

        //参数设设置
        lRet = SetSMAlgoParam(0x00, 0x30);
        if(lRet != ERR_SUCCESS){
            return ERR_SUCCESS;
        }

        vtCmd.push_back(0x83);
        vtCmd.push_back((char)uMKeyNo);
        vtCmd.push_back((char)uKeyNo);
        vtCmd.push_back(vtLoadKeyValBcd);

        lRet = SendReadCmd(vtCmd, vtResult);
        if(lRet != ERR_SUCCESS){

        }

        //Hex to Asi
        if(lpKCV != nullptr){
            CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
            strcpy(lpKCV, autoHex.GetHex());
        }
    }

/*
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
*/
    Log(ThisModule, 1, "SM工作密钥导入成功:uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
    return ERR_SUCCESS;
}

//30-00-00-00(FS#0005)
long CDevPIN_ZTC90::SM4RemoteImportKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostSM2PKeyNo, UINT uKeyUse, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal, LPSTR lpKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.resize(7);
    sprintf(vtCmd.data(), "\x70%02X%02X%02X", uKeyNo, uEppSKeyNo, uHostSM2PKeyNo);
    if(uHostSM2PKeyNo != 0xFF && lpZA != nullptr){
        vtCmd.append(lpZA);
    } else {
        vtCmd.append(64, '0');
    }
    vtCmd.append(lpKeyVal);
    if(uHostSM2PKeyNo != 0xFF && lpSignKeyVal != nullptr){
        vtCmd.append(lpSignKeyVal);
    } else {
        vtCmd.append(128, '0');
    }
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "远程导入SM4密钥失败:%d[uKeyNo=%02x]", lRet, uKeyNo);
        return lRet;
    }

    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    strcpy(lpKCV, autoHex.GetHex());
    return ERR_SUCCESS;
*/
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::SM4CryptData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, QByteArray &vtResultData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpCryptData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    QByteArray vtCryptDataBcd;
    CAutoHex::Hex2Bcd(lpCryptData, vtCryptDataBcd);
    if(vtCryptDataBcd.size() % 16 != 0 || vtCryptDataBcd.size() > 2048){
        Log(ThisModule, __LINE__, "加密数据长度非法(非16倍数或超过2048字节):%d", vtCryptDataBcd.size());
        return ERR_PARAM;
    }

    long lRet = ERR_SUCCESS;
    BYTE byCmdCode = 0x84;

    //激活密钥
    lRet = ActiveSMKey(uKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置参数
    BYTE byItemOptId = 0x10;
    switch (uMode)
    {
    case ECB_EN:
    case ECB_DE:
        byCmdCode = (uMode == ECB_EN) ? 0x84 : 0x85;
        break;
    case CBC_EN:
    case CBC_DE:
        byItemOptId = 0x20;
        byCmdCode = (uMode == CBC_EN) ? 0x84 : 0x85;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持Crypt模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    lRet = SetSMAlgoParam(0x01, byItemOptId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置初始向量
    if(uMode == CBC_EN || uMode == CBC_DE){
        if(lpIVData != nullptr && strlen(lpIVData) > 0){
            QByteArray vtIVDataBcd;
            CAutoHex::Hex2Bcd(lpIVData, vtIVDataBcd);
            lRet = SetIVData(vtIVDataBcd, false);
            if(lRet != ERR_SUCCESS){
                return lRet;
            }
        }
    }

    QByteArray vtCmd;
    QByteArray vtResult;

    vtCmd.push_back(byCmdCode);
    vtCmd.push_back(vtCryptDataBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "SM4加解密失败：%d[KeyNo=%d，Mode=%d]", lRet, uKeyNo, uMode);
        return lRet;
    }

    //Hex to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    vtResultData.append(autoHex.GetHex());
/*
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
*/
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::SM4MACData(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, QByteArray &vtResultData, BYTE byPad/* = 0xFF*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpMacData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    QByteArray vtMacDataBcd;
    CAutoHex::Hex2Bcd(lpMacData, vtMacDataBcd);
    if(vtMacDataBcd.size() > 2048){
        Log(ThisModule, __LINE__, "SM4 Mac加密数据长度错误，超过2048字节:%d", vtMacDataBcd.size());
        return ERR_PARAM;
    }

    //激活密钥
    long lRet = ERR_PARAM;
    lRet = ActiveSMKey(uKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置参数
    BYTE byItemOptId = 0x02;
    switch (uMode)
    {
    case MAC_PBOC:
        byItemOptId = 0x02;
        break;
    case MAC_BANKSYS:
         byItemOptId = 0x03;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持MAC模式：uMode=%d", uMode);
        return ERR_NOT_SUPPORT;
    }
    lRet = SetAlgoParam(0x06, byItemOptId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置初始化向量
    if(lpIVData != nullptr && strlen(lpIVData) > 0){
        QByteArray vtIVDataBcd;
        CAutoHex::Hex2Bcd(lpIVData, vtIVDataBcd);
        lRet = SetIVData(vtIVDataBcd, false);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }
    }

    QByteArray vtCmd;
    QByteArray vtResult;

    vtCmd.push_back(0x88);
    vtCmd.append(vtMacDataBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //Hex to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    vtResultData.append(autoHex.GetHex());
    return ERR_SUCCESS;
/*
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
*/
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::SM4PinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    int iLen = strlen(lpCustomerData);
    if (iLen != 12)
    {
        Log(ThisModule, __LINE__, "无效长度用户数据：%d!=12", iLen);
        return ERR_PARAM;
    }

    long lRet = ERR_SUCCESS;
    //激活密钥
    lRet = ActiveSMKey(uKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置参数
    BYTE byItemOptId = 0x31;
    switch (uFormat)
    {
    case PIN_SM4:
        byItemOptId = 0x31;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持PinBlock模式：%02X", uFormat);
        return ERR_NOT_SUPPORT;
    }
    lRet = SetAlgoParam(0x05, byItemOptId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置卡号或传输码
    lRet = SetPinBlockData(lpCustomerData);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    QByteArray vtCmd;
    QByteArray vtResult;

    vtCmd.push_back(0x87);
    lRet = SendReadCmd(vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "读取密码块失败：%d[KeyNo=%02X,Format=%02X]", lRet, uKeyNo, uFormat);
        return lRet;
    }

    //Hex to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    strcpy(lpPinBlock, autoHex.GetHex());
/*
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
*/
    Log(ThisModule, 1, "读取国密密码块成功");
    return ERR_SUCCESS;
}

//30-00-00-00(FS#0005)
long CDevPIN_ZTC90::SM2ImportKey(UINT uKeyNo, UINT uVendorPKeyNo, BOOL bPublicKey, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    QByteArray vtKeyValBcd;
    CAutoHex::Hex2Bcd(lpKeyVal, vtKeyValBcd);
    if(!bPublicKey){
        return ERR_PARAM;
    }
    if(vtKeyValBcd.size() != 64){
        return ERR_PARAM;
    }

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;
    BYTE byMode = 0x00;         //0x00：无签名 0x01:自签名，导入公钥对应的私钥签名，用公钥验签 0x02:厂商公钥验签

    vtCmd.push_back(0x98);
    vtCmd.push_back(byMode);
    vtCmd.push_back((char)uKeyNo);
    vtCmd.push_back(vtKeyValBcd);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导入主机SM2公钥失败：%d[nKeyNo=%02X]", lRet, uKeyNo);
        return lRet;
    }

    return ERR_SUCCESS;
}

//30-00-00-00(FS#0005)
long CDevPIN_ZTC90::SM2EncryptData(UINT uPKeyNo, LPCSTR lpData, LPSTR lpCryptData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if(lpCryptData == nullptr || lpData == nullptr){
        return ERR_PARAM;
    }

    long lRet = ERR_SUCCESS;
    //激活密钥
    lRet = ActiveSM2Key(uPKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;
    QByteArray vtDataBcd;
    CAutoHex::Hex2Bcd(lpData, vtDataBcd);

    vtCmd.push_back(0x8E);
    vtCmd.append(vtDataBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM2公钥加密失败：%d[nKeyNo=%02X]", lRet, uPKeyNo);
        return lRet;
    }

    //Hex to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    strcpy(lpCryptData, autoHex.GetHex());
    return ERR_SUCCESS;
}

//30-00-00-00(FS#0005)
long CDevPIN_ZTC90::SM2DecryptData(UINT uSKeyNo, LPCSTR lpCryptData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if(lpCryptData == nullptr || lpData == nullptr){
        return ERR_PARAM;
    }

    //激活密钥
    long lRet = ActiveSM2Key(uSKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;
    QByteArray vtCryptDataBcd;
    CAutoHex::Hex2Bcd(lpCryptData, vtCryptDataBcd);

    vtCmd.push_back(0x8F);
    vtCmd.append(vtCryptDataBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM2私钥解密失败：%d[nKeyNo=%02X]", lRet, uSKeyNo);
        return lRet;
    }

    //Hex to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    strcpy(lpData, autoHex.GetHex());

    return ERR_SUCCESS;
}

//30-00-00-00(FS#0005)
long CDevPIN_ZTC90::SM2SignData(UINT uSKeyNo, LPCSTR lpZA, LPCSTR lpData, LPSTR lpSignData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if(lpZA == nullptr || lpData == nullptr){
        return ERR_PARAM;
    }

    //激活密钥
    long lRet = ActiveSM2Key(uSKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;
    QByteArray vtZABcd;
    QByteArray vtDataBcd;
    CAutoHex::Hex2Bcd(lpZA, vtZABcd);
    CAutoHex::Hex2Bcd(lpData, vtDataBcd);

    vtCmd.push_back(0x90);
    vtCmd.append(vtZABcd);
    vtCmd.append(vtDataBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM2签名失败：%d[nKeyNo=%02X]", lRet, uSKeyNo);
        return lRet;
    }

    //Hex to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    strcpy(lpSignData, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::SM2VerifySign(UINT uPKeyNo, LPCSTR lpZA, LPCSTR lpData, LPCSTR lpSignData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    //激活密钥
    long lRet = ActiveSM2Key(uPKeyNo);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    LOGDEVACTION();

    QByteArray vtCmd;
    QByteArray vtResult;
    QByteArray vtZABcd;
    QByteArray vtDataBcd;
    QByteArray vtSignDataBcd;
    CAutoHex::Hex2Bcd(lpZA, vtZABcd);
    CAutoHex::Hex2Bcd(lpData, vtDataBcd);
    CAutoHex::Hex2Bcd(lpSignData, vtSignDataBcd);

    vtCmd.push_back(0x91);
    vtCmd.append(vtZABcd);
    vtCmd.append(vtDataBcd);
    vtCmd.append(vtSignDataBcd);

    lRet = SendReadCmd(vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "SM2验签失败:PKeyNo=%d", uPKeyNo);
        return lRet;
    }

    return ERR_SUCCESS;
}

//30-00-00-00(FS#0005)
long CDevPIN_ZTC90::SM2ExportPKey(UINT uKeyNo, UINT uSignKeyNo, LPSTR lpZA, LPSTR lpSM2PKeyData, LPSTR lpSignData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if(lpSM2PKeyData == nullptr){
        return ERR_PARAM;
    }

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.push_back(0x8D);
    vtCmd.push_back((char)uKeyNo);
    vtCmd.push_back(uSignKeyNo);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导出EPP SM2公钥失败：%d[uKeyNo=%02x]", lRet, uKeyNo);
        return lRet;
    }

    //Hex to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), 64);
    strcpy(lpSM2PKeyData, autoHex.GetHex());
    if(uSignKeyNo != 0xFF){
        autoHex.Bcd2Hex((LPBYTE)vtResult.constData() + 64, 64, lpSignData);
    }
    return ERR_SUCCESS;
}

//30-00-00-00(FS#0005)
/*@func 生成SM2密钥对
 *@param uSKeyNo:私钥密钥索引 uPKeyNo:公钥密钥索引
 */
long CDevPIN_ZTC90::SM2GenerateKey(UINT uSKeyNo, UINT uPKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.push_back(0x8C);
    vtCmd.push_back((char)uSKeyNo);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "生成SM2密钥对失败：%d[uSKeyNo=%02x, uPKeyNo=%02x]", lRet, uSKeyNo, uPKeyNo);
        return lRet;
    }

/* FOR TEST:OUTPUT SM2 PKEY ENCRYPT DATA
    char cData[33] = "30313233343536373839313233343536";
    BYTE cEncryData[1000] = {0};
    SM2EncryptData(0x06, cData, (LPSTR)cEncryData);
    QByteArray temp;
    CAutoHex::Hex2Bcd((LPCSTR)cEncryData, temp);
    char szTemp[1024] = {0};
    for(int i = 0; i < temp.size(); i++){
        sprintf(szTemp + i * 3, "%02X ", (int)temp[i]);
    }
    Log(ThisModule, __LINE__, "加密数据：%s", szTemp);

    BYTE cDecryptData[1000] = {0};
    SM2DecryptData(0x05, (LPCSTR)cEncryData, (LPSTR)cDecryptData);
*/
    return ERR_SUCCESS;
}

//30-00-00-00(FS#0005)
long CDevPIN_ZTC90::SM3CaculateZAData(LPCSTR lpUserData, UINT uPKNum, QByteArray &resultData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if(lpUserData == nullptr){
        return ERR_PARAM;
    }

    LOGDEVACTION();
    long lRet = ERR_SUCCESS;
    QByteArray vtUserDataBcd;
    QByteArray vtPKeyDataBcd;
    CAutoHex::Hex2Bcd(lpUserData, vtUserDataBcd);
    //获取公钥数据
    BYTE byPKeyData[129] = {0};
    lRet = SM2ExportPKey(uPKNum, 0xFF, nullptr, (LPSTR)byPKeyData, nullptr);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }
    CAutoHex::Hex2Bcd((LPCSTR)byPKeyData, vtPKeyDataBcd);

    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.push_back(0x9A);
    vtCmd.push_back(vtUserDataBcd);
    vtCmd.push_back(vtPKeyDataBcd);
    lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "计算ZA值失败:%d[uPKNum=%02x]", lRet, uPKNum);
        return lRet;
    }

    //Hex to Asi
    CAutoHex autoHex((LPBYTE)vtResult.constData(), vtResult.size());
    resultData.append(autoHex.GetHex());
    return ERR_SUCCESS;
}

/*
 *@note  不支持所有密钥的删除,只支持指定密钥的删除
 *
 */
long CDevPIN_ZTC90::SMDeleteKey(UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if(uKeyNo == 0){
        return ERR_SUCCESS;
    }

    QByteArray vtResult;
    QByteArray vtCmd("\x89");

    vtCmd.push_back((char)uKeyNo);

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "删除国密密钥命令失败：%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }   

    Log(ThisModule, 1, "删除国密密钥成功：uKeyNo=%d", uKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::SM4SaveKeyType(UINT uType)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();
/*
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
 */
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::RSAInitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();
/*
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
*/
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::RSAImportRootHostPK(QByteArray strHostPK, QByteArray strSignHostPK)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
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
*/
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::RSAImportRootHostPK(UINT uPKeyNo, UINT uSignKeyNo, int iSignAlgo, QByteArray strHostPKData, QByteArray strSignData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
    LOGDEVACTION();

    CAutoHex autoHex;
    QByteArray vtKeyData;
    QByteArray vtSigData;
    autoHex.Hex2Bcd(strHostPKData.constData(), vtKeyData);
    autoHex.Hex2Bcd(strSignData.constData(), vtSigData);

    QByteArray vtResult;
    QByteArray vtCmd;

    BOOL bPublicKeyValueType = FALSE;
    BYTE byBuffer[1024] = {0};
    LPBYTE lpBuffer = byBuffer;
    WORD wModTagLen = 4;                        //Modulus Tag Len
    BYTE byModTag[4];                           //Modulus Tag
//    WORD wModLen = vtKeyData.size() + 1;        //Modulus Len (With 0x00)
    WORD wModLen = vtKeyData.size();

    WORD wExpTagLen = 4;                        //Exponent Tag Len
    BYTE byExpTag[4];                           //Exponent Tag
    char szExpData[4] = "\x01\x00\x01";         //Exponent Data
    WORD wExpDataLen = 3;                       //Exponent Data	Len

    WORD wSigTagLen = 4;                        //Signature Tag Len
    BYTE bySigTag[4];                           //Signature Tag

    WORD wKeyTagLen = 4;                        //Key Tag Len
    BYTE byKeyTag[4];                           //Key Tag

    WORD wTotalKeyTagLen = 4;                   //Key Tag Len
    BYTE byTotalKeyTag[4];                      //Total Key Tag
    //Signature length no more than 256
    int iSigDataLen = vtSigData.size() > 256 ? 256 : vtSigData.size();
    int iKeyDataLen = vtKeyData.size();
    //Set Command Buffer
    if (uSignKeyNo == 0xFF) {
        lpBuffer[0] = 0x45;
        lpBuffer += 1;
    } else {
        lpBuffer[0] = 0x81;
        lpBuffer += 1;
        *lpBuffer = (BYTE)uPKeyNo;
        lpBuffer += 1;
        *lpBuffer = (BYTE)uSignKeyNo;
        lpBuffer += 1;
    }

    memset( lpBuffer, 0x31, 1 );
    lpBuffer += 1;
    //Create Tag
    if(bPublicKeyValueType == FALSE){
        BuildAsn1Tag( ASN1_INTEGER, wModLen, &wModTagLen, byModTag );
        BuildAsn1Tag( ASN1_INTEGER, wExpDataLen, &wExpTagLen, byExpTag );
        BuildAsn1Tag( ASN1_INTEGER, iSigDataLen, &wSigTagLen, bySigTag );
        BuildAsn1Tag( ASN1_SEQUENCE, wModTagLen + wModLen +
                        wExpTagLen + wExpDataLen,
                        &wKeyTagLen, byKeyTag );
        BuildAsn1Tag( ASN1_SEQUENCE, wKeyTagLen +
                        wModTagLen + wModLen +
                        wExpTagLen + wExpDataLen +
                        wSigTagLen + iSigDataLen,
                        &wTotalKeyTagLen, byTotalKeyTag );
    }else{
        BuildAsn1Tag( ASN1_INTEGER, iSigDataLen, &wSigTagLen, bySigTag );
        BuildAsn1Tag( ASN1_SEQUENCE, iKeyDataLen +
                        wSigTagLen + iSigDataLen,
                        &wTotalKeyTagLen, byTotalKeyTag );
    }
    memcpy( lpBuffer, byTotalKeyTag, wTotalKeyTagLen );
    lpBuffer += wTotalKeyTagLen;

    if(bPublicKeyValueType == FALSE){
        memcpy( lpBuffer, byKeyTag, wKeyTagLen );
        lpBuffer += wKeyTagLen;

        memcpy( lpBuffer, byModTag, wModTagLen );
        lpBuffer += wModTagLen;

        memset( lpBuffer, NULL, 1 );
        lpBuffer += 1;
        memcpy( lpBuffer, vtKeyData.constData(), iKeyDataLen );
        lpBuffer += iKeyDataLen;

        memcpy( lpBuffer, byExpTag, wExpTagLen );
        lpBuffer += wExpTagLen;

        memcpy( lpBuffer, szExpData, wExpDataLen );
        lpBuffer += wExpDataLen;
    }else{
        memcpy( lpBuffer, vtKeyData.constData(), iKeyDataLen);
        lpBuffer += iKeyDataLen;
    }

    memcpy( lpBuffer, bySigTag, wSigTagLen );
    lpBuffer += wSigTagLen;

    memcpy( lpBuffer, vtSigData, iSigDataLen );
    lpBuffer += iSigDataLen;

    vtCmd.append((LPSTR)byBuffer, lpBuffer - byBuffer);
    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载主机RSA公钥命令失败：lRet=%d", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "下载主机RSA公钥命令成功");

    return ERR_SUCCESS;
*/
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::RSAImportSignHostPK(QByteArray strHostPK, QByteArray strSignHostPK)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
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
*/
    return ERR_SUCCESS;
}

long CDevPIN_ZTC90::RSAExportEppEncryptPK(QByteArray &strPK, QByteArray &strSignPK)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
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
    Log(ThisModule, 1, "加密公钥数据[ %s ]签名", (vtResult.at(0) == 0x31) ? "已" : "未");
    if (vtResult.at(0) != 0x31)
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
*/
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::RSAExportEppEncryptPK(UINT uEppPKeyNo, UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo,
                                          QByteArray &strKeyVal, QByteArray &strSignVal)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
    vtCmd.push_back("\x82");
    vtCmd.push_back(uEppPKeyNo);
    vtCmd.push_back(uEppSKeyNo);
    vtCmd.push_back(0x31);
    vtCmd.push_back(0x30);

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导出RSA加密公钥信息失败：lRet=%d", lRet);
        return lRet;
    }

    //返回数据解析
    BYTE byKeyData[1024] = {0};
    WORD wKeyDataLen = sizeof(byKeyData);
    BYTE byRSAKeyData[512] = {0};
    WORD wRSAKeyDataLen = sizeof(byRSAKeyData);
    BYTE byModulus[257] = {0};
    WORD wModulusLen = sizeof(byModulus);
    BYTE bySignature[256] = {0};
    WORD wSignatureLen = sizeof(bySignature);
    WORD wKeyTagLen, wRSAKeyTagLen, wModulusTagLen, wSignatureTagLen = 0;
    LPBYTE lpResultData = (LPBYTE)(vtResult.constData() + 1);
    lRet = ExtractAsn1Data(vtResult.size() - 1, lpResultData, ASN1_SEQUENCE,
            &wKeyTagLen, byKeyData, &wKeyDataLen);
    if (lRet == 0) {
        //Get Kkey data by ANS1 Algorithm (Without Tag)
        lRet = ExtractAsn1Data(wKeyDataLen, byKeyData, ASN1_SEQUENCE,
                                    &wRSAKeyTagLen, byRSAKeyData, &wRSAKeyDataLen);
        if (lRet == 0) {
            //Get RSA Key Data by ANS1 Algorithm (Without Tag)
            lRet = ExtractAsn1Data( wRSAKeyDataLen, byRSAKeyData, ASN1_INTEGER,
                                         &wModulusTagLen, byModulus, &wModulusLen);
            if (lRet == 0) {
                //Get Signature by ANS1 Algorithm (Without Tag)
                lRet = ExtractAsn1Data(wKeyDataLen - wRSAKeyTagLen - wRSAKeyDataLen,
                                            byKeyData + wRSAKeyTagLen + wRSAKeyDataLen,	ASN1_INTEGER,
                                            &wSignatureTagLen, bySignature, &wSignatureLen);
            }
        }
    }

    if(lRet == 0){
        BOOL bPublicKeyValueType = TRUE;
        if (bPublicKeyValueType == FALSE) {
            CAutoHex::Bcd2Hex(&(byModulus[1]), wModulusLen - 1, strKeyVal);
        } else {
            CAutoHex::Bcd2Hex(byKeyData, wKeyDataLen - wSignatureTagLen - wSignatureLen, strKeyVal);
        }
        CAutoHex::Bcd2Hex(bySignature, wSignatureLen, strSignVal);
    } else {
        //解析失败，返回结果为空
    }

    return ERR_SUCCESS;
*/
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::RSAExportEppSN(QByteArray &strSN, QByteArray &strSignSN)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
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
    Log(ThisModule, 1, "EPP序号[ %s ]签名", (vtResult.at(0) == 0x31) ? "已" : "未");
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
*/
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::RSAExportEppSN(UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo, QByteArray &strSN, QByteArray &strSignSN)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd("\x85");
    vtCmd.push_back(uEppSKeyNo);
    vtCmd.push_back(0x31);
    vtCmd.push_back(0x30);

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导出密码键盘SN失败：lRet=%d", lRet);
        return lRet;
    }

    // 分析返回数据
    BYTE byEPPSerialNumSeq[512] = {0};
    WORD wEPPSerialNumSeqLen = sizeof(byEPPSerialNumSeq);
    BYTE byEPPSerialNum[8] = {0};
    WORD wEPPSerialNumLen = sizeof(byEPPSerialNum);
    BYTE bySignature[257] = {0};
    WORD wSignatureLen = sizeof(bySignature);
    WORD wEPPSerialNumSeqTagLen, wEPPSerialNumTagLen, wSignatureTagLen = 0;

    BYTE byAlgo = vtResult.at(0);
    eRSASignAlgo = RSA_SIGN_NO;
    if(byAlgo == 0x31){
        eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
    } else if(byAlgo == 0x32){
        eRSASignAlgo = RSA_SIGN_SSA_PSS;
    }

    LPBYTE lpData = (LPBYTE)vtResult.constData() + 1;
    lRet = ExtractAsn1Data(vtResult.size() - 1, lpData, ASN1_SEQUENCE,
            &wEPPSerialNumSeqTagLen, byEPPSerialNumSeq, &wEPPSerialNumSeqLen);
    if (lRet == 0) {
        //Get EPP Serial Number by ANS1 Algorithm (Without Tag)
        lRet = ExtractAsn1Data( wEPPSerialNumSeqLen, byEPPSerialNumSeq, ASN1_INTEGER,
                                     &wEPPSerialNumTagLen, byEPPSerialNum, &wEPPSerialNumLen );
        if (lRet == 0) {
            //Get Signature by ANS1 Algorithm (Without Tag)
            lRet = ExtractAsn1Data( wEPPSerialNumSeqLen - wEPPSerialNumTagLen - wEPPSerialNumLen,
                                         byEPPSerialNumSeq + wEPPSerialNumTagLen + wEPPSerialNumLen, ASN1_INTEGER,
                                         &wSignatureTagLen, bySignature, &wSignatureLen );
        }
    }

    if(lRet == 0){
        // 序号
        CAutoHex::Bcd2Hex(byEPPSerialNum, wEPPSerialNumLen, strSN);

        // 签名序号
        CAutoHex::Bcd2Hex(bySignature, wSignatureLen, strSignSN);
    } else {
        //解析失败，返回结果为空
    }

    return ERR_SUCCESS;
*/
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::RSAExportEppRandData(QByteArray &strRandData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
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
*/
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::RSAImportDesKey(UINT uMKeyNo, QByteArray strMKVal, QByteArray strSignMKVal, QByteArray &strKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
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
    int nKeyLen = (vtResult.at(0) == 31) ? 16 : 24;
    CAutoHex::Bcd2Hex((BYTE *)(vtResult.data() + 1), 8, strKCV);

    Log(ThisModule, 1, "下载DESKEY成功[KeyLen=%d][KCV=%s]", nKeyLen, strKCV.constData());
    return ERR_SUCCESS;
*/
    return ERR_NOT_SUPPORT;
}

//30-00-00-00(FS#0006)
long CDevPIN_ZTC90::RSAImportDesKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostPKeyNo,
                                    RSADATAALGOTYPE  eRSAEncAlgo, RSASIGNALGOTYPE eRSASignAlgo,
                                    QByteArray strKeyVal, QByteArray strSignVal, QByteArray &strKCV,
                                    bool bUseRandom/* = false*/, int iSymmKeyType/* = 0*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.push_back("\x86");                //命令码
    if(uEppSKeyNo == 0xFF){
        vtCmd.push_back(0x02);
    } else {
        vtCmd.push_back(uEppSKeyNo);            //解密密钥ID
    }

    vtCmd.push_back(uHostPKeyNo);           //验签密钥ID
    vtCmd.push_back(uKeyNo);                //下载DES密钥ID
    vtCmd.push_back(0x30);                  //结合模式

    BYTE byAlgo;
    //RSA加密算法
    switch(eRSAEncAlgo){
    case RSA_ENC_PKCS1_V1_5:
        byAlgo = 0x30;
        break;
    case RSA_ENC_SHA_1:
        byAlgo = 0x31;
        break;
    case RSA_ENC_SHA_256:
        byAlgo = 0x32;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持的RSA加密算法：%d", eRSAEncAlgo);
        return ERR_PARAM;
        break;
    }
    vtCmd.push_back(byAlgo);

    //RSA签名算法
    switch(eRSASignAlgo){
    case RSA_SIGN_NO:
        byAlgo = 0x30;
        break;
    case RSA_SIGN_SSA_PKCS1_V1_5:
        byAlgo = 0x31;
        break;
    case RSA_SIGN_SSA_PSS:
        byAlgo = 0x32;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持的RSA签名算法：%d", eRSASignAlgo);
        return ERR_PARAM;
        break;
    }
    vtCmd.push_back(byAlgo);

    vtCmd.push_back(0x30);          //是否需要随机数 0x30:不需要 0x31:需要

    //Create Tag
    WORD wKeyTagLen = 4;
    BYTE byKeyTag[4] = {0};
    WORD wSigTagLen = 4;
    BYTE bySigTag[4] = {0};
    WORD wDESKeyTagLen = 4;
    BYTE byDESKeyTag[4] = {0};
    WORD wDummySigLen = 3;
    BYTE byDummySig[4] = {0x30, 0x30, 0x30, 0x00};
    QByteArray vtKeyVal;
    QByteArray vtSignVal;
    CAutoHex::Hex2Bcd(strKeyVal, vtKeyVal);
    CAutoHex::Hex2Bcd(strSignVal, vtSignVal);

    BuildAsn1Tag( ASN1_INTEGER, vtKeyVal.size(), &wDESKeyTagLen, byDESKeyTag);
    if (vtSignVal.size() != 0) {
        BuildAsn1Tag( ASN1_INTEGER, vtSignVal.size(), &wSigTagLen, bySigTag);
        BuildAsn1Tag( ASN1_SEQUENCE, wDESKeyTagLen + vtKeyVal.size() +
                        wSigTagLen + vtSignVal.size(), &wKeyTagLen, byKeyTag);
    } else {
        BuildAsn1Tag( ASN1_INTEGER, wDummySigLen, &wSigTagLen, bySigTag);
        BuildAsn1Tag( ASN1_SEQUENCE, wDESKeyTagLen + vtKeyVal.size() +
                        wSigTagLen + wDummySigLen, &wKeyTagLen, byKeyTag);
    }

    //Copy to Command Buffer
    vtCmd.append((LPSTR)byKeyTag, wKeyTagLen);
    vtCmd.append((LPSTR)byDESKeyTag, wDESKeyTagLen);
    vtCmd.append(vtKeyVal.constData(), vtKeyVal.size());
    vtCmd.append((LPSTR)bySigTag, wSigTagLen);

    if (vtSignVal.size() != 0) {
        vtCmd.append(vtSignVal.constData(), vtSignVal.size());
    } else {
        vtCmd.append((LPSTR)byDummySig, wDummySigLen);
    }

    long lRet = SendReadCmd(vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "远程下载DES KEY命令失败：lRet=%d", lRet);
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
    int nKeyLen = 8;
    if(vtResult.at(0) == 0x31){
        nKeyLen = 16;
    } else if(vtResult.at(0) == 0x32){
        nKeyLen = 24;
    }
    CAutoHex::Bcd2Hex((BYTE *)(vtResult.constData() + 1), 8, strKCV);

    Log(ThisModule, 1, "下载DESKEY成功[KeyLen=%d][KCV=%s]", nKeyLen, strKCV.constData());
    return ERR_SUCCESS;
*/
    return ERR_NOT_SUPPORT;
}

//30-00-00-00(FS#0006)
long CDevPIN_ZTC90::RSAGenerateKeyPair(UINT uPKeyNo, UINT uSKeyNo, int iModuleLen, int iExponentValue)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
/*
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.resize(9);
    vtCmd[0] = 0x80;
    vtCmd[1] = (char)uSKeyNo;
    vtCmd[2] = (char)uPKeyNo;
    WORD wModuleLen = EXWORDHL(iModuleLen);
    DWORD dwExponentValue = RVSDWORD(iExponentValue);
    memcpy(vtCmd.data() + 3, &wModuleLen, sizeof(wModuleLen));
    memcpy(vtCmd.data() + 5, &dwExponentValue, sizeof(dwExponentValue));

    long lRet = SendReadCmd(vtCmd, vtResult, 90000);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载RSA密钥对命令失败：lRet=%d, SKeyNo:%d, PKeyNo:%d",
            lRet, uSKeyNo, uPKeyNo);
        return lRet;
    }

    return ERR_SUCCESS;
*/
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::RSAEncryptData(UINT uRSAPKeyNo, RSADATAALGOTYPE eRSAEncAlgo, QByteArray data, QByteArray &result)
{
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::RSADecryptData(UINT uRSASKeyNo, RSADATAALGOTYPE eRSAEncAlgo, QByteArray data, QByteArray &result)
{
    return ERR_NOT_SUPPORT;
}

long CDevPIN_ZTC90::SetDesKeyLenInfo(map<UINT, BYTE> &keyIdMapKeyLen)
{
    m_keyIdMapKeyLen = keyIdMapKeyLen;
    return ERR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
long CDevPIN_ZTC90::PackCmd(LPCSTR lpCmd, DWORD dwCmdLen, LPSTR lpData, DWORD &dwSize)
{
    if (lpCmd == nullptr || lpData == nullptr || dwCmdLen >= dwSize)
        return ERR_DEVPORT_PARAM;

    // Command format: 02h+<Ln>+<CMD>+<DATA>+<LRC>+03h
    // if Ln>255, Ln=0 in command
    BYTE szCmd[8192] = {0};
    int iOffset = 0;
    BYTE byLn = dwCmdLen > 0xFF ? 0 : dwCmdLen;

    //STX
    szCmd[0] = CMD_START;
    iOffset++;

    //Ln
    sprintf((char *)szCmd + iOffset, "%02X", byLn);
    iOffset += 2;

    //<CMD>+<DATA>
    //Hex data => Assic code data(0x30 => 0x33 0x30)
    QByteArray vtAssicCodeData;
    CAutoHex::Bcd2Hex((LPBYTE)lpCmd, dwCmdLen, vtAssicCodeData);
    memcpy(szCmd + iOffset, vtAssicCodeData.constData(), vtAssicCodeData.size());
    iOffset += vtAssicCodeData.size();

    //LRC
    QByteArray vtLRCHexData;
    vtLRCHexData.push_back(byLn);
    vtLRCHexData.append((LPSTR)lpCmd, dwCmdLen);
    memcpy(szCmd + iOffset, GetLRC((LPBYTE)vtLRCHexData.constData(), vtLRCHexData.size()).c_str(), 2);
    iOffset += 2;

    //ETX
    szCmd[iOffset] = CMD_END;
    iOffset++;

    //计算数据总长度
    dwSize = qMin((DWORD)iOffset, dwSize);
    memcpy(lpData, szCmd, dwSize);

    return 0;
}

long CDevPIN_ZTC90::UnPackResp(LPCSTR lpCmd, DWORD dwReadLen, LPSTR lpData, DWORD &dwSize)
{
    THISMODULE(__FUNCTION__);
    // Return format: 02h+<Ln>+<ST>+<DATA>+<LRC>+03h
    DWORD dwDataLen     = 0;    // 数据长度
    int   iSTXOffset    = -1;    // 命令头标识偏移位
    DWORD dwDataOffset  = 0;    // 数据偏移位
    DWORD dwCount       = 0;    // 开始和结束符计算出的数据长度
    bool bCmdOK         = false;

    // 查找开始和结束符
    for (DWORD i = 0; i < dwReadLen; i++)
    {
        if ((BYTE)lpCmd[i] == CMD_START)
        {
            iSTXOffset = i;// 找到开始符
            if(i + 2 < dwReadLen){
                //计算<ST>+<DATA>长度
                int iDataLen = 0;
                sscanf(lpCmd + iSTXOffset + 1, "%02X", &iDataLen);

                if(iDataLen > 0){
                    i += LEN_LEN + iDataLen * 2;
                    if(i + 3 < dwReadLen){
                        i += 3;
                        if((BYTE)lpCmd[i] == CMD_END){
                            bCmdOK = true;
                            dwCount = i - iSTXOffset + 1;
                        } else {
                            //数据格式错误
                            //回滚，重新查找命令头
                            i -= LEN_LEN + iDataLen * 2 + 2;
                            iSTXOffset = -1;
                            break;
                        }
                    } else {
                        //数据长度错误
                        break;
                    }
                } else {
                    //数据长度>255
                }
            } else {
                //数据长度不足
                break;
            }
        } else if ((BYTE)lpCmd[i] == CMD_END) {
            if(iSTXOffset != -1){
                bCmdOK  = true;
                dwCount = i - iSTXOffset + 1;
                break;
            }
        }
    }
    if (!bCmdOK)
    {
        Log(ThisModule, __LINE__, "返回数据结构分析失败[dwReadLen=%u]", dwReadLen);
        return ERR_REDATA;
    }

    dwDataLen = dwCount - FORMAT_DATA_LEN;

    // 校验数据
    QByteArray vtLRCData;
    QByteArray vtLRCDataAssic;
    vtLRCDataAssic.append(lpCmd + iSTXOffset + 1, dwDataLen);
    CAutoHex::Hex2Bcd(vtLRCDataAssic.constData(), vtLRCData);
    std::string strLRC  = GetLRC((LPBYTE)vtLRCData.constData(), vtLRCData.size());
    std::string strCmdLRC;
    strCmdLRC.push_back(lpCmd[iSTXOffset + dwCount - 3]);
    strCmdLRC.push_back(lpCmd[iSTXOffset + dwCount - 2]);
    if (strCmdLRC != strLRC)
    {
        Log(ThisModule, __LINE__, "返回数据检验码LRC匹配失败：CmdLRC[%s] != LRC[%s]", strCmdLRC.c_str(), strLRC.c_str());
        return ERR_REDATA;
    }

    // 返回码：04h is ok
    int iRtnCode = 0;
    sscanf(lpCmd + iSTXOffset + 1 + LEN_LEN, "%02x", &iRtnCode);
    if(iRtnCode != 0x04)
    {
        UpdateStatus(DEVICE_HWERROR, iRtnCode);
        return ConvertErrCode(iRtnCode, ThisModule);
    }
    UpdateStatus(DEVICE_ONLINE, 0);

    // 返回数据<CMD>
    dwDataLen = (dwDataLen - LEN_LEN - CMD_OR_RSP_CODE_LEN)/2;
    dwSize = qMin(dwDataLen, dwSize);

    if (dwSize > 0)
    {
        dwDataOffset = 2;
        memcpy(lpData, vtLRCData.constData() + dwDataOffset, dwSize);
    }

    return 0;
}

string CDevPIN_ZTC90::GetLRC(const BYTE *pszCmd, DWORD dwCmdLen)
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

string CDevPIN_ZTC90::GetRandData()
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

long CDevPIN_ZTC90::SendReadCmd(const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut)
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
    lRet = UnPackResp(vtReadData, vtReadData.size(), szData, dwSize);
    if (lRet == ERR_SUCCESS)
    {
        vtResult.clear();
        vtResult.append(szData, dwSize);
    }
    return lRet;
}

long CDevPIN_ZTC90::SendReadData(const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 发送命令和接收返回数据
    long lRet = m_pDev->Send(vtCmd, vtCmd.size(), dwTimeOut);
    if (lRet != 0)       //30-00-00-00(FT#0068)
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
            vtResult.append(szRead, dwReadLen);
            if (vtResult.size() < FORMAT_DATA_LEN + 1)// 没有达到最小长度
                continue;
            for (int i = 0; i < vtResult.size(); i++)
            {
                if (vtResult.at(i) == CMD_START && !bStart)
                {
                    bStart = true;
                    sscanf(vtResult.constData() + i + 1, "%02X", &dwLen);
                    i += LEN_LEN;
                    if (dwLen > 0)
                    {
                        i += dwLen;
                    }
                }

                if (vtResult.at(i) == CMD_END && bStart)
                {
                    return ERR_DEVPORT_SUCCESS;
                }
            }
        }

    } while (true);

    Log(ThisModule, __LINE__, "读取返回数据超时失败");
    return ERR_DEVPORT_RTIMEOUT;
}

long CDevPIN_ZTC90::ConvertErrCode(BYTE byRTN, LPCSTR ThisModule)
{
    long lRet = ERR_SUCCESS;
    auto it = m_mapErrCode.find(byRTN);
    if (it != m_mapErrCode.end())
    {
        Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][%s]", it->first, it->second.c_str());
    } else {
        Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][未知错误]", byRTN);
    }

    switch(byRTN){
    case PIN_INPUT_LEN_INVALID:
        lRet = ERR_NO_PIN;
        break;
    default:
        lRet = ERR_OTHER;
        break;
    }

    return lRet;
}

void CDevPIN_ZTC90::InitErrCode()
{
    m_mapErrCode[CMD_LEN_ERR]               = STR(CMD_LEN_ERR);
    m_mapErrCode[KEY_VERIFICATION_ERR]      = STR(KEY_VERIFICATION_ERR);
    m_mapErrCode[INVALID_PARAM_OR_MODE]     = STR(INVALID_PARAM_OR_MODE);
    m_mapErrCode[KCV_VERIFICATION_ERR]      = STR(KCV_VERIFICATION_ERR);
    m_mapErrCode[PIN_INPUT_LEN_INVALID]     = STR(PIN_INPUT_LEN_INVALID);
    m_mapErrCode[PRODUCTION_SN_SETTED]      = STR(PRODUCTION_SN_SETTED);
    m_mapErrCode[NO_CARD_IN_PSAM_SLOT]      = STR(NO_CARD_IN_PSAM_SLOT);
    m_mapErrCode[PSAM_CARD_OPT_ERR]         = STR(PSAM_CARD_OPT_ERR);
    m_mapErrCode[PSAM_CARD_NO_POWERON]      = STR(PSAM_CARD_NO_POWERON);
    m_mapErrCode[PSAM_CARD_TYPE_NOT_SUPP]   = STR(PSAM_CARD_TYPE_NOT_SUPP);
    m_mapErrCode[KEY_PRESS_HOLD]            = STR(KEY_PRESS_HOLD);
    m_mapErrCode[TIMEOUT_NO_KEY_PRESS]      = STR(TIMEOUT_NO_KEY_PRESS);
    m_mapErrCode[SM_CHIP_COMM_TIMEOUT]      = STR(SM_CHIP_COMM_TIMEOUT);
    m_mapErrCode[MKEY_NOT_EXIST_OR_INVALID] = STR(MKEY_NOT_EXIST_OR_INVALID);
    m_mapErrCode[WKEY_NOT_EXIST_OR_INVALID] = STR(WKEY_NOT_EXIST_OR_INVALID);
    m_mapErrCode[SM2_RSAKEY_NOT_EXIST_OR_INVALID] = STR(SM2_RSAKEY_NOT_EXIST_OR_INVALID);
    m_mapErrCode[EXC_SM4KEY_NOT_EXIST_OR_INVALID] = STR(EXC_SM4KEY_NOT_EXIST_OR_INVALID);
    m_mapErrCode[ENCRYPT_OR_DECRYPT_FAIL]    = STR(ENCRYPT_OR_DECRYPT_FAIL);
    m_mapErrCode[SIGN_OR_SIGN_VERF_FAIL]     = STR(SIGN_OR_SIGN_VERF_FAIL);
    m_mapErrCode[KEY_EXCHANGE_FAIL]          = STR(KEY_EXCHANGE_FAIL);
    m_mapErrCode[INVALID_RSA_MOLD_HEIGHT]    = STR(INVALID_RSA_MOLD_HEIGHT);
    m_mapErrCode[INVALID_RSA_MOLD_DATA]      = STR(INVALID_RSA_MOLD_DATA);
    m_mapErrCode[INVALID_RSA_EXPONENT_LEN]   = STR(INVALID_RSA_EXPONENT_LEN);
    m_mapErrCode[INVALID_RSA_EXPONENT]       = STR(INVALID_RSA_EXPONENT);
    m_mapErrCode[INVALID_RSA_DATA_STRUCTURE] = STR(INVALID_RSA_DATA_STRUCTURE);
    m_mapErrCode[INVALID_PADDING_DATA]       = STR(INVALID_PADDING_DATA);
    m_mapErrCode[INVALID_RSA_SIGN_DATA]      = STR(INVALID_RSA_SIGN_DATA);
    m_mapErrCode[INVALID_DATA_LEN]           = STR(INVALID_DATA_LEN);
    m_mapErrCode[BATTERY_LOW]                = STR(BATTERY_LOW);
    m_mapErrCode[BATTERY_BAD]                = STR(BATTERY_BAD);
    m_mapErrCode[INVALID_FLASH_PARAM]        = STR(INVALID_FLASH_PARAM);
    m_mapErrCode[WRITE_FLASH_PARAM_FAIL]     = STR(WRITE_FLASH_PARAM_FAIL);
    m_mapErrCode[SELF_DEST_REGISTER_CONFIGURED] = STR(SELF_DEST_REGISTER_CONFIGURED);
    m_mapErrCode[SELF_DEST_REGISTER_LOCKED]  = STR(SELF_DEST_REGISTER_LOCKED);
    m_mapErrCode[CLEAR_SELF_DEST_VERI_FAIL]  = STR(CLEAR_SELF_DEST_VERI_FAIL);
    m_mapErrCode[NO_EXEC_CLEAR_SELE_DEST_VERI] = STR(NO_EXEC_CLEAR_SELE_DEST_VERI);
    m_mapErrCode[CHIP_ABNORMAL]              = STR(CHIP_ABNORMAL);
    return;
}

void CDevPIN_ZTC90::UpdateStatus(WORD wDevice, long lErrCode)
{
    m_stStatus.clear();
    m_stStatus.wDevice = wDevice;
    sprintf(m_stStatus.szErrCode, "0%02X", qAbs(lErrCode));
}

bool CDevPIN_ZTC90::ConvertKeyVal(BYTE bKey, EPP_KEYVAL &stKeyVal)
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

/*
*@function : 键盘和主机双向认证.
*@param    : authKeyData:认证密钥明文数据.
*            randomData:随机数.
*            uKeyNo:认证密钥id，仅在主机认证键盘时使用.
*@return   : ERR_SUCCESS：成功
*            其他:失败
*@history
*@date:2021/7/10 create
*/
long CDevPIN_ZTC90::MutualAuth(QByteArray authKeyData, QByteArray randomData, UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    long lRet = ERR_SUCCESS;
    QByteArray response;

    //认证流程：
    //1.认证密钥初始化到EPP
    lRet = Authenticate(0x10, authKeyData, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "认证密钥初始化失败");
        return lRet;
    }

    //2.获取认证密钥随机数TDES加密结果(主机认证键盘)
    lRet = Authenticate(uKeyNo, randomData, response);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "主机认证键盘失败");
        return lRet;
    }

    //3.主机用认证密钥TDES加密键盘返回数据
    char sz3DesAuth[256] = { 0 };
    m_cDES.gDes(response.data(), authKeyData.data(), sz3DesAuth);

    //4.键盘核验TDES加密结果(键盘认证主机)
    lRet = Authenticate(0x20, sz3DesAuth, response);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "键盘认证主机失败");
        return lRet;
    }

    Log(ThisModule, 1, "主机与键盘双向认证成功");
    return ERR_SUCCESS;
}

QByteArray *CDevPIN_ZTC90::Pading(QByteArray &vtResult, BYTE byPading, bool bEnd/* = true*/, UINT uPadLen/* = 16*/)
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

string CDevPIN_ZTC90::GetTAG(string strHexData, bool bInteger)
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

string CDevPIN_ZTC90::AddTAG(LPCSTR lpHexData, bool bInteger)
{
    std::string strHexData;
    strHexData = lpHexData;
    return strHexData.insert(0, GetTAG(strHexData, bInteger));
}

bool CDevPIN_ZTC90::GetTLVData(LPCSTR lpData, DWORD dwDataLen, vectorString &vtData)
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

string CDevPIN_ZTC90::GetHexPK(string strModulus, string strExponent)
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

bool CDevPIN_ZTC90::GetPKByExportData(LPCSTR lpData, DWORD dwDataLen, vectorString &vtPK)
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

string CDevPIN_ZTC90::FmImportPK(string strHexPK, string strHexSignPK)
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

string CDevPIN_ZTC90::FmImportDesKey(string strHexDES, string strHexSignDES)
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

long CDevPIN_ZTC90::BuildAsn1Tag(WORD wTagType, WORD wDataLen, LPWORD lpwTagLen, LPBYTE lpbyTagBuffer)
{
    long lRet = -1;

    while ( 1 ){
        if(wDataLen < 0){			//Length check
            break;
        }
        if(*lpwTagLen < 2){			//Tag at least 2 byte
            break;
        }
        ZeroMemory(lpbyTagBuffer, *lpwTagLen);
        // Tag type check
        if(	(wTagType == ASN1_INTEGER) ||
                (wTagType == ASN1_SEQUENCE)) {
            memcpy( lpbyTagBuffer, (LPBYTE)(&wTagType), 1 );
        } else {
            break;
        }
        // Length check
        if( wDataLen > 0xFFFF ){			//Data length MAX is 65535				//15-00-05-01(PG#00030)
            break;
        } else if ( wDataLen <= 0x7F ){	//The max length of ShortForm is 127
            memcpy( lpbyTagBuffer + 1,((LPBYTE)(&wDataLen)), 1 );
            *lpwTagLen = 2;
        } else {							// If SHORTFORM_LEN_MAX < wDataLen <= wDataLen_MAX
            int i = 1;
            WORD wTmpwDataLen = wDataLen;
            while ( 1 ) {
                if(wTmpwDataLen > 0xFF){
                    wTmpwDataLen /= (0xFF + 1);
                    i++;
                } else {
                    break;
                }
            }
            *(lpbyTagBuffer + 1) = 0x80 | (BYTE)i;
            wTmpwDataLen = wDataLen;
            for(int j = i; j > 0; j--){
                *(lpbyTagBuffer + 1 + j) = (BYTE)(wTmpwDataLen & 0xFF);
                wTmpwDataLen /= (0xFF + 1);
                *lpwTagLen++;
            }
        }
        lRet = 0;
        break;
    }
    return lRet;
}

long CDevPIN_ZTC90::ExtractAsn1Data( WORD wFieldLen, LPBYTE lpbyField, WORD wTagType,
                      LPWORD lpwTagLen, LPBYTE lpbyData, LPWORD lpwDataLen ){
    WORD wTagLenExtensionSize = 0;
    WORD wTagLenBaseSize = 2;						// Minimum Tag Length
    WORD wBufferLen = wFieldLen;
    WORD wLen = 0;
    long lRet = -1;

    while ( 1 ){
        if( lpbyField == NULL ){
            break;
        }
        if( wFieldLen < wTagLenBaseSize ){			// If the field length is less than minimum tag size
            break;
        }
        if( (lpwDataLen == NULL) || (*lpwDataLen < 0) ){
            break;
        }
        if( lpbyData == NULL ){
            break;
        }
        ::ZeroMemory(lpbyData, *lpwDataLen);
        // Gets data length
        if( *(lpbyField + 1) & 0x80 ){				// In case of Long Form (0x81 or 0x82)
            wTagLenExtensionSize = (WORD)(*(lpbyField + 1) & 0x7F);//Get the length data
            if( wTagLenExtensionSize <= 2 ){
                if( wFieldLen >= wTagLenBaseSize + wTagLenExtensionSize ){
                    //Get the real size of the Data
                    for( int i = 0; i < wTagLenExtensionSize; i++ ){
                        wLen *= (0xFF + 1);
                        wLen += (WORD)(*(lpbyField + 2 + i));
                    }
                } else {							// If the field length is less than tag size
                    break;
                }
            } else {								// Long Form (>= 0x83)
                break;
            }
        } else {									// In case of Short Form
            wLen = *(lpbyField + 1) & 0x7F;
            wTagLenExtensionSize = 0;
        }
        *lpwTagLen = wTagLenBaseSize + wTagLenExtensionSize;
        if(*lpwTagLen + wLen > wFieldLen){			// Calculated length incorrect
            break;
        }
        if ( *lpwDataLen < wLen )	{											//15-00-05-01(PG#00030)
            break;																//15-00-05-01(PG#00030)
        }																		//15-00-05-01(PG#00030)
        *lpwDataLen = wLen;
        memcpy( lpbyData, lpbyField + *lpwTagLen, *lpwDataLen );//Copy the data
        lRet = 0;
        break;
    }
    if(lRet == -1){
        *lpwDataLen = wBufferLen;					// *DataLen is used for tracing later in case of an error
    }
    return lRet;
}

long CDevPIN_ZTC90::SetLoadWKDESMode(bool bIsTDES)
{
    BYTE byMode = bIsTDES ? 0x30 : 0x20;
    return SetAlgoParam(0x00, byMode);
}

long CDevPIN_ZTC90::SetKCVDesMode(bool bIsTDES)
{
    BYTE byMode = bIsTDES ? 0x30 : 0x20;
    return SetAlgoParam(0x01, byMode);
}

//30-00-00-00(FS#0010)
void CDevPIN_ZTC90::ReadConfig()
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
        Log(ThisModule, __LINE__, "PIN加载配置文件失败：%s", strFile.constData());
    }

    CINIReader cIR = cINIFileReader.GetReaderSection("PINInfo");
    m_iClearKeyMode = (int)cIR.GetValue("ClearKeyAsBackspace", 0);

    return;
}

long CDevPIN_ZTC90::SetEppCommConfigParam()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    BYTE byItemId = 0;

    //设置键盘通用参数
    //命令尾加0x03
    byItemId = 0x09;
    lRet = SetCommConfigParam(byItemId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //设置＂更正＂按键的功能
    byItemId = m_iClearKeyMode == 1 ? 0x21 : 0x20;
    lRet = SetCommConfigParam(byItemId);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "设置键盘更正按键功能失败");
        return lRet;
    }

    //Todo:波特率设定

    lRet = SetKCVRelatedParam();

    return lRet;
}

bool CDevPIN_ZTC90::IsTDESKey(UINT uKeyNo)
{
    bool bRet = true;
    if(uKeyNo == 0){
        return bRet;
    }
    map<UINT, BYTE>::const_iterator itr = m_keyIdMapKeyLen.find(uKeyNo);
    if(itr != m_keyIdMapKeyLen.end()){
        bRet = itr->second > 8 ? true : false;
    }

    return bRet;
}

long CDevPIN_ZTC90::ImportAllZeroMKey(bool bIsDes/* = true*/)
{
    THISMODULE(__FUNCTION__);
    long lRet = ERR_SUCCESS;

    char szKCV[64] = { 0 };
    if(bIsDes){
        lRet = ImportMKey(0, 0xFF, gZeroKeyData, szKCV);
    } else {
        lRet = SM4ImportMKey(0, gZeroKeyData, szKCV);
    }

    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载全0 [%s]铭文密钥失败:%d", bIsDes ? "DES":"SM4", lRet);
    }

    return lRet;
}

long CDevPIN_ZTC90::SetKCVRelatedParam()
{
    THISMODULE(__FUNCTION__);

    long lRet = ERR_SUCCESS;
    //设定密钥下载需要返回KCV
    lRet = SetAlgoParam(0x05, 0x05);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "设置密钥下载需要返回KCV失败");
        return lRet;
    }
    //设定kcv长度
    lRet = SetAlgoParam(0x0D, 0x08);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "设置KCV长度失败");
        return lRet;
    }

    return lRet;
}

/*
*@function : 键盘认证命令编辑和发送.
*@param    : byFlag:标志位.
*            data:命令数据.
*            resultData:命令返回数据.
*@return   : ERR_SUCCESS：成功
*            其他:失败
*@history
*@date:2021/7/10 create
*/
long CDevPIN_ZTC90::Authenticate(BYTE byFlag, QByteArray data, QByteArray &resultData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    QByteArray cmd;
    QByteArray response;
    resultData.clear();

    cmd.push_back(0x40);
    cmd.push_back(byFlag);
    cmd.append((LPCSTR)data.constData(), data.size());

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "键盘认证命令[0x%02x]执行失败", byFlag);
        return lRet;
    }

    resultData = response;

    return lRet;
}

long CDevPIN_ZTC90::SetCommConfigParam(BYTE byItemId)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    QByteArray cmd;
    QByteArray response;

    cmd.push_back(0x45);
    cmd.push_back(byItemId);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "设置键盘通用参数[0x%02x]失败", byItemId);
        return lRet;
    }

    return lRet;
}

long CDevPIN_ZTC90::SetAlgoParam(BYTE byItemId, BYTE byItemOptId)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    if(m_iAlgoParamFlag[byItemId] == byItemOptId){
        return lRet;
    }

    QByteArray cmd;
    QByteArray response;

    cmd.push_back(0x46);
    cmd.push_back(byItemId);
    cmd.push_back(byItemOptId);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "设置键盘算法参数[ItemId:0x%02x,ItemOptId:0x%02x]失败",
            byItemId, byItemOptId);
        return lRet;
    }

    m_iAlgoParamFlag[byItemId] = byItemOptId;
    return lRet;
}

long CDevPIN_ZTC90::ActiveKey(UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    QByteArray cmd;
    QByteArray response;

    //设置通用参数,决定激活的是主密钥还是工作密钥
    BYTE byItemId = 0x01;
    BYTE byItemOptId = 0;
    if(uKeyNo <= 0x1F){
        byItemOptId = IsTDESKey(uKeyNo) ? 0x70 : 0x60;
    } else {
        byItemOptId = IsTDESKey(uKeyNo) ? 0x30 : 0x20;
    }
    lRet = SetAlgoParam(byItemId, byItemOptId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    BYTE byMKeyNo = 0x00;
    BYTE byWKeyNo = 0x00;
    if(uKeyNo > 0x1F){
        byWKeyNo = uKeyNo;
    } else {
        byMKeyNo = uKeyNo;
    }
    cmd.push_back(0x43);
    cmd.push_back(byMKeyNo);
    cmd.push_back(byWKeyNo);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "激活密钥[MKeyNo:0x%02x,WKeyNo:0x%02x]失败", byMKeyNo, byWKeyNo);
        return lRet;
    }

    return lRet;
}

long CDevPIN_ZTC90::ActiveSMKey(UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    QByteArray cmd;
    QByteArray response;

    //设置参数,决定激活的是主密钥还是工作密钥
    BYTE byItemId = 0x00;
    BYTE byItemOptId = (uKeyNo <= 0x1F) ? 0x30 : 0x20;
    lRet = SetSMAlgoParam(byItemId, byItemOptId);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    BYTE byMKeyNo = 0x00;
    BYTE byWKeyNo = 0x00;
    if(uKeyNo > 0x1F){
        byWKeyNo = uKeyNo;
    } else {
        byMKeyNo = uKeyNo;
    }
    cmd.push_back(0x86);
    cmd.push_back(0x01);
    cmd.push_back(byMKeyNo);
    cmd.push_back(byWKeyNo);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "激活SM密钥[MKeyNo:0x%02x,WKeyNo:0x%02x]失败", byMKeyNo, byWKeyNo);
        return lRet;
    }

    return lRet;
}

long CDevPIN_ZTC90::ActiveSM2Key(UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    QByteArray cmd;
    QByteArray response;

    cmd.push_back(0x86);
    cmd.push_back(0x02);
    cmd.push_back((char)uKeyNo);
    cmd.push_back((char)0x00);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "激活SM2密钥[MKeyNo:0x%02x]失败", uKeyNo);
        return lRet;
    }

    return lRet;
}

long CDevPIN_ZTC90::SetIVData(QByteArray &ivData, bool bIsDes/* = true*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    QByteArray cmd;
    QByteArray response;

    cmd.push_back(bIsDes ? 0x60 : 0x8B);
    cmd.append(ivData);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "设置初始向量数据失败");
        return lRet;
    }

    return lRet;
}

long CDevPIN_ZTC90::SetPinBlockData(QByteArray pinBlockData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    QByteArray cmd;
    QByteArray response;

    cmd.push_back(0x34);
    cmd.append(pinBlockData);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "设置PinBlock数据(卡号/传输码)错误");
        return lRet;
    }

    return lRet;
}

long CDevPIN_ZTC90::ClearSelfDestroy()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;
    QByteArray cmd;
    QByteArray response;

    cmd.push_back(0xA0);
    cmd.push_back(0x31);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "自毁后重激活失败");
        return lRet;
    }

    return lRet;
}

long CDevPIN_ZTC90::SetSMAlgoParam(BYTE byItemId, BYTE byItemOptId)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;

    QByteArray cmd;
    QByteArray response;

    cmd.push_back(0x81);
    cmd.push_back(byItemId);
    cmd.push_back(byItemOptId);

    lRet = SendReadCmd(cmd, response);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "设置国密算法参数[ItemId:0x%02x,ItemOptId:0x%02x]失败",
            byItemId, byItemOptId);
        return lRet;
    }

    return lRet;
}
