#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSIDC.H"
//////////////////////////////////////////////////////////////////////////
class CAgentIDX : public IAgentBase, public CLogManage
{
public:
    CAgentIDX();
    virtual ~CAgentIDX();

public:
    // 实现命令数据拷贝
    virtual void Release();
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    virtual HRESULT GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult);
protected:
    // 执行的
    // HRESULT Exe_WFS_CMD_IDC_READ(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_RAW_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CHIP_IO(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

    // 数据处理
    HRESULT Get_WFS_IDC_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
protected:
    HRESULT Get_WFS_IDC_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_IDC_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_IDC_RAW_DATA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_IDC_RETAINCARD_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCTRACKEVENT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCARDACT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_RETAINBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData);
    HRESULT Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData);
    // -------- 新增入参回参内存拷贝(接口名以Out结束,实现在AgentPtrOut.cpp) End--------

    HRESULT Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra);
private:
    // 加载库
    bool LoadDll();

private:
    char m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};
