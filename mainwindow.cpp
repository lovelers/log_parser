#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "table_controller.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTime>
#include <QStringList>
#include <QString>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_tablectrl = new table_controller(ui->TableView);
    m_adb = new adb_online();
    QObject::connect(m_adb, SIGNAL(processLogOnline(QByteArray)),
                     m_tablectrl, SLOT(processLogOnline(QByteArray)));
    ui->android_stop_btn->setEnabled(false);
    QWidget::showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_tablectrl) {
        delete m_tablectrl;
        m_tablectrl = NULL;
    }
    if (m_adb) {
        delete m_adb;
        m_adb = NULL;
    }
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
    QString cur_msg = ui->msg_combobox->currentText();

    // remove the duplicate items of ui
    QStringList list;
    list.append(cur_msg);
    for (int i = 0; i < ui->msg_combobox->count(); ++i) {
        QString msg = ui->msg_combobox->itemText(i);
        if (list.contains(msg)) continue;
        list.append(msg);
    }
    ui->msg_combobox->clear();
    ui->msg_combobox->addItems(list);

    qDebug() << "msg combobox = " << list;
    // parse the msg list for filter
    if (!cur_msg.isEmpty()) {
        int index= 0;
        int start_offset = 0;
        while (true) {
            index = cur_msg.indexOf("|", start_offset);
            if (index == -1) {
                filter.msg.append(cur_msg.mid(start_offset, -1));
                break;
            } else {
                filter.msg.append(cur_msg.mid(start_offset, index - start_offset));
                start_offset = index+1;
            }
        }
    }
    qDebug() << "msg filter: " << filter.msg;

    filter.tag = ui->tag_edit->text();
    filter.pid = ui->pid_edit->text();
    filter.tid = ui->tid_edit->text();
    qDebug() << filter.tag << filter.pid << filter.tid << filter.line << endl;

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

// log columns
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


// log levels
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

// android devices
void MainWindow::android_devices_select() {
    if (m_adb) {
        QStringList list = m_adb->checkDevices();
        ui->android_devices_cb->clear();
        if (list.size() == 0) {
            ui->android_devices_cb->insertItem(0, "devices disconnect");
        }
        for (int i = 0; i < list.size(); i++) {
            ui->android_devices_cb->insertItem(i, list.at(i));
        }

    }
}

void MainWindow::android_run() {
    if (m_adb) {
        m_adb->setCmd(ANDROID_RUN);
    }
    if (m_tablectrl) {
        m_tablectrl->android_run();
    }
    this->ui->android_run_btn->setEnabled(false);
    this->ui->android_pause_resume_btn->setEnabled(true);
    this->ui->android_stop_btn->setEnabled(true);
    this->ui->android_clear_btn->setEnabled(true);
}

void MainWindow::android_pause_resume() {
    if (ui->android_pause_resume_btn->text().compare("Pause") == 0) {
        if (m_adb) {
            m_adb->setCmd(ANDROID_PAUSE);
        }
        if (m_tablectrl) {
            m_tablectrl->android_pause();
        }
        ui->android_pause_resume_btn->setText("Resume");
    } else {
        if (m_adb) {
            m_adb->setCmd(ANDROID_RESUME);
        }
        if (m_tablectrl) {
            m_tablectrl->android_resume();
        }
        ui->android_pause_resume_btn->setText("Pause");
    }
    this->ui->android_run_btn->setEnabled(false);
    this->ui->android_pause_resume_btn->setEnabled(true);
    this->ui->android_stop_btn->setEnabled(true);
    this->ui->android_clear_btn->setEnabled(true);
}

void MainWindow::android_stop() {
    if (m_adb) {
        m_adb->setCmd(ANDROID_STOP);
    }
    if (m_tablectrl) {
        m_tablectrl->android_stop();
    }
    this->ui->android_run_btn->setEnabled(true);
    this->ui->android_pause_resume_btn->setEnabled(false);
    this->ui->android_stop_btn->setEnabled(false);
    this->ui->android_clear_btn->setEnabled(false);
}

void MainWindow::android_clear() {
    if (m_adb) {
        m_adb->setCmd(ANDROID_CLEAR);
    }
    if (m_tablectrl) {
        m_tablectrl->android_clear();
    }
}
