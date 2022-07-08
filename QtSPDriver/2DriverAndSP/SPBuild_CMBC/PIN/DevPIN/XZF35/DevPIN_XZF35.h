#pragma once

#include "IDevPIN.h"
#include "IAllDevPort.h"
#include "QtTypeInclude.h"
#include "DevPIN.h"

#define DEF_TIMEOUT_XZ                 (3000 ) // 默认超时

class CDevPIN_XZF35 : public IDevPIN, public CLogManage
{

public:
    CDevPIN_XZF35(LPCSTR lpDevType);
    virtual ~CDevPIN_XZF35();
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
    virtual long PinInput(UINT uMinLen = 4, UINT uMaxLen = 6);
    // 读取输入按键，大于0为按键值，小于0为失败
    virtual long ReadKeyPress(EPP_KEYVAL &stKeyVal, DWORD dwTimeOut = 250);
    // 取消输入
    virtual long CancelInput(bool bPinInput = false);
    // 读取PINBlock
    virtual long GetPinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock);
    // 加解密，支持大数据
    virtual long DataCrypt(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, LPSTR lpData);
    // 计算MAC，支持大数据
    virtual long DataMAC(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, LPSTR lpData);
    // 从键盘中获取8位长度的随机数
    virtual long RandData(LPSTR lpData);
    // 读取对称密钥KCV
    virtual long ReadSymmKeyKCV(UINT uKeyNo, BYTE byKcvMode, LPSTR lpKcv);

    // ---------------------SM国密支持---------------------------------------------------
    // 读取国密芯片（A980）固件信息
    virtual long SMGetFirmware(LPSTR lpSMVer);
    // 导入主密钥，uMKeyNo：0x01 ~ 0x1F
    virtual long SM4ImportMKey(UINT uMKeyNo, LPCSTR lpKeyVal, LPSTR lpKCV);
    // 导入工作密钥，包括：PIN密钥，MAC密钥，加解密密钥，向量密钥
    virtual long SM4ImportWKey(UINT uKeyNo, UINT uMKeyNo, UINT uKeyUse, LPCSTR lpKeyVal, LPSTR lpKCV, BYTE byKcvMode = 2);
    // 远程下载密钥
    virtual long SM4RemoteImportKey(UINT uKeyNo, UINT uEppSKeyNo, UINT uHostSM2PKeyNo, UINT uKeyUse, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal, LPSTR lpKCV);
    // 加解密
    virtual long SM4CryptData(UINT uKeyNo, UINT uMode, LPCSTR lpCryptData, BYTE bPadding, LPCSTR lpIVData, LPSTR lpData);
    // MAC加密数据
    virtual long SM4MACData(UINT uKeyNo, UINT uMode, LPCSTR lpMacData, LPCSTR lpIVData, LPSTR lpData);
    // 读取国密PinBlock
    virtual long SM4PinBlock(UINT uKeyNo, UINT uFormat, LPCSTR lpCustomerData, BYTE bPadding, LPSTR lpPinBlock);
    // 下载非对称密钥，包括：公钥，私钥
    virtual long SM2ImportKey(UINT uKeyNo, UINT uVendorPKeyNo, BOOL bPublicKey, LPCSTR lpZA, LPCSTR lpKeyVal, LPCSTR lpSignKeyVal);
    // 公钥加密数据
    virtual long SM2EncryptData(LPCSTR lpSM2PKeyData, LPCSTR lpData, LPCSTR lpCryptData);
    // 私钥解密数据
    virtual long SM2DecryptData(LPCSTR lpSM2SKeyData, LPCSTR lpCryptData, LPCSTR lpData);
    // 私钥签名
    virtual long SM2SignData(LPCSTR lpSM2SKeyData, LPCSTR lpZA, LPCSTR lpData, LPSTR lpSignData);
    // 公钥验签
    virtual long SM2VerifySign(LPCSTR lpSM2PKeyData, LPCSTR lpZA, LPCSTR lpData, LPCSTR lpSignData);
    // 导出公钥和公钥签名
    virtual long SM2ExportPKey(UINT uKeyNo, UINT uSignKeyNo, LPSTR lpZA, LPSTR lpSM2PKeyData, LPSTR lpSignData);
    // 随机生成SM密钥对
    virtual long SM2GenerateKey(UINT uSKeyNo, UINT uPKeyNo);
    // 生成随机数ZA
    virtual long SM3CaculateZAData(LPCSTR lpUserData, LPSTR lpData);
    // 删除密钥，默认删除全部，按序号只能删除SM4密钥
    virtual long SMDeleteKey(UINT uKeyNo = 0);
    // 设置国密密钥保存方式，uType：0->不相同，1->可相同
    virtual long SM4SaveKeyType(UINT uType = 0);

    // ---------------------RSA公私钥支持，以D22为基础------------------------------------------------
    // 判断RSA是否初始化，D22返回值不一样
    virtual long RSAInitStatus();
    // 导入Host公钥(厂商私钥签名)
    virtual long RSAImportRootHostPK(QByteArray strHostPK, QByteArray strSignHostPK);
    // 导入Host签名公钥(HOST私钥签名)
    virtual long RSAImportSignHostPK(QByteArray strHostPK, QByteArray strSignHostPK);
    // 导出密码键盘内部的加密公钥，厂商私钥签名的，HEX格式的公钥明文和签名，D22只导出加密公钥
    virtual long RSAExportEppEncryptPK(QByteArray &strPK, QByteArray &strSignPK);
    // 导出密码键盘SN和签名SN（厂商私钥签名）
    virtual long RSAExportEppSN(QByteArray &strSN, QByteArray &strSignSN);
    // 生成8位的导入MK随机数据
    virtual long RSAExportEppRandData(QByteArray &strRandData);
    // 导入MK（主密钥）：使用PK（加密公钥）加密MK（主密钥），然后SK（HOST私钥）签名MK（主密钥）
    virtual long RSAImportDesKey(UINT uMKeyNo, QByteArray strMKVal, QByteArray strSignMKVal, QByteArray &strKCV);
