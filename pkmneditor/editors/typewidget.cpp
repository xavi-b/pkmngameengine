#include "typewidget.h"

TypeWidget::TypeWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;

    idLineEdit = new QLineEdit;
    connect(idLineEdit, &QLineEdit::textEdited, this, [=]() {
        type->setId(idLineEdit->text().toStdString());
        emit idChanged();
    });
    formLayout->addRow(tr("ID"), idLineEdit);
    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, &QLineEdit::textEdited, this, [=]() {
        type->setId(nameLineEdit->text().toStdString());
    });
    formLayout->addRow(tr("Name"), nameLineEdit);

    layout->addLayout(formLayout);

    layout->addWidget(new QLabel("Weaknesses"));
    weaknessesSelector = new IdsSelector;
    layout->addWidget(weaknessesSelector);

    layout->addWidget(new QLabel("Resistances"));
    resistancesSelector = new IdsSelector;
    layout->addWidget(resistancesSelector);

    layout->addWidget(new QLabel("Immunities"));
    immunitiesSelector = new IdsSelector;
    layout->addWidget(immunitiesSelector);

    setLayout(layout);
}

void TypeWidget::setAvailableTypes(std::vector<Type::TypePtr> const& types)
{
    QStringList ids;
    for (auto const& t : types)
        ids.append(t->getId().c_str());
    weaknessesSelector->setAvailableIds(ids);
    resistancesSelector->setAvailableIds(ids);
    immunitiesSelector->setAvailableIds(ids);
}

void TypeWidget::setType(Type::TypePtr const& newType)
{
    type = newType;
    idLineEdit->setText(newType->getId().c_str());
    nameLineEdit->setText(newType->getName().c_str());
    QStringList weaknessesIds;
    for (auto const& i : newType->getWeaknesses())
        weaknessesIds.append(i.c_str());
    weaknessesSelector->setSelectedIds(weaknessesIds);
    QStringList resistancesIds;
    for (auto const& i : newType->getResistances())
        resistancesIds.append(i.c_str());
    resistancesSelector->setSelectedIds(resistancesIds);
    QStringList immunitiesIds;
    for (auto const& i : newType->getImmunities())
        immunitiesIds.append(i.c_str());
    immunitiesSelector->setSelectedIds(immunitiesIds);
}
