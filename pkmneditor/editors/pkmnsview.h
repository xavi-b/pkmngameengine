#ifndef PKMNSVIEW_H
#define PKMNSVIEW_H

#include "../objectlistmodel.h"
#include "ability.h"
#include "movedef.h"
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

    void setPkmns(std::vector<PkmnDef::PkmnDefPtr> const& newPkmns);
    void setAvailableAbilities(std::vector<Ability::AbilityPtr> const& abilities);
    void setAvailableMoves(std::vector<MoveDef::MoveDefPtr> const& moves);
    void setAvailablePkmns(std::vector<PkmnDef::PkmnDefPtr> const& pkmns);
    void setSpritesDirectory(QString const& dirName);

signals:
    void back();

private:
    QSortFilterProxyModel* proxyModel;
    ObjectListModel*       model;

    QListView*  listView;
    PkmnWidget* pkmnWidget;

    std::vector<PkmnDef::PkmnDefPtr> pkmns;
};

#endif // PKMNSVIEW_H
