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
    virtual HRESULT GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult);
protected:
    // 查询的
    HRESULT Get_WFS_INF_VDM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_VDM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    //　执行的
    HRESULT Exe_WFS_CMD_VDM_ENTER_MODE_REQ(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_VDM_ENTER_MODE_ACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_VDM_EXIT_MODE_REQ(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_VDM_EXIT_MODE_ACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

    // Manager申请出参内存
    //信息命令
    HRESULT Fmt_WFSVDMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSVDMCAPS(LPVOID lpData, LPWFSRESULT &lpResult);

    //事件
    HRESULT Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData);
    HRESULT Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData);
private:
    // 加载库
    bool LoadDll();
    HRESULT Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra);
private:
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

#endif // AGENTVDM_H
