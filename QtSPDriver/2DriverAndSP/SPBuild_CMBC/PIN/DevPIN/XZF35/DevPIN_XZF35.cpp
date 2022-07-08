#include "DevPIN_XZF35.h"
#include "XZF35Def.h"

#define EXWORDHL(X)  (WORD)((LOBYTE(X) << 8) | HIBYTE(X))
#define EXDWORDHL(X) (DWORD)((DWORD)(LOWORD(X) << 16) | (HIWORD(X)))
#define RVSDWORD(X)  (DWORD)((EXWORDHL(X & 0xffff) << 16) | (EXWORDHL((X & 0xffff0000) >> 16)))

static const char *ThisFile = "DevPIN_XZF35.cpp";
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static EPP_KEYVAL gszKeyVal[] = { \
    {0x30, "0"}, {0x31, "1"}, {0x32, "2"}, {0x33, "3"}, {0x34, "4"},
    {0x35, "5"}, {0x36, "6"}, {0x37, "7"}, {0x38, "8"}, {0x39, "9"},
    {0x1B, "CANCEL"}, {0x0D, "ENTER"}, {0x08, "CLEAR"},
    {0x54, "00"}, {0x2E, "."}, {0x23, "HELP"},
    {0x41, "F1"}, {0x42, "F2"}, {0x43, "F3"}, {0x44, "F4"}, {0x45, "F5"},
    {0x46, "F6"}, {0x47, "F7"}, {0x48, "F8"}, {0x2A, ""}
};
CDevPIN_XZF35::CDevPIN_XZF35(LPCSTR lpDevType)
{
    SetLogFile("PIN_XZ.log", ThisFile, lpDevType);
    InitErrCode();
}

CDevPIN_XZF35::~CDevPIN_XZF35()
{

}

void CDevPIN_XZF35::Release()
{

}

long CDevPIN_XZF35::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if(nullptr == m_pDev){
        if(m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "PIN", "DevPIN_XZF35") != 0){
            UpdateStatus(DEVICE_OFFLINE, ERR_DEVPORT_LIBRARY);
            Log(ThisModule, __LINE__, "加载库(AllDevPort.dll)失败");
            return ERR_DEVPORT_LIBRARY;
        }
    }

    CAllDevPortAutoLog _auto_log(m_pDev, ThisModule);
    long lRet = m_pDev->Open(lpMode);
    if(lRet != 0){
        UpdateStatus(DEVICE_OFFLINE, ERR_DEVPORT_NOTOPEN);
        Log(ThisModule, __LINE__, "打开串口失败:%d[%s]", lRet, lpMode);
        return ERR_DEVPORT_NOTOPEN;
    }

    return ERR_DEVPORT_SUCCESS;
}

long CDevPIN_XZF35::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if(m_pDev->IsOpened()){
        m_pDev->Close();
    }

    return ERR_SUCCESS;
}

long CDevPIN_XZF35::InitEpp()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    //删除所有密钥
    QByteArray vtCmd;
    QByteArray vtResult;

    long lRet = SendReadCmd(DELETE_ALL_KEY, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "删除所有密钥失败：lRet=%d", lRet);
        return lRet;
    }

    return ERR_SUCCESS;
}

long CDevPIN_XZF35::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtCmd;
    QByteArray vtResult;

    long lRet = SendReadCmd(RESET, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "复位命令失败：%d", lRet);
        return lRet;
    }

    UpdateStatus(DEVICE_ONLINE, 0);
    Log(ThisModule, 1, "复位成功");
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::GetStatus(DEVPINSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_cMutex);

    CAutoCopyDevStatus<DEVPINSTATUS> _auto(&stStatus, &m_stStatus);
    QByteArray vtCmd;
    QByteArray vtResult;
    long lRet = SendReadCmd(GET_DEVICE_STATUS, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        UpdateStatus(DEVICE_OFFLINE, lRet);
        Log(ThisModule, __LINE__, "读取查询状态命令失败：%d", lRet);
        return lRet;
    }
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::InstallStatus(bool bClear)
{
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::GetFirmware(LPSTR lpFWVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    char szFWVer[VER_STR_MAX_LEN] = {0};
    char szFWIntVer[VER_STR_MAX_LEN] = {0};
    char szDevSN[DEV_SN_MAX_LEN] = {0};

    QByteArray vtResult;
    QByteArray vtCmd;
    long lRet = SendReadCmd(GET_FIRMWARE_VERSION, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取固件版本号命令失败：%d", lRet);
        return lRet;
    }
    memcpy(szFWVer, vtResult.constData(), vtResult.size());

    vtResult.clear();
    vtCmd.clear();
    lRet = SendReadCmd(GET_FIRMWARE_INNER_VERSION, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取固件内部版本号命令失败：%d", lRet);
        return lRet;
    }
    memcpy(szFWIntVer, vtResult.constData(), vtResult.size());

    vtResult.clear();
    vtCmd.clear();
    lRet = SendReadCmd(READ_SERIAL_NUMBER, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取固件内部版本号命令失败：%d", lRet);
        return lRet;
    }
    memcpy(szDevSN, vtResult.constData(), vtResult.size());

    sprintf(lpFWVer, "[FW:%s][INTFW:%s][SN:%s]", szFWVer, szFWIntVer, szDevSN);
    Log(ThisModule, 1, "EPP固件信息：%s", lpFWVer);
    return 0;
}

long CDevPIN_XZF35::SetKeyValue(LPCSTR lpKeyVal)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if(lpKeyVal == nullptr){
        Log(ThisModule, __LINE__, "设置按键值字符串参数为nullptr.");
        return ERR_PARAM;
    }

    if(strlen(lpKeyVal) == 0){
        return ERR_SUCCESS;
    }
/*
    BYTE byDefaultWosaValueTbl[] = {0x0A, 0x10, 0x00, 0x0F, 0x0E, 0x09, 0x08, 0x07,
                                    0x0D, 0x06, 0x05, 0x04, 0x0B, 0x03, 0x02, 0x01,
                                    0x27, 0x26, 0x25, 0x24, 0x20, 0x21, 0x22, 0x23};
//    BYTE byKBWosaValueTbl[24] = {0};
//    BYTE byKBKeyValueTbl[24] = {0};
    BYTE byUserKeyValueTbl[24] = {0};
    UINT uLen = strlen(lpKeyVal);
    if(uLen != 48){
        Log(ThisModule, __LINE__, "无效按键数据长度：Len=%d[%s]", uLen, lpKeyVal);
        return ERR_PARAM;
    }

    if (m_strAllKeyVal == lpKeyVal)
    {
        Log(ThisModule, 1, "新旧按键值一致，不用重新设置：%s", lpKeyVal);
        return ERR_SUCCESS;
    }

    //键值字符串转换为键值
    uLen = sizeof(byUserKeyValueTbl);
    CAutoHex::Hex2Bcd(lpKeyVal, byUserKeyValueTbl, uLen);

    Log(ThisModule, __LINE__, "用户键值表设定数据：");
    LogHex(ThisModule, (char *)byUserKeyValueTbl, sizeof(byUserKeyValueTbl));

    QByteArray vtResult;
    QByteArray vtCmd;
    vtCmd.append((LPCSTR)byDefaultWosaValueTbl, sizeof(byDefaultWosaValueTbl));
    vtCmd.append((LPCSTR)byUserKeyValueTbl, sizeof(byUserKeyValueTbl));
    long lRet = SendReadCmd(SET_KEY_VALUE_TABLE, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置新按键值失败：%d[%s]", lRet, lpKeyVal);
        return lRet;
    }

    m_strAllKeyVal = lpKeyVal;
    Log(ThisModule, 1, "设置新按键值成功：%s", lpKeyVal);
*/

    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SetActiveKey(DWORD dwActiveFuncKey, DWORD dwActiveFDKKey)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    QByteArray vtCmd;
    QByteArray vtResult;

    ACTIVATEKEY stActivateKey;
    ZeroMemory(&stActivateKey, sizeof(stActivateKey));
    stActivateKey.dwActiveKeys = dwActiveFuncKey;
    stActivateKey.dwActiveFDKs = dwActiveFDKKey;
    vtCmd.append((LPSTR)&stActivateKey, sizeof(stActivateKey));

    long lRet = SendReadCmd(SET_ACTIVE_AND_TERMINATE_KEY, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "激活按键失败：lRet=%d,dwActiveKeys:%u[%u],dwActiveFDKs:%u[%u]",
            lRet, dwActiveFuncKey, stActivateKey.dwActiveKeys, dwActiveFDKKey, stActivateKey.dwActiveFDKs);
        return lRet;
    }

    return ERR_SUCCESS;
}

