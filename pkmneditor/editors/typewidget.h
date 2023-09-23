#ifndef TYPEWIDGET_H
#define TYPEWIDGET_H

#include "type.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QWidget>

class TypeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TypeWidget(QWidget* parent = nullptr);

    void setType(Type::TypePtr const& newType);

signals:
    void idChanged();

private:
    Type::TypePtr type;

    QLineEdit* idLineEdit;
    QLineEdit* nameLineEdit;
};

#endif // TYPEWIDGET_H
