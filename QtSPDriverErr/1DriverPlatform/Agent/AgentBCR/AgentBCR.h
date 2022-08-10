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
    virtual HRESULT GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult);
protected:
    // 查询的
    HRESULT Get_WFS_INF_BCR_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_BCR_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_BCR_READ(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_BCR_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_BCR_SET_GUIDANCE_LIGHT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_BCR_POWER_SAVE_CONTROL(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

protected:
    // 查询的
    HRESULT Get_WFS_INF_BCR_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_INF_BCR_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);

    // 执行的
    HRESULT Exe_WFS_CMD_BCR_READ_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);

    //EVENT
    HRESULT Fmt_WFS_SRVE_BCR_DEVICEPOSITION(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_SRVE_BCR_POWER_SAVE_CHANGE(LPVOID lpData, LPWFSRESULT &lpResult);

    HRESULT Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra);

private:
    // 加载库
    bool LoadDll();
private:
    char m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};
