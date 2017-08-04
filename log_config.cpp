#include "log_config.h"
#include "config.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

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

        jarr = jobj["Persist"].toArray();
        foreach(const QJsonValue &value, jarr) {
            QJsonObject obj = value.toObject();
            Persist persist;
            persist.name.append(obj["key"].toString());
            persist.value.append(obj["value"].toString());
            persist.value_range.append(obj["range"].toString());
            m_persist.append(persist);
            qDebug() << "name:" << persist.name
                     << "value:" << persist.value
                     << "range:" << persist.value_range;
        }

        if (m_keys.size() > 0 && m_persist.size() >0) {
            m_isValid = true;
        } else {
            m_isValid = false;
        }
    }
}

log_info_per_line_t log_config::processPerLine(const QString &&str, cmd_type type){
    log_info_per_line_t line;
    int delimiter = -1;
    switch (type) {
    case LOGCAT_THREADTIME:
#if 0
        logItem.append(str.mid(0, 5)); //Date
        logItem.append(str.mid(6,12)); //Time
        logItem.append(str.mid(31,1)); //Level
        logItem.append(str.mid(19,5)); //Pid
        logItem.append(str.mid(25,5)); //Tid

        delimiter = str.indexOf(QChar(':'), 33);
        logItem.append(str.mid(33, delimiter-33)); // Tag
        logItem.append(str.mid(delimiter+1).simplified()); //msg
#else
        line.date = str.mid(0,5);
        line.time = str.mid(6,12);
        line.pid = str.mid(19,5);
        line.tid = str.mid(25,5);
        line.level = str.mid(31,1);
        delimiter = str.indexOf(QChar(':'), 33);
        line.tag = str.mid(33, delimiter - 33);
        line.msg = str.mid(delimiter+1);
#endif
        break;
    default:
        break;
    }
    return line;
}

log_config::cmd_type log_config::checkCmdType(QString &ss) {
    return LOGCAT_THREADTIME;
}

