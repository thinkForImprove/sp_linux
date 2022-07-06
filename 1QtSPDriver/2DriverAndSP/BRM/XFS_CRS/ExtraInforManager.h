// ExtraInforManager.h: interface for the CExtraInforManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXTRAINFORMANAGER_H__06238F57_CAB7_4C81_9A25_F80B632ECB6D__INCLUDED_)
#define AFX_EXTRAINFORMANAGER_H__06238F57_CAB7_4C81_9A25_F80B632ECB6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IBRMAdapter.h"

#include "ExtraInforHelper.h"
#include <map>
#include <string>
using namespace std;

//实现IBRMExtra接口的类
class CExtraInforManager : public IBRMExtra
{
public:
    CExtraInforManager();
    virtual ~CExtraInforManager();

    //内部访问接口
public:
    //清除内部数据
    void Clear();

    // 遍历记录，成功返回0，返回-1表示失败
    //得到首条记录
    long GetFirstRecord(pair<string, string> &Record);

    //得到下一条记录
    long GetNextRecord(pair<string, string> &Record);

    //得到以'='分隔名字与值、以','分隔记录的字串形式的Extra信息
    //返回：字串
    string GetExtraInfo() const;

    //设置以'='分隔名字与值、以','分隔记录的字串形式的Extra信息
    void SetExtraInfo(const char *pExtra);

    //从Src拷贝数据
    void CopyFrom(const CExtraInforManager &Src);

    //判断是否相等
    BOOL IsEqual(const CExtraInforManager &Src) const;

    //实现IBRMExtra接口
    virtual void AddExtra(LPCSTR lpszKey, LPCSTR lpszValue);

    //数据成员
private:
    std::map<std::string, std::string>              m_mapData;  //保存数据
    std::map<std::string, std::string>::iterator    m_it;       //保存遍历位置
};

#endif // !defined(AFX_EXTRAINFORMANAGER_H__06238F57_CAB7_4C81_9A25_F80B632ECB6D__INCLUDED_)
