#include "mainwindow.h"
#include <QApplication>
#include "XFS_VDM.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CXFS_VDM cXfsVdm;
    cXfsVdm.StartRun();

    return 0;
}
