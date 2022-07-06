#pragma once
#include "ISPBaseClass.h"
#include "XFSIDC.H"

//////////////////////////////////////////////////////////////////////////
#if defined(SPBASEIDC_LIBRARY)
    #define SPBASEIDC_EXPORT Q_DECL_EXPORT
#else
    #define SPBASEIDC_EXPORT Q_DECL_IMPORT
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
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus) = 0;
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps) = 0;
    virtual HRESULT GetFormList(LPSTR &lpFormList) = 0;
    virtual HRESULT GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm) = 0;
    // 执行命令
    virtual HRESULT ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData) = 0;
    virtual HRESULT WriteTrack(const LPWFSIDCWRITETRACK lpWriteData) = 0;
    virtual HRESULT EjectCard(DWORD dwTimeOut) = 0;
    virtual HRESULT RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData) = 0;
    virtual HRESULT ResetCount() = 0;
    virtual HRESULT SetKey(const LPWFSIDCSETKEY lpKeyData) = 0;
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut) = 0;
    virtual HRESULT WriteRawData(const LPWFSIDCCARDDATA *lppCardData) = 0;
    virtual HRESULT ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut) = 0;
    virtual HRESULT Reset(LPWORD lpResetIn) = 0;
    virtual HRESULT ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData) = 0;
    virtual HRESULT ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData) = 0;
#ifdef CARD_REJECT_GD_MODE
    //读卡器新增扩展部分
    virtual HRESULT ReduceCount() = 0;
    virtual HRESULT SetCount(LPWORD lpwCount) = 0;
    virtual HRESULT IntakeCardBack() = 0;
    //退卡部分
    virtual HRESULT CMEjectCard(LPCSTR lpszCardNo) = 0;
    virtual HRESULT CMSetCardData(LPCSTR lpszCardNo) = 0;
    virtual HRESULT CMRetainCard() = 0;
    virtual HRESULT CMStatus(BYTE lpucQuery[118], BYTE lpucStatus[118]) = 0;
    virtual HRESULT CMReduceCount() = 0;
    virtual HRESULT CMSetCount(LPWORD lpwCount) = 0;
    virtual HRESULT CMEmptyCard(LPCSTR lpszCardBox) = 0;
    virtual HRESULT CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024]) = 0;
    virtual HRESULT CMReset() = 0;
#endif
};
//////////////////////////////////////////////////////////////////////////
struct ISPBaseIDC
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

extern "C" SPBASEIDC_EXPORT long CreateISPBaseIDC(LPCSTR lpDevType, ISPBaseIDC *&p);
//////////////////////////////////////////////////////////////////////////
