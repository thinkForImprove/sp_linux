#include <QCoreApplication>
#include "XFS_DEP.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);// 调用此行，让后面能取得到命令行参数
    //std::this_thread::sleep_for(std::chrono::milliseconds(10 * 1000));

    CXFS_DEP gXFS;
    gXFS.StartRun();
    return 0;
}
