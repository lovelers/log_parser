#include "table_controller.h"
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QHeaderView>
#include <QMenu>

#include "show_more_log.h"
table_controller::table_controller(QTableView *view) {
    m_logConfig = log_config::getInstance();

    m_view = view;
    m_model = new table_model();
    m_delegate = new table_item_delegate();

    // here set fixed verital header width to disable the resize of it.
    // it can improve the tableview performance.
    m_view->verticalHeader()->setFixedWidth(50);
    m_view->setModel(m_model);
    m_view->setItemDelegate(m_delegate);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(m_view, SIGNAL(customContextMenuRequested(QPoint)),
                     this, SLOT(tableCustomMenuRequest(QPoint)));

    if (m_logConfig->isConfigValid()) {
        const QVector<QString> &keys = m_logConfig->getKeys();
        const QVector<qint16> &widths = m_logConfig->getWidths();

        int size = keys.size();
        for (int i = 0; i < size; ++i) {
            m_view->setColumnWidth(i, widths.at(i));
        }
    }
    m_scroll_timer = new QTimer(this);
    QObject::connect(m_scroll_timer, SIGNAL(timeout()),
                     this, SLOT(scrollToBottom()));
    m_column_visible = 0xffff; // all the column are visible default
    m_level_visible = 0xffff; // all the log level are visible default

    //show more log
    m_menu =new QMenu(m_view);
    m_show_more = new QAction("show more log");
    m_show_more_same_tag = new QAction("show more log with same tag");
    m_show_more_same_tid = new QAction("show more log with same tid");
    m_show_more_same_pid = new QAction("show more log with same pid");
    QObject::connect(m_show_more, SIGNAL(triggered()),
                     this, SLOT(showMore()));
    QObject::connect(m_show_more_same_tag, SIGNAL(triggered(bool)),
                     this, SLOT(showMoreSameTag()));
    QObject::connect(m_show_more_same_pid, SIGNAL(triggered(bool)),
                     this, SLOT(showMoreSamePid()));
    QObject::connect(m_show_more_same_tid, SIGNAL(triggered(bool)),
                     this, SLOT(showMoreSameTid()));

    m_menu->addAction(m_show_more);
    m_menu->addAction(m_show_more_same_tag);
    m_menu->addAction(m_show_more_same_pid);
    m_menu->addAction(m_show_more_same_tid);
    m_show_more_log = new show_more_log;

    m_log_online = new online_log_process();
    QObject::connect(m_log_online, SIGNAL(processLogOnline(QString,int)),
                     this, SLOT(processLogOnline(QString,int)));
    m_android_online_cmd = ANDROID_STOP;
    m_android_online_level_updated = false;
    m_android_online_filter_updated = false;
}

table_controller::~table_controller() {
    if (m_show_more) delete m_show_more;
    if (m_show_more_same_tag) delete m_show_more_same_tag;
    if (m_show_more_same_pid) delete m_show_more_same_pid;
    if (m_show_more_same_tid) delete m_show_more_same_tid;
    if (m_show_more_log) delete m_show_more_log;

    if (m_menu) delete m_menu;
    if (m_scroll_timer) {
        m_scroll_timer->stop();
        delete m_scroll_timer;
    }

    if (m_log_online) {
        m_log_online->terminate();
        m_log_online->wait();
        m_log_online->destroyed();
        delete m_log_online;
    }
    if (m_delegate) {
        delete m_delegate;
    }
    if (m_model) delete m_model;
}

bool table_controller::checkConfigValid() {
    return m_logConfig->isConfigValid();
}

