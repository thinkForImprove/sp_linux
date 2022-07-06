#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSIDC.H"
//////////////////////////////////////////////////////////////////////////
class CAgentIDC : public IAgentBase, public CLogManage
{
public:
    CAgentIDC();
    virtual ~CAgentIDC();

public:
    // 实现命令数据拷贝
    virtual void Release();
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);

protected:
    // 查询的
    HRESULT Get_WFS_INF_IDC_QUERYFORM(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    // HRESULT Exe_WFS_CMD_IDC_READ(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_RAW_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CHIP_IO(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CHIP_POWER(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

    HRESULT Get_WFS_IDC_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

#ifdef CARD_REJECT_GD_MODE
    HRESULT Exe_WFS_CMD_IDC_CMEJECT_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CMSTATUS(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_SETCARDDATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CMRETAIN_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CMREDUCE_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CMSET_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CMEMPTY_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_GETCARDINFO(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CMRESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_REDUCE_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_SET_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_INTAKE_CARD_BACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
#endif
private:
    // 加载库
    bool LoadDll();

private:
    char m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};
