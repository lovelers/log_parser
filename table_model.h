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
    void updateLogData(const QVector<QVector<QString>> &data);
private:
    log_config *m_log_config;
    QVector<QVector<QString>> log_data;
};

#endif // TABLE_MODEL_H
