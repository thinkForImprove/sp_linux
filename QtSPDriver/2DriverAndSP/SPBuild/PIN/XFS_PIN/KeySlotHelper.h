#pragma once
#include "QtTypeDef.h"
#include "SimpleMutex.h"
#include "IDevPIN.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#include <fstream>
#include <algorithm>
using namespace std;

//////////////////////////////////////////////////////////////////////////
/*   be aware of alignment   */
#pragma pack(push,1)
//////////////////////////////////////////////////////////////////////////
#ifdef QT_WIN32
#define KEYSAVEFILE  "C:/CFES/ETC/KeySlotData.key"
#else
#define KEYSAVEFILE  "/usr/local/CFES/ETC/KeySlotData.key"
#endif

//////////////////////////////////////////////////////////////////////////
#define KEY_USE_CRYPT                       "_DES_CRYPT"
#define KEY_USE_FUNCTION                    "_DES_FUNCTION"
#define KEY_USE_MACING                      "_DES_MACING"
#define KEY_USE_SVENCHEY                    "_DES_SVENCKEY"
//////////////////////////////////////////////////////////////////////////
#define KEY_SM4_CRYPT                       "_SM4_CRYPT"
#define KEY_SM4_FUNCTION                    "_SM4_FUNCTION"
#define KEY_SM4_MACING                      "_SM4_MACING"
//////////////////////////////////////////////////////////////////////////
#define KEY_RSA_SKEY_SUFFIX                 "_RSA_SKEY"                     //30-00-00-00(FS#0006)
//////////////////////////////////////////////////////////////////////////
#define KEY_NOT_NUM                         (0xFF) // 表示没有密钥
/*******************************  DES  *********************************/
#define MKEY_BASE                           (0x10)
#define MKEY_MAX                            (0x2F)
#define WKEY_BASE                           (0x30)
#define WKEY_MAX                            (0x7F)
/*******************************  SM   *********************************/
#define MKEY_BASE_SM                        (0x01)
#define MKEY_MAX_SM                         (0x1F)
#define WKEY_BASE_SM                        (0x20)
#define WKEY_MAX_SM                         (0x7F)
/******************************* SM2  *********************************/
//0x00~0x06有固定用途,暂时保留
#define SM2KEY_BASE                         (0x07)                  //30-00-00-00(FS#0005)
#define SM2kEY_MAX                          (0x1A)                  //30-00-00-00(FS#0005)
#define RSA_SK_BASE                         (0x20)                  //30-00-00-00(FS#0006)
#define RSA_SK_MAX                          (0x24)                  //30-00-00-00(FS#0006)
#define RSA_PK_BASE                         (0x25)                  //30-00-00-00(FS#0006)
#define RSA_PK_MAX                          (0x29)                  //30-00-00-00(FS#0006)
//////////////////////////////////////////////////////////////////////////
typedef struct tag_pin_key_data
{
    char szName[64];            // 密钥名称
    char szKCV[64];             // 密钥校验值
    UINT uNum;                  // 保存在键盘中的序号
    DWORD dwUse;                // 密钥用途
    DWORD dwType;               // 密钥类型：0->主密钥，1工作密钥，2->国密主密钥, 3->国密工作密钥, 4->SM2密钥, 5->RSA密钥  //30-00-00-00(FS#0005)　//30-00-00-00(FS#0006)
    DWORD dwKeyLen;             // 密钥长度
    BOOL  bNewKey;              // 是否为新密钥
    BYTE  byVersion;            // 密钥版本, BCD 编码, 默认值为 0xff
    BYTE  byGeneration;         // 密钥的年代, BCD 编码, 默认值为 0xff
    BYTE  byszActivatingDate[4];// 密钥的激活日期，BCD 编码, 格式为 YYYYMMD, 默认值为 0xffffffff
    BYTE  byszExpiryDate[4];    // 密钥的有效期，BCD 编码，格式为 YYYYMMDD, 默认值为 0xffffffff

    tag_pin_key_data() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_pin_key_data)); }   
} PINKEYDATA;
typedef list<PINKEYDATA>        listPINKEYDATA;
typedef std::map<UINT, UINT>    mapUUINT;
//////////////////////////////////////////////////////////////////////////
class CKeySlotHelper
{
public:
    CKeySlotHelper();
    ~CKeySlotHelper();
public:
    // 密钥序号
    PINKEYDATA FindMKeySlot(QByteArray strName, LPCSTR strEncKeyName = "", bool bIsDesKey = true);
    PINKEYDATA FindWKeySlot(QByteArray strName, bool bIsDesKey = true, int iKeyType = PKU_CRYPT);
    // 查询密钥
    PINKEYDATA FindKey(QByteArray strName, bool bIsDesKey = true);
    void GetAllKey(listPINKEYDATA &ltKeyData);
    // 添加密钥
    void AddKey(const PINKEYDATA &stKeyData);
    // 删除密钥
    long DeleteKey(QByteArray strName, bool bIsDesKey = true);
    long DeleteAllKey();
    // 无此密钥
    bool IsInvalidKey(const PINKEYDATA &stKeyData);
    // 是否为新密钥序号
    bool IsNewKey(const PINKEYDATA &stKeyData);
    // 是否已存满密钥
    bool IsMaxKey(const PINKEYDATA &stKeyData);
    // 判断密钥类型
    bool IsMKey(const PINKEYDATA &stKeyData);
    bool IsWKey(const PINKEYDATA &stKeyData);
    bool IsSMKey(const PINKEYDATA &stKeyData);

