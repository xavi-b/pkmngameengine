#include "mainwindow.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QScrollArea>
#include "imageviewer.h"

#include <iostream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("Map Editor");

    QWidget*     w = new QWidget;
    QHBoxLayout* l = new QHBoxLayout;

    QString root = QDir::currentPath() + "/resources/Graphics";

    QFileSystemModel* model = new QFileSystemModel;
    model->setRootPath(root);
    QTreeView* treeView = new QTreeView;
    treeView->setModel(model);
    treeView->setRootIndex(model->index(root));
    for (int i = 1; i < model->columnCount(); ++i)
        treeView->hideColumn(i);
    l->addWidget(treeView);

    ImageViewer* imageArea = new ImageViewer;
    l->addWidget(imageArea);

    mapArea = new MapperEditor;
    l->addWidget(mapArea, 1);

    w->setLayout(l);
    setCentralWidget(w);

    connect(treeView, &QTreeView::clicked, this, [=](const QModelIndex& index) {
        QFileInfo info = model->fileInfo(index);
        if (info.suffix().compare("png", Qt::CaseInsensitive) == 0)
        {
            imageArea->setPixmap(QPixmap(info.filePath()));
        }
    });

    connect(mapArea->viewer(), &MapperViewer::entered, this, [=]() {
        mapArea->viewer()->contentWidget()->setPixmap(imageArea->contentWidget()->currentSelectionPixmap());
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    int index = event->key() - Qt::Key_0;
    if (index >= 0 && index < int(mapArea->viewer()->contentWidget()->getMap()->getTileLayers().size()))
    {
        if (event->modifiers() & Qt::ControlModifier)
            mapArea->viewer()->contentWidget()->setLayerVisible(
                index, !mapArea->viewer()->contentWidget()->isLayerVisible(index));
        else
            mapArea->viewer()->contentWidget()->setWorkingLayerIndex(index);
        event->accept();
        return;
    }
    event->ignore();
}
