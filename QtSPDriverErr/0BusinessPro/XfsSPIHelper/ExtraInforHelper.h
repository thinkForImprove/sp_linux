// ExtraInforHelper.h: interface for the CExtraInforHelper class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "SimpleMutex.h"
#include "MultiString.h"
#include "QtTypeDef.h"
#include <map>
#include <string>
using namespace std;
//////////////////////////////////////////////////////////////////////////
typedef pair<std::string, std::string>          pairSS;
typedef std::map<std::string, std::string>      mapSString;
//////////////////////////////////////////////////////////////////////////
//扩展状态辅助类
class CExtraInforHelper
{
public:
    CExtraInforHelper();
    virtual ~CExtraInforHelper();
public:
    //清除内部数据
    void Clear();
    // 是否存在
    bool IsExistKey(std::string strKey);
    // 遍历记录，成功返回0，返回-1表示失败
    //得到首条记录
    bool GetFirstRecord(pairSS &Record);
    //得到下一条记录
    bool GetNextRecord(pairSS &Record);
    //得到以'='分隔名字与值、以','分隔记录的字串形式的Extra信息
    //返回：字串
    std::string GetExtraInfo();
    //设置以'='分隔名字与值、以','分隔记录的字串形式的Extra信息
    void SetExtraInfo(const char *pExtra);
    //从Src拷贝数据
    void CopyFrom(CExtraInforHelper &Src);
    //判断是否相等
    bool IsEqual(CExtraInforHelper &Src);
    //添加Extra项
    void AddExtra(LPCSTR lpszKey, LPCSTR lpszValue);
    // 获取Extra
    LPCSTR GetExtra();
    // 获取错误码描述：ErrorDetail=001714010
    LPCSTR GetErrDetail(std::string strErrKey);
private:
    mapSString                      m_mapData;  //保存数据
    mapSString::iterator            m_it;       //保存遍历位置
    CMultiString                    m_cExtra;
    std::string                     m_strErrDetail;
    CSimpleMutex                    m_cMutex;
};
//////////////////////////////////////////////////////////////////////////
