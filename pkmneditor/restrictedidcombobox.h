#ifndef RESTRICTEDIDCOMBOBOX_H
#define RESTRICTEDIDCOMBOBOX_H

#include <QStringList>

class QComboBox;
class QDialog;
class QDialogButtonBox;

QStringList selectableIdsFrom(QStringList const& availableIds, QStringList const& excludeIds);

void configureRestrictedIdComboBox(QComboBox*         comboBox,
                                   QStringList const& availableIds,
                                   QStringList const& excludeIds = {});

bool isRestrictedIdComboBoxValid(QComboBox const* comboBox);

void wireRestrictedIdComboBox(QComboBox* comboBox, QDialogButtonBox* buttonBox, QDialog* dialog);

#endif // RESTRICTEDIDCOMBOBOX_H
