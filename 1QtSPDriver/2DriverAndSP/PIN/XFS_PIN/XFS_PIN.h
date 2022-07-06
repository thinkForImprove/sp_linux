#pragma once
#include "IDevPIN.h"
#include "KeySlotHelper.h"
#include "PinDataHelper.h"
#include "QtTypeInclude.h"

#include <vector>
using namespace std;
//////////////////////////////////////////////////////////////////////////
typedef vector<UINT>             vectorUINT;
#define WSAP_PINGANBANK_CFG      "/home/projects/wsap/cfs/PinPad.ini"
//////////////////////////////////////////////////////////////////////////
class CAutoCloseInput
{
public:
    CAutoCloseInput(IDevPIN *pDev, bool bPinInput): m_pDev(pDev), m_bPinInput(bPinInput) {}
    ~CAutoCloseInput() { if (m_pDev != nullptr) m_pDev->CancelInput(m_bPinInput); }
private:
    IDevPIN *m_pDev;
    bool m_bPinInput;
};
//////////////////////////////////////////////////////////////////////////
class CXFS_PIN : public ICmdFunc, public CLogManage
{
public:
    CXFS_PIN();
    virtual ~CXFS_PIN();
public:
    // 开始运行SP
    long StartRun();

public:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // INFOR
    virtual HRESULT GetStatus(LPWFSPINSTATUS &lpstStatus);
    virtual HRESULT GetCapabilities(LPWFSPINCAPS &lpstCaps);
    virtual HRESULT GetKeyDetail(LPSTR lpszKeyName, LPWFSPINKEYDETAIL *&lppKeyDetail);
    virtual HRESULT GetKeyDetailEx(LPSTR lpszKeyName, LPWFSPINKEYDETAILEX *&lppKeyDetail);
    virtual HRESULT GetFuncKeyDetail(LPULONG lpFDKMask, LPWFSPINFUNCKEYDETAIL &lpFunKeyDetail);

    // EXECUTE
    virtual HRESULT Crypt(const LPWFSPINCRYPT lpCrypt, LPWFSXDATA &lpxCryptData);
    virtual HRESULT ImportKey(const LPWFSPINIMPORT lpImport, LPWFSXDATA &lpxKVC);
    virtual HRESULT GetPIN(const LPWFSPINGETPIN lpGetPin, LPWFSPINENTRY &lpEntry, DWORD dwTimeOut);
    virtual HRESULT GetPinBlock(const LPWFSPINBLOCK lpPinBlock, LPWFSXDATA &lpxPinBlock);
    virtual HRESULT GetData(const LPWFSPINGETDATA lpPinGetData, LPWFSPINDATA &lpPinData, DWORD dwTimeOut);
    virtual HRESULT Initialization(const LPWFSPININIT lpInit, LPWFSXDATA &lpxIdentification);
    virtual HRESULT Reset();
    virtual HRESULT GenerateKCV(const LPWFSPINGENERATEKCV lpGenerateKCV, LPWFSPINKCV &lpKCV);
    virtual HRESULT GetPinBlockEx(const LPWFSPINBLOCKEX lpPinBlockEx, LPWFSXDATA &lpxPinBlock);
    virtual HRESULT ImportKeyEx(const LPWFSPINIMPORTKEYEX lpImportEx);

    // 使用RSA公私密钥下载密钥
    virtual HRESULT StarKeyExchange(LPWFSPINSTARTKEYEXCHANGE &lpStartKeyExchange);
    virtual HRESULT ImportRSAPublicKey(const LPWFSPINIMPORTRSAPUBLICKEY lpPublicKey, LPWFSPINIMPORTRSAPUBLICKEYOUTPUT &lpPublicKeyOutput);
    virtual HRESULT ExportRSAIssuerSignedItem(const LPWFSPINEXPORTRSAISSUERSIGNEDITEM lpSignedItem, LPWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT &lpSignedItemOutput);
    virtual HRESULT ImportRSASignedDesKey(const LPWFSPINIMPORTRSASIGNEDDESKEY lpSignedDESKey, LPWFSPINIMPORTRSASIGNEDDESKEYOUTPUT &lpSignedDESKeyOutput);
    virtual HRESULT GenerateRSAKeyPair(const LPWFSPINGENERATERSAKEYPAIR lpGenerateRSAKeyPair);
    virtual HRESULT ExportRSAEppSignedItem(const LPWFSPINEXPORTRSAEPPSIGNEDITEM lpEppSignedItem, LPWFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT &lpEppSignedItemOutput);

