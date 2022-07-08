#include <QCoreApplication>
#include "XFS_COR.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);// 调用此行，让后面能取得到命令行参数

    CXFS_COR gXFS;
    gXFS.StartRun();

    //return a.exec();
    return 0;
}
