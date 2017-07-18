#ifndef TABLE_ITEM_DELEGATE_H
#define TABLE_ITEM_DELEGATE_H
#include <QStyledItemDelegate>

class table_item_delegate : public QStyledItemDelegate
{
public:
    table_item_delegate();
    void updateMsgFilter(const QStringList & list);
protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QStringList msgFilter;
};

#endif // TABLE_ITEM_DELEGATE_H
