#ifndef LAYERSMODEL_H
#define LAYERSMODEL_H

#include "mapperwidget.h"

#include <QAbstractListModel>

class LayersModel : public QAbstractListModel
{
public:
    explicit LayersModel(QObject* parent = nullptr);

    int           rowCount(QModelIndex const& parent = QModelIndex()) const;
    QVariant      data(QModelIndex const& index, int role = Qt::DisplayRole) const;
    bool          setData(QModelIndex const& index, QVariant const& value, int role);
    Qt::ItemFlags flags(QModelIndex const& index) const;

    void setLayersReference(MapperWidget* map);

private:
    MapperWidget* map;
};

#endif // LAYERSMODEL_H
