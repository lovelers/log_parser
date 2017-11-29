#ifndef SCREEN_SNAPSHOT_H
#define SCREEN_SNAPSHOT_H

#include <QDialog>
#include <QWidget>
#include <QWindow>
#include <QScreen>
#include <QPixmap>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>

namespace Ui {
class screen_snapshot;
}

class mouse_operation
{
private:
    bool is_pressed;
    bool is_released;
    bool is_moving;
    QPoint pressed_point;
    QPoint released_point;
    QPoint moving_point;
public:
    mouse_operation();
    void reset();
    void setPressed(const QPoint &pos);
    void setMoving(const QPoint &pos);
    void setReleased(const QPoint &pos);

    bool getSelectRect(QRect *rect);
};

class screen_snapshot : public QDialog
{
    Q_OBJECT

public:
    explicit screen_snapshot(QWidget *parent = 0);
    ~screen_snapshot();
    void take_shot(const QRect &rect);
private:
    Ui::screen_snapshot *ui;

private:
    WId m_wid;
    QWindow *m_window;
    mouse_operation m_mouse_operation;
    QPushButton *m_ok;
    QPushButton *m_cancel;
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

public slots:
    void okOnClicked();

};

#endif // SCREEN_SNAPSHOT_H
