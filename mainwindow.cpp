#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "table_controller.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTime>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_tablectrl = new table_controller(ui->TableView);

    QWidget::showMaximized();
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
    m_tablectrl->processLog(filename);
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
    if (m_tablectrl == NULL ||
            m_tablectrl->checkConfigValid() == false) {
        QMessageBox::about(this, tr("check json file failed"),
                                    tr("please make sure the log cofing json file locate in the correct position and is valid"));
        this->close();
    }
}

void MainWindow::logFilterOKClicked() {
    log_filter_t filter;
    filter.msg = ui->msg_combobox->currentText();

    // remove the duplicate items
    QStringList list;
    list.append(filter.msg);
    for (int i = 0; i < ui->msg_combobox->count(); ++i) {
        QString msg = ui->msg_combobox->itemText(i);
        if (list.contains(msg)) continue;
        list.append(msg);
    }
    ui->msg_combobox->clear();
    ui->msg_combobox->addItems(list);

    qDebug() << "list = " << list << endl;
    filter.tag = ui->tag_edit->text();
    filter.pid = ui->pid_edit->text();
    filter.tid = ui->tid_edit->text();
    //filter.line = ui->line_edit->text().toInt();
    qDebug() << "filter info: " << filter.msg << filter.tag << filter.pid << filter.tid << filter.line << endl;

    m_tablectrl->processFilter(filter);
}
void MainWindow::logFilterClearClicked() {
    ui->msg_combobox->clearEditText();
    ui->tag_edit->clear();
    ui->pid_edit->clear();
    ui->tid_edit->clear();
    m_tablectrl->showAllLogs();
    //ui->line_edit->clear();
}

void MainWindow::date_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setColumnVisible(TABLE_COL_TYPE_DATE, clicked);
}

void MainWindow::time_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setColumnVisible(TABLE_COL_TYPE_TIME, clicked);
}

void MainWindow::pid_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setColumnVisible(TABLE_COL_TYPE_PID, clicked);
}

void MainWindow::tid_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setColumnVisible(TABLE_COL_TYPE_TID, clicked);
}

void MainWindow::level_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setColumnVisible(TABLE_COL_TYPE_LEVEL, clicked);
}

void MainWindow::tag_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setColumnVisible(TABLE_COL_TYPE_TAG, clicked);
}

void MainWindow::msg_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setColumnVisible(TABLE_COL_TYPE_MSG, clicked);
}

void MainWindow::info_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setLogLevelVisible(LOG_LEVEL_INFO, clicked);
}

void MainWindow::debug_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setLogLevelVisible(LOG_LEVEL_DEBUG, clicked);
}

void MainWindow::verbose_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setLogLevelVisible(LOG_LEVEL_VERBOSE, clicked);
}

void MainWindow::warn_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setLogLevelVisible(LOG_LEVEL_WARN, clicked);
}

void MainWindow::error_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setLogLevelVisible(LOG_LEVEL_ERROR, clicked);
}

void MainWindow::fatal_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    m_tablectrl->setLogLevelVisible(LOG_LEVEL_FATAL, clicked);
}
