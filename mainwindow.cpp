#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "table_controller.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTime>
#include <QStringList>
#include <QString>
#include "goto_line_dialog.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_tablectrl = new table_controller(ui->TableView);
    m_adb = new adb_online();
    m_line_dialog = new goto_line_dialog();
    m_persist_settings = new persist_settings();
    QObject::connect(m_adb, SIGNAL(processLogOnline(QByteArray, int)),
                     m_tablectrl, SLOT(processLogOnline(QByteArray, int)));
    QObject::connect(ui->msg_combobox->lineEdit(), SIGNAL(returnPressed()),
                     this, SLOT(logFilterReturnPress()));
    QObject::connect(ui->tag_edit, SIGNAL(returnPressed()),
                     this, SLOT(logFilterReturnPress()));
    QObject::connect(ui->pid_edit, SIGNAL(returnPressed()),
                     this, SLOT(logFilterReturnPress()));
    QObject::connect(ui->tid_edit, SIGNAL(returnPressed()),
                     this, SLOT(logFilterReturnPress()));

    QObject::connect(m_adb, SIGNAL(setLogTitle(QString)),
                     this, SLOT(setLogTitle(QString)));

    m_line_dialog->setModal(true);
    QObject::connect(m_line_dialog, SIGNAL(sendLineNumber(int)),
                     this, SLOT(recieveLineNumber(int)));
    ui->android_stop_btn->setEnabled(false);

    m_window_title.append("log parser");
    this->setWindowTitle(m_window_title);

    QObject::connect(&check_adb_device_tiemr, SIGNAL(timeout()),
                     this, SLOT(android_devices_select()));
    check_adb_device_tiemr.start(1000);
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
    this->android_stop();
    QString filename = QFileDialog::getOpenFileName(this, tr("open log"), ".", tr("log file (*.txt *.log *log*)"));
    if (filename.isEmpty()) {
        qDebug() << "file is empty()" << endl;
        return;
    }
    this->setLogTitle(filename);
    m_tablectrl->processLog(filename);
}

void MainWindow::adbConnect() {
    qDebug() << "adbConnect" << endl;
}

void MainWindow::recentlyFiles() {
    qDebug() << "recentlyFiles" << endl;
}

void MainWindow::font() {
    bool ok = false;
    qDebug() << " goto font" ;
    QFont font = QFontDialog::getFont(&ok, QFont(), this);
    if (ok) {
        m_tablectrl->setFont(font);
    }
}

void MainWindow::goto_line() {
    qDebug() << "goto here";
    //m_line_dialog->setFocus();
    m_line_dialog->show();
}

void MainWindow::persistSettings() {
    qDebug() << "persistSettings" << endl;
    if (m_persist_settings) m_persist_settings->show();
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

void MainWindow::logFilterReturnPress() {
    log_filter_t filter;

    QString cur_msg = ui->msg_combobox->currentText();
    filter.msg = parseLogFilterText(cur_msg);
    filter.tag = parseLogFilterText(ui->tag_edit->text());
    filter.pid = parseLogFilterText(ui->pid_edit->text());
    filter.tid = parseLogFilterText(ui->tid_edit->text());
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
    qDebug() << "fitler msg:" << filter.msg;
    qDebug() << "filter tag:" << filter.tag;
    qDebug() << "filter pid:" << filter.pid;
    qDebug() << "filter tid:" << filter.tid;
    m_tablectrl->processFilter(filter);
    //ui->TableView->setFocus();
}

QStringList MainWindow::parseLogFilterText(QString text) {
    QStringList list;
    if (!text.isEmpty()) {
        int index= 0;
        int start_offset = 0;
        while (true) {
            index = text.indexOf("|", start_offset);
            QString msg;
            if (index == -1) {
                msg = text.mid(start_offset, -1);
                if (!msg.isEmpty())  list.append(msg);
                break;
            } else {
                msg = text.mid(start_offset, index - start_offset);
                if (!msg.isEmpty()) list.append(msg);
                start_offset = index+1;
            }
        }
    }
    return list;
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
    static QStringList pre_list;
    if (m_adb) {
        QStringList list = m_adb->checkDevices();
        if (list.size() == 0)
            list << "adb disconnect";
        bool needUpdate = false;
        if (list.size() == pre_list.size()) {
            for (int i = 0; i < list.size(); i++) {
                if (!pre_list.contains(list.at(i))) {
                    needUpdate = true;
                    break;
                }
            }
        } else {
            needUpdate = true;
        }
        if (needUpdate == true) {
            qDebug() << "need update new list of devcies";
            ui->android_devices_cb->clear();
            for (int i = 0; i < list.size(); i++) {
                ui->android_devices_cb->insertItem(i, list.at(i));
            }
        }
        pre_list = list;
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

    //check_adb_device_tiemr.start(1000);
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
    ui->android_pause_resume_btn->setText("Pause");
    this->ui->android_pause_resume_btn->setEnabled(false);
    this->ui->android_stop_btn->setEnabled(false);
    this->ui->android_clear_btn->setEnabled(false);

    //check_adb_device_tiemr.stop();
}

void MainWindow::android_clear() {
    if (m_adb) {
        m_adb->setCmd(ANDROID_CLEAR);
    }
    if (m_tablectrl) {
        m_tablectrl->android_clear();
    }
}

void MainWindow::setLogTitle(QString path) {
    this->setWindowTitle(m_window_title+ " : " + path);
}

void MainWindow::table_view_double_clicked(QModelIndex index) {
    qDebug() << index.column();
}

void MainWindow::recieveLineNumber(int line) {
    if (m_tablectrl) {
        m_tablectrl->recieveLineNumber(line);
    }
}
