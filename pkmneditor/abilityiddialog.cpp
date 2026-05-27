#include "abilityiddialog.h"

#include "restrictedidcombobox.h"

AbilityIdDialog::AbilityIdDialog(QStringList const& availableIds, QStringList const& excludeIds, QWidget* parent)
    : QDialog(parent)
{
    QFormLayout* formLayout = new QFormLayout;

    abilityComboBox = new QComboBox;
    configureRestrictedIdComboBox(abilityComboBox, availableIds, excludeIds);
    formLayout->addRow(tr("Ability"), abilityComboBox);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    wireRestrictedIdComboBox(abilityComboBox, buttonBox, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    formLayout->addWidget(buttonBox);

    setLayout(formLayout);
}

std::string AbilityIdDialog::getAbilityId() const
{
    return abilityComboBox->currentText().trimmed().toStdString();
}
