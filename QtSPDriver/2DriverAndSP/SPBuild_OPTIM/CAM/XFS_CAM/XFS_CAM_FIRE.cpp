/***************************************************************************
* 文件名称: XFS_CAM_FIRE.cpp
* 文件描述: CAM模块命令事件消息子处理接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2019年6月15日
* 文件版本: 1.0.0.1
***************************************************************************/

#include "XFS_CAM.h"

void CXFS_CAM::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_CAM::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_CAM::FireEXEE_LIVEERROR_Event(WORD wError)
{
    WFSCAMLIVEERROR stLiveErr;
    stLiveErr.wLiveError = wError;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CAM_LIVEERROR, &stLiveErr);
}

// -------------------------------------- END --------------------------------------

