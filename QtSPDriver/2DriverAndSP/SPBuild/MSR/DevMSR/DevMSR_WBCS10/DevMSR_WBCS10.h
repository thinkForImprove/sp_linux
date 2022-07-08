/***************************************************************
* 文件名称：DevMSR_WBCS10.h
* 文件描述：刷折模块功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEVMSR_WBCS10_H
#define DEVMSR_WBCS10_H


#include "IDevMSR.h"
#include "QtTypeInclude.h"
#include "DevImpl_WBCS10.h"
#include "../XFS_MSR/def.h"

#define LOG_NAME_DEV     "DevMSR_WBCS10.log"

class CDevMSR_WBCS10: public IDevMSR, public CLogManage
{

public:
    CDevMSR_WBCS10();
    ~CDevMSR_WBCS10();

public:
    // 释放接口
    virtual void Release();
    // 功能：打开与设备的连接
    virtual int Open(const char *pMode);
    // 设备初始化
    virtual int Reset();
    // 关闭与设备的连接
    virtual void Close();
    // 取设备状态
    virtual int GetStatus(DEVMSRSTATUS &stStatus);
    // 读卡
    virtual int ReadTracks(STTRACK_INFO &stTrackInfo, INT nTimeOut);
    // 命令取消
    virtual int Cancel();
    // 设置数据
    virtual int SetData(void *vData, WORD wDataType = 0);
    // 获取数据
    virtual int GetData(void *vData, WORD wDataType = 0);

public:
    // 版本号(1DevCam版本/2固件版本/3其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType);

private: // 数据处理
    // WBCS10错误码转换为DevMSR错误码
    LONG lErrCodeChgToMSR(LONG lDevCode);


private:
    CDevImpl_WBCS10     m_pDevWBCS10;      // 设备硬件调用类变量
    DEVMSRSTATUS        m_stOldStatus;     // 保存上一次设备状态
    CSimpleMutex        m_cMutex;

    STMSRDEVINITPARAM   m_stMsrDevInitParam;
    BOOL                m_bReCon;          // 是否断线重连状态
    INT                 m_nRetErrOLD[8];   // 处理错误值保存


};

#endif // DEVMSR_WBCS10_H
