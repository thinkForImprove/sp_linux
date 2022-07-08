#ifndef XFSDATADESC_H
#define XFSDATADESC_H

//#include "XFSADMIN.H"
#include "XFSAPI.H"
#include "XFSCDM.H"
#include "XFSCIM.H"
#include "XFSCIM310.h"
#include "XFSIDC.H"
#include "XFSPIN.H"
#include "XFSPTR.H"
#include "XFSSIU.H"
#include "XFSTTU.H"
#include "XFSVDM.H"
#include "XFSBCR.H"
#include "IWFMShareMenory.h"
#include "XFSBCR.H"
#include "XFSPINCHN.H"                      //30-00-00-00(FS#0003)
#include "XFSCAM.H"
#include "XFSCRD.H"

typedef const char    *LPCTSTR;

//常量定义宏
#define BEGIN_CONST_DEF(Const_Type)     static const CONST_DEF g_const##Const_Type[] = {
#define END_CONST_DEF()                 {NULL, 0} };
#define DEF_CONST(cname)                {#cname, (int)cname},

//常量表定义宏
#define BEGIN_CONST_TABLE()             static const CONST_TABLE_ITEM g_ConstTable[] = {
#define END_CONST_TABLE()               {NULL, NULL}};
#define CONST_TABLE(Const_Type)         {#Const_Type, g_const##Const_Type},

//结构定义宏
#define BEGIN_STRUCT_DEF(StructType)    static const STRUCT_MEMBER g_struct##StructType[] = {
#define END_STRUCT_DEF()                {NULL, NULL} };
#define DEF_MEMBER(itemname, itemtype)  {itemname, itemtype},

//结构表定义
#define BEGIN_STRUCT_TABLE()            static const STRUCT_DEFINE g_StructTable[] = {
#define END_STRUCT_TABLE()              {NULL, NULL} };
#define STRUCT_ITEM(StructType)         {#StructType, g_struct##StructType},

//消息到结构关系定义宏
#define BEGIN_MSG_STRUCT_DEF()          static const MSG2STRUCT g_Msg2Structs[] = {
#define END_MSG_STRUCT_DEF()            {MT_EVENT, 0, NULL, NULL} };
#define DEF_MSG(eType, dwID, pTypeName, pDataName)  {eType, dwID, pTypeName, pDataName},

//结构成员长度定义宏
#define BEGIN_MEMBER_LEN_DEF()          static const MEMBER_LEN g_MemberLenDefTable[] = {
#define END_MEMBER_LEN_DEF()            {NULL, NULL} };
#define DEF_LEN(pMemberName, pLenName)  {pMemberName, pLenName},

//说明：
//Const_Type：对于结构成员的常量定义，格式为“TypeName___成员名”(不要加引号)，
//  如“WFSCDMSTATUS”结构的“fwDevice”成员，该值为“WFSCDMSTATUS___fwDevice”
//  其他固定值如下宏定义(注意：传给CONST_TABLE和STRUCT_TABLE宏时不要加引号)
#define CONST_TYPE_EXECUTE      "EXECUTE_CMD"
#define CONST_TYPE_GET_INFO     "GETINFO_CMD"
#define CONST_TYPE_MESSAGE      "MESSAGE_CODE"
#define CONST_TYPE_ERROR_CODE   "ERROR_CODE"
#define CONST_TYPE_OTHER        "OTHER_CONST"

//常量定义
struct CONST_DEF
{
    const char *pName;  //常量名
    int nValue;         //常量值
};

//常量表ITEM定义
struct CONST_TABLE_ITEM
{
    const char *pName;      //常量表ITEM名
    const CONST_DEF *pDef;  //常量定义
};

//结构成员
struct STRUCT_MEMBER
{
    const char *pName;  //成员名
    const char *pType;  //成员类型
};

//结构定义
struct STRUCT_DEFINE
{
    const char *pName;  //结构名
    const STRUCT_MEMBER *pMembers; //成员变量定义
};

//结构成员长度指定
struct MEMBER_LEN
{
    const char *pName;  //结构成员名，格式为“TypeName___成员名”(不要加引号)
    const char *pLenDef;//长度定义字段，格式为“TypeName___成员名”(不要加引号)
};

typedef enum
{
    MT_EVENT = 0,   //消息：系统、用户、服务、执行
    MT_GI,          //GET INFO
    MT_GC,          //GET INFO返回
    MT_EX,          //EXECUTE
    MT_EC,          //EXECUTE返回
} MSGTYPE;

//消息与数据类型定义
struct MSG2STRUCT
{
    MSGTYPE eMsgType;           //消息类型
    DWORD   dwID;               //命令或消息ID
    const char *pTypeName;  //结构名
    const char *pDataName;      //数据名
};

//查找常量的描述
const  CONST_TABLE_ITEM *GetConstTable();
const STRUCT_DEFINE *GetStructDefine();
const MSG2STRUCT *GetMsg2Struct();
const MEMBER_LEN *GetMemberLenDef();

#endif // XFSDATADESC_H
