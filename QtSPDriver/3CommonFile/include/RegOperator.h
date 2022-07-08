#pragma once

//////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

#include "QtTypeDef.h"
#include "IRegOperator.h"
#include "QtDLLLoader.h"
//////////////////////////////////////////////////////////////////////////
class CRegOperator
{
public:
    typedef vector<std::string> vectorString;
public:
    CRegOperator() {}
    CRegOperator(LPCSTR lpKey, LPCSTR lpRegPath) { SetRegPath(lpKey, lpRegPath); }
    virtual ~CRegOperator() {}

public:
    // 设置子键路径,参数:1-根键;2-子键路径
    BOOL   SetRegPath(LPCSTR lpKey, LPCSTR lpRegPath, bool bAbsPath = false)
    {
        if (!m_pReg)
            m_pReg.Load("RegOperator.dll", "CreateIRegOperator");
        if (m_pReg == nullptr)
            return FALSE;
        return m_pReg->SetRegPath(lpKey, lpRegPath, bAbsPath);
    }
    // 返回类型为DWORD的项的值,参数:1-项名;2-默认值
    DWORD  GetRegValueDWORD(LPCSTR lpKeyName, DWORD dwDefault)
    {
        if (m_pReg == nullptr)
            return dwDefault;
        return m_pReg->GetRegValueDWORD(lpKeyName, dwDefault);
    }
    // 设置类型为DWORD的项的值,如果设置的项不存在则自动创建,参数:1-项名;2-设置值
    BOOL   SetRegValueDWORD(LPCSTR lpKeyName, DWORD dwSetValue)
    {
        if (m_pReg == nullptr)
            return FALSE;
        return m_pReg->SetRegValueDWORD(lpKeyName, dwSetValue);
    }
    // 返回类型为string的项的值,参数:1-项名;2-默认值;3-字符串结果;4-字符串结果缓存大小
    LPCSTR GetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpRetString, DWORD dwBuffSize)
    {
        if (m_pReg == nullptr)
            return lpDefault;
        return m_pReg->GetRegValueSTRING(lpKeyName, lpDefault, lpRetString, dwBuffSize);
    }
    // 返回类型为string的项的值,参数:1-项名;2-默认值
    LPCSTR GetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpDefault)
    {
        if (m_pReg == nullptr)
            return lpDefault;
        return m_pReg->GetRegValueSTRING(lpKeyName, lpDefault);
    }
    // 设置类型为string的项的值,如果设置的项不存在则自动创建,参数:1-项名;2-设置值
    BOOL   SetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpSetString)
    {
        if (m_pReg == nullptr)
            return FALSE;
        return m_pReg->SetRegValueSTRING(lpKeyName, lpSetString);
    }
    // 按序号获取项中所有子项名称
    BOOL   GetRegAllSubKeyName(vectorString &vtSubKeyName)
    {
        vtSubKeyName.clear();
        if (m_pReg == nullptr)
            return FALSE;

        BOOL bRet = FALSE;
        char szBuff[512] = { 0 };
        for (DWORD i = 0; ; i++)
        {
            memset(szBuff, 0x00, sizeof(szBuff));
            bRet = m_pReg->GetRegSubKeyName(i, szBuff, sizeof(szBuff));
            if (!bRet)
                break;
            vtSubKeyName.push_back(szBuff);
        }
        return bRet;
    }
    // 删除指定名称的子项键值
    BOOL   DelectRegSubKey(LPCSTR lpKeyName)
    {
        if (m_pReg == nullptr)
            return FALSE;
        return m_pReg->DelectRegSubKey(lpKeyName);
    }
    // 判断是否为64位系统
    BOOL   Is64System()
    {
        if (m_pReg == nullptr)
            return FALSE;
        return m_pReg->Is64System();
    }
private:
    CQtDLLLoader<IRegOperator>  m_pReg;
};
