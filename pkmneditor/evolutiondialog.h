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
    EvolutionDialog(QStringList const& availablePkmnIds, QStringList const& excludeIds = {}, QWidget* parent = nullptr);

    std::pair<PkmnDef::EvolutionType, PkmnDef::Evolution> getEvolution() const;

private:
    void onTypeChanged();

    QComboBox*     pkmnIdComboBox;
    QComboBox*     typeComboBox;
    QLineEdit*     dataLineEdit;
    QIntValidator* intValidator;
};

#endif // EVOLUTIONDIALOG_H
