#ifndef ADB_ONLINE_H
#define ADB_ONLINE_H
#include <QProcess>
#include <QObject>
#include <QThread>
#include <QTextEdit>
#include <QDebug>
#include <QFile>
typedef enum {
    ANDROID_UNKNOWN,
    ANDROID_RUN,
    ANDROID_CLEAR,
    ANDROID_STOP,
    ANDROID_RESUME,
    ANDROID_PAUSE,
} UI_CMD_TYPE;


class adb_online : public QObject
{
    Q_OBJECT

public:
    adb_online();
    ~adb_online();
    void setOutput(QTextEdit *textEdit);
    void setCmd(UI_CMD_TYPE type);
    QStringList checkDevices();
    class log_load_thread : public QThread {
    public:
        log_load_thread(adb_online *adb);
        ~log_load_thread();
        void setFilePath(QString path);
        void setQProcess(QProcess *process);
        void saveToFile();
    private:
        QFile log_file;
        adb_online *adb;
        QProcess *process;
        QTextEdit *text_edit;
        void run();
    };
private:
    QProcess m_process;
    QTextEdit *m_text_edit;
    UI_CMD_TYPE m_curType;
    log_load_thread *m_logcat_thread;
    void android_run();
    void android_stop();
    void android_clear();
    void android_resume();

public slots:
    void processFinished(int, QProcess::ExitStatus);
    void processError(QProcess::ProcessError);
    void started();
    void readReady();

signals:
    void processLogOnline(const QByteArray &bArray);
};

#endif // ADB_ONLINE_H
