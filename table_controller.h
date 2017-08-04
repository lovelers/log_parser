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
#include "show_more_log.h"

typedef struct {
    QStringList msg;
    QStringList tag;
    QStringList pid;
    QStringList tid;
    qint32 line;
} log_filter_t;

class table_controller : public QObject {

    Q_OBJECT

private:
    QTableView *m_view;
    table_model *m_model;
    log_config *m_logConfig;
    table_item_delegate *m_delegate;

    QMenu *m_menu;
    QAction *m_show_more;
    show_more_log *m_show_more_log;

    qint32 m_column_visible;
    qint32 m_level_visible;

    log_filter_t m_log_filter;
    void updateLogLevelVisible();
    void updateColumnVisible(TABLE_COL_TYPE type, bool visible);
    inline bool isLevelVisible(const QString & str);
    bool isFilterMatched(const log_info_per_line_t &str);

    QTimer *m_scroll_timer;

public:
    table_controller(QTableView *table);
    bool checkConfigValid();
    bool processLog(QString &filename);

    void setColumnVisible(TABLE_COL_TYPE type, bool visiable);
    void setLogLevelVisible(LOG_LEVEL type, bool visiable);

    void processFilter(const log_filter_t& filter);

    void showAllLogs();

    void android_run();
    void android_stop();
    void android_clear();
    void android_resume();
    void android_pause();

    void setFont(const QFont &font);

    void recieveLineNumber(int line);
public slots:
    void processLogOnline(const QByteArray &bArray, int line_count);
    void scrollToBottom();
    void tableCustomMenuRequest(QPoint point);
    void showMore();
};

#endif // table_controller_H
