#pragma once
/***************************************************************
* 文件名称：IDevPIN.h
* 文件描述：声明密码键盘底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年7月18日
* 文件版本：1.0.1
****************************************************************/
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "IAllDevPort.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////
#if defined(IDEVPIN_LIBRARY)
#define DEVPIN_EXPORT     Q_DECL_EXPORT
#else
#define DEVPIN_EXPORT     Q_DECL_IMPORT
#endif

//////////////////////////////////////////////////////////////////////////
// 通信返回码，请查看头文件：IAllDevPort.h
//////////////////////////////////////////////////////////////////////////
// 返回码定义
#define ERR_SUCCESS                                  0x00       // 操作成功
#define ERR_REDATA                                  (-100)      // 无效返回数据
#define ERR_PARAM                                   (-101)      // 无效参数
#define ERR_NOT_SUPPORT                             (-102)      // 不支持功能

#define ERR_HW_A980_D0                              (-0xD0)     // SM2加密失败
#define ERR_HW_A980_D1                              (-0xD1)     // SM2解密失败
#define ERR_HW_A980_D2                              (-0xD2)     // SM2签名失败
#define ERR_HW_A980_D3                              (-0xD3)     // SM2验签失败
#define ERR_HW_A980_D4                              (-0xD4)     // SM2密钥交换失败
#define ERR_HW_A980_D5                              (-0xD5)     // SM2密钥交换校验失败
#define ERR_HW_A980_D6                              (-0xD6)     // 国密硬件错误
#define ERR_HW_A980_D7                              (-0xD7)     // 国密硬件错误
#define ERR_HW_A980_D8                              (-0xD8)     // SM4密钥无效
#define ERR_HW_NOT_A980                             (-0xDE)     // 硬件不支持国密

#define ERR_RSA_NOT_INIT                            (-0xDA)      // RSA密钥没有初始化
#define ERR_RSA_INVALID                             (-0xDB)      // RSA密钥无效，要重新下载
#define ERR_CDM_MAC                                 (-0xDC)      // 命令MAC校验失败
#define ERR_TAMPERED                                (-0xDD)      // 硬件已被篡改

#define ERR_NO_PIN                                  (-0xE2)      // Pinblock无Pin
#define ERR_KEY_LENGTH                              (-0xE3)      // 密钥长度错误
#define ERR_NO_KEYID                                (-0xE4)      // 密钥不存在
#define ERR_KEYID                                   (-0xE5)      // 无效密钥ID
#define ERR_KEY_PAIR                                (-0xE6)      // 无效密钥对
#define ERR_KEYKCV                                  (-0xE7)      // 密钥校验值KCV错误
#define ERR_CMD_LEN                                 (-0xE8)      // 命令长度错误
#define ERR_CMD_DATA                                (-0xE9)      // 无效命令参数
#define ERR_CDM                                     (-0xEA)      // 无效命令
#define ERR_PARAW_TYPE                              (-0xEB)      // 参数类型错误
#define ERR_AUT                                     (-0xEC)      // 认证失败
#define ERR_AUT_LOCK                                (-0xED)      // 认证失败超3次锁定
#define ERR_INPUT_TIMEOUT                           (-0xEE)      // 输入超过10分钟超时
#define ERR_OTHER                                   (-0xEF)      // 其他异常错误

#define ERR_RSA_F1                                  (-0xF1)      // RSA密钥异常
#define ERR_RSA_F2                                  (-0xF2)      // RSA密钥异常
#define ERR_RSA_F3                                  (-0xF3)      // RSA密钥异常
#define ERR_RSA_F4                                  (-0xF4)      // RSA密钥异常
#define ERR_RSA_F5                                  (-0xF5)      // RSA密钥异常
#define ERR_RSA_F6                                  (-0xF6)      // RSA密钥异常
#define ERR_RSA_F7                                  (-0xF7)      // RSA密钥异常
#define ERR_HWARE                                   (-0xFE)      // 硬件故障
#define ERR_HWARE_SWITCH                            (-0xFF)      // 防拆开关未关闭

//////////////////////////////////////////////////////////////////////////
// 状态
enum DEVICE_STATUS
{
    DEVICE_OFFLINE = 0,
    DEVICE_ONLINE = 1,
    DEVICE_HWERROR = 2,
};

// 密钥模式
enum PINKEYUSE
{
    PKU_CRYPT = 0x01,
    PKU_FUNCTION = 0x02,
    PKU_MACING = 0x04,
    PKU_KEYENCKEY = 0x20,
    PKU_NODUPLICATE = 0x40,
    PKU_SVENCKEY = 0x80,
    PKU_CONSTRUCT = 0x100,
    PKU_SECURECONSTRUCT = 0x200,
};

// 密码块模式
enum PINFORMAT
{
    PIN_ASCII = 0x01,
    PIN_ISO9564_0 = 0x02,
    PIN_ISO9564_1 = 0x03,
    PIN_ISO9564_2 = 0x04,
    PIN_ISO9564_3 = 0x05,
    PIN_IBM3624 = 0x06,
    PIN_VISA = 0x07,
    PIN_VISA3 = 0x08,
    PIN_SM4 = 0x09,
};

