#ifndef LOG_FILTER_MSG_HISTORY_H
#define LOG_FILTER_MSG_HISTORY_H
#include "config.h"
#include <QString>
#include <QList>
#include <QFile>
#include <QLockFile>
class LogFilterMsgHistory;

static LogFilterMsgHistory *gLogFilterMsgHistory = nullptr;


class LogFilterMsgHistory
{
private:
    LogFilterMsgHistory();
    int         mMaxCount;
    bool        mIsValidFile;
public:

    ~LogFilterMsgHistory();

    static LogFilterMsgHistory* getInstance()
    {
        if (nullptr == gLogFilterMsgHistory)
        {
            gLogFilterMsgHistory = new LogFilterMsgHistory();
        }
        return gLogFilterMsgHistory;
    }

    int         Append(QStringList msgList);
    QStringList GetValues();
    int         SetMaxCount(int count);

    bool inline IsFileAvaiable()
    {
        return mIsValidFile;
    }
};

#endif // LOG_FILTER_MSG_HISTORY_H
