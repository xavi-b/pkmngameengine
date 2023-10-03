#ifndef PKMNWIDGET_H
#define PKMNWIDGET_H

#include "../movestolearnwidget.h"
#include "../statswidget.h"
#include "pkmndef.h"

#include <QComboBox>
#include <QDir>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

class PkmnWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PkmnWidget(QWidget* parent = nullptr);

    void setPkmn(PkmnDef::PkmnDefPtr const& pkmn);

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
    StatsWidget*        baseStatsWidget;
    StatsWidget*        EVsToLearnWidget;
    MovesToLearnWidget* movesToLearnWidget;

    QString spritesDirectory;
};

#endif // PKMNWIDGET_H
