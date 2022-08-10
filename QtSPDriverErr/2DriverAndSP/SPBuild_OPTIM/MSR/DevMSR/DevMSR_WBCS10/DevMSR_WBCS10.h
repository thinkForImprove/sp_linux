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


#include "IDevIDC.h"
#include "QtTypeInclude.h"
#include "DevImpl_WBCS10.h"
#include "../XFS_MSR/def.h"

#define LOG_NAME_DEV     "DevMSR_WBCS10.log"

/**************************************************************************
*
***************************************************************************/
class CDevMSR_WBCS10: public IDevIDC, public CLogManage, public ConvertVarIDC
{

public:
    CDevMSR_WBCS10();
    ~CDevMSR_WBCS10();

public:
    // 释放接口
    virtual int Release();
    // 打开与设备的连接
    virtual int Open(const char *pMode);
    // 关闭与设备的连接
    virtual int Close();
    // 取消
    virtual int Cancel(unsigned short usMode = 0);
    // 设备复位
    virtual int Reset(MEDIA_ACTION enMediaAct, unsigned short usParam = 0);
    // 取设备状态
    virtual int GetStatus(STDEVIDCSTATUS &stStatus);
    // 介质控制
    //virtual int MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam = 0);
    // 介质读写
    virtual int MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData);
    // 芯片读写
    //virtual int ChipReadWrite(CHIP_RW_MODE enChipMode, STCHIPRW &stChipData);
    // 设置数据
    virtual int SetData(unsigned short usType, void *vData = nullptr);
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData);
    // 获取版本
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize);

private: // 数据处理
    INT lErrCodeChgToIDC(LONG lDevCode);   // WBCS10错误码转换为DevIDC错误码

private:
    CDevImpl_WBCS10     m_pDevWBCS10;      // 设备硬件调用类变量
    STDEVIDCSTATUS      m_stStatusOLD;     // 保存上一次设备状态
    CSimpleMutex        m_cMutex;

    STMSRDEVINITPARAM   m_stMsrDevInitParam;
    BOOL                m_bReCon;          // 是否断线重连状态
    INT                 m_nRetErrOLD[8];   // 处理错误值保存


};

#endif // DEVMSR_WBCS10_H