bool table_controller::processLogFromFile(QString &filename) {
    qDebug()<< "filename = " << filename << endl;
    QTime stime;
    stime.start();
    QFile *file = new QFile(filename);

    QVector<log_info_per_line_t> logData;
    QVector<log_info_per_line_t> filterData;

    log_info_per_line_t log_info_per_line;
    logData.reserve(500000);

    log_type ltype = LOGCAT;
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        // here skip 20 lines, and then check log type
        int skipCount = 20;
        for (int i = 0; i< skipCount; ++i) {
            file->readLine();
        }
        do {
            log_type cur_type = log_config::checkLogType(file->readLine());
            if (ltype == cur_type) {
                break;
            }
            ltype = cur_type;
        } while (true);
        qDebug() << "check the type: " << ltype;
        //m_logConfig->analysisLogType(filename);
        file->seek(0);
        QTextStream in(file);
        int line_count = 1;
        while (!in.atEnd()) {
            log_info_per_line = log_config::processPerLine(in.readLine().trimmed(), ltype);
            log_info_per_line.line = line_count++;
            if (isFilterMatched(log_info_per_line)) {
                filterData.append(log_info_per_line);
            }
            logData.append(log_info_per_line);
        }
        file->close();
    } else {
        qDebug() << "open file failed" << endl;
    }
    m_model->setLogData(logData);
    m_model->setLogFilterData(filterData);
    qDebug() << "total log process diff time" << stime.elapsed() << "ms" << endl;
    return true;
}

void table_controller::setColumnVisible(TABLE_COL_TYPE type, bool visible){
    int isVisible = m_column_visible & (1 << type);
    qDebug("type = %d, isVisible = %d", type, isVisible);
    if (isVisible == 0 && visible == true) {
        m_column_visible |= (1 << type);
        updateColumnVisible(type, visible);
    } else if (isVisible && visible == false) {
        m_column_visible &= ~(1 << type);
        updateColumnVisible(type, visible);
    }
}

void table_controller::updateColumnVisible(TABLE_COL_TYPE type, bool visible) {
    m_view->setColumnHidden(type, !visible);
}

void table_controller::setLogLevelVisible(LOG_LEVEL level, bool visible) {
    int cur_level = m_level_visible & (1 << level);
    qDebug("cur_level = %d, isVisible = %d", cur_level, visible);
    if (cur_level == 0 && visible == true) {
        m_level_visible |= (1 << level);
        updateLogLevelVisible();
    } else if (cur_level && visible == false) {
        m_level_visible &= ~(1 << level);
        updateLogLevelVisible();
    }
}

void table_controller::updateLogLevelVisible() {
    QTime stime;
    stime.start();
    //processFilter(m_log_filter);
    if (m_log_online->isRunning()) {

    }
    qDebug() << "row hidden time diff: " << stime.elapsed() << endl;
}

const QString g_log_level[LOG_LEVEL_MAX] = {
    "I", "V", "W", "D", "E", "F"
};

bool table_controller::isLevelVisible(const QString &leve) {
    if (leve.isEmpty()) {
        //qDebug() << "isEmpty" << endl;
        return true;
    }
    for (int i = 0; i < LOG_LEVEL_MAX; i++ ){
        if (leve.compare(g_log_level[i]) == 0
                && (m_level_visible &(1 << i))) {
             return true;
        }
    }
    return false;
}

void table_controller::setFilter(const log_filter_t& filter) {
    if (filter.line != m_log_filter.line ||
            filter.pid != m_log_filter.pid ||
            filter.tid != m_log_filter.tid ||
            filter.tag != m_log_filter.tag ||
            filter.msg != m_log_filter.msg) {
        m_log_filter = filter;
        qDebug() << "start update nwe filter" << endl;
        if (m_android_online_cmd == ANDROID_STOP ||
                m_android_online_cmd == ANDROID_PAUSE) {
            processFilterPrivate();
        } else {
            m_android_online_filter_updated = true;
        }
        qDebug() << "end update nwe filter" << endl;
    }
}
void table_controller::processFilterPrivate() {
    qDebug() << "start process Filter Private" << endl;
    log_info_t *logData = m_model->getLogDataPtr();
    if (logData->isEmpty()) return;

    log_info_t filterData;
    int row = logData->size();
    qDebug() << "row = " << row << endl;
    QTime stime;
    stime.start();
    for (int i = 0; i < row; ++i) {
        const log_info_per_line_t &vec = logData->at(i);
        //qDebug() << "row_index" << row_index << endl;
        if (isLevelVisible(vec.level)
                && isFilterMatched(vec)) {
            filterData.append(vec);
        } else {
            //m_view->setRowHidden(i, true);
        }
    }
    m_model->setLogFilterData(filterData);
    m_delegate->updateMsgFilter(m_log_filter.msg);
    qDebug() << "filter time diff " << stime.elapsed();
}


