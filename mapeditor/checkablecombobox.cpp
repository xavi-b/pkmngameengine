#include "checkablecombobox.h"

#include <QApplication>
#include <QListView>
#include <QMouseEvent>

CheckBoxDelegate::CheckBoxDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

bool CheckBoxDelegate::editorEvent(QEvent*                     event,
                                   QAbstractItemModel*         model,
                                   QStyleOptionViewItem const& option,
                                   QModelIndex const&          index)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        auto opt     = option;
        auto boxSize = QApplication::style()->subElementRect(QStyle::SE_CheckBoxClickRect, &opt).size();
        opt.rect     = {option.rect.topLeft(), boxSize};

        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent)
        {
            if (opt.rect.contains(mouseEvent->pos()))
            {
                bool value = index.data(Qt::UserRole).toBool();
                model->setData(index, !value, Qt::UserRole);
            }
            else if (option.rect.contains(mouseEvent->pos()))
            {
                emit itemSelected(index.row());
            }
            event->accept();
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void CheckBoxDelegate::paint(QPainter* painter, QStyleOptionViewItem const& option, QModelIndex const& index) const
{
    bool               value = index.data(Qt::UserRole).toBool();
    QStyleOptionButton checkBoxOption;
    checkBoxOption.state |= value ? QStyle::State_On : QStyle::State_Off;
    checkBoxOption.state |= QStyle::State_Enabled;
    checkBoxOption.rect = option.rect.adjusted(0, 0, -1, -1);
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);

    QStyleOptionViewItem textOption(option);
    textOption.rect             = option.rect.adjusted(20, 0, 0, 0);
    textOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    QString text                = index.data(Qt::DisplayRole).toString();
    QApplication::style()->drawItemText(painter,
                                        textOption.rect,
                                        textOption.displayAlignment,
                                        option.palette,
                                        true,
                                        text,
                                        QPalette::WindowText);
}

CheckableComboBox::CheckableComboBox(QWidget* parent) : QComboBox(parent)
{
    setMinimumWidth(200);
    auto delegate = new CheckBoxDelegate;
    connect(delegate, &CheckBoxDelegate::itemSelected, this, [=](int index) {
        setCurrentIndex(index);
        hidePopup();
    });
    view()->setItemDelegate(delegate);
}
