/***************************************************************
* 文件名称：XFS_PPR_FIRE.cpp
* 文件描述：存折打印模块命令事件消息子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_PPR.h"

// Fire消息
void CXFS_PPR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_PPR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_PPR::FireNoMedia(LPCSTR szPrompt)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}
void CXFS_PPR::FireMediaInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

void CXFS_PPR::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

void CXFS_PPR::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}

void CXFS_PPR::FireRetractBinThreshold(USHORT BinNumber, WORD wStatus)
{
    WFSPTRBINTHRESHOLD BinThreshold;
    BinThreshold.usBinNumber     = BinNumber;
    BinThreshold.wRetractBin = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_RETRACTBINTHRESHOLD, &BinThreshold);
}

void CXFS_PPR::FireMediaTaken()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}

void CXFS_PPR::FirePaperThreshold(WORD wSrc, WORD wStatus)
{
    WFSPTRPAPERTHRESHOLD PaperThreshold;
    PaperThreshold.wPaperSource     = wSrc;
    PaperThreshold.wPaperThreshold  = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_PAPERTHRESHOLD, &PaperThreshold);
}

void CXFS_PPR::FireTonerThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_TONERTHRESHOLD, &wStatus);
}

void CXFS_PPR::FireInkThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}

void CXFS_PPR::FireLampThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}

void CXFS_PPR::FireSRVMediaInserted()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIAINSERTED, nullptr);
}

void CXFS_PPR::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}
