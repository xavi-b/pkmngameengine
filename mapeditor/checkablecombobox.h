#ifndef CHECKCOMBOBOX_H
#define CHECKCOMBOBOX_H

#include <QComboBox>
#include <QStyledItemDelegate>
#include <QCheckBox>

class CheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CheckBoxDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

signals:
    void itemSelected(int index);

protected:
    bool editorEvent(QEvent*                     event,
                     QAbstractItemModel*         model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex&          index) override;
};

class CheckableComboBox : public QComboBox
{
    Q_OBJECT

public:
    CheckableComboBox(QWidget* parent = nullptr);
};

#endif // CHECKCOMBOBOX_H
