#pragma once
#include "CUInterface.h"
#include "CashUnitConfig.h"

//钞箱管理接口
struct CCashUnitManager : public ICashUnitManager, public CLogManage
{
    //标准构造函数
    CCashUnitManager();

    //析构函数
    virtual ~CCashUnitManager();

    //实现接口函数
public:
    // init the module
    virtual long Initialize(const LPWFSCIMNOTETYPELIST pNoteTypeList);

    // pick the interface of CDM logical cash unit
    virtual ICUInterface *GetCUInterface_CDM();

    // pick the interface of CIM logical cash unit
    virtual ICUInterface *GetCUInterface_CIM();

    // Uninitialize
    virtual void Uninitialize();

    //保存数据到配置文件
    virtual long SaveData();

    //测试数据从上次保存后是否修改过，是否需要保存数据
    virtual BOOL IsDirty() const;

    //内部成员函数
public:

    //内部数据成员
protected:
    CCUInterface    m_CIM;  //CIM接口对象
    CCUInterface    m_CDM;  //CDM接口对象
    CCashUnitConfig m_Config;//钞箱配置对象

};


