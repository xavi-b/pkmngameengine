#ifndef ITEMSVIEW_H
#define ITEMSVIEW_H

#include "../objectlistmodel.h"
#include "itemswidget.h"
#include "movedef.h"

#include <QInputDialog>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

class ItemsView : public QWidget
{
    Q_OBJECT

public:
    explicit ItemsView(QWidget* parent = nullptr);

    void setItems(std::vector<ItemDef::ItemDefPtr> const& newItems);

    void setAvailableMoves(std::vector<MoveDef::MoveDefPtr> const& moves);

    void setSpritesDirectory(QString const& dirName);

signals:
    void back();

private:
    QSortFilterProxyModel* proxyModel;
    ObjectListModel*       model;

    QListView*   listView;
    ItemsWidget* itemsWidget;
};

#endif // ITEMSVIEW_H
