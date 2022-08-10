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
#if (defined EVENT_NOTIFY_POSTMESSAGE_XFS)
    m_pWFMPostMessage = nullptr;

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
#ifdef EVENT_NOTIFY_POSTMESSAGE_XFS
HRESULT CWFMShareMenory::WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (m_pWFMPostMessage == nullptr)
       return WFS_ERR_INTERNAL_ERROR;

    return m_pWFMPostMessage(hWnd, unMsg, wParam, lParam, dwTimeout);
}
#endif
HRESULT CWFMShareMenory::WFMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMAllocateBuffer(ulSize, ulFlags, lppvData);
}

HRESULT CWFMShareMenory::WFMAllocateMore(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMAllocateMore(ulSize, lpvOriginal, lppvData);
}

HRESULT CWFMShareMenory::WFMFreeBuffer(LPVOID lpvData)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    if (lpvData == nullptr)
        return WFS_ERR_INVALID_POINTER;
    return m_pWFMFreeBuffer(lpvData);
}

HRESULT CWFMShareMenory::WFMGetTraceLevel(HSERVICE hService, LPDWORD lpdwTraceLevel)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMGetTraceLevel(hService, lpdwTraceLevel);
}

HRESULT CWFMShareMenory::WFMOutputTraceData(LPSTR lpszData)
{
    if (!LoadDll())
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMOutputTraceData(lpszData);
}

HRESULT CWFMShareMenory::WFMReleaseDLL(HPROVIDER hProvider)
{
    if (!LoadDll() || m_pWFMReleaseDLL == nullptr)
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMReleaseDLL(hProvider);
}

HRESULT CWFMShareMenory::SIMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData)
{
    if (!LoadDll() || m_pSIMAllocateBuffer == nullptr)
        return WFS_ERR_INTERNAL_ERROR;
    return m_pSIMAllocateBuffer(ulSize, ulFlags, lppvData);
}

HRESULT CWFMShareMenory::SIMAllocateMore(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData)
{
    if (!LoadDll() || m_pSIMAllocateMore == nullptr)
        return WFS_ERR_INTERNAL_ERROR;
    return m_pSIMAllocateMore(ulSize, lpvOriginal, lppvData);
}

HRESULT CWFMShareMenory::SIMFreeBuffer(LPVOID lpvData)
{
    if (!LoadDll() || m_pSIMFreeBuffer == nullptr)
        return WFS_ERR_INTERNAL_ERROR;
    if (lpvData == nullptr)
        return WFS_ERR_INVALID_POINTER;
    return m_pSIMFreeBuffer(lpvData);
}
#ifdef EVENT_NOTIFY_POSTMESSAGE
HRESULT CWFMShareMenory::WFMPostMessage(LPSTR lpstrObjectName, DWORD dwEventID, UINT wParam, LONG lParam, DWORD dwTimeout)
{
    if (!LoadDll() || m_pWFMPostMessage == nullptr)
        return WFS_ERR_INTERNAL_ERROR;
    return m_pWFMPostMessage(lpstrObjectName, dwEventID, wParam, lParam, dwTimeout);
}
#endif
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
    strLibFile = "/usr/lib/libxfs";
#endif
#else
#ifdef EVENT_NOTIFY_POSTMESSAGE
    QString strLibSIMFIle;
    strLibFile = "/usr/lib/liblfs_supp";                //支持共享内存库
    strLibSIMFIle = "/usr/lib/libpisa_supp";            //不支持共享内存库
#elif EVENT_NOTIFY_POSTMESSAGE_XFS                      //30-00-00-00(FT#0074)
    strLibFile = "/usr/lib/libxfs.so";                  //30-00-00-00(FT#0074)
#else
    strLibFile = "/usr/lib/liblfs";
