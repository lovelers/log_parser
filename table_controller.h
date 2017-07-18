#ifndef table_controller_H
#define table_controller_H


#include "log_config.h"
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QThread>
#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include "table_model.h"
#include "table_item_delegate.h"


typedef struct {
    QString msg;
    QString tag;
    QString pid;
    QString tid;
    qint32 line;
} log_filter_t;

class table_controller {
private:
    QTableView *m_view;
    table_model *m_model;
    log_config *m_logConfig;
    table_item_delegate *m_delegate;
    QVector<QVector<QString>> m_logdata;

    qint32 m_column_visible;
    qint32 m_level_visible;

    log_filter_t m_log_filter;
    class log_load_thread : public QThread {
    public:
        log_load_thread(QString &filename, log_config* logConfig, QVector<QVector<QString>> *logData)
        //    :QThread(this)
        {
            this->filename = filename;
            this->logConfig = logConfig;
            this->logData = logData;
            qDebug() << "init" << endl;
        }
        ~log_load_thread() {}

    private:
        QString filename;
        log_config *logConfig;
        QVector<QVector<QString>> *logData;
        void run() {
            QTime stime;
            qDebug()<< "filename = " << filename << endl;
            stime.start();

            QFile *file = new QFile(filename);
            if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(file);
                while (!in.atEnd()) {
                    logData->append((logConfig->processPerLine(in.readLine())));
                }
                file->close();
            } else {
                qDebug() << "open file failed" << endl;
            }
            qDebug() << "process log diff time" << stime.elapsed() << "ms" << endl;
        }
    };
    void updateLogLevelVisible();
    void updateColumnVisible(TABLE_COL_TYPE type, bool visible);
    inline bool isLevelVisible(const QString & str);
    inline bool isPidMatched(const QString &str, const QString &pid);
    inline bool isTidMatched(const QString &str, const QString &tid);
    inline bool isTagMatched(const QString &str, const QString &tag);
    inline bool isMsgMatched(const QString &str, const QStringList &msgList);
    inline bool isFilterMatched(const QVector<QString> &vec, const QStringList & list);
public:
    explicit table_controller(QTableView *table);
    bool checkConfigValid();
    bool processLog(QString &filename);

    void setColumnVisible(TABLE_COL_TYPE type, bool visiable);
    void setLogLevelVisible(LOG_LEVEL type, bool visiable);

    void processFilter(const log_filter_t& filter);

    void showAllLogs();
};

#endif // table_controller_H
