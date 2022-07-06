#ifndef XFSBUFFERLOGGER_H
#define XFSBUFFERLOGGER_H
#include <QtCore/qglobal.h>
#include "XFSDataDesc.h"
#include "StringBuffer.h"

#include <set>
#include <map>
#include <stack>
#include <string>
#include <string.h>

using namespace std;

#define MAX_VALUE_DESC_LEN  (4096)
#define MAX_TYPE_NAME_LEN   (512)

//----------------------- 类型定义 ---------------------
typedef ULONG                               MSG_ID;             //消息唯一ID，＝MakeMsgID(eType, dwID)

typedef map<int, const char *>                CONST_VALUE2NAME; //常量值到名字，即常量组
typedef map<string, CONST_VALUE2NAME>        CONST_TYPE2GROUP;  //常量类型名到常量组

typedef map<string, const STRUCT_DEFINE *>    STR_NAME2DEF;     //结构名字到结构定义
typedef STR_NAME2DEF                        STR_ALIAS2DEF;      //结构别名到结构定义

typedef map<MSG_ID, const MSG2STRUCT *>       MSGID2MSG_STR;    //消息ID到消息结构定义

typedef map<string, const char *>             MEM_NAME2LEN_DEF; //成员名到长度

typedef map<string, int>                     ATOM_TYPE2LEN;     //原子类型名到长度

typedef map<string, int>                     MEM_NAME2VALUE;    //从成员名到值的映射

typedef set<string>                         NULL_END_MEM_NAME_SET;//以NULL结束的成员名集合

typedef set<string>                         ZERO_END_MEM_NAME_SET;//以0结束的成员名集合

typedef set<string>                         ZERO_ZERO_END_MEM_NAME_SET;//以\0\0结束的成员名集合

typedef set<string>                         ZERO_ZERO_ZERO_END_MEM_NAME_SET;//以\0\0\0结束的成员名集合

typedef stack<const STRUCT_DEFINE *>         STRUCT_STACK;      //结构栈

typedef unsigned short      *LPCWSTR;

//字串结束类型枚举
enum STR_END_TYPE
{
    SET_ZERO_END = 0,
    SET_ZZ_END,
    SET_ZZZ_END
};

//形成消息唯一ID
inline MSG_ID MakeMsgID(MSGTYPE eType, DWORD dwID)
{
    return (MSG_ID)(((ULONG)eType) * 10000 + dwID);
}

//记录XFS结果中lpBuffer的辅助类
class CXFSBufferLogger
{
public:
    CXFSBufferLogger(CStringBuffer &StringBuffer);
    virtual ~CXFSBufferLogger();

    //转换常量值为字串
    //如未找到，返回NULL
    const char *ConstValue2Str(const char *pType, int nValue) const;

    //形成日志字串
    //返回以\0结束的字串
    void Log(MSGTYPE eMsgType, DWORD dwID, const char *pDataName, LPCTSTR pData, int nTabNum = 1);

    //得到消息类型描述
    //eMsgType：消息类型
    //返回：MT_EVENT、MT_GI、MT_GC、MT_EX、MT_EC、其他为数字
    static const char *GetMsgTypeDesc(MSGTYPE eMsgType);

    //----------------------- 私有函数 -----------------------------------------------
private:
    //结构类数据类型的记录函数
    void LogStruct(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum);

    //指针类数据类型的记录函数
    void LogLPVOIDData(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum, int nDataLen);
    void LogPointerType(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum);
    void LogNullEndPointer(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum);
    void LogZeroEndPointer(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum);

    //数组类数据类型的记录函数
    void LogArray(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum, int nArraySize);
    BOOL LogArrayToSingleLine(const char *pTypeName, LPCTSTR pData, int nArraySize);

    //原子类数据类型的记录函数
    void LogAtomType(const char *pDataName, const char *pTypeName, LPCTSTR pData, int nTabNum);
    void LogWORDOrDWORDValue(const char *pMemberFullName, int nValue, BOOL bWORD);

