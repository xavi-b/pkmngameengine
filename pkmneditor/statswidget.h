#ifndef STATSWIDGET_H
#define STATSWIDGET_H

#include "pkmndef.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QWidget>

class StatsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatsWidget(QWidget* parent = nullptr);

    std::map<PkmnDef::Stat, size_t> getStats();
    void                            setStats(std::map<PkmnDef::Stat, size_t> newStats);

signals:
    void statsChanged();

private:
    static QString StatToLabel(PkmnDef::Stat e);

    std::map<PkmnDef::Stat, QSpinBox*> widgets;
};

#endif // STATSWIDGET_H
