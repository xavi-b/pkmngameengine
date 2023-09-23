#ifndef TYPESVIEW_H
#define TYPESVIEW_H

#include "../objectlistmodel.h"
#include "typewidget.h"

#include <QInputDialog>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

class TypesView : public QWidget
{
    Q_OBJECT

public:
    explicit TypesView(QWidget* parent = nullptr);

    void setTypes(std::vector<Type::TypePtr> const& newTypes);

signals:
    void back();

private:
    QSortFilterProxyModel* proxyModel;
    ObjectListModel*       model;

    QListView*  listView;
    TypeWidget* typeWidget;

    std::vector<Type::TypePtr> types;
};

#endif // TYPESVIEW_H
