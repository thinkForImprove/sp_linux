#include "WFMShareMenory.h"

//////////////////////////////////////////////////////////////////////////
extern "C" Q_DECL_EXPORT long CreateIWFMShareMenory(IWFMShareMenory *&p)
{
    CWFMShareMenory *pMem = new CWFMShareMenory;
    if (!pMem->LoadDll())
    {
        delete pMem;
        return -1;
    }
    p = pMem;
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CWFMShareMenory::CWFMShareMenory()
{
#ifdef QT_WIN_LINUX_XFS
#ifdef QT_LINUX
    m_pWFMCreateHwnd = nullptr;
    m_pWFMDestroyHwnd = nullptr;
    m_pWFMPostMessage = nullptr;
    m_pWFMGetMessage = nullptr;
#endif
#else
    m_pWFMAllocateBuffer = nullptr;
    m_pWFMAllocateMore = nullptr;
    m_pWFMFreeBuffer = nullptr;
    m_pWFMGetTraceLevel = nullptr;
    m_pWFMOutputTraceData = nullptr;
    m_pWFMReleaseDLL = nullptr;
#if (defined QT_LINUX_MANAGER_PISA) || (defined QT_LINUX_MANAGER_ZJ)
    m_pWFMKillTimer = nullptr;
    m_pWFMSetTimer = nullptr;
    m_pWFMCreateHwnd = nullptr;
    m_pWFMDestroyHwnd = nullptr;
    m_pWFMPostMessage = nullptr;
    m_pWFMGetMessage = nullptr;
#endif
#endif
}

CWFMShareMenory::~CWFMShareMenory()
{
    if (m_cXFSDll.isLoaded())
        m_cXFSDll.unload();
}

void CWFMShareMenory::Release()
{

}

HRESULT CWFMShareMenory::WFMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMAllocateBuffer == nullptr)
        return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMAllocateBuffer(ulSize, ulFlags, lppvData);
}

HRESULT CWFMShareMenory::WFMAllocateMore(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMAllocateMore == nullptr)
        return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMAllocateMore(ulSize, lpvOriginal, lppvData);
}

HRESULT CWFMShareMenory::WFMFreeBuffer(LPVOID lpvData)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMFreeBuffer == nullptr)
        return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMFreeBuffer(lpvData);
}

HRESULT CWFMShareMenory::WFMGetTraceLevel(HSERVICE hService, LPDWORD lpdwTraceLevel)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMGetTraceLevel == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMGetTraceLevel(hService, lpdwTraceLevel);
}

HRESULT CWFMShareMenory::WFMOutputTraceData(LPSTR lpszData)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMOutputTraceData == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMOutputTraceData(lpszData);
}

HRESULT CWFMShareMenory::WFMReleaseDLL(HPROVIDER hProvider)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMReleaseDLL == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMReleaseDLL(hProvider);
}

#ifdef QT_WIN_LINUX_XFS
#ifdef QT_LINUX
HRESULT CWFMShareMenory::WFMCreateHwnd(LPHWND lpHwnd)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMCreateHwnd(lpHwnd);
}

HRESULT CWFMShareMenory::WFMDestroyHwnd(HWND hWnd)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMDestroyHwnd(hWnd);
}

HRESULT CWFMShareMenory::WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMPostMessage(hWnd, unMsg, wParam, lParam, dwTimeout);
}

HRESULT CWFMShareMenory::WFMGetMessage(HWND hWnd, UINT *unMsg, UINT *wParam, LONG *lParam, DWORD dwTimeout)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMGetMessage(hWnd, unMsg, wParam, lParam, dwTimeout);
}
#endif
#else
#if (defined QT_LINUX_MANAGER_PISA) || (defined QT_LINUX_MANAGER_ZJ)
HRESULT CWFMShareMenory::WFMKillTimer(timer_t timer_id)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMKillTimer == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMKillTimer(timer_id);
}
HRESULT CWFMShareMenory::WFMSetTimer(LPSTR object_name, LPVOID context, DWORD time_val, timer_t* timer_id)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMSetTimer == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMSetTimer(object_name, context, time_val, timer_id);
}
HRESULT CWFMShareMenory::WFMCreateHwnd(LPHWND lpHwnd)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMCreateHwnd == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMCreateHwnd(lpHwnd);
}
HRESULT CWFMShareMenory::WFMDestroyHwnd(HWND hWnd)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMDestroyHwnd == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMDestroyHwnd(hWnd);
}
HRESULT CWFMShareMenory::WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMPostMessage == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMPostMessage(hWnd, unMsg, wParam, lParam, dwTimeout);
}
HRESULT CWFMShareMenory::WFMGetMessage(HWND hWnd, UINT *unMsg, UINT *wParam, LONG *lParam, DWORD dwTimeout)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMGetMessage == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMGetMessage(hWnd, unMsg, wParam, lParam, dwTimeout);
}
#endif
#endif