    //字串记录函数
    BOOL LogStringType(const char *pDataName, const char *pTypeName,
                       LPCTSTR pData, int nTabNum);  //返回FALSE表示不为字串
    void LogZeroEndString(const char *pDataName, LPCTSTR pData, int nTabNum);
    void LogZZEndString(const char *pDataName, LPCTSTR pData, int nTabNum);
    void LogZZZEndString(const char *pDataName, LPCTSTR pData, int nTabNum);
    void LogZeroEndWString(const char *pDataName, LPCWSTR pData, int nTabNum);
    void LogZZEndWString(const char *pDataName, LPCWSTR pData, int nTabNum);
    void LogZZZEndWString(const char *pDataName, LPCWSTR pData, int nTabNum);

    //得到指针类结构成员的数据长度
    //如果没有指定长度，返回-1；否则，返回长度
    int GetPointerMemberDataLen(const char *pMemberName) const;

    //得到结构的数据长度
    int GetStructLen(const char *pTypeName) const;

    //得到原子类型数据的长度，原子类型如：BOOL、BYTE、CHAR、SHORT、LONG、WORD等
    int GetAtomDataLen(const char *pTypeName) const;

    //得到数据类型长度
    int GetDataTypeLen(const char *pTypeName) const;

    //转换结构成员的值为描述
    //返回值：NULL，未定义描述
    const char *ConvertMemberValue2Desc(const char *pMemberFullName, int nValue) const;

    //得到结构成员的全名
    bool GetMemberFullName(const char *pMemberName, char szFullName[MAX_TYPE_NAME_LEN]) const;

    //得到消息ID的名字
    const char *GetNameOfMsgID() const; //得到消息ID的名字

    //得到字串的结束类型
    STR_END_TYPE GetStringEndType(const char *pDataName) const;

    //得到长度定义的名字
    //如果是顶级指针，返回消息名字，如“WFS_CDM_GET_STATUS”或消息返回名“WFS_CDM_GET_STATUS_RET”
    //如果是结构成员，返回结构成员的全名
    bool GetLenDefineName(const char *pDataName, char sLenDefName[MAX_TYPE_NAME_LEN]) const;

    //得到以0结束的指针的数组尺寸
    //pNonPtrTypeName：非指针的类型名
    int GetZeroEndPtrArraySize(const char *pNonPtrTypeName, const char *pDataName, LPCTSTR pData);

    //记录一行指针的值
    inline void LogPtrValue(const char *pDataName, LPCTSTR pData, int nTabNum)
    {
        m_StringBuffer.AddTab(nTabNum).AddF("%s: 0x%08.8X", pDataName, pData).EndLine();
    }

    //记录访问数据发生了异常
    void LogException(const char *pDataName, LPCTSTR pData);
    //----------------------- 私有成员 -----------------------------------------------
private:
    CONST_TYPE2GROUP        m_mapConstType2Group;       //常量类型名到常量组
    STR_NAME2DEF            m_mapStrName2Def;           //结构名字到结构定义
    MSGID2MSG_STR           m_mapMsgID2MsgStr;          //消息ID到消息结构定义
    NULL_END_MEM_NAME_SET   m_setNullEndMemName;        //以NULL结束的成员名集合
    ZERO_END_MEM_NAME_SET   m_setZeroEndPtrMemName;     //以0结束的成员名集合
    ZERO_ZERO_END_MEM_NAME_SET m_setZeroZeroEndStrMemName;  //以\0\0结束的成员名集合
    ZERO_ZERO_ZERO_END_MEM_NAME_SET m_setZeroZeroZeroEndStrMemName; //以\0\0\0结束的成员名集合
    ATOM_TYPE2LEN           m_mapAtomType2Len;          //基本数据类型名字到长度的映射
    MEM_NAME2LEN_DEF        m_mapMemName2LenDef;        //成员名到长度定义字段
    MEM_NAME2VALUE          m_mapLenMemName2Values;     //长度定义字段到长度值的映射

    //运行数据
    STRUCT_STACK            m_stackStructCalled;        //调用的结构栈
    CStringBuffer           &m_StringBuffer;            //字串缓冲区
    MSGTYPE                 m_eMsgType;                 //事件类型
    DWORD                   m_dwMsgID;                  //消息ID
};


#endif // XFSBUFFERLOGGER_H
