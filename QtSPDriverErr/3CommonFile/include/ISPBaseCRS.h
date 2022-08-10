#pragma once
#include "ISPBaseClass.h"
#include "XFSCDM.H"
#include "XFSCIM.H"
#include "XFSCIM320.H"

#if defined(SPBASECRS_LIBRARY)
#define SPBASECRSSHARED_EXPORT Q_DECL_EXPORT
#else
#define SPBASECRSSHARED_EXPORT Q_DECL_IMPORT
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

    //辅助接口
    virtual void SetExeFlag(DWORD dwCmd, BOOL bInExe) = 0;
    virtual int GetTooManeyFlag() = 0;   //test#7

    // CDM类型接口
    // 查询命令
    virtual HRESULT CDMGetStatus(LPWFSCDMSTATUS &lpStatus) = 0;
    virtual HRESULT CDMGetCapabilities(LPWFSCDMCAPS &lpCaps) = 0;
    virtual HRESULT CDMGetCashUnitInfo(LPWFSCDMCUINFO &lpCUInfor) = 0;
    virtual HRESULT CDMGetMixType(LPWFSCDMMIXTYPE *&lpMixType) = 0;
    virtual HRESULT CDMGetMixTable(USHORT usMixNumber, LPWFSCDMMIXTABLE &lpMixTable) = 0;
    virtual HRESULT CDMGetPresentStatus(WORD wPos, LPWFSCDMPRESENTSTATUS &stPresentStatus) = 0;
    virtual HRESULT CDMGetCurrencyEXP(LPWFSCDMCURRENCYEXP *&lppCurrencyExp) = 0;
    // 执行命令
    virtual HRESULT CDMDenominate(LPWFSCDMDENOMINATE lpDenoData, LPWFSCDMDENOMINATION &lpDenoOut) = 0;
    virtual HRESULT CDMDispense(LPWFSCDMDISPENSE lpDisData, LPWFSCDMDENOMINATION &pDenoInOut) = 0;
    virtual HRESULT CDMPresent(WORD wPos) = 0;
    virtual HRESULT CDMReject() = 0;
    virtual HRESULT CDMRetract(const WFSCDMRETRACT &stData) = 0;
    virtual HRESULT CDMOpenShutter(WORD wPos) = 0;
    virtual HRESULT CDMCloseShutter(WORD wPos) = 0;
    virtual HRESULT CDMStartEXChange(const LPWFSCDMSTARTEX lpData, LPWFSCDMCUINFO &lpCUInfor) = 0;
    virtual HRESULT CDMEndEXChange(const LPWFSCDMCUINFO lpCUInfo) = 0;
    virtual HRESULT CDMReset(const LPWFSCDMITEMPOSITION lpResetIn) = 0;
    virtual HRESULT CDMSetCashUnitInfo(const LPWFSCDMCUINFO lpCUInfo) = 0;
    virtual HRESULT CDMTestCashUnits(const LPWFSCDMITEMPOSITION lpItemPosition, LPWFSCDMCUINFO &lpCUInfo)// = 0;  //30-00-00-00(FS#0007)
    {
        return WFS_ERR_UNSUPP_COMMAND;  // 新增加接口实例化
    }

    // CIM类型接口
    // 查询命令
    virtual HRESULT CIMGetStatus(LPWFSCIMSTATUS &lpStatus) = 0;
    virtual HRESULT CIMGetCapabilities(LPWFSCIMCAPS &lpCaps) = 0;
    virtual HRESULT CIMGetCashUnitInfo(LPWFSCIMCASHINFO &lpCUInfo) = 0;
    virtual HRESULT CIMGetBankNoteType(LPWFSCIMNOTETYPELIST &lpNoteList) = 0;
    virtual HRESULT CIMGetCashInStatus(LPWFSCIMCASHINSTATUS &lpCashInStatus) = 0;
    virtual HRESULT CIMGetCurrencyEXP(LPWFSCIMCURRENCYEXP *&lppCurrencyExp) = 0;
    // 执行命令
    virtual HRESULT CIMCashInStart(const LPWFSCIMCASHINSTART lpData) = 0;
    virtual HRESULT CIMCashIn(LPWFSCIMNOTENUMBERLIST &lpResult) = 0;
    virtual HRESULT CIMCashInEnd(LPWFSCIMCASHINFO &lpCUInfo) = 0;
    virtual HRESULT CIMCashInRollBack(LPWFSCIMCASHINFO &lpCUInfo) = 0;
    virtual HRESULT CIMRetract(const LPWFSCIMRETRACT lpData) = 0;
    virtual HRESULT CIMOpenShutter(WORD wPos) = 0;
    virtual HRESULT CIMCloseShutter(WORD wPos) = 0;
    virtual HRESULT CIMStartEXChange(const LPWFSCIMSTARTEX lpStartEx, LPWFSCIMCASHINFO &lpCUInfor) = 0;
    virtual HRESULT CIMEndEXChange(const LPWFSCIMCASHINFO lpCUInfo) = 0;
    virtual HRESULT CIMReset(const LPWFSCIMITEMPOSITION lpResetIn) = 0;
    virtual HRESULT CIMConfigureCashInUnits(const LPWFSCIMCASHINTYPE *lppCashInType) = 0;
    virtual HRESULT CIMConfigureNoteTypes(const LPUSHORT lpusNoteIDs) = 0;
    virtual HRESULT CIMSetCashUnitInfo(const LPWFSCIMCASHINFO lpCUInfo) = 0;
    virtual HRESULT CIMCashInLimit(const LPWFSCIMCASHINLIMIT lpCUInfo) = 0;
    virtual HRESULT CIMCashUnitCount(const LPWFSCIMCOUNT lpCIMCount)// = 0;       //30-00-00-00(FS#0022)
    {
        return WFS_ERR_UNSUPP_COMMAND;  // 新增加接口实例化
    }
};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseCRS
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
};

extern "C" Q_DECL_EXPORT long CreateISPBaseCRS(LPCSTR lpDevType, ISPBaseCRS *&p);
//////////////////////////////////////////////////////////////////////////
