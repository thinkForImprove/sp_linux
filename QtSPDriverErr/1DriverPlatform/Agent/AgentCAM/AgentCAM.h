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
    virtual HRESULT GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult);
protected:
    // 查询的
    HRESULT Get_WFS_INF_CAM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CAM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_CAM_TAKE_PICTURE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CAM_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CAM_TAKE_PICTURE_EX(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CAM_DISPLAY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CAM_GET_SIGNATURE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CAM_DISPLAYEX(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

protected:
    // 查询的
    HRESULT Get_WFS_CAM_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_CAM_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    // 执行的
    HRESULT Exe_WFS_CMD_CAM_GET_SIGNATURE_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_CAM_TAKE_DISPLAY_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    //HRESULT Exe_WFS_CMD_CAM_DISPLAYEX(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    //EVENT
    HRESULT Fmt_WFS_EXEE_CAM_LIVEERROR(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData);
    HRESULT Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData);
    // -------- 新增入参回参内存拷贝(接口名以Out结束,实现在AgentCAMOut.cpp) End--------

    HRESULT Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra);
private:
    // 加载库
    bool LoadDll();
private:
    char m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

#endif // AGENTCAM_H
