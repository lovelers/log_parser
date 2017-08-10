#include "adb_online.h"
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QTime>
#include <QDate>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
adb_online::adb_online()
{
    m_process.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    QObject::connect(&m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
                     this, SLOT(processFinished(int,QProcess::ExitStatus)));
    QObject::connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
                     this, SLOT(processError(QProcess::ProcessError)));
    QObject::connect(&m_process, SIGNAL(started()),
                     this, SLOT(started()));
    qDebug() << "get read channel" << m_process.readChannel();
    m_process.setReadChannel(QProcess::StandardOutput);
    m_curType = ANDROID_UNKNOWN;
    //m_logcat_thread = new log_load_thread(this);
    //m_logcat_thread->setQProcess(&m_process);
}

void adb_online::setCmd(ANDROID_ONLINE_CMD type) {
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
    if (m_process.state() == QProcess::Running) {
        m_process.close();
    }

    m_file_path = QDir::currentPath() + "/"
            + QDate::currentDate().toString("yyyy_MM_dd_")
            + QTime::currentTime().toString("hh_mm_ss_")
            + "logcat.txt";
    shell = "adb";
    cmd << "logcat" << "-v" << "threadtime";
    qDebug() << cmd;
    m_process.setStandardOutputFile(m_file_path);
    m_process.start(shell, cmd);
}

void adb_online::android_pause() {
}

void adb_online::android_resume() {
}

void adb_online::android_stop() {
    m_process.kill();
}

void adb_online::android_clear() {
    //android_stop();
    QProcess process;
    process.start("adb", QStringList() << "logcat -c");
    bool finished = process.waitForFinished(1000);
    qDebug() << "wiat for finished:" << finished;

    process.close();
    process.destroyed();
}

void adb_online::processFinished(int exitCode , QProcess::ExitStatus exitStatus) {
    qDebug() << "processFinished, exitCode = " << exitCode
                <<", exitStatus = " << exitStatus;
    if (m_curType != ANDROID_STOP) {
        qDebug() << "finished unexpected! run process again";
        android_run();
    }
}

void adb_online::started() {
    qDebug() << "started";
    emit logOnlinePath(m_file_path);
}

void adb_online::processError(QProcess::ProcessError processError){
    qDebug() << "processError:" << processError;

}

adb_online::~adb_online() {
    m_process.kill();
    m_process.waitForFinished();
    m_process.destroyed();
}

// log_load_thread

QStringList adb_online::checkDevices() {
    //QProcess m_device_check;
    QProcess process;
    process.start("adb", QStringList() << "devices");
    process.waitForFinished(100);//200ms;
    QStringList list;
    do {
        QString str = process.readLine();
        if (str.isEmpty()) {
            break;
        }
        int idx = str.indexOf("\tdevice");
        if (idx != -1) {
            list << str.mid(0, idx);
        }
        //qDebug() << str;
    } while (true);
    //qDebug() << list;
    process.kill();
    process.waitForFinished();
    process.destroyed();
    return list;
}

bool adb_online::adbRootRemount() {
    QProcess process;
    process.start("adb", QStringList() << "root");
    process.waitForFinished(2000);
    QString adbroot = process.readAll();
    process.destroyed();
#if 0
    process.start("adb", QStringList() << "remount");
    process.waitForFinished(2000);
    QString adbremount = process.readAll();

    qDebug() << adbroot << adbremount << endl;
    process.close();
#endif
    if (adbroot.indexOf("failed") == -1) {
        return true;
    }
    return false;
}

QString adb_online::adbProperity(QString key, QString value) {
    QProcess process;
    qDebug() << key << value;
    process.start("adb" , QStringList() << "shell" << "setprop" << key << value);
    process.waitForFinished(100);
    QString setprop = process.readAll().simplified();
    process.close();
#if 0
    process.start("adb", QStringList() << "shell" << "getprop " << key);
    process.waitForFinished(100);
    QString getprop = process.readAll().simplified();
    process.close();

    qDebug() << setprop << getprop << endl;
#endif
    process.destroyed();
    return setprop;
}

void adb_online::adbRestartCamera() {
    QProcess process;
    qDebug() << "restart cameraservice";
    process.start("adb", QStringList() << "shell" << "pkill" << "-l" << "9" << "camera");
    process.waitForFinished(10000);
    process.close();
    process.destroyed();
}
