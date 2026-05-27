#include "evolutiondialog.h"

#include "evolutionsmodel.h"
#include "restrictedidcombobox.h"

EvolutionDialog::EvolutionDialog(QStringList const& availablePkmnIds, QStringList const& excludeIds, QWidget* parent)
    : QDialog(parent)
{
    intValidator = new QIntValidator(this);
    intValidator->setRange(0, 100);

    QFormLayout* formLayout = new QFormLayout;

    pkmnIdComboBox = new QComboBox;
    configureRestrictedIdComboBox(pkmnIdComboBox, availablePkmnIds, excludeIds);
    formLayout->addRow(tr("Pkmn"), pkmnIdComboBox);

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
    wireRestrictedIdComboBox(pkmnIdComboBox, buttonBox, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    formLayout->addWidget(buttonBox);

    setLayout(formLayout);

    onTypeChanged();
}

std::pair<PkmnDef::EvolutionType, PkmnDef::Evolution> EvolutionDialog::getEvolution() const
{
    PkmnDef::Evolution e;
    e.pkmnId  = pkmnIdComboBox->currentText().trimmed().toStdString();
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
