#ifndef ITEMSWIDGET_H
#define ITEMSWIDGET_H

#include "../checkablecombobox.h"
#include "itemdef.h"
#include "movedef.h"

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
    QComboBox*         pocketComboBox;
    QSpinBox*          priceSpinBox;
    QComboBox*         fieldUseComboBox;
    QComboBox*         battleUseComboBox;
    QSpinBox*          sellPriceSpinBox;
    QComboBox*         moveComboBox;
    CheckableComboBox* flagsComboBox;
    QTextEdit*         descriptionTextEdit;

    QString spritesDirectory;
};

#endif // ITEMSWIDGET_H
