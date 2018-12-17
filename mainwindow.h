#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QFontDialog>
#include <QTimer>
#include "table_controller.h"
#include "table_model.h"
#include "adb_online.h"
#include "goto_line_dialog.h"
#include "persist_settings.h"
#include "screen_snapshot.h"
namespace Ui {
class MainWindow;
}

class CheckDeviceRealtimeThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void myShow();
    ~MainWindow();

private:

    Ui::MainWindow*             ui;
    QString                     mWindowTitle;
    QString                     mLogOpenPath;
    table_controller*           mpTableCtrl;
    adb_online*                 mpAdbOnline;
    QFontDialog                 mFontDialog;
    goto_line_dialog*           mpLineDialog;
    persist_settings*           mpPersistSettings;
    screen_snapshot*            mpSnapshot;
    CheckDeviceRealtimeThread*  mpCheckDevices;

    QStringList parseLogFilterText(QString text);

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent * event);
public Q_SLOTS:
    void openLog();
    void adbConnect();
    void recentlyFiles();
    void font();
    void goto_line();
    void do_screen_shot();
    void persistSettings();

    void version();

    // log filter
    void logFilterReturnPress();
    void logFilterClearClicked();

    // log columns
    void date_cb_checked(bool clicked);
    void time_cb_checked(bool clicked);
    void pid_cb_checked(bool clicked);
    void tid_cb_checked(bool clicked);
    void tag_cb_checked(bool checked);
    void level_cb_checked(bool checked);
    void msg_cb_checked(bool checked);

    // log levels
    void info_cb_checked(bool clicked);
    void debug_cb_checked(bool clicked);
    void verbose_cb_checked(bool clicked);
    void warn_cb_checked(bool clicked);
    void error_cb_checked(bool clicked);
    void fatal_cb_checked(bool clicked);

    // android devices
    void android_devices_select();
    void android_run();
    void android_pause_resume();
    void android_stop();
    void android_clear();

    void logOnlinePath(QString path);
    void logStopUnexpected();

    void table_view_double_clicked(QModelIndex index);

    void selectLine(int);

    void logCopy();
private slots:
    void on_msg_combobox_editTextChanged(const QString &arg1);
};
class CheckDeviceRealtimeThread : public QThread
{
    Q_OBJECT
    void run()
    {
        if (m != nullptr)
        {
            m->android_devices_select();
        }
    }
public:
    CheckDeviceRealtimeThread(MainWindow *main)
    {
        m   = main;
        QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(Start()));
    }
    void EventLoop()
    {
        timer.start(1000);
    }
    void ExitEventLoop()
    {
        timer.stop();

        this->exit();
        int try_count = 0;

        while(try_count < 100 || !this->isFinished())
        {
            QThread::sleep(10);
            try_count++;
        }
        if (false == this->isFinished())
        {
            qDebug("wait finisned timeout");
        }
    }
private Q_SLOTS:
    void Start()
    {
        this->start();
    }
private:
    MainWindow *m;
    QTimer      timer;
};
#endif // MAINWINDOW_H
