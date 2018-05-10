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
#include <QApplication>
#include <QClipboard>
#include "table_menu.h"
#include <QtAlgorithms>
table_controller::table_controller(QTableView *view) {
    m_logConfig = log_config::getInstance();

    m_view = view;
    m_model = new table_model();
    m_delegate = new table_item_delegate();

    //m_view->setAcceptDrops(true);
    //m_view->setDropIndicatorShown(true);
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

    m_log_copy = new QAction("Copy");
    m_log_expand = new QAction("Expand");
    m_log_expand_by_tag = new QAction("Expand by tag");
    m_log_expand_by_pid = new QAction("Expand by pid");
    m_log_expand_by_tid = new QAction("Expand by tid");

    QObject::connect(m_log_copy, SIGNAL(triggered(bool)),
                     this, SLOT(logCopy()));
    QObject::connect(m_log_expand, SIGNAL(triggered()),
                     this, SLOT(logExpand()));
    QObject::connect(m_log_expand_by_tag, SIGNAL(triggered(bool)),
                     this, SLOT(logExpandByTag()));
    QObject::connect(m_log_expand_by_pid, SIGNAL(triggered(bool)),
                     this, SLOT(logExpandByPid()));
    QObject::connect(m_log_expand_by_tid, SIGNAL(triggered(bool)),
                     this, SLOT(logExpandByTid()));
    QObject::connect(m_view, SIGNAL(doubleClicked(QModelIndex)),
                     this, SLOT(tableDoubleClick(QModelIndex)));

    m_menu->addAction(m_log_copy);
    m_menu->addAction(m_log_expand);
    m_menu->addAction(m_log_expand_by_tag);
    m_menu->addAction(m_log_expand_by_pid);
    m_menu->addAction(m_log_expand_by_tid);
    m_table_menu = new table_menu;

    m_log_online = new online_log_process();
    QObject::connect(m_log_online, SIGNAL(signalLogOnline(QString,int)),
                     this, SLOT(processLogOnline(QString,int)));
    m_android_online_cmd = ANDROID_STOP;
}

