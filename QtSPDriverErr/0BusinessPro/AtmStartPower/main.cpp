#include "AtmStartPower.h"
#include <QApplication>
#include "QtAppRunning.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 判断是否单实例
    CQtAppRunning cApp;
    if (cApp.IsSingleAppRunning("App_AtmStartPower.exe"))
        return 0;

    // 运行界面
    CAtmStartPower w;
    w.hide();
    return a.exec();
}
