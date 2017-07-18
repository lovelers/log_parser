#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H
#include <QVector>
#include <QString>

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

static log_config *g_logConfig = NULL;
class log_config
{

public:
    static log_config* getInstance() {
        if (g_logConfig == NULL) {
            g_logConfig = new log_config();
        }
        return g_logConfig;
    }
    typedef enum {
        LOGCAT_THREADTIME =0, //logcat -v threadtime
        LOGCAT_TIME, // logcat -v time
        LOGCAT_RATIO_TIME, //logcat -b radio -v time
        LOGCAT_EVENTS_TIME, //logcat -b events -v time
        CAT_PROC_KMSG //cat /proc/kmsg
    } cmd_type;
    bool isConfigValid() const { return m_isValid; }
    const QVector<QString> &getKeys() { return m_keys;}
    const QVector<qint16> &getWidths() { return m_widths;}
    static QVector<QString> processPerLine(const QString &&str, cmd_type type = LOGCAT_THREADTIME);
    static cmd_type checkCmdType(QString &ss);

private:
    QVector<QString> m_keys;
    QVector<qint16> m_widths;
    bool m_isValid;
    log_config();
};

#endif // LOG_CONFIG_H
