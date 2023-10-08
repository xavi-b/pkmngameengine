#include "evolutionsmodel.h"

SortEvolutionsModel::SortEvolutionsModel(QObject* parent) : QSortFilterProxyModel(parent)
{
}

bool SortEvolutionsModel::lessThan(QModelIndex const& source_left, QModelIndex const& source_right) const
{
    static QList<PkmnDef::EvolutionType> levelTypes =
        QList<PkmnDef::EvolutionType>::fromReadOnlyData(EvolutionsModel::LevelTypes);

    PkmnDef::EvolutionType type_left = static_cast<PkmnDef::EvolutionType>(source_left.data(Qt::UserRole + 1).toUInt());
    PkmnDef::EvolutionType type_right =
        static_cast<PkmnDef::EvolutionType>(source_right.data(Qt::UserRole + 1).toUInt());
    if (levelTypes.contains(type_left) && levelTypes.contains(type_right))
    {
        if (source_left.data(Qt::UserRole + 2).toUInt() > source_right.data(Qt::UserRole + 2).toUInt())
            return true;
        if (source_left.data(Qt::UserRole + 2).toUInt() < source_right.data(Qt::UserRole + 2).toUInt())
            return false;
    }
    else
    {
        if (source_left.data(Qt::UserRole + 2).toString() > source_right.data(Qt::UserRole + 2).toString())
            return true;
        if (source_left.data(Qt::UserRole + 2).toString() < source_right.data(Qt::UserRole + 2).toString())
            return false;
    }

    if (source_left.data(Qt::UserRole + 1).toString() > source_right.data(Qt::UserRole + 1).toString())
        return true;
    if (source_left.data(Qt::UserRole + 1).toString() < source_right.data(Qt::UserRole + 1).toString())
        return false;

    if (source_left.data(Qt::UserRole).toString() > source_right.data(Qt::UserRole).toString())
        return true;
    if (source_left.data(Qt::UserRole).toString() < source_right.data(Qt::UserRole).toString())
        return false;

    return false;
}

EvolutionsModel::EvolutionsModel(QObject* parent) : QAbstractListModel(parent)
{
}

int EvolutionsModel::rowCount(QModelIndex const& parent) const
{
    Q_UNUSED(parent)
    return evolutions.size();
}

QVariant EvolutionsModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
        return {};

    auto const& e = evolutions[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0:
            return QVariant::fromValue(e.second.pkmnId.c_str());
        case 1:
            return QVariant::fromValue(PkmnDef::EvolutionTypeToString(e.first).c_str());
        case 2:
            return QVariant::fromValue(e.second.data);
        default:
            return QVariant();
        }
    }
    if (role == Qt::UserRole)
        return QVariant::fromValue(e.second.pkmnId);
    if (role == Qt::UserRole + 1)
        return QVariant::fromValue(PkmnDef::EvolutionTypeToString(e.first).c_str());
    if (role == Qt::UserRole + 2)
        return QVariant::fromValue(e.second.data);

    return {};
}

bool EvolutionsModel::removeRows(int row, int count, QModelIndex const& parent)
{
    Q_UNUSED(parent)
    beginRemoveRows(parent, row, row + count);
    evolutions.erase(evolutions.begin() + row, evolutions.begin() + row + count);
    endRemoveRows();
    return true;
}

void EvolutionsModel::addEvolution(std::pair<PkmnDef::EvolutionType, PkmnDef::Evolution> const& newEvolution)
{
    beginInsertRows(QModelIndex(), evolutions.size(), evolutions.size());
    evolutions.push_back(newEvolution);
    endInsertRows();
}

std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> EvolutionsModel::getEvolutions() const
{
    std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> newEvolutions;
    for (auto const& e : evolutions)
        newEvolutions.emplace(e);
    return newEvolutions;
}

void EvolutionsModel::setEvolutions(std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> const& newEvolutions)
{
    beginResetModel();
    evolutions.clear();
    std::transform(newEvolutions.begin(), newEvolutions.end(), std::back_inserter(evolutions), [](auto const& element) {
        return element;
    });
    endResetModel();
}
