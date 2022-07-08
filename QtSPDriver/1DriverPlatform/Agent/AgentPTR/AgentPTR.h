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
    virtual void Release() override;
    // 实现命令数据拷贝
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData) override;
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData) override;
    // 实现返回数据拷贝
    virtual HRESULT GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData) override;
    virtual HRESULT ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData) override;
    virtual HRESULT CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult) override;

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
    HRESULT Exe_WFS_CMD_PTR_SET_GUIDANCE_LIGHT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

protected:
    HRESULT Get_WFS_PTR_NODATA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    // 查询的
    HRESULT Get_WFS_PTR_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_PTR_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_PTR_FORM_LIST_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_PTR_MEDIA_LIST_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_INF_PTR_QUERY_FORM_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_INF_PTR_QUERY_MEDIA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_INF_PTR_QUERY_FIELD_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    // 执行的
    HRESULT Exe_WFS_CMD_PTR_READ_FORM_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_PTR_RAW_DATA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_PTR_MEDIA_EXTENTS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_PTR_READ_IMAGE_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_PTR_RETRACT_MEDIA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFSMEDIAPRESENTED_Out(LPVOID lpData, LPWFSRESULT &lpResult);

    //EVENT
    HRESULT Fmt_WFS_PTR_NOMEDIA(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_EXEE_PTR_FIELDERROR(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_USRE_PTR_RETRACTBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_USRE_PTR_PAPERTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_DATA_WORD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFS_SRVE_PTR_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_NODATA(LPVOID lpData, LPWFSRESULT &lpResult);
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

