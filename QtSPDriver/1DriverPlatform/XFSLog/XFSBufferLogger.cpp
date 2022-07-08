#include "XFSBufferLogger.h"

#define ADD_ATOM(AtomType)  m_mapAtomType2Len[#AtomType] = sizeof(AtomType)


//测试类型名是否为一个指针，如“LPWFSCDMSTATUS”返回TRUE
inline BOOL TypeNameIsPtr(const char *pTypeName)
{
    if (pTypeName[0] == 'L' && pTypeName[1] == 'P')
        return TRUE;
    if (strchr(pTypeName, '*') != NULL)
        return TRUE;
    return FALSE;
}

//测试类型名是否是一个两重指针，如“LPWFSCDMSTATUS*”返回TRUE
inline BOOL TypeNameIsPtrOfPtr(const char *pTypeName)
{
    if (pTypeName[0] != 'L' || pTypeName[1] != 'P')
        return FALSE;
    if (strchr(pTypeName, '*') == NULL)
        return FALSE;
    return TRUE;
}

//测试类型名是否为一个数组，如：“USHORT[3]”返回TRUE
inline BOOL TypeNameIsArray(const char *pTypeName)
{
    const char *pLeft = strstr(pTypeName, "[");
    const char *pRight = strstr(pTypeName, "]");
    if (pLeft != NULL && pRight != NULL)
        return TRUE;
    return FALSE;
}

//得到数组类型名中的结构部分,如：“USHORT[3]”返回“USHORT”
bool GetTypeNameOfArrayName(const char *pTypeName, int &nArraySize, char szTypeName[MAX_TYPE_NAME_LEN])
{
    memset(szTypeName, 0, sizeof(char)*MAX_TYPE_NAME_LEN);
    strcpy(szTypeName, pTypeName);
    char *pLeft = strstr(szTypeName, "[");
    char *pRight = strstr(szTypeName, "]");
    assert(pLeft != NULL && pRight != NULL);
    if(pLeft == nullptr || pRight == nullptr)
        return false;
    pLeft[0] = 0;
    pRight[0] = 0;
    nArraySize = atol(pLeft + 1);
    return true;
}

//得到指针名中的结构部分，如：“LPWFSRESULT”或“WFSRESULT*”返回“WFSRESULT”
bool GetTypeNameOfPointerName(const char *pTypeName, char szTypeName[MAX_TYPE_NAME_LEN])
{
    if(!TypeNameIsPtr(pTypeName))
        return false;
    memset(szTypeName,0,sizeof(char)*MAX_TYPE_NAME_LEN);
    strcpy(szTypeName, pTypeName);
    char *p = strrchr(szTypeName, '*');
    if (p != NULL)
    {
        *p = 0;
        return true;
    }

    if(szTypeName[0] != 'L' || szTypeName[1] != 'P')
        return false;
    strcpy(szTypeName, pTypeName + 2);
    return true;
}

