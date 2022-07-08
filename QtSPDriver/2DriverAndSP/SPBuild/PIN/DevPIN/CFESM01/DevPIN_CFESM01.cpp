#include "DevPIN_CFESM01.h"

static const char *ThisFile = "DevPIN_CFESM01.cpp";

//键值和键功能描述映射
static EPP_KEYVAL gszKeyVal[] = { \
    {0x30, "0"}, {0x31, "1"}, {0x32, "2"}, {0x33, "3"}, {0x34, "4"},
    {0x35, "5"}, {0x36, "6"}, {0x37, "7"}, {0x38, "8"}, {0x39, "9"},
    {0x1B, "CANCEL"}, {0x0D, "ENTER"}, {0x08, "CLEAR"}, {0x10, "BACKSPACE"},
    {0x54, "00"}, {0x2E, "."}, {0x23, "HELP"}, {0x55, "000"},
    {0x02, "PageUp"}, {0x03, "PageDown"},
    {0x41, "F1"}, {0x42, "F2"}, {0x43, "F3"}, {0x44, "F4"}, {0x45, "F5"},
    {0x46, "F6"}, {0x47, "F7"}, {0x48, "F8"}, {0x2A, ""}
};


CDevPIN_CFESM01::CDevPIN_CFESM01(LPCSTR lpDevType)
{
    SetLogFile("PIN_CFESM01.log", ThisFile, lpDevType);

    //初始化错误表
    InitErrCode();

    m_iClearKeyMode = 0;
    m_bySymmetricKeyMode = SYMMETRIC_KEY_MODE_UNKNOWN;
}

CDevPIN_CFESM01::~CDevPIN_CFESM01()
{
    Close();
}

void CDevPIN_CFESM01::Release()
{
    return;
}

long CDevPIN_CFESM01::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    //读取配置文件
    ReadConfig();

    if(nullptr == m_pDev){
        if(m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "PIN", "DevPIN_CFESM01") != 0){
            UpdateStatus(DEVICE_OFFLINE, ERR_DEVPORT_LIBRARY);
            Log(ThisModule, __LINE__, "加载库(AllDevPort.dll)失败[%s]", m_pDev.LastError().toStdString().c_str());
            return ERR_DEVPORT_LIBRARY;
        }
    }

    LOGDEVACTION();
    long lRet = m_pDev->Open(lpMode);
    if(lRet != 0){
        UpdateStatus(DEVICE_OFFLINE, ERR_DEVPORT_NOTOPEN);
        Log(ThisModule, __LINE__, "打开设备失败:%d[%s]", lRet, lpMode);
        return ERR_DEVPORT_NOTOPEN;
    }

    UpdateStatus(DEVICE_ONLINE, 0);

    EnableRemoveFunc(m_bRemoveFuncSupp);

    //移除状态非法时做移入认证
    RemoveAuth(0x00);

    //设置非法长按键时间
    SetIlligalHoldKeyTime(m_iIllegalHoldKeyTime);

    return ERR_DEVPORT_SUCCESS;
}

long CDevPIN_CFESM01::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if(m_pDev != nullptr){
        if(m_pDev->IsOpened()){
            m_pDev->Close();
        }
    }

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::InitEpp()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    //删除所有密钥
    QByteArray vtCmd;
    QByteArray vtResult;

    long lRet = ExecuteCommand(0x80, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "删除所有密钥失败：lRet=%d", lRet);
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    //删除所有密钥
    QByteArray vtCmd;
    QByteArray vtResult;

    long lRet = ExecuteCommand(0xB0, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "软复位失败：lRet=%d", lRet);
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::GetStatus(DEVPINSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);

    CAutoCopyDevStatus<DEVPINSTATUS> _auto(&stStatus, &m_stStatus);
    QByteArray vtCmd;
    QByteArray vtResult;
    long lRet = ExecuteCommand(0xB5, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        UpdateStatus(DEVICE_OFFLINE, lRet);
        Log(ThisModule, __LINE__, "读取设备状态命令失败：lRet=%d", lRet);
        return lRet;
    }

    if(vtResult.at(5) == 0x00){
        m_stStatus.bIllegalRemove = true;
    }
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::InstallStatus(bool bClear)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::GetFirmware(LPSTR lpFWVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if(lpFWVer == nullptr){
        return ERR_SUCCESS;
    }

    char szFirmwareAuthVer[9] = {0};            //固件认证版本号
    char szFirmwareInnerVer[129] = {0};         //固件内部版本号
    char szMainChipID[65] = {0};                //主芯片ID
    char szProductSN[25] = {0};                 //产品序列号

    QByteArray vtCmd;
    QByteArray vtResult;

    //获取固件认证版本号
    long lRet = ExecuteCommand(0xB6, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "获取固件认证版本号失败：lRet=%d", lRet);
    } else {
        memcpy(szFirmwareAuthVer, vtResult.constData(), vtResult.size());
    }

    //获取固件内部版本号
    vtResult.clear();
    lRet = ExecuteCommand(0xB7, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "获取固件内部版本号失败：lRet=%d", lRet);
    } else {
        memcpy(szFirmwareInnerVer, vtResult.constData(), vtResult.size());
    }

    //获取主芯片ID
    vtResult.clear();
    lRet = ExecuteCommand(0xB8, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "获取主芯片ID失败：lRet=%d", lRet);
    } else {
        memcpy(szMainChipID, vtResult.constData(), vtResult.size());
    }

    //获取产品序列号
    vtResult.clear();
    lRet = ExecuteCommand(0xBA, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "获取产品序列号失败：lRet=%d", lRet);
    } else {
        memcpy(szProductSN, vtResult.constData(), vtResult.size());
    }

    sprintf(lpFWVer, "FirmwareAuthVer:%s|FirmwareInnerVer:%s|MainChipID:%s|ProductSN:%s",
            szFirmwareAuthVer, szFirmwareInnerVer, szMainChipID, szProductSN);

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SetKeyValue(LPCSTR lpKeyVal)
{

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SetActiveKey(DWORD dwActiveFuncKey, DWORD dwActiveFDKKey)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if(m_iClearKeyMode == 1 || (dwActiveFuncKey & 0x00002000)){
        SetFuncOfKeys(true);
    } else {
        SetFuncOfKeys(false);
    }

    //FDK按键键值转换
    DWORD dwActiveFDKKeyConvert = 0;
    ConvertFDKEnableBitData(dwActiveFDKKey, dwActiveFDKKeyConvert);

    QByteArray vtCmd;
    QByteArray vtResult;

    ACTIVATEKEY stActivateKey;
    ZeroMemory(&stActivateKey, sizeof(stActivateKey));
    stActivateKey.dwActiveKeys = RVSDWORD(dwActiveFuncKey);
    stActivateKey.dwActiveFDKs = RVSDWORD(dwActiveFDKKeyConvert);
    vtCmd.append((LPSTR)&stActivateKey, sizeof(stActivateKey));

    long lRet = ExecuteCommand(0x90, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "激活按键失败：lRet=%d,dwActiveKeys:%u[%u],dwActiveFDKs:%u[%u]",
            lRet, dwActiveFuncKey, stActivateKey.dwActiveKeys, dwActiveFDKKey, stActivateKey.dwActiveFDKs);
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::ImportMKey(UINT uMKeyNo, UINT uKeyMode, LPCSTR lpKeyVal, LPSTR lpKCV)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    WORD wValLen = 0;
    DWORD dwKeyUse = 0;
    QByteArray vtResult;
    QByteArray vtCmd;

    //检查密钥索引(0~63)
    if((uKeyNo >= DES_OR_SM4_KEY_MAX_NUM) ||
       (uMKeyNo != 0xFF && (uMKeyNo >= DES_OR_SM4_KEY_MAX_NUM))){
        Log(ThisModule, __LINE__, "无效密钥ID：uKeyNo=%d, uMKeyNo=%d", uKeyNo, uMKeyNo);
        return ERR_PARAM;
    }

    WORD wKeyNo = uKeyNo;
    WORD wEncKeyNo = (uMKeyNo == 0xFF) ? 0xFFFF : uMKeyNo;
    //转换密钥属性
    if(uKeyUse & PKU_CRYPT){
        dwKeyUse |= KEY_ATTR_DATA;
    }
    if(uKeyUse & PKU_FUNCTION){
        dwKeyUse |= KEY_ATTR_PIN;
    }
    if(uKeyUse & PKU_MACING){
        dwKeyUse |= KEY_ATTR_MAC;
    }
    if(uKeyUse & PKU_KEYENCKEY){
        dwKeyUse |= KEY_ATTR_ENC;
    }

    if(dwKeyUse == 0){
        Log(ThisModule, __LINE__, "无效密钥属性：dwKeyUse=%u[%u]", dwKeyUse, uKeyUse);
        return ERR_PARAM;
    }
    dwKeyUse |= (KEY_ATTR_VECTOR_DECRYPT | KEY_ATTR_REMOVE);

    wValLen = strlen(lpKeyVal) / 2;
    if (wValLen != 8 && wValLen != 16 && wValLen != 24)
    {
        Log(ThisModule, __LINE__, "无效Key长度：wValLen=%d", wValLen);
        return ERR_PARAM;
    }

    //切换对称密钥算法模式
    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_DES);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    UINT uLen;
    IMPORTKEY stImportKey;
    ZeroMemory(&stImportKey, sizeof(stImportKey));
    stImportKey.wKeyId = EXWORDHL(wKeyNo);
    stImportKey.wMKeyId = EXWORDHL(wEncKeyNo);
    stImportKey.dwKeyAttr = RVSDWORD(dwKeyUse);
    uLen = sizeof(stImportKey.byKeyValue);
    CAutoHex::Hex2Bcd(lpKeyVal, stImportKey.byKeyValue, uLen);
    vtCmd.append((LPSTR)&stImportKey, sizeof(stImportKey) - (sizeof(stImportKey.byKeyValue) - uLen));

    lRet = ExecuteCommand(0x22, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "工作密钥导入失败：lRet=%d[uKeyNo=%d, uMKeyNo=%d]", lRet, uKeyNo, uMKeyNo);
        return lRet;
    }

    lRet = ReadSymmKeyKCV(wKeyNo, byKcvMode, lpKCV);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    Log(ThisModule, 1, "工作密钥导入成功:uKeyNo=%04X, uMKeyNo=%04X", uKeyNo, uMKeyNo);
    return ERR_SUCCESS;
}

void CDevPIN_CFESM01::SetDesKcvDecryptKey(UINT uKeyNo)
{
    return;
}

