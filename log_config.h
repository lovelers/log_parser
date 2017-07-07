#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H
#include <QVector>
#include <QString>

class log_config;

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
