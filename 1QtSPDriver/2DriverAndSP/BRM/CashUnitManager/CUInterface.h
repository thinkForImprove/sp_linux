#pragma once
#include "ICashUnitManager.h"
#include "CashUnit.h"
#include "ILogWrite.h"
//#include <vector>
//using namespace std;

//钞箱接口实现类
struct CCUInterface : public ICUInterface, public CLogManage
{
    //标准构造函数
    CCUInterface(BOOL bCDM);

    //析构函数
    //调用ClearData删除由自己分配的内存
    virtual ~CCUInterface();

    //实现接口函数
public:
    virtual BOOL IsCDM() const;

    // Convert the information to XFS WFSCIMCASHINFO
    virtual LPWFSCIMCASHINFO BuildByXFSCIMFormat();

    // Convert the information to XFS WFSCDMCUINFO
    virtual LPWFSCDMCUINFO BuildByXFSCDMFormat();

    // Set the information from XFS WFSCIMCASHINFO
    virtual long SetByXFSCIMFormat(const LPWFSCIMCASHINFO lpCUInfor);

    // Set the information from XFS WFSCIMCASHINFO
    virtual long SetByXFSCDMFormat(const LPWFSCDMCUINFO lpCUInfor);

    //Sync Data from another interface by physical index
    virtual long SyncDataByPhysicalIndex(ICUInterface *pSrc, BOOL bModifyStats = FALSE);

    // Count of Logical cash unit
    virtual USHORT GetCUCount() const;

    // Get Logical cash unit information by Number
    virtual ICashUnit *GetCUByNumber(USHORT usNumber);

    // Get First Logical cash unit information by physical Index
    virtual ICashUnit *GetFirstCUByPHIndex(USHORT usPHIndex);

    // Get Next Logical cash unit information by physical Index
    // return NULL if not existing
    virtual ICashUnit *GetNextCUByPHIndex(USHORT usPHIndex);

    virtual int SaveToConfig();

    virtual BOOL IsDirty() const;
    //其他成员函数
public:
    //pNoteTypeList：ponter to note type list, will keep valid in life cycle
    virtual int LoadFromConfig(ICashUnitConfig *pConfig,
                               const LPWFSCIMNOTETYPELIST pNoteTypeList, BOOL bCDM);

    void CopyFrom(const CCUInterface &src, BOOL bBackup);

    //初始化数据，清除本类接口相关数据
    //仅在构造函数中调用
    void InitData();

    //清除本类接口相关数据，如有指针，释放其内存
    void ClearData();

    //内部数据成员
protected:
    const BOOL          m_bCDM;
    std::vector<CCashUnit *> m_CUs; //逻辑钞箱数组
    std::vector<CCashUnit *>::iterator m_itGetCUByPHIndex;  //用于通过物理索引获取钞箱的叠代器
    LPWFSCIMCASHINFO    m_lpCIMCashInfo;
    LPWFSCDMCUINFO      m_lpCDMCashInfo;
};


