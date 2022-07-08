#include "cwframewidget.h"
#include <QPainter>

CwFrameWidget::CwFrameWidget(QWidget* ptr)
    : QOpenGLWidget(ptr)
{

}

CwFrameWidget::~CwFrameWidget()
{

}

void CwFrameWidget::paintEvent(QPaintEvent *e)
{
    QPainter p;
    p.begin(this);
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        p.drawImage(QPoint(0, 0), m_frame);
    }

    p.end();
}

void CwFrameWidget::remove(){
    if(!m_frame.isNull())
    {
        memset(m_frame.bits(), 0, m_frame.width() * m_frame.height() * 3);
    }

}

void CwFrameWidget::refreshFrame(const uchar* data, int w, int h, int c)
{
    if(nullptr == data) {
        return ;
    }

    int width = this->width(); int height = this->height();
    QImage::Format format = (3 == c ? QImage::Format_RGB888 : QImage::Format_Grayscale8);
    if(m_frame.isNull()){
        size_t lenght = width * height * c;
        uchar* buf = (uchar*)malloc(lenght);
        memset(buf, 0x00, lenght);
        m_frame = QImage(buf, width, height, format);
    }
    else if(m_frame.format() != format){
        free(m_frame.bits());
        size_t lenght = width * height * c;
        uchar* buf = (uchar*)malloc(lenght);
        memset(buf, 0x00, lenght);
        m_frame = QImage(buf, width, height, format);
    }

    if((width != w) || (height != h)){
        uint type = (3 == c ? CV_8UC3 : CV_8UC1);
        cv::Mat srcMat(h, w, type, (char*)data);
        cv::Mat dstMat;
        cv::resize(srcMat, dstMat, cv::Size(width, height));
        std::lock_guard<std::mutex> locker(m_mutex);
        memcpy(m_frame.bits(), dstMat.data, dstMat.cols * dstMat.rows * dstMat.channels());
    }
    else if((width == w) && (height == h)){
        std::lock_guard<std::mutex> locker(m_mutex);
        memcpy(m_frame.bits(), data, w * h * c);
    }
}
