#include "mainwindow.h"

#include <QFileDialog>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QTreeView>
#include <iostream>

#include "version.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("Map Editor");

    createMenus();

    QWidget*     w = new QWidget;
    QHBoxLayout* l = new QHBoxLayout;
#ifdef NDEBUG // not in debug
    QString root = QDir::currentPath() + "/resources/Graphics";
#else
    QString root(pkmng::resources_path);
#endif
    QFileSystemModel* model = new QFileSystemModel;
    model->setRootPath(root);
    QTreeView* treeView = new QTreeView;
    treeView->setModel(model);
    treeView->setRootIndex(model->index(root));
    for (int i = 1; i < model->columnCount(); ++i)
        treeView->hideColumn(i);
    l->addWidget(treeView);

    imageViewer = new ImageViewer;
    l->addWidget(imageViewer);

    mapEditor = new MapperEditor;
    l->addWidget(mapEditor, 1);

    w->setLayout(l);
    setCentralWidget(w);

    connect(treeView->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            [=](QModelIndex const& current, QModelIndex const& /*previous*/) {
                QFileInfo info = model->fileInfo(current);
                if (info.suffix().compare("png", Qt::CaseInsensitive) == 0)
                {
                    imageViewer->setPixmap(info.filePath());
                }
            });

    connect(imageViewer->contentWidget(), &ImageWidget::currentSelectionChanged, this, [=]() {
        mapEditor->getMapperViewer()->contentWidget()->setSelectionPixmap(
            imageViewer->contentWidget()->currentSelectionPixmap());
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        mapEditor->getMapperViewer()->contentWidget()->setSelectionPixmap({});
        event->accept();
        return;
    }
    event->ignore();
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    int index = event->key() - Qt::Key_0;
    if (index >= 0
        && index < int(mapEditor->getMapperViewer()->contentWidget()->getWorkingLevel()->getTileLayers().size()))
    {
        if (event->modifiers() & Qt::ControlModifier)
            mapEditor->getMapperViewer()->contentWidget()->setLayerVisible(
                index,
                !mapEditor->getMapperViewer()->contentWidget()->isLayerVisible(index));
        else
            mapEditor->getMapperViewer()->contentWidget()->setWorkingLayerIndex(index);
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
        mapEditor->swapMap(std::make_unique<Map>(5, 5));
    });
    QAction* openAct = new QAction(tr("&Open map"));
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, [=]() {
        QString lastDir  = QSettings().value("lastDir").toString();
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open map"), lastDir, tr("Map (*.pkmap)"));

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

            openedFileName  = fileName;
            QByteArray data = file.readAll();
            mapEditor->swapMap(js::value_to<std::unique_ptr<Map>>(js::parse(data.data())));

            QSettings().setValue("lastDir", QFileInfo(fileName).dir().path());
            QSettings().sync();
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
    QString fileName = openedFileName;

    if (saveAs || openedFileName.isEmpty())
    {
        fileName =
            QFileDialog::getSaveFileName(this, tr("Save map"), saveAs ? "" : openedFileName, tr("Map (*.pkmap)"));
    }

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

        auto& map = mapEditor->getMapperViewer()->contentWidget()->getMap();
        map->setEncounterMethods(mapEditor->getEncountersEditor()->getEncounterMethods());
        file.write(js::serialize(js::value_from(map)).c_str());
    }

    QSettings().setValue("lastDir", QFileInfo(fileName).dir().path());
    QSettings().sync();

    return fileName;
}