bool table_controller::isFilterMatched(const log_info_per_line_t &str) {
    if (m_log_filter.tid.isEmpty() &&
            m_log_filter.pid.isEmpty()&&
            m_log_filter.tag.isEmpty()&&
            m_log_filter.msg.isEmpty()) {
        return true; // indicate no need filter.
    }

    /**
     * if tid is empty, skipped match, goto next item match.
     * if tid is not empty, but not match the str. return false.
     * if tid is not empty, and match the str, goto next item match.
     **/
    if (!m_log_filter.tid.isEmpty()) {
        for (int i = 0; i < m_log_filter.tid.size(); ++i) {
            if(str.tid.indexOf(m_log_filter.tid.at(i)) != -1) {
                break;
            }
            if (i == m_log_filter.tid.size() -1) return false;
        }
    }

    /**
     * if pid is empty, skipped match, goto next item match.
     * if pid is not empty, but not match the str. return false.
     * if pid is not empty, and match the str, goto next item match.
     **/
    if (!m_log_filter.pid.isEmpty()) {
        for (int i = 0; i < m_log_filter.pid.size(); ++i) {
            if(str.pid.indexOf(m_log_filter.pid.at(i)) != -1) {
                break;
            }
            if (i == m_log_filter.pid.size() -1) return false;
        }
    }

    /**
     * if tag is empty, skipped match, goto next item match.
     * if tag is not empty, but not match the str. return false.
     * if tag is not empty, and match the str, goto next item match.
     **/
    if (!m_log_filter.tag.isEmpty()) {
        for (int i = 0; i < m_log_filter.tag.size(); ++i) {
            if(str.tag.indexOf(m_log_filter.tag.at(i)) != -1) {
                break;
            }
            if (i == m_log_filter.tag.size() -1) return false;
        }
    }

    /**
     * if msg is empty, skipp match. and return true.
     * if msg is not empty, but not match the str, return false.
     * if msg is not empty, and match the str, return true;
     **/
    if (!m_log_filter.msg.isEmpty()) {
        for (int i = 0; i < m_log_filter.msg.size(); ++i) {
            if (str.msg.indexOf(m_log_filter.msg.at(i)) != -1)
                return true;
        }
        return false;
    }
    return true;
}

void table_controller::showAllLogs() {
    log_info_t *logData = m_model->getLogDataPtr();
    log_info_t filterData;
    int row = logData->size();
    for (int i = 0; i < row; i++) {
        filterData.append(logData->at(i));
    }
    m_model->setLogFilterData(filterData);
    m_log_filter.msg.clear();
    m_log_filter.pid.clear();
    m_log_filter.tid.clear();
    m_log_filter.tag.clear();
    m_delegate->updateMsgFilter(m_log_filter.msg);
}

void table_controller::processLogOnline(QString str, int line_count) {
    //qDebug() << bArray;
    if (m_android_online_cmd != ANDROID_STOP) {
        if (m_android_online_filter_updated ||
                m_android_online_level_updated) {
            processFilterPrivate();
            m_android_online_filter_updated = false;
            m_android_online_level_updated = false;
            qDebug() << "process Log On line for previous filter info";
        }
    }
    if (str.size() < 50)
        str.resize(50);
    log_info_per_line_t vec = m_logConfig->processPerLine(str);
    vec.line = line_count;
    m_model->appendLogData(vec);
    if (isLevelVisible(vec.level)
            &&isFilterMatched(vec)) {
        m_model->appendLogFilterData(vec);
    }
}

