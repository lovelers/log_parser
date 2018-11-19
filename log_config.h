#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H
#include <QVector>
#include <QString>
#include "config.h"
class log_config;


typedef enum {
    TABLE_COL_TYPE_DATE=0,
    TABLE_COL_TYPE_TIME,
    TABLE_COL_TYPE_LEVEL,
    TABLE_COL_TYPE_PID,
    TABLE_COL_TYPE_TID,
    TABLE_COL_TYPE_TAG,
    TABLE_COL_TYPE_MSG
} TABLE_COL_TYPE;

typedef enum {
    LOG_LEVEL_INFO = 0,
    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_WARN,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_MAX,
} LOG_LEVEL;

typedef struct {
    QString name;
    QString value;
    QString value_range;
} persist_t;

static log_config *g_logConfig = NULL;

typedef struct {
    int line;
    QString date;
    QString time;
    QString level;
    QString pid;
    QString tid;
    QString tag;
    QString msg;
} log_info_per_line_t;

typedef enum {
    UNKNOWN = -1,
    LOGCAT = 0,//logcat
    LOGCAT_THREADTIME, //logcat -v threadtime
    LOGCAT_TIME, // logcat -v time
    //LOGCAT_RATIO_TIME, //logcat -b radio -v time
    //LOGCAT_EVENTS_TIME, //logcat -b events -v time
    LOGCAT_THREADTIME1, // new version.
    CAT_PROC_KMSG, //cat /proc/kmsg
    LOGCAT_DUMP_TO_FILE
} log_type;

typedef struct {
    QString key;
    int     width;
} log_item_t;

struct config_t {
    bool                    isValid;
    QVector<QString>        logCmds;
    QVector<log_item_t>     logItems;
    QVector<persist_t>      persists;
    int                     logExpandNum;
};

typedef QVector<log_info_per_line_t> log_info_t;
class log_config
{

public:
    static log_config* getInstance() {
        if (g_logConfig == NULL) {
            g_logConfig = new log_config();
        }
        return g_logConfig;
    }

    bool isConfigValid() const
    {
        return m_config.isValid;
    }

    const QVector<log_item_t> &GetLogItems()
    {
        return m_config.logItems;
    }

    const QVector<persist_t>& GetPersist()
    {
        return m_config.persists;
    }

    int GetLogExpandNum()
    {
        return m_config.logExpandNum;
    }

    static log_info_per_line_t processPerLine(const QString &str, log_type type = LOGCAT_THREADTIME);
    static log_type checkLogType(const QString &str);
private:

    log_config();
    config_t m_config;
};

#endif // LOG_CONFIG_H
