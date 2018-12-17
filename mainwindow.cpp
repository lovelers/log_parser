#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "table_controller.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTime>
#include <QStringList>
#include <QString>
#include <QDragEnterEvent>
#include <QMimeData>
#include "goto_line_dialog.h"
#include "config.h"
#include "log_filter_msg_history.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mLogOpenPath(QDir::currentPath() + LOG_OUTPUT_DIR),
    mWindowTitle(tr("log_parser"))
{
    ui->setupUi(this);
    mpTableCtrl             = new table_controller(ui->TableView);
    mpAdbOnline             = new adb_online();
    mpLineDialog            = new goto_line_dialog();
    mpPersistSettings       = new persist_settings();
    mpSnapshot              = new screen_snapshot(this);
    mpCheckDevices          = new CheckDeviceRealtimeThread(this);

    QObject::connect(ui->msg_combobox->lineEdit(), SIGNAL(returnPressed()),
                     this, SLOT(logFilterReturnPress()));
    QObject::connect(ui->tag_edit, SIGNAL(returnPressed()),
                     this, SLOT(logFilterReturnPress()));
    QObject::connect(ui->pid_edit, SIGNAL(returnPressed()),
                     this, SLOT(logFilterReturnPress()));
    QObject::connect(ui->tid_edit, SIGNAL(returnPressed()),
                     this, SLOT(logFilterReturnPress()));
    QObject::connect(mpAdbOnline, SIGNAL(logOnlinePath(QString)),
                     this, SLOT(logOnlinePath(QString)));
    QObject::connect(mpAdbOnline, SIGNAL(stopUnexpected()),
                     this, SLOT(logStopUnexpected()));
    mpLineDialog->setModal(true);
    mpLineDialog->setWindowFlags(Qt::FramelessWindowHint);
    QObject::connect(mpLineDialog, SIGNAL(sendLineNumber(int)),
                     this, SLOT(selectLine(int)));
    ui->android_stop_btn->setEnabled(false);
    ui->msg_combobox->setCompleter(nullptr);

    QStringList msglist = LogFilterMsgHistory::getInstance()->GetValue();

    ui->msg_combobox->insertItems(0, msglist);
    this->setWindowTitle(mWindowTitle);
    QWidget::showMaximized();

    mpCheckDevices->EventLoop();
    QDir::current().mkdir(LOG_OUTPUT_DIR);
}

MainWindow::~MainWindow()
{
    qDebug() << "main window destory";
    if (mpLineDialog) delete mpLineDialog;
    if (mpPersistSettings) delete mpPersistSettings;
    if (nullptr != mpSnapshot)
    {
        delete mpSnapshot;
        mpSnapshot = nullptr;
    }
    if (nullptr != mpTableCtrl) {
        mpTableCtrl->setAdbCmd(ANDROID_STOP);
        delete mpTableCtrl;
        mpTableCtrl = nullptr;
    }
    if (mpAdbOnline) {
        mpAdbOnline->setCmd(ANDROID_STOP);
        delete mpAdbOnline;
        mpAdbOnline = nullptr;
    }

    if (nullptr != mpCheckDevices)
    {
        mpCheckDevices->ExitEventLoop();
        delete mpCheckDevices;
        mpCheckDevices = nullptr;
    }

    delete ui;
}

void MainWindow::openLog(){
    qDebug() << "openLog" << endl;
    this->android_stop();
    QString filename = QFileDialog::getOpenFileName(this, tr("open log"), mLogOpenPath, tr("log file (*.txt *.log *log*)"));
    if (filename.isEmpty()) {
        qDebug() << "file is empty()" << endl;
        return;
    }
    QFileInfo info(filename);
    mLogOpenPath = info.absolutePath();
    this->setWindowTitle(mWindowTitle + ":" + filename);
    mpTableCtrl->processLogFromFile(filename);
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
        mpTableCtrl->setFont(font);
    }
}

void MainWindow::goto_line() {
    qDebug() << "goto here";
    //mpLineDialog->setFocus();
    mpLineDialog->show();
    mpLineDialog->activateWindow();
}

void MainWindow::do_screen_shot() {
    qDebug() << "do screen shot";
    //screen_snapshot *snapshot = new screen_snapshot(this);
    mpSnapshot->take_shot(this->geometry());
}


void MainWindow::persistSettings() {
    qDebug() << "persistSettings" << endl;
    if (mpPersistSettings) {
        mpPersistSettings->activateWindow();
        mpPersistSettings->show();
    }
}

void MainWindow::version() {
    qDebug() << "version 1.0" << endl;
}

void MainWindow::myShow() {
    this->show();
    if (mpTableCtrl == nullptr ||
            mpTableCtrl->checkConfigValid() == false) {
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
    mpTableCtrl->setFilter(filter);

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
    mpTableCtrl->showAllLogs();
    //ui->line_edit->clear();
}

// log columns
void MainWindow::date_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setColumnVisible(TABLE_COL_TYPE_DATE, clicked);
}

void MainWindow::time_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setColumnVisible(TABLE_COL_TYPE_TIME, clicked);
}

void MainWindow::pid_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setColumnVisible(TABLE_COL_TYPE_PID, clicked);
}

void MainWindow::tid_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setColumnVisible(TABLE_COL_TYPE_TID, clicked);
}

void MainWindow::level_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setColumnVisible(TABLE_COL_TYPE_LEVEL, clicked);
}

void MainWindow::tag_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setColumnVisible(TABLE_COL_TYPE_TAG, clicked);
}

