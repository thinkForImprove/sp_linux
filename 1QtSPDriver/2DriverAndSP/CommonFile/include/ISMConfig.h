#pragma once
#include "QtTypeDef.h"
struct ISMConfig;
struct ISMConfigNode;

//属性对象
struct ISMConfigAttrib
{
    //读取下一个属性，不存在时返回NULL
    virtual ISMConfigAttrib *NextAttrib() = 0;

    //读取前一个属性，不存在时返回NULL
    virtual ISMConfigAttrib *PrevAttrib() = 0;

    //得到属性的名字
    virtual const char *GetName() = 0;

    //设置属性的名字
    virtual void SetName(const char *pName) = 0;

    //得取属性的值
    virtual const char *GetValue() = 0;

    //设置属性的值
    virtual void SetValue(const char *pValue) = 0;

    //得到属性的整数值，如无法转换，返回FALSE
    virtual BOOL GetInt(int *pValue) = 0;

    //得到属性的
    virtual BOOL GetDouble(double *pValue) = 0;
};

//配置节点对象
struct ISMConfigNode
{
    //得到节点的配置文件对象
    virtual ISMConfig *GetConfig() = 0;

    //得到父节点
    virtual ISMConfigNode *GetParent() = 0;

    //得到节点的TAG名
    virtual const char *GetTagName() = 0;

    //查找第一个子节点，如无，返回NULL
    virtual ISMConfigNode *FirstChildNode() = 0;

    //查找第一个指定TAG的子节点，如无，返回NULL
    virtual ISMConfigNode *FirstChildNode(const char *pszTag) = 0;

    //查找最后一个子节点，如无，返回NULL
    virtual ISMConfigNode *LastChildNode() = 0;

    //查找最后一个指定TAG的子节点，如无，返回NULL
    virtual ISMConfigNode *LastChildNode(const char *pszTag) = 0;

    //下一个兄弟节点，如无，返回NULL
    virtual ISMConfigNode *NextSiblingNode() = 0;

    //下一个指定TAG的兄弟节点，如无，返回NULL
    virtual ISMConfigNode *NextSiblingNode(const char *pszTag) = 0;

    //前一个兄弟节点，如无，返回NULL
    virtual ISMConfigNode *PrevSiblingNode() = 0;

    //前一个指定TAG的兄弟节点，如无，返回NULL
    virtual ISMConfigNode *PrevSiblingNode(const char *pszTag) = 0;

    //在pChild之前插入一个指定TAG的节点，返回插入的节点
    virtual ISMConfigNode *InsertBefore(ISMConfigNode *pChild, const char *pszTag) = 0;

    //在pChild之后插入一个指定TAG的节点，返回插入的节点
    virtual ISMConfigNode *InsertAfter(ISMConfigNode *pChild, const char *pszTag) = 0;

    //在本节点的所有子节点之后插入一个指定TAG的节点，返回插入的节点
    virtual ISMConfigNode *InsertEnd(const char *pszTag) = 0;

    //删除指定子节点
    virtual BOOL RemoveChildNode(ISMConfigNode *pNode) = 0;

    //使节点离开父节点成为单独节点，维护该节点是应用的责任
    //!!! 必须调用InsertCloneXXX或DeleteClone，否则会出现内存泄漏
    virtual BOOL Detach() = 0;

    //在pChild之前插入克隆节点，插入完成后不可调用DeleteClone删除pClone
    virtual BOOL InsertCloneBefore(ISMConfigNode *pChild, ISMConfigNode *pClone) = 0;

    //在pChild之后插入克隆节点，插入完成后不可调用DeleteClone删除pClone
    virtual BOOL InsertCloneAfter(ISMConfigNode *pChild, ISMConfigNode *pClone) = 0;

    //在本节点的所有子节点之后插入克隆节点，插入完成后不可调用DeleteClone删除pClone
    virtual BOOL InsertCloneEnd(ISMConfigNode *pClone) = 0;

    //删除调用CloneNode克隆的节点
    virtual void DeleteClone() = 0;

    //克隆节点
    //注意：!!!! 应用有责任维护克隆的节点，使用完后应调用DeleteClone删除或者InsertCloneXXX系列函数!!!!
    virtual ISMConfigNode *CloneNode() = 0;

    //得到第一个属性对象，如无，返回NULL
    virtual ISMConfigAttrib *FirstAttrib() = 0;

    //得到最后一个属性对象，如无，返回NULL
    virtual ISMConfigAttrib *LastAttrib() = 0;

    //得到指定名字的属性的值，如无，返回NULL
    virtual const char *GetAttrib(const char *pszName) = 0;

    //得到整数属性值，如无或无法转换，返回FALSE
    virtual BOOL GetAttrib(const char *pszName, int *pValue) = 0;

    //得到浮点属性值，如无或无法转换，返回FALSE
    virtual BOOL GetAttrib(const char *pszName, double *pValue) = 0;

    //删除指定名字的属性
    virtual void RemoveAttrib(const char *pName) = 0;

    //设置属性，如指定名字的属性不存在，先创建它
    virtual void SetAttrib(const char *name, const char *pValue) = 0;

    //设置属性，如指定名字的属性不存在，先创建它
    virtual void SetAttrib(const char *name, int nValue) = 0;

    //得到节点的内容
    virtual const char *GetContent() = 0;

    //设置节点的内容
    virtual BOOL SetContent(const char *pContent) = 0;

    //得到使用SetUserData设置的用户数据
    virtual DWORD GetUserData() const = 0;

    //设置用户数据
    virtual void SetUserData(DWORD dwNew) = 0;
};

//配置文件对象
struct ISMConfig
{
    //释放配置文件对象
    virtual void Release() = 0;

    //装载配置文件
    virtual BOOL Load(const char *pszFileName) = 0;

    //保存配置文件
    virtual BOOL Save(const char *pszFileName) = 0;

    //测试装载或保存是否成功
    virtual BOOL Success() = 0;

    //创建新的配置文件对象
    virtual ISMConfigNode *NewFile(const char *pRootTag) = 0;

    //得到错误描述
    virtual const char *GetErrDesc() = 0;

    //得到错误的位置
    virtual void GetErrRowCol(int *pRow, int *pCol) = 0;

    //得到根节点
    virtual ISMConfigNode *GetRootNode() = 0;
};

//创建配置文件对象
extern "C" ISMConfig *SMCreateConfig();

