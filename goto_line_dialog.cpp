#include "goto_line_dialog.h"
#include "ui_line_dialog.h"

goto_line_dialog::goto_line_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GotoLineDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Goto");
    QObject::connect(ui->number_edit, SIGNAL(returnPressed()),
                     this, SLOT(returnKeyPressed()));
}

void goto_line_dialog::returnKeyPressed() {
    emit sendLineNumber(ui->number_edit->text().toInt());
    this->close();
}
