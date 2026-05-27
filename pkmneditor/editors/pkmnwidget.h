#ifndef PKMNWIDGET_H
#define PKMNWIDGET_H

#include "../abilityidswidget.h"
#include "../evolutionswidget.h"
#include "../movestolearnwidget.h"
#include "../statswidget.h"
#include "ability.h"
#include "movedef.h"
#include "pkmndef.h"

#include <QComboBox>
#include <QDir>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QWidget>

class PkmnWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PkmnWidget(QWidget* parent = nullptr);

    void setPkmn(PkmnDef::PkmnDefPtr const& pkmn);

    void setAvailableAbilities(std::vector<Ability::AbilityPtr> const& abilities);
    void setAvailableMoves(std::vector<MoveDef::MoveDefPtr> const& moves);
    void setAvailablePkmns(std::vector<PkmnDef::PkmnDefPtr> const& pkmns);

    void setSpritesDirectory(QString const& dirName);

signals:
    void idChanged();

private:
    PkmnDef::PkmnDefPtr pkmn;

    QLineEdit*          idLineEdit;
    QLineEdit*          nameLineEdit;
    QLabel*             sprite;
    QComboBox*          growthRateComboBox;
    QSpinBox*           baseExpSpinBox;
    QSpinBox*           catchRateSpinBox;
    QSpinBox*           happinessSpinBox;
    StatsWidget*        baseStatsWidget;
    StatsWidget*        EVsToLearnWidget;
    AbilityIdsWidget*   abilitiesWidget;
    AbilityIdsWidget*   hiddenAbilitiesWidget;
    MovesToLearnWidget* movesToLearnWidget;
    EvolutionsWidget*   evolutionsWidget;

    QStringList availableAbilityIds;

    QString spritesDirectory;
};

#endif // PKMNWIDGET_H
