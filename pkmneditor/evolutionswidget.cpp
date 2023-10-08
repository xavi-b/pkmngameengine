#include "evolutionswidget.h"

EvolutionsWidget::EvolutionsWidget(QWidget* parent) : QWidget(parent)
{
    model                           = new EvolutionsModel;
    SortEvolutionsModel* proxyModel = new SortEvolutionsModel;
    proxyModel->sort(0, Qt::DescendingOrder);
    proxyModel->setSourceModel(model);

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);

    QTableView* tableView = new QTableView;
    tableView->setModel(proxyModel);
    vLayout->addWidget(tableView, 1);
    QPushButton* addButton = new QPushButton(tr("Add"));
    connect(addButton, &QPushButton::clicked, this, [=]() {
        EvolutionDialog d(this);
        if (d.exec() == QDialog::Accepted)
        {
            model->addEvolution(d.getEvolution());
            emit evolutionsChanged();
        }
    });
    vLayout->addWidget(addButton);
    QPushButton* removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        QModelIndex const& encountersIndex = tableView->selectionModel()->currentIndex();
        if (encountersIndex.isValid())
        {
            proxyModel->removeRow(encountersIndex.row());
            emit evolutionsChanged();
        }
    });
    vLayout->addWidget(removeButton);
    setLayout(vLayout);
}

std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> EvolutionsWidget::getEvolutions()
{
    return model->getEvolutions();
}

void EvolutionsWidget::setEvolutions(std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> newEvolutions)
{
    model->setEvolutions(newEvolutions);
}