/*
 @param    uKeyMode:０：直接下载密钥　1:单独下载密钥属性
 @return
 */

long CDevPIN_XZF35::ImportMKey(UINT uMKeyNo, UINT uKeyMode, LPCSTR lpKeyVal, LPSTR lpKCV)
{
//    THISMODULE(__FUNCTION__);
//    AutoLogFuncBeginEnd();
//    AutoMutex(m_cMutex);
//    LOGDEVACTION();

//    //密钥索引检查(0~63)
//    if (uMKeyNo >= DES_OR_SM4_KEY_MAX_NUM)
//    {
//        Log(ThisModule, __LINE__, "无效保存主密钥ID：uMKeyNo=%02X", uMKeyNo);
//        return ERR_PARAM;
//    }

//    WORD wValLen = strlen(lpKeyVal)/2;
//    if((uKeyMode == 1 && wValLen != 0) ||
//       (uKeyMode == 0 && wValLen != 8 && wValLen != 16 && wValLen != 24)){
//        Log(ThisModule, __LINE__, "非法Key下载参数（非法长度或模式）：uValLen=%d，uKeyMode=%d", wValLen, uKeyMode);
//        return ERR_PARAM;
//    }

//    QByteArray vtResult;
//    QByteArray vtCmd;

//    long lRet = SwitchModeDESOrSM4(true);
//    if(lRet != ERR_SUCCESS){
//        Log(ThisModule, __LINE__, "主密钥导入失败[切换对称密钥处理模式失败]：lRet=%d[uMKeyNo=%d]", lRet, uMKeyNo);
//        return lRet;
//    }

//    // 下载明文主密钥
//    IMPORTKEY stImportKey;
//    UINT uLen = sizeof(stImportKey.byKeyValue);
//    ZeroMemory(&stImportKey, sizeof(stImportKey));

//    stImportKey.wKeyId = EXWORDHL((WORD)uMKeyNo);
//    //Note:XZ special key attribute[initial vector decrypt] is always setted.
//    //默认为全属性，上层通过use属性进行判断
//    stImportKey.dwKeyAttr = KEY_ATTR_ENC | KEY_ATTR_DATA | KEY_ATTR_PIN | KEY_ATTR_MAC | KEY_ATTR_VECTOR_DECRYPT;
//    stImportKey.wMKeyId = 0xFFFF;
//    //HEX字符串转换为ＢＹＴＥ数组
//    CAutoHex::Hex2Bcd(lpKeyVal, stImportKey.byKeyValue, uLen);
//    uLen = sizeof(stImportKey) - (sizeof(stImportKey.byKeyValue) - uLen);
//    vtCmd.append((LPSTR)&stImportKey, uLen);

//    lRet = SendReadCmd(DES_KEY_LOAD, vtCmd, vtResult);
//    if (lRet != ERR_SUCCESS)
//    {
//        Log(ThisModule, __LINE__, "主密钥导入失败：lRet=%d[uMKeyNo=%02X]", lRet, uMKeyNo);
//        return lRet;
//    }

//    CAutoHex cHex((LPBYTE)vtResult.constData(), vtResult.size());
//    strcpy(lpKCV, cHex.GetHex());
//    Log(ThisModule, 1, "主密钥导入成功:uMKeyNo=%02X", uMKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode/* = 2*/)
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
        Log(ThisModule, __LINE__, "无效密钥ID：uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
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
        Log(ThisModule, __LINE__, "无效Key用途：dwKeyUse=%u[%u]", dwKeyUse, uKeyUse);
        return ERR_PARAM;
    }
    dwKeyUse |= KEY_ATTR_VECTOR_DECRYPT;

    wValLen = strlen(lpKeyVal) / 2;
    if (wValLen != 8 && wValLen != 16 && wValLen != 24)
    {
        Log(ThisModule, __LINE__, "无效Key长度：wValLen=%d", wValLen);
        return ERR_PARAM;
    }

    long lRet = SwitchModeDESOrSM4(true);
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
    lRet = SendReadCmd(DES_KEY_LOAD, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "工作密钥导入失败：lRet=%d[uKeyNo=%02X, uMKeyNo=%02X]", lRet, uKeyNo, uMKeyNo);
        return lRet;
    }   

    lRet = ReadSymmKeyKCV(wKeyNo, byKcvMode, lpKCV);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    Log(ThisModule, 1, "工作密钥导入成功:uKeyNo=%04X, uMKeyNo=%04X", uKeyNo, uMKeyNo);
    return ERR_SUCCESS;
}