    //远程国密密钥下载
    virtual HRESULT EncIo(const LPWFSPINENCIO lpEncIO, LPWFSPINENCIO &lpxEncIOData);       //30-00-00-00(FS#0003)

    virtual HRESULT OnClearCancelSemphoreCount() override;          //30-00-00-00(FT#0070)
protected:
    // 更新状态
    bool UpdateStatus(const DEVPINSTATUS &stStatus);
    // 更新扩展状态，strErrCode错误码的后三位，因前六位是固化的
    void UpdateExtra(string strErrCode, string strDevVer = "");
    // 设备状态是否正常
    bool IsStatusOK();
    // 获取支持的功能键
    ULONG GetFunctionKeys();
    // 获取支持的侧键
    ULONG GetFDKs(LPWFSPINFDK pszstFDK = nullptr);
    // 是否支持密钥用途
    bool IsSupportKeyUse(DWORD dwUse);
    // 转换密钥用途
    UINT ConvertKeyUse(DWORD dwUse);
    // 转换按键值
    ULONG ConvertKeyVal(BYTE byKeyVal, bool &bFDK);
    // 读取按键值，并发送事件
    long GetKeyPress(vectorWFSPINKEY &vtKeys, CInputDataHelper &cInput, DWORD dwTimeOut);
    // 旭子键盘获取Pin输入按键值
    long GetPinKeyPress_XZ(vectorWFSPINKEY &vtKeys, CInputDataHelper &cInput, DWORD dwTimeOut);
    // 按键完成模式
    WORD GetKeyCompletion(ULONG ulKeyVal, bool bTermKey);
    // 发送按键事件
    bool FireKeyPressEvent(WFSPINKEY stKey);
    // 转换密文模式
    UINT ConvertPinFormat(WORD wFormat);
    // 格式化用户数据
    long FmtCustomerData(QByteArray &vtData, UINT uPinFmt);
    // 转换加解密模式
    UINT ConvertCryptType(WORD wMode, WORD wAlgorithm);
    // 计算MAC模式
    WORD GetMacType(bool bSM4 = false);
    // 是否支持国密
    bool IsSupportSM();
    // DES的下载密钥和密码块是否只使用国密，有些银行要求使用DES的一样，因此通过此配置区分
    bool IsOnlyUseSM();
    // 解析密钥用途
    bool SplitKeyUse(DWORD dwUse, vectorUINT &vtUse);
    bool SplitSMKeyUse(DWORD dwUse, vectorUINT &vtUse);
    // 处理激活和禁用的按键
    QByteArray GetAllKeyVal(ULONG ulActiveKey, ULONG ulActiveFDK);
    //读写平安银行配置文件
    int ReadWSAPReg(CINIWriter &cwINI);
    // 读取配置文件
    void ReadConfig();                  //30-00-00-00(FS#0010)
private:
    std::string                       m_strLogicalName;
    std::string                       m_strSPName;
    WFSPINSTATUS                      m_stStatus;
    WFSPINCAPS                        m_stCaps;
    WFSPINKCV                         m_stKCV;
    WFSPINENTRY                       m_stPinEntry;