    // 是否有密钥
    bool IsEmpty();
    // 全部是主密钥
    bool IsAllMKey();

    //SM2密钥处理
    PINKEYDATA FindSM2KeySlot(QByteArray strName);          //30-00-00-00(FS#0005)
    bool FindSM2Key(QByteArray strName, PINKEYDATA &stKey); //30-00-00-00(FS#0005)
    long DeleteSM2Key(QByteArray strName);                  //30-00-00-00(FS#0005)

    //RSA密钥处理
    PINKEYDATA FindRSAKeySlot(QByteArray strName, bool bPkey = true);   //30-00-00-00(FS#0006)
    bool FindRSAKey(QByteArray strName, PINKEYDATA &stKey); //30-00-00-00(FS#0006)
    long DeleteRSAKey(QByteArray strName);                  //30-00-00-00(FS#0006)
private:
    // 读取密钥数据
    long ReadKeyFile();
    // 保存密钥数据
    long SaveKeyFile();
    // 异或加密
    void XOR(const char *pX, const char *pR, UINT uLen, char *pXOR);
    // 查找密钥
    bool FindKeyByName(QByteArray strName, PINKEYDATA &stKey, bool bIsDesKey = true);
private:
//    CSimpleMutex    m_cMutex;
    listPINKEYDATA  m_ltKeyData;
};

#define XZ_SYMMETRIC_KEY_MAX  64			//30-00-00-00(FS#0003)
#define XZ_ASYMMETRIC_KEY_MAX 16			//30-00-00-00(FS#0003)

class CKeySlotHelperXZ
{
public:
    CKeySlotHelperXZ();
    ~CKeySlotHelperXZ();
public:
    // 密钥序号
    PINKEYDATA FindKeySlot(QByteArray strName);
    PINKEYDATA FindSMKeySlot(QByteArray strName);

    PINKEYDATA FindAsymmKeySlot(QByteArray strName, int iKeyType = 4);		//30-00-00-00(FS#0003)
    // 查询密钥
    PINKEYDATA FindKey(QByteArray strName);
    PINKEYDATA FindSMKey(QByteArray strName);
    PINKEYDATA FindAsymmKey(QByteArray strName, int iKeyType = 4);		   //30-00-00-00(FS#0003)

