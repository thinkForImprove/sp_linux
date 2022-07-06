// cjson_object.cpp : 定义静态库的函数。
//

#include "framework.h"
#include "cjson_object.h"


CJsonObject::CJsonObject()
    : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL)
{
    // m_pJsonData = cJSON_CreateObject();
}

CJsonObject::CJsonObject(const std::string& strJson)
    : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL)
{
    Parse(strJson);
}

CJsonObject::CJsonObject(const CJsonObject* pJsonObject)
    : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL)
{
    if (pJsonObject) {
        Parse(pJsonObject->ToString());
    }
}

CJsonObject::CJsonObject(const CJsonObject& oJsonObject)
    : m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL)
{
    Parse(oJsonObject.ToString());
}

CJsonObject::~CJsonObject()
{
    Clear();
}


CJsonObject& CJsonObject::operator=(const CJsonObject& oJsonObject)
{
    std::string tmp_str = oJsonObject.ToString();
    Parse(tmp_str.c_str());
    return(*this);
}

bool CJsonObject::operator==(const CJsonObject& oJsonObject) const
{
    return(this->ToString() == oJsonObject.ToString());
}


bool CJsonObject::AddEmptySubObject(const std::string& strKey)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateObject();
    if (pJsonStruct == NULL) {
        m_strErrMsg = std::string("create sub empty object error!");
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::AddEmptySubArray(const std::string& strKey)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateArray();
    if (pJsonStruct == NULL) {
        m_strErrMsg = std::string("create sub empty array error!");
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::GetKey(std::string& strKey)
{
    if (IsArray()) {
        return(false);
    }
    if (m_pKeyTravers == NULL) {
        if (m_pJsonData != NULL) {
            m_pKeyTravers = m_pJsonData;
        } else if (m_pExternJsonDataRef != NULL) {
            m_pKeyTravers = m_pExternJsonDataRef;
        }
        return(false);
    } else if (m_pKeyTravers == m_pJsonData || m_pKeyTravers == m_pExternJsonDataRef) {
        cJSON* c = m_pKeyTravers->child;
        if (c) {
            strKey = c->string;
            m_pKeyTravers = c->next;
            return(true);
        } else {
            return(false);
        }
    } else {
        strKey = m_pKeyTravers->string;
        m_pKeyTravers = m_pKeyTravers->next;
        return(true);
    }
}

void CJsonObject::ResetTraversing()
{
    if (m_pJsonData != NULL) {
        m_pKeyTravers = m_pJsonData;
    } else {
        m_pKeyTravers = m_pExternJsonDataRef;
    }
}

CJsonObject& CJsonObject::operator[](const std::string& strKey)
{
    std::map<std::string, CJsonObject*>::iterator iter;
    iter = m_mapJsonObjectRef.find(strKey);
    if (iter == m_mapJsonObjectRef.end()) {
        cJSON* pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJSON_Object) {
                pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
            }
        } else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJSON_Object) {
                pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
            }
        }
        if (pJsonStruct == NULL) {
            CJsonObject* pJsonObject = new CJsonObject();
            m_mapJsonObjectRef.insert(std::pair<std::string, CJsonObject*>(strKey, pJsonObject));
            return(*pJsonObject);
        } else {
            CJsonObject* pJsonObject = new CJsonObject(pJsonStruct);
            m_mapJsonObjectRef.insert(std::pair<std::string, CJsonObject*>(strKey, pJsonObject));
            return(*pJsonObject);
        }
    } else {
        return(*(iter->second));
    }
}

CJsonObject& CJsonObject::operator[](unsigned int uiWhich)
{
    std::map<unsigned int, CJsonObject*>::iterator iter;
    iter = m_mapJsonArrayRef.find(uiWhich);
    if (iter == m_mapJsonArrayRef.end()) {
        cJSON* pJsonStruct = NULL;
        if (m_pJsonData != NULL) {
            if (m_pJsonData->type == cJSON_Array) {
                pJsonStruct = cJSON_GetArrayItem(m_pJsonData, uiWhich);
            }
        } else if (m_pExternJsonDataRef != NULL) {
            if (m_pExternJsonDataRef->type == cJSON_Array) {
                pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, uiWhich);
            }
        }
        if (pJsonStruct == NULL) {
            CJsonObject* pJsonObject = new CJsonObject();
            m_mapJsonArrayRef.insert(std::pair<unsigned int, CJsonObject*>(uiWhich, pJsonObject));
            return(*pJsonObject);
        } else {
            CJsonObject* pJsonObject = new CJsonObject(pJsonStruct);
            m_mapJsonArrayRef.insert(std::pair<unsigned int, CJsonObject*>(uiWhich, pJsonObject));
            return(*pJsonObject);
        }
    } else {
        return(*(iter->second));
    }
}

