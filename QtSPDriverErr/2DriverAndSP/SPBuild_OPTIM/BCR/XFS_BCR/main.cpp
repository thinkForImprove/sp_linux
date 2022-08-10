#include <QCoreApplication>
#if defined(CMD_BCR_IDC)    // IDC命令系
#include "XFS_BCR_IDC/XFS_BCR_IDC.h"
#else                       // BCR命令系
#include "XFS_BCR/XFS_BCR.h"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);// 调用此行，让后面能取得到命令行参数
    //std::this_thread::sleep_for(std::chrono::milliseconds(20 * 1000));

    CXFS_BCR gXFS;
    gXFS.StartRun();
    return 0;
}
