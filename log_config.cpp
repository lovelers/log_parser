#include "log_config.h"
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

    //QString log_files = QDir::currentPath() + LOG_OUTPUT_DIR;
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

log_info_per_line_t log_config::processPerLine(const QString &str, log_type type){
    log_info_per_line_t line;
    QRegularExpression re;
    QRegularExpressionMatch match;
    int delimiter = -1;
    int delimiter1 = -1;
    if (type != checkLogType(str)) {
        line.msg = str;
        return line;
    }
    switch (type) {
    case LOGCAT_THREADTIME:
        {
            line.date = str.mid(0,5);
            line.time = str.mid(6,12);
            line.pid = str.mid(19,5);
            line.tid = str.mid(25,5);
            line.level = str.mid(31,1);
            delimiter = str.indexOf(QChar(':'), 33);
            if (delimiter != -1) {
                line.tag = str.mid(33, delimiter - 33);
                line.msg = str.mid(delimiter+1);
            }
        }
        break;
    case LOGCAT_TIME:
        line.date = str.mid(0, 5);
        line.time = str.mid(6, 12);
        line.level = str.mid(19, 1);
        delimiter = str.indexOf(QChar('('), 21);
        delimiter1 = str.indexOf(QString("):"), 21);
        if (delimiter != -1 && delimiter1 != -1 && delimiter < delimiter1) {
            line.msg = str.mid(delimiter1+1);
            line.tag = str.mid(21, delimiter-21);
            line.pid = str.mid(delimiter+1, delimiter1 - delimiter-1);
        } else {
            line.msg = str.mid(21);
        }
        break;

    case LOGCAT:
        line.level = str.mid(0, 1);
        delimiter = str.indexOf(QChar('('), 2);
        delimiter1 = str.indexOf(QString("):"), 2);
        if (delimiter != -1 && delimiter1 != -1 && delimiter < delimiter1) {
            line.tag = str.mid(2, delimiter -2);
            line.pid = str.mid(delimiter+1, delimiter1 - delimiter -1);
            line.msg = str.mid(delimiter1+1);
        } else {
            line.msg = str;
        }
        break;
    default:
        line.msg = str;
        break;
    }

    return line;
}

log_type log_config::checkLogType(const QString &str) {
    if (str.isEmpty()) {
        return UNKNOWN;
    }

    // LOGCAT_THREADTIME checking
    QString level, pid, tid;
    bool isValidLevel, isValidPid, isValidTid;
    level = str.mid(31, 1);
    pid = str.mid(25,5);
    tid = str.mid(19, 5);
    if (!level.isEmpty() && !pid.isEmpty() && !tid.isEmpty()) {
        isValidLevel = (level.compare("D") == 0 ||
                level.compare("E") == 0 ||
                level.compare("I") == 0 ||
                level.compare("W") == 0 ||
                level.compare("V") == 0);
        if (isValidLevel) {
            tid.toInt(&isValidTid, 10);
            if (isValidTid) {
                pid.toInt(&isValidPid, 10);
                if (isValidPid) {
                    return LOGCAT_THREADTIME;
                }
            }
        }
    }
    // LOGCAT_TIME checking.
    level = str.mid(19, 1);
    isValidLevel = (level.compare("D") == 0 ||
            level.compare("E") == 0 ||
            level.compare("I") == 0 ||
            level.compare("W") == 0 ||
            level.compare("V") == 0);

    if (isValidLevel) {
        int s = str.indexOf(QChar(':'), 20);
        if (s != -1) {
            QRegularExpression re;
            re.setPattern("\([\\s]*\\d+\)");
            QRegularExpressionMatch match = re.match(str.mid(20, s-20));
            if (match.hasMatch()) {
                return LOGCAT_TIME;
            }
        }
    }

    // LOGCAT checking.
    level = str.mid(0, 1);
    isValidLevel = (level.compare("D") == 0 ||
            level.compare("E") == 0 ||
            level.compare("I") == 0 ||
            level.compare("W") == 0 ||
            level.compare("V") == 0);
    if(isValidLevel) {
        int s = str.indexOf(QChar(':'),2);
        if (s != -1) {
            QRegularExpression re;
            re.setPattern("\([\\s]*\\d+\)");
            QRegularExpressionMatch match = re.match(str.mid(2, s-2));
            if (match.hasMatch()) return LOGCAT;
        }
    }

    //
    return UNKNOWN;
}

