#pragma once
#include "IDevDEP.h"
#include "IAllDevPort.h"
#include "QtTypeInclude.h"
//////////////////////////////////////////////////////////////////////////
#define DEF_TIMEOUT             (2*1000)
#define ACR500_REPLY_LEN        0x12
//////////////////////////////////////////////////////////////////////////
class CDevDEP_ZDGC : public IDevDEP, public CLogManage
{
public:
    CDevDEP_ZDGC(LPCSTR lpDevType);
    virtual ~CDevDEP_ZDGC();
public:
    // 释放接口
    virtual void Release();
    // 打开连接
    virtual long Open(LPCSTR lpMode);
    // 关闭连接
    virtual long Close();
    // 复位
    virtual long Reset();
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo);
    // 取状态
    virtual long GetStatus(DEVDEPSTATUS &stStatus);
    // 开门
    virtual long OpenShutter();
    // 关门
    virtual long CloseShutter();
protected:
    // 更新状态和错误码
    void UpdateStatus(WORD wDevice, std::string strErrCode);
    // 记录收取数据日志
    void RecordRecvLog(LPSTR pData, int nLen);
    // 取款闸门停止动作
    long StopShutter();

    long SendAndReadCmd(LPCSTR lpSendData, int iSendSize, LPSTR lpRecvData,
                        DWORD &dwInOutData, int iTimeOut = DEF_TIMEOUT);
private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IAllDevPort>   m_pDev;
    std::string                 m_strOpenMode;
    DEVDEPSTATUS                m_stStatus;
    bool                        m_bReading;
    CQtSimpleMutexEx            m_cSysMutex;
};

//////////////////////////////////////////////////////////////////////////

