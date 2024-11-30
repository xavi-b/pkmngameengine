#ifndef CHECKCOMBOBOX_H
#define CHECKCOMBOBOX_H

#include <QCheckBox>
#include <QComboBox>
#include <QStyledItemDelegate>

class CheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CheckBoxDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, QStyleOptionViewItem const& option, QModelIndex const& index) const override;

signals:
    void itemSelected(int index);

protected:
    bool editorEvent(QEvent*                     event,
                     QAbstractItemModel*         model,
                     QStyleOptionViewItem const& option,
                     QModelIndex const&          index) override;
};

class CheckableComboBox : public QComboBox
{
    Q_OBJECT

public:
    CheckableComboBox(QWidget* parent = nullptr);

protected:
    void initStyleOption(QStyleOptionComboBox* option) const;
};

#endif // CHECKCOMBOBOX_H
