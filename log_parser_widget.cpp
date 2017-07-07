#include "log_parser_widget.h"
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QDebug>
#include <QMessageBox>

#include <QThread>
log_parser::log_parser(Ui::MainWindow *ui) {
    this->ui = ui;
    m_logConfig = log_config::getInstance();
    if (m_logConfig->isConfigValid()) {
        const QVector<QString> &keys = m_logConfig->getKeys();
        const QVector<qint16> &widths = m_logConfig->getWidths();
        int size = keys.size();
        QTableWidget *tableWidget = ui->TableWidget;
        tableWidget->setColumnCount(size);
        for (int i = 0; i < keys.size(); ++i) {
            QTableWidgetItem *item = new QTableWidgetItem(keys.at(i));
            item->setTextAlignment(Qt::AlignLeft);
            tableWidget->setHorizontalHeaderItem(i, item);
            qDebug() << "log_parser = " << widths.at(i) << endl;
            tableWidget->setColumnWidth(i, widths.at(i));
        }
        tableWidget->setRowCount(100);
    }
}
void log_parser::setVisible(bool visible) {
    ui->LogParserWidget->setVisible(visible);
}

bool log_parser::checkConfigValid() {
    return m_logConfig->isConfigValid();
}

bool log_parser::processLog(QString &filename) {
    qDebug() << "file = " << filename << endl;
    m_logdata.clear();
    log_load_thread * thread = new log_load_thread(filename, m_logConfig, &m_logdata);
    thread->start();

    volatile int idx = 0;
    QTime stime;
    stime.start();
    QTableWidget *tableWidget = ui->TableWidget;
#if 0
/* the setItem may crash randomly with the thread */
    tableWidget->setRowCount(200000);
    while (thread->isRunning()
           || (thread->isFinished() && idx < m_logdata.size())) {
        QThread::msleep(3);
        int i = idx;
        int new_idx = m_logdata.size();
        for (;i < new_idx; ++i) {
            QVector<QString> vec = m_logdata.at(i);
            for (int j = 0; j <vec.size(); ++j) {
                QTableWidgetItem *item = new QTableWidgetItem(vec.at(j));
                if (vec.at(j).isEmpty() || vec.at(j).isNull()) {
                    qDebug() << "is Empty" << endl;
                } else {
                    tableWidget->setItem(i, j, item);
                }
            }
        }
        idx = new_idx;
    }
    //tableWidget->setRowCount(m_logdata.size()+1);
#else
    thread->wait();
    tableWidget->setRowCount(200000);//m_logdata.size());
    for (int i = 0; i < m_logdata.size(); ++i) {
        QVector<QString> vec = m_logdata.at(i);
        for (int j = 0; j < vec.size(); ++j) {
            //tableWidget->item(i,j)->setText(vec.at(j));
            tableWidget->setItem(i, j, new QTableWidgetItem(vec.at(j)));
        }
    }
    tableWidget->setRowCount(m_logdata.size() + 1);

#endif
    qDebug() << "total log process diff time" << stime.elapsed() << "ms" << endl;
    return true;
}

void log_parser::logFilterOnClick() {
    qDebug() << "logFilter:" <<  ui->editLogFilter->text() << endl;
}
