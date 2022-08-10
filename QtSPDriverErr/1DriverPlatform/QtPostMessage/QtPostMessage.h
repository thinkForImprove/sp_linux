#pragma once
#include "IQtPostMessage.h"
#include "IWFMShareMenory.h"
#include "QtDLLLoader.h"

#ifdef QT_LINUX
#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "XFSAPI.H"
#include <dbus/dbus.h>
#include <unistd.h>
#endif

//////////////////////////////////////////////////////////////////////////
class CQtPostMessage : public IQtPostMessage
{
public:
    CQtPostMessage();
    virtual ~CQtPostMessage();
public:
    // 释放接口
    virtual void Release();
    // 检测窗口
    virtual bool IsWindowHWND(HWND hWnd);
    // 发送消息
    virtual bool PostMessageHWND(HWND hWnd, UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0);

    // 检测窗口，LINUX系统
    virtual bool IsWindowDBus(LPCSTR lpObjectName);
    // 发送消息，LINUX系统
    virtual bool PostMessageDBus(LPCSTR lpObjectName, UINT uMsg, LPVOID lpBuff);
private:
    CQtDLLLoader<IWFMShareMenory>   m_pIWFM;
};


