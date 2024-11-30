#include "itemsview.h"

ItemsView::ItemsView(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout;

    QPushButton* backButton = new QPushButton(tr("Back"));
    connect(backButton, &QPushButton::clicked, this, &ItemsView::back);
    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(backButton);
    hLayout->addStretch(1);
    vLayout->addLayout(hLayout);

    QSplitter* splitter = new QSplitter;
    splitter->setChildrenCollapsible(false);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton* sortButton = new QPushButton(tr("Sort"));
    connect(sortButton, &QPushButton::clicked, this, [=]() {
        if (proxyModel->sortRole() == Qt::InitialSortOrderRole)
        {
            proxyModel->setSortRole(Qt::DisplayRole);
            proxyModel->sort(0);
        }
        else
        {
            proxyModel->setSortRole(Qt::InitialSortOrderRole);
            proxyModel->invalidate();
        }
    });
    leftLayout->addWidget(sortButton);

    listView = new QListView;
    leftLayout->addWidget(listView, 1);

    QPushButton* addButton = new QPushButton(tr("Add"));
    connect(addButton, &QPushButton::clicked, this, [=]() {
        bool    ok = false;
        QString id =
            QInputDialog::getText(this, tr("New Item"), tr("Enter item ID:"), QLineEdit::Normal, QString(), &ok);
        if (ok)
        {
            auto item = std::make_shared<ItemDef>();
            item->setId(id.toStdString());
            model->appendItem(item);
        }
    });
    leftLayout->addWidget(addButton);

    QPushButton* removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        QModelIndex const& index = listView->selectionModel()->currentIndex();
        if (index.isValid())
        {
            QModelIndex const& originalIndex = proxyModel->mapToSource(index);
            model->removeItem(originalIndex.row());
        }
    });
    leftLayout->addWidget(removeButton);

    QWidget* leftWidget = new QWidget;
    leftWidget->setLayout(leftLayout);
    splitter->addWidget(leftWidget);

    itemsWidget = new ItemsWidget;
    splitter->addWidget(itemsWidget);
    splitter->setStretchFactor(1, 1);

    vLayout->addWidget(splitter, 1);

    setLayout(vLayout);

    model      = new ObjectListModel(this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSortRole(Qt::InitialSortOrderRole);
    proxyModel->setSourceModel(model);
    listView->setModel(proxyModel);

    connect(listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [=](QModelIndex const& index) {
        if (index.isValid())
        {
            QModelIndex const&         originalIndex = proxyModel->mapToSource(index);
            TemplateListItem<ItemDef>* item = dynamic_cast<TemplateListItem<ItemDef>*>(model->getItem(originalIndex));
            if (item)
            {
                auto itemDef = item->getPtr();
                itemsWidget->setAvailableItems(model->getObjects<ItemDef>());
                itemsWidget->setItem(itemDef);
            }
        }
    });

    connect(itemsWidget, &ItemsWidget::idChanged, this, [=]() {
        QModelIndex const& index = listView->selectionModel()->currentIndex();
        if (index.isValid())
        {
            QModelIndex const& originalIndex = proxyModel->mapToSource(index);
            emit               model->dataChanged(originalIndex, originalIndex);
        }
    });
}

void ItemsView::setItems(std::vector<ItemDef::ItemDefPtr> const& newItems)
{
    model->setObjects(newItems);
}

void ItemsView::setAvailableMoves(std::vector<MoveDef::MoveDefPtr> const& moves)
{
    itemsWidget->setAvailableMoves(moves);
}

void ItemsView::setSpritesDirectory(QString const& dirName)
{
    itemsWidget->setSpritesDirectory(dirName);
}
