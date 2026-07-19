#ifndef MOVEWIDGET_H
#define MOVEWIDGET_H

#include "movedef.h"
#include "type.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QTextEdit>
#include <QWidget>

class MoveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MoveWidget(QWidget* parent = nullptr);

    void setAvailableTypes(std::vector<Type::TypePtr> const& types);
    void setMove(MoveDef::MoveDefPtr const& newMove);

signals:
    void idChanged();

private:
    void syncFlagsFromUi();

    MoveDef::MoveDefPtr move;

    QLineEdit*   idLineEdit;
    QLineEdit*   nameLineEdit;
    QComboBox*   typeComboBox;
    QComboBox*   categoryComboBox;
    QSpinBox*    powerSpinBox;
    QSpinBox*    accuracySpinBox;
    QSpinBox*    totalPPSpinBox;
    QComboBox*   functionCodeComboBox;
    QSpinBox*    effectChanceSpinBox;
    QComboBox*   targetComboBox;
    QListWidget* flagsListWidget;
    QTextEdit*   descriptionTextEdit;
};

#endif // MOVEWIDGET_H