long CDevPIN_CFESM01::DeleteKey(UINT uKeyNo)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::TextInput()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_vtKeyVal.clear();
    long lRet = SwitchBeep(true);
    if (lRet != ERR_SUCCESS)
    {
        return lRet;
    }

    // 使用这种方法，在设备日志中，有记录明文输入
    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;
    vtCmd.push_back((char)0x00);
    vtCmd.push_back((char)0x00);

    lRet = ExecuteCommand(0x91, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打开明文输入命令失败：%d", lRet);
        SwitchBeep(false);// 关闭
        return lRet;
    }

    Log(ThisModule, 1, "打开明文输入成功");
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::PinInput(UINT uMinLen, UINT uMaxLen, bool bClearKeyAsBackspace/* = false*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_vtKeyVal.clear();
    if (uMinLen > uMaxLen || uMinLen > 0x10 || uMaxLen > 0x10)
    {
        Log(ThisModule, __LINE__, "参数错误：uMinLen=%d, uMaxLen=%d", uMinLen, uMaxLen);
        return ERR_PARAM;;
    }

    long lRet = 0;
    lRet = SwitchBeep(true);
    if (lRet != ERR_SUCCESS)
    {
        return lRet;
    }

    LOGDEVACTION();
    QByteArray vtCmd;
    QByteArray vtResult;

    INPUTPIN_CFES stInputPin;
    stInputPin.byMinLen = uMinLen;
    stInputPin.byMaxLen = uMaxLen;
    stInputPin.byEndMode = 0x00;
    stInputPin.byEchoCode = 0x2A;
    stInputPin.wTimeout = 0x00;
    vtCmd.append((LPCSTR)&stInputPin, sizeof(stInputPin));

    lRet = ExecuteCommand(0x27, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        SwitchBeep(false);          //关闭按键声音
        Log(ThisModule, __LINE__, "打开密文输入命令失败：%d", lRet);
        return lRet;
    }

    Log(ThisModule, __LINE__, "打开密文输入成功");
    return lRet;
}

long CDevPIN_CFESM01::ReadKeyPress(EPP_KEYVAL &stKeyVal, DWORD dwTimeOut)
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

    long lRet = ERR_SUCCESS;
    bool bSendKeyCodeAuto = false;
    if(bSendKeyCodeAuto){
        BYTE szKeyBuff[64] = {0};
        DWORD dwWaitLen = sizeof(szKeyBuff) - 1;
        lRet = m_pDev->Read((char *)szKeyBuff, dwWaitLen, dwTimeOut);
        if (lRet == ERR_SUCCESS)
        {
            for (UINT i = 0; i < dwWaitLen && i < sizeof(szKeyBuff); i++)
            {
                if (ConvertKeyVal(szKeyBuff[i], stKeyVal))
                {
                    m_vtKeyVal.push_back(stKeyVal);
                }
            }

            if (!m_vtKeyVal.empty())
            {
                stKeyVal = m_vtKeyVal.front();
                m_vtKeyVal.pop_front();
            }
        } else if (lRet == ERR_DEVPORT_RTIMEOUT) {
            m_pDev->CancelLog();
        }
    } else {
        QByteArray vtCmd;
        QByteArray vtResult;

        vtCmd.push_back((char)0x00);

        lRet = ExecuteCommand(0x92, vtCmd, vtResult);
        if (lRet == ERR_SUCCESS)
        {
            for(int i = 0; i < vtResult.size(); i++)
            {
                if (ConvertKeyVal(vtResult.at(i), stKeyVal))
                {
                    m_vtKeyVal.push_back(stKeyVal);
                }
            }


            if (!m_vtKeyVal.empty())
            {
                stKeyVal = m_vtKeyVal.front();
                m_vtKeyVal.pop_front();
                Log(ThisModule, __LINE__, "KeyCode:0x%X", stKeyVal.byKeyVal);
            } else {
                //读取按键为空，按照读超时处理
                return ERR_DEVPORT_RTIMEOUT;
            }
        } else {
            Log(ThisModule, __LINE__, "被动模式下提取按键键值命令失败：%d", lRet);
        }
    }

    return lRet;
}

long CDevPIN_CFESM01::CancelInput(bool bPinInput)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    //关闭按键声音
    long lRet = SwitchBeep(false);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;

    lRet = ExecuteCommand(0x93, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "取消[%s]输入命令失败：%d", bPinInput ? "密文" : "明文", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "取消[%s]输入成功", bPinInput ? "密文" : "明文");
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::GetPinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    WORD wFmType = 0;
    WORD wKeyNo = uKeyNo;
    switch (uFormat)
    {
    case PIN_ISO9564_0:
        wFmType = PIN_FORMAT_ISO9564_F0;
        break;
    case PIN_ISO9564_1:
        wFmType = PIN_FORMAT_ISO9564_F1;
        break;
    case PIN_ISO9564_3:
        wFmType = PIN_FORMAT_ISO9564_F3;
        break;
    case PIN_IBM3624:
        wFmType = PIN_FORMAT_IBM3624;
        break;
    case PIN_VISA:
        wFmType = PIN_FORMAT_FORMVISA;
        break;
    case PIN_VISA3:
        wFmType = PIN_FORMAT_FORMVISA3;
        break;
    case PIN_ASCII:
        wFmType = PIN_FORMAT_ANSIX98;
        break;
    case PIN_ISO9564_2:
    default:
        Log(ThisModule, __LINE__, "不支持模式：%02X", uFormat);
        return ERR_NOT_SUPPORT;
    }

    UINT uLen = strlen(lpCustomerData);
    //Todo:每种算法要求的数据长度及是否支持填充字符
    if (uLen != 12 && uLen != 10)
    {
        Log(ThisModule, __LINE__, "无效长度用户数据：%d!=12", uLen);
        return ERR_PARAM;
    }

    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_DES);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    QByteArray vtCmd;
    QByteArray vtResult;

    PINBLOCK stPinBlock;
    ZeroMemory(&stPinBlock, sizeof(stPinBlock));
    stPinBlock.wKeyId = EXWORDHL(wKeyNo);
    stPinBlock.wFormat = EXWORDHL(wFmType);
    stPinBlock.byPadChar = bPadding;
    stPinBlock.byCustomerDataLen = strlen(lpCustomerData);
    strcpy((char *)stPinBlock.byCustomerData, (char *)lpCustomerData);
    uLen = sizeof(stPinBlock) - (sizeof(stPinBlock.byCustomerData) - uLen);

    vtCmd.append((LPSTR)&stPinBlock, uLen);

    lRet = ExecuteCommand(0x28, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取密码块失败：%d[uKeyNo=%02X,uFormat=%02X]", lRet, uKeyNo, wFmType);
        return lRet;
    }

    CAutoHex autoHex(vtResult);
    strcpy(lpPinBlock, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::DataCrypt(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, QByteArray &vtResultData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpCryptData == nullptr || lpIVData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    WORD wAlgorithm = DATA_ALGO_ECB;
    BYTE byOpMode = ENCRYPT_MODE_CFES;
    switch (uMode)
    {
    case ECB_EN:
        break;// ECB // Encryption
    case ECB_DE:
        byOpMode = DECRYPT_MODE_CFES;
        break;// ECB // Decryption
    case CBC_EN:
        wAlgorithm = DATA_ALGO_CBC;
        break;// CBC // Encryption
    case CBC_DE:
        wAlgorithm = DATA_ALGO_CBC;
        byOpMode = DECRYPT_MODE_CFES;
        break;// CBC // Decryption
    default:
        Log(ThisModule, __LINE__, "不支持Crypt模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    //加密数据长度检查
//    if(strlen(lpCryptData) / 2 > 1024) {
//        Log(ThisModule, __LINE__, "Data运算数据长度超过最大值1024:%d", strlen(lpCryptData)/2);
//        return ERR_PARAM;
//    }

    LOGDEVACTION();
    QByteArray vtCmd;
    QByteArray vtResult;
    UINT uLen;

    //切换对称密钥算法模式
    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_DES);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //兼容3DES密钥，选择DES算法.不支持自动检测密钥长度，所以先获取密钥长度.
    GETSYMMKEYINFO stGetSymmKeyInfo;
    stGetSymmKeyInfo.wKeyId = EXWORDHL((WORD)uKeyNo);
    vtCmd.append((LPSTR)&stGetSymmKeyInfo, sizeof(stGetSymmKeyInfo));
    lRet = ExecuteCommand(0x25, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "查询对称密钥信息失败：lRet=%d, uKeyNo=%u", lRet, uKeyNo);
        return lRet;
    }
    BYTE byKeyLen = vtResult.at(0);
    if(byKeyLen > 8){
        switch(wAlgorithm){
        case DATA_ALGO_ECB:
            wAlgorithm = DATA_ALGO_TDES_ECB;
            break;
        case DATA_ALGO_CBC:
            wAlgorithm = DATA_ALGO_TDES_CBC;
            break;
        default:
            break;
        }
    }

#if 1
    vtResult.clear();
    QByteArray vtOperData;
    QByteArray vtInitVec;

    CAutoHex::Hex2Bcd(lpCryptData, vtOperData);
    if(DATA_ALGO_CBC == wAlgorithm || DATA_ALGO_TDES_CBC == wAlgorithm){
        //处理初始向量
        if(lpIVData != nullptr && strlen(lpIVData) > 0 &&
           memcmp(lpIVData, "0000000000000000", 16)){
            CAutoHex::Hex2Bcd(lpIVData, vtInitVec);
        }
    }

    while(vtOperData.size() > 0){
        int iBlockDataSize = qMin(vtOperData.size(), BLOCK_DATA_MAX_SIZE);

        //设置初始化向量
        if(DATA_ALGO_CBC == wAlgorithm || DATA_ALGO_TDES_CBC){
            if(vtInitVec.size() > 0){
                lRet = SetInitVector(0xFFFF, vtInitVec);
                if(lRet != ERR_SUCCESS){
                    return lRet;
                }
            }
        }

        QByteArray vtBlockDataResult;
        lRet = DataOper(uKeyNo, wAlgorithm, byOpMode, bPadding, vtOperData, iBlockDataSize, vtBlockDataResult);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }

        //加解密结果合并
        vtResult.append(vtBlockDataResult);

        if(DATA_ALGO_CBC == wAlgorithm || DATA_ALGO_TDES_CBC == wAlgorithm){
            //加解密结果最后16bytes作为下一次运算的向量
            vtInitVec = vtBlockDataResult.right(8);
        }
        vtOperData.remove(0, iBlockDataSize);
    }
#else
    //设置初始向量
    if(wAlgorithm & (DATA_ALGO_CBC | DATA_ALGO_TDES_CBC) && strlen(lpIVData) > 0){
        SETINITVEC_CFES stSetInitVec;
        ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
        //Todo:暂时不支持向量数据加密
        stSetInitVec.wKeyId = 0xFFFF;
        uLen = sizeof(stSetInitVec.byVector);
        CAutoHex::Hex2Bcd(lpIVData, stSetInitVec.byVector, uLen);
        uLen = sizeof(stSetInitVec) - (sizeof(stSetInitVec.byVector) - uLen);

        vtCmd.clear();
        vtCmd.append((LPSTR)&stSetInitVec, uLen);
        lRet = ExecuteCommand(0x29, vtCmd, vtResult);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "Data运算设置初始化向量失败：lRet=%d, lpIVData=%s", lRet, lpIVData);
            return lRet;
        }
    }

    //Data运算
    DATAOP stDataOp;
    ZeroMemory(&stDataOp, sizeof(stDataOp));
    stDataOp.wKeyId = EXWORDHL(uKeyNo);
    stDataOp.byPadChar = bPadding;
    stDataOp.wAlgorithm = EXWORDHL(wAlgorithm);
    stDataOp.byOperate = byOpMode;
    uLen = sizeof(stDataOp.byData);
    CAutoHex::Hex2Bcd(lpCryptData, stDataOp.byData, uLen);
    stDataOp.wDataLen = EXWORDHL((WORD)uLen);
    uLen = sizeof(stDataOp) - (sizeof(stDataOp.byData) - uLen);

    vtCmd.clear();
    vtCmd.append((LPSTR)&stDataOp, uLen);

    lRet = ExecuteCommand(0x2A, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "Data运算失败：lRet=%d", lRet);
        return lRet;
    }
#endif

    CAutoHex autoHex(vtResult);
    vtResultData.append(autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::DataMAC(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, QByteArray &vtResultData, BYTE byPad/* = 0xFF*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpMacData == nullptr || lpIVData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    WORD wAlgorithm = 0;
    switch (uMode)
    {
    case MAC_ANSIX99:
        wAlgorithm = MAC_ALGO_X99;
        break;
    case MAC_ANSIX919:
        wAlgorithm = MAC_ALGO_X919;
        break;
    case MAC_PBOC:
        wAlgorithm = MAC_ALGO_PBOC;
        break;
    case MAC_CBC:
//        wAlgorithm |= MAC_ALGO_UBC;
        wAlgorithm = MAC_ALGO_X99T;
        break;
    case MAC_ISO16609:
    default:
        Log(ThisModule, __LINE__, "不支持MAC模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    if(wAlgorithm != MAC_ALGO_X99T){
        if(strlen(lpMacData) / 2 > (1024 * 8)){
            Log(ThisModule, __LINE__, "Mac运算数据长度超过最大值8192:%d", strlen(lpMacData)/2);
            return ERR_PARAM;
        }
    }


    LOGDEVACTION();
    QByteArray vtCmd;
    QByteArray vtResult;
    UINT uLen;

    //切换对称密钥算法模式
    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_DES);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //兼容3DES密钥，选择DES算法.不支持自动检测密钥长度，所以先获取密钥长度.
    GETSYMMKEYINFO stGetSymmKeyInfo;
    stGetSymmKeyInfo.wKeyId = EXWORDHL((WORD)uKeyNo);
    vtCmd.append((LPSTR)&stGetSymmKeyInfo, sizeof(stGetSymmKeyInfo));
    lRet = ExecuteCommand(0x25, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "查询对称密钥信息失败：lRet=%d, uKeyNo=%u", lRet, uKeyNo);
        return lRet;
    }

    BYTE byKeyLen = vtResult.at(0);
    if(byKeyLen > 8){
        if(wAlgorithm == MAC_ALGO_X99){
            wAlgorithm = MAC_ALGO_X919;
        }
    } else {
        if(wAlgorithm == MAC_ALGO_X919){
            wAlgorithm = MAC_ALGO_X99;
        }
    }

#if 1
    vtResult.clear();
    QByteArray vtOperData;
    QByteArray vtInitVector;
    CAutoHex::Hex2Bcd(lpMacData, vtOperData);
    if(lpIVData != nullptr && strlen(lpIVData) > 0 &&
       memcmp(lpIVData, "0000000000000000", 16)){
        CAutoHex::Hex2Bcd(lpIVData, vtInitVector);
    }

    while(vtOperData.size() > 0){
        int iBlockDataSize = qMin(vtOperData.size(), BLOCK_DATA_MAX_SIZE);
        if(wAlgorithm != MAC_ALGO_X99T){
            iBlockDataSize = vtOperData.size();
        }

        //设置初始化向量
        if(vtInitVector.size() > 0){
            lRet = SetInitVector(0xFFFF, vtInitVector);
            if(lRet != ERR_SUCCESS){
                return lRet;
            }
        }

        lRet = MacOper(uKeyNo, wAlgorithm, byPad, vtOperData, iBlockDataSize, vtResult);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }

        //上次加密结果作为下次加密操作向量
        vtInitVector = vtResult;
        vtOperData.remove(0, iBlockDataSize);
    }
#else
    //设置初始向量
    if(strlen(lpIVData) > 0){
        SETINITVEC_CFES stSetInitVec;
        ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
        stSetInitVec.wKeyId = 0xFFFF;
        uLen = sizeof(stSetInitVec.byVector);
        CAutoHex::Hex2Bcd(lpIVData, stSetInitVec.byVector, uLen);
        uLen = sizeof(stSetInitVec) - (sizeof(stSetInitVec.byVector) - uLen);

        vtCmd.clear();
        vtCmd.append((LPSTR)&stSetInitVec, uLen);
        lRet = ExecuteCommand(0x29, vtCmd, vtResult);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "Mac运算设置初始化向量失败：lRet=%d, lpIVData=%s", lRet, lpIVData);
            return lRet;
        }
    }

    MACOP_CFES stMacOp;
    ZeroMemory(&stMacOp, sizeof(stMacOp));
    stMacOp.wKeyId = EXWORDHL(uKeyNo);
    stMacOp.wAlgorithm = EXWORDHL(wAlgorithm);
    stMacOp.byPad = byPad;
    uLen = sizeof(stMacOp.byData);
    CAutoHex::Hex2Bcd(lpMacData, stMacOp.byData, uLen);
    stMacOp.wDataLen = EXWORDHL((WORD)uLen);
    uLen = sizeof(stMacOp) - (sizeof(stMacOp.byData) - uLen);

    vtCmd.clear();
    vtCmd.append((LPSTR)&stMacOp, uLen);
    lRet = ExecuteCommand(0x2B, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "Mac运算失败：lRet=%d", lRet);
        return lRet;
    }
#endif

    CAutoHex autoHex(vtResult);
    vtResultData.append(autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RandData(LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;
    RANDOM_CFES stRandom;
    stRandom.byUseType = 0x00;
    stRandom.wRandomDataLen = EXWORDHL(0x08);
    vtCmd.append((LPSTR)&stRandom, sizeof(stRandom));

    long lRet = ExecuteCommand(0xB2, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取8位长度的随机数失败：%d", lRet);
        return lRet;
    }

    //转换为HEX字符串
    CAutoHex autoHex(vtResult);
    strcpy(lpData, autoHex.GetHex());
    return ERR_SUCCESS;
}

/*
 * @param : bySymmKeyType(0:DES 1:AES 2:SM4)
 *
 */
long CDevPIN_CFESM01::ReadSymmKeyKCV(UINT uKeyNo, BYTE byKcvMode, LPSTR lpKcv, BYTE bySymmKeyType/* = 0*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    //切换对称密钥算法模式
    long lRet = SwitchSymmetricKeyMode(bySymmKeyType);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    QByteArray vtCmd;
    QByteArray vtResult;

    READSYMMKEYKCV_CFES stReadSymmKeyKcv;
    stReadSymmKeyKcv.wKeyId = EXWORDHL(uKeyNo);
    stReadSymmKeyKcv.byKcvMode = byKcvMode;
    vtCmd.append((LPSTR)&stReadSymmKeyKcv, sizeof(stReadSymmKeyKcv));

    lRet = ExecuteCommand(0x26, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "密钥KCV获取失败：lRet=%d[uKeyNo=%d, byKCVMode=%d]", lRet, uKeyNo, byKcvMode);
        return lRet;
    }

    CAutoHex cHex(vtResult);
    strcpy(lpKcv, cHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SMGetFirmware(LPSTR lpSMVer)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM4ImportMKey(UINT uMKeyNo, LPCSTR lpKeyVal, LPSTR lpKCV, UINT uEncKeyNo/* = 0xFF*/)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM4ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    DWORD dwKeyUse = 0;
    QByteArray vtResult;
    QByteArray vtCmd;

    if ((uKeyNo >= DES_OR_SM4_KEY_MAX_NUM) ||
        (uMKeyNo != 0xFF && (uMKeyNo >= DES_OR_SM4_KEY_MAX_NUM)))
    {
        Log(ThisModule, __LINE__, "无效保存SM主密钥ID：uKeyNo=%d, uMKeyNo=%d", uKeyNo, uMKeyNo);
        return ERR_PARAM;
    }

    //传入的密钥索引比实际值大１
    WORD wKeyNo = uKeyNo;
    WORD wEncKeyNo = (uMKeyNo == 0xFF) ? 0xFFFF : uMKeyNo;
    //转换密钥属性
    if(uKeyUse & PKU_CRYPT){
        dwKeyUse |= KEY_ATTR_DATA;
    }
    if(uKeyUse & PKU_FUNCTION){
        dwKeyUse |= KEY_ATTR_PIN;
    }
    if(uKeyUse & PKU_MACING){
        dwKeyUse |= KEY_ATTR_MAC;
    }
    if(uKeyUse & PKU_KEYENCKEY){
        dwKeyUse |= KEY_ATTR_ENC;
    }
    if(dwKeyUse == 0){
        Log(ThisModule, __LINE__, "无效Key用途, dwKeyUse=%u", dwKeyUse);
        return ERR_PARAM;
    }
    dwKeyUse |= KEY_ATTR_VECTOR_DECRYPT;

    WORD wValLen = strlen(lpKeyVal) / 2;
    if (wValLen != 16)
    {
        Log(ThisModule, __LINE__, "无效Key长度：wValLen=%d", wValLen);
        return ERR_PARAM;
    }

    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_SM4);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    UINT uLen;
    IMPORTSM4KEY stImportSm4Key;
    memset(&stImportSm4Key, 0, sizeof(stImportSm4Key));
    stImportSm4Key.wKeyId = EXWORDHL(wKeyNo);
    stImportSm4Key.wMKeyId = EXWORDHL(wEncKeyNo);
    stImportSm4Key.ulKeyAttr = RVSDWORD(dwKeyUse);
    uLen = sizeof(stImportSm4Key.byKeyValue);
    CAutoHex::Hex2Bcd(lpKeyVal, stImportSm4Key.byKeyValue, uLen);
    uLen = sizeof(stImportSm4Key) - (sizeof(stImportSm4Key.byKeyValue) - uLen);
    vtCmd.append((LPSTR)&stImportSm4Key, uLen);

    lRet = ExecuteCommand(0x24, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM工作密钥导入失败：lRet=%d[uKeyNo=%02X, uMKeyNo=%02X]", lRet, uKeyNo, uMKeyNo);
        return lRet;
    }

    lRet = ReadSymmKeyKCV(wKeyNo, byKcvMode, lpKCV, SYMMETRIC_KEY_MODE_SM4);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    Log(ThisModule, 1, "SM工作密钥导入成功:uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM4RemoteImportKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostSM2PKeyNo,
                                         UINT uKeyUse, LPCSTR lpUserId, LPCSTR lpCipherTextKeyVal,
                                         LPCSTR lpSignatureVal, LPSTR lpKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    if (uKeyNo >= DES_OR_SM4_KEY_MAX_NUM) {
        Log(ThisModule, __LINE__, "无效保存主密钥ID：uMKeyNo=%d", uKeyNo);
        return ERR_PARAM;
    }

    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_SM4);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "导入SM2加密的SM主密钥失败[切换对称密钥处理模式失败]：lRet=%d[uMKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    UINT uLen;

    // 下载密文主密钥
    BYTE byCmdCode = 0x4F;
    if(uHostSM2PKeyNo == 0xFF){                     //不需要验签
        IMPORTSM4KEYENCBYSM2_CFES stImportSm4KeyEncBySm2;
        ZeroMemory(&stImportSm4KeyEncBySm2, sizeof(stImportSm4KeyEncBySm2));
        stImportSm4KeyEncBySm2.wSM4KeyId = EXWORDHL(uKeyNo);
        stImportSm4KeyEncBySm2.wSM2SKeyId = EXWORDHL(uEppSKeyNo);
        stImportSm4KeyEncBySm2.bySymmKeyMode = SYMMETRIC_KEY_MODE_SM4;
        stImportSm4KeyEncBySm2.dwSM4KeyAttr = RVSDWORD(uKeyUse);
        uLen = sizeof(stImportSm4KeyEncBySm2.byEncryptedKeyValue);
        CAutoHex::Hex2Bcd(lpCipherTextKeyVal, stImportSm4KeyEncBySm2.byEncryptedKeyValue, uLen);
        stImportSm4KeyEncBySm2.wEncryptedKeyValueLen = EXWORDHL(uLen);

        vtCmd.append((LPSTR)&stImportSm4KeyEncBySm2,
                     sizeof(stImportSm4KeyEncBySm2) - (sizeof(stImportSm4KeyEncBySm2.byEncryptedKeyValue) - uLen));
    } else {
        byCmdCode = 0x4C;
        IMPORTSM4KEYENCBYSM2_VERSIGN_CFES stImportSm4KeyEncBySm2;
        memset(&stImportSm4KeyEncBySm2, 0, sizeof(stImportSm4KeyEncBySm2));
        stImportSm4KeyEncBySm2.wSM4KeyId = EXWORDHL(uKeyNo);
        stImportSm4KeyEncBySm2.wSM2SKeyId = EXWORDHL(uEppSKeyNo);
        stImportSm4KeyEncBySm2.wSignKeyId = EXWORDHL(uHostSM2PKeyNo);
        stImportSm4KeyEncBySm2.byDecryptFillAlgo = 0x01;        //不填充,固定值
        stImportSm4KeyEncBySm2.bySignFillAlgo = 0x01;           //不验签
        stImportSm4KeyEncBySm2.dwSM4KeyAttr = RVSDWORD(uKeyUse);
        stImportSm4KeyEncBySm2.bySymmKeyMode = SYMMETRIC_KEY_MODE_SM4;
        stImportSm4KeyEncBySm2.byRandomDataMode = 0x00;
        stImportSm4KeyEncBySm2.byAddMode = 0x00;
        stImportSm4KeyEncBySm2.wUserIdLen = EXWORDHL(strlen(lpUserId));
        //签名
        uLen = sizeof(stImportSm4KeyEncBySm2.bySignValue);
        CAutoHex::Hex2Bcd(lpSignatureVal, stImportSm4KeyEncBySm2.bySignValue, uLen);

        vtCmd.append((LPSTR)&stImportSm4KeyEncBySm2,
                     sizeof(stImportSm4KeyEncBySm2) - sizeof(stImportSm4KeyEncBySm2.byEncryptedKeyValue) -
                     sizeof(stImportSm4KeyEncBySm2.wEncryptedKeyValueLen) - sizeof(stImportSm4KeyEncBySm2.byUserId));
        //用户ID
        if(strlen(lpUserId) > 0){
            vtCmd.append(lpUserId);
        }

        //SM4密钥密文
        uLen = sizeof(stImportSm4KeyEncBySm2.byEncryptedKeyValue);
        CAutoHex::Hex2Bcd(lpCipherTextKeyVal, stImportSm4KeyEncBySm2.byEncryptedKeyValue, uLen);
        stImportSm4KeyEncBySm2.wEncryptedKeyValueLen = EXWORDHL(uLen);
        vtCmd.append((LPSTR)&stImportSm4KeyEncBySm2.wEncryptedKeyValueLen,
                     sizeof(stImportSm4KeyEncBySm2.wEncryptedKeyValueLen) + EXWORDHL(stImportSm4KeyEncBySm2.wEncryptedKeyValueLen));
    }

    lRet = ExecuteCommand(byCmdCode, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导入SM2加密的SM4主密钥失败：lRet=%d[uMKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    //获取国密密钥KCV
    lRet = ReadSymmKeyKCV(uKeyNo, 2, lpKCV, SYMMETRIC_KEY_MODE_SM4);
    if(lRet != ERR_SUCCESS) {
        Log(ThisModule, __LINE__, "获取国密密钥KCV失败:=%d[uMKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM4CryptData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, QByteArray &vtResultData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if (lpCryptData == nullptr || lpIVData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    WORD wAlgorithm = DATA_ALGO_ECB;
    WORD wOpMode = ENCRYPT_MODE_CFES;
    switch (uMode)
    {
    case ECB_EN:
        break;// ECB // Encryption
    case ECB_DE:
        wOpMode = DECRYPT_MODE_CFES;
        break;// ECB // Decryption
    case CBC_EN:
        wAlgorithm = DATA_ALGO_CBC;
        break;// CBC // Encryption
    case CBC_DE:
        wAlgorithm = DATA_ALGO_CBC;
        wOpMode = DECRYPT_MODE_CFES;
        break;// CBC // Decryption
    default:
        Log(ThisModule, __LINE__, "不支持Crypt模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_SM4);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "SM Data运算失败[切换对称密钥处理模式失败]：lRet=%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    QByteArray vtResult;    
#if 1
    QByteArray vtOperData;
    QByteArray vtInitVec;

    CAutoHex::Hex2Bcd(lpCryptData, vtOperData);
    if(DATA_ALGO_CBC == wAlgorithm){
        //处理初始向量
        if(lpIVData != nullptr && strlen(lpIVData) > 0 &&
           memcmp(lpIVData, "00000000000000000000000000000000", 32)){
            CAutoHex::Hex2Bcd(lpIVData, vtInitVec);
        }
    }

    while(vtOperData.size() > 0){
        int iBlockDataSize = qMin(vtOperData.size(), BLOCK_DATA_MAX_SIZE);

        //设置初始化向量
        if(DATA_ALGO_CBC == wAlgorithm){
            if(vtInitVec.size() > 0){
                lRet = SetInitVector(0xFFFF, vtInitVec);
                if(lRet != ERR_SUCCESS){
                    return lRet;
                }
            }
        }

        QByteArray vtBlockDataResult;
        lRet = DataOper(uKeyNo, wAlgorithm, wOpMode, bPadding, vtOperData, iBlockDataSize, vtBlockDataResult);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }

        //加解密结果合并
        vtResult.append(vtBlockDataResult);

        if(DATA_ALGO_CBC == wAlgorithm){
            //加解密结果最后16bytes作为下一次运算的向量
            vtInitVec = vtBlockDataResult.right(16);
        }
        vtOperData.remove(0, iBlockDataSize);
    }
#else
    QByteArray vtCmd;
    UINT uLen;
    //设置初始化向量
    if(wAlgorithm == DATA_ALGO_CBC){
        if(strlen(lpIVData) > 0){
            SETINITVEC_CFES stSetInitVec;
            ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
            stSetInitVec.wKeyId = 0xFFFF;
            uLen = sizeof(stSetInitVec.byVector);
            CAutoHex::Hex2Bcd(lpIVData, stSetInitVec.byVector, uLen);
            uLen = sizeof(stSetInitVec) - (sizeof(stSetInitVec.byVector) - uLen);
            vtCmd.append((LPSTR)&stSetInitVec, uLen);

            lRet = ExecuteCommand(0x29, vtCmd, vtResult);
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "SM Data运算设置初始化向量失败：lRet=%d[uKeyNo=%d]", lRet, uKeyNo);
                return lRet;
            }
        }
    }

    DATAOP stDataOp;
    ZeroMemory(&stDataOp, sizeof(stDataOp));
    stDataOp.wKeyId = EXWORDHL((WORD)uKeyNo);
    stDataOp.byOperate = wOpMode;
    stDataOp.wAlgorithm = EXWORDHL(wAlgorithm);
    stDataOp.byPadChar = bPadding;
    uLen = sizeof(stDataOp.byData);
    CAutoHex::Hex2Bcd(lpCryptData, stDataOp.byData, uLen);
    stDataOp.wDataLen = EXWORDHL((WORD)uLen);
    uLen = sizeof(stDataOp) - (sizeof(stDataOp.byData) - uLen);

    vtCmd.clear();
    vtCmd.append((LPSTR)&stDataOp, uLen);

    lRet = ExecuteCommand(0x2A, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM Data运算失败：lRet=%d[uKeyNo=%02X", lRet, uKeyNo);
        return lRet;
    }
#endif

    CAutoHex autoHex(vtResult);
    vtResultData.append(autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM4MACData(UINT uKeyNo, UINT uMacAlgo,
                                 LPCSTR lpMacData, LPCSTR lpIVData,
                                 QByteArray &vtResultData, BYTE byPad/* = 0xFF*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpMacData == nullptr || lpIVData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    WORD wAlgorithm = 0;
    switch (uMacAlgo)
    {
    case MAC_BANKSYS:
    case MAC_PBOC:
        //Todo:确认哪种算法为国密算法
        wAlgorithm = MAC_ALGO_X99T;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持MAC算法：uMacAlgo=%d", uMacAlgo);
        return ERR_NOT_SUPPORT;
    }

    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_SM4);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "SM Mac运算失败[切换对称密钥处理模式失败]：lRet=%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    LOGDEVACTION();
    QByteArray vtResult;

#if 1
    QByteArray vtOperData;
    QByteArray vtInitVector;
    CAutoHex::Hex2Bcd(lpMacData, vtOperData);
    if(lpIVData != nullptr && strlen(lpIVData) > 0 &&
       memcmp(lpIVData, "00000000000000000000000000000000", 32)){
        CAutoHex::Hex2Bcd(lpIVData, vtInitVector);
    }

    while(vtOperData.size() > 0){
        int iBlockDataSize = qMin(vtOperData.size(), BLOCK_DATA_MAX_SIZE);

        //设置初始化向量
        if(vtInitVector.size() > 0){
            lRet = SetInitVector(0xFFFF, vtInitVector);
            if(lRet != ERR_SUCCESS){
                return lRet;
            }
        }

        lRet = MacOper(uKeyNo, wAlgorithm, byPad, vtOperData, iBlockDataSize, vtResult);
        if(lRet != ERR_SUCCESS){
            return lRet;
        }

        //上次加密结果作为下次加密操作向量
        vtInitVector = vtResult;
        vtOperData.remove(0, iBlockDataSize);
    }

#else
    QByteArray vtCmd;
    UINT uLen;
    //设置初始化向量
    if(strlen(lpIVData) > 0){
        SETINITVEC_CFES stSetInitVec;
        ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
        //明文下载初始化向量
        stSetInitVec.wKeyId = EXWORDHL(0xFFFF);
        uLen = sizeof(stSetInitVec.byVector);
        CAutoHex::Hex2Bcd(lpIVData, stSetInitVec.byVector, uLen);
        uLen = sizeof(stSetInitVec) - (sizeof(stSetInitVec.byVector) - uLen);
        vtCmd.append((LPSTR)&stSetInitVec, uLen);

        lRet = ExecuteCommand(0x29, vtCmd, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "SM Mac运算设置初始化向量失败：lRet=%d[uKeyNo=%d]", lRet, uKeyNo);
            return lRet;
        }
    }

    MACOP_CFES stMacOp;
    ZeroMemory(&stMacOp, sizeof(stMacOp));
    stMacOp.wKeyId = EXWORDHL((WORD)uKeyNo);
    stMacOp.wAlgorithm = EXWORDHL(wAlgorithm);
    stMacOp.byPad = byPad;
    uLen = sizeof(stMacOp.byData);
    CAutoHex::Hex2Bcd(lpMacData, stMacOp.byData, uLen);
    stMacOp.wDataLen = EXWORDHL((WORD)uLen);
    uLen = sizeof(stMacOp) - (sizeof(stMacOp.byData) - uLen);

    vtCmd.clear();
    vtCmd.append((LPSTR)&stMacOp, uLen);

    lRet = ExecuteCommand(0x2B, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM Mac运算失败：lRet=%d[uKeyNo=%d", lRet, uKeyNo);
        return lRet;
    }
#endif
    CAutoHex autoHex(vtResult);
    vtResultData.append(autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM4PinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock)
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

    WORD wFmtType = 0;
    WORD wKeyNo = uKeyNo;
    switch (uFormat)
    {
    case PIN_SM4:
        wFmtType = PIN_FORMAT_ANSIX98;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持模式：%02X", uFormat);
        return ERR_NOT_SUPPORT;
    }

    long lRet = SwitchSymmetricKeyMode(SYMMETRIC_KEY_MODE_SM4);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "SM Pinblock失败[切换对称密钥处理模式失败]：lRet=%d", lRet);
        return lRet;
    }

    LOGDEVACTION();
    QByteArray vtCmd;
    QByteArray vtResult;

    PINBLOCK stPinBlock;
    ZeroMemory(&stPinBlock, sizeof(stPinBlock));
    stPinBlock.wKeyId = EXWORDHL(wKeyNo);
    stPinBlock.wFormat = EXWORDHL(wFmtType);
    stPinBlock.byPadChar = bPadding;
    stPinBlock.byCustomerDataLen = strlen(lpCustomerData);
    strcpy((char *)stPinBlock.byCustomerData, (char *)lpCustomerData);
    uLen = sizeof(stPinBlock) - (sizeof(stPinBlock.byCustomerData) - uLen);

    vtCmd.append((LPSTR)&stPinBlock, uLen);

    lRet = ExecuteCommand(0x28, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM 读取密码块失败：%d[nKeyNo=%d, uFormat=%02X]", lRet, uKeyNo, uFormat);
        return lRet;
    }

    CAutoHex autoHex(vtResult);
    strcpy(lpPinBlock, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM2ImportKey(UINT uKeyNo, UINT uVendorPKeyNo, BOOL bPublicKey, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM2EncryptData(UINT uPKeyNo, LPCSTR lpData, LPSTR lpCryptData)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM2DecryptData(UINT uSKeyNo, LPCSTR lpCryptData, LPSTR lpData)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM2SignData(UINT uSKeyNo, LPCSTR lpZA, LPCSTR lpData, LPSTR lpSignData)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM2VerifySign(UINT uPKeyNo, LPCSTR lpZA, LPCSTR lpData, LPCSTR lpSignData)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM2ExportPKey(UINT uKeyNo, UINT uSignKeyNo, LPSTR lpZA, LPSTR lpSM2PKeyData, LPSTR lpSignData)
{
    return ERR_SUCCESS;
}

/*
 * @func: Generate sm2 key pair.
 * @param:uSKeyNo: Public and Private key id
 *        uPKeyNo: no use
 */
long CDevPIN_CFESM01::SM2GenerateKey(UINT uSKeyNo, UINT uPKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    //检查密钥索引(0~15)
    if(uSKeyNo >= RSA_OR_SM2_KEY_MAX_NUM){
        Log(ThisModule, __LINE__, "无效密钥ID：uKeyNo=%d", uSKeyNo);
        return ERR_PARAM;
    }

    IMPORTSM2KEYPAIR stImportSm2KeyPair;
    ZeroMemory(&stImportSm2KeyPair, sizeof(stImportSm2KeyPair));
    stImportSm2KeyPair.wKeyId = EXWORDHL(uSKeyNo);
    stImportSm2KeyPair.byKeyAttr = 0x01;                //加解密属性
    vtCmd.append((LPSTR)&stImportSm2KeyPair, sizeof(stImportSm2KeyPair));
    long lRet = ExecuteCommand(0x40, vtCmd, vtResult);   //生成SM2密钥对
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM2密钥对生成失败：lRet=%d[uSKeyNo=%d]", lRet, uSKeyNo);
        return lRet;
    }
    Log(ThisModule, 1, "SM2密钥对导入成功:uSKeyNo=%d", uSKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM3CaculateZAData(LPCSTR lpUserData, UINT uPKNum, QByteArray &resultData)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SMDeleteKey(UINT uKeyNo)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM4SaveKeyType(UINT uType)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SM2ExportPKeyForGD(UINT uKeyNo, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    long lRet = ERR_SUCCESS;

    EXPORTSM2EPPSIGNKEY stExportSM2EppSignKey;
    ZeroMemory(&stExportSM2EppSignKey, sizeof(stExportSM2EppSignKey));
    stExportSM2EppSignKey.wKeyId = EXWORDHL(uKeyNo);
//    stExportSM2EppSignKey.byKeyAttr = 0x01;             //加解密属性
    vtCmd.append((LPSTR)&stExportSM2EppSignKey, sizeof(stExportSM2EppSignKey.wKeyId));
    lRet = ExecuteCommand(0x49, vtCmd, vtResult);   //导出SM2公钥
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导出SM2公钥失败：lRet=%d", lRet);
        return lRet;
    }

    //转换为HEX字符串
    CAutoHex autoHex((LPBYTE)vtResult.constData() + 2, 64);
    strcpy(lpData, autoHex.GetHex());
    Log(ThisModule, __LINE__, "SM2 PK Data:%s", lpData);

    Log(ThisModule, 1, "导出SM2公钥成功:lRet=%d ", lRet);
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAInitStatus()
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAImportRootHostPK(QByteArray strHostPK, QByteArray strSignHostPK)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAImportRootHostPK(UINT uPKeyNo, UINT uSignKeyNo, int iSignAlgo, QByteArray strHostPKData, QByteArray strSignData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtCmd;
    QByteArray vtResult;
    QByteArray vtKeyData;
    QByteArray vtSignData;

    DWORD dwSignAlgo = RSA_SIGN_ALGO_NO;
    switch(iSignAlgo){
    case RSA_SIGN_SSA_PKCS1_V1_5:
        dwSignAlgo = RSA_SIGN_ALGO_PKCS_1_V1_5;
        break;
    case RSA_SIGN_SSA_PSS:
        dwSignAlgo = RSA_SIGN_ALGO_PKCS_1_PSS;
        break;
    default :
        Log(ThisModule, __LINE__, "不支持的RSA签名算法:%d", iSignAlgo);
        break;
    }

    //HEX to BIN
    CAutoHex::Hex2Bcd(strHostPKData.constData(), vtKeyData);
    CAutoHex::Hex2Bcd(strSignData.constData(), vtSignData);

    IMPORTHOSTPKEY_CFES stImportHostPKey;
    ZeroMemory(&stImportHostPKey, sizeof(stImportHostPKey));
    stImportHostPKey.byKeyId = uPKeyNo;
    stImportHostPKey.byVerifySignatureKeyId = uSignKeyNo;
    BOOL bNeedDerEncode = FALSE;
    int iOffset = 0;
    if(bNeedDerEncode){
        //公钥数据DER编码
        BYTE byBuffer[1024] = {0};
        //Sequence Tag
        BYTE bySeqTag[4] = {0};
        WORD wSeqTagLen = 4;
        WORD wTotalDataLen = 0;

        //Module Tag
        WORD wModTagLen = 4;                        //Modulus Tag Len
        BYTE byModTag[4] = {0};                     //Modulus Tag
        WORD wModLen = vtKeyData.size();            //Modulus Len (With prefixed 0x00)
        //如果Module数据的第一byte的最高bit为0，需要在数据整体前前置0,变为正数
        if(vtKeyData.at(0) & 0x80){
            wModLen += 1;
            vtKeyData.prepend((char)0x00);
        }

        //Exponent Tag
        WORD wExpTagLen = 4;                        //Exponent Tag Len
        BYTE byExpTag[4] = {0};                     //Exponent Tag
        char szExpData[4] = "\x01\x00\x01";         //Exponent Data
        WORD wExpDataLen = 3;                       //Exponent Data	Len

        BuildAsn1Tag(ASN1_INTEGER, wModLen, &wModTagLen, byModTag);
        BuildAsn1Tag(ASN1_INTEGER, wExpDataLen, &wExpTagLen, byExpTag);
        wTotalDataLen = wModTagLen + wModLen + wExpTagLen + wExpDataLen;
        BuildAsn1Tag(ASN1_SEQUENCE, wTotalDataLen, &wSeqTagLen, bySeqTag);

        //Seq tag
        memcpy(byBuffer, bySeqTag, wSeqTagLen);
        iOffset += wSeqTagLen;
        //Module tag
        memcpy(byBuffer + iOffset, byModTag, wModTagLen);
        iOffset += wModTagLen;
        memcpy(byBuffer + iOffset, (LPSTR)vtKeyData.constData(), vtKeyData.size());
        iOffset += vtKeyData.size();
        //Exponent tag
        memcpy(byBuffer + iOffset, byExpTag, wExpTagLen);
        iOffset += wExpTagLen;
        memcpy(byBuffer + iOffset, szExpData, wExpDataLen);
        iOffset += wExpDataLen;

        stImportHostPKey.wPKeyDataDERLen = EXWORDHL(iOffset);
        memcpy(stImportHostPKey.byPKeyDataDER, byBuffer, iOffset);
    } else {
        iOffset = vtKeyData.size();
        stImportHostPKey.wPKeyDataDERLen = EXWORDHL(iOffset);
        if(iOffset > sizeof(stImportHostPKey.byPKeyDataDER)){
            Log(ThisModule, __LINE__, "DER编码公钥数据超过最大长度%d:%d", sizeof(stImportHostPKey.byPKeyDataDER), iOffset);
            return ERR_PARAM;
        }

        memcpy(stImportHostPKey.byPKeyDataDER, vtKeyData.constData(), iOffset);
    }

    stImportHostPKey.dwKeyAttr = RVSDWORD(RSA_ATTR_PK_ENCRYPT|RSA_ATTR_PK_VERIFY_SIGNATURE);
    stImportHostPKey.dwSignFillAlgo = RVSDWORD(dwSignAlgo);
    stImportHostPKey.dwSignHashAlgo = RVSDWORD(m_iRSARKLVerifySignatureHashAlgo);
    stImportHostPKey.wSignValueLen = EXWORDHL(vtSignData.size());
    if(vtSignData.size() > 0){
        memcpy(stImportHostPKey.bySignValue, (LPSTR)vtSignData.constData(), vtSignData.size());
    }

    vtCmd.append((LPSTR)&stImportHostPKey, 4 + iOffset);
    vtCmd.append((LPSTR)&stImportHostPKey.dwKeyAttr,
                 sizeof(stImportHostPKey) - 4 - sizeof(stImportHostPKey.byPKeyDataDER) -
                 (sizeof(stImportHostPKey.bySignValue) - vtSignData.size()));

    long lRet = ExecuteCommand(0x72, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载主机RSA公钥命令失败：lRet=%d", lRet);
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAImportSignHostPK(QByteArray strHostPK, QByteArray strSignHostPK)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAExportEppEncryptPK(QByteArray &strPK, QByteArray &strSignPK)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAExportEppEncryptPK(UINT uEppPKeyNo, UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo, QByteArray &strKeyVal, QByteArray &strSignVal)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.push_back((char)uEppPKeyNo);

    long lRet = ExecuteCommand(0x70, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导出RSA加密公钥信息失败：lRet=%d", lRet);
        return lRet;
    }

    //返回数据解析
    //拷贝公钥数据
    WORD wPKeyDataDERLen = MAKEWORD(vtResult.at(9), vtResult.at(8));
    if(wPKeyDataDERLen > 0){
//        BYTE byTotalData[1024] = {0};
//        WORD wTotalDataLen = sizeof(byTotalData);
//        BYTE byRSAKeyData[512] = {0};
//        WORD wRSAKeyDataLen = sizeof(byRSAKeyData);
//        BYTE byModulus[257] = {0};
//        WORD wModulusLen = sizeof(byModulus);
//        BYTE bySignature[256] = {0};
//        WORD wSignatureLen = sizeof(bySignature);
//        WORD wSeqTagLen, wRSAKeyTagLen, wModulusTagLen, wSignatureTagLen = 0;

//        LPBYTE lpResultData = (LPBYTE)(vtResult.constData() + 10);
        CAutoHex::Bcd2Hex((LPBYTE)vtResult.constData() + 10, wPKeyDataDERLen, strKeyVal);

//        lRet = ExtractAsn1Data(wPKeyDataDERLen, lpResultData, ASN1_SEQUENCE,
//                &wSeqTagLen, byTotalData, &wTotalDataLen);
//        if (lRet == 0) {
//            lRet = ExtractAsn1Data( wTotalDataLen, byTotalData, ASN1_INTEGER,
//                                         &wModulusTagLen, byModulus, &wModulusLen);

//            //Get Kkey data by ANS1 Algorithm (Without Tag)
//            lRet = ExtractAsn1Data(wTotalDataLen, byTotalData, ASN1_SEQUENCE,
//                                        &wRSAKeyTagLen, byRSAKeyData, &wRSAKeyDataLen);
//            if (lRet == 0) {
//                //Get RSA Key Data by ANS1 Algorithm (Without Tag)
//                lRet = ExtractAsn1Data( wRSAKeyDataLen, byRSAKeyData, ASN1_INTEGER,
//                                             &wModulusTagLen, byModulus, &wModulusLen);
//                if (lRet == 0) {
//                    //Get Signature by ANS1 Algorithm (Without Tag)
//                    lRet = ExtractAsn1Data(wKeyDataLen - wRSAKeyTagLen - wRSAKeyDataLen,
//                                                byKeyData + wRSAKeyTagLen + wRSAKeyDataLen,	ASN1_INTEGER,
//                                                &wSignatureTagLen, bySignature, &wSignatureLen);
//                }
//            }
//        }

//        if(lRet == 0){
//            BOOL bPublicKeyValueType = TRUE;
//            if (bPublicKeyValueType == TRUE) {
//                CAutoHex::Bcd2Hex(&(byModulus[1]), wModulusLen - 1, strKeyVal);
//            } else {
//                CAutoHex::Bcd2Hex(byTotalData, wTotalDataLen, strKeyVal);
//            }
//        } else {
//            //解析失败，返回结果为空
//        }
    }

    eRSASignAlgo = RSA_SIGN_NO;
    switch(vtResult.at(3)){
    case RSA_SIGN_ALGO_PKCS_1_PSS:
        eRSASignAlgo = RSA_SIGN_SSA_PSS;
        break;
    case RSA_SIGN_ALGO_PKCS_1_V1_5:
        eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
        break;
    default :
        break;
    }
    //拷贝签名数据
    if(eRSASignAlgo != RSA_SIGN_NO){
        WORD wSignLen = MAKEWORD(vtResult.at(10 + wPKeyDataDERLen + 1), vtResult.at(10 + wPKeyDataDERLen));
        CAutoHex::Bcd2Hex((LPBYTE)vtResult.constData() + 12 + wPKeyDataDERLen, wSignLen, strSignVal);
    }

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAExportEppSN(QByteArray &strSN, QByteArray &strSignSN)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAExportEppSN(UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo, QByteArray &strSN, QByteArray &strSignSN)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    long lRet = ExecuteCommand(0x71, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "导出EPPID信息失败：lRet=%d", lRet);
        return lRet;
    }

    //返回数据解析
    eRSASignAlgo = RSA_SIGN_NO;
    switch(vtResult.at(3)){
    case RSA_SIGN_ALGO_PKCS_1_PSS:
        eRSASignAlgo = RSA_SIGN_SSA_PSS;
        break;
    case RSA_SIGN_ALGO_PKCS_1_V1_5:
        eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
        break;
    default :
        break;
    }

    //拷贝EPPID数据
    WORD wLen = MAKEWORD(vtResult.at(9), vtResult.at(8));
    if(wLen > 0){
        CAutoHex::Bcd2Hex((LPBYTE)vtResult.constData() + 10, wLen, strSN);
    }

    //拷贝签名数据
    if(eRSASignAlgo != RSA_SIGN_NO){
        WORD wSignatureLen = MAKEWORD(vtResult.at(10 + wLen + 1), vtResult.at(10 + wLen));
        if(wSignatureLen > 0){
            CAutoHex::Bcd2Hex((LPBYTE)vtResult.constData() + 12 + wSignatureLen, wSignatureLen, strSignSN);
        }
    }

    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAExportEppRandData(QByteArray &strRandData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;
    RANDOM_CFES stRandom;
    stRandom.byUseType = 0x02;
    stRandom.wRandomDataLen = EXWORDHL(0x08);
    vtCmd.append((LPSTR)&stRandom, sizeof(stRandom));

    long lRet = ExecuteCommand(0xB2, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取RKL用8位长度随机数失败：%d", lRet);
        return lRet;
    }

    //Bcd to hex
    CAutoHex::Bcd2Hex((LPBYTE)vtResult.constData(), vtResult.size(), strRandData);
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAImportDesKey(UINT uMKeyNo, QByteArray strMKVal, QByteArray strSignMKVal, QByteArray &strKCV)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAImportDesKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostPKeyNo, RSADATAALGOTYPE eRSAEncAlgo, RSASIGNALGOTYPE eRSASignAlgo,
                                      QByteArray strKeyVal, QByteArray strSignVal, QByteArray &strKCV, bool bUseRandom,
                                      int iSymmKeyType/* = 0*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    strKCV.clear();
    QByteArray vtResult;
    QByteArray vtCmd;

    IMPORTSYMMKEYENCBYRSA_CFES stImportSymmKeyEncByRSA;
    ZeroMemory(&stImportSymmKeyEncByRSA, sizeof(stImportSymmKeyEncByRSA));
    stImportSymmKeyEncByRSA.wSymmKeyId = EXWORDHL(uKeyNo);
    stImportSymmKeyEncByRSA.byDecryptKeyId = uEppSKeyNo;
    stImportSymmKeyEncByRSA.byVerifySigntureKeyId = uHostPKeyNo;

    DWORD dwDecryptAlgo;
    //RSA加密算法
    switch(eRSAEncAlgo){
    case RSA_ENC_PKCS1_V1_5:
        dwDecryptAlgo = 0x01;
        break;
    case RSA_ENC_SHA_1:
        dwDecryptAlgo = 0x02;
        break;
    case RSA_ENC_SHA_256:
    default:
        Log(ThisModule, __LINE__, "不支持的RSA解密算法：%d", eRSAEncAlgo);
        return ERR_PARAM;
        break;
    }
    stImportSymmKeyEncByRSA.dwDecryptFillAlgo = RVSDWORD(dwDecryptAlgo);

    //RSA签名算法
    DWORD dwSignAlgo = RSA_SIGN_ALGO_NO;
    switch(eRSASignAlgo){
    case RSA_SIGN_NO:
        dwSignAlgo = RSA_SIGN_ALGO_NO;
        break;
    case RSA_SIGN_SSA_PKCS1_V1_5:
        dwSignAlgo = RSA_SIGN_ALGO_PKCS_1_V1_5;
        break;
    case RSA_SIGN_SSA_PSS:
        dwSignAlgo = RSA_SIGN_ALGO_PKCS_1_PSS;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持的RSA签名算法：%d", eRSASignAlgo);
        return ERR_PARAM;
        break;
    }

    stImportSymmKeyEncByRSA.dwSignFillAlgo = RVSDWORD(dwSignAlgo);
    stImportSymmKeyEncByRSA.dwDecryptHashAlgo = RVSDWORD(m_iRSARKLDecryptHashAlgo);
    stImportSymmKeyEncByRSA.dwSignHashAlgo = RVSDWORD(m_iRSARKLVerifySignatureHashAlgo);
    stImportSymmKeyEncByRSA.dwSymmKeyAttr = RVSDWORD(KEY_ATTR_ENC);
    stImportSymmKeyEncByRSA.bySymmKeyMode = iSymmKeyType == 0 ? SYMMETRIC_KEY_MODE_DES : SYMMETRIC_KEY_MODE_SM4;
    stImportSymmKeyEncByRSA.byRandomDataMode = bUseRandom ? 0x01 : 0x00;
    stImportSymmKeyEncByRSA.byAddMode = 0x00;
    UINT uSignatureLen = 0;
    UINT uLen = sizeof(stImportSymmKeyEncByRSA.byEncryptedSymmKeyValue);
    CAutoHex::Hex2Bcd(strKeyVal.constData(), stImportSymmKeyEncByRSA.byEncryptedSymmKeyValue, uLen);
    stImportSymmKeyEncByRSA.wEncryptedSymmKeyValueLen = EXWORDHL(uLen);
    vtCmd.append((char *)&stImportSymmKeyEncByRSA, sizeof(stImportSymmKeyEncByRSA) - 2 - sizeof(stImportSymmKeyEncByRSA.bySignature) - sizeof(stImportSymmKeyEncByRSA.byEncryptedSymmKeyValue) + uLen);
    if(strSignVal.size() > 0){
        uSignatureLen = sizeof(stImportSymmKeyEncByRSA.bySignature);
        CAutoHex::Hex2Bcd(strSignVal.constData(), stImportSymmKeyEncByRSA.bySignature, uSignatureLen);
        stImportSymmKeyEncByRSA.wSignatureLen = EXWORDHL(uSignatureLen);
    }
    vtCmd.append((char *)&stImportSymmKeyEncByRSA.wSignatureLen, 2 + uSignatureLen);

    long lRet = ExecuteCommand(0x73, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "远程下载DES KEY命令失败：lRet=%d", lRet);
        return lRet;
    }

    //读取密钥KCV
    BYTE byKcv[64] = {0};
    lRet = ReadSymmKeyKCV(uKeyNo, 2, (LPSTR)byKcv,
                          iSymmKeyType == 0 ? SYMMETRIC_KEY_MODE_DES : SYMMETRIC_KEY_MODE_SM4);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    strKCV.append((LPSTR)byKcv);
    Log(ThisModule, 1, "下载DESKEY成功[KCV=%s]", strKCV.constData());
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAGenerateKeyPair(UINT uPKeyNo, UINT uSKeyNo, int iModuleLen, int iExponentValue)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    GENERATERASKEYPAIR_CFES stGenerateRsaKeyPair;
    ZeroMemory(&stGenerateRsaKeyPair, sizeof(stGenerateRsaKeyPair));
    stGenerateRsaKeyPair.byKeyId = uPKeyNo;                 //公私钥id相同
    stGenerateRsaKeyPair.dwKeyLen = RVSDWORD(iModuleLen);
    stGenerateRsaKeyPair.dwExponent = RVSDWORD(iExponentValue);
    stGenerateRsaKeyPair.dwKeyAttr = RVSDWORD(RSA_ATTR_PK_ENCRYPT|RSA_ATTR_SK_DENCRYPT);
    vtCmd.append((LPSTR)&stGenerateRsaKeyPair, sizeof(stGenerateRsaKeyPair));

    long lRet = ExecuteCommand(0x77, vtCmd, vtResult, 20000);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下载RSA密钥对命令失败：lRet=%d, SKeyNo:%d, PKeyNo:%d",
            lRet, uSKeyNo, uPKeyNo);
        return lRet;
    }

//  for test
/*
    QByteArray data = "111111111111111122222222111111111111111122222222111111111111111122222222";
    QByteArray result;
    lRet = RSAEncryptData(uPKeyNo, RSA_ENC_PKCS1_V1_5, data, result);
    if(lRet == ERR_SUCCESS){
        char szTemp[1024] = {0};
        for(int i = 0; i < result.size(); i++){
            sprintf(szTemp + i * 3, "%02X ", (int)result.at(i));
        }
        Log(ThisModule, __LINE__, "RSA加密数据：%s", szTemp);
    }
*/
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSAEncryptData(UINT uRSAPKeyNo, RSADATAALGOTYPE eRSAEncAlgo, QByteArray data, QByteArray &result)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::RSADecryptData(UINT uRSASKeyNo, RSADATAALGOTYPE eRSAEncAlgo, QByteArray data, QByteArray &result)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SetDesKeyLenInfo(std::map<UINT, BYTE> &keyIdMapKeyLen)
{
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::PackCmd(BYTE byCmdCode, const QByteArray &vtCmdDataPart, QByteArray &vtCmd)
{
    long lRet = ERR_SUCCESS;

    //计算CRC
    WORD wCRC = GetCRC((LPBYTE)vtCmdDataPart.constData(), vtCmdDataPart.size());

    //STX
    vtCmd.append((char)HIBYTE(M01_SEND_PKG_STX));
    vtCmd.append((char)LOBYTE(M01_SEND_PKG_STX));

    //CMD
    vtCmd.append(byCmdCode);

    //LEN
    vtCmd.append((char)HIBYTE(vtCmdDataPart.size()));
    vtCmd.append((char)LOBYTE(vtCmdDataPart.size()));

    //DATA
    vtCmd.append(vtCmdDataPart);

    //CRC
    vtCmd.append((char)HIBYTE(wCRC));
    vtCmd.append((char)LOBYTE(wCRC));

    //ETX
    vtCmd.append(M01_SEND_PKG_ETX);

    return lRet;
}

long CDevPIN_CFESM01::UnPackCmd(QByteArray &vtRecvData, QByteArray &vtRespData)
{
    THISMODULE(__FUNCTION__);

    long lRet = ERR_SUCCESS;

    int    iSTXPos = -1;                          //应答包包头位置
    int    iDataPartLen = 0;                      //应答包中数据部分长度

    //从应答数据中解析出应答包
    for(int i = 0; i < vtRecvData.size(); i++){
        //查找包头
        if(vtRecvData.at(i) == HIBYTE(M01_ACK_PKG_STX)){
            if(i + 1 < vtRecvData.size()){
                if(vtRecvData.at(i + 1) == LOBYTE(M01_ACK_PKG_STX)){
                    iSTXPos = i++;
                    continue;
                }
            }
        }

        //查找包尾
        if(iSTXPos != -1){
            if(i == iSTXPos + 5){
                iDataPartLen = MAKEWORD(vtRecvData.at(i), vtRecvData.at(i - 1));
                int iETXPos = i + iDataPartLen + 3;
                if(iETXPos < vtRecvData.size()){
                    if(vtRecvData.at(iETXPos) == M01_ACK_PKG_ETX){
                        break;
                    } else {
                        //从包头标志后重新查找包头
                        i = iSTXPos;
                        iSTXPos = -1;
                    }
                } else {
                    Log(ThisModule, __LINE__, "应答包格式错误：未找到包尾标志");
                    iSTXPos = -1;
                }
            }
        }
    }

    if(iSTXPos == -1){
        Log(ThisModule, __LINE__, "应答包数据格式不正确");
        return ERR_CMD_DATA;
    }

    QByteArray vtValidRecvData;
    vtValidRecvData.append(vtRecvData.constData() + iSTXPos, iDataPartLen + 9);

    //校验CRC
    WORD wCRC = GetCRC((LPBYTE)vtValidRecvData.constData() + 6, iDataPartLen);
    WORD wCRCRet = MAKEWORD(vtValidRecvData.at(vtValidRecvData.size() - 2), vtValidRecvData.at(vtValidRecvData.size() - 3));
    if(wCRC != wCRCRet){
        Log(ThisModule, __LINE__, "返回数据CRC值校验错误");
        return ERR_CMD_DATA;
    }

    //检查命令执行结果码
    BYTE byRetCode = vtValidRecvData.at(3);
    if(byRetCode != 0x00){
        //更新错误码
        if(byRetCode != M01_ERR_NO_VALID_PIN_CODE){
            UpdateStatus(DEVICE_HWERROR, byRetCode, 1);
        }
        return ConvertErrCode(byRetCode, ThisModule);
    }

    if(vtValidRecvData.at(2) != 0xB5){
        UpdateStatus(DEVICE_ONLINE, 0);
    }

    //拷贝命令结果数据
    vtRespData.append(vtValidRecvData.constData() + 6, iDataPartLen);

    return lRet;
}

WORD CDevPIN_CFESM01::GetCRC(const BYTE *lpData, DWORD dwSize)
{
    if(lpData == nullptr || dwSize == 0){
        return 0xFFFF;
    }

    WORD wCRCData = 0x8006;
    WORD wCRC = 0xFFFF;

    while(dwSize--){
        for(int i = 0x80; i != 0; i >>= 1){
            if((wCRC & 0x8000) != 0){
                wCRC <<= 1;
                wCRC ^= wCRCData;
            } else {
                wCRC <<= 1;
            }

            if((*lpData & i) != 0){
                wCRC ^= wCRCData;
            }
        }

        lpData++;
    }

    return wCRC;
}

long CDevPIN_CFESM01::ExecuteCommand(BYTE byCmdCode, const QByteArray &vtCmdDataPart, QByteArray &vtResult, DWORD dwTimeOut)
{
    long lRet = ERR_SUCCESS;

    QByteArray vtCmd;
    QByteArray vtRecvData;
    QByteArray vtRespData;
    //编辑命令
    PackCmd(byCmdCode, vtCmdDataPart, vtCmd);

    //发送命令并接收返回数据
    lRet = SendAndReadData(vtCmd, vtRecvData);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    //解析命令返回数据
    lRet = UnPackCmd(vtRecvData, vtRespData);
    if(vtRespData.size() > 0){
        vtResult = vtRespData;
    }

    return lRet;
}

long CDevPIN_CFESM01::SendAndReadData(QByteArray &vtSendData, QByteArray &vtRecvData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    long lRet = ERR_SUCCESS;

    //发送数据
    int iPadCharCount = M01_USB_PACKET_SIZE - vtSendData.size() % M01_USB_PACKET_SIZE;
    //发送数据为64的整倍数,不足补0
    if(iPadCharCount < M01_USB_PACKET_SIZE){
        vtSendData.append(iPadCharCount, (char)0x00);
    }

    lRet = m_pDev->Send(vtSendData.constData(), vtSendData.size(), M01_WRITE_TIMEOUT);
    if(lRet != 0){
        Log(ThisModule, __LINE__, "数据发送失败：%d", lRet);
        return lRet;
    }

//    BYTE byPacket[M01_USB_PACKET_SIZE] = {0};
//    int iPacketCount = (vtSendData.size() % M01_USB_PACKET_SIZE) ? (vtSendData.size() / M01_USB_PACKET_SIZE) + 1 :
//                                                  (vtSendData.size() / M01_USB_PACKET_SIZE);
//    LPCSTR lpSendData = vtSendData.constData();
//    for(int i = 0; i < iPacketCount; i++){
//        memset(byPacket, 0, sizeof(byPacket));
//        int iPackDataLen = (i == (iPacketCount - 1)) ? vtSendData.size() - i * M01_USB_PACKET_SIZE : M01_USB_PACKET_SIZE;
//        memcpy(byPacket, lpSendData + i * 64, iPackDataLen);

//        long lRet = m_pDev->Send((LPCSTR)byPacket, M01_USB_PACKET_SIZE, M01_WRITE_TIMEOUT);
//        if(lRet != 0){
//            Log(ThisModule, __LINE__, "数据发送失败：%d", lRet);
//            return lRet;
//        }
//    }

    //接收数据
    BYTE byPacket[M01_USB_PACKET_SIZE] = {0};
    ULONG ulStartTime = CQtTime::GetSysTick();
    while(true){
        memset(byPacket, 0, sizeof(byPacket));
        DWORD dwInOutDataLen = sizeof(byPacket);
        lRet = m_pDev->Read((LPSTR)byPacket, dwInOutDataLen, M01_READ_TIMEOUT);
        if(lRet != 0){
            //出错
            Log(ThisModule, __LINE__, "读返回数据出错：%d", lRet);
            break;
        }

        vtRecvData.append((LPCSTR)byPacket, dwInOutDataLen);
        //判断数据是否接收完整
        if(vtRecvData.size() >= 6){
            WORD wDataLen = MAKEWORD(vtRecvData.at(5), vtRecvData.at(4));
            if(vtRecvData.size() >= wDataLen + 9){
                break;
            }
        }

        //检查是否超时
        if(CQtTime::GetSysTick() - ulStartTime > dwTimeOut){
            Log(ThisModule, __LINE__, "接收返回数据超时");
            return ERR_DEVPORT_RTIMEOUT;
        }
    }

    return lRet;
}

long CDevPIN_CFESM01::ConvertErrCode(BYTE byRTN, LPCSTR ThisModule)
{
    long lRet;
    auto it = m_mapErrCode.find(byRTN);
    if (it != m_mapErrCode.end())
    {
        Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][%s]", it->first, it->second.c_str());
        switch (byRTN) {
        case M01_ERR_NO_VALID_PIN_CODE:
            lRet = ERR_NO_PIN;
            break;
        default:
            lRet = byRTN;
            break;
        }
    } else {
        if(byRTN >= 0xD0 && byRTN <= 0xDF){
            Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][M01_ERR_ALGO_LIB_CALC]", byRTN);
            lRet = byRTN;
        } else {
            Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][未知错误]", byRTN);
            lRet = ERR_OTHER;
        }
    }

    return lRet;
}

void CDevPIN_CFESM01::InitErrCode()
{
    m_mapErrCode[M01_ERR_NO_PACKET_EXT] = MACROTOSTR(M01_ERR_NO_PACKET_EXT);
    m_mapErrCode[M01_ERR_VERIFY_CRC] = MACROTOSTR(M01_ERR_VERIFY_CRC);
    m_mapErrCode[M01_ERR_NO_CMD] = MACROTOSTR(M01_ERR_NO_CMD);
    m_mapErrCode[M01_ERR_UNKNOWN] = MACROTOSTR(M01_ERR_UNKNOWN);
    m_mapErrCode[M01_ERR_REMOVE_SWITCH_NO_PRESSED] = MACROTOSTR(M01_ERR_REMOVE_SWITCH_NO_PRESSED);
    m_mapErrCode[M01_ERR_KEY_HOLD] = MACROTOSTR(M01_ERR_KEY_HOLD);
    m_mapErrCode[M01_ERR_CMD_UNSUPPORT] = MACROTOSTR(M01_ERR_CMD_UNSUPPORT);
    m_mapErrCode[M01_ERR_CMD_NOT_COMPLETE] = MACROTOSTR(M01_ERR_CMD_NOT_COMPLETE);
    m_mapErrCode[M01_ERR_CMD_LEN] = MACROTOSTR(M01_ERR_CMD_LEN);
    m_mapErrCode[M01_ERR_CMD_PARAMETER] = MACROTOSTR(M01_ERR_CMD_PARAMETER);
    m_mapErrCode[M01_ERR_CMD_EXEC_TOO_FREQUENTLY] = MACROTOSTR(M01_ERR_CMD_EXEC_TOO_FREQUENTLY);
    m_mapErrCode[M01_ERR_CMD_SEQ] = MACROTOSTR(M01_ERR_CMD_SEQ);
    m_mapErrCode[M01_ERR_DEVICE_LOCKED] = MACROTOSTR(M01_ERR_DEVICE_LOCKED);
    m_mapErrCode[M01_ERR_CMD_LOCKED] = MACROTOSTR(M01_ERR_CMD_LOCKED);
    m_mapErrCode[M01_ERR_CMD_FUNC_NOT_EMPOWER] = MACROTOSTR(M01_ERR_CMD_FUNC_NOT_EMPOWER);
    m_mapErrCode[M01_ERR_UNKNOWN_FIELD_DATA] = MACROTOSTR(M01_ERR_UNKNOWN_FIELD_DATA);
    m_mapErrCode[M01_ERR_FIELD1_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD1_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD2_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD2_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD3_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD3_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD4_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD4_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD5_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD5_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD6_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD6_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD7_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD7_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD8_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD8_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD9_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD9_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD10_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD10_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD11_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD11_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD12_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD12_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD13_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD13_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_FIELD14_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_FIELD14_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_OTHER_FIELD_VALUE_ILLEGAL] = MACROTOSTR(M01_ERR_OTHER_FIELD_VALUE_ILLEGAL);
    m_mapErrCode[M01_ERR_SHIFT_POSITION] = MACROTOSTR(M01_ERR_SHIFT_POSITION);
    m_mapErrCode[M01_ERR_SERIAL_NUMBER_EXIST] = MACROTOSTR(M01_ERR_SERIAL_NUMBER_EXIST);
    m_mapErrCode[M01_ERR_SERIAL_NUMBER_NOT_EXIST] = MACROTOSTR(M01_ERR_SERIAL_NUMBER_NOT_EXIST);
    m_mapErrCode[M01_ERR_PASSWORD_EXIST] = MACROTOSTR(M01_ERR_PASSWORD_EXIST);
    m_mapErrCode[M01_ERR_PASSWORD_NOT_EXIST] = MACROTOSTR(M01_ERR_PASSWORD_NOT_EXIST);
    m_mapErrCode[M01_ERR_NOT_VERIFY_PASSWORD] = MACROTOSTR(M01_ERR_NOT_VERIFY_PASSWORD);
    m_mapErrCode[M01_ERR_PASSWORD_ERROR] = MACROTOSTR(M01_ERR_PASSWORD_ERROR);
    m_mapErrCode[M01_ERR_TWO_PASSWORD_SAME] = MACROTOSTR(M01_ERR_TWO_PASSWORD_SAME);
    m_mapErrCode[M01_ERR_INPUT_CANCEL] = MACROTOSTR(M01_ERR_INPUT_CANCEL);
    m_mapErrCode[M01_ERR_INPUT_TIMEOUT] = MACROTOSTR(M01_ERR_INPUT_TIMEOUT);
    m_mapErrCode[M01_ERR_KEY_NOT_EXIST] = MACROTOSTR(M01_ERR_KEY_NOT_EXIST);
    m_mapErrCode[M01_ERR_KEY_VECTOR_NOT_EXIST] = MACROTOSTR(M01_ERR_KEY_VECTOR_NOT_EXIST);
    m_mapErrCode[M01_ERR_KEY_EXIST] = MACROTOSTR(M01_ERR_KEY_EXIST);
    m_mapErrCode[M01_ERR_KEY_ATTR] = MACROTOSTR(M01_ERR_KEY_ATTR);
    m_mapErrCode[M01_ERR_KEY_LEN] = MACROTOSTR(M01_ERR_KEY_LEN);
    m_mapErrCode[M01_ERR_KEY_ID_OUT_OF_RANGE] = MACROTOSTR(M01_ERR_KEY_ID_OUT_OF_RANGE);
    m_mapErrCode[M01_ERR_KEY_DATA_VERIFY] = MACROTOSTR(M01_ERR_KEY_DATA_VERIFY);
    m_mapErrCode[M01_ERR_INPUT_PIN_LEN] = MACROTOSTR(M01_ERR_INPUT_PIN_LEN);
    m_mapErrCode[M01_ERR_CARDNO_OR_TRANSCODE_LEN] = MACROTOSTR(M01_ERR_CARDNO_OR_TRANSCODE_LEN);
    m_mapErrCode[M01_ERR_CARDNO_OR_TRANSCODE_FORMAT] = MACROTOSTR(M01_ERR_CARDNO_OR_TRANSCODE_FORMAT);
    m_mapErrCode[M01_ERR_NO_VALID_PIN_CODE] = MACROTOSTR(M01_ERR_NO_VALID_PIN_CODE);
    m_mapErrCode[M01_ERR_NOT_EXEC_REMOVE_AUTH_VERIFY] = MACROTOSTR(M01_ERR_NOT_EXEC_REMOVE_AUTH_VERIFY);
    m_mapErrCode[M01_ERR_PRE_OPER_INFO_NOT_EXIST] = MACROTOSTR(M01_ERR_PRE_OPER_INFO_NOT_EXIST);
    m_mapErrCode[M01_ERR_ALGO_FUNC_CALC] = MACROTOSTR(M01_ERR_ALGO_FUNC_CALC);
    m_mapErrCode[M01_ERR_VERIFY_SIGNATURE_FAIL] = MACROTOSTR(M01_ERR_VERIFY_SIGNATURE_FAIL);
    m_mapErrCode[M01_ERR_PARAM_DIFF] = MACROTOSTR(M01_ERR_PARAM_DIFF);
    m_mapErrCode[M01_ERR_NOT_EXEC_REMOVE_AUTH] = MACROTOSTR(M01_ERR_NOT_EXEC_REMOVE_AUTH);
    m_mapErrCode[M01_ERR_FIRMWARE_PROG_BUG] = MACROTOSTR(M01_ERR_FIRMWARE_PROG_BUG);
    m_mapErrCode[M01_ERR_FIRMWARE_UPDATE_VERIFY] = MACROTOSTR(M01_ERR_FIRMWARE_UPDATE_VERIFY);
    m_mapErrCode[M01_ERR_FIRMWARE_UPDATE_PKG_SIZE] = MACROTOSTR(M01_ERR_FIRMWARE_UPDATE_PKG_SIZE);

    return;
}

void CDevPIN_CFESM01::UpdateStatus(WORD wDevice, long lErrCode, BYTE byErrType)
{
    m_stStatus.clear();
    m_stStatus.wDevice = wDevice;
    if(byErrType == 1){
        sprintf(m_stStatus.szErrCode, "1%02X", qAbs(lErrCode));
    } else {
        sprintf(m_stStatus.szErrCode, "0%02X", qAbs(lErrCode));
    }
    return;
}

long CDevPIN_CFESM01::SwitchSymmetricKeyMode(BYTE byMode)
{
    THISMODULE(__FUNCTION__);
    QByteArray vtCmd;
    QByteArray vtResult;

    if(byMode != SYMMETRIC_KEY_MODE_DES &&
       byMode != SYMMETRIC_KEY_MODE_AES &&
       byMode != SYMMETRIC_KEY_MODE_SM4){
        return ERR_PARAM;
    }

    if(byMode == m_bySymmetricKeyMode){
       return ERR_SUCCESS;
    }

    char szModeName[MAX_PATH] = "DES";
    if(byMode == SYMMETRIC_KEY_MODE_AES){
        strcpy(szModeName, "AES");
    } else if(byMode == SYMMETRIC_KEY_MODE_SM4) {
        strcpy(szModeName, "SM4");
    }

    vtCmd.push_back(byMode);

    long lRet = ExecuteCommand(0x20, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "切换对称密钥处理模式命令失败：%d[%s]", lRet, szModeName);
        return lRet;
    }

    m_bySymmetricKeyMode = byMode;
    Log(ThisModule, 1, "切换对称密钥处理模式成功：%s", szModeName);
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::SwitchBeep(bool bBeep)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    BYTE byBeepOpt = bBeep ? 0x01 : 0x00;
    vtCmd.push_back(byBeepOpt);

    long lRet = ExecuteCommand(0xBB, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "%s键盘声音失败：%d", lRet, bBeep ? "Open" : "Close");
        return lRet;
    }
    return ERR_SUCCESS;
}

/*
 * @function
 *
 */
long CDevPIN_CFESM01::SetFuncOfKeys(bool bClearKeyAsBackspace)
{
    THISMODULE(__FUNCTION__);
    //获取默认按键功能表
    QByteArray vtCmd;
    QByteArray vtResult;
    QByteArray vtKeyFuncTbl;

    vtCmd.push_back((char)0x01);
    long lRet = ExecuteCommand(0xC3, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "获取默认按键功能表失败");
        return lRet;
    }

    BYTE byClearKeyIdx = 7;
    vtKeyFuncTbl.append(vtResult);
    if(bClearKeyAsBackspace){
        if(vtResult.at(byClearKeyIdx) != 0x0D){
            vtResult[byClearKeyIdx] = 0x0D;
        } else {
            return ERR_SUCCESS;
        }
    } else {
        if(vtResult.at(byClearKeyIdx) != 0x0C){
            vtResult[byClearKeyIdx] = 0x0C;
        } else {
            return ERR_SUCCESS;
        }
    }

    vtResult.clear();

    lRet = ExecuteCommand(0x95, vtKeyFuncTbl, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置键功能失败：%d, clear按键功能:%s",
            lRet, bClearKeyAsBackspace ? "Backspace" : "Clear");
        return lRet;
    }
    return ERR_SUCCESS;
}

long CDevPIN_CFESM01::EnableRemoveFunc(bool bEnable)
{
    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.push_back(bEnable ? 0x01 : 0x00);
    vtCmd.push_back((char)0x00);

    long lRet = ExecuteCommand(0xB4, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(__FUNCTION__, __LINE__, "%s键盘移除功能失败：%d", lRet, bEnable ? "启用" : "关闭");
        return lRet;
    }
    return ERR_SUCCESS;
}

// iRemoveType:0x00 移入 0x01 移出
long CDevPIN_CFESM01::RemoveAuth(int iRemoveType)
{
    long lRet = ERR_SUCCESS;

    QByteArray vtResult;
    QByteArray vtCmd;

    //获取移除状态
    lRet = ExecuteCommand(0xB5, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(__FUNCTION__, __LINE__, "获取移除状态失败:%ld", lRet);
        return lRet;
    }

    bool bIllegalRemove = (vtResult.at(5) == 0x00);

    if(bIllegalRemove){
        //导入移除认证密钥
        char szKcvValue[256] = {0};
        lRet = ImportWKey(63, 0xFF, PKU_KEYENCKEY|PKU_CRYPT, "30303030303030303030303030303030", szKcvValue);
        if(lRet != ERR_SUCCESS){
            Log(__FUNCTION__, __LINE__, "导入移除认证用密钥失败:%ld", lRet);
            return lRet;
        }

        //获取移除认证随机数
        vtCmd.clear();
        vtResult.clear();
        RANDOM_CFES stRandom;
        stRandom.byUseType = 0x01;
        stRandom.wRandomDataLen = EXWORDHL(0x10);
        vtCmd.append((LPSTR)&stRandom, sizeof(stRandom));

        lRet = ExecuteCommand(0xB2, vtCmd, vtResult);
        if(lRet != ERR_SUCCESS){
            Log(__FUNCTION__, __LINE__, "获取移除认证用随机数失败:%ld", lRet);
            return lRet;
        }

        QByteArray vtData;
        QByteArray vtCryptResult;
        CAutoHex::Bcd2Hex((LPBYTE)vtResult.constData(), vtResult.size(), vtData);
        lRet = DataCrypt(63, ECB_EN, vtData.constData(), 0x00, "", vtCryptResult);
        if(lRet != ERR_SUCCESS){
            Log(__FUNCTION__, __LINE__, "获取移除认证用随机数加密结果失败:%ld", lRet);
            return lRet;
        }
        CAutoHex::Hex2Bcd(vtCryptResult.constData(), vtData);

        //移除认证
        vtCmd.clear();
        vtResult.clear();
        vtCmd.push_back((char)0x00);
        vtCmd.push_back(63);
        vtCmd.append(vtData);
        lRet = ExecuteCommand(0xA0, vtCmd, vtResult);
        if(lRet != ERR_SUCCESS){
            Log(__FUNCTION__, __LINE__, "移除认证失败:%ld", lRet);
            return lRet;
        }

        //移除操作
        vtCmd.clear();
        vtResult.clear();
        vtCmd.push_back(iRemoveType);
        lRet = ExecuteCommand(0xA1, vtCmd, vtResult);
        if(lRet != ERR_SUCCESS){
            Log(__FUNCTION__, __LINE__, "移除操作失败:%ld", lRet);
            return lRet;
        }
    }

    return ERR_SUCCESS;
}

void CDevPIN_CFESM01::ReadConfig()
{
    THISMODULE(__FUNCTION__);

    QByteArray strFile("/PINConfig.ini");
#ifdef QT_WIN32
    strFile.prepend(SPETCPATH);
#else
    strFile.prepend(SPETCPATH);
#endif

    CINIFileReader iniFile;
    if (!iniFile.LoadINIFile(strFile.constData()))
    {
        Log(ThisModule, __LINE__, "PIN加载配置文件失败：%s", strFile.constData());
    }

    CINIReader iniReader = iniFile.GetReaderSection("PINInfo");
    m_iClearKeyMode = (int)iniReader.GetValue("ClearKeyAsBackspace", 0);
    m_iRSARKLDecryptHashAlgo = (int)iniReader.GetValue("RSARKLDecryptHashAlgo", 1);
    m_iRSARKLVerifySignatureHashAlgo = (int)iniReader.GetValue("RSARKLVerifySignatureHashAlgo", 1);
    m_bRemoveFuncSupp = (int)iniReader.GetValue("RemoveFuncSupp", 0);
    m_iIllegalHoldKeyTime = (int)iniReader.GetValue("IllegalHoldKeyTime", 0);

    return;
}

bool CDevPIN_CFESM01::ConvertKeyVal(BYTE bKey, EPP_KEYVAL &stKeyVal)
{
    EPP_KEYVAL *pKeyVal = gszKeyVal;
    UINT uKeyCount = sizeof(gszKeyVal) / sizeof(gszKeyVal[0]);

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

long CDevPIN_CFESM01::BuildAsn1Tag(WORD wTagType, WORD wDataLen, LPWORD lpwTagLen, LPBYTE lpbyTagBuffer)
{
    long lRet = -1;

    while(true){
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
            while(true){
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
            }
            *lpwTagLen = i + 2;
        }
        lRet = 0;
        break;
    }
    return lRet;
}

long CDevPIN_CFESM01::ExtractAsn1Data(WORD wFieldLen, LPBYTE lpbyField, WORD wTagType, LPWORD lpwTagLen, LPBYTE lpbyData, LPWORD lpwDataLen)
{
    WORD wTagLenExtensionSize = 0;
    WORD wTagLenBaseSize = 2;						// Minimum Tag Length
    WORD wBufferLen = wFieldLen;
    WORD wLen = 0;
    long lRet = -1;

    while(true){
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

long CDevPIN_CFESM01::SetInitVector(WORD wKeyId, QByteArray &vtInitVec)
{
    THISMODULE(__FUNCTION__);

    long lRet = ERR_SUCCESS;
    QByteArray vtCmd;
    QByteArray vtResult;
    SETINITVEC_CFES stSetInitVec;

    if(vtInitVec.size() > sizeof(stSetInitVec.byVector)){
        Log(ThisModule, __LINE__, "初始化向量长度%d超多最大值%d", vtInitVec.size(), sizeof(stSetInitVec.byVector));
        return ERR_PARAM;
    }

    ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
    stSetInitVec.wKeyId = EXWORDHL(wKeyId);
    memcpy(stSetInitVec.byVector, vtInitVec.constData(), vtInitVec.size());
    UINT uLen = sizeof(stSetInitVec) - (sizeof(stSetInitVec.byVector) - vtInitVec.size());
    vtCmd.append((LPSTR)&stSetInitVec, uLen);

    lRet = ExecuteCommand(0x29, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置初始化向量失败：lRet=%d[KeyNo=%d]", lRet, wKeyId);
    }

    return lRet;
}

long CDevPIN_CFESM01::DataOper(WORD wKeyId, WORD wAlgorithm, WORD wOperMode, BYTE byPadChar,
                               QByteArray &vtData, UINT uDataLen, QByteArray &vtResult)
{
    THISMODULE(__FUNCTION__);

    long lRet = ERR_SUCCESS;
    QByteArray vtCmd;
    vtResult.clear();
    DATAOP stDataOp;
    if(uDataLen > sizeof(stDataOp.byData)){
        Log(ThisModule, __LINE__, "加解密数据长度%d超过最大长度%d", uDataLen, sizeof(stDataOp.byData));
        return ERR_PARAM;
    }

    ZeroMemory(&stDataOp, sizeof(stDataOp));
    stDataOp.wKeyId = EXWORDHL((WORD)wKeyId);
    stDataOp.byOperate = wOperMode;
    stDataOp.wAlgorithm = EXWORDHL(wAlgorithm);
    stDataOp.byPadChar = byPadChar;
    memcpy(stDataOp.byData, vtData.constData(), uDataLen);
    UINT uLen = uDataLen;
    stDataOp.wDataLen = EXWORDHL((WORD)uLen);
    uLen = sizeof(stDataOp) - (sizeof(stDataOp.byData) - uLen);

    vtCmd.append((LPSTR)&stDataOp, uLen);

    lRet = ExecuteCommand(0x2A, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "加解密运算失败：lRet=%d[KeyNo=%02X", lRet, wKeyId);
    }

    return lRet;
}

long CDevPIN_CFESM01::MacOper(WORD wKeyId, WORD wAlgorithm, BYTE byPadChar, QByteArray &vtData, UINT uDataLen, QByteArray &vtResult)
{
    THISMODULE(__FUNCTION__);

    long lRet = ERR_SUCCESS;
    QByteArray vtCmd;
    vtResult.clear();
    MACOP_CFES stMacOp;
    if(uDataLen > sizeof(stMacOp.byData)){
        Log(ThisModule, __LINE__, "Mac加密数据长度%d大于最大长度%d", uDataLen, sizeof(stMacOp.byData));
        return ERR_PARAM;
    }

    ZeroMemory(&stMacOp, sizeof(stMacOp));
    stMacOp.wKeyId = EXWORDHL(wKeyId);
    stMacOp.wAlgorithm = EXWORDHL(wAlgorithm);
    stMacOp.byPad = byPadChar;
    UINT uLen = uDataLen;
    memcpy(stMacOp.byData, vtData.constData(), uLen);
    stMacOp.wDataLen = EXWORDHL(uLen);
    uLen = sizeof(stMacOp) - (sizeof(stMacOp.byData) - uLen);

    vtCmd.append((LPSTR)&stMacOp, uLen);

    lRet = ExecuteCommand(0x2B, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "Mac运算失败：lRet=%d[uKeyNo=%d", lRet, wKeyId);
    }

    return lRet;
}

void CDevPIN_CFESM01::ConvertFDKEnableBitData(DWORD dwActiveFDKKey, DWORD &dwActiveFDKKeyConvert)
{
    dwActiveFDKKeyConvert = 0;
    //F1->F8 F2->F4 F3->F7 F4->F3 F5->F5 F6->F1 F7->F6 F8->F2
    if(dwActiveFDKKey & 0x01){
        dwActiveFDKKeyConvert |= 0x80;
    }
    if(dwActiveFDKKey & 0x02){
        dwActiveFDKKeyConvert |= 0x08;
    }
    if(dwActiveFDKKey & 0x04){
        dwActiveFDKKeyConvert |= 0x40;
    }
    if(dwActiveFDKKey & 0x08){
        dwActiveFDKKeyConvert |= 0x04;
    }
    if(dwActiveFDKKey & 0x10){
        dwActiveFDKKeyConvert |= 0x10;
    }
    if(dwActiveFDKKey & 0x20){
        dwActiveFDKKeyConvert |= 0x01;
    }
    if(dwActiveFDKKey & 0x40){
        dwActiveFDKKeyConvert |= 0x20;
    }
    if(dwActiveFDKKey & 0x80){
        dwActiveFDKKeyConvert |= 0x02;
    }

    return;
}

bool CDevPIN_CFESM01::SetIlligalHoldKeyTime(int iSecond)
{
    THISMODULE(__FUNCTION__);

    LOGDEVACTION();
    if(iSecond < 0 ||iSecond > 125){
        Log(ThisModule, __LINE__, "设置非法长按键时间值非法：%d", iSecond);
        return false;
    }

    QByteArray vtResult;
    QByteArray vtCmd;

    vtCmd.push_back((char)iSecond);

    long lRet = ExecuteCommand(0x94, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置非法长按键时间失败:%d", iSecond);
        return false;
    }

    return true;
}