//////////////////////////////////////////////////////////////////////
//              CXFSBufferLogger
//////////////////////////////////////////////////////////////////////
CXFSBufferLogger::CXFSBufferLogger(CStringBuffer &StringBuffer) : m_StringBuffer(StringBuffer), m_eMsgType(MT_EVENT), m_dwMsgID(0)
{
    //装载m_mapConstType2Group
    const CONST_TABLE_ITEM *pConstTable = GetConstTable();
    while (pConstTable->pName != NULL)
    {
        const CONST_DEF *pConstDef = pConstTable->pDef;
        CONST_VALUE2NAME &cvalue2Name = m_mapConstType2Group[pConstTable->pName];
        while (pConstDef->pName != NULL)
        {
            cvalue2Name[pConstDef->nValue] = pConstDef->pName;
            pConstDef++;
        }
        pConstTable++;
    }

    //装载m_mapStrName2Def
    const STRUCT_DEFINE *pStrDef = GetStructDefine();
    while (pStrDef->pName != NULL)
    {
        assert(pStrDef->pMembers != NULL);
        m_mapStrName2Def[pStrDef->pName] = pStrDef;
        pStrDef++;
    }

    //装载m_mapMemName2LenDef、m_mapLenMemName2Values
    const MEMBER_LEN *pMemberLen = GetMemberLenDef();
    while (pMemberLen->pName != NULL)
    {
        assert(pMemberLen->pLenDef != NULL);
        if (strcmp(pMemberLen->pLenDef, "NULL_END_PTR") == 0)
        {
            m_setNullEndMemName.insert(pMemberLen->pName);
        }
        else if (strcmp(pMemberLen->pLenDef, "ZERO_END_PTR") == 0)
        {
            m_setZeroEndPtrMemName.insert(pMemberLen->pName);
        }
        else if (strcmp(pMemberLen->pLenDef, "ZERO_ZERO_END_STR") == 0)
        {
            m_setZeroZeroEndStrMemName.insert(pMemberLen->pName);
        }
        else if (strcmp(pMemberLen->pLenDef, "ZERO_ZERO_ZERO_END_STR") == 0)
        {
            m_setZeroZeroZeroEndStrMemName.insert(pMemberLen->pName);
        }
        else
        {
            m_mapMemName2LenDef[pMemberLen->pName] = pMemberLen->pLenDef;
            m_mapLenMemName2Values[pMemberLen->pLenDef] = -1;
        }
        pMemberLen++;
    }

    //装载m_mapMsgID2MsgStr
    const MSG2STRUCT *pMsg2Str = GetMsg2Struct();
    while (pMsg2Str->pTypeName != NULL)
    {
        m_mapMsgID2MsgStr[MakeMsgID(pMsg2Str->eMsgType, pMsg2Str->dwID)] = pMsg2Str;
        pMsg2Str++;
    }

    //原子类型
    ADD_ATOM(char);
    ADD_ATOM(CHAR);
    ADD_ATOM(BOOL);
    ADD_ATOM(BYTE);
    ADD_ATOM(WORD);
    ADD_ATOM(SHORT);
    ADD_ATOM(USHORT);
    ADD_ATOM(DWORD);
    ADD_ATOM(ULONG);
    ADD_ATOM(LONG);
    ADD_ATOM(REQUESTID);
    ADD_ATOM(HSERVICE);
    ADD_ATOM(HRESULT);
    ADD_ATOM(SYSTEMTIME);
    ADD_ATOM(LPBYTE);
    ADD_ATOM(LPWORD);
    ADD_ATOM(LPDWORD);
    ADD_ATOM(LPSHORT);
    ADD_ATOM(LPUSHORT);
    ADD_ATOM(LPLONG);
    ADD_ATOM(LPULONG);
    ADD_ATOM(LPWSTR);
    ADD_ATOM(LPSTR);
    ADD_ATOM(LPVOID);
}

CXFSBufferLogger::~CXFSBufferLogger() {}

const char *CXFSBufferLogger::GetMsgTypeDesc(MSGTYPE eMsgType)
{
    switch (eMsgType)
    {
    case MT_EVENT:
        return "MT_EVENT";
    case MT_GI:
        return "MT_GI";
    case MT_GC:
        return "MT_GC";
    case MT_EX:
        return "MT_EX";
    case MT_EC:
        return "MT_EC";
    default:
    {
        static char szBuf[20];
        sprintf(szBuf, "%d", eMsgType);
        return szBuf;
    }
    }
}

// pNonPtrTypeName：非指针的类型名
int CXFSBufferLogger::GetZeroEndPtrArraySize(const char *pNonPtrTypeName, const char *pDataName, LPCTSTR pData)
{
    assert(!TypeNameIsPtr(pNonPtrTypeName));
    ATOM_TYPE2LEN::const_iterator it = m_mapAtomType2Len.find(pNonPtrTypeName);
    assert(it != m_mapAtomType2Len.end());
    if (it == m_mapAtomType2Len.end())
        return -1;  //不应该出现该条件
    int nSize = 0;
    try
    {
        while (true)
        {
            //查看数据是否为全0
            int i;
            for (i = 0; i < it->second && pData[i] == 0; i++)
                ;
            if (i == it->second)
                break;

            nSize++;
            pData += it->second;
        }
    }
    catch (...)
    {
        LogException(pDataName, pData);
        return -1;
    }

    return nSize;
}

bool CXFSBufferLogger::GetMemberFullName(const char *pMemberName, char szFullName[MAX_TYPE_NAME_LEN]) const
{
    if((m_stackStructCalled.size() <= 0) || m_stackStructCalled.empty())
        return false;

    const STRUCT_DEFINE * strDef = m_stackStructCalled.top();
    if(strDef == nullptr)
        return false;
    //形成成员变量完整名字
    sprintf(szFullName, "%s___%s", strDef->pName, pMemberName);
    return true;
}

const char *CXFSBufferLogger::ConstValue2Str(const char *pType, int nValue) const
{
    CONST_TYPE2GROUP::const_iterator itType = m_mapConstType2Group.find(pType);
    assert(itType != m_mapConstType2Group.end());
    const CONST_VALUE2NAME &ConstValue2Name = itType->second;
    CONST_VALUE2NAME::const_iterator it = ConstValue2Name.find(nValue);
    if (it == ConstValue2Name.end())
        return NULL;
    return it->second;
}

const char *CXFSBufferLogger::GetNameOfMsgID() const
{
    if (m_eMsgType == MT_EC || m_eMsgType == MT_EX)
        return ConstValue2Str(CONST_TYPE_EXECUTE, m_dwMsgID);
    if (m_eMsgType == MT_GC || m_eMsgType == MT_GI)
        return ConstValue2Str(CONST_TYPE_GET_INFO, m_dwMsgID);
    return ConstValue2Str(CONST_TYPE_MESSAGE, m_dwMsgID);
}

