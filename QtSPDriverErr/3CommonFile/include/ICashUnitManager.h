#pragma once
#include "XFSCIM.H"
#include "XFSCDM.H"
#include "CashUnitDefine.h"

//#define CUM_NO_VTABLE     _declspec(novtable)

// NoteIDs of CU can take
#define MAX_NOTEID_SIZE     (8)

//#define ADP_MAX_CU_SIZE       (10)
#define ADP_MAX_VALUECOUNTS (6)

//钞箱信息访问接口
struct ICashUnit
{
    // Number
    virtual USHORT GetNumber() const = 0;

    virtual USHORT GetIndex() const = 0;

    //Is CDM
    virtual BOOL IsCDM() const = 0;

    // Type
    virtual ADP_CASSETTE_TYPE GetType() const = 0;
    virtual long SetType(ADP_CASSETTE_TYPE usType) = 0;

    // Cash Unit Name
    virtual LPCSTR GetCashUnitName() const = 0;
    virtual long SetCashUnitName(LPCSTR lpszCashUnitName) = 0;

    // Physical Position Name
    virtual LPCSTR GetPhysicalPositionName() const = 0;
    virtual long SetPhysicalPositionName(LPCSTR lpPhysicalPositionName) = 0;

    // Unit ID
    virtual void GetUnitID(CHAR cUnitID[5]) const = 0;
    virtual long SetUnitID(CHAR cUnitID[5]) = 0;

    // Currency ID
    virtual void GetCurrencyID(CHAR cCurrencyID[3]) const = 0;
    virtual long SetCurrencyID(CHAR cCurrencyID[3]) = 0;

    // Values
    virtual ULONG GetValues() const = 0;
    virtual long SetValues(ULONG ulValues) = 0;

    // Initial Count
    virtual ULONG GetInitialCount() const = 0;
    virtual long SetInitialCount(ULONG ulInitialCount) = 0;

    // Count
    virtual ULONG GetCount() const = 0;
    virtual long SetCount(ULONG ulCount) = 0;

    // Reject Count
    virtual ULONG GetRejectCount() const = 0;
    virtual long SetRejectCount(ULONG ulRejectCount) = 0;

    // Minimum
    virtual ULONG GetMinimum() const = 0;
    virtual long SetMinimum(ULONG ulMinimum) = 0;

    // Maximum
    virtual ULONG GetMaximum() const = 0;
    virtual long SetMaximum(ULONG ulMaximum) = 0;

    // Status
    virtual CASHUNIT_STATUS GetStatus() const = 0;
    virtual long SetStatus(CASHUNIT_STATUS usStatus, BOOL bForce = FALSE) = 0;

    // AppLock
    virtual BOOL GetAppLock() const = 0;
    virtual long SetAppLock(BOOL bAppLock) = 0;

    // Exchange state
    virtual BOOL GetExchangeState() const = 0;
    virtual long SetExchangeState(BOOL bEx) = 0;

    //Error Count
    virtual DWORD GetErrorCount() const = 0;
    virtual void SetErrorCount(DWORD dwErrorCount) = 0;

    // Hardware Sensors
    virtual BOOL GetHardwareSensors() const = 0;
    virtual long SetHardwareSensors(BOOL bHardwareSensors) = 0;

    virtual DWORD GetNoteIDs(USHORT aryusNoteIDs[MAX_NOTEID_SIZE]) const = 0; // 函数返回值表示NoteIDs的个数，aryusNoteIDs保存得到的ID
    virtual long SetNoteIDs(LPUSHORT lpusNoteIDs, USHORT usSize) = 0;
    virtual long ClearNoteIDs() = 0;

    // Item type
    virtual DWORD GetItemType() const = 0;
    virtual long SetItemType(DWORD fwItemType) = 0;

    // Cash In Count
    virtual ULONG GetCashInCount() const = 0;
    virtual long SetCashInCount(ULONG ulCashInCount) = 0;