private:
    // 打包发送命令
    long PackCmd(WORD wCmd, LPCSTR lpCmdData, DWORD dwCmdDataLen, LPSTR lpData, DWORD &dwSize);
    // 解析返回命令
    long UnPackCmd(LPCSTR lpCmd, DWORD dwReadLen, LPSTR lpData, DWORD &dwSize);
    // 计算LRC
    BYTE GetLRC(const BYTE *pszCmd, DWORD dwCmdLen);
    // 生成8位随机数，返回数据格式为HEX
//    std::string GetRandData();
    // 发送和接收命令
    long SendReadCmd(WORD wCmdCode, const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut = DEF_TIMEOUT_XZ);
    // 发送和接收数据
    long SendReadData(const QByteArray &vtCmd, QByteArray &vtResult, DWORD dwTimeOut = DEF_TIMEOUT_XZ);
    // 转换错误码
    long ConvertErrCode(BYTE byRTN, LPCSTR ThisModule);
    // 错误码和错误信息
    void InitErrCode();
    // 更新状态和错误码
    void UpdateStatus(WORD wDevice, long lErrCode = -1, BYTE byErrType = 0);
    // 转换按键值
    bool ConvertKeyVal(BYTE bKey, EPP_KEYVAL &stKeyVal);    
    // 获取32位长度的认证随机数
//    long AuthRandData(LPCSTR lpUID, QByteArray &vtResult);
    // 键盘认证
//    long AuthEpp(LPCSTR lpUID, UINT uAuthMode, UINT uKeyNo, QByteArray *pvtData = nullptr);
    // 密钥认证
//    long AuthKey(UINT uKeyNo);
    // UID认证
//    long AuthUID();
    // 设置控制模式
//    long SetControlMode(BYTE nMode, BYTE nCmd);
    // 填充字符，保证数据长度为8倍数
//    QByteArray *Pading(QByteArray &vtData, BYTE byPading, bool bEnd = true, UINT uPadLen = 16);
    // 加解密
//    long Crypt(UINT uKeyNo, UINT uCryptType, LPCSTR lpCryptData, QByteArray &vtResult);
    // 计算MAC
//    long MAC(UINT uKeyNo, UINT uMacType, LPCSTR lpMacData, QByteArray &vtResult);
    // 设置密码输入长度
//    long SetPinInputLen(UINT uMinLen = 0, UINT uMaxLen = 12);

    // ---------------------SM国密支持---------------------------------------------------
    // 加解密
//    long SM4Crypt(UINT uKeyNo, UINT uType, UINT uMode, LPCSTR lpCryptData, LPCSTR lpIVData, QByteArray &vtResult);
    // 计算MAC
//    long SM4MAC(UINT uKeyNo, UINT uMacType, LPCSTR lpMacData, LPCSTR lpIVData, QByteArray &vtResult);

    // ---------------------RSA支持---------------------------------------------------
    // 获取TLV长度TAG
//    std::string GetTAG(std::string strHexData, bool bInteger = true);
    // 增加TAG前缀
//    std::string AddTAG(LPCSTR lpHexData, bool bInteger = true);
    // 解析数据
//    bool GetTLVData(LPCSTR lpData, DWORD dwDataLen, vectorString &vtData);
    // 格式化公钥
//    std::string GetHexPK(std::string strModulus, std::string strExponent);
    // 解析导出公钥数据
//    bool GetPKByExportData(LPCSTR lpData, DWORD dwDataLen, vectorString &vtPK);
    // 结构化导入公钥数据
//    std::string FmImportPK(std::string strHexPK, std::string strHexSignPK);
    // 结构化导入DESKEY数据
//    std::string FmImportDesKey(std::string strHexDES, std::string strHexSignDES);

    // 使能按键声音
    long SwitchBeep(bool bBeep);

    // 切换对称密钥模式国际/国密
    long SwitchModeDESOrSM4(bool bDes);
private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IAllDevPort>   m_pDev;
    DEVPINSTATUS                m_stStatus;
    std::string                 m_strAllKeyVal;
    listEPP_KEYVAL              m_vtKeyVal;
    mapByteString               m_mapErrCode;
};