//形成日志字串
//返回以\0结束的字串
void CXFSBufferLogger::Log(MSGTYPE eMsgType, DWORD dwID, const char *pDataName, LPCTSTR pData, int nTabNum)
{
    m_eMsgType = eMsgType;
    m_dwMsgID = dwID;

    //查找消息，找到结构定义
    MSGID2MSG_STR::const_iterator itMsg = m_mapMsgID2MsgStr.find(MakeMsgID(eMsgType, dwID));
    if (itMsg == m_mapMsgID2MsgStr.end())
    {
        m_StringBuffer.AddTab(nTabNum).AddF("ERROR: msg not found(id=%d, type=%s)", dwID, GetMsgTypeDesc(eMsgType)).EndLine();
        return;
    }
    const char *pTypeName = itMsg->second->pTypeName;
    if (itMsg->second->pDataName[0] != 0)
        pDataName = itMsg->second->pDataName;
    assert(pTypeName != NULL);

    //如果类型名为NULL，表示没有参数
    if (strcmp(pTypeName, "NULL") == 0 || pData == NULL)
    {
        LogPtrValue(pDataName, pData, nTabNum);
        return;
    }
    assert(pDataName != NULL && strlen(pDataName) > 0);

    assert(m_stackStructCalled.size() == 0);
    LogPointerType(pDataName, pTypeName, pData, nTabNum);
    assert(m_stackStructCalled.size() == 0);
}

//返回0表示不是原子数据
int CXFSBufferLogger::GetAtomDataLen(const char *pTypeName) const
{
    ATOM_TYPE2LEN::const_iterator it = m_mapAtomType2Len.find(pTypeName);
    if (it == m_mapAtomType2Len.end())
        return 0;
    return it->second;
}

int CXFSBufferLogger::GetStructLen(const char *pTypeName) const
{
    if(TypeNameIsPtr(pTypeName))
        return 0;
    if(GetAtomDataLen(pTypeName) != 0)
        return 0;

    //查找结构定义
    STR_NAME2DEF::const_iterator it = m_mapStrName2Def.find(pTypeName);
    if(it == m_mapStrName2Def.end())
        return 0;
    if(it->first != pTypeName)
        return 0;
    const STRUCT_DEFINE *pStrDef = it->second;

    //计算每个成员的长度
    int nTotalLen = 0;
    const STRUCT_MEMBER *pMember = pStrDef->pMembers;
    while (pMember->pName != nullptr)
    {
        int nLen = GetDataTypeLen(pMember->pType);
        if(nLen <= 0)
            break;
        nTotalLen += nLen;
        pMember++;
    }

    return nTotalLen;
}

int CXFSBufferLogger::GetDataTypeLen(const char *pTypeName) const
{
    if (TypeNameIsArray(pTypeName))
    {
        int nArraySize = 0;
        char sTypeName[MAX_TYPE_NAME_LEN] = {0};
        if(!GetTypeNameOfArrayName(pTypeName, nArraySize, sTypeName))
            return -1;
        return nArraySize * GetDataTypeLen(sTypeName);
    }
    if (TypeNameIsPtr(pTypeName))
        return sizeof(LPVOID);

    int nLen = GetAtomDataLen(pTypeName);
    if (nLen > 0)
        return nLen;

    return GetStructLen(pTypeName);
}

int CXFSBufferLogger::GetPointerMemberDataLen(const char *pMemberName) const
{
    if (m_stackStructCalled.size() == 0)
        return -1;

    char sMemberFullName[MAX_TYPE_NAME_LEN] = {0};
    if(!GetMemberFullName(pMemberName, sMemberFullName))
        return -1;
    MEM_NAME2LEN_DEF::const_iterator it = m_mapMemName2LenDef.find(sMemberFullName);
    if (it == m_mapMemName2LenDef.end())
        return -1;
    MEM_NAME2VALUE::const_iterator itValue = m_mapLenMemName2Values.find(it->second);
    assert(itValue != m_mapLenMemName2Values.end());
    assert(itValue->second >= 0);

    return itValue->second;
}