bool CWFMShareMenory::LoadDll()
{
    if (m_cXFSDll.isLoaded())
        return true;

    QString strLibFile;
#ifdef QT_WIN_LINUX_XFS
#ifdef QT_WIN32
    strLibFile = "msxfs.dll";
#else
    strLibFile = "/usr/lib/liblfs";
#endif
#else
    strLibFile = "/usr/lib/libxfs";
#endif

    m_cXFSDll.setFileName(strLibFile);
    if (!m_cXFSDll.load())
        return false;

#ifdef QT_WIN_LINUX_XFS
    m_pWFMAllocateBuffer = (XFSWFMALLOCATEBUFFER)m_cXFSDll.resolve("WFMAllocateBuffer");
    m_pWFMAllocateMore = (XFSWFMALLOCATEMORE)m_cXFSDll.resolve("WFMAllocateMore");
    m_pWFMFreeBuffer = (XFSWFMFREEBUFFER)m_cXFSDll.resolve("WFMFreeBuffer");
    m_pWFMGetTraceLevel = (XFSWFMGETTRACELEVEL)m_cXFSDll.resolve("WFMGetTraceLevel");
    m_pWFMOutputTraceData = (XFSWFMOUTPUTTRACEDATA)m_cXFSDll.resolve("WFMOutputTraceData");
    m_pWFMReleaseDLL = (XFSWFMRELEASEDLL)m_cXFSDll.resolve("WFMReleaseDLL");

#ifdef QT_LINUX
    m_pWFMCreateHwnd = (WFMCREATEHWND)m_cXFSDll.resolve("WFMCreateHwnd");
    m_pWFMDestroyHwnd = (WFMDESTROYHWND)m_cXFSDll.resolve("WFMDestroyHwnd");
    m_pWFMPostMessage = (WFMPOSTMESSAGE)m_cXFSDll.resolve("WFMPostMessage");
    m_pWFMGetMessage = (WFMGETMESSAGE)m_cXFSDll.resolve("WFMGetMessage");
    if (m_pWFMCreateHwnd == nullptr  ||
        m_pWFMDestroyHwnd == nullptr ||
        m_pWFMPostMessage == nullptr ||
        m_pWFMGetMessage == nullptr)
    {
        m_cXFSDll.unload();
        return false;
    }
#endif
#else
#if (defined QT_LINUX_MANAGER_PISA) || (defined QT_LINUX_MANAGER_ZJ)
    m_pWFMAllocateBuffer = (XFSWFMALLOCATEBUFFER)m_cXFSDll.resolve("WFMAllocateBuffer");
    m_pWFMAllocateMore = (XFSWFMALLOCATEMORE)m_cXFSDll.resolve("WFMAllocateMore");
    m_pWFMFreeBuffer = (XFSWFMFREEBUFFER)m_cXFSDll.resolve("WFMFreeBuffer");
    m_pWFMGetTraceLevel = (XFSWFMGETTRACELEVEL)m_cXFSDll.resolve("WFMGetTraceLevel");
    m_pWFMKillTimer = (XFSWFMKILLTIMER)m_cXFSDll.resolve("WFMKillTimer");
    m_pWFMOutputTraceData = (XFSWFMOUTPUTTRACEDATA)m_cXFSDll.resolve("WFMOutputTraceData");
    m_pWFMReleaseDLL = (XFSWFMRELEASEDLL)m_cXFSDll.resolve("WFMReleaseDLL");
    m_pWFMSetTimer = (XFSWFMSETTIMER)m_cXFSDll.resolve("WFMSetTimer");
    m_pWFMCreateHwnd = (XFSWFMCREATEHWND)m_cXFSDll.resolve("WFMCreateHwnd");
    m_pWFMDestroyHwnd = (XFSWFMDESTROYHWND)m_cXFSDll.resolve("WFMDestroyHwnd");
    m_pWFMPostMessage = (XFSWFMPOSTMESSAGE)m_cXFSDll.resolve("WFMPostMessage");
    m_pWFMGetMessage = (XFSWFMGETMESSAGE)m_cXFSDll.resolve("WFMGetMessage");
#else
    m_pWFMAllocateBuffer = (XFSWFMALLOCATEBUFFER)m_cXFSDll.resolve("LFMAllocateBuffer");
    m_pWFMAllocateMore = (XFSWFMALLOCATEMORE)m_cXFSDll.resolve("LFMAllocateMore");
    m_pWFMFreeBuffer = (XFSWFMFREEBUFFER)m_cXFSDll.resolve("LFMFreeBuffer");
    m_pWFMGetTraceLevel = (XFSWFMGETTRACELEVEL)m_cXFSDll.resolve("LFMGetTraceLevel");
    m_pWFMOutputTraceData = (XFSWFMOUTPUTTRACEDATA)m_cXFSDll.resolve("LFMOutputTraceData");
    m_pWFMReleaseDLL = (XFSWFMRELEASEDLL)m_cXFSDll.resolve("LFMReleaseLib");
#endif
#endif
#if (defined QT_LINUX_MANAGER_PISA) || (defined QT_LINUX_MANAGER_ZJ)
    if (m_pWFMAllocateBuffer == nullptr ||
        m_pWFMAllocateMore == nullptr ||
        m_pWFMFreeBuffer == nullptr ||
        m_pWFMGetTraceLevel == nullptr ||
        m_pWFMKillTimer == nullptr ||
        m_pWFMOutputTraceData == nullptr ||
        m_pWFMReleaseDLL == nullptr ||
        m_pWFMSetTimer == nullptr ||
        m_pWFMCreateHwnd == nullptr ||
        m_pWFMDestroyHwnd == nullptr ||
        m_pWFMPostMessage == nullptr ||
        m_pWFMGetMessage == nullptr)
#else
    if (m_pWFMAllocateBuffer == nullptr ||
        m_pWFMAllocateMore == nullptr ||
        m_pWFMFreeBuffer == nullptr ||
        m_pWFMGetTraceLevel == nullptr ||
        m_pWFMOutputTraceData == nullptr ||
        m_pWFMReleaseDLL == nullptr)
#endif
    {
        m_cXFSDll.unload();
        return false;
    }
    return true;
}
