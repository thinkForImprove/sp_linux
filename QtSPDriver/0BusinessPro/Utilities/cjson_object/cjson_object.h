#ifndef CJSON_OBJECT_H
#define CJSON_OBJECT_H

#include "cjson_object_global.h"
#include <string>
#include <map>
#include "__common_version_def.h"

struct cJSON;
class CJSON_OBJECT_EXPORT CJsonObject
{
    DEFINE_STATIC_VERSION_FUNCTIONS("cjson_object", "0.0.0.0", TYPE_DYNAMIC)

    public:     // method of ordinary json object or json array
        CJsonObject();
        CJsonObject(const std::string& strJson);
        CJsonObject(const CJsonObject* pJsonObject);
        CJsonObject(const CJsonObject& oJsonObject);
        virtual ~CJsonObject();

        CJsonObject& operator=(const CJsonObject& oJsonObject);
        bool operator==(const CJsonObject& oJsonObject) const;
        bool Parse(const std::string& strJson);
        void Clear();
        bool IsEmpty() const;
        bool IsArray() const;
        bool IsNull() const;
        bool SetToNull();
        bool SetToValue(unsigned long long ullValue);
        std::string ToString() const;
        std::string ToFormattedString() const;
        const std::string& GetErrMsg() const
        {
            return(m_strErrMsg);
        }

    public:     // method of ordinary json object
        bool AddEmptySubObject(const std::string& strKey);
        bool AddEmptySubArray(const std::string& strKey);
        bool GetKey(std::string& strKey);
        void ResetTraversing();
        CJsonObject& operator[](const std::string& strKey);
        std::string operator()(const std::string& strKey) const;
        bool Get(const std::string& strKey, CJsonObject& oJsonObject) const;
        bool Get(const std::string& strKey, std::string& strValue) const;
        bool Get(const std::string& strKey, char& cValue) const;
        bool Get(const std::string& strKey, unsigned char& ucValue) const;
        bool Get(const std::string& strKey, short& sValue) const;
        bool Get(const std::string& strKey, unsigned short& usValue) const;
        bool Get(const std::string& strKey, int& iValue) const;
        bool Get(const std::string& strKey, unsigned int& uiValue) const;
        bool Get(const std::string& strKey, long& lValue) const;
        bool Get(const std::string& strKey, unsigned long& ulValue) const;
        bool Get(const std::string& strKey, long long& llValue) const;
        bool Get(const std::string& strKey, unsigned long long& ullValue) const;
        bool Get(const std::string& strKey, bool& bValue) const;
        bool Get(const std::string& strKey, float& fValue) const;
        bool Get(const std::string& strKey, double& dValue) const;
        bool IsNull(const std::string& strKey) const;
        bool Add(const std::string& strKey, const CJsonObject& oJsonObject);
        bool Add(const std::string& strKey, const std::string& strValue);
        bool Add(const std::string& strKey, char cValue);
        bool Add(const std::string& strKey, unsigned char ucValue);
        bool Add(const std::string& strKey, short sValue);
        bool Add(const std::string& strKey, unsigned short usValue);
        bool Add(const std::string& strKey, int iValue);
        bool Add(const std::string& strKey, unsigned int uiValue);
        bool Add(const std::string& strKey, long lValue);
        bool Add(const std::string& strKey, unsigned long ulValue);
        bool Add(const std::string& strKey, long long llValue);
        bool Add(const std::string& strKey, unsigned long long ullValue);
        bool Add(const std::string& strKey, bool bValue, bool bValueAgain);
        bool Add(const std::string& strKey, float fValue);
        bool Add(const std::string& strKey, double dValue);
        bool AddNull(const std::string& strKey);    // add null like this:   "key":null
        bool Delete(const std::string& strKey);
        bool Replace(const std::string& strKey, const CJsonObject& oJsonObject);
        bool Replace(const std::string& strKey, const std::string& strValue);
        bool Replace(const std::string& strKey, char cValue);
        bool Replace(const std::string& strKey, unsigned char ucValue);
        bool Replace(const std::string& strKey, short sValue);
        bool Replace(const std::string& strKey, unsigned short usValue);
        bool Replace(const std::string& strKey, int iValue);
        bool Replace(const std::string& strKey, unsigned int uiValue);
        bool Replace(const std::string& strKey, long lValue);
        bool Replace(const std::string& strKey, unsigned long ulValue);
        bool Replace(const std::string& strKey, long long llValue);
        bool Replace(const std::string& strKey, unsigned long long ullValue);
        bool Replace(const std::string& strKey, bool bValue, bool bValueAgain);
        bool Replace(const std::string& strKey, float fValue);
        bool Replace(const std::string& strKey, double dValue);
        bool ReplaceWithNull(const std::string& strKey);    // replace value with null

