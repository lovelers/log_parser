#ifndef GOTO_LINE_DIALOG_H
#define GOTO_LINE_DIALOG_H

#include <QWidget>
#include <QDialog>

namespace Ui {
class GotoLineDialog;
}
class goto_line_dialog : public QDialog
{
    Q_OBJECT
public:
    explicit goto_line_dialog(QWidget *parent = nullptr);
    int returnLineNumber();
private:
    Ui::GotoLineDialog *ui;
    int line_number;
signals:
    void sendLineNumber(int);
public slots:
    void returnKeyPressed();
};

#endif // GOTO_LINE_DIALOG_H
