#pragma once

//////////////////////////////////////////////////////////////////////////
#define DVEPDL_NO_VTABLE  __declspec(novtable)
//////////////////////////////////////////////////////////////////////////
// 错误码
#define PDL_OK                              (0)     // PDL成功
#define PDL_OPEN_ERR                        (-1)    // 打开设备连接失败
#define PDL_DEV_ERR                         (-2)    // 设备异常失败
#define PDL_CONFIG_ERR                      (-3)    // PDL配置错误，或没有找到配置文件
#define PDL_FILE_ERR                        (-4)    // PDL文件错误，或没有找到PDL文件
#define PDL_XOR_ERR                         (-5)    // 异或值校验失败
#define PDL_MD5_ERR                         (-6)    // MD5值校验失败
#define PDL_SWITCH_ERR                      (-7)    // 切换PDL模式失败
#define PDL_SENDHEX_ERR                     (-8)    // 发送固件数据失败
#define PDL_ERR                             (-9)    // 升级固件失败
//////////////////////////////////////////////////////////////////////////
typedef const char *LPCSTR;
//////////////////////////////////////////////////////////////////////////
struct DVEPDL_NO_VTABLE IDevPDL
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
    virtual long Open(LPCSTR lpMode) = 0;
    // 关闭连接
    virtual long Close() = 0;
    // 是否需要升级
    virtual bool IsNeedDevPDL() = 0;
    // 执行升级
    virtual long UpdateDevPDL() = 0;
    // 测试基本功能是否正常
    virtual long TestDevPDL() = 0;
};

extern "C" long CreateIDevPDL(LPCSTR lpDevType, IDevPDL *&p);

//////////////////////////////////////////////////////////////////////////