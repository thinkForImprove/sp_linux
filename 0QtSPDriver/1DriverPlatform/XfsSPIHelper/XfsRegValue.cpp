#include "XfsRegValue.h"

//////////////////////////////////XFS////////////////////////////////////////
//registry key name
#define KEY_NAME_LOG_PROVIDER       ".DEFAULT\\XFS\\LOGICAL_SERVICES"
#define KEY_NAME_SERVICE_PROVIDER   "SOFTWARE\\XFS\\SERVICE_PROVIDERS"
#define KEY_NAME_CFES_XFS           "SOFTWARE\\CFES\\XFS"
#define KEY_NAME_XFS_MANAGER        "XFS_MANAGER"
//registry value name
#define VALUE_NAME_PROVIDER         "Provider"
#define VALUE_NAME_CLASS            "class"

///////////////////////////////////LFS///////////////////////////////////////
#define LFS_NAME_LOG_PROVIDER       "/etc/xfs/logical_services/"
#define LFS_NAME_SERVICE_PROVIDER   "/etc/xfs/service_providers/"
//////////////////////////////////////////////////////////////////////////
CXfsRegValue::CXfsRegValue()
{
}


CXfsRegValue::~CXfsRegValue()
{
}

void CXfsRegValue::SetLogicalName(LPCSTR lpLogicalName)
{
    m_strLogicalName = lpLogicalName;
#ifdef QT_WIN32
    char szPath[MAX_PATH] = { 0 };
    sprintf(szPath, "%s\\%s", KEY_NAME_LOG_PROVIDER, lpLogicalName);
    m_cReg.SetRegPath("HKEY_USERS", szPath);
    m_strRegPath = szPath;
    m_strSPName = m_cReg.GetRegValueSTRING(VALUE_NAME_PROVIDER, "");
#else
    QString strFile(lpLogicalName);
    strFile.prepend(LFS_NAME_LOG_PROVIDER);
    strFile.append(".ini");
    if (strFile.toStdString() != m_cINI.GetINIFile())
    {
        m_cINI.LoadINIFile(strFile.toStdString());
    }
    CINIReader cINI = m_cINI.GetReaderSection("default");
    m_strSPName = (LPCSTR)cINI["provider"];
#endif
}

LPCSTR CXfsRegValue::GetSPClass()
{
    m_strSPClass = "";
#ifdef QT_WIN32
    char szPath[MAX_PATH] = { 0 };
    sprintf(szPath, "%s\\%s", KEY_NAME_LOG_PROVIDER, m_strLogicalName.c_str());
    m_cReg.SetRegPath("HKEY_USERS", szPath);
    m_strRegPath = szPath;
    m_strSPClass = m_cReg.GetRegValueSTRING(VALUE_NAME_CLASS, "");
    return m_strSPClass.c_str();
#else
    QString strFile(m_strLogicalName.c_str());
    strFile.prepend(LFS_NAME_LOG_PROVIDER);
    strFile.append(".ini");
    if (strFile.toStdString() != m_cINI.GetINIFile())
    {
        m_cINI.LoadINIFile(strFile.toStdString());
    }

    CINIReader cINI = m_cINI.GetReaderSection("default");
    m_strSPClass = (LPCSTR)cINI["class"];
    return m_strSPClass.c_str();
#endif
}

LPCSTR CXfsRegValue::GetSPName()
{
    return m_strSPName.c_str();
}

LPCSTR CXfsRegValue::GetValue(LPCSTR lpValueName, LPCSTR lpDefaultValue)
{
    SwitchSPReg();
#ifdef QT_WIN32
    return m_cReg.GetRegValueSTRING(lpValueName, lpDefaultValue);
#else
    return m_cINI.GetValue("default", lpValueName, lpDefaultValue);
#endif
}

LPCSTR CXfsRegValue::GetValue(LPCSTR lpSubKeyName, LPCSTR lpValueName, LPCSTR lpDefaultValue)
{
    SwitchSubReg(lpSubKeyName);
#ifdef QT_WIN32
    return m_cReg.GetRegValueSTRING(lpValueName, lpDefaultValue);
#else
    CINIReader cINI = m_cINI.GetReaderSection(lpSubKeyName);
    m_strValue = (LPCSTR)cINI[lpValueName];
    if (m_strValue.empty())
        m_strValue = lpDefaultValue;
    return m_strValue.c_str();
#endif
}

DWORD CXfsRegValue::GetValue(LPCSTR lpValueName, DWORD dwDefaultValue)
{
    SwitchSPReg();
#ifdef QT_WIN32
    return m_cReg.GetRegValueDWORD(lpValueName, dwDefaultValue);
#else
    QString strDefVal;
    strDefVal.sprintf("%u", dwDefaultValue);
    strDefVal = m_cINI.GetValue("default", lpValueName, strDefVal.toLocal8Bit().constData());
    return strDefVal.toULong();
#endif
}

