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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    void myShow();
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    table_controller *m_tablectrl;
    table_model *m_model;
    adb_online *m_adb;
    //af_debugger *m_afDebugger;
    QString m_window_title;

    QTimer check_adb_device_tiemr;
    QStringList parseLogFilterText(QString text);

    QFontDialog m_font_dialog;

    goto_line_dialog *m_line_dialog;
    persist_settings *m_persist_settings;
    screen_snapshot *m_snapshot;
    QString m_log_open_path;

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

    void table_view_double_clicked(QModelIndex index);

    void recieveLineNumber(int);

    void logCopy();
};

#endif // MAINWINDOW_H
