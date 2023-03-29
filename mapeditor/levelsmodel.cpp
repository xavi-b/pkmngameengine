#include "levelsmodel.h"

LevelsModel::LevelsModel(QObject* parent) : QAbstractListModel{parent}
{
}

int LevelsModel::rowCount(const QModelIndex& parent) const
{
    if (!map)
        return 0;

    return map->getLevels().size();
}

QVariant LevelsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    if (role == Qt::DisplayRole)
        return index.row();
    if (role == Qt::UserRole)
        return map->getLevels()[index.row()]->isVisible();

    return {};
}

bool LevelsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::UserRole)
    {
        map->setLevelVisible(index.row(), value.toBool());
        return true;
    }

    return false;
}

Qt::ItemFlags LevelsModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags(index);
    flags               = flags & ~Qt::ItemIsSelectable;
    return flags;
}

void LevelsModel::setLevelsReference(MapperWidget* map)
{
    this->map = map;

    connect(map, &MapperWidget::levelAdded, this, [=]() {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        endInsertRows();
    });
    connect(map, &MapperWidget::levelRemoved, this, [=](int index) {
        beginRemoveRows(QModelIndex(), index, index);
        endRemoveRows();
    });
    connect(map, &MapperWidget::levelVisibleChanged, this, [=](int index, bool visible) {
        auto i = this->index(index);
        dataChanged(i, i);
    });
}
