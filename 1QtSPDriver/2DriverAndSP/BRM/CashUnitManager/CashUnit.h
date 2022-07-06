#pragma once
#include "ICashUnitManager.h"
#include "Configurable.h"
#include "CUMngrFunc.h"
//#include "MultiString.h"      //30-00-00-00(FT#0004)
#include "ISOCurrencySet.h"
//#include "ILogWrite.h"        //30-00-00-00(FT#0004)
#include "QtTypeInclude.h"      //30-00-00-00(FT#0004)
#define CLEAR_ARRAY(a)      memset(a, 0, sizeof(a))

//钞箱信息访问接口
struct CCashUnit :  public ICashUnit, public CConfigurable, public CLogManage
{
    //标准构造函数
    CCashUnit(const LPWFSCIMNOTETYPELIST pNoteTypeList, BOOL bCDM);

    //析构函数
    //调用ClearData删除由自己分配的内存
    virtual ~CCashUnit();

    void SetIndex(USHORT usIndex);

    void SetNumber(USHORT usNumber);

    //实现接口函数
public:
    //index
    // Number
    virtual USHORT GetNumber() const;

    //Physical Box Index
    virtual USHORT GetIndex() const;

    virtual BOOL IsCDM() const;

    //identifier
    // Physical Position Name
    virtual LPCSTR GetPhysicalPositionName() const;

    virtual long SetPhysicalPositionName(LPCSTR lpPhysicalPositionName);

    // Unit ID
    virtual void GetUnitID(CHAR cUnitID[5]) const;

    virtual long SetUnitID(CHAR cUnitID[5]);

    // Type
    virtual ADP_CASSETTE_TYPE GetType() const;

    virtual long SetType(ADP_CASSETTE_TYPE usType);

    // Currency ID
    void GetCurrencyID(CHAR cCurrencyID[3]) const;

    virtual long SetCurrencyID(CHAR cCurrencyID[3]);

    // Values
    //virtual void GetPHUnitID(CHAR cUnitID[5]) const
    //{
    //todo
    //
    //    return;
    //}

    //virtual long SetPHUnitID(CHAR cUnitID[5])
    //{
    //todo
    //  return 0;
    //}


    // Values
    virtual ULONG GetValues() const;

    virtual long SetValues(ULONG ulValues);

    // Cash Unit Name
    virtual LPCSTR GetCashUnitName() const;

    virtual long SetCashUnitName(LPCSTR lpszCashUnitName);

    //cim
    // 函数返回值表示NoteIDs的个数，aryusNoteIDs保存得到的ID
    virtual DWORD GetNoteIDs(USHORT aryusNoteIDs[MAX_NOTEID_SIZE]) const;

    virtual long SetNoteIDs(LPUSHORT lpusNoteIDs, USHORT usSize);

    virtual long ClearNoteIDs();

    // Item type
    virtual DWORD GetItemType() const ;

    virtual long SetItemType(DWORD fwItemType);
    // Note Number List
    // 函数返回值表示"钞票ID-张数"信息的组数。
    virtual DWORD GetNoteNumbers(WFSCIMNOTENUMBER aryNoteNum[MAX_NOTEID_SIZE]) const;

    LPWFSCIMNOTENUMBERLIST GetNoteNumbers() const;

    // 一次性设置所有的"钞票ID-张数"信息。
    virtual long SetNoteNumbers(LPWFSCIMNOTENUMBERLIST lpNoteNumList);

    // 取得单种钞票的张数
    virtual long GetNumberByNote(USHORT usNoteID, ULONG &ulCount) const;

    //取得单种钞票的面额
    virtual long GetCurrencyValueByNoteID(USHORT usNoteID, char cCurrency[3], ULONG &ulValue) const;

    // 设置某种钞票的张数，如果设置的NoteID以前没有,那么就新增
    virtual long SetNumbersByNote(USHORT usNoteID, ULONG ulCount);

    // 清理所有的"钞票ID-张数"信息。
    virtual void ClearNoteNumbers();

    //count
    // Count
    virtual ULONG GetCount() const;

    virtual long SetCount(ULONG ulCount);

    // Initial Count
    virtual ULONG GetInitialCount() const;

    virtual long SetInitialCount(ULONG ulInitialCount);

    // Cash In Count
    virtual ULONG GetCashInCount() const;
    virtual long SetCashInCount(ULONG ulCashInCount);

    // Reject Count
    virtual ULONG GetRejectCount() const;
    virtual long SetRejectCount(ULONG ulRejectCount);

    //min max
    // Maximum
    virtual ULONG GetMaximum() const;

    virtual long SetMaximum(ULONG ulMaximum);

    // Minimum
    virtual ULONG GetMinimum() const;

    virtual long SetMinimum(ULONG ulMinimum);

    // Status
    virtual CASHUNIT_STATUS GetStatus() const;

    virtual long SetStatus(CASHUNIT_STATUS usStatus, BOOL bForce = FALSE);

    // Hardware Sensors
    virtual BOOL GetHardwareSensors() const;

    virtual long SetHardwareSensors(BOOL bHardwareSensors);

    // AppLock
    virtual BOOL GetAppLock() const;

    virtual long SetAppLock(BOOL bAppLock);
    //xfs
    // Convert the information to XFS WFSCDMCASHUNIT
    virtual LPWFSCDMCASHUNIT BuildByXFSCDMFormat();

    // Convert the information to XFS WFSCIMCASHIN
    virtual LPWFSCIMCASHIN BuildByXFSCIMFormat();

    //other status
    //Error Count
    virtual DWORD GetErrorCount() const;

    virtual void SetErrorCount(DWORD dwErrorCount);

