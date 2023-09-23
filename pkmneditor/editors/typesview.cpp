#include "typesview.h"

TypesView::TypesView(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout;

    QPushButton* backButton = new QPushButton(tr("Back"));
    connect(backButton, &QPushButton::clicked, this, &TypesView::back);
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
            QInputDialog::getText(this, tr("New Type"), tr("Enter type ID:"), QLineEdit::Normal, QString(), &ok);
        if (ok)
        {
            auto type = std::make_shared<Type>();
            type->setId(id.toStdString());
            model->appendItem(type);
        }
    });
    leftLayout->addWidget(addButton);

    QPushButton* removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        QModelIndex const& index         = listView->selectionModel()->currentIndex();
        QModelIndex const& originalIndex = proxyModel->mapToSource(index);
        model->removeItem(originalIndex.row());
    });
    leftLayout->addWidget(removeButton);

    QWidget* leftWidget = new QWidget;
    leftWidget->setLayout(leftLayout);
    splitter->addWidget(leftWidget);

    typeWidget = new TypeWidget;
    splitter->addWidget(typeWidget);
    splitter->setStretchFactor(1, 1);

    vLayout->addWidget(splitter, 1);

    setLayout(vLayout);

    model      = new ObjectListModel(this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSortRole(Qt::InitialSortOrderRole);
    proxyModel->setSourceModel(model);
    listView->setModel(proxyModel);

    connect(listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [=](QModelIndex const& index) {
        QModelIndex const&      originalIndex = proxyModel->mapToSource(index);
        TemplateListItem<Type>* item          = dynamic_cast<TemplateListItem<Type>*>(model->getItem(originalIndex));
        if (item)
        {
            auto type = item->getPtr();
            typeWidget->setType(type);
        }
    });

    connect(typeWidget, &TypeWidget::idChanged, this, [=]() {
        QModelIndex const& index         = listView->selectionModel()->currentIndex();
        QModelIndex const& originalIndex = proxyModel->mapToSource(index);
        emit               model->dataChanged(originalIndex, originalIndex);
    });
}

void TypesView::setTypes(std::vector<Type::TypePtr> const& newTypes)
{
    model->setObjects(newTypes);
}
