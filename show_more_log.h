#ifndef SHOW_MORE_LOG_H
#define SHOW_MORE_LOG_H

#include <QDialog>

namespace Ui {
class show_more_log;
}

class show_more_log : public QDialog
{
    Q_OBJECT

public:
    explicit show_more_log(QWidget *parent = 0);
    ~show_more_log();
    void appendLog(const QString &log);
    void clearLog();
    void scrollToLine(int line);

private:
    Ui::show_more_log *ui;
};

#endif // SHOW_MORE_LOG_H