std::string CJsonObject::operator()(const std::string& strKey) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(std::string(""));
    }
    if (pJsonStruct->type == cJSON_String) {
        return(pJsonStruct->valuestring);
    } else if (pJsonStruct->type == cJSON_Int) {
        char szNumber[128] = { 0 };
        if (pJsonStruct->sign == -1) {
            if ((long long)pJsonStruct->valueint <= (long long)INT_MAX && (long long)pJsonStruct->valueint >= (long long)INT_MIN) {
                snprintf(szNumber, sizeof(szNumber), "%d", (int)pJsonStruct->valueint);
            } else {
                snprintf(szNumber, sizeof(szNumber), "%lld", (long long)pJsonStruct->valueint);
            }
        } else {
            if (pJsonStruct->valueint <= (unsigned long long)UINT_MAX) {
                snprintf(szNumber, sizeof(szNumber), "%u", (unsigned int)pJsonStruct->valueint);
            } else {
                snprintf(szNumber, sizeof(szNumber), "%llu", pJsonStruct->valueint);
            }
        }
        return(std::string(szNumber));
    } else if (pJsonStruct->type == cJSON_Double) {
        char szNumber[128] = { 0 };
        if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9) {
            snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
        } else {
            snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
        }
    } else if (pJsonStruct->type == cJSON_False) {
        return(std::string("false"));
    } else if (pJsonStruct->type == cJSON_True) {
        return(std::string("true"));
    }
    return(std::string(""));
}

std::string CJsonObject::operator()(unsigned int uiWhich) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, uiWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, uiWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(std::string(""));
    }
    if (pJsonStruct->type == cJSON_String) {
        return(pJsonStruct->valuestring);
    } else if (pJsonStruct->type == cJSON_Int) {
        char szNumber[128] = { 0 };
        if (pJsonStruct->sign == -1) {
            if ((long long)pJsonStruct->valueint <= (long long)INT_MAX && (long long)pJsonStruct->valueint >= (long long)INT_MIN) {
                snprintf(szNumber, sizeof(szNumber), "%d", (int)pJsonStruct->valueint);
            } else {
                snprintf(szNumber, sizeof(szNumber), "%lld", (long long)pJsonStruct->valueint);
            }
        } else {
            if (pJsonStruct->valueint <= (unsigned long long)UINT_MAX) {
                snprintf(szNumber, sizeof(szNumber), "%u", (unsigned int)pJsonStruct->valueint);
            } else {
                snprintf(szNumber, sizeof(szNumber), "%llu", pJsonStruct->valueint);
            }
        }
        return(std::string(szNumber));
    } else if (pJsonStruct->type == cJSON_Double) {
        char szNumber[128] = { 0 };
        if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9) {
            snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
        } else {
            snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
        }
    } else if (pJsonStruct->type == cJSON_False) {
        return(std::string("false"));
    } else if (pJsonStruct->type == cJSON_True) {
        return(std::string("true"));
    }
    return(std::string(""));
}

bool CJsonObject::Parse(const std::string& strJson)
{
    Clear();
    m_pJsonData = cJSON_Parse(strJson.c_str());
    m_pKeyTravers = m_pJsonData;
    if (m_pJsonData == NULL) {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    return(true);
}

void CJsonObject::Clear()
{
    m_pExternJsonDataRef = NULL;
    m_pKeyTravers = NULL;
    if (m_pJsonData != NULL) {
        cJSON_Delete(m_pJsonData);
        m_pJsonData = NULL;
    }
    for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin();
        iter != m_mapJsonArrayRef.end(); ++iter) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
    }
    m_mapJsonArrayRef.clear();
    for (std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.begin();
        iter != m_mapJsonObjectRef.end(); ++iter) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
    }
    m_mapJsonObjectRef.clear();
}

bool CJsonObject::IsEmpty() const
{
    if (m_pJsonData != NULL) {
        return(false);
    } else if (m_pExternJsonDataRef != NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::IsArray() const
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    }

    if (pFocusData == NULL) {
        return(false);
    }

    if (pFocusData->type == cJSON_Array) {
        return(true);
    } else {
        return(false);
    }
}

bool CJsonObject::IsNull() const
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    }

    if (pFocusData == NULL) {
        return(false);
    }

    if (pFocusData->type == cJSON_NULL) {
        return(true);
    } else {
        return(false);
    }
}

