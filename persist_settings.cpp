#include "persist_settings.h"
#include "ui_persist_settings.h"
#include "log_config.h"
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QProcess>
#include "adb_online.h"
persist_settings::persist_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::persist_settings)
{
    ui->setupUi(this);
    const QVector<Persist> & persist =
            log_config::getInstance()->getPersist();
    QList<persist_model_data_t> persist_model_data;
    for (int i = 0; i < persist.size(); ++i) {
        persist_model_data_t s_data;
        s_data.name = persist.at(i).name;
        s_data.default_value = persist.at(i).value;
        s_data.value_range = persist.at(i).value_range;
        s_data.set_value = persist.at(i).value;
        s_data.cur_value = persist.at(i).value;
        persist_model_data.append(s_data);
    }
    m_model = new persist_model();
    m_model->setPersistData(persist_model_data);
    m_view = ui->tableView;

    m_view->setModel(m_model);
    m_view->resizeColumnsToContents();
    QObject::connect(ui->configure, SIGNAL(pressed()),
                     this, SLOT(configurePressed()));
    QObject::connect(ui->reset, SIGNAL(pressed()),
                     this, SLOT(resetPressed()));

}


void persist_settings::configurePressed() {
    if (m_model) {
        const QList<persist_model_data_t> &data = m_model->getPersistData();
        QStringList list = adb_online::checkDevices();
        if (list.size() == 0) {
            qDebug() << "check Devices() failed";
            return;
        }
        if (list.size() > 1) {
            // select the devices.
            return;
        }
        // only one device.
        bool root_remount = adb_online::adbRootRemount();
        if (root_remount == false) {
            // remount faled.
            qDebug() << "root remount failed";
            return;
        }
        for (int i = 0; i < data.size(); i++) {
            if (data.at(i).set_value.compare(data.at(i).cur_value) == 0) {
                //qDebug() << "no change for " << data.at(i).name;
                continue;
            }
            QString cur_value = adb_online::adbProperity(
                        data.at(i).name, data.at(i).set_value);
            m_model->updateCurValue(i, cur_value);
        }
    }
}

void persist_settings::resetPressed() {
    if (m_model) {
        const QList<persist_model_data_t> &data = m_model->getPersistData();
        QStringList list = adb_online::checkDevices();
        if (list.size() == 0) {
            qDebug() << "check Devices() failed";
            return;
        }
        if (list.size() > 1) {
            // select the devices.
            return;
        }
        // only one device.
        bool root_remount = adb_online::adbRootRemount();
        if (root_remount == false) {
            // remount faled.
            qDebug() << "root remount failed";
            return;
        }
        for (int i = 0; i < data.size(); i++) {
            if (data.at(i).set_value.compare(data.at(i).default_value) == 0) {
                //qDebug() << "no change for " << data.at(i).name;
                continue;
            }
            QString cur_value = adb_online::adbProperity(
                        data.at(i).name, data.at(i).default_value);
            if (cur_value.compare(data.at(i).default_value) == 0) {
                m_model->resetToDefaultValue(i);
            }
            //m_model->updateCurValue(i, cur_value);
        }
    }
}

persist_settings::~persist_settings()
{
    delete ui;
    if (m_model) delete m_model;
}


// perssit_model
persist_model::persist_model(QObject *parent):
    QAbstractTableModel(parent)
{

}

void persist_model::setPersistData(const QList<persist_model_data_t> &data) {
    this->beginResetModel();
    m_data = data;
    this->endResetModel();
}

const QList<persist_model_data_t> & persist_model::getPersistData() {
    return m_data;
}

int persist_model::rowCount(const QModelIndex &parent) const {
    return m_data.size();
}

int persist_model::columnCount(const QModelIndex &parent) const {
    return MODEL_DATA_INDEX_MAX;
}

QVariant persist_model::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)  return QVariant();
    if (orientation == Qt::Horizontal) {
        return g_header_vertial[section];
    }
    return QVariant();
}

QVariant persist_model::data(const QModelIndex &index, int role) const {
    if (!index.isValid())  return QVariant();

    if (index.row() >= m_data.size() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
        case MODEL_DATA_INDEX_NAME:
            return m_data.at(index.row()).name;
        case MODEL_DATA_INDEX_DEFAULT_VALUE:
            return m_data.at(index.row()).default_value;
        case MODEL_DATA_INDEX_VALUE_RANGE:
            return m_data.at(index.row()).value_range;
        case MODEL_DATA_INDEX_SET_VALUE:
            return m_data.at(index.row()).set_value;
        case MODEL_DATA_INDEX_CUR_VALUE:
            return m_data.at(index.row()).cur_value;
        }
    }
    return QVariant();
}
Qt::ItemFlags persist_model::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() == MODEL_DATA_INDEX_SET_VALUE) {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractTableModel::flags(index);
}

bool persist_model::setData(const QModelIndex &index, const QVariant &value, int role) {
    qDebug() << "role: " << role << "value:" << value << "index.data:" << index.data();
    QString str = value.toString();
    if (role == Qt::DisplayRole) {
    } else if (role ==Qt::EditRole) {
        if (index.column() == MODEL_DATA_INDEX_SET_VALUE && checkSetValueValidate(str, m_data[index.row()].value_range)) {
            m_data[index.row()].set_value = value.toString();
        }
    } else {
    }
    return false;
}

bool persist_model::checkSetValueValidate(const QString &str, const QString &range) {
    if (str.isEmpty() || str.isNull()) return false;
    if (range.isEmpty() || str.isNull()) return false;
    QRegularExpression re;
    QString range_match = "[\\d+\\d+]";
    re.setPattern(range_match);
    qDebug() << "value range = " << range;
    QRegularExpressionMatch match = re.match(range);
    if (match.hasMatch()) {
        // range format is [xx-xxx];
        int a = range.indexOf('[');
        int b = range.indexOf('-');
        int c = range.indexOf(']');
        int range_min = range.mid(a+1, b-a-1).toInt();
        int range_max = range.mid(b+1, c-b-1).toInt();

        bool ok = false;
        int value = str.toInt(&ok, 10);
        qDebug() << "range_min = " << range_min
                 << ", range_max = " << range_max
                 << "value = " << value
                 << "ok = " << ok;
        if (ok == false) return false;
        if (value > range_max || value < range_min)
            return false;
        else
            return true;
    }
    return false;
}

void persist_model::updateCurValue(int index, QString cur_value) {
    this->beginResetModel();
    m_data[index].cur_value = cur_value;
    this->endResetModel();
}

void persist_model::resetToDefaultValue(int index) {
    this->beginResetModel();
    qDebug() << "reset" << m_data[index].name
             << "cur value" << m_data[index].cur_value
             << "set value " << m_data[index].set_value
             << "to default value " << m_data[index].default_value;

    m_data[index].cur_value = m_data[index].default_value;
    m_data[index].set_value = m_data[index].default_value;
    this->endResetModel();
}
