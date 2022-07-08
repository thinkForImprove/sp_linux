#include "KeySlotHelper.h"
CQtSimpleMutex  m_cMutex;
static const char *gXorData = "5555555555555555";
//////////////////////////////////////////////////////////////////////////
CKeySlotHelper::CKeySlotHelper()
{
    ReadKeyFile();
}

CKeySlotHelper::~CKeySlotHelper()
{

}

//查找主密钥
PINKEYDATA CKeySlotHelper::FindMKeySlot(QByteArray strName, LPCSTR strEncKeyName/* = ""*/, bool bIsDesKey/* = true*/)
{
    AutoMutex(m_cMutex);
    PINKEYDATA stKey;
    if (FindKeyByName(strName, stKey, bIsDesKey))
        return stKey;

    PINKEYDATA stEncKey;
    stEncKey.uNum = KEY_NOT_NUM;
    if(strEncKeyName != nullptr && strlen(strEncKeyName) > 0){
        FindKeyByName(strEncKeyName, stEncKey, bIsDesKey);
    }

    strcpy(stKey.szName, strName);
    stKey.uNum    = KEY_NOT_NUM;
    if(bIsDesKey){
        stKey.dwType = 0;
    } else {
        stKey.dwType = 2;
    }

    mapUUINT vtKeyNum;
    UINT uKeyIdBase = bIsDesKey ? MKEY_BASE : MKEY_BASE_SM;
    UINT uKeyIdMax = bIsDesKey ? MKEY_MAX : MKEY_MAX_SM;
    for (UINT uNum = uKeyIdBase; uNum <= uKeyIdMax; uNum++)
    {
        vtKeyNum[uNum] = uNum;
    }
    for (auto &it : m_ltKeyData)
    {
        if (vtKeyNum.find(it.uNum) != vtKeyNum.end())
            vtKeyNum.erase(it.uNum);
    }
    if (!vtKeyNum.empty())
    {
        if(stEncKey.uNum == KEY_NOT_NUM){
            stKey.uNum = vtKeyNum.begin()->second;
            stKey.bNewKey = TRUE;
        } else {
            //有加密密钥时，密钥索引 > 加密密钥索引
            for(auto &it : vtKeyNum){
                if(it.second > stEncKey.uNum){
                    stKey.uNum = it.second;
                    stKey.bNewKey = TRUE;
                    break;                              //30-00-00-00(FT#0050)
                }
            }
        }

    }
    return stKey;
}

//查找工作密钥
PINKEYDATA CKeySlotHelper::FindWKeySlot(QByteArray strName, bool bIsDesKey/* = true*/, int iKeyType/* = PKU_CRYPT*/)
{
    AutoMutex(m_cMutex);
    PINKEYDATA stKey;
    if (FindKeyByName(strName, stKey, bIsDesKey))
        return stKey;

    strcpy(stKey.szName, strName);
    stKey.uNum = KEY_NOT_NUM;
    if(bIsDesKey){
        stKey.dwType = 1;
    } else {
        stKey.dwType = 3;
    }

    mapUUINT vtKeyNum;
    UINT uKeyIdBase = bIsDesKey ? WKEY_BASE : WKEY_BASE_SM;
    UINT uKeyIdMax = bIsDesKey ? WKEY_MAX : WKEY_MAX_SM;
    for (UINT uNum = uKeyIdBase; uNum <= uKeyIdMax; uNum++)
    {
        vtKeyNum[uNum] = uNum;
    }
    for (auto &it : m_ltKeyData)
    {
        if (vtKeyNum.find(it.uNum) != vtKeyNum.end())
            vtKeyNum.erase(it.uNum);
    }
    if (!vtKeyNum.empty())
    {
        if(bIsDesKey){
            stKey.uNum = vtKeyNum.begin()->second;
            stKey.bNewKey = TRUE;
        } else {
            //nNUm % 4: 0:TPK 1:TAK 2:TDK
            for(auto &it : vtKeyNum){
                if((iKeyType == PKU_FUNCTION && it.second % 4 == 0) ||
                   (iKeyType == PKU_MACING && it.second % 4 == 1) ||
                   (iKeyType == PKU_CRYPT && it.second % 4 == 2)){
                    stKey.uNum = it.second;
                    stKey.bNewKey = true;
                    break;
                }
            }
        }
    }
    return stKey;
}

