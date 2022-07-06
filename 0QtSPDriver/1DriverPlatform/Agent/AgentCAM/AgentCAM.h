#ifndef AGENTCAM_H
#define AGENTCAM_H

#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSCAM.H"

class CAgentCAM : public IAgentBase, public CLogManage
{
public:
    CAgentCAM();
    virtual ~CAgentCAM();
public:
    virtual void Release();
    // 实现命令数据拷贝
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
protected:
    // 查询的
    HRESULT Get_WFS_INF_CAM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CAM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_CAM_TAKE_PICTURE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CAM_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CAM_TAKE_PICTURE_EX(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CAM_DISPLAY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
private:
    // 加载库
    bool LoadDll();
private:
    char m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

#endif // AGENTCAM_H
