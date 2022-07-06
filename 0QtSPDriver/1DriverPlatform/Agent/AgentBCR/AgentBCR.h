#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSBCR.H"

//////////////////////////////////////////////////////////////////////////
class CAgentBCR : public IAgentBase, public CLogManage
{
public:
    CAgentBCR();
    virtual ~CAgentBCR();
public:
    // 实现命令数据拷贝
    virtual void Release();
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
protected:
    // 查询的
    HRESULT Get_WFS_INF_BCR_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_BCR_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_BCR_READ(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_BCR_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_BCR_SET_GUIDANCE_LIGHT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_BCR_POWER_SAVE_CONTROL(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
private:
    // 加载库
    bool LoadDll();
private:
    char m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};
