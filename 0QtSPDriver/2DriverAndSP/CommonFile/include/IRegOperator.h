#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"

//////////////////////////////////////////////////////////////////////////
#if defined(REGOPERATOR_LIBRARY)
    #define REGDLL_EXPORT Q_DECL_EXPORT
#else
    #define REGDLL_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
struct IRegOperator
{
    // 释放接口
    virtual void Release() = 0;
    // 设置子键路径,参数:1-根键 = 0;2-子键路径
    virtual BOOL SetRegPath(LPCSTR lpKey, LPCSTR lpRegPath, bool bAbsPath = false) = 0;
    // 返回类型为DWORD的项的值,参数:1-项名 = 0;2-默认值
    virtual DWORD GetRegValueDWORD(LPCSTR lpKeyName, DWORD dwDefault) = 0;
    // 设置类型为DWORD的项的值,如果设置的项不存在则自动创建,参数:1-项名 = 0;2-设置值
    virtual BOOL SetRegValueDWORD(LPCSTR lpKeyName, DWORD dwSetValue) = 0;
    // 返回类型为string的项的值,参数:1-项名 = 0;2-默认值 = 0;3-字符串结果 = 0;4-字符串结果缓存大小
    virtual LPCSTR GetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpRetString, DWORD dwBuffSize) = 0;
    // 返回类型为string的项的值,参数:1-项名 = 0;2-默认值
    virtual LPCSTR GetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpDefault) = 0;
    // 设置类型为string的项的值,如果设置的项不存在则自动创建,参数:1-项名 = 0;2-设置值
    virtual BOOL SetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpSetString) = 0;
    // 按序号获取项中所有子项名称
    virtual BOOL GetRegSubKeyName(DWORD uIndex, LPSTR lpRetString, DWORD dwBuffSize) = 0;
    // 删除指定名称的子项键值
    virtual BOOL DelectRegSubKey(LPCSTR lpKeyName) = 0;
    // 判断是否为64位系统
    virtual BOOL Is64System() = 0;
};

extern "C" REGDLL_EXPORT long CreateIRegOperator(IRegOperator *&p);
//////////////////////////////////////////////////////////////////////////