//查找密钥统一接口
PINKEYDATA CKeySlotHelper::FindKey(QByteArray strName, bool bIsDesKey/* = true*/)
{
    AutoMutex(m_cMutex);
    QByteArray strNewName;
    PINKEYDATA stKey;
    stKey.uNum = KEY_NOT_NUM;
    if (FindKeyByName(strName, stKey, bIsDesKey))
        return stKey;

    do
    {
        if(!strName.contains("_DES_") && !strName.contains("_SM4_")){
            if(bIsDesKey){
                // 普通密钥
                strNewName = strName + KEY_USE_CRYPT;
                if (FindKeyByName(strNewName, stKey))
                    break;
                strNewName = strName + KEY_USE_FUNCTION;
                if (FindKeyByName(strNewName, stKey))
                    break;
                strNewName = strName + KEY_USE_MACING;
                if (FindKeyByName(strNewName, stKey))
                    break;
                strNewName = strName + KEY_USE_SVENCHEY;
                if (FindKeyByName(strNewName, stKey))
                    break;
            } else {
                // 国密密钥
                strNewName = strName + KEY_SM4_CRYPT;
                if (FindKeyByName(strNewName, stKey, false))
                    break;
                strNewName = strName + KEY_SM4_FUNCTION;
                if (FindKeyByName(strNewName, stKey, false))
                    break;
                strNewName = strName + KEY_SM4_MACING;
                if (FindKeyByName(strNewName, stKey,false))
                    break;
            }
        }
    } while (false);

    // 修改Key名为查找的原名称
    strcpy(stKey.szName, strName);
    return stKey;
}

void CKeySlotHelper::AddKey(const PINKEYDATA &stKeyData)
{
    AutoMutex(m_cMutex);
    for (auto &it : m_ltKeyData)
    {
        if (qstricmp(it.szName, stKeyData.szName) == 0 && stKeyData.dwType == it.dwType)
        {
            memcpy(&it, &stKeyData, sizeof(PINKEYDATA));
            SaveKeyFile();
            return;
        }
    }

    m_ltKeyData.push_back(stKeyData);
    SaveKeyFile();
    return;
}

long CKeySlotHelper::DeleteKey(QByteArray strName, bool bIsDesKey/* = true*/)
{
    AutoMutex(m_cMutex);
    int nPos = 0;
    QByteArray strNewName;
    for (auto it = m_ltKeyData.begin(); it != m_ltKeyData.end();)
    {
        strNewName = it->szName;
        if (-1 != (nPos = strNewName.indexOf("_SM4_"))) // 国密密钥
        {
            strNewName = strNewName.left(nPos);
        }
        else if (-1 != (nPos = strNewName.indexOf("_DES_"))) // 普通密钥
        {
            strNewName = strNewName.left(nPos);
        }

        if (qstricmp(strName, strNewName) == 0)
        {
            if((bIsDesKey && (it->dwType == 0 || it->dwType == 1)) ||               //30-00-00-00(FS#0005)
               (!bIsDesKey) && (it->dwType == 2 || it->dwType == 3)){               //30-00-00-00(FS#0005)
                it = m_ltKeyData.erase(it);
                continue;                                                           //30-00-00-00(FT#0050)
            }                                                                       //30-00-00-00(FS#0005)
        }
        it++;                                                                       //30-00-00-00(FT#0050)
    }

    if(SaveKeyFile() != 0){
        return -1;
    }
    return 0;
}

long CKeySlotHelper::DeleteAllKey()
{
    AutoMutex(m_cMutex);
    // 清空缓存
    m_ltKeyData.clear();
    // 清空文件
    fstream cFile(KEYSAVEFILE, ios::out | ios::trunc | ios::binary);
    if (!cFile.is_open())
        return -1;
    cFile.clear();
    cFile.close();
    return 0;
}

bool CKeySlotHelper::IsInvalidKey(const PINKEYDATA &stKeyData)
{
    return (stKeyData.uNum == KEY_NOT_NUM) ? true : false;
}

bool CKeySlotHelper::IsNewKey(const PINKEYDATA &stKeyData)
{
    if (stKeyData.uNum == KEY_NOT_NUM)
        return false;
    return (stKeyData.bNewKey) ? true : false;
}

bool CKeySlotHelper::IsMaxKey(const PINKEYDATA &stKeyData)
{
    return (stKeyData.uNum == KEY_NOT_NUM) ? true : false;
}

