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

void CwFrameWidget::remove()
{
    if(!m_frame.isNull())
    {
        memset(m_frame.bits(), 0, m_frame.width() * m_frame.height() * 3);
    }
}

void CwFrameWidget::refreshFrame(const uchar* data, int w, int h, int c)
{
    if(nullptr == data)
    {
        return ;
    }

    int width = this->width();
    int height = this->height();
    size_t lenght = width * height * c;

    QImage::Format format = QImage::Format_Grayscale8;
    uint type = CV_8UC1;
    switch(c)
    {
        case 3:
            format = QImage::Format_RGB888;
            type = CV_8UC3;
            break;
        case 4:
            format = QImage::Format_ARGB32;
            type = CV_8UC4;
            break;
    }

    /*if(m_frame.format() != format && !m_frame.isNull())
    {
        free(m_frame.bits());
    }*/

    if(m_frame.isNull())
    {
        uchar* buf = nullptr;
        if (c == 4)
        {
            buf = (uchar*)malloc(sizeof(int) * lenght);
            memset(buf, 0x00, sizeof(int) * lenght);
            m_frame = QImage((uchar*)buf, width, height, QImage::Format_ARGB32);
        } else
        {
            buf = (uchar*)malloc(lenght);
            memset(buf, 0x00, lenght);
            m_frame = QImage((uchar*)buf, width, height, format);
        }
    } else
    if(m_frame.format() != format)
    {
        free(m_frame.bits());
        uchar* buf = nullptr;
        if (c == 4)
        {
            buf = (uchar*)malloc(sizeof(int) * lenght);
            memset(buf, 0x00, sizeof(int) * lenght);
            m_frame = QImage((uchar*)buf, width, height, QImage::Format_ARGB32);
        } else
        {
            buf = (uchar*)malloc(lenght);
            memset(buf, 0x00, lenght);
            m_frame = QImage((uchar*)buf, width, height, format);
        }
    }

    if((width != w) || (height != h))
    {
        //uint type = (3 == c ? CV_8UC3 : CV_8UC1);
        if (type == CV_8UC4)
        {
            QImage qImgBuffer = QImage((uchar*)data, w, h, QImage::Format_ARGB32);
            cv::Mat srcMat(qImgBuffer.height(), qImgBuffer.width(), type,
                           (void*)qImgBuffer.constBits(), qImgBuffer.bytesPerLine());
            cv::Mat dstMat;
            cv::resize(srcMat, dstMat, cv::Size(width, height));
            std::lock_guard<std::mutex> locker(m_mutex);
            memcpy(m_frame.bits(), dstMat.data, dstMat.cols * dstMat.rows * dstMat.channels());
        } else
        {
            cv::Mat srcMat(h, w, type, (char*)data);
            cv::Mat dstMat;
            cv::resize(srcMat, dstMat, cv::Size(width, height));
            std::lock_guard<std::mutex> locker(m_mutex);
            memcpy(m_frame.bits(), dstMat.data, dstMat.cols * dstMat.rows * dstMat.channels());
        }

    } else
    if((width == w) && (height == h))
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        memcpy(m_frame.bits(), data, w * h * c);
    }
}
