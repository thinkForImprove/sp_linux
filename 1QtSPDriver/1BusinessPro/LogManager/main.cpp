#include "LogManager.h"
#include "QtAppRunning.h"
#include <QApplication>

//////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 判断是否单实例
    CQtAppRunning cApp;
    if (cApp.IsSingleAppRunning(LOGMANAGERNAME))
        return 0;
    // 运行界面
    CLogManager w;
    w.hide();
    return a.exec();
}