bool CKeySlotHelper::IsMKey(const PINKEYDATA &stKeyData)
{
    if (stKeyData.uNum == KEY_NOT_NUM)
        return false;
    return (stKeyData.dwType % 2 == 0) ? true : false;
}

bool CKeySlotHelper::IsWKey(const PINKEYDATA &stKeyData)
{
    if (stKeyData.uNum == KEY_NOT_NUM)
        return false;
    return (stKeyData.dwType % 2 != 0) ? true : false;
}

bool CKeySlotHelper::IsSMKey(const PINKEYDATA &stKeyData)
{
    if (stKeyData.uNum == KEY_NOT_NUM)
        return false;
    return (stKeyData.dwType > 1) ? true : false;
}

void CKeySlotHelper::GetAllKey(listPINKEYDATA &ltKeyData)
{
    AutoMutex(m_cMutex);
    QByteArray strName;
    ltKeyData.clear();
    for (auto it : m_ltKeyData)
    {
        strName = it.szName;
        int nPos = strName.indexOf("_SM4_"); // 国密密钥
        if (nPos == -1)
        {
            if (-1 == (nPos = strName.indexOf("_DES_")))// 普通密钥
            {
                ltKeyData.push_back(it);
                continue;
            }
        }

        // 只保存不重复的
        bool bExist = false;
        strName = strName.left(nPos);
        for (auto &itKey : ltKeyData)
        {
            if (strName == itKey.szName)
            {
                bExist = true;
                break;
            }
        }
        if (!bExist)
        {
            strcpy(it.szName, strName.constData());
            ltKeyData.push_back(it);
        }
    }

    return;
}

bool CKeySlotHelper::IsEmpty()
{
    AutoMutex(m_cMutex);
    return m_ltKeyData.empty();
}

bool CKeySlotHelper::IsAllMKey()
{
    AutoMutex(m_cMutex);
    for (auto &it : m_ltKeyData)
    {
        if (it.dwType % 2 != 0)
        {
            return false;
        }
    }
    return true;
}

//获取SM2公钥索引
//30-00-00-00(FS#0005)
PINKEYDATA CKeySlotHelper::FindSM2KeySlot(QByteArray strName)
{
    AutoMutex(m_cMutex);
    PINKEYDATA stKey;
    if (FindSM2Key(strName, stKey))
        return stKey;

    strcpy(stKey.szName, strName);
    stKey.uNum   = KEY_NOT_NUM;
    stKey.dwType = 4;

    mapUUINT vtKeyNum;
    for (UINT uNum = SM2KEY_BASE; uNum <= SM2kEY_MAX; uNum++)
    {
        vtKeyNum[uNum] = uNum;
    }
    for (auto &it : m_ltKeyData)
    {
        if (vtKeyNum.find(it.uNum) != vtKeyNum.end()){
            if(it.dwType == 4){                 //密钥类型检查
                vtKeyNum.erase(it.uNum);
            }
        }
    }
    if (!vtKeyNum.empty())
    {
        stKey.uNum = vtKeyNum.begin()->second;
        stKey.bNewKey = TRUE;
    }
    return stKey;
}

//查找SM2密钥
//30-00-00-00(FS#0005)
bool CKeySlotHelper::FindSM2Key(QByteArray strName, PINKEYDATA &stKey)
{
    for (auto &it : m_ltKeyData)
    {
        if(it.dwType == 4){
            if (qstricmp(strName, it.szName) == 0)
            {
                memcpy(&stKey, &it, sizeof(PINKEYDATA));
                return true;
            }
        }
    }
    return false;
}

//删除SM2密钥
//30-00-00-00(FS#0005)
long CKeySlotHelper::DeleteSM2Key(QByteArray strName)
{
    AutoMutex(m_cMutex);
    for (auto it = m_ltKeyData.begin(); it != m_ltKeyData.end(); it++)
    {
        if((it->dwType == 4) && (qstricmp(strName, it->szName) == 0)){
            m_ltKeyData.erase(it);
            if(SaveKeyFile() != 0){
                return -1;
            }
            break;
        }
    }

    return 0;
}

