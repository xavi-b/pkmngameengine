#ifndef MOVESVIEW_H
#define MOVESVIEW_H

#include "../objectlistmodel.h"
#include "movewidget.h"

#include <QInputDialog>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

class MovesView : public QWidget
{
    Q_OBJECT

public:
    explicit MovesView(QWidget* parent = nullptr);

    void setMoves(std::vector<MoveDef::MoveDefPtr> const& newMoves);
    void setAvailableTypes(std::vector<Type::TypePtr> const& types);

signals:
    void back();

private:
    QSortFilterProxyModel* proxyModel;
    ObjectListModel*       model;

    QListView*  listView;
    MoveWidget* moveWidget;
};

#endif // MOVESVIEW_H
