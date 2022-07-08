#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#pragma once
#define LINUX

#include <QWidget>
#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include <qthread.h>
#include <QSharedMemory>
#include <QProcess>

#include <qprocess.h>

#include <mutex>
#include <QOpenGLWidget>
#include "opencv2/opencv.hpp"

class CwFrameWidget : public QOpenGLWidget
{
public:
    explicit CwFrameWidget(QWidget* ptr);
    virtual ~CwFrameWidget();
    virtual void paintEvent(QPaintEvent *e);

 public slots:
    void remove();
    void refreshFrame(const uchar* data, int w, int h, int c);
private:
    std::mutex m_mutex;
    QImage m_frame;
};

#endif // FRAMEWIDGET_H