DWORD CXfsRegValue::GetValue(LPCSTR lpSubKeyName, LPCSTR lpValueName, DWORD dwDefaultValue)
{
    SwitchSubReg(lpSubKeyName);
#ifdef QT_WIN32
    DWORD dwVal = m_cReg.GetRegValueDWORD(lpValueName, dwDefaultValue);
    return dwVal;
#else
    QString strDefVal;
    CINIReader cINI = m_cINI.GetReaderSection(lpSubKeyName);
    strDefVal = (LPCSTR)cINI[lpValueName];
    if (strDefVal.isEmpty())
        strDefVal.sprintf("%u", dwDefaultValue);
    return strDefVal.toULong();
#endif
}

BOOL CXfsRegValue::SetValue(LPCSTR lpValueName, LPCSTR lpValue)
{
    SwitchSPReg();
#ifdef QT_WIN32
    return m_cReg.SetRegValueSTRING(lpValueName, lpValue);
#else
    CINIWriter cINI = m_cINI.GetWriterSection("default");
    cINI.AddValue(lpValueName, lpValue);
    return cINI.Save() ? TRUE : FALSE;
#endif
}

BOOL CXfsRegValue::SetValue(LPCSTR lpSubKeyName, LPCSTR lpValueName, LPCSTR lpValue)
{
    SwitchSubReg(lpSubKeyName);
#ifdef QT_WIN32
    return m_cReg.SetRegValueSTRING(lpValueName, lpValue);
#else
    CINIWriter cINI = m_cINI.GetWriterSection(lpSubKeyName);
    cINI.AddValue(lpValueName, lpValue);
    return cINI.Save() ? TRUE : FALSE;
#endif
}

BOOL CXfsRegValue::SetValue(LPCSTR lpValueName, DWORD dwValue)
{
    SwitchSPReg();
#ifdef QT_WIN32
    return m_cReg.SetRegValueDWORD(lpValueName, dwValue);
#else
    QString qVal;
    qVal.sprintf("%u", dwValue);
    CINIWriter cINI = m_cINI.GetWriterSection("default");
    cINI.AddValue(lpValueName, qVal.toStdString().c_str());
    return cINI.Save() ? TRUE : FALSE;
#endif
}

BOOL CXfsRegValue::SetValue(LPCSTR lpSubKeyName, LPCSTR lpValueName, DWORD dwValue)
{
    SwitchSubReg(lpSubKeyName);
#ifdef QT_WIN32
    return m_cReg.SetRegValueDWORD(lpValueName, dwValue);
#else
    QString qVal;
    qVal.sprintf("%u", dwValue);
    CINIWriter cINI = m_cINI.GetWriterSection(lpSubKeyName);
    cINI.AddValue(lpValueName, qVal.toStdString().c_str());
    return cINI.Save() ? TRUE : FALSE;
#endif
}

bool CXfsRegValue::SwitchSPReg()
{
#ifdef QT_WIN32
    string strPath = KEY_NAME_SERVICE_PROVIDER + string("\\") + m_strSPName;
    if (m_strRegPath != strPath)
    {
        m_strSubPath = "";
        m_strRegPath = strPath;
        m_cReg.SetRegPath("HKEY_LOCAL_MACHINE", m_strRegPath.c_str());
    }
#else
    QString strFile(m_strSPName.c_str());
    strFile.prepend(LFS_NAME_SERVICE_PROVIDER);
    strFile.append(".ini");
    if (strFile.toStdString() != m_cINI.GetINIFile())
    {
        m_cINI.LoadINIFile(strFile.toStdString());
    }
#endif
    return true;
}

bool CXfsRegValue::SwitchSubReg(LPCSTR lpSubKeyName)
{
    if (lpSubKeyName == nullptr)
        return false;

#ifdef QT_WIN32
    string strPath = KEY_NAME_SERVICE_PROVIDER + string("\\") + m_strSPName + string("\\") + lpSubKeyName;
    if (m_strRegPath != strPath || m_strSubPath != lpSubKeyName)
    {
        m_strSubPath = lpSubKeyName;
        m_strRegPath = strPath;
        m_cReg.SetRegPath("HKEY_LOCAL_MACHINE", m_strRegPath.c_str());
        return true;
    }
#else
    QString strFile(m_strSPName.c_str());
    strFile.prepend(LFS_NAME_SERVICE_PROVIDER);
    strFile.append(".ini");
    if (strFile.toStdString() != m_cINI.GetINIFile())
    {
        m_cINI.LoadINIFile(strFile.toStdString());
    }
#endif
    return true;
}
