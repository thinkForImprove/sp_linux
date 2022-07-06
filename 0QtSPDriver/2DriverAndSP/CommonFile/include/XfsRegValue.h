#pragma once

#include "QtTypeDef.h"

#ifdef QT_WIN32
#include "RegOperator.h"
#else
#include "INIFileReader.h"
#endif
//////////////////////////////////////////////////////////////////////////
class CXfsRegValue
{
public:
    CXfsRegValue();
    ~CXfsRegValue();
public:
    // 设置逻辑名
    void SetLogicalName(LPCSTR lpLogicalName);
    // 通过逻辑名读取Class类型
    LPCSTR GetSPClass();
    // 转换SP的逻辑名为SP名
    LPCSTR GetSPName();
    // 读取SP配置
    LPCSTR GetValue(LPCSTR lpValueName, LPCSTR lpDefaultValue);
    LPCSTR GetValue(LPCSTR lpSubKeyName, LPCSTR lpValueName, LPCSTR lpDefaultValue);
    DWORD  GetValue(LPCSTR lpValueName, DWORD dwDefaultValue);
    DWORD  GetValue(LPCSTR lpSubKeyName, LPCSTR lpValueName, DWORD dwDefaultValue);
    // 更新SP配置
    BOOL SetValue(LPCSTR lpValueName, LPCSTR lpValue);
    BOOL SetValue(LPCSTR lpSubKeyName, LPCSTR lpValueName, LPCSTR lpValue);
    BOOL SetValue(LPCSTR lpValueName, DWORD dwValue);
    BOOL SetValue(LPCSTR lpSubKeyName, LPCSTR lpValueName, DWORD dwValue);
private:
    bool SwitchSPReg();
    bool SwitchSubReg(LPCSTR lpSubKeyName);
private:
    string          m_strLogicalName;
    string          m_strSPClass;
    string          m_strSPName;
    string          m_strRegPath;
    string          m_strSubPath;
    string          m_strValue;

#ifdef QT_WIN32
    CRegOperator    m_cReg;
#else
    CINIFileReader  m_cINI;
#endif
};
