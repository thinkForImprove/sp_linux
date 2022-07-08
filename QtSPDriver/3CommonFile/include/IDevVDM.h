#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "string.h"

#if defined(IDEVVDM_LIBRARY)
#  define DEVVDM_EXPORT Q_DECL_EXPORT
#else
#  define DEVVDM_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
//错误码说明：
//------------------------ 错误码定义 ----------------------------------
#define ERR_VDM_SUCCESS                (0)     // 成功


enum DEVICE_STATUS
{
    DEVICE_OFFLINE = 0,
    DEVICE_ONLINE  = 1
};
//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_vdm_status
{
    WORD wDevice;               // 设备状态
    char szErrCode[8];          // 三位的错误码

    tag_dev_vdm_status() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_dev_vdm_status)); }
} DEVVDMSTATUS;

//////////////////////////////////////////////////////////////////////////
struct IDevVDM
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
    virtual long Open() = 0;
    // 关闭连接
    virtual long Close() = 0;
    // 复位
    virtual long Reset() = 0;
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo) = 0;
    // 取状态
    virtual long GetStatus(DEVVDMSTATUS &stStatus) = 0;
    // 进入VDM请求
    virtual long EnterVDMReq() = 0;
    // 进入VDM
    virtual long EnterVDMAck() = 0;
    // 退出VDM请求
    virtual long ExitVDMReq() = 0;
    // 退出VDM
    virtual long ExitVDMAck() = 0;
};

extern "C" DEVVDM_EXPORT long CreateIDevVDM(LPCSTR lpDevType, IDevVDM *&pDev);
//////////////////////////////////////////////////////////////////////////
