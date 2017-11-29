#include "screen_snapshot.h"
#include "ui_screen_snapshot.h"
#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>
#include <QClipboard>
screen_snapshot::screen_snapshot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::screen_snapshot)
{
    ui->setupUi(this);
    this->setMinimumSize(300, 300);
    this->setMouseTracking(true);
    if (NULL != parent) {
        m_wid = parent->winId();
        m_window = parent->windowHandle();
    } else {
        m_wid = 0;
        m_window = NULL;
    }
    m_ok = new QPushButton(this);
    m_ok->setVisible(false);
    m_ok->setText("OK");
    QObject::connect(m_ok, SIGNAL(pressed()),
                     this, SLOT(okOnClicked()));
    //m_cancel.setText("Cancel");
}

void screen_snapshot::take_shot(const QRect &rect) {
    if (NULL == m_window) {
        qDebug() << "bad window handle";
        return;
    }
    this->setGeometry(rect);
    m_mouse_operation.reset();
    QScreen *screen = m_window->screen();
    QPixmap pixmap = screen->grabWindow(m_wid, 0, 0, -1, -1);
    ui->screenGL->setDefaultImage(pixmap.toImage());
    m_ok->setVisible(false);
    this->show();
}

void screen_snapshot::mousePressEvent(QMouseEvent *event) {
    //qDebug() << "mousePressEvent:" << event->x()  <<"," << event->y();
    m_mouse_operation.setPressed(event->pos());
    m_ok->setVisible(false);
}

void screen_snapshot::mouseReleaseEvent(QMouseEvent *event) {
    //qDebug() << "mouseReleaseEvent" << event->x() << ", " << event->y();
    m_mouse_operation.setReleased(event->pos());
    QRect rect;
    if (true == m_mouse_operation.getSelectRect(&rect))
    {
        ui->screenGL->setSelectRect(rect);
        ui->screenGL->update();
        m_mouse_operation.reset();
        QPoint pos = event->pos();
        m_ok->setGeometry(pos.x(), pos.y(), pos.x()+20, pos.y()+10);
        m_ok->setMaximumHeight(20);
        m_ok->setMaximumWidth(30);
        //m_cancel.setG
        m_ok->setVisible(true);
    }
}

void screen_snapshot::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "mouseMoveEvent" << event->x() << "," << event->y();
    m_mouse_operation.setMoving(event->pos());
    QRect rect;
    m_ok->setVisible(false);
    if (true == m_mouse_operation.getSelectRect(&rect))
    {
        ui->screenGL->setSelectRect(rect);
        ui->screenGL->update();
    }
}

void screen_snapshot::okOnClicked()
{
    QApplication::clipboard()->setImage(ui->screenGL->getSelectImage());
    this->close();
}

screen_snapshot::~screen_snapshot()
{
    delete ui;
}

/////////////////////////mouse_operation////////////////////
mouse_operation::mouse_operation()
{
    reset();
}

void mouse_operation::reset()
{
    is_pressed = false;
    is_released = false;
    is_moving = false;
}

void mouse_operation::setPressed(const QPoint &pos)
{
    is_pressed = true;
    pressed_point = pos;
}

void mouse_operation::setMoving(const QPoint &pos)
{
    is_moving = true;
    moving_point = pos;
}

void mouse_operation::setReleased(const QPoint &pos)
{
    is_released = true;
    released_point = pos;
}
bool mouse_operation::getSelectRect(QRect *rect)
{
    QPoint start;
    QPoint end;
    if (false == is_pressed) return false;
    if (NULL != rect)
    {
        if (true == is_released)
        {
            //use the pressed_point and released_point.
            start = pressed_point;
            end = released_point;
        }
        else
        {
            //use the pressed_point and moving_point;
            start = pressed_point;
            end = moving_point;
        }
        if (start.x() < end.x()
                && start.y() < end.y())
        {
            rect->setTopLeft(start);
            rect->setBottomRight(end);
        }
        else if (start.x() < end.x()
                 && start.y() > end.y())
        {
            rect->setTopRight(end);
            rect->setBottomLeft(start);
        }
        else if (start.x() > end.x()
                 && start.y() > end.y())
        {
            rect->setTopLeft(end);
            rect->setBottomRight(start);
        }
        else {
            rect->setTopRight(start);
            rect->setBottomLeft(end);
        }
        return true;
    }
    return false;
}
