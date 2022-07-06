#pragma once
#include <qt_windows.h>
#pragma comment(lib, "Advapi32.lib")

#include "IRegOperator.h"
#include "SimpleMutex.h"
#include <vector>
#include <string>
using namespace std;

//////////////////////////////////////////////////////////////////////////
class CRegOperator : public IRegOperator
{
public:
    typedef vector<string> vectorString;

public:
    CRegOperator();
    virtual ~CRegOperator();

public:
    // 释放接口
    virtual void Release();
    // 设置子键路径,参数:1-根键;2-子键路径
    virtual BOOL SetRegPath(LPCSTR lpKey, LPCSTR lpRegPath, bool bAbsPath = false);
    // 返回类型为DWORD的项的值,参数:1-项名;2-默认值
    virtual DWORD GetRegValueDWORD(LPCSTR lpKeyName, DWORD dwDefault);
    // 设置类型为DWORD的项的值,如果设置的项不存在则自动创建,参数:1-项名;2-设置值
    virtual BOOL SetRegValueDWORD(LPCSTR lpKeyName, DWORD dwSetValue);
    // 返回类型为string的项的值,参数:1-项名;2-默认值;3-字符串结果;4-字符串结果缓存大小
    virtual LPCSTR GetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpRetString, DWORD dwBuffSize);
    // 返回类型为string的项的值,参数:1-项名;2-默认值
    virtual LPCSTR GetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpDefault);
    // 设置类型为string的项的值,如果设置的项不存在则自动创建,参数:1-项名;2-设置值
    virtual BOOL SetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpSetString);
    // 按序号获取项中所有子项名称
    virtual BOOL GetRegSubKeyName(DWORD uIndex, LPSTR lpRetString, DWORD dwBuffSize);
    // 删除指定名称的子项键值
    virtual BOOL DelectRegSubKey(LPCSTR lpKeyName);
    // 判断是否为64位系统
    virtual BOOL Is64System();

private:
    // 获取项中所有子项名称
    void GetAllRegSubKeyName(vectorString &vtSubKeyName);

private:
    HKEY m_hKEY;
    HKEY m_hFlushKey;
    CQtSimpleMutexEx m_cMutex;
    vectorString m_vtSubKeyName;

    string m_strKey;
    string m_strKeyValue;
    string m_strKeyPath;
    QString m_strWPath;
    QString m_strWKeyName;
    QString m_strData;
};
