#ifndef ABILITYIDDIALOG_H
#define ABILITYIDDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>

class AbilityIdDialog : public QDialog
{
public:
    AbilityIdDialog(QStringList const& availableIds, QStringList const& excludeIds, QWidget* parent = nullptr);

    std::string getAbilityId() const;

private:
    QComboBox* abilityComboBox;
};

#endif // ABILITYIDDIALOG_H
