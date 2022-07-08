#ifndef DEVVDM_H
#define DEVVDM_H

#include "IDevVDM.h"
#include "QtTypeInclude.h"
#include <QProcess>

class CDevVDM : public IDevVDM, public CLogManage
{

public:
    CDevVDM(LPCSTR lpDevType);
    virtual ~CDevVDM();
public:
    // 释放接口
    virtual void Release();
    // 打开连接
    virtual long Open();
    // 关闭连接
    virtual long Close();
    // 复位
    virtual long Reset();
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo);
    // 取状态
    virtual long GetStatus(DEVVDMSTATUS &stStatus);
    // 进入VDM请求
    virtual long EnterVDMReq();
    // 进入VDM
    virtual long EnterVDMAck();
    // 退出VDM请求
    virtual long ExitVDMReq();
    // 退出VDM
    virtual long ExitVDMAck();
private:
    //读取配置文件
    void ReadConfig();
private:
    DEVVDMSTATUS                    m_stDevVdmStatus;

    string                          m_strProgram;               //应用名称或全路径
    string                          m_strArgument;              //命令行参数
    QProcess                        m_vdmPageProcess;
};

#endif // DEVVDM_H
