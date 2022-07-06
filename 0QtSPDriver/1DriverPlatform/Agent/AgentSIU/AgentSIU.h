#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSSIU.H"
//////////////////////////////////////////////////////////////////////////
class CAgentSIU : public IAgentBase, public CLogManage
{
public:
    CAgentSIU();
    virtual ~CAgentSIU();
public:
    virtual void Release();
    // 实现命令数据拷贝
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
protected:
    // 查询的
    HRESULT Get_WFS_INF_SIU_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_SIU_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_SIU_ENABLE_EVENTS(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_SIU_SET_DOOR(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_SIU_SET_INDICATOR(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_SIU_SET_GUIDLIGHT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_SIU_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
private:
    // 加载库
    bool LoadDll();
private:
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

