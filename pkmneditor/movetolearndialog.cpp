#include "movetolearndialog.h"

MoveToLearnDialog::MoveToLearnDialog(QWidget* parent) : QDialog(parent)
{
    QFormLayout* formLayout = new QFormLayout;

    moveIdLineEdit = new QLineEdit;
    formLayout->addRow(tr("Pkmn ID"), moveIdLineEdit);
    levelSpinBox = new QSpinBox;
    levelSpinBox->setRange(0, 100);
    formLayout->addRow(tr("Level"), levelSpinBox);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    formLayout->addWidget(buttonBox);

    setLayout(formLayout);
}

PkmnDef::MoveToLearn MoveToLearnDialog::getMoveToLearn() const
{
    PkmnDef::MoveToLearn e;
    e.id    = moveIdLineEdit->text().toStdString();
    e.level = levelSpinBox->value();
    return e;
}
