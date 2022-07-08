#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "string.h"

#if defined(IDEVBCR_LIBRARY)
#  define DEVBCR_EXPORT Q_DECL_EXPORT
#else
#  define DEVBCR_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
enum DEVICE_STATUS
{
    DEVICE_OFFLINE = 0,
    DEVICE_ONLINE  = 1,
    DEVICE_HWERROR = 2,
};
//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_bcr_status
{
    WORD wDevice;               // 设备状态
    char szErrCode[8];          // 三位的错误码

    tag_dev_bcr_status() { clear(); }
    void clear() { memset(this, 0x00, sizeof(tag_dev_bcr_status)); }
} DEVBCRSTATUS;

//////////////////////////////////////////////////////////////////////////
struct IDevBCR
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
    virtual long GetStatus(DEVBCRSTATUS &stStatus) = 0;
    // 读取二维码
    virtual long ReadBCR(DWORD &dwType, LPSTR lpData, DWORD &dwLen, long lTimeOut) = 0;
    // 取消读取二维码
    virtual long CancelRead() = 0;
};

extern "C" DEVBCR_EXPORT long CreateIDevBCR(LPCSTR lpDevType, IDevBCR *&pDev);
//////////////////////////////////////////////////////////////////////////
