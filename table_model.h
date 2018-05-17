#ifndef TABLE_MODEL_H
#define TABLE_MODEL_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QMutex>
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
    void setLogData(const log_info_t &data);
    void appendLogData(const log_info_per_line_t &data);
    void clearData();
    log_info_t * getLogDataPtr();
    void setLogFilterData(const log_info_t &data);
    void appendLogFilterData(const log_info_per_line_t &data);
    log_info_t *getLogFilterDataPtr();
    QModelIndex getModexIndex(int line, int col);
    int getLogDataLastLine() const;
private:
    log_config *m_log_config;
    log_info_t log_data;
    log_info_t filter_data;
    QMutex filter_data_lock;
    QMutex log_data_lock;
};

#endif // TABLE_MODEL_H