    public:     // method of json array
        int GetArraySize();
        CJsonObject& operator[](unsigned int uiWhich);
        std::string operator()(unsigned int uiWhich) const;
        bool Get(int iWhich, CJsonObject& oJsonObject) const;
        bool Get(int iWhich, std::string& strValue) const;
        bool Get(int iWhich, char& cValue) const;
        bool Get(int iWhich, unsigned char& ucValue) const;
        bool Get(int iWhich, short& sValue) const;
        bool Get(int iWhich, unsigned short& usValue) const;
        bool Get(int iWhich, int& iValue) const;
        bool Get(int iWhich, unsigned int& uiValue) const;
        bool Get(int iWhich, long& lValue) const;
        bool Get(int iWhich, unsigned long& ulValue) const;
        bool Get(int iWhich, long long& llValue) const;
        bool Get(int iWhich, unsigned long long& ullValue) const;
        bool Get(int iWhich, bool& bValue) const;
        bool Get(int iWhich, float& fValue) const;
        bool Get(int iWhich, double& dValue) const;
        bool IsNull(int iWhich) const;
        bool Add(const CJsonObject& oJsonObject);
        bool Add(const std::string& strValue);
        bool Add(char cValue);
        bool Add(unsigned char ucValue);
        bool Add(short sValue);
        bool Add(unsigned short usValue);
        bool Add(int iValue);
        bool Add(unsigned int uiValue);
        bool Add(long lValue);
        bool Add(unsigned long ulValue);
        bool Add(long long llValue);
        bool Add(unsigned long long ullValue);
        bool Add(int iAnywhere, bool bValue);
        bool Add(float fValue);
        bool Add(double dValue);
        bool AddNull();   // add a null value
        bool AddAsFirst(const CJsonObject& oJsonObject);
        bool AddAsFirst(const std::string& strValue);
        bool AddAsFirst(char cValue);
        bool AddAsFirst(unsigned char ucValue);
        bool AddAsFirst(short sValue);
        bool AddAsFirst(unsigned short usValue);
        bool AddAsFirst(int iValue);
        bool AddAsFirst(unsigned int uiValue);
        bool AddAsFirst(long lValue);
        bool AddAsFirst(unsigned long ulValue);
        bool AddAsFirst(long long llValue);
        bool AddAsFirst(unsigned long long ullValue);
        bool AddAsFirst(int iAnywhere, bool bValue);
        bool AddAsFirst(float fValue);
        bool AddAsFirst(double dValue);
        bool AddNullAsFirst();     // add a null value
        bool Delete(int iWhich);
        bool Replace(int iWhich, const CJsonObject& oJsonObject);
        bool Replace(int iWhich, const std::string& strValue);
        bool Replace(int iWhich, char cValue);
        bool Replace(int iWhich, unsigned char ucValue);
        bool Replace(int iWhich, short sValue);
        bool Replace(int iWhich, unsigned short usValue);
        bool Replace(int iWhich, int iValue);
        bool Replace(int iWhich, unsigned int uiValue);
        bool Replace(int iWhich, long lValue);
        bool Replace(int iWhich, unsigned long ulValue);
        bool Replace(int iWhich, long long llValue);
        bool Replace(int iWhich, unsigned long long ullValue);
        bool Replace(int iWhich, bool bValue, bool bValueAgain);
        bool Replace(int iWhich, float fValue);
        bool Replace(int iWhich, double dValue);
        bool ReplaceWithNull(int iWhich);      // replace with a null value

    private:
        CJsonObject(cJSON* pJsonData);

    private:
        cJSON* m_pJsonData;
        cJSON* m_pExternJsonDataRef;
        cJSON* m_pKeyTravers;
        std::string m_strErrMsg;
        std::map<unsigned int, CJsonObject*> m_mapJsonArrayRef;
        std::map<std::string, CJsonObject*> m_mapJsonObjectRef;
};

#endif // CJSON_OBJECT_H
