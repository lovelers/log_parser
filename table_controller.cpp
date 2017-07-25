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
        for (int i = 0; i < size; ++i) {
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
    qDebug()<< "filename = " << filename << endl;
    QTime stime;
    stime.start();
    QFile *file = new QFile(filename);

    QVector<QVector<QString>> logData;
    QVector<QVector<QString>> filterData;
    QVector<qint32> filterLine;

    QVector<QString> tmp;
    logData.reserve(500000);
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(file);
        while (!in.atEnd()) {
            tmp = m_logConfig->processPerLine(in.readLine());
            if (isFilterMatched(tmp)) {
                filterData.append(tmp);
                filterLine.append(logData.size());
            }
            logData.append(tmp);
        }
        file->close();
    } else {
        qDebug() << "open file failed" << endl;
    }
    m_model->setLogData(logData);
    m_model->setLogFilterData(filterData, filterLine);
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
    QVector<QVector<QString>> *logData = m_model->getLogDataPtr();
    int row = logData->size();
    for (int i = 0; i < row; ++i) {
        m_view->setRowHidden(i, !isLevelVisible(logData->at(i).at(TABLE_COL_TYPE_LEVEL)));
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
    m_log_filter = filter;

    QVector<QVector<QString>> *logData = m_model->getLogDataPtr();
    if (logData->isEmpty()) return;

    QVector<QVector<QString>> filterData;
    QVector<qint32> filterLine;
    int row = logData->size();
    qDebug() << "row = " << row << endl;
    QTime stime;
    stime.start();
    for (int i = 0; i < row; ++i) {
        const QVector<QString> &vec = logData->at(i);
        //qDebug() << "row_index" << row_index << endl;
        if (isLevelVisible(vec.at(TABLE_COL_TYPE_LEVEL))
                && isFilterMatched(vec)) {
            filterData.append(vec);
            filterLine.append(i);
        } else {
            //m_view->setRowHidden(i, true);
        }
    }
    m_model->setLogFilterData(filterData, filterLine);
    m_delegate->updateMsgFilter(m_log_filter.msg);
    qDebug() << "filter time diff " << stime.elapsed();
}


bool table_controller::isFilterMatched(const QVector<QString> &str) {

    if (m_log_filter.tid.isEmpty() &&
            m_log_filter.pid.isEmpty()&&
            m_log_filter.tag.isEmpty()&&
            m_log_filter.msg.isEmpty()) {
        return true; // indicate no need filter.
    }

    if (!m_log_filter.tid.isEmpty()
            && str.at(TABLE_COL_TYPE_TID).indexOf(m_log_filter.tid) != -1) {
        return true;
    }

    if (!m_log_filter.pid.isEmpty()
            && str.at(TABLE_COL_TYPE_PID).indexOf(m_log_filter.pid) != -1) {
        return true;
    }

    if (!m_log_filter.tag.isEmpty()
            && str.at(TABLE_COL_TYPE_TAG).indexOf(m_log_filter.tag) != -1) {
        return true;
    }

    for (int i = 0; i < m_log_filter.msg.size(); ++i) {
        if (str.at(TABLE_COL_TYPE_MSG).indexOf(m_log_filter.msg.at(i)) != -1)
            return true;
    }
    return false;
}

void table_controller::showAllLogs() {
    int row = m_model->getLogDataPtr()->size();
    for (int i = 0; i < row; i++) {
        m_view->showRow(i);
    }
}

void table_controller::processLogOnline(const QByteArray &bArray) {
    //qDebug() << bArray;
    QVector<QString> vec = m_logConfig->processPerLine(bArray);
    if (isFilterMatched(vec)) {
        m_model->appendLogFilterData(vec, m_model->getLogDataPtr()->size());
    }
    m_model->appendLogData(vec);
}

void table_controller::android_run() {
    m_model->clearData();
}

void table_controller::android_resume() {
    //
}

void table_controller::android_pause() {
}

void table_controller::android_stop() {
    //
}

void table_controller::android_clear() {
    m_model->clearData();
}
