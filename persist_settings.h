#ifndef PERSIST_SETTINGS_H
#define PERSIST_SETTINGS_H

#include <QDialog>
#include "log_config.h"
#include <QTableView>
namespace Ui {
class persist_settings;
}

typedef struct {
    QString name;
    QString default_value;
    QString value_range;
    QString set_value;
    QString cur_value;
} persist_model_data_t;

typedef enum {
    MODEL_DATA_INDEX_NAME,
    MODEL_DATA_INDEX_DEFAULT_VALUE,
    MODEL_DATA_INDEX_VALUE_RANGE,
    MODEL_DATA_INDEX_SET_VALUE,
    MODEL_DATA_INDEX_CUR_VALUE,
    MODEL_DATA_INDEX_MAX
} MODEL_DATA_INDEX;

const QString g_header_vertial[MODEL_DATA_INDEX_MAX] = {
    "name",
    "default value",
    "value range",
    "set value",
    "cur value"
};

class persist_model : public QAbstractTableModel {
public:
    persist_model(QObject *parent = 0);
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    void setPersistData(const QList<persist_model_data_t> &data);
    const QList<persist_model_data_t> &getPersistData();

    void updateCurValue(int index, QString cur_value);
    void resetToDefaultValue(int index);
private:
    QList<persist_model_data_t> m_data;
    bool checkSetValueValidate(const QString &str, const QString &range);
};

class persist_settings : public QDialog
{
    Q_OBJECT

public:
    explicit persist_settings(QWidget *parent = 0);
    ~persist_settings();

private:
    Ui::persist_settings *ui;
    //QVector<persist_model_data> m_persist_model_data;
    QTableView *m_view;
    persist_model *m_model;

public slots:
    void configurePressed();
    void resetPressed();
};

#endif // PERSIST_SETTINGS_H
