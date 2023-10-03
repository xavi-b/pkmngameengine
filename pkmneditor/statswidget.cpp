#include "statswidget.h"

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    for (size_t i = 0; i < PkmnDef::StatCount; ++i)
    {
        PkmnDef::Stat stat    = static_cast<PkmnDef::Stat>(i);
        QLabel*       label   = new QLabel(StatToLabel(stat));
        QSpinBox*     spinBox = new QSpinBox;
        spinBox->setMinimum(0);
        connect(spinBox, &QSpinBox::valueChanged, this, &StatsWidget::statsChanged);
        QVBoxLayout* vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->addWidget(label);
        vLayout->addWidget(spinBox);
        layout->addLayout(vLayout);
        widgets[stat] = spinBox;
    }

    setLayout(layout);
}

std::map<PkmnDef::Stat, size_t> StatsWidget::getStats()
{
    std::map<PkmnDef::Stat, size_t> stats;
    for (size_t i = 0; i < PkmnDef::StatCount; ++i)
    {
        PkmnDef::Stat stat = static_cast<PkmnDef::Stat>(i);
        stats[stat]        = widgets[stat]->value();
    }
    return stats;
}

void StatsWidget::setStats(std::map<PkmnDef::Stat, size_t> newStats)
{
    for (size_t i = 0; i < PkmnDef::StatCount; ++i)
    {
        PkmnDef::Stat stat = static_cast<PkmnDef::Stat>(i);
        widgets[stat]->setValue(newStats[stat]);
    }
}

QString StatsWidget::StatToLabel(PkmnDef::Stat e)
{
    switch (e)
    {
    case PkmnDef::HP:
        return "HP";
    case PkmnDef::ATTACK:
        return "Atk";
    case PkmnDef::DEFENSE:
        return "Def";
    case PkmnDef::SPECIAL_ATTACK:
        return "Sp. Atk";
    case PkmnDef::SPECIAL_DEFENSE:
        return "Sp. Def";
    case PkmnDef::SPEED:
        return "Speed";
    case PkmnDef::__SIZE_STAT:
        return "__SIZE_STAT";
    }
    return "???";
}
