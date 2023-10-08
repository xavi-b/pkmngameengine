#ifndef EVOLUTIONSMODEL_H
#define EVOLUTIONSMODEL_H

#include "pkmndef.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class SortEvolutionsModel : public QSortFilterProxyModel
{
public:
    explicit SortEvolutionsModel(QObject* parent = nullptr);

protected:
    bool lessThan(QModelIndex const& source_left, QModelIndex const& source_right) const override;
};

class EvolutionsModel : public QAbstractTableModel
{
public:
    static constexpr PkmnDef::EvolutionType LevelTypes[]{PkmnDef::AttackGreater,
                                                         PkmnDef::DefenseGreater,
                                                         PkmnDef::AtkDefEqual,
                                                         PkmnDef::Happiness,
                                                         PkmnDef::HasInParty,
                                                         PkmnDef::HasMove,
                                                         PkmnDef::Level,
                                                         PkmnDef::LevelMale,
                                                         PkmnDef::LevelFemale,
                                                         PkmnDef::HasEmptySlotInParty};

    explicit EvolutionsModel(QObject* parent = nullptr);

    virtual int      columnCount(QModelIndex const& parent) const override;
    virtual int      rowCount(QModelIndex const& parent) const override;
    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
    virtual bool     removeRows(int row, int count, QModelIndex const& parent = QModelIndex()) override;

    void addEvolution(std::pair<PkmnDef::EvolutionType, PkmnDef::Evolution> const& newEvolution);
    std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> getEvolutions() const;
    void setEvolutions(std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> const& newEvolutions);

private:
    std::vector<std::pair<PkmnDef::EvolutionType, PkmnDef::Evolution>> evolutions;
};

#endif // EVOLUTIONSMODEL_H
