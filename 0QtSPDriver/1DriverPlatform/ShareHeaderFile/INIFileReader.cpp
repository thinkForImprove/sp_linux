#include "INIFileReader.h"
#include <sys/stat.h>
#include <QDir>
#include <fstream>
//////////////////////////////////////////////////////////////////////////
CINIReader::AllINIType::AllINIType()
{
    Clear();
}

CINIReader::AllINIType::~AllINIType()
{
}

CINIReader::AllINIType::operator bool() const
{
    return (atol(m_szValue) == 0) ? false : true;
}
CINIReader::AllINIType::operator char() const
{
    return (char)atol(m_szValue);
}
CINIReader::AllINIType::operator short() const
{
    ULONG ulValue = 0;
    if(strncasecmp(m_szValue, "0x", 2) == 0){
        sscanf(m_szValue, "%x", &ulValue);
    } else {
        ulValue = atol(m_szValue);
    }
    return (short)ulValue;
}
CINIReader::AllINIType::operator int() const
{
//    return (int)atol(m_szValue);
    ULONG ulValue = 0;
    if(strncasecmp(m_szValue, "0x", 2) == 0){
        sscanf(m_szValue, "%x", &ulValue);
    } else {
        ulValue = atol(m_szValue);
    }
    return (int)ulValue;
}
CINIReader::AllINIType::operator long() const
{
//    return (long)atol(m_szValue);
    ULONG ulValue = 0;
    if(strncasecmp(m_szValue, "0x", 2) == 0){
        sscanf(m_szValue, "%x", &ulValue);
    } else {
        ulValue = atol(m_szValue);
    }
    return (long)ulValue;
}
CINIReader::AllINIType::operator BYTE() const
{
    return (BYTE)atol(m_szValue);
}
CINIReader::AllINIType::operator WORD() const
{
//    return (WORD)atol(m_szValue);
    ULONG ulValue = 0;
    if(strncasecmp(m_szValue, "0x", 2) == 0){
        sscanf(m_szValue, "%x", &ulValue);
    } else {
        ulValue = atol(m_szValue);
    }
    return (WORD)ulValue;
}
CINIReader::AllINIType::operator UINT() const
{
//    return (UINT)atol(m_szValue);
    ULONG ulValue = 0;
    if(strncasecmp(m_szValue, "0x", 2) == 0){
        sscanf(m_szValue, "%x", &ulValue);
    } else {
        ulValue = atol(m_szValue);
    }
    return (UINT)ulValue;
}
CINIReader::AllINIType::operator ULONG() const
{
//    return (ULONG)stoul(m_szValue);
    ULONG ulValue = 0;
    if(strncasecmp(m_szValue, "0x", 2) == 0){
        sscanf(m_szValue, "%x", &ulValue);
    } else {
        ulValue = stoul(m_szValue);
    }
    return ulValue;
}
CINIReader::AllINIType::operator LPCSTR() const
{
    return m_szValue;
}

void CINIReader::AllINIType::Clear()
{
    memset(m_szValue, 0x00, sizeof(m_szValue));
}

void CINIReader::AllINIType::SetValue(LPCSTR lpVal)
{
    strcpy(m_szValue, lpVal);
}

//////////////////////////////////////////////////////////////////////////
CINIReader::CINIReader()
{

}

CINIReader::~CINIReader()
{

}

CINIReader::AllINIType &CINIReader::operator[](string strName)
{
    m_cINIType.Clear();
    auto it = m_mapString.find(strName);
    if (it != m_mapString.end() && !it->second.empty())
        m_cINIType.SetValue(it->second.c_str());
    return m_cINIType;
}

CINIReader::AllINIType &CINIReader::GetValue(string strName, string strDefVal)
{
    m_cINIType.Clear();
    auto it = m_mapString.find(strName);
    if (it != m_mapString.end() && !it->second.empty())
        m_cINIType.SetValue(it->second.c_str());
    else
        m_cINIType.SetValue(strDefVal.c_str());
    return m_cINIType;
}

CINIReader::AllINIType &CINIReader::GetValue(string strName, DWORD dwDefVal)
{
    char szVal[64] = {0};
#ifdef QT_WIN32
    sprintf(szVal, "%lu", dwDefVal);
#else
    sprintf(szVal, "%u", dwDefVal);
#endif
    return GetValue(strName, string(szVal));
}


