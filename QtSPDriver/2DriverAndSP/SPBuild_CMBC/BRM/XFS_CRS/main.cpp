#include "XFS_CRS.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);// 调用此行，让后面能取得到命令行参数
    //std::this_thread::sleep_for(std::chrono::milliseconds(10 * 1000));

    XFS_CRSImp gXFS;
    gXFS.StartRun();
    return 0;
}
