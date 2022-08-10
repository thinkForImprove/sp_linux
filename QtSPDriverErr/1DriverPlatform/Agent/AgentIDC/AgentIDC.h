#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSIDC.H"
#include "XFSCRD.H"
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
    virtual HRESULT GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult);
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
    HRESULT Exe_WFS_CMD_CMEMPTYALL_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CMCLEARSLOT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

    // 增加 CRD系列命令(Card Dispenser),用于发卡模块 START
    /* CRD Info Commands */
    // WFS_INF_CRD_STATUS
    // WFS_INF_CRD_CAPABILITIES
    // WFS_INF_CRD_CARD_UNIT_INFO // 取卡箱状态/卡箱容量
    /* CRD Execute Commands */
    HRESULT Exe_WFS_CMD_CRD_DISPENSE_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData);     // 发卡
    HRESULT Exe_WFS_CMD_CRD_EJECT_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData);        // 退卡
    HRESULT Exe_WFS_CMD_CRD_RETAIN_CARD(LPVOID lpCmdData, LPVOID &lpCopyCmdData);       // 回收卡
    HRESULT Exe_WFS_CMD_CRD_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);             // 复位
    HRESULT Exe_WFS_CMD_CRD_SET_GUIDANCE_LIGHT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);// 设置灯状态
    HRESULT Exe_WFS_CMD_CRD_SET_CARD_UNIT_INFO(LPVOID lpCmdData, LPVOID &lpCopyCmdData);// 设置卡箱信息
    HRESULT Exe_WFS_CMD_CRD_POWER_SAVE_CONTROL(LPVOID lpCmdData, LPVOID &lpCopyCmdData);// 激活/停用节能功能
    // 增加 CRD系列命令(Card Dispenser),用于发卡模块 END

protected:
    // 查询的
    HRESULT Get_WFS_INF_IDC_QUERYFORM_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_IDC_STATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_IDC_CAPABILITIES_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_IDC_FORMLIST_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_IDC_CRDSTATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_IDC_CRDCAPS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_IDC_CARDUNITINFO_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    // 执行的
    HRESULT Exe_WFS_CMD_IDC_RAW_DATA_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_IDC_CHIP_IO_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_IDC_CHIP_POWER_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Get_WFS_IDC_NODATA_Out(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_IDC_CMSTATUS_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_IDC_RETAIN_CARD_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_IDC_RETAINCARD_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);
    HRESULT Exe_WFS_CMD_IDC_GETCARDINFO_Out(LPVOID lpQueryDetails, LPWFSRESULT &lpResult);

    HRESULT Fmt_WFSIDCTRACKEVENT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_EXEE_CRD_CARDUNITERROR(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCARDACT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_SRVE_CRD_MEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_SRVE_CRD_CARDUNITINFOCHANGED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_SRVE_CRD_DEVICEPOSITION(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_SRVE_CRD_POWER_SAVE_CHANGE(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_RETAINBINTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_USRE_CRD_CARDUNITTHRESHOLD(LPVOID lpData, LPWFSRESULT &lpResult);
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
