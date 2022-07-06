#pragma once
#include "QtTypeDef.h"
struct ICashUnitConfig;
struct ICashUnitConfigItem;

enum CONFIG_TYPE
{
    CT_LOG_CDM = 0,
    CT_LOG_CIM = 1,
    CT_LOG_PH = 2,
};

struct /*_declspec(novtable)*/ ICashUnitConfigItem
{
    //得到配置文件对象
    virtual ICashUnitConfig *GetConfig() = 0;

    //get value
    virtual const char *GetCUValue(const char *pDefault, const char *pValueName) const = 0;
    virtual int GetCUValue(int nDefault, const char *pValueName) const = 0;

    //set value
    //0 success; <0 failure
    virtual int SetCUValue(const char *pDefault, const char *pValueName) = 0;
    virtual int SetCUValue(int nValue, const char *pValueName) = 0;
};

struct /*_declspec(novtable)*/ ICashUnitConfig
{
    //装载配置文件
    virtual int Load(const char *pFileName) = 0;

    //保存配置文件
    virtual int Save() = 0;

    //备份配置文件
    virtual BOOL BackUp(BOOL bLoadsucc) = 0;

    //得到第dwIndex个钞箱配置，不存在返回NULL
    //ct, 配置类型
    virtual ICashUnitConfigItem *GetCUConfig(CONFIG_TYPE ct, DWORD dwIndex) = 0;

    //得到钞箱的配置个数
    virtual DWORD GetCUConfigCount(CONFIG_TYPE ct) const = 0;

    //增加配置项
    virtual ICashUnitConfigItem *AddCUConfig(CONFIG_TYPE ct) = 0;

    //删除配置项
    virtual void DeleteCUConfig(CONFIG_TYPE ct, ICashUnitConfigItem *pItem) = 0;

    //得到公共的钞箱配置信息，不存在返回NULL
    virtual ICashUnitConfigItem *GetCommonConfig() = 0;
};

