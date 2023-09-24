#ifndef LEVELSMODEL_H
#define LEVELSMODEL_H

#include "mapperwidget.h"

#include <QAbstractListModel>

class LevelsModel : public QAbstractListModel
{
public:
    explicit LevelsModel(QObject* parent = nullptr);

    int           rowCount(QModelIndex const& parent = QModelIndex()) const;
    QVariant      data(QModelIndex const& index, int role = Qt::DisplayRole) const;
    bool          setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole);
    Qt::ItemFlags flags(QModelIndex const& index) const;

    void setLevelsReference(MapperWidget* map);

private:
    MapperWidget* map;
};

#endif // LEVELSMODEL_H
