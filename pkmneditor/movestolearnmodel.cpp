#include "movestolearnmodel.h"

SortMovesToLearnModel::SortMovesToLearnModel(QObject* parent) : QSortFilterProxyModel(parent)
{
}

bool SortMovesToLearnModel::lessThan(QModelIndex const& source_left, QModelIndex const& source_right) const
{
    if (source_left.data(Qt::UserRole).toUInt() > source_right.data(Qt::UserRole).toUInt())
        return true;
    if (source_left.data(Qt::UserRole).toUInt() < source_right.data(Qt::UserRole).toUInt())
        return false;

    int compare = source_left.data(Qt::UserRole + 1)
                      .toString()
                      .compare(source_right.data(Qt::UserRole + 1).toString(), Qt::CaseInsensitive);
    if (compare > 0)
        return true;
    if (compare < 0)
        return false;

    return false;
}

MovesToLearnModel::MovesToLearnModel(QObject* parent) : QAbstractListModel(parent)
{
}

int MovesToLearnModel::rowCount(QModelIndex const& parent) const
{
    Q_UNUSED(parent)
    return movesToLearn.size();
}

QVariant MovesToLearnModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
        return {};

    auto const& e = movesToLearn[index.row()];

    if (role == Qt::DisplayRole)
        return QString("%1\t%2").arg(e.level).arg(e.id.c_str());
    if (role == Qt::UserRole)
        return QVariant::fromValue(e.level);
    if (role == Qt::UserRole + 1)
        return QVariant::fromValue(QString(e.id.c_str()));

    return {};
}

bool MovesToLearnModel::removeRows(int row, int count, QModelIndex const& parent)
{
    Q_UNUSED(parent)
    beginRemoveRows(parent, row, row + count);
    movesToLearn.erase(movesToLearn.begin() + row, movesToLearn.begin() + row + count);
    endRemoveRows();
    return true;
}

void MovesToLearnModel::addMoveToLearn(PkmnDef::MoveToLearn const& newMoveToLearn)
{
    beginInsertRows(QModelIndex(), movesToLearn.size(), movesToLearn.size());
    movesToLearn.push_back(newMoveToLearn);
    endInsertRows();
}

std::vector<PkmnDef::MoveToLearn> MovesToLearnModel::getMovesToLearn() const
{
    return movesToLearn;
}

void MovesToLearnModel::setMovesToLearn(std::vector<PkmnDef::MoveToLearn> const& newMovesToLearn)
{
    beginResetModel();
    movesToLearn = newMovesToLearn;
    endResetModel();
}
