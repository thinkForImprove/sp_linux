/***************************************************************
* 文件名称：XFS_IDX_FIRE.cpp
* 文件描述：身份证读卡器模块消息事件处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月25日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_IDX.h"

//----------------------------事件处理----------------------------
// 硬件故障事件
void CXFS_IDX::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

// 状态变化事件
void CXFS_IDX::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

// 进卡事件
void CXFS_IDX::FireCardInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIAINSERTED, nullptr);
}

// 退卡/移走卡事件
void CXFS_IDX::FireMediaRemoved()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIAREMOVED, nullptr);
}

//　吞卡事件
void CXFS_IDX::FireMediaRetained()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_MEDIARETAINED, nullptr);
}

// 回收相关事件
void CXFS_IDX::FireRetainBinThreshold(WORD wReBin)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_IDC_RETAINBINTHRESHOLD, (LPVOID)&wReBin);
}

// 复位时检测到卡事件
void CXFS_IDX::FireMediaDetected(WORD ResetOut)
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_IDC_MEDIADETECTED, (LPVOID)&ResetOut);
}

// 出现无效磁道事件
void CXFS_IDX::FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData)
{
    WFSIDCTRACKEVENT data;
    data.fwStatus   = wStatus;
    data.lpstrTrack = pTrackName;
    data.lpstrData  = pTrackData;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_IDC_INVALIDTRACKDATA, &data);
}