void CDevPIN_XZF35::SetDesKcvDecryptKey(UINT uKeyNo)
{

}

long CDevPIN_XZF35::DeleteKey(UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtCmd;
    QByteArray vtResult;
    WORD wCmdCode;
    if (uKeyNo == 0)
    {
        //删除所有key
        wCmdCode = DELETE_ALL_KEY;
    }
    else
    {
        if(uKeyNo >= DES_OR_SM4_KEY_MAX_NUM){
            Log(ThisModule, __LINE__, "删除国际密钥失败,密钥索引非法：%d[uKeyNo=%d]", uKeyNo);
            return ERR_PARAM;
        }
        //切换对称密钥处理模式
        long lRet = SwitchModeDESOrSM4(true);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "切换对称密钥处理模式失败导致删除密钥失败：%d[uKeyNo=%d]", lRet, uKeyNo);
            return lRet;
        }

        //删除指定key
        wCmdCode = DELETE_MARKED_SYMMETRIC_KEY;
        DELSPECKEY stDelSpecKey;
        ZeroMemory(&stDelSpecKey, sizeof(stDelSpecKey));
        stDelSpecKey.wKeyId = EXWORDHL(uKeyNo);
        vtCmd.append((LPSTR)&stDelSpecKey, sizeof(stDelSpecKey));
    }

    long lRet = SendReadCmd(wCmdCode, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "删除密钥命令失败：%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    Log(ThisModule, 1, "删除密钥成功：uKeyNo=%d", uKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::TextInput()
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

    // 使用这种方法，在设备日志是，有记录明文输入
    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;
    INPUTPLAINTEXT stInputPlainText;
    stInputPlainText.byMaxLen = 0;
    stInputPlainText.byEndMode = 0x00;
    vtCmd.append((LPSTR)&stInputPlainText, sizeof(stInputPlainText));

    lRet = SendReadCmd(INPUT_PLAINTEXT, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打开明文输入命令失败：%d", lRet);
        SwitchBeep(false);// 关闭
        return lRet;
    }

    Log(ThisModule, 1, "打开明文输入成功");
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::PinInput(UINT uMinLen, UINT uMaxLen)
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

    INPUTPIN stInputPin;
    stInputPin.byMinLen = uMinLen;
    stInputPin.byMaxLen = uMaxLen;
    stInputPin.byGetKeyModeAndEndMode = 0x00;
    stInputPin.byEchoCode = 0x2A;
    vtCmd.append((LPCSTR)&stInputPin, sizeof(stInputPin));

    lRet = SendReadCmd(INPUT_PIN, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        SwitchBeep(false);          //关闭按键声音
        Log(ThisModule, __LINE__, "打开密文输入命令失败：%d", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "打开密文输入成功");
    return lRet;
}

long CDevPIN_XZF35::ReadKeyPress(EPP_KEYVAL &stKeyVal, DWORD dwTimeOut)
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
    } else if (lRet == ERR_DEVPORT_RTIMEOUT) {
        m_pDev->CancelLog();
    }
    return lRet;
}

