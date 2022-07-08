#pragma once
#include <QtCore/qglobal.h>

#ifdef QT_WIN32
#include <qt_windows.h>
#pragma comment(lib, "user32.lib")
#endif

#include "QtTypeDef.h"

//////////////////////////////////////////////////////////////////////////
#if defined(QTPOSTMESSAGE_LIBRARY)
#define QTPOSTMSG_EXPORT Q_DECL_EXPORT
#else
#define QTPOSTMSG_EXPORT Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////
struct IQtPostMessage
{
    // 释放接口
    virtual void Release() = 0;
    // 检测窗口
    virtual bool IsWindowHWND(HWND hWnd) = 0;
    // 发送消息
    virtual bool PostMessageHWND(HWND hWnd, UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) = 0;

    // 检测窗口，LINUX系统
    virtual bool IsWindowDBus(LPCSTR lpObjectName) = 0;
    // 发送消息，LINUX系统
    virtual bool PostMessageDBus(LPCSTR lpObjectName, UINT uMsg, LPVOID lpBuff) = 0;
};

extern "C" QTPOSTMSG_EXPORT long CreateIQtPostMessage(IQtPostMessage *&p);
//////////////////////////////////////////////////////////////////////////

