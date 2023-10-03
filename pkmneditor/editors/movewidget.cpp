#include "movewidget.h"

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
        move->setId(nameLineEdit->text().toStdString());
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
        move->setCategory(static_cast<MoveDef::Category>(index));
    });
    formLayout->addRow(tr("Category"), categoryComboBox);

    powerSpinBox = new QSpinBox;
    powerSpinBox->setMinimum(0);
    powerSpinBox->setMaximum(200);
    connect(powerSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        move->setPower(value);
    });
    formLayout->addRow(tr("Power"), powerSpinBox);

    accuracySpinBox = new QSpinBox;
    accuracySpinBox->setMinimum(0);
    accuracySpinBox->setMaximum(100);
    connect(accuracySpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        move->setAccuracy(value);
    });
    formLayout->addRow(tr("Accuracy"), accuracySpinBox);

    totalPPSpinBox = new QSpinBox;
    totalPPSpinBox->setMinimum(0);
    totalPPSpinBox->setMaximum(100);
    connect(totalPPSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        move->setTotalPP(value);
    });
    formLayout->addRow(tr("PP"), totalPPSpinBox);

    layout->addLayout(formLayout);

    setLayout(layout);
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
}
