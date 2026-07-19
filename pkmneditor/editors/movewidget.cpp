#include "movewidget.h"

#include <QAbstractItemView>

MoveWidget::MoveWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;

    idLineEdit = new QLineEdit;
    connect(idLineEdit, &QLineEdit::textEdited, this, [=]() {
        move->setId(idLineEdit->text().toStdString());
        emit idChanged();
    });
    formLayout->addRow(tr("ID"), idLineEdit);

    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, &QLineEdit::textEdited, this, [=]() {
        if (move)
            move->setName(nameLineEdit->text().toStdString());
    });
    formLayout->addRow(tr("Name"), nameLineEdit);

    typeComboBox = new QComboBox;
    connect(typeComboBox, &QComboBox::currentTextChanged, this, [=](QString const& text) {
        if (move)
            move->setType(text.toStdString());
    });
    formLayout->addRow(tr("Type"), typeComboBox);

    categoryComboBox = new QComboBox;
    for (size_t i = 0; i < MoveDef::CategoryCount; ++i)
        categoryComboBox->addItem(MoveDef::CategoryToString(static_cast<MoveDef::Category>(i)).c_str());
    connect(categoryComboBox, &QComboBox::currentIndexChanged, this, [=](int index) {
        if (move)
            move->setCategory(static_cast<MoveDef::Category>(index));
    });
    formLayout->addRow(tr("Category"), categoryComboBox);

    powerSpinBox = new QSpinBox;
    powerSpinBox->setMinimum(0);
    powerSpinBox->setMaximum(200);
    connect(powerSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        if (move)
            move->setPower(value);
    });
    formLayout->addRow(tr("Power"), powerSpinBox);

    accuracySpinBox = new QSpinBox;
    accuracySpinBox->setMinimum(0);
    accuracySpinBox->setMaximum(100);
    connect(accuracySpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        if (move)
            move->setAccuracy(value);
    });
    formLayout->addRow(tr("Accuracy"), accuracySpinBox);

    totalPPSpinBox = new QSpinBox;
    totalPPSpinBox->setMinimum(0);
    totalPPSpinBox->setMaximum(100);
    connect(totalPPSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        if (move)
            move->setTotalPP(value);
    });
    formLayout->addRow(tr("PP"), totalPPSpinBox);

    functionCodeComboBox = new QComboBox;
    for (size_t i = 0; i < MoveDef::FunctionCodeCount; ++i)
        functionCodeComboBox->addItem(MoveDef::FunctionCodeToString(static_cast<MoveDef::FunctionCode>(i)).c_str());
    connect(functionCodeComboBox, &QComboBox::currentIndexChanged, this, [=](int index) {
        if (move && index >= 0)
            move->setFunctionCode(static_cast<MoveDef::FunctionCode>(index));
    });
    formLayout->addRow(tr("Function Code"), functionCodeComboBox);

    effectChanceSpinBox = new QSpinBox;
    effectChanceSpinBox->setMinimum(0);
    effectChanceSpinBox->setMaximum(100);
    connect(effectChanceSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        if (move)
            move->setEffectChance(static_cast<size_t>(value));
    });
    formLayout->addRow(tr("Effect Chance"), effectChanceSpinBox);

    targetComboBox = new QComboBox;
    for (size_t i = 0; i < MoveDef::TargetCount; ++i)
        targetComboBox->addItem(MoveDef::TargetToString(static_cast<MoveDef::Target>(i)).c_str());
    connect(targetComboBox, &QComboBox::currentIndexChanged, this, [=](int index) {
        if (move && index >= 0)
            move->setTarget(static_cast<MoveDef::Target>(index));
    });
    formLayout->addRow(tr("Target"), targetComboBox);

    flagsListWidget = new QListWidget;
    flagsListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    for (size_t i = 0; i < MoveDef::FlagCount; ++i)
        flagsListWidget->addItem(MoveDef::FlagToString(static_cast<MoveDef::Flag>(i)).c_str());
    connect(flagsListWidget, &QListWidget::itemSelectionChanged, this, &MoveWidget::syncFlagsFromUi);
    formLayout->addRow(tr("Flags"), flagsListWidget);

    descriptionTextEdit = new QTextEdit;
    descriptionTextEdit->setAcceptRichText(false);
    connect(descriptionTextEdit, &QTextEdit::textChanged, this, [=]() {
        if (move)
            move->setDescription(descriptionTextEdit->toPlainText().toStdString());
    });
    formLayout->addRow(tr("Description"), descriptionTextEdit);

    layout->addLayout(formLayout);
    setLayout(layout);
}

void MoveWidget::syncFlagsFromUi()
{
    if (!move)
        return;
    std::vector<MoveDef::Flag> selected;
    for (int i = 0; i < flagsListWidget->count(); ++i)
    {
        if (flagsListWidget->item(i)->isSelected())
            selected.push_back(static_cast<MoveDef::Flag>(i));
    }
    move->setFlags(selected);
}

void MoveWidget::setAvailableTypes(std::vector<Type::TypePtr> const& types)
{
    QString currentId = typeComboBox->currentText();
    typeComboBox->clear();
    QStringList ids;
    for (auto const& t : types)
        ids.append(t->getId().c_str());
    typeComboBox->addItems(ids);
    typeComboBox->setCurrentText(currentId);
}

void MoveWidget::setMove(MoveDef::MoveDefPtr const& newMove)
{
    move = newMove;
    idLineEdit->setText(newMove->getId().c_str());
    nameLineEdit->setText(newMove->getName().c_str());
    typeComboBox->setCurrentText(newMove->getType().c_str());
    categoryComboBox->setCurrentIndex(newMove->getCategory());
    powerSpinBox->setValue(newMove->getPower());
    accuracySpinBox->setValue(newMove->getAccuracy());
    totalPPSpinBox->setValue(newMove->getTotalPP());
    functionCodeComboBox->setCurrentIndex(static_cast<int>(newMove->getFunctionCode()));
    effectChanceSpinBox->setValue(static_cast<int>(newMove->getEffectChance()));
    targetComboBox->setCurrentIndex(static_cast<int>(newMove->getTarget()));

    flagsListWidget->blockSignals(true);
    flagsListWidget->clearSelection();
    for (auto flag : newMove->getFlags())
    {
        int index = static_cast<int>(flag);
        if (index >= 0 && index < flagsListWidget->count())
            flagsListWidget->item(index)->setSelected(true);
    }
    flagsListWidget->blockSignals(false);

    descriptionTextEdit->blockSignals(true);
    descriptionTextEdit->setPlainText(newMove->getDescription().c_str());
    descriptionTextEdit->blockSignals(false);
}
