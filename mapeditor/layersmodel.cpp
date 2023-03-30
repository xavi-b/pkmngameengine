#include "layersmodel.h"

LayersModel::LayersModel(QObject* parent) : QAbstractListModel{parent}
{
}

int LayersModel::rowCount(QModelIndex const& parent) const
{
    Q_UNUSED(parent)

    if (!map)
        return 0;

    return map->getWorkingLevelLayers().size();
}

QVariant LayersModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
        return {};

    if (role == Qt::DisplayRole)
        return map->getWorkingLevelLayers()[index.row()]->getTypeName().c_str();
    if (role == Qt::UserRole)
        return map->getWorkingLevelLayers()[index.row()]->isVisible();

    return {};
}

bool LayersModel::setData(QModelIndex const& index, QVariant const& value, int role)
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

Qt::ItemFlags LayersModel::flags(QModelIndex const& index) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags(index);
    flags               = flags & ~Qt::ItemIsSelectable;
    return flags;
}

void LayersModel::setLayersReference(MapperWidget* map)
{
    this->map = map;

    connect(map, &MapperWidget::layerVisibleChanged, this, [=](int index, bool visible) {
        Q_UNUSED(visible)
        auto i = this->index(index);
        dataChanged(i, i);
    });
}
