#include "log_config.h"
#include "config.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

log_config::log_config()
{
    QFile file;
    file.setFileName(LOG_PARSER_JSON);
    m_isValid = false;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString jval = file.readAll();
        qDebug() << "file data:" << jval << endl;
        file.close();
        QJsonDocument jdoc= QJsonDocument::fromJson(jval.toUtf8());
        qDebug() << "print empty: " << jdoc.isNull() << endl;
        QJsonObject jobj = jdoc.object();
        QJsonArray jarr = jobj["LogItem"].toArray();
        foreach(const QJsonValue &value, jarr) {
            QJsonObject obj = value.toObject();
            m_keys.append(obj["key"].toString());
            m_widths.append(obj["width"].toInt());
            qDebug() <<"print " << obj["key"].toString() << endl;
            qDebug() << "print" << obj["width"].toInt() << endl;
        }

        if (m_keys.size() > 0) {
            m_isValid = true;
        } else {
            m_isValid = false;
        }
    }
}

QVector<QString> log_config::processPerLine(const QString &&str, cmd_type type){
    QVector<QString> logItem;
    int delimiter = -1;
    switch (type) {
    case LOGCAT_THREADTIME:
        logItem.append(str.mid(0, 5)); //Date
        logItem.append(str.mid(6,12)); //Time
        logItem.append(str.mid(19,1)); //Level
        logItem.append(str.mid(21,8)); //Pid
        logItem.append(str.mid(30,8)); //Tid
        delimiter = str.indexOf(QChar(':'), 39);
        logItem.append(str.mid(39, delimiter-39)); // Tag
        logItem.append(str.mid(delimiter+1).simplified()); //msg
        break;
    default:
        break;
    }
    return logItem;
}

log_config::cmd_type log_config::checkCmdType(QString &ss) {
    return LOGCAT_THREADTIME;
}

