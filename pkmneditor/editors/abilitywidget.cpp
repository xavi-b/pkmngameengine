#include "abilitywidget.h"

#include <QSignalBlocker>
#include <QVBoxLayout>

AbilityWidget::AbilityWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout     = new QVBoxLayout;
    QFormLayout* formLayout = new QFormLayout;

    idLineEdit = new QLineEdit;
    connect(idLineEdit, &QLineEdit::textEdited, this, [=]() {
        if (!ability)
            return;
        ability->setId(idLineEdit->text().toStdString());
        emit idChanged();
    });
    formLayout->addRow(tr("ID"), idLineEdit);

    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, &QLineEdit::textEdited, this, [=]() {
        if (ability)
            ability->setName(nameLineEdit->text().toStdString());
    });
    formLayout->addRow(tr("Name"), nameLineEdit);

    descriptionTextEdit = new QTextEdit;
    connect(descriptionTextEdit, &QTextEdit::textChanged, this, [=]() {
        if (ability)
            ability->setDescription(descriptionTextEdit->toPlainText().toStdString());
    });
    formLayout->addRow(tr("Description"), descriptionTextEdit);

    layout->addLayout(formLayout);
    setLayout(layout);
}

void AbilityWidget::setAbility(Ability::AbilityPtr const& newAbility)
{
    QSignalBlocker idBlocker(idLineEdit);
    QSignalBlocker nameBlocker(nameLineEdit);
    QSignalBlocker descriptionBlocker(descriptionTextEdit);

    ability = newAbility;

    if (!ability)
        return;

    idLineEdit->setText(ability->getId().c_str());
    nameLineEdit->setText(ability->getName().c_str());
    descriptionTextEdit->setText(ability->getDescription().c_str());
}
