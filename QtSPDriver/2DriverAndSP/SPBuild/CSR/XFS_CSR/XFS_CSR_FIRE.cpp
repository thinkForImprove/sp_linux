/***************************************************************
* 文件名称：XFS_CSR_FIRE.cpp
* 文件描述：票据受理模块事件消息处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年2月24日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_CSR.h"

//-----------------------------------------------------------------------------------
//--------------------------------------事件消息---------------------------------------
// 上报Device HWERR事件
void CXFS_CSR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

// 上报状态变化事件
void CXFS_CSR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

// 上报无媒介事件
void CXFS_CSR::FireNoMedia(LPCSTR szPrompt)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}

// 上报媒介放入事件
void CXFS_CSR::FireMediaInserted()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

// 上报Field错误事件
void CXFS_CSR::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

// 上报Field警告事件
void CXFS_CSR::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}

// 上报回收箱变化事件
void CXFS_CSR::FireRetractBinThreshold(USHORT BinNumber, WORD wStatus)
{
    WFSPTRBINTHRESHOLD BinThreshold;
    BinThreshold.usBinNumber     = BinNumber;
    BinThreshold.wRetractBin = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_RETRACTBINTHRESHOLD, &BinThreshold);
}

// 上报媒介取走事件
void CXFS_CSR::FireMediaTaken()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}

// 上报纸状态/票箱状态变化事件
void CXFS_CSR::FirePaperThreshold(WORD wSrc, WORD wStatus)
{
    WFSPTRPAPERTHRESHOLD PaperThreshold;
    PaperThreshold.wPaperSource     = wSrc;
    PaperThreshold.wPaperThreshold  = wStatus;
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_PAPERTHRESHOLD, &PaperThreshold);
}

// 上报碳带状态变化事件
void CXFS_CSR::FireTonerThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_TONERTHRESHOLD, &wStatus);
}

// 上报墨盒状态变化事件
void CXFS_CSR::FireInkThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}

// 上报灯状态变化事件
void CXFS_CSR::FireLampThreshold(WORD wStatus)
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}

// 本事件指出在没有任何打印执行命令执行的情况下物理媒体已插入设备中。
// 本事件只有当媒介自动进入时才会生成。
void CXFS_CSR::FireSRVMediaInserted()
{
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIAINSERTED, nullptr);
}

// 上报复位中检测到设备内有媒介事件
void CXFS_CSR::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}
