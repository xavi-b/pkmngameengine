#ifndef MOVETOLEARNDIALOG_H
#define MOVETOLEARNDIALOG_H

#include "pkmndef.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

class MoveToLearnDialog : public QDialog
{
public:
    MoveToLearnDialog(QWidget* parent = nullptr);

    PkmnDef::MoveToLearn getMoveToLearn() const;

private:
    QLineEdit* moveIdLineEdit;
    QSpinBox*  levelSpinBox;
};

#endif // MOVETOLEARNDIALOG_H