LPCSTR CINIReader::GetSectionName()
{
    return m_strSectionName.c_str();
}

UINT CINIReader::Count()
{
    return m_mapString.size();
}

LPCSTR CINIReader::GetName(UINT uIndex)
{
    if (uIndex >= m_vtName.size() || m_vtValue[uIndex].empty())
        return m_szNull;
    return m_vtName[uIndex].c_str();
}

LPCSTR CINIReader::GetValue(UINT uIndex)
{
    if (uIndex >= m_vtValue.size() || m_vtValue[uIndex].empty())
        return m_szNull;
    return m_vtValue[uIndex].c_str();
}


//////////////////////////////////////////////////////////////////////////
CINIFileReader::CINIFileReader()
{
    memset(m_szNull, 0x00, sizeof(m_szNull));
    memset(m_szEnvPath, 0x00, sizeof(m_szEnvPath));
}


CINIFileReader::CINIFileReader(string strINIFile)
{
    memset(m_szNull, 0x00, sizeof(m_szNull));
    memset(m_szEnvPath, 0x00, sizeof(m_szEnvPath));
    LoadINIFile(strINIFile);
}

CINIFileReader::~CINIFileReader()
{
}

LPCSTR CINIFileReader::GetEnvPath(string strEnvName)
{
    memset(m_szEnvPath, 0x00, sizeof(m_szEnvPath));
    char *pEnv = getenv(strEnvName.c_str());
    if (pEnv != nullptr)
        strcpy(m_szEnvPath, pEnv);
    return m_szEnvPath;
}

LPCSTR CINIFileReader::GetConfigFile(string strEnvName, string strFile)
{
    // 先读取当前目录的，如果没有配置，则读取环境变量配置
    char szPath[MAX_PATH] = { 0 };
    QString qstrDir = QDir::currentPath();
    QByteArray byDir = qstrDir.toLocal8Bit();
    strcpy(szPath, byDir.constData());
    if (strlen(szPath) != 0)
    {
        memset(m_szEnvPath, 0x00, sizeof(m_szEnvPath));
        sprintf(m_szEnvPath, "%s\\%s", szPath, strFile.c_str());
        if (QFile::exists(m_szEnvPath))     // 判断文件是否存在
            return m_szEnvPath;
    }

    char *pEnv = getenv(strEnvName.c_str());
    if (pEnv != nullptr)
    {
        memset(m_szEnvPath, 0x00, sizeof(m_szEnvPath));
        sprintf(m_szEnvPath, "%s\\%s", pEnv, strFile.c_str());
        if (QFile::exists(m_szEnvPath))     // 判断文件是否存在
            return m_szEnvPath;
    }

    return m_szNull;
}

bool CINIFileReader::LoadINIFile(string strName)
{
    std::lock_guard<recursive_mutex> _auto_lock(m_cMutex);

    m_vtINIFile.clear();
    if (strName.empty())
        return false;
    m_strINIFile = strName;

//    fstream cFile(strName);
    fstream cFile(strName, ios::in);
    if (!cFile.is_open())
        return false;

    string strLine;
    string strSectionName;
    INIFILEINFO stInfo;
    while (!cFile.eof())
    {
        do
        {
            char szLine[512] = { 0 };
            cFile.getline(szLine, sizeof(szLine));     // 没有换行符
            strLine = szLine;
            TrimLeftRight(strLine);

            if (strLine[0] == '\0') break;   // 空行
            if (strLine[0] == ';')  break;   // 注释
            if (strLine[0] == '/')  break;   // 注释
            if (strLine[0] == '[')          // 节点
            {
                TrimLeftRight(strLine, "[]");
                strSectionName = strLine;
                break;
            }
            if (strSectionName.empty())
                break;// 没有节点的内容

            // Key和值分析
            string::size_type nPos = strLine.find('=');
            if (string::npos != nPos)
            {
                string strLeft = strLine.substr(0, nPos);
                TrimLeftRight(strLeft);
                if (!strLeft.empty())   // KEY
                {
                    // 值
                    string strRight = strLine.substr(nPos + 1, strLine.length() - nPos - 1);
                    TrimLeftRight(strRight);

                    UINT lSize = sizeof(stInfo.szSectionName) - 1;
                    UINT lLen = strSectionName.length() > lSize ? lSize : strSectionName.length();
                    memcpy(stInfo.szSectionName, strSectionName.c_str(), lLen);

                    lSize = sizeof(stInfo.szKeyName) - 1;
                    lLen = strLeft.length() > lSize ? lSize : strLeft.length();
                    memcpy(stInfo.szKeyName, strLeft.c_str(), lLen);

                    lSize = sizeof(stInfo.szKeyValue) - 1;
                    lLen = strRight.length() > lSize ? lSize : strRight.length();
                    memcpy(stInfo.szKeyValue, strRight.c_str(), lLen);

                    m_vtINIFile.push_back(stInfo);
                    stInfo.Clear();
                }
            }
        } while (false);
    }
    cFile.close();

    // 保存文件状态信息
    GetFileStat(strName, m_clastModified);
    return true;
}

