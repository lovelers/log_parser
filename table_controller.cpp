#include "table_controller.h"
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QHeaderView>

table_controller::table_controller(QTableView *view) {
    m_logConfig = log_config::getInstance();
    m_view = view;
    m_model = new table_model();
    m_delegate = new table_item_delegate();
    m_view->setModel(m_model);
    m_view->setItemDelegate(m_delegate);
    if (m_logConfig->isConfigValid()) {
        const QVector<QString> &keys = m_logConfig->getKeys();
        const QVector<qint16> &widths = m_logConfig->getWidths();

        int size = keys.size();
        for (int i = 0; i < keys.size(); ++i) {
            m_view->setColumnWidth(i, widths.at(i));
        }
    }
    m_column_visible = 0xffff; // all the column are visible default
    m_level_visible = 0xffff; // all the log level are visible default
}

bool table_controller::checkConfigValid() {
    return m_logConfig->isConfigValid();
}

bool table_controller::processLog(QString &filename) {
    qDebug() << "file = " << filename << endl;
    m_logdata.clear();


    volatile int idx = 0;
    QTime stime;
    stime.start();
    log_load_thread * thread = new log_load_thread(filename, m_logConfig, &m_logdata);
    thread->start();

    /* here we need reserved as much as possible.
     * to avoid the thread access qvector segementfalut issue.
     * the setItem, malloc may crash randomly with the thread
     * root cause should be the vector alloc/access will make the copy failed.
     */
    m_logdata.reserve(500000);
    thread->wait();
    m_model->updateLogData(m_logdata);
    thread->destroyed();
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
    int row = m_logdata.size();
    for (int i = 0; i < row; ++i) {
        m_view->setRowHidden(i, !isLevelVisible(m_logdata.at(i).at(TABLE_COL_TYPE_LEVEL)));
    }
    qDebug() << "row hidden time diff: " << stime.elapsed() << endl;
}

const QString g_log_level[LOG_LEVEL_MAX] = {
    "I", "V", "W", "D", "E", "F"
};

bool table_controller::isLevelVisible(const QString &leve) {
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

void table_controller::processFilter(const log_filter_t& filter) {
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

    int row = m_logdata.size();
    qDebug() << "msg filter =" << filter.msg << "list = " << msgList << "row = " << row << endl;
    for (int i = 0; i < row; ++i) {
        const QVector<QString> &vec = m_logdata.at(i);
        //qDebug() << "row_index" << row_index << endl;
        if (isLevelVisible(vec.at(TABLE_COL_TYPE_LEVEL))
                && isTidMatched(vec.at(TABLE_COL_TYPE_TID), filter.tid)
                && isPidMatched(vec.at(TABLE_COL_TYPE_PID), filter.pid)
                && isTagMatched(vec.at(TABLE_COL_TYPE_TAG), filter.tag)
                && isMsgMatched(vec.at(TABLE_COL_TYPE_MSG), msgList)) {

            m_view->setRowHidden(i, false);
        } else {
            m_view->setRowHidden(i, true);
        }
    }
    m_delegate->updateMsgFilter(msgList);

}


bool table_controller::isTidMatched(const QString &str, const QString &tid) {
    if (tid.isEmpty()) return true;// if the tid is empty, skip match it
    if (str.isEmpty()) return false; // if the str is empty, remove it.

    if (str.indexOf(tid) == -1) {
        return false;
    } else {
        return true;
    }
}

bool table_controller::isPidMatched(const QString &str, const QString &pid) {
    if (pid.isEmpty()) return true;// if the pid is empty, skip match it
    if (str.isEmpty()) return false; // if the str is empty, remove it.

    if (str.indexOf(pid) == -1) {
        return false;
    } else {
        return true;
    }
}

bool table_controller::isTagMatched(const QString &str, const QString &tag) {
    if (tag.isEmpty()) return true;// if the tag is empty, skip match it.
    if (str.isEmpty()) return false; // if the str is empty, remove it.

    if (str.indexOf(tag) == -1) {
        return false;
    } else {
        return true;
    }
}

bool table_controller::isMsgMatched(const QString& str, const QStringList& list) {
    if (list.isEmpty()) return true; // if the msglist is empty, skip match it.
    if (str.isEmpty()) return false; // if the str is empty, remove it.
    for (int i = 0; i < list.size(); ++i) {
        if (str.indexOf(list.at(i)) != -1)
            return true;
    }
    return false;
}

void table_controller::showAllLogs() {
    int row = m_logdata.size();
    for (int i = 0; i < row; i++) {
        m_view->showRow(i);
    }
}
