#include "table_menu.h"
#include "ui_table_menu.h"
#include <QTextCursor>
#include <QTextBlock>
table_menu::table_menu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::table_menu)
{
    ui->setupUi(this);
}

void table_menu::setLog(const QString &log) {
    ui->log_text->setText(log);
    //ui->log_text->document()->setHtml(log);
}

void table_menu::scrollToLine(int line) {
    QTextBlock text_block =  ui->log_text->document()->findBlockByLineNumber(line - 1);
    QTextCursor text_cursor = ui->log_text->textCursor();
    text_cursor.setPosition(text_block.position());
    text_cursor.select(QTextCursor::LineUnderCursor);
    ui->log_text->setTextCursor(text_cursor);
}

table_menu::~table_menu()
{
    delete ui;
}
