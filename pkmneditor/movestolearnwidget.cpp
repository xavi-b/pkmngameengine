#include "movestolearnwidget.h"

MovesToLearnWidget::MovesToLearnWidget(QWidget* parent) : QWidget(parent)
{
    model                             = new MovesToLearnModel;
    SortMovesToLearnModel* proxyModel = new SortMovesToLearnModel;
    proxyModel->sort(0, Qt::DescendingOrder);
    proxyModel->setSourceModel(model);

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);

    QListView* listView = new QListView;
    listView->setModel(proxyModel);
    vLayout->addWidget(listView, 1);
    QPushButton* addButton = new QPushButton(tr("Add"));
    connect(addButton, &QPushButton::clicked, this, [=]() {
        MoveToLearnDialog d(this);
        if (d.exec() == QDialog::Accepted)
        {
            model->addMoveToLearn(d.getMoveToLearn());
            emit movesToLearnChanged();
        }
    });
    vLayout->addWidget(addButton);
    QPushButton* removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        QModelIndex const& encountersIndex = listView->selectionModel()->currentIndex();
        if (encountersIndex.isValid())
        {
            proxyModel->removeRow(encountersIndex.row());
            emit movesToLearnChanged();
        }
    });
    vLayout->addWidget(removeButton);
    setLayout(vLayout);
}

std::vector<PkmnDef::MoveToLearn> MovesToLearnWidget::getMovesToLearn()
{
    return model->getMovesToLearn();
}

void MovesToLearnWidget::setMovesToLearn(std::vector<PkmnDef::MoveToLearn> newMovesToLearn)
{
    model->setMovesToLearn(newMovesToLearn);
}