    // Note Number List
    virtual DWORD GetNoteNumbers(WFSCIMNOTENUMBER aryNoteNum[MAX_NOTEID_SIZE]) const = 0; // 函数返回值表示"钞票ID-张数"信息的组数。
    virtual long SetNoteNumbers(LPWFSCIMNOTENUMBERLIST lpNoteNumList) = 0; // 一次性设置所有的"钞票ID-张数"信息。
    virtual long GetNumberByNote(USHORT usNoteID, ULONG &ulCount) const = 0;  // 取得单种钞票的张数,找不到时返回WFS_ERR_INVALID_DATA
    virtual long SetNumbersByNote(USHORT usNoteID, ULONG ulCount) = 0;  // 设置某种钞票的张数，如果设置的NoteID以前没有,那么就新增
    virtual void ClearNoteNumbers() = 0; // 清理所有的"钞票ID-张数"信息。

    // Check for changes of any item
    // 如果任何数据元素有改变,那么返回一次TRUE.
    //在没有调用新的设置操作导致数据改变的情况发生前,重复调用将返回FALSE.
    virtual BOOL IsItemChanged() = 0;

    // Convert the information to XFS WFSCDMCASHUNIT
    virtual LPWFSCDMCASHUNIT BuildByXFSCDMFormat() = 0;

    // Convert the information to XFS WFSCIMCASHIN
    virtual LPWFSCIMCASHIN BuildByXFSCIMFormat() = 0;
};

//钞箱接口
struct ICUInterface
{
    //Is CDM?
    virtual BOOL IsCDM() const = 0;

    // Count of Logical cash unit
    virtual USHORT GetCUCount() const = 0;

    // Get Logical cash unit information by Number
    virtual ICashUnit *GetCUByNumber(USHORT usNumber) = 0;

    // Get First Logical cash unit information by physical Index
    // return NULL if not existing
    virtual ICashUnit *GetFirstCUByPHIndex(USHORT usPHIndex) = 0;

    // Get Next Logical cash unit information by physical Index
    // return NULL if not existing
    virtual ICashUnit *GetNextCUByPHIndex(USHORT usPHIndex) = 0;

    // Convert the information to XFS WFSCDMCUINFO
    virtual LPWFSCDMCUINFO BuildByXFSCDMFormat() = 0;
    // Convert the information to XFS WFSCIMCASHINFO
    virtual LPWFSCIMCASHINFO BuildByXFSCIMFormat() = 0;

    // Set the information from XFS WFSCDMCUINFO
    virtual long SetByXFSCDMFormat(const LPWFSCDMCUINFO lpCUInfor) = 0;
    // Set the information from XFS WFSCIMCASHINFO
    virtual long SetByXFSCIMFormat(const LPWFSCIMCASHINFO lpCUInfor) = 0;

    //Sync Data from another interface by physical index
    virtual long SyncDataByPhysicalIndex(ICUInterface *pSrc, BOOL bModifyStats = FALSE) = 0;
};

//钞箱管理接口
struct ICashUnitManager
{
    // init the module
    //pNoteTypeList，pointer to Note Type List, this pointer always keep valid
    virtual long Initialize(const LPWFSCIMNOTETYPELIST pNoteTypeList) = 0;

    // pick the interface of CDM logical cash unit
    virtual ICUInterface *GetCUInterface_CDM() = 0;

    // pick the interface of CIM logical cash unit
    virtual ICUInterface *GetCUInterface_CIM() = 0;

    //测试数据从上次保存后是否修改过，是否需要保存数据
    virtual BOOL IsDirty() const = 0;

    //save cassette data to configure file
    virtual long SaveData() = 0;

    // Uninitialize
    virtual void Uninitialize() = 0;
};

//总的创建方法
extern "C" long CreateCashUnitManager(ICashUnitManager *&iInst, const char *pszConfigName = "BRMCashUnit.xml");
// extern "C" ICashUnitManager * CreateCashUnitManager();
extern "C" BOOL CurrencyCodeFitISOList(const char cCurrency[3]);


