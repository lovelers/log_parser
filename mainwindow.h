#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
//#include "af_debugger.h"
class log_parser;
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
    log_parser *m_logParser;
    //af_debugger *m_afDebugger;


public Q_SLOTS:
    void openLog();
    void adbConnect();
    void recentlyFiles();

    void myExit();

    void afDebugger();
    void persistSettings();

    void version();

    // log filter
    void logFilterOnClick();
};

#endif // MAINWINDOW_H
