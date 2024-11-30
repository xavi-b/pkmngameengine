#include "itemswidget.h"

#include <QDir>

ItemsWidget::ItemsWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;

    idLineEdit = new QLineEdit;
    connect(idLineEdit, &QLineEdit::textEdited, this, [=]() {
        item->setId(idLineEdit->text().toStdString());
        emit idChanged();
    });
    formLayout->addRow(tr("ID"), idLineEdit);
    sprite = new QLabel;
    formLayout->addRow(tr("Sprite"), sprite);
    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, &QLineEdit::textEdited, this, [=]() {
        item->setName(nameLineEdit->text().toStdString());
    });
    formLayout->addRow(tr("Name"), nameLineEdit);
    namePluralLineEdit = new QLineEdit;
    connect(namePluralLineEdit, &QLineEdit::textEdited, this, [=]() {
        item->setNamePlural(namePluralLineEdit->text().toStdString());
    });
    formLayout->addRow(tr("Plural"), namePluralLineEdit);
    pocketComboBox = new QComboBox;
    for (size_t i = 0; i < ItemDef::NumberOfPockets; ++i)
        pocketComboBox->addItem(ItemDef::GetPocketName(i).c_str(), QVariant::fromValue(i));
    connect(pocketComboBox, &QComboBox::currentIndexChanged, this, [=]() {
        item->setPocket(pocketComboBox->currentData().toULongLong());
    });
    formLayout->addRow(tr("Pocket"), pocketComboBox);
    priceSpinBox = new QSpinBox;
    priceSpinBox->setMinimum(0);
    priceSpinBox->setMaximum(999999);
    connect(priceSpinBox, &QSpinBox::valueChanged, this, [=]() {
        item->setPrice(priceSpinBox->value());
    });
    formLayout->addRow(tr("Price"), priceSpinBox);
    sellPriceSpinBox = new QSpinBox;
    sellPriceSpinBox->setMinimum(0);
    sellPriceSpinBox->setMaximum(999999);
    connect(sellPriceSpinBox, &QSpinBox::valueChanged, this, [=]() {
        item->setPrice(sellPriceSpinBox->value());
    });
    formLayout->addRow(tr("SellPrice (0 means default = Price / 2)"), sellPriceSpinBox);
    fieldUseComboBox = new QComboBox;
    for (size_t i = 0; i < ItemDef::FieldUseCount; ++i)
        fieldUseComboBox->addItem(ItemDef::FieldUseToString(static_cast<ItemDef::FieldUse>(i)).c_str(),
                                  QVariant::fromValue(i));
    connect(fieldUseComboBox, &QComboBox::currentIndexChanged, this, [=]() {
        item->setFieldUse(static_cast<ItemDef::FieldUse>(fieldUseComboBox->currentData().toULongLong()));
        moveComboBox->setEnabled(item->getFieldUse() == ItemDef::FieldUse::TR);
    });
    formLayout->addRow(tr("Field Use"), fieldUseComboBox);
    battleUseComboBox = new QComboBox;
    for (size_t i = 0; i < ItemDef::BattleUseCount; ++i)
        battleUseComboBox->addItem(ItemDef::BattleUseToString(static_cast<ItemDef::BattleUse>(i)).c_str(),
                                   QVariant::fromValue(i));
    connect(battleUseComboBox, &QComboBox::currentIndexChanged, this, [=]() {
        item->setBattleUse(static_cast<ItemDef::BattleUse>(battleUseComboBox->currentData().toULongLong()));
    });
    formLayout->addRow(tr("Battle Use"), battleUseComboBox);
    flagsComboBox = new CheckableComboBox;
    for (size_t i = 0; i < ItemDef::FlagCount; ++i)
    {
        ItemDef::Flag flag = static_cast<ItemDef::Flag>(i);
        flagsComboBox->addItem(ItemDef::FlagToString(flag).c_str(), false);
    }
    connect(flagsComboBox->model(), &QAbstractItemModel::dataChanged, this, [=]() {
        if (item)
        {
            std::vector<ItemDef::Flag> flags;
            for (int i = 0; i < flagsComboBox->count(); ++i)
            {
                bool checked = flagsComboBox->itemData(i).toBool();
                if (checked)
                    flags.push_back(static_cast<ItemDef::Flag>(i));
            }
            item->setFlags(flags);
        }
    });
    formLayout->addRow(tr("Flags"), flagsComboBox);
    moveComboBox = new QComboBox;
    connect(moveComboBox, &QComboBox::currentTextChanged, this, [=](QString const& text) {
        if (item)
            item->setMoveId(text.toStdString());
    });
    formLayout->addRow(tr("TM Move"), moveComboBox);
    descriptionTextEdit = new QTextEdit;
    formLayout->addRow(tr("Description"), descriptionTextEdit);

    layout->addLayout(formLayout);

    setLayout(layout);
}

void ItemsWidget::setAvailableItems(std::vector<ItemDef::ItemDefPtr> const& items)
{
    QStringList ids;
    for (auto const& t : items)
        ids.append(t->getId().c_str());
}

void ItemsWidget::setAvailableMoves(std::vector<MoveDef::MoveDefPtr> const& moves)
{
    QString currentId = moveComboBox->currentText();
    moveComboBox->clear();
    QStringList ids;
    for (auto const& t : moves)
        ids.append(t->getId().c_str());
    moveComboBox->addItems(ids);
    moveComboBox->setCurrentText(currentId);
}

void ItemsWidget::setItem(ItemDef::ItemDefPtr const& newItem)
{
    item = newItem;
    idLineEdit->setText(newItem->getId().c_str());
    sprite->setPixmap(
        QPixmap(QDir(spritesDirectory).absoluteFilePath(QString("Items/%1.png").arg(newItem->getId().c_str()))));
    nameLineEdit->setText(newItem->getName().c_str());
    namePluralLineEdit->setText(newItem->getNamePlural().c_str());
    pocketComboBox->setCurrentIndex(newItem->getPocket());
    priceSpinBox->setValue(newItem->getPrice());
    sellPriceSpinBox->setValue(newItem->getSellPrice());
    fieldUseComboBox->setCurrentIndex(static_cast<size_t>(newItem->getFieldUse()));
    battleUseComboBox->setCurrentIndex(static_cast<size_t>(newItem->getBattleUse()));
    moveComboBox->setCurrentText(newItem->getMoveId().c_str());
    for (size_t i = 0; i < ItemDef::FlagCount; ++i)
    {
        QSignalBlocker blocker(flagsComboBox->model());

        auto          flags = newItem->getFlags();
        ItemDef::Flag flag  = static_cast<ItemDef::Flag>(i);
        auto          it    = std::find(flags.begin(), flags.end(), flag);
        bool          found = it != flags.end();
        flagsComboBox->setItemData(i, found);
    }
    flagsComboBox->update();
    descriptionTextEdit->setText(newItem->getDescription().c_str());
}

void ItemsWidget::setSpritesDirectory(QString const& dirName)
{
    spritesDirectory = dirName;
    if (item)
        sprite->setPixmap(
            QPixmap(QDir(spritesDirectory).absoluteFilePath(QString("Items/%1.png").arg(item->getId().c_str()))));
}
