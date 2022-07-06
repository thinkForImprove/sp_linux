//CashUnitManager.cpp: 接口实现文件
//目前设计的局限性说明：
//1. 暂不支持SetType、SetByXFSFormat修改钞箱类型，钞箱类型必须与配置文件一致
//2. 上层必须同时维护逻辑钞箱数据和物理钞箱数据，钞箱管理模块不进行同步
//#include "ICashUnitManager.h"
#include <assert.h>
#include "CashUnitManager.h"

//钞箱配置文件名
#define CASH_UNIT_CFG_FILE      "BRMCashUnit.xml"

CCashUnitManager::CCashUnitManager() : m_CDM(TRUE), m_CIM(FALSE)
{
    SetLogFile(LOGFILE, "CCashUnitManager", "CCashUnitManager");
}

//析构函数
CCashUnitManager::~CCashUnitManager()
{
}

//// init the module
long CCashUnitManager::Initialize(const LPWFSCIMNOTETYPELIST pNoteTypeList)
{
    DEFMODULE(CUMngr.Initialize);
    if (pNoteTypeList == NULL)
    {
        Log(ThisModule, -1, "pNoteTypeList = NULL");
        return WFS_ERR_SOFTWARE_ERROR;
    }
    for (USHORT i = 0; i < pNoteTypeList->usNumOfNoteTypes; i++)
    {
        if (pNoteTypeList->lppNoteTypes[i] == NULL)
        {
            Log(ThisModule, -1, "pNoteTypeList->lppNoteTypes[%hd] = NULL", i);
            return WFS_ERR_SOFTWARE_ERROR;
        }
    }

    long lRet = m_Config.Load(CASH_UNIT_CFG_FILE);
    if (lRet < 0)
    {
        //        if (!m_Config.BackUp(FALSE))
        //        {
        //            return WFS_ERR_SOFTWARE_ERROR;
        //        }
        //        lRet = m_Config.Load(CASH_UNIT_CFG_FILE);
        //        if (lRet < 0)
        //        {
        //            return WFS_ERR_SOFTWARE_ERROR;
        //        }
        return WFS_ERR_SOFTWARE_ERROR;
    }
    else
    {
        m_Config.BackUp(TRUE);
    }
    VERIFY_FUNC_CALL(m_CDM.LoadFromConfig(&m_Config, pNoteTypeList, TRUE));
    VERIFY_FUNC_CALL(m_CIM.LoadFromConfig(&m_Config, pNoteTypeList, FALSE));
    return 0;
}

// pick the interface of CDM logical cash unit
ICUInterface *CCashUnitManager::GetCUInterface_CDM()
{
    return &m_CDM;
}

// pick the interface of CIM logical cash unit
ICUInterface *CCashUnitManager::GetCUInterface_CIM()
{
    return &m_CIM;
}

// Uninitialize
void CCashUnitManager::Uninitialize()
{
    SaveData();
    delete this;
}

//保存数据到配置文件
long CCashUnitManager::SaveData()
{
    DEFMODULE(CUMngr.SaveData);
    VERIFY_DIRTY();
    VERIFY_FUNC_CALL(m_CDM.SaveToConfig());
    VERIFY_FUNC_CALL(m_CIM.SaveToConfig());
    VERIFY_FUNC_CALL(m_Config.Save());
    m_Config.BackUp(TRUE);
    return 0;
}

//测试数据从上次保存后是否修改过，是否需要保存数据
BOOL CCashUnitManager::IsDirty() const
{
    if (m_CDM.IsDirty())
        return TRUE;
    if (m_CIM.IsDirty())
        return TRUE;
    return FALSE;
}

//总的创建方法
//extern "C" ICashUnitManager * CreateCashUnitManager()
//{
//    return new CCashUnitManager;
//}

extern "C" long CreateCashUnitManager(ICashUnitManager *&iInst, const char *pszConfigName)
{
    iInst = new CCashUnitManager;
    return 0;
}
