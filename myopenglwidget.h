#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H
#include <QOpenGLWidget>
#include <QPainter>
#include <QBrush>
#include <QImage>
#include <QColor>
#include <QRgb>
#include <QPaintEvent>
#include <QRect>
class MyOpenGLWidget : public QOpenGLWidget
{
public:
    MyOpenGLWidget(QWidget *parent);
    void setDefaultImage(const QImage &image);
    void setSelectRect(const QRect &rect);

    QImage getSelectImage();
private:
    void paintEvent(QPaintEvent *e);
    void initializeGL();
    QPen m_rectPen;
    QBrush m_backgroundBrush;
    QImage m_sourceImage;
    QImage m_sourceGrayImage;
    QRect m_selectRect;
};

#endif // MYOPENGLWIDGET_H
