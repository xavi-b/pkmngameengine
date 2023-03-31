#include "levelsmodel.h"

LevelsModel::LevelsModel(QObject* parent) : QAbstractListModel{parent}
{
}

int LevelsModel::rowCount(QModelIndex const& parent) const
{
    Q_UNUSED(parent)

    if (!map)
        return 0;

    return map->getLevels().size();
}

QVariant LevelsModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
        return {};

    if (role == Qt::DisplayRole)
        return index.row();
    if (role == Qt::UserRole)
        return map->getLevels()[index.row()]->isVisible();

    return {};
}

bool LevelsModel::setData(QModelIndex const& index, QVariant const& value, int role)
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

Qt::ItemFlags LevelsModel::flags(QModelIndex const& index) const
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
        Q_UNUSED(visible)
        auto i = this->index(index);
        emit dataChanged(i, i);
    });
    connect(map, &MapperWidget::reset, this, [=]() {
        beginResetModel();
        endResetModel();
    });
}
