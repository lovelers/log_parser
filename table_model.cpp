#include "table_model.h"

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
        if (filter_line_info.size() < section)
            return QVariant(-1);
        else
            return QVariant(filter_line_info.at(section));
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
        return  filter_data.at(index.row()).at(index.column());
    }
    return QVariant();
}

Qt::ItemFlags table_model::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool table_model::setData(const QModelIndex &index, const QVariant &value, int role) {
    return false;
}

void table_model::appendLogData(const QVector<QString> &data) {
    this->beginResetModel();
    log_data.append(data);
    this->endResetModel();
}

void table_model::setLogData(const QVector<QVector<QString>> &data) {
    log_data = data;
}

void table_model::clearData() {
    log_data.clear();
    this->beginResetModel();
    filter_data.clear();
    filter_line_info.clear();
    this->endResetModel();
}

QVector<QVector<QString>> * table_model::getLogDataPtr() {
    return &log_data;
}

QVector<QVector<QString>> * table_model::getLogFilterData() {
    return &filter_data;
}

QVector<qint32> * table_model::getLogFilterLine() {
    return &filter_line_info;
}

void table_model::setLogFilterData(const QVector<QVector<QString>> &data,
                                   const QVector<qint32> &line_info) {
    this->beginResetModel();
    filter_data = data;
    filter_line_info = line_info;
    this->endResetModel();
}

void table_model::appendLogFilterData(const QVector<QString> &data, int line) {
    this->beginResetModel();
    filter_data.append(data);
    filter_line_info.append(line);
    this->endResetModel();
}
