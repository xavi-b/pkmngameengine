#ifndef ENCOUNTERDIALOG_H
#define ENCOUNTERDIALOG_H

#include "encounter.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

class EncounterDialog : public QDialog
{
public:
    EncounterDialog(QWidget* parent = nullptr);

    Encounter getEncounter() const;

private:
    QLineEdit* pkmnIdLineEdit;
    QSpinBox*  percentageSpinBox;
    QSpinBox*  levelMinSpinBox;
    QSpinBox*  levelMaxSpinBox;
};

#endif // ENCOUNTERDIALOG_H