PINKEYDATA CKeySlotHelper::FindRSAKeySlot(QByteArray strName, bool bPkey/* = true*/)
{
    AutoMutex(m_cMutex);
    PINKEYDATA stKey;
    if (FindRSAKey(strName, stKey))
        return stKey;

    strcpy(stKey.szName, strName);
    stKey.uNum   = KEY_NOT_NUM;
    stKey.dwType = 5;

    mapUUINT vtKeyNum;
    UINT uKeyIdBase = bPkey ? RSA_PK_BASE : RSA_SK_BASE;
    UINT uKeyIdMax = bPkey ? RSA_PK_MAX : RSA_SK_MAX;
    for (UINT uNum = uKeyIdBase; uNum <= uKeyIdMax; uNum++)
    {
        vtKeyNum[uNum] = uNum;
    }
    for (auto &it : m_ltKeyData)
    {
        if (vtKeyNum.find(it.uNum) != vtKeyNum.end()){
            if(it.dwType == 5){                 //密钥类型检查
                vtKeyNum.erase(it.uNum);
            }
        }
    }
    if (!vtKeyNum.empty())
    {
        stKey.uNum = vtKeyNum.begin()->second;
        stKey.bNewKey = TRUE;
    }
    return stKey;
}

bool CKeySlotHelper::FindRSAKey(QByteArray strName, PINKEYDATA &stKey)
{
    for (auto &it : m_ltKeyData)
    {
        if(it.dwType == 5){
            if (qstricmp(strName, it.szName) == 0)
            {
                memcpy(&stKey, &it, sizeof(PINKEYDATA));
                return true;
            }
        }
    }
    return false;
}

long CKeySlotHelper::DeleteRSAKey(QByteArray strName)
{
    AutoMutex(m_cMutex);
    for (auto it = m_ltKeyData.begin(); it != m_ltKeyData.end(); it++)
    {
        if((it->dwType == 5) && (qstricmp(strName, it->szName) == 0)){
            m_ltKeyData.erase(it);
            if(SaveKeyFile() != 0){
                return -1;
            }
            break;
        }
    }

    return 0;
}

long CKeySlotHelper::ReadKeyFile()
{
    // 读取文件
    m_ltKeyData.clear();
    fstream cFile(KEYSAVEFILE, ios::in | ios::binary);
    if (!cFile.is_open())
        return -1;

    PINKEYDATA stKey;
    UINT uSize = sizeof(PINKEYDATA);
    char szData[256] = { 0 };
    char szXor[64]   = { 0 };
    while (true)
    {
        cFile.read(szData, uSize);
        if (cFile.eof())
            break;
        memcpy(&stKey, szData, uSize);
        XOR(stKey.szKCV, gXorData, 16, szXor);
        memcpy(stKey.szKCV, szXor, 16);
        m_ltKeyData.push_back(stKey);
    }
    cFile.close();
    return 0;
}

long CKeySlotHelper::SaveKeyFile()
{
    // 每次都是新文件
    fstream cFile(KEYSAVEFILE, ios::out | ios::trunc | ios::binary);
    if (!cFile.is_open())
        return -1;

    // 写文件
    PINKEYDATA stKey;
    char szXor[64] = {0};
    UINT uSize = sizeof(PINKEYDATA);
    for (auto &it : m_ltKeyData)
    {
        it.bNewKey = FALSE;// 标志为旧密钥
        memcpy(&stKey, &it, uSize);
        XOR(stKey.szKCV, gXorData, 16, szXor);
        memcpy(stKey.szKCV, szXor, 16);
        cFile.write((char *)&stKey, uSize);
    }
    cFile.close();
    return 0;
}

void CKeySlotHelper::XOR(const char *pX, const char *pR, UINT uLen, char *pXOR)
{
    for (UINT i = 0; i < uLen; i++)
    {
        pXOR[i] = pX[i] ^ pR[i];
    }
    return ;
}

bool CKeySlotHelper::FindKeyByName(QByteArray strName, PINKEYDATA &stKey, bool bIsDesKey/* = true*/)
{
    for (auto &it : m_ltKeyData)
    {
        if((bIsDesKey && it.dwType <= 1) || (!bIsDesKey && it.dwType > 1)){
            if (qstricmp(strName, it.szName) == 0)
            {
                memcpy(&stKey, &it, sizeof(PINKEYDATA));
                return true;
            }
        }
    }
    return false;
}

/*-----------------------------------CKeySlotHelperXZ---------------------------*/
CKeySlotHelperXZ::CKeySlotHelperXZ()
{
    ReadKeyFile();
}