bool CJsonObject::SetToNull()
{
    Clear();
    return Parse("null");
}

bool CJsonObject::SetToValue(unsigned long long ullValue)
{
    char sVal[32] = { 0 };

    Clear();
    sprintf(sVal, "%llu", ullValue);

    return Parse(sVal);
}

std::string CJsonObject::ToString() const
{
    char* pJsonString = NULL;
    std::string strJsonData = "";
    if (m_pJsonData != NULL) {
        pJsonString = cJSON_PrintUnformatted(m_pJsonData);
    } else if (m_pExternJsonDataRef != NULL) {
        pJsonString = cJSON_PrintUnformatted(m_pExternJsonDataRef);
    }
    if (pJsonString != NULL) {
        strJsonData = pJsonString;
        free(pJsonString);
    }
    return(strJsonData);
}

std::string CJsonObject::ToFormattedString() const
{
    char* pJsonString = NULL;
    std::string strJsonData = "";
    if (m_pJsonData != NULL) {
        pJsonString = cJSON_Print(m_pJsonData);
    } else if (m_pExternJsonDataRef != NULL) {
        pJsonString = cJSON_Print(m_pExternJsonDataRef);
    }
    if (pJsonString != NULL) {
        strJsonData = pJsonString;
        free(pJsonString);
    }
    return(strJsonData);
}


bool CJsonObject::Get(const std::string& strKey, CJsonObject& oJsonObject) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    char* pJsonString = cJSON_Print(pJsonStruct);
    std::string strJsonData = pJsonString;
    free(pJsonString);
    if (oJsonObject.Parse(strJsonData)) {
        return(true);
    } else {
        return(false);
    }
}

bool CJsonObject::Get(const std::string& strKey, std::string& strValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type != cJSON_String) {
        return(false);
    }
    strValue = pJsonStruct->valuestring;
    return(true);
}

bool CJsonObject::Get(const std::string& strKey, char& cValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        cValue = (char)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        cValue = (char)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, unsigned char& ucValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        ucValue = (unsigned char)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        ucValue = (unsigned char)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, short& sValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        sValue = (short)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        sValue = (short)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, unsigned short& usValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        usValue = (unsigned short)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        usValue = (unsigned short)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, int& iValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        iValue = (int)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        iValue = (int)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, unsigned int& uiValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        uiValue = (unsigned int)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        uiValue = (unsigned int)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, long& lValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        lValue = (long)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        lValue = (long)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, unsigned long& ulValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        ulValue = (unsigned long)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        ulValue = (unsigned long)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, long long& llValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        llValue = (long long)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        llValue = (long long)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, unsigned long long& ullValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        ullValue = (unsigned long long)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        ullValue = (unsigned long long)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, bool& bValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type > cJSON_True) {
        return(false);
    }
    bValue = pJsonStruct->type;
    return(true);
}

bool CJsonObject::Get(const std::string& strKey, float& fValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Double) {
        fValue = (float)(pJsonStruct->valuedouble);
        return(true);
    } else if (pJsonStruct->type == cJSON_Int) {
        fValue = (float)(pJsonStruct->valueint);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(const std::string& strKey, double& dValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Double) {
        dValue = pJsonStruct->valuedouble;
        return(true);
    } else if (pJsonStruct->type == cJSON_Int) {
        dValue = (double)(pJsonStruct->valueint);
        return(true);
    }
    return(false);
}

