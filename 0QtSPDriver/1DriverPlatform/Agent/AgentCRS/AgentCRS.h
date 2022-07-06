﻿#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSCDM.H"
#include "XFSCIM.H"
#include "XFSCIM320.H"

class CAgent_CRS : public IAgentBase, public CLogManage
{
public:
    CAgent_CRS();
    virtual ~CAgent_CRS();

protected:
    virtual void Release();
    // 实现命令数据拷贝
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
protected:
    // 取款命令
    HRESULT CDMGetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT CDMExecute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    // 存款命令
    HRESULT CIMGetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT CIMExecute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
protected:
    // 查询的
    HRESULT Get_WFS_INF_CDM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CDM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CDM_CASH_UNIT_INFO(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CDM_MIX_TYPES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CDM_PRESENT_STATUS(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CDM_CURRENCY_EXP(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_CDM_DENOMINATE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CDM_DISPENSE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CDM_PRESENT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CDM_REJECT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CDM_RETRACT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CDM_OPEN_SHUTTER(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CDM_CLOSE_SHUTTER(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT ExeWFS_CMD_CDM_START_EXCHANGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CDM_END_EXCHANGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CDM_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

    // 查询的
    HRESULT Get_WFS_INF_CIM_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CIM_CAPABILITIES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CIM_CASH_UNIT_INFO(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CIM_BANKNOTE_TYPES(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CIM_CASH_IN_STATUS(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_CIM_CURRENCY_EXP(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_CIM_CASH_IN_START(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_CASH_IN(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_CASH_IN_END(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_CASH_IN_ROLLBACK(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_RETRACT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_OPEN_SHUTTER(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_CLOSE_SHUTTER(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_START_EXCHANGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_END_EXCHANGE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_RESET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_CONFIGURE_NOTETYPES(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_CIM_SET_CASH_IN_LIMIT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);

private:
    bool LoadWFMDll();
private:
    char                            m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

