#ifndef TYPEWIDGET_H
#define TYPEWIDGET_H

#include "../idsselector.h"
#include "type.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

class TypeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TypeWidget(QWidget* parent = nullptr);

    void setAvailableTypes(std::vector<Type::TypePtr> const& types);
    void setType(Type::TypePtr const& type);

signals:
    void idChanged();

private:
    Type::TypePtr type;

    QLineEdit* idLineEdit;
    QLineEdit* nameLineEdit;

    IdsSelector* weaknessesSelector;
    IdsSelector* resistancesSelector;
    IdsSelector* immunitiesSelector;
};

#endif // TYPEWIDGET_H
