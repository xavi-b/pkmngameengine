#ifndef PKMNSVIEW_H
#define PKMNSVIEW_H

#include "../objectlistmodel.h"
#include "pkmnwidget.h"

#include <QInputDialog>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

class PkmnsView : public QWidget
{
    Q_OBJECT

public:
    explicit PkmnsView(QWidget* parent = nullptr);

    void setPkmns(std::vector<Pkmn::PkmnPtr> const& newPkmns);
    void setSpritesDirectory(QString const& dirName);

signals:
    void back();

private:
    QSortFilterProxyModel* proxyModel;
    ObjectListModel*       model;

    QListView*  listView;
    PkmnWidget* pkmnWidget;

    std::vector<Pkmn::PkmnPtr> pkmns;

    QString spritesDirectory;
};

#endif // PKMNSVIEW_H
