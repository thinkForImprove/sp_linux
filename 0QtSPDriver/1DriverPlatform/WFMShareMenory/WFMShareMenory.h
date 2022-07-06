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

#if (defined QT_LINUX_MANAGER_PISA) || (defined QT_LINUX_MANAGER_ZJ)
typedef HRESULT(LFSAPI *XFSWFMKILLTIMER)(timer_t timer_id);
typedef HRESULT(LFSAPI *XFSWFMSETTIMER)(LPSTR object_name, LPVOID context, DWORD time_val, timer_t* timer_id);
typedef HRESULT(LFSAPI *XFSWFMCREATEHWND)(LPHWND lpHwnd);
typedef HRESULT(LFSAPI *XFSWFMDESTROYHWND)(HPROVIDER provider);
typedef HRESULT(LFSAPI *XFSWFMPOSTMESSAGE)(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout);
typedef HRESULT(LFSAPI *XFSWFMGETMESSAGE)(HWND hWnd, UINT *unMsg, UINT *wParam, LONG *lParam, DWORD dwTimeout);
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
    // LINUX专用接口
#ifdef QT_WIN_LINUX_XFS
#ifdef QT_LINUX
    virtual HRESULT WFMCreateHwnd(LPHWND lpHwnd);
    virtual HRESULT WFMDestroyHwnd(HWND hWnd);
    virtual HRESULT WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout);
    virtual HRESULT WFMGetMessage(HWND hWnd, UINT *unMsg, UINT *wParam, LONG *lParam, DWORD dwTimeout);
#endif
#else
#if (defined QT_LINUX_MANAGER_PISA) || (defined QT_LINUX_MANAGER_ZJ)
    virtual HRESULT WFMKillTimer(timer_t timer_id);
    virtual HRESULT WFMSetTimer(LPSTR object_name, LPVOID context, DWORD time_val, timer_t* timer_id);
    virtual HRESULT WFMCreateHwnd(LPHWND lpHwnd);
    virtual HRESULT WFMDestroyHwnd(HWND hWnd);
    virtual HRESULT WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout);
    virtual HRESULT WFMGetMessage(HWND hWnd, UINT *unMsg, UINT *wParam, LONG *lParam, DWORD dwTimeout);
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
#ifdef QT_WIN_LINUX_XFS
#ifdef QT_LINUX
    WFMCREATEHWND               m_pWFMCreateHwnd;
    WFMDESTROYHWND              m_pWFMDestroyHwnd;
    WFMPOSTMESSAGE              m_pWFMPostMessage;
    WFMGETMESSAGE               m_pWFMGetMessage;
#endif
#else
#if (defined QT_LINUX_MANAGER_PISA) || (defined QT_LINUX_MANAGER_ZJ)
    XFSWFMKILLTIMER             m_pWFMKillTimer;
    XFSWFMSETTIMER              m_pWFMSetTimer;
    XFSWFMCREATEHWND            m_pWFMCreateHwnd;
    XFSWFMDESTROYHWND           m_pWFMDestroyHwnd;
    XFSWFMPOSTMESSAGE           m_pWFMPostMessage;
    XFSWFMGETMESSAGE            m_pWFMGetMessage;
#endif
#endif

private:
    QLibrary m_cXFSDll;
};

