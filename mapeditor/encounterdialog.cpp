#include "encounterdialog.h"

EncounterDialog::EncounterDialog(QWidget* parent) : QDialog(parent)
{
    QFormLayout* formLayout = new QFormLayout;

    pkmnIdLineEdit = new QLineEdit;
    formLayout->addRow(tr("Pkmn ID"), pkmnIdLineEdit);
    percentageSpinBox = new QSpinBox;
    percentageSpinBox->setRange(0, 100);
    formLayout->addRow(tr("Percentage"), percentageSpinBox);
    levelMinSpinBox = new QSpinBox;
    levelMinSpinBox->setRange(1, 100);
    formLayout->addRow(tr("Level min"), levelMinSpinBox);
    levelMaxSpinBox = new QSpinBox;
    levelMaxSpinBox->setRange(1, 100);
    formLayout->addRow(tr("Level max"), levelMaxSpinBox);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    formLayout->addWidget(buttonBox);

    setLayout(formLayout);
}

Encounter EncounterDialog::getEncounter() const
{
    Encounter e;
    e.setPkmnId(pkmnIdLineEdit->text().toStdString());
    e.setPercentage(percentageSpinBox->value());
    e.setLevelMin(levelMinSpinBox->value());
    e.setLevelMax(levelMaxSpinBox->value());
    return e;
}
