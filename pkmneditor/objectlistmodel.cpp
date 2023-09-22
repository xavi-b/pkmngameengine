#include "objectlistmodel.h"

ObjectListModel::ObjectListModel(QObject* parent) : QAbstractListModel(parent)
{
}

int ObjectListModel::rowCount(QModelIndex const& /*parent*/) const
{
    return items.size();
}

QVariant ObjectListModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    return getItem(index)->data(role);
}

ObjectListItem* ObjectListModel::getItem(QModelIndex const& index) const
{
    return items.at(index.row()).get();
}