    // 保存密钥
    void AddKey(const PINKEYDATA &stKeyData);
    // 删除密钥
    long DeleteKey(QByteArray strName);
    long DeleteSMKey(QByteArray strName);
    long DeleteAsymmKey(QByteArray strName, int iKeyType = 4);	          //30-00-00-00(FS#0003)
    long DeleteAllKey();
    // 无此密钥
    bool IsInvalidKey(const PINKEYDATA &stKeyData);
    // 是否为新密钥序号
    bool IsNewKey(const PINKEYDATA &stKeyData);
    // 是否已存满密钥
    bool IsMaxKey(const PINKEYDATA &stKeyData);
    // 获取全部密钥
    void GetAllKey(listPINKEYDATA &ltKeyData);
    void GetAllDesKey(listPINKEYDATA &ltKeyData);
    void GetAllSMKey(listPINKEYDATA &ltKeyData);
    void GetAllAsymmKey(listPINKEYDATA &ltKeyData);							//30-00-00-00(FS#0003)
    // 是否有密钥
    bool IsEmpty();

private:
    // 读取密钥数据
    long ReadKeyFile();
    // 保存密钥数据
    long SaveKeyFile();
    // 异或加密
    void XOR(const char *pX, const char *pR, UINT uLen, char *pXOR);
    // 查找密钥
    bool FindKeyByName(QByteArray strName, PINKEYDATA &stKey);
    bool FindSMKeyByName(QByteArray strName, PINKEYDATA &stKey);
    bool FindAsymmKeyByName(QByteArray strName, PINKEYDATA &stKey, int iKeyType = 4);		//30-00-00-00(FS#0003)

private:
//    CQtSimpleMutex  m_cMutex;
    listPINKEYDATA  m_ltKeyData;
};

class CKeySlotHelperC90
{
public:
    CKeySlotHelperC90();
    ~CKeySlotHelperC90();
public:
    // 密钥序号
    PINKEYDATA FindMKeySlot(QByteArray strName, LPCSTR strEncKeyName = "", bool bIsDesKey = true);
    PINKEYDATA FindWKeySlot(QByteArray strName, bool bIsDesKey = true, int iKeyType = PKU_CRYPT);
    // 查询密钥
    PINKEYDATA FindKey(QByteArray strName, bool bIsDesKey = true);
    void GetAllKey(listPINKEYDATA &ltKeyData);
    // 添加密钥
    void AddKey(const PINKEYDATA &stKeyData);
    // 删除密钥
    long DeleteKey(QByteArray strName, bool bIsDesKey = true);
    long DeleteAllKey();
    // 无此密钥
    bool IsInvalidKey(const PINKEYDATA &stKeyData);
    // 是否为新密钥序号
    bool IsNewKey(const PINKEYDATA &stKeyData);
    // 是否已存满密钥
    bool IsMaxKey(const PINKEYDATA &stKeyData);
    // 判断密钥类型
    bool IsMKey(const PINKEYDATA &stKeyData);
    bool IsWKey(const PINKEYDATA &stKeyData);
    bool IsSMKey(const PINKEYDATA &stKeyData);

    // 是否有密钥
    bool IsEmpty();
    // 全部是主密钥
    bool IsAllMKey();

    //SM2密钥处理
    PINKEYDATA FindSM2KeySlot(QByteArray strName, bool bEppPKey = true);
    bool FindSM2Key(QByteArray strName, PINKEYDATA &stKey);
    long DeleteSM2Key(QByteArray strName);
    void GetAllSM2Key(listPINKEYDATA &ltKeyData);

    //RSA密钥处理
    PINKEYDATA FindRSAKeySlot(QByteArray strName, bool bPkey = true);
    bool FindRSAKey(QByteArray strName, PINKEYDATA &stKey);
    long DeleteRSAKey(QByteArray strName);

    std::map<UINT, BYTE>& GetDesKeyLenInfo();
private:
    // 读取密钥数据
    long ReadKeyFile();
    // 保存密钥数据
    long SaveKeyFile();
    // 异或加密
    void XOR(const char *pX, const char *pR, UINT uLen, char *pXOR);
    // 查找密钥
    bool FindKeyByName(QByteArray strName, PINKEYDATA &stKey, bool bIsDesKey = true);
private:
    listPINKEYDATA  m_ltKeyData;
    std::map<UINT, BYTE> m_keyIdMapKeyLen;
};

//////////////////////////////////////////////////////////////////////////
/* restore alignment */
#pragma pack(pop)
