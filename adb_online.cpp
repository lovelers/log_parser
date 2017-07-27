#include "adb_online.h"
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QTime>
#include <QDate>
adb_online::adb_online()
{
    m_process.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    QObject::connect(&m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
                     this, SLOT(processFinished(int,QProcess::ExitStatus)));
    QObject::connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
                     this, SLOT(processError(QProcess::ProcessError)));
    QObject::connect(&m_process, SIGNAL(started()),
                     this, SLOT(started()));
    QObject::connect(&m_process, SIGNAL(readyRead()),
                     this, SLOT(readReady()));
    qDebug() << "get read channel" << m_process.readChannel();
    m_process.setReadChannel(QProcess::StandardOutput);
    m_curType = ANDROID_UNKNOWN;
    m_text_edit = NULL;
    m_logcat_thread = new log_load_thread(this);
    m_logcat_thread->setQProcess(&m_process);

}

QStringList adb_online::checkDevices() {
    QProcess process;
    process.start("adb", QStringList() << "devices");
    process.waitForFinished(100);//200ms;
    QStringList list;
    do {
        QString str = process.readLine();
        if (str.isEmpty()) break;
        int idx = str.indexOf("\tdevice");
        if (idx != -1) {
            list << str.mid(0, idx);
        }
        qDebug() << str;
    } while (true);
    qDebug() << list;
    process.close();
    return list;
}


void adb_online::setCmd(UI_CMD_TYPE type) {
    qDebug() << __func__ << "type: " << type;
    if (m_curType == type) return;
    switch (type) {
    case ANDROID_RUN:
        android_run();
        break;
    case ANDROID_RESUME:
        android_resume();
        break;
    case ANDROID_CLEAR:
        android_clear();
        break;
    case ANDROID_STOP:
        android_stop();
        break;
    case ANDROID_PAUSE:
        android_pause();
        break;
    default:
        break;
    }
    m_curType = type;
}

void adb_online::android_run() {
    QString shell;
    QStringList cmd;
    QString path;

    if (m_process.state() == QProcess::Running) {
        m_process.close();
    }
    if (m_logcat_thread->isRunning()) {
        m_logcat_thread->exit();
        qDebug() << "logcat_thread exit successful";
    }

    path = QDir::currentPath() + "/"
            + QDate::currentDate().toString("yyyy_MM_dd_")
            + QTime::currentTime().toString("hh_mm_ss_")
            + "logcat.txt";
    shell = "adb";
    cmd << "logcat" << "-v" << "threadtime";
    qDebug() << cmd;
    m_process.start(shell, cmd);

    m_logcat_thread->setFilePath(path);
    emit setLogTitle(path + " adb is running");
}

void adb_online::android_pause() {
}

void adb_online::android_resume() {
}

void adb_online::android_stop() {
    m_process.kill();
    m_logcat_thread->exit();
    m_logcat_thread->saveToFile();
}

void adb_online::android_clear() {
    //android_stop();
    QProcess process;
    process.start("adb", QStringList() << "clear");
    bool finished = process.waitForFinished(1000);
    qDebug() << "wiat for finished:" << finished;

    process.close();
    process.destroyed();
}

void adb_online::processFinished(int exitCode , QProcess::ExitStatus exitStatus) {
    qDebug() << "processFinished, exitCode = " << exitCode
                <<", exitStatus = " << exitStatus;
    if (m_curType == ANDROID_RUN) {
        qDebug() << "finished unexpected! run process again";
        android_run();
    }
}

void adb_online::started() {
    qDebug() << "started";
}

void adb_online::processError(QProcess::ProcessError processError){
    qDebug() << "processError:" << processError;

}

void adb_online::readReady() {
    if (m_logcat_thread) {
        //logcat_thread = new log_load_thread(&m_process, m_text_edit);}
        m_logcat_thread->start();
    }
}

adb_online::~adb_online() {
    m_process.kill();
    m_process.destroyed();
    if (m_logcat_thread) {
        m_logcat_thread->exit();
        delete m_logcat_thread;
        m_logcat_thread = NULL;
    }
}

// log_load_thread

adb_online::log_load_thread::log_load_thread(adb_online *adb) {
    this->adb = adb;
}

adb_online::log_load_thread::~log_load_thread() {
   if (log_file.isOpen()) {
       log_file.close();
   }
   log_file.destroyed();
}

void adb_online::log_load_thread::setFilePath(QString path) {

    qDebug() << "origin file:" << log_file.fileName()
             << "new file:" << path;
    if (!log_file.fileName().isEmpty()) {
        if (log_file.fileName().compare(path) == 0) {
            return;
        } else {
            if (log_file.isOpen()) {
                log_file.close();
            }
        }
    }
    log_file.setFileName(path);
    log_file.open(QIODevice::WriteOnly|QIODevice::Text);
}

void adb_online::log_load_thread::saveToFile(){
    qDebug() << "save to file:" << log_file.fileName();
    if (log_file.isOpen()) {
        log_file.close();
    }
}

void adb_online::log_load_thread::setQProcess(QProcess *process) {
    this->process = process;
}

void adb_online::log_load_thread::run() {
    if (process) {
        QByteArray str;
        while (!process->atEnd() && process->isReadable()) {
            str =  process->readLine();
            if (log_file.isWritable()) {
                log_file.write(str);
            }
            if (adb && adb->getCurType() != ANDROID_PAUSE) emit adb->processLogOnline(str);
        }
    }
}
