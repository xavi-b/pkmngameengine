#include "encounterseditor.h"

EncountersEditor::EncountersEditor(QWidget* parent) : QWidget(parent)
{
    QStringListModel* encounterMethodsModel = new QStringListModel;
    QStringList       encounterMethods;
    for (size_t i = EncounterMethod::Type::LAND; i < EncounterMethod::TypeCount; ++i)
        encounterMethods.append(
            EncounterMethod::TypeToString(static_cast<EncounterMethod::Type>(i)).c_str());
    encounterMethodsModel->setStringList(encounterMethods);

    EncountersModel*     encountersModel = new EncountersModel;
    SortEncountersModel* proxyModel      = new SortEncountersModel;
    proxyModel->sort(0, Qt::DescendingOrder);
    proxyModel->setSourceModel(encountersModel);

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setEnabled(false);
    encounterMethodsListView = new QListView;
    encounterMethodsListView->setModel(encounterMethodsModel);
    QSpinBox* densitySpinBox = new QSpinBox;
    densitySpinBox->setMinimum(0);
    densitySpinBox->setMaximum(100);
    connect(densitySpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        QModelIndex const& index = encounterMethodsListView->selectionModel()->currentIndex();
        if (index.isValid())
        {
            auto type            = static_cast<EncounterMethod::Type>(index.row());
            auto encounterMethod = currentEncounterMethod(type);
            if (encounterMethod)
                encounterMethod->setDensity(value);
        }
    });

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* label = new QLabel(tr("Encounters"));
    layout->addWidget(label);

    QHBoxLayout* hLayout = new QHBoxLayout;
    connect(encounterMethodsListView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            [=](QModelIndex const& index) {
                auto type            = static_cast<EncounterMethod::Type>(index.row());
                auto encounterMethod = currentEncounterMethod(type);
                if (encounterMethod)
                {
                    vLayout->setEnabled(true);
                    densitySpinBox->setEnabled(encounterMethod->getType() == EncounterMethod::Type::LAND
                                               || encounterMethod->getType() == EncounterMethod::Type::LAND_NIGHT
                                               || encounterMethod->getType() == EncounterMethod::Type::LAND_MORNING
                                               || encounterMethod->getType() == EncounterMethod::Type::CAVE
                                               || encounterMethod->getType() == EncounterMethod::Type::WATER);
                    densitySpinBox->setValue(densitySpinBox->isEnabled() ? encounterMethod->getDensity() : 0);
                    encountersModel->setEncounters(encounterMethod->getEncounters());
                }
            });
    hLayout->addWidget(encounterMethodsListView);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow(tr("Density"), densitySpinBox);
    vLayout->addLayout(formLayout);
    QListView* encountersListView = new QListView;
    encountersListView->setModel(proxyModel);
    vLayout->addWidget(encountersListView, 1);
    QPushButton* addButton = new QPushButton(tr("Add"));
    connect(addButton, &QPushButton::clicked, this, [=]() {
        EncounterDialog d(this);
        if (d.exec() == QDialog::Accepted)
        {
            QModelIndex const& index = encounterMethodsListView->selectionModel()->currentIndex();
            if (index.isValid())
            {
                encountersModel->addEncounter(d.getEncounter());
                auto type            = static_cast<EncounterMethod::Type>(index.row());
                auto encounterMethod = currentEncounterMethod(type);
                if (encounterMethod)
                    encounterMethod->setEncounters(encountersModel->getEncounters());
            }
        }
    });
    vLayout->addWidget(addButton);
    QPushButton* removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        QModelIndex const& encountersIndex = encountersListView->selectionModel()->currentIndex();
        if (encountersIndex.isValid())
        {
            proxyModel->removeRow(encountersIndex.row());
            QModelIndex const& index = encounterMethodsListView->selectionModel()->currentIndex();
            if (index.isValid())
            {
                auto type            = static_cast<EncounterMethod::Type>(index.row());
                auto encounterMethod = currentEncounterMethod(type);
                if (encounterMethod)
                    encounterMethod->setEncounters(encountersModel->getEncounters());
            }
        }
    });
    vLayout->addWidget(removeButton);

    hLayout->addLayout(vLayout);

    layout->addLayout(hLayout);

    setLayout(layout);
}

std::vector<EncounterMethod> EncountersEditor::getEncounterMethods() const
{
    return encounterMethods;
}

void EncountersEditor::setEncounterMethods(std::vector<EncounterMethod> const& newEncounterMethods)
{
    encounterMethods = newEncounterMethods;
}

EncounterMethod* EncountersEditor::currentEncounterMethod(EncounterMethod::Type type)
{
    auto it = std::find_if(encounterMethods.begin(), encounterMethods.end(), [=](EncounterMethod const& e) {
        return e.getType() == type;
    });
    if (it != encounterMethods.end())
        return &(*it);
    encounterMethods.emplace_back(type);
    return &encounterMethods.back();
}
