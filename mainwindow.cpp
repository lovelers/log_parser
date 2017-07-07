#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "log_parser_widget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTime>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_logParser = new log_parser(ui);
#if 0
    m_afDebugger = new af_debugger();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openLog(){
    qDebug() << "openLog" << endl;
    QString filename = QFileDialog::getOpenFileName(this, tr("open log"), ".", tr("log file (*.txt *.log)"));
    if (filename.isEmpty()) {
        qDebug() << "file is empty()" << endl;
        return;
    }
    m_logParser->processLog(filename);
}

void MainWindow::adbConnect() {
    qDebug() << "adbConnect" << endl;
}

void MainWindow::recentlyFiles() {
    qDebug() << "recentlyFiles" << endl;
}

void MainWindow::afDebugger() {
#if 0
    qDebug() << "afDebugger" << endl;
    if (m_afDebugger) m_afDebugger->show();
    if (m_afDebugger->isVisible()) {
        QString filename = "C:/Work/Cygwin/home/zixiangz/af_debugger/log.txt";
        m_afDebugger->processLog(filename);
    }
#endif
}

void MainWindow::persistSettings() {
    qDebug() << "persistSettings" << endl;
}
void MainWindow::version() {
    qDebug() << "version 1.0" << endl;
}

void MainWindow::myExit() {
    qDebug() << "myExit" << endl;
    this->close();
}

void MainWindow::myShow() {
    this->show();
    if (m_logParser == NULL ||
            m_logParser->checkConfigValid() == false) {
        QMessageBox::about(this, tr("check json file failed"),
                                    tr("please make sure the log cofing json file locate in the correct position and is valid"));
        this->close();
    }
}

void MainWindow::logFilterOnClick() {
    m_logParser->logFilterOnClick();
}