string CINIFileReader::GetINIFile()
{
    return m_strINIFile;
}

UINT CINIFileReader::Count()
{
    return m_vtINIFile.size();
}

bool CINIFileReader::GetInfo(UINT uIndex, INIFILEINFO &stInfo)
{
    if (m_vtINIFile.empty())
        return false;
    if (uIndex >= m_vtINIFile.size())
        return false;
    memcpy(&stInfo, &m_vtINIFile[uIndex], sizeof(INIFILEINFO));
    return true;
}

bool CINIFileReader::GetAllKey(vectorINIFILEINFO &vtKey)
{
    if (m_vtINIFile.empty())
        return false;

    vtKey.clear();
    vtKey.insert(vtKey.begin(), m_vtINIFile.begin(), m_vtINIFile.end());
    return true;
}

CINIReader CINIFileReader::GetReaderSection(string strSectionName)
{
    GetKeyBySection(strSectionName, &m_cINIReader);
    return m_cINIReader;
}

CINIWriter CINIFileReader::GetWriterSection(string strSectionName)
{
    GetKeyBySection(strSectionName, &m_cINIWriter);
    return m_cINIWriter;
}

LPCSTR CINIFileReader::GetValue(string strSectionName, string strKeyName)
{
    for (auto &it : m_vtINIFile)
    {
        if (strSectionName == it.szSectionName &&
            strKeyName == it.szKeyName)
        {
            return it.szKeyValue;
        }
    }
    return m_szNull;
}

LPCSTR CINIFileReader::GetValue(string strSectionName, string strKeyName, string strDefVal)
{
    for (auto &it : m_vtINIFile)
    {
        if (strSectionName == it.szSectionName &&
            strKeyName == it.szKeyName)
        {
            return it.szKeyValue;
        }
    }

    if (strDefVal.empty())
        return m_szNull;
    return strDefVal.c_str();
}

bool CINIFileReader::SetNewSection(string strSectionName)
{
    std::lock_guard<recursive_mutex> _auto_lock(m_cMutex);

    if (m_strINIFile.empty())
        return false;

    //打开文件，读取并判断是否已存在
    fstream cFileR(m_strINIFile);
    if (cFileR.is_open())
    {
        string strLine;
        char szLine[2048] = { 0 };
        while (!cFileR.eof())
        {
            memset(szLine, 0x00, sizeof(szLine));
            cFileR.getline(szLine, sizeof(szLine));     // 没有换行符
            strLine = szLine;
            TrimLeftRight(strLine);
            if (strLine[0] == '[')          // 节点
            {
                TrimLeftRight(strLine, "[]");
                TrimLeftRight(strLine, " ");
                if (strLine == strSectionName)   // 相同节点，直接返回
                {
                    cFileR.close();
                    return true;
                }
            }
        }
        cFileR.close();
    }

    //打开文件，在最后写入数据
    fstream cFileW(m_strINIFile, ios::app);
    if (!cFileW.is_open())
        return false;

    cFileW << "\n[" << strSectionName << "]"; //写入数据
    cFileW.close();
    return true;
}

