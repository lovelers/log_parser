#ifndef ADB_ONLINE_H
#define ADB_ONLINE_H
#include <QProcess>
#include <QObject>
#include <QThread>
#include <QTextEdit>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include "config.h"

class adb_online : public QObject
{
    Q_OBJECT

public:
    adb_online();
    ~adb_online();
    void setCmd(ANDROID_ONLINE_CMD type);
    ANDROID_ONLINE_CMD getCurType() { return m_curType;}
private:
    QProcess m_process;
    ANDROID_ONLINE_CMD m_curType;
    QString m_file_path;
    QTimer m_file_timer;
    void android_run();
    void android_stop();
    void android_clear();
    void android_pause();
    void android_resume();

    void outputDirManagement();

public slots:
    void processFinished(int, QProcess::ExitStatus);
    void processError(QProcess::ProcessError);
    void started();
    void checkFileSize();

signals:
    void logOnlinePath(QString path);
public:
    static QStringList checkDevices();
    static bool adbRootRemount();
    static QString adbProperity(QString key, QString value);
    static void adbRestartCamera();
    void adbKillServer();
    void adbStartServer();
};

#endif // ADB_ONLINE_H
