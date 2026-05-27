#ifndef ABILITYWIDGET_H
#define ABILITYWIDGET_H

#include "ability.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>

class AbilityWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AbilityWidget(QWidget* parent = nullptr);

    void setAbility(Ability::AbilityPtr const& newAbility);

signals:
    void idChanged();

private:
    Ability::AbilityPtr ability;

    QLineEdit* idLineEdit;
    QLineEdit* nameLineEdit;
    QTextEdit* descriptionTextEdit;
};

#endif // ABILITYWIDGET_H