//////////////////////////////////////////////////////////////////////////
void CINIFileReader::GetKeyBySection(string strSectionName, CINIKeyVal *pINIKeyVal)
{
    // 判断文件是否已被更新
    if (IsNeedUpdateINI()) LoadINIFile(m_strINIFile);

    pINIKeyVal->m_mapString.clear();
    for (auto &it : m_vtINIFile)
    {
        if (strSectionName == it.szSectionName)
        {
            pINIKeyVal->m_mapString[it.szKeyName] = it.szKeyValue;
        }
    }

    pINIKeyVal->m_strFileName    = m_strINIFile;
    pINIKeyVal->m_strSectionName = strSectionName;
    pINIKeyVal->m_pINIFileReader = this;
    pINIKeyVal->UpdateKeyNameValue();
    return ;
}

void CINIFileReader::TrimLeftRight(string &strBuff, string strStep /*= " \t\r\n"*/)
{
    string::size_type pos = strBuff.find_last_not_of(strStep);   // 删除后面
    if (pos != string::npos)
    {
        strBuff.erase(pos + 1);
    }

    pos = strBuff.find_first_not_of(strStep);   // 删除前面
    if (pos != string::npos)
    {
        strBuff.erase(0, pos);
    }
    return;
}

bool CINIFileReader::IsNeedUpdateINI()
{
    // 通过比较文件修改时间
    QDateTime clastModified;
    QFileInfo qFile(m_strINIFile.c_str());
    clastModified = qFile.lastModified();
    return (m_clastModified != clastModified);
}

void CINIFileReader::GetFileStat(string strFile, QDateTime &clastModified)
{
    QFileInfo qFile(strFile.c_str());
    clastModified = qFile.lastModified();
}

//////////////////////////////////////////////////////////////////////////
CINIKeyVal::CINIKeyVal(): m_pINIFileReader(nullptr)
{
    memset(m_szNull, 0x00, sizeof(m_szNull));
}

CINIKeyVal::~CINIKeyVal()
{

}

void CINIKeyVal::UpdateKeyNameValue()
{
    m_vtName.clear();
    m_vtValue.clear();
    for (auto &it : m_mapString)
    {
        m_vtName.push_back(it.first);
        m_vtValue.push_back(it.second);
    }
}

CINIWriter::CINIWriter(): m_bSaved(true)
{

}

CINIWriter::~CINIWriter()
{
    Save();
}

void CINIWriter::SetValue(string strName, string strVal)
{
    auto it = m_mapString.find(strName);
    if (it != m_mapString.end())
    {
        m_bSaved = false;
        m_mapSetKey[strName] = strVal;
    }
    return;
}

void CINIWriter::SetValue(string strName, DWORD dwVal)
{
    char szVal[64] = {0};
#ifdef QT_WIN32
    sprintf(szVal, "%lu", dwVal);
#else
    sprintf(szVal, "%u", dwVal);
#endif
    SetValue(strName, string(szVal));
    return;
}

void CINIWriter::AddValue(string strName, string strVal)
{
    m_bSaved = false;
    auto it = m_mapString.find(strName);
    if (it != m_mapString.end())
    {
        m_mapSetKey[strName] = strVal;
    }
    else
    {
        m_mapAddKey[strName] = strVal;
    }
    return;
}

void CINIWriter::AddValue(string strName, DWORD dwVal)
{
    char szVal[64] = {0};
#ifdef QT_WIN32
    sprintf(szVal, "%lu", dwVal);
#else
    sprintf(szVal, "%u", dwVal);
#endif
    AddValue(strName, string(szVal));
    return;
}

void CINIWriter::DeleteKey(string strName)
{
    auto it = m_mapString.find(strName);
    if (it != m_mapString.end())
    {
        m_bSaved = false;
        m_mapDelKey[strName] = it->second;
    }
    return;
}

void CINIWriter::DeleteKey()
{
    m_bSaved    = false;
    m_mapDelKey = m_mapString;
    return;
}

