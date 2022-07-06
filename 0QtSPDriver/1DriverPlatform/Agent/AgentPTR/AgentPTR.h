#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSPTR.H"
//////////////////////////////////////////////////////////////////////////
class CAgentPTR : public IAgentBase, public CLogManage
{
public:
    CAgentPTR();
    virtual ~CAgentPTR();
public:
    virtual void Release();
    // 实现命令数据拷贝
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
protected:

    HRESULT Get_WFS_PTR_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    // 查询的
    HRESULT Get_WFS_INF_PTR_QUERY_FORM(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_PTR_QUERY_MEDIA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_PTR_QUERY_FIELD(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_PTR_CONTROL_MEDIA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_PRINT_FORM(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_READ_FORM(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_RAW_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_MEDIA_EXTENTS(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_RESET_COUNT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_READ_IMAGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_RETRACT_MEDIA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PTR_DISPENSE_PAPER(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
private:
    // 加载库
    bool LoadDll();
private:
    char m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

