#pragma once
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"


//////////////////////////////////////////////////////////////////////////
#if defined(WFMSHAREMENORY_LIBRARY)
#define WFMMEM_EXPORT Q_DECL_EXPORT
#else
#define WFMMEM_EXPORT Q_DECL_IMPORT
#endif

//////////////////////////////////////////////////////////////////////////
#define WFS_MEM_SHARE                        0x00000001
#define WFS_MEM_ZEROINIT                     0x00000002
//#ifdef EVENT_NOTIFY_POSTMESSAGE
//#define WFS_MEM_FLAG                         (WFS_MEM_ZEROINIT)
//#else
#define WFS_MEM_FLAG                         (WFS_MEM_SHARE | WFS_MEM_ZEROINIT)
//#endif
//////////////////////////////////////////////////////////////////////////
struct IWFMShareMenory
{
    virtual void Release() = 0;
    virtual HRESULT WFMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData) = 0;
    virtual HRESULT WFMAllocateMore(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData) = 0;
    virtual HRESULT WFMFreeBuffer(LPVOID lpvData) = 0;
    virtual HRESULT WFMGetTraceLevel(HSERVICE hService, LPDWORD lpdwTraceLevel) = 0;
    virtual HRESULT WFMOutputTraceData(LPSTR lpszData) = 0;
    virtual HRESULT WFMReleaseDLL(HPROVIDER hProvider) = 0;
#ifdef EVENT_NOTIFY_POSTMESSAGE
    virtual HRESULT WFMPostMessage(LPSTR lpstrObjectName, DWORD dwEventID, UINT wParam, LONG lParam, DWORD dwTimeout) = 0;
#elif EVENT_NOTIFY_POSTMESSAGE_XFS  //30-00-00-00(FT#0074)
    virtual HRESULT WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout) = 0;
#endif
    // 内存非进程间共享Manager接口
    virtual HRESULT SIMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppvData) = 0;
    virtual HRESULT SIMAllocateMore(ULONG ulSize, LPVOID lpvOriginal, LPVOID *lppvData) = 0;
    virtual HRESULT SIMFreeBuffer(LPVOID lpvData) = 0;

    // LINUX专用接口
#ifdef QT_WIN_LINUX_XFS
#ifdef QT_LINUX
    virtual HRESULT WFMCreateHwnd(LPHWND lpHwnd) = 0;
    virtual HRESULT WFMDestroyHwnd(HWND hWnd) = 0;
    virtual HRESULT WFMPostMessage(HWND hWnd, UINT unMsg, UINT wParam, LONG lParam, DWORD dwTimeout) = 0;
    virtual HRESULT WFMGetMessage(HWND hWnd, UINT *unMsg, UINT *wParam, LONG *lParam, DWORD dwTimeout) = 0;
#endif
#endif
};

extern "C" WFMMEM_EXPORT long CreateIWFMShareMenory(IWFMShareMenory *&p);
//////////////////////////////////////////////////////////////////////////
