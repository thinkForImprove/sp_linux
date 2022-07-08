#ifndef CDEV_CFESM01_H
#define CDEV_CFESM01_H

#include "IDevPIN.h"
#include "IAllDevPort.h"
#include "QtTypeInclude.h"
#include "XZF35/XZF35Def.h"
#include "CFESM01Def.h"
#include "DevPIN.h"

class CDevPIN_CFESM01 : public IDevPIN,
                        public CLogManage
{
public:
    CDevPIN_CFESM01(LPCSTR lpDevType);
    virtual ~CDevPIN_CFESM01();

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
    //设置使能按键
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
    // 从键盘中获取8位长度的随机数
    virtual long RandData(LPSTR lpData);
    // 读取对称密钥KCV
    virtual long ReadSymmKeyKCV(UINT uKeyNo, BYTE byKcvMode, LPSTR lpKcv, BYTE bySymmKeyType = 0);

    // ---------------------SM国密支持---------------------------------------------------
    // 读取国密芯片（A980）固件信息
    virtual long SMGetFirmware(LPSTR lpSMVer);
    // 导入主密钥，uMKeyNo：0x01 ~ 0x1F
    virtual long SM4ImportMKey(UINT uMKeyNo, LPCSTR lpKeyVal, LPSTR lpKCV, UINT uEncKeyNo = 0xFF);

    // 导入工作密钥，包括：PIN密钥，MAC密钥，加解密密钥，向量密钥
    virtual long SM4ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode = 2);
    // 远程下载密钥
    virtual long SM4RemoteImportKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostSM2PKeyNo,
                                    UINT uKeyUse, LPCSTR lpUserId, LPCSTR lpCipherTextKeyVal,
                                    LPCSTR lpSignatureVal, LPSTR lpKCV);
    // 加解密
    virtual long SM4CryptData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, QByteArray &vtResultData);
    // MAC加密数据
    virtual long SM4MACData(UINT uKeyNo, UINT uMacAlgo, LPCSTR lpMacData, LPCSTR lpIVData, QByteArray &vtResultData, BYTE byPad = 0xFF);
    // 读取国密PinBlock
    virtual long SM4PinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock);
    // 下载非对称密钥，包括：公钥，私钥
    virtual long SM2ImportKey(UINT uKeyNo, UINT uVendorPKeyNo, BOOL bPublicKey, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal);
    // 公钥加密数据
    virtual long SM2EncryptData(UINT uPKeyNo, /*LPCSTR lpSM2PKeyData, */LPCSTR lpData, LPSTR lpCryptData);
    // 私钥解密数据
    virtual long SM2DecryptData(UINT uSKeyNo, /*LPCSTR lpSM2SKeyData, */LPCSTR lpCryptData, LPSTR lpData);
    // 私钥签名
    virtual long SM2SignData(UINT uSKeyNo, LPCSTR lpZA, LPCSTR lpData, LPSTR lpSignData);
    // 公钥验签
    virtual long SM2VerifySign(UINT uPKeyNo, LPCSTR lpZA, LPCSTR lpData, LPCSTR lpSignData);
    // 导出公钥和公钥签名
    virtual long SM2ExportPKey(UINT uKeyNo, UINT uSignKeyNo, LPSTR lpZA, LPSTR lpSM2PKeyData, LPSTR lpSignData);
    // 随机生成SM密钥对
    virtual long SM2GenerateKey(UINT uSKeyNo, UINT uPKeyNo);
    // 生成随机数ZA
    virtual long SM3CaculateZAData(LPCSTR lpUserData, UINT uPKNum, QByteArray &resultData);
    // 删除密钥，默认删除全部，按序号只能删除SM4密钥
    virtual long SMDeleteKey(UINT uKeyNo = 0);
    // 设置国密密钥保存方式，uType：0->不相同，1->可相同
    virtual long SM4SaveKeyType(UINT uType = 0);
    // 导出公钥
    virtual long SM2ExportPKeyForGD(UINT uKeyNo, LPSTR lpData);                               								 //30-00-00-00(FS#00003)


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
                                       QByteArray &strKeyVal, QByteArray &strSignVal);
    // 导出密码键盘SN和签名SN（厂商私钥签名）
    virtual long RSAExportEppSN(QByteArray &strSN, QByteArray &strSignSN);
    virtual long RSAExportEppSN(UINT uEppSKeyNo, RSASIGNALGOTYPE &eRSASignAlgo, QByteArray &strSN, QByteArray &strSignSN);
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
    virtual long RSAEncryptData(UINT uRSAPKeyNo, RSADATAALGOTYPE eRSAEncAlgo, QByteArray data, QByteArray &result);
    // RSA密钥解密数据
    virtual long RSADecryptData(UINT uRSASKeyNo, RSADATAALGOTYPE eRSAEncAlgo, QByteArray data, QByteArray &result);
    // 设置DES密钥长度信息
    virtual long SetDesKeyLenInfo(std::map<UINT, BYTE> &keyIdMapKeyLen);
