#include "mainwindow.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QScrollArea>
#include "imageviewer.h"

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

    QScrollArea* mapArea = new QScrollArea;
    l->addWidget(mapArea, 1);

    w->setLayout(l);
    setCentralWidget(w);

    connect(treeView, &QTreeView::clicked, this, [=](const QModelIndex& index) {
        QFileInfo info = model->fileInfo(index);
        if (info.suffix().compare("png", Qt::CaseInsensitive) == 0)
        {
            imageArea->setImage(QImage(info.filePath()));
        }
    });
}

MainWindow::~MainWindow()
{
}
