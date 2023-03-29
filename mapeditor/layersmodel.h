#ifndef LAYERSMODEL_H
#define LAYERSMODEL_H

#include <QAbstractListModel>
#include "mapperwidget.h"

class LayersModel : public QAbstractListModel
{
public:
    explicit LayersModel(QObject* parent = nullptr);

    int           rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool          setData(const QModelIndex& index, const QVariant& value, int role);
    Qt::ItemFlags flags(const QModelIndex& index) const;

    void setLayersReference(MapperWidget* map);

private:
    MapperWidget* map;
};

#endif // LAYERSMODEL_H