// 加解密模式
enum CRYPTMODE
{
    ECB_EN = 0x01,
    ECB_DE = 0x02,
    CBC_EN = 0x03,
    CBC_DE = 0x04,
};

// MAC模式
enum MACTYPE
{
    MAC_PBOC = 0x01,
    MAC_ANSIX99 = 0x02,
    MAC_ANSIX919 = 0x03,
    MAC_ISO16609 = 0x04,
    MAC_CBC = 0x05,
    MAC_BANKSYS = 0x06,
};

//30-00-00-00(FS#0006) add start
// RSA加密算法
enum RSADATAALGOTYPE
{
    RSA_ENC_PKCS1_V1_5 = 0,
    RSA_ENC_SHA_1 = 1,
    RSA_ENC_SHA_256 = 2
};

//RSA签名算法
enum RSASIGNALGOTYPE
{
    RSA_SIGN_NO = 0,
    RSA_SIGN_SSA_PKCS1_V1_5 = 1,
    RSA_SIGN_SSA_PSS = 2
};
//30-00-00-00(FS#0006) add end
//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_pin_status
{
    WORD wDevice;               // 设备状态
    char szErrCode[8];          // 三位的错误码

    tag_dev_pin_status() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_dev_pin_status)); }
} DEVPINSTATUS;

