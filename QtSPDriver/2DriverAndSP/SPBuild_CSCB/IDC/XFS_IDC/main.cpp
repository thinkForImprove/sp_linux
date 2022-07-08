#include <QCoreApplication>
#include "XFS_IDC.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);// 调用此行，让后面能取得到命令行参数
    //std::this_thread::sleep_for(std::chrono::milliseconds(20 * 1000));

    CXFS_IDC gXFS;
    gXFS.StartRun();
    return 0;
}