private:
    // 打包发送命令
    long PackCmd(BYTE byCmdCode, const QByteArray &vtCmdDataPart, QByteArray &vtCmd);
    // 解析命令返回数据
    long UnPackCmd(QByteArray &vtRecvData, QByteArray &vtRespData);
    // 计算CRC
    WORD GetCRC(const BYTE *lpData, DWORD dwSize);
    // 执行命令
    long ExecuteCommand(BYTE byCmdCode, const QByteArray &vtCmdDataPart, QByteArray &vtResult, DWORD dwTimeOut = M01_CMD_EXEC_TIMEOUT);
    // 发送数据并接收返回数据
    long SendAndReadData(QByteArray &vtSendData, QByteArray &vtRecvData, DWORD dwTimeOut = M01_CMD_EXEC_TIMEOUT);
    // 转换错误码
    long ConvertErrCode(BYTE byRTN, LPCSTR ThisModule);
    // 错误码和错误信息
    void InitErrCode();
    // 更新状态和错误码
    void UpdateStatus(WORD wDevice, long lErrCode = -1, BYTE byErrType = 0);

    // 切换对称密钥模式(DES/TDES,AES,SM4)
    long SwitchSymmetricKeyMode(BYTE byMode);
    // 使能按键声音
    long SwitchBeep(bool bBeep);
    //设定按键功能(16主键->8功能键)
    long SetFuncOfKeys(bool bClearKeyAsBackspace);
    //设置是否启用键盘移除功能
    long EnableRemoveFunc(bool bEnable);
    //移除认证
    long RemoveAuth(int iRemoveType);

    //读取配置文件
    void ReadConfig();

    bool ConvertKeyVal(BYTE bKey, EPP_KEYVAL &stKeyVal);
    void ConvertFDKEnableBitData(DWORD dwActiveFDKKey, DWORD &dwActiveFDKKeyConvert);
    //创建rsa tag
    long BuildAsn1Tag( WORD wTagType, WORD wDataLen, LPWORD lpwTagLen, LPBYTE lpbyTagBuffer);
    //解析DER数据
    long ExtractAsn1Data( WORD wFieldLen, LPBYTE lpbyField,  WORD wTagType, LPWORD lpwTagLen,
                          LPBYTE lpbyData, LPWORD lpwDataLen);
    //设置初始化向量
    long SetInitVector(WORD wKeyId, QByteArray &vtInitVec);
    //数据加解密运算(Data运算)
    long DataOper(WORD wKeyId, WORD wAlgorithm, WORD wOperMode, BYTE byPadChar,
                  QByteArray &vtData, UINT uDataLen, QByteArray &vtResult);
    //mac加密运算
    long MacOper(WORD wKeyId, WORD wAlgorithm, BYTE byPadChar,
                  QByteArray &vtData, UINT uDataLen, QByteArray &vtResult);
    //设置非法长按键时间
    bool SetIlligalHoldKeyTime(int iSecond);

private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IAllDevPort>   m_pDev;
    DEVPINSTATUS                m_stStatus;
    std::string                 m_strAllKeyVal;
    mapByteString               m_mapErrCode;
    listEPP_KEYVAL              m_vtKeyVal;

    //ini配置项
    int                         m_iClearKeyMode;
    int                         m_iRSARKLDecryptHashAlgo;              //RSA RKL,解密Hash算法
    int                         m_iRSARKLVerifySignatureHashAlgo;      //RSA RKL,验签Hash算法
    bool                        m_bRemoveFuncSupp;
    int                         m_iIllegalHoldKeyTime;

    BYTE                        m_bySymmetricKeyMode;
};

#endif // CDEVPIN_CFESM01_H
