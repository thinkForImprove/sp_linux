#pragma once
#include "ICashUnitConfig.h"

//可配置对象基类
//CCashUnit从它继承，保存数据是否脏标志和配置文件节点对象
struct CConfigurable
{
    //构造函数为protected，防止直接实例化
protected:
    CConfigurable()
    {
        m_pCfgItem = nullptr;
        m_bDirty = FALSE;
    }

    //其他成员函数
public:
    virtual int LoadConfig(ICashUnitConfigItem *pCfgItem)
    {
        m_pCfgItem = pCfgItem;
        return 0;
    }

    void SetDirty(BOOL bDirty = TRUE)
    {
        m_bDirty = bDirty;
    }

    virtual BOOL IsDirty() const
    {
        return m_bDirty;
    }

    void CopyFrom(const CConfigurable &src)
    {
        m_bDirty = src.m_bDirty;
        m_pCfgItem = src.m_pCfgItem;
    }

    //内部数据成员
protected:
    friend struct CCUInterface;
    ICashUnitConfigItem *m_pCfgItem;    //配置节点
private:
    BOOL m_bDirty;                      //数据是否需要保存
};


