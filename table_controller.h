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
#include <QTimer>
#include <QFont>
#include <QMenu>
#include <QMutex>
#include "show_more_log.h"
#include "config.h"

typedef struct {
    QStringList msg;
    QStringList tag;
    QStringList pid;
    QStringList tid;
    qint32 line;
} log_filter_t;

class online_log_process : public QThread {

    Q_OBJECT

public:
    online_log_process();
    void setLogPath(QString path);
    void updateLogOnlineCmd(ANDROID_ONLINE_CMD cmd);
signals:
    void processLogOnline(const QString str, int line_count);
private:
    void run();
    QString m_path;
    ANDROID_ONLINE_CMD m_cmd;
};

class table_controller : public QObject {

    Q_OBJECT

private:
    QTableView *m_view;
    table_model *m_model;
    log_config *m_logConfig;
    table_item_delegate *m_delegate;

    QMenu *m_menu;
    QAction *m_show_more;
    QAction *m_show_more_same_tag;
    QAction *m_show_more_same_pid;
    QAction *m_show_more_same_tid;
    show_more_log *m_show_more_log;

    qint32 m_column_visible;
    qint32 m_level_visible;

    log_filter_t m_log_filter;
    QMutex m_log_filter_lock;
    online_log_process *m_log_online;

    ANDROID_ONLINE_CMD m_android_online_cmd;
    bool m_android_online_filter_updated;
    bool m_android_online_level_updated;
    void processFilterPrivate();
    void updateLogLevelVisible();
    void updateColumnVisible(TABLE_COL_TYPE type, bool visible);
    inline bool isLevelVisible(const QString & str);
    bool isFilterMatched(const log_info_per_line_t &str);

    QTimer *m_scroll_timer;

    void android_run();
    void android_stop();
    void android_clear();
    void android_resume();
    void android_pause();

public:
    table_controller(QTableView *table);
    ~table_controller();
    bool checkConfigValid();
    bool processLogFromFile(QString &filename);

    void setColumnVisible(TABLE_COL_TYPE type, bool visiable);
    void setLogLevelVisible(LOG_LEVEL type, bool visiable);

    void setFilter(const log_filter_t& filter);

    void showAllLogs();

    void setAdbCmd(ANDROID_ONLINE_CMD cmd);



    void setFont(const QFont &font);

    void recieveLineNumber(int line);
public slots:
    void processLogOnline(QString str, int line_count);
    void scrollToBottom();
    void tableCustomMenuRequest(QPoint point);
    void showMore();
    void showMoreSameTag();
    void showMoreSamePid();
    void showMoreSameTid();

    void setOnLineLogFile(QString path);
};

#endif // table_controller_H
