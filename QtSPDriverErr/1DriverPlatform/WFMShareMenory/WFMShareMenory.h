#pragma once

#include <QLibrary>
#include "IWFMShareMenory.h"
#include "XFSAPI.H"

//////////////////////////////////////////////////////////////////////////
#ifdef QT_WIN_LINUX_XFS
#ifdef QT_WIN32
#define WINAPI __stdcall
#else
#define WINAPI __attribute__((__cdecl__))
#endif
typedef HRESULT(WINAPI *XFSWFMALLOCATEBUFFER)(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData);
typedef HRESULT(WINAPI *XFSWFMALLOCATEMORE)(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData);
typedef HRESULT(WINAPI *XFSWFMFREEBUFFER)(LPVOID lpvData);
typedef HRESULT(WINAPI *XFSWFMGETTRACELEVEL)(HSERVICE hService, LPDWORD lpdwTraceLevel);
typedef HRESULT(WINAPI *XFSWFMOUTPUTTRACEDATA)(LPSTR lpszData);
typedef HRESULT(WINAPI *XFSWFMRELEASEDLL)(HPROVIDER hProvider);

#ifdef QT_LINUX
typedef HRESULT(WINAPI *WFMCREATEHWND)(LPHWND lpHwnd);
typedef HRESULT(WINAPI *WFMDESTROYHWND)(HWND hWnd);
typedef HRESULT(WINAPI *WFMPOSTMESSAGE)(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout);
typedef HRESULT(WINAPI *WFMGETMESSAGE)(HWND hWnd, UINT *unMsg, UINT *wParam, LONG *lParam, DWORD dwTimeout);
#endif

#else
#define LFSAPI __attribute__((__cdecl__))
typedef HRESULT(LFSAPI *XFSWFMALLOCATEBUFFER)(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData);
typedef HRESULT(LFSAPI *XFSWFMALLOCATEMORE)(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData);
typedef HRESULT(LFSAPI *XFSWFMFREEBUFFER)(LPVOID lpvData);
typedef HRESULT(LFSAPI *XFSWFMGETTRACELEVEL)(HSERVICE hService, LPDWORD lpdwTraceLevel);
typedef HRESULT(LFSAPI *XFSWFMOUTPUTTRACEDATA)(LPSTR lpszData);
typedef HRESULT(LFSAPI *XFSWFMRELEASEDLL)(HPROVIDER hProvider);
#ifdef EVENT_NOTIFY_POSTMESSAGE
typedef HRESULT(LFSAPI *XFSWFMPOSTMESSAGE)(LPSTR lpstrObjectName, DWORD dwEventID, UINT wParam, LONG lParam, DWORD dwTimeout);
#elif EVENT_NOTIFY_POSTMESSAGE_XFS     //30-00-00-00(FT#0074)
typedef HRESULT(LFSAPI *XFSWFMPOSTMESSAGE)(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout);
#endif
#endif
//////////////////////////////////////////////////////////////////////////
class CWFMShareMenory : public IWFMShareMenory
{
public:
    CWFMShareMenory();
    virtual ~CWFMShareMenory();
public:
    virtual void Release();
    virtual HRESULT WFMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData);
    virtual HRESULT WFMAllocateMore(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData);
    virtual HRESULT WFMFreeBuffer(LPVOID lpvData);
    virtual HRESULT WFMGetTraceLevel(HSERVICE hService, LPDWORD lpdwTraceLevel);
    virtual HRESULT WFMOutputTraceData(LPSTR lpszData);
    virtual HRESULT WFMReleaseDLL(HPROVIDER hProvider);

    // 内存非进程间共享Manager接口
    virtual HRESULT SIMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData);
    virtual HRESULT SIMAllocateMore(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData);
    virtual HRESULT SIMFreeBuffer(LPVOID lpvData);

    // LINUX专用接口
#ifdef QT_WIN_LINUX_XFS
#ifdef QT_LINUX
    virtual HRESULT WFMCreateHwnd(LPHWND lpHwnd);
    virtual HRESULT WFMDestroyHwnd(HWND hWnd);
    virtual HRESULT WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout);
    virtual HRESULT WFMGetMessage(HWND hWnd, UINT *unMsg, UINT *wParam, LONG *lParam, DWORD dwTimeout);
#endif
#else
#ifdef EVENT_NOTIFY_POSTMESSAGE
    virtual HRESULT WFMPostMessage(LPSTR lpstrObjectName, DWORD dwEventID, UINT wParam, LONG lParam, DWORD dwTimeout);
#elif EVENT_NOTIFY_POSTMESSAGE_XFS //30-00-00-00(FT#0074)
    virtual HRESULT WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout);
#endif
#endif

public:
    // 动态加载XFS库文件
    bool LoadDll();

protected:
    XFSWFMALLOCATEBUFFER        m_pWFMAllocateBuffer;
    XFSWFMALLOCATEMORE          m_pWFMAllocateMore;
    XFSWFMFREEBUFFER            m_pWFMFreeBuffer;
    XFSWFMGETTRACELEVEL         m_pWFMGetTraceLevel;
    XFSWFMOUTPUTTRACEDATA       m_pWFMOutputTraceData;
    XFSWFMRELEASEDLL            m_pWFMReleaseDLL;


    // 内存非进程间共享Manager接口
    XFSWFMALLOCATEBUFFER        m_pSIMAllocateBuffer;
    XFSWFMALLOCATEMORE          m_pSIMAllocateMore;
    XFSWFMFREEBUFFER            m_pSIMFreeBuffer;

#ifdef QT_WIN_LINUX_XFS
#ifdef QT_LINUX
    WFMCREATEHWND               m_pWFMCreateHwnd;
    WFMDESTROYHWND              m_pWFMDestroyHwnd;
    WFMPOSTMESSAGE              m_pWFMPostMessage;
    WFMGETMESSAGE               m_pWFMGetMessage;
#endif
#else
#if (defined EVENT_NOTIFY_POSTMESSAGE_XFS)||(defined EVENT_NOTIFY_POSTMESSAGE)  //30-00-00-00(FT#0074)
    XFSWFMPOSTMESSAGE           m_pWFMPostMessage;
#endif
#endif

private:
    QLibrary m_cXFSDll;
#ifdef EVENT_NOTIFY_POSTMESSAGE
    QLibrary m_cSIMDll;
#endif
};

