/***************************************************************
* 文件名称: XFS_BCR_FIRE.cpp
* 文件描述: 条码阅读模块子命令事件消息子处理接口(IDC命令系)
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2019年7月6日
* 文件版本: 1.0.0.1
****************************************************************/

#include "XFS_BCR.h"

void CXFS_BCR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_BCR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_BCR::FireSetDevicePosition(WORD wPosition)
{
    m_pBase->FireBarCodePosition(wPosition);
}

void CXFS_BCR::FireSetGuidAnceLight()
{

}

void CXFS_BCR::FirePowerSaveControl(WORD wPowerSaveTime)
{
    WFSBCRPOWERSAVECHANGE stPowSave;
    stPowSave.usPowerSaveRecoveryTime = wPowerSaveTime;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_BCR_POWER_SAVE_CHANGE, &stPowSave);
}

// -------------------------------------- END --------------------------------------
