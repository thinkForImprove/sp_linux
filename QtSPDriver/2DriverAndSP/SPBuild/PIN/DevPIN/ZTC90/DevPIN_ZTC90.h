#pragma once
#include "IDevPIN.h"
#include "IAllDevPort.h"
#include "QtTypeInclude.h"
#include "DevPIN.h"
#include "IntfDataDef_ZTC90.h"
#include "../ZT598/DES.h"
#include "../ZT598/Encrypt.h"

//////////////////////////////////////////////////////////////////////////
#undef LOGDEVACTION
#define LOGDEVACTION()  CAllDevPortAutoLog _auto_log_this_action(m_pDev, ThisModule);\
    if (!m_pDev->IsOpened())\
    {\
        Log(ThisModule, __LINE__, "未打开串口");\
        UpdateStatus(DEVICE_OFFLINE, ERR_DEVPORT_NOTOPEN);\
        return ERR_DEVPORT_NOTOPEN;\
    }
//////////////////////////////////////////////////////////////////////////
class CDevPIN_ZTC90 : public IDevPIN, public CLogManage
{
public:
    CDevPIN_ZTC90(LPCSTR lpDevType);
    virtual ~CDevPIN_ZTC90();
public:
    // 释放接口
    virtual void Release();
    // 打开与设备的连接，格式：COM2:9600,N,8,1
    virtual long Open(LPCSTR lpMode);
    // 关闭与设备的连接
    virtual long Close();
    // 初始化，此会删除所有密钥，RSA密钥除外
    virtual long InitEpp();
    // 复位
    virtual long Reset();
    // 查状态
    virtual long GetStatus(DEVPINSTATUS &stStatus);
    // 安装状态:bClear表示是否清除自毁标志，返回值：0->已安装，1->未安装，其他->未知
    virtual long InstallStatus(bool bClear = false);
    // 读取固件信息
    virtual long GetFirmware(LPSTR lpFWVer);
    // 设置按键值
    virtual long SetKeyValue(LPCSTR lpKeyVal);
    // 设置激活按键
    virtual long SetActiveKey(DWORD dwActiveFuncKey, DWORD dwActiveFDKKey);
    // 导入主密钥，uMKeyNo：0x01 ~ 0x1F
    virtual long ImportMKey(UINT uMKeyNo, UINT uKeyMode, LPCSTR lpKeyVal, LPSTR lpKCV);
    // 导入工作密钥，包括：PIN密钥，MAC密钥，加解密密钥，向量密钥
    virtual long ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode = 2);
    // 设置DES KCV解密Key
    virtual void SetDesKcvDecryptKey(UINT uKeyNo);
    // 删除密钥，默认删除全部
    virtual long DeleteKey(UINT uKeyNo = 0);
    // 明文输入
    virtual long TextInput();
    // 密文输入
    virtual long PinInput(UINT uMinLen = 4, UINT uMaxLen = 6, bool bClearKeyAsBackspace = false);
    // 读取输入按键，大于0为按键值，小于0为失败
    virtual long ReadKeyPress(EPP_KEYVAL &stKeyVal, DWORD dwTimeOut = 250);
    // 取消输入
    virtual long CancelInput(bool bPinInput = false);
    // 读取PINBlock
    virtual long GetPinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock);
    // 加解密，支持大数据
    virtual long DataCrypt(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, QByteArray &vtResultData);
    // 计算MAC，支持大数据
    virtual long DataMAC(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, QByteArray &vtResultData, BYTE byPad = 0xFF);
    // 从键盘中获取32位长度的随机数
    virtual long RandData(LPSTR lpData);
    // 读取对称密钥KCV
    virtual long ReadSymmKeyKCV(UINT uKeyNo, BYTE byKcvMode, LPSTR lpKcv, BYTE bySymmKeyType = 0);
    virtual long SM2ExportPKeyForGD(UINT uKeyNo, LPSTR lpData);   //30-00-00-00(FS#0003)
    // ---------------------SM国密支持---------------------------------------------------
    // 读取国密芯片（A980）固件信息
    virtual long SMGetFirmware(LPSTR lpSMVer);
    // 导入主密钥，uMKeyNo：0x01 ~ 0x1F
    virtual long SM4ImportMKey(UINT uMKeyNo, LPCSTR lpKeyVal, LPSTR lpKCV, UINT uEncKeyNo = 0xFF);
    virtual long SM4ImportMKeyForGD(UINT uMKeyNo, UINT uDKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, UINT uVerifSignKeyNo = 0xFF, LPSTR lpSignData = nullptr);	//30-00-00-00(FS#0003)
    // 导入工作密钥，包括：PIN密钥，MAC密钥，加解密密钥，向量密钥
    virtual long SM4ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode = 2);
    // 远程下载密钥
    virtual long SM4RemoteImportKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostSM2PKeyNo, UINT uKeyUse, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal, LPSTR lpKCV);
    // 加解密
    virtual long SM4CryptData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, QByteArray &vtResultData);
    // MAC加密数据
    virtual long SM4MACData(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, QByteArray &vtResultData, BYTE byPad = 0xFF);
    // 读取国密PinBlock
    virtual long SM4PinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock);
    // 下载非对称密钥，包括：公钥，私钥
    virtual long SM2ImportKey(UINT uKeyNo, UINT uVendorPKeyNo, BOOL bPublicKey, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal);
    // 公钥加密数据
    virtual long SM2EncryptData(UINT uPKeyNo, LPCSTR lpData, LPSTR lpCryptData);
    // 私钥解密数据
    virtual long SM2DecryptData(UINT uSKeyNo, LPCSTR lpCryptData, LPSTR lpData);
    // 私钥签名
    virtual long SM2SignData(UINT uSKeyNo, LPCSTR lpZA, LPCSTR lpData, LPSTR lpSignData);
    // 公钥验签
    virtual long SM2VerifySign(UINT uPKeyNo, LPCSTR lpZA, LPCSTR lpData, LPCSTR lpSignData);
    // 导出公钥和公钥签名
    virtual long SM2ExportPKey(UINT uKeyNo, UINT uSignKeyNo, LPSTR lpZA, LPSTR lpSM2PKeyData, LPSTR lpSignData);
    // 随机生成SM密钥对
    virtual long SM2GenerateKey(UINT uSKeyNo, UINT uPKeyNo);
    // 生成随机数ZA
    virtual long SM3CaculateZAData(LPCSTR lpUserData, UINT uPKNum, QByteArray &resultData);   //30-00-00-00(FS#0005)
    // 删除密钥，默认删除全部，按序号只能删除SM4密钥
    virtual long SMDeleteKey(UINT uKeyNo = 0);
    // 设置国密密钥保存方式，uType：0->不相同，1->可相同
    virtual long SM4SaveKeyType(UINT uType = 0);

    // ---------------------RSA公私钥支持，以D22为基础------------------------------------------------
    // 判断RSA是否初始化，D22返回值不一样
    virtual long RSAInitStatus();
    // 导入Host公钥(厂商私钥签名)
    virtual long RSAImportRootHostPK(QByteArray strHostPK, QByteArray strSignHostPK);
    virtual long RSAImportRootHostPK(UINT uPKeyNo, UINT uSignKeyNo, int iSignAlgo, QByteArray strHostPKData, QByteArray strSignData);
    // 导入Host签名公钥(HOST私钥签名)
    virtual long RSAImportSignHostPK(QByteArray strHostPK, QByteArray strSignHostPK);
    // 导出密码键盘内部的加密公钥，厂商私钥签名的，HEX格式的公钥明文和签名，D22只导出加密公钥
    virtual long RSAExportEppEncryptPK(QByteArray &strPK, QByteArray &strSignPK);
    virtual long RSAExportEppEncryptPK(UINT uEppPKeyNo, UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo,
                                       QByteArray &strKeyVal, QByteArray &strSignVal);                      //30-00-00-00(FS#0006)
    // 导出密码键盘SN和签名SN（厂商私钥签名）
    virtual long RSAExportEppSN(QByteArray &strSN, QByteArray &strSignSN);
    virtual long RSAExportEppSN(UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo, QByteArray &strSN, QByteArray &strSignSN);     //30-00-00-00(FS#0006)
    // 生成8位的导入MK随机数据
    virtual long RSAExportEppRandData(QByteArray &strRandData);
    // 导入MK（主密钥）：使用PK（加密公钥）加密MK（主密钥），然后SK（HOST私钥）签名MK（主密钥）
    virtual long RSAImportDesKey(UINT uMKeyNo, QByteArray strMKVal, QByteArray strSignMKVal, QByteArray &strKCV);
    virtual long RSAImportDesKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostPKeyNo,
                                 RSADATAALGOTYPE  eRSAEncAlgo, RSASIGNALGOTYPE eRSASignAlgo,
                                 QByteArray strKeyVal, QByteArray strSignVal,
                                 QByteArray &strKCV, bool bUseRandom = false,
                                 int iSymmKeyType = 0);
    // 生成RSA密钥对
    virtual long RSAGenerateKeyPair(UINT uPKeyNo, UINT uSKeyNo, int iModuleLen, int iExponentValue);
    // RSA密钥加密数据
    virtual long RSAEncryptData(UINT uRSAPKeyNo, RSADATAALGOTYPE eRSAEncAlgo, QByteArray data, QByteArray &result); //30-00-00-00(FS#0006)
    // RSA密钥解密数据
    virtual long RSADecryptData(UINT uRSASKeyNo, RSADATAALGOTYPE eRSAEncAlgo, QByteArray data, QByteArray &result); //30-00-00-00(FS#0006)

    // 设置DES密钥长度信息
    virtual long SetDesKeyLenInfo(map<UINT, BYTE> &keyIdMapKeyLen);

