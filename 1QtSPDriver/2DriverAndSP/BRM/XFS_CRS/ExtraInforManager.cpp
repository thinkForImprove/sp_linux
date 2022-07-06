#include "ExtraInforManager.h"

#include <QMap>
CExtraInforManager::CExtraInforManager()
{
    m_it = m_mapData.begin();
}

CExtraInforManager::~CExtraInforManager()
{
    Clear();
}

void CExtraInforManager::AddExtra(LPCSTR lpszKey, LPCSTR lpszValue)
{
    map<string, string>::iterator it;
    m_mapData[lpszKey] = lpszValue;
}

long CExtraInforManager::GetFirstRecord(pair<string, string> &Record)
{
    m_it = m_mapData.begin();
    if (m_it == m_mapData.end())
        return -1;
    Record = *m_it;
    return 0;
}

long CExtraInforManager::GetNextRecord(pair<string, string> &Record)
{
    m_it++;
    if (m_it == m_mapData.end())
        return -1;
    Record = *m_it;
    return 0;
}

void CExtraInforManager::Clear()
{
    m_mapData.clear();
    m_it = m_mapData.begin();
}

//得到以'='分隔名字与值、以','分隔记录的字串形式的Extra信息
//返回：字串
string CExtraInforManager::GetExtraInfo() const
{
    std::string s;
    std::map<std::string, std::string>::const_iterator it;
    for (it = m_mapData.begin(); it != m_mapData.end(); it++)
    {
        //const pair<string, string> &p = *it;
        if (s.size() != 0)
        {
            s += ",";
        }
        s += (*it).first + "=" + (*it).second;
    }
    //    for (auto &it : m_mapData)
    //    {
    //        if (s.size() != 0)
    //        {
    //            s += ",";
    //        }
    //        //s += (*it).first + "=" + (*it).second;
    //        s = it.first + "=" + it.second;
    //    }
    return s;
}

//设置以'='分隔名字与值、以','分隔记录的字串形式的Extra信息
void CExtraInforManager::SetExtraInfo(const char *pExtra)
{
    Clear();

    char szBuf[4096];
    strcpy(szBuf, pExtra);
    char *pBuffLeft;
    char *p = strtok(szBuf, ",");
    while (p != NULL)
    {
        char *pEqual = strchr(p, '=');
        if (pEqual != p && pEqual != NULL)
        {
            *pEqual = '\0';
            AddExtra(p, pEqual + 1);
        }
        p = strtok(NULL, ",");
    }
}

//从Src拷贝数据
void CExtraInforManager::CopyFrom(const CExtraInforManager &Src)
{
    if (this == &Src)
        return;

    string sSrc = Src.GetExtraInfo();
    SetExtraInfo(sSrc.c_str());
}

//判断是否相等
BOOL CExtraInforManager::IsEqual(const CExtraInforManager &Src) const
{
    string sSrc = Src.GetExtraInfo();
    string sThis = this->GetExtraInfo();
    return sSrc == sThis;
}

