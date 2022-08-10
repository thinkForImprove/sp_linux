#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSIDC.H"
//////////////////////////////////////////////////////////////////////////
class CAgentMSR : public IAgentBase, public CLogManage
{
public:
    CAgentMSR();
    virtual ~CAgentMSR();

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

    // 数据处理
    HRESULT Get_WFS_IDC_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // Manager申请出参内存
    //信息命令
    HRESULT Fmt_WFSIDCSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCFORM(LPVOID lpData, LPWFSRESULT &lpResult);

    //执行命令
    HRESULT Fmt_WFSIDCRETAINCARD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCARDDATAARY(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCHIPIO(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCHIPPOWEROUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCFORMLIST(LPVOID lpData, LPWFSRESULT &lpResult);

    // 事件数据
    HRESULT Fmt_WFSIDCTRACKEVENT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_RETAINBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCARDACT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData);
    HRESULT Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData);

private:
    // 加载库
    bool LoadDll();
    HRESULT Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra);

private:
    char m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};