table_controller::~table_controller() {
    if (m_log_copy) delete m_log_copy;
    if (m_log_expand) delete m_log_expand;
    if (m_log_expand_by_tag) delete m_log_expand_by_tag;
    if (m_log_expand_by_pid) delete m_log_expand_by_pid;
    if (m_log_expand_by_tid) delete m_log_expand_by_tid;
    if (m_table_menu) delete m_table_menu;
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
        int line_count = 0;
        while (!file->atEnd()) {
            log_info_per_line = log_config::processPerLine(file->readLine().trimmed(), ltype);
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
    if (!m_log_online->isRunning()) {
        processFilterPrivate();
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
        processFilterPrivate();
        qDebug() << "end update nwe filter" << endl;
    }
}
void table_controller::processFilterPrivate() {
    m_log_filter_lock.lock();
    qDebug() << "start process Filter Private" << endl;
    log_info_t *logData = m_model->getLogDataPtr();
    m_delegate->updateMsgFilter(m_log_filter.msg);
    if (logData->isEmpty())
    {
        m_log_filter_lock.unlock();
        return;
    }

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

    qDebug() << "filter time diff " << stime.elapsed();
    m_log_filter_lock.unlock();
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
    m_model->setLogData(filterData);
    m_log_filter.msg.clear();
    m_log_filter.pid.clear();
    m_log_filter.tid.clear();
    m_log_filter.tag.clear();
    m_delegate->updateMsgFilter(m_log_filter.msg);
}

void table_controller::processLogOnline(QString str, int line_count) {
    m_log_filter_lock.lock();
    //qDebug() << bArray;
    if (str.size() < 50)
        str.resize(50);
    log_info_per_line_t vec = log_config::processPerLine(str);
    vec.line = line_count;

    m_model->appendLogData(vec);
    if (isLevelVisible(vec.level)
            &&isFilterMatched(vec)) {
        m_model->appendLogFilterData(vec);
    }
    m_log_filter_lock.unlock();
}

void table_controller::setAdbCmd(ANDROID_ONLINE_CMD cmd) {
    if (cmd != m_android_online_cmd) {
        qDebug() << "new cmd = " << cmd
                 << "cmd = " << m_android_online_cmd;

        if (m_log_online) m_log_online->updateLogOnlineCmd(cmd);
        switch (cmd) {
        case ANDROID_CLEAR:
            m_model->clearData();
            m_log_online->updateLogOnlineCmd(ANDROID_CLEAR);
            //keep cmd to be previous cmd.
            m_android_online_cmd = m_android_online_cmd;
            break;
        case ANDROID_PAUSE:
            m_scroll_timer->stop();
            m_log_online->updateLogOnlineCmd(ANDROID_PAUSE);
            m_android_online_cmd = ANDROID_PAUSE;
            break;
        case ANDROID_RESUME:
            m_scroll_timer->start(10);
            m_log_online->updateLogOnlineCmd(ANDROID_RESUME);
            m_android_online_cmd = ANDROID_RESUME;
            break;
        case ANDROID_RUN:
            m_model->clearData();
            m_scroll_timer->start(10);
            m_log_online->updateLogOnlineCmd(ANDROID_RUN);
            m_log_online->start();
            m_android_online_cmd = ANDROID_RUN;
            break;
        case ANDROID_STOP:
        default:
            m_scroll_timer->stop();
            m_log_online->updateLogOnlineCmd(ANDROID_STOP);
            m_log_online->exit();
            m_log_online->wait(10);
            m_android_online_cmd = ANDROID_STOP;
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
        QModelIndex index = m_model->getModexIndex(line, 0);
        m_view->scrollTo(index);
        m_view->setFocus();
        m_view->selectRow(index.row());
    }
}

void table_controller::tableCustomMenuRequest(QPoint point) {
    if (m_menu) {
        m_menu->popup(m_view->viewport()->mapToGlobal(point));
    }
}

void table_controller::logExpand() {
    QItemSelectionModel *selection= m_view->selectionModel();
    log_info_t* logFilterDataPtr = m_model->getLogFilterDataPtr();
    log_info_t* logDataPtr = m_model->getLogDataPtr();

    if (selection->hasSelection()
            && NULL != logFilterDataPtr
            && NULL != logDataPtr) {
        qDebug() << "hasSelecton";
        //int maxLine = logDataPtr->size()-1;
        int start = selection->selection().first().topLeft().row();
        int end = selection->selection().first().bottomRight().row();
        qDebug() << "start = " << start;
        qDebug() << "end = " << end;

        int logStart = logFilterDataPtr->at(start).line;
        int logEnd = logFilterDataPtr->at(end).line;
        int select = 0;

        if (logStart == logEnd)
        {
            //Do Nothing.
            select = MIN(logStart, 200);
            logStart = MAX(0, logStart - 200);
            logEnd = MIN(logDataPtr->last().line, logEnd + 200);
        }

        QString log;
        qDebug() << "logStart = " << logStart;
        qDebug() << "logEnd = " << logEnd;
        for (int i = logStart; i <= logEnd; ++i)
        {
            log.append(QString::number(logDataPtr->at(i).line));
            log.append("\t");
            log.append(logDataPtr->at(i).date);
            log.append("\t");
            log.append(logDataPtr->at(i).time);
            log.append("\t");
            log.append(logDataPtr->at(i).level);
            log.append("\t");
            log.append(logDataPtr->at(i).pid);
            log.append("\t");
            log.append(logDataPtr->at(i).tid);
            log.append("\t");
            log.append(logDataPtr->at(i).tag);
            log.append("\t");
            log.append(logDataPtr->at(i).msg);
            log.append("\n");
        }
        m_table_menu->setLog(log);
        m_table_menu->scrollToLine(select+1);
        m_table_menu->show();
        m_table_menu->activateWindow();
    }
}

void table_controller::logExpandByType(TABLE_COL_TYPE type)
{
    QItemSelectionModel *selection= m_view->selectionModel();
    log_info_t * logFilterDataPtr = m_model->getLogFilterDataPtr();
    log_info_t * logDataPtr = m_model->getLogDataPtr();
    int maxLine = logDataPtr->size();
    int line = 0;
    QString match;
    QString log;
    int select = 0;
    if (selection->hasSelection()
            && NULL != logFilterDataPtr
            && NULL != logDataPtr) {
        qDebug() << "hasSelecton";
        line = selection->selection().first().topLeft().row();
        switch (type)
        {
            case TABLE_COL_TYPE_TAG:
                match = logFilterDataPtr->at(line).tag;
                for (int i = 0; i < maxLine; i++)
                {
                    if (logDataPtr->at(i).tag.compare(match) == 0) {
                        log.append(QString::number(logDataPtr->at(i).line));
                        log.append("\t");
                        log.append(logDataPtr->at(i).date);
                        log.append("\t");
                        log.append(logDataPtr->at(i).time);
                        log.append("\t");
                        log.append(logDataPtr->at(i).level);
                        log.append("\t");
                        log.append(logDataPtr->at(i).pid);
                        log.append("\t");
                        log.append(logDataPtr->at(i).tid);
                        log.append("\t");
                        log.append(logDataPtr->at(i).tag);
                        log.append("\t");
                        log.append(logDataPtr->at(i).msg);
                        log.append("\n");
                        if (i <= line)
                        {
                            select++;
                        }
                    }
                }
                break;
            case TABLE_COL_TYPE_PID:
                match = logFilterDataPtr->at(line).pid;
                for (int i = 0; i < maxLine; i++)
                {
                    if (logDataPtr->at(i).pid.compare(match) == 0) {
                        log.append(QString::number(logDataPtr->at(i).line));
                        log.append("\t");
                        log.append(logDataPtr->at(i).date);
                        log.append("\t");
                        log.append(logDataPtr->at(i).time);
                        log.append("\t");
                        log.append(logDataPtr->at(i).level);
                        log.append("\t");
                        log.append(logDataPtr->at(i).pid);
                        log.append("\t");
                        log.append(logDataPtr->at(i).tid);
                        log.append("\t");
                        log.append(logDataPtr->at(i).tag);
                        log.append("\t");
                        log.append(logDataPtr->at(i).msg);
                        log.append("\n");
                        if (i <= line)
                        {
                            select++;
                        }
                    }
                }
                break;
            case TABLE_COL_TYPE_TID:
                match = logFilterDataPtr->at(line).tid;
                for (int i = 0; i < maxLine; i++)
                {
                    if (logDataPtr->at(i).tid.compare(match) == 0) {
                        log.append(QString::number(logDataPtr->at(i).line));
                        log.append("\t");
                        log.append(logDataPtr->at(i).date);
                        log.append("\t");
                        log.append(logDataPtr->at(i).time);
                        log.append("\t");
                        log.append(logDataPtr->at(i).level);
                        log.append("\t");
                        log.append(logDataPtr->at(i).pid);
                        log.append("\t");
                        log.append(logDataPtr->at(i).tid);
                        log.append("\t");
                        log.append(logDataPtr->at(i).tag);
                        log.append("\t");
                        log.append(logDataPtr->at(i).msg);
                        log.append("\n");
                        if (i <= line)
                        {
                            select++;
                        }
                    }
                }
                break;
            default:
                break;
        }
        m_table_menu->setLog(log);
        m_table_menu->scrollToLine(select);
        m_table_menu->show();
        m_table_menu->activateWindow();
    }
}

void table_controller::logExpandByTag() {
    logExpandByType(TABLE_COL_TYPE_TAG);
}
void table_controller::logExpandByPid() {
    logExpandByType(TABLE_COL_TYPE_PID);
}
void table_controller::logExpandByTid() {
    logExpandByType(TABLE_COL_TYPE_TID);
}

void table_controller::setOnLineLogFile(QString path) {
    if (m_log_online) {
        if (m_log_online->isRunning()) {
            this->setAdbCmd(ANDROID_STOP);
            m_log_online->setLogPath(path);
            this->setAdbCmd(ANDROID_RUN);
            this->setAdbCmd(m_android_online_cmd);
        } else {
            m_log_online->setLogPath(path);
        }
    }
}

void table_controller::tableDoubleClick(const QModelIndex &index)
{
    qDebug() << "index = " << index.row() << endl;
    logExpand();
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
    qDebug() << "start run online log process with file: " << m_path;
    if (m_path.isEmpty()) return;

    QFile file(m_path);
    if (file.open(QIODevice::ReadOnly) == false) {
        qDebug() << "can't open for read:" << file.fileName();
        return;
    }
    int line_count = 0;

    QString line;
    while (m_cmd != ANDROID_STOP) {
        if (file.isReadable() && m_cmd != ANDROID_PAUSE) {
            line = line.append(file.readLine());
            if (line.endsWith('\n')) {
                emit signalLogOnline(line.trimmed(), line_count++);
                line.clear();
            } else {
                //qDebug() << "not a full line: " << line;
            }
        }
    }

    file.close();

}

void table_controller::logCopy() {
    QItemSelectionModel *selection= m_view->selectionModel();
    if (selection->hasSelection()) {
        qDebug() << "hasSelecton";
        QString text;
        log_info_t * logDataPtr = m_model->getLogFilterDataPtr();
        int start = selection->selection().first().topLeft().row();
        int end = selection->selection().first().bottomRight().row();
        qDebug() << "start = " << start;
        qDebug() << "end = " << end;
        for (int i = start; i <= end; i++) {
            text.append(QString::number(logDataPtr->at(i).line));
            text.append("\t");
            text.append(logDataPtr->at(i).date);
            text.append("\t");
            text.append(logDataPtr->at(i).time);
            text.append("\t");
            text.append(logDataPtr->at(i).level);
            text.append("\t");
            text.append(logDataPtr->at(i).pid);
            text.append("\t");
            text.append(logDataPtr->at(i).tid);
            text.append("\t");
            text.append(logDataPtr->at(i).tag);
            text.append("\t");
            text.append(logDataPtr->at(i).msg);
            text.append("\n");
        }
        QApplication::clipboard()->setText(text);
        qDebug()<<"copy" << text;
        return;
    }
    return;
}


void table_controller::closeWindow() {
    if (m_table_menu) m_table_menu->close();
}
