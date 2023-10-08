#include "evolutiondialog.h"

#include "evolutionsmodel.h"

EvolutionDialog::EvolutionDialog(QWidget* parent) : QDialog(parent)
{
    intValidator = new QIntValidator(this);
    intValidator->setRange(0, 100);

    QFormLayout* formLayout = new QFormLayout;

    pkmnIdLineEdit = new QLineEdit;
    formLayout->addRow(tr("Pkmn ID"), pkmnIdLineEdit);
    typeComboBox = new QComboBox;
    formLayout->addRow(tr("Type"), typeComboBox);
    for (size_t i = 0; i < PkmnDef::EvolutionTypeCount; ++i)
    {
        typeComboBox->addItem(PkmnDef::EvolutionTypeToString(static_cast<PkmnDef::EvolutionType>(i)).c_str(),
                              QVariant::fromValue(i));
    }
    connect(typeComboBox, &QComboBox::currentIndexChanged, this, &EvolutionDialog::onTypeChanged);
    dataLineEdit = new QLineEdit;
    formLayout->addRow(tr("Data"), dataLineEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    formLayout->addWidget(buttonBox);

    setLayout(formLayout);

    onTypeChanged();
}

std::pair<PkmnDef::EvolutionType, PkmnDef::Evolution> EvolutionDialog::getEvolution() const
{
    PkmnDef::Evolution e;
    e.pkmnId  = pkmnIdLineEdit->text().toStdString();
    e.data    = dataLineEdit->text().toStdString();
    auto type = static_cast<PkmnDef::EvolutionType>(typeComboBox->currentData().toUInt());
    return {type, e};
}

void EvolutionDialog::onTypeChanged()
{
    static QList<PkmnDef::EvolutionType> levelTypes =
        QList<PkmnDef::EvolutionType>::fromReadOnlyData(EvolutionsModel::LevelTypes);
    auto type = static_cast<PkmnDef::EvolutionType>(typeComboBox->currentData().toUInt());
    if (levelTypes.contains(type))
        dataLineEdit->setValidator(intValidator);
    else
        dataLineEdit->setValidator(nullptr);
}
