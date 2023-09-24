#ifndef ENCOUNTERSMODEL_H
#define ENCOUNTERSMODEL_H

#include "encounter.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class SortEncountersModel : public QSortFilterProxyModel
{
public:
    explicit SortEncountersModel(QObject* parent = nullptr);

protected:
    bool lessThan(QModelIndex const& source_left, QModelIndex const& source_right) const override;
};

class EncountersModel : public QAbstractListModel
{
public:
    explicit EncountersModel(QObject* parent = nullptr);

    virtual int      rowCount(QModelIndex const& parent) const override;
    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
    virtual bool     removeRows(int row, int count, QModelIndex const& parent = QModelIndex()) override;

    void                   addEncounter(Encounter const& newEncounter);
    std::vector<Encounter> getEncounters() const;
    void                   setEncounters(std::vector<Encounter> const& newEncounters);

private:
    std::vector<Encounter> encounters;
};

#endif // ENCOUNTERSMODEL_H
