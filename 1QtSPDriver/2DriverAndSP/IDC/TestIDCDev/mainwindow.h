#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QLibrary>
#include <QSemaphore>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <future>
using namespace  std;

#include "ILogWrite.h"
#include "IDevIDC.h"
#include "QtDLLLoader.h"

namespace Ui
{
class MainWindow;
}


class MainWindow : public QMainWindow, public CLogManage
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_BUTTON_OPEN_clicked();

    void on_BUTTON_CLOSE_clicked();

    void on_BUTTON_GETFWVERSION_clicked();


    void on_BUTTON_INITEJECT_clicked();

    void on_BUTTON_INITEAT_clicked();

    void on_BUTTON_INITHOLD_clicked();

    void on_BUTTON_DETECTCARD_clicked();

    void on_BUTTON_ACCEPTCARD_clicked();

    void on_BUTTON_READTRACK_clicked();

    void on_BUTTON_EJECTCARD_clicked();

    void on_BUTTON_RETRACTCARD_clicked();

    void on_BUTTON_CANCELACCEPT_clicked();

    void on_BUTTON_WRITECARD_clicked();

    void on_BUTTON_ICCACCEPT_clicked();

    void on_BUTTON_ICCPRESS_clicked();

    void on_BUTTON_ICCACTIVE_clicked();

    void on_BUTTON_ICCDEACTIVE_clicked();

    void on_BUTTON_ICCRESETCOLD_clicked();

    void on_BUTTON_ICCRESETWARM_clicked();

    void on_BUTTON_ICCRELEASE_clicked();

    void on_BUTTON_ICCCHIPIOT0_clicked();

    void on_BUTTON_ICCCHIPIOT1_clicked();

    void on_BUTTON_OPENLED_clicked();

    void on_BUTTON_CLOSELED_clicked();

private:
    long OnOpenDLl();
    void WriteLog(QString strlog, bool bEnter = true);
    void WriteLog(QString strModule, int iRet);
private:
    Ui::MainWindow *ui;

private:
    CQtDLLLoader<IDevIDC> m_pIDCDrv;
};

#endif // MAINWINDOW_H
