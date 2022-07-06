#include "QtPostMessage.h"

//////////////////////////////////////////////////////////////////////////
extern "C" QTPOSTMSG_EXPORT long CreateIQtPostMessage(IQtPostMessage *&p)
{
    p = new CQtPostMessage;
    return (p != nullptr) ? 0 : -1;
}
//////////////////////////////////////////////////////////////////////////
CQtPostMessage::CQtPostMessage()
{
}

CQtPostMessage::~CQtPostMessage()
{

}

void CQtPostMessage::Release()
{
    delete this;
}

bool CQtPostMessage::IsWindowHWND(HWND hWnd)
{
#ifdef QT_WIN32
    return (TRUE == ::IsWindow((HWND)hWnd)) ? true : false;
#else
#ifdef QT_WIN_LINUX_XFS
    return true;
#else
    return false;
#endif
#endif
}

bool CQtPostMessage::PostMessageHWND(HWND hWnd, UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
#ifdef QT_WIN32
    return (TRUE == ::PostMessage((HWND)hWnd, uMsg, wParam, lParam)) ? true : false;
#else
#ifdef QT_LINUX_MANAGER_ZJ
    if (m_pIWFM == nullptr)
    {
        if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
            return false;
        if (m_pIWFM == nullptr)
            return false;
    }

    return (0 == m_pIWFM->WFMPostMessage((HWND)hWnd, uMsg, wParam, lParam, WFS_INDEFINITE_WAIT));
#else
    return false;
#endif
#endif
}

bool CQtPostMessage::IsWindowDBus(LPCSTR lpObjectName)
{
#ifdef QT_WIN32
    return false;
#else
    return true;
#endif
}

bool CQtPostMessage::PostMessageDBus(LPCSTR lpObjectName, UINT uMsg, LPVOID lpBuff)
{
#ifdef QT_WIN32
    return false;
#else

#if 0
    // 因南京银行LINUX应用是合荣的，应用的中间件是使用系统API的，和QtDBus有冲突
    QDBusConnection sess_bus = QDBusConnection::sessionBus();
    if (!sess_bus.isConnected())
        return false;

    char szDBusName[256] = {0};
    sprintf(szDBusName, "%s.p%d", LFS_MGR_DBUS_NAME_PREF, (int)QCoreApplication::applicationPid());
    QDBusInterface remoteApp(szDBusName, lpObjectName, LFS_MGR_DBUS_INTF_NAME);
    if (!remoteApp.isValid())
        return false;

    QString strMethod(LFS_MGR_DBUS_METHOD_NAME);
    QDBusPendingCall hRet = remoteApp.asyncCall(strMethod, (qlonglong)uMsg, (qlonglong)lpBuff);
    return !hRet.isError();
#else
    bool bRet = false;
    DBusMessage *msg = nullptr;
    DBusConnection *pNewdbus = nullptr;
    DBusMessageIter args;
    DBusError err;
    dbus_error_init(&err);
    do
    {
        //步骤1:建立与D-Bus后台的连接
        pNewdbus = dbus_bus_get(DBUS_BUS_SESSION, &err);
        if (nullptr == pNewdbus)
            break;
        if (dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
            break;
        }

        //步骤2:给连接名分配一个well-known的名字作为Bus name
        char dbus_name[256] = {0};
        snprintf(dbus_name, sizeof(dbus_name), "%s.p%d", LFS_MGR_DBUS_NAME_PREF, getpid());
        if (!dbus_bus_name_has_owner(pNewdbus, dbus_name, 0))
        {
            break;
        }

        //步骤3:通过名称，获取回调函数
        msg = dbus_message_new_method_call(dbus_name,                // bus name
                                           lpObjectName,             // object path
                                           LFS_MGR_DBUS_INTF_NAME,   // interface name
                                           LFS_MGR_DBUS_METHOD_NAME);// method name
        if (msg == nullptr)
            break;

        //设置不应答
        dbus_message_set_no_reply(msg, 1);

        //结构数据赋值
        dbus_message_iter_init_append(msg, &args);

        //命令ID
        long long llMsgId = uMsg;                   //add
//del        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT64, &uMsg))
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT64, &llMsgId))     //add
            break;
        //命令执行结果指针
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT64, &lpBuff))
            break;

        //步骤4:发送一个信号
        if (!dbus_connection_send(pNewdbus, msg, nullptr))
            break;

        dbus_connection_flush(pNewdbus);
        bRet = true;
    } while (false);

    //步骤5:释放内存
    if (msg != nullptr)
        dbus_message_unref(msg);
    if (pNewdbus != nullptr)
        dbus_connection_unref(pNewdbus);
    return bRet;
#endif
#endif
}

bool CQtPostMessage::PostMessagePISA(LPCSTR lpObjectName, UINT uMsg, LPVOID lpBuff)
{
    if (m_pIWFM == nullptr)
    {
        if (0 != m_pIWFM.Load("WFMShareMenory.dll", "CreateIWFMShareMenory"))
            return PostMessageDBus(lpObjectName, uMsg, lpBuff);
        else
            return (0 == m_pIWFM->WFMPostMessage((HWND)lpObjectName, uMsg, 0, (LONG)lpBuff, 0));
    }
    else
        return (0 == m_pIWFM->WFMPostMessage((HWND)lpObjectName, uMsg, 0, (LONG)lpBuff, 0));
}
