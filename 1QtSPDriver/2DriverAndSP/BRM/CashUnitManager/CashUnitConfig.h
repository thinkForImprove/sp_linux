#pragma once
#include "ISMConfig.h"
#include "ICashUnitConfig.h"
#include "ILogWrite.h"
//#include <stdlib.h>
//#include <stdio.h>
#include <assert.h>
#include <vector>
using namespace std;

inline int Str2Int(const char *pValue, int iDefault)
{
    int nBase = 10;
    if (pValue[0] == '0' && (pValue[1] == 'x' || pValue[1] == 'X'))
    {
        nBase = 16;
    }

    //转换为整数
    char *p = NULL;
    int iValue = strtol(pValue, &p, nBase);
    if (p == NULL || p == pValue || *p != 0)
        return iDefault;

    return iValue;
}

class CCashUnitConfig : public ICashUnitConfig, public CLogManage
{
    class CConfigItem : public ICashUnitConfigItem
    {
    public:
        CConfigItem(CCashUnitConfig *pConfig, ISMConfigNode *pNode)
        {
            m_pConfig = pConfig;
            m_pNode = pNode;
        }

        //得到配置文件对象
        virtual ICashUnitConfig *GetConfig()
        {
            return m_pConfig;
        }

        //get value
        virtual const char *GetCUValue(const char *pDefault, const char *pValueName) const
        {
            assert(m_pNode != NULL);
            ISMConfigNode *pNode = m_pNode->FirstChildNode(pValueName);
            if (pNode == NULL)
            {
                return pDefault;
            }
            const char *pRet = pNode->GetContent();
            if (pRet == NULL)
                return pDefault;
            return pRet;
        }

        virtual int GetCUValue(int nDefault, const char *pValueName) const
        {
            const char *pValue = GetCUValue((const char *)NULL, pValueName);
            if (pValue == NULL)
            {
                return nDefault;
            }
            return Str2Int(pValue, nDefault);
        }

        //set value
        //0 success; <0 failure
        virtual int SetCUValue(const char *pValue, const char *pValueName)
        {
            assert(m_pNode != NULL);
            ISMConfigNode *pNode = m_pNode->FirstChildNode(pValueName);
            if (pNode == NULL)
            {
                pNode = m_pNode->InsertEnd(pValueName);
                if (pNode == NULL)
                {
                    return -1;
                }
            }
            BOOL bRet = pNode->SetContent(pValue != NULL ? pValue : "");
            if (!bRet)
            {
                return -2;
            }
            return 0;
        }

        virtual int SetCUValue(int nValue, const char *pValueName)
        {
            char szBuf[20];
            //sprintf_s(szBuf, "%d", nValue);
            sprintf(szBuf, "%d", nValue);
            return SetCUValue(szBuf, pValueName);
        }

        ISMConfigNode *GetNode() const
        {
            return m_pNode;
        }

    private:
        CCashUnitConfig *m_pConfig;     //配置文件对象
        ISMConfigNode *m_pNode;         //配置节点
    };
    typedef vector<CConfigItem *> ITEMS;
    typedef ITEMS::iterator ITEMSIT;

public:
    CCashUnitConfig();
    virtual ~CCashUnitConfig();
    void Clear();

    //实现ICashUnitConfig
    //装载配置文件
    int Load(const char *pFileName);

    //保存配置文件
    int Save();

    //备份配置文件
    BOOL BackUp(BOOL bLoadsucc);

    //得到第dwIndex个钞箱配置，不存在返回NULL
    //ct, 配置类型
    virtual ICashUnitConfigItem *GetCUConfig(CONFIG_TYPE ct, DWORD dwIndex)
    {
        assert(ct == CT_LOG_CDM || ct == CT_LOG_CIM || ct == CT_LOG_PH);
        assert(dwIndex < m_CUs[ct].size());
        return m_CUs[ct][dwIndex];
    }

    //得到钞箱的配置个数
    virtual DWORD GetCUConfigCount(CONFIG_TYPE ct) const
    {
        assert(ct == CT_LOG_CDM || ct == CT_LOG_CIM || ct == CT_LOG_PH);
        return m_CUs[ct].size();
    }

    //增加配置项
    virtual ICashUnitConfigItem *AddCUConfig(CONFIG_TYPE ct);

    //删除配置项
    virtual void DeleteCUConfig(CONFIG_TYPE ct, ICashUnitConfigItem *pItem);

    //得到公共的钞箱配置信息，不存在返回NULL
    virtual ICashUnitConfigItem *GetCommonConfig()
    {
        return m_pCommonConfigItem;
    }
    //内部成员函数
private:
    //查找节点
    //ppNode：返回找到节点，如未找到，返回NULL
    //..., 键名，可为多级，最后必须以NULL结束
    BOOL FindNode(ISMConfigNode **ppNode, ...);

    //成员变量
private:
    ISMConfig *m_pConfig;       //配置文件
    ISMConfigNode *m_pRootNode; //根节点
    ITEMS m_CUs[3];
    std::string     m_sFilePath;    //装入数据的文件路径
    std::string     m_sFileBackupPath;  //装入数据的备份文件路径
    CConfigItem *m_pCommonConfigItem; //公共配置信息
};


