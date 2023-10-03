#include "movestolearnwidget.h"

MovesToLearnWidget::MovesToLearnWidget(QWidget* parent) : QWidget(parent)
{
    movesToLearnModel                 = new MovesToLearnModel;
    SortMovesToLearnModel* proxyModel = new SortMovesToLearnModel;
    proxyModel->sort(0, Qt::DescendingOrder);
    proxyModel->setSourceModel(movesToLearnModel);

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);

    QListView* encountersListView = new QListView;
    encountersListView->setModel(proxyModel);
    vLayout->addWidget(encountersListView, 1);
    QPushButton* addButton = new QPushButton(tr("Add"));
    connect(addButton, &QPushButton::clicked, this, [=]() {
        MoveToLearnDialog d(this);
        if (d.exec() == QDialog::Accepted)
        {
            movesToLearnModel->addMoveToLearn(d.getMoveToLearn());
            emit movesToLearnChanged();
        }
    });
    vLayout->addWidget(addButton);
    QPushButton* removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        QModelIndex const& encountersIndex = encountersListView->selectionModel()->currentIndex();
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
    return movesToLearnModel->getMovesToLearn();
}

void MovesToLearnWidget::setMovesToLearn(std::vector<PkmnDef::MoveToLearn> newMovesToLearn)
{
    movesToLearnModel->setMovesToLearn(newMovesToLearn);
}
