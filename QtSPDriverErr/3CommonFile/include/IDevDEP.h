#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "string.h"

#if defined(IDEVDEP_LIBRARY)
#  define DEVDEP_EXPORT Q_DECL_EXPORT
#else
#  define DEVDEP_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
enum DEVICE_STATUS
{
    DEVICE_OFFLINE = 0,
    DEVICE_ONLINE  = 1,
    DEVICE_BUSY    = 2,
    DEVICE_HWERROR = 3,
};
enum SHUTTER_STATUS
{
    SHUTTER_DEP_SHTCLOSED = 0,
    SHUTTER_DEP_SHTOPEN,
    SHUTTER_DEP_SHTJAMMED,
    SHUTTER_DEP_SHTUNKNOWN,
    SHUTTER_DEP_SHTNOTSUPP,
};
enum DEPCONTAINER_STATUS
{
    CONTAINER_DEP_DEPOK = 0,
    CONTAINER_DEP_DEPHIGH,
    CONTAINER_DEP_DEPFULL,
    CONTAINER_DEP_DEPINOP,
    CONTAINER_DEP_DEPMISSING,
    CONTAINER_DEP_DEPUNKNOWN,
    CONTAINER_DEP_DEPNOTSUPP,
};

//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_dep_status
{
    WORD wDevice;               // 设备状态
    WORD wShutterOpen;          // 闸门位置
    WORD wDepContainer;         // 存放箱状态
    char szErrCode[8];          // 三位的错误码
    char szVerInfo[8];         // 版本号

    tag_dev_dep_status() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_dev_dep_status)); }
} DEVDEPSTATUS;

//////////////////////////////////////////////////////////////////////////
struct IDevDEP
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
    virtual long Open(LPCSTR lpMode) = 0;
    // 关闭连接
    virtual long Close() = 0;
    // 复位
    virtual long Reset() = 0;
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo) = 0;
    // 取状态
    virtual long GetStatus(DEVDEPSTATUS &stStatus) = 0;
    // 开门
    virtual long OpenShutter() = 0;
    // 关门
    virtual long CloseShutter() = 0;
};

extern "C" DEVDEP_EXPORT long CreateIDevDEP(LPCSTR lpDevType, IDevDEP *&pDev);
//////////////////////////////////////////////////////////////////////////
