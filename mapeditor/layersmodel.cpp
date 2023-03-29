#include "layersmodel.h"

LayersModel::LayersModel(QObject* parent) : QAbstractListModel{parent}
{
}

int LayersModel::rowCount(const QModelIndex& parent) const
{
    if (!map)
        return 0;

    return map->getWorkingLevelLayers().size();
}

QVariant LayersModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    if (role == Qt::DisplayRole)
        return map->getWorkingLevelLayers()[index.row()]->getTypeName().c_str();
    if (role == Qt::UserRole)
        return map->getWorkingLevelLayers()[index.row()]->isVisible();

    return {};
}

bool LayersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::UserRole)
    {
        map->setLayerVisible(index.row(), value.toBool());
        return true;
    }

    return false;
}

Qt::ItemFlags LayersModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags(index);
    flags               = flags & ~Qt::ItemIsSelectable;
    return flags;
}

void LayersModel::setLayersReference(MapperWidget* map)
{
    this->map = map;

    connect(map, &MapperWidget::layerVisibleChanged, this, [=](int index, bool visible) {
        auto i = this->index(index);
        dataChanged(i, i);
    });
}
