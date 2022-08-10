#include "RegOperator.h"

#define ATODW(A) ((BYTE)A[3] * 256 * 256 * 256 + (BYTE)A[2] * 256 * 256 + (BYTE)A[1] * 256 + (BYTE)A[0])
#define WATODW(A) \
    (((A[3] & 0xFFFF) * 256 * 256 * 256 * 256 * 256 * 256) + ((A[2] & 0xFFFF) * 256 * 256 * 256 * 256) + ((A[1] & 0xFFFF) * 256 * 256) + (A[0] & 0xFFFF))
//////////////////////////////////////////////////////////////////////////
extern "C" REGDLL_EXPORT long CreateIRegOperator(IRegOperator *&p)
{
    p = new CRegOperator;
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
// m_cSimpleMutex加名称是因为多进程在使用此类
CRegOperator::CRegOperator() : m_cMutex("cReg_Mutex_201907171913")
{
    m_hKEY = nullptr;
    m_hFlushKey = nullptr;
}

CRegOperator::~CRegOperator()
{
    AutoMutex(m_cMutex);
    if (nullptr != m_hKEY)
        ::RegCloseKey(m_hKEY);
    if (nullptr != m_hFlushKey)
        ::RegFlushKey(m_hFlushKey);
}

void CRegOperator::Release() {}
BOOL CRegOperator::SetRegPath(LPCSTR lpKey, LPCSTR lpRegPath, bool bAbsPath /* = false*/)
{
    AutoMutex(m_cMutex);
    if (nullptr != m_hKEY)
    {
        ::RegCloseKey(m_hKEY);
        m_hKEY = nullptr;
    }
    if (nullptr == lpKey || nullptr == lpRegPath)
        return FALSE;

    // 转换
    HKEY hKey = nullptr;
    m_strKey = lpKey;
    m_strKeyPath = lpRegPath;
    if (m_strKey == "HKEY_CLASSES_ROOT")
        hKey = HKEY_CLASSES_ROOT;
    else if (m_strKey == "HKEY_CURRENT_USER")
        hKey = HKEY_CURRENT_USER;
    else if (m_strKey == "HKEY_LOCAL_MACHINE")
        hKey = HKEY_LOCAL_MACHINE;
    else if (m_strKey == "HKEY_USERS")
        hKey = HKEY_USERS;
    else if (m_strKey == "HKEY_CURRENT_CONFIG")
        hKey = HKEY_CURRENT_CONFIG;
    else
        return FALSE;

    BOOL b64System = Is64System();
    if (!bAbsPath && hKey == HKEY_LOCAL_MACHINE && b64System)
    {
        string::size_type nPos = m_strKeyPath.find("WOW6432Node");
        if (nPos == string::npos)
        {
            nPos = m_strKeyPath.find("SOFTWARE");
            if (nPos != string::npos)
            {
                m_strKeyPath.insert(nPos + strlen("SOFTWARE"), "\\WOW6432Node");
            }
        }
    }

    DWORD dwAccessType = b64System ? (KEY_WOW64_64KEY | KEY_ALL_ACCESS) : KEY_ALL_ACCESS;
    m_strWPath = QString::fromLocal8Bit(m_strKeyPath.c_str());
    m_vtSubKeyName.clear();
    m_hFlushKey = hKey;
    long lRet = ::RegOpenKeyEx(hKey, m_strWPath.toStdWString().c_str(), 0, dwAccessType, &m_hKEY);
    if (ERROR_SUCCESS != lRet)  // 子键不存在，则创建
    {
        // x86程序在x64系统，创建注册表时，系统会自己增加"WOW6432Node"
        DWORD dwDisposition = 0;
        lRet = ::RegCreateKeyEx(hKey, m_strWPath.toStdWString().c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, dwAccessType, nullptr, &m_hKEY, &dwDisposition);
        if (ERROR_SUCCESS != lRet)
            return FALSE;
    }
    return TRUE;
}

DWORD CRegOperator::GetRegValueDWORD(LPCSTR lpKeyName, DWORD dwDefault)
{
    AutoMutex(m_cMutex);
    if (nullptr == m_hKEY || nullptr == lpKeyName)
        return dwDefault;

    m_strWKeyName = QString::fromLocal8Bit(lpKeyName);
    ULONG ulType = REG_DWORD;
    wchar_t szbyData[64] = {0};
    DWORD dwSize = sizeof(szbyData);
    if (0 == ::RegQueryValueEx(m_hKEY, m_strWKeyName.toStdWString().c_str(), nullptr, &ulType, (LPBYTE)szbyData, &dwSize))
    {
        DWORD dwVal = WATODW(szbyData);
        return dwVal;
    }
    else
    {
        return dwDefault;
    }
}

BOOL CRegOperator::SetRegValueDWORD(LPCSTR lpKeyName, DWORD dwSetValue)
{
    AutoMutex(m_cMutex);
    if (nullptr == m_hKEY || nullptr == lpKeyName)
        return FALSE;

    m_strWKeyName = QString::fromLocal8Bit(lpKeyName);
    ULONG ulType = REG_DWORD;
    return (ERROR_SUCCESS == ::RegSetValueEx(m_hKEY, m_strWKeyName.toStdWString().c_str(), 0, ulType, (BYTE *)&dwSetValue, sizeof(DWORD)));
}

LPCSTR CRegOperator::GetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpRetString, DWORD dwBuffSize)
{
    AutoMutex(m_cMutex);
    m_strKeyValue = (nullptr == lpDefault) ? "" : lpDefault;
    if (nullptr == m_hKEY || nullptr == lpKeyName || nullptr == lpRetString)
        return m_strKeyValue.c_str();

    m_strWKeyName = QString::fromLocal8Bit(lpKeyName);
    wchar_t szbyData[1024] = {0};
    DWORD dwSize = sizeof(szbyData);
    ULONG ulType = REG_SZ;
    if (0 == ::RegQueryValueEx(m_hKEY, m_strWKeyName.toStdWString().c_str(), nullptr, &ulType, (LPBYTE)szbyData, &dwSize))
    {
        m_strData = QString::fromWCharArray(szbyData);
        dwSize = (DWORD)m_strData.length();
        dwBuffSize = (dwSize > dwBuffSize) ? dwBuffSize : dwSize;

        memcpy(lpRetString, m_strData.toLocal8Bit(), dwBuffSize);
    }
    else
    {
        DWORD dwLen = strlen(lpDefault);
        dwBuffSize = (dwLen > dwBuffSize) ? dwBuffSize : dwLen;
        memcpy(lpRetString, lpDefault, dwBuffSize);
    }
    return lpRetString;
}
LPCSTR CRegOperator::GetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpDefault)
{
    AutoMutex(m_cMutex);
    m_strKeyValue = (nullptr == lpDefault) ? "" : lpDefault;
    if (nullptr == m_hKEY || nullptr == lpKeyName)
        return m_strKeyValue.c_str();

    m_strWKeyName = QString::fromLocal8Bit(lpKeyName);
    wchar_t szbyData[1024] = {0};
    DWORD dwSize = sizeof(szbyData);
    ULONG ulType = REG_SZ;
    if (0 == ::RegQueryValueEx(m_hKEY, m_strWKeyName.toStdWString().c_str(), nullptr, &ulType, (LPBYTE)szbyData, &dwSize))
    {
        m_strData = QString::fromWCharArray(szbyData);
        m_strKeyValue = m_strData.toStdString();
    }
    return m_strKeyValue.c_str();
}
BOOL CRegOperator::SetRegValueSTRING(LPCSTR lpKeyName, LPCSTR lpSetString)
{
    AutoMutex(m_cMutex);
    if (nullptr == m_hKEY || nullptr == lpKeyName || nullptr == lpSetString)
        return FALSE;

    m_strData = QString::fromLocal8Bit(lpSetString);
    m_strWKeyName = QString::fromLocal8Bit(lpKeyName);
    ULONG ulType = REG_SZ;
    wstring strwData = m_strData.toStdWString();
    return (ERROR_SUCCESS == ::RegSetValueEx(m_hKEY, m_strWKeyName.toStdWString().c_str(), 0, ulType, (LPBYTE)strwData.c_str(), strwData.length() + 1));
}

