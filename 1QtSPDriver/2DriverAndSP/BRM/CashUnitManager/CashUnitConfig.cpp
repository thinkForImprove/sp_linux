// CashUnitConfig.cpp: implementation of the CCashUnitConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "CashUnitConfig.h"
#include <stdlib.h>
#include <stdio.h>
//#include <STDARG.H>
//#include "Markup.h"
//#define THISFILE  "CashUnitConfig"
static const char *ThisFile = "CashUnitConfig";
static const char *g_paryKey[3][2] =
{
    {"LCU", "CDM"},
    {"LCU", "CIM"},
    {"PCU", NULL}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCashUnitConfig::CCashUnitConfig()
{
    SetLogFile(LOGFILE, ThisFile, ThisFile);
    m_pConfig = NULL;
    m_pRootNode = NULL;
    m_pCommonConfigItem = NULL;
}

CCashUnitConfig::~CCashUnitConfig()
{
    Clear();
}

void CCashUnitConfig::Clear()
{
    if (m_pConfig != NULL)
    {
        m_pConfig->Release();
        m_pConfig = NULL;
    }
    for (int i = 0; i < 3; i++)
    {
        ITEMSIT it;
        for (it = m_CUs[i].begin(); it != m_CUs[i].end(); it++)
        {
            delete *it;
        }
        m_CUs[i].clear();
    }
    if (m_pCommonConfigItem != NULL)
    {
        delete m_pCommonConfigItem;
        m_pCommonConfigItem = NULL;
    }
}

int CCashUnitConfig::Load(const char *pFileName)
{
    const char *ThisModule = "Load";
    //如果配置文件对象不为空，先释放它
    m_pRootNode = NULL;
    Clear();

    //创建配置文件对象
    m_pConfig = SMCreateConfig();
    if (m_pConfig == NULL)
    {
        Log(ThisModule, -1, "SMCreateConfig()=NULL");
        return -1;
    }

    //先从ETCDIR目录装入配置文件
    m_sFilePath =  m_sFilePath + SPETCPATH + "/" + pFileName;
    m_sFileBackupPath = m_sFilePath.substr(0, m_sFilePath.rfind('.'));
    m_sFileBackupPath += ".bak";
    bool bLoadSucc = m_pConfig->Load(m_sFilePath.c_str());

    if (!bLoadSucc)
    {
        bLoadSucc = m_pConfig->Load(m_sFileBackupPath.c_str());
    }

    if (!bLoadSucc)
    {
        int iRow, iCol;
        m_pConfig->GetErrRowCol(&iRow, &iCol);
        Log(ThisModule, -1,
            "m_pConfig->Load(%s) failed(%s) iRow=%d, iCol=%d",
            m_sFilePath.c_str(), m_pConfig->GetErrDesc(),
            iRow, iCol);
        m_pConfig->Release();
        m_pConfig = NULL;
        return -2;
    }


    //得到根节点，如果根节点不存在，释放配置文件对象并返回失败
    m_pRootNode = m_pConfig->GetRootNode();
    if (m_pRootNode == NULL ||
        strcmp(m_pRootNode->GetTagName(), "CUDATA") != 0)
    {
        m_pConfig->Release();
        m_pConfig = NULL;
        return -3;
    }

    //查找并分配CConfigItem节点
    ISMConfigNode *pNode;
    for (int i = 0; i < 3; i++)
    {
        if (!FindNode(&pNode, g_paryKey[i][0], g_paryKey[i][1], NULL))
        {
            Log(ThisModule, -1,
                "FindNode(%s,%s,NULL) Failed",
                g_paryKey[i][0], g_paryKey[i][1] != NULL ? g_paryKey[i][1] : "NULL");
            return -1;
        }
        assert(pNode != NULL);
        ISMConfigNode *pChild = pNode->FirstChildNode("ITEM");
        while (pChild != NULL)
        {
            CCashUnitConfig::CConfigItem *pItem =
            new CCashUnitConfig::CConfigItem(this, pChild);
            m_CUs[i].push_back(pItem);
            pChild = pChild->NextSiblingNode("ITEM");
        }
    }

    //查找并分配公共CConfigItem节点
    if (FindNode(&pNode, "CFG", NULL))
    {
        m_pCommonConfigItem = new CCashUnitConfig::CConfigItem(this, pNode);
    }

    return 0;
}

//保存配置文件
int CCashUnitConfig::Save()
{
    const char *ThisModule = "Save";
    if (m_pConfig == NULL ||
        m_pRootNode == NULL)
    {
        Log(ThisModule, -1, "m_pConfig or m_pRootNode = NULL");
        return -1;
    }
    assert(!m_sFilePath.empty());

    if (!m_pConfig->Save(m_sFilePath.c_str()))
    {
        Log(ThisModule, -1, "m_pConfig->Save(%s) failed: %s",
            m_sFilePath.c_str(), m_pConfig->GetErrDesc());
        return -1;
    }

    return 0;
}

BOOL CCashUnitConfig::BackUp(BOOL bLoadsucc)
{
    string strFile1, strFile2;
    if (bLoadsucc)
    {
        strFile1 = m_sFilePath;
        strFile2 = m_sFileBackupPath;
    }
    else
    {
        strFile1 = m_sFileBackupPath;
        strFile2 = m_sFilePath;
    }
    /*
        CMarkup cmk;
        if (!cmk.Load(strFile1.c_str()))
        {
            Log("BackUp", -1, "文件%s已损坏，备份失败", strFile1.c_str());
            return FALSE;
        }

        if(!CopyFile(strFile1.c_str(),strFile2.c_str(), FALSE))
        {
            Log("BackUp", -1, "~CopyFile(%s)->(%s) failed: %d",
                strFile1.c_str(), strFile2.c_str(), ::GetLastError());
            return FALSE;
        }
        */
    return TRUE;
}

BOOL CCashUnitConfig::FindNode(ISMConfigNode **ppNode, ...)
{
    *ppNode = NULL;

    va_list vl;
    va_start(vl, ppNode);

    ISMConfigNode *pNode = NULL;

    //如果根节点不存在，直接返回FALSE
    if (m_pRootNode == NULL)
        return FALSE;

    //使用可变参数查找对应的节点，直到找不到或者参数为NULL
    pNode = m_pRootNode;
    while (pNode != NULL)
    {
        const char *pKeyName = va_arg(vl, const char *);
        if (pKeyName == NULL)
            break;
        pNode = pNode->FirstChildNode(pKeyName);
    }
    va_end(vl);

    //如果节点为NULL，键名没有找到，返回FALSE
    if (pNode == NULL)
    {
        return FALSE;
    }

    *ppNode = pNode;
    return TRUE;
}

//增加配置项
ICashUnitConfigItem *CCashUnitConfig::AddCUConfig(CONFIG_TYPE ct)
{
    const char *ThisModule = "AddCUConfig";

    assert(ct == CT_LOG_CDM || ct == CT_LOG_CIM || ct == CT_LOG_PH);
    ISMConfigNode *pNode = NULL;
    if (!FindNode(&pNode, g_paryKey[ct][0], g_paryKey[ct][1], NULL))
    {
        Log(ThisModule, -1, "FindNode(%s,%s,NULL) Failed",
            g_paryKey[ct][0], g_paryKey[ct][1] != NULL ? g_paryKey[ct][1] : "NULL");
        return NULL;
    }
    assert(pNode != NULL);
    ISMConfigNode *pNew = pNode->InsertEnd("ITEM");
    if (pNew == NULL)
    {
        Log(ThisModule, -1, "pNode->InsertEnd失败");
        return NULL;
    }

    CConfigItem *pItem = new CConfigItem(this, pNew);
    m_CUs[ct].push_back(pItem);
    return pItem;
}

//删除配置项
void CCashUnitConfig::DeleteCUConfig(CONFIG_TYPE ct, ICashUnitConfigItem *pItem)
{
    assert(ct == CT_LOG_CDM || ct == CT_LOG_CIM || ct == CT_LOG_PH);

    CConfigItem *p = (CConfigItem *)pItem;
    p->GetNode()->Detach();
    p->GetNode()->DeleteClone();
    ITEMS &items = m_CUs[ct];
    for (ITEMS::iterator it = items.begin(); it != items.end(); it++)
    {
        if (*it == p)
        {
            items.erase(it);
            break;
        }
    }
    delete p;
    p = nullptr;
}

