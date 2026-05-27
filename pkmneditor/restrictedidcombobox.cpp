#include "restrictedidcombobox.h"

#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

QStringList selectableIdsFrom(QStringList const& availableIds, QStringList const& excludeIds)
{
    QStringList selectableIds;
    for (auto const& id : availableIds)
    {
        if (!excludeIds.contains(id))
            selectableIds.append(id);
    }
    return selectableIds;
}

void configureRestrictedIdComboBox(QComboBox* comboBox, QStringList const& availableIds, QStringList const& excludeIds)
{
    comboBox->setEditable(true);
    comboBox->setInsertPolicy(QComboBox::NoInsert);

    QStringList const selectableIds = selectableIdsFrom(availableIds, excludeIds);
    for (auto const& id : selectableIds)
        comboBox->addItem(id);

    auto* completer = new QCompleter(selectableIds, comboBox);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    comboBox->setCompleter(completer);
}

bool isRestrictedIdComboBoxValid(QComboBox const* comboBox)
{
    QString const text = comboBox->currentText().trimmed();
    return !text.isEmpty() && comboBox->findText(text, Qt::MatchExactly) >= 0;
}

void wireRestrictedIdComboBox(QComboBox* comboBox, QDialogButtonBox* buttonBox, QDialog* dialog)
{
    QPushButton* okButton        = buttonBox->button(QDialogButtonBox::Ok);
    auto         updateOkEnabled = [=]() {
        okButton->setEnabled(isRestrictedIdComboBoxValid(comboBox));
    };
    QObject::connect(comboBox, &QComboBox::currentTextChanged, dialog, updateOkEnabled);
    updateOkEnabled();
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, [comboBox, dialog]() {
        if (isRestrictedIdComboBoxValid(comboBox))
            dialog->accept();
    });
}