    // Exchange state
    virtual BOOL GetExchangeState() const;

    virtual long SetExchangeState(BOOL bEx);

    //设置数据改变标志
    virtual void SetItemChanged();

    // Check for changes of any item
    // 如果任何数据元素有改变,那么返回一次TRUE.
    //在没有调用新的设置操作导致数据改变的情况发生前,重复调用将返回FALSE.
    virtual BOOL IsItemChanged();

    //其他成员函数
public:
    //从最大最小数中计算设备状态
    CASHUNIT_STATUS ComputeStatusFromMinMax() const;

    //从最大最小及配置信息计算状态
    CASHUNIT_STATUS ComputeStatusFromCfgAndMinMax() const;

    //从配置信息计算状态
    CASHUNIT_STATUS ComputeStatusFromCfg(CASHUNIT_STATUS eStatus) const;

    //初始化数据，清除本类接口相关数据
    //仅在构造函数中调用
    void InitData();

    //清除本类接口相关数据，如有指针，释放其内存
    void ClearData();

    //设置XFS数据结构，保存与本类相关的数据
    long SetXFSData(LPWFSCDMCASHUNIT pData);
    long SetXFSData(LPWFSCIMCASHIN pData);
    int LoadConfig(ICashUnitConfigItem *pCfgItem);
    int SaveConfig();
    void CopyFrom(const CCashUnit &src, BOOL bBackup);

    //是否处于设置XFS状态函数（SetByXFSFormat）调用中
    virtual BOOL IsInSetByXFSFormat() const;

    const LPWFSCIMNOTETYPELIST GetAllNoteTypeList() const;

    void SetVerifyCUInfoFailedFlag(BOOL bFailed);

    //pbItemTypeNoteIDError,用于返回是否是因为ITEM类型、NoteID错误
    //返回值：<0校验失败，返回XFS返回码，szErrorDesc返回错误描述；0，校验成功
    long VerifyCassInfo(BOOL bVerifyType, char szErrorDesc[250] = NULL, BOOL *pbItemTypeNoteIDError = NULL) const; //用于装载配置后校验钞箱信息

private:
    long InnerSetXFSData(LPWFSCDMCASHUNIT pData);
    long InnerSetXFSData(LPWFSCIMCASHIN pData);
    int InnerLoadConfig(ICashUnitConfigItem *pCfgItem);
    long LoadNoteNumber();
    long SaveNoteNumber();
    long LoadNoteIDs();
    long SaveNoteIDs();
    BOOL NoteIDsAreSameCurrencyValue() const;
    BOOL NoteIDIsInNoteTypeList(USHORT usNoteID) const;
    BOOL NoteIDOfNoteNumberIsInNoteTypeList(LPWFSCIMNOTENUMBERLIST lpNoteNumberList) const;
    BOOL NoteIDsAreInNoteTypeList(LPUSHORT lpusNoteIDs, USHORT usSize) const;
    BOOL CurrencyIDIsInNoteTypeList(const char *pCurrencyID) const;
    BOOL NoteTypeIsInNoteTypeList(const char *pCurrencyID, ULONG ulValues) const;
    void BuildCashUnitExtra();
    ICashUnitConfigItem *GetPHCfgItemByIndex(USHORT usPHIndex, ICashUnitConfigItem *pCfgItem) const;  //通过索引得到物理钞箱配置的指针

    //设置是否在SetByXFSFormat函数中的标志
    void SetInSetByXFSFormat(BOOL bIn);

    //私有数据
private:
    BOOL        m_bCDM;     //是否为CDM钞箱，TRUE是，否则为CIM钞箱

    //
    USHORT          m_usPHIndex;
    USHORT          m_usNumber;
    ADP_CASSETTE_TYPE  m_usType;
    std::string          m_sCashUnitName;
    std::string         m_sPhysicalPositionName;
    CHAR            m_cUnitID[5 + 1];
    CHAR            m_cCurrencyID[3 + 1];
    ULONG           m_ulValues;
    ULONG           m_ulInitialCount;
    ULONG           m_ulCount;
    ULONG           m_ulRejectCount;
    ULONG           m_ulMinimum;
    ULONG           m_ulMaximum;
    BOOL            m_bAppLock;
    CASHUNIT_STATUS m_usStatus;
    BOOL            m_bVerifyCUInfoFailed;
    DWORD           m_fwItemType;
    BOOL            m_bHardwareSensors;
    ULONG           m_ulCashInCount;
    USHORT          m_aryusNoteIDs[MAX_NOTEID_SIZE];
    USHORT          m_usNoteIDCount;
    LPWFSCIMNOTENUMBERLIST     m_lpNoteNumberList;
    const LPWFSCIMNOTETYPELIST m_lpAllNoteTypeList;
    DWORD                      m_dwErrorCount;//钞箱错误次数

    BOOL m_bInSetByXFSFormat;       //指示是否处于SetByXFSFormat函数调用中
    BOOL m_bItemChanged;            //指示对象数据是否改变
    BOOL m_bExchangeState;          //指示是否处于交换状态
    BOOL m_bIgnoreMax;              //指示是否更改最大值 30-00-00-00(FT#0004)

    WFSCDMCASHUNIT      m_CDMCashUnit;
    LPWFSCDMPHCU        m_lppCDMPHCU[1];
    WFSCDMPHCU          m_CDMPHCU;

    WFSCIMCASHIN        m_CIMCashUnit;
    LPWFSCIMPHCU        m_lppCIMPHCU[1];
    WFSCIMPHCU          m_CIMPHCU;
    CMultiString        m_msCIMExtra;
    static ULONG        m_ulExchangeCount;
};

