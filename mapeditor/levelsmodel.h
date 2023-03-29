#ifndef LEVELSMODEL_H
#define LEVELSMODEL_H

#include <QAbstractListModel>
#include "mapperwidget.h"

class LevelsModel : public QAbstractListModel
{
public:
    explicit LevelsModel(QObject* parent = nullptr);

    int           rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool          setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex& index) const;

    void setLevelsReference(MapperWidget* map);

private:
    MapperWidget* map;
};

#endif // LEVELSMODEL_H
