#include "log_spreadsheet.h"
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QHeaderView>

log_spreadsheet::log_spreadsheet(QTableWidget *table) {

    m_table = table;
    m_logConfig = log_config::getInstance();
    if (m_logConfig->isConfigValid()) {
        const QVector<QString> &keys = m_logConfig->getKeys();
        const QVector<qint16> &widths = m_logConfig->getWidths();

        int size = keys.size();
        m_table->setColumnCount(size);
        for (int i = 0; i < keys.size(); ++i) {
            QTableWidgetItem *item = new QTableWidgetItem(keys.at(i));
            item->setTextAlignment(Qt::AlignLeft);
            //item->setFlags();
            m_table->setHorizontalHeaderItem(i, item);
            qDebug() << "log_spreadsheet = " << widths.at(i) << endl;
            m_table->setColumnWidth(i, widths.at(i));
        }

    }
    m_table->setRowCount(100);
    m_column_visible = 0xffff; // all the column are visible default
    m_level_visible = 0xffff; // all the log level are visible default
}

bool log_spreadsheet::checkConfigValid() {
    return m_logConfig->isConfigValid();
}

bool log_spreadsheet::processLog(QString &filename) {
    qDebug() << "file = " << filename << endl;
    m_logdata.clear();


    volatile int idx = 0;
    QTime stime;
    stime.start();
    log_load_thread * thread = new log_load_thread(filename, m_logConfig, &m_logdata);
    thread->start();
#if 1
    /* here we need reserved as much as possible.
     * to avoid the thread access qvector segementfalut issue.
     * the setItem, malloc may crash randomly with the thread
     * root cause should be the vector alloc/access will make the copy failed.
     */
    m_logdata.reserve(500000);

    m_table->clearContents();
    int col = m_table->columnCount();
    while (thread->isRunning() ||
           (thread->isFinished() && idx < m_logdata.size())) {
        QThread::msleep(3);
        int i = idx;
        int new_idx = m_logdata.size();
        m_table->setRowCount(new_idx + 1);
        for (;i < new_idx; ++i) {
            /* previous here is the copy function with QVector<QString> vec = m_logdata.at(i),
             * it will make the temporay variable cost too much time to slower the performance.
             */
            const QVector<QString> &vec = m_logdata.at(i);
            for (int j = 0; j < col; ++j) {
                if (vec.at(j).isEmpty() || vec.at(j).isNull()) {
                    qDebug() << "is Empty, i = " << i << "j = " << j << endl;
                } else {
                    m_table->setItem(i, j, new QTableWidgetItem(vec.at(j)));
                }
            }
        }
        idx = new_idx;
    }
#else
    thread->wait();
    tableWidget->setRowCount(m_logdata.size()+1);//m_logdata.size());
    for (int i = 0; i < m_logdata.size(); ++i) {
        QVector<QString> vec = m_logdata.at(i);
        if ( i %1000 == 0) QThread::msleep(3);
        for (int j = 0; j < vec.size(); ++j) {
            if (vec.at(j).isEmpty() || vec.at(j).isNull()) {
                qDebug() << "is Empty, i = " << i << "j = " << j << endl;
             } else {
                tableWidget->setItem(i, j, new QTableWidgetItem(vec.at(j)));
            }
        }
    }
    //tableWidget->setRowCount(m_logdata.size() + 1);

#endif
    thread->destroyed();
    qDebug() << "total log process diff time" << stime.elapsed() << "ms" << endl;
    return true;
}