CKeySlotHelperXZ::~CKeySlotHelperXZ()
{

}

PINKEYDATA CKeySlotHelperXZ::FindKeySlot(QByteArray strName)
{
    AutoMutex(m_cMutex);
    PINKEYDATA stKey;
    if (FindKeyByName(strName, stKey))
        return stKey;

    strcpy(stKey.szName, strName);
    stKey.uNum    = KEY_NOT_NUM;
    stKey.dwType  = 0;

    mapUUINT vtKeyNum;
    for (UINT uNum = 0; uNum < XZ_SYMMETRIC_KEY_MAX; uNum++)
    {
        vtKeyNum[uNum] = uNum;
    }
    for (auto &it : m_ltKeyData)
    {
        if(it.dwType != 2){
            if (vtKeyNum.find(it.uNum) != vtKeyNum.end())
                vtKeyNum.erase(it.uNum);
        }
    }
    if (!vtKeyNum.empty())
    {
        stKey.uNum = vtKeyNum.begin()->second;
        stKey.bNewKey = TRUE;
    }
    return stKey;
}

PINKEYDATA CKeySlotHelperXZ::FindSMKeySlot(QByteArray strName)
{
    AutoMutex(m_cMutex);
    PINKEYDATA stKey;
    if (FindSMKeyByName(strName, stKey))
        return stKey;

    strcpy(stKey.szName, strName);
    stKey.uNum = KEY_NOT_NUM;
    stKey.dwType = 2;

    mapUUINT vtKeyNum;
    for (UINT uNum = 0; uNum < XZ_SYMMETRIC_KEY_MAX; uNum++)
    {
        vtKeyNum[uNum] = uNum;
    }
    for (auto &it : m_ltKeyData)
    {
        if(it.dwType == 2){
            if (vtKeyNum.find(it.uNum) != vtKeyNum.end())
                vtKeyNum.erase(it.uNum);
        }
    }

    if(!vtKeyNum.empty()){
        stKey.uNum = vtKeyNum.begin()->second;
        stKey.bNewKey = TRUE;
    }

    return stKey;
}

PINKEYDATA CKeySlotHelperXZ::FindKey(QByteArray strName)
{
    AutoMutex(m_cMutex);
    PINKEYDATA stKey;
    if (!FindKeyByName(strName, stKey)){
        stKey.uNum = KEY_NOT_NUM;
    }

    return stKey;
}

PINKEYDATA CKeySlotHelperXZ::FindSMKey(QByteArray strName)
{
    AutoMutex(m_cMutex);
    PINKEYDATA stKey;
    if (!FindSMKeyByName(strName, stKey)){
        stKey.uNum = KEY_NOT_NUM;
    }

    return stKey;
}

void CKeySlotHelperXZ::AddKey(const PINKEYDATA &stKeyData)
{
    AutoMutex(m_cMutex);
    for (auto &it : m_ltKeyData)
    {
        if (it.dwType == stKeyData.dwType && qstricmp(it.szName, stKeyData.szName) == 0)
        {
            memcpy(&it, &stKeyData, sizeof(PINKEYDATA));
            SaveKeyFile();
            return;
        }
    }

    m_ltKeyData.push_back(stKeyData);
    SaveKeyFile();
    return;
}

long CKeySlotHelperXZ::DeleteKey(QByteArray strName)
{
    AutoMutex(m_cMutex);
    for (auto it = m_ltKeyData.begin(); it != m_ltKeyData.end(); it++)
    {
        if (it->dwType != 2 && qstricmp(strName.constData(), it->szName) == 0)
        {
            m_ltKeyData.erase(it);
            return SaveKeyFile();
        }
    }
    return -1;
}

long CKeySlotHelperXZ::DeleteSMKey(QByteArray strName)
{
    AutoMutex(m_cMutex);
    for (auto it = m_ltKeyData.begin(); it != m_ltKeyData.end(); it++)
    {
        if (it->dwType == 2 && qstricmp(strName.constData(), it->szName) == 0)
        {
            m_ltKeyData.erase(it);
            return SaveKeyFile();
        }
    }
    return -1;
}

long CKeySlotHelperXZ::DeleteAllKey()
{
    AutoMutex(m_cMutex);
    // 清空缓存
    m_ltKeyData.clear();
    // 清空文件
    fstream cFile(KEYSAVEFILE, ios::out | ios::trunc | ios::binary);
    if (!cFile.is_open())
        return -1;
    cFile.clear();
    cFile.close();
    return 0;
}

