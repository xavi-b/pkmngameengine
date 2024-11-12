#include "mainwindow.h"

#include "version.h"

#include <QFileDialog>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QTreeView>
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

    createMenus();
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
    auto quitAct = new QAction(tr("&Quit"));
    quitAct->setShortcut(QKeySequence::Quit);
    connect(quitAct, &QAction::triggered, this, [this]() {
        close();
    });

    auto exportAsImageAct = new QAction(tr("&Image"));
    connect(exportAsImageAct, &QAction::triggered, this, [this]() {
        static QStringList imgExt{"png", "jpg"};

        QStringList filters;
        std::transform(std::begin(imgExt), std::end(imgExt), std::back_inserter(filters), [](QString const& b) {
            return QString("%1 Image (*.%2);").arg(b.toUpper(), b);
        });

        QFileDialog fileDlg(this, tr("Export as Image"), QSettings().value("lastDir").toString());
        fileDlg.setAcceptMode(QFileDialog::AcceptSave);
        fileDlg.setNameFilters(filters);

        if (QDialog::Accepted == fileDlg.exec())
        {
            auto files          = fileDlg.selectedFiles();
            auto selectedFilter = fileDlg.selectedNameFilter();

            auto idx = filters.indexOf(selectedFilter);

            if (files.size() > 1 || files.isEmpty())
                return;

            auto pattern  = QString(".%1").arg(imgExt[idx]);
            auto fileName = files[0];

            if (!fileName.endsWith(pattern))
                fileName += pattern;

            auto pix = mapEditor->getMapperViewer()->grabInternalImage();

            if (!pix.save(fileName))
                QMessageBox::warning(this, tr("Image Error!"), tr("Image cannot be saved at %1").arg(fileName));
        }
    });

    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    auto exportMenu = fileMenu->addMenu(tr("&Export"));
    exportMenu->addAction(exportAsImageAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    QMenu*   editMenu = menuBar()->addMenu(tr("&Edit"));
    QAction* undoAct  = new QAction(tr("&Undo"));
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setEnabled(false);
    connect(undoAct, &QAction::triggered, this, [=]() {
        mapEditor->getMapperViewer()->contentWidget()->commandUndo();
    });
    connect(mapEditor->getMapperViewer()->contentWidget(), &MapperWidget::commandsIndexChanged, this, [=]() {
        undoAct->setEnabled(mapEditor->getMapperViewer()->contentWidget()->getCommandsIndex() > 0);
    });
    QAction* redoAct = new QAction(tr("&Redo"));
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setEnabled(false);
    connect(redoAct, &QAction::triggered, this, [=]() {
        mapEditor->getMapperViewer()->contentWidget()->commandRedo();
    });
    connect(mapEditor->getMapperViewer()->contentWidget(), &MapperWidget::commandsIndexChanged, this, [=]() {
        redoAct->setEnabled(mapEditor->getMapperViewer()->contentWidget()->getCommandsIndex()
                            < mapEditor->getMapperViewer()->contentWidget()->getCommandsHistorySize());
    });
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (isWindowModified())
    {
        saveFile();
    }

    if (QMessageBox::StandardButton::No
        == QMessageBox::question(this, tr("Quit Application"), tr("Would you really want to close the application ?")))
    {
        event->ignore();
    }
    else
    {
        event->accept();
    }
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
