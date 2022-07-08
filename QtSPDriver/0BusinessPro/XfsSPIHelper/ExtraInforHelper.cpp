// ExtraInforHelper.cpp: implementation of the CExtraInforHelper class.
//
//////////////////////////////////////////////////////////////////////
#include "ExtraInforHelper.h"
//////////////////////////////////////////////////////////////////////////
CExtraInforHelper::CExtraInforHelper()
{
    m_it = m_mapData.begin();
}

CExtraInforHelper::~CExtraInforHelper()
{
    Clear();
}


void CExtraInforHelper::Clear()
{
    AutoMutex(m_cMutex);
    m_cExtra.Clear();
    m_mapData.clear();
    m_it = m_mapData.begin();
}


bool CExtraInforHelper::IsExistKey(std::string strKey)
{
    AutoMutex(m_cMutex);
    if (strKey.empty())
        return false;

    auto it = m_mapData.find(strKey);
    return (it != m_mapData.end());
}

bool CExtraInforHelper::GetFirstRecord(pairSS &Record)
{
    AutoMutex(m_cMutex);
    m_it = m_mapData.begin();
    if (m_it == m_mapData.end())
        return false;

    Record = *m_it;
    return true;
}

bool CExtraInforHelper::GetNextRecord(pairSS &Record)
{
    AutoMutex(m_cMutex);
    if (++m_it == m_mapData.end())
        return false;

    Record = *m_it;
    return true;
}

//得到以'='分隔名字与值、以','分隔记录的字串形式的Extra信息
//返回：字串
std::string CExtraInforHelper::GetExtraInfo()
{
    AutoMutex(m_cMutex);
    std::string s;
    for (auto &it : m_mapData)
    {
        if (s.size() != 0)
            s += ",";
        s += it.first + "=" + it.second;
    }
    return s;
}

//设置以'='分隔名字与值、以','分隔记录的字串形式的Extra信息
void CExtraInforHelper::SetExtraInfo(const char *pExtra)
{
    AutoMutex(m_cMutex);
    Clear();
    char szBuf[4096] = {0};
    strcpy(szBuf, pExtra);
    char *p = strtok(szBuf, ",");
    while (p != nullptr)
    {
        char *pEqual = strchr(p, '=');
        if (pEqual != p && pEqual != nullptr)
        {
            *pEqual = '\0';
            AddExtra(p, pEqual + 1);
        }
        p = strtok(nullptr, ",");
    }
}

//从Src拷贝数据
void CExtraInforHelper::CopyFrom(CExtraInforHelper &Src)
{
    AutoMutex(m_cMutex);
    if (this == &Src)
        return;

    std::string sSrc = Src.GetExtraInfo();
    SetExtraInfo(sSrc.c_str());
}

//判断是否相等
bool CExtraInforHelper::IsEqual(CExtraInforHelper &Src)
{
    AutoMutex(m_cMutex);
    std::string sSrc = Src.GetExtraInfo();
    std::string sThis = this->GetExtraInfo();
    return sSrc == sThis;
}


void CExtraInforHelper::AddExtra(LPCSTR lpszKey, LPCSTR lpszValue)
{
    AutoMutex(m_cMutex);
    m_mapData[lpszKey] = lpszValue;
}

LPCSTR CExtraInforHelper::GetExtra()
{
    AutoMutex(m_cMutex);
    std::string strBuf;
    m_cExtra.Clear();
    for (auto &it : m_mapData)
    {
        strBuf = it.first + "=" + it.second;
        m_cExtra.Add(strBuf.c_str());
    }
    return (LPCSTR)m_cExtra;
}

LPCSTR CExtraInforHelper::GetErrDetail(std::string strErrKey)
{
    AutoMutex(m_cMutex);
    // ErrorDetail=001714010
    m_strErrDetail = "";
    if (!strErrKey.empty())
    {
        auto it = m_mapData.find(strErrKey);
        if (it != m_mapData.end())
        {
            m_strErrDetail = it->first + "=" + it->second;
        }
    }
    return m_strErrDetail.c_str();
}
