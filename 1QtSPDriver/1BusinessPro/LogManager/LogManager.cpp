#include "LogManager.h"
#include "ui_LogManager.h"
#include <QSystemTrayIcon>

//////////////////////////////////////////////////////////////////////////
CLogManager::CLogManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CLogManager),
    m_cMemR(SHAREMEMORY_MUTEX)
{
    // 创建读写共享内存
    m_cMemR.Open(SHAREMEMORY_NAME, true, sizeof(LOGDATA));

    // 托盘图标
    m_sysTray.setIcon(QIcon(":/Image/res/icon.png"));
    m_sysTray.setToolTip("LogManager");

    //托盘菜单项
    QMenu *qMenu = new QMenu(this);
    QAction *qAction = new QAction("Exit LogManager", qMenu);
    qMenu->addAction(qAction);
    m_sysTray.setContextMenu(qMenu);

    //托盘菜单响应
    connect(qAction, SIGNAL(triggered()), this, SLOT(OnExit()));

    //显示托盘
    m_sysTray.show();

    // 启动线程
    ThreadStart();
    m_cLog.ThreadStart();
    m_cBackLog.ThreadStart();       // 启动日志备份线程

    ui->setupUi(this);
}

CLogManager::~CLogManager()
{
    delete ui;
}

void CLogManager::OnExit()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////
void CLogManager::Run()
{
    LOGDATA stLogData;
    while (!m_bQuitRun)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (!m_cMemR.Read(&stLogData, sizeof(LOGDATA)))
            continue;

        // 读到日志
        m_cLog.AddLog(stLogData);
    }
    return;
}

