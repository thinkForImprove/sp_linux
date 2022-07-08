#pragma once

//////////////////////////////////////////////////////////////////////////
#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <mutex>
#include <thread>
//////////////////////////////////////////////////////////////////////////
#include "LogWriteThread.h"
#include "QtShareMemoryRW.h"
#include "SimpleMutex.h"
//////////////////////////////////////////////////////////////////////////
#define SHAREMEMORY_NAME                "ShareMemory_LogWrite"
#define SHAREMEMORY_MUTEX               "Mutex_LogWrite"
#define LOGMANAGERNAME                  "LogManager.exe"
//////////////////////////////////////////////////////////////////////////
namespace Ui
{
class CLogManager;
}

class CLogManager : public QWidget, public CStlOneThread
{
    Q_OBJECT

public:
    explicit CLogManager(QWidget *parent = nullptr);
    ~CLogManager();
private slots:
    void OnExit();
public:
    virtual void Run();
private:
    Ui::CLogManager *ui;
private:
    QSystemTrayIcon                 m_sysTray;
    CQtShareMemoryRW                m_cMemR;
    CLogWriteThread                 m_cLog;
    CLogBackThread                  m_cBackLog;
};
