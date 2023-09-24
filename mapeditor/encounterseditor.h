#ifndef ENCOUNTERSEDITOR_H
#define ENCOUNTERSEDITOR_H

#include "encounterdialog.h"
#include "encountermethod.h"
#include "encountersmodel.h"

#include <QFormLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QWidget>

class EncountersEditor : public QWidget
{
    Q_OBJECT

public:
    explicit EncountersEditor(QWidget* parent = nullptr);

    std::vector<EncounterMethod> getEncounterMethods() const;
    void                         setEncounterMethods(std::vector<EncounterMethod> const& newEncounterMethods);

private:
    EncounterMethod* currentEncounterMethod(EncounterMethod::Type type);

    QListView* encounterMethodsListView;

    std::vector<EncounterMethod> encounterMethods;
};

#endif // ENCOUNTERSEDITOR_H
