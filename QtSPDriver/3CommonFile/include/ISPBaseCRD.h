#pragma once
#include "ISPBaseClass.h"
#include "XFSCRD.H"
#include "XFSIDC.H"

//////////////////////////////////////////////////////////////////////////
#if defined(SPBASECRD_LIBRARY)
    #define SPBASECRD_EXPORT Q_DECL_EXPORT
#else
    #define SPBASECRD_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
// 命令执行和结果数据返回
struct ICmdFunc
{
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName) = 0;
    virtual HRESULT OnClose() = 0;
    virtual HRESULT OnStatus() = 0;
    virtual HRESULT OnWaitTaken() = 0;
    virtual HRESULT OnCancelAsyncRequest() = 0;
    virtual HRESULT OnUpdateDevPDL() = 0;

    // IDC类型接口
    // 查询命令
    virtual HRESULT IDCGetStatus(LPWFSIDCSTATUS &lpStatus) = 0;
    virtual HRESULT IDCGetCapabilities(LPWFSIDCCAPS &lpCaps) = 0;
    virtual HRESULT IDCGetFormList(LPSTR &lpFormList) = 0;
    virtual HRESULT IDCGetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm) = 0;
    // 执行命令
    virtual HRESULT IDCReadTrack(LPCSTR lpFormName, LPSTR lpTrackData) = 0;
    virtual HRESULT IDCWriteTrack(const LPWFSIDCWRITETRACK lpWriteData) = 0;
    virtual HRESULT IDCEjectCard(DWORD dwTimeOut) = 0;
    virtual HRESULT IDCRetainCard(LPWFSIDCRETAINCARD &lpRetainCardData) = 0;
    virtual HRESULT IDCResetCount() = 0;
    virtual HRESULT IDCSetKey(const LPWFSIDCSETKEY lpKeyData) = 0;
    virtual HRESULT IDCReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut) = 0;
    virtual HRESULT IDCWriteRawData(const LPWFSIDCCARDDATA *lppCardData) = 0;
    virtual HRESULT IDCChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut) = 0;
    virtual HRESULT IDCReset(LPWORD lpResetIn) = 0;
    virtual HRESULT IDCChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData) = 0;
    virtual HRESULT IDCParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData) = 0;

    // CRD类型接口
    // 查询命令
    virtual HRESULT CRDGetStatus(LPWFSCRDSTATUS &lpStatus) = 0;
    virtual HRESULT CRDGetCapabilities(LPWFSCRDCAPS &lpCaps) = 0;
    virtual HRESULT CRDGetCardUnitInfo(LPWFSCRDCUINFO &lpwfsCrdCuInfo) = 0;
    // 执行命令
    virtual HRESULT CRDDispense(LPWFSCRDDISPENSE lpWfsCrdDispenser) = 0;
    virtual HRESULT CRDEjectCard(DWORD dwTimeOut) = 0;
    virtual HRESULT CRDRetainCard(LPWFSCRDRETAINCARD lpWfsCrdRetainCard) = 0;
    virtual HRESULT CRDReset(LPWFSCRDRESET lpWfsCrdReset) = 0;
    virtual HRESULT CRDSetCardUnitInfo(LPWFSCRDCUINFO lpWfsCrdCuInfo) = 0;
    virtual HRESULT CRDSetGuidAnceLight(LPWFSCRDSETGUIDLIGHT lpWfsCrdSetGuidLight) = 0;
    virtual HRESULT CRDPowerSaveControl(LPWFSCRDPOWERSAVECONTROL lpWfsCrdPowerSaveControl) = 0;
};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseCRD
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
    virtual bool FireStatusChanged(DWORD dwStatus) = 0;
    // 发送故障状态事件
    virtual bool FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription = nullptr) = 0;
    // 发送卡箱子故障状态事件
    virtual bool FireCardUnitError(LPWFSCRDCUERROR lpwfsCrdCuError) = 0;
    // 发送卡箱子故障状态阈值
    virtual bool FireCardUnitThreshold(LPWFSCRDCARDUNIT lpwfsCrdCardUnit) = 0;
    // 发送卡箱子状态改变
    virtual bool FireCardUnitChange(LPWFSCRDCARDUNIT lpwfsCrdCardUnit) = 0;
};

extern "C" SPBASECRD_EXPORT long CreateISPBaseCRD(LPCSTR lpDevType, ISPBaseCRD *&p);
//////////////////////////////////////////////////////////////////////////
