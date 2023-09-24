#ifndef PKMNWIDGET_H
#define PKMNWIDGET_H

#include "pkmndef.h"

#include <QDir>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
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

    QLineEdit* idLineEdit;
    QLineEdit* nameLineEdit;
    QLabel*    sprite;

    QString spritesDirectory;
};

#endif // PKMNWIDGET_H
