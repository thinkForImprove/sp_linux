
#include <QApplication>
#include <QtWidgets>

#include "DevPINTest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CDevPINTest w;

    QDesktopWidget *pDesk = QApplication::desktop();
    w.move((pDesk->width() - w.width()) / 2, (pDesk->height() - w.height()) / 2);
    w.show();

    return a.exec();
}
