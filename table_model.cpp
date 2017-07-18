#include "table_model.h"

table_model::table_model(QObject *parent):
    QAbstractTableModel(parent)
{
    m_log_config = log_config::getInstance();
#if 0
    for (int i = 0; i < 100000; ++i) {
        QStringList list;
        list << "hello" << "world" << "asdfasdf" << "asdfsdf" << "12" << "24" << "34";
        log_data.append(list);
    }
#endif
}

int table_model::rowCount(const QModelIndex &parent) const {
    if (log_data.size() < 100) return 100;
    return log_data.size();
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
        return QVariant(section+1);
    }
    return QVariant();
}


QVariant table_model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= log_data.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        return  log_data.at(index.row()).at(index.column());
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

void table_model::updateLogData(const QVector<QVector<QString>> &data) {
    this->beginResetModel();
    log_data = data;
    this->endResetModel();
}