bool CJsonObject::IsNull(const std::string& strKey) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Object) {
            pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type != cJSON_NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, const CJsonObject& oJsonObject)
{
    cJSON* pFocusData = NULL;
    std::string tmp_str;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    tmp_str = oJsonObject.ToString();
    cJSON* pJsonStruct = cJSON_Parse(tmp_str.c_str());
    if (pJsonStruct == NULL) {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, char cValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)cValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, unsigned char ucValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ucValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, short sValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)sValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, unsigned short usValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)usValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, int iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)iValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, unsigned int uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)uiValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, long lValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)lValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, unsigned long ulValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ulValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, long long llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)llValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, unsigned long long ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt(ullValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, bool bValue, bool bValueAgain)
{
    Q_UNUSED(bValueAgain);

    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Add(const std::string& strKey, double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::AddNull(const std::string& strKey)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateObject();
        m_pKeyTravers = m_pJsonData;
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
        m_strErrMsg = "key exists!";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateNull();
    if (pJsonStruct == NULL) {
        return(false);
    }
    cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Delete(const std::string& strKey)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON_DeleteItemFromObject(pFocusData, strKey.c_str());
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    m_pKeyTravers = pFocusData;
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, const CJsonObject& oJsonObject)
{
    cJSON* pFocusData = NULL;
    std::string tmp_str;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    tmp_str = oJsonObject.ToString();
    cJSON* pJsonStruct = cJSON_Parse(tmp_str.c_str());
    if (pJsonStruct == NULL) {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, char cValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)cValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, unsigned char ucValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ucValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, short sValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)sValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, unsigned short usValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)usValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, int iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)iValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, unsigned int uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)uiValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, long lValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)lValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, unsigned long ulValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ulValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, long long llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)llValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, unsigned long long ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ullValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, bool bValue, bool bValueAgain)
{
    Q_UNUSED(bValueAgain);

    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(const std::string& strKey, double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::ReplaceWithNull(const std::string& strKey)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Object) {
        m_strErrMsg = "not a json object! json array?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateNull();
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<std::string, CJsonObject*>::iterator iter = m_mapJsonObjectRef.find(strKey);
    if (iter != m_mapJsonObjectRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonObjectRef.erase(iter);
    }
    cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
    if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
        return(false);
    }
    return(true);
}

int CJsonObject::GetArraySize()
{
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            return(cJSON_GetArraySize(m_pJsonData));
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            return(cJSON_GetArraySize(m_pExternJsonDataRef));
        }
    }
    return(0);
}

bool CJsonObject::Get(int iWhich, CJsonObject& oJsonObject) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    char* pJsonString = cJSON_Print(pJsonStruct);
    std::string strJsonData = pJsonString;
    free(pJsonString);
    if (oJsonObject.Parse(strJsonData)) {
        return(true);
    } else {
        return(false);
    }
}

bool CJsonObject::Get(int iWhich, std::string& strValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type != cJSON_String) {
        return(false);
    }
    strValue = pJsonStruct->valuestring;
    return(true);
}

bool CJsonObject::Get(int iWhich, char& cValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        cValue = (char)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        cValue = (char)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, unsigned char& ucValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        ucValue = (unsigned char)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        ucValue = (unsigned char)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, short& sValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        sValue = (short)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        sValue = (short)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, unsigned short& usValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        usValue = (unsigned short)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        usValue = (unsigned short)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, int& iValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        iValue = (int)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        iValue = (int)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, unsigned int& uiValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        uiValue = (unsigned int)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        uiValue = (unsigned int)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, long& lValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        lValue = (long)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        lValue = (long)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, unsigned long& ulValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        ulValue = (unsigned long)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        ulValue = (unsigned long)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, long long& llValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        llValue = (long long)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        llValue = (long long)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, unsigned long long& ullValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Int) {
        ullValue = (unsigned long long)(pJsonStruct->valueint);
        return(true);
    } else if (pJsonStruct->type == cJSON_Double) {
        ullValue = (unsigned long long)(pJsonStruct->valuedouble);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, bool& bValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type > cJSON_True) {
        return(false);
    }
    bValue = pJsonStruct->type;
    return(true);
}

bool CJsonObject::Get(int iWhich, float& fValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Double) {
        fValue = (float)(pJsonStruct->valuedouble);
        return(true);
    } else if (pJsonStruct->type == cJSON_Int) {
        fValue = (float)(pJsonStruct->valueint);
        return(true);
    }
    return(false);
}

bool CJsonObject::Get(int iWhich, double& dValue) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type == cJSON_Double) {
        dValue = pJsonStruct->valuedouble;
        return(true);
    } else if (pJsonStruct->type == cJSON_Int) {
        dValue = (double)(pJsonStruct->valueint);
        return(true);
    }
    return(false);
}

bool CJsonObject::IsNull(int iWhich) const
{
    cJSON* pJsonStruct = NULL;
    if (m_pJsonData != NULL) {
        if (m_pJsonData->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
        }
    } else if (m_pExternJsonDataRef != NULL) {
        if (m_pExternJsonDataRef->type == cJSON_Array) {
            pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
        }
    }
    if (pJsonStruct == NULL) {
        return(false);
    }
    if (pJsonStruct->type != cJSON_NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(const CJsonObject& oJsonObject)
{
    cJSON* pFocusData = NULL;
    std::string tmp_str;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    tmp_str = oJsonObject.ToString();
    cJSON* pJsonStruct = cJSON_Parse(tmp_str.c_str());
    if (pJsonStruct == NULL) {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    unsigned int uiLastIndex = (unsigned int)cJSON_GetArraySize(pFocusData) - 1;
    for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin();
        iter != m_mapJsonArrayRef.end(); ) {
        if (iter->first >= uiLastIndex) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter++);
        } else {
            iter++;
        }
    }
    return(true);
}

