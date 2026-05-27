#include "abilityidswidget.h"

#include "abilityiddialog.h"

#include <QPushButton>
#include <QVBoxLayout>

AbilityIdsWidget::AbilityIdsWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);

    model = new QStringListModel(this);

    QListView* listView = new QListView;
    listView->setModel(model);
    vLayout->addWidget(listView, 1);

    QPushButton* addButton = new QPushButton(tr("Add"));
    connect(addButton, &QPushButton::clicked, this, [=]() {
        AbilityIdDialog d(availableIds, model->stringList(), this);
        if (d.exec() == QDialog::Accepted)
        {
            QStringList ids = model->stringList();
            ids.append(QString::fromStdString(d.getAbilityId()));
            model->setStringList(ids);
            emit abilityIdsChanged();
        }
    });
    vLayout->addWidget(addButton);

    QPushButton* removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        QModelIndex const& index = listView->selectionModel()->currentIndex();
        if (!index.isValid())
            return;

        QStringList ids = model->stringList();
        ids.removeAt(index.row());
        model->setStringList(ids);
        emit abilityIdsChanged();
    });
    vLayout->addWidget(removeButton);

    setLayout(vLayout);
}

std::vector<std::string> AbilityIdsWidget::getAbilityIds() const
{
    std::vector<std::string> abilityIds;
    for (auto const& id : model->stringList())
        abilityIds.push_back(id.toStdString());
    return abilityIds;
}

void AbilityIdsWidget::setAbilityIds(std::vector<std::string> const& abilityIds)
{
    QStringList ids;
    for (auto const& id : abilityIds)
        ids.append(id.c_str());
    model->setStringList(ids);
}

void AbilityIdsWidget::setAvailableIds(QStringList const& newAvailableIds)
{
    availableIds = newAvailableIds;
}
