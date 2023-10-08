#include "pkmnwidget.h"

PkmnWidget::PkmnWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;

    idLineEdit = new QLineEdit;
    connect(idLineEdit, &QLineEdit::textEdited, this, [=]() {
        pkmn->setId(idLineEdit->text().toStdString());
        emit idChanged();
    });
    formLayout->addRow(tr("ID"), idLineEdit);
    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, &QLineEdit::textEdited, this, [=]() {
        pkmn->setId(nameLineEdit->text().toStdString());
    });
    formLayout->addRow(tr("Name"), nameLineEdit);
    sprite = new QLabel;
    formLayout->addRow(tr("Sprite"), sprite);

    baseStatsWidget = new StatsWidget;
    connect(baseStatsWidget, &StatsWidget::statsChanged, this, [=]() {
        pkmn->setBaseStats(baseStatsWidget->getStats());
    });
    formLayout->addRow(tr("Base Stats"), baseStatsWidget);

    EVsToLearnWidget = new StatsWidget;
    connect(EVsToLearnWidget, &StatsWidget::statsChanged, this, [=]() {
        pkmn->setEVsToLearn(EVsToLearnWidget->getStats());
    });
    formLayout->addRow(tr("EVs to learn"), EVsToLearnWidget);

    growthRateComboBox = new QComboBox;
    for (size_t i = 0; i < PkmnDef::GrowthRateCount; ++i)
        growthRateComboBox->addItem(PkmnDef::GrowthRateToString(static_cast<PkmnDef::GrowthRate>(i)).c_str());
    connect(growthRateComboBox, &QComboBox::currentIndexChanged, this, [=](int index) {
        pkmn->setGrowthRate(static_cast<PkmnDef::GrowthRate>(index));
    });
    formLayout->addRow(tr("Growth Rate"), growthRateComboBox);

    baseExpSpinBox = new QSpinBox;
    baseExpSpinBox->setMinimum(0);
    baseExpSpinBox->setMaximum(500);
    connect(baseExpSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        pkmn->setBaseExp(value);
    });
    formLayout->addRow(tr("Base EXP"), baseExpSpinBox);

    catchRateSpinBox = new QSpinBox;
    catchRateSpinBox->setMinimum(0);
    catchRateSpinBox->setMaximum(100);
    connect(catchRateSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        pkmn->setCatchRate(value);
    });
    formLayout->addRow(tr("Catch Rate"), catchRateSpinBox);

    happinessSpinBox = new QSpinBox;
    happinessSpinBox->setMinimum(0);
    happinessSpinBox->setMaximum(255);
    connect(happinessSpinBox, &QSpinBox::valueChanged, this, [=](int value) {
        pkmn->setHappiness(value);
    });
    formLayout->addRow(tr("Base Happiness"), happinessSpinBox);

    movesToLearnWidget = new MovesToLearnWidget;
    connect(movesToLearnWidget, &MovesToLearnWidget::movesToLearnChanged, this, [=]() {
        pkmn->setMovesToLearn(movesToLearnWidget->getMovesToLearn());
    });
    formLayout->addRow(tr("Moves to learn"), movesToLearnWidget);

    layout->addLayout(formLayout);

    setLayout(layout);
}

void PkmnWidget::setPkmn(PkmnDef::PkmnDefPtr const& newPkmn)
{
    pkmn = newPkmn;
    idLineEdit->setText(newPkmn->getId().c_str());
    nameLineEdit->setText(newPkmn->getName().c_str());
    sprite->setPixmap(QPixmap(QDir(spritesDirectory).absoluteFilePath(QString("Front/%1").arg(pkmn->getId().c_str()))));
    baseExpSpinBox->setValue(newPkmn->getBaseExp());
    catchRateSpinBox->setValue(newPkmn->getCatchRate());
    happinessSpinBox->setValue(newPkmn->getHappiness());
    growthRateComboBox->setCurrentIndex(newPkmn->getGrowthRate());
    baseStatsWidget->setStats(newPkmn->getBaseStats());
    EVsToLearnWidget->setStats(newPkmn->getEVsToLearn());
    movesToLearnWidget->setMovesToLearn(newPkmn->getMovesToLearn());
}

void PkmnWidget::setSpritesDirectory(QString const& dirName)
{
    spritesDirectory = dirName;
    if (pkmn)
        sprite->setPixmap(
            QPixmap(QDir(spritesDirectory).absoluteFilePath(QString("Front/%1").arg(pkmn->getId().c_str()))));
}
