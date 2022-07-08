#pragma once
#include "ISPBaseClass.h"
#include "XFSIDC.H"
#include "XFSCRD.H"

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
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT GetFormList(LPSTR &lpFormList){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT GetForm(LPCSTR lpFormName, LPWFSIDCFORM &lpForm){return WFS_ERR_UNSUPP_COMMAND;}
    // 执行命令
    virtual HRESULT ReadTrack(LPCSTR lpFormName, LPSTR lpTrackData){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT WriteTrack(const LPWFSIDCWRITETRACK lpWriteData){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT EjectCard(DWORD dwTimeOut){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT ResetCount(){return WFS_ERR_UNSUPP_COMMAND;};
    virtual HRESULT SetKey(const LPWFSIDCSETKEY lpKeyData){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT WriteRawData(const LPWFSIDCCARDDATA *lppCardData){return WFS_IDC_NOTSUPP;}
    virtual HRESULT ChipIO(const LPWFSIDCCHIPIO lpChipIOIn, LPWFSIDCCHIPIO &lpChipIOOut){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT Reset(LPWORD lpResetIn){return WFS_ERR_UNSUPP_COMMAND;};
    virtual HRESULT ChipPower(LPWORD lpChipPower, LPWFSIDCCHIPPOWEROUT &lpData){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT ParseData(const LPWFSIDCPARSEDATA lpDataIn, LPSTR &lpTrackData){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT ReduceCount(){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT SetCount(LPWORD lpwCount){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT IntakeCardBack(){return WFS_ERR_UNSUPP_COMMAND;}

    // CRM类型接口(退卡)
    virtual HRESULT CMEjectCard(LPCSTR lpszCardNo){return WFS_ERR_UNSUPP_COMMAND;};
    virtual HRESULT CMSetCardData(LPCSTR lpszCardNo){return WFS_ERR_UNSUPP_COMMAND;};
    virtual HRESULT CMRetainCard(){return WFS_ERR_UNSUPP_COMMAND;};
    virtual HRESULT CMStatus(BYTE lpucQuery[118], BYTE lpucStatus[118]){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT CMReduceCount(){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT CMSetCount(LPWORD lpwCount){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT CMEmptyCard(LPCSTR lpszCardBox){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT CMGetCardInfo(LPCSTR lpszQuery, char lpszCardInfo[1024]){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT CMReset(){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT CMEmpytAllCard(){return WFS_ERR_UNSUPP_COMMAND;};
    virtual HRESULT CMClearSlot(LPCSTR lpszSlotNo){return WFS_ERR_UNSUPP_COMMAND;}

    // 增加 CRD系列命令(Card Dispenser) START
    /* CRD Info Commands */
    virtual HRESULT CRD_GetStatus(LPWFSCRDSTATUS &lpStatus){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT CRD_GetCapabilities(LPWFSCRDCAPS &lpCaps){return WFS_ERR_UNSUPP_COMMAND;}
    virtual HRESULT CRD_GetCardUnitInfo(LPWFSCRDCUINFO &lpCardUnit){return WFS_ERR_UNSUPP_COMMAND;}
    /* CRD Execute Commands */
    virtual HRESULT CRD_DispenseCard(const LPWFSCRDDISPENSE lpDispense){return WFS_ERR_UNSUPP_COMMAND;}                // 发卡
    virtual HRESULT CRD_EjecdCard(){return WFS_ERR_UNSUPP_COMMAND;}                                                    // 退卡
    virtual HRESULT CRD_RetainCard(const LPWFSCRDRETAINCARD lpRetainCard){return WFS_ERR_UNSUPP_COMMAND;}              // 回收卡
    virtual HRESULT CRD_Reset(const LPWFSCRDRESET lpResset){return WFS_ERR_UNSUPP_COMMAND;}                            // 复位
    virtual HRESULT CRD_SetGuidanceLight(const LPWFSCRDSETGUIDLIGHT lpSetGuidLight){return WFS_ERR_UNSUPP_COMMAND;}    // 设置灯状态
    virtual HRESULT CRD_SetCardUnitInfo(const LPWFSCRDCUINFO lpCuInfo){return WFS_ERR_UNSUPP_COMMAND;}                 // 设置卡箱信息
    virtual HRESULT CRD_PowerSaveControl(const LPWFSCRDPOWERSAVECONTROL lpPowerCtrl){return WFS_ERR_UNSUPP_COMMAND;}   // 激活/停用节能功能
    // 增加 CRD系列命令(Card Dispenser) END
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
