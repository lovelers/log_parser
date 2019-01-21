#include "log_filter_msg_history.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>

LogFilterMsgHistory::LogFilterMsgHistory()
{
    SetMaxCount(100);
    QFileInfo fileInfo(QString(QLatin1String(LOG_FILTER_MSG_HISTORY)));
    QFile file(QString(QLatin1String(LOG_FILTER_MSG_HISTORY)));

    if (fileInfo.exists()       &&
            fileInfo.isFile()   &&
            fileInfo.isReadable())
    {
        if (file.open(QIODevice::ReadWrite |QIODevice::Text))
        {
            mIsValidFile = true;
            file.close();
        }
        else
        {
            mIsValidFile = false;
            qDebug("file exist, but can read/write: %s", LOG_FILTER_MSG_HISTORY);
        }
    }
    else
    {
        if (!fileInfo.exists())
        {
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                mIsValidFile = true;

            }
            else
            {
                qDebug("can't create file: %s", LOG_FILTER_MSG_HISTORY);
                mIsValidFile = false;
            }
        }
        else
        {
            qDebug("file/dir exist already,but can't access: %s", LOG_FILTER_MSG_HISTORY);
            mIsValidFile = false;
        }
    }
    file.close();

    if (!IsFileAvaiable())
    {
        return;
    }
}

LogFilterMsgHistory::~LogFilterMsgHistory()
{
}

int LogFilterMsgHistory::Append(QStringList msgList)
{
    if (!IsFileAvaiable())
    {
        qDebug("file can't access");
        return -1;
    }

    {
        QFile file(QString(QLatin1String(LOG_FILTER_MSG_HISTORY)));
        if (file.open(QIODevice::ReadOnly |QIODevice::Text))
        {
            QString jval        = file.readAll();
            QJsonDocument jdoc  = QJsonDocument::fromJson(jval.toUtf8());
            QJsonObject jobj    = jdoc.object();
            QVariantList jlist;

            jlist.clear();
            foreach(const QString &msg, msgList)
            {
                if (jlist.size() < mMaxCount)
                {
                    jlist.append(QVariant(msg));
                }
            }
            jobj["msg"]         = QJsonArray::fromVariantList(jlist);
            jdoc                = QJsonDocument(jobj);
            file.close();

            if (file.open(QIODevice::ReadWrite |QIODevice::Truncate |QIODevice::Text))
            {
                file.write(jdoc.toJson());
                file.flush();
                file.close();
            }
        }
        return 0;
    }
}

QStringList LogFilterMsgHistory::GetValues()
{
    if (!IsFileAvaiable())
    {
        qDebug("file can't access");
        return QStringList(nullptr);
    }

    {
        QStringList list;
        QFile file(QString(QLatin1String(LOG_FILTER_MSG_HISTORY)));
        if (file.open(QIODevice::ReadOnly))
        {
            QString jval        = file.readAll();
            QJsonDocument jdoc  = QJsonDocument::fromJson(jval.toUtf8());
            QJsonObject jobj    = jdoc.object();
            QJsonArray jarr     = jobj["msg"].toArray();
            foreach(const QJsonValue &value, jarr)
            {
                list.append(value.toString());
            }
        }
        return list;
    }
}

int LogFilterMsgHistory::SetMaxCount(int count)
{
    mMaxCount = count;
}
