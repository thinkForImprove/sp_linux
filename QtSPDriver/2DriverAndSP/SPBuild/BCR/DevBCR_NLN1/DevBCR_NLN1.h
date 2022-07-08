#pragma once
#include "IDevBCR.h"
#include "IAllDevPort.h"
#include "QtTypeInclude.h"
//////////////////////////////////////////////////////////////////////////
#define DEF_TIMEOUT             (2*1000)
//////////////////////////////////////////////////////////////////////////
class CAutoSetReadingStatus
{
public:
    CAutoSetReadingStatus(bool *pbReading) : m_pbReading(pbReading) { if (m_pbReading != nullptr) *m_pbReading = true; }
    ~CAutoSetReadingStatus() { if (m_pbReading != nullptr) *m_pbReading = false; }
private:
    bool *m_pbReading;
};
//////////////////////////////////////////////////////////////////////////
class CDevBCR_NLN1 : public IDevBCR, public CLogManage
{
public:
    CDevBCR_NLN1(LPCSTR lpDevType);
    virtual ~CDevBCR_NLN1();
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
    virtual long GetStatus(DEVBCRSTATUS &stStatus);
    // 读取二维码
    virtual long ReadBCR(DWORD &dwType, LPSTR lpData, DWORD &dwLen, long lTimeOut);
    // 取消读取二维码
    virtual long CancelRead();
protected:
    // 取消读取命令
    long CancelReadBCRCmd();
    // 更新状态和错误码
    void UpdateStatus(WORD wDevice, std::string strErrCode);
private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IAllDevPort>   m_pDev;
    std::string                 m_strOpenMode;
    DEVBCRSTATUS                m_stStatus;
    bool                        m_bReading;
};

//////////////////////////////////////////////////////////////////////////