typedef struct tag_epp_key_val
{
    BYTE byKeyVal;
    char szKeyName[32];
} EPP_KEYVAL;
//////////////////////////////////////////////////////////////////////////
//接口类定义
struct  IDevPIN
{
    // 释放接口
    virtual void Release() = 0;
    // 打开与设备的连接，格式：COM2:9600,N,8,1
    virtual long Open(LPCSTR lpMode) = 0;
    // 关闭与设备的连接
    virtual long Close() = 0;
    // 初始化，此会删除所有密钥，RSA密钥除外
    virtual long InitEpp() = 0;
    // 复位
    virtual long Reset() = 0;
    // 查状态
    virtual long GetStatus(DEVPINSTATUS &stStatus) = 0;
    // 安装状态:bClear表示是否清除自毁标志，返回值：0->已安装，1->未安装，其他->未知
    virtual long InstallStatus(bool bClear = false) = 0;
    // 读取固件信息
    virtual long GetFirmware(LPSTR lpFWVer) = 0;
    // 设置按键值
    virtual long SetKeyValue(LPCSTR lpKeyVal) = 0;
    // 设置激活按键
    virtual long SetActiveKey(DWORD dwActiveFuncKey, DWORD dwActiveFDKKey) = 0;
    // 导入主密钥，uMKeyNo：0x01 ~ 0x1F
    virtual long ImportMKey(UINT uMKeyNo, UINT uKeyMode, LPCSTR lpKeyVal, LPSTR lpKCV) = 0;
    // 导入工作密钥，包括：PIN密钥，MAC密钥，加解密密钥，向量密钥
    virtual long ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode = 2) = 0;
    // 设置DES KCV解密Key
    virtual void SetDesKcvDecryptKey(UINT uKeyNo) = 0;
    // 删除密钥，默认删除全部
    virtual long DeleteKey(UINT uKeyNo = 0) = 0;
    // 明文输入
    virtual long TextInput() = 0;
    // 密文输入
    virtual long PinInput(UINT uMinLen = 4, UINT uMaxLen = 6) = 0;
    // 读取输入按键，大于0为按键值，小于0为失败
    virtual long ReadKeyPress(EPP_KEYVAL &stKeyVal, DWORD dwTimeOut = 250) = 0;
    // 取消输入
    virtual long CancelInput(bool bPinInput = false) = 0;
    // 读取PINBlock
    virtual long GetPinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock) = 0;
    // 加解密
    virtual long DataCrypt(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, LPSTR lpData) = 0;
    // 计算MAC
    virtual long DataMAC(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, LPSTR lpData) = 0;
    // 从键盘中获取32位长度的随机数
    virtual long RandData(LPSTR lpData) = 0;
    // 读取对称密钥KCV
    virtual long ReadSymmKeyKCV(UINT uKeyNo, BYTE byKcvMode, LPSTR lpKcv) = 0;

    // ---------------------SM国密支持---------------------------------------------------
    // 读取国密芯片（A980）固件信息
    virtual long SMGetFirmware(LPSTR lpSMVer) = 0;
    // 导入主密钥，uMKeyNo：0x01 ~ 0x10
    virtual long SM4ImportMKey(UINT uMKeyNo, LPCSTR lpKeyVal, LPSTR lpKCV) = 0;
    virtual long SM4ImportMKeyForGD(UINT uMKeyNo, UINT uDKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV) = 0;			//30-00-00-00（FS#0003）
    // 导入工作密钥，包括：PIN密钥，MAC密钥，加解密密钥，向量密钥
    virtual long SM4ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode = 0) = 0;
    // 远程下载密钥
    virtual long SM4RemoteImportKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostSM2PKeyNo, UINT uKeyUse, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal, LPSTR lpKCV) = 0;
    // 加解密
    virtual long SM4CryptData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, LPSTR lpData) = 0;
    // MAC加密数据
    virtual long SM4MACData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, LPCSTR lpIVData, LPSTR lpData) = 0;
    // 读取国密PinBlock
    virtual long SM4PinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock) = 0;
    // 下载非对称密钥，包括：公钥，私钥
    virtual long SM2ImportKey(UINT uKeyNo, UINT uVendorPKeyNo, BOOL bPublicKey, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal) = 0;
    // 公钥加密数据
    virtual long SM2EncryptData(UINT uPKeyNo, /*LPCSTR lpSM2PKeyData, */LPCSTR lpData, LPSTR lpCryptData) = 0;
    // 私钥解密数据
    virtual long SM2DecryptData(UINT uSKeyNo, /*LPCSTR lpSM2SKeyData, */LPCSTR lpCryptData, LPSTR lpData) = 0;
    // 私钥签名
    virtual long SM2SignData(UINT uSKeyNo, LPCSTR lpZA, LPCSTR lpData, LPSTR lpSignData) = 0;
    // 公钥验签
    virtual long SM2VerifySign(LPCSTR lpSM2PKeyData, LPCSTR lpZA, LPCSTR lpData, LPCSTR lpSignData) = 0;
    // 导出公钥和公钥签名
    virtual long SM2ExportPKey(UINT uKeyNo, UINT uSignKeyNo, LPSTR lpZA, LPSTR lpSM2PKeyData, LPSTR lpSignData) = 0;
    // 随机生成SM密钥对
    virtual long SM2GenerateKey(UINT uSKeyNo, UINT uPKeyNo) = 0;
    // 生成随机数ZA
    virtual long SM3CaculateZAData(LPCSTR lpUserData, UINT uPKNum, QByteArray &resultData) = 0;       //30-00-00-00(FS#0005)
    // 删除密钥，默认删除全部，按序号只能删除SM4密钥
    virtual long SMDeleteKey(UINT uKeyNo = 0) = 0;
    // 设置国密密钥保存方式，uType：0->不相同，1->可相同
    virtual long SM4SaveKeyType(UINT uType = 0) = 0;
    // 导出公钥
    virtual long SM2ExportPKeyForGD(UINT uKeyNo, LPSTR lpData) = 0;                       //30-00-00-00(FS#0003)

    // ---------------------RSA公私钥支持，以D22为基础------------------------------------------------
    // 判断RSA是否初始化，D22返回值不一样
    virtual long RSAInitStatus() = 0;
    // 导入Host公钥(厂商私钥签名)
    virtual long RSAImportRootHostPK(QByteArray strHostPK, QByteArray strSignHostPK) = 0;
    virtual long RSAImportRootHostPK(UINT uPKeyNo, UINT uSignKeyNo, int iSignAlgo, QByteArray strHostPKData, QByteArray strSignData) = 0;           //30-00-00-00(FS#0006)
    // 导入Host签名公钥(HOST私钥签名)
    virtual long RSAImportSignHostPK(QByteArray strHostPK, QByteArray strSignHostPK) = 0;
    // 导出密码键盘内部的加密公钥，厂商私钥签名的，HEX格式的公钥明文和签名，D22只导出加密公钥
    virtual long RSAExportEppEncryptPK(QByteArray &strPK, QByteArray &strSignPK) = 0;
    virtual long RSAExportEppEncryptPK(UINT uEppPKeyNo, UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo,
                                       QByteArray &strKeyVal, QByteArray &strSignVal) = 0;       //30-00-00-00(FS#0006)
    // 导出密码键盘SN和签名SN（厂商私钥签名）
    virtual long RSAExportEppSN(QByteArray &strSN, QByteArray &strSignSN) = 0;
    virtual long RSAExportEppSN(UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo, QByteArray &strSN, QByteArray &strSignSN) = 0;       //30-00-00-00(FS#0006)
    // 生成8位的导入MK随机数据
    virtual long RSAExportEppRandData(QByteArray &strRandData) = 0;
    // 导入MK（主密钥）：使用PK（加密公钥）加密MK（主密钥），然后SK（HOST私钥）签名MK（主密钥）
    virtual long RSAImportDesKey(UINT uMKeyNo, QByteArray strMKVal, QByteArray strSignMKVal, QByteArray &strKCV) = 0;
    virtual long RSAImportDesKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostPKeyNo,
                                 RSADATAALGOTYPE  eRSAEncAlgo, RSASIGNALGOTYPE eRSASignAlgo,
                                 QByteArray strKeyVal, QByteArray strSignVal,
                                 QByteArray &strKCV) = 0; //30-00-00-00(FS#0006)
    // 生成RSA密钥对
    virtual long RSAGenerateKeyPair(UINT uPKeyNo, UINT uSKeyNo, int iModuleLen, int iExponentValue) = 0;            //30-00-00-00(FS#0006)
};


extern "C" DEVPIN_EXPORT long CreateIDevPIN(LPCSTR lpDevType, IDevPIN *&pDev);
//////////////////////////////////////////////////////////////////////////