void table_controller::setAdbCmd(ANDROID_ONLINE_CMD cmd) {
    if (cmd != m_android_online_cmd) {
        qDebug() << "new cmd = " << cmd
                 << "cmd = " << m_android_online_cmd;
        m_android_online_cmd = cmd;
        if (m_log_online) m_log_online->updateLogOnlineCmd(cmd);
        switch (cmd) {
        case ANDROID_CLEAR:
            m_model->clearData();
            break;
        case ANDROID_PAUSE:
            m_scroll_timer->stop();
            break;
        case ANDROID_RESUME:
            m_scroll_timer->start(10);
            break;
        case ANDROID_RUN:
            m_model->clearData();
            m_scroll_timer->start(10);
            m_log_online->start();
            break;
        case ANDROID_STOP:
        default:
            m_scroll_timer->stop();
            m_log_online->exit();
            break;
        }

    }
}


void table_controller::scrollToBottom() {
    if (m_view) m_view->scrollToBottom();
}

void table_controller::setFont(const QFont &font) {
    if (m_view) m_view->setFont(font);
}


void table_controller::recieveLineNumber(int line) {
    if (m_view && m_model) {
        qDebug() << __func__ << "goto line:" << line;
        m_view->selectRow(line);
        m_view->scrollTo(m_model->getModexIndex(line, 0));
        m_view->setFocus();
    }
}

void table_controller::tableCustomMenuRequest(QPoint point) {
    if (m_menu) {
        m_menu->popup(m_view->viewport()->mapToGlobal(point));
    }
}

void table_controller::showMore() {
    QModelIndex index =
            m_view->indexAt(m_view->viewport()->mapFromGlobal(m_menu->pos()));
    qint32 row = index.row();
    log_info_t *logDataPtr =
            m_model->getLogDataPtr();
    log_info_t more_log;
    if (row < 0 ||
            logDataPtr->size() == 0 ||
            row > logDataPtr->size() ||
            !m_show_more_log)
    {
        return;
    }
    int line = m_model->getLogFilterDataPtr()->at(row).line -1;
    qDebug() << "row = " << row << "line= " << line;
    m_show_more_log->clearLog();
    int front_count = 0;
    int back_count = 0;
    QStringList log;
    for (int i = line; i > 0; --i) {
        QString str;
        str.append(QString::number(logDataPtr->at(i).line));
        str.append("\t");
        str.append(logDataPtr->at(i).time);
        str.append("\t");
        str.append(logDataPtr->at(i).level);
        str.append("\t");
        str.append(logDataPtr->at(i).pid);
        str.append("\t");
        str.append(logDataPtr->at(i).tid);
        str.append("\t");
        str.append(logDataPtr->at(i).tag);
        str.append("\t");
        str.append(logDataPtr->at(i).msg);
        log.insert(0, str);

        if (++front_count == SHOW_MORE_LINES_THRESHOLD) {
            break;
        }
    }
    for (int i = line+1; i < logDataPtr->size(); ++i) {
        QString str;
        str.append(QString::number(logDataPtr->at(i).line));
        str.append("\t");
        str.append(logDataPtr->at(i).time);
        str.append("\t");
        str.append(logDataPtr->at(i).level);
        str.append("\t");
        str.append(logDataPtr->at(i).pid);
        str.append("\t");
        str.append(logDataPtr->at(i).tid);
        str.append("\t");
        str.append(logDataPtr->at(i).tag);
        str.append("\t");
        str.append(logDataPtr->at(i).msg);
        log.append(str);
        if (++back_count == SHOW_MORE_LINES_THRESHOLD) {
            break;
        }
    }
    m_show_more_log->appendLog(log.join("\n"));
    m_show_more_log->scrollToLine(front_count);
    m_show_more_log->show();
    m_show_more_log->activateWindow();
}

