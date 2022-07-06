#pragma once
#include "XFSPIN.H"
#include "ISPBaseClass.h"
#include "XFSPINCHN.H"                      //30-00-00-00(FS#0003)

// 命令执行和结果数据返回
struct ICmdFunc
{
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName) = 0;
    virtual HRESULT OnClose() = 0;
    virtual HRESULT OnStatus() = 0;
    virtual HRESULT OnCancelAsyncRequest() = 0;
    virtual HRESULT OnUpdateDevPDL() = 0;

    // INFOR
    virtual HRESULT GetStatus(LPWFSPINSTATUS &lpStatus) = 0;
    virtual HRESULT GetCapabilities(LPWFSPINCAPS &lpCaps) = 0;
    virtual HRESULT GetKeyDetail(LPSTR lpszKeyName, LPWFSPINKEYDETAIL *&lppKeyDetail) = 0;
    virtual HRESULT GetKeyDetailEx(LPSTR lpszKeyName, LPWFSPINKEYDETAILEX *&lppKeyDetail) = 0;
    virtual HRESULT GetFuncKeyDetail(LPULONG lpFDKMask, LPWFSPINFUNCKEYDETAIL &lpFunKeyDetail) = 0;

    // EXECUTE
    virtual HRESULT Crypt(const LPWFSPINCRYPT lpCrypt, LPWFSXDATA &lpxCryptData) = 0;
    virtual HRESULT ImportKey(const LPWFSPINIMPORT lpImport, LPWFSXDATA &lpxKVC) = 0;
    virtual HRESULT GetPIN(const LPWFSPINGETPIN lpGetPin, LPWFSPINENTRY &lpEntry, DWORD dwTimeOut) = 0;
    virtual HRESULT GetPinBlock(const LPWFSPINBLOCK lpPinBlock, LPWFSXDATA &lpxPinBlock) = 0;
    virtual HRESULT GetData(const LPWFSPINGETDATA lpPinGetData, LPWFSPINDATA &lpPinData, DWORD dwTimeOut) = 0;
    virtual HRESULT Initialization(const LPWFSPININIT lpInit, LPWFSXDATA &lpxIdentification) = 0;
    virtual HRESULT Reset() = 0;
    virtual HRESULT GenerateKCV(const LPWFSPINGENERATEKCV lpGenerateKCV, LPWFSPINKCV &lpKCV) = 0;
    virtual HRESULT GetPinBlockEx(const LPWFSPINBLOCKEX lpPinBlockEx, LPWFSXDATA &lpxPinBlock) = 0;
    virtual HRESULT ImportKeyEx(const LPWFSPINIMPORTKEYEX lpImportEx) = 0;
    virtual HRESULT EncIo(const LPWFSPINENCIO lpEncIO, LPWFSPINENCIO &lpxEncIOData) = 0;         //30-00-00-00(FS#0003)

    virtual HRESULT StarKeyExchange(LPWFSPINSTARTKEYEXCHANGE &lpStartKeyExchange) = 0;
    virtual HRESULT ImportRSAPublicKey(const LPWFSPINIMPORTRSAPUBLICKEY lpPublicKey, LPWFSPINIMPORTRSAPUBLICKEYOUTPUT &lpPublicKeyOutput) = 0;
    virtual HRESULT ExportRSAIssuerSignedItem(const LPWFSPINEXPORTRSAISSUERSIGNEDITEM lpSignedItem, LPWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT &lpSignedItemOutput) = 0;
    virtual HRESULT ImportRSASignedDesKey(const LPWFSPINIMPORTRSASIGNEDDESKEY lpSignedDESKey, LPWFSPINIMPORTRSASIGNEDDESKEYOUTPUT &lpSignedDESKeyOutput) = 0;
    virtual HRESULT GenerateRSAKeyPair(const LPWFSPINGENERATERSAKEYPAIR lpGenerateRSAKeyPair) = 0;
    virtual HRESULT ExportRSAEppSignedItem(const LPWFSPINEXPORTRSAEPPSIGNEDITEM lpEppSignedItem, LPWFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT &lpEppSignedItemOutput) = 0;

    virtual HRESULT OnClearCancelSemphoreCount() = 0;               //30-00-00-00(FT#0070)
};
//////////////////////////////////////////////////////////////////////////
struct ISPBasePIN
{
    // 释放接口
    virtual void Release() = 0;
    // 注册回调接口
    virtual void RegisterICmdFunc(ICmdFunc *pCmdFunc) = 0;
    // 开始运行
    virtual bool StartRun() = 0;
    // 获取SPBase数据
    virtual void GetSPBaseData(SPBASEDATA &stData) = 0;
    // 发送事件
    virtual bool FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData) = 0;
    // 发送状态改变事件
    virtual bool FireStatusChanged(DWORD wStatus) = 0;
    // 发送故障状态事件
    virtual bool FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription = nullptr) = 0;
};

#if defined(SPBASEPIN_LIBRARY)
#define SPBASEPINSHARED_EXPORT Q_DECL_EXPORT
#else
#define SPBASEPINSHARED_EXPORT Q_DECL_IMPORT
#endif

extern "C" SPBASEPINSHARED_EXPORT long CreateISPBasePIN(LPCSTR lpDevType, ISPBasePIN *&p);
//////////////////////////////////////////////////////////////////////////
