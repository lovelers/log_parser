#include "myopenglwidget.h"
#include <QPainter>
#include <QDebug>
MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    :QOpenGLWidget(parent)
{
    m_rectPen.setColor(QColor(0, 0, 0));
    m_rectPen.setWidth(2);
}

void MyOpenGLWidget::initializeGL()
{

}

void MyOpenGLWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter;
    painter.begin(this);
    painter.fillRect(e->rect(), m_backgroundBrush);
    //painter.translate(100, 100);

    painter.save();
    painter.setPen(m_rectPen);
    painter.drawRect(m_selectRect);
    painter.restore();
    painter.end();
}

void MyOpenGLWidget::setDefaultImage(const QImage &image)
{
    m_sourceImage = image;
    m_selectRect.setCoords(0, 0, -1, -1);
    m_sourceGrayImage = image;
    int height = m_sourceGrayImage.height();
    int width = m_sourceGrayImage.width();
    for (int h = 0; h < height; ++h)
    {
        for (int w = 0; w < width; ++w)
        {
            QColor rgb = m_sourceGrayImage.pixelColor(w, h);
            //rgb *= 0.8f;
            rgb.setRedF(rgb.redF()* 0.8f);
            rgb.setGreenF(rgb.greenF() * 0.8f);
            rgb.setBlueF(rgb.blueF() * 0.8f);
            m_sourceGrayImage.setPixelColor(w, h, rgb);
        }
    }

    m_backgroundBrush.setTextureImage(m_sourceGrayImage);
}

void MyOpenGLWidget::setSelectRect(const QRect& rect)
{
    m_selectRect= rect;
    QImage showImage = m_sourceGrayImage;
    int rectw = rect.left();
    int recth = rect.top();
    int rectwidth = rect.width();
    int rectheight = rect.height();
    int width = m_sourceImage.width();
    int height = m_sourceImage.height();

    for (int h = 0; h < height; ++h)
    {
        for (int w = 0; w < width; ++w)
        {
            if (w >rectw && w < rectw+rectwidth
                    && h > recth && h < recth+rectheight)
            {
                showImage.setPixelColor(w, h, m_sourceImage.pixelColor(w, h));
            }
        }
    }
    m_backgroundBrush.setTextureImage(showImage);
}

QImage MyOpenGLWidget::getSelectImage()
{
    return m_sourceImage.copy(m_selectRect);
}