long CDevPIN_XZF35::CancelInput(bool bPinInput)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    LOGDEVACTION();
    QByteArray vtResult;
    QByteArray vtCmd;

    long lRet = SendReadCmd(ABORT_INPUT, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "取消[%s]输入命令失败：%d", bPinInput ? "密文" : "明文", lRet);
        return lRet;
    }

    Log(ThisModule, 1, "取消[%s]输入成功", bPinInput ? "密文" : "明文");
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::GetPinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock)
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

    long lRet = SwitchModeDESOrSM4(true);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "Pinblock失败[切换对称密钥处理模式失败]：lRet=%d", lRet);
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

    lRet = SendReadCmd(GET_PINBLOCK, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取密码块失败：%d[uKeyNo=%02X,uFormat=%02X]", lRet, uKeyNo, wFmType);
        return lRet;
    }

    CAutoHex autoHex(vtResult);
    strcpy(lpPinBlock, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::DataCrypt(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpCryptData == nullptr || lpIVData == nullptr || lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    WORD wAlgorithm = DATA_ALGO_ECB;
    BYTE byOpMode = ENCRYPT_MODE;
    switch (uMode)
    {
    case ECB_EN:
        break;// ECB // Encryption
    case ECB_DE:
        byOpMode = DECRYPT_MODE;
        break;// ECB // Decryption
    case CBC_EN:
        wAlgorithm = DATA_ALGO_CBC;
        break;// CBC // Encryption
    case CBC_DE:
        wAlgorithm = DATA_ALGO_CBC;
        byOpMode = DECRYPT_MODE;
        break;// CBC // Decryption
    default:
        Log(ThisModule, __LINE__, "不支持Crypt模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    //加密数据长度检查
    if(strlen(lpCryptData) / 2 > 1024) { //加密，数据长度超过256
        Log(ThisModule, __LINE__, "Data运算数据长度超过最大值1024");
        return ERR_PARAM;
    }

    LOGDEVACTION();
    QByteArray vtCmd;
    QByteArray vtResult;
    UINT uLen;

    //兼容3DES密钥，选择DES算法.不支持自动检测密钥长度，所以先获取密钥长度.
    GETSYMMKEYINFO stGetSymmKeyInfo;
    stGetSymmKeyInfo.wKeyId = EXWORDHL((WORD)uKeyNo);
    vtCmd.append((LPSTR)&stGetSymmKeyInfo, sizeof(stGetSymmKeyInfo));
    long lRet = SendReadCmd(QUERY_SYMMETRIC_ENC_KEY_INFO, vtCmd, vtResult);
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

    //设置初始向量
    if(wAlgorithm & (DATA_ALGO_CBC | DATA_ALGO_TDES_CBC) && strlen(lpIVData) > 0){
        SETINITVEC stSetInitVec;
        ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
        uLen = sizeof(stSetInitVec.byVector);
        CAutoHex::Hex2Bcd(lpIVData, stSetInitVec.byVector, uLen);

        vtCmd.clear();
        vtCmd.append((LPSTR)&stSetInitVec, uLen);
        lRet = SendReadCmd(SET_INITIAL_PLAINTEXT_VECTOR, vtCmd, vtResult);
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

    lRet = SendReadCmd(DATA_ECB_OR_CBC_ARITHMETIC, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "Data运算失败：lRet=%d", lRet);
        return lRet;
    }

    CAutoHex autoHex(vtResult);
    strcpy(lpData, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::DataMAC(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpMacData == nullptr || lpIVData == nullptr || lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    WORD wAlgorithm = 0;
    switch (uMode)
    {
    case MAC_ANSIX99:
        wAlgorithm |= MAC_ALGO_X99;
        break;
    case MAC_ANSIX919:
        wAlgorithm |= MAC_ALGO_X919;
        break;
    case MAC_PBOC:
        wAlgorithm |= MAC_ALGO_PBCO;
        break;
    case MAC_CBC:
//        wAlgorithm |= MAC_ALGO_UBC;
        wAlgorithm |= MAC_ALGO_X99T;
        break;
    case MAC_ISO16609:
    default:
        Log(ThisModule, __LINE__, "不支持MAC模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    if(strlen(lpMacData) / 2 > 2048){
        Log(ThisModule, __LINE__, "Mac运算数据长度超过最大值2048");
        return ERR_PARAM;
    }

    LOGDEVACTION();
    QByteArray vtCmd;
    QByteArray vtResult;
    UINT uLen;

    //兼容3DES密钥，选择DES算法.不支持自动检测密钥长度，所以先获取密钥长度.
    GETSYMMKEYINFO stGetSymmKeyInfo;
    stGetSymmKeyInfo.wKeyId = EXWORDHL((WORD)uKeyNo);
    vtCmd.append((LPSTR)&stGetSymmKeyInfo, sizeof(stGetSymmKeyInfo));
    long lRet = SendReadCmd(QUERY_SYMMETRIC_ENC_KEY_INFO, vtCmd, vtResult);
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

    //设置初始向量
    if(strlen(lpIVData) > 0){
        SETINITVEC stSetInitVec;
        ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
        uLen = sizeof(stSetInitVec.byVector);
        CAutoHex::Hex2Bcd(lpIVData, stSetInitVec.byVector, uLen);

        vtCmd.clear();
        vtCmd.append((LPSTR)&stSetInitVec, uLen);
        lRet = SendReadCmd(SET_INITIAL_PLAINTEXT_VECTOR, vtCmd, vtResult);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "Mac运算设置初始化向量失败：lRet=%d, lpIVData=%s", lRet, lpIVData);
            return lRet;
        }
    }

    MACOP stMacOp;
    ZeroMemory(&stMacOp, sizeof(stMacOp));
    stMacOp.wKeyId = EXWORDHL(uKeyNo);
    stMacOp.wAlgorithm = EXWORDHL(wAlgorithm);
    uLen = sizeof(stMacOp.byData);
    CAutoHex::Hex2Bcd(lpMacData, stMacOp.byData, uLen);
    stMacOp.wDataLen = EXWORDHL((WORD)uLen);
    uLen = sizeof(stMacOp) - (sizeof(stMacOp.byData) - uLen);

    vtCmd.clear();
    vtCmd.append((LPSTR)&stMacOp, uLen);
    lRet = SendReadCmd(MAC_ARITHMETIC, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "Mac运算失败：lRet=%d", lRet);
        return lRet;
    }

    CAutoHex autoHex(vtResult);
    strcpy(lpData, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::RandData(LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;
    RANDOM stRandom;
    stRandom.byUseType = 0x30;
    stRandom.byRandomBytes = 8;
    vtCmd.append((LPSTR)&stRandom, sizeof(stRandom));

    long lRet = SendReadCmd(GET_RANDOM_NUMBER, vtCmd, vtResult);
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

long CDevPIN_XZF35::SMGetFirmware(LPSTR lpSMVer)
{
    return GetFirmware(lpSMVer);
}

long CDevPIN_XZF35::SM4ImportMKey(UINT uMKeyNo, LPCSTR lpKeyVal, LPSTR lpKCV)
{
//    THISMODULE(__FUNCTION__);
//    AutoLogFuncBeginEnd();
//    AutoMutex(m_cMutex);
//    LOGDEVACTION();

//    QByteArray vtResult;
//    QByteArray vtCmd;
//    WORD wValLen = strlen(lpKeyVal)/2;
//    if (wValLen != 16)
//    {
//        Log(ThisModule, __LINE__, "无效Key长度：wValLen=%d", wValLen);
//        return ERR_PARAM;
//    }

//    if (uMKeyNo >= DES_OR_SM4_KEY_MAX_NUM)
//    {
//        Log(ThisModule, __LINE__, "无效保存主密钥ID：uMKeyNo=%02X", uMKeyNo);
//        return ERR_PARAM;
//    }

//    long lRet = SwitchModeDESOrSM4(false);
//    if(lRet != ERR_SUCCESS){
//        Log(ThisModule, __LINE__, "SM主密钥导入失败[切换对称密钥处理模式失败]：lRet=%d[uMKeyNo=%d]", lRet, uMKeyNo);
//        return lRet;
//    }

//    // 下载明文主密钥
//    UINT uLen;
//    IMPORTSM4KEY stImportSm4Key;
//    memset(&stImportSm4Key, 0, sizeof(stImportSm4Key));
//    stImportSm4Key.wKeyId = EXWORDHL(uMKeyNo);
//    stImportSm4Key.wMKeyId = 0xFFFF;
//    stImportSm4Key.ulKeyAttr = KEY_ATTR_ENC | KEY_ATTR_DATA | KEY_ATTR_PIN | KEY_ATTR_MAC | KEY_ATTR_VECTOR_DECRYPT;
//    uLen = sizeof(stImportSm4Key.byKeyValue);
//    CAutoHex::Hex2Bcd(lpKeyVal, stImportSm4Key.byKeyValue, uLen);
//    uLen = sizeof(stImportSm4Key) - (sizeof(stImportSm4Key.byKeyValue) - uLen);
//    vtCmd.append((LPSTR)&stImportSm4Key, uLen);

//    lRet = SendReadCmd(SM4_KEY_LOAD, vtCmd, vtResult);
//    if (lRet != ERR_SUCCESS)
//    {
//        Log(ThisModule, __LINE__, "SM主密钥导入失败：lRet=%d[uMKeyNo=%02X]", lRet, uMKeyNo);
//        return lRet;
//    }

//    CAutoHex cHex((LPBYTE)vtResult.constData(), vtResult.size());
//    strcpy(lpKCV, cHex.GetHex());
//    Log(ThisModule, 1, "SM主密钥导入成功:uMKeyNo=%02X", uMKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SM4ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode/* = 0*/)
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
        Log(ThisModule, __LINE__, "无效保存SM主密钥ID：uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
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

    long lRet = SwitchModeDESOrSM4(false);
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

    lRet = SendReadCmd(SM4_KEY_LOAD, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM工作密钥导入失败：lRet=%d[uKeyNo=%02X, uMKeyNo=%02X]", lRet, uKeyNo, uMKeyNo);
        return lRet;
    }

    lRet = ReadSymmKeyKCV(wKeyNo, byKcvMode, lpKCV);
    if(lRet != ERR_SUCCESS){
        return lRet;
    }

    Log(ThisModule, 1, "SM工作密钥导入成功:uKeyNo=%02X, uMKeyNo=%02X", uKeyNo, uMKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SM4RemoteImportKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostSM2PKeyNo, UINT uKeyUse, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal, LPSTR lpKCV)
{

}

long CDevPIN_XZF35::SM4CryptData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    LOGDEVACTION();

    if (lpCryptData == nullptr || lpIVData == nullptr || lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    WORD wAlgorithm = DATA_ALGO_ECB;
    WORD wOpMode = ENCRYPT_MODE;
    switch (uMode)
    {
    case ECB_EN:
        break;// ECB // Encryption
    case ECB_DE:
        wOpMode = DECRYPT_MODE;
        break;// ECB // Decryption
    case CBC_EN:
        wAlgorithm = DATA_ALGO_CBC;
        break;// CBC // Encryption
    case CBC_DE:
        wAlgorithm = DATA_ALGO_CBC;
        wOpMode = DECRYPT_MODE;
        break;// CBC // Decryption
    default:
        Log(ThisModule, __LINE__, "不支持Crypt模式：uMode=%02X", uMode);
        return ERR_NOT_SUPPORT;
    }

    QByteArray vtCmd;
    QByteArray vtResult;
    UINT uLen;

    long lRet = SwitchModeDESOrSM4(false);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "SM Data运算失败[切换对称密钥处理模式失败]：lRet=%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    //设置初始化向量
    if(wAlgorithm == DATA_ALGO_CBC){
        if(strlen(lpIVData) > 0){
            SETINITVEC stSetInitVec;
            ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
            uLen = sizeof(stSetInitVec.byVector);
            CAutoHex::Hex2Bcd(lpIVData, stSetInitVec.byVector, uLen);
            vtCmd.append((LPSTR)&stSetInitVec, uLen);

            lRet = SendReadCmd(SET_INITIAL_PLAINTEXT_VECTOR, vtCmd, vtResult);
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "SM Data运算设置初始化向量失败：lRet=%d[uKeyNo=%02X]", lRet, uKeyNo);
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

    lRet = SendReadCmd(DATA_ECB_OR_CBC_ARITHMETIC, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM Data运算失败：lRet=%d[uKeyNo=%02X", lRet, uKeyNo);
        return lRet;
    }

    CAutoHex autoHex(vtResult);
    strcpy(lpData, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SM4MACData(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, LPSTR lpData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    if (lpMacData == nullptr || lpIVData == nullptr || lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效数据指针");
        return ERR_PARAM;
    }

    WORD wAlgorithm = 0;
    switch (uMode)
    {
    case MAC_BANKSYS: 
    case MAC_PBOC:
        //旭子只支持这一种国密算法
        wAlgorithm = MAC_ALGO_X99T;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持MAC模式：uMode=%d", uMode);
        return ERR_NOT_SUPPORT;
    }

    long lRet = SwitchModeDESOrSM4(false);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "SM Mac运算失败[切换对称密钥处理模式失败]：lRet=%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    LOGDEVACTION();
    QByteArray vtCmd;
    QByteArray vtResult;
    UINT uLen;

    //设置初始化向量
    if(strlen(lpIVData) > 0){
        SETINITVEC stSetInitVec;
        ZeroMemory(&stSetInitVec, sizeof(stSetInitVec));
        uLen = sizeof(stSetInitVec.byVector);
        CAutoHex::Hex2Bcd(lpIVData, stSetInitVec.byVector, uLen);
        vtCmd.append((LPSTR)&stSetInitVec, uLen);

        lRet = SendReadCmd(SET_INITIAL_PLAINTEXT_VECTOR, vtCmd, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "SM Mac运算设置初始化向量失败：lRet=%d[uKeyNo=%02X]", lRet, uKeyNo);
            return lRet;
        }
    }

    MACOP stMacOp;
    ZeroMemory(&stMacOp, sizeof(stMacOp));
    stMacOp.wKeyId = EXWORDHL((WORD)uKeyNo);
    stMacOp.wAlgorithm = EXWORDHL(wAlgorithm);
    uLen = sizeof(stMacOp.byData);
    CAutoHex::Hex2Bcd(lpMacData, stMacOp.byData, uLen);
    stMacOp.wDataLen = EXWORDHL((WORD)uLen);
    uLen = sizeof(stMacOp) - (sizeof(stMacOp.byData) - uLen);

    vtCmd.clear();
    vtCmd.append((LPSTR)&stMacOp, uLen);

    lRet = SendReadCmd(MAC_ARITHMETIC, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM Mac运算失败：lRet=%d[uKeyNo=%02X", lRet, uKeyNo);
        return lRet;
    }

    CAutoHex autoHex(vtResult);
    strcpy(lpData, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SM4PinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock)
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
        wFmtType |= PIN_FORMAT_ANSIX98;
        break;
    default:
        Log(ThisModule, __LINE__, "不支持模式：%02X", uFormat);
        return ERR_NOT_SUPPORT;
    }

    long lRet = SwitchModeDESOrSM4(false);
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

    lRet = SendReadCmd(GET_PINBLOCK, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "SM 读取密码块失败：%d[nKeyNo=%02X,uFormat=%02X]", lRet, uKeyNo, uFormat);
        return lRet;
    }

    CAutoHex autoHex(vtResult);
    strcpy(lpPinBlock, autoHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SM2ImportKey(UINT uKeyNo, UINT uVendorPKeyNo, BOOL bPublicKey, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal)
{

}

long CDevPIN_XZF35::SM2EncryptData(LPCSTR lpSM2PKeyData, LPCSTR lpData, LPCSTR lpCryptData)
{

}

long CDevPIN_XZF35::SM2DecryptData(LPCSTR lpSM2SKeyData, LPCSTR lpCryptData, LPCSTR lpData)
{

}

long CDevPIN_XZF35::SM2SignData(LPCSTR lpSM2SKeyData, LPCSTR lpZA, LPCSTR lpData, LPSTR lpSignData)
{

}

long CDevPIN_XZF35::SM2VerifySign(LPCSTR lpSM2PKeyData, LPCSTR lpZA, LPCSTR lpData, LPCSTR lpSignData)
{

}

long CDevPIN_XZF35::SM2ExportPKey(UINT uKeyNo, UINT uSignKeyNo, LPSTR lpZA, LPSTR lpSM2PKeyData, LPSTR lpSignData)
{

}

long CDevPIN_XZF35::SM2GenerateKey(UINT uSKeyNo, UINT uPKeyNo)
{

}

long CDevPIN_XZF35::SM3CaculateZAData(LPCSTR lpUserData, LPSTR lpData)
{

}

long CDevPIN_XZF35::SMDeleteKey(UINT uKeyNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    long lRet;
    WORD wCmdCode;
    QByteArray vtCmd;
    QByteArray vtResult;

    if (uKeyNo == 0)
    {
        //删除所有key
        wCmdCode = DELETE_ALL_KEY;
    }
    else
    {
        if(uKeyNo >= DES_OR_SM4_KEY_MAX_NUM){
            Log(ThisModule, __LINE__, "删除国密密钥失败,密钥索引非法：uKeyNo=%d", uKeyNo);
            return ERR_PARAM;
        }

        //切换对称密钥处理模式
        lRet = SwitchModeDESOrSM4(false);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "删除国密密钥失败[切换对称密钥处理模式失败]：%d", lRet);
            return lRet;
        }

        //删除指定key
        WORD wKeyNo = uKeyNo;
        wCmdCode = DELETE_MARKED_SYMMETRIC_KEY;
        DELSPECKEY stDelSpecKey;
        stDelSpecKey.wKeyId = EXWORDHL(wKeyNo);
        vtCmd.append((LPSTR)&stDelSpecKey, sizeof(stDelSpecKey));
    }

    lRet = SendReadCmd(wCmdCode, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "删除国密密钥命令失败：%d[uKeyNo=%d]", lRet, uKeyNo);
        return lRet;
    }

    Log(ThisModule, 1, "删除国密密钥成功：uKeyNo=%d", uKeyNo);
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SM4SaveKeyType(UINT uType)
{
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::RSAInitStatus()
{
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::RSAImportRootHostPK(QByteArray strHostPK, QByteArray strSignHostPK)
{

}

long CDevPIN_XZF35::RSAImportSignHostPK(QByteArray strHostPK, QByteArray strSignHostPK)
{

}

long CDevPIN_XZF35::RSAExportEppEncryptPK(QByteArray &strPK, QByteArray &strSignPK)
{

}

long CDevPIN_XZF35::RSAExportEppSN(QByteArray &strSN, QByteArray &strSignSN)
{

}

long CDevPIN_XZF35::RSAExportEppRandData(QByteArray &strRandData)
{

}

long CDevPIN_XZF35::RSAImportDesKey(UINT uMKeyNo, QByteArray strMKVal, QByteArray strSignMKVal, QByteArray &strKCV)
{

}

long CDevPIN_XZF35::PackCmd(WORD wCmd, LPCSTR lpCmdData, DWORD dwCmdDataLen, LPSTR lpData, DWORD &dwSize)
{
    //CMD Format:0x1B + <CMD> + <LEN> + <DATA> + LRC + 0x0D0A
    BYTE byCmdPkg[CMD_PKG_MAX_LEN] = {0};
    WORD wEndTag = CMD_ETX;
    DWORD dwPkgLen = 0;
    WORD wLen = 0;

    byCmdPkg[0] = CMD_STX;
    dwPkgLen++;

    memcpy(byCmdPkg + dwPkgLen, &wCmd, sizeof(wCmd));
    dwPkgLen += 2;

    wLen = EXWORDHL((WORD)(dwCmdDataLen + 1));
    memcpy(byCmdPkg + dwPkgLen, &wLen, sizeof(wLen));
    dwPkgLen += 2;

    if(dwCmdDataLen > 0){
        memcpy(byCmdPkg + dwPkgLen, lpCmdData, dwCmdDataLen);
        dwPkgLen += dwCmdDataLen;
    }

    byCmdPkg[dwPkgLen] = GetLRC(byCmdPkg, dwPkgLen);
    dwPkgLen++;

    memcpy(byCmdPkg + dwPkgLen, &wEndTag, sizeof(wEndTag));
    dwPkgLen += 2;

    memcpy(lpData, byCmdPkg, dwPkgLen);
    dwSize = dwPkgLen;
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::UnPackCmd(LPCSTR lpCmd, DWORD dwReadLen, LPSTR lpData, DWORD &dwSize)
{
    //Resp format:0x02 + <RES> + <LEN> + <DATA>
    THISMODULE(__FUNCTION__);
    BYTE byRspData[RSP_PKG_MAX_LEN] = {0};

    DWORD dwRspLen = 0;            //返回包总长度
    DWORD dwDataLen = 0;            //返回包中ＤＡＴＡ长度
    DWORD dwRspOffSet = 0;         //返回包数据偏移量
    bool  bCmdOK = false;

    //查找开始字符
    for(DWORD i = 0; i < dwReadLen; i++){
        if(lpCmd[i] == RSP_STX){
            if(dwReadLen - i >= 4){
                dwDataLen = EXWORDHL(*(WORD *)(lpCmd + 2));
                dwRspLen = dwDataLen + 4;
                if(dwReadLen - i >= dwRspLen){
                    bCmdOK = true;
                    dwRspOffSet = i;
                }
                break;
            }
        }
    }

    if(!bCmdOK){
        Log(ThisModule, __LINE__, "返回数据结构分析失败[dwReadLen=%u]", dwReadLen);
        return ERR_REDATA;
    }

    memcpy(byRspData, lpCmd + dwRspOffSet, dwRspLen);

    BYTE byExecRes = byRspData[1];
    if(byExecRes == RSP_NG){
        BYTE byDevErrCode = byRspData[4];
        UpdateStatus(DEVICE_HWERROR, byDevErrCode, 1);
        return ConvertErrCode(byDevErrCode, ThisModule);
    }
    UpdateStatus(DEVICE_ONLINE, 0);

    //拷贝返回数据
    if(dwDataLen < dwSize){
        dwSize = dwDataLen;
    }

    if(dwSize > 0){
        memcpy(lpData, byRspData + 4, dwSize);
    }
    return ERR_SUCCESS;
}

BYTE CDevPIN_XZF35::GetLRC(const BYTE *pszCmd, DWORD dwCmdLen)
{
    // LRC(Longitudinal Redundancy Check)——1 byte(HEX), Excute OR all bytes of between <STX> and <DATA>
    BYTE byLRC = 0;
    for (DWORD i = 0; i < dwCmdLen; i++)
    {
        byLRC ^= pszCmd[i];
    }

    return byLRC;
}

long CDevPIN_XZF35::SendReadCmd(WORD wCmdCode, const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 命令打包
    BYTE szPackCmd[CMD_PKG_MAX_LEN] = { 0 };
    DWORD dwCmdSize = sizeof(szPackCmd);
    long lRet = PackCmd(wCmdCode, vtCmd.constData(), vtCmd.size(), (char *)szPackCmd, dwCmdSize);
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
        UpdateStatus(DEVICE_OFFLINE, lRet);
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

long CDevPIN_XZF35::SendReadData(const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 发送命令和接收返回数据
    long lRet = m_pDev->Send(vtCmd, vtCmd.size(), dwTimeOut);
    if (lRet != vtCmd.size())
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
            bool bStart = false;
            vtResult.append(szRead, dwReadLen);
            if (vtResult.size() < 5)// 没有达到最小长度
                continue;
            for (int i = 0; i < vtResult.size(); i++)
            {
                if (vtResult.at(i) == RSP_STX && !bStart)
                {
                    bStart = true;
                    //获取数据部分长度
                    WORD wDataLen = EXWORDHL(*(WORD *)(vtResult.constData() + i + 2));

                    if(vtResult.size() - i < wDataLen + 4){
                        //数据部分未接收完整,继续接收
                        break;
                    } else {
                        return ERR_DEVPORT_SUCCESS;
                    }
                }
            }
        }

    } while (true);

    Log(ThisModule, __LINE__, "读取返回数据超时失败");
    return ERR_DEVPORT_RTIMEOUT;
}

long CDevPIN_XZF35::ConvertErrCode(BYTE byRTN, LPCSTR ThisModule)
{
    long lRet;
    auto it = m_mapErrCode.find(byRTN);
    if (it != m_mapErrCode.end())
    {
        Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][%s]", it->first, it->second.c_str());
        switch (byRTN) {
        case 0x19:
            lRet = ERR_NO_PIN;
            break;
        default:
            lRet = byRTN;
            break;
        }
        return lRet;
    } else {
        Log(ThisModule, __LINE__, "命令执行返回码失败[byRTN=%02X][未知错误]", byRTN);
        return ERR_OTHER;
    }
}

void CDevPIN_XZF35::InitErrCode()
{
    m_mapErrCode[0x01] = "Kuhn defense activation";
    m_mapErrCode[0x02] = "Selected algorithm wrong";
    m_mapErrCode[0x03] = "Instruction format error";
    m_mapErrCode[0x07] = "Serial number already exist";
    m_mapErrCode[0x08] = "Serial number not exist";
    m_mapErrCode[0x09] = "Save signature data fail";
    m_mapErrCode[0x0A] = "No version number";
    m_mapErrCode[0x0B] = "Password not exist";
    m_mapErrCode[0x0C] = "Password already exist";
    m_mapErrCode[0x0D] = "Password error";
    m_mapErrCode[0x0E] = "Key not exist";
    m_mapErrCode[0x0F] = "Key already exist";
    m_mapErrCode[0x10] = "Key attribute error";
    m_mapErrCode[0x11] = "Input exist abnormally";
    m_mapErrCode[0x12] = "Key Load fail";
    m_mapErrCode[0x14] = "Same value key already exist in device";
    m_mapErrCode[0x16] = "Authentication data check error";
    m_mapErrCode[0x17] = "Key index out of range";
    m_mapErrCode[0x18] = "Pad data error";
    m_mapErrCode[0x19] = "No pin";
    m_mapErrCode[0x1A] = "Pin already invalid";
    m_mapErrCode[0x1B] = "Pin input interval too short";
    m_mapErrCode[0x1C] = "Pin length error";
    m_mapErrCode[0x1D] = "Card number or transcode length error";
    m_mapErrCode[0x1F] = "Instruction sequence error";
    m_mapErrCode[0x20] = "No memory";
    m_mapErrCode[0x24] = "Keyboard locked";
    m_mapErrCode[0x25] = "Pad data length error";
    m_mapErrCode[0x26] = "Operate timeout";
    m_mapErrCode[0x29] = "No password verification";
    m_mapErrCode[0x30] = "Get pinblock parameter error";
    m_mapErrCode[0x31] = "Necessary data length error";
    m_mapErrCode[0x34] = "Need remove operation";
    m_mapErrCode[0x35] = "Device no initialization";
    m_mapErrCode[0x36] = "Need two-sides verification";
    m_mapErrCode[0x39] = "Write remove infomation format error";
    m_mapErrCode[0x40] = "Set special function of plaint or cipher text fail";
    m_mapErrCode[0x41] = "Write customer infomation error";
    m_mapErrCode[0x42] = "Key length error";
    m_mapErrCode[0x43] = "Customer infomation not exist";
    m_mapErrCode[0x44] = "Firmware self check fail";
    m_mapErrCode[0x45] = "Rsa data greater than rsa mold";
    m_mapErrCode[0x46] = "SM2 or RSA key not activated";
    m_mapErrCode[0x47] = "SM chip access timeout";
    m_mapErrCode[0x48] = "Arithmetic error";
    m_mapErrCode[0x49] = "Signature check error";
    m_mapErrCode[0x4A] = "No supported hash algorithm";
    m_mapErrCode[0x50] = "Public key der analysis fail";
    m_mapErrCode[0x51] = "Password instruction sequence error";
    m_mapErrCode[0x52] = "Input twice password not same";
    m_mapErrCode[0x53] = "Old password verification fail";
    m_mapErrCode[0x54] = "No permit setting shift key";
    m_mapErrCode[0x55] = "Activate key fail as parameter error";
    m_mapErrCode[0x56] = "Rsa signatue invalid";
    m_mapErrCode[0x57] = "Rsa decrypt fail";
    m_mapErrCode[0x58] = "Der data analysis fail";
    m_mapErrCode[0x59] = "Rsa mold length error";
    m_mapErrCode[0x5A] = "Rsa E value error";
    m_mapErrCode[0x60] = "Eppid no import";
    m_mapErrCode[0x71] = "Rsa key pair not exist";
    m_mapErrCode[0x72] = "Rsa key no import";
    m_mapErrCode[0x73] = "Signature error";
    m_mapErrCode[0x74] = "Signature no import";
    m_mapErrCode[0xA5] = "Key hold";
}

/*
 *@param : byErrType:1:驱动错误 0:其他错误
 */
void CDevPIN_XZF35::UpdateStatus(WORD wDevice, long lErrCode, BYTE byErrType/* = 0*/)
{
    m_stStatus.clear();
    m_stStatus.wDevice = wDevice;
    if(byErrType == 1){
        sprintf(m_stStatus.szErrCode, "1%02X", qAbs(lErrCode));
    } else {
        sprintf(m_stStatus.szErrCode, "0%02X", qAbs(lErrCode));
    }
}

bool CDevPIN_XZF35::ConvertKeyVal(BYTE bKey, EPP_KEYVAL &stKeyVal)
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

/*
 @param: byKCVMode: 0: none 1:self 2:zero
 */
long CDevPIN_XZF35::ReadSymmKeyKCV(UINT uKeyNo, BYTE byKcvMode, LPSTR lpKcv)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    long lRet;

    QByteArray vtCmd;
    QByteArray vtResult;

    READSYMMKEYKCV stReadSymmKeyKcv;
    stReadSymmKeyKcv.wKeyIdAndMode = EXWORDHL((WORD)(((uKeyNo + 1) << 4) + byKcvMode));
    vtCmd.append((LPSTR)&stReadSymmKeyKcv, sizeof(stReadSymmKeyKcv));    

    lRet = SendReadCmd(READ_SYMMETRIC_ENC_KEY_KCV, vtCmd, vtResult);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "密钥KCV获取失败：lRet=%d[uKeyNo=%02X, byKCVMode=%02X]", lRet, uKeyNo, byKcvMode);
        return lRet;
    }

    CAutoHex cHex(vtResult);
    strcpy(lpKcv, cHex.GetHex());
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SwitchBeep(bool bBeep)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    LOGDEVACTION();

    QByteArray vtResult;
    QByteArray vtCmd;

    BEEP stBeep;
    stBeep.bySwitch = bBeep ? 0x30 : 0x31;
    vtCmd.append((LPSTR)&stBeep, sizeof(stBeep));

    long lRet = SendReadCmd(SET_ENABLE_BUZZER, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "%s键盘声音失败：%d", lRet, bBeep ? "Open" : "Close");
        return lRet;
    }
    return ERR_SUCCESS;
}

long CDevPIN_XZF35::SwitchModeDESOrSM4(bool bDes)
{
    THISMODULE(__FUNCTION__);
    QByteArray vtCmd;
    QByteArray vtResult;

    SETSYMMKEYHDLMODE stSetSymmKeyHdlMode;
    if(bDes){
        stSetSymmKeyHdlMode.byAlgo = 0x00;
    } else {
        stSetSymmKeyHdlMode.byAlgo = 0x02;
    }

    vtCmd.append((LPSTR)&stSetSymmKeyHdlMode, sizeof(stSetSymmKeyHdlMode));

    long lRet = SendReadCmd(SET_SYMMETRIC_ENC_ALGORITHM, vtCmd, vtResult);
    if (lRet != ERR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "切换对称密钥处理模式命令失败：%d[%s]", lRet, bDes ? "DES" : "SM4");
        return lRet;
    }

    Log(ThisModule, 1, "切换对称密钥处理模式成功：%s", bDes ? "DES" : "SM4");
    return ERR_SUCCESS;
}
