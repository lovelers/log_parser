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

    QObject::connect(&m_file_timer, SIGNAL(timeout()),
                     this, SLOT(checkFileSize()));

    qDebug() << "get read channel" << m_process.readChannel();
    m_process.setReadChannel(QProcess::StandardOutput);
    m_curType = ANDROID_UNKNOWN;
    //m_logcat_thread = new log_load_thread(this);
    //m_logcat_thread->setQProcess(&m_process);
}

void adb_online::setCmd(ANDROID_ONLINE_CMD type) {
    qDebug() << __func__ << "type: " << type;
    if (ANDROID_CLEAR == type)
    {
        android_clear();
        m_curType = type;
    }
    if (m_curType == type) return;
    m_curType = type;
    switch (type) {
    case ANDROID_RUN:
        android_run();
        m_file_timer.start(1000);
        break;
    case ANDROID_RESUME:
        android_resume();
        break;
    case ANDROID_STOP:
        android_stop();
        m_file_timer.stop();
        break;
    case ANDROID_PAUSE:
        android_pause();
        break;
    default:
        break;
    }

}

void adb_online::android_run() {
    // when start run, clear the cache logs first.
    // android_clear();

    QString shell;
    QStringList cmd;
    if (m_process.state() == QProcess::Running) {
        m_process.close();
    }

    m_file_path = QDir::currentPath() + "/"
            + LOG_OUTPUT_DIR + "/"
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
    m_process.waitForFinished(1000);
    outputDirManagement();
}

void adb_online::android_clear() {
    android_stop();
    QProcess process;
    process.start("adb", QStringList() << "logcat" << "-c");
    bool finished = process.waitForFinished(3000);
    qDebug() << "wiat for finished:" << finished;
    process.kill();
    process.destroyed();
    android_run();
}

void adb_online::processFinished(int exitCode , QProcess::ExitStatus exitStatus) {
    qDebug() << "processFinished, exitCode = " << exitCode
                <<", exitStatus = " << exitStatus;
    if (m_curType != ANDROID_STOP) {
        qDebug() << "finished unexpected! run process again";
        emit stopUnexpected();
    }
}

void adb_online::started() {
    qDebug() << "started";
    emit logOnlinePath(m_file_path);
}

void adb_online::processError(QProcess::ProcessError processError){
    qDebug() << "processError:" << processError;

}

void adb_online::checkFileSize()
{
    if (m_process.isOpen())
    {
        QFileInfo info(m_file_path);
        if (info.size() > 50000000) // 50Mb
        {
            m_process.kill();
        }
    }
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
        if (!process.canReadLine())
        {
            break;
        }
        QString str = process.readLine();

        if (str.contains("List of devices"))
        {
            continue;
        }
        int idx = str.indexOf("device");
        if (idx != -1) {
            list << str.mid(0, idx).trimmed();
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
    process.start("adb", QStringList() << "wait-for-device" << "shell" << "pkill" << "-l" << "9" << "camera");
    process.waitForFinished(10000);
    process.kill();
    process.destroyed();
}

void adb_online::adbKillServer() {
    QProcess process;
    qDebug() << "restart server";
    process.start("adb", QStringList() << "kill-server");
    process.waitForFinished(1000);
    process.kill();
    process.destroyed();
}

void adb_online::adbStartServer() {
    QProcess process;
    qDebug() << "start server";
    process.start("adb", QStringList() << "start-server");
    process.waitForFinished(2000);
    process.kill();
    process.destroyed();
}

void adb_online::outputDirManagement() {
    QDir logDir(LOG_OUTPUT_DIR);
    QFileInfoList fileInfoList = logDir.entryInfoList(QDir::Files, QDir::Time);
    const int maxBytes = 1000 * 1000 * 1000; // 1G
    int bytes = 0;
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        bytes += fileInfo.size();
        if (0 == fileInfo.size() || bytes > maxBytes) {
            QFile::setPermissions(fileInfo.absoluteFilePath(), QFileDevice::WriteOwner);
            if ( true == logDir.remove(fileInfo.absoluteFilePath())) {
                qDebug() << "remove empty log file: " << fileInfo.fileName();
            } else {
                qDebug() << "faild to remove empty log file: " << fileInfo.fileName();
            }
        }
    }
}
