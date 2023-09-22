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

    listView = new QListView;
    splitter->addWidget(listView);
    splitter->setStretchFactor(1, 1);

    typeWidget = new TypeWidget;
    splitter->addWidget(typeWidget);

    vLayout->addWidget(splitter, 1);

    setLayout(vLayout);

    model = new ObjectListModel(this);
    listView->setModel(model);
}

void TypesView::setTypes(std::shared_ptr<std::vector<Type::TypePtr>> newTypes)
{
    model->setObjects(newTypes);
}
