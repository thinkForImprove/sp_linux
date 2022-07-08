#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSharedMemory>

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;


public:
    void Release();
    void vTimerStart(unsigned int t);
    unsigned int nConnSharedMem(const char *lpSharedNameData);
    void ShowWin(int x, int y, int w, int h);
    void HideWin();

private:
    QTimer qTimerOut_Data;
    QSharedMemory *m_qSharedMem_Data; //定义共享内存实例指针
    unsigned char *m_szSharedBuff_Data;

public slots:
    void vRefreshFrame();

};

#endif // MAINWINDOW_H
