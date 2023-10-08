#ifndef EVOLUTIONSWIDGET_H
#define EVOLUTIONSWIDGET_H

#include "evolutiondialog.h"
#include "evolutionsmodel.h"
#include "pkmndef.h"

#include <QHBoxLayout>
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

signals:
    void evolutionsChanged();

private:
    EvolutionsModel* model;
};

#endif // EVOLUTIONSWIDGET_H
