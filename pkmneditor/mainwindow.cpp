#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("Pkmns Editor");

    createMenus();

    typesView = new TypesView;

    QWidget*     w          = new QWidget;
    QVBoxLayout* vLayout    = new QVBoxLayout;
    QWidget*     gridWidget = new QWidget;
    QGridLayout* gridLayout = new QGridLayout;

    QPushButton* button1 = new QPushButton(tr("Types"));
    button1->setFixedSize(200, 150);
    connect(button1, &QPushButton::clicked, this, [=]() {
        stackedWidget->setCurrentWidget(typesView);
    });
    gridLayout->addWidget(button1, 0, 0);
    QPushButton* button2 = new QPushButton(tr("Abilities"));
    button2->setFixedSize(200, 150);
    gridLayout->addWidget(button2, 0, 1);
    QPushButton* button3 = new QPushButton(tr("Moves"));
    button3->setFixedSize(200, 150);
    gridLayout->addWidget(button3, 0, 2);
    QPushButton* button4 = new QPushButton(tr("Items"));
    button4->setFixedSize(200, 150);
    gridLayout->addWidget(button4, 1, 0);
    QPushButton* button5 = new QPushButton(tr("Pkmns"));
    button5->setFixedSize(200, 150);
    gridLayout->addWidget(button5, 1, 1);

    gridWidget->setLayout(gridLayout);
    vLayout->addWidget(gridWidget, 0, Qt::AlignCenter);
    w->setLayout(vLayout);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(w);

    connect(typesView, &TypesView::back, this, [=]() {
        stackedWidget->setCurrentIndex(0);
    });
    stackedWidget->addWidget(typesView);

    setCentralWidget(stackedWidget);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openFolderAct = new QAction(tr("Open &folder"));
    connect(openFolderAct, &QAction::triggered, this, [=]() {
        QString lastDir = QSettings().value("lastDir").toString();
        QString dirName = QFileDialog::getExistingDirectory(this, tr("Open folder"), lastDir);

        if (dirName.isEmpty())
            return;
        else
        {
            //            QFile file(fileName);

            //            if (!file.open(QIODevice::ReadOnly))
            //            {
            //                QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            //                return;
            //            }

            openedFileName = "";
            if (types)
                types->clear();

            // Types
            pt::ptree ptTypes;

            std::ifstream typesFile(QDir(dirName).absoluteFilePath("types.txt").toStdString());
            if (typesFile.is_open())
            {
                std::string       line;
                std::stringstream stream;

                while (getline(typesFile, line))
                {
                    if (line[0] != '#')
                        stream << line << '\n';
                }
                typesFile.close();

                pt::ini_parser::read_ini(stream, ptTypes);
                types = std::make_shared<std::vector<Type::TypePtr>>(Type::vectorFromPropertyTree(ptTypes));
                typesView->setTypes(types);
            }
            else
            {
                QMessageBox::warning(this, "Types file issue", "Unable to open file for reading");
            }

            stackedWidget->setCurrentIndex(0);
            QSettings().setValue("lastDir", dirName);
            QSettings().sync();
        }
    });
    QAction* openFileAct = new QAction(tr("&Open file"));
    openFileAct->setShortcuts(QKeySequence::Open);
    connect(openFileAct, &QAction::triggered, this, [=]() {
        QString lastDir  = QSettings().value("lastDir").toString();
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), lastDir, tr("JSON (*.json)"));

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
            js::value  json = js::parse(data.data());

            // Types
            types = std::make_shared<std::vector<Type::TypePtr>>(
                js::value_to<std::vector<Type::TypePtr>>(json.as_object()["types"]));
            typesView->setTypes(types);

            stackedWidget->setCurrentIndex(0);
            QSettings().setValue("lastDir", QFileInfo(fileName).dir().path());
            QSettings().sync();
        }
    });
    QAction* saveAct = new QAction(tr("&Save"));
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
    QAction* saveAsAct = new QAction(tr("Save &as"));
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, [=]() {
        if (QString fileName = saveFile(true); !fileName.isEmpty())
            openedFileName = fileName;
    });
    fileMenu->addAction(openFolderAct);
    fileMenu->addAction(openFileAct);
    fileMenu->addAction(saveAct);
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

        js::object json;

        // Types
        json["types"] = js::value_from(*(types.get()));

        file.write(js::serialize(json).c_str());
    }

    QSettings().setValue("lastDir", QFileInfo(fileName).dir().path());
    QSettings().sync();

    return fileName;
}
