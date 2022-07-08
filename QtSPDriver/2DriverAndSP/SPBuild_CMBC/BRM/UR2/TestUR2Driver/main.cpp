#include "TestUR2Driver.h"
#include <QApplication>
#include <QTextCodec>
#include <QStyleFactory>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    //  QCoreApplication::addLibraryPath("./QtCommon");
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    a.setWindowIcon(QIcon("app.ico"));

    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(codec);

    QDesktopWidget *desktop = QApplication::desktop();
    int desktopWidth = desktop->width();
    int desktopHeight = desktop->height();

    TestUR2Driver DriverTest;
    DriverTest.move((desktopWidth - DriverTest.width()) / 2, (desktopHeight - DriverTest.height()) / 4);
    DriverTest.show();

    return a.exec();
}