const char *CXFSBufferLogger::ConvertMemberValue2Desc(const char *pMemberFullName, int nValue) const
{
    CONST_TYPE2GROUP::const_iterator it = m_mapConstType2Group.find(pMemberFullName);
    if (it == m_mapConstType2Group.end())
    {
        return NULL;
    }
    const CONST_VALUE2NAME &value2name = it->second;
    CONST_VALUE2NAME::const_iterator itValue = value2name.find(nValue);
    if (itValue != value2name.end())
    {
        return itValue->second;
    }

    static char szDesc[MAX_VALUE_DESC_LEN];
    szDesc[0] = 0;
    char *p = szDesc;

    for (itValue = value2name.begin(); itValue != value2name.end(); itValue++)
    {
        if ((nValue & itValue->first) != 0)
        {
            assert(*p == 0);
            if (p != szDesc)
            {
                strcpy(p, " | ");
                p += strlen(p);
            }
            nValue &= ~itValue->first;
            strcpy(p, itValue->second);
            p += strlen(p);
            sprintf(p, "(0x%04X)", itValue->first);
            p += strlen(p);
        }
    }
    if (nValue != 0)
    {
        if (p != szDesc)
        {
            strcpy(p, " | ");
            p += strlen(p);
        }
        sprintf(p, "0x%04X", nValue);
    }

    return szDesc;
}

void CXFSBufferLogger::LogWORDOrDWORDValue(const char *pMemberFullName, int nValue, BOOL bWORD)
{
    const char *pValueDesc = ConvertMemberValue2Desc(pMemberFullName, nValue);  //转换为值描述
    if (pValueDesc != NULL)
    {
        m_StringBuffer.AddF("%s(%u)", pValueDesc, nValue);
    }
    else if (bWORD)
    {
        m_StringBuffer.AddF("0x%04.4X(%u)", nValue, nValue);
    }
    else
    {
        m_StringBuffer.AddF("0x%08.8X(%u)", nValue, nValue);
    }
}

