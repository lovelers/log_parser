#include "log_parser_widget.h"
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QEventLoop>

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


    volatile int idx = 0;
    QTime stime;
    stime.start();
    QTableWidget *tableWidget = ui->TableWidget;
    log_load_thread * thread = new log_load_thread(filename, m_logConfig, &m_logdata);
    thread->start();
#if 1


    /* here we need reserved as much as possible.
     * to avoid the thread access qvector segementfalut issue.
     * the setItem, malloc may crash randomly with the thread
     * root cause should be the vector alloc/access will make the copy failed.
     */

    m_logdata.reserve(500000);

    int col = tableWidget->columnCount();
    while (thread->isRunning() ||
           (thread->isFinished() && idx < m_logdata.size())) {
        QThread::msleep(3);
        int i = idx;
        int new_idx = m_logdata.size();
        tableWidget->setRowCount(new_idx + 1);
        for (;i < new_idx; ++i) {
            /* previous here is the copy function with QVector<QString> vec = m_logdata.at(i),
             * it will make the temporay variable cost too much time to slower the performance.
             */
            const QVector<QString> &vec = m_logdata.at(i);
            for (int j = 0; j < col; ++j) {
                if (vec.at(j).isEmpty() || vec.at(j).isNull()) {
                    qDebug() << "is Empty, i = " << i << "j = " << j << endl;
                } else {
                    tableWidget->setItem(i, j, new QTableWidgetItem(vec.at(j)));
                }
            }
        }
        idx = new_idx;
    }
#else
    thread->wait();
    tableWidget->setRowCount(m_logdata.size()+1);//m_logdata.size());
    for (int i = 0; i < m_logdata.size(); ++i) {
        QVector<QString> vec = m_logdata.at(i);
        if ( i %1000 == 0) QThread::msleep(3);
        for (int j = 0; j < vec.size(); ++j) {
            if (vec.at(j).isEmpty() || vec.at(j).isNull()) {
                qDebug() << "is Empty, i = " << i << "j = " << j << endl;
             } else {
                tableWidget->setItem(i, j, new QTableWidgetItem(vec.at(j)));
            }
        }
    }
    //tableWidget->setRowCount(m_logdata.size() + 1);

#endif
    thread->destroyed();
    qDebug() << "total log process diff time" << stime.elapsed() << "ms" << endl;
    return true;
}

void log_parser::logFilterOnClick() {
    qDebug() << "logFilter:" <<  ui->editLogFilter->text() << endl;
}
