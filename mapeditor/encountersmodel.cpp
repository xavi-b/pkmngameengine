#include "encountersmodel.h"

SortEncountersModel::SortEncountersModel(QObject* parent) : QSortFilterProxyModel(parent)
{
}

bool SortEncountersModel::lessThan(QModelIndex const& source_left, QModelIndex const& source_right) const
{
    if (source_left.data(Qt::UserRole).toUInt() < source_right.data(Qt::UserRole).toUInt())
        return true;
    if (source_left.data(Qt::UserRole).toUInt() > source_right.data(Qt::UserRole).toUInt())
        return false;

    int compare = source_left.data(Qt::UserRole + 1)
                      .toString()
                      .compare(source_right.data(Qt::UserRole + 1).toString(), Qt::CaseInsensitive);
    if (compare > 0)
        return true;
    if (compare < 0)
        return false;

    if (source_left.data(Qt::UserRole + 2).toUInt() < source_right.data(Qt::UserRole + 2).toUInt())
        return true;
    if (source_left.data(Qt::UserRole + 2).toUInt() > source_right.data(Qt::UserRole + 2).toUInt())
        return false;

    if (source_left.data(Qt::UserRole + 3).toUInt() < source_right.data(Qt::UserRole + 3).toUInt())
        return true;
    if (source_left.data(Qt::UserRole + 3).toUInt() > source_right.data(Qt::UserRole + 3).toUInt())
        return false;

    return false;
}

EncountersModel::EncountersModel(QObject* parent) : QAbstractListModel(parent)
{
}

int EncountersModel::rowCount(QModelIndex const& parent) const
{
    Q_UNUSED(parent)
    return encounters.size();
}

QVariant EncountersModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
        return {};

    auto const& e = encounters[index.row()];

    if (role == Qt::DisplayRole)
        return QString("%1%\t%2 : %3 - %4")
            .arg(e.getPercentage())
            .arg(e.getPkmnId().c_str())
            .arg(e.getLevelMin())
            .arg(e.getLevelMax());
    if (role == Qt::UserRole)
        return QVariant::fromValue(e.getPercentage());
    if (role == Qt::UserRole + 1)
        return QVariant::fromValue(QString(e.getPkmnId().c_str()));
    if (role == Qt::UserRole + 2)
        return QVariant::fromValue(e.getLevelMin());
    if (role == Qt::UserRole + 3)
        return QVariant::fromValue(e.getLevelMax());

    return {};
}

bool EncountersModel::removeRows(int row, int count, QModelIndex const& parent)
{
    Q_UNUSED(parent)
    beginRemoveRows(parent, row, row + count);
    encounters.erase(encounters.begin() + row, encounters.begin() + row + count);
    endRemoveRows();
    return true;
}

void EncountersModel::addEncounter(Encounter const& newEncounter)
{
    beginInsertRows(QModelIndex(), encounters.size(), encounters.size());
    encounters.push_back(newEncounter);
    endInsertRows();
}

std::vector<Encounter> EncountersModel::getEncounters() const
{
    return encounters;
}

void EncountersModel::setEncounters(std::vector<Encounter> const& newEncounters)
{
    beginResetModel();
    encounters = newEncounters;
    endResetModel();
}