bool CJsonObject::Add(const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(char cValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)cValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(unsigned char ucValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ucValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(short sValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)sValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(unsigned short usValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)usValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(int iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)iValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(unsigned int uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)uiValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(long lValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)lValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(unsigned long ulValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ulValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(long long llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)llValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(unsigned long long ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ullValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(int iAnywhere, bool bValue)
{
    Q_UNUSED(iAnywhere);

    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Add(double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddNull()
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateNull();
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArray(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(const CJsonObject& oJsonObject)
{
    cJSON* pFocusData = NULL;
    std::string tmp_str;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    tmp_str = oJsonObject.ToString();
    cJSON* pJsonStruct = cJSON_Parse(tmp_str.c_str());
    if (pJsonStruct == NULL) {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin();
        iter != m_mapJsonArrayRef.end(); ) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter++);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(char cValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)cValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(unsigned char ucValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ucValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(short sValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)sValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(unsigned short usValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)usValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(int iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)iValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(unsigned int uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)uiValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(long lValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)lValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(unsigned long ulValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ulValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(long long llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)llValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(unsigned long long ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ullValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(int iAnywhere, bool bValue)
{
    Q_UNUSED(iAnywhere);

    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddAsFirst(double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::AddNullAsFirst()
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData != NULL) {
        pFocusData = m_pJsonData;
    } else if (m_pExternJsonDataRef != NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        m_pJsonData = cJSON_CreateArray();
        pFocusData = m_pJsonData;
    }

    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateNull();
    if (pJsonStruct == NULL) {
        return(false);
    }
    int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
    cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
    int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
    if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Delete(int iWhich)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON_DeleteItemFromArray(pFocusData, iWhich);
    for (std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.begin();
        iter != m_mapJsonArrayRef.end(); ) {
        if (iter->first >= (unsigned int)iWhich) {
            if (iter->second != NULL) {
                delete (iter->second);
                iter->second = NULL;
            }
            m_mapJsonArrayRef.erase(iter++);
        } else {
            iter++;
        }
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, const CJsonObject& oJsonObject)
{
    cJSON* pFocusData = NULL;
    std::string tmp_str;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    tmp_str = oJsonObject.ToString();
    cJSON* pJsonStruct = cJSON_Parse(tmp_str.c_str());
    if (pJsonStruct == NULL) {
        m_strErrMsg = std::string("prase json string error at ") + cJSON_GetErrorPtr();
        return(false);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, const std::string& strValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, char cValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)cValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, unsigned char ucValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ucValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, short sValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)sValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, unsigned short usValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)usValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, int iValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)iValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, unsigned int uiValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)uiValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, long lValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)lValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, unsigned long ulValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ulValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, long long llValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)((unsigned long long)llValue), -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, unsigned long long ullValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateInt((unsigned long long)ullValue, 1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, bool bValue, bool bValueAgain)
{
    Q_UNUSED(bValueAgain);

    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateBool(bValue);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, float fValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::Replace(int iWhich, double dValue)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

bool CJsonObject::ReplaceWithNull(int iWhich)
{
    cJSON* pFocusData = NULL;
    if (m_pJsonData == NULL) {
        pFocusData = m_pExternJsonDataRef;
    } else {
        pFocusData = m_pJsonData;
    }
    if (pFocusData == NULL) {
        m_strErrMsg = "json data is null!";
        return(false);
    }
    if (pFocusData->type != cJSON_Array) {
        m_strErrMsg = "not a json array! json object?";
        return(false);
    }
    cJSON* pJsonStruct = cJSON_CreateNull();
    if (pJsonStruct == NULL) {
        return(false);
    }
    std::map<unsigned int, CJsonObject*>::iterator iter = m_mapJsonArrayRef.find(iWhich);
    if (iter != m_mapJsonArrayRef.end()) {
        if (iter->second != NULL) {
            delete (iter->second);
            iter->second = NULL;
        }
        m_mapJsonArrayRef.erase(iter);
    }
    cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
    if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
        return(false);
    }
    return(true);
}

CJsonObject::CJsonObject(cJSON* pJsonData)
    : m_pJsonData(NULL), m_pExternJsonDataRef(pJsonData), m_pKeyTravers(pJsonData)
{
}