void table_controller::showMoreSameTag() {
    QModelIndex index =
            m_view->indexAt(m_view->viewport()->mapFromGlobal(m_menu->pos()));
    qint32 row = index.row();
    log_info_t *logDataPtr =
            m_model->getLogDataPtr();
    log_info_t more_log;
    if (row < 0 ||
            logDataPtr->size() == 0 ||
            row > logDataPtr->size() ||
            !m_show_more_log)
    {
        return;
    }
    int line = m_model->getLogFilterDataPtr()->at(row).line -1;
    qDebug() << "row = " << row << "line= " << line;
    m_show_more_log->clearLog();
    QString match_tag = logDataPtr->at(line).tag;
    int front_count = 0;
    int back_count = 0;
    QStringList log;
    for (int i  = line; i >= 0; --i) {
        if (logDataPtr->at(i).tag.compare(match_tag) == 0) {
            QString str;
            str.append(QString::number(logDataPtr->at(i).line));
            str.append("\t");
            str.append(logDataPtr->at(i).time);
            str.append("\t");
            str.append(logDataPtr->at(i).level);
            str.append("\t");
            str.append(logDataPtr->at(i).pid);
            str.append("\t");
            str.append(logDataPtr->at(i).tid);
            str.append("\t");
            str.append(logDataPtr->at(i).tag);
            str.append("\t");
            str.append(logDataPtr->at(i).msg);
            log.insert(0, str);
            if (++front_count == SHOW_MORE_LINES_THRESHOLD) {
                break;
            }
        }
    }
    for (int i = line; i < logDataPtr->size(); ++i) {
        if (logDataPtr->at(i).tag.compare(match_tag) == 0) {
            QString str;
            str.append(QString::number(logDataPtr->at(i).line));
            str.append("\t");
            str.append(logDataPtr->at(i).time);
            str.append("\t");
            str.append(logDataPtr->at(i).level);
            str.append("\t");
            str.append(logDataPtr->at(i).pid);
            str.append("\t");
            str.append(logDataPtr->at(i).tid);
            str.append("\t");
            str.append(logDataPtr->at(i).tag);
            str.append("\t");
            str.append(logDataPtr->at(i).msg);
            log.append(str);
            if (++back_count == SHOW_MORE_LINES_THRESHOLD) {
                break;
            }

        }
    }
    m_show_more_log->appendLog(log.join("\n"));
    m_show_more_log->scrollToLine(front_count);
    m_show_more_log->show();
    m_show_more_log->activateWindow();
}
void table_controller::showMoreSamePid() {
    QModelIndex index =
            m_view->indexAt(m_view->viewport()->mapFromGlobal(m_menu->pos()));
    qint32 row = index.row();
    log_info_t *logDataPtr =
            m_model->getLogDataPtr();
    log_info_t more_log;
    if (row < 0 ||
            logDataPtr->size() == 0 ||
            row > logDataPtr->size() ||
            !m_show_more_log)
    {
        return;
    }
    int line = m_model->getLogFilterDataPtr()->at(row).line -1;
    qDebug() << "row = " << row << "line= " << line;
    m_show_more_log->clearLog();
    QString match_pid = logDataPtr->at(line).pid;
    int front_count = 0;
    int back_count = 0;
    QStringList log;
    for (int i  = line; i >= 0; --i) {
        if (logDataPtr->at(i).pid.compare(match_pid) == 0) {
            QString str;
            str.append(QString::number(logDataPtr->at(i).line));
            str.append("\t");
            str.append(logDataPtr->at(i).time);
            str.append("\t");
            str.append(logDataPtr->at(i).level);
            str.append("\t");
            str.append(logDataPtr->at(i).pid);
            str.append("\t");
            str.append(logDataPtr->at(i).tid);
            str.append("\t");
            str.append(logDataPtr->at(i).tag);
            str.append("\t");
            str.append(logDataPtr->at(i).msg);
            log.insert(0, str);
            if (++front_count == SHOW_MORE_LINES_THRESHOLD) {
                break;
            }
        }
    }
    for (int i = line; i < logDataPtr->size(); ++i) {
        if (logDataPtr->at(i).pid.compare(match_pid) == 0) {
            QString str;
            str.append(QString::number(logDataPtr->at(i).line));
            str.append("\t");
            str.append(logDataPtr->at(i).time);
            str.append("\t");
            str.append(logDataPtr->at(i).level);
            str.append("\t");
            str.append(logDataPtr->at(i).pid);
            str.append("\t");
            str.append(logDataPtr->at(i).tid);
            str.append("\t");
            str.append(logDataPtr->at(i).tag);
            str.append("\t");
            str.append(logDataPtr->at(i).msg);
            log.append(str);
            if (++back_count == SHOW_MORE_LINES_THRESHOLD) {
                break;
            }
        }
    }
    m_show_more_log->appendLog(log.join("\n"));
    m_show_more_log->scrollToLine(front_count);
    m_show_more_log->show();
    m_show_more_log->activateWindow();
}
void table_controller::showMoreSameTid() {
    QModelIndex index =
            m_view->indexAt(m_view->viewport()->mapFromGlobal(m_menu->pos()));
    qint32 row = index.row();
    log_info_t *logDataPtr =
            m_model->getLogDataPtr();
    log_info_t more_log;
    if (row < 0 ||
            logDataPtr->size() == 0 ||
            row > logDataPtr->size() ||
            !m_show_more_log)
    {
        return;
    }
    int line = m_model->getLogFilterDataPtr()->at(row).line -1;
    qDebug() << "row = " << row << "line= " << line;
    m_show_more_log->clearLog();
    QString match_tid = logDataPtr->at(line).tid;
    int front_count = 0;
    int back_count = 0;
    QStringList log;
    for (int i  = line; i >= 0; --i) {
        if (logDataPtr->at(i).tid.compare(match_tid) == 0) {
            QString str;
            str.append(QString::number(logDataPtr->at(i).line));
            str.append("\t");
            str.append(logDataPtr->at(i).time);
            str.append("\t");
            str.append(logDataPtr->at(i).level);
            str.append("\t");
            str.append(logDataPtr->at(i).pid);
            str.append("\t");
            str.append(logDataPtr->at(i).tid);
            str.append("\t");
            str.append(logDataPtr->at(i).tag);
            str.append("\t");
            str.append(logDataPtr->at(i).msg);
            log.insert(0, str);
            if (++front_count == SHOW_MORE_LINES_THRESHOLD) {
                break;
            }
        }
    }
    for (int i = line; i < logDataPtr->size(); ++i) {
        if (logDataPtr->at(i).tid.compare(match_tid) == 0) {
            QString str;
            str.append(QString::number(logDataPtr->at(i).line));
            str.append("\t");
            str.append(logDataPtr->at(i).time);
            str.append("\t");
            str.append(logDataPtr->at(i).level);
            str.append("\t");
            str.append(logDataPtr->at(i).pid);
            str.append("\t");
            str.append(logDataPtr->at(i).tid);
            str.append("\t");
            str.append(logDataPtr->at(i).tag);
            str.append("\t");
            str.append(logDataPtr->at(i).msg);
            log.append(str);
            if (++back_count == SHOW_MORE_LINES_THRESHOLD) {
                break;
            }
        }
    }
    m_show_more_log->appendLog(log.join("\n"));
    m_show_more_log->scrollToLine(front_count);
    m_show_more_log->show();
    m_show_more_log->activateWindow();
}

void table_controller::setOnLineLogFile(QString path) {
    if (m_log_online) m_log_online->setLogPath(path);
}

online_log_process::online_log_process() {
    m_path.clear();
}

void online_log_process::setLogPath(QString path) {
    m_path = path;
    qDebug() << "set path to " << path;
}

void online_log_process::updateLogOnlineCmd(ANDROID_ONLINE_CMD cmd) {
    m_cmd = cmd;
    qDebug() << __func__ << "cmd = " << cmd;
}

void online_log_process::run() {
    if (m_path.isEmpty()) return;

    QFile file(m_path);
    if (file.open(QIODevice::ReadOnly) == false) {
        qDebug() << "can't open for read:" << file.fileName();
        return;
    }
    int line_count = 0;
    while (m_cmd != ANDROID_STOP) {
        if (file.isReadable() && m_cmd != ANDROID_PAUSE) {
            QString str = file.readLine().trimmed();
            if (str.isEmpty()) continue;
            //++line_count;
            emit processLogOnline(str, ++line_count);
            //if (line_count %100 == 0)  qDebug() << str;
        } else {
            msleep(2);
        }
    }
}
