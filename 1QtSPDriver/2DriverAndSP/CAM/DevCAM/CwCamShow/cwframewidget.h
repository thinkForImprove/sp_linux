#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H
#include <mutex>
#include <QOpenGLWidget>

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