void log_spreadsheet::setColumnVisible(TABLE_COL_TYPE type, bool visible){
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

void log_spreadsheet::updateColumnVisible(TABLE_COL_TYPE type, bool visible) {
    m_table->setColumnHidden(type, !visible);
}

void log_spreadsheet::setLogLevelVisible(LOG_LEVEL level, bool visible) {
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

void log_spreadsheet::updateLogLevelVisible() {
    QTime stime;
    stime.start();
#if 0
    //here the setRowHidden's performance is very slow.
    QTableWidgetItem *item;
    int row = m_table->rowCount();
    for(int i = 0;i < row; ++i) {
        item = m_table->item(i, TABLE_COL_TYPE_LEVEL);
        if (item) {
            if (item->text().compare(tag) ==0) {
                 m_table->setRowHidden(i, hideden);
            }
        } else {
            qDebug("bad row: %d", i);
        }
    }
#else
    m_table->clearContents();
    int row = m_logdata.size();
    int col = m_table->columnCount();
    m_table->setRowCount(row);
    int row_index = 0;
    for (int i = 0; i < row; ++i) {
        const QVector<QString> &vec = m_logdata.at(i);
        if (isLevelVisible(vec.at(TABLE_COL_TYPE_LEVEL))) {
            //m_table->setVerticalHeaderItem(row_index, new QTableWidgetItem(QString::number(row_index)));
            for (int j = 0; j < col; ++j) {
                m_table->setItem(row_index, j, new QTableWidgetItem(vec.at(j)));
            }
            ++row_index;
        }
    }
    m_table->setRowCount(row_index+1);
#endif
    qDebug() << "row hidden time diff: " << stime.elapsed() << endl;
}

const QString g_log_level[LOG_LEVEL_MAX] = {
    "I", "V", "W", "D", "E", "F"
};

bool log_spreadsheet::isLevelVisible(const QString &leve) {
    if (leve.isEmpty()) {
        qDebug() << "isEmpty" << endl;
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

void log_spreadsheet::processFilter(const log_filter_t& filter) {
    if (m_logdata.isEmpty()) return;

    //msg filter check
    QStringList msgList;

    if (!filter.msg.isEmpty()) {
        int index= 0;
        int start_offset = 0;
        while (true) {
            index = filter.msg.indexOf("|", start_offset);
            if (index == -1) {
                msgList.append(filter.msg.mid(start_offset, -1));
                break;
            } else {
                msgList.append(filter.msg.mid(start_offset, index - start_offset));
                start_offset = index+1;
            }
        }
    }
    qDebug() << "msg filter =" << filter.msg << "list = " << msgList << endl;
    m_table->clearContents();
    int row = m_logdata.size();
    int col = m_table->columnCount();
    m_table->setRowCount(row);
    int row_index = 0;
    for (int i = 0; i < row; ++i) {
        const QVector<QString> &vec = m_logdata.at(i);
        //qDebug() << "row_index" << row_index << endl;
        if (isLevelVisible(vec.at(TABLE_COL_TYPE_LEVEL))
                && isTidMatched(vec.at(TABLE_COL_TYPE_TID), filter.tid)
                && isPidMatched(vec.at(TABLE_COL_TYPE_PID), filter.pid)
                && isTagMatched(vec.at(TABLE_COL_TYPE_TAG), filter.tag)
                && isMsgMatched(vec.at(TABLE_COL_TYPE_MSG), msgList)) {
            for (int j = 0; j < col; ++j) {
                m_table->setItem(row_index, j, new QTableWidgetItem(vec.at(j)));
            }
            ++row_index;
        }
    }
    m_table->setRowCount(row_index+1);
}


bool log_spreadsheet::isTidMatched(const QString &str, const QString &tid) {
    if (tid.isEmpty()) return true;// if the tid is empty, skip match it
    if (str.isEmpty()) return false; // if the str is empty, remove it.

    if (str.indexOf(tid) == -1) {
        return false;
    } else {
        return true;
    }
}

bool log_spreadsheet::isPidMatched(const QString &str, const QString &pid) {
    if (pid.isEmpty()) return true;// if the pid is empty, skip match it
    if (str.isEmpty()) return false; // if the str is empty, remove it.

    if (str.indexOf(pid) == -1) {
        return false;
    } else {
        return true;
    }
}

bool log_spreadsheet::isTagMatched(const QString &str, const QString &tag) {
    if (tag.isEmpty()) return true;// if the tag is empty, skip match it.
    if (str.isEmpty()) return false; // if the str is empty, remove it.

    if (str.indexOf(tag) == -1) {
        return false;
    } else {
        return true;
    }
}

bool log_spreadsheet::isMsgMatched(const QString& str, const QStringList& list) {
    if (list.isEmpty()) return true; // if the msglist is empty, skip match it.
    if (str.isEmpty()) return false; // if the str is empty, remove it.
    for (int i = 0; i < list.size(); ++i) {
        if (str.indexOf(list.at(i)) != -1)
            return true;
    }
    return false;
}

void log_spreadsheet::showAllLogs() {
    if (m_logdata.isEmpty()) return;
    m_table->clearContents();
    int row = m_logdata.size();
    int col = m_table->columnCount();
    m_table->setRowCount(row);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; ++j) {
            m_table->setItem(i, j, new QTableWidgetItem(m_logdata.at(i).at(j)));
        }
    }
}