void MainWindow::msg_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setColumnVisible(TABLE_COL_TYPE_MSG, clicked);
}


// log levels
void MainWindow::info_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setLogLevelVisible(LOG_LEVEL_INFO, clicked);
}

void MainWindow::debug_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setLogLevelVisible(LOG_LEVEL_DEBUG, clicked);
}

void MainWindow::verbose_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setLogLevelVisible(LOG_LEVEL_VERBOSE, clicked);
}

void MainWindow::warn_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setLogLevelVisible(LOG_LEVEL_WARN, clicked);
}

void MainWindow::error_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setLogLevelVisible(LOG_LEVEL_ERROR, clicked);
}

void MainWindow::fatal_cb_checked(bool clicked) {
    qDebug("%s, clicked : %d", __func__,clicked);
    mpTableCtrl->setLogLevelVisible(LOG_LEVEL_FATAL, clicked);
}

// android devices
void MainWindow::android_devices_select() {
    static QStringList pre_list;
    if (mpAdbOnline) {
        QStringList list = mpAdbOnline->checkDevices();
        if (list.isEmpty() || list.size() == 0)
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
            qDebug() << "need update new list of devcies:";
            ui->android_devices_cb->clear();
            for (int i = 0; i < list.size(); i++) {
                qDebug() << "device" << i << ":" << list.at(i) << endl;
                ui->android_devices_cb->insertItem(i, list.at(i));
            }
        }
        pre_list = list;
    }
}

void MainWindow::android_run() {
    if (mpAdbOnline) {
        mpAdbOnline->setCmd(ANDROID_RUN);
    }
    if (mpTableCtrl) {
        mpTableCtrl->setAdbCmd(ANDROID_RUN);
    }
    this->ui->android_run_btn->setEnabled(false);
    this->ui->android_pause_resume_btn->setEnabled(true);
    this->ui->android_stop_btn->setEnabled(true);
    this->ui->android_clear_btn->setEnabled(true);
}

void MainWindow::android_pause_resume() {
    if (ui->android_pause_resume_btn->text().compare("Pause") == 0) {
        if (mpAdbOnline) {
            mpAdbOnline->setCmd(ANDROID_PAUSE);
        }
        if (mpTableCtrl) {
            mpTableCtrl->setAdbCmd(ANDROID_PAUSE);
        }
        ui->android_pause_resume_btn->setText("Resume");
    } else {
        if (mpAdbOnline) {
            mpAdbOnline->setCmd(ANDROID_RESUME);
        }
        if (mpTableCtrl) {
            mpTableCtrl->setAdbCmd(ANDROID_RESUME);
        }
        ui->android_pause_resume_btn->setText("Pause");
    }
    this->ui->android_run_btn->setEnabled(false);
    this->ui->android_pause_resume_btn->setEnabled(true);
    this->ui->android_stop_btn->setEnabled(true);
    this->ui->android_clear_btn->setEnabled(true);
}

void MainWindow::android_stop() {
    if (mpAdbOnline) {
        mpAdbOnline->setCmd(ANDROID_STOP);
    }
    if (mpTableCtrl) {
        mpTableCtrl->setAdbCmd(ANDROID_STOP);
    }
    this->ui->android_run_btn->setEnabled(true);
    ui->android_pause_resume_btn->setText("Pause");
    this->ui->android_pause_resume_btn->setEnabled(false);
    this->ui->android_stop_btn->setEnabled(false);
    this->ui->android_clear_btn->setEnabled(false);
}

void MainWindow::android_clear() {
    if (mpAdbOnline) {
        mpAdbOnline->setCmd(ANDROID_CLEAR);
    }
    if (mpTableCtrl) {
        mpTableCtrl->setAdbCmd(ANDROID_CLEAR);
    }
}

void MainWindow::logOnlinePath(QString path) {
    this->setWindowTitle(mWindowTitle + " : " + path);
    mpTableCtrl->setOnLineLogFile(path);
}

void MainWindow::logStopUnexpected()
{
    android_devices_select();
    android_run();
}
void MainWindow::table_view_double_clicked(QModelIndex index) {
    qDebug() << index.column();
}

void MainWindow::selectLine(int line) {
    if (mpTableCtrl) {
        mpTableCtrl->selectLine(line);
    }
}

void MainWindow::logCopy()
{
    if (mpTableCtrl && ui->TableView->isActiveWindow())
    {
        mpTableCtrl->logCopy();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *data = event->mimeData();
    qDebug() << "dropEvent" <<endl;
    if (data->hasUrls()) {
        if (data->urls().at(0).isLocalFile()) {
            android_stop();
            QString filename = data->urls().first().toLocalFile();
            this->setWindowTitle(mWindowTitle + ":" + filename);
            mpTableCtrl->processLogFromFile(filename);
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
        qDebug() << "dragEnterEvent" <<endl;
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
           qDebug() << "keyPressEvent" <<endl;
    QWidget::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *) {
    qDebug() << "MainWindow Close Event";
    mpTableCtrl->closeWindow();
    mpPersistSettings->close();
    mFontDialog.close();
    mpLineDialog->close();

    QStringList msgList;
    for (int i = 0; i < ui->msg_combobox->count(); ++i)
    {
        msgList.append(ui->msg_combobox->itemText(i));
    }
    LogFilterMsgHistory::getInstance()->Append(msgList);
}

void MainWindow::on_msg_combobox_editTextChanged(const QString &arg1)
{
}
