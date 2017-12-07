#include "table_model.h"
#include <QDebug>

static const int MAX_LOG_INFO_COUNT = 2000000;
table_model::table_model(QObject *parent):
    QAbstractTableModel(parent)
{
    m_log_config = log_config::getInstance();
}

int table_model::rowCount(const QModelIndex &parent) const {
    //if (filter_data.size() < 100) return 100;
    return filter_data.size();
}

int table_model::columnCount(const QModelIndex &parent) const {
    return m_log_config->getKeys().size();
}

QVariant table_model::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        return m_log_config->getKeys().at(section);
    }
    if (orientation == Qt::Vertical) {
        if (filter_data.size() < section)
            return QVariant(-1);
        else
            return QVariant(filter_data.at(section).line);
    }
    return QVariant();
}


QVariant table_model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= filter_data.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
        case TABLE_COL_TYPE_DATE:
            return filter_data.at(index.row()).date;
        case TABLE_COL_TYPE_TIME:
            return filter_data.at(index.row()).time;
        case TABLE_COL_TYPE_LEVEL:
            return filter_data.at(index.row()).level;
        case TABLE_COL_TYPE_PID:
            return filter_data.at(index.row()).pid;
        case TABLE_COL_TYPE_TID:
            return filter_data.at(index.row()).tid;
        case TABLE_COL_TYPE_TAG:
            return filter_data.at(index.row()).tag;
        case TABLE_COL_TYPE_MSG:
            return filter_data.at(index.row()).msg;
        }
    }
    return QVariant();
}

Qt::ItemFlags table_model::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index)| Qt::ItemIsDropEnabled;
}

bool table_model::setData(const QModelIndex &index, const QVariant &value, int role) {
    return false;
}

void table_model::appendLogData(const log_info_per_line_t &data) {
    this->beginResetModel();
    log_data_lock.lock();
    if (log_data.size() > MAX_LOG_INFO_COUNT)
    {
        log_data.removeFirst();
    }
    log_data.append(data);
    log_data_lock.unlock();
    this->endResetModel();
}

void table_model::setLogData(const log_info_t &data) {
    log_data_lock.lock();
    log_data = data;
    log_data_lock.unlock();
}

void table_model::clearData() {
    log_data_lock.lock();
    log_data.clear();
    log_data_lock.unlock();
    this->beginResetModel();
    filter_data_lock.lock();
    filter_data.clear();
    filter_data_lock.unlock();
    this->endResetModel();
}

log_info_t * table_model::getLogDataPtr() {
    return &log_data;
}

log_info_t * table_model::getLogFilterDataPtr() {
    return &filter_data;
}


void table_model::setLogFilterData(const log_info_t &data) {
    this->beginResetModel();
    filter_data_lock.lock();
    if (filter_data.size() > MAX_LOG_INFO_COUNT)
    {
        filter_data.removeFirst();
    }
    filter_data = data;
    filter_data_lock.unlock();
    this->endResetModel();
}

void table_model::appendLogFilterData(const log_info_per_line_t &data) {
    this->beginResetModel();
    filter_data_lock.lock();
    filter_data.append(data);
    filter_data_lock.unlock();
    this->endResetModel();
}

QModelIndex table_model::getModexIndex(int line, int col) {
    int s_start = 0;
    int s_end = filter_data.size() -1;
    int s_mid = s_end / 2;
    int dir = 0;
    if (filter_data.isEmpty()) return this->index(0,col);
    if (line > filter_data.at(s_end).line) return this->index(s_end,col);
    while (true) {
        dir = line - filter_data.at(s_mid).line;
        if (dir > 0) {
            s_start = s_mid;
            s_mid = (s_start + s_end) / 2;
        } else if (dir < 0){
            s_end = s_mid;
            s_mid = (s_start + s_end) / 2;
        } else { // dir == 0
            break;
        }
        if (s_start == s_end ||
                s_start == s_mid ||
                s_end == s_mid) {
            qDebug() << "can't find the target line";
            break;
        }
        qDebug() << "s_start = " << s_start
                 << "s_end  = " << s_end
                 << "s_mid = " << s_mid;
    }
    qDebug() << "find the Index:" << s_mid+1;
    return this->index(s_mid, col);
}