private:
    // 打包发送命令
    long PackCmd(LPCSTR lpCmd, DWORD dwCmdLen, LPSTR lpData, DWORD &dwSize);
    // 解析返回命令
    long UnPackResp(LPCSTR lpCmd, DWORD dwReadLen, LPSTR lpData, DWORD &dwSize);
    // 计算LRC
    std::string GetLRC(const BYTE *pszCmd, DWORD dwCmdLen);
    // 生成8位随机数，返回数据格式为HEX
    std::string GetRandData();
    // 发送和接收命令
    long SendReadCmd(const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut = DEF_TIMEOUT);
    // 发送和接收数据
    long SendReadData(const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut = DEF_TIMEOUT);
    // 转换错误码
    long ConvertErrCode(BYTE byRTN, LPCSTR ThisModule);
    // 错误码和错误信息
    void InitErrCode();
    // 更新状态和错误码
    void UpdateStatus(WORD wDevice, long lErrCode = -1);
    // 转换按键值
    bool ConvertKeyVal(BYTE bKey, EPP_KEYVAL &stKeyVal);
    // 主机和键盘双向认证
    long MutualAuth(QByteArray authKeyData, QByteArray randomData, UINT uKeyNo);
    // 填充字符，保证数据长度为8倍数
    QByteArray *Pading(QByteArray &vtData, BYTE byPading, bool bEnd = true, UINT uPadLen = 16);

    // ---------------------RSA支持---------------------------------------------------
    // 获取TLV长度TAG
    std::string GetTAG(std::string strHexData, bool bInteger = true);
    // 增加TAG前缀
    std::string AddTAG(LPCSTR lpHexData, bool bInteger = true);
    // 解析数据
    bool GetTLVData(LPCSTR lpData, DWORD dwDataLen, vectorString &vtData);
    // 格式化公钥
    std::string GetHexPK(std::string strModulus, std::string strExponent);
    // 解析导出公钥数据
    bool GetPKByExportData(LPCSTR lpData, DWORD dwDataLen, vectorString &vtPK);
    // 结构化导入公钥数据
    std::string FmImportPK(std::string strHexPK, std::string strHexSignPK);
    // 结构化导入DESKEY数据
    std::string FmImportDesKey(std::string strHexDES, std::string strHexSignDES);
    //创建rsa tag
    long BuildAsn1Tag( WORD wTagType, WORD wDataLen, LPWORD lpwTagLen, LPBYTE lpbyTagBuffer);
    //解析DER数据
    long ExtractAsn1Data( WORD wFieldLen, LPBYTE lpbyField,  WORD wTagType, LPWORD lpwTagLen,
                          LPBYTE lpbyData, LPWORD lpwDataLen);
    //设置下载工作密钥3DES/DES模式
    long SetLoadWKDESMode(bool bIsTDES);
    //设置KCV 3DES/DES模式
    long SetKCVDesMode(bool bIsTDES);

    //读取配置文件
    void ReadConfig();                  //30-00-00-00(FS#0010)

    //设置键盘通用配置参数
    long SetEppCommConfigParam();

    //判断DES密钥是否为TDES密钥
    bool IsTDESKey(UINT uKeyNo);

    //导入全0密钥
    long ImportAllZeroMKey(bool bIsDes = true);

    //设置密钥校验值相关参数
    long SetKCVRelatedParam();

    //-------------------------键盘底层命令-----------------------------------
    //键盘认证
    long Authenticate(BYTE byFlag, QByteArray data, QByteArray &resultData);
    //设置通用配置参数
    long SetCommConfigParam(BYTE byItemId);
    //设置算法参数
    long SetAlgoParam(BYTE byItemId, BYTE byItemOptId);
    //激活密钥
    long ActiveKey(UINT uKeyNo);
    //激活国密密钥
    long ActiveSMKey(UINT uKeyNo);
    //激活SM2密钥
    long ActiveSM2Key(UINT uKeyNo);
    //设置加解密初始向量
    long SetIVData(QByteArray &ivData, bool bIsDes = true);
    //设置PINBLOCK计算数据
    long SetPinBlockData(QByteArray pinBlockData);
    //自毁后重激活
    long ClearSelfDestroy();
    //设置国密算法参数
    long SetSMAlgoParam(BYTE byItemId, BYTE byItemOptId);
private:
    bool                        m_bInitDesZeroKey;
    listEPP_KEYVAL              m_vtKeyVal;
    mapByteString               m_mapErrCode;
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IAllDevPort>   m_pDev;
    std::string                 m_strFirmware;
    DEVPINSTATUS                m_stStatus;
    QByteArray                  m_strAllKeyVal;

    //ini配置项
    int                         m_iClearKeyMode;            //30-00-00-00(FS#0010)
    bool                        m_bUseEppAuth;

    UINT                        m_uKcvDecryptKeyNo;

    CCDES                       m_cDES;
    CEncrypt                    m_encrypt;

    map<UINT, BYTE>             m_keyIdMapKeyLen;
    int                         m_iAlgoParamFlag[16];
};
