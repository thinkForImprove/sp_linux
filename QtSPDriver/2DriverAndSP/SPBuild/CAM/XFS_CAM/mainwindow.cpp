#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_qSharedMem_Data = nullptr;
    m_szSharedBuff_Data = nullptr;

    connect(&qTimerOut_Data, SIGNAL(timeout()), this, SLOT(vRefreshFrame()));
}

MainWindow::~MainWindow()
{
    Release();
}

void MainWindow::Release()
{
    qTimerOut_Data.stop();

    if (m_qSharedMem_Data != nullptr)
    {
        if (m_qSharedMem_Data->isAttached())   // 检测程序当前是否关联共享内存
            m_qSharedMem_Data->detach();      // 解除关联
        delete m_qSharedMem_Data;
        m_qSharedMem_Data = nullptr;
    }

    if (m_szSharedBuff_Data != nullptr)
    {
        free(m_szSharedBuff_Data);
        m_szSharedBuff_Data = nullptr;
    }

    delete ui;
}

unsigned int MainWindow::nConnSharedMem(const char *lpSharedNameData)
{
    m_qSharedMem_Data = new QSharedMemory(lpSharedNameData);   // 构造实例对象 + 为实例对象指定关键字(给共享内存命名)

    if (m_qSharedMem_Data->attach(QSharedMemory::ReadOnly) != true)    // 只读方式关联
    {
        if (m_qSharedMem_Data->isAttached() != true)   // 判断共享内存的关联状态
            return 1;
    }

    if (m_qSharedMem_Data->size() < 1)
    {
        return 2;
    }

    m_szSharedBuff_Data = (unsigned char*)malloc(sizeof(unsigned char) * m_qSharedMem_Data->size());
    if (m_szSharedBuff_Data == nullptr)
    {
        m_szSharedBuff_Data = (unsigned char*)malloc(sizeof(unsigned char) * m_qSharedMem_Data->size());
        if (m_szSharedBuff_Data == nullptr)
        {
            m_szSharedBuff_Data = (unsigned char*)malloc(sizeof(unsigned char) * m_qSharedMem_Data->size());
            if (m_szSharedBuff_Data == nullptr)
            {
                return 3;
            }
        }
    }

    return 0;
}

void MainWindow::vTimerStart(unsigned int t)
{
    qTimerOut_Data.start(t);
}

void MainWindow::ShowWin(int x, int y, int w, int h)
{
    Qt::WindowFlags qtWFlags = windowFlags();
    qtWFlags &= (~(Qt::WindowMaximizeButtonHint |
                   Qt::WindowMinimizeButtonHint |
                   Qt::WindowCloseButtonHint));       // 兼容低版本  // 30-00-00-00(FT#0055)
    this->setWindowFlags(qtWFlags | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::BypassWindowManagerHint);
    this->resize(w, h);
    this->move(x, y);
    ui->CamVideo->resize(w,h);
    ui->CamVideo->move(0, 0);
    //this->show();     // 窗口内容以共享内存为准,避免上一次窗口内容遗留,窗口显示放在vRefreshFrame()
}

void MainWindow::HideWin()
{
    this->hide();
}

void MainWindow::vRefreshFrame()
{    
    if (m_qSharedMem_Data->lock())     // 锁定共享内存
    {
        memset(m_szSharedBuff_Data, 0x00, m_qSharedMem_Data->size());
        memcpy(m_szSharedBuff_Data, m_qSharedMem_Data->constData(), m_qSharedMem_Data->size());
        m_qSharedMem_Data->unlock();
    }

    int w = m_szSharedBuff_Data[0] * 255 + m_szSharedBuff_Data[1];    // Width,Mat.Clos
    int h = m_szSharedBuff_Data[2] * 255 + m_szSharedBuff_Data[3];    // Heignt,Mat.Rows
    int c = m_szSharedBuff_Data[4];

    if (w == 0 || h == 0)
    {
        m_qSharedMem_Data->unlock();
        return;
    } else
    {
        // 非异常数据,窗口未显示则Show
        if (this->isVisible() == false)
        {
            this->show();
        }
    }

    // 避免出现缩放时图像渲染错位
    // Cloudwalk返回视频流width值必须以cloudwalk_action.ini中TraceTrimWidth+(4*X)为标准
    // 即一个图像扫描行所占的字节数必须是4的倍数
    // 根据Display指定Width,验证是否符合TraceTrimWidth+(4*X),不符合则以差值重新设定Width
    if (this->width() != w)                                                         // 40-00-00-00(FT#0014)
    {                                                                               // 40-00-00-00(FT#0014)
        int nDisp = (w > this->width() ? w - this->width() : this->width() - w);    // 40-00-00-00(FT#0014)
        int nCount4 = (nDisp / 4) + ((nDisp % 4) >= 2 ? 1 : 0);                     // 40-00-00-00(FT#0014)
        int w1 = (w > this->width() ? w - nCount4 * 4 : w + nCount4 * 4);           // 40-00-00-00(FT#0014)
        if (this->width() != w1 || this->height() != h)                             // 40-00-00-00(FT#0014)
        {                                                                           // 40-00-00-00(FT#0014)
            this->resize(w1, this->height());                                       // 40-00-00-00(FT#0014)
            ui->CamVideo->resize(w1, this->height());                               // 40-00-00-00(FT#0014)
        }                                                                           // 40-00-00-00(FT#0014)
    }                                                                               // 40-00-00-00(FT#0014)

    ui->CamVideo->refreshFrame(&(m_szSharedBuff_Data[5]), w, h, c);
    ui->CamVideo->update();
}
