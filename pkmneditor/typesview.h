#ifndef TYPESVIEW_H
#define TYPESVIEW_H

#include "objectlistmodel.h"
#include "typewidget.h"

#include <QListView>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

class TypesView : public QWidget
{
    Q_OBJECT

public:
    explicit TypesView(QWidget* parent = nullptr);

    void setTypes(std::shared_ptr<std::vector<Type::TypePtr>> newTypes);

signals:
    void back();

private:
    ObjectListModel* model;

    QListView*  listView;
    TypeWidget* typeWidget;

    std::shared_ptr<std::vector<Type::TypePtr>> types;
};

#endif // TYPESVIEW_H
