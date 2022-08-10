/***************************************************************************
* 文件名称：XFS_IDC_FIRE.cpp
* 文件描述：IDC模块命令事件消息子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
***************************************************************************/

#include "XFS_IDC.h"

void CXFS_IDC::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_IDC::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_IDC::FireCardInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIAINSERTED, nullptr);
}

void CXFS_IDC::FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIAREMOVED, nullptr);
}

void CXFS_IDC::FireMediaRetained()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIARETAINED, nullptr);
}

void CXFS_IDC::FireRetainBinThreshold(WORD wReBin)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_IDC_RETAINBINTHRESHOLD, (LPVOID)&wReBin);
}

void CXFS_IDC::FireMediaDetected(WORD ResetOut)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIADETECTED, (LPVOID)&ResetOut);
}

void CXFS_IDC::FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData)
{
    WFSIDCTRACKEVENT data;
    data.fwStatus   = wStatus;
    data.lpstrTrack = pTrackName;
    data.lpstrData  = pTrackData;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDTRACKDATA, &data);
}

// -------------------------------------- END --------------------------------------