BOOL CRegOperator::GetRegSubKeyName(DWORD uIndex, LPSTR lpRetString, DWORD dwBuffSize)
{
    AutoMutex(m_cMutex);
    if (lpRetString == nullptr)
        return FALSE;
    if (m_vtSubKeyName.empty())
        GetAllRegSubKeyName(m_vtSubKeyName);
    if (m_vtSubKeyName.empty())
        return FALSE;
    if (uIndex >= m_vtSubKeyName.size())
        return FALSE;
    memcpy(lpRetString, m_vtSubKeyName[uIndex].c_str(), qMin(dwBuffSize, (DWORD)m_vtSubKeyName[uIndex].length()));
    return TRUE;
}

void CRegOperator::GetAllRegSubKeyName(vectorString &vtSubKeyName)
{
    AutoMutex(m_cMutex);
    vtSubKeyName.clear();
    if (nullptr == m_hKEY)
        return;

    QString strName;
    for (DWORD i = 0;; i++)
    {
        wchar_t szKeyName[256] = {0};
        DWORD dwBufLen = sizeof(szKeyName);
        long lRet = ::RegEnumValue(m_hKEY, i, szKeyName, &dwBufLen, nullptr, nullptr, nullptr, nullptr);
        if (lRet != ERROR_SUCCESS)
            break;

        strName = QString::fromWCharArray(szKeyName);
        vtSubKeyName.push_back(strName.toStdString());
    }
}
BOOL CRegOperator::DelectRegSubKey(LPCSTR lpKeyName)
{
    AutoMutex(m_cMutex);
    if (nullptr == m_hKEY || nullptr == lpKeyName)
        return FALSE;

    m_strWKeyName = QString::fromLocal8Bit(lpKeyName);
    return (ERROR_SUCCESS == ::RegDeleteValue(m_hKEY, m_strWKeyName.toStdWString().c_str()));
}

BOOL CRegOperator::Is64System()
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
