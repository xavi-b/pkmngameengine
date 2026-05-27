#ifndef ABILITIESVIEW_H
#define ABILITIESVIEW_H

#include "../objectlistmodel.h"
#include "abilitywidget.h"

#include <QInputDialog>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

class AbilitiesView : public QWidget
{
    Q_OBJECT

public:
    explicit AbilitiesView(QWidget* parent = nullptr);

    void setAbilities(std::vector<Ability::AbilityPtr> const& newAbilities);

signals:
    void back();

private:
    QSortFilterProxyModel* proxyModel;
    ObjectListModel*       model;

    QListView*     listView;
    AbilityWidget* abilityWidget;
};

#endif // ABILITIESVIEW_H
