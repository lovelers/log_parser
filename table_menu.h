#ifndef table_menu_H
#define table_menu_H

#include <QDialog>

namespace Ui {
class table_menu;
}

class table_menu : public QDialog
{
    Q_OBJECT

public:
    explicit table_menu(QWidget *parent = 0);
    ~table_menu();
    void appendLog(const QString &log);
    void clearLog();
    void scrollToLine(int line);

private:
    Ui::table_menu *ui;
};

#endif // table_menu_H
