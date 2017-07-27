#ifndef TABLE_MODEL_H
#define TABLE_MODEL_H

#include <QWidget>
#include <QAbstractTableModel>
#include "log_config.h"
class table_model : public QAbstractTableModel
{
public:
    table_model(QObject *parent = 0);
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    void setLogData(const QVector<QVector<QString>> &data);
    void appendLogData(const QVector<QString> &data);
    void clearData();
    QVector<QVector<QString>> * getLogDataPtr();
    void setLogFilterData(const QVector<QVector<QString>> &data, const QVector<qint32> &line_info);
    void appendLogFilterData(const QVector<QString> &data, int line);
    inline QModelIndex getLastModelIndex() {
        return this->index(filter_line_info.size(),0);
    }
    QVector<QVector<QString>> *getLogFilterData();
    QVector<qint32> *getLogFilterLine();
    log_config *m_log_config;
    QVector<QVector<QString>> log_data;
    QVector<QVector<QString>> filter_data;
    QVector<qint32> filter_line_info;
};

#endif // TABLE_MODEL_H
