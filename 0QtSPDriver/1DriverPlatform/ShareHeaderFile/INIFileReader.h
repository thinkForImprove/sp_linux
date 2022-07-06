#pragma once

#include "QtTypeDef.h"
#include <vector>
#include <string>
#include <map>
#include <mutex>
using namespace std;
#include <string.h>
#include <QDateTime>
//////////////////////////////////////////////////////////////////////////
typedef const char                 *LPCSTR;
typedef unsigned char               BYTE;
typedef unsigned short              WORD;
typedef unsigned int                UINT;
typedef unsigned long               ULONG;
typedef vector<string>              vectorString;
typedef map<string, string>         mapSString;
//////////////////////////////////////////////////////////////////////////
typedef struct tag_ini_file_info
{
    char szSectionName[128];    // 节点名
    char szKeyName[128];        // Key名
    char szKeyValue[512];       // Key值

    tag_ini_file_info() {Clear();}
    void Clear() {memset(this, 0x00, sizeof(tag_ini_file_info));}
} INIFILEINFO, *LPINIFILEINFO;
typedef vector<INIFILEINFO>  vectorINIFILEINFO;
//////////////////////////////////////////////////////////////////////////
class CINIFileReader;
class CINIKeyVal
{
public:
    CINIKeyVal();
    virtual ~CINIKeyVal();
protected:
    void UpdateKeyNameValue();
protected:
    friend class CINIFileReader;
protected:
    char                m_szNull[32];
    string              m_strFileName;
    string              m_strSectionName;
    mapSString          m_mapString;
    vectorString        m_vtName;
    vectorString        m_vtValue;
    CINIFileReader     *m_pINIFileReader;
};
//////////////////////////////////////////////////////////////////////////
class CINIReader : public CINIKeyVal
{
public:
    class AllINIType
    {
    private:
        AllINIType();
        virtual ~AllINIType();
    public:
        operator bool() const;
        operator char() const;
        operator short() const;
        operator int() const;
        operator long() const;
        operator BYTE() const;
        operator WORD() const;
        operator UINT() const;
        operator ULONG() const;
        operator LPCSTR() const;
    private:
        void Clear();
        void SetValue(LPCSTR lpVal);
    private:
        friend class CINIReader;
    private:
        char  m_szValue[512];
    };
public:
    CINIReader();
    virtual ~CINIReader();
public:
    // 重载[]，返回Key名对应的Key值
    AllINIType &operator[](string strName);
    // 返回Key名对应的Key值
    AllINIType &GetValue(string strName, string strDefVal);
    AllINIType &GetValue(string strName, DWORD dwDefVal);
    // 节点名
    LPCSTR GetSectionName();
    // Key数
    UINT Count();
    // Key名
    LPCSTR GetName(UINT uIndex);
    // Key值
    LPCSTR GetValue(UINT uIndex);
private:
    AllINIType          m_cINIType;
};
//////////////////////////////////////////////////////////////////////////
class CINIWriter : public CINIKeyVal
{
public:
    CINIWriter();
    virtual ~CINIWriter();
public:
    // 修改对应的Key值
    void SetValue(string strName, string strVal);
    void SetValue(string strName, DWORD dwVal);
    // 增加新Key、值和注释，
    void AddValue(string strName, string strVal);
    void AddValue(string strName, DWORD dwVal);
    // 删除Key，包括值
    void DeleteKey(string strName);
    // 删除当前节点下所有Key
    void DeleteKey();
    // 保存更新到文件中
    bool Save(bool bBackup = false);
private:
    // 加载文件
    bool LoadFile(string strName, vectorString &vtFile);
    // 保存文件
    bool SaveFile(string strName, const vectorString &vtFile, bool bBackup = false);
    // 获取KeyName
    bool GetKeyName(string strLine, string &strKeyName);
    // 在最后一个Key后面，增加新Key
    void AddKey(const mapSString &mapAddKey, vectorString &vtNewFile);
private:
    bool                m_bSaved;
    vectorString        m_vtFile;
    mapSString          m_mapSetKey;
    mapSString          m_mapAddKey;
    mapSString          m_mapDelKey;
};
//////////////////////////////////////////////////////////////////////////
class CINIFileReader
{
public:
    CINIFileReader();
    CINIFileReader(string strINIFile);
    virtual ~CINIFileReader();
public:
    // 读取环境变量配置
    LPCSTR GetEnvPath(string strEnvName);
    // 读取配置文件，先从当前目录查找，然后再从环境变量配置目录查找
    LPCSTR GetConfigFile(string strEnvName, string strFile);
    // 读取INI文件
    bool LoadINIFile(string strName);
    // 已加载文件名
    string GetINIFile();
    // 全部配置项数
    UINT Count();
    // 按索引读取配置项
    bool GetInfo(UINT uIndex, INIFILEINFO &stInfo);
    // 读取全部配置
    bool GetAllKey(vectorINIFILEINFO &vtKey);
    // 读取指定节点下的所有Key配置
    CINIReader GetReaderSection(string strSectionName);
    // 读取指定节点下的所有Key配置，然后通过该类修改对应Key值
    CINIWriter GetWriterSection(string strSectionName);
    // 读取指定节点下Key值
    LPCSTR GetValue(string strSectionName, string strKeyName);
    LPCSTR GetValue(string strSectionName, string strKeyName, string strDefVal);
    // 创建新节点，
    bool SetNewSection(string strSectionName);

protected:
    // 获取指定节点下的所有Key配置
    void GetKeyBySection(string strSectionName, CINIKeyVal *pINIKeyVal);
    // 删除左右指定字符
    void TrimLeftRight(string &strBuff, string strStep = " \t\r\n");
    // 判断文件是否已被更新
    bool IsNeedUpdateINI();
    // 读取文件修改时间
    void GetFileStat(string strFile, QDateTime &clastModified);
private:
    friend class CINIWriter;
private:
    string              m_strINIFile;
    vectorINIFILEINFO   m_vtINIFile;
    INIFILEINFO         m_stFindInfo;
    char                m_szEnvPath[256];
    char                m_szNull[32];
    CINIReader          m_cINIReader;
    CINIWriter          m_cINIWriter;
    QDateTime           m_clastModified;
    recursive_mutex     m_cMutex;
};

