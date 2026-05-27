#ifndef MOVETOLEARNDIALOG_H
#define MOVETOLEARNDIALOG_H

#include "pkmndef.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>

class MoveToLearnDialog : public QDialog
{
public:
    MoveToLearnDialog(QStringList const& availableMoveIds,
                      QStringList const& excludeIds = {},
                      QWidget*           parent     = nullptr);

    PkmnDef::MoveToLearn getMoveToLearn() const;

private:
    QComboBox* moveIdComboBox;
    QSpinBox*  levelSpinBox;
};

#endif // MOVETOLEARNDIALOG_H