void CXFSBufferLogger::LogAtomType(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum)
{
    assert(pDataName != NULL);
    assert(pTypeName != NULL);
    assert(pData != NULL);
    assert(!TypeNameIsPtr(pTypeName));      //不允许为指针
    assert(GetAtomDataLen(pTypeName) > 0);  //必须为原子类型

    //形成成员变量完整名字
    char sMemberFullName[MAX_TYPE_NAME_LEN] = {0};
    if(!GetLenDefineName(pDataName, sMemberFullName))
        return;
    //记录数据
    int nValue = 0;                                          //保存变量值，用于设置到m_mapLenMemName2Values
    m_StringBuffer.AddTab(nTabNum).AddF("%s: ", pDataName);  //加TAB及变量名
    try
    {
        if (strcasecmp(pTypeName, "CHAR") == 0)
        {
            // assert(FALSE); //不可能到该路径，XFS结构中没有该类型非指针、非数组类数据
            if (isprint(*pData))
            {
                m_StringBuffer.AddF("%c", *pData);
            }
            else
            {
                m_StringBuffer.AddF("0x%02.2X", *pData);
            }
        }
        else if (strcmp(pTypeName, "BYTE") == 0)
        {
            nValue = *(BYTE *)pData;
            m_StringBuffer.AddF("0x%02.2X(%u)", nValue, nValue);
        }
        else if (strcmp(pTypeName, "BOOL") == 0)
        {
            m_StringBuffer.AddF("%s", *(BOOL *)pData == 0 ? "FALSE" : "TRUE");
        }
        else if (strcmp(pTypeName, "SHORT") == 0)
        {
            nValue = *(SHORT *)pData;
            m_StringBuffer.AddF("0x%04.4X(%hd)", nValue, nValue);
        }
        else if (strcmp(pTypeName, "USHORT") == 0)
        {
            nValue = *(USHORT *)pData;
            m_StringBuffer.AddF("0x%04.4X(%hu)", nValue, nValue);
        }
        else if (strcmp(pTypeName, "LONG") == 0)
        {
            nValue = *(LONG *)pData;
            m_StringBuffer.AddF("0x%08.8X(%ld)", nValue, nValue);
        }
        else if (strcmp(pTypeName, "ULONG") == 0)
        {
            nValue = *(ULONG *)pData;
            m_StringBuffer.AddF("0x%08.8X(%lu)", nValue, nValue);
        }
        else if (strcmp(pTypeName, "WORD") == 0)
        {
            nValue = *(WORD *)pData;
            LogWORDOrDWORDValue(sMemberFullName, nValue, TRUE);
        }
        else if (strcmp(pTypeName, "DWORD") == 0)
        {
            nValue = *(DWORD *)pData;
            LogWORDOrDWORDValue(sMemberFullName, nValue, FALSE);
        }
        else if (strcmp(pTypeName, "REQUESTID") == 0)
        {
            nValue = *(REQUESTID *)pData;
            m_StringBuffer.AddF("0x%08.8X(%ld)", nValue, nValue);
        }
        else if (strcmp(pTypeName, "HSERVICE") == 0)
        {
            nValue = *(HSERVICE *)pData;
            m_StringBuffer.AddF("0x%08.8X(%ld)", nValue, nValue);
        }
        else if (strcmp(pTypeName, "HRESULT") == 0)
        {
            nValue = *(HRESULT *)pData;
            m_StringBuffer.AddF("0x%08.8X(%ld)", nValue, nValue);
        }
        else if (strcmp(pTypeName, "SYSTEMTIME") == 0)
        {
            SYSTEMTIME st = *(SYSTEMTIME *)pData;
            m_StringBuffer.AddF("%04.4d-%02.2d-%02.2d %02.2d:%02.2d:%02.2d.%03.3d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                                st.wMilliseconds);
        }
        else
        {
            assert(FALSE);
            return;
        }
    }
    catch (...)
    {
        LogException(pDataName, pData);
        return;
    }

    m_StringBuffer.EndLine();  //变量记录完成后换行

    //如果m_mapLenMemName2Values存在该成员，把实际值保存在其中
    if (m_mapLenMemName2Values.find(sMemberFullName) != m_mapLenMemName2Values.end())
    {
        assert(nValue >= 0);
        m_mapLenMemName2Values[sMemberFullName] = nValue;
    }
}

bool CXFSBufferLogger::GetLenDefineName(const char *pDataName, char sLenDefName[MAX_TYPE_NAME_LEN]) const
{
    if (m_stackStructCalled.size() == 0)   //未在结构中
    {
        const char *pCmdName = GetNameOfMsgID();
        if(pCmdName == nullptr)
            return false;
        if (m_eMsgType == MT_GC || m_eMsgType == MT_EC)
        {
            sprintf(sLenDefName,"%s_RET",pCmdName);
        }
        else
            strcpy(sLenDefName, pCmdName);
    }
    else
    {
        if(!GetMemberFullName(pDataName,sLenDefName))
            return false;
    }
    return true;
}

STR_END_TYPE CXFSBufferLogger::GetStringEndType(const char *pDataName) const
{
    STR_END_TYPE eStrEndType = SET_ZERO_END;
    char sLenDefName[MAX_TYPE_NAME_LEN] = {0};
    GetLenDefineName(pDataName, sLenDefName);
    if (m_setZeroZeroEndStrMemName.find(sLenDefName) != m_setZeroZeroEndStrMemName.end())
    {
        eStrEndType = SET_ZZ_END;
    }
    else if (m_setZeroZeroZeroEndStrMemName.find(sLenDefName) != m_setZeroZeroZeroEndStrMemName.end())
    {
        eStrEndType = SET_ZZZ_END;
    }
    return eStrEndType;
}

void CXFSBufferLogger::LogZeroEndString(const char *pDataName, LPCTSTR pData, int nTabNum)
{
    m_StringBuffer.AddTab(nTabNum).AddF("%s: ", pDataName).Add(pData).EndLine();
}

void CXFSBufferLogger::LogZeroEndWString(const char *pDataName, LPCWSTR pData, int nTabNum)
{
    m_StringBuffer.AddTab(nTabNum).AddF("%s: ", pDataName).AddF("%S", pData).EndLine();
}

void CXFSBufferLogger::LogZZEndWString(const char *pDataName, LPCWSTR pData, int nTabNum)
{
    LogPtrValue(pDataName, (LPCTSTR)pData, nTabNum);
    while (pData[0] != 0 && pData[1] != 0)
    {
        m_StringBuffer.AddTab(nTabNum + 1).AddF("%S", pData).EndLine();
        pData += wcslen((wchar_t *)pData) + 1;
    }
}

void CXFSBufferLogger::LogZZEndString(const char *pDataName, LPCTSTR pData, int nTabNum)
{
    LogPtrValue(pDataName, pData, nTabNum);
    while (pData[0] != 0 && pData[1] != 0)
    {
        m_StringBuffer.AddTab(nTabNum + 1).Add(pData).EndLine();
        pData += strlen(pData) + 1;
    }
}

void CXFSBufferLogger::LogZZZEndWString(const char *pDataName, LPCWSTR pData, int nTabNum)
{
    LogPtrValue(pDataName, (LPCTSTR)pData, nTabNum);
    int iNum = 0;
    while (pData[0] != 0 && pData[1] != 0 && pData[2] != 0)
    {
        m_StringBuffer.AddTab(nTabNum + 1).AddF("%s[%d]: ", pDataName, iNum).EndLine();
        while (pData[0] != 0 && pData[1] != 0)
        {
            m_StringBuffer.AddTab(nTabNum + 2).AddF("%S", pData).EndLine();
            pData += wcslen((wchar_t *)pData) + 1;
        }
        pData++;
        iNum++;
    }
}

void CXFSBufferLogger::LogZZZEndString(const char *pDataName, LPCTSTR pData, int nTabNum)
{
    LogPtrValue(pDataName, pData, nTabNum);
    int iNum = 0;
    while (pData[0] != 0 && pData[1] != 0 && pData[2] != 0)
    {
        m_StringBuffer.AddTab(nTabNum + 1).AddF("%s[%d]: ", pDataName, iNum).EndLine();
        while (pData[0] != 0 && pData[1] != 0)
        {
            m_StringBuffer.AddTab(nTabNum + 2).Add(pData).EndLine();
            pData += strlen(pData) + 1;
        }
        pData++;
        iNum++;
    }
}

void CXFSBufferLogger::LogLPVOIDData(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum, int nDataLen)
{
    LogPtrValue(pDataName, pData, nTabNum);
    int nIndex = 0;
    int nCountPerLine = 32;
    try
    {
        for (int i = 0; i < (nDataLen + nCountPerLine - 1) / nCountPerLine; i++)
        {
            m_StringBuffer.AddTab(nTabNum + 1).AddF("%04.4X : ", nIndex);
            for (int j = 0; j < nCountPerLine; j++)
            {
                if (nIndex >= nDataLen)
                    break;
                m_StringBuffer.AddF("%02.2X ", pData[nIndex]);
                nIndex++;
            }
            m_StringBuffer.EndLine();
        }
    }
    catch (...)
    {
        LogException(pDataName, pData);
    }
}

BOOL CXFSBufferLogger::LogStringType(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum)
{
    enum STR_END_TYPE eStrEndType = SET_ZERO_END;

    try
    {
        if (strcmp(pTypeName, "LPSTR") == 0)
        {
            STR_END_TYPE eStrEndType = GetStringEndType(pDataName);
            switch (eStrEndType)
            {
            case SET_ZERO_END:
                LogZeroEndString(pDataName, pData, nTabNum);
                break;

            case SET_ZZ_END:
                LogZZEndString(pDataName, pData, nTabNum);
                break;

            case SET_ZZZ_END:
                LogZZZEndString(pDataName, pData, nTabNum);
                break;

            default:
                assert(false);
                break;
            }
            return TRUE;
        }

        if (strcmp(pTypeName, "LPWSTR") == 0)
        {
            STR_END_TYPE eStrEndType = GetStringEndType(pDataName);
            switch (eStrEndType)
            {
            case SET_ZERO_END:
                LogZeroEndWString(pDataName, (LPCWSTR)pData, nTabNum);
                break;

            case SET_ZZ_END:
                LogZZEndWString(pDataName, (LPCWSTR)pData, nTabNum);
                break;

            case SET_ZZZ_END:
                LogZZZEndWString(pDataName, (LPCWSTR)pData, nTabNum);
                break;

            default:
                assert(false);
                break;
            }
            return TRUE;
        }
    }
    catch (...)
    {
        LogException(pDataName, pData);
        return TRUE;
    }

    return FALSE;
}

void CXFSBufferLogger::LogNullEndPointer(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum)
{
    char sTypeName[MAX_TYPE_NAME_LEN] = {0};
    if(!GetTypeNameOfPointerName(pTypeName,sTypeName))
        return;
    //去掉一重指针后不是双重指针
    //去掉一重指针后一定是指针
    if(TypeNameIsPtrOfPtr(sTypeName))
        return;
    if(!TypeNameIsPtr(sTypeName))
        return;

    if (TypeNameIsPtrOfPtr(sTypeName) ||
        !TypeNameIsPtr(sTypeName))
    {
        m_StringBuffer.AddTab(nTabNum).
        AddF("ERROR: LogNullEndPointer: %s is PtrOfPtr or not ptr(pDataName=%s)",
             sTypeName, pDataName).EndLine();
        return; //本分支一般不会进入，如果进入，一定是配置错误，为防止死循环，增加该分支
    }

    LogPtrValue(pDataName, pData, nTabNum);
    LPCTSTR *ppPtrArray = (LPCTSTR *)pData;
    int nPtrIndex = 0;
    try
    {
        while (*ppPtrArray != NULL)
        {
            char szDataName[MAX_TYPE_NAME_LEN + 20];
            sprintf(szDataName, "%s[%d]", pDataName, nPtrIndex);
            LogPointerType(szDataName, sTypeName, *ppPtrArray, nTabNum + 1);
            ppPtrArray++;
            nPtrIndex++;
        }
    }
    catch (...)
    {
        LogException(pDataName, pData);
    }
}

void CXFSBufferLogger::LogZeroEndPointer(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum)
{
    char sTypeName[MAX_TYPE_NAME_LEN] = {0};
    if(!GetTypeNameOfPointerName(pTypeName,sTypeName))
        return;
    ////以0结束的指针不可以是指向指针的指针，并且是原子数据类型
    if(GetAtomDataLen(sTypeName) <= 0)
        return;
    if(TypeNameIsPtr(sTypeName))
        return;

    if (GetAtomDataLen(sTypeName) == 0 ||
        TypeNameIsPtr(sTypeName))
    {
        m_StringBuffer.AddTab(nTabNum).
        AddF("ERROR: LogZeroEndPointer: len of %s is 0 or is ptr(pDataName=%s)",
             sTypeName, pDataName).EndLine();
        return; //该分支一般不会进入，进入意味着程序错误，为防止错误，增加该分支
    }

    int nArraySize = GetZeroEndPtrArraySize(sTypeName, pDataName, pData);
    assert(nArraySize >= 0);

    LogArray(pDataName, sTypeName, pData, nTabNum, nArraySize);
}

void CXFSBufferLogger::LogPointerType(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum)
{
    assert(TypeNameIsPtr(pTypeName));

    //处理指针为空的情况
    if (pData == nullptr)
    {
        LogPtrValue(pDataName, pData, nTabNum);
        return;
    }

    //处理字串类型
    if (LogStringType(pDataName, pTypeName, pData, nTabNum))
        return;

    //处理指针的数据长度由其他成员指定的情况
    //去掉类型前的LP或后面的*

    char sTypeName[MAX_TYPE_NAME_LEN] = {0};
    if(!GetTypeNameOfPointerName(pTypeName,sTypeName))
        return;

    int nLen = GetPointerMemberDataLen(pDataName);
    if (nLen != -1)   //由其他成员指定其长度
    {
        if (strcmp(pTypeName, "LPVOID") == 0 || strcmp(pTypeName, "VOID") == 0)
        {
            LogLPVOIDData(pDataName, pTypeName, pData, nTabNum, nLen);
            return;
        }
        //m_StringBuffer.AddTab(nTabNum).AddF("%s: 0x%08X", pDataName, pData).EndLine();
        LogArray(pDataName, sTypeName, pData, nTabNum, nLen);
        return;
    }

    //处理ZERO_END_PTR
    char szTemp[MAX_TYPE_NAME_LEN] = {0};
    if(!GetLenDefineName(pDataName,szTemp))
        return;
    if (m_setZeroEndPtrMemName.find(szTemp) != m_setZeroEndPtrMemName.end())
    {
        LogZeroEndPointer(pDataName, pTypeName, pData, nTabNum);
        return;
    }

    //处理NULL_END_PTR: 如果消息的参数是双重指针，一定是以NULL结束
    if (TypeNameIsPtrOfPtr(pTypeName))
    {
        if (m_stackStructCalled.size() == 0 ||
            m_setNullEndMemName.find(szTemp) != m_setNullEndMemName.end())
        {
            LogNullEndPointer(pDataName, pTypeName, pData, nTabNum);
            return;
        }
    }

    //如果指针不是以NULL结束、也没有指定长度，那么一定不是指向指针的指针
    assert(!TypeNameIsPtrOfPtr(pTypeName));
    if (TypeNameIsPtrOfPtr(pTypeName))
    {
        assert(FALSE);
        m_StringBuffer.AddTab(nTabNum).
        AddF("ERROR: LogPointerType: %s is PtrOfPtr(pDataName=%s)",
             pTypeName, pDataName).EndLine();
        return;                 //本分支一般不进入，为防止程序错误造成死循环而增加
    }

    //处理数据为LPVOID类型
    //LPVOID必须指定长度，仅WFSRESULT中lpBuffer会是这种情况，只记录指针地址
    if (strcmp(pTypeName, "LPVOID") == 0 ||
        strcmp(pTypeName, "VOID*") == 0)
    {
        LogPtrValue(pDataName, pData, nTabNum);
        return;
    }

    //原子类型的指针都必须指定长度，如果没有指定，一定是非法数据
    if (GetAtomDataLen(sTypeName) > 0)
    {
        LogArray(pDataName, sTypeName, pData, nTabNum, 1);
        return;
    }

    //其他情况：处理指针指向结构的情况
    LogStruct(pDataName, sTypeName, pData, nTabNum);
}

BOOL CXFSBufferLogger::LogArrayToSingleLine(const char *pTypeName, LPCTSTR pData, int nArraySize)
{
    if (strcasecmp(pTypeName, "char") == 0)
    {
        for (int i = 0; i < nArraySize; i++)
        {
            if (isprint(pData[i]))
                m_StringBuffer.AddF("%c", pData[i]);
            else
                m_StringBuffer.AddF("[%02.2X]", pData[i]);
        }
        return TRUE;
    }
    if (strcasecmp(pTypeName, "BOOL") == 0)
    {
        BOOL *pBOOLData = NULL;
        bool *pboolData = NULL;
        if (strcmp(pTypeName, "BOOL") == 0)
            pBOOLData = (BOOL *)pData;
        else
            pboolData = (bool *)pData;
        for (int i = 0; i < nArraySize; i++)
        {
            if ((pBOOLData != NULL && pBOOLData[i]) || (pboolData != NULL && pboolData[i]))
                m_StringBuffer.Add("TRUE ");
            else
                m_StringBuffer.Add("FALSE ");
        }
        return TRUE;
    }

    if (strcasecmp(pTypeName, "BYTE") == 0)
    {
        for (int i = 0; i < nArraySize; i++)
        {
            m_StringBuffer.AddF("%02.2X ", (BYTE)pData[i]);
        }
        return TRUE;
    }

    return FALSE;
}

void CXFSBufferLogger::LogArray(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum, int nArraySize)
{
    int nLen = GetDataTypeLen(pTypeName);
    //类型名不能又是一个数组
    if(nLen <= 0)
        return;
    if(TypeNameIsArray(pTypeName))
        return;
    //log name
    m_StringBuffer.AddTab(nTabNum).AddF("%s: ", pDataName);

    try
    {
        //处理单行记录的情况
        if (LogArrayToSingleLine(pTypeName, pData, nArraySize))
        {
            m_StringBuffer.EndLine();
            return;
        }

        //循环调用记录每个元素
        LPCTSTR pTemp = pData;
        m_StringBuffer.AddF(" 0x%08.8X", pData).EndLine();
        for (int i = 0; i < nArraySize; i++)
        {
            char szName[200];
            sprintf(szName, "%s[%d]", pDataName, i);  //形成子元素的数据名

            if (TypeNameIsPtr(pTypeName))  //处理指针的情况
            {
                LogPointerType(szName, pTypeName, *(LPCTSTR *)pTemp, nTabNum + 1);
            }
            else
            {
                assert(GetAtomDataLen(pTypeName) > 0);  //应该为原子类型
                LogAtomType(szName, pTypeName, pTemp, nTabNum + 1);
            }

            pTemp += nLen;
        }
    }
    catch (...)
    {
        LogException(pDataName, pData);
    }
}

void CXFSBufferLogger::LogStruct(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum)
{
    //判定必须为结构类型
    if(GetStructLen(pTypeName) <= 0)
        return;
    if(pData == nullptr)
        return;

    //处理结构类型
    STR_NAME2DEF::const_iterator itStr = m_mapStrName2Def.find(pTypeName);
    if(itStr == m_mapStrName2Def.end())
        return;

    const STRUCT_DEFINE *pStrDef = itStr->second;
    m_stackStructCalled.push(pStrDef);
    //记录结构的名字和地址
    LogPtrValue(pDataName, pData, nTabNum);
    m_StringBuffer.AddTab(nTabNum).AddF("%s Structure:", pStrDef->pName).EndLine();
    //记录结构成员
    const STRUCT_MEMBER *pMember = pStrDef->pMembers;
    int nPtrOffset = 0;
    if(pMember == nullptr)
    {
        m_StringBuffer.AddF("pMember == null");
        m_stackStructCalled.pop();
        return;
    }
    while (pMember->pName != NULL)
    {
        if (TypeNameIsArray(pMember->pType))  //处理数组情况
        {
            int nArraySize;
            char sTypeName[MAX_TYPE_NAME_LEN] = {0};
            if(!GetTypeNameOfArrayName(pMember->pType, nArraySize, sTypeName))
                break;
            LogArray(pMember->pName, sTypeName,
                     pData + nPtrOffset, nTabNum + 1, nArraySize);
        }
        else if (TypeNameIsPtr(pMember->pType))  //处理指针的情况
        {
            LPCTSTR pNewData = nullptr;
            try
            {
                pNewData = *(LPCTSTR *)(pData + nPtrOffset);
                if (pNewData == nullptr)
                {
                    LogPtrValue(pMember->pName, pNewData, nTabNum);
//                    m_StringBuffer.AddF("读取(%s)(%s)的数据时发生意外,nPtrOffset(%d)", pMember->pType, pTypeName, nPtrOffset).EndLine();
//                    break;
                }
            }
            catch (...)
            {
                LogException(pDataName, pData);
                break;
            }
            LogPointerType(pMember->pName, pMember->pType, pNewData, nTabNum + 1);
        }
        else
        {
            assert(GetAtomDataLen(pMember->pType) > 0);  //应该为原子数据类型
            LogAtomType(pMember->pName, pMember->pType, pData + nPtrOffset, nTabNum + 1);
        }

        int nLen = GetDataTypeLen(pMember->pType);
        if(nLen <= 0)
            break;
        nPtrOffset += nLen;
        pMember++;
    }
    if(m_stackStructCalled.size() > 0)
        m_stackStructCalled.pop();
}

void CXFSBufferLogger::LogException(const char *pDataName, LPCTSTR pData)
{
    char s[MAX_TYPE_NAME_LEN] = {0};
    if(GetLenDefineName(pDataName, s))
        m_StringBuffer.EndLine().AddF("EXCEPTION: %s=0x%08.8X", s, pData).EndLine();
}
