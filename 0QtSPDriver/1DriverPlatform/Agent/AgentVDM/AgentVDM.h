#ifndef AGENTVDM_H
#define AGENTVDM_H
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSVDM.H"
//////////////////////////////////////////////////////////////////////////
class CAgentVDM : public IAgentBase, public CLogManage
{
public:
    CAgentVDM();
    virtual ~CAgentVDM();
public:
    virtual void Release();
    // 实现命令数据拷贝
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
protected:
    // 查询的
    HRESULT Get_WFS_INF_VDM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_VDM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    //　执行的
    HRESULT Exe_WFS_CMD_VDM_ENTER_MODE_REQ(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_VDM_ENTER_MODE_ACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_VDM_EXIT_MODE_REQ(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_VDM_EXIT_MODE_ACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
private:
    // 加载库
    bool LoadDll();
private:
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

#endif // AGENTVDM_H
