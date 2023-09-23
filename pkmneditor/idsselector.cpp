#include "idsselector.h"

IdsSelector::IdsSelector(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    listView = new QListView;
    listView->setSelectionMode(QAbstractItemView::MultiSelection);
    idsModel = new QStringListModel;
    listView->setModel(idsModel);
    layout->addWidget(listView);

    setLayout(layout);
}

void IdsSelector::setAvailableIds(QStringList const& newAvailableIds)
{
    QStringList selectedIds = getSelectedIds();
    idsModel->setStringList(newAvailableIds);
    setSelectedIds(selectedIds);
}

QStringList IdsSelector::getSelectedIds() const
{
    auto        indexes = listView->selectionModel()->selectedIndexes();
    QStringList selectedIds;
    for (auto const& index : indexes)
    {
        if (index.isValid())
        {
            selectedIds.append(index.data().toString());
        }
    }
    return selectedIds;
}

void IdsSelector::setSelectedIds(QStringList const& newSelectedIds)
{
    listView->selectionModel()->clear();
    for (auto const& s : newSelectedIds)
    {
        int row = idsModel->stringList().indexOf(s);
        if (row >= 0)
        {
            auto index = idsModel->index(row);
            listView->selectionModel()->select(index, QItemSelectionModel::Select);
        }
    }
}
