#ifndef EVOLUTIONSWIDGET_H
#define EVOLUTIONSWIDGET_H

#include "evolutiondialog.h"
#include "evolutionsmodel.h"
#include "pkmndef.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableView>
#include <QWidget>

class EvolutionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EvolutionsWidget(QWidget* parent = nullptr);

    std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> getEvolutions();
    void setEvolutions(std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> newEvolutions);
    void setAvailablePkmnIds(QStringList const& pkmnIds);

signals:
    void evolutionsChanged();

private:
    EvolutionsModel* model;
    QStringList      availablePkmnIds;
};

#endif // EVOLUTIONSWIDGET_H