    CQtDLLLoader<IDevPIN>             m_pDev;
    CQtDLLLoader<ISPBasePIN>          m_pBase;
    CExtraInforHelper                 m_cExtra;
    CAutoEvent                        m_cCancelEvent;
    CXfsRegValue                      m_cXfsReg;
    CXfsRegValue                      m_cWSAPReg;
    CSimpleMutex                     *m_pMutexGetStatus;
    BOOL                              m_bSupportSM;
    BOOL                              m_bIsSupportSM4;
    BOOL                              m_bOnlyUseSM;
    BOOL                              m_bRemovCustomerLast;
    DWORD                             m_dwDesMacType;
    DWORD                             m_dwSM4MacType;
    std::string                       m_strNotCheckFDK;
    std::string                       m_strAllKeyVal;
    std::string                       m_strDevName;
    BOOL                              m_bGetPinEntChkMinLen;
    WORD                              m_wSM4CryptAlgorithm;


    CKeySlotHelper                    m_cKey;
    CKeySlotHelperXZ                  m_cKeyXZ;
    CWFSPINKEYDETAIL                  m_cKeyDetail;
    CWFSPINKEYDETAILEX                m_cKeyDetailEx;
    CWFSPINFUNCKEYDETAIL              m_cFuncKeyDetail;
    CWFSXDATA                         m_cXData;
    CWFSPINDATA                       m_cGetDataKeys;
    CWFSPINIMPORTRSAPUBLICKEYOUTPUT   m_cImportRsaPK;           //30-00-00-00(FS#0005)
    CWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT m_cExportRsaItem;    //30-00-00-00(FS#0005)
    CWFSPINIMPORTRSASIGNEDDESKEYOUTPUT     m_cImportRsaSignedDesKey;    //30-00-00-00(FS#0005)
    std::string                       m_strPort;

    //For 远程密钥下载WFS_CMD_PIN_ENC_IO
 //30-00-00-00(FS#0003)  start
    LPWFSRESULT                     lpPinEncIoResult;
    LPPROTCHNIMPORTSM2PUBLICKEYIN   lpvData;
    LPPROTCHNIMPORTSM2PUBLICKEYOUT  lpvDataOut;
    LPWFSPINENCIO                   lpEncIoOut;
    LPWFSXDATA                      lpxKeyCheckValue;
    LPBYTE                          lpbData;

    CWFSENCIODATA                   m_stENCIOData;                      //30-00-00-00(FS#0001)
    CWFSGENERATESM2KEYPAIRDATA      m_stGenerateSm2KeyPairData;         //30-00-00-00(FS#0001)
    CWFSEXPORTSM2EPPSIGNITEMDATA    m_stExportSm2EppSignItemData;       //30-00-00-00(FS#0001)
    CWFSIMPORTSM4DATA               m_stImportSm4Data;                  //30-00-00-00(FS#0001)

    LPPROTCHNIMPORTSM2SIGNEDSM4KEY lpImportSm2SignSm4KeyIn;
    LPPROTCHNGENERATESM2KEYPAIRIN  lpGenerateSm2KeyPairIn;

    //for WFS_CMD_ENC_IO_CHN_EXPORT_SM2_EPP_SIGNED_ITEM
    LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT lpvExportSigItemDataOut;
    //for WFS_CMD_ENC_IO_CHN_IMPORT_SM2_SIGNED_SM4_KEY
    LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT lpImportSm2SignSm4KeyOut;
    //for WFS_CMD_ENC_IO_CHN_GENERATE_SM2_KEY_PAIR
    LPPROTCHNGENERATESM2KEYPAIROUT lpGenerateSm2KeyPairOut;
 //   DWORD	dwLoadKeyUseTemp;
    WORD wLoadKeyCatalog;					//键类别
    LPWFSXDATA	    lpxKeyValueSM2; //RSA PK

 //30-00-00-00(FS#0003)  end

    //ini配置项
    bool                              m_bSupportBackspace;      //30-00-00-00(FS#0010)
    bool                              m_bRSAUseSM2Key;          //30-00-00-00(FS#0006)
    bool                              m_bFireEnterEventBefMinLen;//30-00-00-00(FS#0008)

    string                            m_strRSAKeyPairName;      //30-00-00-00(FS#0006)
};