bool CINIWriter::Save(bool bBackup/* = false*/)
{
    // 防止多次保存
    if (m_bSaved)
        return true;

    // 加载INI文件
    if (!LoadFile(m_strFileName, m_vtFile))
        return false;

    // 分析文件
    string strLine;
    string strKeyName;
    bool bAddKey = false;
    bool bFindSection = false;
    vectorString vtNewFile;
    for (auto &itF : m_vtFile)
    {
        if (!bFindSection && string::npos != itF.find(m_strSectionName))
        {
            bFindSection = true;// 找到对应节点
            vtNewFile.push_back(itF);
            continue;
        }
        if (!bFindSection)
        {
            vtNewFile.push_back(itF);
            continue;
        }
        if (itF[0] == ';' || itF[0] == '/' || itF[0] == '\0')   // 注释或空行
        {
            vtNewFile.push_back(itF);
            continue;
        }
        if (itF[0] == '[')   // 开始其他节点
        {
            // 增加的
            AddKey(m_mapAddKey, vtNewFile);
            vtNewFile.push_back(itF);
            bAddKey = true;
            bFindSection = false;
            continue;
        }

        // 获取Key名
        if (!GetKeyName(itF, strKeyName))
        {
            vtNewFile.push_back(itF);
            continue;
        }

        // 删除的
        auto itD = m_mapDelKey.find(strKeyName);
        if (itD != m_mapDelKey.end())
            continue;

        // 修改的
        auto itS = m_mapSetKey.find(strKeyName);
        if (itS != m_mapSetKey.end())
        {
            strLine = itS->first + "=" + itS->second;
            vtNewFile.push_back(strLine);
            continue;
        }

        // 其他的
        vtNewFile.push_back(itF);
    }
    // 如果没有增加，则增加在最后
    if (!bAddKey)
        AddKey(m_mapAddKey, vtNewFile);

    // 保存文件
    if (!SaveFile(m_strFileName, vtNewFile, bBackup))
        return false;

    // 重新加载INI文件
    if (m_pINIFileReader != nullptr)
        m_pINIFileReader->LoadINIFile(m_strFileName);

    m_bSaved = true;
    return true;
}

bool CINIWriter::LoadFile(string strName, vectorString &vtFile)
{
    std::lock_guard<recursive_mutex> _auto_lock(m_pINIFileReader->m_cMutex);

    fstream cFile(strName);
    if (!cFile.is_open())
        return false;

    string strLine;
    char szLine[2048] = { 0 };
    while (!cFile.eof())
    {
        memset(szLine, 0x00, sizeof(szLine));
        cFile.getline(szLine, sizeof(szLine));     // 没有换行符
        strLine = szLine;
        m_pINIFileReader->TrimLeftRight(strLine);
        vtFile.push_back(strLine);
    }
    cFile.close();
    return true;
}

bool CINIWriter::SaveFile(string strName, const vectorString &vtFile, bool bBackup/* = false*/)
{
    std::lock_guard<recursive_mutex> _auto_lock(m_pINIFileReader->m_cMutex);
    // 备份文件
    if (bBackup)
    {
        string strNewName = strName + ".bak";
        remove(strNewName.c_str());
        rename(strName.c_str(), strNewName.c_str());
    }

    // ios::trunc 如果所操作的文件已经存在了，就先删除
    std::fstream cFile(strName, ios::out | ios::trunc);
    if (!cFile.is_open())
        return false;

    string strLine;
    for (auto it = vtFile.begin(); it != vtFile.end();)
    {
        strLine = *it;
        if (++it != vtFile.end())
            strLine += '\n'; // 增加一换行符
        cFile.write(strLine.c_str(), strLine.length());
    }

    cFile.close();
    return true;
}

bool CINIWriter::GetKeyName(string strLine, string &strKeyName)
{
    string::size_type nPos = strLine.find('=');
    if (string::npos != nPos)
    {
        strKeyName = strLine.substr(0, nPos);
        m_pINIFileReader->TrimLeftRight(strKeyName);
        return true;
    }
    return false;
}

void CINIWriter::AddKey(const mapSString &mapAddKey, vectorString &vtNewFile)
{
    string strLine;
    ULONG uCount = 0;
    vectorString vtFile(vtNewFile);

    // 反向查找
    for (auto it = vtNewFile.rbegin(); it != vtNewFile.rend(); it++)
    {
        uCount++;
        if ((*it)[0] == '[')     // 节点
        {
            uCount = vtNewFile.size() - uCount + 1;
            break;
        }
        if (GetKeyName(*it, strLine))
        {
            uCount = vtNewFile.size() - uCount + 1;
            break;
        }
    }

    vtNewFile.clear();
    ULONG uIndex = 0;
    for (auto &itF : vtFile)
    {
        vtNewFile.push_back(itF);
        if (++uIndex == uCount)
        {
            for (auto &itA : mapAddKey)
            {
                strLine = itA.first + "=" + itA.second;
                vtNewFile.push_back(strLine);
            }
        }
    }
    return;
}