bool CKeySlotHelperXZ::IsInvalidKey(const PINKEYDATA &stKeyData)
{
    return (stKeyData.uNum == KEY_NOT_NUM) ? true : false;
}

bool CKeySlotHelperXZ::IsNewKey(const PINKEYDATA &stKeyData)
{
    if(stKeyData.uNum == KEY_NOT_NUM){
        return false;
    }
    return stKeyData.bNewKey ? true : false;
}

bool CKeySlotHelperXZ::IsMaxKey(const PINKEYDATA &stKeyData)
{
    return (stKeyData.uNum == KEY_NOT_NUM) ? true : false;
}

void CKeySlotHelperXZ::GetAllKey(listPINKEYDATA &ltKeyData)
{
    AutoMutex(m_cMutex);
    ltKeyData.clear();
    ltKeyData = m_ltKeyData;

    return;
}

void CKeySlotHelperXZ::GetAllDesKey(listPINKEYDATA &ltKeyData)
{
    AutoMutex(m_cMutex);
    ltKeyData.clear();
    for(auto &it : m_ltKeyData){
        if(it.dwType != 2){
            ltKeyData.push_back(it);
        }
    }

    return;
}

void CKeySlotHelperXZ::GetAllSMKey(listPINKEYDATA &ltKeyData)
{
    AutoMutex(m_cMutex);
    ltKeyData.clear();
    for(auto &it : m_ltKeyData){
        if(it.dwType == 2){
            ltKeyData.push_back(it);
        }
    }

    return;
}

bool CKeySlotHelperXZ::IsEmpty()
{
    AutoMutex(m_cMutex);
    return m_ltKeyData.empty();
}

long CKeySlotHelperXZ::ReadKeyFile()
{
    // 读取文件
    m_ltKeyData.clear();
    fstream cFile(KEYSAVEFILE, ios::in | ios::binary);
    if (!cFile.is_open())
        return -1;

    PINKEYDATA stKey;
    UINT uSize = sizeof(PINKEYDATA);
    char szData[256] = { 0 };
    char szXor[64]   = { 0 };
    while (true)
    {
        cFile.read(szData, uSize);
        if (cFile.eof())
            break;
        memcpy(&stKey, szData, uSize);
        XOR(stKey.szKCV, gXorData, 16, szXor);
        memcpy(stKey.szKCV, szXor, 16);
        m_ltKeyData.push_back(stKey);
    }
    cFile.close();
    return 0;
}

long CKeySlotHelperXZ::SaveKeyFile()
{
    // 每次都是新文件
    fstream cFile(KEYSAVEFILE, ios::out | ios::trunc | ios::binary);
    if (!cFile.is_open())
        return -1;

    // 写文件
    PINKEYDATA stKey;
    char szXor[64] = {0};
    UINT uSize = sizeof(PINKEYDATA);
    for (auto &it : m_ltKeyData)
    {
        it.bNewKey = FALSE;// 标志为旧密钥
        memcpy(&stKey, &it, uSize);
        XOR(stKey.szKCV, gXorData, 16, szXor);
        memcpy(stKey.szKCV, szXor, 16);
        cFile.write((char *)&stKey, uSize);
    }
    cFile.close();
    return 0;
}

void CKeySlotHelperXZ::XOR(const char *pX, const char *pR, UINT uLen, char *pXOR)
{
    for (UINT i = 0; i < uLen; i++)
    {
        pXOR[i] = pX[i] ^ pR[i];
    }
    return ;
}

bool CKeySlotHelperXZ::FindKeyByName(QByteArray strName, PINKEYDATA &stKey)
{
    for (auto &it : m_ltKeyData)
    {
        if (it.dwType != 2 && qstricmp(strName, it.szName) == 0)
        {
            memcpy(&stKey, &it, sizeof(PINKEYDATA));
            return true;
        }
    }
    return false;
}

bool CKeySlotHelperXZ::FindSMKeyByName(QByteArray strName, PINKEYDATA &stKey)
{
    for (auto &it : m_ltKeyData)
    {
        if (it.dwType == 2 && qstricmp(strName, it.szName) == 0)
        {
            memcpy(&stKey, &it, sizeof(PINKEYDATA));
            return true;
        }
    }
    return false;
}
