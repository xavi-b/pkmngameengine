#include "movetolearndialog.h"

#include "restrictedidcombobox.h"

MoveToLearnDialog::MoveToLearnDialog(QStringList const& availableMoveIds,
                                     QStringList const& excludeIds,
                                     QWidget*           parent)
    : QDialog(parent)
{
    QFormLayout* formLayout = new QFormLayout;

    moveIdComboBox = new QComboBox;
    configureRestrictedIdComboBox(moveIdComboBox, availableMoveIds, excludeIds);
    formLayout->addRow(tr("Move"), moveIdComboBox);

    levelSpinBox = new QSpinBox;
    levelSpinBox->setRange(0, 100);
    formLayout->addRow(tr("Level"), levelSpinBox);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    wireRestrictedIdComboBox(moveIdComboBox, buttonBox, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    formLayout->addWidget(buttonBox);

    setLayout(formLayout);
}

PkmnDef::MoveToLearn MoveToLearnDialog::getMoveToLearn() const
{
    PkmnDef::MoveToLearn e;
    e.id    = moveIdComboBox->currentText().trimmed().toStdString();
    e.level = levelSpinBox->value();
    return e;
}
