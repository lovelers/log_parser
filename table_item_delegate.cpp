#include "table_item_delegate.h"
#include "log_config.h"
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QPainter>
#include <QTextDocument>

table_item_delegate::table_item_delegate()
{

}

void table_item_delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == TABLE_COL_TYPE_MSG) {
        if (msgFilter.isEmpty()) {
            QStyledItemDelegate::paint(painter, option, index);
        } else {
            //const QFont& font =  painter->font();
            QString str = index.data().toString();
            for (int i = 0; i < msgFilter.size(); i++) {
                int index = str.indexOf(msgFilter.at(i), 0, Qt::CaseInsensitive);
                if (index != -1) {
                    int mark_end = str.indexOf("</font>", index);
                    int mark_start;
                    if (mark_end != -1) {
                        mark_start = str.indexOf("<font color='red'", index);
                    }
                    if (mark_start > mark_end) {
                        str.insert(index + msgFilter.at(i).size(), "</font>");
                        str.insert(index, "<font color='red'>");
                    }
                }
            }
            auto options = option;
            initStyleOption(&options, index);

            painter->save();
            QTextDocument doc;
            options.text = str;
            doc.setHtml(options.text);

            options.text = "";
            options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

            painter->translate(options.rect.left(), options.rect.top());
            //painter->setPen(Qt::Blue);
            QRect clip(0, 0, options.rect.width(), options.rect.height());
            doc.drawContents(painter, clip);

            painter->restore();
        }
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

void table_item_delegate::updateMsgFilter(const QStringList & list) {
    msgFilter = list;
    qDebug() << "updateMsgFilter: " << msgFilter;
}

