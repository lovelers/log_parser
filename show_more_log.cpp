#include "show_more_log.h"
#include "ui_show_more_log.h"
#include <QTextCursor>
#include <QTextBlock>
show_more_log::show_more_log(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::show_more_log)
{
    ui->setupUi(this);
}

void show_more_log::appendLog(const QString &log) {
    ui->log_text->append(log);
}

void show_more_log::clearLog() {
    ui->log_text->clear();
   // ui->log_text->setCursor();
}

void show_more_log::scrollToLine(int line) {
    QTextBlock text_block =  ui->log_text->document()->findBlockByLineNumber(line - 1);
    QTextCursor text_cursor = ui->log_text->textCursor();
    text_cursor.setPosition(text_block.position());
    text_cursor.select(QTextCursor::LineUnderCursor);
    ui->log_text->setTextCursor(text_cursor);
}

show_more_log::~show_more_log()
{
    delete ui;
}
