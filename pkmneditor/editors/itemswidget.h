#ifndef ITEMSWIDGET_H
#define ITEMSWIDGET_H

#include "../checkablecombobox.h"
#include "itemdef.h"
#include "movedef.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QWidget>

class ItemsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ItemsWidget(QWidget* parent = nullptr);

    void setAvailableItems(std::vector<ItemDef::ItemDefPtr> const& items);
    void setItem(ItemDef::ItemDefPtr const& item);

    void setAvailableMoves(std::vector<MoveDef::MoveDefPtr> const& moves);

    void setSpritesDirectory(QString const& dirName);

signals:
    void idChanged();

private:
    ItemDef::ItemDefPtr item;

    QLineEdit*         idLineEdit;
    QLabel*            sprite;
    QLineEdit*         nameLineEdit;
    QLineEdit*         namePluralLineEdit;
    QLineEdit*         portionNameLineEdit;
    QLineEdit*         portionNamePluralLineEdit;
    QComboBox*         pocketComboBox;
    QSpinBox*          priceSpinBox;
    QSpinBox*          bpPriceSpinBox;
    QComboBox*         fieldUseComboBox;
    QComboBox*         battleUseComboBox;
    QSpinBox*          sellPriceSpinBox;
    QComboBox*         moveComboBox;
    CheckableComboBox* flagsComboBox;
    QCheckBox*         consumableCheckBox;
    QLineEdit*         naturalGiftTypeLineEdit;
    QSpinBox*          naturalGiftPowerSpinBox;
    QTextEdit*         descriptionTextEdit;

    QString spritesDirectory;
};

#endif // ITEMSWIDGET_H
