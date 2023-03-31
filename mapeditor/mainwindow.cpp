#include "mainwindow.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include "imageviewer.h"

#include <iostream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("Map Editor");

    createMenus();

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

    connect(treeView, &QTreeView::clicked, this, [=](QModelIndex const& index) {
        QFileInfo info = model->fileInfo(index);
        if (info.suffix().compare("png", Qt::CaseInsensitive) == 0)
        {
            imageArea->setPixmap(info.filePath());
        }
    });

    connect(mapArea->viewer()->contentWidget(), &MapperWidget::entered, this, [=]() {
        mapArea->viewer()->contentWidget()->setSelectionPixmap(imageArea->contentWidget()->currentSelectionPixmap());
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    int index = event->key() - Qt::Key_0;
    if (index >= 0 && index < int(mapArea->viewer()->contentWidget()->getWorkingLevel()->getTileLayers().size()))
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

void MainWindow::createMenus()
{
    QMenu*   fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* newAct   = new QAction(tr("&New map"));
    newAct->setShortcuts(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, [=]() {
        openedFileName.clear();
        mapArea->viewer()->contentWidget()->swapMap(std::make_unique<Map>(5, 5));
    });
    QAction* openAct = new QAction(tr("&Open map"));
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open map"), "", tr("Map (*.pkmap)"));

        if (fileName.isEmpty())
            return;
        else
        {
            QFile file(fileName);

            if (!file.open(QIODevice::ReadOnly))
            {
                QMessageBox::information(this, tr("Unable to open file"), file.errorString());
                return;
            }

            openedFileName = fileName;
            QDataStream in(&file);
            QByteArray  data;
            in >> data;
            mapArea->viewer()->contentWidget()->swapMap(js::value_to<std::unique_ptr<Map>>(js::parse(data.data())));
        }
    });
    QAction* saveAct = new QAction(tr("&Save map"));
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
    QAction* saveAsAct = new QAction(tr("Save &as map"));
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, [=]() {
        if (QString fileName = saveFile(true); !fileName.isEmpty())
            openedFileName = fileName;
    });
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
}

QString MainWindow::saveFile(bool saveAs)
{
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save map"), saveAs ? "" : openedFileName, tr("Map (*.pkmap)"));

    if (fileName.isEmpty())
        return {};
    else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return {};
        }

        QDataStream out(&file);
        out << QByteArray(serialize(js::value_from(mapArea->viewer()->contentWidget()->getMap())).c_str());
    }

    return fileName;
}
