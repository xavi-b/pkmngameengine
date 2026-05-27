#include "pkmnwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

PkmnWidget::PkmnWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    QTabWidget* tabWidget = new QTabWidget;

    QWidget*     generalTab  = new QWidget;
    QFormLayout* generalForm = new QFormLayout(generalTab);

    idLineEdit = new QLineEdit;
    connect(idLineEdit, &QLineEdit::textEdited, this, [=]() {
        pkmn->setId(idLineEdit->text().toStdString());
        emit idChanged();
    });
    generalForm->addRow(tr("ID"), idLineEdit);

    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, &QLineEdit::textEdited, this, [=]() {
        pkmn->setName(nameLineEdit->text().toStdString());
    });
    generalForm->addRow(tr("Name"), nameLineEdit);

    sprite = new QLabel;
    generalForm->addRow(tr("Sprite"), sprite);

    baseStatsWidget = new StatsWidget;
    connect(baseStatsWidget, &StatsWidget::statsChanged, this, [=]() {
        pkmn->setBaseStats(baseStatsWidget->getStats());
    });
    generalForm->addRow(tr("Base Stats"), baseStatsWidget);

    EVsToLearnWidget = new StatsWidget;
    connect(EVsToLearnWidget, &StatsWidget::statsChanged, this, [=]() {
        pkmn->setEVsToLearn(EVsToLearnWidget->getStats());
    });
    generalForm->addRow(tr("EVs to learn"), EVsToLearnWidget);

    growthRateComboBox = new QComboBox;
    for (size_t i = 0; i < PkmnDef::GrowthRateCount; ++i)
        growthRateComboBox->addItem(PkmnDef::GrowthRateToString(static_cast<PkmnDef::GrowthRate>(i)).c_str());
    connect(growthRateComboBox, &QComboBox::currentIndexChanged, this, [=](int index) {
        pkmn->setGrowthRate(static_cast<PkmnDef::GrowthRate>(index));
    });
    generalForm->addRow(tr("Growth Rate"), growthRateComboBox);

    baseExpSpinBox = new QSpinBox;
    baseExpSpinBox->setMinimum(0);
    baseExpSpinBox->setMaximum(500);
    connect(baseExpSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        pkmn->setBaseExp(value);
    });
    generalForm->addRow(tr("Base EXP"), baseExpSpinBox);

    catchRateSpinBox = new QSpinBox;
    catchRateSpinBox->setMinimum(0);
    catchRateSpinBox->setMaximum(100);
    connect(catchRateSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        pkmn->setCatchRate(value);
    });
    generalForm->addRow(tr("Catch Rate"), catchRateSpinBox);

    happinessSpinBox = new QSpinBox;
    happinessSpinBox->setMinimum(0);
    happinessSpinBox->setMaximum(255);
    connect(happinessSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        pkmn->setHappiness(value);
    });
    generalForm->addRow(tr("Base Happiness"), happinessSpinBox);

    tabWidget->addTab(generalTab, tr("General"));

    QWidget*     learnsetTab    = new QWidget;
    QHBoxLayout* learnsetLayout = new QHBoxLayout(learnsetTab);

    QWidget*     leftColumn = new QWidget;
    QFormLayout* leftForm   = new QFormLayout(leftColumn);

    abilitiesWidget = new AbilityIdsWidget;
    connect(abilitiesWidget, &AbilityIdsWidget::abilityIdsChanged, this, [=]() {
        pkmn->setAbilities(abilitiesWidget->getAbilityIds());
    });
    leftForm->addRow(tr("Abilities"), abilitiesWidget);

    hiddenAbilitiesWidget = new AbilityIdsWidget;
    connect(hiddenAbilitiesWidget, &AbilityIdsWidget::abilityIdsChanged, this, [=]() {
        pkmn->setHiddenAbilities(hiddenAbilitiesWidget->getAbilityIds());
    });
    leftForm->addRow(tr("Hidden abilities"), hiddenAbilitiesWidget);

    QWidget*     rightColumn = new QWidget;
    QFormLayout* rightForm   = new QFormLayout(rightColumn);

    movesToLearnWidget = new MovesToLearnWidget;
    connect(movesToLearnWidget, &MovesToLearnWidget::movesToLearnChanged, this, [=]() {
        pkmn->setMovesToLearn(movesToLearnWidget->getMovesToLearn());
    });
    rightForm->addRow(tr("Moves to learn"), movesToLearnWidget);

    evolutionsWidget = new EvolutionsWidget;
    connect(evolutionsWidget, &EvolutionsWidget::evolutionsChanged, this, [=]() {
        pkmn->setEvolutions(evolutionsWidget->getEvolutions());
    });
    rightForm->addRow(tr("Evolutions"), evolutionsWidget);

    learnsetLayout->addWidget(leftColumn, 1);
    learnsetLayout->addWidget(rightColumn, 1);

    tabWidget->addTab(learnsetTab, tr("Learnset"));

    layout->addWidget(tabWidget);
    setLayout(layout);
}

void PkmnWidget::setPkmn(PkmnDef::PkmnDefPtr const& newPkmn)
{
    pkmn = newPkmn;
    idLineEdit->setText(newPkmn->getId().c_str());
    nameLineEdit->setText(newPkmn->getName().c_str());
    sprite->setPixmap(QPixmap(
        QDir(spritesDirectory).absoluteFilePath(QString("Pokemon/Front/%1.png").arg(newPkmn->getId().c_str()))));
    baseExpSpinBox->setValue(newPkmn->getBaseExp());
    catchRateSpinBox->setValue(newPkmn->getCatchRate());
    happinessSpinBox->setValue(newPkmn->getHappiness());
    growthRateComboBox->setCurrentIndex(newPkmn->getGrowthRate());
    baseStatsWidget->setStats(newPkmn->getBaseStats());
    EVsToLearnWidget->setStats(newPkmn->getEVsToLearn());

    abilitiesWidget->setAbilityIds(newPkmn->getAbilities());
    hiddenAbilitiesWidget->setAbilityIds(newPkmn->getHiddenAbilities());

    movesToLearnWidget->setMovesToLearn(newPkmn->getMovesToLearn());
    evolutionsWidget->setEvolutions(newPkmn->getEvolutions());
}

void PkmnWidget::setAvailableAbilities(std::vector<Ability::AbilityPtr> const& abilities)
{
    availableAbilityIds.clear();
    for (auto const& ability : abilities)
        availableAbilityIds.append(ability->getId().c_str());
    abilitiesWidget->setAvailableIds(availableAbilityIds);
    hiddenAbilitiesWidget->setAvailableIds(availableAbilityIds);
}

void PkmnWidget::setAvailableMoves(std::vector<MoveDef::MoveDefPtr> const& moves)
{
    QStringList moveIds;
    for (auto const& move : moves)
        moveIds.append(move->getId().c_str());
    movesToLearnWidget->setAvailableMoveIds(moveIds);
}

void PkmnWidget::setAvailablePkmns(std::vector<PkmnDef::PkmnDefPtr> const& pkmns)
{
    QStringList pkmnIds;
    for (auto const& pkmnDef : pkmns)
        pkmnIds.append(pkmnDef->getId().c_str());
    evolutionsWidget->setAvailablePkmnIds(pkmnIds);
}

void PkmnWidget::setSpritesDirectory(QString const& dirName)
{
    spritesDirectory = dirName;
    if (pkmn)
        sprite->setPixmap(QPixmap(
            QDir(spritesDirectory).absoluteFilePath(QString("Pokemon/Front/%1.png").arg(pkmn->getId().c_str()))));
}
