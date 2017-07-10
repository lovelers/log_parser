#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <QWidget>
#include "log_config.h"
#include "ui_mainwindow.h"

#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QThread>
#include <QObject>
class log_parser {
private:
    Ui::MainWindow *ui;
    log_config *m_logConfig;
    QVector<QVector<QString>> m_logdata;

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

public:

    explicit log_parser(Ui::MainWindow *ui);
    void setVisible(bool visible = false);
    bool checkConfigValid();
    bool processLog(QString &filename);
    void logFilterOnClick();

};

#endif // LOG_PARSER_H
