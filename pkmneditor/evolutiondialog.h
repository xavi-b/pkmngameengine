#ifndef EVOLUTIONDIALOG_H
#define EVOLUTIONDIALOG_H

#include "pkmndef.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>

class EvolutionDialog : public QDialog
{
public:
    EvolutionDialog(QWidget* parent = nullptr);

    std::pair<PkmnDef::EvolutionType, PkmnDef::Evolution> getEvolution() const;

private:
    void onTypeChanged();

    QLineEdit*     pkmnIdLineEdit;
    QComboBox*     typeComboBox;
    QLineEdit*     dataLineEdit;
    QIntValidator* intValidator;
};

#endif // EVOLUTIONDIALOG_H