#endif
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
#if (defined EVENT_NOTIFY_POSTMESSAGE_XFS)           //30-00-00-00(FT#0074)
    m_pWFMAllocateBuffer = (XFSWFMALLOCATEBUFFER)m_cXFSDll.resolve("WFMAllocateBuffer");    //30-00-00-00(FT#0074)
    m_pWFMAllocateMore = (XFSWFMALLOCATEMORE)m_cXFSDll.resolve("WFMAllocateMore");          //30-00-00-00(FT#0074)
    m_pWFMFreeBuffer = (XFSWFMFREEBUFFER)m_cXFSDll.resolve("WFMFreeBuffer");                //30-00-00-00(FT#0074)
    m_pWFMPostMessage = (XFSWFMPOSTMESSAGE)m_cXFSDll.resolve("WFMPostMessage");             //30-00-00-00(FT#0074)

#else
    m_pWFMReleaseDLL = nullptr;
    m_pWFMAllocateBuffer = (XFSWFMALLOCATEBUFFER)m_cXFSDll.resolve("LFMAllocateBuffer");
    m_pWFMAllocateMore = (XFSWFMALLOCATEMORE)m_cXFSDll.resolve("LFMAllocateMore");
    m_pWFMFreeBuffer = (XFSWFMFREEBUFFER)m_cXFSDll.resolve("LFMFreeBuffer");
    m_pWFMGetTraceLevel = (XFSWFMGETTRACELEVEL)m_cXFSDll.resolve("LFMGetTraceLevel");
    m_pWFMOutputTraceData = (XFSWFMOUTPUTTRACEDATA)m_cXFSDll.resolve("LFMOutputTraceData");
//    m_pWFMReleaseDLL = (XFSWFMRELEASEDLL)m_cXFSDll.resolve("LFMReleaseLib");
#endif
#if (defined EVENT_NOTIFY_POSTMESSAGE_XFS)       //30-00-00-00(FT#0074)
    if (m_pWFMAllocateBuffer == nullptr ||       //30-00-00-00(FT#0074)
        m_pWFMAllocateMore == nullptr ||         //30-00-00-00(FT#0074)
        m_pWFMFreeBuffer == nullptr ||           //30-00-00-00(FT#0074)
        m_pWFMPostMessage == nullptr){           //30-00-00-00(FT#0074)
        m_cXFSDll.unload();                      //30-00-00-00(FT#0074)
        return false;                            //30-00-00-00(FT#0074)
    }
#else
    if (m_pWFMAllocateBuffer == nullptr ||
        m_pWFMAllocateMore == nullptr ||
        m_pWFMFreeBuffer == nullptr ||
        m_pWFMGetTraceLevel == nullptr ||
        m_pWFMOutputTraceData == nullptr)
    {
        m_cXFSDll.unload();
        return false;
    }

    m_pSIMAllocateBuffer = nullptr;
    m_pSIMAllocateMore = nullptr;
    m_pSIMFreeBuffer = nullptr;
#ifdef EVENT_NOTIFY_POSTMESSAGE
    m_pWFMPostMessage = nullptr;
    m_cSIMDll.setFileName(strLibSIMFIle);
    if (!m_cSIMDll.load())
        return false;

    m_pSIMAllocateBuffer = (XFSWFMALLOCATEBUFFER)m_cSIMDll.resolve("LFMAllocateBuffer");
    m_pSIMAllocateMore = (XFSWFMALLOCATEMORE)m_cSIMDll.resolve("LFMAllocateMore");
    m_pSIMFreeBuffer = (XFSWFMFREEBUFFER)m_cSIMDll.resolve("LFMFreeBuffer");
    m_pWFMPostMessage = (XFSWFMPOSTMESSAGE)m_cSIMDll.resolve("LFMPostMessage");
    if(m_pSIMAllocateBuffer == nullptr ||
       m_pSIMAllocateMore == nullptr ||
       m_pSIMFreeBuffer == nullptr ||
       m_pWFMPostMessage == nullptr){
        m_cSIMDll.unload();
        return false;
    }
#endif
#endif
#endif

    return true;
}
